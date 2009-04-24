/****************************************************************************
*                unix.cpp
*
*  This module implements UNIX specific routines.
*
*  from Persistence of Vision Raytracer
*  Copyright 1995,1999 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file.
*  If POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by email to team-coord@povray.org or visit us on the web at
*  http://www.povray.org. The latest version of POV-Ray may be found at this site.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
* Various modifications from Mike Fleetwood, February 1999
* In a nutshell, they allow for enhanced (and fixed) default INI support.
* They include two new functions, UNIX_Process_Env and UNIX_Process_Povray_Ini
* and associated modifications as noted below
*
* Multiple binaries differing only in their display glued together by
* Mark Gordon, June 2000.
*
*****************************************************************************/

/*
 * X Window System code:
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
 * 98-12-05 Mark Gordon (mtgordon@povray.org)
 *  
 * - 3.1 update to parameters in XWIN_display_plot_rect 
 *
 * Version 7
 */

#define IO_RESTRICTIONS_DISABLED 0

#include "frame.h"
#include "povproto.h"   /* For Status_Info, pov_terminate */
#include "pov_util.h"   /* For pov_stricmp */
#include "povray.h"     /* For Stop_Flag */
#include "file_pov.h"   /* For Locate_File */
#include "optout.h"     /* For Banner() */
#include "optin.h"     /* For povray_inifile2object() */
#include "userio.h"     /* for POV_Std functions */
#include "userdisp.h"
#include "povms.h"
#include "povmsrec.h"
#include <sys/time.h>   /* For gettimeofday() and time() */
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

/* if X libs are present... */

#ifndef X_DISPLAY_MISSING

/* X11 includes */
#include "povxlib.h" /* to work around Xlib using C++ keywords */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

/* POV-specific includes */

#include "parse.h"     /* For MAError */
#include "xpovicon.xbm" /* This is the black & white POV icon */
#include "xpovmask.xbm" /* This is the transparency mask for the icon */

#endif /* X_DISPLAY_MISSING  */

/* if svgalib present, we need the following libraries */

#ifdef HAVE_LIBVGA

#include <vga.h>
#include <vgagl.h>
#include <ctype.h>

#endif /* HAVE_LIBVGA  */

/*****************************************************************************
 * Local preprocessor defines
 ******************************************************************************/

/* X11-specific local preprocessor defines */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef X_DISPLAY_MISSING

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

static char theTITLE[] =    "Persistence of Vision\0";
static char theICONNAME[] = "POV-Ray\0"; /* Short name for the icon */
static char theCLASS[] =    "Povray";  /* Should begin with a capital */
static char theNAME[] =     "povray";  /* Can be overridden with -name <name> */

/*
 * If USE_CURSOR is defined, we will get a watch cursor in the window
 * while we are still rendering, and it will disappear when finished.
 */

#ifdef USE_CURSOR
#  include <X11/cursorfont.h>
#endif /* USE_CURSOR  */

/*
 * What release of X11 we are using?  X11R5 and up should already define
 * this, but X11R3 and X11R4 don't, so it has to be specified here manually.
 * If it isn't set, assume X11R4.  If you have X11R3, then change it!!!
 */

#ifndef XlibSpecificationRelease
#define XlibSpecificationRelease 4
#endif /* XlibSpecificationRelease  */

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

#endif /* X_DISPLAY_MISSING  */

/* SVGAlib-specific local preprocessor defines */

#ifdef HAVE_LIBVGA

#ifndef theRED
#define theRED   0
#endif /* theRED  */
#ifndef theGREEN
#define theGREEN 1
#endif /* theGREEN  */
#ifndef theBLUE
#define theBLUE  2
#endif /* theBLUE  */

#define SVGA_DEBUG 0

#endif /* HAVE_LIBVGA  */

/*****************************************************************************
* Local typedefs
******************************************************************************/


/*****************************************************************************
* Local variables
******************************************************************************/

/* 
 * These are used when determining which (if any) display functions are
 * to be used.
 */

static int UsingXwin; /* effectively a boolean, determined at startup */
static int UsingSVGA; /* effectively a boolean, determined at startup */

/*
 * Environment variables, checked once and cached.
 */

char *home_value;

/*
 * Variables used by the I/O-Restriction code
 */

FILE *conf_fp;
char sysconf_name[FILE_NAME_MAX];
char userconf_name[FILE_NAME_MAX];

static bool io_all_allowed;   /* whether I/O restriction is turned off */
static bool io_read_allowed;  /* whether all read access is allowed */
static bool io_restricted;    /* whether I/O restricted to specified 
                                directories only */
static bool shellout_allowed; /* whether shellouts are allowed */
static bool cwd_io_allowed;   /* whether current directory is exempted from 
                                restrictions */
bool system_OK;               /* whether the file currently under consideration
				 is allowed under system-wide settings */


/* These store lists of directories from which reading and/or writing
   are allowed, as specified by various methods.  More specific methods
   can only make things more restrictive, not less so.  */

static char system_read_paths [64][FILE_NAME_MAX];
static char system_write_paths [64][FILE_NAME_MAX];
static char user_read_paths [64][FILE_NAME_MAX];
static char user_write_paths [64][FILE_NAME_MAX];
static char commandline_read_paths [64][FILE_NAME_MAX];
static char commandline_write_paths [64][FILE_NAME_MAX];

/* Used by both X11 and SVGA versions, may as well pull it out of the
 * preprocessor conditionals.  Not like it takes up much memory if you're
 * not using one of those.
 */

static int          *theErrors[3];          /* error diffusion storage arrays */

/*
 * Variables used by the histogram timers
 */

#if PRECISION_TIMER_AVAILABLE
static struct timeval hstart, hstop;
#endif /* PRECISION_TIMER_AVAILABLE  */

/* X11-specific local variables*/

#ifndef X_DISPLAY_MISSING
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
static char         *theCustomTitle = NULL;       /* passed w/ -title  */
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
#ifdef USE_CURSOR
static Cursor        theCursor;
#endif /* USE_CURSOR */
#endif /* X_DISPLAY_MISSING */

/* SVGAlib-specific preprocessor defines */

#ifdef HAVE_LIBVGA
static int scale, xoff, yoff;
static int couldwrite = false;
static GraphicsContext physicalScreen,
                       internalScreen;
#endif /* HAVE_LIBVGA */

/*****************************************************************************
* Static functions
******************************************************************************/

static int     XwinMode PARAMS((int argc, char *argv[]));
static int     SVGAMode PARAMS((void));
static bool    TestAccessAllowed PARAMS((const char *Filename, unsigned int FileType, bool IsWrite));

#ifndef X_DISPLAY_MISSING
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
static void     SetTitle PARAMS((void));
static void     PlotPixel PARAMS((int x, int y, unsigned char Red,
                                  unsigned char Green, unsigned char Blue,
                                  unsigned char Alpha));
#endif /* X_DISPLAY_MISSING  */

/* copied from Windows version -mtg */
// This has some issues; will revisit for 3.5.1

int parse_ini_file(char *filename)
{
 POVMSObject obj;

  if(POVMS_ASSERT(POVMSObject_New(&obj, kPOVObjectClass_ROptions) == 0,
       "POVMSObject_New failed, cannot ini file line.") == false)
    return -1;

  if(povray_inifile2object(filename, &obj) != 0) {
       //Error("Cannot open ini file %s.", filename);
       return -1;
  } 

// Read all options from all (recursively included) INI files
// povray_read_all_options(&obj);


  if(POVMS_ASSERT(Receive_RenderOptions(&obj, NULL, 0) == 0,
      "Receive_RenderOptions failed, cannot use ini file.") == false)
    return -1;

  (void)POVMSObject_Delete(&obj);

  return 0;
}

/*****************************************************************************
*
* FUNCTION matherr
*
* INPUT pointer to exception
*
* OUTPUT 
*
* RETURNS 1
*
* AUTHOR
*
* DESCRIPTION
*
*    The error handler for floating point exceptions.  Not totally critical,
*    but can avoid aborting a trace that gets a floating-point error during
*    the render.  It tries to return a reasonable value for the errors.
*    However, this slows down the rendering noticably.
*
*    Note: this is a SystemVism, so don't be surprised if this doesn't work
*    on BSDish systems.  It shouldn't be critical, and it should still compile.
*
* CHANGES
*
******************************************************************************/

