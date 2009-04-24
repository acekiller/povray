/****************************************************************************
*                   quadrics.h
*
*  This module contains all defines, typedefs, and prototypes for QUADRICS.CPP.
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
* $File: //depot/povray/3.5/source/quadrics.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef QUADRICS_H
#define QUADRICS_H

#include "planes.h"



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define QUADRIC_OBJECT (BASIC_OBJECT)



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Quadric_Struct QUADRIC;

struct Quadric_Struct
{
  OBJECT_FIELDS
  VECTOR Square_Terms;
  VECTOR Mixed_Terms;
  VECTOR Terms;
  DBL Constant;
  bool Automatic_Bounds;
};



/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS Quadric_Methods;



/*****************************************************************************
* Global functions
******************************************************************************/

QUADRIC *Create_Quadric (void);
void Compute_Quadric_BBox (QUADRIC *Quadric, VECTOR Min, VECTOR Max);
void Compute_Plane_Min_Max (PLANE *Plane, VECTOR Min, VECTOR Max);



#endif
