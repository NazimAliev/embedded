/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

//#include "../af.h" 
#include "zboard.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <arm_neon.h>

int sockfd;
struct sockaddr_in servaddr, cliaddr;
char* ip_log;
int port_log;

void cmul(const float* x, const float* y, int len, float* z);

const char* testfile = "rtl_1024MHz_512ks.dat";

fdata_t* fdata;

int testInit()
{

#if ZBOARD_TESTMODE == 0
	fprintf(stderr, "testInit error: this function is not work in no = 2 mode");
	return -1;
#endif

	int i;
	// map_t (4*(int32 = 4 bytes)) * MAPSIZE
	gMap = malloc(MAPSIZE * sizeof(map_t));
	// fdata_t (4*(float = 4 bytes)) * MAPSIZE
	fdata = malloc(MAPSIZE * sizeof(fdata_t));

#if ZBOARD_TESTMODE == 2
	// для режима 2 вся работа уже сделана, данные будут заполняться на лету в testRead()
	return 0;
#endif

	// далее - работа в режиме 1

	// запись gMap возрастающими значениями int32
	// чтобы различать отдельные выборки, к каждой выборке добавляется +10 
	int j = 0;
	for(i = 0; i < MAPSIZE; i++)
	{
		gMap[i].si = j+1;
		gMap[i].sq = j+2;
		gMap[i].ri = j+3;
		gMap[i].rq = j+4;
		j += 10;
	}
	return 0;
}

#define PI (3.14159)

int testRead(/* map_t* gMap */)
{
#if ZBOARD_TESTMODE != 2
	fprintf(stderr, "testInit error: this function work only in mode 2");
	return 0;
#endif

/*
  float pwr = 500000.0;
                float f1 = 8000.0;
                float f2 = 2000.0;
                float w = 2.0*3.14159*idx/FPGA;
                // тестовое заполнение буфера вместо xdma
                // sig + ref, в фильтре ref должно быть подавлено и останется sig
                inSigBufI[j] = (int)(pwr * sinf(w*f1) + pwr * sinf(w*f2));
                inSigBufQ[j] = (int)(pwr * cosf(w*f1) + pwr * cosf(w*f2));
                //inSigBufI[j] = (int)(pwr * sinf(w*f2));
                //inSigBufQ[j] = (int)(pwr * cosf(w*f2));
                // ref
                inRefBufI[j] = (int)(pwr * sinf(w*f2));
                inRefBufQ[j] = (int)(pwr * cosf(w*f2));


*/

	// FPGA = 488281 выборок в секунду
	// t20ms = 0.02 * FPGA индекс для 20ms
	// sin(2*PI*f / FPGA) - f будет в герцах
	int i;
	float k = 10000.0;
	int amin = 5;
	int amax = 10;
	int fmin = 100;
	int fmax = 10000;
	int a = 0;
	int f = 0;
	int t20ms = (int)(0.02 * FPGA);
	printf("\tt20ms = %d samples\n", t20ms);

	for(i = 0; i < MAPSIZE; i++) 
	{
		if(!(i % t20ms))
		{
			// новые значения амплитуды и фазы каждые 20ms
			a = (rand()%(amax - amin)) + amin;
			f = (rand()%(fmax - fmin)) + fmin;
			//printf("\t[%d] a = %d f = %d\n", i, a, f);
		}
		fdata[i].ri = k*a*sinf((2.0*PI*f*i) / FPGA);
		fdata[i].rq = k*a*cosf((2.0*PI*f*i) / FPGA);
		fdata[i].si = k*a*sinf((2.0*PI*(1.001*f)*i) / FPGA);
		fdata[i].sq = k*a*cosf((2.0*PI*(1.001*f)*i) / FPGA);

		gMap[i].ri = (int32_t)(fdata[i].ri);
		gMap[i].rq = (int32_t)(fdata[i].rq);
		gMap[i].si = (int32_t)(fdata[i].si);
		gMap[i].sq = (int32_t)(fdata[i].sq);
/*
		// все ref файловые данные из gMap во float массив
		fdata[i].ri = (float)gMap[i].ri;
		fdata[i].rq = (float)gMap[i].rq;

		// сдвигаем и умножаем на exp(j)
		T = (2*3.14159*f*i) / 1000000000.0;
		fdata[i].si = ktf * (fdata[i].ri * cosf(T) + fdata[i].rq * sinf(T));	// inSigBufI
		fdata[i].sq = ktf * (fdata[i].rq * cosf(T) - fdata[i].ri * sinf(T));	// inSigBufQ

		// заносим sig полученный из ref сдвигом по частоте в gMap
		// добавляем сдвиг по времени
		// добавляем в сигнал помеху - опорный FM сигнал
		if(i >= t)
		{
			gMap[i].si = (int32_t)(fdata[i - t].si + fdata[i].ri); 
			gMap[i].sq = (int32_t)(fdata[i - t].sq + fdata[i].rq); 
		}
		else
		{
			gMap[i].si = 0;
			gMap[i].sq = 0;
		}
		// на каждый блок сдвигаем сигнал по частоте и времени
		if(i % NSAMPLES == 0)
		{
			t += 4;
			f -= 2000000;
		}
*/
	}
	return 0;
}

