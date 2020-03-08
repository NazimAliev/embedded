/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_APP
#define LW_APP

#include <xdc/std.h>
#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <ti/sdo/ce/universal/universal.h>
#include "../codec_dsp/codec_dsp.h"

//extern UNIVERSAL_InArgs            universalInArgs;
//extern UNIVERSAL_OutArgs           universalOutArgs;
extern LW_AF_InArgs				   lwafInArgs;
extern LW_AF_OutArgs			   lwafOutArgs;
extern UNIVERSAL_DynamicParams     universalDynParams;
extern UNIVERSAL_Status            universalStatus;
extern XDM1_BufDesc                universalInBufDesc;
extern XDM1_BufDesc                universalOutBufDesc;

void time_init();
void time_start();
void time_delta();
int readImgFile(int16_t *inBufImg, char* fileName);
int writeImgFile(int16_t *inBufImg, char* fileName);
void genTestData(XDAS_Int8 *inBufSig, XDAS_Int8 *inBufRef);
int findMax(const int16_t* buf);
void logInt(int16_t* buf);

#endif
