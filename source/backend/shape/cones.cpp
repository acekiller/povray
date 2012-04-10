/*******************************************************************************
 * cones.cpp
 *
 * This module implements the cone primitive.
 * This file was written by Alexander Enzmann.    He wrote the code for
 * cones and generously provided us these enhancements.
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
 * $File: //depot/povray/smp/source/backend/shape/cones.cpp $
 * $Revision: #34 $
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
#include "backend/shape/cones.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/scene/threaddata.h"
#include "base/pov_err.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const DBL Cone_Tolerance = 1.0e-9;

#define close(x, y) (fabs(x-y) < EPSILON ? 1 : 0)

/* Part of the cone/cylinder hit. [DB 9/94] */

const int BASE_HIT = 1;
const int CAP_HIT  = 2;
const int SIDE_HIT = 3;



/*****************************************************************************
*
* FUNCTION
*
*   All_Cone_Intersections
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

bool Cone::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int Intersection_Found, cnt, i;
	VECTOR IPoint;
	CONE_INT I[4];

	Intersection_Found = false;

	if ((cnt = Intersect(ray, I, Thread)) != 0)
	{
		for (i = 0; i < cnt; i++)
		{
			VEvaluateRay(IPoint, ray.Origin, I[i].d, ray.Direction);

			if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
			{
				Depth_Stack->push(Intersection(I[i].d,IPoint,this,I[i].t));
				Intersection_Found = true;
			}
		}
	}

	return (Intersection_Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_cone
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

int Cone::Intersect(const Ray& ray, CONE_INT *Intersection, TraceThreadData *Thread) const
{
	int i = 0;
	DBL a, b, c, z, t1, t2, len;
	DBL d;
	VECTOR P, D;

	Thread->Stats()[Ray_Cone_Tests]++;

	/* Transform the ray into the cones space */

	MInvTransPoint(P, ray.Origin, Trans);
	MInvTransDirection(D, ray.Direction, Trans);

	VLength(len, D);
	VInverseScaleEq(D, len);

	if (Test_Flag(this, CYLINDER_FLAG))
	{
		/* Solve intersections with a cylinder */

		a = D[X] * D[X] + D[Y] * D[Y];

		if (a > EPSILON)
		{
			b = P[X] * D[X] + P[Y] * D[Y];

			c = P[X] * P[X] + P[Y] * P[Y] - 1.0;

			d = b * b - a * c;

			if (d >= 0.0)
			{
				d = sqrt(d);

				t1 = (-b + d) / a;
				t2 = (-b - d) / a;

				z = P[Z] + t1 * D[Z];

				if ((t1 > Cone_Tolerance) && (t1 < MAX_DISTANCE) && (z >= 0.0) && (z <= 1.0))
				{
					Intersection[i].d   = t1 / len;
					Intersection[i++].t = SIDE_HIT;
				}

				z = P[Z] + t2 * D[Z];

				if ((t2 > Cone_Tolerance) && (t1 < MAX_DISTANCE) && (z >= 0.0) && (z <= 1.0))
				{
					Intersection[i].d   = t2 / len;
					Intersection[i++].t = SIDE_HIT;
				}
			}
		}
	}
	else
	{
		/* Solve intersections with a cone */

		a = D[X] * D[X] + D[Y] * D[Y] - D[Z] * D[Z];

		b = D[X] * P[X] + D[Y] * P[Y] - D[Z] * P[Z];

		c = P[X] * P[X] + P[Y] * P[Y] - P[Z] * P[Z];

		if (fabs(a) < EPSILON)
		{
			if (fabs(b) > EPSILON)
			{
				/* One intersection */

				t1 = -0.5 * c / b;

				z = P[Z] + t1 * D[Z];

				if ((t1 > Cone_Tolerance) && (t1 < MAX_DISTANCE) && (z >= dist) && (z <= 1.0))
				{
					Intersection[i].d   = t1 / len;
					Intersection[i++].t = SIDE_HIT;
				}
			}
		}
		else
		{
			/* Check hits against the side of the cone */

			d = b * b - a * c;

			if (d >= 0.0)
			{
				d = sqrt(d);

				t1 = (-b - d) / a;
				t2 = (-b + d) / a;

				z = P[Z] + t1 * D[Z];

				if ((t1 > Cone_Tolerance) && (t1 < MAX_DISTANCE) && (z >= dist) && (z <= 1.0))
				{
					Intersection[i].d   = t1 / len;
					Intersection[i++].t = SIDE_HIT;
				}

				z = P[Z] + t2 * D[Z];

				if ((t2 > Cone_Tolerance) && (t1 < MAX_DISTANCE) && (z >= dist) && (z <= 1.0))
				{
					Intersection[i].d   = t2 / len;
					Intersection[i++].t = SIDE_HIT;
				}
			}
		}
	}

	if (Test_Flag(this, CLOSED_FLAG) && (fabs(D[Z]) > EPSILON))
	{
		d = (1.0 - P[Z]) / D[Z];

		a = (P[X] + d * D[X]);

		b = (P[Y] + d * D[Y]);

		if (((Sqr(a) + Sqr(b)) <= 1.0) && (d > Cone_Tolerance) && (d < MAX_DISTANCE))
		{
			Intersection[i].d   = d / len;
			Intersection[i++].t = CAP_HIT;
		}

		d = (dist - P[Z]) / D[Z];

		a = (P[X] + d * D[X]);

		b = (P[Y] + d * D[Y]);

		if ((Sqr(a) + Sqr(b)) <= (Test_Flag(this, CYLINDER_FLAG) ? 1.0 : Sqr(dist))
			&& (d > Cone_Tolerance) && (d < MAX_DISTANCE))
		{
			Intersection[i].d   = d / len;
			Intersection[i++].t = BASE_HIT;
		}
	}

	if (i)
		Thread->Stats()[Ray_Cone_Tests_Succeeded]++;

	return (i);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Cone
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

