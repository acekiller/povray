/****************************************************************************
*                   media.h
*
*  This module contains all defines, typedefs, and prototypes for MEDIA.CPP.
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
* $File: //depot/povray/3.5/source/media.h $
* $Revision: #10 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef MEDIA_H
#define MEDIA_H

#include "point.h"

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Scattering types. */

#define ISOTROPIC_SCATTERING            1
#define MIE_HAZY_SCATTERING             2
#define MIE_MURKY_SCATTERING            3
#define RAYLEIGH_SCATTERING             4
#define HENYEY_GREENSTEIN_SCATTERING    5
#define SCATTERING_TYPES                5

/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Light_List_Struct LIGHT_LIST;
typedef struct Media_Interval_Struct MEDIA_INTERVAL;
typedef struct Lit_Interval_Struct LIT_INTERVAL;

struct Lit_Interval_Struct
{
  int lit;
  DBL s0, s1, ds;
};

struct Media_Interval_Struct
{
  int lit;
  int samples;
  DBL s0, s1, ds;
  COLOUR od;
  COLOUR te;
  COLOUR te2;
};

struct Light_List_Struct
{
  bool active;
  DBL s0, s1;
  LIGHT_SOURCE *Light;
};


/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void Simulate_Media (IMEDIA **, RAY *, INTERSECTION *, COLOUR , int );

void Backtrace_Simulate_Media(IMEDIA **Media_List, RAY *Ray, INTERSECTION *Inter, COLOUR Colour);

void Initialize_Media_Code();
void Deinitialize_Media_Code();

IMEDIA *Create_Media (void);
IMEDIA *Copy_Media (IMEDIA *);
void Destroy_Media (IMEDIA *);
void Transform_Media (IMEDIA *IMedia, TRANSFORM *Trans);
void Transform_Density (PIGMENT *Density, TRANSFORM *Trans);
void Post_Media (IMEDIA *);

#endif
