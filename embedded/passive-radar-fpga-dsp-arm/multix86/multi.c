/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 *  ======== app.c ========
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <math.h>

#include "../af.h"
#include "multi.h"

int gFrames;
int gFifo;

// структура для хранения внутренних данных udp
// для каждого потока - своя для multithread
rdata_t* rdata1 = NULL;
rdata_t* rdata2 = NULL;

// массивы для приема cfar с 2-х частотных каналов
uint8_t* ic1 = NULL;
uint8_t* ic2 = NULL;
// массивы для приема af matrix с 2-х частотных каналов
uint16_t* ia1 = NULL;
uint16_t* ia2 = NULL;

int main(int argc, char* argv[])
{
	printf("\nmulti.c compiled options:\n\tMULTI_TESTMODE: %d\n\tAF_UDP: %d\n",
        MULTI_TESTMODE, AF_UDP);

    // сколько блоков данных ожидается принять
    if(argc == 1)
		gFrames = 1;
    else
		gFrames = atoi(argv[1]);

    printf("\nmulti.c frames: %d\n", gFrames);

	// ВАЖНО: если на том конце pipe никого нет, то вызов заблокируется
	printf("I will be blocked here? Don't forget run main_multi.py to read FIFO\n");
    gFifo = open("/tmp/af.fifo", O_WRONLY);
    if(gFifo == -1)
    {
        perror("main: error opening fifo, you forget mkfifo /tmp/af.fifo ?");
        return -1;
    }
	printf("Ok, fifo block passed\n");

	ic1 = (uint8_t*)malloc(sizeof(uint8_t) * ABUFSIZE / BYTESIZE);
	ic2 = (uint8_t*)malloc(sizeof(uint8_t) * ABUFSIZE / BYTESIZE);
	ia1 = (uint16_t*)malloc(sizeof(uint8_t) * 2*ABUFSIZE);
	ia2 = (uint16_t*)malloc(sizeof(uint8_t) * 2*ABUFSIZE);
	rdata1 = malloc(sizeof(rdata_t));
	rdata2 = malloc(sizeof(rdata_t));

	if(ic1 == NULL || ic2 == NULL || ia1 == NULL || ia2 == NULL || rdata1 == NULL || rdata2 == NULL)
	{
		printf("app: malloc error\n");
		exit(1);
	}

#if MULTI_TESTMODE==1
/*
	CFAR буфер заполняется по правилу:
	pos = f + NSAMPLES * t;
    cfarBuf[pos/8] |= (128 >> (pos % 8));
*/
	int i;
	for(i = 0; i < (ABUFSIZE / BYTESIZE); i++)
	{
		//printf("%d ", i);
		ic1[i] = 0;
		ic2[i] = 0;
//		if(i % (NSAMPLES/8/2) == 0)
//			ic2[i] = 1;
	}
	// отметка в нижнем левом углу: начало массива
	ic1[0] = 0b10010001;
	ic1[8] = 0b10000000;
	// отметка в верхнем левом углу
	//ic1[ROWS / BYTESIZE - 1] = 0b0000001;
	// отметка в правом нижнем углу
	//ic1[ABUFSIZE / BYTESIZE - 1 - ROWS / BYTESIZE + 1] = 0b10000000;
	// отметка в правом верхнем углу: конец массива
	//ic1[ABUFSIZE / BYTESIZE - 1] = 0b00000001;

	// d_gl = (x_gl**2 + y_gl**2) * 0.1
	// d2_gl = d_gl ** 0.5 + 0.1
	// Z = np.sin(i*im.d_gl) / im.d2_gl

	int x, y;
	//uint16_t z;
	i = 0;
	for(x = 0; x < ROWS; x++)
	{
		for(y = 0; y < NSAMPLES; y++)
		{
			/*
			float d1, d2;
			d1 = (x*x + y*y)*0.0001;
			d2 = sqrtf(d1) + 0.1;
			z = (uint16_t)(100000 * (sinf(d1)+1.0) / d2);
			*/
			//z = (uint16_t)y;
			ia1[y+x*NSAMPLES] = (i++)/8;
			//ia1[y+x*NSAMPLES] = (i++)/64;
			//ia1[y+x*NSAMPLES] = (y+x*NSAMPLES) / 10;
			//ia2[y+x*NSAMPLES] = (y+x*NSAMPLES) / 10;;
		}
	}
#else
    udpInitRecv(PORT2MULTI_1, rdata1);
    udpInitRecv(PORT2MULTI_2, rdata2);
#endif

    // XXX зацикливание здесь
    /*************************************************/
    processLoop();
    /**************************************************/

    printf( "app terminated: release resources\n");
#if MULTI_TESTMODE!=1
    udpCloseRecv(rdata1);
    udpCloseRecv(rdata2);
#endif
    close(gFifo);

	free(ic1);
	free(ic2);
	free(ia1);
	free(ia2);
	free(rdata1);
	free(rdata2);

    return (0);
}

