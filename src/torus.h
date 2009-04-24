/****************************************************************************
*                   torus.h
*
*  This module contains all defines, typedefs, and prototypes for TORUS.CPP.
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
* $File: //depot/povray/3.5/source/torus.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef TORUS_H
#define TORUS_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define TORUS_OBJECT (STURM_OK_OBJECT)

/* Generate additional torus statistics. */

#define TORUS_EXTRA_STATS 1



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Torus_Struct TORUS;

/*
 * Torus structure.
 *
 *   R : Major radius
 *   r : Minor radius
 */

struct Torus_Struct
{
  OBJECT_FIELDS
  DBL R, r;
};



/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

TORUS *Create_Torus (void);
void  Compute_Torus_BBox (TORUS *Torus);
int   Test_Thick_Cylinder (VECTOR P, VECTOR D, DBL h1, DBL h2, DBL r1, DBL r2);



#endif
