/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <ti/dsplib/dsplib.h>

#include "../../af.h"
#include "afmath.h"

#include <xdc/std.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/universal/universal.h>
#include <ti/sdo/ce/osal/Memory.h>

#include "gen_twiddle_fft16x16.h"
#include "gen_twiddle_fft16x32.h"

/* Global definitions */
void vec_cx_mpy(const short *restrict a, const short *restrict b, short *restrict c);
void vec_cx_dot(const short *restrict inBuf, UInt *restrict outBuf);
void write_bit(UChar* cfarBuf, int f, int t, Bool bit);

void log_af(int id, const char* buf, UShort* afBuf);
void log_array_c(const char* str, const short* iqBuf, int offset, int N, const char* file, int line, const char* function);
void log_array_ci(const char* str, const int* iqBuf, int offset, int N, const char* file, int line, const char* function);
void log_array_uchar(const char* str, const UChar* buf, int offset, int N, const char* file, int line, const char* function);
void log_array_short(const char* str, const short* buf, int offset, int N, const char* file, int line, const char* function);
void log_array_ushort(const char* str, const UShort* buf, int offset, int N, const char* file, int line, const char* function);
void log_array_int(const char* str, const int* buf, int N, const char* file, int line, const char* function);
void log_array_uint(const char* str, const UInt* buf, int offset, int N, const char* file, int line, const char* function);


volatile clock_t clk()
{
  //return clock();
  return Timestamp_get32();
}
clock_t start, stop;
clock_t overhead;

void clock_start()
{
    start = clk();
    stop = clk();
    overhead = stop - start;
    start = clk();
}

void clock_stop(char* str)
{
    int delta;
    stop = clk(); 
    if(stop < start)
		delta = (0xFFFFFFFF - start) + stop - overhead;
    else
		delta = stop - start - overhead;
    //Log_print3(Diags_USER1, "\n\n%s: %ld ticks, ~%d us\n", (IArg)str, delta, delta/200);
}

/* 
    Scale need to be applied (1/2^SCALE) to input data for 16x32 and 32x32
    FFT for their output to match 16x16. This is dure to the inherent 
    scaling present in 16x16 kernel
*/
// XXX

//#define SCALE_MATRIX2AF (6)
#define SCALE_MATRIX2AF (16)
#define SCALE_FFT2SHORT (NSAMPLES)
#define SCALE_COMPLEXMUX (16)	// для памяти: не забывать!
#define LOG1

/* Align the tables that we have to use */
//#pragma DATA_ALIGN(twiddles, 8);

/*

Правила для logMode расписаны в файле logrules.txt

*/

void afmath_init(short* twiddle)
{
    gen_twiddle_fft16x32(twiddle, NSAMPLES);
    //log_array_c("twiddle", (Short*)twiddle, 0, 64, __FILE__, __LINE__, __FUNCTION__);
}

// внимание: портит sigBuf!
void afmath_fft(short* sigBuf, UChar* tmpBuf, UChar* tmp2Buf, short* twiddle)
{
    clock_start();
    DSP_fft16x32(twiddle, NSAMPLES, (int*)sigBuf, (int*)tmpBuf);
    clock_stop("Standalone FFT");
    //log_array_ci("tmp2Buf", (int*)tmp2Buf, 0, 32, __FILE__, __LINE__, __FUNCTION__);
    //log_array_c("standalone FFT16", b, 0, 8, __FILE__, __LINE__, __FUNCTION__);
}


