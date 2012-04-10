/*******************************************************************************
 * mesh.h
 *
 * This module contains all defines, typedefs, and prototypes for MESH.CPP.
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
 * $File: //depot/povray/smp/source/backend/shape/mesh.h $
 * $Revision: #26 $
 * $Change: 5128 $
 * $DateTime: 2010/08/30 15:57:31 $
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


#ifndef MESH_H
#define MESH_H

#include "backend/bounding/bbox.h"

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define MESH_OBJECT (PATCH_OBJECT+HIERARCHY_OK_OBJECT) // NOTE: During parsing, the PATCH_OBJECT type flag may be cleared if an inside_vector is specified


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Mesh_Data_Struct MESH_DATA;
typedef struct Mesh_Triangle_Struct MESH_TRIANGLE;

typedef struct Hash_Table_Struct HASH_TABLE;
typedef struct UV_Hash_Table_Struct UV_HASH_TABLE;

struct Mesh_Data_Struct
{
	int References;                /* Number of references to the mesh. */
	long Number_Of_UVCoords;       /* Number of UV coords in the mesh.  */
	long Number_Of_Normals;        /* Number of normals in the mesh.    */
	long Number_Of_Triangles;      /* Number of trinagles in the mesh.  */
	long Number_Of_Vertices;       /* Number of vertices in the mesh.   */
	SNGL_VECT *Normals, *Vertices; /* Arrays of normals and vertices.   */
	UV_VECT *UVCoords;             /* Array of UV coordinates           */
	MESH_TRIANGLE *Triangles;      /* Array of triangles.               */
	const BBOX_TREE *Tree;         /* Bounding box tree for mesh.       */
	VECTOR Inside_Vect;            /* vector to use to test 'inside'    */
};

struct Mesh_Triangle_Struct
{
	unsigned int Smooth:1;         /* Is this a smooth triangle.            */
	unsigned int Dominant_Axis:2;  /* Dominant axis.                        */
	unsigned int vAxis:2;          /* Axis for smooth triangle.             */
	unsigned int ThreeTex:1;       /* Color Triangle Patch.                 */
	long Normal_Ind;               /* Index of unsmoothed triangle normal.  */
	long P1, P2, P3;               /* Indices of triangle vertices.         */
	long Texture;                  /* Index of triangle texture.            */
	long Texture2, Texture3;       /* Color Triangle Patch.                 */
	long N1, N2, N3;               /* Indices of smoothed triangle normals. */
	long UV1, UV2, UV3;            /* Indicies of UV coordinate vectors     */
	SNGL Distance;                 /* Distance of triangle along normal.    */
	SNGL_VECT Perp;                /* Vector used for smooth triangles.     */
};

struct Hash_Table_Struct
{
	int Index;
	SNGL_VECT P;
	HASH_TABLE *Next;
};

struct UV_Hash_Table_Struct
{
	int Index;
	UV_VECT P;
	UV_HASH_TABLE *Next;
};

class Mesh : public ObjectBase
{
	public:
		MESH_DATA *Data;               /* Mesh data holding triangles.    */
		long Number_Of_Textures;       /* Number of textures in the mesh.   */
		TEXTURE **Textures;            /* Array of texture references.      */
		short has_inside_vector;

		Mesh();
		virtual ~Mesh();

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

		void Test_Mesh_Opacity();

		void Create_Mesh_Hash_Tables();
		bool Compute_Mesh_Triangle(MESH_TRIANGLE *Triangle, int Smooth, VECTOR P1, VECTOR P2, VECTOR P3, VECTOR S_Normal);
		void Build_Mesh_BBox_Tree();
		bool Degenerate(VECTOR P1, VECTOR P2, VECTOR P3);
		void Init_Mesh_Triangle(MESH_TRIANGLE *Triangle);
		void Destroy_Mesh_Hash_Tables();
		int Mesh_Hash_Vertex(int *Number_Of_Vertices, int *Max_Vertices, SNGL_VECT **Vertices, VECTOR Vertex);
		int Mesh_Hash_Normal(int *Number_Of_Normals, int *Max_Normals, SNGL_VECT **Normals, VECTOR Normal);
		int Mesh_Hash_Texture(int *Number_Of_Textures, int *Max_Textures, TEXTURE ***Textures, TEXTURE *Texture);
		int Mesh_Hash_UV(int *Number, int *Max, UV_VECT **Elements, UV_VECT aPoint);
		void Smooth_Mesh_Normal(VECTOR Result, const MESH_TRIANGLE *Triangle, const VECTOR IPoint) const;

		void Determine_Textures(Intersection *, bool, WeightedTextureVector&, TraceThreadData *);
	protected:
		bool Intersect(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread);
		void Compute_Mesh_BBox();
		void MeshUV(const VECTOR P, const MESH_TRIANGLE *Triangle, UV_VECT Result) const;
		void compute_smooth_triangle(MESH_TRIANGLE *Triangle, const VECTOR P1, const VECTOR P2, const VECTOR P3);
		bool intersect_mesh_triangle(const Ray& ray, const MESH_TRIANGLE *Triangle, DBL *Depth) const;
		bool test_hit(const MESH_TRIANGLE *Triangle, const Ray& OrigRay, DBL Depth, DBL len, IStack& Depth_Stack, TraceThreadData *Thread);
		void get_triangle_bbox(const MESH_TRIANGLE *Triangle, BBOX *BBox) const;
		bool intersect_bbox_tree(const Ray& ray, const Ray& Orig_Ray, DBL len, IStack& Depth_Stack, TraceThreadData *Thread);
		bool inside_bbox_tree(Ray& ray, TraceThreadData *Thread) const;
		void get_triangle_vertices(const MESH_TRIANGLE *Triangle, VECTOR P1, VECTOR P2, VECTOR P3) const;
		void get_triangle_normals(const MESH_TRIANGLE *Triangle, VECTOR N1, VECTOR N2, VECTOR N3) const;
		void get_triangle_uvcoords(const MESH_TRIANGLE *Triangle, UV_VECT U1, UV_VECT U2, UV_VECT U3) const;
		static int mesh_hash(HASH_TABLE **Hash_Table, int *Number, int *Max, SNGL_VECT **Elements, VECTOR aPoint);

private:
		// these are used temporarily during parsing and are destroyed
		// when the parser has finished constructing the object
		static HASH_TABLE **Vertex_Hash_Table;
		static HASH_TABLE **Normal_Hash_Table;
		static UV_HASH_TABLE **UV_Hash_Table;
};

}

#endif
