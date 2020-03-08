/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 *  ======== app.c ========
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../af.h"
#include "app.h"
#include "../codec_dsp/afmath.h"

#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>

#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <ti/sdo/ce/universal/universal.h>

#define MAXSERVERNAMELEN 128
#define MAXVERSIONSIZE 128

/*
 * If an XDAIS algorithm _may_ use DMA, buffers provided to it need to be
 * aligned on a cache boundary.
 */

#ifdef CACHE_ENABLED

/*
 * If buffer alignment isn't set on the compiler's command line, set it here
 * to a default value.
 */
#ifndef BUFALIGN
#define BUFALIGN 128
#endif
#else

/* Not a cached system, no buffer alignment constraints */
#define BUFALIGN Memory_DEFAULTALIGNMENT

#endif

//#define LOG

typedef struct ProcMap 
{
    String procName;  /* Command line name of proc to load server (eg "dsp") */
    String multiProcName; /* MultiProc name (eg "DSP") */
} ProcMap;

ProcMap procMap[] = 
{
    {"dsp", "DSP"}
};


static String universalName  = "af_proc";

int gFrames;
int gFlag;

rdata_t* rdata = NULL;

int sockfd;
struct sockaddr_in servaddr, cliaddr;
char* ip_log;
int port_log;

void signal_handler(int signum)
{
    fprintf(stderr, "signal_handler: signal %d\n", signum);
    udpCloseRecv(rdata);
    udpCloseSend();
    gFlag = 1;
    //exit(signum);
}

UNIVERSAL_Handle	  		hUniversal;
//UNIVERSAL_InArgs            universalInArgs;
//UNIVERSAL_OutArgs           universalOutArgs;
LW_AF_InArgs				lwafInArgs;
LW_AF_OutArgs				lwafOutArgs;
UNIVERSAL_DynamicParams     universalDynParams;
UNIVERSAL_Status            universalStatus;
XDM1_BufDesc                universalInBufDesc;
XDM1_BufDesc                universalOutBufDesc;

// аргумент командной строки
// если отсутствует то принимается gFrames = 1 это режим для dsp_client.py
// в этом режиме обработка полученных данных выполняется один раз и результат отсылается клиенту
// если присутствует и > 1 то обработка зацикливается на количество циклов gFrames
// это режим для af_client.py
// данные клиенту не отсылаются

/*
 *  ======== main ========
 */

