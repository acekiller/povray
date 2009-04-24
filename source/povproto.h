/****************************************************************************
*                   povproto.h
*
*  This module defines the prototypes for all system-independent functions.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/

#ifndef POVPROTO_H
#define POVPROTO_H



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
void display_finished PARAMS((void));
void display_init PARAMS((int width, int height));
void display_close PARAMS((void));
void display_plot PARAMS((int x, int y, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha));
void display_plot_rect PARAMS((int x1, int x2, int y1, int y2,
  unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha));

/* Prototypes for functions defined in mem.c */
#include "mem.h"

/* Prototypes for functions defined in userio.c */

int CDECL Banner PARAMS((char *format,...));
int CDECL Warning PARAMS((DBL level, char *format,...));
int CDECL Render_Info PARAMS((char *format,...));
int CDECL Status_Info PARAMS((char *format,...));
int CDECL Statistics PARAMS((char *format,...));
int CDECL Error_Line PARAMS((char *format,...));
int CDECL Error PARAMS((char *format,...));
int CDECL Debug_Info PARAMS((char *format, ...));

void Terminate_POV PARAMS((int i));

#endif
