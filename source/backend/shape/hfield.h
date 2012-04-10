/*******************************************************************************
 * hfield.h
 *
 * This module contains all defines, typedefs, and prototypes for HFIELD.CPP.
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
 * $File: //depot/povray/smp/source/backend/shape/hfield.h $
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

#ifndef HFIELD_H
#define HFIELD_H

#include "backend/bounding/bbox.h"
#include "backend/shape/boxes.h"

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define HFIELD_OBJECT (BASIC_OBJECT+HIERARCHY_OK_OBJECT)

/* Generate additional height field statistics. */

#define HFIELD_EXTRA_STATS 1


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct HField_Data_Struct HFIELD_DATA;
typedef struct HField_Block_Struct HFIELD_BLOCK;
typedef struct HField_Normal_Struct HFIELD_NORMAL;
typedef short HF_Normals[3];

struct HField_Normal_Struct
{
	DBL fx, fz;
	VECTOR normal;
};

struct HField_Block_Struct
{
	int xmin, xmax;
	int zmin, zmax;
	DBL ymin, ymax;
};

struct HField_Data_Struct
{
	int References;
	int Normals_Height;  /* Needed for Destructor */
	int max_x, max_z;
	HF_VAL min_y, max_y;
	int block_max_x, block_max_z;
	int block_width_x, block_width_z;
	HF_VAL **Map;
	HF_Normals **Normals;
	HFIELD_BLOCK **Block;
};

class HField : public ObjectBase
{
	public:
		VECTOR bounding_corner1;
		VECTOR bounding_corner2;
		HFIELD_DATA *Data;

		HField();
		virtual ~HField();

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

		void Compute_HField(ImageData *image);
	protected:
		static DBL normalize(VECTOR A, const VECTOR B);
		void smooth_height_field(int xsize, int zsize);
		bool intersect_pixel(int x, int z, const Ray& ray, DBL height1, DBL height2, IStack &HField_Stack, const Ray &RRay, DBL mindist, DBL maxdist, TraceThreadData *Thread);
		static int add_single_normal(HF_VAL **data, int xsize, int zsize, int x0, int z0,int x1, int z1,int x2, int z2, VECTOR N);
		bool dda_traversal(const Ray &ray, const VECTOR Start, const HFIELD_BLOCK *Block, IStack &HField_Stack, const Ray &RRay, DBL mindist, DBL maxdist, TraceThreadData *Thread);
		bool block_traversal(const Ray &ray, const VECTOR Start, IStack &HField_Stack, const Ray &RRay, DBL mindist, DBL maxdist, TraceThreadData *Thread);
		void build_hfield_blocks();
};

}

#endif
