/****************************************************************************
*                   spheres.h
*
*  This module contains all defines, typedefs, and prototypes for SPHERES.C.
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


#ifndef SPHERES_H
#define SPHERES_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define SPHERE_OBJECT (BASIC_OBJECT)



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Sphere_Struct SPHERE;

struct Sphere_Struct
{
  OBJECT_FIELDS
  TRANSFORM *Trans;
  VECTOR Center;
  DBL    Radius;
};



/*****************************************************************************
* Global variables
******************************************************************************/




/*****************************************************************************
* Global functions
******************************************************************************/

SPHERE *Create_Sphere PARAMS((void));
void Compute_Sphere_BBox PARAMS((SPHERE *Sphere));
int Intersect_Sphere PARAMS((RAY *Ray, VECTOR Center, DBL Radius2, DBL *Depth1, DBL *Depth2));
void *Copy_Sphere PARAMS((OBJECT *Object));
void Transform_Sphere PARAMS((OBJECT *Object, TRANSFORM *Trans));
void Destroy_Sphere PARAMS((OBJECT *Object));

#endif
