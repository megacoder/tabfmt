## Process this file with automake to produce Makefile.in.

## Makefile.am for the tests subdirectory of the Tabfmt package.
## Copyright (C) 2005, 2006 Claudio Jolowicz
## 
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of the
## License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
## 
## $Id: Makefile.am,v 1.1.1.1 2009/07/16 02:17:25 reynolds Exp $

TESTS = test-1

EXTRA_DIST = $(TESTS)

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) PATH=.:../src:$$PATH \
		    tabfmt=`echo tabfmt|sed '$(transform)'` \
		    $(SHELL)

## tests/Makefile.am ends here.
