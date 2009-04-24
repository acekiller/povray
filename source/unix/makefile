#***************************************************************
#*
#*  UNIX Makefile for Persistence of Vision Raytracer v3.1e
#*
#*  Aug-95 : Created by Chris Cason.
#*  Nov-95 : Modified by Andreas Dilger to combine X and non-X.
#*  Mar-96 : Modified by Andreas Dilger to add SVGA
#*  Nov-98 : Modified by Mark Gordon to remove halo and add media and interior.
#*  Jan-99 : Modified by Mike Fleetwood to add John Cairns 3.1 changes
#*  Feb-99 : Modified by Mike Fleetwood to add a more flexible default 
#*           INI file search method 
#*  Mar-99 : More cleanup by Mark Gordon, including suggestions by Axel Hecht
#*  Mar-99 : Modified by Mark Gordon to add -ffast-math to Linux compiler flags 
#*
#***************************************************************

# set ODIR to the location where you want the object files placed.
ODIR=.

# set SRCDIR to the location where the main POV-Ray source is located.
SRCDIR=..

# UTARGET is the name of the text-only UNIX version
UTARGET=povray

# XTARGET is the name of the X-Windows executable.
XTARGET=x-povray

# STARGET is the name of the SVGA executable.
STARGET=s-povray

# libpng and zlib include and library directories.
#
# Use libXXX.a if you want to force static linking of the libraries.
#   Use -Llibdir -lXXX if you want to use shared libraries (if they are
#   available).  It doesn't matter if you only have libXXX.a libraries.
PNGDIR = $(SRCDIR)/libpng
LIBPNGINC = -I$(PNGDIR)
#LIBPNGLIB = $(PNGDIR)/libpng.a
LIBPNGLIB = -L$(PNGDIR) -lpng
#LIBPNGLIB = -L/usr/local/lib -lpng

# The following options are added at the suggestion of Axel Hecht
# Modify png_povDEP accordingly if you use this option
# PREFIX = $HOME
# LIBPNGINCDIR = $(PREFIX)/include
# LIBPNGINC = -I$(LIBPNGINCDIR)
# LIBPNGLIB = -L$(PREFIX)/lib -lpng

ZLIBDIR = $(SRCDIR)/zlib
ZLIBINC = -I$(ZLIBDIR)
#ZLIBLIB = $(ZLIBDIR)/libz.a
ZLIBLIB = -L$(ZLIBDIR) -lz
#ZLIBLIB = -L/usr/local/lib -lz

# The following options are added at the suggestion of Axel Hecht
# PREFIX = $HOME
# ZLIBINCDIR = $(PREFIX)/include
# ZLIBINC = -I$(LIBPNGINCDIR)
# ZLIBLIB = -L$(PREFIX)/lib -lpng

# Define the default POV-Ray library location
POVLIBDIR = /usr/local/lib/povray31

# This is the suffix for object files.
OBJ     = .o

# This is the command used to delete a file
RM    = /bin/rm -f

# The X11 variable should point to the top-level directory where the X11
#   libraries and include files are located.  It should really be set in
#   your system startup script (ie .cshrc or .profile) so that it can be
#   different for each platform without having to change the makefile.
# If you already have it set, comment all of these lines out.  Otherwise,
#   uncomment one of the following lines, or edit one to suit your system.
#X11 = /usr/local/X11R6

# for some SunOS systems:
#X11 = /usr/openwin        

# for some AIX systems:
#X11 = /usr/lpp/X11        

X11 = /usr/X11R6

# You probably won't need to change these if you have the X11 variable
#   above set properly...
XLIBINC = -I$(X11)/include
XLIBLIB = -L$(X11)/lib -lX11

# Libraries for the SVGA Linux version
SLIBLIB = -lvga -lvgagl

SRCINC  = -I. -I$(SRCDIR)

