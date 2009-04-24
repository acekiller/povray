/****************************************************************************
*                   parstxtr.h
*
*  This header file is included by all all language parsing C modules in
*  POV-Ray.
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

#ifndef PARSTXTR_H
#define PARSTXTR_H

#include "atmosph.h"

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/



/*****************************************************************************
* Global typedefs
******************************************************************************/



/*****************************************************************************
* Global variables
******************************************************************************/

extern short Have_Vector;
extern TEXTURE *Default_Texture;



/*****************************************************************************
* Global functions
******************************************************************************/

TEXTURE *Parse_Texture PARAMS((void));
void Parse_Pigment PARAMS((PIGMENT **Pigment_Ptr));
void Parse_Tnormal PARAMS((TNORMAL **Tnormal_Ptr));
void Parse_Finish PARAMS((FINISH **Finish_Ptr));
void Parse_Halo PARAMS((HALO **Halo_Ptr));
ATMOSPHERE *Parse_Atmosphere PARAMS((void));
FOG *Parse_Fog PARAMS((void));
RAINBOW *Parse_Rainbow PARAMS((void));                      
SKYSPHERE *Parse_Skysphere PARAMS((void));
IMAGE *Parse_Image PARAMS((int Legal));

#endif
