/****************************************************************************
*                xwindows.c
*
*  This module implements X Window Display system specific routines.
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


/*
 * Original author: Marek Rzewuski, Avstikkeren 11, 1156 Oslo 11, Norway or
 * marekr@ifi.uio.no on Internet.
 */

/*
 * 91-07-29 Allan H. Wax (Wax.OSBU_South@Xerox.COM) - several improvements.
 */

/*
 * 94-08-19 Karl Schultz (kws@fc.hp.com) Rewrite and reorganization.
 * - added support for DirectColor and TrueColor visual classes.
 * - added Floyd-Steinberg dithering for 8-bit PseudoColor visual class.
 * - allocate private PseudoColor colormap if not enough cells left in default.
 * - fixed numerous bugs/problems.
 * - check for button press to leave program
 *
 * Version 5
 */

/*
 * 96-02-26 Andreas Dilger (adilger@enel.ucalgary.ca)  Rewrite and reorg.
 *
 * - removed dead code from previous generations of modifications.
 * - incorporated some enhancements from the 30.7.1993 version of Marek's code.
 *    - check for 'q' or 'Q' keypress to stop rendering if +x is specified
 *      or to quit if rendering is complete.
 *    - handle WM_DELETE_WINDOW for ICCCMness.
 *    - set the X error handler.
 * - added "Persistence of Vision" and the scene name to title bar.
 * - added support for visuals with depths other than 1, 8, and 24 bits.
 * - made dithering work properly for more arbitrary pixel drawing order.
 * - made dithering work with any type of visual, if needed.
 * - take grayscale output into account when generating colormap.
 * - added dynamic palette allocation (works well with other applications).
 * - handle the standard ICCCM command line arguments before POV does.
 * - set the X fatal I/O error handler.
 * - output pixels earlier than every scanline if rendering is slow.
 * - tried to make it portable from X11R3 to X11R6.
 * - make the display only open once for an animation.
 *
 * Version 6
 */

/*
 * System and X11 includes
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <math.h>

/*
 * POV specific includes
 */

#include "frame.h"
#include "parse.h"     /* For MAError */
#include "povproto.h"
#include "povray.h"
#include "xpovicon.xbm" /* This is the black & white POV icon */
#include "xpovmask.xbm" /* This is the transparency mask for the icon */

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* Level of debug output - 0 none, 1 some, 2 color info, 3 pixel output */
#define  X11_DEBUG 0

/*
 * The number of color cells on each side of theColorCube.  This number
 * cubed will give the total number of cells.  We don't need too many,
 * since there are usually only 256 colors available, but it allows us
 * to handle all the colormapped and grayscale visuals in the same way,
 * and to reduce aliasing problems if we have a wierd number of colors.
 */ 

#define theCUBESIZE 16
#define theGRAYSIZE 1024

#define theWAITTIME  10  /* Wait up to this many seconds to write to screen */
#define theQUERYFREQ 10  /* How often to check the colormap for updates */

/*
 * These are the default X application names, and the default long title and
 * icon name.  Override with -title <title> or -name <name>.
 */

#define theTITLE      "Persistence of Vision"
#define theICONNAME   "POV-Ray"     /* Short name for the icon */
#define theCLASS      "Povray"      /* Should begin with a capital */
#define theNAME       "povray"      /* Can be overridden with -name <name> */

/*
 * If USE_CURSOR is defined, we will get a watch cursor in the window
 * while we are still rendering, and it will disappear when finished.
 */

#ifdef USE_CURSOR
#  include <X11/cursorfont.h>
#endif

/*
 * What release of X11 we are using?  X11R5 and up should already define
 * this, but X11R3 and X11R4 don't, so it has to be specified here manually.
 * If it isn't set, assume X11R4.  If you have X11R3, then change it!!!
 */

#ifndef XlibSpecificationRelease
#define XlibSpecificationRelease 4
#endif

#define theEVENTMASK  (ButtonPressMask      | \
                       KeyPressMask         | \
                       ExposureMask         | \
                       StructureNotifyMask)

#define theRED   0
#define theGREEN 1
#define theBLUE  2

/* Which command-line parameters for the X Window Display have been set */
#define theCLASSSET    0x0001
#define theWIDTH       0x0004
#define theHEIGHT      0x0008
#define theSTARTX      0x0010
#define theSTARTY      0x0020
#define theNEGATIVEX   0x0040
#define theNEGATIVEY   0x0080
#define theICONSTATE   0x0100
#define thePRIVATECMAP 0x0200

/*****************************************************************************
* Local typedefs
******************************************************************************/


/*****************************************************************************
* Local variables
******************************************************************************/

/*
 * These are the command-line arguments that are handled for the X Windows
 * initialization.  They are parsed before the basic POV-Ray parameters so
 * that we can convert these settings into the relevant POV-Ray options if
 * needed, or remove them if not needed.
 */
static int           oargc = 0;                   /* Old argument count */
static char        **oargv = NULL;                /* Old argument list */
static int           nargc = 0;                   /* New argument count */
static char        **nargv = NULL;                /* New argument list */
static char         *theDisplayName = NULL;       /* display to open on */
static char         *theName = NULL;              /* internal name of program */
static char         *theTitle = NULL;             /* title to put on window */
static int           theClass = None;             /* visual class to use */
static int           theXVal;                     /* x position to start at */
static int           theYVal;                     /* y position to start at */
static int           theCurrY = 0;                /* the current row rendered */
static int           theMaxY = 0;                 /* the highest row rendered */
static int           theParameters = 0;           /* which parameters set */
static unsigned int  theBorderWidth = 4;          /* width of window borders */

/*
 * These are the settings for the actual display and window.
 */

static Display      *theDisplay = NULL;    /* which display on this system */
static int           theScreen;            /* which screen number of display */
static Window        theWindow;            /* the window on this screen */
static GC            theGC;                /* method of drawing on screen */
static XImage       *theImage = NULL;      /* off screen image storage */
static Atom          WM_PROTOCOLS;         /* a window manager protocol msg */
static Atom          WM_DELETE_WINDOW;     /* if window manager kills app */

/*
 * These are for the color quantization and dithering.
 */

static unsigned long theColorShift[3];      /* RGB offsets */
static unsigned long theColorSize[3];       /* RGB bits */
static Bool          fastbig;               /* if we have 8x8x8 RGB TrueColor */
static Bool          fastlittle;            /* if we have 8x8x8 BGR TrueColor */
static Colormap      theColormap = None;    /* our window's color map */
static unsigned int  theCells;              /* the number of colormap cells */
static int           theMaxCell = 1;        /* Maximum index we can allocate */
static unsigned long theMaxDist;            /* Maximum color error allowed */
static XColor       *theColorCube = NULL;   /* the color cube/gray ramp */
static XColor       *theColors = NULL;      /* the color map installed */
static int          *theErrors[3];          /* error diffusion storage arrays */
#ifdef USE_CURSOR
static Cursor        theCursor;
#endif

/*****************************************************************************
* Static functions
******************************************************************************/

static int      CheckArgs PARAMS((int argc, char **argv, char *match));
static void     RemoveArgs PARAMS((int *argc, char **argv, int index));
static void     GetBestColormap PARAMS((Visual *theVisual));
static void     GetBestGraymap PARAMS((Visual *theVisual));
static void     GetBestIndex PARAMS((long r, long g, long b, XColor *theCell));
static XColor  *GetBestColor PARAMS((long r, long g, long b));
static void     SelectVisual PARAMS((Visual **theVisual, unsigned int *theDepth));
static void     BuildColormap PARAMS((Visual *theVisual));
static Bool     HandleXEvents PARAMS((XEvent *theEvent, Bool finished));
static int      HandleXError PARAMS((Display *ErrorDisplay,
                                     XErrorEvent *ErrorEvent));
static int      HandleXIOError PARAMS((Display *ErrorDisplay));
static void     PlotPixel PARAMS((int x, int y, unsigned char Red,
                                  unsigned char Green, unsigned char Blue,
                                  unsigned char Alpha));


