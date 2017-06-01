#/bin/sh

docker run -v $(pwd):/src trzeci/emscripten:sdk-tag-1.37.3-64bit emcc libteol0/teonet_l0_client.c -c
docker run -v $(pwd):/src trzeci/emscripten:sdk-tag-1.37.3-64bit emcc main_select.cpp -std=c++11 teonet_l0_client.o -o main_select.js

# node main_select.js teocli++js gt1.kekalan.net 9010 ps-server
# dist/Debug/GNU-Linux/teocli teocli++js gt1.kekalan.net 9010 ps-server
