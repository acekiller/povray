# IBM Makefile for Persistence of Vision Raytracer 
# This file is released to the public domain.
# For use with the ZORTECH C/C++ protected mode compiler
# Note for the IBM Version:
# Uses system environment variable LIB for the linker's .LIB file path.
# (Example:  Set LIB=C:\LIB)  if you change this the two .LNK files will also
# need to be changed as well.  The system environment variable CMODEL also
# may be defined for the memory model of the compiler.  PV-Ray requires
# the "Large" memory model.  (Example Set CMODEL=l)  If you don't want to
# use the DOS environment variables, uncomment the following two lines:
#
#
# MAKE Macros and Such...
#

CMODEL  =x
LIB     =d:\zt\lib
CC      =ztc -c
OBJ     =obj
MACHINE_OBJ = ibm.$(OBJ) ztc386.$(OBJ)

LINKER  =blinkx /CO/CH/NOE/MAP

# Uncomment for 80268 (ZPM DOS extender) instruction set usage
# Normally, you'll use this:
#
CFLAGS  =-m$(CMODEL) -o+all -o+loop -bx -f -a2


.c.obj :
	$(CC) $(CFLAGS) $*.c


F1 = povray.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) boxes.$(OBJ) 
F2 = camera.$(OBJ) colour.$(OBJ) cones.$(OBJ) csg.$(OBJ) discs.$(OBJ) 
F3 = dump.$(OBJ) express.$(OBJ) gifdecod.$(OBJ) gif.$(OBJ) hfield.$(OBJ) 
F4 = iff.$(OBJ) image.$(OBJ) lighting.$(OBJ) matrices.$(OBJ) normal.$(OBJ) 
F5 = objects.$(OBJ) parse.$(OBJ) pigment.$(OBJ) planes.$(OBJ) point.$(OBJ)
F6 = poly.$(OBJ) quadrics.$(OBJ) raw.$(OBJ) ray.$(OBJ) render.$(OBJ) 
F7 = spheres.$(OBJ) targa.$(OBJ) texture.$(OBJ) tokenize.$(OBJ) 
F8 = triangle.$(OBJ) txttest.$(OBJ) vect.$(OBJ) $(MACHINE_OBJ)

PVOBJS = $(F1) $(F2) $(F3) $(F4) $(F5) $(F6) $(F7) $(F8) 

# POV-Ray Specific Dependencies
#
povray.exe : $(PVOBJS)
	$(LINKER) @ibmzor3.lnk

#
# Specific module/header dependencies for PV-Ray:
#
 
povray.$(OBJ) : povray.c povproto.h frame.h vector.h config.h

bezier.$(OBJ) : bezier.c povproto.h frame.h vector.h config.h

blob.$(OBJ) : blob.c povproto.h frame.h config.h

bound.$(OBJ) : bound.c povproto.h frame.h vector.h config.h

boxes.$(OBJ) : boxes.c povproto.h frame.h config.h

camera.$(OBJ) : camera.c povproto.h frame.h vector.h config.h

colour.$(OBJ) : colour.c povproto.h frame.h config.h

cones.$(OBJ) : cones.c povproto.h frame.h vector.h config.h

csg.$(OBJ) : csg.c povproto.h frame.h vector.h config.h

discs.$(OBJ) : discs.c povproto.h frame.h vector.h config.h

express.$(OBJ) : express.c povproto.h frame.h config.h parse.h

gifdecod.$(OBJ) : gifdecod.c povproto.h frame.h config.h

gif.$(OBJ) : gif.c povproto.h frame.h config.h

hfield.$(OBJ) : hfield.c povproto.h frame.h vector.h config.h

ibm.$(OBJ) :    ibm.c povproto.h frame.h config.h

iff.$(OBJ) : iff.c povproto.h frame.h config.h

image.$(OBJ) : image.c povproto.h frame.h vector.h config.h texture.h

lighting.$(OBJ) : lighting.c povproto.h frame.h vector.h config.h

matrices.$(OBJ) : matrices.c povproto.h frame.h vector.h config.h

normal.$(OBJ) : normal.c povproto.h frame.h vector.h config.h texture.h

objects.$(OBJ) : objects.c povproto.h frame.h vector.h config.h

parse.$(OBJ) : parse.c povproto.h frame.h config.h parse.h

pigment.$(OBJ) : pigment.c povproto.h frame.h vector.h config.h texture.h

planes.$(OBJ) : planes.c povproto.h frame.h vector.h config.h

poly.$(OBJ) : poly.c povproto.h frame.h vector.h config.h

quadrics.$(OBJ) : quadrics.c povproto.h frame.h vector.h config.h

raw.$(OBJ) :    raw.c povproto.h frame.h config.h

ray.$(OBJ) : ray.c povproto.h frame.h vector.h config.h

render.$(OBJ) : render.c povproto.h frame.h vector.h config.h

spheres.$(OBJ) : spheres.c povproto.h frame.h vector.h config.h

texture.$(OBJ) : texture.c povproto.h frame.h vector.h config.h texture.h

tokenize.$(OBJ) : tokenize.c povproto.h frame.h config.h

triangle.$(OBJ) : triangle.c povproto.h frame.h vector.h config.h

txttest.$(OBJ) : txttest.c povproto.h frame.h vector.h config.h texture.h

vect.$(OBJ) : vect.c povproto.h frame.h config.h

ztc386.$(OBJ) : ztc386.asm
	masm -ml ztc386;

