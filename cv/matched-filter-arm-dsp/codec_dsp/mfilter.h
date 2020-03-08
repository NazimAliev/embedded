/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF_MATH
#define LW_AF_MATH

#include <xdc/std.h>

void mfilter_init(int16_t* bufTwiddle,
                    int16_t* inBufImgRef,
                    int16_t* bufTmp,
                    int16_t* bufImgTmp,
                    int16_t* bufImgRef);
void mfilter_convolve(const int16_t* bufTwiddle,
						int16_t* inBufImgSig,
						int16_t* bufImgRef,
						int16_t* bufTmp,
						int16_t* bufImgTmp1,
						int16_t* bufImgTmp2,
						int16_t* outBufImg);

void logComplexArrayN(char* title, int16_t* buf, int n);
void logComplexArrayNxN(char* title, int16_t* buf);

#endif
