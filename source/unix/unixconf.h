/*****************************************************************************
*               unixconf.h
*
*  This header file contains all constants and types required to run on a
*  UNIX system.
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
* Modifications by Mike Fleetwood, February 1999
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
 * Define the default location of the POV-Ray library
 */
#ifndef POV_LIB_DIR
#define POV_LIB_DIR    "/usr/local/lib/povray31"
#endif

/**********************************************************/
/*
 * Search for and process a default POV-Ray INI file.  The first file
 * from the list below is used:
 *     $POVINI                          Environment over-ride defaults file
 *     ./povray.ini                     Per-directory defaults file
 *     $HOME/.povrayrc                  User defaults file
 *     POV_LIB_DIR/povray.ini           System wide fall-back defaults file
 */
#define READ_ENV_VAR           UNIX_Process_Env();
#define PROCESS_POVRAY_INI     UNIX_Process_Povray_Ini();

void UNIX_Process_Env          PARAMS((void));
void UNIX_Process_Povray_Ini   PARAMS((void));

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

#define MAIN_RETURN_TYPE       int
#define MAIN_RETURN_STATEMENT  return 0;




