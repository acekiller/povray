/*******************************************************************************
 * photonsortingtask.cpp
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
 * $File: //depot/povray/smp/source/backend/lighting/photonsortingtask.cpp $
 * $Revision: #14 $
 * $Change: 5468 $
 * $DateTime: 2011/08/14 12:25:36 $
 * $Author: jgrimbert $
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
#include "backend/lighting/photonsortingtask.h"
#include "backend/lighting/photonshootingstrategy.h"
#include "lightgrp.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*
    If you pass a NULL for the "strategy" parameter, then this will
    load the photon map from a file.
    Otherwise, it will:
      1) merge
      2) sort
      3) compute gather options
      4) clean up memory (delete the non-merged maps and delete the strategy)
*/
PhotonSortingTask::PhotonSortingTask(ViewData *vd, vector<PhotonMap*> surfaceMaps, vector<PhotonMap*> mediaMaps, PhotonShootingStrategy* strategy) :
	RenderTask(vd),
	surfaceMaps(surfaceMaps),
	mediaMaps(mediaMaps),
	strategy(strategy),
	messageFactory(10, 370, "Photon", vd->GetSceneData()->backendAddress, vd->GetSceneData()->frontendAddress, vd->GetSceneData()->sceneId, 0), // TODO FIXME - Values need to come from the correct place!
	cooperate(*this)
{
}

PhotonSortingTask::~PhotonSortingTask()
{
}

void PhotonSortingTask::SendProgress(void)
{
#if 0
	// TODO FIXME PHOTONS
	// for now, we won't send this, as it can be confusing on the front-end due to out-of-order delivery from multiple threads.
	// we need to create a new progress message for sorting.
	if (timer.ElapsedRealTime() > 1000)
	{
		timer.Reset();
		POVMS_Object obj(kPOVObjectClass_PhotonProgress);
		obj.SetInt(kPOVAttrib_CurrentPhotonCount, (GetSceneData()->surfacePhotonMap.numPhotons + GetSceneData()->mediaPhotonMap.numPhotons));
		RenderBackend::SendViewOutput(GetViewData()->GetViewId(), GetSceneData()->frontendAddress, kPOVMsgIdent_Progress, obj);
	}
#endif
}

void PhotonSortingTask::Run()
{
	// quit right away if photons not enabled
	if (!GetSceneData()->photonSettings.photonsEnabled) return;

	Cooperate();

	if(strategy!=NULL)
	{
		delete strategy;
		sortPhotonMap();
	}
	else
	{
		if (!this->load())
			messageFactory.Error(POV_EXCEPTION_STRING("Failed to load photon map from disk"), "Could not load photon map (%s)",GetSceneData()->photonSettings.fileName);

		// set photon options automatically
		if (GetSceneData()->surfacePhotonMap.numPhotons>0)
			GetSceneData()->surfacePhotonMap.setGatherOptions(GetSceneData()->photonSettings,false);
		if (GetSceneData()->mediaPhotonMap.numPhotons>0)
			GetSceneData()->mediaPhotonMap.setGatherOptions(GetSceneData()->photonSettings,true);
	}

	// good idea to make sure all warnings and errors arrive frontend now [trf]
	SendProgress();
	Cooperate();
}

void PhotonSortingTask::Stopped()
{
	// nothing to do for now [trf]
}

void PhotonSortingTask::Finish()
{
	GetViewDataPtr()->timeType = SceneThreadData::kPhotonTime;
	GetViewDataPtr()->realTime = ConsumedRealTime();
	GetViewDataPtr()->cpuTime = ConsumedCPUTime();
}


