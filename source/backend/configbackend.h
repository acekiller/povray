/*******************************************************************************
 * configbackend.h
 *
 * This header file defines all types that can be configured by platform
 * specific code for backend use. It further allows insertion of platform
 * specific function prototypes making use of those types.
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 1991-2003 Persistence of Vision Team
 * Copyright 2003-2009 Persistence of Vision Raytracer Pty. Ltd.
 * ---------------------------------------------------------------------------
 * NOTICE: This source code file is provided so that users may experiment
 * with enhancements to POV-Ray and to port the software to platforms other
 * than those supported by the POV-Ray developers. There are strict rules
 * regarding how you are permitted to use this file. These rules are contained
 * in the distribution and derivative versions licenses which should have been
 * provided with this file.
 *
 * These licences may be found online, linked from the end-user license
 * agreement that is located at http://www.povray.org/povlegal.html
 * ---------------------------------------------------------------------------
 * POV-Ray is based on the popular DKB raytracer version 2.12.
 * DKBTrace was originally written by David K. Buck.
 * DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
 * ---------------------------------------------------------------------------
 * $File: //depot/povray/smp/source/backend/configbackend.h $
 * $Revision: #16 $
 * $Change: 5490 $
 * $DateTime: 2011/09/14 16:28:15 $
 * $Author: clipka $
 *******************************************************************************/

/*********************************************************************************
 * NOTICE
 *
 * This file is part of a BETA-TEST version of POV-Ray version 3.7. It is not
 * final code. Use of this source file is governed by both the standard POV-Ray
 * licences referred to in the copyright header block above this notice, and the
 * following additional restrictions numbered 1 through 4 below:
 *
 *   1. This source file may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd.
 *
 *   2. This notice may not be altered or removed.
 *   
 *   3. Binaries generated from this source file by individuals for their own
 *      personal use may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd. Such personal-use binaries
 *      are not required to have a timeout, and thus permission is granted in
 *      these circumstances only to disable the timeout code contained within
 *      the beta software.
 *   
 *   4. Binaries generated from this source file for use within an organizational
 *      unit (such as, but not limited to, a company or university) may not be
 *      distributed beyond the local organizational unit in which they were made,
 *      unless written permission is obtained from Persistence of Vision Raytracer
 *      Pty. Ltd. Additionally, the timeout code implemented within the beta may
 *      not be disabled or otherwise bypassed in any manner.
 *
 * The following text is not part of the above conditions and is provided for
 * informational purposes only.
 *
 * The purpose of the no-redistribution clause is to attempt to keep the
 * circulating copies of the beta source fresh. The only authorized distribution
 * point for the source code is the POV-Ray website and Perforce server, where
 * the code will be kept up to date with recent fixes. Additionally the beta
 * timeout code mentioned above has been a standard part of POV-Ray betas since
 * version 1.0, and is intended to reduce bug reports from old betas as well as
 * keep any circulating beta binaries relatively fresh.
 *
 * All said, however, the POV-Ray developers are open to any reasonable request
 * for variations to the above conditions and will consider them on a case-by-case
 * basis.
 *
 * Additionally, the developers request your co-operation in fixing bugs and
 * generally improving the program. If submitting a bug-fix, please ensure that
 * you quote the revision number of the file shown above in the copyright header
 * (see the '$Revision:' field). This ensures that it is possible to determine
 * what specific copy of the file you are working with. The developers also would
 * like to make it known that until POV-Ray 3.7 is out of beta, they would prefer
 * to emphasize the provision of bug fixes over the addition of new features.
 *
 * Persons wishing to enhance this source are requested to take the above into
 * account. It is also strongly suggested that such enhancements are started with
 * a recent copy of the source.
 *
 * The source code page (see http://www.povray.org/beta/source/) sets out the
 * conditions under which the developers are willing to accept contributions back
 * into the primary source tree. Please refer to those conditions prior to making
 * any changes to this source, if you wish to submit those changes for inclusion
 * with POV-Ray.
 *
 *********************************************************************************/

#ifndef CONFIGBACKEND_H
#define CONFIGBACKEND_H

#include "syspovconfigbackend.h"

