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
#include "../../udp/udp.h"

extern UNIVERSAL_Handle            hUniversal;
//extern UNIVERSAL_InArgs            universalInArgs;
//extern UNIVERSAL_OutArgs           universalOutArgs;
extern LW_AF_InArgs				   lwafInArgs;
extern LW_AF_OutArgs			   lwafOutArgs;
extern UNIVERSAL_DynamicParams     universalDynParams;
extern UNIVERSAL_Status            universalStatus;
extern XDM1_BufDesc                universalInBufDesc;
extern XDM1_BufDesc                universalOutBufDesc;

extern int gFrames;
extern int gFlag;

extern int sockfd;
extern struct sockaddr_in servaddr, cliaddr;
extern char* ip_log;
extern int port_log;

void processLoop();
int tcpInit(char* ip, int port);

extern rdata_t* rdata;

#endif
