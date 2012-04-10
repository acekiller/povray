/*******************************************************************************
 * point.cpp
 *
 * This module implements the point & spot light source primitive.
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
 * $File: //depot/povray/smp/source/backend/lighting/point.cpp $
 * $Revision: #25 $
 * $Change: 5512 $
 * $DateTime: 2011/11/12 14:22:14 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/colour/colour.h"
#include "point.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
*
* FUNCTION
*
*   All_Light_Source_Intersections
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

bool LightSource::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	if(!children.empty())
	{
		if(children[0]->Bound.empty() || Ray_In_Bound(ray, children[0]->Bound, Thread))
		{
			if(children[0]->All_Intersections(ray, Depth_Stack, Thread))
				return true;
		}
	}

	return false;
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

bool LightSource::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	if(!children.empty())
	{
		if(children[0]->Inside(IPoint, Thread))
			return true;
	}

	return false;
}



/*****************************************************************************
*
* FUNCTION
*
*   Light_Source_Normal
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	if(!children.empty())
		children[0]->Normal(Result, Inter, Thread);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::Translate(const VECTOR Vector, const TRANSFORM *tr)
{
	VAddEq(Center, Vector);
	VAddEq(Points_At, Vector);

	if(!children.empty())
		Translate_Object(children[0], Vector, tr);

	if(Projected_Through_Object != NULL )
		Translate_Object(Projected_Through_Object, Vector, tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::Transform(const TRANSFORM *tr)
{
	DBL len;

	MTransPoint(Center, Center,    tr);
	MTransPoint(Points_At, Points_At, tr);
	MTransDirection(Axis1, Axis1,     tr);
	MTransDirection(Axis2, Axis2,     tr);

	MTransDirection(Direction, Direction, tr);

	/* Make sure direction has unit length. */

	VLength(len, Direction);

	if(len > EPSILON)
		VInverseScaleEq(Direction, len);

	if(!children.empty())
		Transform_Object(children[0], tr);

	if(Projected_Through_Object != NULL)
		Transform_Object(Projected_Through_Object, tr);
}
	
	


/*****************************************************************************
*
* FUNCTION
*
*   Invert_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::Invert()
{
	if(!children.empty())
		Invert_Object(children[0]);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

LightSource::LightSource() : CompoundObject(LIGHT_OBJECT)
{
	Set_Flag(this, NO_SHADOW_FLAG);

	colour = RGBColour(1.0);
	Make_Vector(Direction, 0.0, 0.0, 0.0);
	Make_Vector(Center,    0.0, 0.0, 0.0);
	Make_Vector(Points_At, 0.0, 0.0, 1.0);
	Make_Vector(Axis1,     0.0, 0.0, 1.0);
	Make_Vector(Axis2,     0.0, 1.0, 0.0);

	Coeff   = 0.0;
	Radius  = 0.0;
	Falloff = 0.0;

	Fade_Distance = 0.0;
	Fade_Power    = 0.0;

// TODO  Shadow_Cached_Object = NULL;
	Projected_Through_Object= NULL;
	blend_map            = NULL;

	Light_Type = POINT_SOURCE;

	Area_Light = false;
	Use_Full_Area_Lighting = false; // JN2007: Full area lighting
	Jitter     = false;
	Orient     = false;
	Circular   = false;
	Parallel   = false;
	Photon_Area_Light = false;

	Area_Size1 = 0;
	Area_Size2 = 0;

	Adaptive_Level = 100;

	Media_Attenuation = false;
	Media_Interaction = true;

// TODO  for(i = 0; i < 6; i++)
// TODO    Light_Buffer[i] = NULL;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

ObjectPtr LightSource::Copy()
{
	LightSource *New = new LightSource();

	/* Copy light source. */

	*New = *this;

	if(!children.empty())
		New->children[0] = Copy_Object(children[0]);
	New->Projected_Through_Object = Copy_Object(Projected_Through_Object);

	/* NK phmap */
	New->blend_map = Copy_Blend_Map(blend_map);

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Light_Source
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