/*****************************************************************************
*
* FUNCTION  XWIN_init_povray
*
* INPUT     argc, argv - the command-line arguments
*
* OUTPUT
*
* RETURNS   argc, argv - command-line args with the X Windows specific
*                        arguments removed and/or converted into something
*                        corresponding to a POV-Ray option
*
* AUTHOR    Andreas Dilger
*
* DESCRIPTION  For X Windows, in order to be ICCCM compliant, the application
*              needs to handle certain command-line options.  Some of these
*              options can be converted to the equivalent POV-Ray options,
*              so that we can let POV handle them like we always have, some
*              are irrelevant.  The -fn (short for -font) option conflicts
*              with the POV-Ray for specifying PNG output format, so it will
*              be illegal, since I'd hate to have to figure out what is a font
*              name and what is an option.  As well, the -icon(ic) option can
*              potentially conflict with a scene name "con" or "conic".  It is
*              very unfortunate that these are reasonable scene names too.  It
*              will print out a warning if this option is tried, and there is
*              a scene file of this name in the current dir.  Actions will be:
*              
*              -bw WWW
*              -borderwidth WWW         -> override default border width
*              -disp <display name>,
*              -display <display name>  -> +d, save display name
*              -font                    -> discard with font name
*              -geom WWWxHHH+X+Y,
*              -geometry WWWxHHH+X+Y    -> +wWWW +hHHH, save X, Y location
*              -help                    -> X specific help
*              -icon, -iconic           -> save iconic state
*              -name                    -> save application name
*              -owncmap                 -> use a private colormap
*              -title                   -> save title bar name
*              -visual <visual type>    -> save visual type
*
*              As well, the application should register the command line with
*              the window manager in case it may be used, God only knows why.
*              This means saving a copy of the command line, since we don't
*              have any of the window stuff set up yet.
*
*              We don't use POV_MALLOC because this memory can't be freed
*              until after the call to mem_release_all() in Terminate_POV.
*
* CHANGES
*
*   Add option to set border width.  June 23, 1996  [AED]
*   Add option to force private colormap.   July 16, 1996 [AED]
*
******************************************************************************/
void XWIN_init_povray(argc, argv)
int *argc;
char **argv[];
{
  int index;

  /* These are the original args.  Since we can't free them (because we
   * didn't allocate them, and because a requirement for ICCCMness is to
   * tell the window manager our command-line args), we may as well save
   * these for later.
   */
  oargc = *argc;
  oargv = *argv;

  /* Make a new copy of the args which we are free to butcher */
  if ((nargv = (char **)malloc((oargc + 1) * sizeof(char *))) == NULL)
    MAError("X argv", (oargc + 1)*sizeof(char *));

  nargc = oargc;
  for (index = 0; index < nargc; index++)
  {
    if ((nargv[index] = (char *)malloc(strlen(oargv[index]) + 1)) == NULL)
      MAError("X argv[]", strlen(oargv[index]) + 1);

    strcpy(nargv[index], oargv[index]);
  }

  nargv[nargc] = NULL;

  while ((index = CheckArgs(nargc, nargv, "-help")) ||
         (index = CheckArgs(nargc, nargv, "--help")))
  {
    printf("usage: %s [+/-option ...] [INI_file] +I input_file\n", nargv[0]);
    printf("               [-display <display_name>] [-title <title>]\n");
    printf("               [-geometry <WIDTHxHEIGHT><+XOFF+YOFF>] [-iconic]\n");
    printf("               [-owncmap] [-visual <visual_type>]\n");
    printf("\nuse '%s -h' for a complete list of options.\n", nargv[0]);

    Terminate_POV(0);
  }

  /*
   * We can set the border width for those people without window managers,
   * or for window managers that allow windows with no borders to "hide"
   * in the bakground.
   */
  while ((index = CheckArgs(nargc, nargv, "-bw")) ||
         (index = CheckArgs(nargc, nargv, "-borderwidth")))
  {
    theBorderWidth = atoi(nargv[index + 1]);
    RemoveArgs(&nargc, nargv, index + 1);
    RemoveArgs(&nargc, nargv, index);
  }

  /*
   * This allows us to override the DISPLAY variable to display on
   * another comupter than the one rendering it.
   */
  while ((index = CheckArgs(nargc, nargv, "-disp")) ||
         (index = CheckArgs(nargc, nargv, "-display")))
  {
    if (theDisplayName != NULL)
      free(theDisplayName);

    /* If the next option starts with a '-' or a '+' then it is
     * actually another option, and the user didn't specify the
     * display.  Although this should be done with '+d', we may
     * as well check for it if people do it this way, and they do.
     * Doesn't help if we get an INI file as the next option, but
     * nothing can be done about this.
     */
    if (nargv[index + 1][0] != '+' && nargv[index + 1][0] != '-')
    {
      theDisplayName = nargv[index + 1];
      nargv[index + 1] = NULL;
      RemoveArgs(&nargc, nargv, index + 1);
#if (X11_DEBUG > 0)
      Debug_Info("Using display at %s\n", nargv[index + 1]);
#endif
    }
    else
    {
      theDisplayName = NULL;
#if (X11_DEBUG > 0)
      Debug_Info("Using the default display\n");
#endif
    }

    strcpy(nargv[index], "+d");
  }

  /*
   * We don't care about fonts, as we don't use them.
   */
  while ((index = CheckArgs(nargc, nargv, "-font")))
  {
    RemoveArgs(&nargc, nargv, index + 1);
    RemoveArgs(&nargc, nargv, index);
  }

  /*
   * Set the window geometry.  This also sets the image rendering size,
   * and conversely, changing the rendering size also changes the window
   * size.  We don't do anything clever (yet) for cases where the window
   * is actually larger than the display resolution.
   */
  while ((index = CheckArgs(nargc, nargv, "-geom")) ||
         (index = CheckArgs(nargc, nargv, "-geometry")))
  {
    int flag;
    unsigned int theWidth, theHeight;

#if (X11_DEBUG > 0)
    Debug_Info("Using geometry %s\n", nargv[index + 1]);
#endif

    flag = XParseGeometry(nargv[index + 1], &theXVal, &theYVal,
                                              &theWidth, &theHeight);

    if (flag & XValue)    theParameters |= theSTARTX;
    if (flag & YValue)    theParameters |= theSTARTY;
    if (flag & XNegative) theParameters |= theNEGATIVEX;
    if (flag & YNegative) theParameters |= theNEGATIVEY;

    if (flag & HeightValue)  /* Need to set or remove argv[index + 1] first */
    {
      free(nargv[index + 1]);
      if ((nargv[index + 1] = (char *)malloc((size_t)8)) == NULL)
        MAError("X argv[]", (size_t)8);

      sprintf(nargv[index + 1], "+h%d", theHeight);
      theParameters |= theWIDTH;
    }
    else
    {
      RemoveArgs(&nargc, nargv, index + 1);
    }

    if (flag & WidthValue)
    {
      free(nargv[index]);
      if ((nargv[index] = (char *)malloc(8)) == NULL)
        MAError("X argv[]", (size_t)8);
      
      sprintf(nargv[index], "+w%d", theWidth);
      theParameters |= theHEIGHT;
    }
    else
    {
      RemoveArgs(&nargc, nargv, index);
    }
  }

  /*
   * Start up as an icon, rather than a window.
   */
  while ((index = CheckArgs(nargc, nargv, "-icon")) ||
         (index = CheckArgs(nargc, nargv, "-iconic")))
  {
    theParameters |= theICONSTATE;
    RemoveArgs(&nargc, nargv, index);
  }

  /*
   * Change the application name.  However, since POV doesn't understand
   * any app-defaults yet, this is pretty much useless at this time.
   */
  while ((index = CheckArgs(nargc, nargv, "-name")))
  {
    if (theName != NULL)
      free(theName);

    theName = nargv[index];
    nargv[index] = NULL;

    RemoveArgs(&nargc, nargv, index + 1);
    RemoveArgs(&nargc, nargv, index);
  }

  /*
   * Force POV-Ray to have a private colormap.  I don't know if people
   * would want this, but here it is.
   */
  while ((index = CheckArgs(nargc, nargv, "-owncmap")))
  {
    theParameters |= thePRIVATECMAP;
    RemoveArgs(&nargc, nargv, index);
  }

  /*
   * Set the title on the window to something other than POV-Ray:<scene>.
   * Note that if you set the title manually, you don't get the scene name.
   */
  while ((index = CheckArgs(nargc, nargv, "-title")))
  {
    if (theTitle != NULL)
      free(theTitle);

    theTitle = nargv[index + 1];
    nargv[index + 1] = NULL;

    RemoveArgs(&nargc, nargv, index + 1);
    RemoveArgs(&nargc, nargv, index);
  }

  /*
   * Override the default visual type.  While this has been good for me
   * during debugging, I don't know why anyone would actually need it as
   * POV is pretty good at finding the best visual, and using it well.
   */
  while ((index = CheckArgs(nargc, nargv, "-visual")))
  {
    theParameters |= theCLASSSET;

#if (X11_DEBUG > 0)
    Debug_Info("Using %s visual\n", nargv[index + 1]);
#endif

    if (!pov_stricmp(nargv[index + 1], "StaticGray"))
      theClass = StaticGray;
    else if (!pov_stricmp(nargv[index + 1], "GrayScale"))
      theClass = GrayScale;
    else if (!pov_stricmp(nargv[index + 1], "StaticColor"))
      theClass = StaticColor;
    else if (!pov_stricmp(nargv[index + 1], "PseudoColor"))
      theClass = PseudoColor;
    else if (!pov_stricmp(nargv[index + 1], "TrueColor"))
      theClass = TrueColor;
    else if (!pov_stricmp(nargv[index + 1], "DirectColor"))
      theClass = DirectColor;
    else
    {
      theParameters ^= theCLASSSET;
    }

    RemoveArgs(&nargc, nargv, index + 1);
    RemoveArgs(&nargc, nargv, index);
  }

  *argv = nargv;
  *argc = nargc;
}


/*****************************************************************************
*
* FUNCTION   CheckArgs
*
* INPUT      argc - number of arguments in argv
*            argv - list of arguments
*            match - the argument to find in argv
*
* OUTPUT
*
* RETURNS    index number of match in argv, or 0 if not found
*
* AUTHOR     Andreas Dilger
*
* DESCRIPTION  Tries to find the given option in argv
*
* CHANGES
*
******************************************************************************/

static int CheckArgs(argc, argv, match)
int argc;
char *argv[];
char *match;
{
  int index;
  
  /* If we get to argv[0] we haven't found the desired string */
  for (index = 1; index < argc; index++)
  {
    if (!strcmp(argv[index], match))
      break;
  }

  if (index == argc)
    index = 0;

  return(index);
}

/*****************************************************************************
*
* FUNCTION   RemoveArgs
*
* INPUT      argc - number of arguments in argv
*            argv - list of arguments
*            index - the argument to remove from argv
*
* OUTPUT
*
* RETURNS
*
* AUTHOR       Andreas Dilger
*
* DESCRIPTION  Removes the given option from argv and shuffles the rest down
*
* CHANGES
*
******************************************************************************/

static void RemoveArgs(argc, argv, index)
int *argc;
char *argv[];
int index;
{
  if (index >= *argc || index == 0)
    return;

  if (argv[index] != NULL)
  {
    free(argv[index]);
  } 

  for (; index < *argc; index++)
  {
    argv[index] = argv[index + 1];
  }
  (*argc)--;
}



/*****************************************************************************
*
* FUNCTION   XWIN_finish_povray
*
* INPUT      exit status
*
* OUTPUT
*
* RETURNS
*
* AUTHOR       Andreas Dilger
*
* DESCRIPTION  Frees memory allocated during XWIN_init_povray and during
*              XWIN_display_init.
*
* CHANGES
*
*   Freeing of memory and X display stuff moved here from so that we don't
*   need to re-initialize for each frame of an animation. July 1996, [AED]
*
******************************************************************************/

void XWIN_finish_povray(status)
int status;
{
  int i;

  if (theDisplay)
  {
    if (theImage->data != NULL)
    {
      free(theImage->data);
      theImage->data = NULL;
    }

    XDestroyImage(theImage);
    theImage = NULL;

    XDestroyWindow(theDisplay, theWindow);

    if (theColormap != None)
    {
      XFreeColormap(theDisplay, theColormap);
    }
    theMaxCell = 1;

    XFreeGC(theDisplay, theGC);

    XCloseDisplay(theDisplay);
    theDisplay = NULL;
  }

  for (i = 0; i < 3; i++)
  {
    if (theErrors[i] != NULL)
    {
      free(theErrors[i]);
      theErrors[i] = NULL;
    }
  }

  if (theColorCube != NULL)
  {
    free(theColorCube);
    theColorCube = NULL;
  }

  if (theColors != NULL)
  {
    free(theColors);
    theColors = NULL;
  }

  if (nargv != NULL)
  {
    for (i = 0; i < nargc; i++)
    {
      if (nargv[i] != NULL)
        free(nargv[i]);
    }
    free(nargv);
  }

  if (theTitle != NULL)
    free(theTitle);
  
  if (theName != NULL)
    free(theName);

  if (theDisplayName != NULL)
    free(theDisplayName);

  exit(status);
}


