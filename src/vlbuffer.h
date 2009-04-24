/****************************************************************************
*                   vlbuffer.h
*
*  This module contains all defines, typedefs, and prototypes for VLBUFFER.CPP.
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
* $File: //depot/povray/3.5/source/vlbuffer.h $
* $Revision: #8 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef VLBUFFER_H
#define VLBUFFER_H

#include "frame.h"
#include "bbox.h"



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* flag to mark a node as pruned */

#define PRUNE_CHECK 128
#define PRUNE_TEMPORARY 128



/* Define minimum and maximum values for buffer coordinates. */

#define MIN_BUFFER_ENTRY -32000
#define MAX_BUFFER_ENTRY  32000



/* Define maximum number of clippoints. */

#define MAX_CLIP_POINTS 20



/* Define all six coordinate axes. */

#define XaxisP 0
#define XaxisM 1
#define YaxisP 2
#define YaxisM 3
#define ZaxisP 4
#define ZaxisM 5



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Project_Tree_Leaf_Struct PROJECT_TREE_LEAF;
typedef struct Project_Queue_Struct PROJECT_QUEUE;

struct Project_Tree_Leaf_Struct
{
  unsigned short is_leaf;
  BBOX_TREE *Node;
  PROJECT Project;
};

struct Project_Queue_Struct
{
  unsigned QSize;
  unsigned Max_QSize;
  PROJECT_TREE_NODE **Queue;
};




/*****************************************************************************
* Global variables
******************************************************************************/

extern PROJECT_QUEUE *Node_Queue;
extern PRIORITY_QUEUE *VLBuffer_Queue;



/*****************************************************************************
* Global functions
******************************************************************************/

void Clip_Polygon (VECTOR *Points, int *PointCnt, VECTOR VX1, VECTOR VX2, VECTOR VY1, VECTOR VY2, DBL DX1, DBL DX2, DBL DY1, DBL DY2);

void Initialize_VLBuffer_Code (void);
void Reinitialize_VLBuffer_Code (void);
void Deinitialize_VLBuffer_Code (void);

void Destroy_Project_Tree (PROJECT_TREE_NODE *Node);



#endif