#ifdef UNDERFLOW
#ifdef exception

int matherr(x)
struct exception *x;
{
  switch (x->type)
  {
    case DOMAIN:
    case OVERFLOW:

      x->retval = 1.0e17;

      break;

    case SING:
    case UNDERFLOW:

      x->retval = 0.0;

      break;
  }

  return (1);
}

#endif /* exception  */
#endif /* UNDERFLOW  */



#if PRECISION_TIMER_AVAILABLE

/*****************************************************************************
*
* FUNCTION  UNIX_Timer_Start
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Andreas Dilger   Oct 1995
*
* DESCRIPTION  Starts the histogram timer
*
* CHANGES
*
******************************************************************************/

void UNIX_Timer_Start()
{
  gettimeofday(&hstart, (struct timezone *)NULL);
}


/*****************************************************************************
*
* FUNCTION  UNIX_Timer_Stop
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Andreas Dilger   Oct 1995
*
* DESCRIPTION  Stops the histogram timer
*
* CHANGES
*
******************************************************************************/

void UNIX_Timer_Stop()
{
  gettimeofday(&hstop, (struct timezone *)NULL);
}


/*****************************************************************************
*
* FUNCTION  UNIX_Timer_Count
*
* INPUT
*
* OUTPUT
*
* RETURNS  The elapsed real time between start and stop in 10^-5s increments
*
* AUTHOR  Andreas Dilger   Oct 1995
*
* DESCRIPTION  The elapsed wall-clock time between the last two calls to
*              accumulate_histogram is measured in tens of microseconds.
*              Using microseconds for the histogram timer would only allow
*              1.2 hours per histogram bucket (ie 2^32 x 10^-6s in an unsigned
*              32 bit long), which may not be enough in some rare cases :-)
*              Note that since this uses wall-clock time rather than CPU
*              time, the histogram will have noise in it because of the
*              task switching going on.
*
* CHANGES
*
******************************************************************************/

int UNIX_Timer_Count()
{
  return ((hstop.tv_sec - hstart.tv_sec) * 100000 +
          (hstop.tv_usec - hstart.tv_usec + 5) / 10);
}

#endif /* PRECISION_TIMER_AVAILABLE */


/*****************************************************************************
*
* FUNCTION  UNIX_Abort_Handler
*
* INPUT  signum - signal number, or 0 if not called by a signal
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Andreas Dilger   Oct 1995
*
* DESCRIPTION
*      This is the signal handler.  If it is called by a signal (signal > 0),
*      then it will call set the global "Stop_Flag", and the rendering will
*      be aborted when the current pixel is complete.  We re-initialize the
*      signal handler for those older systems that reset the signal handlers
*      when they are called.  If we are really quitting (signal = 0) or this
*      routine has been called many times, we set the signal handlers back to
*      their default action, so that further siganls are handled normally.
*      This is so we don't get caught in a loop if we are handling a signal
*      caused by the shutdown process (ie file output to a pipe that's closed).
*
* CHANGES
*
*   Sept 1, 1996   Don't write to stderr if there have been many interrupts,
*                  so we don't write to a PIPE that's closed and cause more
*                  interrupts. [AED]
*
******************************************************************************/

void UNIX_Abort_Handler(int signum)
{
  UNIX_Abort_Start();

  if (signum > 0 && ++Stop_Flag < 5)
    {
      switch (signum)
	{
#ifdef SIGINT
      case SIGINT: Status_Info("\nGot %d SIGINT.", Stop_Flag); break;
#endif /* SIGINT  */
#ifdef SIGPIPE
	case SIGPIPE: Status_Info("\nGot %d SIGPIPE.", Stop_Flag); break;
#endif /* SIGPIPE  */
#ifdef SIGTERM
	case SIGTERM: Status_Info("\nGot %d SIGTERM.", Stop_Flag); break;
#endif /* SIGTERM  */
	default: Status_Info("\nGot %d unknown signal (%d).", Stop_Flag, signum); break;
	}
    }
  
  if (signum == 0 || Stop_Flag > 10)
    {
#ifdef SIGTERM
      signal(SIGTERM, SIG_DFL);
#endif /* SIGTERM  */
#ifdef SIGINT
      signal(SIGINT, SIG_DFL);
#endif /* SIGINT  */
#ifdef SIGPIPE
      signal(SIGPIPE, SIG_DFL);
#endif /* SIGPIPE  */
    }
}


/*****************************************************************************
*
* FUNCTION  UNIX_Abort_Start
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Andreas Dilger   Oct 1995
*
* DESCRIPTION  Initializes the signal handlers
*
* CHANGES
*
*   Apr 1996: Add SIGFPE to be ignored, bacause of floating point exceptions.
*             This isn't a great solution, but I don't know of another way to
*             prevent core dumps on Linux systems that use older versions of
*             libm. [AED]
*
******************************************************************************/

void UNIX_Abort_Start()
{
#ifdef SIGTERM
  signal(SIGTERM, UNIX_Abort_Handler);
#endif /* SIGTERM */
#ifdef SIGINT
  signal(SIGINT, UNIX_Abort_Handler);
#endif /* SIGINT */
#ifdef SIGPIPE
  signal(SIGPIPE, UNIX_Abort_Handler);
#endif /* SIGPIPE */
#if !defined(OVERFLOW) && defined(SIGFPE) /* avoid floating point exceptions */
  signal(SIGFPE, SIG_IGN);
#endif /* !defined(OVERFLOW) && defined (SIGPFE) */
}

/*****************************************************************************
*
* FUNCTION  UNIX_Process_Env
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Mike Fleetwood  Jan 1999
*
* DESCRIPTION  Process a default INI file pointed to by the environment
*              variable POVINI.  Leaves the global variable Option_String_Ptr
*              set to non-NULL if and only if the environment variable POVINI
*              points to a file which was read.
*
* CHANGES
*
******************************************************************************/

void UNIX_Process_Env(void)
{
  if ((Option_String_Ptr = getenv("POVINI")) != NULL)
  {
    if (parse_ini_file(Option_String_Ptr))
    {
      Warning(0, "Could not find '%s' as specified in POVINI environment.\n",
	      Option_String_Ptr);
      Option_String_Ptr = NULL;
    }
  }
  return;
}

/*****************************************************************************
*
* FUNCTION  UNIX_Process_Povray_Ini
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Mike Fleetwood  Jan 1999
*
* DESCRIPTION  If UNIX_Process_Env() processed a default INI file from the
*              POVINI environment variable then do nothing; otherwise search
*              for a default INI file, reading the first one found, from the
*              list:
*                  ./povray.ini
*                  $HOME/.povrayrc
*                  SYSCONFDIR/povray.ini
*
* CHANGES
*
******************************************************************************/

void UNIX_Process_Povray_Ini(void)
{
  char ini_name[FILE_NAME_MAX];
  int oldwarn = opts.Warning_Level;

  if (Option_String_Ptr != NULL)
    return;

  opts.Warning_Level = 0;  

  if (!parse_ini_file("./povray.ini")) {
    opts.Warning_Level = oldwarn;
    return;
  }
  if ((Option_String_Ptr = home_value) != NULL)
  {
    strncpy(ini_name, Option_String_Ptr, FILE_NAME_MAX);
    strncat(ini_name, "/.povrayrc", FILE_NAME_MAX);
    if (!parse_ini_file(ini_name)) {
      opts.Warning_Level = oldwarn;
      return;
    }
    else
      Option_String_Ptr = NULL;
  }
  opts.Warning_Level = oldwarn;
  strncpy(ini_name, SYSCONFDIR, FILE_NAME_MAX);
  strncat(ini_name, "/povray.ini", FILE_NAME_MAX);
  parse_ini_file(ini_name);
  return;
}
/*****************************************************************************
*
* FUNCTION  POV_basename
*
* INPUT path
*
* OUTPUT none (beware side effects)
*
* RETURNS final filename component of the path
*
* AUTHOR  Christopher James Huff
*
* DESCRIPTION Implementation of basename, roughly patterned after SUSv2
*              
*
* CHANGES
*
******************************************************************************/

char * POV_basename(char * path)
{
    int j = 0;
    int idx = 0;//current starting location of copy
    while(path[j])
    {
        if(path[j] == '/')
        {
            idx = 0;
        }
        else
        {
            path[idx] = path[j];
            idx++;
        }
        j++;
    }
    path[idx] = '\0';
    return path;
}

