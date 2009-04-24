/****************************************************************************
*                   warps.h
*
*  This module contains all defines, typedefs, and prototypes for WARPS.CPP.
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
* 10/23/1998 Talious: Added Spherical, Cylindrical, Toroidal warps
*
* $File: //depot/povray/3.5/source/warps.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
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
#define BLACK_HOLE_WARP     3
#define EXTRA_TURB_WARP     4
#define TRANSFORM_WARP      5
#define CYLINDRICAL_WARP    6
#define SPHERICAL_WARP      7
#define TOROIDAL_WARP       8
#define PLANAR_WARP         9


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Repeat_Warp REPEAT;
typedef struct Trans_Warp TRANS;
typedef struct Black_Hole_Warp BLACK_HOLE;
typedef struct Spherical_Warp SPHEREW;
typedef struct Cylindrical_Warp CYLW;
typedef struct Toroidal_Warp TOROIDAL;
typedef struct Planar_Warp PLANARW;

struct Toroidal_Warp
{
  WARP_FIELDS
  VECTOR Orientation_Vector;
  DBL DistExp;
  DBL MajorRadius;
};

struct Cylindrical_Warp
{
  WARP_FIELDS
  VECTOR Orientation_Vector;
  DBL DistExp;
};

struct Planar_Warp
{
  WARP_FIELDS
  VECTOR Orientation_Vector;
  DBL OffSet;
};

struct Spherical_Warp
{
  WARP_FIELDS
  VECTOR Orientation_Vector;
  DBL DistExp;
};

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

void Warp_EPoint (VECTOR TPoint, VECTOR EPoint, TPATTERN *TPat);
WARP *Create_Warp (int Warp_Type);
void Destroy_Warps (WARP *Warps);
WARP *Copy_Warps (WARP *Old);
void Warp_Classic_Turb (VECTOR TPoint, VECTOR EPoint, TPATTERN *TPat);
void Warp_Normal (VECTOR TNorm, VECTOR ENorm, TPATTERN *TPat, int DontScaleBumps);
void UnWarp_Normal (VECTOR TNorm, VECTOR ENorm, TPATTERN *TPat, int DontScaleBumps);

#endif
