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

# Libraries to build.
noinst_LIBRARIES = libvfe.a

# Source files.
libvfe_a_SOURCES = \
vfecontrol.cpp vfe.h vfedisplay.cpp syspovdebug.h syspovprotobackend.h vfepovms.cpp syspovprotofrontend.h vfe.cpp vfeprecomp.h vfesession.h vfeconf.h vfesession.cpp vfeprecomp.cpp vfepovms.h syspovprotobase.h unix/platformbase.cpp unix/unixoptions.h unix/syspovconfig.h unix/unixoptions.cpp unix/syspovconfigbase.h unix/vfeplatform.cpp unix/vfeplatform.h unix/syspovconfigfrontend.h unix/syspovconfigbackend.h unix/unixconsole.cpp

# Include paths for headers.
AM_CPPFLAGS = \
  -I$(top_srcdir)/vfe/unix \
  -I$(top_srcdir)/unix \
  -I$(top_srcdir)/source \
  -I$(top_srcdir)/source/base \
  -I$(top_srcdir)/source/backend

# Extra definitions for compiling.
# They cannot be placed in config.h since they indirectly rely on $prefix.
DEFS = \
  @DEFS@ \
  -DPOVLIBDIR=\"@datadir@/@PACKAGE@-@VERSION_BASE@\" \
  -DPOVCONFDIR=\"@sysconfdir@/@PACKAGE@/@VERSION_BASE@\" \
  -DPOVCONFDIR_BACKWARD=\"@sysconfdir@\"
