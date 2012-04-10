/*******************************************************************************
 * hfield.cpp
 *
 *   This file implements the height field shape primitive.  The shape is
 *   implemented as a collection of triangles which are calculated as
 *   needed.  The basic intersection routine first computes the rays
 *   intersection with the box marking the limits of the shape, then
 *   follows the line from one intersection point to the other, testing the
 *   two triangles which form the pixel for an intersection with the ray at
 *   each step.
 *
 *   height field added by Doug Muir with lots of advice and support
 *   from David Buck and Drew Wells.
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
 * $File: //depot/povray/smp/source/backend/shape/hfield.cpp $
 * $Revision: #35 $
 * $Change: 5103 $
 * $DateTime: 2010/08/22 06:58:49 $
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

/****************************************************************************
*
*  Explanation:
*
*    -
*
*  Syntax:
*
*  ---
*
*  Aug 1994 : Merged functions for CSG height fields into functions for
*             non-CSG height fiels. Moved all height field map related
*             data into one data structure. Fixed memory problems. [DB]
*
*  Feb 1995 : Major rewrite of the height field intersection tests. [DB]
*
*****************************************************************************/

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/shape/hfield.h"
#include "backend/support/imageutil.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/scene/threaddata.h"
#include "base/pov_err.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define sign(x) (((x) >= 0.0) ? 1.0 : -1.0)

#define Get_Height(x, z) ((DBL)Data->Map[(z)][(x)])

/* Small offest. */

const DBL HFIELD_OFFSET = 0.001;

const DBL HFIELD_TOLERANCE = 1.0e-6;



/*****************************************************************************
*
* FUNCTION
*
*   All_HField_Intersections
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Feb 1995 : Modified to work with new intersection functions. [DB]
*
******************************************************************************/

bool HField::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	int Side1, Side2;
	VECTOR Start;
	Ray Temp_Ray;
	DBL depth1, depth2;

	Thread->Stats()[Ray_HField_Tests]++;

	MInvTransPoint(Temp_Ray.Origin, ray.Origin, Trans);
	MInvTransDirection(Temp_Ray.Direction, ray.Direction, Trans);

#ifdef HFIELD_EXTRA_STATS
	Thread->Stats()[Ray_HField_Box_Tests]++;
#endif

	if (!Box::Intersect(Temp_Ray,NULL,bounding_corner1,bounding_corner2,&depth1,&depth2,&Side1,&Side2))
	{
		return(false);
	}

#ifdef HFIELD_EXTRA_STATS
	Thread->Stats()[Ray_HField_Box_Tests_Succeeded]++;
#endif

	if (depth1 < HFIELD_TOLERANCE)
	{
		depth1 = HFIELD_TOLERANCE;

		if (depth1 > depth2)
		{
			return(false);
		}
	}

	VEvaluateRay(Start, Temp_Ray.Origin, depth1, Temp_Ray.Direction);

	if (block_traversal(Temp_Ray, Start, Depth_Stack, ray, depth1, depth2, Thread))
	{
		Thread->Stats()[Ray_HField_Tests_Succeeded]++;

		return(true);
	}
	return(false);
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

bool HField::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	int px, pz;
	DBL x,z,y1,y2,y3,water, dot1, dot2;
	VECTOR Local_Origin, H_Normal, Test;

	MInvTransPoint(Test, IPoint, Trans);

	water = bounding_corner1[Y];

	if ((Test[X] < 0.0) || (Test[X] >= bounding_corner2[X]) ||
	    (Test[Z] < 0.0) || (Test[Z] >= bounding_corner2[Z]))
	{
		return(Test_Flag(this, INVERTED_FLAG));
	}

	if (Test[Y] >= bounding_corner2[Y])
	{
		return(Test_Flag(this, INVERTED_FLAG));
	}

	if (Test[Y] < water)
	{
		return(!Test_Flag(this, INVERTED_FLAG));
	}

	px = (int)Test[X];
	pz = (int)Test[Z];

	x = Test[X] - (DBL)px;
	z = Test[Z] - (DBL)pz;

	if ((x+z) < 1.0)
	{
		y1 = max(Get_Height(px,   pz), water);
		y2 = max(Get_Height(px+1, pz), water);
		y3 = max(Get_Height(px,   pz+1), water);

		Make_Vector(Local_Origin,(DBL)px,y1,(DBL)pz);

		Make_Vector(H_Normal, y1-y2, 1.0, y1-y3);
	}
	else
	{
		px = (int)ceil(Test[X]);
		pz = (int)ceil(Test[Z]);

		y1 = max(Get_Height(px,   pz), water);
		y2 = max(Get_Height(px-1, pz), water);
		y3 = max(Get_Height(px,   pz-1), water);

		Make_Vector(Local_Origin,(DBL)px,y1,(DBL)pz);

		Make_Vector(H_Normal, y2-y1, 1.0, y3-y1);
	}

	VDot(dot1, Test, H_Normal);
	VDot(dot2, Local_Origin, H_Normal);

	if (dot1 < dot2)
	{
		return(!Test_Flag(this, INVERTED_FLAG));
	}

	return(Test_Flag(this, INVERTED_FLAG));
}



