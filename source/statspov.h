/*******************************************************************************
 * statspov.h
 *
 * This module contains all defines, typedefs, and prototypes for statspov.cpp.
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
 * $File: //depot/povray/smp/source/statspov.h $
 * $Revision: #28 $
 * $Change: 5215 $
 * $DateTime: 2010/11/30 18:41:01 $
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

#ifndef STATSPOV_H
#define STATSPOV_H

#include "base/types.h"
#include "backend/frame.h"

namespace pov
{

typedef enum INTSTATS
{
	/* Computations are performed on these three */
	Number_Of_Pixels = 0,
	Number_Of_Pixels_Supersampled,
	Number_Of_Samples,
	Number_Of_Rays,
	Calls_To_DNoise,
	Calls_To_Noise,
	ADC_Saves,

	/* intersecion stack */
	Istack_overflows,

	/* objects */
	Ray_RBezier_Tests,
	Ray_RBezier_Tests_Succeeded,
	Ray_Bicubic_Tests,
	Ray_Bicubic_Tests_Succeeded,
	Ray_Blob_Tests,
	Ray_Blob_Tests_Succeeded,
	Blob_Element_Tests,
	Blob_Element_Tests_Succeeded,
	Blob_Bound_Tests,
	Blob_Bound_Tests_Succeeded,
	Ray_Box_Tests,
	Ray_Box_Tests_Succeeded,
	Ray_Cone_Tests,
	Ray_Cone_Tests_Succeeded,
	Ray_CSG_Intersection_Tests,
	Ray_CSG_Intersection_Tests_Succeeded,
	Ray_CSG_Merge_Tests,
	Ray_CSG_Merge_Tests_Succeeded,
	Ray_CSG_Union_Tests,
	Ray_CSG_Union_Tests_Succeeded,
	Ray_Disc_Tests,
	Ray_Disc_Tests_Succeeded,
	Ray_Fractal_Tests,
	Ray_Fractal_Tests_Succeeded,
	Ray_HField_Tests,
	Ray_HField_Tests_Succeeded,
	Ray_HField_Box_Tests,
	Ray_HField_Box_Tests_Succeeded,
	Ray_HField_Triangle_Tests,
	Ray_HField_Triangle_Tests_Succeeded,
	Ray_HField_Block_Tests,
	Ray_HField_Block_Tests_Succeeded,
	Ray_HField_Cell_Tests,
	Ray_HField_Cell_Tests_Succeeded,
	Ray_IsoSurface_Tests,
	Ray_IsoSurface_Tests_Succeeded,
	Ray_IsoSurface_Bound_Tests,
	Ray_IsoSurface_Bound_Tests_Succeeded,
	Ray_IsoSurface_Cache,
	Ray_IsoSurface_Cache_Succeeded,
	Ray_Lathe_Tests,
	Ray_Lathe_Tests_Succeeded,
	Lathe_Bound_Tests,
	Lathe_Bound_Tests_Succeeded,
	Ray_Mesh_Tests,
	Ray_Mesh_Tests_Succeeded,
	Ray_Ovus_Tests,
	Ray_Ovus_Tests_Succeeded,
	Ray_Plane_Tests,
	Ray_Plane_Tests_Succeeded,
	Ray_Polygon_Tests,
	Ray_Polygon_Tests_Succeeded,
	Ray_Prism_Tests,
	Ray_Prism_Tests_Succeeded,
	Prism_Bound_Tests,
	Prism_Bound_Tests_Succeeded,
	Ray_Parametric_Tests,
	Ray_Parametric_Tests_Succeeded,
	Ray_Par_Bound_Tests,
	Ray_Par_Bound_Tests_Succeeded,
	Ray_Quadric_Tests,
	Ray_Quadric_Tests_Succeeded,
	Ray_Poly_Tests,
	Ray_Poly_Tests_Succeeded,
	Ray_Sphere_Tests,
	Ray_Sphere_Tests_Succeeded,
	Ray_Sphere_Sweep_Tests,
	Ray_Sphere_Sweep_Tests_Succeeded,
	Ray_Superellipsoid_Tests,
	Ray_Superellipsoid_Tests_Succeeded,
	Ray_Sor_Tests,
	Ray_Sor_Tests_Succeeded,
	Sor_Bound_Tests,
	Sor_Bound_Tests_Succeeded,
	Ray_Torus_Tests,
	Ray_Torus_Tests_Succeeded,
	Torus_Bound_Tests,
	Torus_Bound_Tests_Succeeded,
	Ray_Triangle_Tests,
	Ray_Triangle_Tests_Succeeded,
	Ray_TTF_Tests,
	Ray_TTF_Tests_Succeeded,

	/* crackle cache */
	CrackleCache_Tests,
	CrackleCache_Tests_Succeeded,

	/* bounding etc */
	Bounding_Region_Tests,
	Bounding_Region_Tests_Succeeded,
	Clipping_Region_Tests,
	Clipping_Region_Tests_Succeeded,

	/* isosurface and functions */
	Ray_IsoSurface_Find_Root,
	Ray_Function_VM_Calls,
	Ray_Function_VM_Instruction_Est,

	/* Vista and light buffer */
	VBuffer_Tests,
	VBuffer_Tests_Succeeded,
	LBuffer_Tests,
	LBuffer_Tests_Succeeded,

	/* Media */
	Media_Samples,
	Media_Intervals,

	/* Ray */
	Reflected_Rays_Traced,
	Refracted_Rays_Traced,
	Transmitted_Rays_Traced,
	Internal_Reflected_Rays_Traced,
	Shadow_Cache_Hits,
	Shadow_Rays_Succeeded,
	Shadow_Ray_Tests,

	nChecked,
	nEnqueued,
	totalQueues,
	totalQueueResets,
	totalQueueResizes,
	Polynomials_Tested,
	Roots_Eliminated,

