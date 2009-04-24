/****************************************************************************
*                   truetype.h
*
*  This module contains all defines, typedefs, and prototypes for TRUETYPE.CPP.
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
* $File: //depot/povray/3.5/source/truetype.h $
* $Revision: #11 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef TRUETYPE_H
#define TRUETYPE_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define TTF_OBJECT (BASIC_OBJECT)



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct TrueTypeFont_Struct TTF;

struct TrueTypeFont_Struct
{
  OBJECT_FIELDS
  void *glyph;      /* (GlyphPtr) Pointer to the glyph */
  DBL depth;        /* Amount of extrusion */
};



/*****************************************************************************
* Global variables
******************************************************************************/

extern METHODS TTF_Methods;



/*****************************************************************************
* Global functions
******************************************************************************/

void ProcessNewTTF (OBJECT *object, char *filename, UCS2 *text_string, DBL depth, VECTOR offset);
void Compute_TTF_BBox (TTF *Ttf);
void FreeFontInfo (void);


#endif
