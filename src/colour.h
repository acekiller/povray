/****************************************************************************
*                   colour.h
*
*  This module contains all defines, typedefs, and prototypes for COLOUR.CPP.
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
* $File: //depot/povray/3.5/source/colour.h $
* $Revision: #12 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/* NOTE: FRAME.H contains other colour stuff. */

#ifndef COLOUR_H
#define COLOUR_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define GREY_SCALE(C) (0.297*(C)[0] + 0.589*(C)[1] + 0.114*(C)[2])

#define GREY_SCALE3(CR,CG,CB) (0.297*(CR) + 0.589*(CG) + 0.114*(CB))

/*****************************************************************************
* Global typedefs
******************************************************************************/




/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

COLOUR *Create_Colour (void);
COLOUR *Copy_Colour (COLOUR Old);
BLEND_MAP_ENTRY *Create_BMap_Entries (int Map_Size);
BLEND_MAP_ENTRY *Copy_BMap_Entries (BLEND_MAP_ENTRY *Old,int Map_Size,int Type);
BLEND_MAP *Create_Blend_Map (void);
BLEND_MAP *Copy_Blend_Map (BLEND_MAP *Old);
DBL Colour_Distance (COLOUR colour1, COLOUR colour2);
DBL Colour_Distance_RGBT (COLOUR colour1, COLOUR colour2);
void Add_Colour (COLOUR result, COLOUR colour1, COLOUR colour2);
void Scale_Colour (COLOUR result, COLOUR colour, DBL factor);
void Clip_Colour (COLOUR result, COLOUR colour);
void Destroy_Blend_Map (BLEND_MAP *BMap);
DBL RGBtoHue( COLOUR c );




#endif
