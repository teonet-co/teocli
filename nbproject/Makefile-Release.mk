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
	${OBJECTDIR}/libteol0/teonet_l0_client.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/main_select.o \
	${OBJECTDIR}/main_select.o \
	${OBJECTDIR}/nbind/main_select.o \
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

${OBJECTDIR}/libteol0/teonet_l0_client.o: libteol0/teonet_l0_client.c
	${MKDIR} -p ${OBJECTDIR}/libteol0
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libteol0/teonet_l0_client.o libteol0/teonet_l0_client.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/main_select.o: main_select.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select.o main_select.c

${OBJECTDIR}/main_select.o: main_select.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select.o main_select.cpp

${OBJECTDIR}/nbind/main_select.o: nbind/main_select.cpp
	${MKDIR} -p ${OBJECTDIR}/nbind
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind/main_select.o nbind/main_select.cpp

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
