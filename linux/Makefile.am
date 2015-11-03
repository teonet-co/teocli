# Linux make file

lib_LTLIBRARIES = libteocli.la
teocliincludedir=$(includedir)/teocli
nobase_teocliinclude_HEADERS = ../libteol0/teonet_l0_client.h
libteocli_la_SOURCES = ../libteol0/teonet_l0_client.c
libteocli_la_LDFLAGS = $(AM_LDFLAGS) -version-info 1:0:0 
		
noinst_PROGRAMS = teocli	
teocli_SOURCES = ../main.c
teocli_LDFLAGS = $(AM_LDFLAGS) -lteocli	

noinst_PROGRAMS += teocli_s	
teocli_s_SOURCES = ../main_select.c
teocli_s_LDFLAGS = $(AM_LDFLAGS) -lteocli	

uninstall-local:
	-rm -r $(teocliincludedir)