/*****************************************************************************
*
* FUNCTION  POV_dirname
*
* INPUT path
*
* OUTPUT none (beware side effects)
*
* RETURNS directory component of the path 
*
* AUTHOR  Christopher James Huff
*
* DESCRIPTION  Implementation of dirname, roughly patterned after SUSv2
*              
*
* CHANGES
*
******************************************************************************/

char * POV_dirname(char * path)
{
    int idx = -1;//index of last '/' in string
    for(int j = 0; path[j]; j++)
        if(path[j] == '/')
            idx = j;
    
    if(idx != -1)
        path[idx + 1] = '\0';

    return path;
}

/*****************************************************************************
*
* FUNCTION  incwd
*
* INPUT Filename
*
* OUTPUT Whether the file is in the current directory
*
* RETURNS int (effectively boolean)
*
* AUTHOR  Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION  Returns whether the requested file is in the current working
*              directory.
*
* CHANGES
*
******************************************************************************/

int incwd (const char *Filename)
{
  char dir[FILE_NAME_MAX];
  char base[FILE_NAME_MAX];
  char filename_copy[FILE_NAME_MAX];

  getcwd(dir, FILE_NAME_MAX);
  strncpy(filename_copy, Filename, FILE_NAME_MAX);

  if (strncmp(POV_basename(filename_copy), dir, strlen(dir)) == 0)
    return (TRUE);
  else
    return (FALSE);
}

/*****************************************************************************
*
* FUNCTION  parse_path
*
* INPUT Filename
*
* OUTPUT none
*
* RETURNS void
*
* AUTHOR  Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION  Replaces instances of . and .. in the input string.
*              Note: uses side effects.  Evil.  
*
* CHANGES New for 3.5
*
******************************************************************************/

void canonicalize_path (char *Path)
{
  char path_copy[FILE_NAME_MAX];
  char *match_beginning;
  char *match_end;

  if (match_beginning=strstr(Path,"%INSTALLDIR%"))
    {
      match_end=match_beginning+strlen("%INSTALLDIR%");
      *match_beginning='\0';
      strncpy(path_copy, Path, FILE_NAME_MAX);
      strncat(path_copy, POV_LIB_DIR, strlen(POV_LIB_DIR));
      strncat(path_copy, match_end, FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path, path_copy, FILE_NAME_MAX);
      canonicalize_path(Path);
    }

  if (Path[0] != '/')
    {
      getcwd(path_copy, FILE_NAME_MAX);
      strncat(path_copy, "/", 1);
      strncat(path_copy, Path, FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path,path_copy, FILE_NAME_MAX);
      canonicalize_path(Path);
    }

  if (match_beginning=strstr(Path,"/./"))
    {
      match_end=match_beginning+strlen("/./");
      *match_beginning='\0';
      strncpy(path_copy, Path, FILE_NAME_MAX);
      strncat(path_copy, "/", 1);
      strncat(path_copy, match_end, FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path, path_copy, FILE_NAME_MAX);
      canonicalize_path(Path);
    }

  if (strncmp(Path,"//",FILE_NAME_MAX)==0)
    {
      strncpy(path_copy, "/", FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path, path_copy, FILE_NAME_MAX);
      return;
    }

  if (strncmp(Path,"/..",FILE_NAME_MAX)==0)
    {
      strncpy(path_copy, "/", FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path, path_copy, FILE_NAME_MAX);
      return;
    }

  if (match_beginning=strstr(Path,"//"))
    {
      match_end=match_beginning+strlen("//");
      *match_beginning='\0';
      strncpy(path_copy, Path, FILE_NAME_MAX);
      strncat(path_copy, "/", 1);
      strncat(path_copy, match_end, FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path, path_copy, FILE_NAME_MAX);
      canonicalize_path(Path);
    }
  
  if (match_beginning=strstr(Path,"/../"))
    {
      match_end=match_beginning+strlen("/../");
      *match_beginning='\0';
      *strrchr(Path,'/')='\0';
      ++match_beginning;
      strncpy(path_copy, Path, FILE_NAME_MAX);
      strncat(path_copy, "/", 1);
      strncat(path_copy, match_end, FILE_NAME_MAX);
      strncat(path_copy, "\0", 1);
      strncpy(Path, path_copy, FILE_NAME_MAX);
      canonicalize_path(Path);
    }
  

  if (match_beginning=strstr(Path,"/."))
    {
      if (strncmp(match_beginning,"/.", FILE_NAME_MAX) == 0)
	{
	  *match_beginning='\0';
	  canonicalize_path(Path);
	}
    }

  if (match_beginning=strstr(Path,"/.."))
    {
      if (strncmp(match_beginning,"/..", FILE_NAME_MAX) == 0)
	{
	  *strrchr(Path,'/')='\0';
	  *strrchr(Path,'/')='\0';
	  canonicalize_path(Path);
	}
    }
  return;
}

/*****************************************************************************
*
* FUNCTION  subdir
*
* INPUT Filename, system paths, user paths
*
* OUTPUT 
*
* RETURNS a boolean, effectively, telling whether the file is in any of the
*           directories in the list (or their subdirectories).
*
* AUTHOR  Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION  Checks to see whether the file requested is in a directory
*              specified in either SYSCONFDIR/povray.conf or $HOME/.povray.conf
*              or in a subdirectory of any of the dirs in the list.
*
* CHANGES Security bug fixed October 2002
*
******************************************************************************/

int subdir (const char *Filename, char system_paths[64][FILE_NAME_MAX], char user_paths[64][FILE_NAME_MAX])
{
  int i;
  char cwd[FILE_NAME_MAX];
  char *parent_dirname;
  char *basename_pointer;
  char filename_buffer[FILE_NAME_MAX];
  char string_buffer[FILE_NAME_MAX];
  
  if (strncmp(Filename,"../",3) == 0)
    {
      if (getcwd(cwd, FILE_NAME_MAX) == NULL)
	{
	  Error(0, "Can't determine current directory!");
	  return (0);
	}
      strncpy(filename_buffer, Filename, FILE_NAME_MAX);
      basename_pointer=filename_buffer+2;
      strncpy(string_buffer, basename_pointer, FILE_NAME_MAX);
      parent_dirname=POV_dirname(cwd);
      return (subdir(strncat(parent_dirname, string_buffer, FILE_NAME_MAX), system_paths, user_paths));
    }
  if (strncmp(Filename,"./",3) == 0)
    {
      if (getcwd(cwd, FILE_NAME_MAX) == NULL)
	{
	  Error(0, "Can't determine current directory!");
	  return (0);
	}
      strncpy(filename_buffer, Filename, FILE_NAME_MAX);
      basename_pointer=filename_buffer+1;
      return (subdir(strncat(cwd,basename_pointer, FILE_NAME_MAX), system_paths, user_paths));
    }
  if (Filename[0] != '/')
    {
      if (getcwd(cwd, FILE_NAME_MAX) == NULL)
	{
	  Error(0, "Can't determine current directory!");
	  return (0);
	}
      strncpy(filename_buffer, Filename, FILE_NAME_MAX);
      return (subdir(strncat(strncat(cwd, "/", FILE_NAME_MAX), filename_buffer, FILE_NAME_MAX), system_paths, user_paths));
    }
  for (i=0; i<64; ++i)
    {
      if (system_paths[i][0] != (char)NULL)
	{
	  if (strncmp(Filename, system_paths[i], strlen(system_paths[i])) == 0)
	    {
	      system_OK=true;
	      break;
	    }
	}
    }
  if (! system_OK)
    {
      return (FALSE);
    }
  for (i=0; i<64; ++i)
    {
      if (user_paths[i][0] != (char)NULL)
	{
	  if (strncmp(Filename, user_paths[i], strlen(user_paths[i])) == 0)
	    {
	      return (TRUE);
	    }
	}
    }
  return (FALSE);
}

/*****************************************************************************
*
* FUNCTION UNIX_Allow_File_Write
*
* INPUT     Filename, FileType
*
* OUTPUT    
*
* RETURNS   TRUE if file writing is allowed, else FALSE
*
* AUTHOR   Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION
*
*    Performs tests to determine whether file writing is allowed
*
* CHANGES
*
*    Brand new, March 2002 [mtg]
*  
******************************************************************************/

