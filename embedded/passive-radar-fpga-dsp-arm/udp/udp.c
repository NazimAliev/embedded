/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "udp.h"

#define LOG

// длина заголовка в байтах за вычетом длины магического числа
#define HEADER_LEN (4)
// магическое слово для синхронизации после сбоя
// состоит из двух байт
#define MAGIC_BYTE_1 (uint8_t)(135)
#define MAGIC_BYTE_2 (uint8_t)(24)
#define MTU (1024)

/*

Формат посылки:

Заголовок: магическое слово (два байта), контрольная сумма (два байта), идентификатор (два байта)
Данные: массив длиной len

Прием датаграммы:

Побайтный поиск магического слова для вхождения в синхронизацию в len + 6 раз
Если магическое слово найдено сразу, прием оставшихся четырех байт и затем данных
Если магическое слово найдено не сразу, то предупреждение о срыве синхронизации
Если магическое слово не найдено, то сообщение об ошибке 

Количество принятых данных должно соответствовать len
Функция должна вернуть слово идентификатора для распознавания типа данных

*/

// XXX ВАЖНО: количество байт в буфере на отправке и приеме должно быть одно и то же, иначе потеря данных
// sendto(buf, 2) -> recvfrom(buf, 1), recvfrom(buf, 1) работать не будет: второй байт потеряется на
// первом же recvfrom

void sorry(char *s);
uint16_t Fletcher16(const uint8_t* data, int count);

static int isInitSend;
// send параметры
int send_fd;
struct sockaddr_in send_addr;
int send_len;


int udpInitRecv(int port, rdata_t* rdata)
{   
    // init recv 
    rdata->recv_len = sizeof(rdata->recv_addr);
    rdata->recv_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(rdata->recv_fd == -1)
    {
        sorry("recv socket");
		return -1;
    }

    bzero(&rdata->recv_addr, rdata->recv_len);
    rdata->recv_addr.sin_family = AF_INET;
    rdata->recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rdata->recv_addr.sin_port = htons(port);

    int n;
    n = bind(rdata->recv_fd,(struct sockaddr *)&rdata->recv_addr, rdata->recv_len);
    if(n == -1)
    {
        sorry("recv bind");
		return -1;
    }

    rdata->isInitRecv = 1;
    fprintf(stderr, "udpInitRecv: recv address 0.0.0.0:%d\n", port);
    return 0;
}

int udpInitSend(const char* server_addr, int port)
{   
    // init send 
    send_len = sizeof(send_addr);
    send_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if(send_fd == -1)
    {
        sorry("send socket");
		return -1;
    }

    bzero(&send_addr, send_len);
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(server_addr);
    send_addr.sin_port = htons(port);

    isInitSend = 1;
    fprintf(stderr, "udpInitSend: send address %s:%d\n", server_addr, port);
    return 0;
}

int udpRecv(rdata_t* rdata, uint8_t *pInMsg, int len, uint16_t* msg_id)
{
    int n;
    uint8_t mag1, mag2;
    int sync_lost_flag = 0;
    uint16_t check_sum;
    
    if(rdata->isInitRecv != 1)
    {
        fprintf(stderr, "udpRecv error: no init lib\n");
		return -1;
    }

    // поиск магического слова
    int i = 0;
    while(1)
    {
        rdata->recv_client_len = sizeof(rdata->client_addr);
        n = recvfrom(rdata->recv_fd, &mag1, 1, 0, (struct sockaddr *)&rdata->recv_addr, &rdata->recv_client_len);

    	if(n == -1)
		{
            sorry("udpRecv: MAGIC_BYTE_1");
	    	return -1;
		}

		if(mag1 == MAGIC_BYTE_1)
		{
    	    rdata->recv_client_len = sizeof(rdata->client_addr);
            n = recvfrom(rdata->recv_fd, &mag2, 1, 0, (struct sockaddr *)&rdata->recv_addr, &rdata->recv_client_len);

    		if(n == -1)
	    	{
        		sorry("udpRecv: MAGIC_BYTE_2");
				return -1;
	    	}

	    	if(mag2 == MAGIC_BYTE_2)
	        	break;
		}
		if(sync_lost_flag == 0)
		{
	    	// выводим сообщение только в первый раз чтобы не засорять экран
	    	fprintf(stderr, "udpRecv warning: syncronization lost, try seek magic word m1: %u, m2: %u\n",\
				mag1, mag2);
	    	sync_lost_flag = 1;
			//FIXME
			break;
		}
		i++;
		if(i > 20000) 
		{
	    	// таймаут: не нашли магическое слово 
	    	fprintf(stderr, "udpRecv error: timeout find magic word\n");
	    	return -1;
		}
    } // while

    // магическое слово найдено, принимаем остальную часть заголовка
    rdata->recv_client_len = sizeof(rdata->client_addr);
    n = recvfrom(rdata->recv_fd, pInMsg, HEADER_LEN, 0, (struct sockaddr *)&rdata->recv_addr, &rdata->recv_client_len);

    if(n == -1 || n != HEADER_LEN)
    {
        sorry("udpRecv: HEADER_LEN");
		return -1;
    }
    
    check_sum = *(uint16_t*)(pInMsg + 0);
    *msg_id = *(uint16_t*)(pInMsg + HEADER_LEN / 2);

    // принимаем данные
    int received_bytes = 0;
    int remaining_bytes = len;
    i = 0;
    while(1)
    {
        rdata->recv_client_len = sizeof(rdata->client_addr);
        n = recvfrom(rdata->recv_fd, pInMsg + received_bytes, remaining_bytes, 0, (struct sockaddr *)&rdata->recv_addr, &rdata->recv_client_len);

	    //fprintf(stderr, "\tudpRecv %d / %d\n", n, remaining_bytes);
		//usleep(500);
        if(n == -1)
		{
            sorry("udpRecv: error recv data");
	    	return -1;
		}

		received_bytes += n;
		remaining_bytes -= n;

		if(remaining_bytes == 0)
	    	break; 

		i++;
		if(i > 1000000) 
		{
	    	// таймаут: что-то пошло не так 
	    	fprintf(stderr, "udpRecv error: recvfrom timeout\n");
	    	return -1;
		}

    } // while

    // проверка контрольной суммы
    uint16_t sum;
    sum = Fletcher16(pInMsg, len);
    if(sum != check_sum) 
    {
        fprintf(stderr, "udpRecv checksum error: in header=%u, calculated=%u\n", check_sum, sum);
        return -1;
    }

    return received_bytes;
}

