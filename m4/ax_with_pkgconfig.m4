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

# Autoconf macro to add a --with-package option with pkg-config fallback. -*- Mode: Autoconf -*-
#
# AX_WITH_PKGCONFIG(prefix, package, function, [action-if-found, [action-if-not-found]])
# Add a --with-<package>=PATH option to configure.
# This option allows selecting an alternative installation for PACKAGE.
# If the option is not specified it will implicitly use pkg-config.
#
# In any case, if PACKAGE is found configure will define
# <PREFIX>_CFLAGS and <PREFIX>_LIBS, as per pkg-config conventions.
# Additionally, the macro HAVE_PKG_<PREFIX> will be defined to 1 if
# the package was found in one way or another.
#
# If `--with-package=no' is specified, ACTION-IF-NOT-FOUND will always
# be executed if provided; otherwise, the fourth and fifth arguments,
# both optional, will behave as one will expect.
AC_DEFUN([AX_WITH_PKGCONFIG],
  [AC_ARG_WITH([$2],
     [AS_HELP_STRING([--with-][$2][=PATH],
        [search PATH for a $2 installation])])
dnl
   AS_VAR_COPY([with_lib], [withval])
   AS_CASE([$withval],
     [no],
     [scanmore=no;m4_if([$5], ,:,[$5])],
     [yes],
     [usepkgconfig=yes],
     [usepkgconfig=no])
   AS_IF([test "x$with_lib" = "x"], [usepkgconfig=yes])
dnl
   AS_IF([test "x$usepkgconfig" = "xyes"],
     [PKG_CHECK_MODULES([$1], [$2],
        [AC_DEFINE([HAVE_PKG_$1], [1], [Define to 1 if we have $1])
	 m4_if([$4], ,:,[$4])],
	[m4_if([$5], ,:,[$m5])])],
     [AS_IF([test "x$scanmore" = "x"],
        [AS_VAR_COPY([old_cflags], [CFLAGS])
         AS_VAR_COPY([old_cppflags], [CPPFLAGS])
         AS_VAR_COPY([old_ldflags], [LDFLAGS])
dnl
         CFLAGS="$CFLAGS -I$with_lib/include"
         CPPFLAGS="$CPPFLAGS -I$with_lib/include"
         LDFLAGS="$LDFLAGS -L$with_lib/lib"
dnl
         AS_VAR_SET([libname],
           [m4_bpatsubst([$2], [\(lib\)], [])])
dnl
         AC_CHECK_LIB([$libname],[$3],[foundlib=yes],[foundlib=no])
         AS_IF([test "x$foundlib" = "xyes"],
           [AC_DEFINE([HAVE_PKG_$1], [1], [Define to 1 if we have $1])
            AS_VAR_SET([$1_CFLAGS], ["-I$with_lib/include"])
            AS_VAR_SET([$1_LIBS], ["-L$with_lib/libs -l$libname"])
            m4_if([$4], ,:,[$4])]
           [m4_if([$5], ,:,[$5])])
dnl
         AS_VAR_SET([CFLAGS], [$old_cflags])
         AS_VAR_SET([CPPFLAGS], [$old_cppflags])
         AS_VAR_SET([LDFLAGS], [$old_ldflags])
         AS_UNSET([libname])
         AS_UNSET([foundlib])
         AS_UNSET([old_cflags])
         AS_UNSET([old_cppflags])
         AS_UNSET([old_ldflags])])])
dnl
   AS_UNSET([with_lib])
   AS_UNSET([usepkgconfig])
   AS_UNSET([scanmore])])
