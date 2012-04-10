/*******************************************************************************
 * camera.cpp
 *
 * This module implements methods for managing the viewpoint.
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
 * $File: //depot/povray/smp/source/backend/scene/camera.cpp $
 * $Revision: #20 $
 * $Change: 5136 $
 * $DateTime: 2010/09/02 11:36:18 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/scene/camera.h"
#include "backend/scene/objects.h"
#include "backend/texture/normal.h"
#include "backend/texture/pigment.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
*
* FUNCTION
*
*   Translate_Camera
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
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

void Camera::Translate(const VECTOR Vector)
{
	VAddEq(Location, Vector);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Camera
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
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

void Camera::Rotate(const VECTOR Vector)
{
	TRANSFORM Trans;

	Compute_Rotation_Transform(&Trans, Vector);
	Transform(&Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Camera
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
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

void Camera::Scale(const VECTOR Vector)
{
	TRANSFORM Trans;

	Compute_Scaling_Transform(&Trans, Vector);
	Transform(&Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Camera
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
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

void Camera::Transform(const TRANSFORM *Trans)
{
	MTransPoint(Location, Location, Trans);
	MTransDirection(Direction, Direction, Trans);
	MTransDirection(Up, Up, Trans);
	MTransDirection(Right, Right, Trans);
}



/*****************************************************************************
*
* METHOD
*
*   Camera::Init
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
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

void Camera::Init()
{
	Make_Vector(Location,    0.0,  0.0, 0.0);
	Make_Vector(Direction,   0.0,  0.0, 1.0);
	Make_Vector(Up,          0.0,  1.0, 0.0);
	Make_Vector(Right,       1.33, 0.0, 0.0);
	Make_Vector(Sky,         0.0,  1.0, 0.0);
	Make_Vector(Look_At,     0.0,  0.0, 1.0);
	Make_Vector(Focal_Point, 0.0,  0.0, 1.0);

	/* Init focal blur stuff (not used by default). */
	Blur_Samples        = 0;
	Blur_Samples_Min    = 0;
	Confidence          = 0.9;
	Variance            = 1.0 / 10000.0;
	Aperture            = 0.0;
	Focal_Distance      = -1.0;

	/* Set default camera type and viewing angle. [DB 7/94] */
	Type = PERSPECTIVE_CAMERA;
	Angle = 90.0;

	/* Default view angle for spherical camera. [MH 6/99] */
	H_Angle = 360;
	V_Angle = 180;

	/* Do not perturb primary rays by default. [DB 7/94] */
	Tnormal = NULL;

	Bokeh = NULL; // no user-defined bokeh by default

	Trans = Create_Transform();

	Rays_Per_Pixel = 1;
	Face_Distribution_Method = 0;
	Smooth = false;
	Max_Ray_Distance = 0.0;
}

/*****************************************************************************
*
* FUNCTION
*
*   Create_Camera
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
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

Camera::Camera()
{
	Init();
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Camera
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   POV-Ray Team
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

Camera& Camera::operator=(const Camera& src)
{
	Assign_Vector(Location, src.Location);
	Assign_Vector(Direction, src.Direction);
	Assign_Vector(Up, src.Up);
	Assign_Vector(Right, src.Right);
	Assign_Vector(Sky, src.Sky);
	Assign_Vector(Look_At, src.Look_At);
	Assign_Vector(Focal_Point, src.Focal_Point);

	Focal_Distance = src.Focal_Distance;
	Aperture = src.Aperture;
	Blur_Samples = src.Blur_Samples;
	Blur_Samples_Min = src.Blur_Samples_Min;
	Confidence = src.Confidence;
	Variance = src.Variance;
	Type = src.Type;
	Angle = src.Angle;
	H_Angle = src.H_Angle;
	V_Angle = src.V_Angle;

	if (Tnormal != NULL)
		Destroy_Tnormal(Tnormal);
	Tnormal = src.Tnormal ? Copy_Tnormal(src.Tnormal) : NULL;
	if (Trans != NULL)
		Destroy_Transform(Trans);
	Trans = src.Trans ? Copy_Transform(src.Trans) : NULL;

	if (Bokeh != NULL)
		Destroy_Pigment(Bokeh);
	Bokeh = src.Bokeh ? Copy_Pigment(src.Bokeh) : NULL;

	for (std::vector<ObjectPtr>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
		Destroy_Object(*it);
	Meshes.clear();
	for (std::vector<ObjectPtr>::const_iterator it = src.Meshes.begin(); it != src.Meshes.end(); it++)
		Meshes.push_back(Copy_Object(*it));
	Face_Distribution_Method = src.Face_Distribution_Method;
	Rays_Per_Pixel = src.Rays_Per_Pixel;
	Max_Ray_Distance = src.Max_Ray_Distance;
	Mesh_Index = src.Mesh_Index;
	for (int i = 0; i < 10; i++)
	{
		U_Xref[i] = src.U_Xref[i];
		V_Xref[i] = src.V_Xref[i];
	}
	Smooth = src.Smooth;

	return *this;
}

Camera::Camera(const Camera& src)
{
	Tnormal = NULL;
	Trans = NULL;
	Bokeh = NULL;
	operator=(src);
}

/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Camera
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

Camera::~Camera()
{
	Destroy_Tnormal(Tnormal);
	Destroy_Transform(Trans);
	Destroy_Pigment(Bokeh);
	for (std::vector<ObjectPtr>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
		Destroy_Object(*it);
	Meshes.clear();
}

}
