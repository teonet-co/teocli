# Linux make file

lib_LTLIBRARIES = libteol0.la

teol0includedir=$(includedir)/teol0

nobase_teol0include_HEADERS = ../libteol0/teonet_l0_client.h

libteol0_la_SOURCES = ../libteol0/teonet_l0_client.c

libteol0_la_LDFLAGS = $(AM_LDFLAGS) -version-info 1:0:0 

uninstall-local:
	-rm -r $(teol0includedir)

#py_module: 
#    ./teonet_l0_client.make.sh
