/****************************************************************************
*                   optout.h
*
*  This module contains all defines, typedefs, and prototypes for OPTOUT.CPP.
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
* $File: //depot/povray/3.5/source/optout.h $
* $Revision: #55 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef OPTOUT_H
#define OPTOUT_H

#include "povray.h"

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* These are used by OPTOUT.C and the machine specific modules */

#define POV_RAY_IS_OFFICIAL 0
#define POV_RAY_VERSION "3.5"
#define POV_RAY_COPYRIGHT "Copyright 1991-2002 POV-Ray Team(tm)"
#define OFFICIAL_VERSION_NUMBER 350

#define DISTRIBUTION_MESSAGE_1 "This is an unofficial version compiled by:"
#error You must complete the following DISTRIBUTION_MESSAGE macro
#define DISTRIBUTION_MESSAGE_2 " FILL IN NAME HERE........................."
#define DISTRIBUTION_MESSAGE_3 " The POV-Ray Team(tm) is not responsible for supporting this version."

/* Number of help pages (numbered 0 to MAX_HELP_PAGE). */

#define MAX_HELP_PAGE   7

/* Used by author output */

#define NUMBER_LENGTH   19
#define OUTPUT_LENGTH   15

#define NUMBER_OF_AUTHORS_ACROSS    4


/*****************************************************************************
* Global typedefs
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/

/* These are available for GUI environments that may display them in a credits dialog */
extern char *Primary_Developers[];
extern char *Contributing_Authors[];
extern INTERSECTION_STATS_INFO intersection_stats[];

/*****************************************************************************
* Global functions
******************************************************************************/

void Print_Credits(void);
void Print_Options(void);
void Print_Stats(char *title, COUNTER *);
void Usage(int n, int f);
void Print_Help_Screens(void);
void Print_Authors(void);
int Banner(char *format,...);
int Statistics(char *format,...);
int Render_Info(char *format,...);
int Status_Info(char *format,...);

#endif