# The exact compiler options may depend on your compiler.  Feel free to
#   modify these as required.
#
# Most compilers are usually satisfied with the -ansi option, if they have
#   a choice between ANSI and non-ANSI compiling.  Use the ANSI mode if
#   possible.  If you can't get -ansi to work or something similar, you may
#   want to add -D__STDC__ to force ANSI prototypes instead of K&R prototypes.
#
# The option -DMEM_TRACE enables memory debugging, but shouldn't be needed
#   for compiles unless you are debugging new code, as it slows things down.
# The option -DPOV_COMMENTS enables informational comments in PNG and PPM
#   format output files (see png_pov.c and ppm.c for more info).
#
# The POVLIBDIR section was added by Mike Fleetwood.
# The -ffast-math flag was added to the Linux flags by Mark Gordon
#
#
# CFLAGS      = -O2 -ansi -c -DCOMPILER_VER=\".`uname`.$(CC)\" $(SRCINC) $(LIBPNGINC) $(ZLIBINC) $(XLIBINC)
LFLAGS      = $(LIBPNGLIB) $(ZLIBLIB) -lm

# Linux compiler flags
CFLAGS      = -O6 -finline-functions -ffast-math -c -ansi -DCOMPILER_VER=\".`uname`.$(CC)\" -DPOV_LIB_DIR=\"$(POVLIBDIR)\" $(SRCINC) $(LIBPNGINC) $(ZLIBINC)

# HPUX compiler flags
#CFLAGS      = +O2 -finline-functions -c -Aa -D_HPUX_SOURCE -DCOMPILER_VER=\".`uname`.$(CC)\" $(SRCINC) $(LIBPNGINC) $(ZLIBINC)

# Your compiler
CC          = gcc

# This decides which version of the executable you will get, either the
# Unix text-only version, the X-Windows version, or the SVGAlib version.
# default: config.h unix
#default: unix
default: xwin
#default: svga

#
# End of user specific options
#

POVOBJS = $(ODIR)/atmosph$(OBJ)  \
          $(ODIR)/bbox$(OBJ)     \
          $(ODIR)/bcyl$(OBJ)     \
          $(ODIR)/bezier$(OBJ)   \
          $(ODIR)/blob$(OBJ)     \
          $(ODIR)/boxes$(OBJ)    \
          $(ODIR)/bsphere$(OBJ)  \
          $(ODIR)/camera$(OBJ)   \
          $(ODIR)/chi2$(OBJ)     \
          $(ODIR)/colour$(OBJ)   \
          $(ODIR)/cones$(OBJ)    \
          $(ODIR)/csg$(OBJ)      \
          $(ODIR)/discs$(OBJ)    \
          $(ODIR)/express$(OBJ)  \
          $(ODIR)/fractal$(OBJ)  \
          $(ODIR)/gif$(OBJ)      \
          $(ODIR)/gifdecod$(OBJ) \
          $(ODIR)/hcmplx$(OBJ)   \
          $(ODIR)/hfield$(OBJ)   \
          $(ODIR)/iff$(OBJ)      \
          $(ODIR)/image$(OBJ)    \
          $(ODIR)/interior$(OBJ) \
          $(ODIR)/lathe$(OBJ)    \
          $(ODIR)/lbuffer$(OBJ)  \
          $(ODIR)/lighting$(OBJ) \
          $(ODIR)/matrices$(OBJ) \
	  $(ODIR)/media$(OBJ)    \
          $(ODIR)/mem$(OBJ)      \
          $(ODIR)/mesh$(OBJ)     \
          $(ODIR)/normal$(OBJ)   \
          $(ODIR)/objects$(OBJ)  \
          $(ODIR)/octree$(OBJ)   \
          $(ODIR)/optin$(OBJ)    \
          $(ODIR)/optout$(OBJ)   \
          $(ODIR)/parse$(OBJ)    \
          $(ODIR)/parstxtr$(OBJ) \
          $(ODIR)/pattern$(OBJ)  \
          $(ODIR)/pgm$(OBJ)      \
          $(ODIR)/pigment$(OBJ)  \
          $(ODIR)/planes$(OBJ)   \
          $(ODIR)/png_pov$(OBJ)  \
          $(ODIR)/point$(OBJ)    \
          $(ODIR)/poly$(OBJ)     \
          $(ODIR)/polygon$(OBJ)  \
          $(ODIR)/polysolv$(OBJ) \
          $(ODIR)/povray$(OBJ)   \
          $(ODIR)/ppm$(OBJ)      \
          $(ODIR)/prism$(OBJ)    \
          $(ODIR)/quadrics$(OBJ) \
          $(ODIR)/quatern$(OBJ)  \
          $(ODIR)/rad_data$(OBJ) \
          $(ODIR)/radiosit$(OBJ) \
          $(ODIR)/ray$(OBJ)      \
          $(ODIR)/render$(OBJ)   \
          $(ODIR)/sor$(OBJ)      \
          $(ODIR)/spheres$(OBJ)  \
          $(ODIR)/super$(OBJ)    \
          $(ODIR)/targa$(OBJ)    \
          $(ODIR)/texture$(OBJ)  \
          $(ODIR)/tokenize$(OBJ) \
          $(ODIR)/torus$(OBJ)    \
          $(ODIR)/triangle$(OBJ) \
          $(ODIR)/truetype$(OBJ) \
          $(ODIR)/txttest$(OBJ)  \
          $(ODIR)/userio$(OBJ)   \
          $(ODIR)/vbuffer$(OBJ)  \
          $(ODIR)/vlbuffer$(OBJ) \
          $(ODIR)/warps$(OBJ)    \
          $(MACHINE_OBJ)

