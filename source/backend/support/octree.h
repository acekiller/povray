/*******************************************************************************
 * octree.h
 *
 * Oct-tree routine prototypes.  Use by Radiosity calculation routies.
 *
 * Implemented by and (c) 1994 Jim McElhiney, mcelhiney@acm.org or cserve 71201,1326
 * All standard POV distribution rights granted.  All other rights reserved.
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
 * $File: //depot/povray/smp/source/backend/support/octree.h $
 * $Revision: #20 $
 * $Change: 5209 $
 * $DateTime: 2010/11/28 18:25:20 $
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

#ifndef OCTREE_H
#define OCTREE_H

#include "backend/frame.h"
#include "base/fileinputoutput.h"

namespace pov
{
using namespace pov_base;

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define OT_BIAS 10000000.


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef unsigned short OT_TILE;
#define OT_TILE_MAX USHRT_MAX

typedef unsigned char OT_PASS;
#define OT_PASS_INVALID 0
#define OT_PASS_FIRST   1
#define OT_PASS_FINAL   UCHAR_MAX
#define OT_PASS_MAX     (OT_PASS_FINAL-2) // OT_PASS_FINAL-1 is reserved

typedef unsigned char OT_DEPTH;
#define OT_DEPTH_MAX UCHAR_MAX

typedef struct ot_block_struct OT_BLOCK;
typedef struct ot_id_struct OT_ID;
typedef struct ot_node_struct OT_NODE;
typedef struct ot_read_param_struct OT_READ_PARAM;
typedef struct ot_read_info_struct OT_READ_INFO;

// Each node in the oct-tree has a (possibly null) linked list of these data blocks off it.
struct ot_block_struct
{
	// TODO for memory efficiency we could probably use single-precision data types for the vector stuff
	OT_BLOCK    *next;      // next block in the same node
	Vector3d    Point;
	Vector3d    S_Normal;
	Vector3d    To_Nearest_Surface;
	RGBColour   dx, dy, dz; // gradients, not colors, but used only to manipulate colors [trf]
	RGBColour   Illuminance;
	SNGL        Harmonic_Mean_Distance;
	SNGL        Nearest_Distance;
	SNGL        Quality;    // quality of the data from which this sample was aggregated
	OT_TILE     TileId;     // tile in which this sample was taken
	OT_PASS     Pass;       // pass during which this sample was taken (OT_PASS_FINAL for final render)
	OT_DEPTH    Bounce_Depth;
};

// This is the information necessary to name an oct-tree node.
struct ot_id_struct
{
	int x, y, z;
	int Size;
};

// These are the structures that make up the oct-tree itself, known as nodes
struct ot_node_struct
{
	OT_ID    Id;
	OT_BLOCK *Values;
	OT_NODE  *Kids[8];
};

// These are informations the octree reader needs to know
struct ot_read_param_struct
{
	DBL       RealErrorBound;
};

// These are informations the octree reader generates
struct ot_read_info_struct
{
	RGBColour Gather_Total;
	long      Gather_Total_Count;
	DBL       Brightness;
	bool      FirstRadiosityPass;
};

/*****************************************************************************
* Global functions
******************************************************************************/

void ot_ins (OT_NODE **root, OT_BLOCK *new_block, OT_ID *new_id);
bool ot_dist_traverse (OT_NODE *subtree, const Vector3d& point, int bounce_depth, bool (*func)(OT_BLOCK *block, void *handle1), void *handle2);
void ot_index_sphere (const Vector3d& point, DBL radius, OT_ID *id);
void ot_index_box (const Vector3d& min_point, const Vector3d& max_point, OT_ID *id);
bool ot_save_tree (OT_NODE *root, OStream *fd);
bool ot_write_block (OT_BLOCK *bl, void * handle);
bool ot_free_tree (OT_NODE **root_ptr);
bool ot_read_file (OT_NODE **root, IStream * fd, const OT_READ_PARAM* param, OT_READ_INFO* info);
void ot_newroot (OT_NODE **root_ptr);
void ot_parent (OT_ID *dad, OT_ID *kid);

}

#endif