int UNIX_Allow_File_Write (const char *Filename, const unsigned int FileType)
{
  char filename[FILE_NAME_MAX];
  strncpy(filename, Filename, FILE_NAME_MAX);
  canonicalize_path(filename);
  system_OK=false;
  if (IO_RESTRICTIONS_DISABLED)
    {
      return (TRUE);
    }
  if (strncmp(sysconf_name, filename, FILE_NAME_MAX) == 0)
    {
      Error("Writing to %s is not permitted.\n", filename);
      return (FALSE);
    }
  else if (strncmp(userconf_name, filename, FILE_NAME_MAX) == 0)
    {
      Error("Writing to %s is not permitted.\n", filename);
      return (FALSE);
    }
  else if (io_all_allowed)
    {
      return (TRUE);
    }
  else if (cwd_io_allowed && incwd(filename))
    {
      return (TRUE);
    }
  else if (subdir(filename, system_write_paths, user_write_paths))
    {
      return (TRUE);
    }
  Error("Writing to %s is not permitted.\nCheck the configuration in %s/povray.conf or ~/.povray.conf.\n", filename, SYSCONFDIR);
  return (FALSE);
}

/*****************************************************************************
*
* FUNCTION UNIX_Allow_File_Read
*
* INPUT     Filename, FileType
*
* OUTPUT    
*
* RETURNS   TRUE if file reading is allowed, else FALSE
*
* AUTHOR   Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION
*
*    Performs tests to determine whether file reading is allowed
*
* CHANGES
*
*    Brand new, March 2002 [mtg]
*  
******************************************************************************/

int UNIX_Allow_File_Read (const char *Filename, const unsigned int FileType)
{
  char filename[FILE_NAME_MAX];
  system_OK=false;
  if (IO_RESTRICTIONS_DISABLED)
    {
      return (TRUE); /* This disables I/O restrictions at build time */
    }
  if (io_all_allowed)
    {
    return (TRUE);
    }
  else if (io_read_allowed)
    {
    return (TRUE);
    }
  strncpy(filename, Filename, FILE_NAME_MAX);
  canonicalize_path(filename);
  if (cwd_io_allowed && incwd(filename))
    {
      return (TRUE);
    }
  else if (subdir(filename, system_read_paths, user_read_paths))
    {
      return (TRUE);
    }
  Error("Reading from %s is not permitted.\nCheck the configuration in %s/povray.conf or ~/.povray.conf.\n", filename, SYSCONFDIR);
  return (FALSE);
}
/*****************************************************************************
*
* FUNCTION UNIX_System
*
* INPUT     wrapper for system(3)
*
* OUTPUT    
*
* RETURNS   Return value of child process, or failure if it's not allowed.
*
* AUTHOR   Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION
*
*    Passes the string to system(3) if and only if system calls are
*    allowed.
*
* CHANGES
*
*    Brand new, July 2002 [mtg]
*  
******************************************************************************/

int UNIX_System (const char *string)
{
  if (IO_RESTRICTIONS_DISABLED)
    {
      return(system(string));
    }
  if (shellout_allowed)
    {
      return(system(string));
    }
  else
    {
      Warning(0,"Shellout not allowed under your configuration.");
      return(-1);
    }
}

/*****************************************************************************
*
* FUNCTION  UNIX_Process_Povray_Conf
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION Process a povray.conf file (either sysconfdir/povray.conf
*             or ~/.povray.conf)
*              
* CHANGES New as of July 2002
*
******************************************************************************/

void UNIX_Process_Povray_Conf(FILE *conf_file, char *conf_name, char read_paths[64][FILE_NAME_MAX], char write_paths[64][FILE_NAME_MAX])
{
  char line[FILE_NAME_MAX];
  int i; /* index into path arrays */
  char directory[FILE_NAME_MAX];
  while (fgets(line, FILE_NAME_MAX, conf_file)!= NULL)
    {
      if ((line[0]=='\n') || (line[0]==';')) /* Ignore whitespace, semicolon */
	{
	  /* Do nothing */ ;
	}

      else if (strcmp(line,"[File I/O Security]\n") == 0)
	{
	  while (fgets(line, FILE_NAME_MAX, conf_file)!=NULL)
	    {
	      if (strcmp(line,"none\n") == 0)
		{
		  io_all_allowed=true;
		  break;
		}
	      else if (strcmp(line,"read-only\n") == 0)
		{
		  io_read_allowed=true;
		  io_all_allowed=false;
		  break;
		}
	      else if (strcmp(line,"restricted\n") == 0)
		{
		  io_restricted=true;
		  io_all_allowed=false;
		  io_read_allowed=false;
		  break;
		}
	      else if ((line[0]=='\n') || (line[0]==';'))
		{
		  /* Do nothing */ ;
		}
	      else
		{
		Warning(0, "Could not parse File I/O Security option %s in file %s", 
			line, conf_name);
		break;
		}
	    }
	}
      
      else if (strcmp(line,"[Shellout Security]\n") == 0)
	{
	  while (fgets(line, FILE_NAME_MAX, conf_file)!=NULL)
	    {
	      if (strcmp(line,"allowed\n") == 0)
		{
		  shellout_allowed=true;
		  break;
		}
	      else if (strcmp(line,"forbidden\n") == 0)
		{
		  shellout_allowed=false;
		  break;
		}
	      else if ((line[0]=='\n') || (line[0]==';'))
		{
		  /* Do nothing */ ;
		}
	      else
		{
		Warning(0, "Could not parse Shellout Security option %s in file %s", 
			line, conf_name);
		break;
		}
	    }
	}
      
      else if (strcmp(line,"[Current Directory Read/Write]\n") == 0)
	{
	  while (fgets(line, FILE_NAME_MAX, conf_file)!=NULL)
	    {
	      if (strcmp(line,"allowed\n") == 0)
		{
		  cwd_io_allowed=true;
		  break;
		}
	      else if (strcmp(line,"forbidden\n") == 0)
		{
		  cwd_io_allowed=false;
		  break;
		}
	      else if ((line[0]=='\n') || (line[0]==';'))
		{
		  /* Do nothing */ ;
		}
	      else
		{
		Warning(0, "Could not parse Current Directory Read/Write  option %s in file %s", line, conf_name);
		break;
		}
	    }
	}
      
      else if (strcmp(line,"[Permitted Input Paths]\n") == 0)
	{
	  while ((fgets(line, FILE_NAME_MAX, conf_file)!= NULL) && (((line[0] >= '0') && (line[0] <= '9')) || (line[0] == ';' )))
	  {
	      if ((line[0] >= '0') && (line[0] <= '9'))
	      {
		sscanf(line, "%d=%s", &i, directory);
		canonicalize_path(directory);
		if (i <= 64)
		  {
		    strncpy(read_paths[i], directory, FILE_NAME_MAX);
		  }
		else
		  {
		    Warning(0, "Only 64 Permitted Input Paths are allowed, excess are ignored");
		  }
	      }
	      else
		{
		  Warning(0, "Output file paths must start with a number 0-63, invalid path declarations are ignored.");
		}
	      
	    }
	}

      else if (strcmp(line,"[Permitted Output Paths]\n") == 0)
	{
	  while ((fgets(line, FILE_NAME_MAX, conf_file)!= NULL) && (((line[0] >= '0') && (line[0] <= '9')) || (line[0] == ';' )))
	    {
	      if ((line[0] >= '0') && (line[0] <= '9'))
		{
		sscanf(line, "%d=%s", &i, directory);
		canonicalize_path(directory);
		if (i <= 64)
		  {
		    strncpy(write_paths[i], directory, FILE_NAME_MAX);
		  }
		else
		  {
		    Warning(0, "Only 64 Permitted Ouput Paths are allowed, excess are ignored.");
		  }
	      }
	      else
		{
		  Warning(0, "Output file paths must start with a number 0-63, invalid path declarations are ignored.");
		}
	    }
	}
      
      else {
	Warning(0, "Unrecognized line in %s: %s\n", conf_name, line);
      }
      
    }
  /* if no read paths specified, at least include POV_LIB_DIR 
     and SYSCONFDIR */
  for (i=0;i<64;++i)
    {
      if (strlen(read_paths[i])!=0)
	{
	  return;
	}
    }
  strncpy(read_paths[0],POV_LIB_DIR,FILE_NAME_MAX);
  strncpy(read_paths[1],SYSCONFDIR,FILE_NAME_MAX);

  return;
}

