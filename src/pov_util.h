/****************************************************************************
*                   pov_util.h
*
*  This module contains all defines, typedefs, and prototypes for POV_UTIL.CPP.
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
* $File: //depot/povray/3.5/source/pov_util.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef POV_UTIL_H
#define POV_UTIL_H

#include "povms.h"

unsigned closest_power_of_2(unsigned theNumber);
int pov_stricmp(const char *s1, const char *s2);
void POV_Std_Split_Time(DBL time_dif,unsigned long *hrs,unsigned long *mins,DBL *secs);
int POVMSUtil_SetFormatString(POVMSObjectPtr object, POVMSType key, char *format, ...); // Note: Strings may not contain \0 characters codes!

#endif
