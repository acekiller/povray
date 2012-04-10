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
noinst_LIBRARIES = libbackend.a

# Source files.
libbackend_a_SOURCES = \
povray.cpp math/splines.cpp math/matrices.h math/mathutil.cpp math/hcmplx.cpp math/quatern.cpp math/polysolv.cpp math/chi2.h math/chi2.cpp math/vector.h math/quatern.h math/mathutil.h math/splines.h math/matrices.cpp math/polysolv.h math/hcmplx.h bounding/bcyl.h bounding/bbox.cpp bounding/boundingtask.h bounding/bsphere.h bounding/bbox.h bounding/boundingtask.cpp bounding/bsphere.cpp bounding/bcyl.cpp colour/colutils.cpp colour/spectral.h colour/colour.cpp colour/spectral.cpp colour/colutils.h colour/colour.h support/taskqueue.cpp support/task.h support/msgutil.h support/statistics.h support/bsptree.h support/octree.cpp support/imageutil.h support/taskqueue.h support/msgutil.cpp support/task.cpp support/fileutil.h support/fileutil.cpp support/jitter.cpp support/octree.h support/statistics.cpp support/bsptree.cpp support/randomsequences.h support/jitter.h support/fixedallocator.h support/imageutil.cpp support/simplevector.h support/randomsequences.cpp scene/view.h scene/objects.cpp scene/camera.h scene/scene.h scene/threaddata.cpp scene/camera.cpp scene/atmosph.cpp scene/view.cpp scene/scene.cpp scene/objects.h scene/threaddata.h scene/atmosph.h render/tracepixel.h render/rendertask.h render/radiositytask.h render/tracepixel.cpp render/ray.h render/trace.cpp render/radiositytask.cpp render/ray.cpp render/tracetask.cpp render/tracetask.h render/trace.h render/rendertask.cpp pattern/warps.h pattern/pattern.cpp pattern/pattern.h pattern/warps.cpp configbackend.h shape/discs.cpp shape/bezier.cpp shape/fpmetric.h shape/sor.h shape/mesh.cpp shape/spheres.cpp shape/fractal.cpp shape/fractal.h shape/blob.cpp shape/planes.h shape/quadrics.cpp shape/boxes.cpp shape/polygon.h shape/torus.cpp shape/super.cpp shape/hfield.cpp shape/blob.h shape/triangle.h shape/isosurf.cpp shape/poly.cpp shape/discs.h shape/boxes.h shape/cones.cpp shape/lathe.cpp shape/sphsweep.h shape/bezier.h shape/mesh.h shape/prism.cpp shape/torus.h shape/spheres.h shape/planes.cpp shape/csg.h shape/polygon.cpp shape/triangle.cpp shape/super.h shape/prism.h shape/fpmetric.cpp shape/csg.cpp shape/sphsweep.cpp shape/ovus.cpp shape/quadrics.h shape/lathe.h shape/ovus.h shape/sor.cpp shape/truetype.h shape/poly.h shape/truetype.cpp shape/isosurf.h shape/cones.h shape/hfield.h parser/parstxtr.cpp parser/reswords.cpp parser/express.cpp parser/reswords.h parser/function.cpp parser/parsestr.cpp parser/tokenize.cpp parser/fnsyntax.cpp parser/parse.h parser/parse.cpp povray.h frame.h texture/avxfma4check.h texture/normal.cpp texture/pigment.cpp texture/texture.h texture/pigment.h texture/normal.h texture/texture.cpp interior/media.cpp interior/interior.cpp interior/interior.h interior/media.h lighting/photonsortingtask.cpp lighting/photonstrategytask.cpp lighting/subsurface.cpp lighting/radiosity.cpp lighting/photonestimationtask.h lighting/radiosity.h lighting/photonshootingtask.cpp lighting/point.h lighting/photonshootingstrategy.cpp lighting/photons.cpp lighting/photonstrategytask.h lighting/photonsortingtask.h lighting/photons.h lighting/subsurface.h lighting/photonshootingstrategy.h lighting/photonshootingtask.h lighting/photonestimationtask.cpp lighting/rad_data.cpp lighting/point.cpp vm/fnpovfpu.h vm/fnpovfpu.cpp vm/fnintern.cpp vm/fnintern.h vm/fncode.h vm/fncode.cpp control/messagefactory.h control/renderbackend.h control/messagefactory.cpp control/benchmark.h control/benchmark.cpp control/renderbackend.cpp

# Include paths for headers.
AM_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/source \
  -I$(top_builddir)/source \
  -I$(top_srcdir)/source/base \
  -I$(top_srcdir)/unix \
  -I$(top_srcdir)/vfe \
  -I$(top_srcdir)/vfe/unix
