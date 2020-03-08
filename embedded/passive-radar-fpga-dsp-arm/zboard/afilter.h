/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF_AFILTER
#define LW_AF_AFILTER

#include <stdint.h>

int neInit();
int neInitThread(int nt);
// input/output is I/Q buffers
// sig: SIGBUFSIZE
// ref: SIGBUFSIZE
// y: SIGBUFSIZE / 2
void neFilter(const int32_t* sig, const int32_t* ref, short* y, int nt);
void neCloseThread(int nt);

#endif
