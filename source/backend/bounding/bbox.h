/*******************************************************************************
 * bbox.h
 *
 * This module contains all defines, typedefs, and prototypes for BBOX.CPP.
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
 * $File: //depot/povray/smp/source/backend/bounding/bbox.h $
 * $Revision: #24 $
 * $Change: 5401 $
 * $DateTime: 2011/02/08 21:06:55 $
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

/* NOTE: FRAME.H contains other bound stuff. */

#ifndef BBOX_H
#define BBOX_H

#include <map>

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Generate additional bbox statistics. */

#define BBOX_EXTRA_STATS 1


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef int VECTORI[3];

class Rayinfo
{
	public:
		BBOX_VECT slab_num;
		BBOX_VECT slab_den;
		VECTORI nonzero;
		VECTORI positive;

		explicit Rayinfo(const Ray& ray)
		{
			DBL t;

			slab_num[X] = ray.Origin[X];
			slab_num[Y] = ray.Origin[Y];
			slab_num[Z] = ray.Origin[Z];

			if((nonzero[X] = ((t = ray.Direction[X]) != 0.0)) != 0)
			{
				slab_den[X] = 1.0 / t;
				positive[X] = (ray.Direction[X] > 0.0);
			}

			if((nonzero[Y] = ((t = ray.Direction[Y]) != 0.0)) != 0)
			{
				slab_den[Y] = 1.0 / t;
				positive[Y] = (ray.Direction[Y] > 0.0);
			}

			if((nonzero[Z] = ((t = ray.Direction[Z]) != 0.0)) != 0)
			{
				slab_den[Z] = 1.0 / t;
				positive[Z] = (ray.Direction[Z] > 0.0);
			}
		}
};

/*****************************************************************************
* Global functions
******************************************************************************/

struct PriorityQueue
{
	struct Qelem
	{
		DBL depth;
		BBOX_TREE *node;
	};

	unsigned QSize;
	unsigned Max_QSize;
	Qelem *Queue;

	PriorityQueue();
	~PriorityQueue();
};

void Build_BBox_Tree(BBOX_TREE **Root, size_t numOfFiniteObjects, BBOX_TREE **&Finite, size_t numOfInfiniteObjects, BBOX_TREE **Infinite, size_t& maxfinitecount);
void Build_Bounding_Slabs(BBOX_TREE **Root, vector<ObjectPtr>& objects, unsigned int& numberOfFiniteObjects, unsigned int& numberOfInfiniteObjects, unsigned int& numberOfLightSources);

void Recompute_BBox(BBOX *bbox, const TRANSFORM *trans);
void Recompute_Inverse_BBox(BBOX *bbox, const TRANSFORM *trans);
bool Intersect_BBox_Tree(PriorityQueue& pqueue, const BBOX_TREE *Root, const Ray& ray, Intersection *Best_Intersection, TraceThreadData *Thread);
bool Intersect_BBox_Tree(PriorityQueue& pqueue, const BBOX_TREE *Root, const Ray& ray, Intersection *Best_Intersection, const RayObjectCondition& precondition, const RayObjectCondition& postcondition, TraceThreadData *Thread);
void Check_And_Enqueue(PriorityQueue& Queue, const BBOX_TREE *Node, const BBOX *BBox, Rayinfo *rayinfo, TraceThreadData *Thread);
void Priority_Queue_Delete(PriorityQueue& Queue, DBL *key, const BBOX_TREE **Node);
void Destroy_BBox_Tree(BBOX_TREE *Node);


/*****************************************************************************
* Inline functions
******************************************************************************/

// Calculate the volume of a bounding box. [DB 8/94]
inline void BOUNDS_VOLUME(DBL& a, const BBOX& b)
{
	a = b.Lengths[X] * b.Lengths[Y] * b.Lengths[Z];
}

}

#endif
