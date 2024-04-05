dnl This is free and unencumbered software released into the public domain.
dnl
dnl Anyone is free to copy, modify, publish, use, compile, sell, or
dnl distribute this software, either in source code form or as a compiled
dnl binary, for any purpose, commercial or non-commercial, and by any
dnl means.
dnl
dnl In jurisdictions that recognize copyright laws, the author or authors
dnl of this software dedicate any and all copyright interest in the
dnl software to the public domain. We make this dedication for the benefit
dnl of the public at large and to the detriment of our heirs and
dnl successors. We intend this dedication to be an overt act of
dnl relinquishment in perpetuity of all present and future rights to this
dnl software under copyright law.
dnl
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
dnl EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
dnl OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
dnl ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
dnl OTHER DEALINGS IN THE SOFTWARE.
dnl
dnl For more information, please refer to <http://unlicense.org/>

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
