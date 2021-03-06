#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/emscripten/test/echo_server/test_sockets_echo_client.o \
	${OBJECTDIR}/emscripten/test/echo_server/test_sockets_echo_server.o \
	${OBJECTDIR}/libteol0/teonet_l0_client.o \
	${OBJECTDIR}/libteol0/teonet_socket.o \
	${OBJECTDIR}/libteol0/teonet_time.o \
	${OBJECTDIR}/libtrudp/ci-build/make_package.o \
	${OBJECTDIR}/libtrudp/embedded/teoccl/ci-build/make_package.o \
	${OBJECTDIR}/libtrudp/embedded/teoccl/src/hash.o \
	${OBJECTDIR}/libtrudp/embedded/teoccl/src/list.o \
	${OBJECTDIR}/libtrudp/embedded/teoccl/src/map.o \
	${OBJECTDIR}/libtrudp/embedded/teoccl/src/queue.o \
	${OBJECTDIR}/libtrudp/examples/read_queue.o \
	${OBJECTDIR}/libtrudp/examples/snake.o \
	${OBJECTDIR}/libtrudp/examples/trudp2p.o \
	${OBJECTDIR}/libtrudp/examples/trudp_pth.o \
	${OBJECTDIR}/libtrudp/examples/trudpcat.o \
	${OBJECTDIR}/libtrudp/examples/trudpcat_ev.o \
	${OBJECTDIR}/libtrudp/src/packet.o \
	${OBJECTDIR}/libtrudp/src/packet_queue.o \
	${OBJECTDIR}/libtrudp/src/trudp.o \
	${OBJECTDIR}/libtrudp/src/trudp_channel.o \
	${OBJECTDIR}/libtrudp/src/trudp_ev.o \
	${OBJECTDIR}/libtrudp/src/trudp_receive_queue.o \
	${OBJECTDIR}/libtrudp/src/trudp_send_queue.o \
	${OBJECTDIR}/libtrudp/src/trudp_stat.o \
	${OBJECTDIR}/libtrudp/src/trudp_utils.o \
	${OBJECTDIR}/libtrudp/src/udp.o \
	${OBJECTDIR}/libtrudp/src/utils.o \
	${OBJECTDIR}/libtrudp/src/write_queue.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/main_cpp.o \
	${OBJECTDIR}/main_select.o \
	${OBJECTDIR}/main_select_common.o \
	${OBJECTDIR}/main_select_cpp.o \
	${OBJECTDIR}/main_select_trudp.o \
	${OBJECTDIR}/nbind/main_select_cpp.o \
	${OBJECTDIR}/nodejs/example/hello/hello.o \
	${OBJECTDIR}/nodejs/src/connector.o \
	${OBJECTDIR}/nodejs/src/errno_exeption.o \
	${OBJECTDIR}/nodejs/src/libteol0/teonet_l0_client.o \
	${OBJECTDIR}/nodejs/src/teo_exeption.o \
	${OBJECTDIR}/nodejs/src/teo_main.o \
	${OBJECTDIR}/nodejs/src/teo_packet.o \
	${OBJECTDIR}/python/teonet_l0_client.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lws2_32

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/emscripten/test/echo_server/test_sockets_echo_client.o: emscripten/test/echo_server/test_sockets_echo_client.c
	${MKDIR} -p ${OBJECTDIR}/emscripten/test/echo_server
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/emscripten/test/echo_server/test_sockets_echo_client.o emscripten/test/echo_server/test_sockets_echo_client.c

${OBJECTDIR}/emscripten/test/echo_server/test_sockets_echo_server.o: emscripten/test/echo_server/test_sockets_echo_server.c
	${MKDIR} -p ${OBJECTDIR}/emscripten/test/echo_server
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/emscripten/test/echo_server/test_sockets_echo_server.o emscripten/test/echo_server/test_sockets_echo_server.c

${OBJECTDIR}/libteol0/teonet_l0_client.o: libteol0/teonet_l0_client.c
	${MKDIR} -p ${OBJECTDIR}/libteol0
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libteol0/teonet_l0_client.o libteol0/teonet_l0_client.c