LightSource::~LightSource()
{
	if(blend_map)
	{
		Destroy_Blend_Map(blend_map);
		blend_map=NULL;
	}

	Destroy_Object(Projected_Through_Object);
}

/*****************************************************************************
*
* FUNCTION
*
*   cubic_spline
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
*   Cubic spline that has tangents of slope 0 at x == low and at x == high.
*   For a given value "pos" between low and high the spline value is returned.
*
* CHANGES
*
*   -
*
******************************************************************************/

DBL cubic_spline(DBL low, DBL  high, DBL  pos)
{
	/* Check to see if the position is within the proper boundaries. */
	if(pos < low)
		return 0.0;
	else
	{
		if(pos >= high)
			return 1.0;
	}

	/* Normalize to the interval [0...1]. */
	pos = (pos - low) / (high - low);

	/* See where it is on the cubic curve. */
	return(3 - 2 * pos) * pos * pos;
}



/*****************************************************************************
*
* FUNCTION
*
*   Attenuate_Light
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
*   -
*
* CHANGES
*
*   Jan 1995 : Added attenuation due to atmospheric scattering and light
*              source distance. Added cylindrical light source. [DB]
*
******************************************************************************/

DBL Attenuate_Light (const LightSource *Light, Ray &ray, DBL Distance)
{
	DBL len, k, costheta;
	DBL Attenuation = 1.0;
	VECTOR P, V1;

	/* If this is a spotlight then attenuate based on the incidence angle. */

	switch (Light->Light_Type)
	{
		case SPOT_SOURCE:

			VDot(costheta, ray.Direction, Light->Direction);

			if(Distance>0.0) costheta = -costheta;

			if (costheta > 0.0)
			{
				Attenuation = pow(costheta, Light->Coeff);

				if (Light->Radius > 0.0 && costheta < Light->Radius)
				{
					Attenuation *= cubic_spline(Light->Falloff, Light->Radius, costheta);
				}
			}
			else
			{
				return 0.0; //Attenuation = 0.0;
			}

			break;

		case CYLINDER_SOURCE:

			// Project light->point onto light direction
			// to make sure that we're on the correct side of the light
			VSub(V1, ray.Origin, Light->Center);
			VDot(k, V1, Light->Direction);

			if (k > 0.0)
			{
				// Now subtract that from the light-direction.  This will
				// give us a vector showing us the distance from the
				// point to the center of the cylinder.
				VLinComb2(P, 1.0, V1, -k, Light->Direction);
				VLength(len, P);

				if (len < Light->Falloff)
				{
					DBL dist = 1.0 - len / Light->Falloff;

					Attenuation = pow(dist, Light->Coeff);

					if (Light->Radius > 0.0 && len > Light->Radius)
					{
						Attenuation *= cubic_spline(0.0, 1.0 - Light->Radius / Light->Falloff, dist);
					}
				}
				else
				{
					return 0.0; //Attenuation = 0.0;
				}
			}
			else
			{
				return 0.0; //Attenuation = 0.0;
			}

			break;
	}

	if (Attenuation > 0.0)
	{
		/* Attenuate light due to light source distance. */

		if ((Light->Fade_Power > 0.0) && (fabs(Light->Fade_Distance) > EPSILON))
		{
			Attenuation *= 2.0 / (1.0 + pow(Distance / Light->Fade_Distance, Light->Fade_Power));
		}
	}

	return(Attenuation);
}

/*****************************************************************************
*
* FUNCTION
*
*   Light_Source_UVCoord
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Nathan Kopp -- adapted from Light_Source_Normal by the POV-Ray Team
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void LightSource::UVCoord(UV_VECT Result, const Intersection *Inter, TraceThreadData *Thread) const
{
	if(!children.empty())
		children[0]->UVCoord(Result, Inter, Thread);
}

}