/*****************************************************************************
*
* FUNCTION  XWIN_display_init
*
* INPUT  width, height - size of desired window
*
* OUTPUT
*
* RETURNS
*    global variables theGC, theImage, theErrors set up
*
* AUTHOR
*
* DESCRIPTION
*
*   Create the X window, using the best Visual and Colormap.  Sets the
*   window properties, size, icon, and error handling routines.
*
* CHANGES
*
*   Handle the usual X command-line setting values.  [AED]
*   Set up the window title to hold the scene name.  [AED]
*
******************************************************************************/
void XWIN_display_init(width, height)
int width, height;
{
  Visual              *theVisual = NULL;
  unsigned int         theDepth;
  XSetWindowAttributes theWindowAttributes;
  unsigned long        theWindowMask;
  XSizeHints           theSizeHints;
  Pixmap               theIcon;
  Pixmap               theMask;
#if (XlibSpecificationRelease > 3)
  XWMHints             theWMHints;
#endif
  XClassHint           theClassHints;
  XGCValues            theGCValues;
  int                  theDispWidth, theDispHeight;
  long                 i;
  size_t               size;

  /* If we have already set up the display, don't do it again */
  if (theDisplay != NULL)
    return;

  if ((theDisplay = XOpenDisplay(theDisplayName)) == NULL)
  {
    if (strlen(XDisplayName(theDisplayName)) == 0)
    {
      Error_Line("\nDISPLAY variable is not set, and you are not specifying");
      Error_Line("\na display on the command-line with -display.");
    }
    else
    {
      Error_Line("\nThe display '%s' is not a valid display,",
                 XDisplayName(theDisplayName));
      Error_Line("\nor you do not have permission to open a window there.");
    }
    Error("\nError opening X display.\n");
  }

  /*
   * set up the non-fatal and fatal X error handlers
   */

#if (X11_DEBUG > 0)
  Debug_Info("\nActually displaying at %s\n", XDisplayName(theDisplayName));
#else
  XSetErrorHandler(HandleXError);
  XSetIOErrorHandler(HandleXIOError);
#endif

  theScreen     = DefaultScreen(theDisplay);
  theDispWidth  = DisplayWidth(theDisplay, theScreen);
  theDispHeight = DisplayHeight(theDisplay, theScreen);

  SelectVisual(&theVisual, &theDepth);

  BuildColormap(theVisual);

  /*
   * Accept only Expose events while creating the window.
   * It is reccommended to accept these events, in case we miss them
   * before we begin to accept all the other events.  Also ask for a
   * backing store so we don't have to refresh the window all the time.
   */

  theWindowAttributes.backing_store    = WhenMapped;
  theWindowAttributes.background_pixel = BlackPixel(theDisplay, theScreen);
  theWindowAttributes.border_pixel     = BlackPixel(theDisplay, theScreen);
  theWindowAttributes.colormap         = theColormap;
  theWindowAttributes.event_mask       = ExposureMask;
  theWindowMask = CWBackingStore | CWBackPixel | CWBorderPixel |
                  CWColormap | CWEventMask;

#ifdef USE_CURSOR
  if ((theCursor = XCreateFontCursor(theDisplay,XC_watch))!=(Cursor)None)
  {
    theWindowAttributes.cursor = theCursor;
    theWindowMask |= CWCursor;
  }
#endif

  /*
   * take the user starting geometry into account
   */

  if (theParameters & theSTARTX)
  {
    if (theParameters & theNEGATIVEX)
    {
      theXVal = theDispWidth - Frame.Screen_Width - theXVal - theBorderWidth;
      theParameters ^= theNEGATIVEX;
    }
  }
  else
  {
    theXVal = (theDispWidth - Frame.Screen_Width) / 2 - theBorderWidth;
  }

  if (theXVal < 0)   /* Make sure the title bar is on the screen */
    theXVal = 0;

  if (theParameters & theSTARTY)
  {
    if (theParameters & theNEGATIVEY)
    {
      theYVal = theDispHeight - Frame.Screen_Height - theYVal - theBorderWidth;
      theParameters ^= theNEGATIVEY;
    }
  }
  else
  {
    theYVal = (theDispHeight - Frame.Screen_Height) / 2 - theBorderWidth;
  }

  if (theYVal < 0)   /* Make sure the title bar is on the screen */
    theYVal = 0;

  theWindow = XCreateWindow(theDisplay, RootWindow(theDisplay, theScreen),
                            theXVal, theYVal,
                            (unsigned int)Frame.Screen_Width,
                            (unsigned int)Frame.Screen_Height,
                            theBorderWidth, (int)theDepth, InputOutput,
                            theVisual, theWindowMask, &theWindowAttributes);

  /*
   * tell the window manager what size of window we want
   */

#if (XlibSpecificationRelease > 3)
  theSizeHints.base_width   =
#endif
  theSizeHints.width        =                     /* Obsolete */
  theSizeHints.min_width    =
  theSizeHints.max_width    =
  theSizeHints.min_aspect.x =
  theSizeHints.max_aspect.x = Frame.Screen_Width;
#if (XlibSpecificationRelease > 3)
  theSizeHints.base_height  =
#endif
  theSizeHints.height       =                     /* Obsolete */
  theSizeHints.min_height   =
  theSizeHints.max_height   =
  theSizeHints.min_aspect.y =
  theSizeHints.max_aspect.y = Frame.Screen_Height;
  theSizeHints.flags = PSize | PMinSize | PMaxSize | PAspect;
  if (theParameters & theWIDTH || theParameters & theHEIGHT)
  {
    theSizeHints.flags     |= USSize;
  }
  if (theParameters & theSTARTX || theParameters & theSTARTY)
  {
    theSizeHints.x          = theXVal;            /* Obsolete */
    theSizeHints.y          = theYVal;            /* Obsolete */
    theSizeHints.flags     |= PPosition | USPosition;
  }

#if (XlibSpecificationRelease > 3)
  theSizeHints.flags |= PBaseSize;

  XSetWMNormalHints(theDisplay, theWindow, &theSizeHints);
#else
  XSetNormalHints(theDisplay, theWindow, &theSizeHints);
#endif

  /*
   * This is the actual name of the application to the window manager,
   * for purposes of defaults and such.  Not to be confused with the
   * name on the title bar or the icon, which is just fluff.
   */

  theClassHints.res_name  = theName == NULL ? theNAME : theName;
  theClassHints.res_class = theCLASS;

  XSetClassHint(theDisplay, theWindow, &theClassHints);

  /*
   * make an icon to attatch to the window
   */

  theIcon = XCreateBitmapFromData(theDisplay, theWindow, (char *)xpovicon_bits,
                                  xpovicon_width, xpovicon_height);

  theMask = XCreateBitmapFromData(theDisplay, theWindow, (char *)xpovmask_bits,
                                  xpovmask_width, xpovmask_height);

  /*
   * tell the window manager what to do with the icon
   */

  theWMHints.icon_pixmap   = theIcon;
  theWMHints.input         = True;
  theWMHints.initial_state = theParameters&theICONSTATE?IconicState:NormalState;
  theWMHints.flags         = IconPixmapHint|InputHint|StateHint|IconMaskHint;
  theWMHints.icon_mask     = theMask;

  XSetWMHints(theDisplay, theWindow, &theWMHints);

  XSetIconName(theDisplay, theWindow, theICONNAME);

  /*
   * tell the window manager what title to use for this window
   */

  if (theTitle == NULL)
  {
    size_t len = strlen(theTITLE) + strlen(opts.Scene_Name) + 2;

    /*
     * Assuming characters 10 pixels wide, is the long title too long
     * for the window?  If so, use the shorter icon title for the
     * window too.  We use malloc() instead of POV_MALLOC here because
     * we may have used malloc() on for theTitle in XWIN_povray_init(),
     * and we should be able to free() this without causing a POV_MALLOC
     * error or a memory leak message.
     */
    if (len * 10 > Frame.Screen_Width)
    {
      len = strlen(theICONNAME) + strlen(opts.Scene_Name) + 2;
      if ((theTitle = (char *)malloc(len + 1)) == NULL)
        MAError("X Title", len + 1);
      sprintf(theTitle, "%s: %s", theICONNAME, opts.Scene_Name);
    }
    else
    {
      if ((theTitle = (char *)malloc(len + 1)) == NULL)
        MAError("X Title", len + 1);
      sprintf(theTitle, "%s: %s", theTITLE, opts.Scene_Name);
    }
  }

  XStoreName(theDisplay, theWindow, theTitle);

  /*
   * tell the window manager about the command-line parameters for ICCCM
   */

  XSetCommand(theDisplay, theWindow, oargv, oargc);

  /*
   * create a graphics context for drawing
   */

  theGCValues.function = GXcopy;

  theGC = XCreateGC(theDisplay, theWindow, GCFunction, &theGCValues);

  /*
   * Now that we are finished setting everything up, we will begin
   * handling input for this window.
   */
    
  XSelectInput(theDisplay, theWindow, theEVENTMASK);

  /*
   * This tells the window manager that we want to know if the window is
   * being killed so we can catch it and do an orderly shutdown.
   */

  WM_DELETE_WINDOW = XInternAtom(theDisplay, "WM_DELETE_WINDOW", False);
  WM_PROTOCOLS = XInternAtom(theDisplay, "WM_PROTOCOLS", False);
  
#if (XlibSpecificationRelease > 3)
  XSetWMProtocols(theDisplay, theWindow, &WM_DELETE_WINDOW, 1);
#else
  XChangeProperty(theDisplay, theWindow, WM_PROTOCOLS, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)&WM_DELETE_WINDOW, 1);
#endif

  /*
   * Now, could we please see the window on the screen?  Until now, we have
   * dealt with a window which has been created but has not appeared on the
   * screen.  Mapping the window places it visibly on the screen.
   */

  XMapWindow(theDisplay, theWindow);

  /*
   * Allocate and create XImage to save the image if it gets covered
   */

  theImage = XCreateImage(theDisplay, theVisual, theDepth, ZPixmap, 0, NULL,
                          (unsigned int)Frame.Screen_Width,
                          (unsigned int)Frame.Screen_Height,
                          BitmapPad(theDisplay), 0);

  size = theImage->bytes_per_line * Frame.Screen_Height;

  if ((theImage->data = (char *)calloc(1, size)) == NULL)
    MAError("X image data", size);

  /*
   * Allocate space for dithering option
   */

  for (i = 0; i < 3; i++)
  {
    if (fastlittle == TRUE || fastbig == TRUE)
    {
      theErrors[i] = NULL;
    }
    else
    {
      if ((theErrors[i] = (int *)calloc(Frame.Screen_Width, sizeof(int))) == NULL)
        MAError("X dither buffer", Frame.Screen_Width * sizeof(int));
    }
  }

  XFlush(theDisplay);
}


