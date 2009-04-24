/****************************************************************************
*                   function.h
*
*  This module contains all defines, typedefs, and prototypes for fnsyntax.c.
*
*  This module is based on code by D. Skarda, T. Bily and R. Suzuki.
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
* $File: //depot/povray/3.5/source/function.h $
* $Revision: #14 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef FUNCTION_H
#define FUNCTION_H

typedef unsigned int FUNCTION;
typedef FUNCTION * FUNCTION_PTR;

FUNCTION_PTR Parse_Function(void);
FUNCTION_PTR Parse_FunctionContent(void);
FUNCTION_PTR Parse_DeclareFunction(int *token_id, char *fn_name, bool is_loca);
void Destroy_Function(FUNCTION_PTR Function);
FUNCTION_PTR Copy_Function(FUNCTION_PTR Function);

#endif
