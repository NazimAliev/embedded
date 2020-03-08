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

#include "../mf.h"
#include "app.h"
#include "../codec_dsp/mfilter.h"

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


void signal_handler(int signum)
{
    fprintf(stderr, "signal_handler: signal %d\n", signum);
	exit(-1);
}

XDAS_Int8 *inBufSig;
XDAS_Int8 *inBufRef;
XDAS_Int8 *outBufFft;
XDAS_Int8 *inBufImgSig;
XDAS_Int8 *inBufImgRef;
XDAS_Int8 *outBufImg;
XDAS_Int8 *versionBuf;

UNIVERSAL_Handle	  		hUniversal;
//UNIVERSAL_InArgs            universalInArgs;
//UNIVERSAL_OutArgs           universalOutArgs;
LW_AF_InArgs				lwafInArgs;
LW_AF_OutArgs				lwafOutArgs;
UNIVERSAL_DynamicParams     universalDynParams;
UNIVERSAL_Status            universalStatus;
XDM1_BufDesc                universalInBufDesc;
XDM1_BufDesc                universalOutBufDesc;

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
	inBufSig = NULL;
	inBufRef = NULL;
	outBufFft = NULL;
	inBufImgSig = NULL;
	inBufImgRef = NULL;
	outBufImg = NULL;
	versionBuf = NULL;
	String mapFileName = "omap3530_memmap.txt";
	Engine_Error retVal;
	Engine_Desc engDesc;

	if(argc != 6)
	{
		printf("Usage: %s sigFile refFile inParmU16 inParmU32 outFile\n", argv[0]);
		exit(0);
	}

	// read data from file
	
	printf("\n\n****\nStart af_app...\n"
			"\tN:\t\t%d\n"
			"\tBUFSIZE_1D:\t%d\n"
			"\n\n", \
			N, BUFSIZE_1D);

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
	inBufImgSig = (XDAS_Int8 *)Memory_alloc(BUFSIZE_2D, &allocParams);
	inBufImgRef = (XDAS_Int8 *)Memory_alloc(BUFSIZE_2D, &allocParams);
	outBufImg = (XDAS_Int8 *)Memory_alloc(BUFSIZE_2D, &allocParams);
	inBufSig = (XDAS_Int8 *)Memory_alloc(BUFSIZE_1D, &allocParams);
	inBufRef = (XDAS_Int8 *)Memory_alloc(BUFSIZE_1D, &allocParams);
	// выходной буфер для тестовых целей, на самом деле нужны только координаты max 
	outBufFft = (XDAS_Int8 *)Memory_alloc(BUFSIZE_1D, &allocParams);

	versionBuf = (XDAS_Int8 *)Memory_alloc(MAXVERSIONSIZE, &allocParams);

	if ((inBufSig == NULL)
		|| (inBufRef == NULL)
		|| (inBufImgSig == NULL)
		|| (inBufImgRef == NULL)
		|| (outBufImg == NULL)
		|| (outBufFft == NULL)
		|| (versionBuf == NULL)) 
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
	universalInBufDesc.numBufs = 4;
	universalOutBufDesc.numBufs = 2;
	universalInBufDesc.descs[0].bufSize = BUFSIZE_1D;
	universalInBufDesc.descs[1].bufSize = BUFSIZE_1D;
	universalInBufDesc.descs[2].bufSize = BUFSIZE_2D;
	universalInBufDesc.descs[3].bufSize = BUFSIZE_2D;
	universalOutBufDesc.descs[0].bufSize = BUFSIZE_1D;
	universalOutBufDesc.descs[1].bufSize = BUFSIZE_2D;

	universalInBufDesc.descs[0].buf = inBufSig;
	universalInBufDesc.descs[1].buf = inBufRef;
	universalInBufDesc.descs[2].buf = inBufImgSig;
	universalInBufDesc.descs[3].buf = inBufImgRef;
	universalOutBufDesc.descs[0].buf = outBufFft;
	universalOutBufDesc.descs[1].buf = outBufImg;

	/* initialize all "sized" fields */
	//universalInArgs.size    = sizeof(universalInArgs);
	//universalOutArgs.size   = sizeof(universalOutArgs);
	lwafInArgs.universal.size = sizeof(lwafInArgs);
	lwafOutArgs.universal.size = sizeof(lwafOutArgs);
	lwafInArgs.inParmU16 = 0; 
	lwafInArgs.inParmU32 = 0; 

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
	Memory_cacheWbInv(inBufSig, BUFSIZE_1D);
	Memory_cacheWbInv(inBufRef, BUFSIZE_1D);
	Memory_cacheWbInv(inBufImgSig, BUFSIZE_2D);
	Memory_cacheWbInv(inBufImgRef, BUFSIZE_2D);
#else
#error Unvalidated config - add appropriate fread-related cache maintenance
#endif
	/* Per DMA Rule 7, our output buffer cache lines must be cleaned */
	Memory_cacheInv(outBufFft, BUFSIZE_1D);
	Memory_cacheInv(outBufImg, BUFSIZE_2D);
#endif

#ifdef CACHE_ENABLED
	if (XDM_ISACCESSMODE_WRITE(universalOutBufDesc.descs[0].accessMask)) 
	{
		Memory_cacheWb(outBufFft, BUFSIZE_1D);
		Memory_cacheWb(outBufImg, BUFSIZE_2D);
	}
#endif

	// XXX
	/*************************************************/
 	int res;

	// read img sig file
    res = readImgFile((int16_t*)inBufImgSig, argv[1]);
		if(res == -1) goto end;
	// read img ref file
    res = readImgFile((int16_t*)inBufImgRef, argv[2]);
		if(res == -1) goto end;
    //genTestData(inBufImgSig, inBufImgRef);
	//logInt((int16_t*)inBufImgSig);
	//logInt((int16_t*)inBufImgRef);
	// call once
	time_init();
	// start/delta pair will show delay
    time_start();
    res = UNIVERSAL_process(hUniversal, &universalInBufDesc,
            &universalOutBufDesc, NULL, (UNIVERSAL_InArgs *)&lwafInArgs, (UNIVERSAL_OutArgs *)&lwafOutArgs);
    time_delta();
	//logInt((int16_t*)outBufImg);
	// write output img file
    res = writeImgFile((int16_t*)outBufImg, argv[5]);
		if(res == -1) goto end;
	// now nothing in pars
    printf("\tres: %d, outParmU16: %d\n", res, lwafOutArgs.outParmU16);
	// find x,y of marker after convolve
	int max = findMax((int16_t*)outBufImg);
    printf("\tmax: [%d,%d]\n", max/N, max % N);
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
		Memory_free(inBufSig, BUFSIZE_1D, &allocParams);
	}
	if (inBufRef) 
	{
		Memory_free(inBufRef, BUFSIZE_1D, &allocParams);
	}
	if (inBufImgSig) 
	{
		Memory_free(inBufImgSig, BUFSIZE_2D, &allocParams);
	}
	if (inBufImgRef) 
	{
		Memory_free(inBufImgRef, BUFSIZE_2D, &allocParams);
	}
	if (outBufFft) 
	{
		Memory_free(outBufFft, BUFSIZE_1D, &allocParams);
	}
	if (outBufImg) 
	{
		Memory_free(outBufFft, BUFSIZE_2D, &allocParams);
	}
	if (versionBuf) 
	{
		Memory_free(versionBuf, MAXVERSIONSIZE, &allocParams);
	}

	printf("app done\n");
	return (0);
}
