/****************************************************************************
*                   atmoshp.h
*
*  This module contains all defines, typedefs, and prototypes for ATMOSPH.C.
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


#ifndef ATMOSPH_H
#define ATMOSPH_H

#include "warps.h"

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Define fog types. DMF */

#define ORIG_FOG    1
#define GROUND_MIST 2
#define FOG_TYPES   2

/* Atmospheric scattering types. [DB 11/94] */

#define ISOTROPIC_SCATTERING            1
#define MIE_HAZY_SCATTERING             2
#define MIE_MURKY_SCATTERING            3
#define RAYLEIGH_SCATTERING             4
#define HENYEY_GREENSTEIN_SCATTERING    5
#define SCATTERING_TYPES                5


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Atmosphere_Struct ATMOSPHERE;
typedef struct Fog_Struct FOG;
typedef struct Rainbow_Struct RAINBOW;
typedef struct Skysphere_Struct SKYSPHERE;

/* Atmosphere structure. [DB 11/94] */

struct Atmosphere_Struct
{
  int Type;               /* Scattering type                              */
  int Samples;            /* Number of samples to take                    */
  DBL Distance;           /* Attenuation distance for exponential falloff */
  DBL Distance_Threshold; /* Min. allowed attenuation                     */
  DBL Scattering;         /* Amount of scattering (lighten/darken)        */
  DBL Eccentricity;       /* Exponent of the Henyey-Greenstein function   */
  COLOUR Colour;          /* Color of atmosphere                          */
  int AA_Level;           /* Max. aa-subdivision level                    */
  DBL AA_Threshold;       /* Threshold for supersampling                  */
  DBL Jitter;             /* Amount of jittering                          */
};



struct Fog_Struct
{
  int Type;
  DBL Distance;
  DBL Alt;
  DBL Offset;
  COLOUR Colour;
  VECTOR Up;
  TURB *Turb;
  SNGL Turb_Depth;
  FOG *Next;
};



/* Rainbow structure. [DB 8/94] */

struct Rainbow_Struct
{
  DBL Distance;
  DBL Jitter;
  DBL Angle, Width;
  DBL Arc_Angle, Falloff_Angle, Falloff_Width;
  VECTOR Antisolar_Vector;
  VECTOR Up_Vector, Right_Vector;
  PIGMENT *Pigment;
  RAINBOW *Next;
};



/* Skysphere structure. [DB 8/94] */

struct Skysphere_Struct
{
  int Count;           /* Number of pigments.       */
  PIGMENT **Pigments;  /* Pigment(s) to use.        */
  TRANSFORM *Trans;    /* Skysphere transformation. */
};



/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void Initialize_Atmosphere_Code PARAMS((void));
void Deinitialize_Atmosphere_Code PARAMS((void));

void Do_Infinite_Atmosphere PARAMS((RAY *Ray, COLOUR Colour));
void Do_Finite_Atmosphere PARAMS((RAY *Ray, INTERSECTION *Intersection, COLOUR Colour, int Light_Source_Flag));

ATMOSPHERE *Create_Atmosphere PARAMS((void));
void *Copy_Atmosphere PARAMS((ATMOSPHERE *Atmosphere));
void Destroy_Atmosphere PARAMS((ATMOSPHERE *Atmosphere));

FOG *Create_Fog PARAMS((void));
void *Copy_Fog PARAMS((FOG *Fog));
void Destroy_Fog PARAMS((FOG *Fog));

RAINBOW *Create_Rainbow PARAMS((void));
void *Copy_Rainbow PARAMS((RAINBOW *Rainbow));
void Destroy_Rainbow PARAMS((RAINBOW *Rainbow));

SKYSPHERE *Create_Skysphere PARAMS((void));
void *Copy_Skysphere PARAMS((SKYSPHERE *Skysphere));
void Destroy_Skysphere PARAMS((SKYSPHERE *Skysphere));
void Scale_Skysphere PARAMS((SKYSPHERE *Skysphere, VECTOR Vector));
void Rotate_Skysphere PARAMS((SKYSPHERE *Skysphere, VECTOR Vector));
void Translate_Skysphere PARAMS((SKYSPHERE *Skysphere, VECTOR Vector));
void Transform_Skysphere PARAMS((SKYSPHERE *Skysphere, TRANSFORM *Trans));



#endif
