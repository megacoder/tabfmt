## Process this file with automake to produce Makefile.in.

## Makefile.am for the man subdirectory of the Tabfmt package.
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
## $Id: Makefile.am,v 1.2 2009/07/17 16:40:01 treynold Exp $

dist_man_MANS = tabfmt.1
man_aux = $(dist_man_MANS:.1=.x)
HELP2MAN = $(srcdir)/help2man
noinst_SCRIPTS = $(HELP2MAN)
EXTRA_DIST = $(man_aux) $(noinst_SCRIPTS)
MAINTAINERCLEANFILES = Makefile.in $(dist_man_MANS)

# Depend on configure.ac to get version number changes.
common_dep = $(top_srcdir)/configure.ac
srcsrcdir = $(top_srcdir)/src
tabfmt.1: $(common_dep) $(srcdir)/tabfmt.x $(srcsrcdir)/tabfmt.c

SUFFIXES = .x .1

.x.1:
	echo "Updating man page $@"
	$(PERL) $(HELP2MAN) \
	    --include=$(srcdir)/$*.x \
	    --output=$@ ../src/$*

# man/Makefile.am ends here
