/*******************************************************************************
 * vector.h
 *
 * This module contains macros to perform operations on vectors.
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
 * $File: //depot/povray/smp/source/backend/math/vector.h $
 * $Revision: #11 $
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

#ifndef VECTOR_H
#define VECTOR_H

#include "backend/frame.h"

namespace pov
{

/*****************************************************************************
* Inline functions
******************************************************************************/

// Vector Add
inline void VAdd(VECTOR a, const VECTOR b, const VECTOR c)
{
	a[X] = b[X] + c[X];
	a[Y] = b[Y] + c[Y];
	a[Z] = b[Z] + c[Z];
}

inline void VAdd(SNGL_VECT a, const VECTOR b, const VECTOR c)
{
	a[X] = b[X] + c[X];
	a[Y] = b[Y] + c[Y];
	a[Z] = b[Z] + c[Z];
}

inline void VAdd(SNGL_VECT a, const SNGL_VECT b, const SNGL_VECT c)
{
	a[X] = b[X] + c[X];
	a[Y] = b[Y] + c[Y];
	a[Z] = b[Z] + c[Z];
}

inline void VAddEq(VECTOR a, const VECTOR b)
{
	a[X] += b[X];
	a[Y] += b[Y];
	a[Z] += b[Z];
}

inline void VAddEq(SNGL_VECT a, const VECTOR b)
{
	a[X] += b[X];
	a[Y] += b[Y];
	a[Z] += b[Z];
}

inline void VAddEq(SNGL_VECT a, const SNGL_VECT b)
{
	a[X] += b[X];
	a[Y] += b[Y];
	a[Z] += b[Z];
}

// Vector Subtract
inline void VSub(VECTOR a, const VECTOR b, const VECTOR c)
{
	a[X] = b[X] - c[X];
	a[Y] = b[Y] - c[Y];
	a[Z] = b[Z] - c[Z];
}

inline void VSub(SNGL_VECT a, const VECTOR b, const VECTOR c)
{
	a[X] = b[X] - c[X];
	a[Y] = b[Y] - c[Y];
	a[Z] = b[Z] - c[Z];
}

inline void VSub(VECTOR a, const SNGL_VECT b, const VECTOR c)
{
	a[X] = b[X] - c[X];
	a[Y] = b[Y] - c[Y];
	a[Z] = b[Z] - c[Z];
}

inline void VSub(VECTOR a, const VECTOR b, const SNGL_VECT c)
{
	a[X] = b[X] - c[X];
	a[Y] = b[Y] - c[Y];
	a[Z] = b[Z] - c[Z];
}

inline void VSub(VECTOR a, const SNGL_VECT b, const SNGL_VECT c)
{
	a[X] = b[X] - c[X];
	a[Y] = b[Y] - c[Y];
	a[Z] = b[Z] - c[Z];
}

inline void VSub(SNGL_VECT a, const SNGL_VECT b, const SNGL_VECT c)
{
	a[X] = b[X] - c[X];
	a[Y] = b[Y] - c[Y];
	a[Z] = b[Z] - c[Z];
}

inline void VSubEq(VECTOR a, const VECTOR b)
{
	a[X] -= b[X];
	a[Y] -= b[Y];
	a[Z] -= b[Z];
}

inline void VSubEq(SNGL_VECT a, const VECTOR b)
{
	a[X] -= b[X];
	a[Y] -= b[Y];
	a[Z] -= b[Z];
}

inline void VSubEq(SNGL_VECT a, const SNGL_VECT b)
{
	a[X] -= b[X];
	a[Y] -= b[Y];
	a[Z] -= b[Z];
}

// Scale - Multiply Vector by a Scalar
inline void VScale(VECTOR a, const VECTOR b, DBL k)
{
	a[X] = b[X] * k;
	a[Y] = b[Y] * k;
	a[Z] = b[Z] * k;
}

// Scale - Multiply Vector by a Scalar
inline void VScale(SNGL_VECT a, const VECTOR b, DBL k)
{
	a[X] = b[X] * k;
	a[Y] = b[Y] * k;
	a[Z] = b[Z] * k;
}

