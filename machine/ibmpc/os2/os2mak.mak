# IBM Makefile for Persistence of Vision Raytracer Versio 2.0
# Developed with & for IBM CSet/2 C++ under OS/2 2.1
# These make & link files will create a text only compile of POV-Ray
# Albert Shan/Bill Pulver - 9/93   

CC              = icc
OBJ             = obj
MACHINE_OBJ     = os2.$(OBJ)

LINKER  = link386 /PM:VIO /EXEPACK /STACK:90000

# Use or dont use optimization, depending on the CSD level of your
# copy of IBM's compiler. Most files will optimize @ CSD 32 or higher
# for CSet/2 1.0. The alternate CFLAGS may also be used.
# CSet/2 C++ works "out of the box". This file is setup for optimization
# as is.

CFLAGS  = /Sm /Ss+ /Gm+ /Gs+ /Gf+ /C+ /Q+ /O+
#CFLAGS  = /Sm /Ss+ /Gm+ /C+ /Q+ /O-

# Override MS's NMAKE implicit rules for making an .OBJ file from a
# .C file.
#
.c.obj :
 $(CC) $(CFLAGS) $*.c

POVOBJS = povray.$(OBJ) render.$(OBJ) tokenize.$(OBJ) \
          objects.$(OBJ) spheres.$(OBJ) quadrics.$(OBJ) lighting.$(OBJ) \
          express.$(OBJ) matrices.$(OBJ) csg.$(OBJ) \
          hfield.$(OBJ) image.$(OBJ) camera.$(OBJ) discs.$(OBJ) bound.$(OBJ) \
          txttest.$(OBJ) colour.$(OBJ) normal.$(OBJ) ray.$(OBJ) point.$(OBJ)\
          planes.$(OBJ) iff.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ) blob.$(OBJ)\
          triangle.$(OBJ) raw.$(OBJ) dump.$(OBJ) poly.$(OBJ) \
          bezier.$(OBJ) vect.$(OBJ) boxes.$(OBJ) pigment.$(OBJ) \
          cones.$(OBJ) $(MACHINE_OBJ) parse.$(OBJ) texture.$(OBJ) targa.$(OBJ)

# POV-Ray - Specific Dependencies
#
povray: povray.exe 
  @echo.
  @echo ********* D O N E !!!!! **********


povray.exe : $(POVOBJS)
             $(LINKER) @ibmcs2.lnk

# /O- turns optimization off because CSet/2 produces erranous code
# for those source files. Depending on the CSD level of your compiler
# you may want to expirement.
#

povray.$(OBJ) : povray.c povproto.h frame.h vector.h config.h
        $(CC) $(CFLAGS) /DNOPRNBUF /O+ $*.c

tokenize.$(OBJ) : tokenize.c povproto.h frame.h config.h

parse.$(OBJ) : parse.c povproto.h frame.h config.h
#        $(CC) $(CFLAGS) /O- $*.c

render.$(OBJ) : render.c povproto.h frame.h vector.h config.h

lighting.$(OBJ) : lighting.c povproto.h frame.h vector.h config.h

texture.$(OBJ) : texture.c povproto.h frame.h vector.h config.h texture.h
#        $(CC) $(CFLAGS) /O- $*.c

txttest.$(OBJ) : txttest.c povproto.h frame.h vector.h config.h texture.h

objects.$(OBJ) : objects.c povproto.h frame.h vector.h config.h

hfield.$(OBJ) : hfield.c povproto.h frame.h vector.h config.h
	$(CC) $(CFLAGS) /O- $*.c

spheres.$(OBJ) : spheres.c povproto.h frame.h vector.h config.h

point.$(OBJ) : point.c povproto.h frame.h vector.h config.h

planes.$(OBJ) : planes.c povproto.h frame.h vector.h config.h

boxes.$(OBJ) : boxes.c povproto.h frame.h vector.h config.h

blob.$(OBJ) : blob.c povproto.h frame.h vector.h config.h

quadrics.$(OBJ) : quadrics.c povproto.h frame.h vector.h config.h

poly.$(OBJ) : poly.c povproto.h frame.h vector.h config.h
#        $(CC) $(CFLAGS) /O- $*.c

bezier.$(OBJ) : bezier.c povproto.h frame.h vector.h config.h

vect.$(OBJ) : vect.c povproto.h frame.h config.h

matrices.$(OBJ) : matrices.c povproto.h frame.h vector.h config.h

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

express.$(OBJ) : express.c frame.h vector.h povproto.h parse.h

image.$(OBJ) : image.c frame.h vector.h povproto.h texture.h

camera.$(OBJ) : camera.c frame.h vector.h povproto.h

bound.$(OBJ) : bound.c frame.h vector.h povproto.h

normal.$(OBJ) : normal.c frame.h vector.h povproto.h texture.h

discs.$(OBJ) : discs.c frame.h vector.h povproto.h

pigment$(OBJ) : pigment.c frame.h vector.h povproto.h texture.h

cones$(OBJ) : cones.c frame.h vector.h povproto.h