Int main(Int argc, String argv[])
{
	String engineName = "af_engine";
	Engine_Handle ce = NULL;
	Engine_Attrs attrs;
	hUniversal = NULL;
	Memory_AllocParams allocParams;
	XDAS_Int8 *inBufSig = NULL;
	XDAS_Int8 *inBufRef = NULL;
	XDAS_Int8 *outBufCfar = NULL;
	XDAS_Int8 *outBufAf = NULL;
	XDAS_Int8 *versionBuf = NULL;   /* acquire optional version from codecs */
	String mapFileName = "omap3530_memmap.txt";
	Engine_Error retVal;
	Engine_Desc engDesc;

	gFlag = 0; // признак завершения по Ctrl-C

	printf("\nCompiled options:\n\tDSP_TESTMODE: %d\n\tAF_UDP: %d\n\tPROFILE: %d\n",
		DSP_TESTMODE, AF_UDP, NPROFILE);
	if(argc != 8)
	{
		printf("Usage: %s frames ip_multi port_multi ip_log port_log log_mode cfar_level\n", argv[0]);
		exit(0);
	}

	gFrames = atoi(argv[1]);
	int res = tcpInit(argv[4], atoi(argv[5]));
	if(res == -1)
		exit(1);

	printf("\nframes: %d\n", gFrames);

	printf("\n\n****\nStart af_app...\n"
			"\tNSAMPLES:\t%d\n"
			"\tBUFSIZE:\t%d\n"
			"\tSIGBUFSIZE:\t%d\n"
			"\tROWS:\t\t%d\n"
			"\tABUFSIZE:\t%d\n****\n\n", \
			NSAMPLES, BUFSIZE, SIGBUFSIZE, ROWS, ABUFSIZE);

	signal(SIGINT, signal_handler);
	/* init Codec Engine */
	CERuntime_init();

	/* Enable all trace for xdc.runtime.Main */
	Diags_setMask("xdc.runtime.Main+EX1234567");

	/*
	 * Create the Engine with a remote Server and add register the
	 * appropriate stub functions.
	 *
	 * Note, this can also be done in a config script.
	 */
	retVal = Engine_addStubFxns("UNIVERSAL_STUBS",
			(IALG_Fxns *)&UNIVERSAL_STUBS);
	if (retVal != Engine_EOK) 
	{
		printf("App-> ERROR: Runtime Engine_addStubFxns() failed (0x%x)\n",
				retVal);
		goto end;
	}

	/* Add an engine for each slave processor */

	Engine_initDesc(&engDesc);
	engDesc.name = engineName;
	engDesc.memMap = mapFileName;
	engDesc.remoteName = "server_dsp.xe64P";

	retVal = Engine_add(&engDesc);
	if (retVal != Engine_EOK) 
	{
		printf("main-> ERROR: Runtime Engine_add() failed (0x%x)\n", retVal);
		goto end;
	}

	/*
	 * Allocate buffers.
	 * Note that the .flags field (cache) is ignored on BIOS-based systems.
	 */
	allocParams.type = Memory_CONTIGPOOL;
	allocParams.flags = Memory_NONCACHED;
	allocParams.align = BUFALIGN;
	allocParams.seg = 0;

	/* выделение памяти для входного и выходного буферов AF */

	// BUFSIZE уже учитывает что числа комплексные
	inBufSig = (XDAS_Int8 *)Memory_alloc(SIGBUFSIZE, &allocParams);
	inBufRef = (XDAS_Int8 *)Memory_alloc(BUFSIZE, &allocParams);
	// битовый массив CFAR размером ABUFSIZE/BYTESIZE
	outBufCfar = (XDAS_Int8 *)Memory_alloc(ABUFSIZE/BYTESIZE, &allocParams);
	// байтовая матрица AF размером ABUFSIZE 
	outBufAf = (XDAS_Int8 *)Memory_alloc(ABUFSIZE*2, &allocParams);

	versionBuf = (XDAS_Int8 *)Memory_alloc(MAXVERSIONSIZE, &allocParams);

	if ((inBufSig == NULL) || (inBufRef == NULL) || (outBufCfar == NULL) || (outBufAf == NULL) || (versionBuf == NULL)) 
	{
		printf("App-> ERROR: can't allocate memory\n");
		goto end;
	}

	/* Initialize attrs fields to default values, and set the procId */
	Engine_initAttrs(&attrs);
	attrs.procId = (char *) "DSP";

	/* Open the Engine */
	if ((ce = Engine_open(engineName, &attrs, NULL)) == NULL) 
	{
		printf("App-> ERROR: can't open engine %s\n", engineName);
		goto end;
	}
	/* allocate and initialize universal alg on the engine */
	hUniversal = UNIVERSAL_create(ce, universalName, NULL);
	if (hUniversal == NULL) 
	{
		printf( "App-> ERROR: can't open codec %s\n", universalName);
		goto end;
	}

	/* Process Loop initialisation */

	/* initialize bufDescs */
	universalInBufDesc.numBufs = 2;
	universalOutBufDesc.numBufs = 2;
	universalInBufDesc.descs[0].bufSize = SIGBUFSIZE;
	universalInBufDesc.descs[1].bufSize = BUFSIZE;
	universalOutBufDesc.descs[0].bufSize = ABUFSIZE / BYTESIZE;
	universalOutBufDesc.descs[1].bufSize = 2*ABUFSIZE;

	universalInBufDesc.descs[0].buf = inBufSig;
	universalInBufDesc.descs[1].buf = inBufRef;
	universalOutBufDesc.descs[0].buf = outBufCfar;
	universalOutBufDesc.descs[1].buf = outBufAf;

	/* initialize all "sized" fields */
	//universalInArgs.size    = sizeof(universalInArgs);
	//universalOutArgs.size   = sizeof(universalOutArgs);
	lwafInArgs.universal.size = sizeof(lwafInArgs);
	lwafOutArgs.universal.size = sizeof(lwafOutArgs);
	lwafInArgs.logMode = atoi(argv[6]);
	lwafInArgs.cfarLevel = atoi(argv[7]);

	universalDynParams.size = sizeof(universalDynParams);
	universalStatus.size    = sizeof(universalStatus);

	/* if the codecs support it, dump their versions */
	universalStatus.data.numBufs = 1;
	universalStatus.data.descs[0].buf = versionBuf;
	universalStatus.data.descs[0].bufSize = MAXVERSIONSIZE;
	universalStatus.data.descs[1].buf = NULL;

#ifdef CACHE_ENABLED
	/* invalidate versionBuf it before the alg fills it */
	Memory_cacheInv(versionBuf, MAXVERSIONSIZE);
#endif

	UNIVERSAL_control(hUniversal, XDM_GETVERSION, &universalDynParams,
			&universalStatus);

#ifdef CACHE_ENABLED
#if defined(xdc_target__isaCompatible_64P) || \
	defined(xdc_target__isaCompatible_64T)
	Memory_cacheWbInv(inBufSig, SIGBUFSIZE);
	Memory_cacheWbInv(inBufRef, BUFSIZE);
#else
#error Unvalidated config - add appropriate fread-related cache maintenance
#endif
	/* Per DMA Rule 7, our output buffer cache lines must be cleaned */
	Memory_cacheInv(outBufCfar, ABUFSIZE/BYTESIZE);
	Memory_cacheInv(outBufAf, 2*ABUFSIZE);
#endif

#ifdef CACHE_ENABLED
	if (XDM_ISACCESSMODE_WRITE(universalOutBufDesc.descs[0].accessMask)) 
	{
		Memory_cacheWb(outBufCfar, ABUFSIZE/BYTESIZE);
		Memory_cacheWb(outBufAf, 2*ABUFSIZE);
	}
#endif

	rdata = malloc(sizeof(rdata_t));
	if(rdata == NULL)
		printf("app: rdata is zero pointer\n");
	printf("app.c: before udpInit\n");
	udpInitRecv(PORT2BI, rdata);

	udpInitSend(argv[2], atoi(argv[3]));

	// XXX
	/*************************************************/
	processLoop();
	/**************************************************/

end:
	printf( "app terminated: release resources\n");
	/* teardown the codec */
	if (hUniversal) 
	{
		UNIVERSAL_delete(hUniversal);
	}
	/* close the engine */
	if (ce) 
	{
		Engine_close(ce);
	}
	if (inBufSig) 
	{
		Memory_free(inBufSig, SIGBUFSIZE, &allocParams);
	}
	if (inBufRef) 
	{
		Memory_free(inBufRef, BUFSIZE, &allocParams);
	}
	if (outBufCfar) 
	{
		Memory_free(outBufCfar, ABUFSIZE/BYTESIZE, &allocParams);
	}
	if (outBufAf) 
	{
		Memory_free(outBufAf, ABUFSIZE*2, &allocParams);
	}
	if (versionBuf) 
	{
		Memory_free(versionBuf, MAXVERSIONSIZE, &allocParams);
	}

	udpCloseRecv(rdata);
	free(rdata);
	udpCloseSend();
	//fflush(sockfd);
	close(sockfd);

	printf("app done\n");
	return (0);
}

int tcpInit(char* ip, int port)
{
    // инициализация tcp клиента для отправки лога на осциллограф лаптопа
    ip_log = ip;
    port_log = port;
	fprintf(stderr, "tcpInit %s:%d\n", ip_log, port_log);

    return 0;
}

