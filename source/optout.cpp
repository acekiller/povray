/*******************************************************************************
 * optout.cpp
 *
 * This module contains functions for credit, usage and options.
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
 * $File: //depot/povray/smp/source/optout.cpp $
 * $Revision: #20 $
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

#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <algorithm>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "base/povmsgid.h"
#include "backend/math/vector.h"

#include "povray.h"
#include "statspov.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/


/*****************************************************************************
* Local typedefs
******************************************************************************/


/*****************************************************************************
* Local variables
******************************************************************************/


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/

const INTERSECTION_STATS_INFO intersection_stats[kPOVList_Stat_Last] =
{
	{ kPOVList_Stat_RBezierTest,        Ray_RBezier_Tests, Ray_RBezier_Tests_Succeeded,
	  "Bezier Patch" },
	{ kPOVList_Stat_BicubicTest,        Ray_Bicubic_Tests, Ray_Bicubic_Tests_Succeeded,
	  "Bicubic Patch" },
	{ kPOVList_Stat_BlobTest,           Ray_Blob_Tests, Ray_Blob_Tests_Succeeded,
	  "Blob" },
	{ kPOVList_Stat_BlobCpTest,         Blob_Element_Tests, Blob_Element_Tests_Succeeded,
	  "Blob Component" },
	{ kPOVList_Stat_BlobBdTest,         Blob_Bound_Tests, Blob_Bound_Tests_Succeeded,
	  "Blob Bound" },
	{ kPOVList_Stat_BoxTest,            Ray_Box_Tests, Ray_Box_Tests_Succeeded,
	  "Box" },
	{ kPOVList_Stat_ConeCylTest,        Ray_Cone_Tests, Ray_Cone_Tests_Succeeded,
	  "Cone/Cylinder" },
	{ kPOVList_Stat_CSGIntersectTest,   Ray_CSG_Intersection_Tests, Ray_CSG_Intersection_Tests_Succeeded,
	  "CSG Intersection" },
	{ kPOVList_Stat_CSGMergeTest,       Ray_CSG_Merge_Tests, Ray_CSG_Merge_Tests_Succeeded,
	  "CSG Merge" },
	{ kPOVList_Stat_CSGUnionTest,       Ray_CSG_Union_Tests, Ray_CSG_Union_Tests_Succeeded,
	  "CSG Union" },
	{ kPOVList_Stat_DiscTest,           Ray_Disc_Tests, Ray_Disc_Tests_Succeeded,
	  "Disc" },
	{ kPOVList_Stat_FractalTest,        Ray_Fractal_Tests, Ray_Fractal_Tests_Succeeded,
	  "Fractal" },
	{ kPOVList_Stat_HFTest,             Ray_HField_Tests, Ray_HField_Tests_Succeeded,
	  "Height Field" },
	{ kPOVList_Stat_HFBoxTest,          Ray_HField_Box_Tests, Ray_HField_Box_Tests_Succeeded,
	  "Height Field Box" },
	{ kPOVList_Stat_HFTriangleTest,     Ray_HField_Triangle_Tests, Ray_HField_Triangle_Tests_Succeeded,
	  "Height Field Triangle" },
	{ kPOVList_Stat_HFBlockTest,        Ray_HField_Block_Tests, Ray_HField_Block_Tests_Succeeded,
	  "Height Field Block" },
	{ kPOVList_Stat_HFCellTest,         Ray_HField_Cell_Tests, Ray_HField_Cell_Tests_Succeeded,
	  "Height Field Cell" },
	{ kPOVList_Stat_IsosurfaceTest,     Ray_IsoSurface_Tests, Ray_IsoSurface_Tests_Succeeded,
	  "Isosurface" },
	{ kPOVList_Stat_IsosurfaceBdTest,   Ray_IsoSurface_Bound_Tests, Ray_IsoSurface_Bound_Tests_Succeeded,
	  "Isosurface Container" },
	{ kPOVList_Stat_IsosurfaceCacheTest,Ray_IsoSurface_Cache, Ray_IsoSurface_Cache_Succeeded,
	  "Isosurface Cache" },
	{ kPOVList_Stat_LatheTest,          Ray_Lathe_Tests, Ray_Lathe_Tests_Succeeded,
	  "Lathe" },
	{ kPOVList_Stat_LatheBdTest,        Lathe_Bound_Tests, Lathe_Bound_Tests_Succeeded,
	  "Lathe Bound" },
	{ kPOVList_Stat_MeshTest,           Ray_Mesh_Tests, Ray_Mesh_Tests_Succeeded,
	  "Mesh" },
	{ kPOVList_Stat_OvusTest,           Ray_Ovus_Tests, Ray_Ovus_Tests_Succeeded,
	  "Ovus" },
	{ kPOVList_Stat_PlaneTest,          Ray_Plane_Tests, Ray_Plane_Tests_Succeeded,
	  "Plane" },
	{ kPOVList_Stat_PolygonTest,        Ray_Polygon_Tests, Ray_Polygon_Tests_Succeeded,
	  "Polygon" },
	{ kPOVList_Stat_PrismTest,          Ray_Prism_Tests, Ray_Prism_Tests_Succeeded,
	  "Prism" },
	{ kPOVList_Stat_PrismBdTest,        Prism_Bound_Tests, Prism_Bound_Tests_Succeeded,
	  "Prism Bound" },
	{ kPOVList_Stat_ParametricTest,     Ray_Parametric_Tests, Ray_Parametric_Tests_Succeeded,
	  "Parametric" },
	{ kPOVList_Stat_ParametricBoxTest,  Ray_Par_Bound_Tests, Ray_Par_Bound_Tests_Succeeded,
	  "Parametric Bound" },
	{ kPOVList_Stat_QuardicTest,        Ray_Quadric_Tests, Ray_Quadric_Tests_Succeeded,
	  "Quadric" },
	{ kPOVList_Stat_QuadPolyTest,       Ray_Poly_Tests, Ray_Poly_Tests_Succeeded,
	  "Quartic/Poly" },
	{ kPOVList_Stat_SphereTest,         Ray_Sphere_Tests, Ray_Sphere_Tests_Succeeded,
	  "Sphere" },
	{ kPOVList_Stat_SphereSweepTest,    Ray_Sphere_Sweep_Tests, Ray_Sphere_Sweep_Tests_Succeeded,
	  "Sphere Sweep" },
	{ kPOVList_Stat_SuperellipsTest,    Ray_Superellipsoid_Tests, Ray_Superellipsoid_Tests_Succeeded,
	  "Superellipsoid" },
	{ kPOVList_Stat_SORTest,            Ray_Sor_Tests, Ray_Sor_Tests_Succeeded,
	  "Surface of Revolution" },
	{ kPOVList_Stat_SORBdTest,          Sor_Bound_Tests, Sor_Bound_Tests_Succeeded,
	  "Surface of Rev. Bound" },
	{ kPOVList_Stat_TorusTest,          Ray_Torus_Tests, Ray_Torus_Tests_Succeeded,
	  "Torus" },
	{ kPOVList_Stat_TorusBdTest,        Torus_Bound_Tests, Torus_Bound_Tests_Succeeded,
	  "Torus Bound" },
	{ kPOVList_Stat_TriangleTest,       Ray_Triangle_Tests, Ray_Triangle_Tests_Succeeded,
	  "Triangle" },
	{ kPOVList_Stat_TTFontTest,         Ray_TTF_Tests, Ray_TTF_Tests_Succeeded,
	  "True Type Font" },
	{ kPOVList_Stat_BoundObjectTest,    Bounding_Region_Tests, Bounding_Region_Tests_Succeeded,
	  "Bounding Object" },
	{ kPOVList_Stat_ClipObjectTest,     Clipping_Region_Tests, Clipping_Region_Tests_Succeeded,
	  "Clipping Object" },
	{ kPOVList_Stat_BoundingBoxTest,    nChecked, nEnqueued,
	  "Bounding Box" },
	{ kPOVList_Stat_LightBufferTest,    LBuffer_Tests, LBuffer_Tests_Succeeded,
	  "Light Buffer" },
	{ kPOVList_Stat_VistaBufferTest,    VBuffer_Tests, VBuffer_Tests_Succeeded,
	  "Vista Buffer" },
	{ kPOVList_Stat_Last, MaxIntStat, MaxIntStat, NULL }
};

}
