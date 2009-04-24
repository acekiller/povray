/****************************************************************************
*                   isosurf.h
*
*  This module contains all defines, typedefs, and prototypes for isosurf.cpp.
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
* $File: //depot/povray/3.5/source/isosurf.h $
* $Revision: #23 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef ISOSURF_H
#define ISOSURF_H

#include "function.h"

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define ISOSURFACE_OBJECT      (BASIC_OBJECT)
#define ISOSURFACE_MAXTRACE    10

#define OK_X         1
#define OK_Y         2
#define OK_Z         4
#define OK_R         8
#define OK_S        16
#define OK_T        32
#define OK_U        64
#define OK_V       128


/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS IsoSurface_Methods;


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct IsoSurface_Struct ISOSURFACE;
typedef struct { DBL t,f; } ISO_Pair;
typedef struct
{
	unsigned int refcnt;
	DBL max_gradient, gradient;
	DBL eval_max, eval_cnt, eval_gradient_sum, eval_var;
} ISO_Max_Gradient;

struct IsoSurface_Struct
{
	OBJECT_FIELDS 
	FUNCTION_PTR Function;
	DBL max_gradient;
	DBL gradient;
	DBL threshold;
	DBL accuracy;
	DBL eval_param[3];
	int max_trace;
	int Inv3;
	bool closed;
	bool eval;

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
	VECTOR P,D;
	DBL Vlength;
	DBL tl;
	DBL fmax;
	ISO_Max_Gradient *mginfo;
	bool cache;
};


/*****************************************************************************
* Global functions
******************************************************************************/

ISOSURFACE *Create_IsoSurface (void);
void Destroy_IsoSurface (OBJECT *Object);
void *Copy_IsoSurface (OBJECT *Object);
void Compute_IsoSurface_BBox (ISOSURFACE *Box);


#endif