/*****************************************************************************
*
* FUNCTION UNIX_Process_IO_Restrictions
*
* INPUT     none
*
* OUTPUT    None
*
* RETURNS  None
*
* AUTHOR   Mark Gordon <mtgordon@povray.org> July 2002
*
* DESCRIPTION
*
*    Performs the I/O Restriction initialization, including calls to parse
*      configuration files.
*
* CHANGES
*
*    Brand new, July 2002 [mtg]
*  
******************************************************************************/

void UNIX_Process_IO_Restrictions (void)
{
  int i;

  /* Default values */
  io_all_allowed=false;
  io_read_allowed=false;
  io_restricted=true;
  shellout_allowed=false;
  cwd_io_allowed=true;

  for (i=0; i<64; ++i)
    {
      system_read_paths[i][0]=(char)NULL;
      system_write_paths[i][0]=(char)NULL;
      user_read_paths[i][0]=(char)NULL;
      user_read_paths[i][0]=(char)NULL;
    }

  if (home_value != NULL)
    {
      strncpy(userconf_name, home_value, FILE_NAME_MAX);
      strncat(userconf_name, "/.povray.conf", FILE_NAME_MAX);
      if ((conf_fp=fopen(userconf_name, "r")) != NULL)
	{
	  UNIX_Process_Povray_Conf(conf_fp, userconf_name, user_read_paths, user_write_paths);
	  fclose(conf_fp);
	}
      
    }
  
  strncpy(sysconf_name, SYSCONFDIR, FILE_NAME_MAX);
  strncat(sysconf_name, "/povray.conf", FILE_NAME_MAX);
  if ((conf_fp=fopen(sysconf_name, "r")) != NULL)
    {
      UNIX_Process_Povray_Conf(conf_fp, sysconf_name, system_read_paths, system_write_paths);
      fclose(conf_fp);
    }
  return;
}

/*****************************************************************************
*
* FUNCTION UNIX_init_povray
*
* INPUT     argc, argv - the command-line arguments
*
* OUTPUT   
*
* RETURNS   argc, argv - command-line args with the X Windows specific
*                        arguments removed and/or converted into something
*                        corresponding to a POV-Ray option (note: this only
*                        holds true if X initialization is done)
*
* RETURNS  None
*
* AUTHOR   Mark Gordon <mtgordon@povray.org>
*
* DESCRIPTION
*
*    Performs X, SVGA, or ASCII initialization (using function pointers).
*
* CHANGES
*
*    Brand new, June 2000 [mtg]
*  
******************************************************************************/

void UNIX_init_povray (int *argc, char **argv[])
{

  /* 
   * set these to explicit values, since the other methods of setting these are
   * conditionally compiled.
   */
  UsingXwin = false;
  UsingSVGA = false;

  /* 
   * Assume these and override as needed
   */
  
  UNIX_finish_povray = &POV_Std_Finish_Povray;
  UNIX_display_init = &POV_Std_Display_Init;
  UNIX_display_plot = &POV_Std_Display_Plot;
  UNIX_display_plot_rect = &POV_Std_Display_Plot_Rect;
  UNIX_display_plot_box = &POV_Std_Display_Plot_Box;
  UNIX_display_finished = &POV_Std_Display_Finished;
  UNIX_display_close = &POV_Std_Display_Close;
  UNIX_Test_Abort = &POV_Std_Test_Abort;

  home_value=getenv("HOME");

  /* 
   * If we have X available to us, find out if we want to use it.
   * We'll use it if we're currently in an X session or if we're
   * passing -display on the command line.  If we decide that here,
   * set appropriate function counters to reduce later calculation 
   */

#ifndef X_DISPLAY_MISSING
  if (UsingXwin = XwinMode(*argc, *argv))
    {
      XWIN_init_povray(argc, argv);
      UNIX_finish_povray = &XWIN_finish_povray;
      UNIX_display_init = &XWIN_display_init;
      UNIX_display_plot = &XWIN_display_plot;
      UNIX_display_plot_rect = &XWIN_display_plot_rect;
      UNIX_display_plot_box = &XWIN_display_plot_box;
      UNIX_display_finished = &XWIN_display_finished;
      UNIX_display_close = &XWIN_display_close;
      UNIX_Test_Abort = &XWIN_Test_Abort;
      UNIX_Process_IO_Restrictions();
      return;
    }
#endif /* X_DISPLAY_MISSING */

  /* 
   * If we have SVGA available to us, find out if we want to use it.
   * We must be at the console, and we must either be root or running
   * suid root.
   */

#ifdef HAVE_LIBVGA
  if (UsingSVGA = SVGAMode())
    {
      int i;
      for (i = 0; i < 3; i++)
	{
	  theErrors[i] = NULL;
	}
      SVGA_init_povray();
      UNIX_finish_povray = &SVGA_finish_povray;
      UNIX_display_init = &SVGA_display_init;
      UNIX_display_plot = &SVGA_display_plot;
      UNIX_display_plot_rect = &SVGA_display_plot_rect;
      UNIX_display_plot_box = &SVGA_display_plot_box;
      UNIX_display_finished = &SVGA_display_finished;
      UNIX_display_close = &SVGA_display_close;
      UNIX_Test_Abort = &SVGA_Test_Abort;
      UNIX_Process_IO_Restrictions();
      return;
    }
#endif /* HAVE_LIBVGA */

  /* Insert other display types here (e.g. frame buffer) 
   * if you feel like it  
   */
  
  UNIX_Process_IO_Restrictions();
  return;
}

/*****************************************************************************
*
* FUNCTION  POV_Std_Finish_Povray
*
* INPUT 
*
* OUTPUT
*
* RETURNS
*
* AUTHOR Mark Gordon <mtgordon@povray.org>
*
* DESCRIPTION
*
*    Just a dummy function to satisfy the function pointer.
*
* CHANGES
*
*    Brand-new, June 2000. [mtg]
*
******************************************************************************/


void POV_Std_Finish_Povray(void)
{
  /* no special finish_povray behavior for Std version */
  // exit(0);
}

#ifdef HAVE_LIBVGA

/*****************************************************************************
*
* FUNCTION  SVGA_display_finished
*
* INPUT status
*
* OUTPUT
*
* RETURNS
*
* AUTHOR Mark Gordon <mtgordon@povray.org>
*
* DESCRIPTION
*
*    Just a dummy function to satisfy the function pointer.
*
* CHANGES
*
*    Brand-new, June 2000. [mtg]
*
******************************************************************************/


void SVGA_display_finished(void)
{
  /* no special display_finish behavior for SVGA version */
  return;
}

/*****************************************************************************
*
* FUNCTION  SVGA_finish_povray
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR Mark Gordon <mtgordon@povray.org>
*
* DESCRIPTION
*
*    Just a dummy function to satisfy the function pointer.
*
* CHANGES
*
*    Brand-new, June 2000. [mtg]
*
******************************************************************************/

void SVGA_finish_povray(void)
{
  /* no special finish_povray behavior for SVGA version */
  // exit(0);
}


/*****************************************************************************
*
* FUNCTION  SVGA_Test_Abort
*
* INPUT
*
* OUTPUT
*
* RETURNS a number to add to Stop_Flag
*
* AUTHOR Mark Gordon <mtgordon@povray.org>
*
* DESCRIPTION
*
*    If user wants to quit, this returns TRUE, which gets added to Stop_Flag.
*
* CHANGES
*
*    Brand-new, June 2000. [mtg]
*
******************************************************************************/


int SVGA_Test_Abort(void)
{
  return ((opts.Options&EXITENABLE && toupper(vga_getkey())=='Q') ? true : false);
}

#endif /* HAVE_LIBVGA */

/*****************************************************************************
*
* FUNCTION  POV_Std_Test_Abort
*
* INPUT
*
* OUTPUT
*
* RETURNS 0
*
* AUTHOR Mark Gordon <mtgordon@povray.org>
*
* DESCRIPTION
*
*    Just a dummy function to satisfy the function pointer.
*
* CHANGES
*
*    Brand-new, June 2000. [mtg]
*
******************************************************************************/

int POV_Std_Test_Abort(void)
{
/*
 * This is the render abort function.  It is not needed for the command-line interface, 
 * since the abort is handled via interrupts.
 */
  return (0);
}

