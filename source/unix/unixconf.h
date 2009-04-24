/*****************************************************************************
*               unixconf.h
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

/*
 * Below are several defines needed by the Unix parts of POV.  You
 * may redefine them as needed in this config.h file. Each has a default
 * which will be defined in frame.h if you don't define it here.
 */

/*
 * This should use the standard C header files, if they exist, or just
 * our own prototypes if they don't.  Modify to suit your needs.
 */

#ifdef __STDC__
#define PARAMS(x) x
#  ifndef CONST
#  define CONST const
#  endif
#include <stdlib.h>
#include <unistd.h>
#else
#define PARAMS(x) ()
#  ifndef volatile
#  define volatile
#  endif
int unlink(char *);
char *getenv(char *);
void *malloc(size_t);
void free(void *);
double fmod(double, double);
double atof(char *);
#endif

/*
 * Try to use system specified values for the maximum filename length, or
 * at least some Unix sized ones, rather than DOS sized ones.
 */
#if defined (PATH_MAX)
#  define POV_NAME_MAX PATH_MAX
#  define FILE_NAME_MAX PATH_MAX
#elif defined (_POSIX_PATH_MAX)
#  define POV_NAME_MAX _POSIX_PATH_MAX
#  define FILE_NAME_MAX _POSIX_PATH_MAX
#else
#  define POV_NAME_MAX 200
#  define FILE_NAME_MAX 250
#endif

/*
 * Try to read in ONE of the default POV-Ray INI files, in the order:
 * 1) whatever POVINI is pointing to,
 * 2) the file .povrayrc in the user's home directory
 * 3) the file povray.ini in the current directory
 */
#define READ_ENV_VAR \
  if ((Option_String_Ptr = getenv("POVINI")) != NULL) \
    if (!parse_ini_file(Option_String_Ptr)) \
    { \
      Warning(0.0, "Could not find '%s' as specified in POVINI environment.\n",\
              Option_String_Ptr); \
      Option_String_Ptr = NULL; \
    }

#define PROCESS_POVRAY_INI \
  if (Option_String_Ptr==NULL && (Option_String_Ptr = getenv("HOME"))!=NULL) { \
    char ini_name[FILE_NAME_MAX]; \
    strncpy(ini_name, Option_String_Ptr, FILE_NAME_MAX); \
    strncat(ini_name, "/.povrayrc", FILE_NAME_MAX); \
    if (parse_ini_file(ini_name) == FALSE) \
      parse_ini_file("povray.ini"); \
  } else \
    parse_ini_file("povray.ini");

#define TIME_ELAPSED (tstop - tstart);

/*
 * This is the image file format that POV will choose by default if none is
 * specified on the command-line.  Although the default value is already 't'
 * for TGA format files, many Unix sites may want to change this to 'p' to
 * get ppm format files, which more Unix utilities understand.
 */
#define DEFAULT_OUTPUT_FORMAT  't'

/*
 * What level of memory statistics we want.  See mem.h for more info.
 */
#define MEM_STATS 1

/*
 * The UNIX precision timer uses gettimeofday().  If this is unavailable,
 * define PRECISION_TIMER_AVAILABLE to 0, which means you won't be able
 * to generate histogram output files.
 */

#define PRECISION_TIMER_AVAILABLE 1

#if PRECISION_TIMER_AVAILABLE

#define PRECISION_TIMER_INIT
#define PRECISION_TIMER_START UNIX_Timer_Start();
#define PRECISION_TIMER_STOP  UNIX_Timer_Stop();
#define PRECISION_TIMER_COUNT UNIX_Timer_Count();

void UNIX_Timer_Start PARAMS((void));
void UNIX_Timer_Stop  PARAMS((void));
int  UNIX_Timer_Count PARAMS((void));

#endif /* PRECISION_TIMER_AVAILABLE */

/*
 * Some systems don't have setvbuf.  If not, just define it away - it's
 * not critical to the raytracer.  It just sets up the disk buffers.
 * #define setvbuf(w,x,y,z)
 */

/*
 * This sets up the signal handler during a rendering so that if CTRL-C is
 * pressed, the normal user abort function is called.  If your system
 * doesn't support signal(), you can try to modify these functions to use
 * an available signal function, or just comment these out, although
 * in this case you will lose anything in the output buffer if the trace
 * is interrupted with CTRL-C.
 */
#define POV_PRE_RENDER   UNIX_Abort_Start();
#define POV_PRE_SHUTDOWN UNIX_Abort_Handler(0);
void UNIX_Abort_Start PARAMS((void));
void UNIX_Abort_Handler PARAMS((int signum));