/*****************************************************************************
*
* FUNCTION  PlotPixel
*
* INPUT
*
*    x, y coordinates to plot the pixel
*    red, green, blue, alpha values in the range [0, 255] to plot
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Andreas Dilger, Feb 1996
*
* DESCRIPTION
*
*   This routine plots the pixel to the off-screen image storage.
*
*   Although we are using Floyd-Steinberg dithering, the implementation
*   is slightly unusual, since we sometimes call this routine to plot
*   pixels when drawing blocks to the screen instead of scanlines.  This
*   caused problems with the error accumulation, since we never cleared
*   the errors until the end of the scanline.  To avoid the scanline/block
*   problem there is only a single error accumulation array, which is
*   filled in as we traverse the scanline.  This neccesitates an extra
*   pixel of error accumulation, for the error shifted to the lower right.
*   However, we save on clearing and swapping the arrays after every scanline.
*
*   There is a slight discontinuity for blocks that are not drawn in the
*   regular top-down, left-to-right raster order, but this is minimal
*   compared to the problems with the previous two-array implementation.
*
* CHANGES
*
******************************************************************************/

#ifdef __STDC__
static void     PlotPixel(int x, int y, unsigned char Red, unsigned char Green,
                          unsigned char Blue, unsigned char Alpha)
#else
static void PlotPixel(x, y, Red, Green, Blue, Alpha)
int x, y;
unsigned char Red, Green, Blue, Alpha;
#endif
{
  unsigned long thePixel = 0;

  /*
   * If we have a full 8 bits for each color, we can just output
   * directly to the screen, because we only get 8 bits of color
   * data anyways, hence no errors and no need to mess with all
   * the dithering stuff.  Otherwise, we accumulate the color errors,
   * even for TrueColor, in case we have very few colors available
   * like a 332 (8-bit) or 444 (Color NeXT 12-bit).
   */

  if (fastlittle)
  {
    thePixel = (Red << 16) | (Green << 8) | (Blue);
  }
  else if (fastbig)
  {
    thePixel = (Blue << 16) | (Green << 8) | (Red);
  }
  else
  {
    XColor     *theColor;
    long       wantColor[3], gotColor[3];
    int        i;

    /*
     * Add in accumulated errors at this location to the desired color
     * to compensate for previous innacuracies in the displayed color.
     */

    wantColor[theRED]   = (long)Red   * 0x100 + theErrors[theRED][x];
    wantColor[theGREEN] = (long)Green * 0x100 + theErrors[theGREEN][x];
    wantColor[theBLUE]  = (long)Blue  * 0x100 + theErrors[theBLUE][x];

#if (X11_DEBUG > 2)
    Debug_Info("\norig[%5d %5d %5d] + err[%5d %5d %5d]->",
             (long)Red * 0x100, (long)Green * 0x100, (long)Blue * 0x100,
             theErrors[theRED][x],theErrors[theGREEN][x],theErrors[theBLUE][x]);
    Debug_Info("want[%5d %5d %5d]->\n",
             wantColor[theRED], wantColor[theGREEN], wantColor[theBLUE]);
#endif

    /* clamp the colors to the valid range */

    for (i = 0; i < 3; i++)
    {
      wantColor[i] = (wantColor[i] < 0) ? 0 :
                     ((wantColor[i] > 0xffff) ? 0xffff : wantColor[i]);
    }

    switch (theClass)
    {
      case StaticGray:
      case GrayScale:
      case StaticColor:
      case PseudoColor:

        /* get the best color match for the calculated pixel value */

        theColor = GetBestColor(wantColor[theRED],
                                wantColor[theGREEN],
                                wantColor[theBLUE]);
        thePixel = theColor->pixel;

        gotColor[theRED]   = theColor->red;
        gotColor[theGREEN] = theColor->green;
        gotColor[theBLUE]  = theColor->blue;
        break;

      case TrueColor:
      case DirectColor:

        /*
         * First work out the color in the range needed for this visual.
         */

        gotColor[theRED]  = wantColor[theRED]  *theColorSize[theRED]  /0x10000;
        gotColor[theGREEN]= wantColor[theGREEN]*theColorSize[theGREEN]/0x10000;
        gotColor[theBLUE] = wantColor[theBLUE] *theColorSize[theBLUE] /0x10000;

        thePixel=((gotColor[theRED]   << theColorShift[theRED]  ) |
                  (gotColor[theGREEN] << theColorShift[theGREEN]) |
                  (gotColor[theBLUE]  << theColorShift[theBLUE] ));

        /*
         * Now, convert it back to a color in the range of [0-0xffff], so
         * we can add any truncation effects that were encountered
         * during the color conversion back into the error array.
         */

        gotColor[theRED]  = gotColor[theRED]  * 0x10000/theColorSize[theRED];
        gotColor[theGREEN]= gotColor[theGREEN]* 0x10000/theColorSize[theGREEN];
        gotColor[theBLUE] = gotColor[theBLUE] * 0x10000/theColorSize[theBLUE];
        break;
    }

#if (X11_DEBUG > 2)
    Debug_Info("    got[%5d %5d %5d] -> error[",
               gotColor[theRED], gotColor[theGREEN], gotColor[theBLUE]);
#endif

    /*
     * Compute errors and accumulate in error diffusion array.  As well,
     * since we are keeping all the errors in a single array, things are
     * a little different than usual.  The error array from x = 0 to the
     * current x value are for the next scanline, and the errors to the
     * right are for the current scanline.  This means we have to keep
     * the error for the lower right pixel in a separate location until
     * we use the error for the pixel to the right.
     */

    for (i = 0; i < 3; i++)
    {
      static long drError[3] = {0, 0, 0};
      long error;

      /* find the error between the desired color and actually used color */

      error = (wantColor[i] - gotColor[i] + 8) / 16;

#if (X11_DEBUG > 2)
      Debug_Info("%5d ", error * 16);
#endif
      if (x > 0)
      {
        theErrors[i][x - 1] += 3 * error;            /* down & left */
      }

      theErrors[i][x] = 5 * error + drError[i];      /* down */

      if (x < Frame.Screen_Width - 1)
      {
        drError[i] = error;                          /* down & right */
        theErrors[i][x + 1] += 7 * error;            /* right */
      }
      else
      {
        drError[i] = 0;         /* next scanline start with no error */
      }
    }
#if (X11_DEBUG > 2)
    Debug_Info("]\n");
    i = fgetc(stdin);
#endif
  }

  /*
   * Put calculated pixel in the off-screen image storage.
   */

  XPutPixel(theImage, x, y, thePixel);

  if (y > theMaxY)
  {
    theMaxY = y;
  }
  theCurrY = y;
}



/*****************************************************************************
*
* FUNCTION   XWIN_display_plot
*
* INPUT   x, y location of pixel
*         Red, Green, Blue, Alpha color components of pixel
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*    It is too slow to write each pixel to the screen as it is generated. For
*    simple pictures, we spend far too much time drawing on the screen instead
*    of raytracing. So, we'll update the window with the contents of the image
*    when we reach the end of the scanline, or if we have been rendering for
*    several seconds without an update.
*
*    The different prototype declarations are because some compilers don't
*    like mixing ANSI and K&R declarations for some variable types.
*
* CHANGES
*
******************************************************************************/

#ifdef __STDC__
void XWIN_display_plot(int x, int y, unsigned char Red, unsigned char Green,
                       unsigned char Blue, unsigned char Alpha)
#else
void XWIN_display_plot(x, y, Red, Green, Blue, Alpha)
int x, y;
unsigned char Red, Green, Blue, Alpha;
#endif
{
  static DBL lasttime = 0, thistime = 0, deltatime = 0;

  PlotPixel(x, y, Red, Green, Blue, Alpha);

  if (x % 32 == 31)
  {
    STOP_TIME
    thistime = TIME_ELAPSED
    deltatime = thistime - lasttime;
  }

  if (x >= opts.Last_Column - 1 || deltatime > theWAITTIME)
  {
    XPutImage(theDisplay, theWindow, theGC, theImage, 0, y, 0, y,
              (unsigned int)x + 1, 1);

    XFlush(theDisplay);
    lasttime = thistime;
    deltatime = 0;
  }
}


/*****************************************************************************
*
* FUNCTION   XWIN_display_plot_rect
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Andreas Dilger
*
* DESCRIPTION
*
*    It is too slow to write each pixel to the screen as it is generated. For
*    simple pictures, we spend far too much time drawing on the screen instead
*    of raytracing. So, we'll update the window with the contents of the image
*    when we reach the end of the scanline, if we have just output a large
*    block, or if we have been rendering for several seconds without an update,
*    but only if we can see it.  We call the PlotPixel routine for each pixel
*    rather than just drawing a block, as this allows us to dither the large
*    blocks if we don't have the exact color.
*
*    We reduce the dithering errors to the left and the right of the block
*    just displayed because the errors accumulate there for each row in the
*    block, but they only get "used" by the next block to the right or the
*    block to the lower left.  By dividing the errors by the height of the
*    block just displayed, we "average" the error from the previous block
*    for the edge of the next block.
*
* CHANGES
*
*    Sept 2, 1996: Reduce error to left of block by height. [AED]
*
******************************************************************************/

#ifdef __STDC__
void XWIN_display_plot_rect(int x1, int x2, int y1, int y2, unsigned char Red,
                unsigned char Green, unsigned char Blue, unsigned char Alpha)
