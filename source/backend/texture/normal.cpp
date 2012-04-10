/*******************************************************************************
 * normal.cpp
 *
 * This module implements solid texturing functions that perturb the surface
 * normal to create a bumpy effect. 
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
 * $File: //depot/povray/smp/source/backend/texture/normal.cpp $
 * $Revision: #29 $
 * $Change: 5179 $
 * $DateTime: 2010/11/01 13:56:10 $
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

/*
 * Some texture ideas garnered from SIGGRAPH '85 Volume 19 Number 3,
 * "An Image Synthesizer" By Ken Perlin.
 *
 * Further Ideas Garnered from "The RenderMan Companion" (Addison Wesley)
 */

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/texture/normal.h"
#include "backend/texture/texture.h"
#include "backend/texture/pigment.h"
#include "backend/support/imageutil.h"
#include "backend/scene/objects.h"
#include "backend/scene/threaddata.h"
#include "backend/math/vector.h"
#include "base/pov_err.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Local constants
******************************************************************************/

static const VECTOR Pyramid_Vect [4]= {{ 0.942809041,-0.333333333, 0.0},
                                       {-0.471404521,-0.333333333, 0.816496581},
                                       {-0.471404521,-0.333333333,-0.816496581},
                                       { 0.0        , 1.0        , 0.0}};

/*****************************************************************************
* Static functions
******************************************************************************/

static void ripples (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR Vector, const TraceThreadData *Thread);
static void waves (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR Vector, const TraceThreadData *Thread);
static void bumps (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal);
static void dents (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, const TraceThreadData *Thread);
static void wrinkles (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal);
static void quilted (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal);
static DBL Hermite_Cubic (DBL T1, const UV_VECT UV1, const UV_VECT UV2);
static DBL Do_Slope_Map (DBL value, const BLEND_MAP *Blend_Map);
static void Do_Average_Normals (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, Intersection *Inter, const Ray *ray, TraceThreadData *Thread);
static void facets (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, TraceThreadData *Thread);

/*****************************************************************************
*
* FUNCTION
*
*   ripples
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void ripples (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, const TraceThreadData *Thread)
{
	register unsigned int i;
	register DBL length, scalar, index;
	VECTOR point;

	for (i = 0 ; i < Thread->numberOfWaves ; i++)
	{
		VSub (point, EPoint, *Thread->waveSources[i]);
		VLength (length, point);

		if (length == 0.0)
			length = 1.0;

		index = length * Tnormal->Frequency + Tnormal->Phase;

		scalar = cycloidal(index) * Tnormal ->Amount;

		VAddScaledEq(normal, scalar / (length * (DBL)Thread->numberOfWaves), point);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   waves
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void waves (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, const TraceThreadData *Thread)
{
	register unsigned int i;
	register DBL length, scalar, index, sinValue ;
	VECTOR point;

	for (i = 0 ; i < Thread->numberOfWaves ; i++)
	{
		VSub (point, EPoint, *Thread->waveSources[i]);

		VLength (length, point);

		if (length == 0.0)
		{
			length = 1.0;
		}

		index = length * Tnormal->Frequency * Thread->waveFrequencies[i] + Tnormal->Phase;

		sinValue = cycloidal(index);

		scalar = sinValue * Tnormal->Amount / Thread->waveFrequencies[i];

		VAddScaledEq(normal, scalar / (length * (DBL)Thread->numberOfWaves), point);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   bumps
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void bumps (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal)
{
	VECTOR bump_turb;

	/* Get normal displacement value. */

	DNoise (bump_turb, EPoint);

	/* Displace "normal". */

	VAddScaledEq(normal, Tnormal->Amount, bump_turb);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*   Dents is similar to bumps, but uses noise() to control the amount of
*   dnoise() perturbation of the object normal...
*
* CHANGES
*
******************************************************************************/

static void dents (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, const TraceThreadData *Thread)
{
	DBL noise;
	VECTOR stucco_turb;

	noise = Noise (EPoint, GetNoiseGen((TPATTERN*)Tnormal, Thread));

	noise = noise * noise * noise * Tnormal->Amount;

	/* Get normal displacement value. */

	DNoise(stucco_turb, EPoint);

	/* Displace "normal". */

	VAddScaledEq(normal, noise, stucco_turb);
}




