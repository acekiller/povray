/****************************************************************************
*                   parsestr.h
*
*  This module contains all defines, typedefs, and prototypes for parsestr.cpp.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copright 1996-2002 Persistence of Vision Team
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
* $File: //depot/povray/3.5/source/parsestr.h $
* $Revision: #6 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef PARSESTR_H
#define PARSESTR_H

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/


/*****************************************************************************
* Global typedefs
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/


/*****************************************************************************
* Global functions
******************************************************************************/

char *Parse_C_String(bool pathname = false);
UCS2 *Parse_String(bool pathname = false);

UCS2 *String_To_UCS2(char *str, bool pathname = false);
char *UCS2_To_String(UCS2 *str, bool pathname = false);

UCS2 *UCS2_strcat(UCS2 *s1, UCS2 *s2);
int UCS2_strlen(UCS2 *str);
int UCS2_strcmp(UCS2 *s1, UCS2 *s2);
void UCS2_strcpy(UCS2 *s1, UCS2 *s2);
void UCS2_strncpy(UCS2 *s1, UCS2 *s2, int n);
void UCS2_strupr(UCS2 *str);
void UCS2_strlwr(UCS2 *str);

#endif
