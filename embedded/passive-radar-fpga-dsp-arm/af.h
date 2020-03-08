/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF
#define LW_AF

// XXX
// не ставить a*b без круглых скобок! неправильно раскрывается в программе

/* 
 * Размер входного буфера
 * частота выборки в АЦП: 125 MS/s
 * после децимации в CIC dec=256 поступление выборок в XDMA со скоростю 125 MS/s / 256 =
 * 488281.25 S/s в полосе 50 кГц.
 *
 * 1 кГц - 488 выборок
 * каждый отчет данных структуры map_t состоит из 4-х int32_t выборок sig_I, sig_Q, ref_I, ref_Q
 * если MAPSIZE = 256 * 1024 = 262144 (на самом деле память будет распределена 512 * 1024: вторая часть для Tx)
 * то буфер map накопится за 262144 / 488281 = 0.54 секунды. 
 * Тестовой частоте 1000 в имитаторах zboard и dsp соответствует отметка FFT:
 * NSAMPLES/FPGA = 1000 * 16384 / 488291 = 33.5
 * Эта же отметка FFT 33.5 соответствует реальной разнице частот с генератора Red Pitaya:
 * 1000 = 49999000 Гц - 49998000 Гц
*/

// столько выборок делает FPGA в секунду
#define FPGA (488281.25)

// тестовые режимы

// ZBOARD mode: zboard/
// 0: без теста, работа с device XDMA
// 1: вместо XDMA при инициализации gMap заполняется возрастающими с 0 int32 значениями:
//      sig i = 0, sig q = 1, ref i = 2, ref q = 3, sig i = 10, sig q = 11, ... 
// 2: gMap заполняется на лету FM данными 
#define ZBOARD_TESTMODE (0)

// DSP mode: bi/
// 0: udp данные принимаются с zboard
// 1: в программе имитируются входные данные (sig, ref)
// 2: в программе имитируются выходные данные (afBuf)
#define DSP_TESTMODE (0)

// MULTI mode: multix86/
// 0: udp данные принимаются с dsp
// 1: входные данные имитируются в самой программе
#define MULTI_TESTMODE (1)

// 1: адаптивный фильтр включен; 0: выключен
#define ADAPT (0)

// =1 посылать af матрицу по UDP
// настройка для main_app/proc и multix
#define AF_UDP (1)

#define NSAMPLES (4*1024)  /* must be multiple of 128 for cache/DMA reasons */

// параметры адаптивного фильтра
// размер входной выборки: SLICESIZE
// количество звеньев TAPS = SLICESIZE - 1
// коэффициентов p: TAPS
// коэффициентов h: SLICESIZE
// размер выходной выборки: SLICESIZE
// assert(SLICESIZE * SLICES == NSAMPLES)
#define SLICESIZE (1024) 
//#define TAPS (SLICESIZE - 1)
#define TAPS (512)
#define SLICES (4)
//#define SLICES (1)

#if SLICESIZE * SLICES != NSAMPLES
	#error SLICESIZE * SLICES != NSAMPLES !
#endif

// time axe for AF
//ROWS = 512 for max range 150km and srate=1MSamples/s
#define ROWS (32)   // размерность по времени неусеченной в кольцевой буфер матрицы AF

// столько строк длиной short NSAMPLES занимает лог в abBuf
#define LOGROWSLEN (7)

/*
размер окна CFAR
www--C--www
размер промежуточных данных AF перед FAR:
NSAMPLES * WINSIZE 
*/
#define WINSIZE (7)     // должно быть нечетным!
#define HALFWIN ((WINSIZE - 1)/2)

/* ***** переменные ниже не меняются ******/

// 8 bits in byte
#define BYTESIZE (8)
#define INTSIZE (4)
// BYTES! 2* due to complex samples, 2* due to sizeof(short)
// OR 2*2 due to sizeof(int)
#define BUFSIZE  (NSAMPLES * INTSIZE)

// сигнальный буфер считывается со смещением вплоть до ROWS, поэтому не должно быть выхода за пределы буфера
// SIGBUFSIZE используется в двух разных смыслах, но с одним и тем же значением:
// 1. отдельный I или Q буфер состоящий из int32_t слов (вход neFilter)
// 2. совместный I/Q буфер состоящий из int16_t слов (выход neFilter, вход afmath)
// так должно было быть с запасом ROWS справа потому что корр. функция идет со сдвигом справа:
// #define SIGBUFSIZE  ((NSAMPLES+ROWS) * INTSIZE)
// однако для удобства и округления замещаем ROWS на SLICESIZE, поэтому
#define SIGBUFSIZE  ((NSAMPLES + SLICESIZE) * INTSIZE)

#if SLICESIZE < ROWS
	#error SLICESIZE < ROWS !
#endif

// размерность матрицы AF - не количество байт!
#define ABUFSIZE (NSAMPLES * ROWS)

#define DISP_X (800)
#define DISP_Y (600)
#define DISPSIZE (DISP_X * DISP_Y)
#define DISP_X_IDENT (600)
// видеобуфер размером 800х600 ячеек, каждая 2 байта (слово) 
#define DISPBUFSIZE (2 * DISPSIZE)
// признак пересечения эллипсов, должен отличаться от кода цвета
#define EL_CROSS (65535)
// расширение пикселов CFAR вправо и вниз соответственно
#define PIXSIZE_X (1)
#define PIXSIZE_Y (1)

#define MASK (128)

#if MASK == (128)
	// CFAR отображается как есть: бит к пикселю
	#define MASKLEN (1)
#elif MASK == (128 + 64)
	// CFAR 2 bits -> 1 pix
	#define MASKLEN (2)
#elif MASK == (128 + 64 + 32 + 16)
	// CFAR 4 bits -> 1 pix
	#define MASKLEN (4)
#else
	#error WRONG MASK TYPE !
#endif 

#define PORT2BI (32000)
#define PORT2MULTI_1 (32001)
#define PORT2MULTI_2 (32002)

#endif

/*
mod.sh:
MODPATH=/lib/modules/3.0.50/kernel/drivers/dsp/

# start syslink and cmem
insmod $MODPATH/syslink.ko TRACE=1 TRACEFAILURE=1

# bbxm mem=256M version
insmod $MODPATH/cmemk.ko phys_start=0x90000000 phys_end=0x90C00000 pools=20x4096,10x131072,2x5242880

omap3530_memmap.txt:
# DDR2
DSP, 0x91C00000, 0x91C00000, 0x91C00000, 0x2000000, 0x5, 0
# ALG Heap
DSP, 0x90C00000, 0x90C00000, 0x90C00000, 0x1000000, 0x5, 0
# SR 1
DSP, 0x93D00000, 0x93D00000, 0x93D00000, 0x0100000, 0x5, 0
# SR 0
DSP, 0x93C00000, 0x93C00000, 0x93C00000, 0x0100000, 0x5, 0

*/

