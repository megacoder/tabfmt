dnl Process this file with autoconf to produce a configure script.

# configure.ac for the Tabfmt package.
# Copyright (C) 2005, 2006 Claudio Jolowicz
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# $Id: configure.ac,v 1.1.1.1 2009/07/16 02:17:25 reynolds Exp $

# Prologue
AC_INIT([tabfmt], [0.1], [tabfmt-devel@lists.sourceforge.net])
AC_PREREQ(2.59)
AC_REVISION($Revision: 1.1.1.1 $)
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_SRCDIR(src/tabfmt.c)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([gnits dist-bzip2 -Wall -Werror])
AC_GNU_SOURCE

# Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_INSTALL
AC_PROG_LIBTOOL
dnl for help2man
AC_CHECK_PROG(PERL, perl, perl)

# Checks for libraries.

# Invoke gnulib.
gl_MODULES(assert dirname error exit free getopt getline \
  malloc realloc size_max strtol)
gl_LIBTOOL
gl_INIT

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/types.h strings.h errno.h limits.h error.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])
AC_CHECK_FUNCS(bzero memset, break)
AC_CHECK_FUNCS(strchr index, break)
dnl gnulib's lib/error.c uses strerror without checks, if we don't have _LIBC...

dnl internationalization macros
AM_GNU_GETTEXT(external)
AM_GNU_GETTEXT_VERSION([0.14.5])

# Epilogue
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile man/Makefile \
           doc/Makefile po/Makefile.in tests/Makefile])
dnl AC_CONFIG_FILES([stamp-h], [echo timestamp > stamp-h])
AC_OUTPUT

dnl configure.ac ends here.
