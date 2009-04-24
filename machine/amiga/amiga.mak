# Makefile for Persistence of Vision Raytracer 
# This file is released to the public domain.
#
# MAKE Macros and Such...
#

#***************************************************************
#*
#*                      Amiga Options
#*
#***************************************************************

# Uncomment for Amiga Lattice C for IEEE w/o coprocessor
#CFLAGS = -cusrft -fi -m0 -b0 -O -v

# Uncomment for Amiga Lattice C with 68881
CFLAGS = -cusrft -f8 -m2 -b0 -O -v -DM68881

# Flags for debugging
#CFLAGS = -cusrft -q5w5e -d5 -DAMIGA


LIBS881  = lib:lcm881.lib lib:lcnb.lib lib:amiga.lib
LIBSIEEE = lib:lcmieee.lib lib:lcnb.lib lib:amiga.lib

OBJ         = o
MACHINE_OBJ = amiga.$(OBJ)

POVOBJS= povray.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) boxes.$(OBJ)  \
         camera.$(OBJ) colour.$(OBJ) cones.$(OBJ) csg.$(OBJ) discs.$(OBJ)   \
         dump.$(OBJ) express.$(OBJ) gif.$(OBJ) gifdecod.$(OBJ)              \
         hfield.$(OBJ) iff.$(OBJ) image.$(OBJ) lighting.$(OBJ)              \
         matrices.$(OBJ) normal.$(OBJ) objects.$(OBJ) parse.$(OBJ)          \
         pigment.$(OBJ) planes.$(OBJ) point.$(OBJ) poly.$(OBJ)              \
         quadrics.$(OBJ) raw.$(OBJ) ray.$(OBJ) render.$(OBJ) spheres.$(OBJ) \
         targa.$(OBJ) texture.$(OBJ) tokenize.$(OBJ) triangle.$(OBJ)        \
         txttest.$(OBJ) vect.$(OBJ) $(MACHINE_OBJ)

#  Amiga Linkage...
#
 povray881: $(POVOBJS)
        blink <with <
        from lib:c.o $(POVOBJS) LIB $(LIBS881) to povray881
        <

 povrayieee: $(POVOBJS)
        blink <with <
        from lib:c.o $(POVOBJS) LIB $(LIBSIEEE) to povrayieee
        <
