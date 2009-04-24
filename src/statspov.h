/****************************************************************************
*                   statspov.h
*
*  This module contains all defines, typedefs, and prototypes for statspov.cpp.
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
* $File: //depot/povray/3.5/source/statspov.h $
* $Revision: #8 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef STATSPOV_H
#define STATSPOV_H

void add_numbers(char *result, char  *c1, char  *c2);
void counter_to_string(COUNTER *counter, char *string, int len);
void init_statistics(COUNTER *pstats);
void sum_statistics(COUNTER *ptotalstats, COUNTER *pstats);
void check_stats(register int y, register int doingMosaic, register int pixWidth);
void print_intersection_stats(char *text, COUNTER *tests, COUNTER  *succeeded);
void Print_Stats(char *title, COUNTER *pstats);
int Statistics(char *format, ...);

#endif