#else
void XWIN_display_plot_rect(x1, x2, y1, y2, Red, Green, Blue, Alpha)
int x1, x2, y1, y2;
unsigned char Red, Green, Blue, Alpha;
#endif
{
  static DBL lasttime = 0, thistime = 0;
  unsigned int width, height;
  int i, j;

  width = x2 - x1 + 1;
  height = y2 - y1 + 1;

  for (j = y1; j <= y2; j++)
  {
    for (i = x1; i <= x2; i++)
    {
      PlotPixel(i, j, Red, Green, Blue, Alpha);
    }
  }

  if (fastlittle == FALSE && fastbig == FALSE )
  {
    /* Fix up the error to the right of the block */
    if (x2 < Frame.Screen_Width - 1)
    {
      for (i = 0; i < 3; i++)
      {
        theErrors[i][x2 + 1] /= (int)height;
      }
    }

    /* Fix up the error to the left of the block */
    if (x1 > 0)
    {
      for (i = 0; i < 3; i++)
      {
        theErrors[i][x1 - 1] /= (int)height;
      }
    }
  }

  STOP_TIME
  thistime = TIME_ELAPSED

  if (height > 8)
  {
    XPutImage(theDisplay, theWindow, theGC, theImage,
              x1, y1, x1, y1, width, height);
    XFlush(theDisplay);

    lasttime = thistime;
  }
  else if (x2 >= opts.Last_Column - 1 || thistime - lasttime > theWAITTIME)
  {
    XPutImage(theDisplay, theWindow, theGC, theImage,
              0, y1, 0, y1, (unsigned int)x2 + 1, height);
    XFlush(theDisplay);

    lasttime  = thistime;
  }
}



/*****************************************************************************
*
* FUNCTION   XWIN_display_plot_box
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR     Andreas Dilger
*
* DESCRIPTION
*
*    Draw a rectangular hollow box on the screen.  Because we are drawing
*    directly to the window, and not the image store, we don't need to
*    refresh the screen after drawing.  However, if we do a refresh of the
*    whole screen, the boxes will then be lost.  Oh well.
*
* CHANGES     Mar 1996: Creation
*
******************************************************************************/

#ifdef __STDC__
void XWIN_display_plot_box(int x1, int y1, int x2, int y2, unsigned char Red,
                unsigned char Green, unsigned char Blue, unsigned char Alpha)
#else
void XWIN_display_plot_box(x1, y1, x2, y2, Red, Green, Blue, Alpha)
int x1, y1, x2, y2;
unsigned char Red, Green, Blue, Alpha;
#endif
{
  long thePixel = 0;

  if (fastlittle)
  {
    thePixel = (Red << 16) | (Green << 8) | (Blue);
  }
  else if (fastbig)
  {
    thePixel = (Blue << 16) | (Green << 8) | (Red);
  }
  else
  {
    XColor     *theColor;
    long       gotColor[3];

    switch (theClass)
    {
      case StaticGray:
      case GrayScale:
      case StaticColor:
      case PseudoColor:

        /* get the best color match for the calculated pixel value */

        theColor = GetBestColor((long)Red * 0x100, (long)Green * 0x100,
                                (long)Blue * 0x100);
        thePixel = theColor->pixel;
        break;

      case TrueColor:
      case DirectColor:

        /*
         * First work out the color in the range needed for this visual.
         */

        gotColor[theRED]   = Red   * theColorSize[theRED]  / 0x100;
        gotColor[theGREEN] = Green * theColorSize[theGREEN]/ 0x100;
        gotColor[theBLUE]  = Blue  * theColorSize[theBLUE] / 0x100;

        thePixel=((gotColor[theRED]   << theColorShift[theRED]  ) |
                  (gotColor[theGREEN] << theColorShift[theGREEN]) |
                  (gotColor[theBLUE]  << theColorShift[theBLUE] ));
    }
  }
  XDrawRectangle(theDisplay, theWindow, theGC, x1, y1,
                 (unsigned int)x2-x1+1, (unsigned int)y2-y1+1);
}



/*****************************************************************************
*
* FUNCTION  XWIN_display_finished
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Wait for user to press a mouse button or the 'q' key in the rendering
*   window, then return to the caller.
*
* CHANGES
*
******************************************************************************/

void XWIN_display_finished()
{
  if (theDisplay && opts.Options & PROMPTEXIT)
  {
    Bool   finished = False;

    Status_Info("\nClick on window to exit...");

#ifdef USE_CURSOR
    /* Get rid of the watch cursor now that we don't need it anymore */
    XUndefineCursor(theDisplay, theWindow);
    XFreeCursor(theDisplay, theCursor);
#endif

    /* Sit and handle events until we quit, but don't busy wait */
    while (!finished)
    {
      XEvent theEvent;

      XNextEvent(theDisplay, &theEvent);
      finished = HandleXEvents(&theEvent, True);
    }
  }
}


/*****************************************************************************
*
* FUNCTION  XWIN_Test_Abort
*
* INPUT
*
* OUTPUT
*
* RETURNS  whether or not the user wants to quit
*
* AUTHOR   Andreas Dilger, Feb 1996
*
* DESCRIPTION
*
*   This is the polling user abort checker called by POV-Ray for TEST_ABORT
*   if the EXITENABLE flag is set (+x).  This is called every pixel, or less
*   often if the +Xnnn switch is set to test after every nnn pixels.
*
* CHANGES
*
******************************************************************************/

int XWIN_Test_Abort()
{
  XEvent theEvent;

  if (theDisplay && 
      XCheckWindowEvent(theDisplay, theWindow, theEVENTMASK, &theEvent))
  {
    return (HandleXEvents(&theEvent, False) ? TRUE : FALSE);
  }

  return FALSE;
}


/*****************************************************************************
*
* FUNCTION  HandleXError
*
* INPUT
*
* OUTPUT
*
* RETURNS  the return value is ignored
*
* AUTHOR   Andreas Dilger, Feb 1996
*
* DESCRIPTION
*
*   This routine is called when there is a minor X error, such as an invalid
*   palette index, or a bad value in a function call.  We should really do
*   more than just print the error, but errors should really only occur if
*   there is a bug in this code, and there isn't (of course), so we don't
*   need to worry about it.
*
* CHANGES
*
******************************************************************************/

static int HandleXError(ErrorDisplay, ErrorEvent)
Display *ErrorDisplay;
XErrorEvent *ErrorEvent;
{
  char message[130];

  XGetErrorText(ErrorDisplay, ErrorEvent->error_code, message, 125);
  Warning(0.0, "\nX error: %s", message);

  return (TRUE);  /* Return value is needed, but is actually ignored */
}


/*****************************************************************************
*
* FUNCTION  HandleXIOError
*
* INPUT
*
* OUTPUT
*
* RETURNS  the return value is ignored
*
* AUTHOR   Andreas Dilger, Feb 1996
*
* DESCRIPTION
*
*   This routine is called when there is a fatal X error, such as when the
*   connection to the X window is lost.  POV will try to save things at
*   this point, but it may not be able to do anything before it gets killed.
*
* CHANGES
*
******************************************************************************/

static int HandleXIOError(ErrorDisplay)
Display *ErrorDisplay;
{
  theDisplay = NULL;

  Error("\nFatal X Windows error on %s.\n", XDisplayName(theDisplayName));

  return (TRUE);  /* Return value is needed, but is actually ignored */
}


/*****************************************************************************
*
* FUNCTION HandleXEvents
*
* INPUT
*
*   theEvent - the event to be handled
*   finished - if we are finished the rendering or not
*
* OUTPUT
*
* RETURNS  TRUE if we are supposed to stop rendering
*
* AUTHOR   Andreas Dilger, Feb 1996
*
* DESCRIPTION
*
*   Check for various events and handle them.  If we need to refresh the
*   window, we only do so once for all the pending events by finding the
*   largest rectangle that we need to update, and updating only that.
*   This may not always be a win, but it is in the most common situation
*   when an overlapping window is dragged across this one and there are
*   many small ExposeEvents.
*
* CHANGES
*
******************************************************************************/

int HandleXEvents(theEvent, finished)
XEvent *theEvent;
Bool    finished;
{
  Bool   refresh = False;
  Bool   abortRender = False;
  int    refresh_x_min = theImage->width - 1;
  int    refresh_x_max = 0;
  int    refresh_y_min = theImage->height - 1;
  int    refresh_y_max = 0;

  do
  {
    KeySym theKeySym;

#if (X11_DEBUG > 0)
  Debug_Info("\nGot event %ld - ", theEvent->type);
#endif

    switch (theEvent->type)
    {
      case Expose:
        /*
         * Gather up pending expose events and handle them all at once.
         * Depending on the type of exposures, we may win some, we may lose
         * some.  The neurotic case is when opposing corners are uncovered,
         * but hopefully this won't happen too often, since we have asked
         * for a backing store.
         */
#if (X11_DEBUG > 0)
        Debug_Info("Expose\n");
#endif
        refresh_x_min = min(theEvent->xexpose.x, refresh_x_min);
        refresh_y_min = min(theEvent->xexpose.y, refresh_y_min);
        refresh_x_max = max(theEvent->xexpose.x + theEvent->xexpose.width,
                            refresh_x_max);
        refresh_y_max = theMaxY + 1;
        refresh = True;
        break;
      case KeyPress:
        /*
         * The 'q' and 'Q' keys will quit the display, either if the +x
         * (EXITENABLE) option is set, or if the image has finished
         * rendering.  CTRL-R and CTRL-L refresh the whole window.
         */
#if (X11_DEBUG > 0)
        Debug_Info("KeyPress\n");
#endif
        theKeySym = XKeycodeToKeysym(theDisplay, theEvent->xkey.keycode, 0);
        if ((opts.Options & EXITENABLE || finished) &&
            (theKeySym == XK_Q || theKeySym == XK_q))
        {
          abortRender = True;
        }
        else if (theEvent->xkey.state | ControlMask && (theKeySym == XK_L ||
                 theKeySym == XK_l || theKeySym == XK_R || theKeySym == XK_r))
        {
          refresh_y_min = 0;
          refresh_y_max = Frame.Screen_Height;
        }
        else
        {
          refresh_y_min = theCurrY;
          refresh_y_max = theCurrY + 1;
        }
        refresh_x_min = 0;
        refresh_x_max = Frame.Screen_Width;
        refresh = True;
        break;
      case ButtonPress:
        /*
         * A button click on the window will quit the preview if we are
         * finished rendering, otherwise it will cause a window refresh.
         */
#if (X11_DEBUG > 0)
        Debug_Info("ButtonPress\n");
#endif
        refresh_x_min = 0;
        refresh_y_min = theCurrY;
        refresh_x_max = Frame.Screen_Width;
        refresh_y_max = theCurrY + 1;
        abortRender |= finished;
        refresh = True;
        break;
      case ClientMessage:
        /*
         * We should get the WM_DELETE_WINDOW atom when the user closes
         * the window rather than exiting POV normally.  However, I'm
         * not sure how well this works.
         */
#if (X11_DEBUG > 0)
        Debug_Info("ClientMessage %ld [WM_DELETE_WINDOW = %ld]\n",
                    theEvent->xclient.data.l[0], WM_DELETE_WINDOW);
#endif
        if (theEvent->xclient.message_type == WM_PROTOCOLS &&
            theEvent->xclient.data.l[0] == WM_DELETE_WINDOW)
        {
          abortRender = True;
          refresh     = False;
        }
        break;
      case DestroyNotify:
#if (X11_DEBUG > 0)
        Debug_Info("DestroyNotify\n");
#endif
        abortRender = True;
        refresh     = False;
        break;
      case MapNotify:
        /*
         * We get this when the window is first displayed, and when it
         * returns from being iconified.  We should also get ExposeEvents
         * that cover the whole window, but we want to be safe.  That's
         * why we handle all of the pending display events at once.
         */
#if (X11_DEBUG > 0)
        Debug_Info("MapNotify\n");
#endif
        refresh_x_min = 0;
        refresh_x_max = Frame.Screen_Width;
        refresh_y_min = 0;
        refresh_y_max = theMaxY + 1;
        refresh = True;
        break;
#if (X11_DEBUG > 0)
      case UnmapNotify:
        Debug_Info("UnmapNotify\n");
        break;
      case ConfigureNotify:
        Debug_Info("ConfigureNotify\n");
        break;
      case ReparentNotify:
        Debug_Info("ReparentNotify\n");
        break;
      default:
        Debug_Info("unknown Event\n");
        break;
#endif
    }
  }
  while (XCheckWindowEvent(theDisplay, theWindow, theEVENTMASK, theEvent));

  if (refresh && !abortRender)
  {
#if (X11_DEBUG > 0)
    Debug_Info("Refresh %dx%d+%d+%d\n", refresh_x_min, refresh_y_min,
              refresh_x_max - refresh_x_min, refresh_y_max - refresh_y_min);
#endif
    XPutImage(theDisplay,theWindow,theGC,theImage,
              refresh_x_min, refresh_y_min, refresh_x_min, refresh_y_min,
              (unsigned int)(refresh_x_max - refresh_x_min),
              (unsigned int)(refresh_y_max - refresh_y_min));
    XFlush(theDisplay);
  }

  return (abortRender);
}



