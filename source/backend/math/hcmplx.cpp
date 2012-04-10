/*******************************************************************************
 * hcmplx.cpp
 *
 * This module implements hypercomplex Julia fractals.
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
 * $File: //depot/povray/smp/source/backend/math/hcmplx.cpp $
 * $Revision: #19 $
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
#include "backend/math/vector.h"
#include "backend/shape/fractal.h" // TODO - Where should hcmplx.h/hcmplx.cpp go? Are they really math? [trf]
#include "backend/shape/spheres.h" // TODO - Move sphere intersection function to math code! [trf]
#include "backend/math/hcmplx.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const DBL Fractal_Tolerance = 1e-8;

#define HMult(xr,yr,zr,wr,x1,y1,z1,w1,x2,y2,z2,w2)        \
	(xr) = (x1) * (x2) - (y1) * (y2) - (z1) * (z2) + (w1) * (w2);   \
	(yr) = (y1) * (x2) + (x1) * (y2) - (w1) * (z2) - (z1) * (w2);   \
	(zr) = (z1) * (x2) - (w1) * (y2) + (x1) * (z2) - (y1) * (w2);   \
	(wr) = (w1) * (x2) + (z1) * (y2) + (y1) * (z2) + (x1) * (w2);

#define HSqr(xr,yr,zr,wr,x,y,z,w)         \
	(xr) = (x) * (x) - (y) * (y) - (z) * (z) + (w) * (w) ;  \
	(yr) = 2.0 * ( (x) * (y) - (z) * (w) );     \
	(zr) = 2.0 * ( (z) * (x) - (w) * (y) );       \
	(wr) = 2.0 * ( (w) * (x) + (z) * (y) );



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Static functions
******************************************************************************/

static int HReciprocal (DBL * xr, DBL * yr, DBL * zr, DBL * wr, DBL x, DBL y, DBL z, DBL w);
static void HFunc (DBL * xr, DBL * yr, DBL * zr, DBL * wr, DBL x, DBL y, DBL z, DBL w, const Fractal *f);

/******** Computations with Hypercomplexes **********/

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
* CHANGES
*
******************************************************************************/

static int HReciprocal(DBL *xr, DBL  *yr, DBL  *zr, DBL  *wr, DBL  x, DBL  y, DBL  z, DBL  w)
{
	DBL det, mod, xt_minus_yz;

	det = ((x - w) * (x - w) + (y + z) * (y + z)) * ((x + w) * (x + w) + (y - z) * (y - z));

	if (det == 0.0)
	{
		return (-1);
	}

	mod = (x * x + y * y + z * z + w * w);

	xt_minus_yz = x * w - y * z;

	*xr = (x * mod - 2 * w * xt_minus_yz) / det;
	*yr = (-y * mod - 2 * z * xt_minus_yz) / det;
	*zr = (-z * mod - 2 * y * xt_minus_yz) / det;
	*wr = (w * mod - 2 * x * xt_minus_yz) / det;

	return (0);
}



/*****************************************************************************
*
* FUNCTION Hfunc
*
* INPUT 4D Hypercomplex number, pointer to fractal object
*
* OUTPUT  calculates the 4D generalization of fractal->Complex_Function
*
* RETURNS void
*
* AUTHOR
*
*   Pascal Massimino
*
* DESCRIPTION
*   Hypercomplex numbers allow generalization of any complex->complex
*   function in a uniform way. This function implements a general
*   unary 4D function based on the corresponding complex function. 
*
* CHANGES
*  Generalized to use Complex_Function()   TW 
*
******************************************************************************/

