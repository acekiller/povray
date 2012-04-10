/*******************************************************************************
 * quatern.cpp
 *
 * This module implements Quaternion algebra julia fractals.
 *
 * This file was written by Pascal Massimino.
 * Revised and updated for POV-Ray 3.x by Tim Wegner
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
 * $File: //depot/povray/smp/source/backend/math/quatern.cpp $
 * $Revision: #18 $
 * $Change: 5091 $
 * $DateTime: 2010/08/06 11:17:18 $
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
#include "povray.h"
#include "backend/math/vector.h"
#include "backend/shape/fractal.h"
#include "backend/math/quatern.h"
#include "backend/shape/spheres.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define Deriv_z2(n1,n2,n3,n4)               \
{                                           \
	tmp = (n1)*x - (n2)*y - (n3)*z - (n4)*w;  \
	(n2) = (n1)*y + x*(n2);                   \
	(n3) = (n1)*z + x*(n3);                   \
	(n4) = (n1)*w + x*(n4);                   \
	(n1) = tmp;                               \
}

#define Deriv_z3(n1,n2,n3,n4)              \
{                                          \
	dtmp = 2.0*((n2)*y + (n3)*z + (n4)*w);   \
	dtmp2 = 6.0*x*(n1) - dtmp;               \
	(n1) = ( (n1)*x3 - x*dtmp )*3.0;         \
	(n2) = (n2)*x4 + y*dtmp2;                \
	(n3) = (n3)*x4 + z*dtmp2;                \
	(n4) = (n4)*x4 + w*dtmp2;                \
}


/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Static functions
******************************************************************************/



/*****************************************************************************
* Local variables
******************************************************************************/



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

