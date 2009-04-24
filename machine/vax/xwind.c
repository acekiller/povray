/****************************************************************************
*                xwind.c
*
*  This file contains the XWindows code for the display-as-you-trace feature.
*
*  from Persistence of Vision Raytracer
*  Copyright 1993 Persistence of Vision Team
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

/******************************************************************************/
/*                                                                            */
/* X Windows code for POV-Ray.                                                */
/* Written by Christopher J. Cason.                                           */
/* CIS 100032,1644                                                            */
/* Internet 100032.1644@compuserve.com                                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Original IBM VGA "colour" output routines for MS/DOS by Aaron A. Collins.  */
/*                                                                            */
/* Converted for X Windows and arbitrary #of colours by Christopher J. Cason. */
/*                                                                            */
/* This will deliver approximate colorings using HSV values for the selection.*/
/* The palette map is divided into 4 parts - upper and lower half generated   */
/* with full and half "value" (intensity), respectively.  These halves are    */
/* further halved by full and half saturation values of each range (pastels). */
/* There are three constant colors, black, white, and grey.  They are used    */
/* when the saturation is low enough that the hue becomes undefined, and which*/
/* one is selected is based on a simple range map of "value".  Usage of the   */
/* palette is accomplished by converting the requested color RGB into an HSV  */
/* value.  If the saturation is too low (< .25) then black, white or grey is  */
/* selected.  If there is enough saturation to consider looking at the hue,   */
/* then the hue range of 1-63 is scaled into one of the 4 palette quadrants   */
/* based on its "value" and "saturation" characteristics.                     */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* define X_GETS_ARGS if you want to use X options such as '-display xxx:n.n' */
/*                                                                            */
/******************************************************************************/

#define DBL             double
#define NAME            "POV-Ray"

#include <stdio.h>
#include <math.h>
#include <x11/xlib.h>
#include <x11/xutil.h>
#include <x11/stringdefs.h>
#include <x11/intrinsic.h>
#include <x11/shell.h>
#include "config.h"

#define MIN_COLOURS     128
#define MAX_COLOURS     256

#define min(x,y)        ((x) < (y) ? (x) : (y))
#define max(x,y)        ((x) > (y) ? (x) : (y))

char                    filename [192] ;              /* don't really need it */
unsigned                gWidth ;                      /* +wxxx                */
unsigned                gHeight ;                     /* +hyyy                */
unsigned                gScreenNumber ;               /* screen number        */
unsigned                nColours ;                    /* colours available    */
unsigned                quarterPalette ;              /* 1/4 of nColours      */
unsigned                CooperationLevel = 5 ;        /* for message loop     */
unsigned                screenWidth ;                 /* of root screen       */
unsigned                screenHeight ;                /* ditto                */
unsigned long           gColours [MAX_COLOURS] ;      /* colour lookup        */
GC                      gGc ;                         /* graphics context     */
Screen                  *gScreen ;                    /* the X screen         */
XImage                  *gXimage = NULL ;             /* to buffer the trace  */
Widget                  gParent ;                     /* parent widget        */
Widget                  gPopupwindow ;                /* trace displayed here */
Window                  gWindow ;                     /* child of gPopupWin.. */
Display                 *gDisplay ;                   /* X display connection */
Colormap                gColourmap ;                  /* the colourmap        */
XtAppContext            gAppcontext ;                 /* application context  */

/* in POVRAY.C                                                                */
unsigned alt_main (unsigned argc, char *argv []) ;

/******************************************************************************/
/*                                                                            */
/* set a pixel in an XImage                                                   */
/*                                                                            */
/******************************************************************************/

void SetPixel (Display *display, XImage *ximage, Window window, GC gc, unsigned x, unsigned y, unsigned index)
{
  static unsigned       lastY = 0 ;

  if (lastY > y) lastY = y ;
  if (y != lastY)
  {
    /* for efficiency, only display the pixels after a full line is buffered  */
    XPutImage (display, window, gc, ximage, 0, lastY, 0, lastY, gWidth, gHeight) ;
    lastY = y ;
  }
  XPutPixel (ximage, x, y, gColours [index]) ;
}

/******************************************************************************/
/*                                                                            */
/* create an XImage                                                           */
/*                                                                            */
/******************************************************************************/