#
# Specific module header dependencies for POV-Ray:
#

atmosphDEP              = $(SRCDIR)/atmosph.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
			  $(SRCDIR)/chi2.h     \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/lighting.h \
			  $(SRCDIR)/matrices.h \
			  $(SRCDIR)/media.h    \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
			  $(SRCDIR)/ray.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

bboxDEP                 = $(SRCDIR)/bbox.c     \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

bcylDEP                 = $(SRCDIR)/bcyl.c     \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/bcyl.h     \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/vector.h   \
                          config.h             \
                          $(SRCDIR)/povproto.h \

bezierDEP               = $(SRCDIR)/bezier.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/bezier.h   \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

blobDEP                 = $(SRCDIR)/blob.c     \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/blob.h     \
                          $(SRCDIR)/bsphere.h  \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

boxesDEP                = $(SRCDIR)/boxes.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

bsphereDEP              = $(SRCDIR)/bsphere.c  \
                          $(SRCDIR)/bsphere.h  \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/vector.h

cameraDEP               = $(SRCDIR)/camera.c   \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/vector.h

chi2DEP                 = $(SRCDIR)/chi2.c     \
                          $(SRCDIR)/chi2.h     \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/povproto.h

colourDEP               = $(SRCDIR)/colour.c   \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/warps.h

conesDEP                = $(SRCDIR)/cones.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/cones.h    \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

csgDEP                  = $(SRCDIR)/csg.c      \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/csg.h      \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hfield.h   \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/planes.h   \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/quadrics.h \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

discsDEP                = $(SRCDIR)/discs.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/discs.h    \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

expressDEP              = $(SRCDIR)/express.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/express.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/parstxtr.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

fractalDEP              = $(SRCDIR)/fractal.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/fractal.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hcmplx.h   \
                          $(SRCDIR)/quatern.h  \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

gifDEP                  = $(SRCDIR)/gif.c      \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/gif.h      \
                          $(SRCDIR)/gifdecod.h \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

gifdecodDEP             = $(SRCDIR)/gifdecod.c \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/gif.h      \
                          $(SRCDIR)/gifdecod.h \
                          $(SRCDIR)/povproto.h

mediaDEP                = $(SRCDIR)/media.c    \
			  config.h             \
			  $(SRCDIR)/atmosph.h  \
			  $(SRCDIR)/bbox.h     \
			  $(SRCDIR)/camera.h   \
                          $(SRCDIR)/chi2.h     \
			  $(SRCDIR)/colour.h   \
			  $(SRCDIR)/frame.h    \
			  $(SRCDIR)/interior.h \
			  $(SRCDIR)/lighting.h \
			  $(SRCDIR)/matrices.h \
			  $(SRCDIR)/media.h    \
			  $(SRCDIR)/mem.h      \
			  $(SRCDIR)/objects.h  \
			  $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h \
			  $(SRCDIR)/point.h    \
			  $(SRCDIR)/povproto.h \
			  $(SRCDIR)/povray.h   \
			  $(SRCDIR)/ray.h      \
			  $(SRCDIR)/render.h   \
			  $(SRCDIR)/texture.h  \
			  $(SRCDIR)/vector.h   \
			  $(SRCDIR)/vlbuffer.h \
			  $(SRCDIR)/warps.h
    
