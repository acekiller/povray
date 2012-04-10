/*******************************************************************************
 * fractal.cpp
 *
 * This module implements the fractal sets primitive.
 *
 * This file was written by Pascal Massimino.
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
 * $File: //depot/povray/smp/source/backend/shape/fractal.cpp $
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
#include "backend/shape/fractal.h"
#include "backend/math/quatern.h"
#include "backend/math/hcmplx.h"
#include "backend/scene/threaddata.h"
#include "base/pov_err.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const DBL Fractal_Tolerance = 1e-7;

/*****************************************************************************
* Local variables
******************************************************************************/

const COMPLEX_FUNCTION_METHOD Complex_Function_List[] =
{
	/* must match STYPE list in fractal.h */
	Complex_Exp,
	Complex_Ln,
	Complex_Sin,
	Complex_ASin,
	Complex_Cos,
	Complex_ACos,
	Complex_Tan,
	Complex_ATan,
	Complex_Sinh,
	Complex_ASinh,
	Complex_Cosh,
	Complex_ACosh,
	Complex_Tanh,
	Complex_ATanh,
	Complex_Pwr
};



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
*   Pascal Massimino
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

bool Fractal::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int Intersection_Found;
	int Last = 0;
	int CURRENT, NEXT;
	DBL Depth, Depth_Max;
	DBL Dist, Dist_Next, Len;

	VECTOR IPoint, Mid_Point, Next_Point, Real_Pt;
	VECTOR Real_Normal, F_Normal;
	VECTOR Direction;
	Ray New_Ray;

	Thread->Stats()[Ray_Fractal_Tests]++;

	Intersection_Found = false;

	/* Get into Fractal's world. */

	if (Trans != NULL)
	{
		MInvTransDirection(Direction, ray.Direction, Trans);
		VDot(Len, Direction, Direction);

		if (Len == 0.0)
		{
			return (false);
		}

		if (Len != 1.0)
		{
			Len = 1.0 / sqrt(Len);
			VScaleEq(Direction, Len);
		}

		Assign_Vector(New_Ray.Direction, Direction);
		MInvTransPoint(New_Ray.Origin, ray.Origin, Trans);
	}
	else
	{
		Assign_Vector(Direction, ray.Direction);
		New_Ray = ray;
		Len = 1.0;
	}

	/* Bound fractal. */

	if (!F_Bound(New_Ray, this, &Depth, &Depth_Max))
	{
		return (false);
	}

	if (Depth_Max < Fractal_Tolerance)
	{
		return (false);
	}

	if (Depth < Fractal_Tolerance)
	{
		Depth = Fractal_Tolerance;
	}

	/* Jump to starting point */

	VScale(Next_Point, Direction, Depth);
	VAddEq(Next_Point, New_Ray.Origin);

	CURRENT = D_Iteration(Next_Point, this, Direction, &Dist, Thread->Fractal_IStack);

	/* Light ray starting inside ? */

	if (CURRENT)
	{
		VAddScaledEq(Next_Point, 2.0 * Fractal_Tolerance, Direction);

		Depth += 2.0 * Fractal_Tolerance;

		if (Depth > Depth_Max)
		{
			return (false);
		}

		CURRENT = D_Iteration(Next_Point, this, Direction, &Dist, Thread->Fractal_IStack);
	}

	/* Ok. Trace it */

	while (Depth < Depth_Max)
	{
		/*
		 * Get close to the root: Advance with Next_Point, keeping track of last
		 * position in IPoint...
		 */

		while (1)
		{
			if (Dist < Precision)
				Dist = Precision;

			Depth += Dist;

			if (Depth > Depth_Max)
			{
				if (Intersection_Found)
					Thread->Stats()[Ray_Fractal_Tests_Succeeded]++;
				return (Intersection_Found);
			}

			Assign_Vector(IPoint, Next_Point);
			VAddScaledEq(Next_Point, Dist, Direction);

			NEXT = D_Iteration(Next_Point, this, Direction, &Dist_Next, Thread->Fractal_IStack);

			if (NEXT != CURRENT)
			{
				/* Set surface was crossed... */

				Depth -= Dist;
				break;
			}
			else
			{
				Dist = Dist_Next; /* not reached */
			}
		}

		/* then, polish the root via bisection method... */

		while (Dist > Fractal_Tolerance)
		{
			Dist *= 0.5;
			VAddScaled(Mid_Point, IPoint, Dist, Direction);

			Last = Iteration(Mid_Point, this, Thread->Fractal_IStack);

			if (Last == CURRENT)
			{
				Assign_Vector(IPoint, Mid_Point);

				Depth += Dist;

				if (Depth > Depth_Max)
				{
					if (Intersection_Found)
						Thread->Stats()[Ray_Fractal_Tests_Succeeded]++;
					return (Intersection_Found);
				}
			}
		}

		if (CURRENT == false) /* Mid_Point isn't inside the set */
		{
			VAddScaledEq(IPoint, Dist, Direction);

			Depth += Dist;

			Iteration(IPoint, this, Thread->Fractal_IStack);
		}
		else
		{
			if (Last != CURRENT)
			{
				Iteration(IPoint, this, Thread->Fractal_IStack);
			}
		}

		if (Trans != NULL)
		{
			MTransPoint(Real_Pt, IPoint, Trans);
			Normal_Calc(this, F_Normal, Thread->Fractal_IStack);
			MTransNormal(Real_Normal, F_Normal, Trans);
		}
		else
		{
			Assign_Vector(Real_Pt, IPoint);
			Normal_Calc(this, Real_Normal, Thread->Fractal_IStack);
		}

		if (Clip.empty() || Point_In_Clip(Real_Pt, Clip, Thread))
		{
			VNormalize(Real_Normal, Real_Normal);
			Depth_Stack->push(Intersection(Depth * Len, Real_Pt, Real_Normal, this));
			Intersection_Found = true;

			/* If fractal isn't used with CSG we can exit now. */

			if (!(Type & IS_CHILD_OBJECT))
			{
				break;
			}
		}

		/* Start over where work was left */

		Assign_Vector(IPoint, Next_Point);
		Dist = Dist_Next;
		CURRENT = NEXT;

	}

	if (Intersection_Found)
		Thread->Stats()[Ray_Fractal_Tests_Succeeded]++;
	return (Intersection_Found);
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
*   Pascal Massimino
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

