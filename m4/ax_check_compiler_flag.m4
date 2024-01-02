# AX_CHECK_COMPILER_FLAG(flag, [action-if-found, [action-if-not-found]]) -*- Mode: Autoconf -*-
#
# Test the current compiler for support of FLAG, which is an option
# passed on the comand line.
# For example, testing for `-fflag` is equivalent to execute
# `$(compiler) -fflag`.
# If the flag is supported, ACTION-IF-FOUND will be executed,
# otherwise ACTION-IF-NOT-FOUND.
AC_DEFUN([AX_CHECK_COMPILER_FLAG],
 [AS_VAR_SET([supported_flag], [no])
  AS_VAR_COPY([old_cflags], [CFLAGS])
dnl
  CFLAGS="$CFLAGS -Werror $1"
dnl
  AC_MSG_CHECKING([if compiler supports $1])
  AC_LINK_IFELSE([AC_LANG_SOURCE([int main() { return 0; }])],
    [AS_VAR_SET([supported_flag], [yes])],
    [AS_VAR_SET([supported_flag], [no])])
  AC_MSG_RESULT([$supported_flag])
dnl
  AS_VAR_SET([CFLAGS], [$old_cflags])
  AS_UNSET([old_cflags])
dnl
  AS_IF([test "x$supported_flag" = "xyes"],
    [m4_if([$2], ,:,[$2])],
    [m4_if([$3], ,:,[$3])])
dnl
  AS_UNSET([$supported_flag])])