${OBJECTDIR}/libteol0/teonet_socket.o: libteol0/teonet_socket.c
	${MKDIR} -p ${OBJECTDIR}/libteol0
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libteol0/teonet_socket.o libteol0/teonet_socket.c

${OBJECTDIR}/libteol0/teonet_time.o: libteol0/teonet_time.c
	${MKDIR} -p ${OBJECTDIR}/libteol0
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libteol0/teonet_time.o libteol0/teonet_time.c

${OBJECTDIR}/libtrudp/ci-build/make_package.o: libtrudp/ci-build/make_package.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/ci-build
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/ci-build/make_package.o libtrudp/ci-build/make_package.c

${OBJECTDIR}/libtrudp/embedded/teoccl/ci-build/make_package.o: libtrudp/embedded/teoccl/ci-build/make_package.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/embedded/teoccl/ci-build
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/embedded/teoccl/ci-build/make_package.o libtrudp/embedded/teoccl/ci-build/make_package.c

${OBJECTDIR}/libtrudp/embedded/teoccl/src/hash.o: libtrudp/embedded/teoccl/src/hash.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/embedded/teoccl/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/embedded/teoccl/src/hash.o libtrudp/embedded/teoccl/src/hash.c

${OBJECTDIR}/libtrudp/embedded/teoccl/src/list.o: libtrudp/embedded/teoccl/src/list.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/embedded/teoccl/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/embedded/teoccl/src/list.o libtrudp/embedded/teoccl/src/list.c

${OBJECTDIR}/libtrudp/embedded/teoccl/src/map.o: libtrudp/embedded/teoccl/src/map.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/embedded/teoccl/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/embedded/teoccl/src/map.o libtrudp/embedded/teoccl/src/map.c

${OBJECTDIR}/libtrudp/embedded/teoccl/src/queue.o: libtrudp/embedded/teoccl/src/queue.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/embedded/teoccl/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/embedded/teoccl/src/queue.o libtrudp/embedded/teoccl/src/queue.c

${OBJECTDIR}/libtrudp/examples/read_queue.o: libtrudp/examples/read_queue.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/examples
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/examples/read_queue.o libtrudp/examples/read_queue.c

${OBJECTDIR}/libtrudp/examples/snake.o: libtrudp/examples/snake.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/examples
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/examples/snake.o libtrudp/examples/snake.c

${OBJECTDIR}/libtrudp/examples/trudp2p.o: libtrudp/examples/trudp2p.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/examples
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/examples/trudp2p.o libtrudp/examples/trudp2p.c

${OBJECTDIR}/libtrudp/examples/trudp_pth.o: libtrudp/examples/trudp_pth.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/examples
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/examples/trudp_pth.o libtrudp/examples/trudp_pth.c

${OBJECTDIR}/libtrudp/examples/trudpcat.o: libtrudp/examples/trudpcat.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/examples
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/examples/trudpcat.o libtrudp/examples/trudpcat.c

${OBJECTDIR}/libtrudp/examples/trudpcat_ev.o: libtrudp/examples/trudpcat_ev.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/examples
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/examples/trudpcat_ev.o libtrudp/examples/trudpcat_ev.c

${OBJECTDIR}/libtrudp/src/packet.o: libtrudp/src/packet.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/packet.o libtrudp/src/packet.c

${OBJECTDIR}/libtrudp/src/packet_queue.o: libtrudp/src/packet_queue.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/packet_queue.o libtrudp/src/packet_queue.c

${OBJECTDIR}/libtrudp/src/trudp.o: libtrudp/src/trudp.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp.o libtrudp/src/trudp.c

${OBJECTDIR}/libtrudp/src/trudp_channel.o: libtrudp/src/trudp_channel.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp_channel.o libtrudp/src/trudp_channel.c

${OBJECTDIR}/libtrudp/src/trudp_ev.o: libtrudp/src/trudp_ev.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp_ev.o libtrudp/src/trudp_ev.c

