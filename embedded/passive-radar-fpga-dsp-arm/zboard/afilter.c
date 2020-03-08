/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 *  ======== afilter.c ========
 *  version for ZBOARD
 */

#include "../af.h"
#include "afilter.h"
#include "zboard.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>

// уменьшение при преобразовании float -> short
#define SCALE (80.0)
//#define SCALE (10.0)
// tmp_ar в параметрах функций не передается, используется напрямую как глобальная переменная

void hcalc(ne10_float32_t* x, ne10_float32_t* d, ne10_float32_t* rxx, ne10_float32_t* Rxx, ne10_float32_t* Pdx, ne10_float32_t* a, ne10_float32_t* sig, ne10_float32_t*p, ne10_float32_t* tmp_ar, ne10_float32_t* h);
void lattice(ne10_float32_t* x, ne10_float32_t* p, ne10_float32_t* h, ne10_float32_t* b1, ne10_float32_t *f, ne10_float32_t* b, ne10_float32_t* fout);
void toeplitz(ne10_float32_t* ar, ne10_float32_t* m);
void correlate(ne10_float32_t* x1, ne10_float32_t* x2, ne10_float32_t* y);
void correlate_neon(ne10_float32_t* x1, ne10_float32_t* x2, ne10_float32_t* tmp_ar, ne10_float32_t* y);
//void correlate(ne10_float32_t* __restrict x1, ne10_float32_t* __restrict x2, ne10_float32_t* __restrict y);

// thread-safe support
typedef struct
{
	ne10_float32_t* sig_ar;
	ne10_float32_t* ref_ar;
	ne10_float32_t* y_ar;
	ne10_float32_t* rxx_ar;
	ne10_float32_t* Rxx_mat;
	ne10_float32_t* Pdx_ar;
	ne10_float32_t* a_mat;
	ne10_float32_t* s_ar;
	ne10_float32_t* p_ar;
	ne10_float32_t* h_ar;
	ne10_float32_t* f_ar;
	ne10_float32_t* b_ar;
	ne10_float32_t* b1_ar;
	ne10_float32_t* fout_ar;
	ne10_float32_t* e_ar;
	ne10_float32_t* tmp_ar;
} afilter_t;

// память для I потока
afilter_t ai;
// память для Q потока
afilter_t aq;

// запускается в thread и обрабатывает только I или Q компоненту
// размер за раз обрабатываемой выборки - N
void neFilter(const int32_t* sig, const int32_t* ref, short* y, int nt)
{
	int i;
	float amp;
	float scale;

	afilter_t* a;
	if(nt == 0)
		a = &ai;
	else
		a = &aq;

	// sig, ref имеют размерность N = NSAMPLES + SLICESIZE
	// последнее нужно для запаса справа, потому что в ФН массив сдвигается вправо на значение ROWS
	// соответственно ROWS должно быть меньше SLICESIZE, что контролируется в af.h
	// входная выборка нарезается на SLICE+1 слайсы
	// будет SLICES+1 подвыборок для каждой из которых пересчитываются TAPS коэффициентов
	// lattice запускается с параметром SLICESIZE
	// hcalc запускается с параметром TAPS !!!
	// таким образом время оценки меньше длительности сигнала

	// преобразование int -> float 
	for(i = 0; i < N; i++)
	{
		a->sig_ar[i] = (ne10_float32_t) sig[i];
		a->ref_ar[i] = (ne10_float32_t) ref[i];
	}

#if ADAPT == 0 
	// сквозной пропуск данных для отладки
	// для измерений берем только начало массива
	amp = rmsf(a->sig_ar, SLICESIZE);
	scale = (2.5 * amp) / (SHRT_MAX);
	for(i = 0; i < N; i++)
		y[i] = (short)(a->sig_ar[i] / scale);

	sleep(3);
	return;
#endif

	// sig_ar, ref_ar содержат float I 
	// цикл по слайсам, в результате просматривается вся выборка I N
	int j;
	for(i = 0, j = 0; i < SLICES + 1; i++, j += SLICESIZE) 
	{ 
		// обработка одного слайса, функции hcalc, lattice берут выборку SLICESIZE

		// I only, Q in the other thread: 
		//if(nt == 0)
		//	time_start();
		hcalc(a->ref_ar + j, a->sig_ar + j, a->rxx_ar, a->Rxx_mat, a->Pdx_ar, a->a_mat, a->s_ar, a->p_ar, a->tmp_ar, a->h_ar);
		//if(nt == 0)
		//	time_stop("  hcalc");
		//if(nt == 0)
		//	time_start();
		lattice(a->ref_ar + j, a->p_ar, a->h_ar, a->b1_ar, a->f_ar, a->b_ar, a->fout_ar + j); 
		//if(nt == 0)
		//	time_stop("  lattice");
	}

	for(i = 0; i < N; i++)
		a->e_ar[i] = (a->sig_ar[i] - a->fout_ar[i]);

	// преобразование float -> short
	// для измерений берем только начало массива
	amp = rmsf(a->e_ar, SLICESIZE);
	scale = (2.5 * amp) / (SHRT_MAX);
	//fprintf(stderr, "\t[%d] amp: %.1f, scale: %.1f\n",
    //	nt, amp, scale);

	for(i = 0; i < N; i++)
		y[i] = (short)(a->e_ar[i] / scale);
}


