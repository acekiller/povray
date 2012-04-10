/*******************************************************************************
 * boxes.cpp
 *
 * This module implements the box primitive.
 * This file was written by Alexander Enzmann.  He wrote the code for
 * boxes and generously provided us these enhancements.
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
 * $File: //depot/povray/smp/source/backend/shape/boxes.cpp $
 * $Revision: #30 $
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
#include "backend/shape/boxes.h"
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

/* Minimal intersection depth. */

const DBL DEPTH_TOLERANCE = 1.0e-6;

/* Two values are equal if their difference is small than CLOSE_TOLERANCE. */

const DBL CLOSE_TOLERANCE = 1.0e-6;

/* Side hit. */

const int SIDE_X_0 = 1;
const int SIDE_X_1 = 2;
const int SIDE_Y_0 = 3;
const int SIDE_Y_1 = 4;
const int SIDE_Z_0 = 5;
const int SIDE_Z_1 = 6;



/*****************************************************************************
*
* FUNCTION
*
*   All_Box_Intersections
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

bool Box::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int Intersection_Found;
	int Side1, Side2;
	DBL Depth1, Depth2;
	VECTOR IPoint;

	Thread->Stats()[Ray_Box_Tests]++;

	Intersection_Found = false;

	if (Intersect(ray, Trans, bounds[0], bounds[1], &Depth1, &Depth2, &Side1, &Side2))
	{
		if (Depth1 > DEPTH_TOLERANCE)
		{
			VEvaluateRay(IPoint, ray.Origin, Depth1, ray.Direction);

			if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
			{
				Depth_Stack->push(Intersection(Depth1,IPoint,this,Side1));

				Intersection_Found = true;
			}
		}

		VEvaluateRay(IPoint, ray.Origin, Depth2, ray.Direction);

		if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
		{
			Depth_Stack->push(Intersection(Depth2,IPoint,this,Side2));

			Intersection_Found = true;
		}
	}

	if (Intersection_Found)
		Thread->Stats()[Ray_Box_Tests_Succeeded]++;

	return (Intersection_Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_Box
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
*   Sep 1994 : Added code to decide which side was hit in the case
*              intersection points are close to each other. This removes
*              some ugly artefacts one could observe at the corners of
*              boxes due to the usage of the wrong normal vector. [DB]
*
******************************************************************************/

