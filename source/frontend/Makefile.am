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
noinst_LIBRARIES = libfrontend.a

# Source files.
libfrontend_a_SOURCES = \
processrenderoptions.cpp display.cpp simplefrontend.h imageprocessing.cpp parsermessagehandler.h rendermessagehandler.h console.cpp imageprocessing.h display.h filemessagehandler.cpp animationprocessing.h console.h renderfrontend.cpp parsermessagehandler.cpp shelloutprocessing.cpp defaultplatformbase.cpp imagemessagehandler.cpp animationprocessing.cpp defaultplatformbase.h configfrontend.h processrenderoptions.h renderfrontend.h shelloutprocessing.h imagemessagehandler.h filemessagehandler.h rendermessagehandler.cpp

# Include paths for headers.
AM_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/source \
  -I$(top_builddir)/source \
  -I$(top_srcdir)/source/base \
  -I$(top_srcdir)/unix \
  -I$(top_srcdir)/vfe \
  -I$(top_srcdir)/vfe/unix