inline void VScale(SNGL_VECT a, const SNGL_VECT b, SNGL k)
{
	a[X] = b[X] * k;
	a[Y] = b[Y] * k;
	a[Z] = b[Z] * k;
}

inline void VScaleEq(VECTOR a, DBL k)
{
	a[X] *= k;
	a[Y] *= k;
	a[Z] *= k;
}

inline void VScaleEq(SNGL_VECT a, SNGL k)
{
	a[X] *= k;
	a[Y] *= k;
	a[Z] *= k;
}

// Inverse Scale - Divide Vector by a Scalar
inline void VInverseScale(VECTOR a, const VECTOR b, DBL k)
{
	DBL tmp = 1.0 / k;
	a[X] = b[X] * tmp;
	a[Y] = b[Y] * tmp;
	a[Z] = b[Z] * tmp;
}

inline void VInverseScale(SNGL_VECT a, const SNGL_VECT b, SNGL k)
{
	SNGL tmp = 1.0 / k;
	a[X] = b[X] * tmp;
	a[Y] = b[Y] * tmp;
	a[Z] = b[Z] * tmp;
}

inline void VInverseScaleEq(VECTOR a, DBL k)
{
	DBL tmp = 1.0 / k;
	a[X] *= tmp;
	a[Y] *= tmp;
	a[Z] *= tmp;
}

inline void VInverseScaleEq(SNGL_VECT a, SNGL k)
{
	SNGL tmp = 1.0 / k;
	a[X] *= tmp;
	a[Y] *= tmp;
	a[Z] *= tmp;
}

// Dot Product - Gives Scalar angle (a) between two vectors (b) and (c)
inline void VDot(DBL& a, const VECTOR b, const VECTOR c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z];
}

inline void VDot(SNGL& a, const VECTOR b, const VECTOR c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z];
}

inline void VDot(DBL& a, const VECTOR b, const SNGL_VECT c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z];
}

inline void VDot(DBL& a, const SNGL_VECT b, const VECTOR c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z];
}

inline void VDot(DBL& a, const SNGL_VECT b, const SNGL_VECT c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z];
}

inline void VDot(SNGL& a, const SNGL_VECT b, const SNGL_VECT c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z];
}

// Cross Product - returns Vector (a) = (b) x (c)
inline void VCross(VECTOR a, const VECTOR b, const VECTOR c)
{
	VECTOR tmp;

	tmp[X] = b[Y] * c[Z] - b[Z] * c[Y];
	tmp[Y] = b[Z] * c[X] - b[X] * c[Z];
	tmp[Z] = b[X] * c[Y] - b[Y] * c[X];

	Assign_Vector(a, tmp);
}

// Evaluate - returns Vector (a) = Multiply Vector (b) by Vector (c)
inline void VEvaluate(VECTOR a, const VECTOR b, const VECTOR c)
{
	a[X] = b[X] * c[X];
	a[Y] = b[Y] * c[Y];
	a[Z] = b[Z] * c[Z];
}

inline void VEvaluateEq(VECTOR a, const VECTOR b)
{
	a[X] *= b[X];
	a[Y] *= b[Y];
	a[Z] *= b[Z];
}

// Divide - returns Vector (a) = Divide Vector (a) by Vector (b)
inline void VDivEq(VECTOR a, const VECTOR b)
{
	a[X] /= b[X];
	a[Y] /= b[Y];
	a[Z] /= b[Z];
}

// Simple Scalar Square Macro
inline DBL Sqr(DBL a)
{
	return a * a;
}

inline SNGL Sqr(SNGL a)
{
	return a * a;
}

// Vector Length - returs Scalar Euclidean Length (a) of Vector (b)
inline void VLength(DBL& a, const VECTOR b)
{
	a = sqrt(b[X] * b[X] + b[Y] * b[Y] + b[Z] * b[Z]);
}

inline void VLength(SNGL& a, const SNGL_VECT b)
{
	a = sqrt(b[X] * b[X] + b[Y] * b[Y] + b[Z] * b[Z]);
}

