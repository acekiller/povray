/*******************************************************************************
 * blob.h
 *
 * This module contains all defines, typedefs, and prototypes for BLOB.CPP.
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
 * $File: //depot/povray/smp/source/backend/shape/blob.h $
 * $Revision: #25 $
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


#ifndef BLOB_H
#define BLOB_H

#include "backend/bounding/bsphere.h"

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define BLOB_OBJECT (STURM_OK_OBJECT+HIERARCHY_OK_OBJECT)

/* Do not use the first bit!!! (Used for enter/exit in intersection test) */

#define BLOB_SPHERE               2
#define BLOB_CYLINDER             4
#define BLOB_ELLIPSOID            8
#define BLOB_BASE_HEMISPHERE     16
#define BLOB_APEX_HEMISPHERE     32
#define BLOB_BASE_HEMIELLIPSOID  64
#define BLOB_APEX_HEMIELLIPSOID 128


/* Define max. number of blob components. */

#define MAX_BLOB_COMPONENTS 1000000

/* Generate additional blob statistics. */

#define BLOB_EXTRA_STATS 1



/*****************************************************************************
* Global typedefs
******************************************************************************/

class Blob_Element
{
	public:
		short Type;       /* Type of component: sphere, hemisphere, cylinder */
		int index;
		VECTOR O;         /* Element's origin                                */
		DBL len;          /* Cylinder's length                               */
		DBL rad2;         /* Sphere's/Cylinder's radius^2                    */
		DBL c[3];         /* Component's coeffs                              */
		TEXTURE *Texture; /* Component's texture                             */
		TRANSFORM *Trans; /* Component's transformation                      */

		Blob_Element();
		~Blob_Element();
};

class Blob_Data
{
	public:
		int Number_Of_Components;   /* Number of components     */
		DBL Threshold;              /* Blob threshold           */
		vector<Blob_Element> Entry; /* Array of blob components */
		BSPHERE_TREE *Tree;         /* Bounding hierarchy       */

		Blob_Data(int count = 0);
		~Blob_Data();

		Blob_Data *AcquireReference(void);
		void ReleaseReference(void);

	private:
		int References;             /* Number of references     */
};

struct Blob_List_Struct
{
	Blob_Element elem;  /* Current element          */
	Blob_List_Struct *next;    /* Pointer to next element  */
};

struct Blob_Interval_Struct
{
	int type;
	DBL bound;
	const Blob_Element *Element;
};

class Blob : public ObjectBase
{
	public:
		Blob_Data *Data;
		TEXTURE **Element_Texture;

		Blob();
		virtual ~Blob();

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

		void Determine_Textures(Intersection *, bool, WeightedTextureVector&, TraceThreadData *);

		Blob_List_Struct *Create_Blob_List_Element();
		void Create_Blob_Element_Texture_List(Blob_List_Struct *BlobList, int npoints);
		int Make_Blob(DBL threshold, Blob_List_Struct *bloblist, int npoints, TraceThreadData *Thread);

		void Test_Blob_Opacity();

		static void Translate_Blob_Element(Blob_Element *Element, VECTOR Vector);
		static void Rotate_Blob_Element(Blob_Element *Element, VECTOR Vector);
		static void Scale_Blob_Element(Blob_Element *Element, VECTOR Vector);
		static void Invert_Blob_Element(Blob_Element *Element);
		static void Transform_Blob_Element(Blob_Element *Element, TRANSFORM *Trans);
	private:
		static void element_normal(VECTOR Result, const VECTOR P, const Blob_Element *Element);
		static int intersect_element(const VECTOR P, const VECTOR D, const Blob_Element *Element, DBL mindist, DBL *t0, DBL *t1, TraceThreadData *Thread);
		static void insert_hit(const Blob_Element *Element, DBL t0, DBL t1, Blob_Interval_Struct *intervals, unsigned int *cnt);
		int determine_influences(const VECTOR P, const VECTOR D, DBL mindist, Blob_Interval_Struct *intervals, TraceThreadData *Thread) const;
		DBL calculate_field_value(const VECTOR P, TraceThreadData *Thread) const;
		static DBL calculate_element_field(const Blob_Element *Element, const VECTOR P);

		static int intersect_cylinder(const Blob_Element *Element, const VECTOR P, const VECTOR D, DBL mindist, DBL *tmin, DBL *tmax);
		static int intersect_hemisphere(const Blob_Element *Element, const VECTOR P, const VECTOR D, DBL mindist, DBL *tmin, DBL *tmax);
		static int intersect_sphere(const Blob_Element *Element, const VECTOR P, const VECTOR D, DBL mindist, DBL *tmin, DBL *tmax);
		static int intersect_ellipsoid(const Blob_Element *Element, const VECTOR P, const VECTOR D, DBL mindist, DBL *tmin, DBL *tmax);

		static void get_element_bounding_sphere(const Blob_Element *Element, VECTOR Center, DBL *Radius2);
		void build_bounding_hierarchy();

		void determine_element_texture(const Blob_Element *Element, TEXTURE *Texture, const VECTOR P, WeightedTextureVector&);

		static bool insert_node(BSPHERE_TREE *Node, unsigned int *size, TraceThreadData *Thread);

		void getLocalIPoint(VECTOR lip, Intersection *isect) const;
};

}

#endif
