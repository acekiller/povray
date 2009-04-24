/*****************************************************************************
*               svgaconf.h
*
*  This header file contains all constants and types required to run on a
*  UNIX system.
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

/* Below are several defines needed by the SVGAlib part of POV.  You
 * may redefine them as needed in this config.h file.  Each has a default
 * which will be defined in frame.h if you don't define it here.
 */

#include <vga.h>
#include "unixconf.h"

/*
 * This initializes the SVGAlib part of the display, including the suid-root
 * part which grabs direct access to the VGA video memory.
 */
#define STARTUP_POVRAY SVGA_init_povray();
void SVGA_init_povray PARAMS((void));

/*
 * These functions handle all aspects of the SVGAlib display for POV.
 */
#define POV_DISPLAY_INIT(w,h) SVGA_display_init(w,h)
#define POV_DISPLAY_PLOT(x,y,r,g,b,a) SVGA_display_plot(x,y,r,g,b,a)
#define POV_DISPLAY_CLOSE SVGA_display_close();
#define POV_DISPLAY_PLOT_RECT(x1,y1,x2,y2,r,g,b,a) SVGA_display_plot_rect(x1,y1,x2,y2,r,g,b,a)
#define POV_DISPLAY_PLOT_BOX(x1,y1,x2,y2,r,g,b,a) SVGA_display_plot_box(x1,y1,x2,y2,r,g,b,a)
void SVGA_display_init PARAMS((int w, int h));
void SVGA_display_plot PARAMS((int x, int y,
                               unsigned char Red, unsigned char Green,
                               unsigned char Blue, unsigned char Alpha));
void SVGA_display_plot_rect PARAMS((int x1, int x2, int y1, int y2,
                                    unsigned char Red, unsigned char Green,
                                    unsigned char Blue, unsigned char Alpha));
void SVGA_display_plot_box PARAMS((int x1, int y1, int x2, int y2,
                                    unsigned char Red, unsigned char Green,
                                    unsigned char Blue, unsigned char Alpha));
void SVGA_display_close PARAMS((void));

/*
 * This is the function which checks if the user wants to quit.
 */
#define TEST_ABORT if (opts.Options&EXITENABLE && toupper(vga_getkey())=='Q') \
                      Stop_Flag++;
