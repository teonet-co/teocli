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
	${OBJECTDIR}/main_cpp.o \
	${OBJECTDIR}/main_select_cpp.o \
	${OBJECTDIR}/main_select_trudp.o \
	${OBJECTDIR}/nbind_ser/libteonet-js.o \
	${OBJECTDIR}/nbind_ser/node_modules/nbind/src/common.o \
	${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em/Binding.o \
	${OBJECTDIR}/nbind_ser/node_modules/nbind/src/reflect.o \
	${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Binding.o \
	${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Buffer.o \
	${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.o \
	${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.o


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

#${OBJECTDIR}/main_select_trudp.o: main_select_trudp.c
#	${MKDIR} -p ${OBJECTDIR}
#	${RM} "$@.d"
#	$(COMPILE.c) -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_select_trudp.o main_select_trudp.c

${OBJECTDIR}/nbind_ser/libteonet-js.o: nbind_ser/libteonet-js.cpp
	${MKDIR} -p ${OBJECTDIR}/nbind_ser
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/libteonet-js.o nbind_ser/libteonet-js.cpp

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/common.o: nbind_ser/node_modules/nbind/src/common.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/common.o nbind_ser/node_modules/nbind/src/common.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em/Binding.o: nbind_ser/node_modules/nbind/src/em/Binding.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em/Binding.o nbind_ser/node_modules/nbind/src/em/Binding.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/reflect.o: nbind_ser/node_modules/nbind/src/reflect.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/reflect.o nbind_ser/node_modules/nbind/src/reflect.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Binding.o: nbind_ser/node_modules/nbind/src/v8/Binding.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Binding.o nbind_ser/node_modules/nbind/src/v8/Binding.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Buffer.o: nbind_ser/node_modules/nbind/src/v8/Buffer.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Buffer.o nbind_ser/node_modules/nbind/src/v8/Buffer.cc

${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.o: nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.o nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.cc

${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.o: nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.o nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.cc

${OBJECTDIR}/nbind_ser/libteonet-js.o: nbind_ser/libteonet-js.cpp
	${MKDIR} -p ${OBJECTDIR}/nbind_ser
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/libteonet-js.o nbind_ser/libteonet-js.cpp

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/common.o: nbind_ser/node_modules/nbind/src/common.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/common.o nbind_ser/node_modules/nbind/src/common.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em/Binding.o: nbind_ser/node_modules/nbind/src/em/Binding.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/em/Binding.o nbind_ser/node_modules/nbind/src/em/Binding.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/reflect.o: nbind_ser/node_modules/nbind/src/reflect.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/reflect.o nbind_ser/node_modules/nbind/src/reflect.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Binding.o: nbind_ser/node_modules/nbind/src/v8/Binding.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Binding.o nbind_ser/node_modules/nbind/src/v8/Binding.cc

${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Buffer.o: nbind_ser/node_modules/nbind/src/v8/Buffer.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/nbind/src/v8/Buffer.o nbind_ser/node_modules/nbind/src/v8/Buffer.cc

${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.o: nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.o nbind_ser/node_modules/node-gyp/gyp/data/win/large-pdb-shim.cc

${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.o: nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.cc
	${MKDIR} -p ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHAVE_MINGW -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.o nbind_ser/node_modules/node-gyp/src/win_delay_load_hook.cc

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