void afmath_af(const Short* sigBuf, Short* refBuf, UChar* tmpBuf, UChar* tmp2Buf, UInt* matrixBuf, UShort* afBuf, const Short* twiddle, UShort logMode)
{
	//log_array_c("sigBuf", (Short*)sigBuf, 0, 16, __FILE__, __LINE__, __FUNCTION__);
	//log_array_c("refBuf", (Short*)refBuf, 0, 16, __FILE__, __LINE__, __FUNCTION__);

	// refBuf = conj(refBuf)
	int f;
	for(f = 0; f < NSAMPLES; f++)
	{
		// порядок следования ImRe из-за функции vec_cx_mpy: она интерпретирует таким образом
		refBuf[2*f] = - refBuf[2*f];
	}
	if(logMode < ROWS)
	{
		//Log_print2(Diags_USER1, "\nafmath_af logMode=%d < ROWS=%d\n", logMode, ROWS);
		// 1)
		log_af(1, (char*)refBuf, afBuf);
	}

	// Time shift scan
	int t;

	Int* pIntTmp = (Int*)tmpBuf;
	UInt* pUIntTmp = (UInt*)tmpBuf;
	UShort* pUShortTmp = (UShort*)tmpBuf;
	Short* pShortTmp2 = (Short*)tmp2Buf;

	int warn1 = 0;
	int warn2 = 0;
	for(t = 0; t < ROWS; t++)
	{
		//if(t>8) continue; // FIXME
		//Log_print1(Diags_USER1,yy "\n\nt: %d", t);
		//start = clk();

		// sigBuf, refBuf идут в формате ImRe
		// tmpBug = sigBuf * refBuf
		// результат идет в порядке ImRe
		// результат в tmpBuf будет округлен: сдвинут вправо на 16 разрядов, т.е. разделен на 65536

		// clock_start();
		// tmpBuf = sigBuf * refBuf: complex multiplication
		vec_cx_mpy(sigBuf + t*2, refBuf, (Short*)tmpBuf);
		// if(t == 0)
			//clock_stop("mpy");
		if(logMode == t)
		{
			// log_array_c("sig*ref", (Short*)tmpBuf, 0, 8, __FILE__, __LINE__, __FUNCTION__);
			// лучше sig показать здесь а не выше: увидеть сдвиг
			// 0)
			log_af(0, (char*)(sigBuf + t*2), afBuf);
			// 2)
			log_af(2, (char*)tmpBuf, afBuf);
		}

		for(f = 0; f < NSAMPLES; f++)
		{
			// преобразуем short в int для FFT
			// восстанавливаем порядок ReIm
			*((int*)tmp2Buf + 2*f + 1) = *((short*)tmpBuf + 2*f);
			*((int*)tmp2Buf + 2*f) = *((short*)tmpBuf + 2*f + 1);
		}	    

		clock_start();
		//log_array_ci("sig*ref norm", (int*)tmp2Buf, 0, 8, __FILE__, __LINE__, __FUNCTION__);
		DSP_fft16x32(twiddle, NSAMPLES, (int*)tmp2Buf, (int*)tmpBuf);
		if(t == 0)
			clock_stop("FFT");
		//if(t == 0)
		//	log_array_ci("DSP", (Int*)(tmpBuf), 64, 64, __FILE__, __LINE__, __FUNCTION__);

		if(logMode == t)
		{
			// 3)
			log_af(3, (char*)tmpBuf, afBuf);
		}

		Int intscale;
		for(f = 0; f < 2*NSAMPLES; f++)
		{
			intscale = pIntTmp[f] / (SCALE_FFT2SHORT);
			// выше объявлено Short* pShortTmp2 = (Short*)tmp2Buf
			pShortTmp2[f] = (Short)intscale;
			// проверка на возможное переполнение
			if(intscale < SHRT_MIN + 10 || intscale > SHRT_MAX - 10)
				warn1++;
		}

		if(logMode == t)
		{
			// 4)
			log_af(4, (char*)pShortTmp2, afBuf);
			//log_array_c("log short DSP pShortTmp2", pShortTmp2, 0, 8, __FILE__, __LINE__, __FUNCTION__);
			//log_array_c("log short DSP logPtr", (Short*)logPtr, 0, 8, __FILE__, __LINE__, __FUNCTION__);
		}

		//log_array_c("rounded DSP", (Short*)tmp2Buf, 0, 8, __FILE__, __LINE__, __FUNCTION__);

		clock_start();
		vec_cx_dot((Short*)tmp2Buf, (UInt*)tmpBuf);
		if(t == 0)
			clock_stop("dot");

		if(logMode == t)
		{
			// 5)
			log_af(5, (char*)tmpBuf, afBuf);
			log_array_uint("dot", (UInt*)tmpBuf, 0, 16, __FILE__, __LINE__, __FUNCTION__);
		}

		//log_array_uint("dot", (UInt*)tmpBuf, 0, 4, __FILE__, __LINE__, __FUNCTION__);

		// !!! сейчас tmpBuf должен интерпретироваться не как short массив, а как UInt 
		//if(t == 8)
		//log_array_uint("dot", pUInt, 0, NSAMPLES, __FILE__, __LINE__, __FUNCTION__);

		UShort mscale;
		// переопределение указателя - здесь нужен указатель на short
		for(f = 0; f < NSAMPLES; f++)
		{
			// добавляем массив dot к матрице для CFAR в формате int
			// оставляя слева столбцы шириной в половину окна CFAR
			// выше объявлено UInt* pUIntTmp = (UInt*)tmpBuf
			// выше объявлено UShort* pUShortTmp = (UShort*)tmpBuf;
			matrixBuf[(t + HALFWIN)*NSAMPLES + f] = pUIntTmp[f];
#if DSP_TESTMODE == 2
			// тестовая возрастающая последовательность
			mscale = (UShort)((t*NSAMPLES + f) >> 3);
#else
			mscale = (UShort)(pUIntTmp[f] >> SCALE_MATRIX2AF);
#endif
			if(logMode >= ROWS)
			{
				// режим без лога: afBuf записывается как есть 
				afBuf[t*NSAMPLES + f] = mscale;
			}
			// проверка на возможное переполнение
			if(mscale > USHRT_MAX - 10)
				warn2++;
			// возвращаем масштабированное значение в массив для лога
			pUShortTmp[f] = mscale;
		} // for(f)
		// 6)
		// логирование:
		// в конец лога записывается одна строка afBuf(t)
		if(logMode == t)
		{
			//log_af(6, (char*)pUIntTmp, afBuf);
			log_af(6, (char*)tmpBuf, afBuf);
			//log_array_uint("matrix", matrixBuf, t*NSAMPLES, 16, __FILE__, __LINE__, __FUNCTION__);
			//log_array_uint("pUIntTmp", pUIntTmp, t*NSAMPLES, 16, __FILE__, __LINE__, __FUNCTION__);
			//log_array_ushort("afBuf", afBuf, (t+6)*NSAMPLES, 16, __FILE__, __LINE__, __FUNCTION__);
		}
	} //for(t)
	if(warn1)
		Log_print1(Diags_USER1, "\nafmath_af FFT: probably overflow int -> short %d times\n", warn1);
	if(warn2)
		Log_print1(Diags_USER1, "\nafmath_af dot: probably overflow int -> short %d times\n", warn2);
}