/*****************************************************************************
*
* FUNCTION  XWIN_display_close
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*    Reset the values used by the X display, in case we are animating.
*
* CHANGES
*
*    Moved memory freeing and X display closing to XWIN_finish_povray
*    so that we dont re-do everything during an animation.  July, 1996 [AED]
*
******************************************************************************/

void XWIN_display_close()
{
  int i;

  for (i = 0; i < 3; i++)
  {
    if (theErrors[i] != NULL)
    {
      memset(theErrors[i], 0, Frame.Screen_Width * sizeof(int));
    }
  }

  theMaxY = theCurrY = 0;
}



/*****************************************************************************
*
* FUNCTION  SelectVisual
*
* INPUT
*
* OUTPUT
*
*   global variable theClass is set
*
* RETURNS
*
*   theVisual - which visual we have chosen
*
* AUTHOR
*
* DESCRIPTION
*
*   Find the "best" visual, using this criteria:
*
*   1) Use the user specified visual class, if it is available.
*   2) Use the default visual if it has the deepest depth.
*   3) Deepest TrueColor visual
*   4) Deepest DirectColor visual
*   5) Deepest PseudoColor visual
*   6) Deepest StaticColor visual
*   8) Deepest StaticGray visual
*   9) Default visual
*
* CHANGES
*
*   Added tests for PseudoColor visuals other than 8 bits, StaticColor [AED]
*
******************************************************************************/

static void SelectVisual(theVisual, theDepth)
Visual **theVisual;
unsigned int *theDepth;
{
  XVisualInfo *theDefaultVisualInfo, *theVisualInfo, *theBestVisualInfo, *p;
  XVisualInfo template;
  int theDeepestDepth = 0;
  int nItems;
  int i;

  /*
   * Obtain the default VisualInfo for the in case we don't find a better one.
   */
 
  *theVisual = DefaultVisual(theDisplay, theScreen);
 
  template.visualid = (*theVisual)->visualid;
 
  if ((theBestVisualInfo = theDefaultVisualInfo =
       XGetVisualInfo(theDisplay, VisualIDMask, &template, &nItems)) == NULL)
  {
    Error("\nError obtaining X visual info for %s.\n",
           XDisplayName(theDisplayName));
  }

  /*
   * Get a list of all visuals on the screen.
   */

  template.screen = theScreen;

  if ((theVisualInfo =
     XGetVisualInfo(theDisplay, VisualScreenMask, &template, &nItems)) == NULL)
  {
    Error("\nError obtaining X visual info for %s.\n",
           XDisplayName(theDisplayName));
  }

  *theDepth = 0;

  /*
   * Look for the visual of the user specified class or use the default
   * visual if it has the deepest depth.  We'll gain nothing but color
   * flashing if we use a different visual/colormap at the same depth.
   */

  if (theParameters & theCLASSSET)
  {
    for (i = 0, p = theVisualInfo; i < nItems; i++, p++)
    {
      if ((p->class == theClass) && (p->depth > *theDepth))
      {
        *theDepth = p->depth;
        theBestVisualInfo = p;
      }
    }

    if (*theDepth == 0)
    {
      Status_Info("\nUnable to get ");
      switch (theClass)
      {
        case TrueColor:
          Status_Info("TrueColor");
          break;
        case DirectColor:
          Status_Info("DirectColor");
          break;
        case PseudoColor:
          Status_Info("PseudoColor");
          break;
        case StaticColor:
          Status_Info("StaticColor");
          break;
        case GrayScale:
          Status_Info("GrayScale");
          break;
        case StaticGray:
          Status_Info("StaticGray");
          break;
      }
      Status_Info(" visual as requested.");
    }
  }

  /*
   * If we can't get the requested visual, find the deepest available depth.
   * If this is the same as the default depth, then use the default visual.
   */

  if (*theDepth == 0)
  {
    for (i = 0; i < nItems; i++)
    {
      if (theVisualInfo[i].depth > theDeepestDepth)
      {
        theDeepestDepth = theVisualInfo[i].depth;
      }
    }

    if (theDeepestDepth == DefaultDepth(theDisplay, theScreen))
      *theDepth = theDeepestDepth;
  }

  /*
   * Look for deepest TrueColor
   */

  if (*theDepth == 0)
  {
    for (i = 0, p = theVisualInfo; i < nItems; i++, p++)
    {
      if ((p->class == TrueColor) && (p->depth > *theDepth))
      {
        *theDepth = p->depth;
        theBestVisualInfo = p;
      }
    }
  }

  /*
   * Look for deepest DirectColor
   */

  if (*theDepth == 0)
  {
    for (i = 0, p = theVisualInfo; i < nItems; i++, p++)
    {
      if ((p->class == DirectColor) && (p->depth > *theDepth))
      {
        *theDepth = p->depth;
        theBestVisualInfo = p;
      }
    }
  }

  /*
   * Look for deepest PseudoColor
   */

  if (*theDepth == 0)
  {
    for (i = 0, p = theVisualInfo; i < nItems; i++, p++)
    {
      if ((p->class == PseudoColor) && (p->depth > *theDepth))
      {
        *theDepth = p->depth;
        theBestVisualInfo = p;
      }
    }
  }

  /*
   * Look for deepest StaticColor
   */

  if (*theDepth == 0)
  {
    for (i = 0, p = theVisualInfo; i < nItems; i++, p++)
    {
      if ((p->class == StaticColor) && (p->depth > *theDepth))
      {
        *theDepth = p->depth;
        theBestVisualInfo = p;
      }
    }
  }

  /*
   * Look for deepest StaticGray
   */

  if (*theDepth == 0)
  {
    for (i = 0, p = theVisualInfo; i < nItems; i++, p++)
    {
      if ((p->class == StaticGray) && (p->depth > *theDepth))
      {
        *theDepth = p->depth;
        theBestVisualInfo = p;
      }
    }
  }

  *theDepth = theBestVisualInfo->depth;
  *theVisual = theBestVisualInfo->visual;
  if (*theVisual != DefaultVisual(theDisplay, theScreen))
    theParameters |= thePRIVATECMAP;
  theCells = theBestVisualInfo->colormap_size;
  theClass = theBestVisualInfo->class;
  theParameters |= theCLASSSET;  /* Save for next time if we are animating */

  /* Set up the RGB color masks for the given bit depth */
  if ((theClass == TrueColor) || (theClass == DirectColor))
  {
    unsigned long a;

    theColorShift[theRED] = 0;

    for (a = theBestVisualInfo->red_mask; (a & 1) == 0; a >>= 1)
    {
      theColorShift[theRED]++;
    }

    theColorSize[theRED]   = (theBestVisualInfo->red_mask >>
                                                theColorShift[theRED]) + 1;

    theColorShift[theGREEN] = 0;

    for (a = theBestVisualInfo->green_mask; (a & 1) == 0; a >>= 1)
    {
      theColorShift[theGREEN]++;
    }

    theColorSize[theGREEN] = (theBestVisualInfo->green_mask >>
                                                theColorShift[theGREEN]) + 1;

    theColorShift[theBLUE] = 0;

    for (a = theBestVisualInfo->blue_mask; (a & 1) == 0; a >>= 1)
    {
      theColorShift[theBLUE]++;
    }

    theColorSize[theBLUE]  = (theBestVisualInfo->blue_mask >>
                                                theColorShift[theBLUE]) + 1;

    /*
     * See if we have a normal 24 bit Truecolor visual, so we don't have
     * to do lots of math when displaying each pixel.
     */

    fastbig = True;
    fastlittle = True;

    for (a = 0; a < 3; a++)
    {
      if (theColorShift[a] != a*8 || theColorSize[a] != 0x100)
      {
#if (X11_DEBUG > 0)
        Debug_Info("No fastbig because color[%d] shift = %d, size = %d\n",
                    a, theColorShift[a], theColorSize[a]);
#endif
        fastbig = False;
      }

      if (theColorShift[a] != (2 - a)*8 || theColorSize[a] != 0x100)
      {
#if (X11_DEBUG > 0)
        Debug_Info("No fastlittle because color[%d] shift = %d, size = %d\n",
                    a, theColorShift[a], theColorSize[a]);
#endif
        fastlittle = False;
      }
    }

    /*
     * We need to do this because if the visual has an alpha channel,
     * this is included in the depth, which we don't want to consider.
     */

    a = theColorSize[theRED] * theColorSize[theGREEN] *
                               theColorSize[theBLUE];
    theDeepestDepth = 0;

    while (a > 1)
    {
      a >>= 1;
      theDeepestDepth++;
    }
  }

#if (X11_DEBUG > 0)
  Debug_Info("theDepth = %d\n", *theDepth);
#endif

  Status_Info("\nUsing ");
  switch (theClass)
  {
    case TrueColor:
      Status_Info("%d bit TrueColor", theDeepestDepth);
      break;
    case DirectColor:
      Status_Info("%d bit DirectColor", theDeepestDepth);
      break;
    case PseudoColor:
      Status_Info("%d color PseudoColor", theCells);
      break;
    case StaticColor:
      Status_Info("%d color StaticColor", theCells);
      break;
    case GrayScale:
      Status_Info("%d level GrayScale", theCells);
      opts.PaletteOption = GREY;
      break;
    case StaticGray:
      Status_Info("%d level StaticGray", theCells);
      opts.PaletteOption = GREY;
      break;
  }
  Status_Info(" visual...");

  XFree((void *)theDefaultVisualInfo);
  XFree((void *)theVisualInfo);
}



