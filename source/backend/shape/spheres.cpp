/*******************************************************************************
 * spheres.cpp
 *
 * This module implements the sphere primitive.
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
 * $File: //depot/povray/smp/source/backend/shape/spheres.cpp $
 * $Revision: #31 $
 * $Change: 5103 $
 * $DateTime: 2010/08/22 06:58:49 $
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
#include "backend/bounding/bbox.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/shape/spheres.h"
#include "backend/scene/threaddata.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const DBL DEPTH_TOLERANCE = 1.0e-6;


/*****************************************************************************
*
* FUNCTION
*
*   All_Sphere_Intersection
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

bool Sphere::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	Thread->Stats()[Ray_Sphere_Tests]++;

	if(Do_Ellipsoid)
	{
		register int Intersection_Found;
		DBL Depth1, Depth2, len;
		VECTOR IPoint;
		Ray New_Ray;

		// Transform the ray into the ellipsoid's space

		MInvTransPoint(New_Ray.Origin, ray.Origin, Trans);
		MInvTransDirection(New_Ray.Direction, ray.Direction, Trans);

		VLength(len, New_Ray.Direction);
		VInverseScaleEq(New_Ray.Direction, len);

		Intersection_Found = false;

		if(Intersect(New_Ray, Center, Sqr(Radius), &Depth1, &Depth2))
		{
			Thread->Stats()[Ray_Sphere_Tests_Succeeded]++;
			if((Depth1 > DEPTH_TOLERANCE) && (Depth1 < MAX_DISTANCE))
			{
				VEvaluateRay(IPoint, New_Ray.Origin, Depth1, New_Ray.Direction);
				MTransPoint(IPoint, IPoint, Trans);

				if(Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
				{
					Depth_Stack->push(Intersection(Depth1 / len, IPoint, this));
					Intersection_Found = true;
				}
			}

			if((Depth2 > DEPTH_TOLERANCE) && (Depth2 < MAX_DISTANCE))
			{
				VEvaluateRay(IPoint, New_Ray.Origin, Depth2, New_Ray.Direction);
				MTransPoint(IPoint, IPoint, Trans);

				if(Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
				{
					Depth_Stack->push(Intersection(Depth2 / len, IPoint, this));
					Intersection_Found = true;
				}
			}
		}

		return(Intersection_Found);
	}
	else
	{
		register int Intersection_Found;
		DBL Depth1, Depth2;
		VECTOR IPoint;

		Intersection_Found = false;

		if(Intersect(ray, Center, Sqr(Radius), &Depth1, &Depth2))
		{
			Thread->Stats()[Ray_Sphere_Tests_Succeeded]++;
			if((Depth1 > DEPTH_TOLERANCE) && (Depth1 < MAX_DISTANCE))
			{
				VEvaluateRay(IPoint, ray.Origin, Depth1, ray.Direction);

				if(Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
				{
					Depth_Stack->push(Intersection(Depth1, IPoint, this));
					Intersection_Found = true;
				}
			}

			if((Depth2 > DEPTH_TOLERANCE) && (Depth2 < MAX_DISTANCE))
			{
				VEvaluateRay(IPoint, ray.Origin, Depth2, ray.Direction);

				if(Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
				{
					Depth_Stack->push(Intersection(Depth2, IPoint, this));
					Intersection_Found = true;
				}
			}
		}

		return(Intersection_Found);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_Sphere
*
* INPUT
*
*   Ray     - Ray to test intersection with
*   Center  - Center of the sphere
*   Radius2 - Squared radius of the sphere
*   Depth1  - Lower intersection distance
*   Depth2  - Upper intersection distance
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

bool Sphere::Intersect(const Ray& ray, const VECTOR Center, DBL Radius2, DBL *Depth1, DBL *Depth2)
{
	DBL OCSquared, t_Closest_Approach, Half_Chord, t_Half_Chord_Squared;
	VECTOR Origin_To_Center;

	VSub(Origin_To_Center, Center, ray.Origin);

	VDot(OCSquared, Origin_To_Center, Origin_To_Center);

	VDot(t_Closest_Approach, Origin_To_Center, ray.Direction);

	if ((OCSquared >= Radius2) && (t_Closest_Approach < EPSILON))
		return(false);

	t_Half_Chord_Squared = Radius2 - OCSquared + Sqr(t_Closest_Approach);

	if (t_Half_Chord_Squared > EPSILON)
	{
		Half_Chord = sqrt(t_Half_Chord_Squared);

		*Depth1 = t_Closest_Approach - Half_Chord;
		*Depth2 = t_Closest_Approach + Half_Chord;

		return(true);
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Sphere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

bool Sphere::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	DBL OCSquared;
	VECTOR Origin_To_Center;

	if(Do_Ellipsoid)
	{
		DBL OCSquared;
		VECTOR Origin_To_Center, New_Point;

		/* Transform the point into the sphere's space */

		MInvTransPoint(New_Point, IPoint, Trans);

		VSub(Origin_To_Center, Center, New_Point);

		VDot(OCSquared, Origin_To_Center, Origin_To_Center);

		if (Test_Flag(this, INVERTED_FLAG))
			return(OCSquared > Sqr(Radius));
		else
			return(OCSquared < Sqr(Radius));
	}
	else
	{
		VSub(Origin_To_Center, Center, IPoint);

		VDot(OCSquared, Origin_To_Center, Origin_To_Center);

		if(Test_Flag(this, INVERTED_FLAG))
			return(OCSquared > Sqr(Radius));
		else
			return(OCSquared < Sqr(Radius));
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Sphere_Normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

void Sphere::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	if(Do_Ellipsoid)
	{
		VECTOR New_Point;
		// Transform the point into the sphere's space
		MInvTransPoint(New_Point, Inter->IPoint, Trans);
		VSub(Result, New_Point, Center);
		MTransNormal(Result, Result, Trans);
		VNormalize(Result, Result);
	}
	else
	{
		VSub(Result, Inter->IPoint, Center);
		VInverseScaleEq(Result, Radius);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Shere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

ObjectPtr Sphere::Copy()
{
	Sphere *New = new Sphere();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	return(New);
}


/*****************************************************************************
*
* FUNCTION
*
*   Translate_Sphere
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   ?
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

void Sphere::Translate(const VECTOR Vector, const TRANSFORM *tr)
{
	if(Trans == NULL)
	{
		VAddEq(Center, Vector);

		Compute_BBox();
	}
	else
	{
		Transform(tr);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Sphere
*
* INPUT
*
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

void Sphere::Rotate(const VECTOR, const TRANSFORM *tr)
{
	if(Trans == NULL)
	{
		MTransPoint(Center, Center, tr);

		Compute_BBox();
	}
	else
	{
		Transform(tr);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Sphere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

void Sphere::Scale(const VECTOR Vector, const TRANSFORM *tr)
{
	if ((Vector[X] != Vector[Y]) || (Vector[X] != Vector[Z]))
	{
		if (Trans == NULL)
		{
			// treat sphere as ellipsoid as it's unevenly scaled
			Do_Ellipsoid = true; // FIXME - parser needs to select sphere or ellipsoid
			Trans = Create_Transform();
		}
	}

	if (Trans == NULL)
	{
		VScaleEq(Center, Vector[X]);

		Radius *= fabs(Vector[X]);

		Compute_BBox();
	}
	else
	{
		Transform(tr);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Sphere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

void Sphere::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Sphere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

Sphere::Sphere() : ObjectBase(SPHERE_OBJECT)
{
	Make_Vector(Center, 0.0, 0.0, 0.0);

	Radius = 1.0;

	Trans = NULL;
	Do_Ellipsoid = false; // FIXME
}

/*****************************************************************************
*
* FUNCTION
*
*   Transform_Sphere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

void Sphere::Transform(const TRANSFORM *tr)
{
	if(Trans == NULL)
	{
		Do_Ellipsoid = true;
		Trans = Create_Transform();
	}

	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Sphere
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   ?
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

Sphere::~Sphere()
{
#if(DUMP_OBJECT_DATA == 1)
	Debug_Info("{ // SPHERE \n");
	DUMP_OBJECT_FIELDS(this);
	Debug_Info("\t{ %f, %f, %f }, // Center\n", \
	           (DBL)Center[X],  \
	           (DBL)Center[Y],  \
	           (DBL)Center[Z]); \
	Debug_Info("\t%f // Radius\n", (DBL)Radius);
	Debug_Info("}\n");
#endif

	Destroy_Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Sphere_BBox
*
* INPUT
*
*   Sphere - Sphere
*   
* OUTPUT
*
*   Sphere
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a sphere.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void Sphere::Compute_BBox()
{
	Make_BBox(BBox, Center[X] - Radius, Center[Y] - Radius,  Center[Z] - Radius, 2.0 * Radius, 2.0 * Radius, 2.0 * Radius);

	if(Trans != NULL)
	{
		Recompute_BBox(&BBox, Trans);
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Sphere_UVCoord
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Nathan Kopp   (adapted from spherical_image_map)
*
* DESCRIPTION
*
*   Find the UV coordinates of a sphere.
*   Map a point (x, y, z) on a sphere of radius 1 to a 2-d image. (Or is it the
*   other way around?)
*
* CHANGES
*
*   -
*
******************************************************************************/

void Sphere::UVCoord(UV_VECT Result, const Intersection *Inter, TraceThreadData *Thread) const
{
	DBL len, phi, theta;
	DBL x,y,z;
	VECTOR New_Point, New_Center;

	/* Transform the point into the sphere's space */
	if (UV_Trans != NULL)
	{

		MInvTransPoint(New_Point, Inter->IPoint, UV_Trans);

		if (Trans != NULL)
			MTransPoint(New_Center, Center, Trans);
		else
			Assign_Vector(New_Center, Center);

		MInvTransPoint(New_Center, New_Center, UV_Trans);
	}
	else
	{
		Assign_Vector(New_Point, Inter->IPoint);
		Assign_Vector(New_Center, Center);
	}
	x = New_Point[X]-New_Center[X];
	y = New_Point[Y]-New_Center[Y];
	z = New_Point[Z]-New_Center[Z];

	len = sqrt(x * x + y * y + z * z);

	if (len == 0.0)
		return;
	else
	{
		x /= len;
		y /= len;
		z /= len;
	}

	/* Determine its angle from the x-z plane. */
	phi = 0.5 + asin(y) / M_PI; /* This will be from 0 to 1 */

	/* Determine its angle from the point (1, 0, 0) in the x-z plane. */
	len = x * x + z * z;

	if (len > EPSILON)
	{
		len = sqrt(len);
		if (z == 0.0)
		{
			if (x > 0)
				theta = 0.0;
			else
				theta = M_PI;
		}
		else
		{
			theta = acos(x / len);
			if (z < 0.0)
				theta = TWO_M_PI - theta;
		}

		theta /= TWO_M_PI;  /* This will be from 0 to 1 */
	}
	else
		/* This point is at one of the poles. Any value of xcoord will be ok... */
		theta = 0;

	Result[U] = theta;
	Result[V] = phi;
}

bool Sphere::Intersect_BBox(BBoxDirection, const BBOX_VECT&, const BBOX_VECT&, BBOX_VAL) const
{
	return true;
}

}
