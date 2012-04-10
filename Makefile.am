##############################################################################
#               Makefile.am
#
# Process this file with automake to produce Makefile.in
#
# from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
# Copyright 1991-2003 Persistence of Vision Team
# Copyright 2003-2009 Persistence of Vision Raytracer Pty. Ltd.
# ---------------------------------------------------------------------------
# NOTICE: This source code file is provided so that users may experiment
# with enhancements to POV-Ray and to port the software to platforms other
# than those supported by the POV-Ray developers. There are strict rules
# regarding how you are permitted to use this file. These rules are contained
# in the distribution and derivative versions licenses which should have been
# provided with this file.
#
# These licences may be found online, linked from the end-user license
# agreement that is located at http://www.povray.org/povlegal.html
# ---------------------------------------------------------------------------
# POV-Ray is based on the popular DKB raytracer version 2.12.
# DKBTrace was originally written by David K. Buck.
# DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
##############################################################################

# Makefile.am for the source distribution of POV-Ray 3.7 for UNIX
# Written by POV-Ray Bugtracker http://bugs.povray.org/

# Directories.
povlibdir = @datadir@/@PACKAGE@-@VERSION_BASE@
povdocdir = @datadir@/doc/@PACKAGE@-@VERSION_BASE@
povconfdir = @sysconfdir@/@PACKAGE@/@VERSION_BASE@
povuser = $(HOME)/.@PACKAGE@
povconfuser = $(povuser)/@VERSION_BASE@
povinstall = $(top_builddir)/install.log
povowner = @povowner@
povgroup = @povgroup@

# Directories to build.
SUBDIRS = source vfe unix

# Additional files to distribute.
EXTRA_DIST = \
  bootstrap kde_install.sh \
  doc icons include ini scenes scripts \
  povray.ini.in changes.txt revision.txt

# Additional files to clean with 'make distclean'.
DISTCLEANFILES = $(top_builddir)/povray.ini
CONFIG_CLEAN_FILES = $(top_builddir)/source/jversion.h

# Render a test scene for 'make check'.
# This is meant to run before 'make install'.
check: all
	$(top_builddir)/unix/povray +i$(top_srcdir)/scenes/advanced/biscuit.pov -f +d +p +v +w320 +h240 +a0.3 +L$(top_srcdir)/include

# Install scripts in povlibdir.
nobase_povlib_SCRIPTS = scripts/allscene.sh scripts/render_anim.sh scripts/render_scene.sh scripts/portfolio.sh scripts/rerunpov.sh scripts/allanim.sh scripts/runpov.sh

# Install documentation in povdocdir.
povdoc_DATA = AUTHORS ChangeLog NEWS

# Install configuration and INI files in povconfdir.
dist_povconf_DATA = povray.conf
povray.conf:

povconf_DATA = povray.ini
povray.ini:
	cat $(top_srcdir)/povray.ini.in | sed "s,__POVLIBDIR__,$(povlibdir),g" > $(top_builddir)/povray.ini

# Install man page.
dist_man_MANS = povray.1