#define WINCELLS 4	// столько ячеек суммируется, зависит от количества слагаемых выше

UShort afmath_cfar2(UInt* matrixBuf, UChar* cfarBuf, UInt cfarLevel)
{
	clock_start();
	int f, t;	// текущая частота
	int pos;
	UInt cut;	// Cell Under Test
	UInt win;	// уровень во всех соседних ячейках окна
	//Log_print1(Diags_USER1, "\ncfarLevel: %d\n", cfarLevel);
	memset(cfarBuf, 0, ABUFSIZE/BYTESIZE);
	int count = 0; 	// уровень false alarm - сколько было выше порога на весь массив
	// заполнение граничных столбцов матрицы CFAR слева и справа
	for(f=0; f<NSAMPLES; f++)
	{
		// заполнение левого поля
		matrixBuf[(t + 2)*NSAMPLES + f] =  matrixBuf[(t + 4)*NSAMPLES + f];
		matrixBuf[(t + 1)*NSAMPLES + f] =  matrixBuf[(t + 5)*NSAMPLES + f];
		matrixBuf[(t + 0)*NSAMPLES + f] =  matrixBuf[(t + 6)*NSAMPLES + f];
		// заполнение правого поля
		matrixBuf[(t + ROWS + 0)*NSAMPLES + f] =  matrixBuf[(t + ROWS - 2)*NSAMPLES + f];
		matrixBuf[(t + ROWS + 1)*NSAMPLES + f] =  matrixBuf[(t + ROWS - 3)*NSAMPLES + f];
		matrixBuf[(t + ROWS + 2)*NSAMPLES + f] =  matrixBuf[(t + ROWS - 4)*NSAMPLES + f];
	}
	int tm;	// координата t в matrix
	for(f=0; f<NSAMPLES; f++)
	{
		//Log_print1(Diags_USER1, "=========== f: %d", f);
		for(t = 0, tm = HALFWIN; tm < ROWS + HALFWIN; t++, tm++)
		{
#if WINSIZE == 7    
			// значения ячеек окна с пропуском на guard ячейки
			// XXX количество слагаемых должно быть меньше HALFWINSIZE !!! XXX
			win = (
					matrixBuf[(tm - 2)*NSAMPLES + f] +
					matrixBuf[(tm - 3)*NSAMPLES + f] + 
					matrixBuf[(tm + 2)*NSAMPLES + f] + 
					matrixBuf[(tm + 3)*NSAMPLES + f]
				  ) / WINCELLS;

#else
#error "*** WINSIZE ERROR ***"
#endif
			// TODO сравнение с порогом: int _cmpgt2(int src1, int src2);
			// TODO или еще лучше сразу записывать биты после сравнения: int _cmpeq4(int src1, int src2);
			// TODO запись бита на определенное место: uint _set(uint src2, uint csta, uint cstb);
			cut = matrixBuf[tm*NSAMPLES + f];
			if(cut > (win + cfarLevel))
			{
				pos = f + NSAMPLES * t;
				cfarBuf[pos/8] |= (128 >> (pos % 8));
				//Log_print3(Diags_USER1, "%d[f=%d t=%d]", cut, f, t);
				count++;
			}
		} //for(t)
	} //for(f)
	clock_stop("CFAR");
	Log_print1(Diags_USER1, "\nFA: %d times\n", count);
	return count;
}