/*****************************************************************************
*
* FUNCTION
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   Wrinkles - This is my implementation of the dented() routine, using
*   a surface iterative fractal derived from DTurbulence.
*
*   This is a 3-D version (thanks to DNoise()...) of the usual version
*   using the singular Noise()...
*
*   Seems to look a lot like wrinkles, however... (hmmm)
*
*   Idea garnered from the April 89 Byte Graphics Supplement on RenderMan,
*   refined from "The RenderMan Companion, by Steve Upstill of Pixar,
*   (C) 1990 Addison-Wesley.
*
* CHANGES
*
******************************************************************************/

static void wrinkles (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal)
{
	register int i;
	register DBL scale = 1.0;
	VECTOR result, value, value2;

	Make_Vector(result, 0.0, 0.0, 0.0);

	for (i = 0; i < 10; scale *= 2.0, i++)
	{
		VScale(value2,EPoint,scale);
		DNoise(value, value2);

		result[X] += fabs(value[X] / scale);
		result[Y] += fabs(value[Y] / scale);
		result[Z] += fabs(value[Z] / scale);
	}

	/* Displace "normal". */

	VAddScaledEq(normal, Tnormal->Amount, result);
}


/*****************************************************************************
*
* FUNCTION
*
*   quilted
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dan Farmer '94
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void quilted (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal)
{
	VECTOR value;
	DBL t;

	value[X] = EPoint[X]-FLOOR(EPoint[X])-0.5;
	value[Y] = EPoint[Y]-FLOOR(EPoint[Y])-0.5;
	value[Z] = EPoint[Z]-FLOOR(EPoint[Z])-0.5;

	t = sqrt(value[X]*value[X]+value[Y]*value[Y]+value[Z]*value[Z]);

	t = quilt_cubic(t, Tnormal->Vals.Quilted.Control0, Tnormal->Vals.Quilted.Control1);

	value[X] *= t;
	value[Y] *= t;
	value[Z] *= t;

	VAddScaledEq (normal, Tnormal->Amount,value);
}

/*****************************************************************************
*
* FUNCTION
*
*   facets
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Ronald Parker '98
*   
* DESCRIPTION
* 
*   This pattern is based on the "Crackle" pattern and creates a faceted
*   look on a curved surface.
*
* CHANGES
*
******************************************************************************/