XImage *CreateXImage (Display *display, Visual *visual,
                      unsigned depth, unsigned width, unsigned height)
{
  unsigned    format ;
  unsigned    number_of_bytes ;
  XImage      *ximage ;

  format = depth == 1 ? XYBitmap : ZPixmap ;

  ximage = XCreateImage (display, visual, depth, format,
                         0, NULL, width, height, XBitmapPad (display), 0) ;

  if (ximage == NULL) return (NULL) ;

  number_of_bytes = ximage->bytes_per_line * ximage->height ;
  if ((ximage->data = malloc (number_of_bytes)) == NULL) return (NULL) ;
  return (ximage) ;
}

/******************************************************************************/
/*                                                                            */
/* create an XImage to be compatible with the supplied widget                 */
/*                                                                            */
/******************************************************************************/

XImage *CreateXImageFromWidget (Widget widget, unsigned width, unsigned height)
{
  unsigned    depth ;
  XImage      *ximage ;
  Visual      *visual ;
  Display     *display ;

  XtVaGetValues (widget, XtNdepth, &depth, XtNvisual, &visual, NULL) ;
  display = XtDisplay (widget) ;
  ximage = CreateXImage (display, visual, depth, width, height) ;
  return (ximage) ;
}

/******************************************************************************/
/*                                                                            */
/* make an XImage given a widget, width and height                            */
/*                                                                            */
/******************************************************************************/

XImage *MakeImage (Widget widget, unsigned width, unsigned height)
{
  unsigned              x ;
  unsigned              y ;
  XImage                *ximage ;

  if ((ximage = CreateXImageFromWidget (widget, width, height)) != NULL)
  {
    for (x = 0 ; x < width ; x++)
      for (y = 0 ; y < height ; y++)
        XPutPixel (ximage, x, y, gColours [0]) ;
  }

  return (ximage) ;
}

/******************************************************************************/
/*                                                                            */
/* redraw the traced data using the XImage                                    */
/*                                                                            */
/******************************************************************************/

void processExpose (XExposeEvent *expose)
{
  if (gXimage == NULL || (void *) gWindow == NULL) return ;
  XPutImage (gDisplay, gWindow, gGc, gXimage,
             expose->x, expose->y,
             expose->x, expose->y,
             expose->width, expose->height) ;
}

/******************************************************************************/
/*                                                                            */
/* this is called by the COOPERATE macro from within POVRAY                   */
/*                                                                            */
/******************************************************************************/

void XTraceEventHandler (void)
{
  XEvent                event ;
  static unsigned       cooperationCounter = 10 ;

  if (CooperationLevel < 10 && --cooperationCounter) return ;
  cooperationCounter = 11 - CooperationLevel ;

  if (CooperationLevel > 5)
  {
    /* loop until all messages retrieved */
    while (XtAppPending (gAppcontext))
    {
      XtAppNextEvent (gAppcontext, &event) ;
      if (event.type == Expose && event.xexpose.window == gWindow)
      {
        if (event.xexpose.count == 0) processExpose (&event.xexpose) ;
        return ;
      }
      XtDispatchEvent (&event) ;
    }
  }
  else
  {
    /* only process 1 event */
    if (XtAppPending (gAppcontext))
    {
      XtAppNextEvent (gAppcontext, &event) ;
      if (event.type == Expose && event.xexpose.window == gWindow)
      {
        if (event.xexpose.count == 0) processExpose (&event.xexpose) ;
        return ;
      }
      XtDispatchEvent (&event) ;
    }
  }
}

/******************************************************************************/
/*                                                                            */
/* called if the user closes the popup window during a trace                  */
/*                                                                            */
/******************************************************************************/

void destroyProc (Widget widget)
{
  XDestroyImage (gXimage) ;
  XFreeGC (gDisplay, gGc) ;
  gPopupwindow = NULL ;
}

/******************************************************************************/
/*                                                                            */
/* create a popup window, centered on the screen                              */
/*                                                                            */
/******************************************************************************/

