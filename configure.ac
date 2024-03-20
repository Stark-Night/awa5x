AC_PREREQ([2.71])

AC_INIT([awa5x], [1.0.0], [starknightawawa@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/awa5.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AM_INIT_AUTOMAKE([foreign tar-pax])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_RANLIB

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [enable facilities to improve debugging])])

AC_ARG_ENABLE([awa-tracing],
  [AS_HELP_STRING([--enable-awa-tracing], [trace opcodes execution])])

AS_IF([test "x$enable_debug" = "xyes"],
  [AC_DEFINE([DEBUG], [1], [Define to 1 if debug is enabled])
   AX_VAR_PATDEL([CFLAGS], [\(-g\|-O2\)])
   CFLAGS="$CFLAGS -g3"
   AX_CHECK_COMPILER_FLAG([-fsanitize=address], [CFLAGS="$CFLAGS -fsanitize=address"])
   AX_CHECK_COMPILER_FLAG([-fsanitize=undefined], [CFLAGS="$CFLAGS -fsanitize=undefined"])])

AS_IF([test "x$enable_awa_tracing" = "xyes"],
  [AC_DEFINE([OPCODE_TRACING], [1], [Define to 1 if opcode tracing is enabled])])

AC_OUTPUT