bool Box::Intersect(const Ray& ray, const TRANSFORM *Trans, const VECTOR Corner1, const VECTOR Corner2, DBL *Depth1, DBL  *Depth2, int *Side1, int  *Side2)
{
	int smin = 0, smax = 0;    /* Side hit for min/max intersection. */
	DBL t, tmin, tmax;
	VECTOR P, D;

	/* Transform the point into the boxes space */

	if (Trans != NULL)
	{
		MInvTransPoint(P, ray.Origin, Trans);
		MInvTransDirection(D, ray.Direction, Trans);
	}
	else
	{
		Assign_Vector(P, ray.Origin);
		Assign_Vector(D, ray.Direction);
	}

	tmin = 0.0;
	tmax = BOUND_HUGE;

	/*
	 * Sides first.
	 */

	if (D[X] < -EPSILON)
	{
		t = (Corner1[X] - P[X]) / D[X];

		if (t < tmin) return(false);

		if (t <= tmax)
		{
			smax = SIDE_X_0;
			tmax = t;
		}

		t = (Corner2[X] - P[X]) / D[X];

		if (t >= tmin)
		{
			if (t > tmax) return(false);

			smin = SIDE_X_1;
			tmin = t;
		}
	}
	else
	{
		if (D[X] > EPSILON)
		{
			t = (Corner2[X] - P[X]) / D[X];

			if (t < tmin) return(false);

			if (t <= tmax)
			{
				smax = SIDE_X_1;
				tmax = t;
			}

			t = (Corner1[X] - P[X]) / D[X];

			if (t >= tmin)
			{
				if (t > tmax) return(false);

				smin = SIDE_X_0;
				tmin = t;
			}
		}
		else
		{
			if ((P[X] < Corner1[X]) || (P[X] > Corner2[X]))
			{
				return(false);
			}
		}
	}

	/*
	 * Check Top/Bottom.
	 */

	if (D[Y] < -EPSILON)
	{
		t = (Corner1[Y] - P[Y]) / D[Y];

		if (t < tmin) return(false);

		if (t <= tmax - CLOSE_TOLERANCE)
		{
			smax = SIDE_Y_0;
			tmax = t;
		}
		else
		{
			/*
			 * If intersection points are close to each other find out
			 * which side to use, i.e. is most probably hit. [DB 9/94]
			 */

			if (t <= tmax + CLOSE_TOLERANCE)
			{
				if (-D[Y] > fabs(D[X])) smax = SIDE_Y_0;
			}
		}

		t = (Corner2[Y] - P[Y]) / D[Y];

		if (t >= tmin + CLOSE_TOLERANCE)
		{
			if (t > tmax) return(false);

			smin = SIDE_Y_1;
			tmin = t;
		}
		else
		{
			/*
			 * If intersection points are close to each other find out
			 * which side to use, i.e. is most probably hit. [DB 9/94]
			 */

			if (t >= tmin - CLOSE_TOLERANCE)
			{
				if (-D[Y] > fabs(D[X])) smin = SIDE_Y_1;
			}
		}
	}
	else
	{
		if (D[Y] > EPSILON)
		{
			t = (Corner2[Y] - P[Y]) / D[Y];

			if (t < tmin) return(false);

			if (t <= tmax - CLOSE_TOLERANCE)
			{
				smax = SIDE_Y_1;
				tmax = t;
			}
			else
			{
				/*
				 * If intersection points are close to each other find out
				 * which side to use, i.e. is most probably hit. [DB 9/94]
				 */

				if (t <= tmax + CLOSE_TOLERANCE)
				{
					if (D[Y] > fabs(D[X])) smax = SIDE_Y_1;
				}
			}

			t = (Corner1[Y] - P[Y]) / D[Y];

			if (t >= tmin + CLOSE_TOLERANCE)
			{
				if (t > tmax) return(false);

				smin = SIDE_Y_0;
				tmin = t;
			}
			else
			{
				/*
				 * If intersection points are close to each other find out
				 * which side to use, i.e. is most probably hit. [DB 9/94]
				 */

				if (t >= tmin - CLOSE_TOLERANCE)
				{
					if (D[Y] > fabs(D[X])) smin = SIDE_Y_0;
				}
			}
		}
		else
		{
			if ((P[Y] < Corner1[Y]) || (P[Y] > Corner2[Y]))
			{
				return(false);
			}
		}
	}

	/* Now front/back */

	if (D[Z] < -EPSILON)
	{
		t = (Corner1[Z] - P[Z]) / D[Z];

		if (t < tmin) return(false);

		if (t <= tmax - CLOSE_TOLERANCE)
		{
			smax = SIDE_Z_0;
			tmax = t;
		}
		else
		{
			/*
			 * If intersection points are close to each other find out
			 * which side to use, i.e. is most probably hit. [DB 9/94]
			 */

			if (t <= tmax + CLOSE_TOLERANCE)
			{
				switch (smax)
				{
					case SIDE_X_0 :
					case SIDE_X_1 : if (-D[Z] > fabs(D[X])) smax = SIDE_Z_0; break;

					case SIDE_Y_0 :
					case SIDE_Y_1 : if (-D[Z] > fabs(D[Y])) smax = SIDE_Z_0; break;
				}
			}
		}

		t = (Corner2[Z] - P[Z]) / D[Z];

		if (t >= tmin + CLOSE_TOLERANCE)
		{
			if (t > tmax) return(false);

			smin = SIDE_Z_1;
			tmin = t;
		}
		else
		{
			/*
			 * If intersection points are close to each other find out
			 * which side to use, i.e. is most probably hit. [DB 9/94]
			 */

			if (t >= tmin - CLOSE_TOLERANCE)
			{
				switch (smin)
				{
					case SIDE_X_0 :
					case SIDE_X_1 : if (-D[Z] > fabs(D[X])) smin = SIDE_Z_1; break;

					case SIDE_Y_0 :
					case SIDE_Y_1 : if (-D[Z] > fabs(D[Y])) smin = SIDE_Z_1; break;
				}
			}
		}
	}
	else
	{
		if (D[Z] > EPSILON)
		{
			t = (Corner2[Z] - P[Z]) / D[Z];

			if (t < tmin) return(false);

			if (t <= tmax - CLOSE_TOLERANCE)
			{
				smax = SIDE_Z_1;
				tmax = t;
			}
			else
			{
				/*
				 * If intersection points are close to each other find out
				 * which side to use, i.e. is most probably hit. [DB 9/94]
				 */

				if (t <= tmax + CLOSE_TOLERANCE)
				{
					switch (smax)
					{
						case SIDE_X_0 :
						case SIDE_X_1 : if (D[Z] > fabs(D[X])) smax = SIDE_Z_1; break;

						case SIDE_Y_0 :
						case SIDE_Y_1 : if (D[Z] > fabs(D[Y])) smax = SIDE_Z_1; break;
					}
				}
			}

			t = (Corner1[Z] - P[Z]) / D[Z];

			if (t >= tmin + CLOSE_TOLERANCE)
			{
				if (t > tmax) return(false);

				smin = SIDE_Z_0;
				tmin = t;
			}
			else
			{
				/*
				 * If intersection points are close to each other find out
				 * which side to use, i.e. is most probably hit. [DB 9/94]
				 */

				if (t >= tmin - CLOSE_TOLERANCE)
				{
					switch (smin)
					{
						case SIDE_X_0 :
						case SIDE_X_1 : if (D[Z] > fabs(D[X])) smin = SIDE_Z_0; break;

						case SIDE_Y_0 :
						case SIDE_Y_1 : if (D[Z] > fabs(D[Y])) smin = SIDE_Z_0; break;
					}
				}
			}
		}
		else
		{
			if ((P[Z] < Corner1[Z]) || (P[Z] > Corner2[Z]))
			{
				return(false);
			}
		}
	}

	if (tmax < DEPTH_TOLERANCE)
	{
		return (false);
	}

	*Depth1 = tmin;
	*Depth2 = tmax;

	*Side1 = smin;
	*Side2 = smax;

	return(true);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Box
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

bool Box::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	VECTOR New_Point;

	/* Transform the point into box space. */

	if (Trans != NULL)
	{
		MInvTransPoint(New_Point, IPoint, Trans);
	}
	else
	{
		Assign_Vector(New_Point,IPoint);
	}

	/* Test to see if we are outside the box. */

	if ((New_Point[X] < bounds[0][X]) || (New_Point[X] > bounds[1][X]))
	{
		return (Test_Flag(this, INVERTED_FLAG));
	}

	if ((New_Point[Y] < bounds[0][Y]) || (New_Point[Y] > bounds[1][Y]))
	{
		return (Test_Flag(this, INVERTED_FLAG));
	}

	if ((New_Point[Z] < bounds[0][Z]) || (New_Point[Z] > bounds[1][Z]))
	{
		return (Test_Flag(this, INVERTED_FLAG));
	}

	/* Inside the box. */

	return (!Test_Flag(this, INVERTED_FLAG));
}