Widget CreatePopupWindow (Widget parent, char *name, unsigned width, unsigned height, void (*destroyProc) (Widget))
{
  int         n ;
  unsigned    x = 0 ;
  unsigned    y = 0 ;
  Arg         args [8] ;
  Widget      window ;

  n = 0 ;
  if (width < screenWidth)
    x = screenWidth / 2 - width / 2 ;
  if (height < screenHeight)
    y = screenHeight / 2 - height / 2 ;
  XtSetArg (args [n], XtNwidth, width) ; n++ ;
  XtSetArg (args [n], XtNheight, height) ; n++ ;
  XtSetArg (args [n], XtNx, x) ; n++ ;
  XtSetArg (args [n], XtNy, y) ; n++ ;
  window = XtCreatePopupShell (name, topLevelShellWidgetClass, parent, args, n) ;
  XtAddCallback (window, XtNdestroyCallback, destroyProc, window) ;
  return (window) ;
}

/******************************************************************************/
/*                                                                            */
/* allocate a colour based on the RGB values and place it at index            */
/*                                                                            */
/******************************************************************************/

void set_palette (unsigned index, unsigned red, unsigned green, unsigned blue)
{
  XColor      colour ;

  colour.pixel = gColours [index] ;
  colour.flags = DoRed | DoGreen | DoBlue ;
  colour.red = (65535 * red) / 256 ;
  colour.blue = (65535 * blue) / 256 ;
  colour.green = (65535 * green) / 256 ;
  XStoreColor (gDisplay, gColourmap, &colour) ;
}

/******************************************************************************/
/*                                                                            */
/* Conversion from Hue, Saturation, Value to Red, Green, and Blue and back    */
/* From "Computer Graphics", Donald Hearn & M. Pauline Baker, p. 304          */
/* Extracted from the POV machine specific file IBM.C and modified for X      */
/*                                                                            */
/* See the start of this file for information as to how this code works       */
/*                                                                            */
/******************************************************************************/

void hsv_to_rgb (DBL hue, DBL s, DBL v, unsigned *r, unsigned *g, unsigned *b)
{
  DBL         i ;
  DBL         f ;
  DBL         p1 ;
  DBL         p2 ;
  DBL         p3 ;
  DBL         xh ;
  DBL         nr ;
  DBL         ng ;
  DBL         nb ;

  if (hue == 360.0) hue = 0.0 ;

  xh = hue / 60.0 ;               /* convert hue to be in 0..6 */
  i = floor (xh) ;                /* i = greatest integer <= h */
  f = xh - i ;                    /* f = fractional part of h  */
  p1 = v * (1 - s) ;
  p2 = v * (1 - (s * f)) ;
  p3 = v * (1 - (s * (1 - f))) ;

  switch ((int) i)
  {
    case 0 :
         nr = v ;
         ng = p3 ;
         nb = p1 ;
         break ;

    case 1 :
         nr = p2 ;
         ng = v ;
         nb = p1 ;
         break ;

    case 2 :
         nr = p1 ;
         ng = v ;
         nb = p3 ;
         break ;

    case 3 :
         nr = p1 ;
         ng = p2 ;
         nb = v ;
         break ;

    case 4 :
         nr = p3 ;
         ng = p1 ;
         nb = v ;
         break ;

    case 5 :
         nr = v ;
         ng = p1 ;
         nb = p2 ;
         break ;

    default :
         nr = ng = nb = 0 ;
  }
  *r = (unsigned) (nr * 255.0) ;
  *g = (unsigned) (ng * 255.0) ;
  *b = (unsigned) (nb * 255.0) ;
}

void rgb_to_hsv (unsigned r, unsigned g, unsigned b, DBL *h, DBL *s, DBL *v)
{
  DBL         m ;
  DBL         r1 ;
  DBL         g1 ;
  DBL         b1 ;
  DBL         nr ;
  DBL         ng ;
  DBL         nb ;
  DBL         nh = 0.0 ;
  DBL         ns = 0.0 ;
  DBL         nv ;

  nr = (DBL) r / 255.0 ;
  ng = (DBL) g / 255.0 ;
  nb = (DBL) b / 255.0 ;

  nv = max (nr, max (ng, nb)) ;
  m = min (nr, min (ng, nb)) ;

  if (nv != 0.0) ns = (nv - m) / nv ;

  if (ns == 0.0)
  {
    /* hue undefined if no saturation */
    *h = 0.0 ;
    *s = 0.0 ;
    *v = nv ;
    return ;
  }

  r1 = (nv - nr) / (nv - m) ;     /* distance of color from red   */
  g1 = (nv - ng) / (nv - m) ;     /* distance of color from green */
  b1 = (nv - nb) / (nv - m) ;     /* distance of color from blue  */

  if (nv == nr)
  {
    if (m == ng)
      nh = 5.0 + b1 ;
    else
      nh = 1.0 - g1 ;
  }

  if (nv == ng)
  {
    if (m == nb)
      nh = 1.0 + r1 ;
    else
      nh = 3.0 - b1 ;
  }

  if (nv == nb)
  {
    if (m == nr)
      nh = 3.0 + g1 ;
    else
      nh = 5.0 - r1 ;
  }

  *h = nh * 60.0 ;                /* return h converted to degrees */
  *s = ns ;
  *v = nv ;
}

