/*******************************************************************************
 * threaddata.cpp
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
 * $File: //depot/povray/smp/source/backend/scene/threaddata.cpp $
 * $Revision: #51 $
 * $Change: 5467 $
 * $DateTime: 2011/08/14 10:39:59 $
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
#include "backend/scene/threaddata.h"
#include "backend/scene/view.h"
#include "backend/scene/scene.h"
#include "backend/scene/objects.h"
#include "backend/shape/fractal.h"
#include "backend/shape/blob.h"
#include "backend/shape/isosurf.h"
#include "backend/texture/texture.h"
#include "backend/vm/fnpovfpu.h"
#include "backend/bounding/bcyl.h"
#include "backend/support/statistics.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

SceneThreadData::SceneThreadData(shared_ptr<SceneData> sd): sceneData(sd)
{
	for(int i = 0 ; i < 4 ; i++)
		Fractal_IStack[i] = NULL;
	Fractal::Allocate_Iteration_Stack(Fractal_IStack, sceneData->Fractal_Iteration_Stack_Length);
	Max_Blob_Queue_Size = 1;
	Blob_Coefficient_Count = sceneData->Max_Blob_Components * 5;
	Blob_Interval_Count = sceneData->Max_Blob_Components * 2;
	Blob_Queue = (void **)POV_MALLOC(sizeof(void **), "Blob Queue");
	Blob_Coefficients = (DBL *)POV_MALLOC(sizeof(DBL) * Blob_Coefficient_Count, "Blob Coefficients");
	Blob_Intervals = new Blob_Interval_Struct [Blob_Interval_Count];
	isosurfaceData = (ISO_ThreadData *)POV_MALLOC(sizeof(ISO_ThreadData), "Isosurface Data");
	isosurfaceData->ctx = NULL;
	isosurfaceData->current = NULL;
	isosurfaceData->cache = false;
	isosurfaceData->Inv3 = 1;
	isosurfaceData->fmax = 0.0;
	isosurfaceData->tl = 0.0;
	isosurfaceData->Vlength = 0.0;

	functionContext = sceneData->functionVM->NewContext(this);
	functionPatternContext.resize(sceneData->functionPatternCount);

	BCyl_Intervals = POV_MALLOC (4*sceneData->Max_Bounding_Cylinders*sizeof(BCYL_INT), "lathe intersection list");
	BCyl_RInt = POV_MALLOC (2*sceneData->Max_Bounding_Cylinders*sizeof(BCYL_INT), "lathe intersection list");
	BCyl_HInt = POV_MALLOC (2*sceneData->Max_Bounding_Cylinders*sizeof(BCYL_INT), "lathe intersection list");

	Facets_Last_Seed = 0x80000000;

	timeType = kUnknownTime;
	cpuTime = 0;
	realTime = 0;

	qualityFlags = QUALITY_9;

	for(vector<LightSource *>::iterator it = sceneData->lightSources.begin(); it != sceneData->lightSources.end(); it++)
		lightSources.push_back(static_cast<LightSource *> (Copy_Object(*it))) ;

	// all of these are for photons
	LightSource *photonLight = NULL;
	ObjectPtr photonObject = NULL;
	litObjectIgnoresPhotons = false;
	hitObject = false;    // did we hit the target object? (for autostop)
	photonSpread = 0.0; // photon spread (in radians)
	photonDepth = 0.0;  // total distance from light to intersection
	passThruThis = false;           // is this a pass-through object?
	passThruPrev = false;           // was the previous object pass-through?
	Light_Is_Global = false;       // is the current light global? (not part of a light_group?)

	CrCache_MaxAge = 1;
	progress_index = 0;

	surfacePhotonMap = new PhotonMap();
	mediaPhotonMap = new PhotonMap();

	// advise the crackle cache's unordered_map that we don't mind hash collisions
	// while this is a very high load factor, the simple fact is that the cost of
	// allocating memory at render time (each insert into the table requires an alloc
	// as the container doesn't pre-emptively allocate, unlike e.g. std::vector) is
	// quite high, particularly when we have multiple threads contending for the heap
	// lock.
	Crackle_Cache.max_load_factor(50.0);

	numberOfWaves = sd->numberOfWaves;
	Initialize_Waves(waveFrequencies, waveSources, numberOfWaves);
}

SceneThreadData::~SceneThreadData()
{
	sceneData->functionVM->DeleteContext(functionContext);

	POV_FREE(BCyl_HInt);
	POV_FREE(BCyl_RInt);
	POV_FREE(BCyl_Intervals);
	POV_FREE(Blob_Coefficients);
	POV_FREE(Blob_Queue);
	POV_FREE(isosurfaceData);
	Fractal::Free_Iteration_Stack(Fractal_IStack);
	delete surfacePhotonMap;
	delete mediaPhotonMap;
	delete[] (Blob_Interval_Struct *) Blob_Intervals;
	for(vector<LightSource *>::iterator it = lightSources.begin(); it != lightSources.end(); it++)
		Destroy_Object(*it);
}

void SceneThreadData::AfterTile()
{
	Crackle_Cache_Type::iterator    it;

	// this serves as a render block index
	progress_index++;

	// probably we ought to have a means for the end-user to choose the preferred maximum bytes reserved for the cache
	// for now, we have hard-coded values. we also do not discard any entries that are from the current block, even if
	// the cache size is exceeded. also, note that the cache size is per-thread. finally, don't forget that erasing
	// elements doesn't in and of itself return the freed memory to the heap.
	if (Crackle_Cache.size() * sizeof(Crackle_Cache_Type::value_type) < 15 * 1024 * 1024)
		return;
	while (Crackle_Cache.size() * sizeof(Crackle_Cache_Type::value_type) > 10 * 1024 * 1024)
	{
		// search the cache for the oldest entries
		int oldest = std::numeric_limits<int>::max();
		for (it = Crackle_Cache.begin(); it != Crackle_Cache.end(); it++)
			if (it->second.last_used < oldest)
				oldest = (int) it->second.last_used;

		// don't remove any entries from the most recent block
		if (oldest == progress_index - 1)
			break;

		for (it = Crackle_Cache.begin(); it != Crackle_Cache.end(); )
		{
			if (it->second.last_used == oldest)
			{
				it = Crackle_Cache.erase(it);
				continue;
			}
			it++;
		}
	}
}

ViewThreadData::ViewThreadData(ViewData *vd) :
	SceneThreadData(vd->GetSceneData()),
	viewData(vd)
{
}

ViewThreadData::~ViewThreadData()
{
}

unsigned int ViewThreadData::GetWidth() const
{
	return viewData->GetWidth();
}

unsigned int ViewThreadData::GetHeight() const
{
	return viewData->GetHeight();
}

const POVRect& ViewThreadData::GetRenderArea()
{
	return viewData->GetRenderArea();
}

}
