#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_DEFUN([AC_DEFINE_SUBST], [
AC_DEFINE([$1], [$2], [$3])
AC_SUBST([$1], ['$2'])
])

AC_PREREQ([2.69])

AC_INIT([teocli], [0.2.0], [Kirill Scherba <kirill@scherba.ru>, Anton Vasiliev <si1ver1502@gmail.com>, Max Ponka <mpano91@gmail.com>])

AC_DEFINE_SUBST(LIBRARY_CURRENT,  2, [teocli dynamic library version])
AC_DEFINE_SUBST(LIBRARY_REVISION, 0, [teocli dynamic library version])
AC_DEFINE_SUBST(LIBRARY_AGE,      0, [teocli dynamic library version])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB


AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lteocli':
AC_CHECK_LIB([teocli], [main])


# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CACHE_SAVE

LT_INIT

AC_CONFIG_FILES([Makefile])

# Call trudp ./configure script recursively.
AC_CONFIG_SUBDIRS([../libtrudp])

AC_OUTPUT
