/*****************************************************************************
*               xwinconf.h
*
*  This header file contains all constants and types required to run on a
*  X Windows Display system.  This also uses the UNIX config file unixconf.h
*  for the non-display portions of the UNIX code.
*
*  from Persistence of Vision Raytracer
*  Copyright 1995 Persistence of Vision Team
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

/* Below are several defines needed by the X Windows part of POV.  You
 * may redefine them as needed in this config.h file.  Each has a default
 * which will be defined in frame.h if you don't define it here.
 */

#include "unixconf.h"

/*
 * These handle the parsing of the usual X Windows command-line args.
 */
#define STARTUP_POVRAY    XWIN_init_povray(&argc, &argv);
#define FINISH_POVRAY(i)  XWIN_finish_povray(i);
void XWIN_init_povray PARAMS((int *argc, char **argv[]));
void XWIN_finish_povray PARAMS((int status));

/*
 * These are the X Windows display routines.
 */
#define POV_DISPLAY_INIT(w,h) XWIN_display_init(w,h)
#define POV_DISPLAY_PLOT(x,y,r,g,b,a) XWIN_display_plot(x,y,r,g,b,a)
#define POV_DISPLAY_PLOT_RECT(x1,y1,x2,y2,r,g,b,a) XWIN_display_plot_rect(x1,y1,x2,y2,r,g,b,a)
#define POV_DISPLAY_PLOT_BOX(x1,y1,x2,y2,r,g,b,a) XWIN_display_plot_box(x1,y1,x2,y2,r,g,b,a)
#define POV_DISPLAY_FINISHED  XWIN_display_finished();
#define POV_DISPLAY_CLOSE XWIN_display_close();
void XWIN_display_init PARAMS((int w, int h));
void XWIN_display_plot PARAMS((int x, int y,
                               unsigned char Red, unsigned char Green,
                               unsigned char Blue, unsigned char Alpha));
void XWIN_display_plot_rect PARAMS((int x1, int x2, int y1, int y2,
                                    unsigned char Red, unsigned char Green,
                                    unsigned char Blue, unsigned char Alpha));
void XWIN_display_plot_box PARAMS((int x1, int y1, int x2, int y2,
                                    unsigned char Red, unsigned char Green,
                                    unsigned char Blue, unsigned char Alpha));
void XWIN_display_finished PARAMS((void));
void XWIN_display_close PARAMS((void));

/*
 * This is the render abort function for X Windows.  It is not needed for
 * the command-line interface, since the abort is handled via interrupts.
 */
#define TEST_ABORT Stop_Flag += XWIN_Test_Abort();
int XWIN_Test_Abort PARAMS((void));
