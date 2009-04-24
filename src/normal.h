/****************************************************************************
*                   normal.h
*
*  This module contains all defines, typedefs, and prototypes for NORMAL.CPP.
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
* $File: //depot/povray/3.5/source/normal.h $
* $Revision: #8 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/* NOTE: FRAME.H contains other normal stuff. */

#ifndef NORMAL_H
#define NORMAL_H



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

TNORMAL *Create_Tnormal (void);
TNORMAL *Copy_Tnormal (TNORMAL *Old);
void Translate_Tnormal (TNORMAL *Tnormal, VECTOR Vector);
void Rotate_Tnormal (TNORMAL *Tnormal, VECTOR Vector);
void Scale_Tnormal (TNORMAL *Tnormal, VECTOR Vector);
void Transform_Tnormal (TNORMAL *Tnormal, TRANSFORM *Trans);
void Destroy_Tnormal (TNORMAL *Tnormal);
void Post_Tnormal (TNORMAL *Tnormal);
void Perturb_Normal (VECTOR Layer_Normal, TNORMAL *Tnormal, VECTOR IPoint, INTERSECTION *Intersection);


#endif
