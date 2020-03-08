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

#include "../af.h"
#include "multi.h"

#define THREADS 2
#define THREAD_1 0
#define THREAD_2 1

typedef struct
{
	// инкрементируемый номер потока
	uint32_t nt;
	// thred-safe udp data
	rdata_t* rdata;
	uint8_t* inBufCfar;
	uint16_t* inBufAf;
} targ_t;

void* proc(void* arg);

void processLoop()
{
	int res;

	pthread_t trd[THREADS];
	// чтобы не проверять в потоке его id и в зависимости от этого брать соответствующие данные,
	// сразу передаем их в структуру которая будет аргументом потока
	targ_t targ[THREADS] = { {0, rdata1, ic1, ia1}, {1, rdata2, ic2, ia2} };

	int i;
	for(i = 0; i < gFrames; i++)
	{
		// создание двух параллельных потоков, каждый слушает свой порт udp 
		pthread_create(&trd[THREAD_1], NULL, proc, &targ[THREAD_1]);
		pthread_create(&trd[THREAD_2], NULL, proc, &targ[THREAD_2]);

		// перед multi обработкой со всех каналов дождемся окончания потоков
		pthread_join(trd[THREAD_1], NULL);
		pthread_join(trd[THREAD_2], NULL);
		targ[THREAD_1].nt+=2;
		targ[THREAD_2].nt+=2;
		// данные udp со всех каналов приняты
		// FIXME
        //res = write(gFifo, (uint8_t*)ic1, ABUFSIZE / BYTESIZE);
		//printf("processLoop: ic1 write FIFO %d bytes\n", res);
#if AF_UDP == 1
		// разбиваем матрицу на две части и передаем дважды
        res = write(gFifo, (uint8_t*)ia1, 2*ABUFSIZE);
		printf("processLoop: ia1 write FIFO %d bytes\n", res);
#endif
		// XXX
		// раскомментировать если будет второй частотный канал
        //res = write(gFifo, (uint8_t*)ic2, ABUFSIZE / BYTESIZE);
		//printf("processLoop: ic2 write FIFO %d bytes\n", res);
        //res = write(gFifo, (uint8_t*)ia2, 2*ABUFSIZE);
		//printf("processLoop: ia2 write FIFO %d bytes\n", res);
	}
}

// функция потока - только для того чтобы одновременно принять UDP с двух каналов
void* proc(void* arg)
{
	targ_t targ = *(targ_t*)arg;

	uint32_t nt = targ.nt;
	// FIXME пропускаем все каналы кроме первого - для отладки
	if(nt % 2)
		return NULL;

#if MULTI_TESTMODE==1
	sleep(1);
#else
	uint16_t msg_id;
	int res1;
    // получаем UDP данные для потока
    res1 = udpRecv(targ.rdata, (uint8_t*)targ.inBufCfar, ABUFSIZE / BYTESIZE, &msg_id);
	printf("[%d] cfar data recv: %d bytes\n", nt, res1);
#if AF_UDP==1
	int res2;
    res2 = udpRecv(targ.rdata, (uint8_t*)targ.inBufAf, 2*ABUFSIZE, &msg_id);
	printf("[%d] af data recv: %d bytes\n", nt, res2);
#endif
#endif

    return NULL;
}
