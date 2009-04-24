# Makefile for Persistence of Vision Raytracer 
# This file is released to the public domain.
#
#
# MAKE Macros and Such...
#

#***************************************************************
#*
#*                      UNIX Makefile
#*
#***************************************************************

# The exact options may depend on your compiler.  Feel free to modify
# these as required.

# Added for colorx addition.  You _do_ have $X11 defined by your .cshrc,
# don't you?
X11 = /usr
CFLAGS=		-c -O 
LFLAGS =	-O  $(X11)/lib/libXt.a $(X11)/lib/libXext.a -lX11 \
                -o povray
CC =		cc

OBJ	= o
MACHINE_OBJ	= xwindows.$(OBJ)

# Make's implicit rules for making a .o file from a .c file...
#
.c.o :
	$(CC) $(CFLAGS) $*.c


POVOBJS = povray.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) boxes.$(OBJ)  \
	  camera.$(OBJ) colour.$(OBJ) cones.$(OBJ) csg.$(OBJ) discs.$(OBJ)   \
	  dump.$(OBJ) express.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ)              \
	  hfield.$(OBJ) iff.$(OBJ) image.$(OBJ) lighting.$(OBJ)              \
	  matrices.$(OBJ) normal.$(OBJ) objects.$(OBJ) parse.$(OBJ)          \
	  pigment.$(OBJ) planes.$(OBJ) point.$(OBJ) poly.$(OBJ)              \
	  quadrics.$(OBJ) raw.$(OBJ) ray.$(OBJ) render.$(OBJ) spheres.$(OBJ) \
	  targa.$(OBJ) texture.$(OBJ) tokenize.$(OBJ) triangle.$(OBJ)        \
	  txttest.$(OBJ) vect.$(OBJ) $(MACHINE_OBJ)

povray:	$(POVOBJS)
	cc $(POVOBJS) -lm $(LFLAGS)

povray.$(OBJ) : povray.c povproto.h frame.h vector.h config.h

xwindows.$(OBJ): xwindows.c xpov.icn frame.h povproto.h config.h
	$(CC) $(CFLAGS) -I$(X11)/include xwindows.c

bezier.$(OBJ): config.h frame.h povproto.h vector.h

blob.$(OBJ): config.h frame.h povproto.h vector.h

bound.$(OBJ): config.h frame.h povproto.h vector.h

boxes.$(OBJ): config.h frame.h povproto.h vector.h

camera.$(OBJ): config.h frame.h povproto.h vector.h

colour.$(OBJ): config.h frame.h povproto.h vector.h

cones.$(OBJ): config.h frame.h povproto.h vector.h

csg.$(OBJ): config.h frame.h povproto.h vector.h

discs.$(OBJ): config.h frame.h povproto.h vector.h

dump.$(OBJ): config.h frame.h povproto.h

express.$(OBJ): config.h frame.h parse.h povproto.h vector.h

gif.$(OBJ): config.h frame.h povproto.h

gifdecod.$(OBJ): config.h frame.h povproto.h

hfield.$(OBJ): config.h frame.h povproto.h vector.h

iff.$(OBJ): config.h frame.h povproto.h

image.$(OBJ): config.h frame.h povproto.h texture.h vector.h

lighting.$(OBJ): config.h frame.h povproto.h vector.h

matrices.$(OBJ): config.h frame.h povproto.h vector.h

normal.$(OBJ): config.h frame.h povproto.h texture.h vector.h

objects.$(OBJ): config.h frame.h povproto.h vector.h

parse.$(OBJ): config.h frame.h parse.h povproto.h vector.h

pigment.$(OBJ): config.h frame.h povproto.h texture.h vector.h

planes.$(OBJ): config.h frame.h povproto.h vector.h

point.$(OBJ): config.h frame.h povproto.h vector.h

poly.$(OBJ): config.h frame.h povproto.h vector.h

quadrics.$(OBJ): config.h frame.h povproto.h vector.h

raw.$(OBJ): config.h frame.h povproto.h

ray.$(OBJ): config.h frame.h povproto.h vector.h

render.$(OBJ): config.h frame.h povproto.h vector.h

spheres.$(OBJ): config.h frame.h povproto.h vector.h

targa.$(OBJ): config.h frame.h povproto.h

texture.$(OBJ): config.h frame.h povproto.h texture.h vector.h

tokenize.$(OBJ): config.h frame.h parse.h povproto.h

triangle.$(OBJ): config.h frame.h povproto.h vector.h

txttest.$(OBJ): config.h frame.h povproto.h texture.h vector.h

vect.$(OBJ): config.h frame.h povproto.h vector.h