bool Cone::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	DBL w2, z2, offset = (Test_Flag(this, CLOSED_FLAG) ? -EPSILON : EPSILON);
	VECTOR New_Point;

	/* Transform the point into the cones space */

	MInvTransPoint(New_Point, IPoint, Trans);

	/* Test to see if we are inside the cone */

	w2 = New_Point[X] * New_Point[X] + New_Point[Y] * New_Point[Y];

	if (Test_Flag(this, CYLINDER_FLAG))
	{
		/* Check to see if we are inside a cylinder */

		if ((w2 > 1.0 + offset) ||
		    (New_Point[Z] < 0.0 - offset) ||
		    (New_Point[Z] > 1.0 + offset))
		{
			return (Test_Flag(this, INVERTED_FLAG));
		}
		else
		{
			return (!Test_Flag(this, INVERTED_FLAG));
		}
	}
	else
	{
		/* Check to see if we are inside a cone */

		z2 = New_Point[Z] * New_Point[Z];

		if ((w2 > z2 + offset) ||
		    (New_Point[Z] < dist - offset) ||
		    (New_Point[Z] > 1.0+offset))
		{
			return (Test_Flag(this, INVERTED_FLAG));
		}
		else
		{
			return (!Test_Flag(this, INVERTED_FLAG));
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Cone_Normal
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	/* Transform the point into the cones space */

	MInvTransPoint(Result, Inter->IPoint, Trans);

	/* Calculating the normal is real simple in canonical cone space */

	switch (Inter->i1)
	{
		case SIDE_HIT:

			if (Test_Flag(this, CYLINDER_FLAG))
			{
				Result[Z] = 0.0;
			}
			else
			{
				Result[Z] = -Result[Z];
			}

			break;

		case BASE_HIT:

			Make_Vector(Result, 0.0, 0.0, -1.0);

			break;

		case CAP_HIT:

			Make_Vector(Result, 0.0, 0.0, 1.0);

			break;
	}

	/* Transform the point out of the cones space */

	MTransNormal(Result, Result, Trans);

	VNormalize(Result, Result);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Transform(const TRANSFORM *tr)
{
	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

Cone::Cone() : ObjectBase(CONE_OBJECT)
{
	Make_Vector(apex, 0.0, 0.0, 1.0);
	Make_Vector(base, 0.0, 0.0, 0.0);

	apex_radius = 1.0;
	base_radius = 0.0;

	dist = 0.0;

	Trans = Create_Transform();

	/* Cone/Cylinder has capped ends by default. */

	Set_Flag(this, CLOSED_FLAG);

	/* Default bounds */

	Make_BBox(BBox, -1.0, -1.0, 0.0, 2.0, 2.0, 1.0);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

ObjectPtr Cone::Copy()
{
	Cone *New = new Cone();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);
	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Cylinder
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Cylinder()
{
	apex_radius = 1.0;
	base_radius = 1.0;

	Set_Flag(this, CYLINDER_FLAG); // This is a cylinder.
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Cone_Data
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1996: check for equal sized ends (cylinder) first [AED]
*
******************************************************************************/

void Cone::Compute_Cone_Data()
{
	DBL tlen, len, tmpf;
	VECTOR tmpv, axis, origin;

	/* Process the primitive specific information */

	/* Find the axis and axis length */

	VSub(axis, apex, base);

	VLength(len, axis);

	if (len < EPSILON)
	{
		throw POV_EXCEPTION_STRING("Degenerate cone/cylinder."); // TODO FIXME - should a possible error
	}
	else
	{
		VInverseScaleEq(axis, len);
	}
	/* we need to trap that case first */
	if (fabs(apex_radius - base_radius) < EPSILON)
	{
		/* What we are dealing with here is really a cylinder */

		Set_Flag(this, CYLINDER_FLAG);

		Compute_Cylinder_Data();

		return;
	}

	if (apex_radius < base_radius)
	{
		/* Want the bigger end at the top */

		Assign_Vector(tmpv,base);
		Assign_Vector(base,apex);
		Assign_Vector(apex,tmpv);

		tmpf = base_radius;
		base_radius = apex_radius;
		apex_radius = tmpf;
		VScaleEq(axis, -1.0);
	}
	/* apex & base are different, yet, it might looks like a cylinder */
	tmpf = base_radius * len / (apex_radius - base_radius);

	VScale(origin, axis, tmpf);

	VSub(origin, base, origin);

	tlen = tmpf + len;
	/* apex is always bigger here */
	if (((apex_radius - base_radius)*len/tlen) < EPSILON)
	{
		/* What we are dealing with here is really a cylinder */

		Set_Flag(this, CYLINDER_FLAG);

		Compute_Cylinder_Data();

		return;
	}

	dist = tmpf / tlen;
	/* Determine alignment */
	Compute_Coordinate_Transform(Trans, origin, axis, apex_radius, tlen);

	/* Recalculate the bounds */

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Cylinder_Data
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

void Cone::Compute_Cylinder_Data()
{
	DBL tmpf;
	VECTOR axis;

	VSub(axis, apex, base);

	VLength(tmpf, axis);

	if (tmpf < EPSILON)
	{
		throw POV_EXCEPTION_STRING("Degenerate cylinder, base point = apex point."); // TODO FIXME - should a possible error
	}
	else
	{
		VInverseScaleEq(axis, tmpf);

		Compute_Coordinate_Transform(Trans, base, axis, apex_radius, tmpf);
	}

	dist = 0.0;

	/* Recalculate the bounds */

	Compute_BBox();
}




/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Cone
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Alexander Enzmann
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

Cone::~Cone()
{
	Destroy_Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Cone_BBox
*
* INPUT
*
*   Cone - Cone/Cylinder
*
* OUTPUT
*
*   Cone
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate the bounding box of a cone or cylinder.
*
* CHANGES
*
*   Aug 1994 : Creation.
*       2000 : cone  bounding fix
*
******************************************************************************/

void Cone::Compute_BBox()
{
	Make_BBox(BBox, -1.0, -1.0, dist, 2.0, 2.0, 1.0-dist);

	Recompute_BBox(&BBox, Trans);
}

}