/*****************************************************************************
*
* FUNCTION XwinMode
*
* INPUT     argc, argv - the command-line arguments
*
* OUTPUT    None
*
* RETURNS   TRUE or FALSE (whether to use X preview functions)
*
* AUTHOR   mtgordon@povray.org <Mark Gordon>
*
* DESCRIPTION
*
*    Determines whether preview should use functions from the
*    X Window System.  It assumes you want X preview if you're 
*    currently in X (as determined by checking for the presence 
*    of the $DISPLAY environment variable) or if you're passing 
*    -display on the command line.  Unless I'm mistaken, one of these
*    has to be true in order for you to use X-based preview.
*    
*    If you don't have X installed (as determined by the configure script),
*    this is just going to return false.  Since the official binary
*    will depend on X to run, you'll need to recompile if you don't have X.
*
* CHANGES
*
*    Brand-new, June 2000. [mtg]
*
******************************************************************************/

int XwinMode (int argc, char *argv[])
{
#ifndef X_DISPLAY_MISSING
  int index;
  
  /* if $DISPLAY is non-null, return true */
  if (getenv("DISPLAY") != NULL)


  {
    return (true);
  }
  /* if passing -display, return true */
  for (index = 1; index < argc; index++)
    {
      if (!strncmp(argv[index], "-display\0", 9))
	{
	  return (true);
	}
    }
#endif /* X_DISPLAY_MISSING */
  return (false);
}

/*****************************************************************************
*
* FUNCTION SVGAMode
*
* INPUT    None
*
* OUTPUT   None
*
* RETURNS  TRUE or FALSE (whether to use SVGALib display functions)
*
* AUTHOR   mtgordon@povray.org <Mark Gordon>
*
* DESCRIPTION
*
*    Determines whether preview should use SVGAlib functions.
*    It assumes you want SVGA preview if you are at a TTY and either
*    you're root or POV-Ray is running suid root.  I've tried my best
*    to reduce the risk of buffer-overflow exploits for anyone who wants
*    to run POV-Ray suid root, but I still feel like I'm playing with fire.
*    Run suid root at your own risk.
*    
*    If you don't have SVGAlib installed (as determined by the configure 
*    script), this is just going to return false.  Since the official binary
*    will depend on SVGAlib to run, you'll need to recompile if you don't 
*    SVGAlib.
*
* CHANGES
*
*    Brand-new as of June 2000.  Let me know if you find any problems.
*
******************************************************************************/

int SVGAMode (void)
{
#ifdef HAVE_LIBVGA
  /* if you're set on using the X version, don't need to check further */
  if (UsingXwin)
    { 
      return (false);
    }
  /* if this isn't actually a TTY, no need for SVGA */
  if (!strstr(ttyname(0), "tty")) 
    {
      return (false);
    } 
  /* if POV-Ray is setuid-root, proceed */
  if (geteuid() == 0)
    {
      return (true);
    }
  /* or if the user is actually root, proceed */
  if (getuid() == 0)
    {
      return (true);
    }
#endif /* HAVE_LIBVGA  */
  return (false);
}


#ifndef X_DISPLAY_MISSING

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
void XWIN_init_povray(int *argc, char **argv[])
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

    povray_terminate();
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
#endif /* (X11_DEBUG > 0)  */
    }
    else
    {
      theDisplayName = NULL;
#if (X11_DEBUG > 0)
      Debug_Info("Using the default display\n");
#endif /* (X11_DEBUG > 0) */
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
#endif /* (X11_DEBUG > 0)  */

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

    if ((theCustomTitle = (char *)malloc(strlen(theTitle)+1)) == NULL)
      {
	MAError("theCustomTitle", (strlen(theTitle)));
      }
    strncpy(theCustomTitle, theTitle, strlen(theTitle));

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
#endif /* (X11_DEBUG > 0)  */

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

static int CheckArgs(int argc, char *argv[], char *match)
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

static void RemoveArgs(int *argc, char *argv[], int index)
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
* INPUT
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

void XWIN_finish_povray(void)
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


  if (theCustomTitle != NULL)
    free(theCustomTitle);


  if (theName != NULL)
    free(theName);

  if (theDisplayName != NULL)
    free(theDisplayName);

  // exit(0);
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
int XWIN_display_init(int width, int height)
{
  static Visual              *theVisual = NULL;
  static unsigned int         theDepth;
  static XSetWindowAttributes theWindowAttributes;
  static unsigned long        theWindowMask;
  static XSizeHints           theSizeHints;
  static Pixmap               theIcon;
  static Pixmap               theMask;
#if (XlibSpecificationRelease > 3)
  static  XWMHints             theWMHints;
#endif /* (XlibSpecificationRelease > 3) */
  static  XClassHint           theClassHints;
  static  XGCValues            theGCValues;
  static  int                  theDispWidth, theDispHeight;
  static  long                 i;
  static  size_t               size;

  /* If we have already set up the display, don't do it again */
  if (theDisplay != NULL)
    return (true);

  if ((theDisplay = XOpenDisplay(theDisplayName)) == NULL)
  {
    if (strlen(XDisplayName(theDisplayName)) == 0)
    {
      Error("\nDISPLAY variable is not set, and you are not specifying");
      Error("\na display on the command-line with -display.");
    }
    else
    {
      Error("\nThe display '%s' is not a valid display,",
                 XDisplayName(theDisplayName));
      Error("\nor you do not have permission to open a window there.");
    }
    Error("\nError opening X display.\n");
  }

  /*
   * set up the non-fatal and fatal X error handlers
   */

#if (X11_DEBUG > 0)
  Debug_Info("\nActually displaying at %s\n", XDisplayName(theDisplayName));
#else /* (X11_DEBUG <= 0)  */
  XSetErrorHandler(HandleXError);
  XSetIOErrorHandler(HandleXIOError);
#endif /* (X11_DEBUG > 0)  */

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
#endif /* USE_CURSOR */

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
#endif /* (XlibSpecificationRelease > 3) */
  theSizeHints.width        =                     /* Obsolete */
  theSizeHints.min_width    =
  theSizeHints.max_width    =
  theSizeHints.min_aspect.x =
  theSizeHints.max_aspect.x = Frame.Screen_Width;
#if (XlibSpecificationRelease > 3)
  theSizeHints.base_height  =
#endif /* (XlibSpecificationRelease > 3) */
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
#else /* (XlibSpecificationRelease <= 3) */
  XSetNormalHints(theDisplay, theWindow, &theSizeHints);
#endif /* (XlibSpecificationRelease > 3) */

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

  SetTitle();

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
#else /* (XlibSpecificationRelease <= 3) */
  XChangeProperty(theDisplay, theWindow, WM_PROTOCOLS, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)&WM_DELETE_WINDOW, 1);
#endif /* (XlibSpecificationRelease > 3) */

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
    if (fastlittle == true || fastbig == true)
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
  return (true);
}


/*****************************************************************************
*
* FUNCTION  SetTitle
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR  Mark Gordon, June 2000
*
* DESCRIPTION
*
*   This routine plots the pixel to the off-screen image storage.
*
* CHANGES
*
*   The current version is pretty much cut & pasted from one Andreas wrote,
*   so I can't really take the credit for it working, just the blame if it
*   causes problems.  I plan to scramble most of the X code in the near
*   future.
*
******************************************************************************/

