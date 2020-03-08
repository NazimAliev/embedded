/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_UDP
#define LW_UDP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>

typedef struct
{
    int recv_fd;
    struct sockaddr_in recv_addr, client_addr;
    int recv_len;
    socklen_t recv_client_len;
    int isInitRecv;
} rdata_t;

int udpInitRecv(int port, rdata_t* rdata);
int udpInitSend(const char* server_addr, int port);
int udpRecv(rdata_t* rdata, uint8_t *pInMsg, int len, uint16_t* msg_id);
int udpSend(const uint8_t *pOutMsg, int len, uint16_t msg_id);
void udpCloseRecv(rdata_t* rdata);
void udpCloseSend();

#endif
