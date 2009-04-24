# Makefile for Persistence of Vision Raytracing Program
# This file is released to the public domain.
#
# This make file requires a Borland MAKE utility which supports
# conditional directives and supports the .autodepend directive.
# it should work for most any Turbo or Borland C or C++
#
# Written by Chris Young CIS:[76702,1655]

#***************************************************************
#*
#*                      User Defined Options
#*
#***************************************************************
#
# Enable each option below with MAKE -DOPTION or by uncommenting
# the appropriate line.
#
# Select only one CPU type
#CPU86	=1
#CPU186	=1
CPU286	=1
#
# Select only one FPU type
#FPU87	=1
FPU287	=1
#
# Debug info included
#DEBUG	=1
#

# Select compiler by commenting or uncommenting
#  Switches
#   -P-C        Turns C++ off, uses .C extension, not .CPP
#CC	=TCC
#CC	=BCCX -P-C
CC	=BCC -P-C

#***************************************************************
#*  Compiler & Linker Options set automatically from User
#*  Defined Switches above
#***************************************************************

# NOTE: -a is word align
!if $d(CPU86)
CPU	=-1-
!elif $d(CPU186)
CPU	=-1 -a
!elif $d(CPU286)
CPU	=-2 -a
!else
!error Must define a cpu type
!endif

!if $d(FPU287)
FPU	=-f287
FPULIB	=fp87
!elif $d(FPU87)
FPU	=-f87
FPULIB	=fp87
!else
FPU	=-f
FPULIB	=emu
!endif

!if $d(DEBUG)
CDEBUG	=-v
LDEBUG	=-llv
!else
CDEBUG	=-v-
LDEBUG	=
!endif

# Compiler flags common to all CPUs & FPUs
#
#  -ml  large memory model
#  -r   use registers
#  -K   default character unsigned
#  -G   optimize for speed rather than size
#  -O   optimize jumps
#  -Z   optimize register use
#  -d   merge duplicate strings
#  -c   compile only
#  -N-  stack overflow check off
#  -B   compile via assembly
#  -k-  standard stack on all calls off

FIXED	=$(CC) -ml -r -K -G -O -Z -d -c -N- -k-

CFLAGS	=$(FIXED) $(CDEBUG) $(CPU) $(FPU)

#***************************************************************
#*
#*                          Common  Stuff
#*
#***************************************************************

POVOBJS	= povray.obj bezier.obj blob.obj bound.obj boxes.obj camera.obj \
          colour.obj cones.obj csg.obj discs.obj dump.obj express.obj \
          gifdecod.obj gif.obj hfield.obj ibm.obj iff.obj image.obj \
          lighting.obj matrices.obj normal.obj objects.obj parse.obj \
          pigment.obj planes.obj point.obj poly.obj quadrics.obj raw.obj \
          ray.obj render.obj spheres.obj targa.obj texture.obj tokenize.obj \
          triangle.obj txttest.obj vect.obj ai.lib

# Linkage: Use BCC to invoke TLINK
# NOTE: AUTODEPEND doesn't work here.
#
povray.exe : $(POVOBJS)
	$(CFLAGS) -c- $(LDEBUG) @ibmbc.lnk
#
# Specific module/header dependencies for POV-Ray are not shown because
# the .autodepend feature takes care of it.
#

.AUTODEPEND

.c.obj :
 $(CFLAGS) $*