/*****************************************************************************
*
* FUNCTION
*
*   HField_Normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   2000 : NK crash bugfix
*
******************************************************************************/

void HField::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	int px,pz, i;
	DBL x,z,y1,y2,y3,u,v;
	VECTOR Local_Origin;
	VECTOR n[5];

	if(Inter->haveNormal == true)
	{
		Assign_Vector(Result,Inter->INormal);
		return;
	}

	MInvTransPoint(Local_Origin, Inter->IPoint, Trans);

	px = (int)Local_Origin[X];
	pz = (int)Local_Origin[Z];

	if (px>Data->max_x)
		px=Data->max_x;
	if (pz>Data->max_z)
		pz=Data->max_z;

	x = Local_Origin[X] - (DBL)px;
	z = Local_Origin[Z] - (DBL)pz;

	if (Test_Flag(this, SMOOTHED_FLAG))
	{
		n[0][X] = Data->Normals[pz][px][0];
		n[0][Y] = Data->Normals[pz][px][1];
		n[0][Z] = Data->Normals[pz][px][2];
		n[1][X] = Data->Normals[pz][px+1][0];
		n[1][Y] = Data->Normals[pz][px+1][1];
		n[1][Z] = Data->Normals[pz][px+1][2];
		n[2][X] = Data->Normals[pz+1][px][0];
		n[2][Y] = Data->Normals[pz+1][px][1];
		n[2][Z] = Data->Normals[pz+1][px][2];
		n[3][X] = Data->Normals[pz+1][px+1][0];
		n[3][Y] = Data->Normals[pz+1][px+1][1];
		n[3][Z] = Data->Normals[pz+1][px+1][2];

		for (i = 0; i < 4; i++)
		{
			MTransNormal(n[i], n[i], Trans);
			VNormalizeEq(n[i]);
		}

		u = (1.0 - x);
		v = (1.0 - z);

		Result[X] = v*(u*n[0][X] + x*n[1][X]) + z*(u*n[2][X] + x*n[3][X]);
		Result[Y] = v*(u*n[0][Y] + x*n[1][Y]) + z*(u*n[2][Y] + x*n[3][Y]);
		Result[Z] = v*(u*n[0][Z] + x*n[1][Z]) + z*(u*n[2][Z] + x*n[3][Z]);
	}
	else
	{
		if ((x+z) <= 1.0)
		{
			/* Lower triangle. */

			y1 = Get_Height(px,   pz);
			y2 = Get_Height(px+1, pz);
			y3 = Get_Height(px,   pz+1);

			Make_Vector(Result, y1-y2, 1.0, y1-y3);
		}
		else
		{
			/* Upper triangle. */

			y1 = Get_Height(px+1, pz+1);
			y2 = Get_Height(px,   pz+1);
			y3 = Get_Height(px+1, pz);

			Make_Vector(Result, y2-y1, 1.0, y3-y1);
		}

		MTransNormal(Result, Result, Trans);
	}

	VNormalize(Result, Result);
}



/*****************************************************************************
*
* FUNCTION
*
*   normalize
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

DBL HField::normalize(VECTOR A, const VECTOR  B)
{
	DBL tmp;

	VLength(tmp, B);

	if (fabs(tmp) > EPSILON)
	{
		VInverseScale(A, B, tmp);
	}
	else
	{
		Make_Vector(A, 0.0, 1.0, 0.0);
	}

	return(tmp);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_pixel
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   2000 : NK crash bugfix
*
******************************************************************************/

