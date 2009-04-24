/****************************************************************************
*                   triangle.h
*
*  This module contains all defines, typedefs, and prototypes for TRIANGLE.CPP.
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
* $File: //depot/povray/3.5/source/triangle.h $
* $Revision: #10 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef TRIANGLE_H
#define TRIANGLE_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define TRIANGLE_OBJECT        (PATCH_OBJECT)
#define SMOOTH_TRIANGLE_OBJECT (PATCH_OBJECT)
/* NK 1998 double_illuminate - removed +DOUBLE_ILLUMINATE from smooth_triangle */


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Triangle_Struct TRIANGLE;
typedef struct Smooth_Triangle_Struct SMOOTH_TRIANGLE;
typedef struct Smooth_Color_Triangle_Struct SMOOTH_COLOR_TRIANGLE; /* AP */

struct Triangle_Struct
{
  OBJECT_FIELDS
  VECTOR  Normal_Vector;
  DBL     Distance;
  unsigned int  Dominant_Axis:2;
  unsigned int  vAxis:2;  /* used only for smooth triangles */
  VECTOR  P1, P2, P3;
};

struct Smooth_Triangle_Struct
{
  OBJECT_FIELDS
  VECTOR  Normal_Vector;
  DBL     Distance;
  unsigned int  Dominant_Axis:2;
  unsigned int  vAxis:2;         /* used only for smooth triangles */
  VECTOR  P1, P2, P3;
  VECTOR  N1, N2, N3, Perp;
};

struct Smooth_Color_Triangle_Struct /* AP */
{
  OBJECT_FIELDS
  VECTOR  Normal_Vector;
  DBL     Distance;
  unsigned int  Dominant_Axis:2;
  unsigned int  vAxis:2;         /* used only for smooth triangles */
  VECTOR  P1, P2, P3;
  VECTOR  N1, N2, N3, Perp;
  int magic;
  COLOUR  C1, C2, C3;
};


/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS Triangle_Methods;
extern METHODS Smooth_Triangle_Methods;



/*****************************************************************************
* Global functions
******************************************************************************/

TRIANGLE *Create_Triangle (void);
SMOOTH_TRIANGLE *Create_Smooth_Triangle (void);
int Compute_Triangle  (TRIANGLE *Triangle, int Smooth);
void Compute_Triangle_BBox (TRIANGLE *Triangle);
/* AP */
DBL Calculate_Smooth_T(VECTOR IPoint, VECTOR P1, VECTOR P2, VECTOR P3);

#endif
