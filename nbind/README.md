# Teonet native node C/C++ client with nbind

[nbind](https://github.com/charto/nbind) is a set of headers that make your C++11 library accessible from 
JavaScript. With a single #include statement, your C++ compiler generates the 
necessary bindings without any additional tools. Your library is then usable as 
a Node.js addon or, if compiled to asm.js with Emscripten, directly in web pages 
without any plugins.

## Source files

- main_select.cpp - Native C++ Teonet client based at libteol0/teocli and libteol0/teonet_l0_client.c
- index.js - JavaScript example

## Install sources from npm

    # Create new folder with name of application
    mkdir teoclinb
    cd teoclinb

    # Install from npm
    npm i teocli-l0
    npm i teocli-nbind
    rm -r node_modules/teocli-nbind/node_modules
    mv -f node_modules/teocli-nbind/* ./

    # Update projects package.json
    npm init

## Build this application

To build this application use:

    npm i

## Execute this example

    npm test teo-cli-nb 127.0.0.1 9010 teo-peers "Hello world!"
