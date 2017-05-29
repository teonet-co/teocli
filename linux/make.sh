#!/bin/sh
# 
# File:   make.sh
# Author: Kirill Scherba <kirill@scherba.ru>
#
# Created on May 29, 2017, 4:37:50 PM
#

gcc -o ./libteocli.so ../libteol0/teonet_l0_client.c -shared -fPIC
gcc -o ./teocli ../main.c ./libteocli.so
gcc -o ./teocli_s ../main_select.c ./libteocli.so
#sudo ldconfig
