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
CND_PLATFORM=GNU-Linux-x86
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
	${OBJECTDIR}/python/teonet_l0_client.o \
	${OBJECTDIR}/python/teonet_l0_client_wrap.o


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

${OBJECTDIR}/python/teonet_l0_client.o: python/teonet_l0_client.i 
	${MKDIR} -p ${OBJECTDIR}/python
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/python/teonet_l0_client.o python/teonet_l0_client.i

${OBJECTDIR}/python/teonet_l0_client_wrap.o: python/teonet_l0_client_wrap.c 
	${MKDIR} -p ${OBJECTDIR}/python
	${RM} "$@.d"
	$(COMPILE.c) -DHAVE_MINGW -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/python/teonet_l0_client_wrap.o python/teonet_l0_client_wrap.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