/*
 * Platform name default.
 */
#ifndef POVRAY_PLATFORM_NAME
	#define POVRAY_PLATFORM_NAME "Unknown Platform"
#endif

/*
 * These functions define macros which do checking for memory allocation,
 * and can also do other things.  Check existing code before you change them,
 * since they aren't simply replacements for malloc, calloc, realloc, and free.
 */
#ifndef POV_MALLOC
#define POV_MALLOC(size,msg)        pov_malloc ((size), __FILE__, __LINE__, (msg))
#endif

#ifndef POV_CALLOC
	#define POV_CALLOC(nitems,size,msg) pov_calloc ((nitems), (size), __FILE__, __LINE__, (msg))
#endif

#ifndef POV_REALLOC
	#define POV_REALLOC(ptr,size,msg)   pov_realloc ((ptr), (size), __FILE__, __LINE__, (msg))
#endif

#ifndef POV_FREE
	#define POV_FREE(ptr)               do { pov_free ((void *)(ptr), __FILE__, __LINE__); (ptr) = NULL; } while(false)
#endif

#ifndef POV_MEM_INIT
	#define POV_MEM_INIT()              mem_init()
#endif

#ifndef POV_MEM_RELEASE_ALL
	#define POV_MEM_RELEASE_ALL()       mem_release_all()
#endif

#ifndef POV_STRDUP
	#define POV_STRDUP(str)             pov_strdup(str)
#endif

// For those systems that don't have memmove, this can also be pov_memmove
#ifndef POV_MEMMOVE
	#define POV_MEMMOVE(dst,src,len)    pov_memmove((dst),(src),(len))
#endif

#ifndef POV_MEMCPY
	#define POV_MEMCPY(dst,src,len)     memcpy((dst),(src),(len))
#endif

#ifndef POV_MEM_STATS
	#define POV_MEM_STATS                       0
	#define POV_GLOBAL_MEM_STATS(a,f,c,p,s,l)   (false)
	#define POV_THREAD_MEM_STATS(a,f,c,p,s,l)   (false)
	#define POV_MEM_STATS_RENDER_BEGIN()
	#define POV_MEM_STATS_RENDER_END()
	#define POV_MEM_STATS_COOKIE                void *
#endif

/*
 * To allow GUI platforms like the Mac to access a command line and provide
 * a command line only interface (for debugging) a different call to an
 * internal function of the standard library is required. This macro takes
 * both argc and argv and is expected to return argc.
 */
#ifndef GETCOMMANDLINE
	#define GETCOMMANDLINE(ac,av) ac
#endif

#ifndef CONFIG_MATH       // Macro for setting up any special FP options
	#define CONFIG_MATH
#endif

/* Specify number of source file lines printed before error line, their maximum length and
 * the error marker text that is appended to mark the error
 */
#ifndef POV_NUM_ECHO_LINES
	#define POV_NUM_ECHO_LINES 5
#endif

#ifndef POV_ERROR_MARKER_TEXT
	#define POV_ERROR_MARKER_TEXT " <----ERROR\n"
#endif

#ifndef POV_WHERE_ERROR
	#define POV_WHERE_ERROR(fn,ln,cl,ts)
#endif

// Default for Max_Trace_Level
#ifndef MAX_TRACE_LEVEL_DEFAULT
	#define MAX_TRACE_LEVEL_DEFAULT 5
#endif

// Upper bound for max_trace_level specified by the user
#ifndef MAX_TRACE_LEVEL_LIMIT
	#define MAX_TRACE_LEVEL_LIMIT 256
#endif

// Various numerical constants that are used in the calculations
#ifndef EPSILON     // A small value used to see if a value is nearly zero
	#define EPSILON 1.0e-10
#endif

#ifndef HUGE_VAL    // A very large value, can be considered infinity
	#define HUGE_VAL 1.0e+17
#endif

/*
 * If the width of a bounding box in one dimension is greater than
 * the critical length, the bounding box should be set to infinite.
 */

#ifndef CRITICAL_LENGTH
	#define CRITICAL_LENGTH 1.0e+6
#endif