${OBJECTDIR}/libtrudp/src/trudp_receive_queue.o: libtrudp/src/trudp_receive_queue.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp_receive_queue.o libtrudp/src/trudp_receive_queue.c

${OBJECTDIR}/libtrudp/src/trudp_send_queue.o: libtrudp/src/trudp_send_queue.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp_send_queue.o libtrudp/src/trudp_send_queue.c

${OBJECTDIR}/libtrudp/src/trudp_stat.o: libtrudp/src/trudp_stat.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp_stat.o libtrudp/src/trudp_stat.c

${OBJECTDIR}/libtrudp/src/trudp_utils.o: libtrudp/src/trudp_utils.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/trudp_utils.o libtrudp/src/trudp_utils.c

${OBJECTDIR}/libtrudp/src/udp.o: libtrudp/src/udp.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/udp.o libtrudp/src/udp.c

${OBJECTDIR}/libtrudp/src/utils.o: libtrudp/src/utils.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/utils.o libtrudp/src/utils.c

${OBJECTDIR}/libtrudp/src/write_queue.o: libtrudp/src/write_queue.c
	${MKDIR} -p ${OBJECTDIR}/libtrudp/src
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libtrudp/src/write_queue.o libtrudp/src/write_queue.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/main_cpp.o: main_cpp.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_cpp.o main_cpp.cpp

${OBJECTDIR}/main_select.o: main_select.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select.o main_select.c

${OBJECTDIR}/main_select_common.o: main_select_common.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select_common.o main_select_common.c

${OBJECTDIR}/main_select_cpp.o: main_select_cpp.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select_cpp.o main_select_cpp.cpp

${OBJECTDIR}/main_select_trudp.o: main_select_trudp.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select_trudp.o main_select_trudp.c

${OBJECTDIR}/nbind/main_select_cpp.o: nbind/main_select_cpp.cpp
	${MKDIR} -p ${OBJECTDIR}/nbind
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind/main_select_cpp.o nbind/main_select_cpp.cpp

${OBJECTDIR}/nodejs/example/hello/hello.o: nodejs/example/hello/hello.cc
	${MKDIR} -p ${OBJECTDIR}/nodejs/example/hello
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/example/hello/hello.o nodejs/example/hello/hello.cc

${OBJECTDIR}/nodejs/src/connector.o: nodejs/src/connector.cpp
	${MKDIR} -p ${OBJECTDIR}/nodejs/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/src/connector.o nodejs/src/connector.cpp

${OBJECTDIR}/nodejs/src/errno_exeption.o: nodejs/src/errno_exeption.cpp
	${MKDIR} -p ${OBJECTDIR}/nodejs/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/src/errno_exeption.o nodejs/src/errno_exeption.cpp

${OBJECTDIR}/nodejs/src/libteol0/teonet_l0_client.o: nodejs/src/libteol0/teonet_l0_client.c
	${MKDIR} -p ${OBJECTDIR}/nodejs/src/libteol0
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/src/libteol0/teonet_l0_client.o nodejs/src/libteol0/teonet_l0_client.c

${OBJECTDIR}/nodejs/src/teo_exeption.o: nodejs/src/teo_exeption.cpp
	${MKDIR} -p ${OBJECTDIR}/nodejs/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/src/teo_exeption.o nodejs/src/teo_exeption.cpp

${OBJECTDIR}/nodejs/src/teo_main.o: nodejs/src/teo_main.cpp
	${MKDIR} -p ${OBJECTDIR}/nodejs/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/src/teo_main.o nodejs/src/teo_main.cpp

${OBJECTDIR}/nodejs/src/teo_packet.o: nodejs/src/teo_packet.cpp
	${MKDIR} -p ${OBJECTDIR}/nodejs/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nodejs/src/teo_packet.o nodejs/src/teo_packet.cpp

${OBJECTDIR}/python/teonet_l0_client.o: python/teonet_l0_client.i
	${MKDIR} -p ${OBJECTDIR}/python
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/python/teonet_l0_client.o python/teonet_l0_client.i

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