void PhotonSortingTask::sortPhotonMap()
{
	vector<PhotonMap*>::iterator mapIter;
	for(mapIter = surfaceMaps.begin(); mapIter != surfaceMaps.end(); mapIter++)
	{
		GetSceneData()->surfacePhotonMap.mergeMap(*mapIter);
		//delete (*mapIter);
	}
	for(mapIter = mediaMaps.begin(); mapIter != mediaMaps.end(); mapIter++)
	{
		GetSceneData()->mediaPhotonMap.mergeMap(*mapIter);
		//delete (*mapIter);
	}
	
	/* now actually build the kd-tree by sorting the array of photons */
	if (GetSceneData()->surfacePhotonMap.numPhotons>0)
	{
	//povwin::WIN32_DEBUG_FILE_OUTPUT("\n\nsurfacePhotonMap.buildTree about to be called\n");

		GetSceneData()->surfacePhotonMap.buildTree();
		GetSceneData()->surfacePhotonMap.setGatherOptions(GetSceneData()->photonSettings,false);
//		povwin::WIN32_DEBUG_FILE_OUTPUT("gatherNumSteps: %d\n",GetSceneData()->surfacePhotonMap.gatherNumSteps);
//		povwin::WIN32_DEBUG_FILE_OUTPUT("gatherRadStep: %lf\n",GetSceneData()->surfacePhotonMap.gatherRadStep);
//		povwin::WIN32_DEBUG_FILE_OUTPUT("minGatherRad: %lf\n",GetSceneData()->surfacePhotonMap.minGatherRad);
//		povwin::WIN32_DEBUG_FILE_OUTPUT("minGatherRadMult: %lf\n",GetSceneData()->surfacePhotonMap.minGatherRadMult);
//		povwin::WIN32_DEBUG_FILE_OUTPUT("numBlocks: %d\n",GetSceneData()->surfacePhotonMap.numBlocks);
//		povwin::WIN32_DEBUG_FILE_OUTPUT("numPhotons: %d\n",GetSceneData()->surfacePhotonMap.numPhotons);
	}

#ifdef GLOBAL_PHOTONS
	/* ----------- global photons ------------- */
	if (globalPhotonMap.numPhotons>0)
	{
		globalPhotonMap.buildTree();
		globalPhotonMap.setGatherOptions(false);
	}
#endif

	/* ----------- media photons ------------- */
	if (GetSceneData()->mediaPhotonMap.numPhotons>0)
	{
		GetSceneData()->mediaPhotonMap.buildTree();
		GetSceneData()->mediaPhotonMap.setGatherOptions(GetSceneData()->photonSettings,true);
	}

	if (GetSceneData()->surfacePhotonMap.numPhotons+
#ifdef GLOBAL_PHOTONS
	    globalPhotonMap.numPhotons+
#endif
	    GetSceneData()->mediaPhotonMap.numPhotons > 0)
	{
		/* should we load the photon map now that it is built? */
		if (GetSceneData()->photonSettings.fileName && !GetSceneData()->photonSettings.loadFile)
		{
			/* status bar for user */
//			Send_Progress("Saving Photon Maps", PROGRESS_SAVING_PHOTON_MAPS);
			if (!this->save())
				messageFactory.Warning(0,"Could not save photon map.");
		}
	}
	else
	{
		if (GetSceneData()->photonSettings.fileName && !GetSceneData()->photonSettings.loadFile)
			messageFactory.Warning(0,"Could not save photon map - no photons!");
	}
}


