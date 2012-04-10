/*******************************************************************************
 * pattern.h
 *
 * This module contains all defines, typedefs, and prototypes for PATTERN.CPP.
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
 * $File: //depot/povray/smp/source/backend/pattern/pattern.h $
 * $Revision: #26 $
 * $Change: 5215 $
 * $DateTime: 2010/11/30 18:41:01 $
 * $Author: chrisc $
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

/* NOTE: FRAME.H contains other pattern stuff. */

#ifndef PATTERN_H
#define PATTERN_H

#include <boost/functional/hash/hash.hpp>

#include "backend/frame.h"
#include "base/fileinputoutput.h"

#ifdef _MSC_VER
	#pragma warning (disable: 4396)
	#include <boost/unordered_map.hpp>
	#pragma warning (default: 4396)
#else
	#include <boost/unordered_map.hpp>
#endif

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define LAST_SPECIAL_PATTERN     BITMAP_PATTERN
#define LAST_NORM_ONLY_PATTERN   DENTS_PATTERN
#define LAST_INTEGER_PATTERN     HEXAGON_PATTERN

enum PATTERN_IDS
{
	NO_PATTERN = 0,
	PLAIN_PATTERN,
	AVERAGE_PATTERN,
	UV_MAP_PATTERN,
	BITMAP_PATTERN,

/* These former normal patterns require special handling.  They too
   must be kep seperate for now.*/
	WAVES_PATTERN,
	RIPPLES_PATTERN,
	WRINKLES_PATTERN,
	BUMPS_PATTERN,
	QUILTED_PATTERN,
	FACETS_PATTERN,
	DENTS_PATTERN,

/* These patterns return integer values.  They must be kept
   together in the list.  Any new integer functions added must be added
   here and the list renumbered. */
	CHECKER_PATTERN,
	OBJECT_PATTERN,
	BRICK_PATTERN,
	CUBIC_PATTERN, // JN2007: Cubic pattern
	SQUARE_PATTERN,
	TRIANGULAR_PATTERN,
	HEXAGON_PATTERN,

/* These patterns return float values.  They must be kept together
   and seperate from those above. */
	BOZO_PATTERN,
	CELLS_PATTERN,
	MARBLE_PATTERN,
	WOOD_PATTERN,
	SPOTTED_PATTERN,
	AGATE_PATTERN,
	GRANITE_PATTERN,
	GRADIENT_PATTERN,
	ONION_PATTERN,
	LEOPARD_PATTERN,
	JULIA_PATTERN,
	JULIA3_PATTERN,
	JULIA4_PATTERN,
	JULIAX_PATTERN,
	MANDEL_PATTERN,
	MANDEL3_PATTERN,
	MANDEL4_PATTERN,
	MANDELX_PATTERN,
	MAGNET1M_PATTERN,
	MAGNET1J_PATTERN,
	MAGNET2M_PATTERN,
	MAGNET2J_PATTERN,
	RADIAL_PATTERN,
	CRACKLE_PATTERN,
	SPIRAL1_PATTERN,
	SPIRAL2_PATTERN,
	PLANAR_PATTERN,
	SPHERICAL_PATTERN,
	BOXED_PATTERN,
	CYLINDRICAL_PATTERN,
	DENSITY_FILE_PATTERN,
	FUNCTION_PATTERN,
	SLOPE_PATTERN,
	AOI_PATTERN,
	PIGMENT_PATTERN,
	IMAGE_PATTERN,
	PAVEMENT_PATTERN,
	TILING_PATTERN
};

/* Pattern flags */

#define NO_FLAGS              0
#define HAS_FILTER            1
#define FULL_BLOCKING         2
#define POST_DONE             4
#define DONT_SCALE_BUMPS_FLAG 8 /* scale bumps for normals */
#define NOISE_FLAG_1         16 /* this flag and the next one work together */
#define NOISE_FLAG_2         32 /* neither=default, 1=orig,2=range,3=perlin */
#define NOISE_FLAGS         NOISE_FLAG_1+NOISE_FLAG_2

#define Destroy_Turb(t) if ((t)!=NULL) POV_FREE(t);

#define RAMP_WAVE     0
#define SINE_WAVE     1
#define TRIANGLE_WAVE 2
#define SCALLOP_WAVE  3
#define CUBIC_WAVE    4
#define POLY_WAVE     5

/* Interpolation types. */

enum
{
	NO_INTERPOLATION        = 0,
	TRILINEAR_INTERPOLATION = 1,
	TRICUBIC_INTERPOLATION  = 2
};

/*****************************************************************************
* Global typedefs
******************************************************************************/

// see boost functional/hash example
class Crackle_Cell_Coord
{
	int x;
	int y;
	int z;
public:
	Crackle_Cell_Coord() : x(0), y(0), z(0) {}
	Crackle_Cell_Coord(int x, int y, int z) : x(x), y(y), z(z) {}

	bool operator==(Crackle_Cell_Coord const& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	friend std::size_t hash_value(Crackle_Cell_Coord const& p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.x);
		boost::hash_combine(seed, p.y);
		boost::hash_combine(seed, p.z);

		return seed;
	}
};

struct Crackle_Cache_Entry
{
	size_t last_used;
	VECTOR data[81];
};

typedef boost::unordered_map<Crackle_Cell_Coord, Crackle_Cache_Entry, boost::hash<Crackle_Cell_Coord> > Crackle_Cache_Type;

/*****************************************************************************
* Global variables
******************************************************************************/


/*****************************************************************************
* Global constants
******************************************************************************/


/*****************************************************************************
* Global functions
******************************************************************************/

DBL Evaluate_TPat (const TPATTERN *TPat, const VECTOR EPoint, const Intersection *Isection, const Ray *ray, TraceThreadData *Thread);
void Init_TPat_Fields (TPATTERN *Tpat);
void Copy_TPat_Fields (TPATTERN *New, const TPATTERN *Old);
void Destroy_TPat_Fields (TPATTERN *Tpat);
void Translate_Tpattern (TPATTERN *Tpattern, const VECTOR Vector);
void Rotate_Tpattern (TPATTERN *Tpattern, const VECTOR Vector);
void Scale_Tpattern (TPATTERN *Tpattern, const VECTOR Vector);
void Transform_Tpattern (TPATTERN *Tpattern, const TRANSFORM *Trans);
DBL quilt_cubic (DBL t,DBL p1,DBL p2);
void Search_Blend_Map (DBL value, const BLEND_MAP *Blend_Map,
                       BLEND_MAP_ENTRY **Prev, BLEND_MAP_ENTRY **Cur);
int GetNoiseGen (const TPATTERN *TPat, const TraceThreadData *Thread);

DENSITY_FILE *Create_Density_File ();
DENSITY_FILE *Copy_Density_File (DENSITY_FILE *);
void Destroy_Density_File (DENSITY_FILE *);
void Read_Density_File (IStream *dfile, DENSITY_FILE *df);
int PickInCube (const VECTOR tv, VECTOR p1);

void InitializePatternGenerators(void);

}

#endif
