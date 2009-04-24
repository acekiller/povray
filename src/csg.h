/****************************************************************************
*                   csg.h
*
*  This module contains all defines, typedefs, and prototypes for CSG.CPP.
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
* $File: //depot/povray/3.5/source/csg.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef CSG_H
#define CSG_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define UNION_OBJECT        (IS_COMPOUND_OBJECT)
#define MERGE_OBJECT        (IS_COMPOUND_OBJECT)
#define INTERSECTION_OBJECT (IS_COMPOUND_OBJECT)

/* CSG types */

#define CSG_UNION_TYPE             1
#define CSG_INTERSECTION_TYPE      2
#define CSG_DIFFERENCE_TYPE        4
#define CSG_MERGE_TYPE             8
#define CSG_SINGLE_TYPE           16



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct CSG_Struct CSG;

struct CSG_Struct
{
  COMPOUND_FIELDS
  int do_split;
};



/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS CSG_Intersection_Methods;
extern METHODS CSG_Merge_Methods;
extern METHODS CSG_Union_Methods;



/*****************************************************************************
* Global functions
******************************************************************************/

CSG *Create_CSG_Union (void);
CSG *Create_CSG_Merge (void);
CSG *Create_CSG_Intersection (void);
void Compute_CSG_BBox (OBJECT *Object);
void Determine_CSG_Textures(CSG *Csg, VECTOR IPoint, int *Count, TEXTURE **Textures, DBL *Weights);


#endif