/*****************************************************************************
*
* FUNCTION
*
*   Box_Normal
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

void Box::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	switch (Inter->i1)
	{
		case SIDE_X_0: Make_Vector(Result, -1.0,  0.0,  0.0); break;
		case SIDE_X_1: Make_Vector(Result,  1.0,  0.0,  0.0); break;
		case SIDE_Y_0: Make_Vector(Result,  0.0, -1.0,  0.0); break;
		case SIDE_Y_1: Make_Vector(Result,  0.0,  1.0,  0.0); break;
		case SIDE_Z_0: Make_Vector(Result,  0.0,  0.0, -1.0); break;
		case SIDE_Z_1: Make_Vector(Result,  0.0,  0.0,  1.0); break;

		default: throw POV_EXCEPTION_STRING("Unknown box side in Box_Normal().");
	}

	/* Transform the point into the boxes space. */

	if (Trans != NULL)
	{
		MTransNormal(Result, Result, Trans);

		VNormalize(Result, Result);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Box
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

void Box::Translate(const VECTOR Vector, const TRANSFORM *tr)
{
	if (Trans == NULL)
	{
		VAddEq(bounds[0], Vector);

		VAddEq(bounds[1], Vector);

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
*   Rotate_Box
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

void Box::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Box
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

void Box::Scale(const VECTOR Vector, const TRANSFORM *tr)
{
	DBL temp;

	if (Trans == NULL)
	{
		VEvaluateEq(bounds[0], Vector);
		VEvaluateEq(bounds[1], Vector);

		if (bounds[0][X] > bounds[1][X])
		{
			temp = bounds[0][X];

			bounds[0][X] = bounds[1][X];
			bounds[1][X] = temp;
		}

		if (bounds[0][Y] > bounds[1][Y])
		{
			temp = bounds[0][Y];

			bounds[0][Y] = bounds[1][Y];
			bounds[1][Y] = temp;
		}

		if (bounds[0][Z] > bounds[1][Z])
		{
			temp = bounds[0][Z];

			bounds[0][Z] = bounds[1][Z];
			bounds[1][Z] = temp;
		}

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
*   Invert_Box
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

void Box::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Box
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

void Box::Transform(const TRANSFORM *tr)
{
	if(Trans == NULL)
		Trans = Create_Transform();

	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Box
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

Box::Box() : ObjectBase(BOX_OBJECT)
{
	Make_Vector(bounds[0], -1.0, -1.0, -1.0);
	Make_Vector(bounds[1],  1.0,  1.0,  1.0);

	Make_BBox(BBox, -1.0, -1.0, -1.0, 2.0, 2.0, 2.0);

	Trans = NULL;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Box
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

ObjectPtr Box::Copy()
{
	Box *New = new Box();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Box
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

Box::~Box()
{
	Destroy_Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Box_BBox
*
* INPUT
*
*   Box - Box
*
* OUTPUT
*
*   Box
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate the bounding box of a box.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void Box::Compute_BBox()
{
	Assign_BBox_Vect(BBox.Lower_Left, bounds[0]);

	VSub(BBox.Lengths, bounds[1], bounds[0]);

	if (Trans != NULL)
	{
		Recompute_BBox(&BBox, Trans);
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Box_UVCoord
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Nathan Kopp, Lutz Kretzschmar
*
* DESCRIPTION
*
*        +-----+
*        ^  4  |
*        z     |
*  +-----+--x>-#--z>-+-<x--+
*  |     ^     |     |     |
*  |  1  y  5  |  2  |  6  |
*  |     |     |     |     |
*  +-----O--x>-+-----+-----+
*        |     |
*        |  3  |
*        +-----+
*
*  planes:
*  1: min x   2: max x
*  3: min y   4: max y
*  5: min z   6: max z
*
*  O : Origin
*  # : <1,1,0>
*
* CHANGES
*
*   The code was changed to use somthing similar to environmental cube mappping
*
*   1        +-----+           #
*            |     |
* V          z  4  |
*            |     |
*  .6  +--z>-+--x>-+-<z--+-<x--+
*      |     ^     |     |     |
*      |  1  y  5  |  2  |  6  |
*      |     |     |     |     |
*  .3  +-----+--x>-+-----+-----+
*            ^     |
*            z  3  |
*            |     |
*  0   O     +-----+
*  
*      0    .25    .5   .75    1
*                            U
*	
*  planes:
*  1: min x   2: max x
*  3: min y   4: max y
*  5: max z   6: min z
*
*  O : Origin of U,V map
*  # : <1,1,0>
*
******************************************************************************/

void Box::UVCoord(UV_VECT Result, const Intersection *Inter, TraceThreadData *Thread) const
{
	VECTOR P, Box_Diff;

	/* Transform the point into the cube's space */
	if (Trans != NULL)
		MInvTransPoint(P, Inter->IPoint, Trans);
	else
		Assign_Vector(P, Inter->IPoint);

	VSub(Box_Diff,bounds[1],bounds[0]);

	/* this line moves the bottom,left,front corner of the box to <0,0,0> */
	VSubEq(P, bounds[0]);
	/* this line normalizes the face offsets */
	VDivEq(P, Box_Diff);

	/* if no normalize above, then we should use Box->UV_Trans and also
	   inverse-transform the bounds */

	/* The following code does a variation of cube environment mapping. All the
	   textures are not mirrored when the cube is viewed from outside. */

	switch (Inter->i1)
	{
		case SIDE_X_0:
			Result[U] =               (P[Z] / 4.0);
			Result[V] = (1.0 / 3.0) + (P[Y] / 3.0);
			break;
		case SIDE_X_1:
			Result[U] = (3.0 / 4.0) - (P[Z] / 4.0);
			Result[V] = (1.0 / 3.0) + (P[Y] / 3.0);
			break;
		case SIDE_Y_0:
			Result[U] = (1.0 / 4.0) + (P[X] / 4.0);
			Result[V] =               (P[Z] / 3.0);
			break;
		case SIDE_Y_1:
			Result[U] = (1.0 / 4.0) + (P[X] / 4.0);
			Result[V] = (3.0 / 3.0) - (P[Z] / 3.0);
			break;
		case SIDE_Z_0:
			Result[U] =  1.0        - (P[X] / 4.0);
			Result[V] = (1.0 / 3.0) + (P[Y] / 3.0);
			break;
		case SIDE_Z_1:
			Result[U] = (1.0 / 4.0) + (P[X] / 4.0);
			Result[V] = (1.0 / 3.0) + (P[Y] / 3.0);
			break;

		default: throw POV_EXCEPTION_STRING("Unknown box side in Box_Normal().");
	}
}

bool Box::Intersect_BBox(BBoxDirection, const BBOX_VECT&, const BBOX_VECT&, BBOX_VAL) const
{
	return true;
}

}
