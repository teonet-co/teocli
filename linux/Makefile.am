# Linux make file

SUBDIRS = ../libtrudp

AM_CXXFLAGS = -std=c++11 -I../libtrudp/src/ -I../libtrudp/embedded/teoccl/src/
AM_CFLAGS = -I../libtrudp/src/ -I../libtrudp/embedded/teoccl/src/

lib_LTLIBRARIES = libteocli.la
libteocli_la_SOURCES = ../libteol0/teonet_l0_client.c  ../libteol0/teonet_socket.c ../libteol0/teonet_time.c
include_HEADERS = ../libteol0/teonet_l0_client.h ../libteol0/teonet_socket.h ../libteol0/teonet_time.h ../libteol0/teonet_platform.h ../libteol0/connection_interface.h
libteocli_la_LDFLAGS = $(AM_LDFLAGS) -version-info 7:0:0 

noinst_PROGRAMS =

noinst_PROGRAMS += teocli
teocli_SOURCES = ../main.c
teocli_LDADD = libteocli.la

noinst_PROGRAMS += teocli_s
teocli_s_SOURCES = ../main_select.c
teocli_s_LDADD = libteocli.la

#noinst_PROGRAMS += teocli_cpp  
#teocli_cpp_SOURCES = ../main_cpp.cpp
#teocli_cpp_LDADD = libteocli.la 

#noinst_PROGRAMS += teocli_s_cpp
#teocli_s_cpp_SOURCES = ../main_select_cpp.cpp
#teocli_s_cpp_LDADD = libteocli.la


LIBS = ../libtrudp/src/.libs/libtrudp.a ../libtrudp/embedded/teoccl/src/.libs/libteoccl.a

noinst_PROGRAMS += teocli_s_trudp
teocli_s_trudp_SOURCES = ../main_select_trudp.c
teocli_s_trudp_LDADD = libteocli.la
