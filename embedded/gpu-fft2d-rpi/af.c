/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "mailbox.h"
#include "gpu_fft.h"

/*
 Ось x_samples = N выборок для FFT и результат FFT одновременно
 Ось y_samples = сколько раз мы сдвигаем сигнал перед FFT
 Функция неопределенности имеет размерность x_samples x y_samples
*/

char Usage[] =
    "Usage: af.bin x-samples_as_log2_N\n"
    "log2_N = log2(FFT_length),       log2_N = 8...22\n";

// check GPU prepare errors
int test_ret(int ret)
{
    switch(ret) 
	{
        case -1: fprintf(stderr, "Unable to enable V3D. Please check your firmware is up to date.\n"); return -1;
        case -2: fprintf(stderr, "This value log2_N not supported.  Try between 8 and 22.\n");         return -1;
        case -3: fprintf(stderr, "Out of memory.  Try a smaller batch or increase GPU memory.\n");     return -1;
        case -4: fprintf(stderr, "Unable to map Videocore peripherals into ARM memory space.\n");      return -1;
        case -5: fprintf(stderr, "Can't open libbcm_host.\n");                                         return -1;
    }
	return ret;
}

unsigned Microseconds(void) 
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

typedef struct GPU_FFT_COMPLEX cx_t;

// multiply two complex numbers
cx_t mul_cx(cx_t a, cx_t b)
{
	cx_t out;
	out.re = a.re*b.re - a.im*b.im;
	out.im = a.re*b.im + a.im*b.re;
	return out;
}

// generate signal and reference with time and freq shift
void genIQ(cx_t* sig, cx_t* ref, int N)
{
	// input data
	float f = 100.0; // frequency
	float twight = 50.0; // signal wight
	int dt = 100; // signal time shift to right (delay)
	int df = 150; // frequency shift of signal, not ref
	float F = 2*GPU_FFT_PI/N;
	float dev = 0.8;	// frequency deviation

	cx_t f_shift, tmp;

	int i,j;
	for (i=0, j=dt; i<N; i++,j++) 
	{
		// shift sig right delay=dt points and wight=300
		if(i<= twight)
		{
			sig[j].re = 2.0*cos(F*(f+dev*i)*i);
			sig[j].im = 2.0*sin(F*(f+dev*i)*i);
			f_shift.re = 1.0*cos(-F*df*i);
			f_shift.im = 1.0*sin(-F*df*i);
			tmp = mul_cx(sig[j], f_shift);
			sig[j] = tmp;
		}
		else
		{
			// calloc must zero signal before, but we shoult assure
			sig[j].re = 0;
			sig[j].im = 0;
		}
		if(i<= twight)
		{
			ref[i].re = 2.0*cos(F*(f+dev*i)*i);
			ref[i].im = 2.0*sin(F*(f+dev*i)*i);
		}
		else
		{
			// calloc must zero signal before, but we shoult assure
			ref[i].re = 0;
			ref[i].im = 0;
		}
	}
} 

// prepare AF matrix for FFT batch
int batch_prepare(const cx_t* sig, const cx_t* ref, struct GPU_FFT *fft, int x_samples)
{
	cx_t s, r, mul;
	int x,y;
    cx_t *base;
	// y axis, size the same as x
	for(y=0; y<x_samples; y++)
	{
		base = fft->in + y*fft->step;
		// x axis
		for(x=0; x<x_samples; x++)
		{
			// shift signal
			s = sig[y + x];
			r = ref[x];
			// multiply conj 
			r.im = -r.im;
			mul = mul_cx(s, r);
			// write to batch
			base[x] = mul;
		}
	}

	return 0;
}

int main(int argc, char *argv[]) 
{
	int x_samples;
    int ret, log2_N, mb;
	int sz = sizeof(float);

    cx_t *base;
    struct GPU_FFT *fft;
	cx_t* signal = NULL;
	cx_t* refer = NULL;

    if (argc != 2) 
	{
        fprintf(stderr, Usage);
        return -1;
    }

    log2_N = atoi(argv[1]);
    x_samples = 1<<log2_N; // FFT length

	fprintf(stderr, "x_samples x x_samples = %d x %d = %d matrix\n", x_samples, x_samples, x_samples*x_samples);
	mb = mbox_open();
	// y_samples = jobs
    ret = gpu_fft_prepare(mb, log2_N, GPU_FFT_REV, x_samples, &fft); // call once
	test_ret(ret);

	// memory only for input, output live in fft structure
	signal = calloc(2*x_samples, sizeof(cx_t));
	refer = calloc(x_samples, sizeof(cx_t));
	if(signal == NULL || refer == NULL)
	{
		fprintf(stderr, "calloc error, exit\n");
    	gpu_fft_release(fft); // Videocore memory lost if not freed !
		exit(-1);
	}

	// generate input signals
	genIQ(signal, refer, x_samples);

	// preparint batch for fft
	batch_prepare(signal, refer, fft, x_samples);

	// execute batch
	usleep(1); // Yield to OSs
	gpu_fft_execute(fft); // call one or many times

	// output to stdout 
	FILE *const fout = fdopen(dup(fileno(stdout)), "wb");
	char arr[sz];
	int wrote = 0;
	int t,f;

	// scan output batch to byte stream
	for (t=0; t<x_samples; t++) 
	{
		base = fft->out + t*fft->step; // output buffer
		float tmp;
		int d;
		// x_samples/2 so need only 1st half of FFT
		for (f=0; f<x_samples/2; f++) 
		{
			tmp = base[f].re;
			memcpy(arr,&tmp,sz);
			d = fwrite(arr, 1, sz, fout);
			wrote += d;
			tmp = base[f].im;
			memcpy(arr,&tmp,sz);
			d = fwrite(arr, 1, sz, fout);
			wrote += d;
		}
	}

	fprintf(stderr, "wrote = %d bytes\n", wrote);

	fflush(fout);
	fclose(fout);
	free(signal);
	free(refer);
    gpu_fft_release(fft); // Videocore memory lost if not freed !
    return 0;
}
