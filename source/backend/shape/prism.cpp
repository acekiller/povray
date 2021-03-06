/*******************************************************************************
 * prism.cpp
 *
 * This module implements functions that manipulate prisms.
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
 * $File: //depot/povray/smp/source/backend/shape/prism.cpp $
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

/****************************************************************************
*
*  Explanation:
*
*    The prism primitive is defined by a set of points in 2d space which
*    are interpolated by linear, quadratic, or cubic splines. The resulting
*    2d curve is swept along a straight line to form the final prism object.
*
*    All calculations are done in the object's (u,v,w)-coordinate system
*    with the (w)-axis being the sweep axis.
*
*    One spline segment in the (u,v)-plane is given by the equations
*
*      fu(t) = Au * t * t * t + Bu * t * t + Cu * t + Du  and
*      fv(t) = Av * t * t * t + Bv * t * t + Cv * t + Dv,
*
*    with the parameter t ranging from 0 to 1.
*
*    To intersect a ray R = P + k * D transformed into the object's
*    coordinate system with the linear swept prism object, the equation
*
*      Dv * fu(t) - Du * fv(t) - (Pu * Dv - Pv * Du) = 0
*
*    has to be solved for t. For valid intersections (0 <= t <= 1)
*    the corresponding k can be calculated from equation
*
*      Pu + k * Du = fu(t) or Pv + k * Dv = fv(t).
*
*    In the case of conic sweeping the equation
*
*      (Pv * Dw - Pw * Dv) * fu(t) - (Pu * Dw - Pw * Du) * fv(t)
*                                  + (Pu * Dv - Pv * Du) = 0
*
*    has to be solved for t. For valid intersections (0 <= t <= 1)
*    the corresponding k can be calculated from equation
*
*      Pu + k * Du = (Pw + k * Dw) * fu(t) or
*      Pv + k * Dv = (Pw + k * Dw) * fv(t).
*
*    Note that the polynomal to solve has the same degree as the
*    spline segments used.
*
*    Note that Pu, Pv, Pw and Du, Dv, Dw denote the coordinates
*    of the vectors P and D.
*
*  Syntax:
*
*    prism {
*      [ linear_sweep | cubic_sweep ]
*      [ linear_spline | quadratic_spline | cubic_spline ]
*
*      height1, height2,
*      number_of_points
*
*      <P(0)>, <P(1)>, ..., <P(n-1)>
*
*      [ open ]
*      [ sturm ]
*    }
*
*    Note that the P(i) are 2d vectors.
*
*  ---
*
*  Ideas for the prism was taken from:
*
*    James T. Kajiya, "New techniques for ray tracing procedurally
*    defined objects", Computer Graphics, 17(3), July 1983, pp. 91-102
*
*    Kirk ...
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
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/scene/threaddata.h"
#include "backend/math/polysolv.h"
#include "backend/shape/prism.h"
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

/* Part of the prism hit. */

const int BASE_HIT   = 1;
const int CAP_HIT    = 2;
const int SPLINE_HIT = 3;



/*****************************************************************************
*
* FUNCTION
*
*   All_Prism_Intersections
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
*   Determine ray/prism intersection and clip intersection found.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Prism::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	bool Found = false ;
	VECTOR IPoint;
	int j, n;
	DBL k, u, v, w, h, len;
	DBL x[4];
	DBL y[3];
	DBL k1, k2, k3;
	VECTOR P, D;
	PRISM_SPLINE_ENTRY *Entry;
	DBL distance ;

	/* Don't test degenerate prisms. */
	if (Test_Flag(this, DEGENERATE_FLAG))
		return (false);

	Thread->Stats()[Ray_Prism_Tests]++;

	/* Transform the ray into the prism space */
	MInvTransPoint(P, ray.Origin, Trans);
	MInvTransDirection(D, ray.Direction, Trans);
	VLength(len, D);
	VInverseScaleEq(D, len);

	/* Test overall bounding rectangle. */

#ifdef PRISM_EXTRA_STATS
	Thread->Stats()[Prism_Bound_Tests]++;
#endif

	if (((D[X] >= 0.0) && (P[X] > x2)) ||
	    ((D[X] <= 0.0) && (P[X] < x1)) ||
	    ((D[Z] >= 0.0) && (P[Z] > y2)) ||
	    ((D[Z] <= 0.0) && (P[Z] < y1)))
	{
		return(false);
	}

#ifdef PRISM_EXTRA_STATS
	Thread->Stats()[Prism_Bound_Tests_Succeeded]++;
