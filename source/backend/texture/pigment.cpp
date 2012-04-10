/*******************************************************************************
 * pigment.cpp
 *
 * This module implements solid texturing functions that modify the color
 * transparency of an object's surface.
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
 * $File: //depot/povray/smp/source/backend/texture/pigment.cpp $
 * $Revision: #31 $
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

/*
   Some texture ideas garnered from SIGGRAPH '85 Volume 19 Number 3, 
   "An Image Synthesizer" By Ken Perlin.
   Further Ideas Garnered from "The RenderMan Companion" (Addison Wesley).
*/

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/texture/pigment.h"
#include "backend/pattern/pattern.h"
#include "backend/pattern/warps.h"
#include "backend/support/imageutil.h"
#include "backend/scene/threaddata.h"
#include "base/pov_err.h"

#include "povrayold.h" // TODO FIXME

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
* Local variables
******************************************************************************/

static BLEND_MAP_ENTRY Black_White_Entries[2] =
	{{0.0, false, {{0.0, 0.0, 0.0, 0.0, 0.0}}},
	{1.0, false, {{1.0, 1.0, 1.0, 0.0, 0.0}}}};

const BLEND_MAP Gray_Default_Map =
	{ -1,  2,  false, COLOUR_TYPE,  Black_White_Entries};

static BLEND_MAP_ENTRY Bozo_Entries[6] =
	{{0.4, false, {{1.0, 1.0, 1.0, 0.0, 0.0}}},
	 {0.4, false, {{0.0, 1.0, 0.0, 0.0, 0.0}}},
	 {0.6, false, {{0.0, 1.0, 0.0, 0.0, 0.0}}},
	 {0.6, false, {{0.0, 0.0, 1.0, 0.0, 0.0}}},
	 {0.8, false, {{0.0, 0.0, 1.0, 0.0, 0.0}}},
	 {0.8, false, {{1.0, 0.0, 0.0, 0.0, 0.0}}}};

const BLEND_MAP Bozo_Default_Map =
	{ -1,  6,  false, COLOUR_TYPE,  Bozo_Entries};

static BLEND_MAP_ENTRY Wood_Entries[2] =
	{{0.6, false, {{0.666, 0.312,  0.2,   0.0, 0.0}}},
	 {0.6, false, {{0.4,   0.1333, 0.066, 0.0, 0.0}}}};

const BLEND_MAP Wood_Default_Map =
	{ -1,  2,  false, COLOUR_TYPE,  Wood_Entries};

static BLEND_MAP_ENTRY Mandel_Entries[5] =
	{{0.001, false, {{0.0, 0.0, 0.0, 0.0, 0.0}}},
	 {0.001, false, {{0.0, 1.0, 1.0, 0.0, 0.0}}},
	 {0.012, false, {{1.0, 1.0, 0.0, 0.0, 0.0}}},
	 {0.015, false, {{1.0, 0.0, 1.0, 0.0, 0.0}}},
	 {0.1,   false, {{0.0, 1.0, 1.0, 0.0, 0.0}}}};

const BLEND_MAP Mandel_Default_Map =
	{ -1,  5,  false, COLOUR_TYPE,  Mandel_Entries};

static BLEND_MAP_ENTRY Agate_Entries[6] =
	{{0.0, false, {{1.0,  1.0,  1.0,  0.0, 0.0}}},
	 {0.5, false, {{0.95, 0.75, 0.5,  0.0, 0.0}}},
	 {0.5, false, {{0.9,  0.7,  0.5,  0.0, 0.0}}},
	 {0.6, false, {{0.9,  0.7,  0.4,  0.0, 0.0}}},
	 {0.6, false, {{1.0,  0.7,  0.4,  0.0, 0.0}}},
	 {1.0, false, {{0.6,  0.3,  0.0,  0.0, 0.0}}}};