int udpSend(const uint8_t *pOutMsg, int len, uint16_t msg_id)
{
    int n;
    uint8_t mag;
    uint8_t head[HEADER_LEN];

    if(isInitSend != 1)
    {
        fprintf(stderr, "udpSend error: no init lib\n");
		return -1;
    }

    // передача магического слова
    mag = MAGIC_BYTE_1;
    n = sendto(send_fd, &mag, 1, 0, (struct sockaddr *)&send_addr, send_len);
    mag = MAGIC_BYTE_2;
    n = sendto(send_fd, &mag, 1, 0, (struct sockaddr *)&send_addr, send_len);

    if(n == -1)
    {
        sorry("udpSend: error send magic word");
        return -1;
    }

    // подсчет контрольной суммы
    uint16_t sum = Fletcher16(pOutMsg, len);

    // формирование заголовка
    // head[0], head[1]: контрольная сумма
    memcpy(head, (uint8_t*)&sum, HEADER_LEN / 2);
    // head[2], head[3]: msg_id
    memcpy(head + HEADER_LEN / 2, (uint8_t*)&msg_id, HEADER_LEN / 2);

    // передача заголовка
    n = sendto(send_fd, &head, HEADER_LEN, 0, (struct sockaddr *)&send_addr, send_len);
    if(n == -1)
    {
        sorry("udpSend: error send header");
        return -1;
    }

    // передаем данные
    int i = 0;
    int sent_bytes = 0;
    int remaining_bytes = len;
    int bs;
    
    i = 0;
    while(1)
    {
		if(remaining_bytes >= MTU)
	    	bs = MTU;
		else
	    	bs = remaining_bytes;

    	n = sendto(send_fd, pOutMsg + sent_bytes, bs, 0, (struct sockaddr *)&send_addr, send_len);
		usleep(1000);

    	if(n == -1)
		{
        	sorry("udpSend: error send data");
	    	return -1;
		}

		sent_bytes += n;
		remaining_bytes -= n;

		if(remaining_bytes == 0)
	    	break; 

		i++;
		if(i > 1000000) 
		{
	    	// таймаут: что-то пошло не так 
	    	fprintf(stderr, "udpSend error: sendto timeout\n");
	    	return -1;
		}
    } // while
    return sent_bytes;
}

uint16_t Fletcher16(const uint8_t* data, int count)
{
   uint16_t sum1 = 12345;
   uint16_t sum2 = 54321;
   int index;
 
   for( index = 0; index < count; ++index )
   {
      sum1 = (sum1 + data[index]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }
 
   return (sum2 << 8) | sum1;
}

void udpCloseRecv(rdata_t* rdata)
{
    if(rdata->isInitRecv != 1)
    {
        fprintf(stderr, "udpClose error: no recv init lib\n");
		return;
    }
    close(rdata->recv_fd);
}

void udpCloseSend()
{
    if(isInitSend != 1)
    {
        fprintf(stderr, "udpClose error: no send init lib\n");
		return;
    }
    close(send_fd);
    isInitSend = 0;
}

void sorry(char *s)
{
    perror(s);
    return;
}