# Remove all unwanted files for 'make dist(check)'.
# Make all files user read-writeable.
dist-hook:
	chmod -R u+rw $(distdir)
	chmod 755 $(distdir)/scripts/*
	rm -f    `find $(distdir) -name "*.h.in~"`
	rm -f -r `find $(distdir) -name autom4te.cache`
	rm -f -r `find $(distdir) -name .libs`
	rm -f    $(distdir)/source/jversion.h

# Manage various data files for 'make install'.
# Creates an install.log file to record created folders and files.
# Folder paths are prepended (using POSIX printf) to ease later removal in 'make uninstall'.
# Don't be too verbose so as to easily spot possible problems.
install-data-local:
	cat /dev/null > $(povinstall);
	@echo "Creating data directories..."; \
	list='$(top_srcdir)/icons $(top_srcdir)/include $(top_srcdir)/ini $(top_srcdir)/scenes'; \
	dirlist=`find $$list -type d | sed s,$(top_srcdir)/,,`; \
	for p in "" $$dirlist ; do \
	  $(mkdir_p) $(DESTDIR)$(povlibdir)/$$p && printf "%s\n" "$(DESTDIR)$(povlibdir)/$$p" "`cat $(povinstall)`" > $(povinstall); \
	done; \
	echo "Copying data files..."; \
	filelist=`find $$list -type f | sed s,$(top_srcdir)/,,`; \
	for f in $$filelist ; do \
	  $(INSTALL_DATA) $(top_srcdir)/$$f $(DESTDIR)$(povlibdir)/$$f && echo "$(DESTDIR)$(povlibdir)/$$f" >> $(povinstall); \
	done
	@echo "Creating documentation directories..."; \
	dirlist=`find $(top_srcdir)/doc/ -type d | sed s,$(top_srcdir)/doc/,,`; \
	for p in "" $$dirlist ; do \
	  $(mkdir_p) $(DESTDIR)$(povdocdir)/$$p && printf "%s\n" "$(DESTDIR)$(povdocdir)/$$p" "`cat $(povinstall)`" > $(povinstall); \
	done
	@echo "Copying documentation files..."; \
	filelist=`find $(top_srcdir)/doc/ -type f | sed s,$(top_srcdir)/doc/,,`; \
	for f in $$filelist ; do \
	  $(INSTALL_DATA) $(top_srcdir)/doc/$$f $(DESTDIR)$(povdocdir)/$$f && echo "$(DESTDIR)$(povdocdir)/$$f" >> $(povinstall); \
	done
	@echo "Creating user directories..."; \
	for p in $(povuser) $(povconfuser) ; do \
	  $(mkdir_p) $$p && chown $(povowner) $$p && chgrp $(povgroup) $$p && printf "%s\n" "$$p" "`cat $(povinstall)`" > $(povinstall); \
	done
	@echo "Copying user configuration and INI files..."; \
	for f in povray.conf povray.ini ; do \
	  if test -f $(povconfuser)/$$f; then \
	    echo "Creating backup of $(povconfuser)/$$f"; \
	    mv -f $(povconfuser)/$$f $(povconfuser)/$$f.bak; \
	  fi; \
	done; \
	$(INSTALL_DATA) $(top_srcdir)/povray.conf $(povconfuser)/povray.conf && chown $(povowner) $(povconfuser)/povray.conf && chgrp $(povgroup) $(povconfuser)/povray.conf  && echo "$(povconfuser)/povray.conf" >> $(povinstall); \
	$(INSTALL_DATA) $(top_builddir)/povray.ini $(povconfuser)/povray.ini && chown $(povowner) $(povconfuser)/povray.ini && chgrp $(povgroup) $(povconfuser)/povray.ini  && echo "$(povconfuser)/povray.ini" >> $(povinstall)

# Remove data, config, and empty folders for 'make uninstall'.
# Use 'hook' instead of 'local' so as to properly remove *empty* folders (e.g. scripts).
# The last echo prevents getting error from failed rmdir command.
uninstall-hook:
	@if test -f $(top_builddir)/install.log ; then \
	  rmdir $(DESTDIR)$(povlibdir)/scripts; \
	  echo "Using install info from $(top_builddir)/install.log"; \
	  echo "Removing data, documentation, and configuration files..."; \
	  for f in `cat $(top_builddir)/install.log` ; do \
	    test -f $$f && rm -f $$f ; \
	  done; \
	  echo "Removing empty directories..."; \
	  for f in `cat $(top_builddir)/install.log` ; do \
	    test -d $$f && rmdir $$f ; \
	  done; \
	  echo "Removing $(top_builddir)/install.log" && rm -f $(top_builddir)/install.log ; \
	else \
	  "Removing all data unconditionally"; \
	  rm -f -r $(DESTDIR)$(povlibdir); \
	  rm -f -r $(DESTDIR)$(povdocdir); \
	  rm -f    $(DESTDIR)$(povconfdir)/povray.ini; \
	  rmdir    $(DESTDIR)$(povconfdir); \
	  rmdir    $(DESTDIR)$(sysconfdir)/@PACKAGE@; \
	  rm -f    $(povconfuser)/povray.conf; \
	  rm -f    $(povconfuser)/povray.ini; \
	  rmdir    $(povconfuser); \
	  rmdir    $(povuser); \
	fi; \
	echo "Uninstall finished"