interiorDEP             = $(SRCDIR)/interior.c \
			  config.h             \
			  $(SRCDIR)/atmosph.h  \
			  $(SRCDIR)/bbox.h     \
			  $(SRCDIR)/camera.h   \
			  $(SRCDIR)/chi2.h     \
			  $(SRCDIR)/colour.h   \
			  $(SRCDIR)/frame.h    \
			  $(SRCDIR)/media.h    \
			  $(SRCDIR)/interior.h \
			  $(SRCDIR)/lighting.h \
			  $(SRCDIR)/matrices.h \
			  $(SRCDIR)/mem.h      \
			  $(SRCDIR)/objects.h  \
			  $(SRCDIR)/pattern.h  \
			  $(SRCDIR)/pigment.h  \
			  $(SRCDIR)/point.h    \
			  $(SRCDIR)/povproto.h \
			  $(SRCDIR)/povray.h   \
			  $(SRCDIR)/ray.h      \
			  $(SRCDIR)/render.h   \
			  $(SRCDIR)/texture.h  \
			  $(SRCDIR)/vector.h   \
			  $(SRCDIR)/vlbuffer.h \
			  $(SRCDIR)/warps.h  

hcmplxDEP               = $(SRCDIR)/hcmplx.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/fractal.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hcmplx.h   \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

hfieldDEP               = $(SRCDIR)/hfield.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hfield.h   \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

iffDEP                  = $(SRCDIR)/iff.c      \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/iff.h      \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

imageDEP                = $(SRCDIR)/image.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

quaternDEP              = $(SRCDIR)/quatern.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/fractal.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/quatern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

latheDEP                = $(SRCDIR)/lathe.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
			  $(SRCDIR)/bcyl.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/lathe.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/torus.h    \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

lbufferDEP              = $(SRCDIR)/lbuffer.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/lbuffer.h  \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/triangle.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

lightingDEP             = $(SRCDIR)/lighting.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/blob.h     \
                          $(SRCDIR)/bsphere.h  \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/interior.h \
                          $(SRCDIR)/lbuffer.h  \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/media.h    \
                          $(SRCDIR)/mesh.h     \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/ray.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

matricesDEP             = $(SRCDIR)/matrices.c \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/vector.h

memDEP                  = $(SRCDIR)/mem.c      \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/mem.h      \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/povproto.h

meshDEP                 = $(SRCDIR)/mesh.c     \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/mesh.h     \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

normalDEP               = $(SRCDIR)/normal.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/txttest.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

objectsDEP              = $(SRCDIR)/objects.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/interior.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

octreeDEP               = $(SRCDIR)/octree.c   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/vector.h

optinDEP                = $(SRCDIR)/optin.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/mem.h      \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/optin.h    \
                          $(SRCDIR)/optout.h   \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/png_pov.h  \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/userio.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

optoutDEP               = $(SRCDIR)/optout.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/bezier.h   \
                          $(SRCDIR)/blob.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/bsphere.h  \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/cones.h    \
                          config.h             \
                          $(SRCDIR)/csg.h      \
                          $(SRCDIR)/discs.h    \
                          $(SRCDIR)/fractal.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hfield.h   \
                          $(SRCDIR)/lathe.h    \
                          $(SRCDIR)/lbuffer.h  \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/mesh.h     \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/optin.h    \
                          $(SRCDIR)/optout.h   \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pgm.h      \
                          $(SRCDIR)/planes.h   \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/poly.h     \
                          $(SRCDIR)/polygon.h  \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/prism.h    \
                          $(SRCDIR)/quadrics.h \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/sor.h      \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/super.h    \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/torus.h    \
                          $(SRCDIR)/triangle.h \
                          $(SRCDIR)/truetype.h \
                          $(SRCDIR)/userio.h   \
                          $(SRCDIR)/vbuffer.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

parseDEP                = $(SRCDIR)/parse.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/bezier.h   \
                          $(SRCDIR)/blob.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/bsphere.h  \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/colour.h   \
                          $(SRCDIR)/cones.h    \
                          config.h             \
                          $(SRCDIR)/csg.h      \
                          $(SRCDIR)/discs.h    \
                          $(SRCDIR)/express.h  \
                          $(SRCDIR)/fractal.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/gif.h      \
                          $(SRCDIR)/hfield.h   \
                          $(SRCDIR)/iff.h      \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/interior.h \
                          $(SRCDIR)/lathe.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/mesh.h     \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/parstxtr.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pgm.h      \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/planes.h   \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/poly.h     \
                          $(SRCDIR)/polygon.h  \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/prism.h    \
                          $(SRCDIR)/quadrics.h \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/sor.h      \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/super.h    \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/torus.h    \
                          $(SRCDIR)/triangle.h \
                          $(SRCDIR)/truetype.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

