/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <image.h>
/* Global definitions */
/* Input NxN grayscale 8-bpp image */
extern char inBuf[N][N];
int col_re[N][N]={0};
int col_im[N][N]= {0};
/* Align the tables that we have to use */
#pragma DATA_ALIGN(x_ref, 8);
int   x_ref [2*N];
#pragma DATA_ALIGN(x_16x16, 8);
short   x_16x16 [2*N];
#pragma DATA_ALIGN(y_16x16, 8);
short   y_16x16 [2*N];
#pragma DATA_ALIGN(w_16x16, 8);
short   w_16x16 [2*N];
#pragma DATA_ALIGN(x_16x32, 8);
int   x_16x32 [2*N];
#pragma DATA_ALIGN(y_16x32, 8);
int   y_16x32 [2*N];
#pragma DATA_ALIGN(w_16x32, 8);
short   w_16x32 [2*N];
#pragma DATA_ALIGN(x_16x16, 8);
int   x_16x16 [2*N];
#pragma DATA_ALIGN(y_16x16, 8);
int   y_16x16 [2*N];
#pragma DATA_ALIGN(w_16x16, 8);
int   w_16x16 [2*N];

int n=0;

void generateInput (n) 
{
	// TODO: scale input data img255 -> int16
	for(i=0;i<N;i++)
	{
		x_16x16[2*i] = inBuf[n][i];     
		x_16x16[2*i + 1] = 0;
	}
}

void mfilter_fft_init () 
{
	int i,j;
	gen_twiddle_fft16x16(w_16x16, N);
}

void mfilter_fft2()
{
	/* Do FFTs across each row of the input image */
	for(n =0;n<N;n++)
	{
		/* Generate the input data */
		generateInput (n);
		DSP_fft16x16(w_16x16, N, x_16x16, y_16x16);
		for (i = 0, j = 0; j < N; i+=2, j++)
		{
			col_re[n][j] = y_16x16[i];
			col_im[n][j] = y_16x16[i + 1];
		}
	}
	/* Do FFTs across each column of the output array of the previous row FFTs*/
	for(n =0;n<N;n++)
	{
		for (i = 0, j = 0; j < N; i+=2, j++) 
		{
			x_16x16[i] = col_re[j][n]/N;
			x_16x16[i + 1] = col_im[j][n]/N;
		}
		DSP_fft16x16(w_16x16, N, x_16x16, y_16x16);
		for (i = 0, j = 0; j < N; i+=2, j++) 
		{
			col_re[j][n] = y_16x16[i];
			col_im[j][n] = y_16x16[i + 1];
		}
	}

void ifft2()
{
	/* Do IFFT across each row of the previous 2D-FFT output array */
	for(n =0;n<N;n++)
	{
		/* Generate the input data */
		for (i = 0, j = 0; j < N; i+=2, j++) 
		{
			x_16x16[i] = col_re[n][j];
			x_16x16[i + 1] = col_im[n][j];
		}
		/* Call the various FFT routines */
		DSP_ifft16x16(w_16x16, N, x_16x16, y_16x16);
		for (i = 1, j = 0; j < N; i+=2, j++)
		{
			col_re[n][j] = y_16x16[i];
			col_im[n][j] = y_16x16[i + 1];
		}
	}
}

	/* Do IFFT across each column of the output array of the previous row IFFTs */
	for(n =0;n<N;n++)
	{
		for (i = 0, j = 0; j < N; i+=2, j++) 
		{
			x_16x16[i] = col_re[j][n];
			x_16x16[i + 1] = col_im[j][n];
		}
		DSP_ifft16x16(w_16x16, N, x_16x16, y_16x16);
		for (i = 0, j = 0; j < N; i+=2, j++) 
		{
			col_re[j][n] = y_16x16[i];
			col_im[j][n] = y_16x16[i + 1];
		}
	}
}
