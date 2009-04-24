/****************************************************************************
*                   povproto.h
*
*  This module defines the prototypes for all system-independent functions.
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
* $File: //depot/povray/3.5/source/povproto.h $
* $Revision: #12 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#ifndef POVPROTO_H
#define POVPROTO_H

/* Prototypes for functions defined in mem.c */
#include "pov_mem.h"
#include "userio.h"


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

/* Prototypes for machine specific functions defined in "computer".c (ibm.c amiga.c unix.c etc.)*/
void display_finished (void);
int display_init (int width, int height);
void display_close (void);
void display_plot (int x, int y, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);
void display_plot_rect (int x1, int x2, int y1, int y2,
  unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);

void povray_terminate(void);

/* Prototypes for functions defined in userio.c */

int CDECL Banner(char *format,...);
int CDECL Debug_Info(char *format, ...);
int CDECL Render_Info(char *format,...);
int CDECL Status_Info(char *format,...);
int CDECL Statistics(char *format,...);

int CDECL Warning(unsigned int level, char *format,...);
int CDECL WarningAt(unsigned int level, char *filename, long line, char *format, ...);
int CDECL Error(char *format,...);
int CDECL PossibleError(char *format,...);
int CDECL ErrorAt(char *filename, long line, char *format, ...);

/* Prototypes for functions defined in benchmark.c */

bool Write_Benchmark_File (const char *Scene_File_Name, const char *INI_File_Name) ;
unsigned int Get_Benchmark_Version (void) ;

#endif
