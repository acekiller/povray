/****************************************************************************
*                   boxes.h
*
*  This module contains all defines, typedefs, and prototypes for BOXES.CPP.
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
* $File: //depot/povray/3.5/source/boxes.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef BOXES_H
#define BOXES_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define BOX_OBJECT (BASIC_OBJECT)



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Box_Struct BOX;

struct Box_Struct
{
  OBJECT_FIELDS
  VECTOR bounds[2];
};



/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS Box_Methods;



/*****************************************************************************
* Global functions
******************************************************************************/

int  Intersect_Box (RAY *Ray, TRANSFORM *Trans, VECTOR Corner1, VECTOR Corner2, DBL *Depth1, DBL *Depth2, int *Side1, int *Side2);
BOX  *Create_Box (void);
void Destroy_Box (OBJECT *Object);
BOX *Copy_Box (OBJECT *Object);
void Compute_Box_BBox (BOX *Box);

#endif
