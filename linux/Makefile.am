## Process this file with automake to produce Makefile.in
#

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = \
    -Wall \
    -std=gnu11 \
    -fPIC \
    -DUSE_LIBEV=1 \
    -I../libtrudp/src \
    -I../libtrudp/libs/teobase/include \
    -I../libtrudp/libs/teoccl/include \
    -I../ \
    -I../libtrudp/libs/teobase/src \
    -I../libtrudp/libs/teoccl/src \
    # end of AM_CFLAGS

AM_CXXFLAGS = \
    -Wall \
    -std=c++17 \
    -fPIC \
    -DUSE_LIBEV=1 \
    -I../libtrudp/src \
    -I../libtrudp/libs/teobase/include \
    -I../libtrudp/libs/teoccl/include \
    -I../ \
    -I../libtrudp/libs/teobase/src \
    -I../libtrudp/libs/teoccl/src \
    # end of AM_CXXFLAGS


lib_LTLIBRARIES = libteocli.la

teocli_includedir = \
    $(includedir)/libtrudp/src \
    $(includedir)/libtrudp/libs/teobase/src \
    $(includedir)/libtrudp/libs/teobase/include \
    $(includedir)/libtrudp/libs/teoccl/src \
    $(includedir)/libtrudp/libs/teoccl/include \
    # end of teocli_includedir

libteocli_la_SOURCES = \
    ../libteol0/teonet_l0_client.c \
    ../libteol0/teonet_l0_client_options.c \
    ../libteol0/teonet_l0_client_crypt.c \
    \
    ../libtinycrypt/tinycrypt.c \
    ../libtinycrypt/tiny-AES-c/aes.c \
    ../libtinycrypt/tiny-ECDH-c/ecdh.c \
    \
    ../libtrudp/src/packet.c \
    ../libtrudp/src/packet_queue.c \
    ../libtrudp/src/trudp.c \
    ../libtrudp/src/trudp_options.c \
    ../libtrudp/src/trudp_channel.c \
    ../libtrudp/src/trudp_receive_queue.c \
    ../libtrudp/src/trudp_send_queue.c \
    ../libtrudp/src/trudp_stat.c \
    ../libtrudp/src/trudp_utils.c \
    ../libtrudp/src/udp.c \
    ../libtrudp/src/write_queue.c \
    ../libtrudp/src/trudp_ev.c \
    \
    ../libtrudp/libs/teobase/src/teobase/logging.c \
    ../libtrudp/libs/teobase/src/teobase/mutex.c \
    ../libtrudp/libs/teobase/src/teobase/socket.c \
    ../libtrudp/libs/teobase/src/teobase/time.c \
    \
    ../libtrudp/libs//teoccl/src/teoccl/array_list.c \
    ../libtrudp/libs//teoccl/src/teoccl/hash.c \
    ../libtrudp/libs//teoccl/src/teoccl/map.c \
    ../libtrudp/libs//teoccl/src/teoccl/memory.c \
    ../libtrudp/libs//teoccl/src/teoccl/queue.c \
    # ../libtrudp/libs//teoccl/src/teoccl/deque.c \
    # ../libtrudp/libs//teoccl/src/teoccl/linked_list.c \
    # ../libtrudp/libs//teoccl/src/teoccl/list.c \
    # ../libtrudp/libs//teoccl/src/teoccl/que.c \
    # ../libtrudp/libs//teoccl/src/teoccl/stack.c \
    # end of libteocli_la_SOURCES
#  ^^^^ Some containers are not production ready yet ^^^

#include_HEADERS = \
#    ../libteol0/teonet_l0_client.h

libteocli_la_LDFLAGS = $(AM_LDFLAGS) -lpthread -version-info $(LIBRARY_CURRENT):$(LIBRARY_REVISION):$(LIBRARY_AGE)


noinst_PROGRAMS =

noinst_PROGRAMS += teocli
teocli_SOURCES = ../main.c
teocli_LDADD = libteocli.la -lev

noinst_PROGRAMS += teocli_s
teocli_s_SOURCES = ../main_select.c
teocli_s_LDADD = libteocli.la -lev

noinst_PROGRAMS += teocli_cpp
teocli_cpp_SOURCES = ../main_cpp.cpp
teocli_cpp_LDADD = libteocli.la -lev

noinst_PROGRAMS += teocli_s_cpp
teocli_s_cpp_SOURCES = ../main_select_cpp.cpp
teocli_s_cpp_LDADD = libteocli.la -lev

noinst_PROGRAMS += teocli_s_trudp
teocli_s_trudp_SOURCES = ../main_select_trudp.c
teocli_s_trudp_LDADD = libteocli.la -lev

noinst_PROGRAMS += teocli_s_common
teocli_s_common_SOURCES = ../main_select_common.c
teocli_s_common_LDADD = libteocli.la -lev

noinst_PROGRAMS += teocli_s_common_thread
teocli_s_common_thread_SOURCES = ../main_select_common_thread.c
teocli_s_common_thread_LDADD = libteocli.la -lpthread -lev
