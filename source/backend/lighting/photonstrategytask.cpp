/*******************************************************************************
 * photonstrategytask.cpp
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
 * $File: //depot/povray/smp/source/backend/lighting/photonstrategytask.cpp $
 * $Revision: #12 $
 * $Change: 5316 $
 * $DateTime: 2011/01/01 04:54:21 $
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
#include "backend/lighting/photonshootingstrategy.h"
#include "backend/lighting/photonstrategytask.h"
#include "lightgrp.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

PhotonStrategyTask::PhotonStrategyTask(ViewData *vd, PhotonShootingStrategy* strategy) :
	RenderTask(vd),
	strategy(strategy),
	messageFactory(10, 370, "Photon", vd->GetSceneData()->backendAddress, vd->GetSceneData()->frontendAddress, vd->GetSceneData()->sceneId, 0), // TODO FIXME - Values need to come from the correct place!
	cooperate(*this)
{
	// do nothing
}

PhotonStrategyTask::~PhotonStrategyTask()
{
	//delete strategy;
}

void PhotonStrategyTask::SendProgress(void)
{
	if (timer.ElapsedRealTime() > 1000)
	{
		timer.Reset();
		POVMS_Object obj(kPOVObjectClass_PhotonProgress);
		obj.SetInt(kPOVAttrib_CurrentPhotonCount, GetSceneData()->surfacePhotonMap.numPhotons + GetSceneData()->mediaPhotonMap.numPhotons);
		RenderBackend::SendViewOutput(GetViewData()->GetViewId(), GetSceneData()->frontendAddress, kPOVMsgIdent_Progress, obj);
	}
}

void PhotonStrategyTask::Run()
{
	// quit right away if photons not enabled
	if (!GetSceneData()->photonSettings.photonsEnabled) return;

	Cooperate();

	/*  loop through global light sources  */
	GetViewDataPtr()->Light_Is_Global = true;
	for(vector<LightSource *>::iterator Light = GetSceneData()->lightSources.begin(); Light != GetSceneData()->lightSources.end(); Light++)
	{
		if ((*Light)->Light_Type != FILL_LIGHT_SOURCE)
		{
			if ((*Light)->Light_Type == CYLINDER_SOURCE && !(*Light)->Parallel)
				messageFactory.Warning(0,"Cylinder lights should be parallel when used with photons.");

			/* do object-specific lighting */
			SearchThroughObjectsCreateUnits(GetSceneData()->objects, (*Light));
		}

		Cooperate();
	}

	// loop through light_group light sources
/*
	TODO
	GetSceneData()->photonSettings.Light_Is_Global = false;
	for(vector<LightSource *>::iterator Light_Group_Light = GetSceneData()->lightGroupLights.begin(); Light_Group_Light != GetSceneData()->lightGroupLights.end(); Light_Group_Light++)
	{
		Light = Light_Group_Light->Light;

		if (Light->Light_Type == CYLINDER_SOURCE && !Light->Parallel)
			messageFactory.Warning(0,"Cylinder lights should be parallel when used with photons.");

		// do object-specific lighting
		SearchThroughObjectsCreateUnits(GetSceneData()->objects, Light);

		Cooperate();
		SendProgress();
	}
*/
	// good idea to make sure all warnings and errors arrive frontend now [trf]
	Cooperate();

	strategy->start();
}

void PhotonStrategyTask::Stopped()
{
	// nothing to do for now [trf]
}

void PhotonStrategyTask::Finish()
{
	GetViewDataPtr()->timeType = SceneThreadData::kPhotonTime;
	GetViewDataPtr()->realTime = ConsumedRealTime();
	GetViewDataPtr()->cpuTime = ConsumedCPUTime();
}


/*****************************************************************************

 FUNCTION

  SearchThroughObjectsCreateUnits()

  Searches through 'object' and all siblings  and children of 'object' to
  locate objects with PH_TARGET_FLAG set.  This flag means that the object
  receives photons.

  Preconditions:
    Photon mapping initialized (InitBacktraceEverything() called)
    'Object' is a object (with or without siblings)
    'Light' is a light source in the scene

  Postconditions:
    Work units (combination of light + target) have been added to the
    strategy.

******************************************************************************/

void PhotonStrategyTask::SearchThroughObjectsCreateUnits(vector<ObjectPtr>& Objects, LightSource *Light)
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
				strategy->createUnitsForCombo((*Sib), Light, renderDataPtr, sceneData);
			}

			Cooperate();
			SendProgress();
		}
		/* if it has children, check them too */
		else if(((*Sib)->Type & IS_COMPOUND_OBJECT))
		{
			SearchThroughObjectsCreateUnits(((CSG *)(*Sib))->children, Light);
		}
	}
}


}