// Vector Distance - returs Scalar Euclidean Distance (a) between two points/Vectors (b) and (c)
inline void VDist(DBL& a, const VECTOR b, const VECTOR c)
{
	VECTOR tmp;
	VSub(tmp, b, c);
	VLength(a, tmp);
}

// Normalize a Vector - returns a vector (length of 1) that points at (b)
inline void VNormalize(VECTOR a, const VECTOR b)
{
	DBL tmp;
	VLength(tmp, b);
	VInverseScale(a, b, tmp);
}

inline void VNormalize(SNGL_VECT a, const SNGL_VECT b)
{
	SNGL tmp;
	VLength(tmp, b);
	VInverseScale(a, b, tmp);
}

inline void VNormalizeEq(VECTOR a)
{
	DBL tmp;
	VLength(tmp, a);
	VInverseScaleEq(a, tmp);
}

// Compute a Vector (a) Halfway Between Two Given Vectors (b) and (c)
inline void VHalf(VECTOR a, const VECTOR b, const VECTOR c)
{
	a[X] = 0.5 * (b[X] + c[X]);
	a[Y] = 0.5 * (b[Y] + c[Y]);
	a[Z] = 0.5 * (b[Z] + c[Z]);
}

// Calculate the sum of the sqares of the components of a vector.  (the square of its length)
inline DBL VSumSqr(VECTOR a)
{
	return a[X] * a[X] + a[Y] * a[Y] + a[Z] * a[Z];
}

// Linear combination of 2 vectors. [DB 7/94]
//   v = k1 * v1 + k2 * v2
inline void VLinComb2(VECTOR v, DBL k1, const VECTOR v1, DBL k2, const VECTOR v2)
{
	v[X] = k1 * v1[X] + k2 * v2[X];
	v[Y] = k1 * v1[Y] + k2 * v2[Y];
	v[Z] = k1 * v1[Z] + k2 * v2[Z];
}

// Linear combination of 3 vectors. [DB 7/94]
//   v = k1 * v1 + k2 * v2 + k3 * v3
inline void VLinComb3(VECTOR v, DBL k1, const VECTOR v1, DBL k2, const VECTOR v2, DBL k3, const VECTOR v3)
{
	v[X] = k1 * v1[X] + k2 * v2[X] + k3 * v3[X];
	v[Y] = k1 * v1[Y] + k2 * v2[Y] + k3 * v3[Y];
	v[Z] = k1 * v1[Z] + k2 * v2[Z] + k3 * v3[Z];
}

// Evaluate a ray equation. [DB 7/94]
//   IPoint = Origin + depth * Direction
inline void VEvaluateRay(VECTOR IPoint, const VECTOR Origin, DBL depth, const VECTOR Direction)
{
	IPoint[X] = Origin[X] + depth * Direction[X];
	IPoint[Y] = Origin[Y] + depth * Direction[Y];
	IPoint[Z] = Origin[Z] + depth * Direction[Z];
}

// Add a scaled vector. [DB 7/94]
//   v  = v1 + k * v2;
//   v += k * v2;
inline void VAddScaled(VECTOR v, const VECTOR v1, DBL k, const VECTOR v2)
{
	v[X] = v1[X] + k * v2[X];
	v[Y] = v1[Y] + k * v2[Y];
	v[Z] = v1[Z] + k * v2[Z];
}

inline void VAddScaledEq(VECTOR v, DBL k, const VECTOR v2)
{
	v[X] += k * v2[X];
	v[Y] += k * v2[Y];
	v[Z] += k * v2[Z];
}

// Inverse Scale - Divide Vector by a Scalar
inline void V4D_InverseScaleEq(VECTOR_4D a, DBL k)
{
	DBL tmp = 1.0 / k;
	a[X] *= tmp;
	a[Y] *= tmp;
	a[Z] *= tmp;
	a[T] *= tmp;
}

// Dot Product - Gives Scalar angle (a) between two vectors (b) and (c)
inline void V4D_Dot(DBL& a, const VECTOR_4D b, const VECTOR_4D c)
{
	a = b[X] * c[X] + b[Y] * c[Y] + b[Z] * c[Z] + b[T] * c[T];
}

}

#endif


