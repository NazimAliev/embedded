/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_MULTIX86_APP
#define LW_MULTIX86_APP

#include "../udp/udp.h"

extern int gFrames;
extern int gFifo;

void processLoop();

// данные о входящем соединении для каждого из двух потоков - thread safe
extern rdata_t* rdata1;
extern rdata_t* rdata2;

// входные буферы CFAR и AF
extern uint8_t* ic1;
extern uint8_t* ic2;
extern uint16_t* ia1;
extern uint16_t* ia2;
#endif
