/****************************************************************************
*                   fpmetric.h
*
*  This module contains all defines, typedefs, and prototypes for fpmetric.cpp.
*
*  This module was written by D.Skarda&T.Bily and modified by R.Suzuki.
*  Ported to POV-Ray 3.5 by Thorsten Froehlich.
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
* $File: //depot/povray/3.5/source/fpmetric.h $
* $Revision: #10 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef FPMETRIC_H
#define FPMETRIC_H


/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define PARAMETRIC_OBJECT        (PATCH_OBJECT) 


/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS Parametric_Methods;


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Parametric_Struct PARAMETRIC;
typedef struct PrecompParValues_Struct PRECOMP_PAR_DATA; 

struct PrecompParValues_Struct 
{
   int      use, depth;
   char     flags;

   DBL      *Low[3], *Hi[3];     /*  X,Y,Z  */
 };


struct Parametric_Struct 
{
	OBJECT_FIELDS
	FUNCTION_PTR Function[3];
	DBL umin, umax, vmin, vmax;
	DBL accuracy;
  DBL max_gradient;
	int Inverted; 

	int container_shape;
	union
	{
		struct
		{
			VECTOR center;
			DBL radius;
		} sphere;
		struct
		{
			VECTOR corner1;
			VECTOR corner2;
		} box;
	} container;

	// internal use only
	PRECOMP_PAR_DATA *PData;
	DBL last_u, last_v;
};


/*****************************************************************************
* Global functions
******************************************************************************/

PARAMETRIC  *Create_Parametric (void);
void Destroy_Parametric (OBJECT *Object);
void *Copy_Parametric (OBJECT *Object);
void Compute_Parametric_BBox (PARAMETRIC *Param);

PRECOMP_PAR_DATA *Precompute_Parametric_Values(PARAMETRIC *Par, char flags, int depth);
PRECOMP_PAR_DATA *Copy_PrecompParVal (PRECOMP_PAR_DATA *PPV);
void Destroy_PrecompParVal (PRECOMP_PAR_DATA *PPV);

#endif
