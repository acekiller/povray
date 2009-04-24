/****************************************************************************
*                   vbuffer.h
*
*  This module contains all defines, typedefs, and prototypes for VBUFFER.C.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/


#ifndef VBUFFER_H
#define VBUFFER_H

#include "vlbuffer.h"



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/




/*****************************************************************************
* Global typedefs
******************************************************************************/




/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void Prune_Vista_Tree PARAMS((int y));
void Trace_Primary_Ray PARAMS((RAY *Ray, COLOUR Colour, DBL Weight, int x));
void Build_Vista_Buffer PARAMS((void));
void Destroy_Vista_Buffer PARAMS((void));
void Draw_Vista_Buffer PARAMS((void));



#endif
