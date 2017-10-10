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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/libteol0/teonet_l0_client.o \
	${OBJECTDIR}/main_cpp.o
	
	
#	${OBJECTDIR}/main_select_cpp.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/teocli ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/libteol0/teonet_l0_client.o: libteol0/teonet_l0_client.c
	${MKDIR} -p ${OBJECTDIR}/libteol0
	${RM} "$@.d"
	$(COMPILE.c) -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libteol0/teonet_l0_client.o libteol0/teonet_l0_client.c

${OBJECTDIR}/main_cpp.o: main_cpp.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_cpp.o main_cpp.cpp

${OBJECTDIR}/main_select_cpp.o: main_select_cpp.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select_cpp.o main_select_cpp.cpp

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