#ifndef BOUND_HUGE  // Maximum lengths of a bounding box.
	#define BOUND_HUGE 2.0e+10
#endif

/*
 * These values determine the minimum and maximum distances
 * that qualify as ray-object intersections.
 */

//#define SMALL_TOLERANCE 1.0e-6 // TODO FIXME #define SMALL_TOLERANCE 0.001
//#define MAX_DISTANCE 1.0e+10 // TODO FIXME #define MAX_DISTANCE 1.0e7
#define SMALL_TOLERANCE 0.001
#define MAX_DISTANCE 1.0e7

#define MIN_ISECT_DEPTH 1.0e-4

#ifndef DBL_FORMAT_STRING
	#define DBL_FORMAT_STRING "%lf"
#endif

// Some implementations of scanf return 0 on failure rather than EOF
#ifndef SCANF_EOF
	#define SCANF_EOF EOF
#endif

// Adjust to match floating-point parameter(s) of functions in math.h/cmath
#ifndef SYS_MATH_PARAM
	#define SYS_MATH_PARAM double
#endif

// Adjust to match floating-point return value of functions in math.h/cmath
#ifndef SYS_MATH_RETURN
	#define SYS_MATH_RETURN double
#endif

// Function that executes functions, the parameter is the function index
#ifndef POVFPU_Run
	#define POVFPU_Run(ctx, fn) POVFPU_RunDefault(ctx, fn)
#endif

// Adjust to add system specific handling of functions like just-in-time compilation
#if (SYS_FUNCTIONS == 0)

// Note that if SYS_FUNCTIONS is 1, it will enable the field dblstack
// in FPUContext_Struct and corresponding calculations in POVFPU_SetLocal
// as well as POVFPU_NewContext.
#define SYS_FUNCTIONS 0

// Called after a function has been added, parameter is the function index
#define SYS_ADD_FUNCTION(fe)
// Called before a function is deleted, parameter is a pointer to the FunctionEntry_Struct
#define SYS_DELETE_FUNCTION(fe)
// Called inside POVFPU_Init after everything else has been inited
#define SYS_INIT_FUNCTIONS()
// Called inside POVFPU_Terminate before anything else is deleted
#define SYS_TERM_FUNCTIONS()
// Called inside POVFPU_Reset before anything else is reset
#define SYS_RESET_FUNCTIONS()

// Adjust to add system specific fields to FunctionEntry_Struct
#define SYS_FUNCTION_ENTRY

#endif // SYS_FUNCTIONS

#ifndef POV_SYS_THREAD_STARTUP
	#define POV_SYS_THREAD_STARTUP
#endif

#ifndef POV_SYS_THREAD_CLEANUP
	#define POV_SYS_THREAD_CLEANUP
#endif

#ifndef CDECL
	#define CDECL
#endif

#ifndef ALIGN16
	#define ALIGN16
#endif

#ifndef FORCEINLINE
	#define FORCEINLINE inline
#endif

#ifndef INLINE_NOISE
	#define INLINE_NOISE
#endif

#ifndef USE_FASTER_NOISE
	#define USE_FASTER_NOISE 0
#endif

#ifndef NEW_LINE_STRING
	#define NEW_LINE_STRING "\n"
#endif

// If compiler version is undefined, then make it 'u' for unknown
#ifndef COMPILER_VER
	#define COMPILER_VER ".u"
#endif

#ifndef QSORT
	#define QSORT(a,b,c,d) qsort((a),(b),(c),(d))
#endif

#ifndef POV_PARSE_PATH_STRING
	#error "A valid POV_PARSE_PATH_STRING macro is required!"
#endif


/*
 * Font related macros [trf]
 */
#ifndef POV_CONVERT_TEXT_TO_UCS2
	#define POV_CONVERT_TEXT_TO_UCS2(ts, tsl, as) (NULL)
#endif

#ifndef POV_ALLOW_FILE_READ
	#define POV_ALLOW_FILE_READ(f,t) (1)
#endif

#ifndef POV_ALLOW_FILE_WRITE
	#define POV_ALLOW_FILE_WRITE(f,t) (1)
#endif

#include "syspovprotobackend.h"

#endif
