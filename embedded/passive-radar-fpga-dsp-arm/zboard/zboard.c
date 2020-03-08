/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 *** zboard.c ***
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <malloc.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

//#include "../af.h"
#include "afilter.h"
#include "../udp/udp.h"
#include "zboard.h"

#define THREADS 2
#define THREAD_I 0
#define THREAD_Q 1

#define AMP 200 

/*
	Логика тестовых режимов определяемых ZBOARD_TESTMODE

	ZBOARD_TESTMODE = 0
	Рабочий режим
	Инициализация: xdmaInit() при старте программы
	Заполнение gMap: xdmaRead() в каждом цикле

	ZBOARD_TESTMODE = 1
	данные для gMap формируются один раз при инициализации - простая последовательность	
	Инициализация: testInit() при старте программы
	Заполнение gMap: один раз в testInit()

	ZBOARD_TESTMODE = 2
	данные для gMap заполняются FM значениями на лету	
	Инициализация: testInit() при старте программы
	Заполнение gMap: testRead() в каждом цикле

*/

map_t* gMap;

int32_t* inSigBufI = NULL;
int32_t* inRefBufI = NULL;

// Q - данные с FPGA для THREAD_Q
int32_t* inSigBufQ = NULL;
int32_t* inRefBufQ = NULL;

// выходы потоков
short* outSigBufI = NULL;
short* outSigBufQ = NULL;

// упакованные в единый I/Q буфер данные потоков для передачи по UDP
short* outSigBufIQ = NULL;

// перекодированные из int32 в int16 входные данные ref (они не меняются в ходе обработки)
short* outRefBufIQ = NULL;

pthread_t trd[THREADS];
uint32_t targ[THREADS];
uint32_t thread_count = 0;

int thread_start();
void* proc(void* arg);
void memAlloc();
void memFree();

//N = NSAMPLES + SLICESIZE