/*****************************************************************************
*
* FUNCTION  BuildColormap
*
* INPUT
*
*   theVisual - type of visual we are using
*
* OUTPUT
*
*   global variables theColormap and theColorCube set and contents filled in
*
* RETURNS
*
* AUTHOR      Andreas Dilger   Feb, 1996
*
* DESCRIPTION
*
*   Create a Colormap structure.  To make life easier, we will always use a
*   color cube for PseudoColor and StaticColor, and a grayscale ramp for
*   Grayscale, and StaticGray visuals, as well as PseudoColor and StaticColor
*   visuals with the +dG option.  This color cube/gray ramp will be filled in
*   with the best approximation XColor values, so we will not have to search
*   the color space for each pixel to find the best colormap index, we can
*   still do a good job with Static visuals, and we can treat all of the
*   paletted visuals in the same way.  If POV-Ray wants a grayscale preview
*   with a color visual, then do so.
*
* CHANGES
*
******************************************************************************/

static void BuildColormap(theVisual)
Visual *theVisual;
{
  switch (theClass)
  {
    int rmask, gmask, bmask;
    int i, j;

    case PseudoColor:

      /*
       * Find the largest number of entries to fit in the given colormap.
       * This currently handles colormaps up to 8 bits (256 entries), with
       * a few spaces left over for the system colors.
       */

      theColorSize[theRED] = theColorSize[theGREEN] = theColorSize[theBLUE] = 6;

      for (i = theColorSize[theRED]; i > 2; i--)
      {
        for (j = 0; j < 3; j++)
        {
          if (theColorSize[theRED] * theColorSize[theGREEN] *
                                     theColorSize[theBLUE] <= theCells)
          {
            break;
          }

          theColorSize[j]--;
        }
      }

      /* This is the maximum number of new colormap entries we will allocate */
      theMaxCell = theColorSize[theRED] * theColorSize[theGREEN] *
                   theColorSize[theBLUE];
      /* If we want a grayscale preview, do it */
      if (opts.PaletteOption == GREY)
      {
        GetBestGraymap(theVisual);
      }
      else
      {
        GetBestColormap(theVisual);
      }
      break;

    case StaticColor:
      theColormap = XCreateColormap(theDisplay,
                                    RootWindow(theDisplay, theScreen),
                                    theVisual, AllocNone);

      theMaxCell = theCells;
      theMaxDist = 0x7fffffff;
      if ((theColors = (XColor *)calloc(theCells, sizeof(XColor))) == NULL)
        MAError("X local colormap", theCells * sizeof(XColor));
      for (i = 0; i < theCells; i++)
      {
        theColors[i].pixel = i;
      }
      if ((theColorCube = (XColor *)calloc(theCUBESIZE*theCUBESIZE*theCUBESIZE,
                                           sizeof(XColor))) == NULL)
        MAError("X color cube",
                theCUBESIZE*theCUBESIZE*theCUBESIZE*sizeof(XColor));
      break;

    case GrayScale:

      /*
       * Try to find the largest number of colors that will fit in the
       * colormap, with a few left over for other applications.  The largest
       * reasonable frame buffer is 10 bits, so we start with 1000 entries
       * and work our way down.   The color size should be at least 2 in
       * the end, for a total of 8 entries.  I don't think such a thing as
       * a 1- or 2-bit GrayScale visual exists (ie not StaticGray).
       */

      theColorSize[theRED]=theColorSize[theGREEN]=theColorSize[theBLUE] = 10;

      for (i = theColorSize[theRED]; i > 2; i--)
      {
        if (theColorSize[theRED] * theColorSize[theGREEN] *
                                   theColorSize[theBLUE] <= theCells)
        {
          break;
        }
        theColorSize[theRED]--;
        theColorSize[theGREEN]--;
        theColorSize[theBLUE]--;
      }

      /* This is the maximum number of new colormap entries we will allocate */
      theMaxCell = theColorSize[theRED] * theColorSize[theGREEN] *
                   theColorSize[theBLUE];
      GetBestGraymap(theVisual);
      break;

    case StaticGray:
      theColormap = XCreateColormap(theDisplay,
                                    RootWindow(theDisplay, theScreen),
                                    theVisual, AllocNone);

      theMaxCell = theCells;
      theMaxDist = 0x7fffffff;
      if ((theColors = (XColor *)calloc(theCells, sizeof(XColor))) == NULL)
        MAError("X local colormap", theCells * sizeof(XColor));
      for (i = 0; i < theCells; i++)
      {
        theColors[i].pixel = i;
      }
      if ((theColorCube = (XColor *)calloc(theGRAYSIZE, sizeof(XColor))) == NULL)
        MAError("X gray ramp", theGRAYSIZE * sizeof(XColor));
      break;

    case TrueColor:
      theColormap = XCreateColormap(theDisplay,
                                    RootWindow(theDisplay, theScreen),
                                    theVisual, AllocNone);
      break;

    case DirectColor:
      theColormap = XCreateColormap(theDisplay,
                                    RootWindow(theDisplay, theScreen),
                                    theVisual, AllocNone);

      rmask = (theColorSize[theRED]   - 1)*(0x10000/theColorSize[theRED]);
      gmask = (theColorSize[theGREEN] - 1)*(0x10000/theColorSize[theGREEN]);
      bmask = (theColorSize[theBLUE]  - 1)*(0x10000/theColorSize[theBLUE]);

      /*
       * This should do the job of initializing a DirectColor visual, but
       * it has had relatively little testing, as I only have a single
       * 8-bit DirectColor visual to test it out on.  What's really lacking
       * is something smart to do if the XAllocColor() call fails.
       */
      for (i = 0; i < theCells; i++)
      {
        XColor tmp;
        tmp.red = tmp.green = tmp.blue = i * 0xffff / (theCells - 1);

        tmp.red   &= rmask;
        tmp.green &= gmask;
        tmp.blue  &= bmask;
        tmp.flags = DoRed | DoGreen | DoBlue;

        XAllocColor(theDisplay, theColormap, &tmp);
      }
      break;
  }
}



/*****************************************************************************
*
* FUNCTION GetBestColormap
*
* INPUT
*
*    theVisual - the type of visual to use
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Builds a minimal palette for a PseudoColor visual to be filled as
*   needed as the rendering progresses.
*
* CHANGES
*
*   June, 1996: Changed to deferred colormap allocation [AED]
*
******************************************************************************/

static void GetBestColormap(theVisual)
Visual *theVisual;
{
  unsigned long rd, gd, bd;

  /* We only need to allocate this once if we are doing an animation */
  if (theColors == NULL)
  {
    int index;
    if ((theColors = (XColor *)calloc(theCells, sizeof(XColor))) == NULL)
        MAError("X local colormap", theCells * sizeof(XColor));
    for (index = 0; index < theCells; index++)
    {
      theColors[index].pixel = index;
    }

    if ((theColorCube = (XColor *)calloc(theCUBESIZE*theCUBESIZE*theCUBESIZE,
                                         sizeof(XColor))) == NULL)
      MAError("X color cube",
              theCUBESIZE*theCUBESIZE*theCUBESIZE*sizeof(XColor));
  }

  if (!(theParameters & thePRIVATECMAP))
  {
    if ((theColormap = DefaultColormap(theDisplay, theScreen)) == None)
    {
      Error("\nUnable to get default X colormap.\n");
    }
  }
  else /* We aren't using the default visual, so we need a private colormap */
  {
    int low_colors;

    if (theColormap == None && (theColormap = XCreateColormap(theDisplay,
           RootWindow(theDisplay, theScreen), theVisual, AllocAll)) == None)
    {
      Error("\nUnable to get private X colormap.\n");
    }

    /*
     * We'll put the color cube in the top end of the colormap with the
     * intent of setting the lower cells to match those of the default
     * colormap.  The window manager and the clients that are already
     * running are hopefully using just these lower cells. After we copy
     * the colors in thse low cells from the default map to the new map,
     * there should be less color flashing.
     */

    theMaxCell = (theCells * 7 + 7) / 8;
    low_colors = theCells - theColorSize[theRED] * theColorSize[theGREEN] *
                                                   theColorSize[theBLUE];

    XQueryColors(theDisplay, DefaultColormap(theDisplay, theScreen),
                 theColors, low_colors);
    XStoreColors(theDisplay, theColormap, theColors, low_colors);
  }
  rd = 0x3000 / (theColorSize[theRED] - 1);
  gd = 0x3000 / (theColorSize[theGREEN] - 1);
  bd = 0x3000 / (theColorSize[theBLUE] - 1);

  theMaxDist = rd*rd + gd*gd + bd*bd;

  /* Make sure we get at least the eight corners of the color cube */
  for (rd = 0; rd < theCUBESIZE; rd += theCUBESIZE - 1)
  for (gd = 0; gd < theCUBESIZE; gd += theCUBESIZE - 1)
  for (bd = 0; bd < theCUBESIZE; bd += theCUBESIZE - 1)
    GetBestIndex(rd ? 0xffff : 0, gd ? 0xffff : 0, bd ? 0xffff : 0,
                 &theColorCube[(rd * theCUBESIZE + gd) * theCUBESIZE + bd]);

  /* Go for middle gray as well */
  GetBestIndex(0x8000, 0x8000, 0x8000,
               &theColorCube[((theCUBESIZE * theCUBESIZE + theCUBESIZE) *
                               theCUBESIZE + theCUBESIZE) / 2]);
}


