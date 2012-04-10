/*******************************************************************************
 * discs.cpp
 *
 * This module implements the disc primitive.
 * This file was written by Alexander Enzmann.  He wrote the code for
 * discs and generously provided us these enhancements.
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
 * $File: //depot/povray/smp/source/backend/shape/discs.cpp $
 * $Revision: #26 $
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
#include "backend/shape/discs.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/scene/threaddata.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

const DBL DEPTH_TOLERANCE = 1.0e-6;

/*****************************************************************************
*
* FUNCTION
*
*   All_Disc_Intersections
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

bool Disc::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int Intersection_Found;
	DBL Depth;
	VECTOR IPoint;

	Intersection_Found = false;

	Thread->Stats()[Ray_Disc_Tests]++;
	if (Intersect(ray, &Depth))
	{
		Thread->Stats()[Ray_Disc_Tests_Succeeded]++;
		VEvaluateRay(IPoint, ray.Origin, Depth, ray.Direction);

		if (Clip.empty() || Point_In_Clip (IPoint, Clip, Thread))
		{
			Depth_Stack->push(Intersection(Depth,IPoint,this));
			Intersection_Found = true;
		}
	}

	return (Intersection_Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_Disc
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

bool Disc::Intersect(const Ray& ray, DBL *Depth) const
{
	DBL t, u, v, r2, len;
	VECTOR P, D;

	/* Transform the point into the discs space */

	MInvTransPoint(P, ray.Origin, Trans);
	MInvTransDirection(D, ray.Direction, Trans);

	VLength(len, D);
	VInverseScaleEq(D, len);

	if (fabs(D[Z]) > EPSILON)
	{
		t = -P[Z] / D[Z];

		if (t >= 0.0)
		{
			u = P[X] + t * D[X];
			v = P[Y] + t * D[Y];

			r2 = Sqr(u) + Sqr(v);

			if ((r2 >= iradius2) && (r2 <= oradius2))
			{
				*Depth = t / len;

				if ((*Depth > DEPTH_TOLERANCE) && (*Depth < MAX_DISTANCE))
					return (true);
			}
		}
	}

	return (false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Disc
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

bool Disc::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	VECTOR New_Point;

	/* Transform the point into the discs space */

	MInvTransPoint(New_Point, IPoint, Trans);

	if (New_Point[Z] >= 0.0)
	{
		/* We are outside. */

		return (Test_Flag(this, INVERTED_FLAG));
	}
	else
	{
		/* We are inside. */

		return (!Test_Flag(this, INVERTED_FLAG));
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Disc_Normal
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

void Disc::Normal (VECTOR Result, Intersection *, TraceThreadData *) const
{
	Assign_Vector(Result, normal);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Disc
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

void Disc::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Disc
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

void Disc::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Disc
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

void Disc::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Disc
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

void Disc::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Disc
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

void Disc::Transform(const TRANSFORM *tr)
{
	MTransNormal(normal, normal, tr);

	VNormalize(normal, normal);

	Compose_Transforms(Trans, tr);

	/* Recalculate the bounds */

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Disc
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

Disc::Disc() : ObjectBase(DISC_OBJECT)
{
	Make_Vector (center, 0.0, 0.0, 0.0);
	Make_Vector (normal, 0.0, 0.0, 1.0);

	iradius2 = 0.0;
	oradius2 = 1.0;

	d = 0.0;

	Trans = Create_Transform();

	/* Default bounds */

	Make_BBox(BBox, -1.0, -1.0, -SMALL_TOLERANCE, 2.0,  2.0, 2.0 * SMALL_TOLERANCE);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Disc
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
*   Sep 1994 : Fixed memory leakage [DB]
*
******************************************************************************/

ObjectPtr Disc::Copy()
{
	Disc *New = new Disc();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Disc
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

Disc::~Disc()
{
	Destroy_Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Disc
*
* INPUT
*
*   Disc - Disc
*
* OUTPUT
*
*   Disc
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate the transformation that scales, rotates, and translates
*   the disc to the desired location and orientation.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void Disc::Compute_Disc()
{
	Compute_Coordinate_Transform(Trans, center, normal, 1.0, 1.0);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Disc_BBox
*
* INPUT
*
*   Disc - Disc
*
* OUTPUT
*
*   Disc
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate the bounding box of a disc.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void Disc::Compute_BBox()
{
	DBL rad;

	rad = sqrt(oradius2);

	Make_BBox(BBox, -rad, -rad, -SMALL_TOLERANCE, 2.0*rad, 2.0*rad, 2.0*SMALL_TOLERANCE);

	Recompute_BBox(&BBox, Trans);
}

}