parstxtrDEP             = $(SRCDIR)/parstxtr.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/express.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/gif.h      \
                          $(SRCDIR)/iff.h      \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/media.h    \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/parstxtr.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pgm.h      \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/png_pov.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

patternDEP              = $(SRCDIR)/pattern.c  \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/txttest.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/warps.h

pgmDEP                  = $(SRCDIR)/pgm.c      \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/pgm.h      \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

pigmentDEP              = $(SRCDIR)/pigment.c  \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/txttest.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/warps.h

planesDEP               = $(SRCDIR)/planes.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/planes.h   \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

png_povDEP              = $(SRCDIR)/png_pov.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/optout.h   \
                          $(PNGDIR)/png.h      \
                          $(SRCDIR)/png_pov.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

#png_povDEP              = $(SRCDIR)/png_pov.c  \
#                          $(SRCDIR)/atmosph.h  \
#                          $(SRCDIR)/bbox.h     \
#                          $(SRCDIR)/camera.h   \
#                          config.h             \
#                          $(SRCDIR)/frame.h    \
#                          $(SRCDIR)/optout.h   \
#                          $(LIBPNGINCDIR)/png.h\
#                          $(SRCDIR)/png_pov.h  \
#                          $(SRCDIR)/point.h    \
#                          $(SRCDIR)/povproto.h \
#                          $(SRCDIR)/povray.h   \
#                          $(SRCDIR)/render.h   \
#                          $(SRCDIR)/vlbuffer.h \
#                          $(SRCDIR)/warps.h

pointDEP                = $(SRCDIR)/point.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

polyDEP                 = $(SRCDIR)/poly.c     \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/poly.h     \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

polygonDEP              = $(SRCDIR)/polygon.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polygon.h  \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

polysolvDEP             = $(SRCDIR)/polysolv.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

povrayDEP               = $(SRCDIR)/povray.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/bezier.h   \
                          $(SRCDIR)/blob.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/bsphere.h  \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/cones.h    \
                          config.h             \
                          $(SRCDIR)/csg.h      \
                          $(SRCDIR)/discs.h    \
                          $(SRCDIR)/express.h  \
                          $(SRCDIR)/fractal.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hfield.h   \
                          $(SRCDIR)/lathe.h    \
                          $(SRCDIR)/lbuffer.h  \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/mem.h      \
                          $(SRCDIR)/mesh.h     \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/optin.h    \
                          $(SRCDIR)/optout.h   \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pgm.h      \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/planes.h   \
                          $(SRCDIR)/png_pov.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/poly.h     \
                          $(SRCDIR)/polygon.h  \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/prism.h    \
                          $(SRCDIR)/quadrics.h \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/sor.h      \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/super.h    \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/torus.h    \
                          $(SRCDIR)/triangle.h \
                          $(SRCDIR)/truetype.h \
                          $(SRCDIR)/userio.h   \
                          $(SRCDIR)/vbuffer.h  \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

ppmDEP                  = $(SRCDIR)/ppm.c      \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/optout.h   \
                          $(SRCDIR)/pgm.h      \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ppm.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

prismDEP                = $(SRCDIR)/prism.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/prism.h    \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

quadricsDEP             = $(SRCDIR)/quadrics.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/planes.h   \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/quadrics.h \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

rad_dataDEP             = $(SRCDIR)/rad_data.c \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/radiosit.h \
                          config.h

radiositDEP             = $(SRCDIR)/radiosit.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/optin.h    \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/ray.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

rayDEP                  = $(SRCDIR)/ray.c      \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/interior.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/ray.h      \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h

renderDEP               = $(SRCDIR)/render.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/chi2.h     \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/interior.h \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/octree.h   \
                          $(SRCDIR)/optout.h   \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/radiosit.h \
                          $(SRCDIR)/ray.h      \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/userio.h   \
                          $(SRCDIR)/vbuffer.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

