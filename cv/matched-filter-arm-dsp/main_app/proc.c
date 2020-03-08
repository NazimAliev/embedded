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

#include "../mf.h"
#include "app.h"

#include <xdc/std.h>
#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <ti/sdo/ce/universal/universal.h>

struct timeval tv_prev;

void time_init()
{
	// timestamp prepare
    tv_prev.tv_sec = 0;
    tv_prev.tv_usec = 0;
    return;
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

int readImgFile(int16_t *inBufImg, char* fileName)
{
	// TODO
	// interleave Re,Im and Im at first as DSP_FFT requeres
	// sizeof img is BUFSIZE_2D/2 because doesn't contain zero img part
	// inBufImg* size is BUFSIZE_2D - contains as img as real part
	// ref buf should be the same size as sig buf, and zero outside ref picture

    FILE* fp;
    int bytes;
	unsigned char img[N*N];

	// fill signal buffer
    fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        printf("Error: can't open img file %s \n", fileName);
        return(-1);
    }
    bytes = fread(img, sizeof(int8_t), N*N, fp);
    fclose(fp);
    if(bytes != N*N)
    {
        printf("Error read %s: img file size %d doesn't equal N*N, should be %d \n", fileName, bytes, N*N);
		return(-1);
    }
	// zero img path
    bzero(inBufImg, BUFSIZE_2D);
	int i;
	int mean = 0;
	for(i=0; i<N*N; i++)
	{
		mean += img[i];
	}
	mean /= N*N;
	for(i=0; i<N*N; i++)
	{
		// fill real part from file
		//inBufImg[2*i] = (int16_t)(img[i] - mean);
		inBufImg[2*i] = (int16_t)(img[i]);
	}
	return(0);
}


void genTestData(XDAS_Int8 *inBufSig, XDAS_Int8 *inBufRef)
{
	int n,i,j;
	//float freq = 10.0;
	int16_t* ptr;
	int16_t* ptr2;
	ptr = (int16_t*)inBufSig;
	ptr2 = (int16_t*)inBufRef;
	/* Convert the floating point data to reference fixed point data */
	for(n=0; n<N; n++)
	{
		for(i=0,j=0; j<N; i+=2,j++)
		{
			ptr[2*N*n+i] = 2*N*n+i;
			ptr[2*N*n+i+1] = 2*N*n+i+1;
			ptr2[2*N*n+i] = 2*N*n+i+2;
			ptr2[2*N*n+i+1] = 2*N*n+i+3;
		}
	}
}

int findMax(const int16_t* buf)
{
	int16_t max = 0;
	int16_t pix;
	int outx = 0;
	int outy = 0;
	int n,i,j;
	for(n=0; n<N; n++)
	{
		for(i=0,j=0; j<N; i+=2,j++)
		{
			pix = buf[2*N*n+i];
			if(pix < 0)
				pix = -pix;
			if(pix < max)
				continue;
			max = pix;
			outx = n;
			outy = j;
		}
	}
	printf("max: %d ", max);
	return(N*outy+outx);
}

int writeImgFile(int16_t *inBufImg, char* fileName)
{
    FILE* fp;
    int bytes;

	// fill signal buffer
    fp = fopen(fileName, "wb");
    if(fp == NULL)
    {
        printf("Error: can't open file to write %s \n", fileName);
        return(-1);
    }
    bytes = fwrite((char*)inBufImg, sizeof(int16_t), 2*N*N, fp);
    fclose(fp);
    if(bytes != 2*N*N)
    {
        printf("Error write %s: img file size %d doesn't equal N*N, should be %d \n", fileName, bytes, 2*N*N);
		return(-1);
    }
	return(0);
}

void logInt(int16_t* buf)
{
	int n,i,j;
	for(n=0; n<N; n++)
	{
		if(n>2 && n<N-2)
			continue;
		for(i=0,j=0; j<N; i+=2,j++)
		{
			if(j>2 && j<N-2)
				continue;
			printf("[%03d,%03d] Re %d\t", n,j, buf[2*N*n+i] );
			printf("Im %d\n", buf[2*N*n+i+1]);
		}
	}
}