// Решетчатый фильтр
/*
СОБСТВЕННАЯ версия
Фильтр извлекает полезный сигнал x из входной выборки s = x + ref,
где ref - сигнал помехи

Алгоритм:
 - решетчатый фильтр настраивает коэффициенты 'p' в соответствии с ref;
 - 'b' массив решетчатого фильтра подается на вход FIR фильтра;
 - коэффициенты 'h' FIR фильтра настраиваются в соответствии с s;
 - на выходе FIR фильтра формируется оценка ref, которая вычитается из входной выборки s/
 - в результате чего на выходе фильтра присутствует сигнал x

порядок фильтра
taps

столько раз будут пересчитываться коэффициенты фильтра
нарезка входной выборки по слайсам
в каждом слайсе taps выборок
slices

размер выборки входного сигнала
N = slicesize * slices

Выход фильтра: полезный сигнал x и ошибка
*/

void lattice(ne10_float32_t* x, ne10_float32_t* p, ne10_float32_t* h, ne10_float32_t* b1, ne10_float32_t *f, ne10_float32_t* b, ne10_float32_t* fout)
{

	//f = x
	//b = x
	int i;
	for(i = 0; i < SLICESIZE; i++)
	{
		f[i] = x[i];
		b[i] = x[i];
		fout[i] = x[i] * h[0];
	}

	int k;
	ne10_float32_t temp;
	// внешний цикл по звеньям фильтра
	for(k = 0; k < TAPS; k++)
	{
		b1[0] = 0;
		for(i = 1; i < SLICESIZE; i++)
			b1[i] = b[i-1];

		// внутренний цикл: полный выход за все время на выходе одного звена
		for(i = 0; i < SLICESIZE; i++)
		{
			temp = f[i] - p[k] * b1[i];
			b[i] = b1[i] - f[i] * p[k];
			f[i] = temp;
			fout[i] += b[i] * h[k+1];
		}
	}
	return;
}

/*
вычисление коэффициентов отражения p и синтезирующего фильтра h
на основе входной выборки x
для коэффициентов h дополнительно нужна выборка желаемого сигнала d

A[столбец, строка]

size of p : len-1 (TAPS)!
size of h : len (SLICESIZE)
*/

void hcalc(ne10_float32_t* x, ne10_float32_t* d, ne10_float32_t* rxx, ne10_float32_t* Rxx, ne10_float32_t* Pdx, ne10_float32_t* a, ne10_float32_t* s, ne10_float32_t*p, ne10_float32_t* tmp_ar, ne10_float32_t* h)
{
	// самостоятельное вычисление коэффициентов
	int i, k;

	for(i = 0; i < TAPS; i++)
	{
		p[i] = 0;
		s[i] = 0;
	}
	for(i = 0; i < TAPS*TAPS; i++)
	{
		a[i] = 0;
		Rxx[i] = 0;
	}
	correlate_neon(x, x, tmp_ar, rxx);
	toeplitz(rxx, Rxx);
	correlate_neon(d, x, tmp_ar, Pdx); 

	p[0] = rxx[1] / rxx[0];
	a[0] = p[0];
	s[0] = rxx[0] * (1 + (p[0]*p[0]));
	h[0] = Pdx[0] / rxx[0];
	h[1] = (Pdx[1] - a[0] * Pdx[0]) / s[0];

	ne10_float32_t Ra;
	ne10_float32_t m3;
	for(k = 1; k < TAPS; k++)
	{
		/*
		   m1 = Rxx[k+1, 1:k+1]
		   m2 = a[0:k, k-1]
		   Ra = np.sum(m1 * m2)
		 */
		Ra = 0;
		for(i = 0; i < k; i++)
			Ra += Rxx[(k+1)*TAPS + i+1] * a[i*TAPS + k-1];

		p[k] = (rxx[k+1] + Ra) / s[k-1];
		a[k*TAPS + k] = p[k];

		for(i = 0; i < k; i++)
			a[i*TAPS + k] = a[i*TAPS + k-1] - p[k] * a[(k-1-i)*TAPS +  k-1]; 

		s[k] = s[k-1] * (1-p[k]*p[k]);

		m3 = 0;
		for(i = 0; i < k+1; i++)
			m3 += Pdx[i] * a[(k-i)*TAPS + k];

		h[k+1] = (Pdx[k+1] + m3)/s[k];
	}
}

/*
 Преобразование Теплица
 for i in range(6):
   for j in range(6):
     b[i,j] = a[np.abs(i-j)]
*/
void toeplitz(ne10_float32_t* ar, ne10_float32_t* m)
{
	int i,j;
	for(i = 0; i < TAPS; i++)
		for(j = 0; j < TAPS; j++)
			m[j + i*TAPS] = ar[abs(i-j)];
}

