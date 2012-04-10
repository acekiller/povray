/*******************************************************************************
 * sphsweep.h
 *
 * Contributed by Jochen Lippert
 *
 * This module contains the global defines, typedefs, and prototypes
 * for sphsweep.cpp.
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
 * $File: //depot/povray/smp/source/backend/shape/sphsweep.h $
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


#ifndef SPHERE_SWEEP_H
#define SPHERE_SWEEP_H

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define SPHERE_SWEEP_OBJECT     (BASIC_OBJECT)

/* Sphere sweep interpolated by a piecewise linear function */
#define LINEAR_SPHERE_SWEEP             0

/* Sphere sweep interpolated by a cubic Catmull-Rom-Spline function */
#define CATMULL_ROM_SPLINE_SPHERE_SWEEP 1

/* Sphere sweep approximated by a cubic B-Spline function */
#define B_SPLINE_SPHERE_SWEEP           2

/* Maximum number of coefficients of the polynomials describing one segment */
#define SPH_SWP_MAX_COEFS               4



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Sphere_Sweep_Sphere_Struct SPHSWEEP_SPH;
typedef struct Sphere_Sweep_Segment_Struct SPHSWEEP_SEG;
typedef struct Sphere_Sweep_Intersection_Structure SPHSWEEP_INT;

/* Single sphere, used to connect two adjacent segments */
struct Sphere_Sweep_Sphere_Struct
{
	VECTOR      Center;
	DBL         Radius;
};

/* One segment of the sphere sweep */
struct Sphere_Sweep_Segment_Struct
{
	SPHSWEEP_SPH  Closing_Sphere[2];        /* Spheres closing the segment   */
	VECTOR  Center_Deriv[2];    /* Derivatives of center funcs for 0 and 1   */
	DBL     Radius_Deriv[2];    /* Derivatives of radius funcs for 0 and 1   */
	int     Num_Coefs;                      /* Number of coefficients        */
	VECTOR  Center_Coef[SPH_SWP_MAX_COEFS]; /* Coefs of center polynomial    */
	DBL     Radius_Coef[SPH_SWP_MAX_COEFS]; /* Coefs of radius polynomial    */
};

// Temporary storage for intersection values
struct Sphere_Sweep_Intersection_Structure
{
	DBL     t;          // Distance along ray
	VECTOR  Point;      // Intersection point
	VECTOR  Normal;     // Normal at intersection point
};

/* The complete object */
class SphereSweep : public ObjectBase
{
	public:
		int             Interpolation;
		int             Num_Modeling_Spheres;   /* Number of modeling spheres    */
		SPHSWEEP_SPH    *Modeling_Sphere;       /* Spheres describing the shape  */
		int             Num_Spheres;            /* Number of single spheres      */
		SPHSWEEP_SPH    *Sphere;                /* Spheres that close segments   */
		int             Num_Segments;           /* Number of tubular segments    */
		SPHSWEEP_SEG    *Segment;               /* Tubular segments              */
		DBL             Depth_Tolerance;        /* Preferred depth tolerance     */

		SphereSweep();
		virtual ~SphereSweep();

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

		void Compute();
	protected:
		bool Intersect(Ray& ray, VECTOR Center, DBL Radius2, DBL *Depth1, DBL *Depth2);
		static bool Intersect_Sphere(const Ray &ray, const SPHSWEEP_SPH *Sphere, SPHSWEEP_INT *Isect);
		static int Intersect_Segment(const Ray &ray, const SPHSWEEP_SEG *Segment, SPHSWEEP_INT *Isect, TraceThreadData *Thread);
		static int Find_Valid_Points(SPHSWEEP_INT *Inter, int Num_Inter, const Ray &ray);
		static int Comp_Isects(const void *Intersection_1, const void *Intersection_2);
		static int bezier_01(int degree, const DBL* Coef, DBL* Roots, bool sturm, DBL tolerance, TraceThreadData *Thread);
};

}

#endif
