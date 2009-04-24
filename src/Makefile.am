## Process this file with automake to produce Makefile.in
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
jversion.h :
	@echo "Generating jversion.h"
	strings /usr/lib/libjpeg.a /usr/local/lib/libjpeg.a | grep '^.*  [0-3]*[0-9]-[A-Z][a-z][a-z]-[12][0-9][0-9][0-9]' | head -n 1 | sed 's/^\(.*\)/#define JVERSION \"\1\"/' > jversion.h || echo "ERROR: Can't find libjpeg.a"
bin_PROGRAMS = povray
povray_SOURCES = atmosph.cpp bbox.cpp bcyl.cpp benchmark.cpp bezier.cpp blob.cpp boxes.cpp bsphere.cpp camera.cpp chi2.cpp colour.cpp colutils.cpp cones.cpp csg.cpp discs.cpp express.cpp file_pov.cpp fncode.cpp fnintern.cpp fnpovfpu.cpp fnsyntax.cpp fpmetric.cpp fractal.cpp function.cpp gif.cpp gifdecod.cpp hcmplx.cpp hfield.cpp histogra.cpp iff.cpp image.cpp interior.cpp isosurf.cpp jpeg_pov.cpp lathe.cpp lbuffer.cpp lightgrp.cpp lighting.cpp matrices.cpp media.cpp mesh.cpp normal.cpp objects.cpp octree.cpp optin.cpp optout.cpp parse.cpp parsestr.cpp parstxtr.cpp pattern.cpp pgm.cpp photons.cpp pigment.cpp planes.cpp png_pov.cpp point.cpp poly.cpp polygon.cpp polysolv.cpp pov_mem.cpp povms.cpp povmscpp.cpp povmsend.cpp povmsrec.cpp povray.cpp pov_util.cpp ppm.cpp prism.cpp quadrics.cpp quatern.cpp rad_data.cpp radiosit.cpp ray.cpp render.cpp renderio.cpp sor.cpp spheres.cpp sphsweep.cpp splines.cpp statspov.cpp super.cpp targa.cpp texture.cpp tiff_pov.cpp tokenize.cpp torus.cpp triangle.cpp truetype.cpp txttest.cpp userdisp.cpp userio.cpp vbuffer.cpp vlbuffer.cpp warps.cpp unix.cpp povxlib.c config.h jversion.h optout.h povxlib.h atmosph.h bbox.h bcyl.h bezier.h blob.h boxes.h bsphere.h camera.h chi2.h colour.h colutils.h cones.h csg.h discs.h express.h file_pov.h fncode.h fnintern.h fnpovfpu.h fnsyntax.h fpmetric.h fractal.h frame.h function.h gifdecod.h gif.h hcmplx.h hfield.h histogra.h iff.h image.h interior.h isosurf.h jpeg_pov.h lathe.h lbuffer.h lightgrp.h lighting.h matrices.h media.h mesh.h normal.h objects.h octree.h optin.h parse.h parsestr.h parstxtr.h pattern.h pgm.h photons.h pigment.h planes.h png_pov.h point.h polygon.h poly.h polysolv.h pov_err.h pov_mem.h povmscpp.h povmsend.h povmsgid.h povms.h povmsrec.h povproto.h povray.h pov_util.h ppm.h prism.h quadrics.h quatern.h radiosit.h ray.h render.h renderio.h sor.h spheres.h sphsweep.h splines.h statspov.h super.h targa.h texture.h tiff_pov.h tokenize.h torus.h triangle.h truetype.h txttest.h userdisp.h userio.h vbuffer.h vector.h vlbuffer.h warps.h  conf.h
DEFS = -DPREFIX=\"$(prefix)\" -DPOV_LIB_DIR=\"@datadir@/povray-3.5\" -DCOMPILER_VER=\".Linux.$(CC)\" -DSYSCONFDIR=\"$(sysconfdir)\" -DUSE_IO_RESTRICTIONS=\"$(ac_use_io_restrictions)\"
NOMULTICHAR = `if [ "X$(CC)" = "Xgcc" ]; then echo "-Wno-multichar"; fi `
LDADD = @X_LIBS@
CFLAGS = -O3 -mcpu=i586 -march=i586 -finline-functions -ffast-math -fomit-frame-pointer -funroll-loops -fexpensive-optimizations -malign-double -foptimize-sibling-calls -minline-all-stringops -Wno-multichar
CXXFLAGS = $(NOMULTICHAR) -O3 -finline-functions -ffast-math -fomit-frame-pointer -funroll-loops -fexpensive-optimizations -malign-double -foptimize-sibling-calls -minline-all-stringops -Wno-multichar
LDFLAGS = @X_CFLAGS@
EXTRA_DIST = xpovicon.xbm xpovmask.xbm xpovicon.xpm 