int main(int argc, char* argv[])
{
	int frame;
	int blocks;
	char* ip_log;
	int port_log;
	int res;

	fprintf(stderr, "Compiled modes:\n\tZBOARD_TESTMODE: %d\n\tADAPT: %d\n\tDEC: %d\n",\
		ZBOARD_TESTMODE, ADAPT, DEC);
	if(argc != 5)
	{
		fprintf(stderr, "Usage: %s blocks ip_bi ip_log port_log\n", argv[0]);
		exit(0);
	}

	blocks = atoi(argv[1]);
	fprintf(stderr, "ZBOARD start NSAMPLES: %d\n\tframes: %d\n\tsamples+slicesize N: %d\n\tblock size: %d x N samples\n\tmapbuf size: %d b / %d b\n\tmap size: %d\n",\
			NSAMPLES, blocks, N, BLOCKS, MAPBUFSIZE, 8*1024*1024, MAPSIZE);
	fprintf(stderr, "AFILTER parms:\n\tSLICESIZE: %d\n\tSLICES: %d\n\tTAPS: %d\n",\
			SLICESIZE, SLICES, TAPS);

	test_mul();
	assert(N < MAPSIZE);
	assert(BLOCKS < MAPSIZE / N);
	ip_log = argv[3];
	port_log = atoi(argv[4]);


#if ZBOARD_TESTMODE == 0
	res = xdmaInit();
#else
	// подготовка всех тестовых режимов вместо XDMA
	res = testInit();
#endif
	if(res != 0)
		exit(1);

	res = neInit();
	if(res != 0)
		exit(1);

	res = tcpInit(ip_log, port_log);
	if(res != 0)
		exit(1);


	neInitThread(THREAD_I);
	neInitThread(THREAD_Q);
	memAlloc();
	udpInitSend(argv[2], PORT2BI);

	//int32_t* pMap = (int32_t*)gMap;

	// blocks_count будет уменьшаться поблочно а не по вызовам xdma
	int blocks_count;
	blocks_count = blocks;
	frame = 0;

	while(1) 
	{
		fprintf(stderr, "Frame [%d]\n", frame);

#if ZBOARD_TESTMODE == 0

		time_start();
		// опрос драйвера XDMA
		xdmaRead(/* map_t* gMap */);
		int elapsed = time_stop("xdma read");

		// пропуск "плохих" запросов к xdma
		if(elapsed < 100000)
		{
			fprintf(stderr, "  * skipped *\n");
			usleep(1000000);
			continue;
		}
#else
		// задержку вводим для всех тестовых режимов
		sleep(1);	// имитация интервала поступления данных с XDMA
#endif

#if ZBOARD_TESTMODE == 2
		// чтение FM данных на лету 
		// в режимах 1, 3 testRead() не нужен: данные однократно заполняются в testInit()
		// testRead() заполняет сразу всю gMap
		testRead(/* map_t* gMap */);
#endif

		/*
		 * к этому моменту gMap заполнен или с xdma,
		 * или в testRead() в тестовом режиме 2,
		 * или был заполнен однократно в других тестовых режимах
		 *
		 */

		// поскольку в map располагается больше чем один блок состоящий из N,
		// проходим поблочно.
		// 
		int offset = 0;
		int i;
		for(i = 0; i < BLOCKS; i++)
		{
			// подготовка массивов для потоков
			int j;
			int idx;
			// N на SLICESIZE больше чем NSAMPES, поэтому
			// надеемся  на децимацию чтобы не выйти за границы массива
			for(j = 0; j < N; j++)
			{
				idx = j*DEC + offset;
				inSigBufI[j] = gMap[idx].si;
				inSigBufQ[j] = gMap[idx].sq;
				inRefBufI[j] = gMap[idx].ri;
				inRefBufQ[j] = gMap[idx].rq;
			}
			offset += (NSAMPLES * DEC);
			fprintf(stderr, "\t[%d/%d]", frame, i);

			// измерение амплитуды REF (на самом деле RMS)
			int amp_i;
			int scale_i;
			int amp_q;
			int scale_q;
			// для измерений берем только начало массива для быстродействия
			amp_i = rms(inSigBufI, SLICESIZE);	
			amp_q = rms(inSigBufQ, SLICESIZE);	
			fprintf(stderr, "\tSigI: (%i) %i uV, SigQ: (%i) %i uV\n", 
				amp_i, amp_i/AMP, amp_q, amp_q/AMP);
			// для измерений берем только начало массива для быстродействия
			amp_i = rms(inRefBufI, SLICESIZE);	
			amp_q = rms(inRefBufQ, SLICESIZE);	
			// масштабирующий коэффициент для приведения Ref int -> short
			scale_i = (2 * amp_i) / (SHRT_MAX);
			if(scale_i == 0)
				scale_i = 1;
			scale_q = (2 * amp_q) / (SHRT_MAX);
			if(scale_q == 0)
				scale_q = 1;
			fprintf(stderr, "\t\tRefI: (%i) %i uV, RefQ: (%i) %i uV, Scale I/Q: %i/%i\n", 
				amp_i, amp_i/AMP, amp_q, amp_q/AMP, scale_i, scale_q);


			// вызов 2-х экземпляров адаптивного фильтра I,Q в двух потоках
			// АФ получает N = NSAMPLES (16*1024) + SLICESIZE (512>ROWS) выборок
			// и обрабатывает их порциями по SLICESIZE (512) выборок, всего SLICES (32) + 1 порций
			// результат работы - также N выборок, в байтах SIGBUFSIZE
			// по UDP signal посылается размером SIGBUFSIZE: N = NSAMPLES + SLICESIZE выборок,
			// ref посылается размером BUFSIZE: NSAMPLES выборок
			// остаток SLICESIZE в конце сигнала нужен потому что сигнал двигается влево вплоть до ROWS позиций
			// поэтому ROWS должно быть меньше чем SLICESIZE

			//time_start();
//*************************
			thread_start();
//*************************
			//elapsed = time_stop("\t\tafilter");
 
			// упаковка в один IQ массив после neFilter
			int k;
			for(j = 0, k = 0; j < N; j++, k+=2)
			{
				// outSig заполняется в thread_start/proc/neFilter()
				outSigBufIQ[k] = outSigBufI[j];
				outSigBufIQ[k+1] = outSigBufQ[j];
				outRefBufIQ[k] = (short) (inRefBufI[j] / scale_i);
				outRefBufIQ[k+1] = (short) (inRefBufQ[j] / scale_q);
			}

			// посылаем полученные данные
			res = udpSend((uint8_t*)outSigBufIQ, SIGBUFSIZE, 11);
			fprintf(stderr, "\t\t%d bytes zb -> bb\n", res);
			res = udpSend((uint8_t*)outRefBufIQ, BUFSIZE, 12);
			fprintf(stderr, "\t\t%d bytes zb -> bb\n", res);
			sleep(2);	// dsp нужно время для обработки блока!

			// FIXME посылаем не весь массив!
			// 32-битные массивы
#ifndef LOG_Q
			writelog(0, (uint8_t*)inSigBufI, BUFSIZE);
			writelog(2, (uint8_t*)inRefBufI, BUFSIZE);
#else
			writelog(1, (uint8_t*)inSigBufQ, BUFSIZE);
			writelog(3, (uint8_t*)inRefBufQ, BUFSIZE);
#endif

			// 16-битные массивы
			writelog(4, (uint8_t*)outSigBufIQ, BUFSIZE);
			writelog(5, (uint8_t*)outRefBufIQ, BUFSIZE);
			blocks_count--;
			if(blocks_count <= 0)
				break;
		} // for(BLOCKS)
		if(blocks_count <= 0)
				break;
		frame++;
	} // while 
	memFree();

#if ZBOARD_TESTMODE == 0
	xdmaClose();
#else
	testClose();
#endif

	neCloseThread(THREAD_I);
	neCloseThread(THREAD_Q);
	udpCloseSend();

	return 0;
}

