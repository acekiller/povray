/****************************************************************************
*                   splines.h
*
*  Contributed by ???
*
*  This module contains all defines, typedefs, and prototypes for splines.cpp.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996-2002 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file.
*  If POVLEGAL.DOC is not available it may be found online at -
*
*    http://www.povray.org/povlegal.html.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
* $File: //depot/povray/3.5/source/splines.h $
* $Revision: #14 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef SPLINE_H
#define SPLINE_H
/* Generic header for spline modules */

#include "frame.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "config.h"

#define INIT_SPLINE_SIZE     16

#define LINEAR_SPLINE         1
#define QUADRATIC_SPLINE      2
#define NATURAL_SPLINE        3
#define CATMULL_ROM_SPLINE    4

SPLINE * Create_Spline(int Type);
SPLINE * Copy_Spline(SPLINE * Old);
void Destroy_Spline(SPLINE * Spline);
void Insert_Spline_Entry(SPLINE * Spline, DBL p, EXPRESS v);
DBL Get_Spline_Val(SPLINE * sp, DBL p, EXPRESS v, int *Terms);

#endif

