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

# Programs to build.
bin_PROGRAMS = povray

# Source files.
povray_SOURCES = \
  disp.h \
  disp_sdl.cpp disp_sdl.h \
  disp_text.cpp disp_text.h

# Include paths for headers.
AM_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/source \
  -I$(top_builddir)/source \
  -I$(top_srcdir)/source/backend \
  -I$(top_srcdir)/source/base \
  -I$(top_srcdir)/source/frontend \
  -I$(top_srcdir)/vfe \
  -I$(top_srcdir)/vfe/unix

# Libraries to link with.
# Beware: order does matter!
LDADD = \
  $(top_builddir)/vfe/libvfe.a \
  $(top_builddir)/source/backend/libbackend.a \
  $(top_builddir)/source/frontend/libfrontend.a \
  $(top_builddir)/source/base/libbase.a \
  $(top_builddir)/source/libpovray.a
