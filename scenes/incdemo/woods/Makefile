# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

#
#   Makefile.am
#
#  from Persistence of Vision Raytracer
#  Copyright 1995,1999 Persistence of Vision Team
#---------------------------------------------------------------------------
#  NOTICE: This source code file is provided so that users may experiment
#  with enhancements to POV-Ray and to port the software to platforms other
#  than those supported by the POV-Ray Team.  There are strict rules under
#  which you are permitted to use this file.  The rules are in the file
#  named POVLEGAL.DOC which should be distributed with this file.
#  If POVLEGAL.DOC is not available or for more info please contact the POV-Ray
#  Team Coordinator by email to team-coord@povray.org or visit us on the web at
#  http://www.povray.org. The latest version of POV-Ray may be found at this 
#  site.
#
# This program is based on the popular DKB raytracer version 2.12.
# DKBTrace was originally written by David K. Buck.
# DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
#
#
##############################################################################


SHELL = /bin/sh

srcdir = .
top_srcdir = ../../..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/povray
pkglibdir = $(libdir)/povray
pkgincludedir = $(includedir)/povray

top_builddir = ../../..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
CC = gcc
CPP = gcc -E
CXX = c++
CXXCPP = c++ -E
MAKEINFO = makeinfo
PACKAGE = povray
VERSION = 3.50c

woodsdir = ${prefix}/share/povray-3.5/scenes/incdemo/woods
woods_DATA = ash.pov bubinga.pov cedar.pov morewood.doc orngwood.pov teak.pov testcam.inc testobjs.inc whiteash.pov wood_10.pov wood_11.pov wood_12.pov wood_13.pov wood_14.pov wood_15.pov wood_16.pov wood_17.pov wood_18.pov wood_19.pov wood_1.pov wood_2.pov wood_3.pov wood_4.pov wood_5.pov wood_6.pov wood_7.pov wood_8.pov wood_9.pov woods1.pov woods2.pov woods.doc 
EXTRA_DIST = ash.pov bubinga.pov cedar.pov morewood.doc orngwood.pov teak.pov testcam.inc testobjs.inc whiteash.pov wood_10.pov wood_11.pov wood_12.pov wood_13.pov wood_14.pov wood_15.pov wood_16.pov wood_17.pov wood_18.pov wood_19.pov wood_1.pov wood_2.pov wood_3.pov wood_4.pov wood_5.pov wood_6.pov wood_7.pov wood_8.pov wood_9.pov woods1.pov woods2.pov woods.doc 
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../../src/conf.h
CONFIG_CLEAN_FILES = 
DATA =  $(woods_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu scenes/incdemo/woods/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-woodsDATA: $(woods_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(woodsdir)
	@list='$(woods_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(woodsdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(woodsdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(woodsdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(woodsdir)/$$p; \
	  fi; fi; \
	done

uninstall-woodsDATA:
	@$(NORMAL_UNINSTALL)
	list='$(woods_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(woodsdir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = scenes/incdemo/woods

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu scenes/incdemo/woods/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am: install-woodsDATA
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-woodsDATA
uninstall: uninstall-am
all-am: Makefile $(DATA)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(woodsdir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: uninstall-woodsDATA install-woodsDATA tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
