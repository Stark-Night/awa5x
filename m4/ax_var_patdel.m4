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

# Autoconf macro to remove a pattern from a variable. -*- Mode: Autoconf -*-
#
# AX_VAR_PATDEL(var, pat)
# Remove the substring(s) matching `pat` from the variable `var`.
# `var` must be written without the leading $ (dollar sign).
AC_DEFUN([AX_VAR_PATDEL],
  [AS_VAR_COPY([workvar], [$1])
   workvar=`printf "%s\n" "$workvar" | sed "s/$2//g"`
   AS_VAR_COPY([$1], [workvar])
   AS_UNSET([workvar])])
