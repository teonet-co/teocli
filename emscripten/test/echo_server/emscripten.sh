#!/bin/sh

npm i

docker run --rm -v $(pwd):/src trzeci/emscripten:sdk-tag-1.37.36-64bit emcc test_sockets_echo_server.c -o test_sockets_echo_server.js -DSOCKK=9099 -s NO_EXIT_RUNTIME=0
docker run --rm -v $(pwd):/src trzeci/emscripten:sdk-tag-1.37.36-64bit emcc test_sockets_echo_client.c -o test_sockets_echo_client.js -DSOCKK=9099 -s NO_EXIT_RUNTIME=0

gcc -o test_sockets_echo_server test_sockets_echo_server.c -DSOCKK=9099
gcc -o test_sockets_echo_client test_sockets_echo_client.c -DSOCKK=9099

