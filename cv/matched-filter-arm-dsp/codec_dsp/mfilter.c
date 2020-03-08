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

#include "../mf.h"
#include "mfilter.h"

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

/* Global definitions */
void cx_fft (int16_t* bufTwiddle, int16_t* inBufSig, int16_t* outBufFft);
void cx_fft2 (const int16_t* bufTwiddle,
                    int16_t* inBufImg,
                    int16_t* bufTmp,
                    int16_t* bufImgTmp,
                    int16_t* outBufImg);
void cx_ifft2 (const int16_t* bufTwiddle,
                    int16_t* inBufImg,
                    int16_t* bufTmp,
                    int16_t* bufImgTmp,
                    int16_t* outBufImg);

void vec_cx_mpy(const short *restrict a, const short *restrict b, short *restrict c);
void vec_cx_dot(const short *restrict inBuf, UInt *restrict outBuf);

/* 
    Scale need to be applied (1/2^SCALE) to input data for 16x32 and 32x32
    FFT for their output to match 16x16. This is dure to the inherent 
    scaling present in 16x16 kernel
*/

/* Align the tables that we have to use */
//#pragma DATA_ALIGN(twiddles, 8);

void mfilter_init(int16_t* bufTwiddle,
					int16_t* inBufImgRef,
					int16_t* bufTmp,
					int16_t* bufImgTmp, 
					int16_t* bufImgRef) 
{
	/*
	memcpy(bufTmp1,inBufImgRef,2*N*sizeof(int16_t));
	logComplexArrayN("nBufImgRef", bufTmp1,0);
	*/
	// gen twiddle
    gen_twiddle_fft16x16(bufTwiddle, N);
	// once calculate conj(FFT(ref))
	cx_fft2(bufTwiddle, inBufImgRef, bufTmp, bufImgTmp, bufImgRef); 
	// 2*N because complex values!
	int n,i,j;
	for(n=0; n<N; n++)
	{
		for(i=0,j=0; j<N; i+=2,j++)
		{
			// conjuation
			bufImgRef[2*N*n + i + 1] =  - bufImgRef[2*N*n + i + 1];
		}
	}
	logComplexArrayNxN("fft(ref).conj", bufImgRef);
}

//======================================================================================
#define SCALE (16)
void mfilter_convolve(const int16_t* bufTwiddle,
						int16_t* inBufImgSig,
						int16_t* bufImgRef, 
						int16_t* bufTmp,
						int16_t* bufImgTmp1, 
						int16_t* bufImgTmp2, 
						int16_t* outBufImg) 
{
	int n,i,j;
	// FIXME Need to scale, overflow?
    for(n=0; n<N; n++)
    {
        for (i = 0, j = 0; j < N; i+=2, j++)
        {
			inBufImgSig[2*N*n + i] =  inBufImgSig[2*N*n + i] >> 3;
			inBufImgSig[2*N*n + i + 1] =  inBufImgSig[2*N*n + i + 1] >> 3;
		}
	}
	cx_fft2(bufTwiddle, inBufImgSig, bufTmp, bufImgTmp1, bufImgTmp2); 
	logComplexArrayNxN("fft(sig)", bufImgTmp2);
	// result in bufImgTmp2
	// TODO: mul without loop, change N to N*N in vec_cx_mpy function
	// complex mul sig*ref.conj
	// change ReIm -> ImRe
	int16_t sigRe;
	int16_t sigIm;
	int16_t refRe;
	int16_t refIm;
	int32_t re;
	int32_t im;
    for(n=0; n<N; n++)
    {
        for (i = 0, j = 0; j < N; i+=2, j++)
        {
			sigRe =  bufImgTmp2[2*N*n + i];
			sigIm =  bufImgTmp2[2*N*n + i + 1];
			refRe =  bufImgRef[2*N*n + i];
			refIm =  bufImgRef[2*N*n + i + 1];
			re = sigRe*refRe - sigIm*refIm;
			bufImgTmp1[2*N*n + i] = (int16_t)(re >> SCALE);
			im = sigRe*refIm + sigIm*refRe;
			bufImgTmp1[2*N*n + i + 1] = (int16_t)(im >> SCALE);
        }
    }
	/*
    for(n=0; n<N; n++)
	{
		// mul one row 
		vec_cx_mpy(bufImgTmp1 + 2*N*n, bufImgRef + 2*N*n, bufImgTmp2 + 2*N*n);
	}
	*/
	// result in bufImgTmp1

	cx_ifft2(bufTwiddle, bufImgTmp1, bufTmp, bufImgTmp2, outBufImg); 
	//logComplexArrayNxN("convolve output", outBufImg);

	// result in outBufImg
	// TODO: find max, may be in ARM?
}
//======================================================================================