// произведение двух массивов комплексных чисел c = a * b размером 2*NSAMPLES
// порядок Im, Re
// результат помещается также в short за счет округления вправо на 16 разрядов
// XXX отбрасывает младшую часть!
void vec_cx_mpy(const short *restrict a, const short *restrict b, short *restrict c)
{
	unsigned int a3_a2, a1_a0; /* Packed 16-bit values */
	unsigned int b3_b2, b1_b0;
	unsigned int c3_c2, c1_c0;
	int i;

	#pragma MUST_ITERATE (2*NSAMPLES, 2*NSAMPLES);
	for (i = 0; i < 2*NSAMPLES; i += INTSIZE)
	{
		/* Load two complex numbers from the a[] array. */
		/* The complex values loaded are represented as ’a3 + a2 * j’ */
		/* and ’a1 + a0 * j’. That is, the real components are a3 */
		/* and a1, and the imaginary components are a2 and a0. */
		a3_a2 = _hill(_mem8(&a[i]));
		a1_a0 = _loll(_mem8(&a[i]));
		//Log_print2(Diags_USER1, "a3_a2: %d a1_a0: %d", a3_a2, a1_a0);
		/* Load two complex numbers from the b[] array. */
		b3_b2 = _hill(_mem8(&b[i]));
		b1_b0 = _loll(_mem8(&b[i]));
		//Log_print2(Diags_USER1, "b3_b2: %d b1_b0: %d", b3_b2, b1_b0);
		/* Perform the complex multiplies using _cmpyr. */
		// результат округлен: сдвинут вправо на 16 разрядов, т.е. разделен на 65536
		c3_c2 = _cmpyr(a3_a2, b3_b2);
		c1_c0 = _cmpyr(a1_a0, b1_b0);
		//Log_print2(Diags_USER1, "c3_c2: %d c1_c0: %d", c3_c2, c1_c0);
		/* Store the results. */
		_mem8(&c[i]) = _itoll(c3_c2, c1_c0);
	}
}

// вычисляет dot продукт Im, Re частей входного комплексного массива размером 2*NSAMPLES
// out = Im*Im + Re*Re
// выходной результат в UInt, за счет этого точность не теряется
void vec_cx_dot(const short *restrict inBuf, UInt *restrict outBuf)
{
	/*************************
	расположить NSAMPLES комплексных отсчета (x2) short tmp2Buf
	в буфер модулей длиной NSAMPLES int tmpBuf
	*************************/

	// 4 shorts in 2 ints
	unsigned int re0_im0, re1_im1; 
	unsigned int d0, d1;
	int i, j;

    #pragma MUST_ITERATE (2*NSAMPLES, 2*NSAMPLES);
	for(i = 0, j = 0; i < 2*NSAMPLES; i += 2, j++)
	{
	    // обрабатывается участок буфера длиной 16 bytes, 8 shorts, 4 комплексных числа
	    // re0 = buf[0]
	    // im0 = buf[1]

	    // 8 байт, 4 shorts, 2 комплексных числа
	    re0_im0 = _loll(_mem8(&inBuf[i]));
	    re1_im1 = _hill(_mem8(&inBuf[i]));

	    // модули двух комплексных чисел, 2 ints
	    // d0 = re0 * re0 + im0 * im0
	    d0 = _dotp2(re0_im0, re0_im0);
	    // d1 = re1 * re1 + im1 * im1
	    d1 = _dotp2(re1_im1, re1_im1);

	    // tmpBuf освободился после умножения, задействуем для размещения результата
	    // 2 ints пакуется в 8 bytes 
	    // в результате массив NSAMPLES int должен быть равен BUFSIZE
	    _mem8(&outBuf[j]) = _itoll(d1, d0);
	} //for(f)
}

