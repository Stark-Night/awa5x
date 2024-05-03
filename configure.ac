dnl awa5x - Extended AWA5.0
dnl Copyright © 2024 Starknights
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.
AC_PREREQ([2.71])

AC_INIT([awa5x], [2.0.1], [starknightawawa@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/awa5.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile doc/awa5x.html doc/book/Makefile vendors/Makefile])

AM_INIT_AUTOMAKE([foreign tar-pax])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_RANLIB
AC_CANONICAL_HOST

AS_CASE(["$host_os"],
  [*mingw*], [have_wapi=yes],
  [have_wapi=no])
AM_CONDITIONAL([HAVE_WAPI], [test "x$have_wapi" = "xyes"])

AC_CHECK_FUNCS([getline], [getline_f=yes], [getline_f=no])
AM_CONDITIONAL([HAVE_GETLINE], [test "x$getline_f" = "xyes"])

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [enable facilities to improve debugging])])

AC_ARG_ENABLE([awa-tracing],
  [AS_HELP_STRING([--enable-awa-tracing], [trace opcodes execution])])

AC_ARG_ENABLE([book],
  [AS_HELP_STRING([--enable-book], [build the AWA5 book])])

AS_IF([test "x$enable_debug" = "xyes"],
  [AC_DEFINE([DEBUG], [1], [Define to 1 if debug is enabled])
   AX_VAR_PATDEL([CFLAGS], [\(-g\|-O2\)])
   CFLAGS="$CFLAGS -g3"
   AX_CHECK_COMPILER_FLAG([-fsanitize=address], [CFLAGS="$CFLAGS -fsanitize=address"])
   AX_CHECK_COMPILER_FLAG([-fsanitize=undefined], [CFLAGS="$CFLAGS -fsanitize=undefined"])])

AS_IF([test "x$enable_awa_tracing" = "xyes"],
  [AC_DEFINE([OPCODE_TRACING], [1], [Define to 1 if opcode tracing is enabled])])

have_latex=no
AS_IF([test "x$enable_book" = "xyes"],
  [AC_CHECK_PROGS([LATEX], [xelatex lualatex], [no])
   AS_IF([test "x$LATEX" != "xno"], [have_latex=yes])])
AM_CONDITIONAL([HAVE_LATEX], [test "x$have_latex" != "xno"])

AC_OUTPUT
