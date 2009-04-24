/****************************************************************************
*                   texture.h
*
*  This file contains defines and variables for the txt*.c files
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

/* NOTE: FRAME.H contains other texture stuff. */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "pattern.h"
#include "warps.h"

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define RNDMASK       0x7FFF
#define RNDMULTIPLIER ((DBL)0.000030518509476)

/*
 * Macro to create random number in the [0; 1] range.
 */

#define FRAND() ((DBL)POV_RAND()*RNDMULTIPLIER)

#define FLOOR(x)  ((x) >= 0.0 ? floor(x) : (0.0 - floor(0.0 - (x)) - 1.0))

#define Hash3d(a,b,c) \
  hashTable[(int)(hashTable[(int)(hashTable[(int)((a) & 0xfffL)] ^ ((b) & 0xfffL))] ^ ((c) & 0xfffL))]

#define Hash2d(a,b)   \
  hashTable[(int)(hashTable[(int)((a) & 0xfffL)] ^ ((b) & 0xfffL))]

#define Hash1d(a,b)   \
  hashTable[(int)(a) ^ ((b) & 0xfffL)]

#define INCRSUM(m,s,x,y,z)  \
  ((s)*(RTable[m]*0.5 + RTable[m+1]*(x) + RTable[m+2]*(y) + RTable[m+3]*(z)))

#define INCRSUMP(mp,s,x,y,z) \
  ((s)*((mp[0])*0.5 + (mp[1])*(x) + (mp[2])*(y) + (mp[3])*(z)))


/*****************************************************************************
* Global typedefs
******************************************************************************/



/*****************************************************************************
* Global variables
******************************************************************************/

extern short *hashTable;
extern DBL *frequency;               /* dmf */
extern unsigned int Number_Of_Waves; /* dmf */
extern VECTOR *Wave_Sources;         /* dmf */



/*****************************************************************************
* Global functions
******************************************************************************/

void Compute_Colour PARAMS((COLOUR Colour,PIGMENT *Pigment, DBL value));
void Initialize_Noise PARAMS((void));
void Free_Noise_Tables PARAMS((void));
DBL Noise PARAMS((VECTOR EPoint));
void DNoise PARAMS((VECTOR result, VECTOR EPoint));
DBL Turbulence PARAMS((VECTOR EPoint, TURB *Turb));
void DTurbulence PARAMS((VECTOR result, VECTOR EPoint, TURB *Turb));
DBL cycloidal PARAMS((DBL value));
DBL Triangle_Wave PARAMS((DBL value));
void Translate_Textures PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Rotate_Textures PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Scale_Textures PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Transform_Textures PARAMS((TEXTURE *Textures, TRANSFORM *Trans));
void Destroy_Textures PARAMS((TEXTURE *Textures));
void Post_Textures PARAMS((TEXTURE *Textures));
FINISH *Create_Finish PARAMS((void));
FINISH *Copy_Finish PARAMS((FINISH *Old));
TEXTURE *Create_PNF_Texture PARAMS((void));
TEXTURE *Copy_Texture_Pointer PARAMS((TEXTURE *Texture));
TEXTURE *Copy_Textures PARAMS((TEXTURE *Textures));
TEXTURE *Create_Texture PARAMS((void));
int Test_Opacity PARAMS((TEXTURE *Texture));
TURB *Create_Turb PARAMS((void));
int POV_Std_rand PARAMS((void));
void POV_Std_srand PARAMS((int seed));


#endif