const BLEND_MAP Agate_Default_Map =
	{ -1,  6,  false, COLOUR_TYPE,  Agate_Entries};

static BLEND_MAP_ENTRY Radial_Entries[4] =
	{{0.0,   false, {{0.0, 1.0, 1.0, 0.0, 0.0}}},
	 {0.333, false, {{1.0, 1.0, 0.0, 0.0, 0.0}}},
	 {0.666, false, {{1.0, 0.0, 1.0, 0.0, 0.0}}},
	 {1.0,   false, {{0.0, 1.0, 1.0, 0.0, 0.0}}}};

const BLEND_MAP Radial_Default_Map =
	{ -1,  4,  false, COLOUR_TYPE,  Radial_Entries};

static BLEND_MAP_ENTRY Marble_Entries[3] =
	{{0.0, false, {{0.9, 0.8,  0.8,  0.0, 0.0}}},
	 {0.9, false, {{0.9, 0.08, 0.08, 0.0, 0.0}}},
	 {0.9, false, {{0.0, 0.0, 0.0, 0.0, 0.0}}}};

const BLEND_MAP Marble_Default_Map =
	{ -1,  3,  false, COLOUR_TYPE,  Marble_Entries};

static BLEND_MAP_ENTRY Brick_Entries[2] =
	{{0.0, false, {{0.5, 0.5,  0.5,  0.0, 0.0}}},
	 {1.0, false, {{0.6, 0.15, 0.15, 0.0, 0.0}}}};

const BLEND_MAP Brick_Default_Map =
	{ -1,  2,  false, COLOUR_TYPE,  Brick_Entries};

static BLEND_MAP_ENTRY Hex_Entries[3] =
	{{0.0, false, {{0.0, 0.0, 1.0, 0.0, 0.0}}},
	 {1.0, false, {{0.0, 1.0, 0.0, 0.0, 0.0}}},
	 {2.0, false, {{1.0, 0.0, 0.0, 0.0, 0.0}}}};

const BLEND_MAP Hex_Default_Map =
	{ -1, 3, false,COLOUR_TYPE, Hex_Entries};

// JN2007: Cubic pattern
static BLEND_MAP_ENTRY Cubic_Entries[6] =
	{{0.0, false, {{1.0, 0.0, 0.0, 0.0, 0.0}}},
	 {1.0, false, {{0.0, 1.0, 0.0, 0.0, 0.0}}},
	 {1.0, false, {{0.0, 0.0, 1.0, 0.0, 0.0}}},
	 {1.0, false, {{1.0, 1.0, 0.0, 0.0, 0.0}}},
	 {1.0, false, {{0.0, 1.0, 1.0, 0.0, 0.0}}},
	 {2.0, false, {{1.0, 0.0, 1.0, 0.0, 0.0}}}};
const BLEND_MAP Cubic_Default_Map =
	{ -1, 6, false,COLOUR_TYPE, Cubic_Entries};

const BLEND_MAP Check_Default_Map =
	{ -1, 2, false,COLOUR_TYPE, Hex_Entries}; /* Yes... Hex_Entries, not Check [CY] */

const BLEND_MAP Triangular_Default_Map =
	{ -1, 6, false,COLOUR_TYPE, Cubic_Entries}; /* Yes... Cubic_Entries, not Triangular [JG] */

const BLEND_MAP Square_Default_Map =
	{ -1, 4, false,COLOUR_TYPE, Cubic_Entries}; /* Yes... Cubic_Entries, not Square [JG] */



/*****************************************************************************
* Static functions
******************************************************************************/
static void Do_Average_Pigments (Colour& colour, const PIGMENT *Pigment, const VECTOR EPoint, const Intersection *Intersect, const Ray *ray, TraceThreadData *Thread);



/*****************************************************************************
*
* FUNCTION
*
*   Create_Pigment
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   pointer to the created pigment
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION   : Allocate memory for new pigment and initialize it to
*                 system default values.
*
* CHANGES
*
******************************************************************************/

