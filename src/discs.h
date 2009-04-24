/****************************************************************************
*                   discs.h
*
*  This module contains all defines, typedefs, and prototypes for DISCS.CPP.
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
* $File: //depot/povray/3.5/source/discs.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#ifndef DISCS_H
#define DISCS_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define DISC_OBJECT            (BASIC_OBJECT)



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Disc_Struct DISC;

struct Disc_Struct
{
  OBJECT_FIELDS
  VECTOR center;    /* Center of the disc */
  VECTOR normal;    /* Direction perpendicular to the disc (plane normal) */
  DBL d;            /* The constant part of the plane equation */
  DBL iradius2;     /* Distance from center to inner circle of the disc */
  DBL oradius2;     /* Distance from center to outer circle of the disc */
};



/*****************************************************************************
* Global variables
******************************************************************************/




/*****************************************************************************
* Global functions
******************************************************************************/

DISC *Create_Disc (void);
void Compute_Disc (DISC *Disc);

#endif