#endif

	/* Number of intersections found. */

	/* What kind of sweep is used? */

	switch (Sweep_Type)
	{
		case LINEAR_SWEEP :

			if (fabs(D[Y]) < EPSILON)
			{
				if ((P[Y] < Height1) || (P[Y] > Height2))
					return(false);
			}
			else
			{
				if (Test_Flag(this, CLOSED_FLAG))
				{
					/* Intersect ray with the cap-plane. */

					k = (Height2 - P[Y]) / D[Y];

					if ((k > DEPTH_TOLERANCE) && (k < MAX_DISTANCE))
					{
						u = P[X] + k * D[X];
						v = P[Z] + k * D[Z];

						if (in_curve(u, v, Thread))
						{
							distance = k / len;
							if ((distance > DEPTH_TOLERANCE) && (distance < MAX_DISTANCE))
							{
								VEvaluateRay(IPoint, ray.Origin, distance, ray.Direction);
								if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
								{
									Depth_Stack->push (Intersection (distance, IPoint, this, CAP_HIT, 0, 0));
									Found = true;
								}
							}
						}
					}

					/* Intersect ray with the base-plane. */

					k = (Height1 - P[Y]) / D[Y];

					if ((k > DEPTH_TOLERANCE) && (k < MAX_DISTANCE))
					{
						u = P[X] + k * D[X];
						v = P[Z] + k * D[Z];

						if (in_curve(u, v, Thread))
						{
							distance = k / len;
							if ((distance > DEPTH_TOLERANCE) && (distance < MAX_DISTANCE))
							{
								VEvaluateRay(IPoint, ray.Origin, distance, ray.Direction);
								if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
								{
									Depth_Stack->push (Intersection (distance, IPoint, this, BASE_HIT, 0, 0));
									Found = true;
								}
							}
						}
					}
				}
			}

			/* Intersect ray with all spline segments. */
			if ((fabs(D[X]) > EPSILON) || (fabs(D[Z]) > EPSILON))
			{
				Entry = Spline->Entry;
				for (j = 0; j < Number; j++, Entry++)
				{
#ifdef PRISM_EXTRA_STATS
					Thread->Stats()[Prism_Bound_Tests]++;
#endif
					/* Test spline's bounding rectangle (modified Cohen-Sutherland). */
					if (((D[X] >= 0.0) && (P[X] > Entry->x2)) ||
					    ((D[X] <= 0.0) && (P[X] < Entry->x1)) ||
					    ((D[Z] >= 0.0) && (P[Z] > Entry->y2)) ||
					    ((D[Z] <= 0.0) && (P[Z] < Entry->y1)))
					{
						continue;
					}

					/* Number of roots found. */
					n = 0;
					switch (Spline_Type)
					{
						case LINEAR_SPLINE :

#ifdef PRISM_EXTRA_STATS
							Thread->Stats()[Prism_Bound_Tests_Succeeded]++;
#endif
							/* Solve linear equation. */

							x[0] = Entry->C[X] * D[Z] - Entry->C[Y] * D[X];
							x[1] = D[Z] * (Entry->D[X] - P[X]) - D[X] * (Entry->D[Y] - P[Z]);
							if (fabs(x[0]) > EPSILON)
								y[n++] = -x[1] / x[0];
							break;

						case QUADRATIC_SPLINE :

#ifdef PRISM_EXTRA_STATS
							Thread->Stats()[Prism_Bound_Tests_Succeeded]++;
#endif

							/* Solve quadratic equation. */

							x[0] = Entry->B[X] * D[Z] - Entry->B[Y] * D[X];
							x[1] = Entry->C[X] * D[Z] - Entry->C[Y] * D[X];
							x[2] = D[Z] * (Entry->D[X] - P[X]) - D[X] * (Entry->D[Y] - P[Z]);

							n = Solve_Polynomial(2, x, y, false, 0.0, Thread);
							break;

						case CUBIC_SPLINE :
						case BEZIER_SPLINE :
							if (test_rectangle(P, D, Entry->x1, Entry->y1, Entry->x2, Entry->y2))
							{
#ifdef PRISM_EXTRA_STATS
								Thread->Stats()[Prism_Bound_Tests_Succeeded]++;
#endif

								/* Solve cubic equation. */
								x[0] = Entry->A[X] * D[Z] - Entry->A[Y] * D[X];
								x[1] = Entry->B[X] * D[Z] - Entry->B[Y] * D[X];
								x[2] = Entry->C[X] * D[Z] - Entry->C[Y] * D[X];
								x[3] = D[Z] * (Entry->D[X] - P[X]) - D[X] * (Entry->D[Y] - P[Z]);
								n = Solve_Polynomial(3, x, y, Test_Flag(this, STURM_FLAG), 0.0, Thread);
							}
							break;
					}

					/* Test roots for valid intersections. */
					while (n--)
					{
						w = y[n];

						if ((w >= 0.0) && (w <= 1.0))
						{
							if (fabs(D[X]) > EPSILON)
							{
								k = (w * (w * (w * Entry->A[X] + Entry->B[X]) + Entry->C[X]) + Entry->D[X] - P[X]) / D[X];
							}
							else
							{
								k = (w * (w * (w * Entry->A[Y] + Entry->B[Y]) + Entry->C[Y]) + Entry->D[Y] - P[Z]) / D[Z];
							}

							/* Verify that intersection height is valid. */
							h = P[Y] + k * D[Y];
							if ((h >= Height1) && (h <= Height2))
							{
								distance = k / len;
								if ((distance > DEPTH_TOLERANCE) && (distance < MAX_DISTANCE))
								{
									VEvaluateRay(IPoint, ray.Origin, distance, ray.Direction);
									if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
									{
										Depth_Stack->push (Intersection (distance, IPoint, this, SPLINE_HIT, j, w));
										Found = true;
									}
								}
							}
						}
					}
				}
			}

			break;

		/*************************************************************************
		* Conic sweep.
		**************************************************************************/

		case CONIC_SWEEP :

			if (fabs(D[Y]) < EPSILON)
			{
				if ((P[Y] < Height1) || (P[Y] > Height2))
					return(false);
			}
			else
			{
				if (Test_Flag(this, CLOSED_FLAG))
				{
					/* Intersect ray with the cap-plane. */
					if (fabs(Height2) > EPSILON)
					{
						k = (Height2 - P[Y]) / D[Y];

						if ((k > DEPTH_TOLERANCE) && (k < MAX_DISTANCE))
						{
							u = (P[X] + k * D[X]) / Height2;
							v = (P[Z] + k * D[Z]) / Height2;

							if (in_curve(u, v, Thread))
							{
								distance = k / len;
								if ((distance > DEPTH_TOLERANCE) && (distance < MAX_DISTANCE))
								{
									VEvaluateRay(IPoint, ray.Origin, distance, ray.Direction);
									if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
									{
										Depth_Stack->push (Intersection (distance, IPoint, this, CAP_HIT, 0, 0));
										Found = true;
									}
								}
							}
						}
					}

					/* Intersect ray with the base-plane. */

					if (fabs(Height1) > EPSILON)
					{
						k = (Height1 - P[Y]) / D[Y];

						if ((k > DEPTH_TOLERANCE) && (k < MAX_DISTANCE))
						{
							u = (P[X] + k * D[X]) / Height1;
							v = (P[Z] + k * D[Z]) / Height1;

							if (in_curve(u, v, Thread))
							{
								distance = k / len;
								if ((distance > DEPTH_TOLERANCE) && (distance < MAX_DISTANCE))
								{
									VEvaluateRay(IPoint, ray.Origin, distance, ray.Direction);
									if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
									{
										Depth_Stack->push (Intersection (distance, IPoint, this, BASE_HIT, 0, 0));
										Found = true;
									}
								}
							}
						}
					}
				}
			}

			/* Precompute ray-only dependant constants. */
			k1 = P[Z] * D[Y] - P[Y] * D[Z];
			k2 = P[Y] * D[X] - P[X] * D[Y];
			k3 = P[X] * D[Z] - P[Z] * D[X];

			/* Intersect ray with the spline segments. */
			if ((fabs(D[X]) > EPSILON) || (fabs(D[Z]) > EPSILON))
			{
				Entry = Spline->Entry ;
				for (j = 0; j < Number; j++, Entry++)
				{
					/* Test spline's bounding rectangle (modified Cohen-Sutherland). */
					if (((D[X] >= 0.0) && (P[X] > Entry->x2)) ||
					    ((D[X] <= 0.0) && (P[X] < Entry->x1)) ||
					    ((D[Z] >= 0.0) && (P[Z] > Entry->y2)) ||
					    ((D[Z] <= 0.0) && (P[Z] < Entry->y1)))
					{
						continue;
					}

					/* Number of roots found. */

					n = 0;
					switch (Spline_Type)
					{
						case LINEAR_SPLINE :

							/* Solve linear equation. */
							x[0] = Entry->C[X] * k1 + Entry->C[Y] * k2;
							x[1] = Entry->D[X] * k1 + Entry->D[Y] * k2 + k3;

							if (fabs(x[0]) > EPSILON)
								y[n++] = -x[1] / x[0];
							break;

						case QUADRATIC_SPLINE :

							/* Solve quadratic equation. */
							x[0] = Entry->B[X] * k1 + Entry->B[Y] * k2;
							x[1] = Entry->C[X] * k1 + Entry->C[Y] * k2;
							x[2] = Entry->D[X] * k1 + Entry->D[Y] * k2 + k3;

							n = Solve_Polynomial(2, x, y, false, 0.0, Thread);
							break;

						case CUBIC_SPLINE :
						case BEZIER_SPLINE :

							/* Solve cubic equation. */
							x[0] = Entry->A[X] * k1 + Entry->A[Y] * k2;
							x[1] = Entry->B[X] * k1 + Entry->B[Y] * k2;
							x[2] = Entry->C[X] * k1 + Entry->C[Y] * k2;
							x[3] = Entry->D[X] * k1 + Entry->D[Y] * k2 + k3;

							n = Solve_Polynomial(3, x, y, Test_Flag(this, STURM_FLAG), 0.0, Thread);
							break;
					}

					/* Test roots for valid intersections. */

					while (n--)
					{
						w = y[n];

						if ((w >= 0.0) && (w <= 1.0))
						{
							k = w * (w * (w * Entry->A[X] + Entry->B[X]) + Entry->C[X]) + Entry->D[X];
							h = D[X] - k * D[Y];

							if (fabs(h) > EPSILON)
							{
								k = (k * P[Y] - P[X]) / h;
							}
							else
							{
								k = w * (w * (w * Entry->A[Y] + Entry->B[Y]) + Entry->C[Y]) + Entry->D[Y];

								h = D[Z] - k * D[Y];

								if (fabs(h) > EPSILON)
								{
									k = (k * P[Y] - P[Z]) / h;
								}
								else
								{
									/* This should never happen! */
									continue;
								}
							}

							/* Verify that intersection height is valid. */
							h = P[Y] + k * D[Y];
							if ((h >= Height1) && (h <= Height2))
							{
								distance = k / len;
								if ((distance > DEPTH_TOLERANCE) && (distance < MAX_DISTANCE))
								{
									VEvaluateRay(IPoint, ray.Origin, distance, ray.Direction);
									if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
									{
										Depth_Stack->push (Intersection (distance, IPoint, this, SPLINE_HIT, j, w));
										Found = true;
									}
								}
							}
						}
					}
				}
			}
			break;

		default:
			throw POV_EXCEPTION_STRING("Unknown sweep type in intersect_prism().");
	}

	if (Found)
		Thread->Stats()[Ray_Prism_Tests_Succeeded]++;
	return(Found);
}