#if defined(MEM_STATS)
	MemStat_Smallest_Alloc,
	MemStat_Largest_Alloc,
	MemStat_Largest_Mem_Usage,
#if (MEM_STATS>=2)
	MemStat_Total_Allocs,
	MemStat_Total_Frees,
#endif
#endif

	/* NK phmap */
	Number_Of_Photons_Shot,
	Number_Of_Photons_Stored,
	Number_Of_Global_Photons_Stored,
	Number_Of_Media_Photons_Stored,
	Priority_Queue_Add,
	Priority_Queue_Remove,
	Gather_Performed_Count,
	Gather_Expanded_Count,

	// [CLi] radiosity total stats (all pre- & final traces, all recursion depths)
	Radiosity_ReuseCount,             // ambient value queries satisfied without taking a new sample
	Radiosity_GatherCount,            // number of samples gathered
	Radiosity_UnsavedCount,           // number of samples gathered but not stored in cache
	Radiosity_RayCount,               // number of rays shot to gather samples
	Radiosity_OctreeNodes,            // number of nodes in octree
	Radiosity_OctreeLookups,          // number of blocks examined for sample lookup
	Radiosity_OctreeAccepts0,         // number of blocks accepted by pass & tile id check
	Radiosity_OctreeAccepts1,         // number of blocks accepted by quick out-of-range check
	Radiosity_OctreeAccepts2,         // number of blocks accepted by next more sophisticated check
	Radiosity_OctreeAccepts3,         // number of blocks accepted by next more sophisticated check
	Radiosity_OctreeAccepts4,         // number of blocks accepted by next more sophisticated check
	Radiosity_OctreeAccepts5,         // number of blocks accepted by next more sophisticated check
	// [CLi] radiosity "top level" recursion stats (all pre- & final traces)
	Radiosity_TopLevel_ReuseCount,    // ambient value queries satisfied without taking a new sample
	Radiosity_TopLevel_GatherCount,   // number of samples gathered
	Radiosity_TopLevel_RayCount,      // number of rays shot to gather samples
	// [CLi] radiosity final trace stats (all recursion depths)
	Radiosity_Final_ReuseCount,       // ambient value queries satisfied without taking a new sample
	Radiosity_Final_GatherCount,      // number of samples gathered
	Radiosity_Final_RayCount,         // number of rays shot to gather samples
	// [CLi] radiosity detailed sample stats
	Radiosity_SamplesTaken_PTS1_R0,   // number of samples gathered during pretrace step 1 at recursion depth 0
	Radiosity_SamplesTaken_PTS1_R1,   //  ...
	Radiosity_SamplesTaken_PTS1_R2,
	Radiosity_SamplesTaken_PTS1_R3,
	Radiosity_SamplesTaken_PTS1_R4ff, // number of samples gathered during pretrace step 1 at recursion depth 4 or deeper
	Radiosity_SamplesTaken_PTS2_R0,   //  ...
	Radiosity_SamplesTaken_PTS2_R1,
	Radiosity_SamplesTaken_PTS2_R2,
	Radiosity_SamplesTaken_PTS2_R3,
	Radiosity_SamplesTaken_PTS2_R4ff,
	Radiosity_SamplesTaken_PTS3_R0,
	Radiosity_SamplesTaken_PTS3_R1,
	Radiosity_SamplesTaken_PTS3_R2,
	Radiosity_SamplesTaken_PTS3_R3,
	Radiosity_SamplesTaken_PTS3_R4ff,
	Radiosity_SamplesTaken_PTS4_R0,
	Radiosity_SamplesTaken_PTS4_R1,
	Radiosity_SamplesTaken_PTS4_R2,
	Radiosity_SamplesTaken_PTS4_R3,
	Radiosity_SamplesTaken_PTS4_R4ff,
	Radiosity_SamplesTaken_PTS5ff_R0, // number of samples gathered during pretrace step 5 or deeper at recursion depth 0
	Radiosity_SamplesTaken_PTS5ff_R1, //  ...
	Radiosity_SamplesTaken_PTS5ff_R2,
	Radiosity_SamplesTaken_PTS5ff_R3,
	Radiosity_SamplesTaken_PTS5ff_R4ff,
	Radiosity_SamplesTaken_Final_R0,  // number of samples gathered during final render at recursion depth 0
	Radiosity_SamplesTaken_Final_R1,  //  ...
	Radiosity_SamplesTaken_Final_R2,
	Radiosity_SamplesTaken_Final_R3,
	Radiosity_SamplesTaken_Final_R4ff,
	Radiosity_QueryCount_R0,          // ambient value queries at recursion depth 0
	Radiosity_QueryCount_R1,          // ...
	Radiosity_QueryCount_R2,          // ...
	Radiosity_QueryCount_R3,          // ...
	Radiosity_QueryCount_R4ff,        // ...

	/* Must be the last */
	MaxIntStat

} IntStatsIndex;

typedef enum FPSTATS
{
	Radiosity_Weight_R0 = 0,          // summed-up weight of radiosity sample rays at recursion depth 0 during final trace
	Radiosity_Weight_R1,              //  ...
	Radiosity_Weight_R2,
	Radiosity_Weight_R3,
	Radiosity_Weight_R4ff,

	/* Must be the last */
	MaxFPStat
} FPStatsIndex;

typedef struct intersection_stats_info
{
	int povms_id;
	IntStatsIndex stat_test_id;
	IntStatsIndex stat_suc_id;
	char *infotext;
} INTERSECTION_STATS_INFO;

extern const INTERSECTION_STATS_INFO intersection_stats[];

}

#endif