void cmul(const float* x, const float* y, int len, float* z)
{
	int i, j;
	for(i = 0, j = 0; i < len; i++, j+=2)
	{
		z[j] = x[j] * y[j] - x[j+1] * y[j+1];
		z[j] = x[j] * y[j+1] + y[j] * x[j+1];
	}
}
int testClose()
{
	free(fdata);
	free(gMap);
	return 0;
}

struct timeval tv_prev;

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

int time_stop(const char* str)
{
    struct timeval tv;
    time_t         s;
    suseconds_t    us;
    int elapsed;

    if (gettimeofday(&tv, NULL) == -1)
    {
        return -1;
    }

    s = tv.tv_sec - tv_prev.tv_sec;
    us = tv.tv_usec - tv_prev.tv_usec;

    elapsed = s * 1000000l + us;
    tv_prev = tv;
    fprintf(stderr, "%s: %d us\n", str, elapsed);
    return elapsed;
}

// вычисление амплитуды сигнала для проверки переполнения
int rms(const int* x, int len)
{
	int i;
	float f;
	float sum = 0;
	uint32_t res;
	for(i = 0; i < len; i++)
	{
		f = (float) x[i];
		sum += f * f;
	}
	sum /= len;	
	res = (int32_t)sqrt(sum);
	return res;
}

float rmsf(const float* x, int len)
{
	int i;
	float sum = 0;
	for(i = 0; i < len; i++)
	{
		sum += x[i] * x[i];
	}
	sum /= len;	
	return sqrt(sum);
}

int tcpInit(char* ip, int port)
{
	ip_log = ip;
	port_log = port;
	fprintf(stderr, "tcpInit %s:%d\n", ip_log, port_log);
    return 0;
}

// отправка содержимого массивов по tcp на main_mon.py для индикации
int writelog(int id, const uint8_t* data, int len)
{
    int res;
    // инициализация tcp клиента для отправки лога на осциллограф лаптопа
    sockfd=socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_log);
    servaddr.sin_port=htons(port_log);

	//fprintf(stderr, "\t[%d] writelog ---> %s:%d %db\n", id, ip_log, port_log, len);
    res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(res == -1 )
    {
        perror("\twritelog");
		close(sockfd);
        return -1;
    }
    sendto(sockfd, (char*)data, len, 0,
             (struct sockaddr *)&servaddr, sizeof(servaddr));
	close(sockfd);

    //fprintf(stderr, "\tSigI\tSigQ\tRefI\tRefQ\n");
    assert(len >= 4);
    //fprintf(stderr, "\t%x\t%x\t%x\t%x\n", data[0], data[1], data[2], data[3]);
    //fprintf(stderr, "\t%x\t%x\t%x\t%x\n", data[len-4], data[len-3], data[len-2], data[len-1]);
    return 0;
}

int test_mul()
{
    int i;
    int k;
	const int n = 512;
	const int K = 512;
    float x1[n], x2[n], y[n];

	// подготовка перемножаемых массивов
	srand(1);
    for (i = 0; i < n; i++)
	{
        x1[i] = (float)rand()/(float)RAND_MAX;
        x2[i] = (float)rand()/(float)RAND_MAX;
	}


	// стандартное умножение
    time_start();
    for (k=0; k < K; k++)
    	for (i = 0; i < n; i++)
			y[i] = x1[i] * x2[i];
    time_stop(" standard mul");

	// NEON умножение
    time_start();
    for (k=0; k < K; k++)
    	for (i = 0; i < n; i += 4)
			vst1q_f32(y+i, vmulq_f32(vld1q_f32(x1+i), vld1q_f32(x2+i)));
    time_stop(" NEON mul");

	// ne10 умножение
    time_start();
    for (k=0; k < K; k++)
		ne10_mul_float_neon((ne10_float32_t*) y, (ne10_float32_t*) x1, (ne10_float32_t*) x2, n);
    time_stop(" ne10 neon mul");

    return 0;
}