static void HFunc(DBL *xr, DBL  *yr, DBL  *zr, DBL  *wr, DBL  x, DBL  y, DBL  z, DBL  w, const Fractal *f)
{
	CMPLX a, b, ra, rb;

	/* convert to duplex form */
	a.x = x - w;
	a.y = y + z;
	b.x = x + w;
	b.y = y - z;

	/* apply function to each part */
	Complex_Function(&ra, &a, f);
	Complex_Function(&rb, &b, f);

	/* convert back */
	*xr = .5 * (ra.x + rb.x);
	*yr = .5 * (ra.y + rb.y);
	*zr = .5 * (ra.y - rb.y);
	*wr = .5 * (rb.x - ra.x);
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
* CHANGES
*
******************************************************************************/

/*------------------ z2 Iteration method ------------------*/

int Iteration_HCompl(const VECTOR IPoint, const Fractal *HCompl, DBL **IterStack)
{
	int i;
	DBL yz, xw;
	DBL Exit_Value;
	DBL x, y, z, w;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		yz = y * y + z * z;
		xw = x * x + w * w;

		if ((xw + yz) > Exit_Value)
		{
			return (false);
		}

		IterStack[X][i] = xw - yz + HCompl->Julia_Parm[X];
		IterStack[Y][i] = 2.0 * (x * y - z * w) + HCompl->Julia_Parm[Y];
		IterStack[Z][i] = 2.0 * (x * z - w * y) + HCompl->Julia_Parm[Z];
		IterStack[W][i] = 2.0 * (x * w + y * z) + HCompl->Julia_Parm[T];

		w = IterStack[W][i];
		x = IterStack[X][i];

		z = IterStack[Z][i];
		y = IterStack[Y][i];
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
* CHANGES
*
******************************************************************************/

int D_Iteration_HCompl(VECTOR IPoint, const Fractal *HCompl, const VECTOR& Direction, DBL *Dist, DBL **IterStack)
{
	int i;
	DBL yz, xw;
	DBL Exit_Value, F_Value, Step;
	DBL x, y, z, w;
	VECTOR H_Normal;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		yz = y * y + z * z;
		xw = x * x + w * w;

		if ((F_Value = xw + yz) > Exit_Value)
		{
			Normal_Calc_HCompl(H_Normal, i - 1, HCompl, IterStack);

			VDot(Step, H_Normal, Direction);

			if (Step < -Fractal_Tolerance)
			{
				Step = -2.0 * Step;

				if ((F_Value > HCompl->Precision * Step) && (F_Value < 30 * HCompl->Precision * Step))
				{
					*Dist = F_Value / Step;

					return (false);
				}
			}

			*Dist = HCompl->Precision;

			return (false);
		}

		IterStack[X][i] = xw - yz + HCompl->Julia_Parm[X];
		IterStack[Y][i] = 2.0 * (x * y - z * w) + HCompl->Julia_Parm[Y];
		IterStack[Z][i] = 2.0 * (x * z - w * y) + HCompl->Julia_Parm[Z];
		IterStack[W][i] = 2.0 * (x * w + y * z) + HCompl->Julia_Parm[T];

		w = IterStack[W][i];
		x = IterStack[X][i];

		z = IterStack[Z][i];
		y = IterStack[Y][i];
	}

	*Dist = HCompl->Precision;

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
* CHANGES
*
******************************************************************************/

void Normal_Calc_HCompl(VECTOR Result, int N_Max, const Fractal *, DBL **IterStack)
{
	DBL n1, n2, n3, n4;
	int i;
	DBL x, y, z, w;
	DBL xx, yy, zz, ww;
	DBL Pow;

	/*
	 * Algebraic properties of hypercomplexes allows simplifications in
	 * computations...
	 */

	x = IterStack[X][0];
	y = IterStack[Y][0];
	z = IterStack[Z][0];
	w = IterStack[W][0];

	Pow = 2.0;

	for (i = 1; i < N_Max; ++i)
	{

		/*
		 * For a map z->f(z), f depending on c, one must perform here :
		 *
		 * (x,y,z,w) * df/dz(IterStack[X][i],IterStack[Y][i],IterStack[Z][i],IterStack[W][i]) -> (x,y,z,w) ,
		 *
		 * up to a constant.
		 */

		/******************* Case z->z^2+c *****************/

		/* the df/dz part needs no work */

		HMult(xx, yy, zz, ww, IterStack[X][i], IterStack[Y][i], IterStack[Z][i], IterStack[W][i], x, y, z, w);

		w = ww;
		z = zz;
		y = yy;
		x = xx;

		Pow *= 2.0;
	}

	n1 = IterStack[X][N_Max] * Pow;
	n2 = IterStack[Y][N_Max] * Pow;
	n3 = IterStack[Z][N_Max] * Pow;
	n4 = IterStack[W][N_Max] * Pow;

	Result[X] = x * n1 + y * n2 + z * n3 + w * n4;
	Result[Y] = -y * n1 + x * n2 - w * n3 + z * n4;
	Result[Z] = -z * n1 - w * n2 + x * n3 + y * n4;
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
* CHANGES
*
******************************************************************************/

int F_Bound_HCompl(Ray & ray, const Fractal *fractal, DBL *Depth_Min, DBL  *Depth_Max)
{
	return (Sphere::Intersect(ray, fractal->Center, fractal->Radius_Squared, Depth_Min, Depth_Max));
}

/****************************************************************/
/*--------------------------- z3 -------------------------------*/
/****************************************************************/



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
* CHANGES
*
******************************************************************************/

int Iteration_HCompl_z3(const VECTOR IPoint, const Fractal *HCompl, DBL **IterStack)
{
	int i;
	DBL Norm, xx, yy, zz, ww;
	DBL Exit_Value;
	DBL x, y, z, w;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		Norm = x * x + y * y + z * z + w * w;

		/* is this test correct ? */
		if (Norm > Exit_Value)
		{
			return (false);
		}

		/*************** Case: z->z^2+c *********************/
		HSqr(xx, yy, zz, ww, x, y, z, w);

		x = IterStack[X][i] = xx + HCompl->Julia_Parm[X];
		y = IterStack[Y][i] = yy + HCompl->Julia_Parm[Y];
		z = IterStack[Z][i] = zz + HCompl->Julia_Parm[Z];
		w = IterStack[W][i] = ww + HCompl->Julia_Parm[T];

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
* CHANGES
*
******************************************************************************/

int D_Iteration_HCompl_z3(VECTOR IPoint, const Fractal *HCompl, const VECTOR& Direction, DBL *Dist, DBL **IterStack)
{
	int i;
	DBL xx, yy, zz, ww;
	DBL Exit_Value, F_Value, Step;
	DBL x, y, z, w;
	VECTOR H_Normal;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		F_Value = x * x + y * y + z * z + w * w;

		if (F_Value > Exit_Value)
		{
			Normal_Calc_HCompl_z3(H_Normal, i - 1, HCompl, IterStack);

			VDot(Step, H_Normal, Direction);

			if (Step < -Fractal_Tolerance)
			{
				Step = -2.0 * Step;

				if ((F_Value > HCompl->Precision * Step) && (F_Value < 30 * HCompl->Precision * Step))
				{
					*Dist = F_Value / Step;

					return (false);
				}
			}

			*Dist = HCompl->Precision;

			return (false);
		}

		/*************** Case: z->z^2+c *********************/

		HSqr(xx, yy, zz, ww, x, y, z, w);

		x = IterStack[X][i] = xx + HCompl->Julia_Parm[X];
		y = IterStack[Y][i] = yy + HCompl->Julia_Parm[Y];
		z = IterStack[Z][i] = zz + HCompl->Julia_Parm[Z];
		w = IterStack[W][i] = ww + HCompl->Julia_Parm[T];
	}

	*Dist = HCompl->Precision;

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
* CHANGES
*
******************************************************************************/

void Normal_Calc_HCompl_z3(VECTOR Result, int N_Max, const Fractal *, DBL **IterStack)
{
	DBL n1, n2, n3, n4;
	int i;
	DBL x, y, z, w;
	DBL xx, yy, zz, ww;

	/*
	 * Algebraic properties of hypercomplexes allows simplifications in
	 * computations...
	 */

	x = IterStack[X][0];
	y = IterStack[Y][0];
	z = IterStack[Z][0];
	w = IterStack[W][0];

	for (i = 1; i < N_Max; ++i)
	{
		/*
		 * For a map z->f(z), f depending on c, one must perform here :
		 * 
		 * (x,y,z,w) * df/dz(IterStack[X][i],IterStack[Y][i],IterStack[Z][i],IterStack[W][i]) -> (x,y,z,w) ,
		 * 
		 * up to a constant.
		 */

		/******************* Case z->z^2+c *****************/

		/* the df/dz part needs no work */

		HMult(xx, yy, zz, ww, IterStack[X][i], IterStack[Y][i], IterStack[Z][i], IterStack[W][i], x, y, z, w);

		x = xx;
		y = yy;
		z = zz;
		w = ww;
	}

	n1 = IterStack[X][N_Max];
	n2 = IterStack[Y][N_Max];
	n3 = IterStack[Z][N_Max];
	n4 = IterStack[W][N_Max];

	Result[X] = x * n1 + y * n2 + z * n3 + w * n4;
	Result[Y] = -y * n1 + x * n2 - w * n3 + z * n4;
	Result[Z] = -z * n1 - w * n2 + x * n3 + y * n4;
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
* CHANGES
*
******************************************************************************/

int F_Bound_HCompl_z3(Ray &ray, const Fractal *fractal, DBL *Depth_Min, DBL *Depth_Max)
{
	return F_Bound_HCompl(ray, fractal, Depth_Min, Depth_Max);
}

/*--------------------------- Inv -------------------------------*/


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
* CHANGES
*
******************************************************************************/

int Iteration_HCompl_Reciprocal(const VECTOR IPoint, const Fractal *HCompl, DBL **IterStack)
{
	int i;
	DBL Norm, xx, yy, zz, ww;
	DBL Exit_Value;
	DBL x, y, z, w;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		Norm = x * x + y * y + z * z + w * w;

		if (Norm > Exit_Value)
		{
			return (false);
		}

		HReciprocal(&xx, &yy, &zz, &ww, x, y, z, w);

		x = IterStack[X][i] = xx + HCompl->Julia_Parm[X];
		y = IterStack[Y][i] = yy + HCompl->Julia_Parm[Y];
		z = IterStack[Z][i] = zz + HCompl->Julia_Parm[Z];
		w = IterStack[W][i] = ww + HCompl->Julia_Parm[T];

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
* CHANGES
*
******************************************************************************/

int D_Iteration_HCompl_Reciprocal(VECTOR IPoint, const Fractal *HCompl, const VECTOR& Direction, DBL *Dist, DBL **IterStack)
{
	int i;
	DBL xx, yy, zz, ww;
	DBL Exit_Value, F_Value, Step;
	DBL x, y, z, w;
	VECTOR H_Normal;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		F_Value = x * x + y * y + z * z + w * w;

		if (F_Value > Exit_Value)
		{
			Normal_Calc_HCompl_Reciprocal(H_Normal, i - 1, HCompl, IterStack);

			VDot(Step, H_Normal, Direction);

			if (Step < -Fractal_Tolerance)
			{
				Step = -2.0 * Step;

				if ((F_Value > HCompl->Precision * Step) && F_Value < (30 * HCompl->Precision * Step))
				{
					*Dist = F_Value / Step;

					return (false);
				}
			}

			*Dist = HCompl->Precision;

			return (false);
		}

		HReciprocal(&xx, &yy, &zz, &ww, x, y, z, w);

		x = IterStack[X][i] = xx + HCompl->Julia_Parm[X];
		y = IterStack[Y][i] = yy + HCompl->Julia_Parm[Y];
		z = IterStack[Z][i] = zz + HCompl->Julia_Parm[Z];
		w = IterStack[W][i] = ww + HCompl->Julia_Parm[T];

	}

	*Dist = HCompl->Precision;

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
* CHANGES
*
******************************************************************************/

void Normal_Calc_HCompl_Reciprocal(VECTOR Result, int N_Max, const Fractal *, DBL **IterStack)
{
	DBL n1, n2, n3, n4;
	int i;
	DBL x, y, z, w;
	DBL xx, yy, zz, ww;
	DBL xxx, yyy, zzz, www;

	/*
	 * Algebraic properties of hypercomplexes allows simplifications in
	 * computations...
	 */

	x = IterStack[X][0];
	y = IterStack[Y][0];
	z = IterStack[Z][0];
	w = IterStack[W][0];

	for (i = 1; i < N_Max; ++i)
	{
		/******************* Case: z->1/z+c *****************/

		HReciprocal(&xx, &yy, &zz, &ww, IterStack[X][i], IterStack[Y][i], IterStack[Z][i], IterStack[W][i]);

		HSqr(xxx, yyy, zzz, www, xx, yy, zz, ww);

		HMult(xx, yy, zz, ww, x, y, z, w, -xxx, -yyy, -zzz, -www);

		x = xx;
		y = yy;
		z = zz;
		w = ww;
	}

	n1 = IterStack[X][N_Max];
	n2 = IterStack[Y][N_Max];
	n3 = IterStack[Z][N_Max];
	n4 = IterStack[W][N_Max];

	Result[X] = x * n1 + y * n2 + z * n3 + w * n4;
	Result[Y] = -y * n1 + x * n2 - w * n3 + z * n4;
	Result[Z] = -z * n1 - w * n2 + x * n3 + y * n4;
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
* CHANGES
*
******************************************************************************/

int F_Bound_HCompl_Reciprocal(Ray &ray, const Fractal *fractal, DBL *Depth_Min, DBL  *Depth_Max)
{
	return F_Bound_HCompl(ray, fractal, Depth_Min, Depth_Max);
}

/*--------------------------- Function -------------------------------*/


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
* CHANGES
*
******************************************************************************/

int Iteration_HCompl_Func(const VECTOR IPoint, const Fractal *HCompl, DBL **IterStack)
{
	int i;
	DBL Norm, xx, yy, zz, ww;
	DBL Exit_Value;
	DBL x, y, z, w;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Y];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		Norm = x * x + y * y + z * z + w * w;

		if (Norm > Exit_Value)
		{
			return (false);
		}

		HFunc(&xx, &yy, &zz, &ww, x, y, z, w, HCompl);

		x = IterStack[X][i] = xx + HCompl->Julia_Parm[X];
		y = IterStack[Y][i] = yy + HCompl->Julia_Parm[Y];
		z = IterStack[Z][i] = zz + HCompl->Julia_Parm[Z];
		w = IterStack[W][i] = ww + HCompl->Julia_Parm[T];

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
* CHANGES
*
******************************************************************************/

int D_Iteration_HCompl_Func(VECTOR IPoint, const Fractal *HCompl, const VECTOR& Direction, DBL *Dist, DBL **IterStack)
{
	int i;
	DBL xx, yy, zz, ww;
	DBL Exit_Value, F_Value, Step;
	DBL x, y, z, w;
	VECTOR H_Normal;

	x = IterStack[X][0] = IPoint[X];
	y = IterStack[Y][0] = IPoint[Y];
	z = IterStack[Z][0] = IPoint[Z];
	w = IterStack[W][0] = (HCompl->SliceDist
	                - HCompl->Slice[X]*x
	                - HCompl->Slice[Y]*y
	                - HCompl->Slice[Z]*z)/HCompl->Slice[T];

	Exit_Value = HCompl->Exit_Value;

	for (i = 1; i <= HCompl->Num_Iterations; ++i)
	{
		F_Value = x * x + y * y + z * z + w * w;

		if (F_Value > Exit_Value)
		{
			Normal_Calc_HCompl_Func(H_Normal, i - 1, HCompl, IterStack);

			VDot(Step, H_Normal, Direction);

			if (Step < -Fractal_Tolerance)
			{
				Step = -2.0 * Step;

				if ((F_Value > HCompl->Precision * Step) && F_Value < (30 * HCompl->Precision * Step))
				{
					*Dist = F_Value / Step;

					return (false);
				}
			}

			*Dist = HCompl->Precision;

			return (false);
		}

		HFunc(&xx, &yy, &zz, &ww, x, y, z, w, HCompl);

		x = IterStack[X][i] = xx + HCompl->Julia_Parm[X];
		y = IterStack[Y][i] = yy + HCompl->Julia_Parm[Y];
		z = IterStack[Z][i] = zz + HCompl->Julia_Parm[Z];
		w = IterStack[W][i] = ww + HCompl->Julia_Parm[T];

	}

	*Dist = HCompl->Precision;

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
* CHANGES
*
******************************************************************************/

void Normal_Calc_HCompl_Func(VECTOR Result, int N_Max, const Fractal *fractal, DBL **IterStack)
{
	DBL n1, n2, n3, n4;
	int i;
	DBL x, y, z, w;
	DBL xx, yy, zz, ww;
	DBL xxx, yyy, zzz, www;

	/*
	 * Algebraic properties of hypercomplexes allows simplifications in
	 * computations...
	 */

	x = IterStack[X][0];
	y = IterStack[Y][0];
	z = IterStack[Z][0];
	w = IterStack[W][0];

	for (i = 1; i < N_Max; ++i)
	{
		/**************** Case: z-> f(z)+c ************************/

		HFunc(&xx, &yy, &zz, &ww, IterStack[X][i], IterStack[Y][i], IterStack[Z][i], IterStack[W][i], fractal);

		HMult(xxx, yyy, zzz, www, xx, yy, zz, ww, x, y, z, w);

		x = xxx;
		y = yyy;
		z = zzz;
		w = www;
	}

	n1 = IterStack[X][N_Max];
	n2 = IterStack[Y][N_Max];
	n3 = IterStack[Z][N_Max];
	n4 = IterStack[W][N_Max];

	Result[X] = x * n1 + y * n2 + z * n3 + w * n4;
	Result[Y] = -y * n1 + x * n2 - w * n3 + z * n4;
	Result[Z] = -z * n1 - w * n2 + x * n3 + y * n4;
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
* CHANGES
*
******************************************************************************/

int F_Bound_HCompl_Func(Ray &ray, const Fractal *fractal, DBL *Depth_Min, DBL  *Depth_Max)
{
	return F_Bound_HCompl(ray, fractal, Depth_Min, Depth_Max);
}

/*****************************************************************************
*
* FUNCTION  Complex transcental functions
*
* INPUT     pointer to source complex number
*
* OUTPUT    fn(input)
*
* RETURNS   void
*
* AUTHOR
*
*   Tim Wegner
*
* DESCRIPTION  Calculate common functions on complexes
*   Since our purpose is fractals, error checking is lax.            
*
* CHANGES
*
******************************************************************************/

void Complex_Mult (CMPLX *target, CMPLX *source1, const CMPLX *source2)
{
	DBL tmpx;
	tmpx = source1->x * source2->x - source1->y * source2->y;
	target->y = source1->x * source2->y + source1->y * source2->x;
	target->x = tmpx;
}

void Complex_Div (CMPLX *target, CMPLX *source1, CMPLX *source2)
{
	DBL mod,tmpx,yxmod,yymod;
	mod = Sqr(source2->x) + Sqr(source2->y);
	if (mod==0)
		return;
	yxmod = source2->x/mod;
	yymod = - source2->y/mod;
	tmpx = source1->x * yxmod - source1->y * yymod;
	target->y = source1->x * yymod + source1->y * yxmod;
	target->x = tmpx;
} /* End Complex_Mult() */

void Complex_Exp (CMPLX *target, CMPLX *source, const CMPLX *)
{
	DBL expx;
	expx = exp(source->x);
	target->x = expx * cos(source->y);
	target->y = expx * sin(source->y);
} /* End Complex_Exp() */

void Complex_Sin (CMPLX *target, CMPLX *source, const CMPLX *)
{
	target->x = sin(source->x) * cosh(source->y);
	target->y = cos(source->x) * sinh(source->y);
} /* End Complex_Sin() */

void Complex_Sinh (CMPLX *target, CMPLX *source, const CMPLX *)
{
	target->x = sinh(source->x) * cos(source->y);
	target->y = cosh(source->x) * sin(source->y);
} /* End Complex_Sinh() */


void Complex_Cos (CMPLX *target, CMPLX *source, const CMPLX *)
{
	target->x = cos(source->x) * cosh(source->y);
	target->y = -sin(source->x) * sinh(source->y);
} /* End Complex_Cos() */

void Complex_Cosh (CMPLX *target, CMPLX *source, const CMPLX *)
{
	target->x = cosh(source->x) * cos(source->y);
	target->y = sinh(source->x) * sin(source->y);
} /* End Complex_Cosh() */


void Complex_Ln (CMPLX *target, CMPLX *source, const CMPLX *)
{
	DBL mod,zx,zy;
	mod = sqrt(source->x * source->x + source->y * source->y);
	zx = log(mod);
	zy = atan2(source->y,source->x);

	target->x = zx;
	target->y = zy;
} /* End Complex_Ln() */

void Complex_Sqrt(CMPLX *target, CMPLX *source)
{
	DBL mag;
	DBL theta;

	if(source->x == 0.0 && source->y == 0.0)
	{
		target->x = target->y = 0.0;
	}
	else
	{
		mag   = sqrt(sqrt(Sqr(source->x) + Sqr(source->y)));
		theta = atan2(source->y, source->x) / 2;
		target->y = mag * sin(theta);
		target->x = mag * cos(theta);
	}
} /* End Complex_Sqrt() */

/* rz=Arcsin(z)=-i*Log{i*z+sqrt(1-z*z)} */
void Complex_ASin(CMPLX *target, CMPLX *source, const CMPLX *)
{
	CMPLX tempz1,tempz2;

	Complex_Mult(&tempz1, source, source);
	tempz1.x = 1 - tempz1.x; tempz1.y = -tempz1.y;
	Complex_Sqrt( &tempz1, &tempz1);

	tempz2.x = -source->y; tempz2.y = source->x;
	tempz1.x += tempz2.x;  tempz1.y += tempz2.y;

	Complex_Ln( &tempz1, &tempz1, NULL);
	target->x = tempz1.y;  target->y = -tempz1.x;
} /* End Complex_ASin() */


void Complex_ACos(CMPLX *target, CMPLX *source, const CMPLX *)
{
	CMPLX temp;

	Complex_Mult(&temp, source, source);
	temp.x -= 1;
	Complex_Sqrt(&temp, &temp);

	temp.x += source->x; temp.y += source->y;

	Complex_Ln(&temp, &temp, NULL);
	target->x = temp.y;  target->y = -temp.x;
} /* End Complex_ACos() */

void Complex_ASinh(CMPLX *target, CMPLX *source, const CMPLX *)
{
	CMPLX temp;

	Complex_Mult (&temp, source, source);
	temp.x += 1;
	Complex_Sqrt (&temp, &temp);
	temp.x += source->x; temp.y += source->y;
	Complex_Ln(target, &temp, NULL);
} /* End Complex_ASinh */

/* rz=Arccosh(z)=Log(z+sqrt(z*z-1)} */
void Complex_ACosh (CMPLX *target, CMPLX *source, const CMPLX *)
{
	CMPLX tempz;
	Complex_Mult(&tempz, source, source);
	tempz.x -= 1;
	Complex_Sqrt (&tempz, &tempz);
	tempz.x = source->x + tempz.x; tempz.y = source->y + tempz.y;
	Complex_Ln (target, &tempz, NULL);
} /* End Complex_ACosh() */

/* rz=Arctanh(z)=1/2*Log{(1+z)/(1-z)} */
void Complex_ATanh(CMPLX *target, CMPLX *source, const CMPLX *)
{
	CMPLX temp0,temp1,temp2;

	if( source->x == 0.0)
	{
		target->x = 0;
		target->y = atan( source->y);
		return;
	}
	else
	{
		if( fabs(source->x) == 1.0 && source->y == 0.0)
		{
			return;
		}
		else if( fabs( source->x) < 1.0 && source->y == 0.0)
		{
			target->x = log((1+source->x)/(1-source->x))/2;
			target->y = 0;
			return;
		}
		else
		{
			temp0.x = 1 + source->x; temp0.y = source->y;
			temp1.x = 1 - source->x; temp1.y = -source->y;
			Complex_Div(&temp2, &temp0, &temp1);
			Complex_Ln(&temp2, &temp2, NULL);
			target->x = .5 * temp2.x; target->y = .5 * temp2.y;
			return;
		}
	}
} /* End Complex_ATanh() */

/* rz=Arctan(z)=i/2*Log{(1-i*z)/(1+i*z)} */
void Complex_ATan(CMPLX *target, CMPLX *source, const CMPLX *)
{
	CMPLX temp0,temp1,temp2,temp3;
	if( source->x == 0.0 && source->y == 0.0)
		target->x = target->y = 0;
	else if( source->x != 0.0 && source->y == 0.0){
		target->x = atan(source->x);
		target->y = 0;
	}
	else if( source->x == 0.0 && source->y != 0.0){
		temp0.x = source->y;  temp0.y = 0.0;
		Complex_ATanh(&temp0, &temp0, NULL);
		target->x = -temp0.y; target->y = temp0.x;
	}
	else if( source->x != 0.0 && source->y != 0.0)
	{
		temp0.x = -source->y; temp0.y = source->x;
		temp1.x = 1 - temp0.x; temp1.y = -temp0.y;
		temp2.x = 1 + temp0.x; temp2.y = temp0.y;

		Complex_Div(&temp3, &temp1, &temp2);
		Complex_Ln(&temp3, &temp3, NULL);
		target->x = -temp3.y * .5; target->y = .5 * temp3.x;
	}
} /* End Complex_ATanz() */

void Complex_Tan(CMPLX *target, CMPLX *source, const CMPLX *)
{
	DBL x, y, sinx, cosx, sinhy, coshy, denom;
	x = 2 * source->x;
	y = 2 * source->y;
	sinx = sin(x); cosx = cos(x);
	sinhy = sinh(y); coshy = cosh(y);
	denom = cosx + coshy;
	if(denom == 0)
		return;
	target->x = sinx/denom;
	target->y = sinhy/denom;
} /* End Complex_Tan() */

void Complex_Tanh(CMPLX *target, CMPLX *source, const CMPLX *)
{
	DBL x, y, siny, cosy, sinhx, coshx, denom;
	x = 2 * source->x;
	y = 2 * source->y;
	siny = sin(y); cosy = cos(y);
	sinhx = sinh(x); coshx = cosh(x);
	denom = coshx + cosy;
	if(denom == 0)
		return;
	target->x = sinhx/denom;
	target->y = siny/denom;
} /* End Complex_Tanh() */


void Complex_Pwr (CMPLX *target, CMPLX *source1, const CMPLX *source2)
{
	CMPLX cLog, t;
	DBL e2x;

	if(source1->x == 0 && source1->y == 0)
	{
		target->x = target->y = 0.0;
		return;
	}

	Complex_Ln (&cLog, source1, NULL);
	Complex_Mult (&t, &cLog, source2);

	if(t.x < -690)
		e2x = 0;
	else
		e2x = exp(t.x);
	target->x = e2x * cos(t.y);
	target->y = e2x * sin(t.y);
}

}