/* savePhotonMap()

  Saves the caustic photon map to a file.

  Preconditions:
    InitBacktraceEverything was called
    the photon map has been built and balanced
    photonSettings.fileName contains the filename to save

  Postconditions:
    Returns 1 if success, 0 if failure.
    If success, the photon map has been written to the file.
*/
int PhotonSortingTask::save()
{
	Photon *ph;
	FILE *f;
	int i;
	size_t err;

	f = fopen(GetSceneData()->photonSettings.fileName, "wb");
	if (!f) return 0;

	/* caustic photons */
	fwrite(&GetSceneData()->surfacePhotonMap.numPhotons, sizeof(GetSceneData()->surfacePhotonMap.numPhotons),1,f);
	if (GetSceneData()->surfacePhotonMap.numPhotons>0 && GetSceneData()->surfacePhotonMap.head)
	{
		for(i=0; i<GetSceneData()->surfacePhotonMap.numPhotons; i++)
		{
			ph = &(PHOTON_AMF(GetSceneData()->surfacePhotonMap.head, i));
			err = fwrite(ph, sizeof(Photon), 1, f);

			if (err<=0)
			{
				/* fwrite returned an error! */
				fclose(f);
				return 0;
			}
		}
	}
	else
	{
		messageFactory.PossibleError("Photon map for surface is empty.");
	}

#ifdef GLOBAL_PHOTONS
	/* global photons */
	fwrite(&globalPhotonMap.numPhotons, sizeof(globalPhotonMap.numPhotons),1,f);
	if (globalPhotonMap.numPhotons>0 && globalPhotonMap.head)
	{
		for(i=0; i<globalPhotonMap.numPhotons; i++)
		{
			ph = &(PHOTON_AMF(globalPhotonMap.head, i));
			err = fwrite(ph, sizeof(Photon), 1, f);

			if (err<=0)
			{
				/* fwrite returned an error! */
				fclose(f);
				return 0;
			}
		}
	}
	else
	{
		messageFactory.PossibleError("Global photon map is empty.");
	}
#endif

	/* media photons */
	fwrite(&GetSceneData()->mediaPhotonMap.numPhotons, sizeof(GetSceneData()->mediaPhotonMap.numPhotons),1,f);
	if (GetSceneData()->mediaPhotonMap.numPhotons>0 && GetSceneData()->mediaPhotonMap.head)
	{
		for(i=0; i<GetSceneData()->mediaPhotonMap.numPhotons; i++)
		{
			ph = &(PHOTON_AMF(GetSceneData()->mediaPhotonMap.head, i));
			err = fwrite(ph, sizeof(Photon), 1, f);

			if (err<=0)
			{
				/* fwrite returned an error! */
				fclose(f);
				return 0;
			}
		}
	}
	else
	{
		messageFactory.PossibleError("Photon map for media is empty.");
	}

	fclose(f);
	return true;
}

/* loadPhotonMap()

  Loads the caustic photon map from a file.

  Preconditions:
    InitBacktraceEverything was called
    the photon map is empty
    renderer->sceneData->photonSettings.fileName contains the filename to load

  Postconditions:
    Returns 1 if success, 0 if failure.
    If success, the photon map has been loaded from the file.
    If failure then the render should stop with an error
*/
int PhotonSortingTask::load()
{
	int i;
	size_t err;
	Photon *ph;
	FILE *f;
	int numph;

	if (!GetSceneData()->photonSettings.photonsEnabled) return 0;

	messageFactory.Warning(0,"Starting the load of photon file %s\n",GetSceneData()->photonSettings.fileName);

	f = fopen(GetSceneData()->photonSettings.fileName, "rb");
	if (!f) return 0;

	fread(&numph, sizeof(numph),1,f);

	for(i=0; i<numph; i++)
	{
		ph = GetSceneData()->surfacePhotonMap.AllocatePhoton();
		err = fread(ph, sizeof(Photon), 1, f);

		if (err<=0)
		{
			/* fread returned an error! */
			fclose(f);
			return 0;
		}
	}

	if (!feof(f)) /* for backwards file format compatibility */
	{

#ifdef GLOBAL_PHOTONS
		/* global photons */
		fread(&numph, sizeof(numph),1,f);
		for(i=0; i<numph; i++)
		{
			ph = GetSceneData()->globalPhotonMap.AllocatePhoton();
			err = fread(ph, sizeof(Photon), 1, f);

			if (err<=0)
			{
				/* fread returned an error! */
				fclose(f);
				return 0;
			}
		}
#endif

		/* media photons */
		fread(&numph, sizeof(numph),1,f);
		for(i=0; i<numph; i++)
		{
			ph = GetSceneData()->mediaPhotonMap.AllocatePhoton();
			err = fread(ph, sizeof(Photon), 1, f);

			if (err<=0)
			{
				/* fread returned an error! */
				fclose(f);
				return 0;
			}
		}

	}

	fclose(f);
	return true;
}


}
