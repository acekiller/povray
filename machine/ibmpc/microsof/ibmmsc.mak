# IBM Makefile for Persistence of Vision Raytracer
# This file is released to the public domain.
# For use with Microsoft C 6.00A (or 6.00AX) on the IBM-PC

# Note for the IBM Version:
# Uses system environment variable LIB for the linker's .LIB file path.
# (Example:  Set LIB=C:\LIB)  if you change this the two .LNK files will also
# need to be changed as well.  The system environment variable CMODEL also
# may be defined for the memory model of the compiler.  POV-Ray requires
# the "Large" memory model.  (Example Set CMODEL=L)  If you don't want to
# use the DOS environment variables, uncomment the following two lines:

#LIB		=\lib
CMODEL  	=L

# If you have MSC 6.00AX, and Extended/XMS/VCPI memory to compile in,
# uncomment the following line:

#EXTMEM		=/EM				# Uncomment this line for MSC 6.00AX

CC      	=cl
OBJ     	=obj
MACHINE_OBJ	= ibm.$(OBJ)

LINKER  =link /EXEPACK /PACKC /FARC /STACK:12288

# Uncomment for 8086/8088 instruction set usage
#
#CFLAGS =/A$(CMODEL) $(EXTMEM) /Gmsr /FPi /H32 /c /J
#
# Uncomment for 80186/80268 (incl. V20) instruction set usage
# Normally, you'll use this:
#
CFLAGS  =/A$(CMODEL) $(EXTMEM) /Gmsr2 /FPi87 /H32 /c /J /W3
#
# Or, try this quicker version:
#
#CFLAGS =/qc /A$(CMODEL) $(EXTMEM) /FPi87 /c /J /Gsr2
#
# If you have OS/2 to compile under, fix directory name to reflect
# your configuration, and add this option to CFLAGS above...
#
# /B2 C:\msc\c2l.exe
#
# (Note this is superseded by /EM for MSC 6.00AX under DOS)

# Override MS's NMAKE implicit rules for making an .OBJ file from a
# .C file.
#
.c.obj :
	$(CC) $(CFLAGS) /Ogiltaz $*.c

# The optimization option:
#
# /Oaxz
#
# Is purported by Microsoft to produce the fastest possible code.  In fact it
# will break the RGB->HSV->RGB routines in IBM.C, if using the 8087 emulator,
# and, it appears to break height fields and boxes.  So, we are using:
#
# /Ogiltaz
#
# This optimization string works for IBM.C and should then have worked for ALL
# files, but, somehow, it used to BREAK the compiler on certain modules! (Internal
# compiler error C1001 on the old TEXTURE.C, etc.)  Oh, well...  However, with
# the current IBM MSC 6.00A, and after TEXTURE.C got broken up into several sub-
# files, the /Ogiltaz optimization switch seems to work for everything.  Great.
# Try using /Od to disable optimization completely if you're having any strange
# problems, for isolation purposes.
#

POVOBJS = povray.$(OBJ) render.$(OBJ) tokenize.$(OBJ) parse.$(OBJ) \
	  objects.$(OBJ) spheres.$(OBJ) quadrics.$(OBJ) lighting.$(OBJ) \
	  express.$(OBJ) texture.$(OBJ) matrices.$(OBJ) csg.$(OBJ) bound.$(OBJ) \
	  hfield.$(OBJ) txttest.$(OBJ) colour.$(OBJ) ray.$(OBJ) point.$(OBJ)\
	  planes.$(OBJ) iff.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ) blob.$(OBJ)\
	  triangle.$(OBJ) raw.$(OBJ) dump.$(OBJ) targa.$(OBJ) poly.$(OBJ) \
	  bezier.$(OBJ) vect.$(OBJ) boxes.$(OBJ) camera.$(OBJ) cones.$(OBJ) \
	  discs.$(OBJ) image.$(OBJ) normal.$(OBJ) pigment.$(OBJ) $(MACHINE_OBJ)

# POV-Ray - Specific Dependencies
#
povray.exe : $(POVOBJS)
	$(LINKER) @ibmmsc.lnk
#
# This version uses MS-Link's overlay feature to share code space for the
# parser and GIF-decoder modules, etc.  Try at your own risk!
#
#povray.exe : $(POVOBJS)
#	$(LINKER) @ibmovl.lnk

povray.$(OBJ) : povray.c povproto.h frame.h vector.h config.h

tokenize.$(OBJ) : tokenize.c povproto.h frame.h config.h

parse.$(OBJ) : parse.c povproto.h frame.h config.h

render.$(OBJ) : render.c povproto.h frame.h vector.h config.h

image.$(OBJ) : image.c povproto.h frame.h vector.h config.h texture.h

lighting.$(OBJ) : lighting.c povproto.h frame.h vector.h config.h

camera.$(OBJ) : camera.c povproto.h frame.h vector.h config.h

express.$(OBJ) : express.c povproto.h frame.h config.h parse.h

pigment.$(OBJ) : pigment.c povproto.h frame.h vector.h config.h texture.h

texture.$(OBJ) : texture.c povproto.h frame.h vector.h config.h texture.h

txttest.$(OBJ) : txttest.c povproto.h frame.h vector.h config.h texture.h

objects.$(OBJ) : objects.c povproto.h frame.h vector.h config.h

hfield.$(OBJ) : hfield.c povproto.h frame.h vector.h config.h

spheres.$(OBJ) : spheres.c povproto.h frame.h vector.h config.h

point.$(OBJ) : point.c povproto.h frame.h vector.h config.h

planes.$(OBJ) : planes.c povproto.h frame.h vector.h config.h

boxes.$(OBJ) : boxes.c povproto.h frame.h vector.h config.h

cones.$(OBJ) : cones.c povproto.h frame.h vector.h config.h

discs.$(OBJ) : discs.c povproto.h frame.h vector.h config.h

bound.$(OBJ) : bound.c povproto.h frame.h vector.h config.h

blob.$(OBJ) : blob.c povproto.h frame.h vector.h config.h

quadrics.$(OBJ) : quadrics.c povproto.h frame.h vector.h config.h

poly.$(OBJ) : poly.c povproto.h frame.h vector.h config.h

bezier.$(OBJ) : bezier.c povproto.h frame.h vector.h config.h

vect.$(OBJ) : vect.c povproto.h frame.h config.h

matrices.$(OBJ) : matrices.c povproto.h frame.h vector.h config.h

normal.$(OBJ) : normal.c povproto.h frame.h vector.h config.h texture.h

csg.$(OBJ) : csg.c povproto.h frame.h vector.h config.h

colour.$(OBJ) : colour.c povproto.h frame.h config.h

ray.$(OBJ) : ray.c povproto.h frame.h vector.h config.h

iff.$(OBJ) : iff.c povproto.h frame.h config.h

gif.$(OBJ) : gif.c povproto.h frame.h config.h

gifdecod.$(OBJ) : gifdecod.c povproto.h frame.h config.h

raw.$(OBJ) : raw.c povproto.h frame.h config.h

dump.$(OBJ) : dump.c povproto.h frame.h config.h

targa.$(OBJ) : targa.c povproto.h frame.h config.h

triangle.$(OBJ) : triangle.c povproto.h frame.h vector.h config.h

ibm.$(OBJ) : ibm.c povproto.h frame.h config.h
	$(CC) $(CFLAGS) /Ogiltaz /D__STDC__ $*.c