PIGMENT *Create_Pigment ()
{
	PIGMENT *New;

	New = (PIGMENT *)POV_MALLOC(sizeof (PIGMENT), "pigment");

	Init_TPat_Fields((TPATTERN *)New);

	New->colour.clear();
	New->Quick_Colour = Colour(-1.0,-1.0,-1.0);
	New->Blend_Map = NULL;

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Pigment
*
* INPUT
*
*   Old -- point to pigment to be copied
*   
* RETURNS
*
*   pointer to the created pigment
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION   : Allocate memory for new pigment and initialize it to
*                 values in existing pigment Old.
*
* CHANGES
*
******************************************************************************/

PIGMENT *Copy_Pigment (const PIGMENT *Old)
{
	PIGMENT *New;

	if (Old != NULL)
	{
		New = Create_Pigment ();

		Copy_TPat_Fields ((TPATTERN *)New, (const TPATTERN *)Old);

		if (Old->Type == PLAIN_PATTERN)
			New->colour = Old->colour;
		New->Quick_Colour = Old->Quick_Colour;
		New->Next = (TPATTERN *)Copy_Pigment((const PIGMENT *)Old->Next);
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
*   Destroy_Pigment
*
* INPUT
*
*   pointer to pigment to destroied
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION   : free all memory associated with given pigment
*
* CHANGES
*
******************************************************************************/

void Destroy_Pigment (PIGMENT *Pigment)
{
	if (Pigment != NULL)
	{
		Destroy_Pigment((PIGMENT *)Pigment->Next);

		Destroy_TPat_Fields ((TPATTERN *)Pigment);

		POV_FREE(Pigment);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Post_Pigment
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Chris Young
*   
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

int Post_Pigment(PIGMENT *Pigment)
{
	int i, Has_Filter;
	BLEND_MAP *Map;

	if (Pigment == NULL)
	{
		throw POV_EXCEPTION_STRING("Missing pigment");
	}

	if (Pigment->Flags & POST_DONE)
	{
		return(Pigment->Flags & HAS_FILTER);
	}

	if (Pigment->Type == NO_PATTERN)
	{
		Pigment->Type = PLAIN_PATTERN;

		Pigment->colour.clear() ;

;// TODO MESSAGE    Warning(150, "No pigment type given.");
	}

	Pigment->Flags |= POST_DONE;

	switch (Pigment->Type)
	{
		case PLAIN_PATTERN:

			Destroy_Warps (Pigment->Warps);

			Pigment->Warps = NULL;

			break;

		case NO_PATTERN:
		case BITMAP_PATTERN:

			break;

		default:

			if (Pigment->Blend_Map == NULL)
			{
				switch (Pigment->Type)
				{
					case BOZO_PATTERN:    Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Bozo_Default_Map);  break;
					case BRICK_PATTERN:   Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Brick_Default_Map); break;
					case WOOD_PATTERN:    Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Wood_Default_Map);  break;
					case MANDEL_PATTERN:  Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Mandel_Default_Map);break;
					case RADIAL_PATTERN:  Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Radial_Default_Map);break;
					case AGATE_PATTERN:   Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Agate_Default_Map); break;
					case MARBLE_PATTERN:  Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Marble_Default_Map);break;
					case HEXAGON_PATTERN: Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Hex_Default_Map);   break;
					case SQUARE_PATTERN:  Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Square_Default_Map);break;
					case TRIANGULAR_PATTERN: Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Triangular_Default_Map);break;
					case CUBIC_PATTERN:   Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Cubic_Default_Map); break; // JN2007: Cubic pattern
					case CHECKER_PATTERN: Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Check_Default_Map); break;
					case AVERAGE_PATTERN: break;// TODO MESSAGE Error("Missing pigment_map in average pigment"); break;
					case OBJECT_PATTERN:  Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Check_Default_Map); break;
					default:              Pigment->Blend_Map = const_cast<BLEND_MAP *>(&Gray_Default_Map);  break;
				}
			}

			break;
	}

	/* Now we test wether this pigment is opaque or not. [DB 8/94] */

	Has_Filter = false;

	if ((fabs(Pigment->colour[pFILTER]) > EPSILON) ||
	    (fabs(Pigment->colour[pTRANSM]) > EPSILON))
	{
		Has_Filter = true;
	}

	if ((Pigment->Type == BITMAP_PATTERN) &&
	    (Pigment->Vals.image != NULL))
	{
		// bitmaps are transparent if they are used only once, or the image is not opaque
		Has_Filter |= (Pigment->Vals.image->Once_Flag) || !is_image_opaque(Pigment->Vals.image);
	}

	if ((Map = Pigment->Blend_Map) != NULL)
	{
		if ((Map->Type == PIGMENT_TYPE) || (Map->Type == DENSITY_TYPE))
		{
			for (i = 0; i < Map->Number_Of_Entries; i++)
			{
				Has_Filter |= Post_Pigment(Map->Blend_Map_Entries[i].Vals.Pigment);
			}
		}
		else
		{
			for (i = 0; i < Map->Number_Of_Entries; i++)
			{
				Has_Filter |= fabs(Map->Blend_Map_Entries[i].Vals.colour[pFILTER])>EPSILON;
				Has_Filter |= fabs(Map->Blend_Map_Entries[i].Vals.colour[pTRANSM])>EPSILON;
			}
		}
	}

	if (Has_Filter)
	{
		Pigment->Flags |= HAS_FILTER;
	}

	if (Pigment->Next != NULL)
	{
		Post_Pigment((PIGMENT *)Pigment->Next);
	}

	return(Has_Filter);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Pigment