int thread_start()
{
    int res;

	targ[THREAD_I] = thread_count++;
	res = pthread_create(&trd[THREAD_I], NULL, proc, &targ[THREAD_I]);
	if(res)
	{
		fprintf(stderr, "processLoop error: create THREAD_I failed, res: %d\n", res);
		return -1;
	} 

	targ[THREAD_Q] = thread_count++;
	res = pthread_create(&trd[THREAD_Q], NULL, proc, &targ[THREAD_Q]);
	if(res)
	{
		fprintf(stderr, "processLoop error: create THREAD_Q failed, res: %d\n", res);
		return -1;
	} 
	// дожидаемся завершения потоков I,Q
	pthread_join(trd[THREAD_I], NULL);
	pthread_join(trd[THREAD_Q], NULL);

	return 0;

}

// функция потока
void* proc(void* arg)
{
    uint32_t targ = *(uint32_t*)arg;
    int32_t* pSig;
    int32_t* pRef;
    short* pTarget;

	// номер потока: nt=0 для четного (I), nt=1 для нечетного (Q)
	int nt = targ % 2;
    // who am I ?
    if(nt == 0)
    {
		pSig = inSigBufI;
		pRef = inRefBufI;
		pTarget = outSigBufI;
    }
    else
    {
		pSig = inSigBufQ;
		pRef = inRefBufQ;
		pTarget = outSigBufQ;
    }
    neFilter(pSig, pRef, pTarget, nt);

    return NULL;
}

void memAlloc()
{
	// I (Q) int32 alloc:
    inSigBufI = malloc(SIGBUFSIZE);
    inRefBufI = malloc(SIGBUFSIZE);
    inSigBufQ = malloc(SIGBUFSIZE);
    inRefBufQ = malloc(SIGBUFSIZE);

	// I (Q) int16 alloc:
    outSigBufI = malloc(SIGBUFSIZE / 2);
    outSigBufQ = malloc(SIGBUFSIZE / 2);

	// I/Q int16 alloc:
    outSigBufIQ = malloc(SIGBUFSIZE);
    outRefBufIQ = malloc(SIGBUFSIZE);
}

void memFree()
{
    free(inSigBufI);
    free(inRefBufI);
    free(inSigBufQ);
    free(inRefBufQ);
    free(outSigBufI);
    free(outSigBufQ);
    free(outSigBufIQ);
    free(outRefBufIQ);
}