/*
	tcp лог внутренних массивов, каждая запись длиной NSAMPLES short
	записи располагаются в afBuf друг за другом в случае если logMode < ROWS,
	в этом случае вместо матрицы AF в afBuf формируется лог
	номер записи id соответствует описанию logrules.txt

	поскольку длина записи ограничена NSAMPLES short, копируется не весь массив.
	short буфер скопировался бы весь
	short iq больше в 2 раза: копируется первая и последняя четверть буфера
	int больше в 2 раза: копируется первая и последняя четверть буфера
	int iq больше в 4 раза: копируется первая и последняя восьмая часть буфера 

	с каким типом буфера мы имеем дело, определяем по id
	смещение shift определяет место каждого id в afBuf
*/

void log_af(int id, const char* buf, UShort* afBuf)
{
	int offset; // смещение short в afBuf
	int len; // длина копируемого буфера в БАЙТАХ

	switch(id)
	{
		// 0)
		// short sig iq
		case 0: len = sizeof(short) * NSAMPLES * 2; break;
		// 1)
		// short conj(ref) iq
		case 1: len = sizeof(short) * NSAMPLES * 2; break;
		// 2)
		// short sig*ref iq
		case 2: len = sizeof(short) * NSAMPLES * 2; break;
		// 3)
		// int dsp iq
		case 3: len = sizeof(int) * NSAMPLES * 2; break;
		// 4)
		// short dsp iq
		case 4: len = sizeof(short) * NSAMPLES * 2; break;
		// 5)
		// int dot
		case 5: len = sizeof(int) * NSAMPLES; break;
		// 6)
		// short af
		case 6: len = sizeof(short) * NSAMPLES; break;
		// unknown id ?
		default: return;
	}

	// здесь значение id будет проверенным
	offset = id * NSAMPLES;

	const char* src;
	UShort* dst;
	// копирование левой половины буфера
	src = buf;
	dst = afBuf + offset;
	memcpy((char*)dst, src, NSAMPLES);

	// копирование правой половины буфера
	src = buf + len - NSAMPLES;
	dst = afBuf + offset + NSAMPLES/2;
	memcpy((char*)dst, src, NSAMPLES);
}

// выводит комплексный массив short чисел длиной N комплексных чисел
void log_array_c(const char* str, const short* iqBuf, int offset, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		// ImRe order
		Log_print3(Diags_USER1, "[%d] %d / %d",\
			n+offset, iqBuf[2*(n+offset)], iqBuf[2*(n+offset)+1]);
	#endif
}

// выводит комплексный массив int чисел длиной N комплексных чисел
void log_array_ci(const char* str, const int* iqBuf, int offset, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		// ImRe order
		Log_print3(Diags_USER1, "[%d] %d / %d",\
			n+offset, iqBuf[2*(n+offset)], iqBuf[2*(n+offset)+1]);
	#endif
}

// выводит массив чисел UChar длиной N
// выводит массив чисел UChar длиной N
void log_array_uchar(const char* str, const UChar* buf, int offset, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		Log_print3(Diags_USER8, "[%d] %d 0x%x",\
			n+offset, buf[n+offset], buf[n+offset]);
	#endif
}
// выводит массив чисел short длиной N
void log_array_short(const char* str, const short* buf, int offset, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		Log_print2(Diags_USER8, "[%d] %d",\
			n+offset, buf[n+offset]);
	#endif
}

// выводит массив чисел unsigned short длиной N
void log_array_ushort(const char* str, const UShort* buf, int offset, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		Log_print2(Diags_USER8, "[%u] %u",\
			n+offset, buf[n+offset]);
	#endif
}

// выводит массив чисел int длиной N
void log_array_int(const char* str, const int* buf, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		Log_print2(Diags_USER8, "[%d] %d",\
			n, buf[n]);
	#endif
}

// выводит массив чисел UInt длиной N
void log_array_uint(const char* str, const UInt* buf, int offset, int N, const char* file, int line, const char* function)
{
	#ifdef LOG1
	Log_print4(Diags_USER1, "\n\n%s:%d, %s()\n%s\n-----",\
		(IArg)file, line, (IArg)function, (IArg)str);
	int n;
	for(n=0; n<N; n++)
		Log_print2(Diags_USER8, "[%u] %u",\
			n+offset, buf[n+offset]);
	#endif
}
