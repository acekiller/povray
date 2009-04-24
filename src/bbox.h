/****************************************************************************
*                   bbox.h
*
*  This module contains all defines, typedefs, and prototypes for BBOX.CPP.
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
* $File: //depot/povray/3.5/source/bbox.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/* NOTE: FRAME.H contains other bound stuff. */

#ifndef BBOX_H
#define BBOX_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Generate additional bbox statistics. */

#define BBOX_EXTRA_STATS 1


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef int VECTORI[3];
typedef struct Rayinfo_Struct RAYINFO;
typedef struct Qelem_Struct QELEM;
typedef struct Priority_Queue_Struct PRIORITY_QUEUE;

struct Rayinfo_Struct
{
  VECTOR slab_num;
  VECTOR slab_den;
  VECTORI nonzero;
  VECTORI positive;
};

struct Qelem_Struct
{
  DBL Depth;
  BBOX_TREE *Node;
};

struct Priority_Queue_Struct
{
  unsigned QSize;
  unsigned Max_QSize;
  QELEM *Queue;
};



/*****************************************************************************
* Global variables
******************************************************************************/

extern BBOX_TREE *Root_Object;

extern long numberOfFiniteObjects, numberOfInfiniteObjects;


/*****************************************************************************
* Global functions
******************************************************************************/

void Initialize_BBox_Code (void);
void Deinitialize_BBox_Code (void);
void Build_Bounding_Slabs (BBOX_TREE **Root);
void Destroy_Bounding_Slabs (void);
void Recompute_BBox (BBOX *bbox, TRANSFORM *trans);
void Recompute_Inverse_BBox (BBOX *bbox, TRANSFORM *trans);
int  Intersect_BBox_Tree (BBOX_TREE *Root, RAY *ray, INTERSECTION *Best_Intersection, OBJECT **Best_Object, int shadow_flag);
void Check_And_Enqueue (PRIORITY_QUEUE *Queue, BBOX_TREE *Node, BBOX *BBox, RAYINFO *rayinfo);
void Priority_Queue_Delete (PRIORITY_QUEUE *Queue, DBL *key, BBOX_TREE **Node);
void Build_BBox_Tree (BBOX_TREE **Root, long nFinites, BBOX_TREE **&Finite, long numberOfInfiniteObjects, BBOX_TREE **Infinite);
void Destroy_BBox_Tree (BBOX_TREE *Node);
void Create_Rayinfo (RAY *Ray, RAYINFO *rayinfo);

PRIORITY_QUEUE *Create_Priority_Queue (unsigned QSize);
void Destroy_Priority_Queue (PRIORITY_QUEUE *Queue);



#endif
