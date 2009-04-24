/****************************************************************************
*                os2conf.h
*
*  This header file contains all constants and types required to compile a
*  text mode only EXE of POV-Ray for use with OS/2 2.x.
*  RENAME this file to CONFIG.H before compiling.
*
*  Developed for & with IBM's CSet/2 C/C++ compiler.
*
*  From Persistence of Vision Raytracer 
*  Copyright 1991, 1992, 1993 Persistence of Vision Team
*  Bill Pulver 2-1-93
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum. (GO GRAPHDEV) The latest version of POV-Ray may be found there 
*  as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/
/*  Below are several defines needed by the generic parts of POV.  You
*   may redefine them as needed in this config.h file.
*
*   The following constants are needed by various POV modules.  Each has
*   a default (shown below) which is will be defined in frame.h if you don't 
*   define them here.
*
*   #define EPSILON 1.0e-10               - a small value used for POLY
*   #define FILE_NAME_LENGTH 150          - default file name length
*   #define HUGE_VAL 1.0e+17              - a really big number
*   #define DBL_FORMAT_STRING "%lf"       - the string to use for scanfs of doubles
*   #define DEFAULT_OUTPUT_FORMAT  'd'    - the default +f format
*   #define RED_RAW_FILE_EXTENSION ".red" - for +fr, this is the extension for the
*                                           red file
*   #define GREEN_RAW_FILE_EXTENSION ".grn"  - ditto for the green file
*   #define BLUE_RAW_FILE_EXTENSION ".blu"   - ditto again for the blue file
*   #define FILENAME_SEPARATOR "/"        - the character that separates names
*                                           in a path.
*   #define CASE_SENSITIVE_DEFAULT 2     - controls case sensitivity in DAT files
*   #define READ_FILE_STRING "rb"        - special binary (raw, uncooked) 
*   #define WRITE_FILE_STRING "wb"         modes of fopen()
*   #define APPEND_FILE_STRING "ab"
*   #define NORMAL '0'                   - machine-specific PaletteOption settings 
*   #define GREY   'G'
*
*   These routines are required by POV to compute trace times.  The defaults 
*   shown below are defined in most versions of C.  You may redefine them if
*   your compiler doesn't support these.  If time is completely unsupported 
*   define them as 0.
*
*   #define START_TIME time(&tstart);     
*   #define STOP_TIME  time(&tstop);
*   #define TIME_ELAPSED difftime (tstop, tstart);
*
*   Note difftime can be replaced with: #define TIME_ELAPSED (tstop - tstart); 
*   in some cases. 
*
*   These are optional routines that POV calls.  You may define them
*   or if undefined they will be defined as empty in frame.h.
*
*   #define STARTUP_POVRAY       - first executable statement of main
*                                  (useful for initialization)
*   #define PRINT_OTHER_CREDITS  - used for people who extend the machine specific modules
*                                  (put your own printf's here)
*   #define TEST_ABORT           - code to test for ^C or abort
*                                  (called on each pixel)
*   #define FINISH_POVRAY        - last statement before exiting normally
*   #define COOPERATE            - called for multi-tasking
*  
*   These items are standard on some C compilers.  If they are not defined
*   on your compiler or are called something different you may define your 
*   own or use the defaults supplied in frame.h as shown below.
*
*   #define DBL double                    - floating point precision
*   #define ACOS acos                     - arc cosine of DBL
*   #define SQRT sqrt                     - square root of DBL
*   #define POW pow                       - x to the y power - x, y DBL
*   #define COS cos                       - cosine of DBL
*   #define SIN sin                       - sine of DBL
*   #define labs(x) (long) ((x<0)?-x:x)   - Returns the absolute value of long integer x
*   #define max (x,y) ((x<y)?y:x)         - greater of x and y
*
*   On some systems "sprintf" returns the string rather than the length
*   of the string.  If the string is returned, define the following:
*   #define STRLN(x) strlen(x)
*
*   #define setvbuf(w,x,y,z)              - some systems don't understand setvbuf.
*                                           If not, just define it away - it's
*                                           not critical to the raytracer.  It
*                                           just buffers disk accesses.
*   This macro controls prototyping and is required by POV.  Choose one
*   of the two options below or if left undefined it will default to ANSI 
*   in frame.h
*
*   #define PARAMS (x) x                  - do ANSI prototyping
*   #define PARAMS (x) ()                 - do K&R non-ANSI prototyping
*
*   These allow command line vs non-command line versions and alternate
*   main routines.  Normally they should be undefined.  Define them 1 if needed.
*   #define ALTMAIN 1
*   #define NOCMDLINE 1
****************************************************/

#ifdef _STDC_
#define PARAMS(x) x
#else
#define PARAMS(x) ()
#define volatile
#endif

#define COMPILER_VER ".ibmCset/2++"
#define DBL double


/* Mask floating point underflow, etc. for IBM C Set/2, 2 ways are          */
/* defined below.                                                           */
/* IBM C Set/2 does not trap open matherr, we must use _control87 to trap   */
/* them. We must also disable stream buffering. NOTE: The I/O buffer        */
/* disabling is handled as MACRO invoked from the makefile in this case.    */
/* (NOPRNBUF)                                                               */

#include <conio.h>                       
#include <float.h>

/* #define CONFIG_MATH _control87 (CW_DEFAULT + EM_UNDERFLOW + EM_OVERFLOW + EM_ZERODIVIDE, MCW_EM);*/
#define CONFIG_MATH _control87(MCW_EM, MCW_EM);

#ifdef NOPRNBUF
void Initialize (void)
{
  /* Disable buffering for ...printf calls for C Set/2 */
  setvbuf (stdout, NULL, _IONBF, 0);
  setvbuf (stderr, NULL, _IONBF, 0);
}
#define STARTUP_POVRAY Initialize ();
#endif



/* Define default output file format as TARGA */
#define DEFAULT_OUTPUT_FORMAT   't'



/* Allow Keyboard abort, slows down rendering quite a bit under OS/2 due to */
/* KB polling. IDLE_TIME may be able to compensate to a certain extent....  */
/* NOTE THAT YOU MAY NEED TO ADD/DELETE UNDERSCORES IN FRONT OF kbhit AND   */
/* getch. It compiles fine either way for me but did not work for another   */
/* person until the "_"'s were deleted.......                               */

#define TEST_ABORT if (Options & EXITENABLE) if (kbhit()) { Stop_Flag = TRUE; getch(); }

#define EPSILON 1.0e-15


/* Need prototypes for these standard routines */

char *getenv PARAMS((char *name));
char *malloc PARAMS((int size));
void free PARAMS((void *));
#define MAIN_TYPE int
