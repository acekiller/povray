/*******************************************************************************
 * photonestimationtask.cpp
 *
 * This module implements Photon Mapping.
 *
 * Author: Nathan Kopp
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
 * $File: //depot/povray/smp/source/backend/lighting/photonestimationtask.cpp $
 * $Revision: #10 $
 * $Change: 5088 $
 * $DateTime: 2010/08/05 17:08:44 $
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
#include "base/povms.h"
#include "base/povmsgid.h"
#include "backend/math/vector.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/shape/csg.h"
#include "backend/support/octree.h"
#include "backend/bounding/bbox.h"
#include "backend/scene/threaddata.h"
#include "backend/scene/scene.h"
#include "backend/scene/view.h"
#include "backend/support/msgutil.h"
#include "backend/lighting/point.h"
#include "backend/lighting/photonestimationtask.h"
#include "lightgrp.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

PhotonEstimationTask::PhotonEstimationTask(ViewData *vd) :
	RenderTask(vd),
	cooperate(*this)
{
	photonCountEstimate = 0;
}

PhotonEstimationTask::~PhotonEstimationTask()
{
}


void PhotonEstimationTask::Run()
{
	// quit right away if photons not enabled
	if (!GetSceneData()->photonSettings.photonsEnabled) return;

	if (GetSceneData()->photonSettings.surfaceCount==0) return;

	Cooperate();

	//  COUNT THE PHOTONS
	DBL factor;
	photonCountEstimate = 0.0;

	// global lights
	GetViewDataPtr()->Light_Is_Global = true;
	for(vector<LightSource *>::iterator Light = GetSceneData()->lightSources.begin(); Light != GetSceneData()->lightSources.end(); Light++)
	{
		if((*Light)->Light_Type != FILL_LIGHT_SOURCE)
			SearchThroughObjectsEstimatePhotons(GetSceneData()->objects, *Light);
	}

	// light_group lights
	/*
	TODO
	renderer->sceneData->photonSettings.Light_Is_Global = false;
	for(vector<LightSource *>::iterator Light_Group_Light = renderer->sceneData->lightGroupLights.begin(); Light_Group_Light != renderer->sceneData->lightGroupLights.end(); Light_Group_Light++)
	{
		Light = Light_Group_Light->Light;
		if (Light->Light_Type != FILL_LightSource)
		{
			SearchThroughObjectsEstimatePhotons(GetSceneData()->objects, *Light);
		}
	}
	*/

	factor = (DBL)photonCountEstimate/GetSceneData()->photonSettings.surfaceCount;
	factor = sqrt(factor);
	GetSceneData()->photonSettings.surfaceSeparation *= factor;


	// good idea to make sure all warnings and errors arrive frontend now [trf]
	Cooperate();
}

void PhotonEstimationTask::Stopped()
{
	// nothing to do for now [trf]
}

void PhotonEstimationTask::Finish()
{
	GetViewDataPtr()->timeType = SceneThreadData::kPhotonTime;
	GetViewDataPtr()->realTime = ConsumedRealTime();
	GetViewDataPtr()->cpuTime = ConsumedCPUTime();
}


void PhotonEstimationTask::SearchThroughObjectsEstimatePhotons(vector<ObjectPtr>& Objects, LightSource *Light)
{
	ViewThreadData *renderDataPtr = GetViewDataPtr();
	shared_ptr<SceneData> sceneData = GetSceneData();

	/* check this object and all siblings */
	for(vector<ObjectPtr>::iterator Sib = Objects.begin(); Sib != Objects.end(); Sib++)
	{
		if(Test_Flag((*Sib), PH_TARGET_FLAG) &&
		    !((*Sib)->Type & LIGHT_SOURCE_OBJECT))
		{
			/* do not shoot photons if global lights are turned off for ObjectPtr */
			if(!Test_Flag((*Sib), NO_GLOBAL_LIGHTS_FLAG))
			{
				EstimatePhotonsForObjectAndLight((*Sib), Light);
			}

			Cooperate();
		}
		/* if it has children, check them too */
		else if(((*Sib)->Type & IS_COMPOUND_OBJECT))
		{
			SearchThroughObjectsEstimatePhotons(((CSG *)(*Sib))->children, Light);
		}
	}
}

void PhotonEstimationTask::EstimatePhotonsForObjectAndLight(ObjectPtr Object, LightSource *Light)
{
	int mergedFlags=0;             /* merged flags to see if we should shoot photons */
	ViewThreadData *renderDataPtr = GetViewDataPtr();

	/* first, check on various flags... make sure all is a go for this ObjectPtr */
	LightTargetCombo combo(Light,Object);
	mergedFlags = combo.computeMergedFlags();

	if (!( ((mergedFlags & PH_RFR_ON_FLAG) && !(mergedFlags & PH_RFR_OFF_FLAG)) ||
	       ((mergedFlags & PH_RFL_ON_FLAG) && !(mergedFlags & PH_RFL_OFF_FLAG)) ))
		/* it is a no-go for this object... bail out now */
		return;

	if(!Object) return;

	ShootingDirection shootingDirection(Light,Object);
	shootingDirection.compute();

	/* calculate the spacial separation (spread) */
	renderDataPtr->photonSpread = combo.target->Ph_Density*GetSceneData()->photonSettings.surfaceSeparation;

	/* if rays aren't parallel, divide by dist so we get separation at a distance of 1 unit */
	if (!combo.light->Parallel)
	{
		renderDataPtr->photonSpread /= shootingDirection.dist;
	}

	/* try to guess the number of photons */
	DBL x=shootingDirection.rad / (combo.target->Ph_Density*GetSceneData()->photonSettings.surfaceSeparation);
	x=x*x*M_PI;

	if ( ((mergedFlags & PH_RFR_ON_FLAG) && !(mergedFlags & PH_RFR_OFF_FLAG)) &&
	     ((mergedFlags & PH_RFL_ON_FLAG) && !(mergedFlags & PH_RFL_OFF_FLAG)) )
	{
		x *= 1.5;  /* assume 2 times as many photons with both reflection & refraction */
	}

	if ( !Test_Flag(combo.target, PH_IGNORE_PHOTONS_FLAG) )
	{
		if ( ((mergedFlags & PH_RFR_ON_FLAG) && !(mergedFlags & PH_RFR_OFF_FLAG)) )
		{
			if ( ((mergedFlags & PH_RFL_ON_FLAG) && !(mergedFlags & PH_RFL_OFF_FLAG)) )
				x *= 3;  /* assume 3 times as many photons if ignore_photons not used */
			else
				x *= 2;  /* assume less for only refraction */
		}
	}

	x *= 0.5;  /* assume 1/2 of photons hit target ObjectPtr */

	photonCountEstimate += x;
}

}