void cx_fft (int16_t* bufTwiddle, int16_t* inBufSig, int16_t* outBufFft) 
{
    DSP_fft16x16(bufTwiddle, N, inBufSig, outBufFft);
}

void cx_fft2(const int16_t* bufTwiddle,
					int16_t* inBufImg,
					int16_t* bufTmp,
					int16_t* bufImgTmp,
					int16_t* outBufImg) 
{
	int n,i,j;

	// scan rows
    for(n=0; n<N; n++)
    {
        DSP_fft16x16(bufTwiddle, N, inBufImg + 2*N*n, bufTmp);
		// get row.T 
		// copy row to col
        for (i = 0, j = 0; j < N; i+=2, j++)
        {
            bufImgTmp[2*n + 2*N*j] = bufTmp[i];
            bufImgTmp[2*n + 1 + 2*N*j] = bufTmp[i + 1];
        }
    }
    for(n=0; n<N; n++)
    {
		// fft from row.T
        DSP_fft16x16(bufTwiddle, N, bufImgTmp + 2*N*n, bufTmp);
		for (i = 0, j = 0; j < N; i+=2, j++)
		{
			outBufImg[2*n + 2*N*j] = bufTmp[i];
			outBufImg[2*n + 1 + 2*N*j] = bufTmp[i+1];
		}
	}
	//logComplexArrayNxN("outBufImg", outBufImg);
}

// use fft2 to calculate ifft2:
// ifft2 = conj(fft2(conj(x)))N*N
// no need do high level conjuation because anyway we calculate real/abs values, so we have:
// fft2(conj(x))/(N*N)
void cx_ifft2(const int16_t* bufTwiddle,
					int16_t* inBufImg,
					int16_t* bufTmp,
					int16_t* bufImgTmp,
					int16_t* outBufImg) 
{
	int n,i,j;
	// conjuate input buffer
    for(n=0; n<N; n++)
	{
		for (i = 0, j = 0; j < N; i+=2, j++)
		{
			inBufImg[2*N*n + i + 1] = - inBufImg[2*N*n + i +1];
		}
	}

	cx_fft2(bufTwiddle, inBufImg, bufTmp, bufImgTmp, outBufImg); 

	// conjuate output buffer
    for(n=0; n<N; n++)
	{
		for (i = 0, j = 0; j < N; i+=2, j++)
		{
			outBufImg[2*N*n + i + 1] = - outBufImg[2*N*n + i +1];
		}
	}
}

// произведение двух массивов комплексных чисел c = a * b размером 2*N
// порядок Im, Re
// результат помещается также в short за счет округления вправо на 16 разрядов
// XXX отбрасывает младшую часть!
void vec_cx_mpy(const short *restrict a, const short *restrict b, short *restrict c)
{
	unsigned int a3_a2, a1_a0; /* Packed 16-bit values */
	unsigned int b3_b2, b1_b0;
	unsigned int c3_c2, c1_c0;
	int i;

	#pragma MUST_ITERATE (2*N, 2*N);
	for (i = 0; i < 2*N; i += 4)
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

// вычисляет dot продукт Im, Re частей входного комплексного массива размером 2*N
// out = Im*Im + Re*Re
// выходной результат в UInt, за счет этого точность не теряется
void vec_cx_dot(const short *restrict inBuf, UInt *restrict outBuf)
{
	/*************************
	расположить N комплексных отсчета (x2) short tmp2Buf
	в буфер модулей длиной N int tmpBuf
	*************************/

	// 4 shorts in 2 ints
	unsigned int re0_im0, re1_im1; 
	unsigned int d0, d1;
	int i, j;

    #pragma MUST_ITERATE (2*N, 2*N);
	for(i = 0, j = 0; i < 2*N; i += 2, j++)
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
	    // в результате массив N int должен быть равен BUFSIZE
	    _mem8(&outBuf[j]) = _itoll(d1, d0);
	} //for(f)
}

void logComplexArrayN(char* title, int16_t* buf, int n)
{
	if(n>2 && n<N-2)
			return;
	int i,j;
	for(i=0,j=0; j<N; i+=2,j++)
	{
		if(n>2 && n<N-2)
			continue;
		Log_print4(Diags_USER1, "%s Re[%d,%d] %d", (IArg)title, n,i, buf[i]);
		Log_print4(Diags_USER1, "%s Im[%d,%d] %d", (IArg)title, n,i+1, buf[i+1]);
	}
}
void logComplexArrayNxN(char* title, int16_t* buf)
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
			Log_print5(Diags_USER1, "[%03d,%03d] Re %d\tIm %d %s",
				 n,j, buf[2*N*n+i], buf[2*N*n+i+1], (IArg)title);
		}
	}
}
