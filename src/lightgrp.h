/****************************************************************************
*                   LIGHTGRP.H
*
*  This module contains all defines, typedefs, and prototypes for LIGHTGRP.CPP.
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
* $File: //depot/povray/3.5/source/lightgrp.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#ifndef LIGHTGRP_H
#define LIGHTGRP_H

#include "csg.h"

// This struct is for creating a list of light sources that is INDEPENDENT
// from the Next_Light_Source pointer that is part of the light source
// struct.
typedef struct light_group_light_struct LIGHT_GROUP_LIGHT;

struct light_group_light_struct {
  LIGHT_SOURCE* Light;
  LIGHT_GROUP_LIGHT* Next;
};


void Promote_Local_Lights(CSG *Object);
bool Check_Photon_Light_Group(OBJECT* Object);

#endif
