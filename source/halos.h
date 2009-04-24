/****************************************************************************
*                   halos.h
*
*  This module contains all defines, typedefs, and prototypes for HALOS.C.
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


#ifndef HALOS_H
#define HALOS_H

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Mapping types. */

#define HALO_PLANAR_MAP      1
#define HALO_SPHERICAL_MAP   2
#define HALO_CYLINDRICAL_MAP 3
#define HALO_BOX_MAP         4

/* Halo_Type flags */

#define HALO_NO_HALO        0
#define HALO_CONSTANT       1
#define HALO_LINEAR         2
#define HALO_CUBIC          3
#define HALO_POLY           4
#define HALO_VOLUME_OBJECT  5

/* Rendering_Type flags */

#define HALO_EMITTING             0
#define HALO_ATTENUATING          1
#define HALO_GLOWING              2
#define HALO_DUST                 3
#define HALO_HAZE                 4
#define HALO_VOLUME_RENDERED      5
#define HALO_VOL_REND_WITH_LIGHT  6


/*****************************************************************************
* Global typedefs
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void Do_Halo PARAMS((HALO *Halo, RAY *Ray, INTERSECTION *Ray_Intersection, COLOUR Colour, int Light_Ray_Flag));
HALO *Create_Halo PARAMS((void));
HALO *Copy_Halo PARAMS((HALO *Old));
void Destroy_Halo PARAMS((HALO *Halo));

void Translate_One_Halo PARAMS((HALO *Halo, TRANSFORM *Trans));
void Scale_One_Halo PARAMS((HALO *Halo, TRANSFORM *Trans));
void Rotate_One_Halo PARAMS((HALO *Halo, TRANSFORM *Trans));
void Transform_One_Halo PARAMS((HALO *Halo, TRANSFORM *Trans));
void Transform_Halo PARAMS((HALO *Halo, TRANSFORM *Trans));
void Transform_Halo_Container PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Rotate_Halo_Container PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Scale_Halo_Container PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Translate_Halo_Container PARAMS((TEXTURE *Textures, TRANSFORM *Trans));

void Post_Halo PARAMS((TEXTURE *Texture));
#endif
