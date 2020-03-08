/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_ZBOARD
#define LW_ZBOARD

#include "../af.h"
#include <stdint.h>
#include "inc/NE10.h"

// int32 sig I/Q, int 32 ref I/Q
// si sq ri rq si sq ri rq ...
// i, q по 4 байта = int32_t

typedef struct
{
	int32_t si;
	int32_t sq;
	int32_t ri;
	int32_t rq;
} map_t;

typedef struct
{
	float si;
	float sq;
	float ri;
	float rq;
} fdata_t;

/* 
 * длина блока в адаптивный фильтр N = NSAMPLES + SLICESIZE = 16 * 1024 + 512 = 16384 + 512 = 16896
 *
 * всего блоков будет BLOCKS = 262144 / 16876 = 15.5 = примем 14 
 * при децимации DEC = 2 BLOCKS = 7 
 *
*/

#define N (NSAMPLES + SLICESIZE)

// коэффициент децимации
//#define DEC (28)
#define DEC (32)

#define MAPSIZE (256 * 1024)	// количество map_t данных
// сколько блоков NSAMPLES в MAP_SIZE
//#define BLOCKS (14)
#define BLOCKS (MAPSIZE / (N * DEC))

#if N * DEC > MAPSIZE 
	#error BLOCKS = 0 
#endif

// в байтах: sizeof(map_t) * sizeof(int32_t) * MAPSIZE
// или тоже самое: sizeof(rdata_t) * sizeof(float) * MAPSIZE
// для выделения памяти
#define MAPBUFSIZE  (4 * 4 * MAPSIZE)

extern map_t* gMap;

// формирование теста вместо XDMA в ту же область памяти gMap
int testInit();
int testRead(/* map_t* gMap */);
int testClose();

// реальные XDMA функции, взаимоисключающие с testXXX
int xdmaInit();
int xdmaRead(/* map_t* gMap */);
int xdmaClose();

void time_start();
int time_stop(const char* str);

int tcpInit(char* ip, int port);
int writelog(int id, const uint8_t* data, int len);
int rms(const int* x, int len);
float rmsf(const float* x, int len);
int test_mul();
#endif
