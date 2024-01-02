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
