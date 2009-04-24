/****************************************************************************
*                   pigment.h
*
*  This module contains all defines, typedefs, and prototypes for PIGMENT.CPP.
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
* $File: //depot/povray/3.5/source/pigment.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/* NOTE: FRAME.H contains other pigment stuff. */

#ifndef PIGMENT_H
#define PIGMENT_H



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
* Global constants
******************************************************************************/

extern BLEND_MAP Brick_Default_Map;
extern BLEND_MAP Hex_Default_Map;
extern BLEND_MAP Check_Default_Map;


/*****************************************************************************
* Global functions
******************************************************************************/

PIGMENT *Create_Pigment (void);
PIGMENT *Copy_Pigment (PIGMENT *Old);
void Destroy_Pigment (PIGMENT *Pigment);
int Post_Pigment (PIGMENT *Pigment);
int Compute_Pigment (COLOUR Colour, PIGMENT *Pigment, VECTOR IPoint, INTERSECTION *Intersect);
void Make_Pigment_Entries (void);

#endif
