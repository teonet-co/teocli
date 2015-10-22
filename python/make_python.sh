#!/bin/sh

rm teonet_l0_client.py teonet_l0_client_wrap.c

swig -python teonet_l0_client.i

gcc -c ../libteol0/teonet_l0_client.c teonet_l0_client_wrap.c \
    -I/usr/include/python2.7 -fPIC

# Linux
ld -shared teonet_l0_client.o teonet_l0_client_wrap.o -o _teocli.so

# MacOS
# gcc -dynamiclib teonet_l0_client.o teonet_l0_client_wrap.o -o _teonet_l0_client.dylib

cp _teocli.so ../../../examples
