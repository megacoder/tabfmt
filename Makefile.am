## Process this file with automake to produce Makefile.in

## Makefile.am for the Tabfmt package.
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

SUBDIRS = po doc lib src man tests
ACLOCAL_AMFLAGS = -I m4
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure config.h.in
EXTRA_DIST = autogen.sh ChangeLog.0 ChangeLog.1

# Maintainer targets

ChangeLog: .ChangeLog

.ChangeLog:
	cd $(top_srcdir) && \
	if test -d CVS ; then \
	  cvs2cl --fsf --FSF \
            --usermap .cvsusers \
	    --window 86400 --ignore ChangeLog \
            --utc --log-opts -d """>2006-01-07 19:53""" && \
	  cat .cvscopying >>ChangeLog ; \
	fi

listing: tabfmt.ps configure.ac.ps Makefile.am.ps

tabfmt.ps: $(top_srcdir)/src/tabfmt.c
	enscript --fancy-header --columns=2 --landscape --highlight \
	  --nup=2 --output=$@ --color $<

configure.ac.ps: $(top_srcdir)/configure.ac
	enscript --fancy-header --columns=2 --landscape --highlight=m4 \
	  --nup=2 --output=$@ --color $<

Makefile.am.ps: $(top_srcdir)/Makefile.am
	enscript --fancy-header --columns=2 --landscape --highlight=makefile \
	  --nup=2 --output=$@ --color $<

## Makefile.am ends here
