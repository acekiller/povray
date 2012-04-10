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

# Directories to build.
SUBDIRS = backend base frontend

# Libraries to build.
noinst_LIBRARIES = libpovray.a

# Source files.
libpovray_a_SOURCES = \
jversion.h backend/povray.cpp backend/math/splines.cpp backend/math/matrices.h backend/math/mathutil.cpp backend/math/hcmplx.cpp backend/math/quatern.cpp backend/math/polysolv.cpp backend/math/chi2.h backend/math/chi2.cpp backend/math/vector.h backend/math/quatern.h backend/math/mathutil.h backend/math/splines.h backend/math/matrices.cpp backend/math/polysolv.h backend/math/hcmplx.h backend/bounding/bcyl.h backend/bounding/bbox.cpp backend/bounding/boundingtask.h backend/bounding/bsphere.h backend/bounding/bbox.h backend/bounding/boundingtask.cpp backend/bounding/bsphere.cpp backend/bounding/bcyl.cpp backend/colour/colutils.cpp backend/colour/spectral.h backend/colour/colour.cpp backend/colour/spectral.cpp backend/colour/colutils.h backend/colour/colour.h backend/support/taskqueue.cpp backend/support/task.h backend/support/msgutil.h backend/support/statistics.h backend/support/bsptree.h backend/support/octree.cpp backend/support/imageutil.h backend/support/taskqueue.h backend/support/msgutil.cpp backend/support/task.cpp backend/support/fileutil.h backend/support/fileutil.cpp backend/support/jitter.cpp backend/support/octree.h backend/support/statistics.cpp backend/support/bsptree.cpp backend/support/randomsequences.h backend/support/jitter.h backend/support/fixedallocator.h backend/support/imageutil.cpp backend/support/simplevector.h backend/support/randomsequences.cpp backend/scene/view.h backend/scene/objects.cpp backend/scene/camera.h backend/scene/scene.h backend/scene/threaddata.cpp backend/scene/camera.cpp backend/scene/atmosph.cpp backend/scene/view.cpp backend/scene/scene.cpp backend/scene/objects.h backend/scene/threaddata.h backend/scene/atmosph.h backend/render/tracepixel.h backend/render/rendertask.h backend/render/radiositytask.h backend/render/tracepixel.cpp backend/render/ray.h backend/render/trace.cpp backend/render/radiositytask.cpp backend/render/ray.cpp backend/render/tracetask.cpp backend/render/tracetask.h backend/render/trace.h backend/render/rendertask.cpp backend/pattern/warps.h backend/pattern/pattern.cpp backend/pattern/pattern.h backend/pattern/warps.cpp backend/configbackend.h backend/shape/discs.cpp backend/shape/bezier.cpp backend/shape/fpmetric.h backend/shape/sor.h backend/shape/mesh.cpp backend/shape/spheres.cpp backend/shape/fractal.cpp backend/shape/fractal.h backend/shape/blob.cpp backend/shape/planes.h backend/shape/quadrics.cpp backend/shape/boxes.cpp backend/shape/polygon.h backend/shape/torus.cpp backend/shape/super.cpp backend/shape/hfield.cpp backend/shape/blob.h backend/shape/triangle.h backend/shape/isosurf.cpp backend/shape/poly.cpp backend/shape/discs.h backend/shape/boxes.h backend/shape/cones.cpp backend/shape/lathe.cpp backend/shape/sphsweep.h backend/shape/bezier.h backend/shape/mesh.h backend/shape/prism.cpp backend/shape/torus.h backend/shape/spheres.h backend/shape/planes.cpp backend/shape/csg.h backend/shape/polygon.cpp backend/shape/triangle.cpp backend/shape/super.h backend/shape/prism.h backend/shape/fpmetric.cpp backend/shape/csg.cpp backend/shape/sphsweep.cpp backend/shape/ovus.cpp backend/shape/quadrics.h backend/shape/lathe.h backend/shape/ovus.h backend/shape/sor.cpp backend/shape/truetype.h backend/shape/poly.h backend/shape/truetype.cpp backend/shape/isosurf.h backend/shape/cones.h backend/shape/hfield.h backend/parser/parstxtr.cpp backend/parser/reswords.cpp backend/parser/express.cpp backend/parser/reswords.h backend/parser/function.cpp backend/parser/parsestr.cpp backend/parser/tokenize.cpp backend/parser/fnsyntax.cpp backend/parser/parse.h backend/parser/parse.cpp backend/povray.h backend/frame.h backend/texture/avxfma4check.h backend/texture/normal.cpp backend/texture/pigment.cpp backend/texture/texture.h backend/texture/pigment.h backend/texture/normal.h backend/texture/texture.cpp backend/interior/media.cpp backend/interior/interior.cpp backend/interior/interior.h backend/interior/media.h backend/lighting/photonsortingtask.cpp backend/lighting/photonstrategytask.cpp backend/lighting/subsurface.cpp backend/lighting/radiosity.cpp backend/lighting/photonestimationtask.h backend/lighting/radiosity.h backend/lighting/photonshootingtask.cpp backend/lighting/point.h backend/lighting/photonshootingstrategy.cpp backend/lighting/photons.cpp backend/lighting/photonstrategytask.h backend/lighting/photonsortingtask.h backend/lighting/photons.h backend/lighting/subsurface.h backend/lighting/photonshootingstrategy.h backend/lighting/photonshootingtask.h backend/lighting/photonestimationtask.cpp backend/lighting/rad_data.cpp backend/lighting/point.cpp backend/vm/fnpovfpu.h backend/vm/fnpovfpu.cpp backend/vm/fnintern.cpp backend/vm/fnintern.h backend/vm/fncode.h backend/vm/fncode.cpp backend/control/messagefactory.h backend/control/renderbackend.h backend/control/messagefactory.cpp backend/control/benchmark.h backend/control/benchmark.cpp backend/control/renderbackend.cpp lightgrp.cpp precomp.cpp optout.cpp pov_mem.cpp lightgrp.h precomp.h pov_mem.h base/textstreambuffer.h base/textstream.h base/povms.h base/povms.cpp base/platformbase.h base/image/ppm.h base/image/colourspace.cpp base/image/pgm.h base/image/pgm.cpp base/image/openexr.cpp base/image/jpeg.cpp base/image/jpeg_pov.h base/image/image.h base/image/colourspace.h base/image/bmp.h base/image/iff.h base/image/encoding.cpp base/image/encoding.h base/image/targa.h base/image/tiff.cpp base/image/targa.cpp base/image/png_pov.h base/image/hdr.h base/image/png.cpp base/image/ppm.cpp base/image/gifdecod.cpp base/image/openexr.h base/image/image.cpp base/image/gif.cpp base/image/bmp.cpp base/image/iff.cpp base/image/hdr.cpp base/image/metadata.h base/image/tiff_pov.h base/image/gif.h base/timer.h base/fileinputoutput.h base/configbase.h base/types.h base/pointer.h base/path.h base/fileinputoutput.cpp base/textstreambuffer.cpp base/animation/animation.h base/animation/animation.cpp base/animation/moov.cpp base/animation/moov.h base/path.cpp base/timer.cpp base/povmscpp.h base/povmsgid.h base/povdebug.h base/processoptions.h base/textstream.cpp base/processoptions.cpp base/stringutilities.cpp base/stringutilities.h base/pov_err.h base/pov_err.cpp base/povmscpp.cpp povrayold.h frontend/processrenderoptions.cpp frontend/display.cpp frontend/simplefrontend.h frontend/imageprocessing.cpp frontend/parsermessagehandler.h frontend/rendermessagehandler.h frontend/console.cpp frontend/imageprocessing.h frontend/display.h frontend/filemessagehandler.cpp frontend/animationprocessing.h frontend/console.h frontend/renderfrontend.cpp frontend/parsermessagehandler.cpp frontend/shelloutprocessing.cpp frontend/defaultplatformbase.cpp frontend/imagemessagehandler.cpp frontend/animationprocessing.cpp frontend/defaultplatformbase.h frontend/configfrontend.h frontend/processrenderoptions.h frontend/renderfrontend.h frontend/shelloutprocessing.h frontend/imagemessagehandler.h frontend/filemessagehandler.h frontend/rendermessagehandler.cpp statspov.h

# Include paths for headers.
AM_CPPFLAGS = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/source/backend \
  -I$(top_srcdir)/source/base \
  -I$(top_srcdir)/source/frontend \
  -I$(top_srcdir)/unix \
  -I$(top_srcdir)/vfe \
  -I$(top_srcdir)/vfe/unix