//void correlate(ne10_float32_t* __restrict x1, ne10_float32_t* __restrict x2, ne10_float32_t* __restrict y)
void correlate(ne10_float32_t* x1, ne10_float32_t* x2, ne10_float32_t* y)
{
	int i, k;
	for(k = 0; k < TAPS; k++)
	{
		y[k] = 0;
		for(i = 0; i < TAPS; i++)
			if(i+k < TAPS)
				y[k] += x2[i] * x1[i+k];
	}
}

void correlate_neon(ne10_float32_t* x1, ne10_float32_t* x2, ne10_float32_t* tmp_ar, ne10_float32_t* y)
{
	int i, k;
	for(k = 0; k < TAPS; k++)
	{
		ne10_mul_float_neon(tmp_ar, x1 + k, x2, TAPS - k);

		y[k] = 0;
		for(i = 0; i < TAPS - k; i++)
			y[k] += tmp_ar[i];

	}
}

int neInit()
{
	assert(SLICESIZE*(SLICES+1) == N);
	ne10_result_t status;
	fprintf (stderr, "Going to initialze NE10...\n");

	status = ne10_init();

	if (status != NE10_OK)
	{
		fprintf (stderr, "NE10 init failed.\n");
		return -1;
	}

	fprintf (stderr, "NE10 has been initialized.\n");
	return 0;
}

int neInitThread(int nt)
{
	afilter_t* a;
	if(nt == 0)
		a = &ai;
	else
		a = &aq;

	a->sig_ar = NULL;
	a->ref_ar = NULL;
	a->y_ar = NULL;
	a->rxx_ar = NULL;
	a->Rxx_mat = NULL;
	a->Pdx_ar = NULL;
	a->a_mat = NULL;
	a->s_ar = NULL;
	a->p_ar = NULL;
	a->h_ar = NULL;
	a->f_ar = NULL;
	a->b_ar = NULL;
	a->b1_ar = NULL;
	a->fout_ar = NULL;
	a->e_ar = NULL;
	a->tmp_ar = NULL;

	// float массивы длиной N, только I или  Q
	a->sig_ar = (ne10_float32_t*) NE10_MALLOC (N * sizeof (ne10_float32_t));
	a->ref_ar = (ne10_float32_t*) NE10_MALLOC (N * sizeof (ne10_float32_t));
	a->y_ar = (ne10_float32_t*) NE10_MALLOC (N * sizeof (ne10_float32_t));

	a->rxx_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->Rxx_mat = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * SLICESIZE * sizeof (ne10_float32_t));
	a->Pdx_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->a_mat = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * SLICESIZE * sizeof (ne10_float32_t));
	a->s_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->p_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->h_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->f_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->b_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->b1_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE * sizeof (ne10_float32_t));
	a->fout_ar = (ne10_float32_t*) NE10_MALLOC (N * sizeof (ne10_float32_t));
	a->e_ar = (ne10_float32_t*) NE10_MALLOC (N * sizeof (ne10_float32_t));
	// память для хранения состояний iir фильтра и рабочий буфер для изменения порядка коэффициентов
	a->tmp_ar = (ne10_float32_t*) NE10_MALLOC (SLICESIZE* sizeof (ne10_float32_t));


	if(a->sig_ar == NULL || 
			a->ref_ar == NULL || 
			a->y_ar == NULL ||
			a->rxx_ar == NULL || 
			a->Rxx_mat == NULL || 
			a->Pdx_ar == NULL || 
			a->a_mat == NULL || 
			a->s_ar == NULL || 
			a->p_ar == NULL || 
			a->h_ar == NULL || 
			a->f_ar == NULL || 
			a->b_ar == NULL || 
			a->b1_ar == NULL || 
			a->tmp_ar == NULL || 
			a->fout_ar == NULL)
	{
		fprintf(stderr, "Error NE10_MALLOC\n");
		return -2;
	}

	return 0;
}

void neCloseThread(int nt)
{
	afilter_t* a;
	if(nt == 0)
		a = &ai;
	else
		a = &aq;

	NE10_FREE(a->sig_ar);
	NE10_FREE(a->ref_ar);
	NE10_FREE(a->y_ar);
	NE10_FREE(a->rxx_ar);
	NE10_FREE(a->Rxx_mat);
	NE10_FREE(a->Pdx_ar);
	NE10_FREE(a->a_mat);
	NE10_FREE(a->s_ar);
	NE10_FREE(a->p_ar);
	NE10_FREE(a->h_ar);
	NE10_FREE(a->f_ar);
	NE10_FREE(a->b_ar);
	NE10_FREE(a->b1_ar);
	NE10_FREE(a->fout_ar);
	NE10_FREE(a->e_ar);
	NE10_FREE(a->tmp_ar);
}