sorDEP                  = $(SRCDIR)/sor.c      \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/sor.h      \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

spheresDEP              = $(SRCDIR)/spheres.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/spheres.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

superDEP                = $(SRCDIR)/super.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/super.h    \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

svgaDEP                 = svga.c               \
                          config.h             \
                          unixconf.h           \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h

targaDEP                = $(SRCDIR)/targa.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/targa.h    \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

textureDEP              = $(SRCDIR)/texture.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/image.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/normal.h   \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/pigment.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

tokenizeDEP             = $(SRCDIR)/tokenize.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          $(SRCDIR)/colour.h   \
                          config.h             \
                          $(SRCDIR)/express.h  \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

torusDEP                = $(SRCDIR)/torus.c    \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/polysolv.h \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/torus.h    \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

triangleDEP             = $(SRCDIR)/triangle.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/triangle.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

truetypeDEP             = $(SRCDIR)/truetype.c \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/csg.h      \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/truetype.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

txttestDEP              = $(SRCDIR)/txttest.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/txttest.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

unixDEP                 = unix.c               \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/povproto.h \
			  $(SRCDIR)/povray.h   \
			  $(SRCDIR)/optin.h

userioDEP               = $(SRCDIR)/userio.c   \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/parse.h    \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/tokenize.h \
                          $(SRCDIR)/userio.h   \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

vbufferDEP              = $(SRCDIR)/vbuffer.c  \
                          $(SRCDIR)/atmosph.h  \
                          $(SRCDIR)/bbox.h     \
                          $(SRCDIR)/boxes.h    \
                          $(SRCDIR)/camera.h   \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/hfield.h   \
                          $(SRCDIR)/lighting.h \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/objects.h  \
                          $(SRCDIR)/point.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/povray.h   \
                          $(SRCDIR)/render.h   \
                          $(SRCDIR)/triangle.h \
                          $(SRCDIR)/userio.h   \
                          $(SRCDIR)/vbuffer.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h \
                          $(SRCDIR)/warps.h

vlbufferDEP             = $(SRCDIR)/vlbuffer.c \
                          $(SRCDIR)/bbox.h     \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/vlbuffer.h

warpsDEP                = $(SRCDIR)/warps.c    \
                          config.h             \
                          $(SRCDIR)/frame.h    \
                          $(SRCDIR)/matrices.h \
                          $(SRCDIR)/pattern.h  \
                          $(SRCDIR)/povproto.h \
                          $(SRCDIR)/texture.h  \
                          $(SRCDIR)/vector.h   \
                          $(SRCDIR)/warps.h

xwindowsDEP             = xwindows.c           \
                          config.h             \
                          unixconf.h           \
                          $(SRCDIR)/frame.h    \
                          xpovicon.xbm         \
                          xpovmask.xbm         \
                          $(SRCDIR)/povproto.h

config.h:
	@echo
	@echo 'You need to select which executable version you want'
	@echo 'to make (xwin, svga, or unix), and then use the'
	@echo 'appropriate make command to create the config.h file.'
	@echo 'Use "make newunix" to make the text-only version.'
	@echo 'Use "make newxwin" to make the X Windows version.'
	@echo 'Use "make newsvga" to make the SVGAlib version.'
	@echo
	@echo 'If you want to build the X-Windows version, you should make'
	@echo 'sure that the X11 variable is set correctly.  It is currently'
	@echo 'set to '$(X11)'.  By default this is set at the top of the'
	@echo 'makefile, but can also be set in your shell startup script.'
	@echo
	@echo 'Both the X Windows and SVGAlib versions also use the'
	@echo 'unixconf.h file, so it must still be present when'
	@echo 'compiling the display capable versions of POV-Ray.'
	@echo
	@false

unix:	$(UTARGET)
	@echo

$(UTARGET):	$(POVOBJS) $(ODIR)/unix$(OBJ)
	$(CC) $(POVOBJS) $(ODIR)/unix$(OBJ) $(LFLAGS) -o $(UTARGET)

svga:	$(STARGET)
	@echo

$(STARGET):	$(POVOBJS) $(ODIR)/svga$(OBJ) $(ODIR)/unix$(OBJ)
	$(CC) $(POVOBJS) $(ODIR)/svga$(OBJ) $(ODIR)/unix$(OBJ) $(LFLAGS) $(SLIBLIB) -o $(STARGET)

