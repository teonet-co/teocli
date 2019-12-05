#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_DEFUN([AC_DEFINE_SUBST], [
AC_DEFINE([$1], [$2], [$3])
AC_SUBST([$1], ['$2'])
])

AC_PREREQ([2.69])

AC_INIT([teocli], [0.3.0], [Kirill Scherba <kirill@scherba.ru>, Anton Vasiliev <si1ver1502@gmail.com>, Max Ponka <mpano91@gmail.com>])

AC_DEFINE_SUBST(LIBRARY_CURRENT,  3, [teocli dynamic library version])
AC_DEFINE_SUBST(LIBRARY_REVISION, 0, [teocli dynamic library version])
AC_DEFINE_SUBST(LIBRARY_AGE,      0, [teocli dynamic library version])


AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([../libteol0/teonet_l0_client_crypt.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])


AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES([yes])


AC_PROG_CC
AC_PROG_CXX
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])


LT_PREREQ([2.4])
LT_INIT

AC_CONFIG_FILES([Makefile])

# Call trudp ./configure script recursively.
#AC_CONFIG_SUBDIRS([../libtrudp])

AC_OUTPUT
