/*******************************************************************************
 * sor.cpp
 *
 * This module implements functions that manipulate surfaces of revolution.
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
 * $File: //depot/povray/smp/source/backend/shape/sor.cpp $
 * $Revision: #32 $
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
*    The surface of revolution primitive is defined by a set of points
*    in 2d space wich are interpolated by cubic splines. The resulting
*    2d function is rotated about an axis to form the final object.
*
*    All calculations are done in the object's (u,v,w)-coordinate system
*    with the (w)-axis being the rotation axis.
*
*    One spline segment in the (r,w)-plane is given by the equation
*
*      r^2 = f(w) = A * w * w * w + B * w * w + C * w + D.
*
*    To intersect a ray R = P + k * D transformed into the object's
*    coordinate system with the surface of revolution, the equation
*
*      (Pu + k * Du)^2 + (Pv + k * Dv)^2 = f(Pw + k * Dw)
*
*    has to be solved for k (cubic polynomial in k).
*
*    Note that Pu, Pv, Pw and Du, Dv, Dw denote the coordinates
*    of the vectors P and D.
*
*  Syntax:
*
*    revolution
*    {
*      number_of_points,
*
*      <P[0]>, <P[1]>, ..., <P[n-1]>
*
*      [ open ]
*    }
*
*    Note that the P[i] are 2d vectors where u corresponds to the radius
*    and v to the height.
*
*    Note that the first and last point, i.e. P[0] and P[n-1], are used
*    to determine the derivatives at the end point.
*
*    Note that the x coordinate of a point corresponds to the radius and
*    the y coordinate to the height; the z coordinate isn't used.
*
*  ---
*
*  Ideas for the surface of revolution were taken from:
*
*    P. Burger and D. Gillies, "Rapid Ray Tracing of General Surfaces
*    of Revolution", New Advances in Computer Graphics, Proceedings
*    of CG International '89, R. A. Earnshaw, B. Wyvill (Eds.),
*    Springer, ..., pp. 523-531
*
*  ---
*
*  May 1994 : Creation. [DB]
*
*****************************************************************************/

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/bounding/bbox.h"
#include "backend/math/polysolv.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/shape/sor.h"
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

const DBL DEPTH_TOLERANCE = 1.0e-4;

/* Part of the surface of revolution hit. */

const int BASE_PLANE = 1;
const int CAP_PLANE  = 2;
const int CURVE      = 3;

/* Max. number of intersecions per spline segment. */

const int MAX_INTERSECTIONS_PER_SEGMENT = 4;



/*****************************************************************************
*
* FUNCTION
*
*   All_Sor_Intersections
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
*   Determine ray/surface of revolution intersection and
*   clip intersection found.
*
* CHANGES
*
*   May 1994 : Creation.
*   Oct 1996 : Changed code to include faster version. [DB]
*
******************************************************************************/

