# Make file for POV-Ray Version 2.12 for use with Intel's "Code Builder Kit" 
# compiler to produce a 32 bit protected mode version.
# This is the make file that the "official" POV-Ray team executable was
# created from.
#
# This version will link in the coprocessor emulation module. The EXE
# will use a coprocessor if one is detected in the machine.
# The Virtual Memory Manager will also be linked in by default.
#
# IMPORTANT: When editing this file do NOT replace the TAB's at the
# beginnng of the lines with all spaces, it seems to break the OPUS
# "make" utility shipped with Code Builder.
# (In the HDRS, OBJS & SRCS lists for example.)
#
# Bill Pulver --- 1/94

CC      = icc         
CFLAGS  = /O3 /znosignedchar /zmod486 /znoansi /zfloatsync

#/D_mathh_builtin        This switch will force the compiler to use the
#                        Library math functions instead of the faster 
#                        inline functions.

                      
DEST    = .           
                      
EXTHDRS		=
HDRS		= config.h extend.h frame.h parse.h povproto.h texture.h \
		  tiga.h tigadefs.h vector.h

LINKFLAGS  =  /e$(PROGRAM) /xiobuf=64k /s90000

# ------------------- LINKFLAGS options -------------------------------------
# /sxxxxxx   Use higher stack values to increase the maximum number of
#            levels that reflections can be calculated for. The default
#            CB stack allows a max_trace_level of about 20 -> 25. 512K
#            raises it to over 500, depending on the image, but trace
#            times get -REAL- long. 
#
# /xiobuf    Sets the size of the I/O buffer for the EXE. Range >=4k to <=64k
#
#            Use /n to generate no copro code with version 1.0x of ICB.
#
# 1.1x links the FPU emulator in by default. Use the /F switch for 1.1x 
# versions of ICB to suppress linking of the FPU emulator.
#
# Use /xnovm to generate an EXE without the virtual memory manager linked in.



LDMAP   =             # Optional - to name the linker map file
LIBS    =             # Optional - library files (full pathnames)


OBJS		= bezier.obj blob.obj bound.obj boxes.obj camera.obj \
		  colour.obj cones.obj csg.obj discs.obj dump.obj \
		  express.obj gif.obj gifdecod.obj hfield.obj ibm.obj \
		  iff.obj image.obj lighting.obj matrices.obj normal.obj \
		  objects.obj parse.obj pigment.obj planes.obj point.obj \
		  poly.obj povray.obj quadrics.obj raw.obj ray.obj \
		  render.obj spheres.obj targa.obj texture.obj tokenize.obj \
		  triangle.obj txttest.obj vect.obj _get_ebx.obj

PROGRAM = POVRAY


SRCS		= bezier.c blob.c bound.c boxes.c camera.c colour.c cones.c \
		  csg.c discs.c dump.c express.c gif.c gifdecod.c hfield.c \
		  ibm.c iff.c image.c lighting.c matrices.c normal.c \
		  objects.c parse.c pigment.c planes.c point.c poly.c \
		  povray.c quadrics.c raw.c ray.c render.c spheres.c targa.c \
		  texture.c tokenize.c triangle.c txttest.c vect.c


# This is the only required target. It states that the PROGRAM depends
# on all files listed in the OBJS and LIBS macros. The command for updating
# the library is the line following the dependency line.
#
# -----------------------------------------------------------------------
# Create linker response file since the command line is too long for DOS.
# Link program.
# Erase linker response file so that later runs are not >> echoed into
# the x.LRF file & confuse the linker.
# -----------------------------------------------------------------------


CB_LINK_RESP=$(PROGRAM,B,S/.*/&.LRF)

$(PROGRAM):  $(OBJS)
	echo $(OBJS)	   > $(CB_LINK_RESP)
	echo $(LINKFLAGS) >> $(CB_LINK_RESP)
	$(CC) @$(CB_LINK_RESP)
        @echo
        @echo Link completed. Now clean things up.....
        erase $(CB_LINK_RESP)
        @echo ********** D O N E ************


### MKMF:  Do not remove this line!  Automatic dependencies follow.

bezier.obj: config.h frame.h povproto.h vector.h

blob.obj: config.h frame.h povproto.h vector.h

bound.obj: config.h frame.h povproto.h vector.h

boxes.obj: config.h frame.h povproto.h vector.h

camera.obj: config.h frame.h povproto.h vector.h

colour.obj: config.h frame.h povproto.h vector.h

cones.obj: config.h frame.h povproto.h vector.h

csg.obj: config.h frame.h povproto.h vector.h

discs.obj: config.h frame.h povproto.h vector.h

dump.obj: config.h frame.h povproto.h

express.obj: config.h frame.h parse.h povproto.h vector.h

gif.obj: config.h frame.h povproto.h

gifdecod.obj: config.h frame.h povproto.h

hfield.obj: config.h frame.h povproto.h vector.h

ibm.obj: config.h extend.h frame.h povproto.h tiga.h tigadefs.h

iff.obj: config.h frame.h povproto.h

image.obj: config.h frame.h povproto.h texture.h vector.h

lighting.obj: config.h frame.h povproto.h vector.h

matrices.obj: config.h frame.h povproto.h vector.h

normal.obj: config.h frame.h povproto.h texture.h vector.h

objects.obj: config.h frame.h povproto.h vector.h

parse.obj: config.h frame.h parse.h povproto.h vector.h

pigment.obj: config.h frame.h povproto.h texture.h vector.h

planes.obj: config.h frame.h povproto.h vector.h

point.obj: config.h frame.h povproto.h vector.h

poly.obj: config.h frame.h povproto.h vector.h

povray.obj: config.h frame.h povproto.h

quadrics.obj: config.h frame.h povproto.h vector.h

raw.obj: config.h frame.h povproto.h

ray.obj: config.h frame.h povproto.h vector.h

render.obj: config.h frame.h povproto.h vector.h

spheres.obj: config.h frame.h povproto.h vector.h

targa.obj: config.h frame.h povproto.h

texture.obj: config.h frame.h povproto.h texture.h vector.h

tokenize.obj: config.h frame.h parse.h povproto.h

triangle.obj: config.h frame.h povproto.h vector.h

txttest.obj: config.h frame.h povproto.h texture.h vector.h

vect.obj: config.h frame.h povproto.h vector.h
