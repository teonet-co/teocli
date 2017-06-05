#!/bin/sh

docker run --rm -v $(pwd):/src trzeci/emscripten:sdk-tag-1.37.3-64bit emcc test_sockets_echo_server.c -o test_sockets_echo_server.js -DSOCKK=9099

docker run --rm -v $(pwd):/src trzeci/emscripten:sdk-tag-1.37.3-64bit emcc test_sockets_echo_client.c -o test_sockets_echo_client.js -DSOCKK=9099


