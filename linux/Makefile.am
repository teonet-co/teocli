# Linux make file

AM_CXXFLAGS = -std=c++11

lib_LTLIBRARIES = libteocli.la # libtrudp.la

teocliincludedir=$(includedir)/teocli

nobase_teocliinclude_HEADERS = ../libteol0/teonet_l0_client.h ../libteol0/teonet_socket.h ../libteol0/teonet_time.h ../libteol0/teonet_platform.h
libteocli_la_SOURCES = ../libteol0/teonet_l0_client.c  ../libteol0/teonet_socket.c ../libteol0/teonet_time.c
libteocli_la_LDFLAGS = $(AM_LDFLAGS) -version-info 7:0:0 
	
#libtrudp_la_SOURCES = \
#	../../trudp/src/hash.c \
#	../../trudp/src/map.c \
#	../../trudp/src/packet.c \
#	../../trudp/src/packet_queue.c \
#	../../trudp/src/queue.c \
#	../../trudp/src/trudp.c \
#	../../trudp/src/trudp_receive_queue.c \
#	../../trudp/src/trudp_send_queue.c \
#	../../trudp/src/trudp_channel.c \
#	../../trudp/src/trudp_utils.c \
#	../../trudp/src/trudp_stat.c \
#	../../trudp/src/trudp_ev.c \
#	../../trudp/src/udp.c \
#	../../trudp/src/utils.c \
#	../../trudp/src/write_queue.c	
#
#       ../../trudp/src/*.c	
#libtrudp_la_LDFLAGS = $(AM_LDFLAGS) -version-info 2:0:0 
	

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

AM_CFLAGS = -I../libtrudp/src/ -I../libtrudp/embedded/teoccl/src/
LIBS = ../libtrudp/src/.libs/libtrudp.a ../libtrudp/embedded/teoccl/src/.libs/libteoccl.a

noinst_PROGRAMS += teocli_s_trudp
teocli_s_trudp_SOURCES = ../main_select_trudp.c
teocli_s_trudp_LDFLAGS = $(AM_LDFLAGS) -lteocli

uninstall-local:
	-rm -r $(teocliincludedir)
