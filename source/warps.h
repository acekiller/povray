/****************************************************************************
*                   warps.h
*
*  This module contains all defines, typedefs, and prototypes for WARPS.C.
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

/* NOTE: FRAME.H contains other warp stuff. */

#ifndef WARP_H
#define WARP_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Warp types */
#define NO_WARP             0
#define CLASSIC_TURB_WARP   1
#define REPEAT_WARP         2
#define SPIRAL_WARP         3
#define BLACK_HOLE_WARP     4
#define EXTRA_TURB_WARP     5
#define TRANSFORM_WARP      6



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Repeat_Warp REPEAT;
typedef struct Trans_Warp TRANS;
typedef struct Spiral_Warp SPIRAL;
typedef struct Black_Hole_Warp BLACK_HOLE;

struct Repeat_Warp
{
  WARP_FIELDS
  int Axis;
  SNGL Width;
  VECTOR Flip, Offset;
};

struct Trans_Warp
{
  WARP_FIELDS
  TRANSFORM Trans;
};

struct Spiral_Warp
{
  WARP_FIELDS
  VECTOR Center;
  SNGL Strength, Phase;
};

struct Black_Hole_Warp

{

  WARP_FIELDS

  VECTOR      Center ;

  VECTOR      Repeat_Vector ;

  VECTOR      Uncertainty_Vector ;

  DBL         Strength ;

  DBL         Radius ;

  DBL         Radius_Squared ;

  DBL         Inverse_Radius ;

  DBL         Power ;

  short       Inverted ;

  short       Type ;

  short       Repeat ;

  short       Uncertain ;

} ;






/*****************************************************************************
* Global variables
******************************************************************************/


/*****************************************************************************
* Global constants
******************************************************************************/


/*****************************************************************************
* Global functions
******************************************************************************/

void Warp_EPoint PARAMS((VECTOR TPoint, VECTOR EPoint, TPATTERN *TPat));
WARP *Create_Warp PARAMS((int Warp_Type));
void Destroy_Warps PARAMS((WARP *Warps));
WARP *Copy_Warps PARAMS((WARP *Old));

#endif
