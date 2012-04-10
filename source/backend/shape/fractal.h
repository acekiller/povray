/*******************************************************************************
 * fractal.h
 *
 * This module contains all defines, typedefs, and prototypes for FRACTAL.CPP.
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
 * $File: //depot/povray/smp/source/backend/shape/fractal.h $
 * $Revision: #20 $
 * $Change: 5009 $
 * $DateTime: 2010/06/05 10:39:30 $
 * $Author: chrisc $
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

#ifndef FRACTAL_H
#define FRACTAL_H

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define QUATERNION_TYPE    0
#define HYPERCOMPLEX_TYPE  1

/* Hcmplx function stypes must come first */
#define EXP_STYPE          0
#define LN_STYPE           1
#define SIN_STYPE          2
#define ASIN_STYPE         3
#define COS_STYPE          4
#define ACOS_STYPE         5
#define TAN_STYPE          6
#define ATAN_STYPE         7
#define SINH_STYPE         8
#define ASINH_STYPE        9
#define COSH_STYPE        10
#define ACOSH_STYPE       11
#define TANH_STYPE        12
#define ATANH_STYPE       13
#define PWR_STYPE         14

/* end function stypes */
#define SQR_STYPE         15
#define CUBE_STYPE        16
#define RECIPROCAL_STYPE  17

#define Iteration(V,F,IS) ( (*((F)->Iteration_Method))(V,F,IS) )
#define Normal_Calc(F,V,IS) ( (*((F)->Normal_Calc_Method))(V,(F)->Num_Iterations,F,IS) )
#define F_Bound(R,F,dm,dM) ( (*((F)->F_Bound_Method))(R,F,dm,dM) )
#define D_Iteration(V,F,I,D,IS) ( (*((F)->D_Iteration_Method))(V,F,I,D,IS) )
#define Complex_Function(t,s,F) ( (*((F)->Complex_Function_Method))(t,s,&(F)->exponent) )

/*****************************************************************************
* Global typedefs
******************************************************************************/

class Fractal;

typedef struct cmplx { DBL x,y; } CMPLX;
typedef void (*NORMAL_CALC_METHOD) (VECTOR, int, const Fractal *, DBL **);
typedef int (*ITERATION_METHOD) (const VECTOR, const Fractal *, DBL **);
typedef int (*D_ITERATION_METHOD) (VECTOR, const Fractal *, const VECTOR&, DBL *, DBL **);
typedef int (*F_BOUND_METHOD) (Ray &, const Fractal *, DBL *, DBL *);
typedef void (*COMPLEX_FUNCTION_METHOD) (CMPLX *, CMPLX *, const CMPLX *);

class Fractal : public ObjectBase
{
	public:
		VECTOR Center;
		DBL Julia_Parm[4];
		DBL Slice[4];                 /* vector perpendicular to slice plane */
		DBL SliceDist;                /* distance from slice plane to origin */
		DBL Exit_Value;
		int Num_Iterations;           /* number of iterations */
		DBL Precision;                /* Precision value */
		int Inverted;
		int Algebra;                  /* Quaternion or Hypercomplex */
		int Sub_Type;
		CMPLX exponent;               /* exponent of power function */
		DBL Radius_Squared;           /* For F_Bound(), if needed */
		NORMAL_CALC_METHOD Normal_Calc_Method;
		ITERATION_METHOD Iteration_Method;
		D_ITERATION_METHOD D_Iteration_Method;
		F_BOUND_METHOD F_Bound_Method;
		COMPLEX_FUNCTION_METHOD Complex_Function_Method;

		Fractal();
		virtual ~Fractal();

		virtual ObjectPtr Copy();

		virtual bool All_Intersections(const Ray&, IStack&, TraceThreadData *);
		virtual bool Inside(const VECTOR, TraceThreadData *) const;
		virtual void Normal(VECTOR, Intersection *, TraceThreadData *) const;
		virtual void Translate(const VECTOR, const TRANSFORM *);
		virtual void Rotate(const VECTOR, const TRANSFORM *);
		virtual void Scale(const VECTOR, const TRANSFORM *);
		virtual void Transform(const TRANSFORM *);
		virtual void Invert();
		virtual void Compute_BBox();

		static void Free_Iteration_Stack(DBL **IStack);
		static void Allocate_Iteration_Stack(DBL **IStack, int Len);

		int SetUp_Fractal(void);
};

/*****************************************************************************
* Global variables
******************************************************************************/

}

#endif
