/*******************************************************************************
 * bezier.h
 *
 * This module contains all defines, typedefs, and prototypes for BEZIER.CPP.
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
 * $File: //depot/povray/smp/source/backend/shape/bezier.h $
 * $Revision: #17 $
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


#ifndef BEZIER_H
#define BEZIER_H

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define BICUBIC_PATCH_OBJECT (PATCH_OBJECT)
/* NK 1998 double_illuminate - removed +DOUBLE_ILLUMINATE from bicubic_patch */

#define BEZIER_INTERIOR_NODE 0
#define BEZIER_LEAF_NODE 1

#define MAX_PATCH_TYPE 2



/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef DBL DISTANCES[4][4];
typedef DBL WEIGHTS[4][4];
typedef struct Bezier_Node_Struct BEZIER_NODE;
typedef struct Bezier_Child_Struct BEZIER_CHILDREN;
typedef struct Bezier_Vertices_Struct BEZIER_VERTICES;

struct Bezier_Child_Struct
{
	BEZIER_NODE *Children[4];
};

struct Bezier_Vertices_Struct
{
	float uvbnds[4];
	VECTOR Vertices[4];
};

struct Bezier_Node_Struct
{
	int Node_Type;      /* Is this an interior node, or a leaf */
	VECTOR Center;      /* Center of sphere bounding the (sub)patch */
	DBL Radius_Squared; /* Radius of bounding sphere (squared) */
	int Count;          /* # of subpatches associated with this node */
	void *Data_Ptr;     /* Either pointer to vertices or pointer to children */
};

class BicubicPatch : public ObjectBase
{
	public:
		int Patch_Type, U_Steps, V_Steps;
		VECTOR Control_Points[4][4];
		UV_VECT ST[4];
		VECTOR Bounding_Sphere_Center;
		DBL Bounding_Sphere_Radius;
		DBL Flatness_Value;
		DBL accuracy;
		BEZIER_NODE *Node_Tree;
		WEIGHTS *Weights;

		BicubicPatch();
		virtual ~BicubicPatch();

		virtual ObjectPtr Copy();

		virtual bool All_Intersections(const Ray&, IStack&, TraceThreadData *);
		virtual bool Inside(const VECTOR, TraceThreadData *) const;
		virtual void Normal(VECTOR, Intersection *, TraceThreadData *) const;
		virtual void UVCoord(UV_VECT, const Intersection *, TraceThreadData *) const;
		virtual void Translate(const VECTOR, const TRANSFORM *);
		virtual void Rotate(const VECTOR, const TRANSFORM *);
		virtual void Scale(const VECTOR, const TRANSFORM *);
		virtual void Transform(const TRANSFORM *);
		virtual void Invert();
		virtual void Compute_BBox();

		void Precompute_Patch_Values();
	protected:
		static void bezier_value(const VECTOR(*Control_Points)[4][4], DBL u0, DBL v0, VECTOR P, VECTOR N);
		bool intersect_subpatch(const Ray&, const VECTOR [3], const DBL [3], const DBL [3], DBL *, VECTOR, VECTOR, DBL *, DBL *) const;
		static void find_average(int, const VECTOR *, VECTOR, DBL *);
		static bool spherical_bounds_check(const Ray &, const VECTOR, DBL);
		int intersect_bicubic_patch0(const Ray& , IStack&, TraceThreadData *);
		static DBL point_plane_distance(const VECTOR, const VECTOR, DBL);
		static DBL determine_subpatch_flatness(const VECTOR(*)[4][4]);
		bool flat_enough(const VECTOR(*)[4][4]) const;
		static void bezier_bounding_sphere(const VECTOR(*)[4][4], VECTOR, DBL *);
		int bezier_subpatch_intersect(const Ray &, const VECTOR(*)[4][4], DBL, DBL, DBL, DBL, IStack&, TraceThreadData *);
		static void bezier_split_left_right(const VECTOR(*)[4][4], VECTOR(*)[4][4], VECTOR(*)[4][4]);
		static void bezier_split_up_down(const VECTOR(*)[4][4], VECTOR(*)[4][4], VECTOR(*)[4][4]);
		int bezier_subdivider(const Ray &, const VECTOR(*)[4][4], DBL, DBL, DBL, DBL, int, IStack&, TraceThreadData *);
		static void bezier_tree_deleter(BEZIER_NODE *Node);
		BEZIER_NODE *bezier_tree_builder(const VECTOR(*Patch)[4][4], DBL u0, DBL u1, DBL v0, DBL v1, int depth, int& max_depth_reached);
		int bezier_tree_walker(const Ray &, const BEZIER_NODE *, IStack&, TraceThreadData *);
		static BEZIER_NODE *create_new_bezier_node(void);
		static BEZIER_VERTICES *create_bezier_vertex_block(void);
		static BEZIER_CHILDREN *create_bezier_child_block(void);
		static bool subpatch_normal(const VECTOR v1, const VECTOR v2, const VECTOR v3, VECTOR Result, DBL *d);
		static void Compute_Texture_UV(const UV_VECT p, const UV_VECT st[4], UV_VECT t);
};



}

#endif