bool Sor::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	Thread->Stats()[Ray_Sor_Tests]++;

	if (Intersect(ray, Depth_Stack, Thread))
	{
		Thread->Stats()[Ray_Sor_Tests_Succeeded]++;

		return(true);
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_sor
*
* INPUT
*
*   Ray          - Ray
*   Sor   - Sor
*   Intersection - Sor intersection structure
*   
* OUTPUT
*
*   Intersection
*   
* RETURNS
*
*   int - Number of intersections found
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Determine ray/surface of revolution intersection.
*
*   NOTE: The curve is rotated about the y-axis!
*         Order reduction cannot be used.
*
* CHANGES
*
*   May 1994 : Creation.
*   Oct 1996 : Changed code to include faster version. [DB]
*
******************************************************************************/

bool Sor::Intersect(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int cnt;
	int found, j, n;
	DBL a, b, k, h, len, u, v, r0;
	DBL x[4];
	DBL y[3];
	DBL best;
	VECTOR P, D;
	SOR_SPLINE_ENTRY *Entry;

	/* Transform the ray into the surface of revolution space. */

	MInvTransPoint(P, ray.Origin, Trans);

	MInvTransDirection(D, ray.Direction, Trans);

	VLength(len, D);

	VInverseScaleEq(D, len);

	/* Test if ray misses object's bounds. */

#ifdef SOR_EXTRA_STATS
	Thread->Stats()[Sor_Bound_Tests]++;
#endif

	if (((D[Y] >= 0.0) && (P[Y] >  Height2)) ||
	    ((D[Y] <= 0.0) && (P[Y] <  Height1)) ||
	    ((D[X] >= 0.0) && (P[X] >  Radius2)) ||
	    ((D[X] <= 0.0) && (P[X] < -Radius2)))
	{
		return(false);
	}

	/* Get distance of the ray from rotation axis (= y axis). */

	r0 = P[X] * D[Z] - P[Z] * D[X];

	if ((a = D[X] * D[X] + D[Z] * D[Z]) > 0.0)
	{
		r0 /= sqrt(a);
	}

	/* Test if ray misses object's bounds. */

	if (r0 > Radius2)
	{
		return(false);
	}

	/* Test base/cap plane. */

	found = false;

	best = BOUND_HUGE;

	if (Test_Flag(this, CLOSED_FLAG) && (fabs(D[Y]) > EPSILON))
	{
		/* Test base plane. */

		if (Base_Radius_Squared > DEPTH_TOLERANCE)
		{
			k = (Height1 - P[Y]) / D[Y];

			u = P[X] + k * D[X];
			v = P[Z] + k * D[Z];

			b = u * u + v * v;

			if (b <= Base_Radius_Squared)
			{
				if (test_hit(ray, Depth_Stack, k / len, k, BASE_PLANE, 0, Thread))
				{
					found = true;

					if (k < best)
					{
						best = k;
					}
				}
			}
		}

		/* Test cap plane. */

		if (Cap_Radius_Squared > DEPTH_TOLERANCE)
		{
			k = (Height2 - P[Y]) / D[Y];

			u = P[X] + k * D[X];
			v = P[Z] + k * D[Z];

			b = u * u + v * v;

			if (b <= Cap_Radius_Squared)
			{
				if (test_hit(ray, Depth_Stack, k / len, k, CAP_PLANE, 0, Thread))
				{
					found = true;

					if (k < best)
					{
						best = k;
					}
				}
			}
		}
	}

	/* Intersect all cylindrical bounds. */
	BCYL_INT *intervals = (BCYL_INT *) Thread->BCyl_Intervals ;
	BCYL_INT *rint = (BCYL_INT *) Thread->BCyl_RInt ;
	BCYL_INT *hint = (BCYL_INT *) Thread->BCyl_HInt ;

	if ((cnt = Intersect_BCyl(Spline->BCyl, intervals, rint, hint, P, D)) == 0)
	{
#ifdef SOR_EXTRA_STATS
		if (found)
			Thread->Stats()[Sor_Bound_Tests_Succeeded]++;
#endif
		return(found);
	}

#ifdef SOR_EXTRA_STATS
	Thread->Stats()[Sor_Bound_Tests_Succeeded]++;
#endif

/* Step through the list of intersections. */

	for (j = 0; j < cnt; j++)
	{
		/* Get current segment. */

		Entry = &Spline->Entry[intervals[j].n];

		/* If we already have the best intersection we may exit. */

		if (!(Type & IS_CHILD_OBJECT) && (intervals[j].d[0] > best))
		{
			break;
		}

		/* Cubic curve. */

		x[0] = Entry->A * D[Y] * D[Y] * D[Y];

/*
		x[1] = D[Y] * D[Y] * (3.0 * Entry->A * P[Y] + Entry->B) - D[X] * D[X] - D[Z] * D[Z];
*/
		x[1] = D[Y] * D[Y] * (3.0 * Entry->A * P[Y] + Entry->B) - a;

		x[2] = D[Y] * (P[Y] * (3.0 * Entry->A * P[Y] + 2.0 * Entry->B) + Entry->C) - 2.0 * (P[X] * D[X] + P[Z] * D[Z]);

		x[3] = P[Y] * (P[Y] * (Entry->A * P[Y] + Entry->B) + Entry->C) + Entry->D - P[X] * P[X] - P[Z] * P[Z];

		n = Solve_Polynomial(3, x, y, Test_Flag(this, STURM_FLAG), 0.0, Thread);

		while (n--)
		{
			k = y[n];

			h = P[Y] + k * D[Y];

			if ((h >= Spline->BCyl->height[Spline->BCyl->entry[intervals[j].n].h1]) &&
			    (h <= Spline->BCyl->height[Spline->BCyl->entry[intervals[j].n].h2]))
			{
				if (test_hit(ray, Depth_Stack, k / len, k, CURVE, intervals[j].n, Thread))
				{
					found = true;

					if (y[n] < best)
					{
						best = k;
					}
				}
			}
		}
	}

	return(found);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Sor
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
*   int - true if inside
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Return true if point lies inside the surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Sor::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	int i;
	DBL r0, r;
	VECTOR P;
	SOR_SPLINE_ENTRY *Entry=NULL;

	/* Transform the point into the surface of revolution space. */

	MInvTransPoint(P, IPoint, Trans);

	/* Test if we are inside the cylindrical bound. */

	if ((P[Y] >= Height1) && (P[Y] <= Height2))
	{
		r0 = P[X] * P[X] + P[Z] * P[Z];

		/* Test if we are inside the cylindrical bound. */

		if (r0 <= Sqr(Radius2))
		{
			/* Now find the segment the point is in. */

			for (i = 0; i < Number; i++)
			{
				Entry = &Spline->Entry[i];

				if ((P[Y] >= Spline->BCyl->height[Spline->BCyl->entry[i].h1]) &&
				    (P[Y] <= Spline->BCyl->height[Spline->BCyl->entry[i].h2]))
				{
					break;
				}
			}

			/* Have we found any segment? */

			if (i < Number)
			{
				r = P[Y] * (P[Y] * (P[Y] * Entry->A + Entry->B) + Entry->C) + Entry->D;

				if (r0 <= r)
				{
					/* We're inside. */

					return(!Test_Flag(this, INVERTED_FLAG));
				}
			}
		}
	}

	/* We're outside. */

	return(Test_Flag(this, INVERTED_FLAG));
}



/*****************************************************************************
*
* FUNCTION
*
*   Sor_Normal
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
*   Calculate the normal of the surface of revolution in a given point.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	DBL k;
	VECTOR P;
	SOR_SPLINE_ENTRY *Entry;
	VECTOR N;

	switch (Inter->i1)
	{
		case CURVE:

			/* Transform the intersection point into the surface of revolution space. */

			MInvTransPoint(P, Inter->IPoint, Trans);

			if (P[X] * P[X] + P[Z] * P[Z] > DEPTH_TOLERANCE)
			{
				Entry = &Spline->Entry[Inter->i2];

				k = 0.5 * (P[Y] * (3.0 * Entry->A * P[Y] + 2.0 * Entry->B) + Entry->C);

				N[X] = P[X];
				N[Y] = -k;
				N[Z] = P[Z];
			}

			break;

		case BASE_PLANE:

			Make_Vector(N, 0.0, -1.0, 0.0);

			break;


		case CAP_PLANE:

			Make_Vector(N, 0.0, 1.0, 0.0);

			break;
	}

	/* Transform the normal out of the surface of revolution space. */

	MTransNormal(Result, N, Trans);

	VNormalize(Result, Result);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Sor
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
*   Translate a surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Sor
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
*   Rotate a surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Sor
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
*   Scale a surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Sor
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
*   Transform a surface of revolution and recalculate its bounding box.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Transform(const TRANSFORM *tr)
{
	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Sor
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
*   Invert a surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Sor
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   SOR * - new surface of revolution
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Create a new surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

Sor::Sor() : ObjectBase(SOR_OBJECT)
{
	Trans = Create_Transform();

	Spline = NULL;

	Radius2             = 0.0;
	Base_Radius_Squared = 0.0;
	Cap_Radius_Squared  = 0.0;

	/* SOR should have capped ends by default. CEY 3/98*/

	Set_Flag(this, CLOSED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Sor
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*   
* RETURNS
*
*   void * - New surface of revolution
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Copy a surface of revolution structure.
*
*   NOTE: The splines are not copied, only the number of references is
*         counted, so that Destray_Sor() knows if they can be destroyed.
*
* CHANGES
*
*   May 1994 : Creation.
*
*   Sep 1994 : fixed memory leakage [DB]
*
******************************************************************************/

ObjectPtr Sor::Copy()
{
	Sor *New = new Sor();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	New->Spline->References++;

	return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Sor
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
*   Destroy a surface of revolution.
*
*   NOTE: The splines are destroyed if they are no longer used by any copy.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

Sor::~Sor()
{
	Destroy_Transform(Trans);

	if (--(Spline->References) == 0)
	{
		Destroy_BCyl(Spline->BCyl);

		POV_FREE(Spline->Entry);

		POV_FREE(Spline);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Sor_BBox
*
* INPUT
*
*   Sor - Sor
*   
* OUTPUT
*
*   Sor
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a surface of revolution.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Compute_BBox()
{
	Make_BBox(BBox, -Radius2, Height1, -Radius2,
		2.0 * Radius2, Height2 - Height1, 2.0 * Radius2);

	Recompute_BBox(&BBox, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Sor
*
* INPUT
*
*   Sor - Sor
*   P          - Points defining surface of revolution
*   
* OUTPUT
*
*   Sor
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer, June 1994
*   
* DESCRIPTION
*
*   Calculate the spline segments of a surface of revolution
*   from a set of points.
*
*   Note that the number of points in the surface of revolution has to be set.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Sor::Compute_Sor(UV_VECT *P, TraceThreadData *Thread)
{
	int i, n;
	DBL *tmp_r1;
	DBL *tmp_r2;
	DBL *tmp_h1;
	DBL *tmp_h2;
	DBL A, B, C, D, w;
	DBL xmax, xmin, ymax, ymin;
	DBL k[4], x[4];
	DBL y[2], r[2];
	DBL c[3];
	MATRIX Mat;

	/* Allocate Number segments. */

	if (Spline == NULL)
	{
		Spline = (SOR_SPLINE *)POV_MALLOC(sizeof(SOR_SPLINE), "spline segments of surface of revoluion");

		Spline->References = 1;

		Spline->Entry = (SOR_SPLINE_ENTRY *)POV_MALLOC(Number*sizeof(SOR_SPLINE_ENTRY), "spline segments of surface of revoluion");
	}
	else
	{
		throw POV_EXCEPTION_STRING("Surface of revolution segments are already defined.");
	}

	/* Allocate temporary lists. */

	tmp_r1 = (DBL *)POV_MALLOC(Number * sizeof(DBL), "temp lathe data");
	tmp_r2 = (DBL *)POV_MALLOC(Number * sizeof(DBL), "temp lathe data");
	tmp_h1 = (DBL *)POV_MALLOC(Number * sizeof(DBL), "temp lathe data");
	tmp_h2 = (DBL *)POV_MALLOC(Number * sizeof(DBL), "temp lathe data");

	/* We want to know the size of the overall bounding cylinder. */

	xmax = ymax = -BOUND_HUGE;
	xmin = ymin =  BOUND_HUGE;

	/* Calculate segments, i.e. cubic patches. */

	for (i = 0; i < Number; i++)
	{
		if ((fabs(P[i+2][Y] - P[i][Y]) < EPSILON) ||
		    (fabs(P[i+3][Y] - P[i+1][Y]) < EPSILON))
		{
			throw POV_EXCEPTION_STRING("Incorrect point in surface of revolution.");
		}

		/* Use cubic interpolation. */

		k[0] = P[i+1][X] * P[i+1][X];
		k[1] = P[i+2][X] * P[i+2][X];
		k[2] = (P[i+2][X] - P[i][X]) / (P[i+2][Y] - P[i][Y]);
		k[3] = (P[i+3][X] - P[i+1][X]) / (P[i+3][Y] - P[i+1][Y]);

		k[2] *= 2.0 * P[i+1][X];
		k[3] *= 2.0 * P[i+2][X];

		w = P[i+1][Y];

		Mat[0][0] = w * w * w;
		Mat[0][1] = w * w;
		Mat[0][2] = w;
		Mat[0][3] = 1.0;

		Mat[2][0] = 3.0 * w * w;
		Mat[2][1] = 2.0 * w;
		Mat[2][2] = 1.0;
		Mat[2][3] = 0.0;

		w = P[i+2][Y];

		Mat[1][0] = w * w * w;
		Mat[1][1] = w * w;
		Mat[1][2] = w;
		Mat[1][3] = 1.0;

		Mat[3][0] = 3.0 * w * w;
		Mat[3][1] = 2.0 * w;
		Mat[3][2] = 1.0;
		Mat[3][3] = 0.0;

		MInvers(Mat, Mat);

		/* Calculate coefficients of cubic patch. */

		A = k[0] * Mat[0][0] + k[1] * Mat[0][1] + k[2] * Mat[0][2] + k[3] * Mat[0][3];
		B = k[0] * Mat[1][0] + k[1] * Mat[1][1] + k[2] * Mat[1][2] + k[3] * Mat[1][3];
		C = k[0] * Mat[2][0] + k[1] * Mat[2][1] + k[2] * Mat[2][2] + k[3] * Mat[2][3];
		D = k[0] * Mat[3][0] + k[1] * Mat[3][1] + k[2] * Mat[3][2] + k[3] * Mat[3][3];

		if (fabs(A) < EPSILON) A = 0.0;
		if (fabs(B) < EPSILON) B = 0.0;
		if (fabs(C) < EPSILON) C = 0.0;
		if (fabs(D) < EPSILON) D = 0.0;

		Spline->Entry[i].A = A;
		Spline->Entry[i].B = B;
		Spline->Entry[i].C = C;
		Spline->Entry[i].D = D;

		/* Get minimum and maximum radius**2 in current segment. */

		y[0] = P[i+1][Y];
		y[1] = P[i+2][Y];

		x[0] = x[2] = P[i+1][X];
		x[1] = x[3] = P[i+2][X];

		c[0] = 3.0 * A;
		c[1] = 2.0 * B;
		c[2] = C;

		n = Solve_Polynomial(2, c, r, false, 0.0, Thread);

		while (n--)
		{
			if ((r[n] >= y[0]) && (r[n] <= y[1]))
			{
				x[n] = sqrt(r[n] * (r[n] * (r[n] * A + B) + C) + D);
			}
		}

		/* Set current segment's bounding cylinder. */

		tmp_r1[i] = min(min(x[0], x[1]), min(x[2], x[3]));
		tmp_r2[i] = max(max(x[0], x[1]), max(x[2], x[3]));

		tmp_h1[i] = y[0];
		tmp_h2[i] = y[1];

		/* Keep track of overall bounding cylinder. */

		xmin = min(xmin, tmp_r1[i]);
		xmax = max(xmax, tmp_r2[i]);

		ymin = min(ymin, tmp_h1[i]);
		ymax = max(ymax, tmp_h2[i]);

/*
		fprintf(stderr, "bound spline segment %d: ", i);
		fprintf(stderr, "r = %f - %f, h = %f - %f\n", tmp_r1[i], tmp_r2[i], tmp_h1[i], tmp_h2[i]);
*/
	}

	/* Set overall bounding cylinder. */

	Radius1 = xmin;
	Radius2 = xmax;

	Height1 = ymin;
	Height2 = ymax;

	/* Get cap radius. */

	w = tmp_h2[Number-1];

	A = Spline->Entry[Number-1].A;
	B = Spline->Entry[Number-1].B;
	C = Spline->Entry[Number-1].C;
	D = Spline->Entry[Number-1].D;

	if ((Cap_Radius_Squared = w * (w * (A * w + B) + C) + D) < 0.0)
	{
		Cap_Radius_Squared = 0.0;
	}

	/* Get base radius. */

	w = tmp_h1[0];

	A = Spline->Entry[0].A;
	B = Spline->Entry[0].B;
	C = Spline->Entry[0].C;
	D = Spline->Entry[0].D;

	if ((Base_Radius_Squared = w * (w * (A * w + B) + C) + D) < 0.0)
	{
		Base_Radius_Squared = 0.0;
	}

	/* Get bounding cylinder. */

	Spline->BCyl = Create_BCyl(Number, tmp_r1, tmp_r2, tmp_h1, tmp_h2);

	/* Get rid of temp. memory. */

	POV_FREE(tmp_h2);
	POV_FREE(tmp_h1);
	POV_FREE(tmp_r2);
	POV_FREE(tmp_r1);
}



/*****************************************************************************
*
* FUNCTION
*
*   test_hit
*
* INPUT
*
*   Sor         - Pointer to lathe structure
*   Ray         - Current ray
*   Depth_Stack - Current depth stack
*   d, t, n     - Intersection depth, type and number
*
* OUTPUT
*
*   Depth_Stack
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Test if a hit is valid and push if on the intersection depth.
*
* CHANGES
*
*   Oct 1996 : Creation.
*
******************************************************************************/

bool Sor::test_hit(const Ray &ray, IStack& Depth_Stack, DBL d, DBL k, int t, int n, TraceThreadData *Thread)
{
	VECTOR IPoint;

	if ((d > DEPTH_TOLERANCE) && (d < MAX_DISTANCE))
	{
		VEvaluateRay(IPoint, ray.Origin, d, ray.Direction);

		if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
		{
			/* is the extra copy of d redundant? */
			Depth_Stack->push(Intersection(d, IPoint, this, t, n, k));

			return(true);
		}
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Sor_UVCoord
*
* INPUT
*
*   Result - UV coordinates of intersection (u - rotation, v = height)
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
*   Nathan Kopp
*   
* DESCRIPTION
*
*   
*
* CHANGES
*
*   Oct 1998 : Creation.
*
******************************************************************************/

void Sor::UVCoord(UV_VECT Result, const Intersection *Inter, TraceThreadData *Thread) const
{
	DBL len, theta;
	DBL h, v_per_segment;
	VECTOR P;

	/* Transform the point into the lathe space. */
	MInvTransPoint(P, Inter->IPoint, Trans);

	/* Determine its angle from the point (1, 0, 0) in the x-z plane. */
	len = P[X] * P[X] + P[Z] * P[Z];

	if (len > EPSILON)
	{
		len = sqrt(len);
		if (P[Z] == 0.0)
		{
			if (P[X] > 0)
				theta = 0.0;
			else
				theta = M_PI;
		}
		else
		{
			theta = acos(P[X] / len);
			if (P[Z] < 0.0)
				theta = TWO_M_PI - theta;
		}

		theta /= TWO_M_PI;  /* This will be from 0 to 1 */
	}
	else
		/* This point is at one of the poles. Any value of xcoord will be ok... */
		theta = 0;

	Result[U] = theta;

	/* ------------------- now figure out v --------------------- */
	switch (Inter->i1)
	{
		case CURVE:
			/* h is width of this segment */
			h =
			 Spline->BCyl->height[Spline->BCyl->entry[Inter->i2].h2] -
			 Spline->BCyl->height[Spline->BCyl->entry[Inter->i2].h1];

			/* change in v per segment... divide total v (1.0) by number of segments */
			v_per_segment = 1.0/(Number);

			/* now find the current v given the current y */
			Result[V] = (P[Y] - Spline->BCyl->height[Spline->BCyl->entry[Inter->i2].h1]) / h
			           * v_per_segment + (Inter->i2*v_per_segment);

			break;

		case BASE_PLANE:
			/*Result[V] = 0;*/
			Result[V] = sqrt(P[X]*P[X]+P[Z]*P[Z])/sqrt(Base_Radius_Squared)-1;
			break;

		case CAP_PLANE:
			/*Result[V] = 1;*/
			Result[V] = -sqrt(P[X]*P[X]+P[Z]*P[Z])/sqrt(Cap_Radius_Squared)+2;
			break;
	}

	/*Result[V] = 0;*/

}

}
