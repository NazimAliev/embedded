/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

// the below define is a hack
#define u32 unsigned int
#define dma_cookie_t unsigned int

#include "xdma.h"
//#include "../af.h" 
#include "zboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FILEPATH "/dev/xdma"

int fd;

struct xdma_dev dev;
struct xdma_chan_cfg rx_config;
struct xdma_buf_info rx_buf;
struct xdma_transfer rx_trans;

struct xdma_chan_cfg tx_config;
struct xdma_buf_info tx_buf;
struct xdma_transfer tx_trans;

int xdmaInit()
{  
	// файл в dev существует? в противном случае запишется обычный файл xdma

	if(access(FILEPATH, F_OK) == -1 )
	{
		perror("xmdaInit: xdma driver not loaded, run insmod xdma.ko at first");
		return -1;
	}

	/* Open a file for writing.
	 * Note: "O_WRONLY" mode is not sufficient when mmaping.
	 */

	fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
	if(fd == -1) 
	{
		perror("xmdaInit: error opening file for writing");
		return -1;
	}

	/* 
		mmap the file to get access to the memory area.
		2* чтобы выделить вторую половину для tx которая не используется
	 */
	gMap = mmap(0, 2*MAPBUFSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(gMap == MAP_FAILED) 
	{
		close(fd);
		perror("xdmaInit: error mmapping the file");
		return -1;
	}

	/* Query driver for number of devices.
	 */
	int num_devices = 0;
	if(ioctl(fd, XDMA_GET_NUM_DEVICES, &num_devices) < 0) 
	{
		perror("xmdaInit: error ioctl getting device num");
		return -1;
	}

	/* Query driver for number of devices.
	 */
	dev.tx_chan = (u32) NULL;
	dev.tx_cmp = (u32) NULL;
	dev.rx_chan = (u32) NULL;
	dev.rx_cmp = (u32) NULL;
	dev.device_id = num_devices - 1;
	if(ioctl(fd, XDMA_GET_DEV_INFO, &dev) < 0) 
	{
		perror("xdmaInit: error ioctl getting device info");
		return -1;
	}

	rx_config.chan = dev.rx_chan;
	rx_config.dir = XDMA_DEV_TO_MEM;
	rx_config.coalesc = 1;
	rx_config.delay = 0;
	rx_config.reset = 0;
	if(ioctl(fd, XDMA_DEVICE_CONTROL, &rx_config) < 0) 
	{
		perror("xdmaInit: error ioctl config rx chan");
		return -1;
	}

#ifdef TX
	tx_config.chan = dev.tx_chan;
	tx_config.dir = XDMA_MEM_TO_DEV;
	tx_config.coalesc = 1;
	tx_config.delay = 0;
	tx_config.reset = 0;
	if (ioctl(fd, XDMA_DEVICE_CONTROL, &tx_config) < 0) 
	{
		perror("xdmaInit: error ioctl config tx chan");
		return -1;
	}
#endif

	/* Query driver to run test function.
	
	if(ioctl(fd, XDMA_TEST_TRANSFER, NULL) < 0) 
	{
		perror("Error ioctl test transition");
		return -1;
	}
	*/

	fprintf(stderr, "XDMA init completed\n");
	return 0;
}

int xdmaRead(/* map_t* gMap */)
{

	rx_buf.chan = dev.rx_chan;
	rx_buf.completion = dev.rx_cmp;
	rx_buf.cookie = (u32) NULL;
	rx_buf.buf_offset = (u32) NULL;
	rx_buf.buf_size = (u32) (MAPBUFSIZE);
	rx_buf.dir = XDMA_DEV_TO_MEM;

	if(ioctl(fd, XDMA_PREP_BUF, &rx_buf) < 0) 
	{
		perror("xdmaRead: error ioctl set rx buf");
		return -1;
	}

#ifdef TX
	tx_buf.chan = dev.tx_chan;
	tx_buf.completion = dev.tx_cmp;
	tx_buf.cookie = (u32) NULL;
	tx_buf.buf_offset = (u32) (MAPBUFSIZE);
	tx_buf.buf_size = (u32) (MAPBUFSIZE);
	tx_buf.dir = XDMA_MEM_TO_DEV;
	if (ioctl(fd, XDMA_PREP_BUF, &tx_buf) < 0) 
	{
		perror("xdmaRead: error ioctl set tx buf");
		return -1;
	}
#endif

	rx_trans.chan = dev.rx_chan;
	rx_trans.completion = dev.rx_cmp;
	rx_trans.cookie = rx_buf.cookie;
	rx_trans.wait = 0;

	if(ioctl(fd, XDMA_START_TRANSFER, &rx_trans) < 0) 
	{
		perror("xdmaRead: error ioctl start rx trans");
		return -1;
	}

#ifdef TX
	tx_trans.chan = dev.tx_chan;
	tx_trans.completion = dev.tx_cmp;
	tx_trans.cookie = tx_buf.cookie;
	tx_trans.wait = 0;
	if (ioctl(fd, XDMA_START_TRANSFER, &tx_trans) < 0) 
	{
		perror("xdmaRead: error ioctl start tx trans");
		return -1;
	}
#endif

	//usleep(50000);

	if(ioctl(fd, XDMA_STOP_TRANSFER, &rx_trans) < 0) 
	{
		perror("xdmaRead: error ioctl start rx trans");
		return -1;
	}


	fprintf(stderr, "XDMA read completed\n");
	return 0;
}

int xdmaClose()
{
	/* Don't forget to free the mmapped memory
	 */
	if(munmap(gMap, 2*MAPBUFSIZE) == -1) 
	{
		perror("xdmaClose: error un-mmapping the file");
		/* Decide here whether to close(fd) and exit() or not. Depends... */
	}

	/* Un-mmaping doesn't close the file, so we still need to do that.
	 */
	close(fd);
	return 0;
}

