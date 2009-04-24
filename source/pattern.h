/****************************************************************************
*                   pattern.h
*
*  This module contains all defines, typedefs, and prototypes for PATTERN.C.
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

/* NOTE: FRAME.H contains other pattern stuff. */

#ifndef PATTERN_H
#define PATTERN_H


/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define NO_PATTERN               0
#define PLAIN_PATTERN            1
#define AVERAGE_PATTERN          2
#define BITMAP_PATTERN           3

#define LAST_SPECIAL_PATTERN     BITMAP_PATTERN

/* These former normal patterns require special handling.  They too
   must be kep seperate for now.*/

#define WAVES_PATTERN            4
#define RIPPLES_PATTERN          5
#define WRINKLES_PATTERN         6
#define BUMPS_PATTERN            7
#define BUMPY1_PATTERN           8
#define BUMPY2_PATTERN           9
#define BUMPY3_PATTERN          10
#define QUILTED_PATTERN         11
#define DENTS_PATTERN           12

#define LAST_NORM_ONLY_PATTERN    DENTS_PATTERN

/* These patterns return integer values.  They must be kept
   together in the list.  Any new integer functions added must be added
   here and the list renumbered. */
   
#define CHECKER_PATTERN         13
#define BRICK_PATTERN           14
#define HEXAGON_PATTERN         15

#define LAST_INTEGER_PATTERN     HEXAGON_PATTERN

/* These patterns return float values.  They must be kept together
   and seperate from those above. */

#define BOZO_PATTERN            16
#define MARBLE_PATTERN          17
#define WOOD_PATTERN            18
#define SPOTTED_PATTERN         19
#define AGATE_PATTERN           20
#define GRANITE_PATTERN         21
#define GRADIENT_PATTERN        22
#define PATTERN1_PATTERN        23
#define PATTERN2_PATTERN        24
#define PATTERN3_PATTERN        25
#define ONION_PATTERN           26
#define LEOPARD_PATTERN         27
#define MANDEL_PATTERN          28
#define RADIAL_PATTERN          29
#define CRACKLE_PATTERN         30
#define SPIRAL1_PATTERN         31
#define SPIRAL2_PATTERN         32
#define INCIDENCE_PATTERN       33


/* Pattern flags */

#define NO_FLAGS      0
#define HAS_FILTER    1
#define FULL_BLOCKING 2
#define POST_DONE     4

#define Destroy_Turb(t) if ((t)!=NULL) POV_FREE(t);

#define RAMP_WAVE     0
#define SINE_WAVE     1
#define TRIANGLE_WAVE 2
#define SCALLOP_WAVE  4


/*****************************************************************************
* Global typedefs
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global constants
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

DBL Evaluate_TPat PARAMS((TPATTERN *TPat, VECTOR EPoint));
void Init_TPat_Fields PARAMS((TPATTERN *Tpat));
void Copy_TPat_Fields PARAMS((TPATTERN *New, TPATTERN *Old));
void Destroy_TPat_Fields PARAMS((TPATTERN *Tpat));
void Translate_Tpattern PARAMS((TPATTERN *Tpattern, VECTOR Vector));
void Rotate_Tpattern PARAMS((TPATTERN *Tpattern, VECTOR Vector));
void Scale_Tpattern PARAMS((TPATTERN *Tpattern, VECTOR Vector));
void Transform_Tpattern PARAMS((TPATTERN *Tpattern, TRANSFORM *Trans));
DBL quilt_cubic PARAMS((DBL t,DBL p1,DBL p2));
void Search_Blend_Map PARAMS((DBL value,BLEND_MAP *Blend_Map,
 BLEND_MAP_ENTRY **Prev, BLEND_MAP_ENTRY **Cur));



#endif