*
* INPUT
*
*   Pigment - Info about this pigment
*   EPoint  - 3-D point at which pattern is evaluated
*   Intersection - structure holding info about object at intersection point
*
* OUTPUT
*
*   colour  - Resulting color is returned here.
*
* RETURNS
*
*   int - true,  if a color was found for the given point
*         false, if no color was found (e.g. areas outside an image map
*                that has the once option)
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*   Given a 3d point and a pigment, compute colour from that layer.
*   (Formerly called "Colour_At", or "Add_Pigment")
*
* CHANGES
*   Added pigment map support [CY 11/94]
*   Added Intersection parameter for UV support NK 1998
*
******************************************************************************/

bool Compute_Pigment (Colour& colour, const PIGMENT *Pigment, const VECTOR EPoint, const Intersection *Intersect, const Ray *ray, TraceThreadData *Thread)
{
	int Colour_Found;
	VECTOR TPoint;
	DBL value;
	register DBL fraction;
	BLEND_MAP_ENTRY *Cur, *Prev;
	Colour Temp_Colour;
	BLEND_MAP *Blend_Map = Pigment->Blend_Map;
	UV_VECT UV_Coords;

	if ((Thread->qualityFlags & Q_QUICKC) != 0 && Pigment->Quick_Colour[pRED] != -1.0 && Pigment->Quick_Colour[pGREEN] != -1.0 && Pigment->Quick_Colour[pBLUE] != -1.0)
	{
		colour = Pigment->Quick_Colour;
		return (true);
	}

	if (Pigment->Type <= LAST_SPECIAL_PATTERN)
	{
		Colour_Found = true;

		switch (Pigment->Type)
		{
			case NO_PATTERN:

				colour.clear();

				break;

			case PLAIN_PATTERN:

				colour = Pigment->colour;

				break;

			case AVERAGE_PATTERN:

				Warp_EPoint (TPoint, EPoint, (TPATTERN *)Pigment);

				Do_Average_Pigments(colour, Pigment, TPoint, Intersect, ray, Thread);

				break;

			case UV_MAP_PATTERN:
				if(Intersect == NULL)
					throw POV_EXCEPTION_STRING("The 'uv_mapping' pattern cannot be used as part of a pigment function!");

				Cur = &(Pigment->Blend_Map->Blend_Map_Entries[0]);

				if (Blend_Map->Type == COLOUR_TYPE)
				{
					Colour_Found = true;

					Assign_Colour(*colour, Cur->Vals.colour);
				}
				else
				{
					/* Don't bother warping, simply get the UV vect of the intersection */
					Intersect->Object->UVCoord(UV_Coords, Intersect, Thread);
					TPoint[X] = UV_Coords[U];
					TPoint[Y] = UV_Coords[V];
					TPoint[Z] = 0;

					if (Compute_Pigment(colour, Cur->Vals.Pigment,TPoint,Intersect, ray, Thread))
						Colour_Found = true;
				}

				break;

			case BITMAP_PATTERN:

				Warp_EPoint (TPoint, EPoint, (TPATTERN *)Pigment);

				colour.clear();

				Colour_Found = image_map (TPoint, Pigment, colour);

				break;

			default:

				throw POV_EXCEPTION_STRING("Pigment type not yet implemented.");
		}

		return(Colour_Found);
	}

	Colour_Found = false;

	/* NK 19 Nov 1999 added Warp_EPoint */
	Warp_EPoint (TPoint, EPoint, (TPATTERN *)Pigment);
	value = Evaluate_TPat ((TPATTERN *)Pigment,TPoint,Intersect, ray, Thread);

	Search_Blend_Map (value, Blend_Map, &Prev, &Cur);

	if (Blend_Map->Type == COLOUR_TYPE)
	{
		Colour_Found = true;

		Assign_Colour(*colour, Cur->Vals.colour);
	}
	else
	{
		Warp_EPoint (TPoint, EPoint, (TPATTERN *)Pigment);

		if (Compute_Pigment(colour, Cur->Vals.Pigment,TPoint,Intersect, ray, Thread))
			Colour_Found = true;
	}

	if (Prev != Cur)
	{
		if (Blend_Map->Type == COLOUR_TYPE)
		{
			Colour_Found = true;

			Assign_Colour(*Temp_Colour, Prev->Vals.colour);
		}
		else
		{
			if (Compute_Pigment(Temp_Colour, Prev->Vals.Pigment, TPoint, Intersect, ray, Thread))
				Colour_Found = true;
		}

		fraction = (value - Prev->value) / (Cur->value - Prev->value);

		colour = Temp_Colour + fraction * (colour - Temp_Colour);
	}

	return(Colour_Found);
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
*   Added Intersection parameter for UV support NK 1998
*
******************************************************************************/

static void Do_Average_Pigments (Colour& colour, const PIGMENT *Pigment, const VECTOR EPoint, const Intersection *Intersect, const Ray *ray, TraceThreadData *Thread)
{
	int i;
	Colour LC;
	BLEND_MAP *Map = Pigment->Blend_Map;
	SNGL Value;
	SNGL Total = 0.0;

	colour.clear();

	for (i = 0; i < Map->Number_Of_Entries; i++)
	{
		Value = Map->Blend_Map_Entries[i].value;

		Compute_Pigment (LC, Map->Blend_Map_Entries[i].Vals.Pigment, EPoint, Intersect, ray, Thread);

		colour += LC * Value;
		Total  += Value;
	}
	colour /= Total;
}

void Evaluate_Density_Pigment(PIGMENT *pigm, const Vector3d& p, RGBColour& c, TraceThreadData *ttd)
{
	Colour lc;

	c.set(1.0);

	while(pigm != NULL)
	{
		lc.clear();

		Compute_Pigment(lc, pigm, *p, NULL, NULL, ttd);

		c.red()   *= lc.red();
		c.green() *= lc.green();
		c.blue()  *= lc.blue();

		pigm = (PIGMENT *)pigm->Next;
	}
}

}

