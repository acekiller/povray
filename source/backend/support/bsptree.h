/*******************************************************************************
 * bsptree.h
 *
 * This file contains ... TODO ...
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
 * $File: //depot/povray/smp/source/backend/support/bsptree.h $
 * $Revision: #44 $
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

#ifndef POVRAY_BACKEND_BSPTREE_H
#define POVRAY_BACKEND_BSPTREE_H

#include <vector>
#include <list>
#include <cstdio>

#include "backend/frame.h"
#include "backend/math/vector.h"

namespace pov
{

class BSPTree;

class BSPTree
{
	public:
		class Mailbox
		{
				friend class BSPTree;
			public:
				inline Mailbox(unsigned int range) : objects((range >> 5) + 1), count(0) { }
				inline void clear() { count = 0; memset(&objects[0], 0, objects.size() * sizeof(unsigned int)); } // using memset here as std::fill may not be fast with every standard libaray [trf]
				inline unsigned int size() const { return count; }

				inline bool insert(unsigned int i)
				{
					if((objects[i >> 5] & (1 << (i & 31))) == 0)
					{
						objects[i >> 5] |= (1 << (i & 31));
						count++;
						return true;
					}
					return false;
				}
			private:
				/// bit marking object (by index) in mailbox
				vector<unsigned int> objects;
				/// number of objects in mailbox
				unsigned int count;

				/// unavailable
				Mailbox();
		};

		class Objects
		{
			public:
				Objects() { }
				virtual ~Objects() { }

				virtual unsigned int size() const = 0;
				virtual float GetMin(unsigned int axis, unsigned int) const = 0;
				virtual float GetMax(unsigned int axis, unsigned int) const = 0;
		};

		class Intersect
		{
			public:
				Intersect() { }
				virtual ~Intersect() { }

				virtual bool operator()(unsigned int index, double& maxdist) = 0;
				virtual bool operator()() const = 0;
		};

		class Inside
		{
			public:
				Inside() { }
				virtual ~Inside() { }

				virtual bool operator()(unsigned int index) = 0;
				virtual bool operator()() const = 0;
		};

		class Progress
		{
			public:
				Progress() { }
				virtual ~Progress() { }

				virtual void operator()(unsigned int nodes) const = 0;
		};

		BSPTree(unsigned int md = 0, float oic = 0.0f, float bac = 0.0f, float cac = 0.0f, float mc = 0.0f);
		virtual ~BSPTree();

		bool operator()(const Ray& ray, Intersect& isect, Mailbox& mailbox, double maxdist);
		bool operator()(const Vector3d& origin, Inside& inside, Mailbox& mailbox, bool earlyExit = false);

		void build(const Progress& progress, const Objects& objects,
		           unsigned int& nodes, unsigned int& splitNodes, unsigned int& objectNodes, unsigned int& emptyNodes,
		           unsigned int& maxObjects, float& averageObjects, unsigned int& maxDepth, float& averageDepth,
		           unsigned int& aborts, float& averageAborts, float& averageAbortObjects, const UCS2String& inputFile);

		void clear();
	private:
		struct Node
		{
			enum NodeType
			{
				Split = 0,
				Object = 1
			};

			// data if type is split node
			enum SplitData
			{
				AxisX = 0,
				AxisY = 1,
				AxisZ = 2,
				NoAxis = 3
			};

			// data if type is object node
			enum ObjectData
			{
				Empty = 0,
				SingleObject = 1,
				DoubleObject = 2,
				ObjectList = 3
			};

			// four bytes
			unsigned type : 1;
			unsigned data : 2;
			unsigned index : 29;

			// four bytes (required)
			union
			{
				float plane;
				unsigned int index2;
			};
		};

		struct Split
		{
			enum Side
			{
				Min = 0,
				Max = 1
			};

			// four bytes
			unsigned se : 1;
			unsigned index : 31;

			// four bytes (required)
			float plane;

			inline Split() : se(Min), index(0), plane(0.0f) { }
			inline Split(Side s, unsigned int i, float p) : se(s), index(i), plane(p) { }

			inline bool operator<(const Split& r) const { return (plane < r.plane); }

			struct CompareIndex
			{
				inline bool operator()(const Split& left, const Split& right) const { return (left.index < right.index); }
			};
		};

		struct TraceStack
		{
			unsigned int inode;
			float rentry;
			float rexit;
		};

		/// array of all nodes
		vector<Node> nodes;
		/// array of all object pointer lists
		vector<unsigned int> lists;
		/// splits, only used while building tree
		vector<Split> splits[3];
		/// lower left corner of bounding box
		Vector3d bmin;
		/// upper right corner of bounding box
		Vector3d bmax;
		/// user-defined max depth (<= MAX_BSP_TREE_LEVEL)
		const unsigned int maxDepth;
		/// user-defined object intersection cost
		const float objectIsectCost;
		/// user-defined base access cost
		const float baseAccessCost;
		/// user-defined child access cost
		const float childAccessCost;
		/// user-define miss chance
		const float missChance;
		/// last node progress counter
		unsigned int lastProgressNodeCounter;
		/// maximum objects in node
		unsigned int maxObjectsInNode;
		/// maximum tree depth
		unsigned int maxTreeDepth;
		/// maximum tree depth nodes
		unsigned int maxTreeDepthNodes;
		/// empty node counter
		unsigned int emptyNodeCounter;
		/// object node counter
		unsigned int objectNodeCounter;
		/// objects in tree counter
		POV_LONG objectsInTreeCounter;
		/// objects at maximum depth counter
		POV_LONG objectsAtMaxDepthCounter;
		/// tree depth counter
		POV_LONG treeDepthCounter;
		/// object index list (only used while building tree)
		vector<unsigned int> indices;

		void BuildRecursive(const Progress& progress, const Objects& objects, unsigned int inode, unsigned int indexbegin, unsigned int indexend, BBOX& cell, unsigned int maxlevel);
		void SetObjectNode(unsigned int inode, unsigned int indexbegin, unsigned int indexend);

		void ReadRecursive(const Progress& progress, FILE *infile, unsigned int inode, unsigned int level, unsigned int maxIndex);
		char *GetLine(char *str, int len, FILE *infile);
		void ValidateBounds(FILE *infile, const Objects& objects);
};

}

#endif // POVRAY_BACKEND_BSPTREE_H
