/****************************************************************************
*                   point.h
*
*  This module contains all defines, typedefs, and prototypes for POINT.CPP.
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
* $File: //depot/povray/3.5/source/point.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef POINT_H
#define POINT_H

#include "vlbuffer.h"



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define LIGHT_OBJECT (IS_COMPOUND_OBJECT+PATCH_OBJECT+LIGHT_SOURCE_OBJECT)



/* Light source types. */

#define POINT_SOURCE       1
#define SPOT_SOURCE        2
#define FILL_LIGHT_SOURCE  3
#define CYLINDER_SOURCE    4



/*****************************************************************************
* Global typedefs
******************************************************************************/

// moved to frame.h [trf]


/*****************************************************************************
* Global variables
******************************************************************************/




/*****************************************************************************
* Global functions
******************************************************************************/

LIGHT_SOURCE *Create_Light_Source (void);
DBL Attenuate_Light (LIGHT_SOURCE *Light_Source, RAY *Light_Source_Ray, DBL Distance);
COLOUR **Create_Light_Grid (int Size1, int Size2);
extern METHODS Light_Source_Methods ;

#endif
