/*******************************************************************************
 * atmosph.cpp
 *
 * This module contains all functions for atmospheric effects.
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
 * $File: //depot/povray/smp/source/backend/scene/atmosph.cpp $
 * $Revision: #21 $
 * $Change: 5573 $
 * $DateTime: 2011/12/02 21:56:12 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/scene/atmosph.h"
#include "backend/texture/texture.h"
#include "backend/texture/pigment.h"
#include "backend/pattern/warps.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"

#include "povray.h" // TODO

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
*
* FUNCTION
*
*   Create_Fog
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   FOG * - created fog
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a fog.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

FOG *Create_Fog()
{
	FOG *New;

	New = (FOG *)POV_MALLOC(sizeof(FOG), "fog");

	New->Type = ORIG_FOG;

	New->Distance = 0.0;
	New->Alt      = 0.0;
	New->Offset   = 0.0;

	New->colour.clear();

	Make_Vector(New->Up, 0.0, 1.0, 0.0);

	New->Turb = NULL;
	New->Turb_Depth = 0.5;

	New->Next = NULL;

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Fog
*
* INPUT
*
*   Old - fog to copy
*
* OUTPUT
*
* RETURNS
*
*   FOG * - new fog
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy a fog.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

FOG *Copy_Fog(FOG *Old)
{
	FOG *New;

	New = Create_Fog();

	*New = *Old;

	New->Turb = (TURB *)Copy_Warps(((WARP *)Old->Turb));

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Fog
*
* INPUT
*
*   Fog - fog to destroy
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
*   Destroy a fog.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Fog(FOG *Fog)
{
	if (Fog != NULL)
	{
		Destroy_Turb(Fog->Turb);

		POV_FREE(Fog);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Rainbow
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   RAINBOW * - created rainbow
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a rainbow.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

RAINBOW *Create_Rainbow()
{
	RAINBOW *New;

	New = (RAINBOW *)POV_MALLOC(sizeof(RAINBOW), "fog");

	New->Distance = MAX_DISTANCE;
	New->Jitter   = 0.0;
	New->Angle    = 0.0;
	New->Width    = 0.0;

	New->Falloff_Width  = 0.0;
	New->Arc_Angle      = 180.0;
	New->Falloff_Angle  = 180.0;

	New->Pigment = NULL;

	Make_Vector(New->Antisolar_Vector, 0.0, 0.0, 0.0);

	Make_Vector(New->Right_Vector, 1.0, 0.0, 0.0);
	Make_Vector(New->Up_Vector, 0.0, 1.0, 0.0);

	New->Next = NULL;

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Rainbow
*
* INPUT
*
*   Old - rainbow to copy
*
* OUTPUT
*
* RETURNS
*
*   RAINBOW * - new rainbow
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy a rainbow.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

RAINBOW *Copy_Rainbow(RAINBOW *Old)
{
	RAINBOW *New;

	New = Create_Rainbow();

	*New = *Old;

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Rainbow
*
* INPUT
*
*   Rainbow - rainbow to destroy
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
*   Destroy a rainbow.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Rainbow(RAINBOW *Rainbow)
{
	if (Rainbow != NULL)
	{
		Destroy_Pigment(Rainbow->Pigment);

		POV_FREE(Rainbow);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Skysphere
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   SKYSPHERE * - created skysphere
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a skysphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

SKYSPHERE *Create_Skysphere()
{
	SKYSPHERE *New;

	New = (SKYSPHERE *)POV_MALLOC(sizeof(SKYSPHERE), "fog");

	New->Count = 0;
	New->Emission = RGBColour(1.0);

	New->Pigments = NULL;

	New->Trans = Create_Transform();

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Skysphere
*
* INPUT
*
*   Old - skysphere to copy
*
* OUTPUT
*
* RETURNS
*
*   SKYSPHERE * - copied skysphere
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy a skysphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

SKYSPHERE *Copy_Skysphere(SKYSPHERE *Old)
{
	int i;
	SKYSPHERE *New;

	New = Create_Skysphere();

	Destroy_Transform(New->Trans);

	*New = *Old;

	New->Trans = Copy_Transform(Old->Trans);

	if (New->Count > 0)
	{
		New->Pigments = (PIGMENT **)POV_MALLOC(New->Count*sizeof(PIGMENT *), "skysphere pigment");

		for (i = 0; i < New->Count; i++)
		{
			New->Pigments[i] = Copy_Pigment(Old->Pigments[i]);
		}
	}

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Skysphere
*
* INPUT
*
*   Skysphere - skysphere to destroy
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
*   Destroy a skysphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Skysphere(SKYSPHERE *Skysphere)
{
	int i;

	if (Skysphere != NULL)
	{
		for (i = 0; i < Skysphere->Count; i++)
		{
			Destroy_Pigment(Skysphere->Pigments[i]);
		}

		POV_FREE(Skysphere->Pigments);

		Destroy_Transform(Skysphere->Trans);

		POV_FREE(Skysphere);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Skysphere
*
* INPUT
*
*   Vector - Rotation vector
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Rotate a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Rotate_Skysphere(SKYSPHERE *Skysphere, VECTOR Vector)
{
	TRANSFORM Trans;

	Compute_Rotation_Transform(&Trans, Vector);

	Transform_Skysphere(Skysphere, &Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Skysphere
*
* INPUT
*
*   Vector - Scaling vector
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Scale a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Scale_Skysphere(SKYSPHERE *Skysphere, VECTOR Vector)
{
	TRANSFORM Trans;

	Compute_Scaling_Transform(&Trans, Vector);

	Transform_Skysphere(Skysphere, &Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Skysphere
*
* INPUT
*
*   Vector - Translation vector
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Translate a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Translate_Skysphere(SKYSPHERE *Skysphere, VECTOR Vector)
{
	TRANSFORM Trans;

	Compute_Translation_Transform(&Trans, Vector);

	Transform_Skysphere(Skysphere, &Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Skysphere
*
* INPUT
*
*   Trans  - Pointer to transformation
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Transform a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Transform_Skysphere(SKYSPHERE *Skysphere, TRANSFORM *Trans)
{
	if (Skysphere->Trans == NULL)
	{
		Skysphere->Trans = Create_Transform();
	}

	Compose_Transforms(Skysphere->Trans, Trans);
}

}