int Iteration_z3(const VECTOR point, const Fractal *Julia, DBL **IterStack)
{
	int i;
	DBL x, y, z, w;
	DBL d, x2, tmp;
	DBL Exit_Value;

	IterStack[X][0] = x = point[X];
	IterStack[Y][0] = y = point[Y];
	IterStack[Z][0] = z = point[Z];
	IterStack[W][0] = w = (Julia->SliceDist
	                - Julia->Slice[X]*x
	                - Julia->Slice[Y]*y
	                - Julia->Slice[Z]*z)/Julia->Slice[T];

	Exit_Value = Julia->Exit_Value;

	for (i = 1; i <= Julia->Num_Iterations; ++i)
	{
		d = y * y + z * z + w * w;

		x2 = x * x;

		if ((d + x2) > Exit_Value)
		{
			return (false);
		}

		tmp = 3.0 * x2 - d;

		IterStack[X][i] = x = x * (x2 - 3.0 * d) + Julia->Julia_Parm[X];
		IterStack[Y][i] = y = y * tmp + Julia->Julia_Parm[Y];
		IterStack[Z][i] = z = z * tmp + Julia->Julia_Parm[Z];
		IterStack[W][i] = w = w * tmp + Julia->Julia_Parm[T];
	}

	return (true);
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

int Iteration_Julia(const VECTOR point, const Fractal *Julia, DBL **IterStack)
{
	int i;
	DBL x, y, z, w;
	DBL d, x2;
	DBL Exit_Value;

	IterStack[X][0] = x = point[X];
	IterStack[Y][0] = y = point[Y];
	IterStack[Z][0] = z = point[Z];
	IterStack[W][0] = w = (Julia->SliceDist
	                - Julia->Slice[X]*x
	                - Julia->Slice[Y]*y
	                - Julia->Slice[Z]*z)/Julia->Slice[T];

	Exit_Value = Julia->Exit_Value;

	for (i = 1; i <= Julia->Num_Iterations; ++i)
	{
		d = y * y + z * z + w * w;

		x2 = x * x;

		if ((d + x2) > Exit_Value)
		{
			return (false);
		}

		x *= 2.0;

		IterStack[Y][i] = y = x * y + Julia->Julia_Parm[Y];
		IterStack[Z][i] = z = x * z + Julia->Julia_Parm[Z];
		IterStack[W][i] = w = x * w + Julia->Julia_Parm[T];
		IterStack[X][i] = x = x2 - d + Julia->Julia_Parm[X];;

	}

	return (true);
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
* D_Iteration puts in *Dist a lower bound for the distance from *point to the
* set
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

/*----------- Distance estimator + iterations ------------*/

int D_Iteration_z3(VECTOR point, const Fractal *Julia, const VECTOR &, DBL *Dist, DBL **IterStack)
{
	int i, j;
	DBL Norm, d;
	DBL xx, yy, zz;
	DBL x, y, z, w;
	DBL tmp, x2;
	DBL Exit_Value;
	DBL Pow;

	x = IterStack[X][0] = point[X];
	y = IterStack[Y][0] = point[Y];
	z = IterStack[Z][0] = point[Z];
	w = IterStack[W][0] = (Julia->SliceDist
	                - Julia->Slice[X]*x
	                - Julia->Slice[Y]*y
	                - Julia->Slice[Z]*z)/Julia->Slice[T];

	Exit_Value = Julia->Exit_Value;

	for (i = 1; i <= Julia->Num_Iterations; i++)
	{
		d = y * y + z * z + w * w;

		x2 = x * x;

		if ((Norm = d + x2) > Exit_Value)
		{
			/* Distance estimator */

			x = IterStack[X][0];
			y = IterStack[Y][0];
			z = IterStack[Z][0];
			w = IterStack[W][0];

			Pow = 1.0 / 3.0;

			for (j = 1; j < i; ++j)
			{
				xx = x * IterStack[X][j] - y * IterStack[Y][j] - z * IterStack[Z][j] - w * IterStack[W][j];
				yy = x * IterStack[Y][j] + y * IterStack[X][j] - z * IterStack[W][j] + w * IterStack[Z][j];
				zz = x * IterStack[Z][j] + y * IterStack[W][j] + z * IterStack[X][j] - w * IterStack[Y][j];
				w  = x * IterStack[W][j] - y * IterStack[Z][j] + z * IterStack[Y][j] + w * IterStack[X][j];

				x = xx;
				y = yy;
				z = zz;

				Pow /= 3.0;
			}

			*Dist = Pow * sqrt(Norm / (x * x + y * y + z * z + w * w)) * log(Norm);

			return (false);
		}

		tmp = 3.0 * x2 - d;

		IterStack[X][i] = x = x * (x2 - 3.0 * d) + Julia->Julia_Parm[X];
		IterStack[Y][i] = y = y * tmp + Julia->Julia_Parm[Y];
		IterStack[Z][i] = z = z * tmp + Julia->Julia_Parm[Z];
		IterStack[W][i] = w = w * tmp + Julia->Julia_Parm[T];
	}

	*Dist = Julia->Precision;

	return (true);
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

int D_Iteration_Julia(VECTOR point, const Fractal *Julia, const VECTOR &, DBL *Dist, DBL **IterStack)
{
	int i, j;
	DBL Norm, d;
	DBL Exit_Value;
	DBL x, y, z, w;
	DBL xx, yy, zz, x2;
	DBL Pow;

	x = IterStack[X][0] = point[X];
	y = IterStack[Y][0] = point[Y];
	z = IterStack[Z][0] = point[Z];
	w = IterStack[W][0] = (Julia->SliceDist
	                - Julia->Slice[X]*x
	                - Julia->Slice[Y]*y
	                - Julia->Slice[Z]*z)/Julia->Slice[T];

	Exit_Value = Julia->Exit_Value;

	for (i = 1; i <= Julia->Num_Iterations; i++)
	{
		d = y * y + z * z + w * w;

		x2 = x * x;

		if ((Norm = d + x2) > Exit_Value)
		{
			/* Distance estimator */

			x = IterStack[X][0];
			y = IterStack[Y][0];
			z = IterStack[Z][0];
			w = IterStack[W][0];

			Pow = 1.0 / 2.0;

			for (j = 1; j < i; ++j)
			{
				xx = x * IterStack[X][j] - y * IterStack[Y][j] - z * IterStack[Z][j] - w * IterStack[W][j];
				yy = x * IterStack[Y][j] + y * IterStack[X][j] + w * IterStack[Z][j] - z * IterStack[W][j];
				zz = x * IterStack[Z][j] + z * IterStack[X][j] + y * IterStack[W][j] - w * IterStack[Y][j];
				w  = x * IterStack[W][j] + w * IterStack[X][j] + z * IterStack[Y][j] - y * IterStack[Z][j];

				x = xx;
				y = yy;
				z = zz;

				Pow /= 2.0;
			}

			*Dist = Pow * sqrt(Norm / (x * x + y * y + z * z + w * w)) * log(Norm);

			return (false);
		}

		x *= 2.0;

		IterStack[Y][i] = y = x * y + Julia->Julia_Parm[Y];
		IterStack[Z][i] = z = x * z + Julia->Julia_Parm[Z];
		IterStack[W][i] = w = x * w + Julia->Julia_Parm[T];
		IterStack[X][i] = x = x2 - d + Julia->Julia_Parm[X];

	}

	*Dist = Julia->Precision;

	return (true);
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
* Provided the iterations sequence has been built, perform the computation of
* the Normal
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Normal_Calc_z3(VECTOR Result, int N_Max, const Fractal *, DBL **IterStack)
{
	DBL
	n11 = 1.0, n12 = 0.0, n13 = 0.0, n14 = 0.0,
	n21 = 0.0, n22 = 1.0, n23 = 0.0, n24 = 0.0,
	n31 = 0.0, n32 = 0.0, n33 = 1.0, n34 = 0.0;

	DBL x, y, z, w;
	int i;
	DBL tmp, dtmp, dtmp2, x2, x3, x4;

	x = IterStack[X][0];
	y = IterStack[Y][0];
	z = IterStack[Z][0];
	w = IterStack[W][0];

	for (i = 1; i <= N_Max; i++)
	{
		tmp = y * y + z * z + w * w;

		x2 = x * x;
		x3 = x2 - tmp;
		x4 = 3.0 * x2 - tmp;

		Deriv_z3(n11, n12, n13, n14);
		Deriv_z3(n21, n22, n23, n24);
		Deriv_z3(n31, n32, n33, n34);

		x = IterStack[X][i];
		y = IterStack[Y][i];
		z = IterStack[Z][i];
		w = IterStack[W][i];
	}

	Result[X] = n11 * x + n12 * y + n13 * z + n14 * w;
	Result[Y] = n21 * x + n22 * y + n23 * z + n24 * w;
	Result[Z] = n31 * x + n32 * y + n33 * z + n34 * w;
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

void Normal_Calc_Julia(VECTOR Result, int N_Max, const Fractal *, DBL **IterStack)
{
	DBL
	n11 = 1.0, n12 = 0.0, n13 = 0.0, n14 = 0.0,
	n21 = 0.0, n22 = 1.0, n23 = 0.0, n24 = 0.0,
	n31 = 0.0, n32 = 0.0, n33 = 1.0, n34 = 0.0;
	DBL tmp;
	DBL x, y, z, w;
	int i;

	x = IterStack[X][0];
	y = IterStack[Y][0];
	z = IterStack[Z][0];
	w = IterStack[W][0];

	for (i = 1; i <= N_Max; i++)
	{
		Deriv_z2(n11, n12, n13, n14);
		Deriv_z2(n21, n22, n23, n24);
		Deriv_z2(n31, n32, n33, n34);

		x = IterStack[X][i];
		y = IterStack[Y][i];
		z = IterStack[Z][i];
		w = IterStack[W][i];
	}

	Result[X] = n11 * x + n12 * y + n13 * z + n14 * w;
	Result[Y] = n21 * x + n22 * y + n23 * z + n24 * w;
	Result[Z] = n31 * x + n32 * y + n33 * z + n34 * w;
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

int F_Bound_Julia(Ray& ray, const Fractal *fractal, DBL *Depth_Min, DBL *Depth_Max)
{
	return (Sphere::Intersect(ray, fractal->Center, fractal->Radius_Squared, Depth_Min, Depth_Max));
}

}
