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
noinst_LIBRARIES = libbase.a

# Source files.
libbase_a_SOURCES = \
textstreambuffer.h textstream.h povms.h povms.cpp platformbase.h image/ppm.h image/colourspace.cpp image/pgm.h image/pgm.cpp image/openexr.cpp image/jpeg.cpp image/jpeg_pov.h image/image.h image/colourspace.h image/bmp.h image/iff.h image/encoding.cpp image/encoding.h image/targa.h image/tiff.cpp image/targa.cpp image/png_pov.h image/hdr.h image/png.cpp image/ppm.cpp image/gifdecod.cpp image/openexr.h image/image.cpp image/gif.cpp image/bmp.cpp image/iff.cpp image/hdr.cpp image/metadata.h image/tiff_pov.h image/gif.h timer.h fileinputoutput.h configbase.h types.h pointer.h path.h fileinputoutput.cpp textstreambuffer.cpp animation/animation.h animation/animation.cpp animation/moov.cpp animation/moov.h path.cpp timer.cpp povmscpp.h povmsgid.h povdebug.h processoptions.h textstream.cpp processoptions.cpp stringutilities.cpp stringutilities.h pov_err.h pov_err.cpp povmscpp.cpp

# Include paths for headers.
AM_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/source \
  -I$(top_builddir)/source \
  -I$(top_srcdir)/unix \
  -I$(top_srcdir)/vfe \
  -I$(top_srcdir)/vfe/unix
