/****************************************************************************
*                   fnintern.h
*
*  This module contains all defines, typedefs, and prototypes for fnintern.cpp.
*
*  This module is inspired by code by D. Skarda, T. Bily and R. Suzuki.
*  It includes functions based on code first introduced by many other
*  contributors.
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
* $File: //depot/povray/3.5/source/fnintern.h $
* $Revision: #10 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef FNINTERN_H
#define FNINTERN_H

typedef struct
{
	DBL (*fn)(DBL *ptr, unsigned int fn);
	unsigned int parameter_cnt;
} Trap;

typedef struct
{
	void (*fn)(DBL *ptr, unsigned int fn, unsigned int sp);
	unsigned int parameter_cnt;
} TrapS;

extern Trap POVFPU_TrapTable[];
extern TrapS POVFPU_TrapSTable[];

extern unsigned int POVFPU_TrapTableSize;
extern unsigned int POVFPU_TrapSTableSize;

#endif