xwin:	$(XTARGET)
	@echo

$(XTARGET):	$(POVOBJS) $(ODIR)/xwindows$(OBJ) $(ODIR)/unix$(OBJ)
	$(CC) $(POVOBJS) $(ODIR)/xwindows$(OBJ) $(ODIR)/unix$(OBJ) $(LFLAGS) $(XLIBLIB) -o $(XTARGET)

# Only these files need to be rebuilt if the display type was changed
DISPOBJS = $(ODIR)/povray$(OBJ) $(ODIR)/render$(OBJ) \
           $(ODIR)/userio$(OBJ) $(ODIR)/vbuffer$(OBJ)

newunix:
	-@$(RM) $(DISPOBJS)
	-@cp unixconf.h config.h
	-@touch $(ODIR)/*$(OBJ)
	-@$(MAKE) unix

newsvga:
	-@$(RM) $(DISPOBJS)
	-@cp svgaconf.h config.h
	-@touch $(ODIR)/*$(OBJ)
	-@$(MAKE) svga

newxwin:
	-@$(RM) $(DISPOBJS)
	-@cp xwinconf.h config.h
	-@touch $(ODIR)/*$(OBJ)
	-@$(MAKE) xwin

clean:
	-@$(RM) $(POVOBJS) $(ODIR)/unix$(OBJ) $(ODIR)/xwindows$(OBJ)
	-@$(RM) $(ODIR)/svga$(OBJ) $(UTARGET) $(XTARGET) $(STARGET)

$(ODIR)/atmosph$(OBJ)  : $(atmosphDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/atmosph.c -o $@

$(ODIR)/bbox$(OBJ)     : $(bboxDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/bbox.c -o $@

$(ODIR)/bcyl$(OBJ)     : $(bcylDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/bcyl.c -o $@

$(ODIR)/bezier$(OBJ)   : $(bezierDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/bezier.c -o $@

$(ODIR)/blob$(OBJ)     : $(blobDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/blob.c -o $@

$(ODIR)/boxes$(OBJ)    : $(boxesDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/boxes.c -o $@

$(ODIR)/bsphere$(OBJ)  : $(bsphereDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/bsphere.c -o $@

$(ODIR)/camera$(OBJ)   : $(cameraDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/camera.c -o $@

$(ODIR)/chi2$(OBJ)     : $(chi2DEP)
	$(CC) $(CFLAGS) $(SRCDIR)/chi2.c -o $@

$(ODIR)/colour$(OBJ)   : $(colourDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/colour.c -o $@

$(ODIR)/cones$(OBJ)    : $(conesDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/cones.c -o $@

$(ODIR)/csg$(OBJ)      : $(csgDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/csg.c -o $@

$(ODIR)/discs$(OBJ)    : $(discsDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/discs.c -o $@

$(ODIR)/express$(OBJ)  : $(expressDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/express.c -o $@

$(ODIR)/fractal$(OBJ)  : $(fractalDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/fractal.c -o $@

$(ODIR)/gif$(OBJ)      : $(gifDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/gif.c -o $@

$(ODIR)/gifdecod$(OBJ) : $(gifdecodDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/gifdecod.c -o $@

$(ODIR)/hcmplx$(OBJ)   : $(hcmplxDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/hcmplx.c -o $@

$(ODIR)/hfield$(OBJ)   : $(hfieldDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/hfield.c -o $@

$(ODIR)/iff$(OBJ)      : $(iffDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/iff.c -o $@

$(ODIR)/image$(OBJ)    : $(imageDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/image.c -o $@

$(ODIR)/interior$(OBJ) : $(interiorDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/interior.c -o $@

$(ODIR)/lathe$(OBJ)    : $(latheDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/lathe.c -o $@

$(ODIR)/lbuffer$(OBJ)  : $(lbufferDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/lbuffer.c -o $@

$(ODIR)/lighting$(OBJ) : $(lightingDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/lighting.c -o $@

$(ODIR)/matrices$(OBJ) : $(matricesDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/matrices.c -o $@

$(ODIR)/media$(OBJ)    : $(mediaDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/media.c -o $@

$(ODIR)/mem$(OBJ)      : $(memDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/mem.c -o $@

$(ODIR)/mesh$(OBJ)     : $(meshDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/mesh.c -o $@

$(ODIR)/normal$(OBJ)   : $(normalDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/normal.c -o $@

$(ODIR)/objects$(OBJ)  : $(objectsDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/objects.c -o $@

$(ODIR)/octree$(OBJ)   : $(octreeDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/octree.c -o $@

$(ODIR)/optin$(OBJ)    : $(optinDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/optin.c -o $@

$(ODIR)/optout$(OBJ)   : $(optoutDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/optout.c -o $@

$(ODIR)/parse$(OBJ)    : $(parseDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/parse.c -o $@

$(ODIR)/parstxtr$(OBJ) : $(parstxtrDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/parstxtr.c -o $@

$(ODIR)/pattern$(OBJ)  : $(patternDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/pattern.c -o $@

$(ODIR)/pgm$(OBJ)      : $(pgmDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/pgm.c -o $@

$(ODIR)/pigment$(OBJ)  : $(pigmentDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/pigment.c -o $@

$(ODIR)/planes$(OBJ)   : $(planesDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/planes.c -o $@

$(ODIR)/point$(OBJ)    : $(pointDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/point.c -o $@

$(ODIR)/poly$(OBJ)     : $(polyDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/poly.c -o $@

$(ODIR)/polygon$(OBJ)  : $(polygonDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/polygon.c -o $@

$(ODIR)/polysolv$(OBJ) : $(polysolvDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/polysolv.c -o $@

$(ODIR)/povray$(OBJ)   : $(povrayDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/povray.c -o $@

$(ODIR)/png_pov$(OBJ)  : $(png_povDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/png_pov.c -o $@

$(ODIR)/ppm$(OBJ)      : $(ppmDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/ppm.c -o $@

$(ODIR)/prism$(OBJ)    : $(prismDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/prism.c -o $@

$(ODIR)/quadrics$(OBJ) : $(quadricsDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/quadrics.c -o $@

$(ODIR)/quatern$(OBJ)  : $(quaternDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/quatern.c -o $@

$(ODIR)/rad_data$(OBJ) : $(rad_dataDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/rad_data.c -o $@

$(ODIR)/radiosit$(OBJ) : $(radiositDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/radiosit.c -o $@

$(ODIR)/ray$(OBJ)      : $(rayDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/ray.c -o $@

$(ODIR)/render$(OBJ)   : $(renderDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/render.c -o $@

$(ODIR)/sor$(OBJ)      : $(sorDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/sor.c -o $@

$(ODIR)/spheres$(OBJ)  : $(spheresDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/spheres.c -o $@

$(ODIR)/super$(OBJ)    : $(superDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/super.c -o $@

$(ODIR)/svga$(OBJ) : $(svgaDEP)
	$(CC) $(CFLAGS) svga.c -o $@

$(ODIR)/targa$(OBJ)    : $(targaDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/targa.c -o $@

$(ODIR)/texture$(OBJ)  : $(textureDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/texture.c -o $@

$(ODIR)/tokenize$(OBJ) : $(tokenizeDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/tokenize.c -o $@

$(ODIR)/torus$(OBJ)    : $(torusDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/torus.c -o $@

$(ODIR)/triangle$(OBJ) : $(triangleDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/triangle.c -o $@

$(ODIR)/truetype$(OBJ) : $(truetypeDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/truetype.c -o $@

$(ODIR)/txttest$(OBJ)  : $(txttestDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/txttest.c -o $@

$(ODIR)/unix$(OBJ)     : $(unixDEP)
	$(CC) $(CFLAGS) unix.c -o $@

$(ODIR)/userio$(OBJ)   : $(userioDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/userio.c -o $@

$(ODIR)/vbuffer$(OBJ)  : $(vbufferDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/vbuffer.c -o $@

$(ODIR)/vlbuffer$(OBJ) : $(vlbufferDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/vlbuffer.c -o $@

$(ODIR)/warps$(OBJ)    : $(warpsDEP)
	$(CC) $(CFLAGS) $(SRCDIR)/warps.c -o $@

$(ODIR)/xwindows$(OBJ) : $(xwindowsDEP)
	$(CC) $(CFLAGS) $(XLIBINC) xwindows.c -o $@
















