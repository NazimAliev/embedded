#!/bin/bash


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


FRAMES=4
# вызов сервера - графической программы рендеринга
./main_multi.py $FRAMES &

# вызов клиента - интерфейс UDP -> FIFO
bin/debug/multi.x86 $FRAMES 

# IMPORTANT
# вызов сервера должен быть ДО вызова клиента, иначе
# функция open(FIFO...) клиента заблокируется