static void facets (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, TraceThreadData *Thread)
{
	int    i;
	int    thisseed;
	DBL    sum, minsum;
	VECTOR sv, tv, dv, t1, add, newnormal, pert;
	DBL    scale;
	int    UseSquare;
	int    UseUnity;
	DBL    Metric;

	VECTOR *cv = Thread->Facets_Cube;
	Metric = Tnormal->Vals.Facets.Metric;

	UseSquare = (Metric == 2 );
	UseUnity  = (Metric == 1 );

	VNormalize( normal, normal );

	if ( Tnormal->Vals.Facets.UseCoords )
	{
		Assign_Vector(tv,EPoint);
	}
	else
	{
		Assign_Vector(tv,normal);
	}

	if ( Tnormal->Vals.Facets.Size < 1e-6 )
	{
		scale = 1e6;
	}
	else
	{
		scale = 1. / Tnormal->Vals.Facets.Size;
	}

	VScaleEq( tv, scale );

	/*
	 * Check to see if the input point is in the same unit cube as the last
	 * call to this function, to use cache of cubelets for speed.
	 */

	thisseed = PickInCube(tv, t1);

	if (thisseed != Thread->Facets_Last_Seed)
	{
		/*
		 * No, not same unit cube.  Calculate the random points for this new
		 * cube and its 80 neighbours which differ in any axis by 1 or 2.
		 * Why distance of 2?  If there is 1 point in each cube, located
		 * randomly, it is possible for the closest random point to be in the
		 * cube 2 over, or the one two over and one up.  It is NOT possible
		 * for it to be two over and two up.  Picture a 3x3x3 cube with 9 more
		 * cubes glued onto each face.
		 */

		/* Now store a points for this cube and each of the 80 neighbour cubes. */

		int cvc = 0;

		for (add[X] = -2.0; add[X] < 2.5; add[X] +=1.0)
		{
			for (add[Y] = -2.0; add[Y] < 2.5; add[Y] += 1.0)
			{
				for (add[Z] = -2.0; add[Z] < 2.5; add[Z] += 1.0)
				{
					/* For each cubelet in a 5x5 cube. */

					if ((fabs(add[X])>1.5)+(fabs(add[Y])>1.5)+(fabs(add[Z])>1.5) <= 1.0)
					{
						/* Yes, it's within a 3d knight move away. */

						VAdd(sv, tv, add);

						PickInCube(sv, t1);

						cv[cvc][X] = t1[X];
						cv[cvc][Y] = t1[Y];
						cv[cvc][Z] = t1[Z];
						cvc++;
					}
				}
			}
		}

		Thread->Facets_Last_Seed = thisseed;
		Thread->Facets_CVC = cvc;
	}

	/*
	 * Find the point with the shortest distance from the input point.
	 */

	VSub(dv, cv[0], tv);
	if ( UseSquare )
	{
		minsum  = VSumSqr(dv);
	}
	else if ( UseUnity )
	{
		minsum = dv[X]+dv[Y]+dv[Z];
	}
	else
	{
		minsum = pow(fabs(dv[X]), Metric)+
		         pow(fabs(dv[Y]), Metric)+
		         pow(fabs(dv[Z]), Metric);
	}

	Assign_Vector( newnormal, cv[0] );

	/* Loop for the 81 cubelets to find closest. */

	for (i = 1; i < Thread->Facets_CVC; i++)
	{
		VSub(dv, cv[i], tv);

		if ( UseSquare )
		{
			sum  = VSumSqr(dv);
		}
		else
		{
			if ( UseUnity )
			{
				sum = dv[X]+dv[Y]+dv[Z];
			}
			else
			{
				sum = pow(fabs(dv[X]), Metric)+
				      pow(fabs(dv[Y]), Metric)+
				      pow(fabs(dv[Z]), Metric);
			}
		}

		if (sum < minsum)
		{
			minsum = sum;
			Assign_Vector( newnormal, cv[i] );
		}
	}

	if ( Tnormal->Vals.Facets.UseCoords )
	{
		DNoise( pert, newnormal );
		VDot( sum, pert, normal );
		VScale( newnormal, normal, sum );
		VSubEq( pert, newnormal );
		VAddScaledEq( normal, Tnormal->Vals.Facets.UseCoords, pert );
	}
	else
	{
		Assign_Vector( normal, newnormal );
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Create_Tnormal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   pointer to the created Tnormal
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION   : Allocate memory for new Tnormal and initialize it to
*                 system default values.
*
* CHANGES
*
******************************************************************************/


TNORMAL *Create_Tnormal ()
{
	TNORMAL *New;

	New = (TNORMAL *)POV_MALLOC(sizeof(TNORMAL), "normal");

	Init_TPat_Fields((TPATTERN *)New);

	New->Amount = 0.5;

	/* NK delta */
	New->Delta = (float)0.02; /* this is a good starting point for delta */

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Tnormal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

TNORMAL *Copy_Tnormal (const TNORMAL *Old)
{
	TNORMAL *New;

	if (Old != NULL)
	{
		New = Create_Tnormal();

		Copy_TPat_Fields ((TPATTERN *)New, (const TPATTERN *)Old);

		New->Amount = Old->Amount;
		New->Delta = Old->Delta;
	}
	else
	{
		New = NULL;
	}

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Tnormal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Destroy_Tnormal(TNORMAL *Tnormal)
{
	if (Tnormal != NULL)
	{
		Destroy_TPat_Fields ((TPATTERN *)Tnormal);

		POV_FREE(Tnormal);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Post_Tnormal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Post_Tnormal (TNORMAL *Tnormal)
{
	int i;
	BLEND_MAP *Map;

	if (Tnormal != NULL)
	{
		if (Tnormal->Flags & POST_DONE)
		{
			return;
		}

		if (Tnormal->Type == NO_PATTERN)
		{
			throw POV_EXCEPTION_STRING("No normal type given.");
		}

		Tnormal->Flags |= POST_DONE;

		if ((Map = Tnormal->Blend_Map) != NULL)
		{
			for (i = 0; i < Map->Number_Of_Entries; i++)
			{
				switch (Map->Type)
				{
					case PIGMENT_TYPE:

						Post_Pigment(Map->Blend_Map_Entries[i].Vals.Pigment);

						break;

					case NORMAL_TYPE:
						Map->Blend_Map_Entries[i].Vals.Tnormal->Flags |=
							(Tnormal->Flags & DONT_SCALE_BUMPS_FLAG);

						Post_Tnormal(Map->Blend_Map_Entries[i].Vals.Tnormal);

						break;

					case TEXTURE_TYPE:

						Post_Textures(Map->Blend_Map_Entries[i].Vals.Texture);

						break;

					case SLOPE_TYPE:
					case COLOUR_TYPE:
					case PATTERN_TYPE:

						break;

					default:

						throw POV_EXCEPTION_STRING("Unknown pattern type in Post_Tnormal.");
				}
			}
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Perturb_Normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
* CHANGES
*    Added intersectin parameter for UV mapping - NK 1998
*
******************************************************************************/

void Perturb_Normal(VECTOR Layer_Normal, const TNORMAL *Tnormal, const VECTOR EPoint, Intersection *Intersection, const Ray *ray, TraceThreadData *Thread)
{
	VECTOR TPoint,P1;
	DBL value1,value2,Amount;
	int i;
	BLEND_MAP *Blend_Map;
	BLEND_MAP_ENTRY *Prev, *Cur;

	if (Tnormal==NULL)
	{
		return;
	}

	/* If normal_map present, use it and return */

	if ((Blend_Map=Tnormal->Blend_Map) != NULL)
	{
		if ((Blend_Map->Type == NORMAL_TYPE) && (Tnormal->Type == UV_MAP_PATTERN))
		{
			UV_VECT UV_Coords;

			Cur = &(Tnormal->Blend_Map->Blend_Map_Entries[0]);

			/* Don't bother warping, simply get the UV vect of the intersection */
			Intersection->Object->UVCoord(UV_Coords, Intersection, Thread);
			TPoint[X] = UV_Coords[U];
			TPoint[Y] = UV_Coords[V];
			TPoint[Z] = 0;

			Perturb_Normal(Layer_Normal,Cur->Vals.Tnormal,TPoint,Intersection,ray,Thread);
			VNormalizeEq(Layer_Normal);
			Assign_Vector(Intersection->PNormal, Layer_Normal); /* -hdf- June 98 */

			return;
		}
		else if ((Blend_Map->Type == NORMAL_TYPE) && (Tnormal->Type != AVERAGE_PATTERN))
		{
			/* NK 19 Nov 1999 added Warp_EPoint */
			Warp_EPoint (TPoint, EPoint, (TPATTERN *)Tnormal);
			value1 = Evaluate_TPat((TPATTERN *)Tnormal,TPoint,Intersection,ray,Thread);

			Search_Blend_Map (value1,Blend_Map,&Prev,&Cur);

			Warp_Normal(Layer_Normal,Layer_Normal, (TPATTERN *)Tnormal, Test_Flag(Tnormal,DONT_SCALE_BUMPS_FLAG));
			Assign_Vector(P1,Layer_Normal);

			Warp_EPoint (TPoint, EPoint, (TPATTERN *)Tnormal);

			Perturb_Normal(Layer_Normal,Cur->Vals.Tnormal,TPoint,Intersection,ray,Thread);

			if (Prev != Cur)
			{
				Perturb_Normal(P1,Prev->Vals.Tnormal,TPoint,Intersection,ray,Thread);

				value2 = (value1-Prev->value)/(Cur->value-Prev->value);
				value1 = 1.0-value2;

				VLinComb2(Layer_Normal,value1,P1,value2,Layer_Normal);
			}

			UnWarp_Normal(Layer_Normal,Layer_Normal,(TPATTERN *)Tnormal, Test_Flag(Tnormal,DONT_SCALE_BUMPS_FLAG));

			VNormalizeEq(Layer_Normal);

			Assign_Vector(Intersection->PNormal, Layer_Normal); /* -hdf- June 98 */

			return;
		}
	}

	/* No normal_map. */

	if (Tnormal->Type <= LAST_NORM_ONLY_PATTERN)
	{
		Warp_Normal(Layer_Normal,Layer_Normal, (TPATTERN *)Tnormal,
		            Test_Flag(Tnormal,DONT_SCALE_BUMPS_FLAG));

		Warp_EPoint (TPoint, EPoint, (TPATTERN *)Tnormal);

		switch (Tnormal->Type)
		{
			case BITMAP_PATTERN:    bump_map    (TPoint, Tnormal, Layer_Normal);            break;
			case BUMPS_PATTERN:     bumps       (TPoint, Tnormal, Layer_Normal);            break;
			case DENTS_PATTERN:     dents       (TPoint, Tnormal, Layer_Normal, Thread);    break;
			case RIPPLES_PATTERN:   ripples     (TPoint, Tnormal, Layer_Normal, Thread);    break;
			case WAVES_PATTERN:     waves       (TPoint, Tnormal, Layer_Normal, Thread);    break;
			case WRINKLES_PATTERN:  wrinkles    (TPoint, Tnormal, Layer_Normal);            break;
			case QUILTED_PATTERN:   quilted     (TPoint, Tnormal, Layer_Normal);            break;
			case FACETS_PATTERN:    facets      (TPoint, Tnormal, Layer_Normal, Thread);    break;
			case AVERAGE_PATTERN:   Do_Average_Normals (TPoint, Tnormal, Layer_Normal, Intersection, ray, Thread); break;
			default:
				throw POV_EXCEPTION_STRING("Normal pattern not yet implemented.");
		}

		UnWarp_Normal(Layer_Normal,Layer_Normal, (TPATTERN *)Tnormal,
		              Test_Flag(Tnormal,DONT_SCALE_BUMPS_FLAG));
	}
	else
	{
		Warp_Normal(Layer_Normal,Layer_Normal, (TPATTERN *)Tnormal,
		            Test_Flag(Tnormal,DONT_SCALE_BUMPS_FLAG));

		Amount=Tnormal->Amount * -5.0; /*fudge factor*/
		Amount*=0.02/Tnormal->Delta; /* NK delta */

		/* warp the center point first - this is the last warp */
		Warp_EPoint(TPoint,EPoint,(TPATTERN *)Tnormal);

		for(i=0; i<=3; i++)
		{
			VAddScaled(P1,TPoint,Tnormal->Delta,Pyramid_Vect[i]); /* NK delta */
			value1 = Do_Slope_Map(Evaluate_TPat((TPATTERN *)Tnormal,P1,Intersection,ray,Thread),Blend_Map);
			VAddScaledEq(Layer_Normal,value1*Amount,Pyramid_Vect[i]);
		}

		UnWarp_Normal(Layer_Normal,Layer_Normal,(TPATTERN *)Tnormal,
		              Test_Flag(Tnormal,DONT_SCALE_BUMPS_FLAG));

	}

	if ( Intersection )
		Assign_Vector(Intersection->PNormal, Layer_Normal); /* -hdf- June 98 */
}



/*****************************************************************************
*
* FUNCTION
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

static DBL Do_Slope_Map (DBL value, const BLEND_MAP *Blend_Map)
{
	DBL Result;
	BLEND_MAP_ENTRY *Prev, *Cur;

	if (Blend_Map == NULL)
	{
		return(value);
	}

	Search_Blend_Map (value,Blend_Map,&Prev,&Cur);

	if (Prev == Cur)
	{
		return(Cur->Vals.Point_Slope[0]);
	}

	Result = (value-Prev->value)/(Cur->value-Prev->value);

	return(Hermite_Cubic(Result, Prev->Vals.Point_Slope, Cur->Vals.Point_Slope));
}



/*****************************************************************************
*
* FUNCTION
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

static DBL Hermite_Cubic(DBL T1, const UV_VECT UV1, const UV_VECT UV2)
{
	DBL TT=T1*T1;
	DBL TTT=TT*T1;
	DBL rv;        /* simplified equation for poor Symantec */

	rv  = TTT*(UV1[1]+UV2[1]+2.0*(UV1[0]-UV2[0]));
	rv += -TT*(2.0*UV1[1]+UV2[1]+3.0*(UV1[0]-UV2[0]));
	rv += T1*UV1[1] +UV1[0];

	return (rv);
}



/*****************************************************************************
*
* FUNCTION
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
*    Added intersectin parameter for UV mapping - NK 1998
*
******************************************************************************/

static void Do_Average_Normals (const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal, Intersection *Inter, const Ray *ray, TraceThreadData *Thread)
{
	int i;
	BLEND_MAP *Map = Tnormal->Blend_Map;
	SNGL Value;
	SNGL Total = 0.0;
	VECTOR V1,V2;

	Make_Vector (V1, 0.0, 0.0, 0.0);

	for (i = 0; i < Map->Number_Of_Entries; i++)
	{
		Value = Map->Blend_Map_Entries[i].value;

		Assign_Vector(V2,normal);

		Perturb_Normal(V2,Map->Blend_Map_Entries[i].Vals.Tnormal,EPoint,Inter,ray,Thread);

		VAddScaledEq(V1,Value,V2);

		Total += Value;
	}

	VInverseScale(normal,V1,Total);
}

}
