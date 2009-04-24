/****************************************************************************
*                   mem.h
*
*  This module contains all defines, typedefs, and prototypes for mem.c
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


#ifndef MEM_H
#define MEM_H



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

void mem_init PARAMS((void));
void mem_mark PARAMS((void));
void mem_release PARAMS((int LogFile));
void mem_release_all PARAMS((int LogFile));
void *pov_malloc PARAMS((size_t size, char *file, int line, char *msg));
void *pov_calloc PARAMS((size_t nitems, size_t size, char *file, int line, char *msg));
void *pov_realloc PARAMS((void *ptr, size_t size, char *file, int line, char *msg));
void pov_free PARAMS((void *ptr, char *file, int line));

#if defined(MEM_STATS)
/* These are level 1 routines */
size_t mem_stats_current_mem_usage PARAMS((void));
size_t mem_stats_largest_mem_usage PARAMS((void));
size_t mem_stats_smallest_alloc PARAMS((void));
size_t mem_stats_largest_alloc PARAMS((void));
/* These are level 2 routines */
#if (MEM_STATS>=2)
char* mem_stats_smallest_file PARAMS((void));
int mem_stats_smallest_line PARAMS((void));
char* mem_stats_largest_file PARAMS((void));
int mem_stats_largest_line PARAMS((void));
long int mem_stats_total_allocs PARAMS((void));
long int mem_stats_total_frees PARAMS((void));
#endif
#endif

void *pov_memmove PARAMS((void *dest, void *src, size_t length));

#endif /* MEM_H */
