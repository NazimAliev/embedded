#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

CC = g++
CFLAGS = -DDEBUG -g -Wall
SRCS = video.cpp
PROG = video 

OPENCV = `pkg-config opencv --cflags --libs`
#OPENCV = -I/usr/local/include/opencv -I/usr/local/include  -L/usr/local/lib -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab -lrt -lpthread -lm -ldl

LIBS = $(OPENCV)

$(PROG):$(SRCS)
	@$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
