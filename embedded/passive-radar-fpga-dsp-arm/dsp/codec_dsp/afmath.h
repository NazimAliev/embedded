/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF_MATH
#define LW_AF_MATH

#include <xdc/std.h>

extern int gFrames;
extern int gFlag;

void afmath_init(short* twiddle);
void afmath_fft(short* sigBuf, UChar* tmpBuf, UChar* tmp2Buf, short* twiddle);
void afmath_af(const Short* sigBuf, Short* refBuf, UChar* tmpBuf, UChar* tmp2Buf, UInt* matrixBuf, UShort* afBuf, const Short* twiddle, UShort logMode);
void afmath_norm(const UInt* matrixBuf, UShort* afBuf);
void afmath_cfar(const UInt* matrixBuf, UChar* cfarBuf);
UShort afmath_cfar2(UInt* matrixBuf, UChar* cfarBuf, UInt cfarLevel);

#endif