bool Fractal::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	int Result;
	VECTOR New_Point;

	if (Trans != NULL)
	{
		MInvTransPoint(New_Point, IPoint, Trans);

		Result = Iteration(New_Point, this, Thread->Fractal_IStack);
	}
	else
	{
		Result = Iteration(IPoint, this, Thread->Fractal_IStack);
	}

	if (Inverted)
	{
		return (!Result);
	}
	else
	{
		return (Result);
	}
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Normal(VECTOR Result, Intersection *Intersect, TraceThreadData *) const
{
	Assign_Vector(Result, Intersect->INormal);
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
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
*   Pascal Massimino
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*   Mar 1996 : Moved call to Recompute_BBox to Compute_Fractal_BBox() (TW)
*
******************************************************************************/

void Fractal::Transform(const TRANSFORM *tr)
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
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Invert()
{
	Inverted ^= true;
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*   Mar 1996 : Added call to recompute_BBox() to bottom (TW)
*
******************************************************************************/

void Fractal::Compute_BBox()
{
	DBL R;

	switch (Algebra)
	{
		case QUATERNION_TYPE:

			R = 1.0 + sqrt(Sqr(Julia_Parm[X]) + Sqr(Julia_Parm[Y]) + Sqr(Julia_Parm[Z]) + Sqr(Julia_Parm[T]));
			R += Fractal_Tolerance; /* fix bug when Julia_Parameter exactly 0 */

			if (R > 2.0)
			{
				R = 2.0;
			}

			Exit_Value = Sqr(R) + Fractal_Tolerance;

			break;

		case HYPERCOMPLEX_TYPE:
		default:

			R = 4.0;

			Exit_Value = 16.0;

			break;
	}

	Radius_Squared = Sqr(R);

	Inverted = false;

	Make_BBox(BBox, -R, -R, -R, 2.0 * R, 2.0 * R, 2.0 * R);

	Recompute_BBox(&BBox, Trans);
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
*   Pascal Massimino
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

Fractal::Fractal() : ObjectBase(BASIC_OBJECT)
{
	Trans = NULL;

	Make_Vector(Center, 0.0, 0.0, 0.0);

	Julia_Parm[X] = 1.0;
	Julia_Parm[Y] = 0.0;
	Julia_Parm[Z] = 0.0;
	Julia_Parm[T] = 0.0;

	Slice[X] = 0.0;
	Slice[Y] = 0.0;
	Slice[Z] = 0.0;
	Slice[T] = 1.0;
	SliceDist = 0.0;

	Exit_Value = 4.0;

	Num_Iterations = 20;

	Precision = 1.0 / 20.0;

	Inverted = false;

	Algebra = QUATERNION_TYPE;

	Sub_Type = SQR_STYPE;

	Normal_Calc_Method = NULL;
	Iteration_Method   = NULL;
	D_Iteration_Method = NULL;
	F_Bound_Method     = NULL;
	Complex_Function_Method = NULL;

	Radius_Squared = 0.0;
	exponent.x = 0.0;
	exponent.y = 0.0;
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

ObjectPtr Fractal::Copy()
{
	Fractal *New = new Fractal();
	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	return (New);
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

Fractal::~Fractal()
{
	Destroy_Transform(Trans);
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

int Fractal::SetUp_Fractal(void)
{
	switch (Algebra)
	{
		case QUATERNION_TYPE:

			switch(Sub_Type)
			{
				case CUBE_STYPE:
					Iteration_Method = Iteration_z3;
					Normal_Calc_Method = Normal_Calc_z3;
					D_Iteration_Method = D_Iteration_z3;
					break;
				case SQR_STYPE:
					Iteration_Method = Iteration_Julia;
					D_Iteration_Method = D_Iteration_Julia;
					Normal_Calc_Method = Normal_Calc_Julia;
					break;
				default:
					throw POV_EXCEPTION_STRING("Illegal function: quaternion only supports sqr and cube");
			}
			F_Bound_Method = F_Bound_Julia;

			break;

		case HYPERCOMPLEX_TYPE:

			switch (Sub_Type)
			{
				case RECIPROCAL_STYPE:

					Iteration_Method = Iteration_HCompl_Reciprocal;
					Normal_Calc_Method = Normal_Calc_HCompl_Reciprocal;
					D_Iteration_Method = D_Iteration_HCompl_Reciprocal;
					F_Bound_Method = F_Bound_HCompl_Reciprocal;

					break;

				case EXP_STYPE:
				case LN_STYPE:
				case SIN_STYPE:
				case ASIN_STYPE:
				case COS_STYPE:
				case ACOS_STYPE:
				case TAN_STYPE:
				case ATAN_STYPE:
				case SINH_STYPE:
				case ASINH_STYPE:
				case COSH_STYPE:
				case ACOSH_STYPE:
				case TANH_STYPE:
				case ATANH_STYPE:
				case PWR_STYPE:

					Iteration_Method = Iteration_HCompl_Func;
					Normal_Calc_Method = Normal_Calc_HCompl_Func;
					D_Iteration_Method = D_Iteration_HCompl_Func;
					F_Bound_Method = F_Bound_HCompl_Func;
					Complex_Function_Method = Complex_Function_List[Sub_Type];

					break;

				case CUBE_STYPE:

					Iteration_Method = Iteration_HCompl_z3;
					Normal_Calc_Method = Normal_Calc_HCompl_z3;
					D_Iteration_Method = D_Iteration_HCompl_z3;
					F_Bound_Method = F_Bound_HCompl_z3;

					break;

				default:  /* SQR_STYPE or else... */

					Iteration_Method = Iteration_HCompl;
					Normal_Calc_Method = Normal_Calc_HCompl;
					D_Iteration_Method = D_Iteration_HCompl;
					F_Bound_Method = F_Bound_HCompl;

					break;
			}

			break;

		default:

			throw POV_EXCEPTION_STRING("Algebra unknown in fractal.");
	}

	Compute_BBox();

	return Num_Iterations;
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
*   Pascal Massimino
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Allocate_Iteration_Stack(DBL **IStack, int Len)
{
	Free_Iteration_Stack(IStack);
	if (Len == 0)
		return ;
	const int len = (Len + 1) * sizeof(DBL);
	for (int i = 0 ; i < 4 ; i++)
		IStack [i] = (DBL *) POV_MALLOC(len, "fractal iteration stack");
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
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Fractal::Free_Iteration_Stack(DBL **IStack)
{
	for (int i = 0 ; i < 4 ; i++)
	{
		if (IStack [i] != NULL)
		{
			POV_FREE (IStack [i]) ;
			IStack [i] = NULL ;
		}
	}
}

}