bool HField::intersect_pixel(int x, int z, const Ray &ray, DBL height1, DBL height2, IStack& HField_Stack, const Ray &RRay, DBL mindist, DBL maxdist, TraceThreadData *Thread)
{
	int Found;
	DBL dot, depth1, depth2;
	DBL s, t, y1, y2, y3, y4;
	DBL min_y2_y3, max_y2_y3;
	DBL max_height, min_height;
	VECTOR P, N, V1;

#ifdef HFIELD_EXTRA_STATS
	Thread->Stats()[Ray_HField_Cell_Tests]++;
#endif

	if (z>Data->max_z) z = Data->max_z;
	if (x>Data->max_x) x = Data->max_x;

	y1 = Get_Height(x,   z);
	y2 = Get_Height(x+1, z);
	y3 = Get_Height(x,   z+1);
	y4 = Get_Height(x+1, z+1);

	/* Do we hit this cell at all? */

	if (y2 < y3)
	{
		min_y2_y3 = y2;
		max_y2_y3 = y3;
	}
	else
	{
		min_y2_y3 = y3;
		max_y2_y3 = y2;
	}

	min_height = min(min(y1, y4), min_y2_y3);
	max_height = max(max(y1, y4), max_y2_y3);

	if ((max_height < height1) || (min_height > height2))
	{
		return(false);
	}

#ifdef HFIELD_EXTRA_STATS
	Thread->Stats()[Ray_HField_Cell_Tests_Succeeded]++;
#endif

	Found = false;

	/* Check if we'll hit first triangle. */

	min_height = min(y1, min_y2_y3);
	max_height = max(y1, max_y2_y3);

	if ((max_height >= height1) && (min_height <= height2))
	{
#ifdef HFIELD_EXTRA_STATS
		Thread->Stats()[Ray_HField_Triangle_Tests]++;
#endif

		/* Set up triangle. */

		Make_Vector(P, (DBL)x, y1, (DBL)z);

		/*
		 * Calculate the normal vector from:
		 *
		 * N = V2 x V1, with V1 = <1, y2-y1, 0>, V2 = <0, y3-y1, 1>.
		 */

		Make_Vector(N, y1-y2, 1.0, y1-y3);

		/* Now intersect the triangle. */

		VDot(dot, N, ray.Direction);

		if ((dot > EPSILON) || (dot < -EPSILON))
		{
			VSub(V1, P, ray.Origin);

			VDot(depth1, N, V1);

			depth1 /= dot;

			if ((depth1 >= mindist) && (depth1 <= maxdist))
			{
				s = ray.Origin[X] + depth1 * ray.Direction[X] - (DBL)x;
				t = ray.Origin[Z] + depth1 * ray.Direction[Z] - (DBL)z;

				if ((s >= -0.0001) && (t >= -0.0001) && ((s+t) <= 1.0001))
				{
#ifdef HFIELD_EXTRA_STATS
					Thread->Stats()[Ray_HField_Triangle_Tests_Succeeded]++;
#endif

					VEvaluateRay(P, RRay.Origin, depth1, RRay.Direction);

					if (Clip.empty() || Point_In_Clip(P, Clip, Thread))
					{
						if (Test_Flag(this, SMOOTHED_FLAG))  // TODO - someone needs to understand this wholeif-else! [trf]
							HField_Stack->push(Intersection(depth1, P, this));
						else
						{
							VECTOR tmp;
							Assign_Vector(tmp,N);
							MTransNormal(tmp,tmp,Trans);
							VNormalize(tmp,tmp);
							HField_Stack->push(Intersection(depth1, P, tmp, this));
						}

						Found = true;
					}
				}
			}
		}
	}

	/* Check if we'll hit second triangle. */

	min_height = min(y4, min_y2_y3);
	max_height = max(y4, max_y2_y3);

	if ((max_height >= height1) && (min_height <= height2))
	{
#ifdef HFIELD_EXTRA_STATS
		Thread->Stats()[Ray_HField_Triangle_Tests]++;
#endif

		/* Set up triangle. */

		Make_Vector(P, (DBL)(x+1), y4, (DBL)(z+1));

		/*
		 * Calculate the normal vector from:
		 *
		 * N = V2 x V1, with V1 = <-1, y3-y4, 0>, V2 = <0, y2-y4, -1>.
		 */

		Make_Vector(N, y3-y4, 1.0, y2-y4);

		/* Now intersect the triangle. */

		VDot(dot, N, ray.Direction);

		if ((dot > EPSILON) || (dot < -EPSILON))
		{
			VSub(V1, P, ray.Origin);

			VDot(depth2, N, V1);

			depth2 /= dot;

			if ((depth2 >= mindist) && (depth2 <= maxdist))
			{
				s = ray.Origin[X] + depth2 * ray.Direction[X] - (DBL)x;
				t = ray.Origin[Z] + depth2 * ray.Direction[Z] - (DBL)z;

				if ((s <= 1.0001) && (t <= 1.0001) && ((s+t) >= 0.9999))
				{
#ifdef HFIELD_EXTRA_STATS
					Thread->Stats()[Ray_HField_Triangle_Tests_Succeeded]++;
#endif

					VEvaluateRay(P, RRay.Origin, depth2, RRay.Direction);

					if (Clip.empty() || Point_In_Clip(P, Clip, Thread))
					{
						if (Test_Flag(this, SMOOTHED_FLAG)) // TODO - someone needs to understand this wholeif-else! [trf]
							HField_Stack->push(Intersection(depth2, P, this));
						else
						{
							VECTOR tmp;
							Assign_Vector(tmp,N);
							MTransNormal(tmp,tmp,Trans);
							VNormalize(tmp,tmp);
							HField_Stack->push(Intersection(depth2, P, tmp, this));
						}

						Found = true;
					}
				}
			}
		}
	}

	return(Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   add_single_normal
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

int HField::add_single_normal(HF_VAL **data, int xsize, int zsize, int x0, int z0, int x1, int z1, int x2, int z2, VECTOR N)
{
	VECTOR v0, v1, v2, t0, t1, Nt;

	if ((x0 < 0) || (z0 < 0) ||
	    (x1 < 0) || (z1 < 0) ||
	    (x2 < 0) || (z2 < 0) ||
	    (x0 > xsize) || (z0 > zsize) ||
	    (x1 > xsize) || (z1 > zsize) ||
	    (x2 > xsize) || (z2 > zsize))
	{
		return(0);
	}
	else
	{
		Make_Vector(v0, x0, (DBL)data[z0][x0], z0);
		Make_Vector(v1, x1, (DBL)data[z1][x1], z1);
		Make_Vector(v2, x2, (DBL)data[z2][x2], z2);

		VSub(t0, v2, v0);
		VSub(t1, v1, v0);

		VCross(Nt, t0, t1);

		normalize(Nt, Nt);

		if (Nt[Y] < 0.0)
		{
			VScaleEq(Nt, -1.0);
		}

		VAddEq(N, Nt);

		return(1);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   smooth_height_field
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   Given a height field that only contains an elevation grid, this
*   routine will walk through the data and produce averaged normals
*   for all points on the grid.
*
* CHANGES
*
*   -
*
******************************************************************************/

void HField::smooth_height_field(int xsize, int zsize)
{
	int i, j, k;
	VECTOR N;
	HF_VAL **map = Data->Map;

	/* First off, allocate all the memory needed to store the normal information */

	Data->Normals_Height = zsize+1;

	Data->Normals = (HF_Normals **)POV_MALLOC((zsize+1)*sizeof(HF_Normals *), "height field normals");

	for (i = 0; i <= zsize; i++)
	{
		Data->Normals[i] = (HF_Normals *)POV_MALLOC((xsize+1)*sizeof(HF_Normals), "height field normals");
	}

	/*
	 * For now we will do it the hard way - by generating the normals
	 * individually for each elevation point.
	 */

	for (i = 0; i <= zsize; i++)
	{
		for (j = 0; j <= xsize; j++)
		{
			Make_Vector(N, 0.0, 0.0, 0.0);

			k = 0;

			k += add_single_normal(map, xsize, zsize, j, i, j+1, i, j, i+1, N);
			k += add_single_normal(map, xsize, zsize, j, i, j, i+1, j-1, i, N);
			k += add_single_normal(map, xsize, zsize, j, i, j-1, i, j, i-1, N);
			k += add_single_normal(map, xsize, zsize, j, i, j, i-1, j+1, i, N);

			if (k == 0)
			{
				throw POV_EXCEPTION_STRING("Failed to find any normals at.");
			}

			normalize(N, N);

			Data->Normals[i][j][0] = (short)(32767 * N[X]);
			Data->Normals[i][j][1] = (short)(32767 * N[Y]);
			Data->Normals[i][j][2] = (short)(32767 * N[Z]);
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_HField
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*
* DESCRIPTION
*
*   Copy image data into height field map. Create bounding blocks
*   for the block traversal. Calculate normals for smoothed height fields.
*
* CHANGES
*
*   Feb 1995 : Modified to work with new intersection functions. [DB]
*
******************************************************************************/

void HField::Compute_HField(ImageData *image)
{
	int x, z, max_x, max_z;
	HF_VAL min_y, max_y, temp_y;

	/* Get height field map size. */

	max_x = image->iwidth;
	max_z = image->iheight;

	/* Allocate memory for map. */

	Data->Map = (HF_VAL **)POV_MALLOC(max_z*sizeof(HF_VAL *), "height field");

	for (z = 0; z < max_z; z++)
	{
		Data->Map[z] = (HF_VAL *)POV_MALLOC(max_x*sizeof(HF_VAL), "height field");
	}

	/* Copy map. */

	min_y = 65535L;
	max_y = 0;

	for (z = 0; z < max_z; z++)
	{
		for (x = 0; x < max_x; x++)
		{
			temp_y = image_height_at(image, x, max_z - z - 1);

			Data->Map[z][x] = temp_y;

			min_y = min(min_y, temp_y);
			max_y = max(max_y, temp_y);
		}
	}

	/* Resize bounding box. */

	Data->min_y = min_y;
	Data->max_y = max_y;

	bounding_corner1[Y] = max((DBL)min_y, bounding_corner1[Y]) - HFIELD_OFFSET;
	bounding_corner2[Y] = (DBL)max_y + HFIELD_OFFSET;

	/* Compute smoothed height field. */

	if (Test_Flag(this, SMOOTHED_FLAG))
	{
		smooth_height_field(max_x-1, max_z-1);
	}

	Data->max_x = max_x-2;
	Data->max_z = max_z-2;

	build_hfield_blocks();
}



/*****************************************************************************
*
* FUNCTION
*
*   build_hfield_blocks
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Create the bounding block hierarchy used by the block traversal.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void HField::build_hfield_blocks()
{
	int x, z, nx, nz, wx, wz;
	int i, j;
	int xmin, xmax, zmin, zmax;
	DBL y, ymin, ymax, water;

	/* Get block size. */

	nx = max(1, (int)(sqrt((DBL)Data->max_x)));
	nz = max(1, (int)(sqrt((DBL)Data->max_z)));

	/* Get dimensions of sub-block. */

	wx = (int)ceil((DBL)(Data->max_x + 2) / (DBL)nx);
	wz = (int)ceil((DBL)(Data->max_z + 2) / (DBL)nz);

	/* Increase number of sub-blocks if necessary. */

	if (nx * wx < Data->max_x + 2)
	{
		nx++;
	}

	if (nz * wz < Data->max_z + 2)
	{
		nz++;
	}

	if (!Test_Flag(this, HIERARCHY_FLAG) || ((nx == 1) && (nz == 1)))
	{
		/* We don't want a bounding hierarchy. Just use one block. */

		Data->Block = (HFIELD_BLOCK **)POV_MALLOC(sizeof(HFIELD_BLOCK *), "height field blocks");

		Data->Block[0] = (HFIELD_BLOCK *)POV_MALLOC(sizeof(HFIELD_BLOCK), "height field blocks");

		Data->Block[0][0].xmin = 0;
		Data->Block[0][0].xmax = Data->max_x;
		Data->Block[0][0].zmin = 0;
		Data->Block[0][0].zmax = Data->max_z;

		Data->Block[0][0].ymin = bounding_corner1[Y];
		Data->Block[0][0].ymax = bounding_corner2[Y];

		Data->block_max_x = 1;
		Data->block_max_z = 1;

		Data->block_width_x = Data->max_x + 2;
		Data->block_width_z = Data->max_y + 2;

//		Debug_Info("Height field: %d x %d (1 x 1 blocks)\n", Data->max_x+2, Data->max_z+2);

		return;
	}

//	Debug_Info("Height field: %d x %d (%d x %d blocks)\n", Data->max_x+2, Data->max_z+2, nx, nz);

	/* Allocate memory for blocks. */

	Data->Block = (HFIELD_BLOCK **)POV_MALLOC(nz*sizeof(HFIELD_BLOCK *), "height field blocks");

	/* Store block information. */

	Data->block_max_x = nx;
	Data->block_max_z = nz;

	Data->block_width_x = wx;
	Data->block_width_z = wz;

	water = bounding_corner1[Y];

	for (z = 0; z < nz; z++)
	{
		Data->Block[z] = (HFIELD_BLOCK *)POV_MALLOC(nx*sizeof(HFIELD_BLOCK), "height field blocks");

		for (x = 0; x < nx; x++)
		{
			/* Get block's borders. */

			xmin = x * wx;
			zmin = z * wz;

			xmax = min((x + 1) * wx - 1, Data->max_x);
			zmax = min((z + 1) * wz - 1, Data->max_z);

			/* Find min. and max. height in current block. */

			ymin = BOUND_HUGE;
			ymax = -BOUND_HUGE;

			for (i = xmin; i <= xmax+1; i++)
			{
				for (j = zmin; j <= zmax+1; j++)
				{
					y = Get_Height(i, j);

					ymin = min(ymin, y);
					ymax = max(ymax, y);
				}
			}

			/* Store block's borders. */

			Data->Block[z][x].xmin = xmin;
			Data->Block[z][x].xmax = xmax;
			Data->Block[z][x].zmin = zmin;
			Data->Block[z][x].zmax = zmax;

			Data->Block[z][x].ymin = max(ymin, water) - HFIELD_OFFSET;
			Data->Block[z][x].ymax = ymax + HFIELD_OFFSET;
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void HField::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_HField
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void HField::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void HField::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void HField::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void HField::Transform(const TRANSFORM *tr)
{
	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_HField
*
* INPUT
*
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   Allocate and intialize a height field.
*
* CHANGES
*
*   Feb 1995 : Modified to work with new intersection functions. [DB]
*
******************************************************************************/

HField::HField() : ObjectBase(HFIELD_OBJECT)
{
	Trans = Create_Transform();

	Make_Vector(bounding_corner1, -1.0, -1.0, -1.0);
	Make_Vector(bounding_corner2,  1.0,  1.0,  1.0);

	/* Allocate height field data. */

	Data = (HFIELD_DATA *)POV_MALLOC(sizeof(HFIELD_DATA), "height field");

	Data->References = 1;

	Data->Normals_Height = 0;

	Data->Map     = NULL;
	Data->Normals = NULL;

	Data->max_x = 0;
	Data->max_z = 0;

	Data->block_max_x = 0;
	Data->block_max_z = 0;

	Data->block_width_x = 0;
	Data->block_width_z = 0;

	Set_Flag(this, HIERARCHY_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   NOTE: The height field data is not copied, only the number of references
*         is counted, so that Destray_HField() knows if it can be destroyed.
*
* CHANGES
*
*   -
*
******************************************************************************/

ObjectPtr HField::Copy()
{
	HField *New = new HField();

	POV_FREE (New->Data);

	/* Copy height field. */

	Destroy_Transform(New->Trans);
	*New = *this;
	New->Trans = Copy_Transform(Trans);

	Assign_Vector(New->bounding_corner1, bounding_corner1);
	Assign_Vector(New->bounding_corner2, bounding_corner2);

	New->Data = Data;
	New->Data->References++;

	return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_HField
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Doug Muir, David Buck, Drew Wells
*   
* DESCRIPTION
*
*   NOTE: The height field data is destroyed if it's no longer
*         used by any copy.
*
* CHANGES
*
*   Feb 1995 : Modified to work with new intersection functions. [DB]
*
******************************************************************************/

HField::~HField()
{
	int i;

	Destroy_Transform(Trans);

	if (--(Data->References) == 0)
	{
		if (Data->Map != NULL)
		{
			for (i = 0; i < Data->max_z+2; i++)
			{
				if (Data->Map[i] != NULL)
				{
					POV_FREE (Data->Map[i]);
				}
			}

			POV_FREE (Data->Map);
		}

		if (Data->Normals != NULL)
		{
			for (i = 0; i < Data->Normals_Height; i++)
			{
				POV_FREE (Data->Normals[i]);
			}

			POV_FREE (Data->Normals);
		}

		if (Data->Block != NULL)
		{
			for (i = 0; i < Data->block_max_z; i++)
			{
				POV_FREE(Data->Block[i]);
			}

			POV_FREE(Data->Block);
		}

		POV_FREE (Data);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_HField_BBox
*
* INPUT
*
*   HField - Height field
*   
* OUTPUT
*
*   HField
*   
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a height field.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void HField::Compute_BBox()
{
	// [ABX 20.01.2004] Low_Left introduced to hide BCC 5.5 bug
	BBOX_VECT& Low_Left = BBox.Lower_Left;

	Assign_BBox_Vect(Low_Left, bounding_corner1);

	VSub (BBox.Lengths, bounding_corner2, bounding_corner1);

	if (Trans != NULL)
	{
		Recompute_BBox(&BBox, Trans);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   dda_traversal
*
* INPUT
*
*   Ray    - Current ray
*   HField - Height field
*   Start  - Start point for the walk
*   Block  - Sub-block of the height field to traverse
*   
* OUTPUT
*   
* RETURNS
*
*   int - true if intersection was found
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Traverse the grid cell of the height field using a modified DDA.
*
*   Based on the following article:
*
*     Musgrave, F. Kenton, "Grid Tracing: Fast Ray Tracing for Height
*     Fields", Research Report YALEU-DCS-RR-39, Yale University, July 1988
*
*   You should note that there are (n-1) x (m-1) grid cells in a height
*   field of (image) size n x m. A grid cell (i,j), 0 <= i <= n-1,
*   0 <= j <= m-1, extends from x = i to x = i + 1 - epsilon and
*   y = j to y = j + 1 -epsilon.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

bool HField::dda_traversal(const Ray &ray, const VECTOR Start, const HFIELD_BLOCK *Block, IStack &HField_Stack, const Ray &RRay, DBL mindist, DBL maxdist, TraceThreadData *Thread)
{
	char *dda_msg = "Illegal grid value in dda_traversal().\n"
	                "The height field may contain dark spots. To eliminate them\n"
	                "moving the camera a tiny bit should help. For more information\n"
	                "read the user manual!";
	int found;
	int xmin, xmax, zmin, zmax;
	int x, z, signx, signz;
	DBL ymin, ymax, y1, y2;
	DBL px, pz, dx, dy, dz;
	DBL delta, error, x0, z0;
	DBL neary, fary, deltay;

	/* Setup DDA. */

	found = false;

	px = Start[X];
	pz = Start[Z];

	/* Get dimensions of current block. */

	xmin = Block->xmin;
	xmax = min(Block->xmax + 1, Data->max_x);
	zmin = Block->zmin;
	zmax = min(Block->zmax + 1, Data->max_z);

	ymin = min(Start[Y], Block->ymin) - EPSILON;
	ymax = max(Start[Y], Block->ymax) + EPSILON;

	/* Check for illegal grid values (caused by numerical inaccuracies). */

	if (px < (DBL)xmin)
	{
		if (px < (DBL)xmin - HFIELD_OFFSET)
		{
;// TODO MESSAGE      Warning(0, dda_msg);

			return(false);
		}
		else
		{
			px = (DBL)xmin;
		}
	}
	else
	{
		if (px > (DBL)xmax + 1.0 - EPSILON)
		{
			if (px > (DBL)xmax + 1.0 + EPSILON)
			{
;// TODO MESSAGE        Warning(0, dda_msg);

				return(false);
			}
			else
			{
				px = (DBL)xmax + 1.0 - EPSILON;
			}
		}
	}

	if (pz < (DBL)zmin)
	{
		if (pz < (DBL)zmin - HFIELD_OFFSET)
		{
;// TODO MESSAGE      Warning(0, dda_msg);

			return(false);
		}
		else
		{
			pz = (DBL)zmin;
		}
	}
	else
	{
		if (pz > (DBL)zmax + 1.0 - EPSILON)
		{
			if (pz > (DBL)zmax + 1.0 + EPSILON)
			{
;// TODO MESSAGE        Warning(0, dda_msg);

				return(false);
			}
			else
			{
				pz = (DBL)zmax + 1.0 - EPSILON;
			}
		}
	}

	dx = ray.Direction[X];
	dy = ray.Direction[Y];
	dz = ray.Direction[Z];

	/*
	 * Here comes the DDA algorithm.
	 */

	/* Choose algorithm depending on the driving axis. */

	if (fabs(dx) >= fabs(dz))
	{
		/*
		 * X-axis is driving axis.
		 */

		delta = fabs(dz / dx);

		x = (int)px;
		z = (int)pz;

		x0 = px - floor(px);
		z0 = pz - floor(pz);

		signx = sign(dx);
		signz = sign(dz);

		/* Get initial error. */

		if (dx >= 0.0)
		{
			if (dz >= 0.0)
			{
				error = z0 + delta * (1.0 - x0) - 1.0;
			}
			else
			{
				error = -(z0 - delta * (1.0 - x0));
			}
		}
		else
		{
			if (dz >= 0.0)
			{
				error = z0 + delta * x0 - 1.0;
			}
			else
			{
				error = -(z0 - delta * x0);
			}
		}

		/* Get y differential. */

		deltay = dy / fabs(dx);

		if (dx >= 0.0)
		{
			neary = Start[Y] - x0 * deltay;

			fary = neary + deltay;
		}
		else
		{
			neary = Start[Y] - (1.0 - x0) * deltay;

			fary = neary + deltay;
		}

		/* Step through the cells. */

		do
		{
			if (neary < fary)
			{
				y1 = neary;
				y2 = fary;
			}
			else
			{
				y1 = fary;
				y2 = neary;
			}

			if (intersect_pixel(x, z, ray, y1, y2, HField_Stack, RRay, mindist, maxdist, Thread))
			{
				if (Type & IS_CHILD_OBJECT)
				{
					found = true;
				}
				else
				{
					return(true);
				}
			}

			if (error > EPSILON)
			{
				z += signz;

				if ((z < zmin) || (z > zmax))
				{
					break;
				}
				else
				{
					if (intersect_pixel(x, z, ray, y1, y2, HField_Stack, RRay, mindist, maxdist, Thread))
					{
						if (Type & IS_CHILD_OBJECT)
						{
							found = true;
						}
						else
						{
							return(true);
						}
					}
				}

				error--;
			}
			else
			{
				if (error > -EPSILON)
				{
					z += signz;

					error--;
				}
			}

			x += signx;

			error += delta;

			neary = fary;

			fary += deltay;
		}
		while ((neary >= ymin) && (neary <= ymax) && (x >= xmin) && (x <= xmax) && (z >= zmin) && (z <= zmax));
	}
	else
	{
		/*
		 * Z-axis is driving axis.
		 */

		delta = fabs(dx / dz);

		x = (int)px;
		z = (int)pz;

		x0 = px - floor(px);
		z0 = pz - floor(pz);

		signx = sign(dx);
		signz = sign(dz);

		/* Get initial error. */

		if (dz >= 0.0)
		{
			if (dx >= 0.0)
			{
				error = x0 + delta * (1.0 - z0) - 1.0;
			}
			else
			{
				error = -(x0 - delta * (1.0 - z0));
			}
		}
		else
		{
			if (dx >= 0.0)
			{
				error = x0 + delta * z0 - 1.0;
			}
			else
			{
				error = -(x0 - delta * z0);
			}
		}

		/* Get y differential. */

		deltay = dy / fabs(dz);

		if (dz >= 0.0)
		{
			neary = Start[Y] - z0 * deltay;

			fary = neary + deltay;
		}
		else
		{
			neary = Start[Y] - (1.0 - z0) * deltay;

			fary = neary + deltay;
		}

		/* Step through the cells. */

		do
		{
			if (neary < fary)
			{
				y1 = neary;
				y2 = fary;
			}
			else
			{
				y1 = fary;
				y2 = neary;
			}

			if (intersect_pixel(x, z, ray, y1, y2, HField_Stack, RRay, mindist, maxdist, Thread))
			{
				if (Type & IS_CHILD_OBJECT)
				{
					found = true;
				}
				else
				{
					return(true);
				}
			}

			if (error > EPSILON)
			{
				x += signx;

				if ((x < xmin) || (x > xmax))
				{
					break;
				}
				else
				{
					if (intersect_pixel(x, z, ray, y1, y2, HField_Stack, RRay, mindist, maxdist, Thread))
					{
						if (Type & IS_CHILD_OBJECT)
						{
							found = true;
						}
						else
						{
							return(true);
						}
					}
				}

				error--;
			}
			else
			{
				if (error > -EPSILON)
				{
					x += signx;

					error--;
				}
			}

			z += signz;

			error += delta;

			neary = fary;

			fary += deltay;
		}
		while ((neary >= ymin-EPSILON) && (neary <= ymax+EPSILON) &&
		       (x >= xmin) && (x <= xmax) &&
		       (z >= zmin) && (z <= zmax));
	}

	return(found);
}



/*****************************************************************************
*
* FUNCTION
*
*   block_traversal
*
* INPUT
*
*   Ray    - Current ray
*   HField - Height field
*   Start  - Start point for the walk
*
* OUTPUT
*
* RETURNS
*
*   int - true if intersection was found
*   
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Traverse the blocks of the height field.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
*   Aug 1996 : Fixed bug as reported by Dean M. Phillips:
*              "I found a bug in the height field code which resulted
*              in "Illegal grid value in dda_traversal()." messages
*              along with dark vertical lines in the height fields.
*              This turned out to be caused by overlooking the units in
*              which some boundary tests in two different places were
*              made. It was easy to fix.
*
******************************************************************************/

bool HField::block_traversal(const Ray &ray, const VECTOR Start, IStack &HField_Stack, const Ray &RRay, DBL mindist, DBL maxdist, TraceThreadData *Thread)
{
	int xmax, zmax;
	int x, z, nx, nz, signx, signz;
	int found = false;
	int dx_zero, dz_zero;
	DBL px, pz, dx, dy, dz;
	DBL maxdv;
	DBL ymin, ymax, y1, y2;
	DBL neary, fary;
	DBL k1, k2, dist;
	VECTOR nearP, farP;
	HFIELD_BLOCK *Block;

	px = Start[X];
	pz = Start[Z];

	dx = ray.Direction[X];
	dy = ray.Direction[Y];
	dz = ray.Direction[Z];

	maxdv = (dx > dz) ? dx : dz;

	/* First test for 'perpendicular' rays. */

	if ((fabs(dx) < EPSILON) && (fabs(dz) < EPSILON))
	{
		x = (int)px;
		z = (int)pz;

		neary = Start[Y];

		if (dy >= 0.0)
		{
			fary = 65536.0;
		}
		else
		{
			fary = 0.0;
		}

		return intersect_pixel(x, z, ray, min(neary, fary), max(neary, fary), HField_Stack, RRay, mindist, maxdist, Thread);
	}

	/* If we don't have blocks we just step through the grid. */

	if ((Data->block_max_x <= 1) && (Data->block_max_z <= 1))
	{
		return dda_traversal(ray, Start, &Data->Block[0][0], HField_Stack, RRay, mindist, maxdist, Thread);
	}

	/* Get dimensions of grid. */

	xmax = Data->block_max_x;
	zmax = Data->block_max_z;

	ymin = (DBL)Data->min_y - EPSILON;
	ymax = (DBL)Data->max_y + EPSILON;

	dx_zero = (fabs(dx) < EPSILON);
	dz_zero = (fabs(dz) < EPSILON);

	signx = sign(dx);
	signz = sign(dz);

	/* Walk on the block grid. */

	px /= Data->block_width_x;
	pz /= Data->block_width_z;

	x = (int)px;
	z = (int)pz;

	Assign_Vector(nearP, Start);

	neary = Start[Y];

	/*
	 * Here comes the block walk algorithm.
	 */

	do
	{
#ifdef HFIELD_EXTRA_STATS
		Thread->Stats()[Ray_HField_Block_Tests]++;
#endif

		/* Get current block. */

		Block = &Data->Block[z][x];

		/* Intersect ray with bounding planes. */

		if (dx_zero)
		{
			k1 = BOUND_HUGE;
		}
		else
		{
			if (signx >= 0)
			{
				k1 = ((DBL)Block->xmax + 1.0 - ray.Origin[X]) / dx;
			}
			else
			{
				k1 = ((DBL)Block->xmin - ray.Origin[X]) / dx;
			}
		}

		if (dz_zero)
		{
			k2 = BOUND_HUGE;
		}
		else
		{
			if (signz >= 0)
			{
				k2 = ((DBL)Block->zmax + 1.0 - ray.Origin[Z]) / dz;
			}
			else
			{
				k2 = ((DBL)Block->zmin - ray.Origin[Z]) / dz;
			}
		}

		/* Figure out the indices of the next block. */

		if (dz_zero || ((!dx_zero) && (k1<k2 - EPSILON / maxdv) && (k1>0.0)))
/*		if ((k1 < k2 - EPSILON / maxdv) && (k1 > 0.0)) */
		{
			/* Step along the x-axis. */

			dist = k1;

			nx = x + signx;
			nz = z;
		}
		else
		{
			if (dz_zero || ((!dx_zero) && (k1<k2 + EPSILON / maxdv) && (k1>0.0)))
/*			if ((k1 < k2 + EPSILON / maxdv) && (k1 > 0.0))  */
			{
				/* Step along both axis (very rare case). */

				dist = k1;

				nx = x + signx;
				nz = z + signz;
			}
			else
			{
				/* Step along the z-axis. */

				dist = k2;

				nx = x;
				nz = z + signz;
			}
		}

		/* Get point where ray leaves current block. */

		VEvaluateRay(farP, ray.Origin, dist, ray.Direction);

		fary = farP[Y];

		if (neary < fary)
		{
			y1 = neary;
			y2 = fary;
		}
		else
		{
			y1 = fary;
			y2 = neary;
		}

		/* Can we hit current block at all? */

		if ((y1 <= (DBL)Block->ymax + EPSILON) && (y2 >= (DBL)Block->ymin - EPSILON))
		{
			/* Test current block. */

#ifdef HFIELD_EXTRA_STATS
			Thread->Stats()[Ray_HField_Block_Tests_Succeeded]++;
#endif

			if (dda_traversal(ray, nearP, &Data->Block[z][x], HField_Stack, RRay, mindist, maxdist, Thread))
			{
				if (Type & IS_CHILD_OBJECT)
				{
					found = true;
				}
				else
				{
					return(true);
				}
			}
		}

		/* Step to next block. */

		x = nx;
		z = nz;

		Assign_Vector(nearP, farP);

		neary = fary;
	}
	while ((x >= 0) && (x < xmax) && (z >= 0) && (z < zmax) && (neary >= ymin) && (neary <= ymax));

	return(found);
}

}