/*****************************************************************************
*
* FUNCTION
*
*   Inside_Prism
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
*   Test if point lies inside a prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Prism::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	VECTOR P;

	/* Transform the point into the prism space. */

	MInvTransPoint(P, IPoint, Trans);

	if ((P[Y] >= Height1) && (P[Y] < Height2))
	{
		if (Sweep_Type == CONIC_SWEEP)
		{
			/* Scale x and z coordinate. */

			if (fabs(P[Y]) > EPSILON)
			{
				P[X] /= P[Y];
				P[Z] /= P[Y];
			}
			else
			{
				P[X] = P[Z] = HUGE_VAL;
			}
		}

		if (in_curve(P[X], P[Z], Thread))
		{
			return(!Test_Flag(this, INVERTED_FLAG));
		}
	}

	return(Test_Flag(this, INVERTED_FLAG));
}



/*****************************************************************************
*
* FUNCTION
*
*   Prism_Normal
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
*   Calculate the normal of the prism in a given point.
*
* CHANGES
*
*   May 1994 : Creation.
*
*   Jul 1997 : Fixed bug as reported by Darko Rozic. [DB]
*
******************************************************************************/

void Prism::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	VECTOR P;
	PRISM_SPLINE_ENTRY Entry;
	VECTOR N;

	Make_Vector(N, 0.0, 1.0, 0.0);

	if (Inter->i1 == SPLINE_HIT)
	{
		Entry = Spline->Entry[Inter->i2];

		switch (Sweep_Type)
		{
			case LINEAR_SWEEP:

				N[X] =   Inter->d1 * (3.0 * Entry.A[Y] * Inter->d1 + 2.0 * Entry.B[Y]) + Entry.C[Y];
				N[Y] =   0.0;
				N[Z] = -(Inter->d1 * (3.0 * Entry.A[X] * Inter->d1 + 2.0 * Entry.B[X]) + Entry.C[X]);

				break;

			case CONIC_SWEEP:

				/* Transform the point into the prism space. */

				MInvTransPoint(P, Inter->IPoint, Trans);

				if (fabs(P[Y]) > EPSILON)
				{
					N[X] =   Inter->d1 * (3.0 * Entry.A[Y] * Inter->d1 + 2.0 * Entry.B[Y]) + Entry.C[Y];
					N[Z] = -(Inter->d1 * (3.0 * Entry.A[X] * Inter->d1 + 2.0 * Entry.B[X]) + Entry.C[X]);
					N[Y] = -(P[X] * N[X] + P[Z] * N[Z]) / P[Y];
				}

				break;

			default:

				throw POV_EXCEPTION_STRING("Unknown sweep type in Prism_Normal().");
		}
	}

	/* Transform the normalt out of the prism space. */

	MTransNormal(Result, N, Trans);

	VNormalize(Result, Result);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Prism
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
*   Translate a prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Prism
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
*   Rotate a prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Prism
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
*   Scale a prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Prism
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
*   Transform a prism and recalculate its bounding box.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Transform(const TRANSFORM *tr)
{
	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Prism
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
*   Invert a prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Prism
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   PRISM * - new prism
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Create a new prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

Prism::Prism() : ObjectBase(PRISM_OBJECT)
{
	Trans = Create_Transform();

	x1      = 0.0;
	x2      = 0.0;
	y1      = 0.0;
	y2      = 0.0;
	u1      = 0.0;
	u2      = 0.0;
	v1      = 0.0;
	v2      = 0.0;
	Height1 = 0.0;
	Height2 = 0.0;

	Number = 0;

	Spline_Type = LINEAR_SPLINE;
	Sweep_Type  = LINEAR_SWEEP;

	Spline = NULL;

	Set_Flag(this, CLOSED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Prism
*
* INPUT
*
*   Object - Object
*   
* OUTPUT
*   
* RETURNS
*
*   void * - New prism
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Copy a prism structure.
*
*   NOTE: The splines are not copied, only the number of references is
*         counted, so that Destray_Prism() knows if they can be destroyed.
*
* CHANGES
*
*   May 1994 : Creation.
*
*   Sep 1994 : fixed memory leakage [DB]
*
******************************************************************************/

ObjectPtr Prism::Copy()
{
	Prism *New = new Prism();
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
*   Destroy_Prism
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
*   Destroy a prism.
*
*   NOTE: The splines are destroyed if they are no longer used by any copy.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

Prism::~Prism()
{
	Destroy_Transform(Trans);

	if (--(Spline->References) == 0)
	{
		POV_FREE(Spline->Entry);
		POV_FREE(Spline);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Prism_BBox
*
* INPUT
*
*   Prism - Prism
*   
* OUTPUT
*
*   Prism
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a prism.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Compute_BBox()
{
	Make_BBox(BBox, x1, Height1, y1,
		x2 - x1, Height2 - Height1, y2 - y1);

	Recompute_BBox(&BBox, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   in_curve
*
* INPUT
*
*   Prism - Prism to test
*   u, v  - Coordinates
*   
* OUTPUT
*   
* RETURNS
*
*   int - true if inside
*   
* AUTHOR
*
*   Dieter Bayer, June 1994
*   
* DESCRIPTION
*
*   Test if a 2d point lies inside a prism's spline curve.
*
*   We travel from the current point in positive u direction
*   and count the number of crossings with the curve.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

int Prism::in_curve(DBL u, DBL v, TraceThreadData *Thread) const
{
	int i, n, NC;
	DBL k, w;
	DBL x[4];
	DBL y[3];
	PRISM_SPLINE_ENTRY Entry;

	NC = 0;

	/* First test overall bounding rectangle. */

	if ((u >= u1) && (u <= u2) &&
	    (v >= v1) && (v <= v2))
	{
		for (i = 0; i < Number; i++)
		{
			Entry = Spline->Entry[i];

			/* Test if current segment can be hit. */

			if ((v >= Entry.v1) && (v <= Entry.v2) && (u <= Entry.u2))
			{
				x[0] = Entry.A[Y];
				x[1] = Entry.B[Y];
				x[2] = Entry.C[Y];
				x[3] = Entry.D[Y] - v;

				n = Solve_Polynomial(3, x, y, Test_Flag(this, STURM_FLAG), 0.0, Thread);

				while (n--)
				{
					w = y[n];

					if ((w >= 0.0) && (w <= 1.0))
					{
						k  = w * (w * (w * Entry.A[X] + Entry.B[X]) + Entry.C[X]) + Entry.D[X] - u;

						if (k >= 0.0)
						{
							NC++;
						}
					}
				}
			}
		}
	}

	return(NC & 1);
}



/*****************************************************************************
*
* FUNCTION
*
*   test_rectangle
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer, July 1994
*   
* DESCRIPTION
*
*   Test if the 2d ray (= P + t * D) intersects a rectangle.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

bool Prism::test_rectangle(const VECTOR P, const VECTOR D, DBL x1, DBL z1, DBL x2, DBL z2)
{
	DBL dmin, dmax, tmin, tmax;

	if (fabs(D[X]) > EPSILON)
	{
		if (D[X] > 0.0)
		{
			dmin = (x1 - P[X]) / D[X];
			dmax = (x2 - P[X]) / D[X];

			if (dmax < EPSILON)
			{
				return(false);
			}
		}
		else
		{
			dmax = (x1 - P[X]) / D[X];

			if (dmax < EPSILON)
			{
				return(false);
			}

			dmin = (x2 - P[X]) / D[X];
		}

		if (dmin > dmax)
		{
			return(false);
		}
	}
	else
	{
		if ((P[X] < x1) || (P[X] > x2))
		{
			return(false);
		}
		else
		{
			dmin = -BOUND_HUGE;
			dmax =  BOUND_HUGE;
		}
	}

	if (fabs(D[Z]) > EPSILON)
	{
		if (D[Z] > 0.0)
		{
			tmin = (z1 - P[Z]) / D[Z];
			tmax = (z2 - P[Z]) / D[Z];
		}
		else
		{
			tmax = (z1 - P[Z]) / D[Z];
			tmin = (z2 - P[Z]) / D[Z];
		}

		if (tmax < dmax)
		{
			if (tmax < EPSILON)
			{
				return(false);
			}

			if (tmin > dmin)
			{
				if (tmin > tmax)
				{
					return(false);
				}

				dmin = tmin;
			}
			else
			{
				if (dmin > tmax)
				{
					return(false);
				}
			}

			/*dmax = tmax; */ /*not needed CEY[1/95]*/
		}
		else
		{
			if (tmin > dmin)
			{
				if (tmin > dmax)
				{
					return(false);
				}

				/* dmin = tmin; */  /*not needed CEY[1/95]*/
			}
		}
	}
	else
	{
		if ((P[Z] < z1) || (P[Z] > z2))
		{
			return(false);
		}
	}

	return(true);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Prism
*
* INPUT
*
*   Prism - Prism
*   P     - Points defining prism
*   
* OUTPUT
*
*   Prism
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer, June 1994
*
* DESCRIPTION
*
*   Calculate the spline segments of a prism from a set of points.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

void Prism::Compute_Prism(UV_VECT *P, TraceThreadData *Thread)
{
	int i, n, number_of_splines;
	int i1, i2, i3;

	DBL x[4], y[4];
	DBL xmin, xmax, ymin, ymax;
	DBL c[3];
	DBL r[2];

	UV_VECT A, B, C, D, First;

	/* Allocate Object->Number segments. */

	if (Spline == NULL)
	{
		Spline = (PRISM_SPLINE *)POV_MALLOC(sizeof(PRISM_SPLINE), "spline segments of prism");
		Spline->References = 1;
		Spline->Entry = (PRISM_SPLINE_ENTRY *)POV_MALLOC(Number*sizeof(PRISM_SPLINE_ENTRY), "spline segments of prism");
	}
	else
	{
		/* This should never happen! */
		throw POV_EXCEPTION_STRING("Prism segments are already defined.");
	}

	/***************************************************************************
  * Calculate segments.
  ****************************************************************************/

	/* We want to know the size of the overall bounding rectangle. */

	xmax = ymax = -BOUND_HUGE;
	xmin = ymin =  BOUND_HUGE;

	/* Get first segment point. */

	switch (Spline_Type)
	{
		case LINEAR_SPLINE:

			Assign_UV_Vect(First, P[0]);

			break;

		case QUADRATIC_SPLINE:
		case CUBIC_SPLINE:

			Assign_UV_Vect(First, P[1]);

			break;
	}

	for (i = number_of_splines = 0; i < Number-1; i++)
	{
		/* Get indices of previous and next two points. */

		i1 = i + 1;
		i2 = i + 2;
		i3 = i + 3;

		switch (Spline_Type)
		{
			/*************************************************************************
			* Linear spline (nothing more than a simple polygon).
			**************************************************************************/

			case LINEAR_SPLINE :

				if (i1 >= Number)
				{
					throw POV_EXCEPTION_STRING("Too few points in prism. Prism not closed? Control points missing?");
				}

				/* Use linear interpolation. */

				A[X] =  0.0;
				B[X] =  0.0;
				C[X] = -1.0 * P[i][X] + 1.0 * P[i1][X];
				D[X] =  1.0 * P[i][X];

				A[Y] =  0.0;
				B[Y] =  0.0;
				C[Y] = -1.0 * P[i][Y] + 1.0 * P[i1][Y];
				D[Y] =  1.0 * P[i][Y];

				x[0] = x[2] = P[i][X];
				x[1] = x[3] = P[i1][X];

				y[0] = y[2] = P[i][Y];
				y[1] = y[3] = P[i1][Y];

				break;

			/*************************************************************************
			* Quadratic spline.
			**************************************************************************/

			case QUADRATIC_SPLINE :

				if (i2 >= Number)
				{
					throw POV_EXCEPTION_STRING("Too few points in prism.");
				}

				/* Use quadratic interpolation. */

				A[X] =  0.0;
				B[X] =  0.5 * P[i][X] - 1.0 * P[i1][X] + 0.5 * P[i2][X];
				C[X] = -0.5 * P[i][X]                  + 0.5 * P[i2][X];
				D[X] =                  1.0 * P[i1][X];

				A[Y] =  0.0;
				B[Y] =  0.5 * P[i][Y] - 1.0 * P[i1][Y] + 0.5 * P[i2][Y];
				C[Y] = -0.5 * P[i][Y]                  + 0.5 * P[i2][Y];
				D[Y] =                  1.0 * P[i1][Y];

				x[0] = x[2] = P[i1][X];
				x[1] = x[3] = P[i2][X];

				y[0] = y[2] = P[i1][Y];
				y[1] = y[3] = P[i2][Y];

				break;

			/*************************************************************************
			* Cubic spline.
			**************************************************************************/

			case CUBIC_SPLINE :

				if (i3 >= Number)
				{
					throw POV_EXCEPTION_STRING("Too few points in prism.");
				}

				/* Use cubic interpolation. */

				A[X] = -0.5 * P[i][X] + 1.5 * P[i1][X] - 1.5 * P[i2][X] + 0.5 * P[i3][X];
				B[X] =        P[i][X] - 2.5 * P[i1][X] + 2.0 * P[i2][X] - 0.5 * P[i3][X];
				C[X] = -0.5 * P[i][X]                  + 0.5 * P[i2][X];
				D[X] =                        P[i1][X];

				A[Y] = -0.5 * P[i][Y] + 1.5 * P[i1][Y] - 1.5 * P[i2][Y] + 0.5 * P[i3][Y];
				B[Y] =        P[i][Y] - 2.5 * P[i1][Y] + 2.0 * P[i2][Y] - 0.5 * P[i3][Y];
				C[Y] = -0.5 * P[i][Y]                  + 0.5 * P[i2][Y];
				D[Y] =                        P[i1][Y];

				x[0] = x[2] = P[i1][X];
				x[1] = x[3] = P[i2][X];

				y[0] = y[2] = P[i1][Y];
				y[1] = y[3] = P[i2][Y];

				break;

			/*************************************************************************
			* Bezier spline.
			**************************************************************************/

			case BEZIER_SPLINE :

				if (i3 >= Number)
				{
					throw POV_EXCEPTION_STRING("Too few points in prism. Prism not closed? Control points missing?");
				}

				/* Use Bernstein blending function interpolation. */

				A[X] = P[i][X] - 3.0 * P[i1][X] + 3.0 * P[i2][X] -       P[i3][X];
				B[X] =           3.0 * P[i1][X] - 6.0 * P[i2][X] + 3.0 * P[i3][X];
				C[X] =           3.0 * P[i2][X] - 3.0 * P[i3][X];
				D[X] =                                                   P[i3][X];

				A[Y] = P[i][Y] - 3.0 * P[i1][Y] + 3.0 * P[i2][Y] -       P[i3][Y];
				B[Y] =           3.0 * P[i1][Y] - 6.0 * P[i2][Y] + 3.0 * P[i3][Y];
				C[Y] =           3.0 * P[i2][Y] - 3.0 * P[i3][Y];
				D[Y] =                                                   P[i3][Y];

				x[0] = P[i][X];
				x[1] = P[i1][X];
				x[2] = P[i2][X];
				x[3] = P[i3][X];

				y[0] = P[i][Y];
				y[1] = P[i1][Y];
				y[2] = P[i2][Y];
				y[3] = P[i3][Y];

				break;

			default:

				throw POV_EXCEPTION_STRING("Unknown spline type in Compute_Prism().");
		}

		Assign_UV_Vect(Spline->Entry[number_of_splines].A, A);
		Assign_UV_Vect(Spline->Entry[number_of_splines].B, B);
		Assign_UV_Vect(Spline->Entry[number_of_splines].C, C);
		Assign_UV_Vect(Spline->Entry[number_of_splines].D, D);

		if ((Spline_Type == QUADRATIC_SPLINE) ||
		    (Spline_Type == CUBIC_SPLINE))
		{
			/* Get maximum coordinates in current segment. */

			c[0] = 3.0 * A[X];
			c[1] = 2.0 * B[X];
			c[2] = C[X];

			n = Solve_Polynomial(2, c, r, false, 0.0, Thread);

			while (n--)
			{
				if ((r[n] >= 0.0) && (r[n] <= 1.0))
				{
					x[n] = r[n] * (r[n] * (r[n] * A[X] + B[X]) + C[X]) + D[X];
				}
			}

			c[0] = 3.0 * A[Y];
			c[1] = 2.0 * B[Y];
			c[2] = C[Y];

			n = Solve_Polynomial(2, c, r, false, 0.0, Thread);

			while (n--)
			{
				if ((r[n] >= 0.0) && (r[n] <= 1.0))
				{
					y[n] = r[n] * (r[n] * (r[n] * A[Y] + B[Y]) + C[Y]) + D[Y];
				}
			}
		}

		/* Set current segment's bounding rectangle. */

		Spline->Entry[number_of_splines].x1 = min(min(x[0], x[1]), min(x[2], x[3]));

		Spline->Entry[number_of_splines].x2 =
		Spline->Entry[number_of_splines].u2 = max(max(x[0], x[1]), max(x[2], x[3]));

		Spline->Entry[number_of_splines].y1 =
		Spline->Entry[number_of_splines].v1 = min(min(y[0], y[1]), min(y[2], y[3]));

		Spline->Entry[number_of_splines].y2 =
		Spline->Entry[number_of_splines].v2 = max(max(y[0], y[1]), max(y[2], y[3]));

		/* Keep track of overall bounding rectangle. */

		xmin = min(xmin, Spline->Entry[number_of_splines].x1);
		xmax = max(xmax, Spline->Entry[number_of_splines].x2);

		ymin = min(ymin, Spline->Entry[number_of_splines].y1);
		ymax = max(ymax, Spline->Entry[number_of_splines].y2);

		number_of_splines++;

		/* Advance to next segment. */

		switch (Spline_Type)
		{
			case LINEAR_SPLINE:

				if ((fabs(P[i1][X] - First[X]) < EPSILON) &&
				    (fabs(P[i1][Y] - First[Y]) < EPSILON))
				{
					i++;

					if (i+1 < Number)
					{
						Assign_UV_Vect(First, P[i+1]);
					}
				}

				break;

			case QUADRATIC_SPLINE:

				if ((fabs(P[i2][X] - First[X]) < EPSILON) &&
				    (fabs(P[i2][Y] - First[Y]) < EPSILON))
				{
					i += 2;

					if (i+2 < Number)
					{
						Assign_UV_Vect(First, P[i+2]);
					}
				}

				break;

			case CUBIC_SPLINE:

				if ((fabs(P[i2][X] - First[X]) < EPSILON) &&
				    (fabs(P[i2][Y] - First[Y]) < EPSILON))
				{
					i += 3;

					if (i+2 < Number)
					{
						Assign_UV_Vect(First, P[i+2]);
					}
				}

				break;

			case BEZIER_SPLINE:

				i += 3;

				break;
		}
	}

	Number = number_of_splines;

	/* Set overall bounding rectangle. */

	x1 =
	u1 = xmin;
	x2 =
	u2 = xmax;

	y1 =
	v1 = ymin;
	y2 =
	v2 = ymax;

	if (Sweep_Type == CONIC_SWEEP)
	{
		/* Recalculate bounding rectangles. */

		for (i = 0; i < Number; i++)
		{
			x[0] = Spline->Entry[i].x1 * Height1;
			x[1] = Spline->Entry[i].x1 * Height2;
			x[2] = Spline->Entry[i].x2 * Height1;
			x[3] = Spline->Entry[i].x2 * Height2;

			xmin = min(min(x[0], x[1]), min(x[2], x[3]));
			xmax = max(max(x[0], x[1]), max(x[2], x[3]));

			Spline->Entry[i].x1 = xmin;
			Spline->Entry[i].x2 = xmax;

			y[0] = Spline->Entry[i].y1 * Height1;
			y[1] = Spline->Entry[i].y1 * Height2;
			y[2] = Spline->Entry[i].y2 * Height1;
			y[3] = Spline->Entry[i].y2 * Height2;

			ymin = min(min(y[0], y[1]), min(y[2], y[3]));
			ymax = max(max(y[0], y[1]), max(y[2], y[3]));

			Spline->Entry[i].y1 = ymin;
			Spline->Entry[i].y2 = ymax;
		}

		/* Recalculate overall bounding rectangle. */

		x[0] = x1 * Height1;
		x[1] = x1 * Height2;
		x[2] = x2 * Height1;
		x[3] = x2 * Height2;

		xmin = min(min(x[0], x[1]), min(x[2], x[3]));
		xmax = max(max(x[0], x[1]), max(x[2], x[3]));

		x1 = xmin;
		x2 = xmax;

		y[0] = y1 * Height1;
		y[1] = y1 * Height2;
		y[2] = y2 * Height1;
		y[3] = y2 * Height2;

		ymin = min(min(y[0], y[1]), min(y[2], y[3]));
		ymax = max(max(y[0], y[1]), max(y[2], y[3]));

		y1 = ymin;
		y2 = ymax;
	}
}

}
