/*******************************************************************************
 * planes.cpp
 *
 * This module implements functions that manipulate planes.
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
 * $File: //depot/povray/smp/source/backend/shape/planes.cpp $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/scene/threaddata.h"
#include "backend/shape/planes.h"

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
*   All_Plane_Intersections
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

bool Plane::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	DBL Depth;
	VECTOR IPoint;

	if (Intersect(ray, &Depth, Thread))
	{
		VEvaluateRay(IPoint, ray.Origin, Depth, ray.Direction);

		if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
		{
			Depth_Stack->push(Intersection(Depth,IPoint,this));
			return(true);
		}
	}

	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Intersect_Plane
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

bool Plane::Intersect(const Ray& ray, DBL *Depth, TraceThreadData *Thread) const
{
	DBL NormalDotOrigin, NormalDotDirection;
	VECTOR P, D;

	Thread->Stats()[Ray_Plane_Tests]++;

	if (Trans == NULL)
	{
		VDot(NormalDotDirection, Normal_Vector, ray.Direction);

		if (fabs(NormalDotDirection) < EPSILON)
		{
			return(false);
		}

		VDot(NormalDotOrigin, Normal_Vector, ray.Origin);
	}
	else
	{
		MInvTransPoint(P, ray.Origin, Trans);
		MInvTransDirection(D, ray.Direction, Trans);

		VDot(NormalDotDirection, Normal_Vector, D);

		if (fabs(NormalDotDirection) < EPSILON)
		{
			return(false);
		}

		VDot(NormalDotOrigin, Normal_Vector, P);
	}

	*Depth = -(NormalDotOrigin + Distance) / NormalDotDirection;

	if ((*Depth >= DEPTH_TOLERANCE) && (*Depth <= MAX_DISTANCE))
	{
		Thread->Stats()[Ray_Plane_Tests_Succeeded]++;
		return (true);
	}
	else
	{
		return (false);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Plane
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

bool Plane::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	DBL Temp;
	VECTOR P;

	if(Trans == NULL)
	{
		VDot(Temp, IPoint, Normal_Vector);
	}
	else
	{
		MInvTransPoint(P, IPoint, Trans);

		VDot(Temp, P, Normal_Vector);
	}

	return((Temp + Distance) < EPSILON);
}



/*****************************************************************************
*
* FUNCTION
*
*   Plane_Normal
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

void Plane::Normal(VECTOR Result, Intersection *, TraceThreadData *) const
{
	Assign_Vector(Result, Normal_Vector);

	if(Trans != NULL)
	{
		MTransNormal(Result, Result, Trans);

		VNormalize(Result, Result);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Plane
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

void Plane::Translate(const VECTOR Vector, const TRANSFORM *tr)
{
	VECTOR Translation;

	if(Trans == NULL)
	{
		VEvaluate (Translation, Normal_Vector, Vector);

		Distance -= Translation[X] + Translation[Y] + Translation[Z];

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
*   Rotate_Plane
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

void Plane::Rotate(const VECTOR, const TRANSFORM *tr)
{
	if(Trans == NULL)
	{
		MTransDirection(Normal_Vector, Normal_Vector, tr);

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
*   Scale_Plane
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

void Plane::Scale(const VECTOR Vector, const TRANSFORM *tr)
{
	DBL Length;

	if(Trans == NULL)
	{
		VDivEq(Normal_Vector, Vector);

		VLength(Length, Normal_Vector);

		VInverseScaleEq(Normal_Vector, Length);

		Distance /= Length;

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
*   Invert_Plane
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

void Plane::Invert()
{
	VScaleEq(Normal_Vector, -1.0);

	Distance *= -1.0;
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Plane
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

void Plane::Transform(const TRANSFORM *tr)
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
*   Create_Plane
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

Plane::Plane() : ObjectBase(PLANE_OBJECT)
{
	Make_Vector(Normal_Vector, 0.0, 1.0, 0.0);

	Distance = 0.0;

	Trans = NULL;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Plane
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

ObjectPtr Plane::Copy()
{
	Plane *New = new Plane();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Plane
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

Plane::~Plane()
{
	Destroy_Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Plane_BBox
*
* INPUT
*
*   Plane - Plane
*   
* OUTPUT
*
*   Plane
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a plane (it's always infinite).
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void Plane::Compute_BBox()
{
	Make_BBox(BBox, -BOUND_HUGE/2, -BOUND_HUGE/2, -BOUND_HUGE/2,
		BOUND_HUGE, BOUND_HUGE, BOUND_HUGE);

	if (!Clip.empty())
	{
		BBox = Clip[0]->BBox; // FIXME - only supports one clip object? [trf]
	}
}

bool Plane::Intersect_BBox(BBoxDirection, const BBOX_VECT&, const BBOX_VECT&, BBOX_VAL) const
{
	return true;
}

}
