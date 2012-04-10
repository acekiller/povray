/*******************************************************************************
 * polygon.cpp
 *
 * This module implements functions that manipulate polygons.
 *
 * This module was written by Dieter Bayer [DB].
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
 * $File: //depot/povray/smp/source/backend/shape/polygon.cpp $
 * $Revision: #29 $
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

/****************************************************************************
*
*  Explanation:
*
*  Syntax:
*
*  ---
*
*  The "inside polygon"-test was taken from:
*
*    E. Haines, "An Introduction to Ray-Tracing", ..., pp. 53-59
*
*  ---
*
*  May 1994 : Creation. [DB]
*
*  Oct 1994 : Changed polygon structure. Polygon points are now stored
*             in a seperate structure. This - together with the use of
*             a transformation - allows to keep just one point definition
*             for multiple copies of one polygon. [DB]
*
*****************************************************************************/

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/shape/polygon.h"
#include "backend/scene/threaddata.h"
#include "base/pov_err.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* Minimal intersection depth for a valid intersection. */
const DBL DEPTH_TOLERANCE = 1.0e-8;

/* If |x| < ZERO_TOLERANCE x is assumed to be 0. */
const DBL ZERO_TOLERANCE = 1.0e-10;



/*****************************************************************************
*
* FUNCTION
*
*   All_Polygon_Intersections
*
* INPUT
*
*   Object      - Object
*   Ray         - Ray
*   Depth_Stack - Intersection stack
*   
* OUTPUT
*
*   Depth_Stack
*   
* RETURNS
*
*   int - true, if a intersection was found
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Determine ray/polygon intersection and clip intersection found.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Polygon::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	DBL Depth;
	VECTOR IPoint;

	if (Intersect(ray, &Depth, Thread))
	{
		VEvaluateRay(IPoint, ray.Origin, Depth, ray.Direction);

		if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
		{
			Depth_Stack->push(Intersection(Depth, IPoint, this));

			return(true);
		}
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_poylgon
*
* INPUT
*
*   Ray     - Ray
*   Polyg - Polygon
*   Depth   - Depth of intersection found
*   
* OUTPUT
*
*   Depth
*   
* RETURNS
*
*   int - true if intersection found
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Determine ray/polygon intersection.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Polygon::Intersect(const Ray& ray, DBL *Depth, TraceThreadData *Thread) const
{
	DBL x, y, len;
	VECTOR p, d;

	/* Don't test degenerate polygons. */

	if (Test_Flag(this, DEGENERATE_FLAG))
		return(false);

	Thread->Stats()[Ray_Polygon_Tests]++;

	/* Transform the ray into the polygon space. */

	MInvTransPoint(p, ray.Origin, Trans);

	MInvTransDirection(d, ray.Direction, Trans);

	VLength(len, d);

	VInverseScaleEq(d, len);

	/* Intersect ray with the plane in which the polygon lies. */

	if (fabs(d[Z]) < ZERO_TOLERANCE)
		return(false);

	*Depth = -p[Z] / d[Z];

	if ((*Depth < DEPTH_TOLERANCE) || (*Depth > MAX_DISTANCE))
		return(false);

	/* Does the intersection point lie inside the polygon? */

	x = p[X] + *Depth * d[X];
	y = p[Y] + *Depth * d[Y];

	if (in_polygon(Data->Number, Data->Points, x, y))
	{
		Thread->Stats()[Ray_Polygon_Tests_Succeeded]++;

		*Depth /= len;

		return (true);
	}
	else
		return (false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Polygon
*
* INPUT
*
*   IPoint - Intersection point
*   Object - Object
*   
* OUTPUT
*   
* RETURNS
*
*   int - always false
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Polygons can't be used in CSG.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Polygon::Inside(const VECTOR, TraceThreadData *Thread) const
{
	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Polygon_Normal
*
* INPUT
*
*   Result - Normal vector
*   Object - Object
*   Inter  - Intersection found
*   
* OUTPUT
*
*   Result
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the normal of the polygon in a given point.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Normal(VECTOR Result, Intersection *, TraceThreadData *) const
{
	Assign_Vector(Result, S_Normal);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Polygon
*
* INPUT
*
*   Object - Object
*   Vector - Translation vector
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Translate a polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Polygon
*
* INPUT
*
*   Object - Object
*   Vector - Rotation vector
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Rotate a polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Polygon
*
* INPUT
*
*   Object - Object
*   Vector - Scaling vector
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Scale a polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Polygon
*
* INPUT
*
*   Object - Object
*   Trans  - Transformation to apply
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Transform a polygon by transforming all points
*   and recalculating the polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Transform(const TRANSFORM *tr)
{
	VECTOR N;

	if(Trans == NULL)
		Trans = Create_Transform();

	Compose_Transforms(Trans, tr);

	Make_Vector(N, 0.0, 0.0, 1.0);
	MTransNormal(S_Normal, N, Trans);

	VNormalizeEq(S_Normal);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Polygon
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Invert a polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Invert()
{
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Polygon
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   POLYGON * - new polygon
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Create a new polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

Polygon::Polygon() : ObjectBase(POLYGON_OBJECT)
{
	Trans = Create_Transform();

	Make_Vector(S_Normal, 0.0, 0.0, 1.0);

	Data = NULL;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Polygon
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*   
* RETURNS
*
*   void * - New polygon
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Copy a polygon structure.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

ObjectPtr Polygon::Copy()
{
	Polygon *New = new Polygon();

	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);
	New->Data = Data;
	New->Data->References++;

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Polygon
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Destroy a polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
*   Dec 1994 : Fixed memory leakage. [DB]
*
******************************************************************************/

Polygon::~Polygon()
{
	if (--(Data->References) == 0)
	{
		POV_FREE (Data->Points);

		POV_FREE (Data);
	}

	Destroy_Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Polygon
*
* INPUT
*
*   Polyg - Polygon
*   Points  - 3D points describing the polygon
*   
* OUTPUT
*
*   Polyg
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Compute the following things for a given polygon:
*
*     - Polygon transformation
*
*     - Array of 2d points describing the shape of the polygon
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Compute_Polygon(int number, VECTOR *points)
{
	int i;
	DBL x, y, z, d;
	VECTOR o, u, v, w, N;
	MATRIX a, b;

	/* Create polygon data. */

	if (Data == NULL)
	{
		Data = (POLYGON_DATA *)POV_MALLOC(sizeof(POLYGON_DATA), "polygon points");

		Data->References = 1;

		Data->Number = number;

		Data->Points = (UV_VECT *)POV_MALLOC(number*sizeof(UV_VECT), "polygon points");
	}
	else
	{
		throw POV_EXCEPTION_STRING("Polygon data already computed.");
	}

	/* Get polygon's coordinate system (one of the many possible) */

	Assign_Vector(o, points[0]);

	/* Find valid, i.e. non-zero u vector. */

	for (i = 1; i < number; i++)
	{
		VSub(u, points[i], o);

		if (VSumSqr(u) > EPSILON)
		{
			break;
		}
	}

	if (i == number)
	{
		Set_Flag(this, DEGENERATE_FLAG);

;// TODO MESSAGE    Warning(0, "Points in polygon are co-linear. Ignoring polygon.");
	}

	/* Find valid, i.e. non-zero v and w vectors. */

	for (i++; i < number; i++)
	{
		VSub(v, points[i], o);

		VCross(w, u, v);

		if ((VSumSqr(v) > EPSILON) && (VSumSqr(w) > EPSILON))
		{
			break;
		}
	}

	if (i == number)
	{
		Set_Flag(this, DEGENERATE_FLAG);

;// TODO MESSAGE    Warning(0, "Points in polygon are co-linear. Ignoring polygon.");
	}

	VCross(u, v, w);
	VCross(v, w, u);

	VNormalize(u, u);
	VNormalize(v, v);
	VNormalize(w, w);

	MIdentity(a);
	MIdentity(b);

	a[3][0] = -o[X];
	a[3][1] = -o[Y];
	a[3][2] = -o[Z];

	b[0][0] =  u[X];
	b[1][0] =  u[Y];
	b[2][0] =  u[Z];

	b[0][1] =  v[X];
	b[1][1] =  v[Y];
	b[2][1] =  v[Z];

	b[0][2] =  w[X];
	b[1][2] =  w[Y];
	b[2][2] =  w[Z];

	MTimesC(Trans->inverse, a, b);

	MInvers(Trans->matrix, Trans->inverse);

	/* Project points onto the u,v-plane (3D --> 2D) */

	for (i = 0; i < number; i++)
	{
		x = points[i][X] - o[X];
		y = points[i][Y] - o[Y];
		z = points[i][Z] - o[Z];

		d = x * w[X] + y * w[Y] + z * w[Z];

		if (fabs(d) > ZERO_TOLERANCE)
		{
			Set_Flag(this, DEGENERATE_FLAG);

;// TODO MESSAGE      Warning(0, "Points in polygon are not co-planar. Ignoring polygons.");
		}

		Data->Points[i][X] = x * u[X] + y * u[Y] + z * u[Z];
		Data->Points[i][Y] = x * v[X] + y * v[Y] + z * v[Z];
	}

	Make_Vector(N, 0.0, 0.0, 1.0);
	MTransNormal(S_Normal, N, Trans);

	VNormalizeEq(S_Normal);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Polygon_BBox
*
* INPUT
*
*   Polyg - Polygon
*   
* OUTPUT
*
*   Polyg
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a polygon.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Polygon::Compute_BBox()
{
	int i;
	VECTOR p, Puv, Min, Max;

	Min[X] = Min[Y] = Min[Z] =  BOUND_HUGE;
	Max[X] = Max[Y] = Max[Z] = -BOUND_HUGE;

	for (i = 0; i < Data->Number; i++)
	{
		Puv[X] = Data->Points[i][X];
		Puv[Y] = Data->Points[i][Y];
		Puv[Z] = 0.0;

		MTransPoint(p, Puv, Trans);

		Min[X] = min(Min[X], p[X]);
		Min[Y] = min(Min[Y], p[Y]);
		Min[Z] = min(Min[Z], p[Z]);
		Max[X] = max(Max[X], p[X]);
		Max[Y] = max(Max[Y], p[Y]);
		Max[Z] = max(Max[Z], p[Z]);
	}

	Make_BBox_from_min_max(BBox, Min, Max);

	if (fabs(BBox.Lengths[X]) < SMALL_TOLERANCE)
	{
		BBox.Lower_Left[X] -= SMALL_TOLERANCE;
		BBox.Lengths[X]    += 2.0 * SMALL_TOLERANCE;
	}

	if (fabs(BBox.Lengths[Y]) < SMALL_TOLERANCE)
	{
		BBox.Lower_Left[Y] -= SMALL_TOLERANCE;
		BBox.Lengths[Y]    += 2.0 * SMALL_TOLERANCE;
	}

	if (fabs(BBox.Lengths[Z]) < SMALL_TOLERANCE)
	{
		BBox.Lower_Left[Z] -= SMALL_TOLERANCE;
		BBox.Lengths[Z]    += 2.0 * SMALL_TOLERANCE;
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   in_polygon
*
* INPUT
*
*   Number - Number of points
*   Points - Points describing polygon's shape
*   u, v   - 2D-coordinates of the point to test
*   
* OUTPUT
*   
* RETURNS
*
*   int - true, if inside
*   
* AUTHOR
*
*   Eric Haines, 3D/Eye Inc, erich@eye.com
*   
* DESCRIPTION
*
* ======= Crossings Multiply algorithm ===================================
*
* This version is usually somewhat faster than the original published in
* Graphics Gems IV; by turning the division for testing the X axis crossing
* into a tricky multiplication test this part of the test became faster,
* which had the additional effect of making the test for "both to left or
* both to right" a bit slower for triangles than simply computing the
* intersection each time.  The main increase is in triangle testing speed,
* which was about 15% faster; all other polygon complexities were pretty much
* the same as before.  On machines where division is very expensive (not the
* case on the HP 9000 series on which I tested) this test should be much
* faster overall than the old code.  Your mileage may (in fact, will) vary,
* depending on the machine and the test data, but in general I believe this
* code is both shorter and faster.  This test was inspired by unpublished
* Graphics Gems submitted by Joseph Samosky and Mark Haigh-Hutchinson.
* Related work by Samosky is in:
*
* Samosky, Joseph, "SectionView: A system for interactively specifying and
* visualizing sections through three-dimensional medical image data",
* M.S. Thesis, Department of Electrical Engineering and Computer Science,
* Massachusetts Institute of Technology, 1993.
*
*
* Shoot a test ray along +X axis.  The strategy is to compare vertex Y values
* to the testing point's Y and quickly discard edges which are entirely to one
* side of the test ray.
*
* CHANGES
*
*   -
*
******************************************************************************/

bool Polygon::in_polygon(int number, const UV_VECT *points, DBL u, DBL  v)
{
	register int i, yflag0, yflag1, inside_flag;
	register DBL ty, tx;
	register const DBL *vtx0, *vtx1, *first;

	tx = u;
	ty = v;

	vtx0 = &points[0][X];
	vtx1 = &points[1][X];

	first = vtx0;

	/* get test bit for above/below X axis */

	yflag0 = (vtx0[Y] >= ty);

	inside_flag = false;

	for (i = 1; i < number; )
	{
		yflag1 = (vtx1[Y] >= ty);

		/*
		 * Check if endpoints straddle (are on opposite sides) of X axis
		 * (i.e. the Y's differ); if so, +X ray could intersect this edge.
		 * The old test also checked whether the endpoints are both to the
		 * right or to the left of the test point.  However, given the faster
		 * intersection point computation used below, this test was found to
		 * be a break-even proposition for most polygons and a loser for
		 * triangles (where 50% or more of the edges which survive this test
		 * will cross quadrants and so have to have the X intersection computed
		 * anyway).  I credit Joseph Samosky with inspiring me to try dropping
		 * the "both left or both right" part of my code.
		 */

		if (yflag0 != yflag1)
		{
			/*
			 * Check intersection of pgon segment with +X ray.
			 * Note if >= point's X; if so, the ray hits it.
			 * The division operation is avoided for the ">=" test by checking
			 * the sign of the first vertex wrto the test point; idea inspired
			 * by Joseph Samosky's and Mark Haigh-Hutchinson's different
			 * polygon inclusion tests.
			 */

			if (((vtx1[Y]-ty) * (vtx0[X]-vtx1[X]) >= (vtx1[X]-tx) * (vtx0[Y]-vtx1[Y])) == yflag1)
			{
				inside_flag = !inside_flag;
			}
		}

		/* Move to the next pair of vertices, retaining info as possible. */

		if ((i < number-2) && (vtx1[X] == first[X]) && (vtx1[Y] == first[Y]))
		{
			vtx0 = &points[++i][X];
			vtx1 = &points[++i][X];

			yflag0 = (vtx0[Y] >= ty);

			first = vtx0;
		}
		else
		{
			vtx0 = vtx1;
			vtx1 = &points[++i][X];

			yflag0 = yflag1;
		}
	}

	return(inside_flag);
}

}
