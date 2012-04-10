/*******************************************************************************
 * interior.cpp
 *
 * This module contains all functions for interior stuff.
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
 * $File: //depot/povray/smp/source/backend/interior/interior.cpp $
 * $Revision: #21 $
 * $Change: 5408 $
 * $DateTime: 2011/02/21 15:17:08 $
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
#include "backend/interior/interior.h"
#include "backend/texture/texture.h"
#include "backend/lighting/subsurface.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/* How many subrays to trace for dispersive media */
#define DEFAULT_DISP_NELEMS  7

Interior::Interior()
{
	References = 1;

	IOR = 0.0;
	Old_Refract = 1.0;

	Dispersion  = 1.0;
	Disp_NElems = DEFAULT_DISP_NELEMS;

	Caustics = 0.0;

	Fade_Distance = 0.0;
	Fade_Power    = 0.0;

	hollow = false;

	subsurface = shared_ptr<SubsurfaceInterior>();
}

Interior::Interior(const Interior& source)
{
	References = 1;
	Disp_NElems = source.Disp_NElems;
	Dispersion = source.Dispersion;
	Old_Refract = source.Old_Refract;
	Fade_Distance = source.Fade_Distance;
	Fade_Power = source.Fade_Power;
	Fade_Colour = source.Fade_Colour;
	media = source.media;
	hollow = source.hollow;
	IOR = source.IOR;
	subsurface = shared_ptr<SubsurfaceInterior>(source.subsurface);
	Caustics = source.Caustics;
}

Interior::~Interior()
{
}

void Interior::Transform(const TRANSFORM *trans)
{
	for(vector<Media>::iterator i(media.begin());i != media.end(); i++)
		i->Transform(trans);
}

void Interior::PostProcess()
{
	for(vector<Media>::iterator i(media.begin());i != media.end(); i++)
		i->PostProcess();
}

/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Interior
*
* INPUT
*
*   Interior - interior to destroy
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
*   Destroy an interior.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Interior(Interior *interior)
{
	if((interior != NULL) && (--(interior->References) == 0))
		delete interior;
}

/*****************************************************************************
*
* FUNCTION
*
*   Copy_Interior_Pointer
*
* INPUT
*
*   Old - interior to copy
*
* OUTPUT
*
* RETURNS
*
*   INTERIOR * - new interior
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy an interior by increasing number of references.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

Interior *Copy_Interior_Pointer(Interior *Old)
{
	if (Old != NULL)
	{
		Old->References++;
	}

	return(Old);
}

MATERIAL *Create_Material()
{
	MATERIAL *New;

	New = (MATERIAL *)POV_MALLOC(sizeof(MATERIAL), "material");

	New->Texture  = NULL;
	New->Interior_Texture  = NULL;
	New->interior = NULL;

	return(New);
}

MATERIAL *Copy_Material(MATERIAL *Old)
{
	MATERIAL *New;

	if (Old != NULL)
	{
		New = Create_Material();

		*New = *Old;

		New->Texture  = Copy_Textures(Old->Texture);
		New->Interior_Texture  = Copy_Textures(Old->Interior_Texture);
		if (Old->interior != NULL)
			New->interior = new Interior(*(Old->interior));

		return(New);
	}
	else
	{
		return(NULL);
	}
}

void Destroy_Material(MATERIAL *Material)
{
	if (Material != NULL)
	{
		Destroy_Textures(Material->Texture);
		Destroy_Textures(Material->Interior_Texture);
		Destroy_Interior(Material->interior);

		POV_FREE(Material);
	}
}

}