/******************************************************************************/
/*                                                                            */
/* set up the X palette                                                       */
/* try to allocate at least as many colours as MIN_COLOURS                    */
/* exit with an error if we cannot get at least this many                     */
/* divide the available colour up into four quadrants, and divide each        */
/* quadrant up into specific hues from 0 to 359 degrees                       */
/*                                                                            */
/******************************************************************************/

void palette_init (void)
{
  unsigned    m ;
  unsigned    r ;
  unsigned    g ;
  unsigned    b ;
  DBL         hue ;
  DBL         sat ;
  DBL         val ;

  for (nColours = MAX_COLOURS ; nColours >= MIN_COLOURS ; nColours--)
    if (XAllocColorCells (gDisplay, gColourmap, False, NULL, 0, gColours, nColours))
      break ;
  if (nColours < MIN_COLOURS)
  {
    printf ("failed to allocate colour cells needed for display\r\n") ;
    exit (1) ;
  }

  quarterPalette = nColours / 4 ;

  /* for the first quarter of the palette ... */
  for (m = 1 ; m < quarterPalette ; m++)
  {
    /* normalise to 360 */
    hue = 360.0 * ((DBL) (m)) / (DBL) quarterPalette ;
    hsv_to_rgb (hue, 0.5, 0.5, &r, &g, &b) ;
    set_palette (m, r, g, b) ;

    hue = 360.0 * ((DBL) (m)) / (DBL) quarterPalette ;
    hsv_to_rgb (hue, 1.0, 0.5, &r, &g, &b) ;
    set_palette (m + quarterPalette, r, g, b) ;

    hue = 360.0 * ((DBL) (m)) / (DBL) quarterPalette ;
    hsv_to_rgb (hue, 0.5, 1.0, &r, &g, &b) ;
    set_palette (m + quarterPalette * 2, r, g, b) ;

    hue = 360.0 * ((DBL) (m)) / (DBL) quarterPalette ;
    hsv_to_rgb (hue, 1.0, 1.0, &r, &g, &b) ;
    set_palette (m + quarterPalette * 3, r, g, b) ;
  }

  set_palette (0, 0, 0, 0) ;                        /* black        */
  set_palette (quarterPalette, 255, 255, 255) ;     /* white        */
  set_palette (quarterPalette * 2, 128, 128, 128) ; /* dark grey    */
  set_palette (quarterPalette * 3, 192, 192, 192) ; /* light grey   */
}

/******************************************************************************/
/*                                                                            */
/* display code called directly by POV-Ray via [machine-name].C               */
/*                                                                            */
/******************************************************************************/

void x_display_finished ()
{
}

