/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 *  ======== proc.c ========
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <limits.h>
#include <math.h>

#include "../../af.h"
#include "app.h"

#include <xdc/std.h>
#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <ti/sdo/ce/universal/universal.h>


#define THREADS 2
#define THREAD_1 0
#define THREAD_2 1

struct timeval tv_prev;

int writelog(int id, const char* data);
void time_start();
void time_delta();

void processLoop()
{
	XDAS_Int8 *inBufSig;
	XDAS_Int8 *inBufRef;
	XDAS_Int8 *outBufCfar;
	XDAS_Int8 *outBufAf;

	inBufSig = universalInBufDesc.descs[0].buf;
	inBufRef = universalInBufDesc.descs[1].buf;
	outBufCfar = universalOutBufDesc.descs[0].buf;
	outBufAf = universalOutBufDesc.descs[1].buf;

	// timestamp prepare
	tv_prev.tv_sec = 0;
	tv_prev.tv_usec = 0;

	short* pSig = (short*)inBufSig;
	short* pRef = (short*)inBufRef;
	int res;
	int i;
	int j, k;
	for(i=0; i<gFrames; i++)
	{
#if DSP_TESTMODE != 0
		printf("processLoop: DSP_TESTMODE ENABLED !! Disable it to get real job with input UDP\n");
		float f1 = 2000.0;
		float f2 = 3000.0;
		// тестовый режим - имитация входного short буфера который должен поступать с zboard
		// данные по udp не принимаются, программа не блокируется на вызовах recv
		// период поступления данных по udp имитируется вызовом sleep
		for(j = 0, k = 0; j < NSAMPLES; j++, k+=2)
		{
			float w = 2.0*3.14159*j/FPGA;
			pSig[k] = (short)(SHRT_MAX * sinf(w*f1)); 
			pSig[k+1] = (short)(SHRT_MAX * cosf(w*f1)); 
			pRef[k] = (short)(SHRT_MAX * sinf(w*f2)); 
			pRef[k+1] = (short)(SHRT_MAX * cosf(w*f2)); 
		}
		sleep(1);
#else
		uint16_t msg_id;
		// получаем сигнал и опору по UDP для потока
		// отправку данных по UDP выполняет сам поток
		res = udpRecv(rdata, (uint8_t*)inBufSig, SIGBUFSIZE, &msg_id);
		printf("processLoop: udpRecv sig bytes: %d, msg_id: %d\n", res, msg_id);
		res = udpRecv(rdata, (uint8_t*)inBufRef, BUFSIZE, &msg_id);
		printf("processLoop: udpRecv ref bytes: %d, msg_id: %d\n", res, msg_id);

#endif
		// меняем ReIm порядок на ImRe, поскольку afmath.c принимает в таком порядке
		short x;
		for(j = 0, k = 0; j < NSAMPLES; j++, k+=2)
		{
			x = pSig[k];
			pSig[k] = pSig[k+1];
			pSig[k+1] = x;
			x = pRef[k];
			pRef[k] = pRef[k+1];
			pRef[k+1] = x;
		}
		time_start();
		res = UNIVERSAL_process(hUniversal, &universalInBufDesc,
				&universalOutBufDesc, NULL, (UNIVERSAL_InArgs *)&lwafInArgs, (UNIVERSAL_OutArgs *)&lwafOutArgs);
		time_delta();
		printf("\tres: %d, alarms: %d\n", res, lwafOutArgs.alarms);
		res = udpSend((uint8_t*)outBufCfar, ABUFSIZE/BYTESIZE, 23);
		printf("proc: udpSend outBufCfar %d bytes\n", res);
#if AF_UDP==1
		res = udpSend((uint8_t*)outBufAf, 2*ABUFSIZE, 24);
		printf("proc: udpSend outBufAf %d bytes\n", res);
#endif

		if(lwafInArgs.logMode < ROWS)
		{
			for(j = 0; j < LOGROWSLEN; j++)	
			{
				writelog(j, (char*)outBufAf + j * sizeof(short)*NSAMPLES);
			}
		}

		if(gFlag == 1)
			break;
	} // for
}

// отправка содержимого массивов по tcp на main_mon.py для индикации
// отправляются только первые sizeof(short)*NSAMPLES байт массива, для унификации лога
int writelog(int id, const char* data)
{
    int res;
	int N = sizeof(short) * NSAMPLES;
    sockfd=socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_log);
    servaddr.sin_port=htons(port_log);

    res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(res == -1 )
    {
        perror("\twritelog");
        close(sockfd);
        return -1;
    }
	/*
    sendto(sockfd, (char*)data, N, 0,
             (struct sockaddr *)&servaddr, sizeof(servaddr));
	*/
    res = write(sockfd, (char*)data, N);
    //fprintf(stderr, "\t[%d] log ---> %s:%d %db res: %d\n", id, ip_log, port_log, N, res);
    close(sockfd);

    return 0;
}

void time_start()
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) == -1)
    {
        return;
    }
    tv_prev = tv;
    return;
}

void time_delta()
{
    struct timeval tv;
    time_t         s;
    suseconds_t    us;
    int elapsed;

    if (gettimeofday(&tv, NULL) == -1)
    {
        return;
    }

    s = tv.tv_sec - tv_prev.tv_sec;
    us = tv.tv_usec - tv_prev.tv_usec;

    elapsed = s * 1000000l + us;
    tv_prev = tv;
    fprintf(stderr, "-time elapsed: %d us\n", elapsed);
    return;
}