/*****************************************************************************
*
* FUNCTION   GetBestGraymap
*
* INPUT      theVisual  -  visual to build gray map for
*
* OUTPUT
*
* RETURNS
*
* AUTHOR     Andreas Dilger
*
* DESCRIPTION
*
*   Builds a minimal grayscale palette for either a PseudoColor or
*   GrayScale visual that will be filled in as needed.
*
* CHANGES
*
*   June, 1996: Changed to deferred colormap allocation [AED]
*
******************************************************************************/

static void GetBestGraymap(theVisual)
Visual *theVisual;
{
  if (theColors == NULL)
  {
    int index;
    if ((theColors = (XColor *)calloc(theCells, sizeof(XColor))) == NULL)
      MAError("X local colormap", theCells * sizeof(XColor));
    for (index = 0; index < theCells; index++)
    {
      theColors[index].pixel = index;
    }

    if ((theColorCube = (XColor *)calloc(theGRAYSIZE, sizeof(XColor))) == NULL)
      MAError("X gray ramp", theGRAYSIZE * sizeof(XColor));
  }

  /*
   * Get the default colormap, and try to use this first.
   */

  if (!(theParameters & thePRIVATECMAP))
  {
    if ((theColormap = DefaultColormap(theDisplay, theScreen)) == None)
    {
      Error("\nUnable to get default X colormap.\n");
    }
  }
  else /* We aren't using the default visual - create a private colormap */
  {
    int low_colors;

    if (theColormap == None && (theColormap = XCreateColormap(theDisplay,
           RootWindow(theDisplay, theScreen), theVisual, AllocAll)) == None)
    {
      Error("\nUnable to get private X colormap.\n");
    }

    /*
     * We'll put the gray ramp in the top end of the colormap with the
     * intent of setting the lower cells to match those of the default
     * colormap.  The window manager and the clients that are already
     * running are hopefully using just these lower cells. After we copy
     * the colors in thse low cells from the default map to the new map,
     * there should be less color flashing.
     */

    theMaxCell = (theCells * 7 + 7) / 8;
    low_colors = theCells - theColorSize[theRED] * theColorSize[theGREEN] *
                                                   theColorSize[theBLUE];

    XQueryColors(theDisplay, DefaultColormap(theDisplay, theScreen),
                 theColors, low_colors);
    /*
     * If we have a GrayScale visual, we may as well store the low colors
     * in their grayscale equivalent, as the server will only use the red
     * component anyways, and this will give us more colors to work with
     * without affecting the root window too much.
     */
    if (theClass == GrayScale)
    {
      unsigned long gray;
      int index;

      for (index = 0; index < low_colors; index++)
      {
        gray = (307 * theColors[index].red + 599 * theColors[index].green +
                118 * theColors[index].blue) / 1024;
        theColors[index].red =
        theColors[index].green =
        theColors[index].blue = gray;
      }
    }

    XStoreColors(theDisplay, theColormap, theColors, low_colors);
  }

  theMaxDist = 0x8000 / (theMaxCell - 1);
  theMaxDist = 3 * theMaxDist * theMaxDist;

  /* Make sure we get at least black, middle gray, and white */
  GetBestIndex(0x0000, 0x0000, 0x0000, &theColorCube[0]);
  GetBestIndex(0xffff, 0xffff, 0xffff, &theColorCube[theGRAYSIZE - 1]);
  GetBestIndex(0x8000, 0x8000, 0x8000, &theColorCube[theGRAYSIZE / 2]);
}


/*****************************************************************************
*
* FUNCTION GetBestIndex
*
* INPUT r, g, b  - index into color cube to match
*
* OUTPUT
*
* RETURNS theCell - the ColorCube entry filled in with the best color match
*
* AUTHOR  Andreas Dilger.
*
* DESCRIPTION
*
*    Finds the nearest color in the given colormap using a euclidean distance.
*    This routine also does deferred colormap allocation based on requested
*    colors, rather than allocating a static color cube at the beginning of
*    the render.  If we can't find a colormap entry that is within the given
*    error bound, then we try to allocate the given color.  In any case, we
*    return the index of the color closest to that specified.
*
*    This is considerably faster than allocating a fixed set of colors at the
*    beginning and filling the color cube, as most images don't use the full
*    range of colors.  It also has the advantage that it is possible to get
*    more accurate colors in the colormap than a generic color cube.
*
* CHANGES
*
*    Changed for dynamic colormap allocation.  July 9, 1996 [AED]
*
******************************************************************************/

static void GetBestIndex(r, g, b, theCell)
long r, g, b;
XColor *theCell;
{
  static int numAlloc = 0, count = 200;
  unsigned long bestDist = 0xffffffffL;
  int bestIndex = 0, newIndex = FALSE, nCell;

  /*
   * Get a copy of the colormap periodically.  I tried the ColormapNotify
   * Event, but it didn't seem to register a change in a colormap entry
   * like I thought it should.
   */
  if ((count++ > 20 && numAlloc%10 == 0)||(numAlloc < theCells && count > 200))
  {
    XQueryColors(theDisplay, theColormap, theColors, (int)theCells);
#if (X11_DEBUG > 1)
    Debug_Info("Updated colormap (MaxDist = %ld)\n", theMaxDist);
#endif

    /* The static visuals won't change, so pretend we allocated everything */
    if (theClass != PseudoColor && theClass != GrayScale)
    {
      numAlloc = theCells;
    }
    count = 0;
  }

  /*
   * Work out the Euclidean distance for each colormap entry.  We need
   * to divide these by 2, or the maximum possible distance won't fit
   * into a 32-bit unsigned long, and we don't really need floats for this.
   */
  for (nCell = 0; nCell < theCells; nCell++)
  {
    unsigned long dist;
    long rd, gd, bd;

    rd = (theColors[nCell].red   - r) / 2;
    gd = (theColors[nCell].green - g) / 2;
    bd = (theColors[nCell].blue  - b) / 2;

    dist = rd*rd + gd*gd + bd*bd;

    /* Is this the best match we've found so far? */
    if (dist < bestDist)
    {
      bestDist = dist;
      bestIndex = nCell;
    }
  }

#if (X11_DEBUG > 1)
  Debug_Info("Need <%04X, %04X, %04X> ", r, g, b);
  Debug_Info("found <%04X, %04X, %04X> (dist = %ld)\n",
             theColors[bestIndex].red, theColors[bestIndex].green,
             theColors[bestIndex].blue, bestDist);
#endif

  /*
   * If we couldn't find a close enough entry and we haven't already
   * allocated too many entries, or if we haven't allocated this entry
   * ourselves we will try to allocate the desired color now.
   */
  if ((numAlloc < theMaxCell && bestDist > theMaxDist) || bestIndex >= numAlloc)
  {
    XColor want;

    if (bestDist > theMaxDist)
    {
      want.red   = r;
      want.green = g;
      want.blue  = b;
      want.pixel = theCells - numAlloc - 1;
#if (X11_DEBUG > 1)
      Debug_Info("Allocating new color ");
#endif
    }
    else
    {
      want = theColors[bestIndex];
#if (X11_DEBUG > 1)
      Debug_Info("Re-allocating existsing color ");
#endif
    }
    want.flags = DoRed | DoGreen | DoBlue;

    /*
     * Either we are just storing the desired color in our private
     * colormap, or we are trying to allocate it in the shared colormap.
     */
    if (theColormap != DefaultColormap(theDisplay, theScreen))
    {
      XStoreColor(theDisplay, theColormap, &want);
      newIndex = TRUE;
    }
    else if (XAllocColor(theDisplay, theColormap, &want))
    {
      newIndex = TRUE;
    }
#if (X11_DEBUG > 1)
    else
    {
      Debug_Info("failed\n");
    }
#endif


    /*
     * We need to update our local color table with the data for the
     * old location of this palette index.
     */
    if (newIndex)
    {
      for (nCell = numAlloc; nCell < theCells; nCell++)
      {
        if (want.pixel == theColors[nCell].pixel)
        {
          /* Save actual pixel colors in the next local color table entry */
          XColor tmp = theColors[numAlloc];

          theColors[numAlloc] = want;
          theColors[nCell] = tmp;
#if (X11_DEBUG > 1)
          Debug_Info("%d (palette %d)\n", numAlloc, want.pixel);
#endif
          bestIndex = numAlloc++;
          break;
        }
      }
    }
  }
#if (X11_DEBUG > 1)
  else
  {
    if (bestIndex < numAlloc)
      Debug_Info("We don't need to allocate again (already allocated)\n");
    else if (bestDist <= theMaxDist)
      Debug_Info("We don't need to allocate new (close match)\n");
    else if (numAlloc >= theMaxCell)
      Debug_Info("We've allocated too many colors already (%d of %d)\n",
             numAlloc, theMaxCell);
  }
#endif

  *theCell = theColors[bestIndex];
  /* This just tells use we have filled in this cell already, nothing else */
  theCell->flags = DoRed | DoGreen | DoBlue;
}



/*****************************************************************************
*
* FUNCTION GetBestColor
*
* INPUT r, g, b  - desired color to match in the range [0, 0xffff]
*
* OUTPUT
*
* RETURNS
*
*   pointer to the color which is closest, whether the output is grayscale
*   or color.
*
* AUTHOR
*
* DESCRIPTION
*
*    Returns the pointer from the color cube or ramp which was previously
*    calculated to be the closest to the desired color.  If we haven't
*    already found the closest color for this cell, then we need to find it
*    first.  One must be careful with the order of operations when working
*    out the index, to avoid overflow with large color and index values.
*
* CHANGES
*
*    Add deferred color cube/ramp allocation and filling here.
*
******************************************************************************/

static XColor *GetBestColor (r, g, b)
long r, g, b;
{
  unsigned long index;

  if (opts.PaletteOption == GREY)
  {
    long gray;

    gray = (r * 307 + g * 599 + b * 118) / 1024;
    index = gray * theGRAYSIZE / 0x10000;

    /* Check if we have found the closest color for this cell yet */
    if (!theColorCube[index].flags)
    {
      GetBestIndex(gray, gray, gray, &theColorCube[index]);
    }
  }
  else
  {
    long rc, gc, bc;

    rc = r * theCUBESIZE / 0x10000;
    gc = g * theCUBESIZE / 0x10000;
    bc = b * theCUBESIZE / 0x10000;

    index = (rc * theCUBESIZE + gc) * theCUBESIZE + bc;

    /* Check if we have found the closest color yet */
    if (!theColorCube[index].flags)
    {
      GetBestIndex(r, g, b, &theColorCube[index]);
    }
  }

  return (&theColorCube[index]);
}

