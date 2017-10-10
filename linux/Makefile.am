# Linux make file

AM_CXXFLAGS = -std=c++11

lib_LTLIBRARIES = libteocli.la
teocliincludedir=$(includedir)/teocli
nobase_teocliinclude_HEADERS = ../libteol0/teonet_l0_client.h
libteocli_la_SOURCES = ../libteol0/teonet_l0_client.c
libteocli_la_LDFLAGS = $(AM_LDFLAGS) -version-info 2:0:0 

noinst_PROGRAMS =
		
noinst_PROGRAMS += teocli	
teocli_SOURCES = ../main.c
teocli_LDFLAGS = $(AM_LDFLAGS) -lteocli

noinst_PROGRAMS += teocli_s	
teocli_s_SOURCES = ../main_select.c
teocli_s_LDFLAGS = $(AM_LDFLAGS) -lteocli	

noinst_PROGRAMS += teocli_cpp	
teocli_cpp_SOURCES = ../main_cpp.cpp
teocli_cpp_LDFLAGS = $(AM_LDFLAGS) -lteocli 

noinst_PROGRAMS += teocli_s_cpp	
teocli_s_cpp_SOURCES = ../main_select_cpp.cpp
teocli_s_cpp_LDFLAGS = $(AM_LDFLAGS) -lteocli 

uninstall-local:
	-rm -r $(teocliincludedir)