void SetTitle (void)
{
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
  return;
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

static void     PlotPixel(int x, int y, unsigned char Red, unsigned char Green,
                          unsigned char Blue, unsigned char Alpha)
{
  unsigned long thePixel = 0;
  unsigned int backColor;

  /*
   * If we have a full 8 bits for each color, we can just output
   * directly to the screen, because we only get 8 bits of color
   * data anyways, hence no errors and no need to mess with all
   * the dithering stuff.  Otherwise, we accumulate the color errors,
   * even for TrueColor, in case we have very few colors available
   * like a 332 (8-bit) or 444 (Color NeXT 12-bit).
   */

  if ( Alpha && (opts.Options & OUTPUT_ALPHA)) {
    if ( (x&8) == (y&8) ) {
      backColor = 0xff * (Alpha);
    }
    else {
      backColor = 0xc0 * (Alpha);
    }
    
    Red = ((unsigned int)Red * (255-Alpha) + backColor)/255;
    Blue = ((unsigned int)Blue * (255-Alpha) + backColor)/255;
    Green = ((unsigned int)Green * (255-Alpha) + backColor)/255;    
  }

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
#endif /* (X11_DEBUG > 2) */

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
#endif /* (X11_DEBUG > 2) */

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
#endif /* (X11_DEBUG > 2) */
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
#endif /* (X11_DEBUG > 2) */
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

void XWIN_display_plot(int x, int y, unsigned int Red, unsigned int Green,
                       unsigned int Blue, unsigned int Alpha)
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
* UNOFFICIAL CHANGES (Mark Gordon)
*
*    Dec 5, 1998: Change parameter order per Thorsten Froehlich
*
******************************************************************************/

void XWIN_display_plot_rect(int x1, int y1, int x2, int y2, unsigned int Red,
                unsigned int Green, unsigned int Blue, unsigned int Alpha)
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

  if (fastlittle == false && fastbig == false )
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

void XWIN_display_plot_box(int x1, int y1, int x2, int y2, unsigned int Red,
                unsigned int Green, unsigned int Blue, unsigned int Alpha)
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
    return (HandleXEvents(&theEvent, False) ? true : false);
  }

  return false;
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

static int HandleXError(Display *ErrorDisplay, XErrorEvent *ErrorEvent)
{
  char message[130];

  XGetErrorText(ErrorDisplay, ErrorEvent->error_code, message, 125);
  Warning(0, "\nX error: %s", message);

  return (true);  /* Return value is needed, but is actually ignored */
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

static int HandleXIOError(Display *ErrorDisplay)
{
  theDisplay = NULL;

  Error("\nFatal X Windows error on %s.\n", XDisplayName(theDisplayName));

  return (true);  /* Return value is needed, but is actually ignored */
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

int HandleXEvents(XEvent *theEvent, Bool finished)
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

static void SelectVisual(Visual **theVisual, unsigned int *theDepth)
{
  XVisualInfo *theDefaultVisualInfo, *theVisualInfo, *theBestVisualInfo, *p;
  XVisualInfo theTemplate;
  int theDeepestDepth = 0;
  int nItems;
  int i;

  /*
   * Obtain the default VisualInfo for the in case we don't find a better one.
   */
 
  *theVisual = DefaultVisual(theDisplay, theScreen);
 
  theTemplate.visualid = (*theVisual)->visualid;
 
  if ((theBestVisualInfo = theDefaultVisualInfo =
       XGetVisualInfo(theDisplay, VisualIDMask, &theTemplate, &nItems)) == NULL)
  {
    Error("\nError obtaining X visual info for %s.\n",
           XDisplayName(theDisplayName));
  }

  /*
   * Get a list of all visuals on the screen.
   */

  theTemplate.screen = theScreen;

  if ((theVisualInfo =
     XGetVisualInfo(theDisplay, VisualScreenMask, &theTemplate, &nItems)) == NULL)
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
      if ((pclass(p) == theClass) && (p->depth > *theDepth))
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
      if ((pclass(p) == TrueColor) && (p->depth > *theDepth))
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
      if ((pclass(p) == DirectColor) && (p->depth > *theDepth))
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
      if ((pclass(p) == PseudoColor) && (p->depth > *theDepth))
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
      if ((pclass(p) == StaticColor) && (p->depth > *theDepth))
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
      if ((pclass(p) == StaticGray) && (p->depth > *theDepth))
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
  theClass = pclass(theBestVisualInfo);
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

static void BuildColormap(Visual *theVisual)
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

static void GetBestColormap(Visual *theVisual)
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

static void GetBestGraymap(Visual *theVisual)
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

static void GetBestIndex(long r, long g, long b, XColor *theCell)
{
  static int numAlloc = 0, count = 200;
  unsigned long bestDist = 0xffffffffL;
  int bestIndex = 0, newIndex = false, nCell;

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
      newIndex = true;
    }
    else if (XAllocColor(theDisplay, theColormap, &want))
    {
      newIndex = true;
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

static XColor *GetBestColor (long r, long g, long b)
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

#endif

/* end of X code */







/* beginning of SVGA code */

#ifdef HAVE_LIBVGA

/*****************************************************************************
*
* FUNCTION SVGA_init_povray
*
* INPUT    None
*
* OUTPUT   
*
* RETURNS  None
*
* AUTHOR   jepler@inetnebr.com <Jeff Epler>
*
* DESCRIPTION
*
*    Starts SVGA povray, and grabs access to the actual video memory.  This
*    is why we need to make s-povray suid root.
*
* CHANGES
*
******************************************************************************/

void SVGA_init_povray (void)
{
  /* if display is turned off, revert to non-SVGA, or non-root users won't
     be able to run it from the console */
  if (!(opts.Options & DISPLAY))
    {
      UNIX_finish_povray = &POV_Std_Finish_Povray;
      UNIX_display_init = &POV_Std_Display_Init;
      UNIX_display_plot = &POV_Std_Display_Plot;
      UNIX_display_plot_rect = &POV_Std_Display_Plot_Rect;
      UNIX_display_plot_box = &POV_Std_Display_Plot_Box;
      UNIX_display_finished = &POV_Std_Display_Finished;
      UNIX_display_close = &POV_Std_Display_Close;
      UNIX_Test_Abort = &POV_Std_Test_Abort;
      return;
    }
  seteuid(0);
  vga_init();
  seteuid(getuid());
  vga_runinbackground(1);
}

/*****************************************************************************
*
* FUNCTION  SVGA_display_init
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
*     Initializes the SVGA display using the best graphics mode possible.
*
* CHANGES
*
*     Fixed video mode selection, added grayscale palette.  July 22, 1996 [AED]
*
******************************************************************************/

#define MODE(a, b, c) G ## a ## x ## b ## x ## c
#define TRY_MODE(x, y)\
  if (w <= x && h <= y) {\
    int trymode;\
    if (opts.PaletteOption == 'T') trymode = MODE(x, y, 16M);\
    else if (opts.PaletteOption == 'H') trymode = MODE(x, y, 32K);\
    else trymode = MODE(x, y, 256);\
    if (vga_hasmode(trymode))\
    {\
      mode = trymode;\
      break;\
    }\
  }

int SVGA_display_init(int width, int height)
{
  int i, w, h;
  int mode = TEXT;

  /* Should we do something with vga_getdefaultmode() here first? */

TRY_MODES:
  scale = 1;
  w = width;
  h = height;

  while (mode == TEXT && scale <= 8) {
#if (SVGA_DEBUG > 0)
    Debug_Info("Trying palette mode %c at scale %d\n",opts.PaletteOption,scale);
#endif
    TRY_MODE(320, 200);
    TRY_MODE(640, 480);
    TRY_MODE(800, 600);
    TRY_MODE(1024, 768);
    TRY_MODE(1280, 1024);
    scale++;
    w = width / scale;
    h = height / scale;
  }

  if (mode == TEXT) {
    if (opts.PaletteOption == 'T') {
      Render_Info("\nUnable to find true-color SVGA modes.  "
                  "Trying high-color modes.");
      opts.PaletteOption = 'H';
      goto TRY_MODES;
    }
    else if (opts.PaletteOption == 'H') {
      Render_Info("\nUnable to find high-color SVGA modes.  "
                  "Trying paletted modes.");
      opts.PaletteOption = '0';
      goto TRY_MODES;
    }
    else {
      Render_Info("\nUnable to find any useful SVGA modes.  No display.");
      opts.Options &= ~DISPLAY;
      return (false);
    }
  }

#if (SVGA_DEBUG > 0)
  Debug_Info("Using palette mode %c at %dx%d (scale = %d)\n", opts.PaletteOption,
             w, h, scale);
#endif

  if (scale > 1)
    Render_Info("Scaling by %d:1 on-screen\n", scale);

  vga_setmode(mode);
  gl_setcontextvga(mode);
  if (opts.PaletteOption == GREY)
  {
    for (i = 0; i < 256; i++)
    {
      int gray;
      gray = (i + 2) / 4;

      gl_setpalettecolor(i, gray, gray, gray);
    }
  }
  else
  {
    gl_setrgbpalette();
  }
  gl_getcontext(&physicalScreen);
  gl_clearscreen(0);

  xoff = (WIDTH - w)/2;
  yoff = (HEIGHT - h)/2;

  SVGA_display_plot_box(0, 0, width, height, 255, 255, 255, 0);

  gl_setcontextvgavirtual(mode);
  gl_getcontext(&internalScreen);
  gl_clearscreen(0);
  SVGA_display_plot_box(0, 0, width, height, 255, 255, 255, 0);

  /* Allocate space for dithering */
  for (i = 0; i < 3; i++)
  {
    theErrors[i] = (int *)POV_CALLOC(width, sizeof(int), "dither array");
  }
  return (true);
}



/*****************************************************************************
*
* FUNCTION  SVGA_display_close
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
* CHANGES
*
******************************************************************************/

void SVGA_display_close (void)
{
  int i;

  vga_setmode(TEXT);

  for (i = 0; i < 3; i++)
  {
    if (theErrors[i] != NULL)
    {
      POV_FREE(theErrors[i]);
      theErrors[i] = NULL;
    }
  }
}



/*****************************************************************************
*
* FUNCTION  SVGA_display_plot
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
* CHANGES
*
*    Mar 7, 1996:  Added dithering code from X Windows [AED]
*    July 22, 1996: Added grayscale display code [AED]
*
******************************************************************************/

void SVGA_display_plot(int x, int y, unsigned int Red, unsigned int Green,
                       unsigned int Blue, unsigned int Alpha)
{
  int want[3], got[3];
  int i;
  int sx = x / scale + xoff,
      sy = y / scale + yoff;

#if (SVGA_DEBUG > 1)
  Debug_Info("Orig = <%03d, %03d, %03d>  ", Red, Green, Blue);
  Debug_Info("Error = <%03d, %03d, %03d>\n", theErrors[theRED][x],
         theErrors[theGREEN][x], theErrors[theBLUE][x]);
#endif

  want[theRED]   = (int)Red   + theErrors[theRED][x];
  want[theGREEN] = (int)Green + theErrors[theGREEN][x];
  want[theBLUE]  = (int)Blue  + theErrors[theBLUE][x];

  /* clamp colors to the valid range */
  for (i = 0; i < 3; i++)
  {
    want[i] = (want[i] < 0) ? 0 : ((want[i] > 255) ? 255 : want[i]);
  }

  /* Output the pixel, then find what we actually got.  This could be
   * more efficient, but then we'd have to have special cases for all
   * of the color depths, like in the X Windows code.
   */
  if (opts.PaletteOption == GREY)
  {
    gl_setpixel(sx,sy,want[theRED]);
    /* I should be able to use getpixelrgb to do this, but it doesn't work */
    got[theRED] = got[theGREEN] = got[theBLUE] = (want[theRED] / 4) * 4;
  }
  else
  {
    gl_setpixelrgb(sx, sy, want[theRED], want[theGREEN], want[theBLUE]);
    gl_getpixelrgb(sx, sy, &got[theRED], &got[theGREEN], &got[theBLUE]);
  }

#if (SVGA_DEBUG > 1)
  Debug_Info("  Want <%03d, %03d, %03d>",want[theRED],want[theGREEN],want[theBLUE]);
  Debug_Info("  got <%03d, %03d, %03d>\n", got[theRED],got[theGREEN],got[theBLUE]);
#endif

  vga_lockvc();    /* Will this solve all our problems? */
  if ((i = vga_oktowrite())) {
    if (!couldwrite)
      gl_copyscreen(&physicalScreen);

    gl_setcontext(&physicalScreen);
    if (opts.PaletteOption == GREY)
    {
      gl_setpixel(sx,sy,want[theRED]);
    }
    else
    {
      gl_setpixelrgb(sx, sy, want[theRED], want[theGREEN], want[theBLUE]);
    }
    gl_setcontext(&internalScreen);
  }
  vga_unlockvc();
  couldwrite = i;

  /*
   * Compute errors and accumulate in error diffusion array.  As well,
   * since we are keeping all the errors in a single array, things are
   * a little different than usual.  The error array from x = 0 to the
   * current x value are for the next scanline, and the errors to the
   * right are for the current scanline.  This means we have to keep
   * the error for the lower right pixel in a separate location until
   * we use the error from the pixel to the right.
   */

  for (i = 0; i < 3; i++)
  {
    static int drError[3] = {0, 0, 0};
    int error;

    /* find the error between the desired color and actually used color */

    error = want[i] - got[i];

    if (x > 0)
    {
      theErrors[i][x - 1] += (3 * error + 8) / 16;        /* down & left */
    }

    theErrors[i][x] = (5 * error + 8) / 16 + drError[i];  /* down */

    if (sx < Frame.Screen_Width - 1)
    {
      drError[i] = (error + 8)/ 16;                       /* down & right */
      theErrors[i][x + 1] += (7 * error + 8) / 16;        /* right */
    }
    else
    {
      drError[i] = 0;         /* next scanline start with no error */
    }
  }
}



/*****************************************************************************
*
* FUNCTION  SVGA_display_plot_rect
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
*    Draw a block of pixels in the given color.  We call SVGA_display_plot()
*    repeatedly so that we can take advantage of dithering over the whole
*    block.
*
* CHANGES
*
*    Fixed display loop to be horizontal for dithering.  Jul, 1996. [AED]
*    Modified order of parameters for 3.1.  March, 1999 [mtg]
*
******************************************************************************/

void SVGA_display_plot_rect(int x1, int y1, int x2, int y2,
                            unsigned int Red, unsigned int Green,
                            unsigned int Blue, unsigned int Alpha)
{
  int i;
  int xx, yy;
  int height = y2 - y1 + 1;

  for (yy = y1; yy <= y2; yy += scale)
    for (xx = x1; xx <= x2; xx += scale)
      SVGA_display_plot(xx, yy, Red, Green, Blue, Alpha);

  if (x2 < Frame.Screen_Width - 1)
  {
    for (i = 0; i < 3; i++) 
    {
      theErrors[i][x2 + 1] /= height;
    } 
  }

  if (x1 > 0)
  {
    for (i = 0; i < 3; i++) 
    {
      theErrors[i][x1 - 1] /= height;
    } 
  }
}



/*****************************************************************************
*
* FUNCTION     SVGA_display_plot_box
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR       adilger@enel.ucalgary.ca (Andreas Dilger)
*
* DESCRIPTION  Display the outline of a box in the given color
*
* CHANGES      Mar 1996: Creation
*
******************************************************************************/

void SVGA_display_plot_box(int x1, int y1, int x2, int y2,
                           unsigned int Red, unsigned int Green,
                           unsigned int Blue, unsigned int Alpha)
{
  int i, c;
  int sx, sy, ex, ey;

  sx = x1 / scale + xoff;
  ex = (x2 - 1)/ scale + xoff;
  sy = y1/ scale + yoff;
  ey = (y2 - 1)/ scale + yoff;

  if (opts.PaletteOption == GREY)
  {
    c = 255;
  }
  else
  {
    c = gl_rgbcolor(Red, Green, Blue);
  }

  gl_line(sx, sy, ex, sy, c);
  gl_line(sx, ey, ex, ey, c);
  gl_line(sx, sy, sx, ey, c);
  gl_line(ex, sy, ex, ey, c);

  vga_lockvc();   /* Will this solve all our problems? */
  if ((i = vga_oktowrite())) {
    if (!couldwrite)
      gl_copyscreen(&physicalScreen);

    gl_setcontext(&physicalScreen);
    gl_line(sx, sy, ex, sy, c);
    gl_line(sx, ey, ex, ey, c);
    gl_line(sx, sy, sx, ey, c);
    gl_line(ex, sy, ex, ey, c);
    gl_setcontext(&internalScreen);
  }
  vga_unlockvc();
  couldwrite = i;
}

#endif

void (*UNIX_finish_povray) PARAMS((void));
int (*UNIX_display_init) PARAMS((int w, int h));
void (*UNIX_display_plot) PARAMS((int x, int y,
                               unsigned int Red, unsigned int Green,
                               unsigned int Blue, unsigned int Alpha));
void (*UNIX_display_plot_rect) PARAMS((int x1, int y1, int x2, int y2,
                                    unsigned int Red, unsigned int Green,
                                    unsigned int Blue, unsigned int Alpha));
void (*UNIX_display_plot_box) PARAMS((int x1, int y1, int x2, int y2,
                                    unsigned int Red, unsigned int Green,
                                    unsigned int Blue, unsigned int Alpha));
void (*UNIX_display_finished) PARAMS((void));
void (*UNIX_display_close) PARAMS((void));
int (*UNIX_Test_Abort) PARAMS((void));