void x_display_init (unsigned width, unsigned height)
{
  unsigned              n ;
  unsigned              y = 0 ;
  unsigned              x = 0 ;
  unsigned long         white ;
  unsigned long         black ;
  Arg                   args [20] ;

  gDisplay = XtDisplay (gParent) ;
  gScreen = XtScreen (gParent) ;
  gScreenNumber = XScreenNumberOfScreen (gScreen) ;
  gColourmap = DefaultColormapOfScreen (gScreen) ;
  screenWidth = DisplayWidth (gDisplay, gScreenNumber) ;
  screenHeight = DisplayHeight (gDisplay, gScreenNumber) ;

  white = WhitePixel (gDisplay, gScreenNumber) ;
  black = BlackPixel (gDisplay, gScreenNumber) ;

  n = 0 ;
  x = screenWidth / 2 - gWidth / 2 ;
  y = screenHeight / 2 - gHeight / 2 ;
  XtSetArg (args [n], XtNx, x) ; n++ ;
  XtSetArg (args [n], XtNy, y) ; n++ ;
  XtSetValues (gParent, args, n) ;

  gWidth = width ;
  gHeight = height ;
  gPopupwindow = CreatePopupWindow (gParent, NAME, width, height, destroyProc) ;

  XtRealizeWidget (gPopupwindow) ;
  XtMapWidget (gPopupwindow) ;

  gWindow = XCreateSimpleWindow (gDisplay, XtWindow (gPopupwindow), 0, 0, width, height, 0, white, black) ;
  XSetWindowBackground (gDisplay, gWindow, black) ;
  XMapWindow (gDisplay, gWindow) ;
  XtPopup (gPopupwindow, XtGrabNonexclusive) ;

  palette_init () ;

  gGc = XCreateGC (gDisplay, gWindow, 0L, NULL) ;
  gXimage = MakeImage (gPopupwindow, width, height) ;

  /* draw a rectangle into the XImage to show trace progress */

  for (x = 0 ; x < width ; x++)
  {
    XPutPixel (gXimage, x, 0, white) ;
    XPutPixel (gXimage, x, height - 1, white) ;
  }

  for (y = 0 ; y < height ; y++)
  {
    XPutPixel (gXimage, 0, y, white) ;
    XPutPixel (gXimage, width - 1, y, white) ;
  }

  XSelectInput (gDisplay, gWindow, ExposureMask) ;
}

void x_display_close ()
{
  if (gPopupwindow)
    XtDestroyWidget (gPopupwindow) ;
}

void x_display_plot (unsigned x, unsigned y, char Red, char Green, char Blue)
{
  unsigned    colour ;
  DBL         h ;
  DBL         s ;
  DBL         v ;

  if (gPopupwindow == NULL) return ;

  rgb_to_hsv ((unsigned) Red, (unsigned) Green, (unsigned) Blue, &h, &s, &v) ;

  if (s < 0.20)         /* black or white if no saturation of colour ... */
  {
    if (v < 0.25)
      colour = 0 ;                  /* black      */
    else if (v > 0.8)
      colour = quarterPalette ;     /* white      */
    else if (v > 0.5)
      colour = quarterPalette * 3 ; /* light grey */
    else
      colour = quarterPalette * 2 ; /* dark grey  */
  }
  else
  {
    colour = (unsigned) ((DBL) quarterPalette * h / 360.0) ;

    if (colour == 0)
      colour = 1 ;                   /* avoid black, white or grey         */

    if (colour > quarterPalette)
      colour = quarterPalette ;      /* avoid same                         */

    if (v > 0.50)
      colour += quarterPalette * 2 ; /* colours 128-255 for high intensity */

    if (s > 0.50)                    /* more than half saturated ?         */
      colour += quarterPalette ;     /* colour range 64-128 or 192-255     */
  }

  /* only displays after a full line is received (the Y co-ordinate changes)  */
  SetPixel (gDisplay, gXimage, gWindow, gGc, x, y, colour) ;
}

/******************************************************************************/
/*                                                                            */
/* our main routine. when XWIND is linked in this is main () for POV-Ray      */
/*                                                                            */
/******************************************************************************/

int main (unsigned argc, char *argv [])
{
  unsigned              n ;
  unsigned              count = 0 ;
  Arg                   args [20] ;

  strncpy (filename, argv [0], sizeof (filename)) ;

  n = 0 ;
  XtSetArg (args [n], XtNmappedWhenManaged, False) ; n++ ;
  XtSetArg (args [n], XtNallowShellResize, False) ; n++ ;
  XtSetArg (args [n], XtNwidth, 10) ; n++ ;
  XtSetArg (args [n], XtNheight, 10) ; n++ ;

/* define X_GETS_ARGS if you want to use X options such as '-display xxx:n.n' */

#ifdef X_GETS_ARGS
  gParent = XtAppInitialize (&gAppcontext, NAME, NULL, 0, &argc, argv, NULL, args, n) ;
#else
  gParent = XtAppInitialize (&gAppcontext, NAME, NULL, 0, &count, NULL, NULL, args, n) ;
#endif

  /* call POV-Ray ! */
  alt_main (argc, argv) ;
  return (0) ;
}