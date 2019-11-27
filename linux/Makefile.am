## Process this file with automake to produce Makefile.in
#

SUBDIRS = ../libtrudp

AM_CFLAGS = \
    -g -Wall \
    -std=gnu11 \
    -fPIC \
    -I../libtrudp/src \
    -I../libtrudp/libs/teobase/include \
    -I../libtrudp/libs/teoccl/include \
    -pthread  \
    -Wpointer-arith

AM_CXXFLAGS = \
    -g -Wall \
    -std=c++14 \
    -fPIC \
    -I../libtrudp/src \
    -I../libtrudp/libs/teobase/include \
    -I../libtrudp/libs/teoccl/include


lib_LTLIBRARIES = libteocli.la

libteocli_la_SOURCES = \
    ../libteol0/teonet_l0_client.c \
    ../libtinycrypt/tinycrypt.c \
    ../libtinycrypt/tiny-AES-c/aes.c \
    ../libtinycrypt/tiny-ECDH-c/ecdh.c

#include_HEADERS = \
#    ../libteol0/teonet_l0_client.h

libteocli_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBRARY_CURRENT):$(LIBRARY_REVISION):$(LIBRARY_AGE) 

noinst_PROGRAMS =

noinst_PROGRAMS += teocli
teocli_SOURCES = ../main.c
teocli_LDADD = libteocli.la

noinst_PROGRAMS += teocli_s
teocli_s_SOURCES = ../main_select.c
teocli_s_LDADD = libteocli.la

noinst_PROGRAMS += teocli_cpp
teocli_cpp_SOURCES = ../main_cpp.cpp
teocli_cpp_LDADD = libteocli.la

noinst_PROGRAMS += teocli_s_cpp
teocli_s_cpp_SOURCES = ../main_select_cpp.cpp
teocli_s_cpp_LDADD = libteocli.la

LIBS = ../libtrudp/src/.libs/libtrudp.a

noinst_PROGRAMS += teocli_s_trudp
teocli_s_trudp_SOURCES = ../main_select_trudp.c
teocli_s_trudp_LDADD = libteocli.la

noinst_PROGRAMS += teocli_s_common
teocli_s_common_SOURCES = ../main_select_common.c
teocli_s_common_LDADD = libteocli.la

noinst_PROGRAMS += teocli_s_common_thread
teocli_s_common_thread_SOURCES = ../main_select_common_thread.c
teocli_s_common_thread_LDADD = libteocli.la
