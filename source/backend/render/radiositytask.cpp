/*******************************************************************************
 * radiositytask.cpp
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
 * $File: //depot/povray/smp/source/backend/render/radiositytask.cpp $
 * $Revision: #29 $
 * $Change: 5028 $
 * $DateTime: 2010/06/21 18:02:23 $
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

#include <boost/thread.hpp>
#include <boost/bind.hpp>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"

#include "base/types.h"
#include "base/timer.h"

#include "backend/render/radiositytask.h"
#include "backend/scene/threaddata.h"
#include "backend/scene/scene.h"
#include "backend/scene/view.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

using namespace pov_base;

RadiosityTask::RadiosityTask(ViewData *vd, DBL ptsz, DBL ptesz, unsigned int pts, unsigned int ptsc, unsigned int nt) :
	RenderTask(vd),
	trace(vd, GetViewDataPtr(), vd->GetSceneData()->parsedMaxTraceLevel, vd->GetSceneData()->parsedAdcBailout,
	      vd->GetQualityFeatureFlags(), cooperate, media, radiosity, !vd->GetSceneData()->radiositySettings.vainPretrace),
	cooperate(*this),
	media(GetViewDataPtr(), &trace, &photonGatherer),
	radiosity(vd->GetSceneData(), GetViewDataPtr(),
	          vd->GetSceneData()->radiositySettings, vd->GetRadiosityCache(), cooperate, false, Vector3d(vd->GetCamera().Location)),
	photonGatherer(&vd->GetSceneData()->surfacePhotonMap, vd->GetSceneData()->photonSettings),
	pretraceStep(pts),
	pretraceStepCount(ptsc),
	pretraceStartSize(ptsz),
	pretraceEndSize(ptesz),
	pretraceCoverage(vd->GetSceneData()->radiositySettings.nearestCountAPT),
	nominalThreads(nt)
{
}

RadiosityTask::~RadiosityTask()
{
}

void RadiosityTask::Run()
{
	//RandomIntSequence rands(0, 2, 2047);
	//RandomIntSequence::Generator randgen(&rands);
	RandomDoubleSequence rands(-1.0, 1.0, 2047);
	RandomDoubleSequence::Generator randgen(&rands);

	DBL width = GetViewData()->GetWidth();
	DBL height = GetViewData()->GetHeight();

	POVRect rect;
	vector<Vector2d> pixelpositions;
	vector<Colour> pixelcolors;
	unsigned int serial;

	float progressWeightTotal = (pow(4.0f, (float)pretraceStepCount) - 1.0) / 3.0; // equal to SUM[i=0,N-1](pow(4.0, i))

	ViewData::BlockInfo* pInfo;

	while(GetViewData()->GetNextRectangle(rect, serial, pInfo, nominalThreads) == true)
	{
		RadiosityBlockInfo* pBlockInfo = dynamic_cast<RadiosityBlockInfo*>(pInfo);
		if (!pBlockInfo)
		{
			if (pInfo)
			{
				delete pInfo;
				pInfo = NULL;
			}
			pBlockInfo = new RadiosityBlockInfo();
		}

		unsigned int currentStep = pretraceStep + pBlockInfo->pass;
		double pretraceSize     = max(pretraceStartSize * pow(0.5f, (float)pBlockInfo->pass),     pretraceEndSize);
		double nextPretraceSize = max(pretraceStartSize * pow(0.5f, (float)pBlockInfo->pass + 1), pretraceEndSize);

		radiosity.BeforeTile((nominalThreads? serial % nominalThreads : 0), pretraceStep + pBlockInfo->pass);
		randgen.SetSeed((pretraceStep + pBlockInfo->pass) * 17 + serial * 13); // make sure our jitter is different (but reproducible) for each pass and tile

		unsigned int px = (rect.GetWidth()  + pretraceSize - 1) / pretraceSize;
		unsigned int py = (rect.GetHeight() + pretraceSize - 1) / pretraceSize;

		unsigned int nextPx = (rect.GetWidth()  + nextPretraceSize - 1) / nextPretraceSize;
		unsigned int nextPy = (rect.GetHeight() + nextPretraceSize - 1) / nextPretraceSize;

		double startX   = ceil((DBL(rect.left)  - 0.5) / pretraceSize) * pretraceSize;
		double startY   = ceil((DBL(rect.top)   - 0.5) / pretraceSize) * pretraceSize;
		double endX     = DBL(rect.right)       + 0.5;
		double endY     = DBL(rect.bottom)      + 0.5;

		// make sure we start with the pixel buffer cleared
		pixelpositions.clear();
		pixelpositions.reserve(px * py);
		pixelcolors.clear();
		pixelcolors.reserve(px * py);

		double jitter = min(1.0, pretraceSize / 2.0);
		double offset = (pretraceSize - 1.0) / 2.0;
		unsigned int subBlockCount = pBlockInfo->incompleteSubBlocks.size();

		int subBlockDivideX;
		int subBlockDivideY;

		if (pretraceCoverage != 0)
		{
			// for the next pass, subdivide further as long as that this leaves us with at least 4x4 pixels
			subBlockDivideX = max( 1, (int)floor((DBL)nextPx / (DBL)(pBlockInfo->subBlockCountX * 4)) );
			subBlockDivideY = max( 1, (int)floor((DBL)nextPy / (DBL)(pBlockInfo->subBlockCountY * 4)) );
		}
		else
		{
			// don't subdivide if we're not using adaptive pretrace
			subBlockDivideX = 1;
			subBlockDivideY = 1;
		}

		for (int sub = 0; sub < subBlockCount; sub ++)
		{

			radiosity.ResetTopLevelStats();
			int pixelCount = 0;

			int subX = pBlockInfo->incompleteSubBlocks.front().subBlockPosX;
			int subY = pBlockInfo->incompleteSubBlocks.front().subBlockPosY;

			DBL subStartX = rect.left + (DBL)rect.GetWidth()  * (DBL) subX / (DBL)(pBlockInfo->subBlockCountX);
			DBL subEndX   = subStartX + (DBL)rect.GetWidth()               / (DBL)(pBlockInfo->subBlockCountX);
			DBL subStartY = rect.top  + (DBL)rect.GetHeight() * (DBL) subY / (DBL)(pBlockInfo->subBlockCountY);
			DBL subEndY   = subStartY + (DBL)rect.GetHeight()              / (DBL)(pBlockInfo->subBlockCountY);

			for(DBL y = startY; y < subEndY; y += pretraceSize)
			{
				if (y < subStartY)
					continue;

				for(DBL x = startX; x < subEndX; x += pretraceSize)
				{
					if (x < subStartX)
						continue;

					Colour col;

					trace(x + offset + jitter * randgen(), y + offset + jitter * randgen(), width, height, col);

					pixelpositions.push_back(Vector2d(x, y));
					pixelcolors.push_back(col);
					pixelCount ++;

					Cooperate();
				}
			}

			long  queryCount;
			float reuse;
			radiosity.GetTopLevelStats(queryCount, reuse);

			bool again;
			if (pixelCount < 9)
				// shoot at least a certain number of rays
				// (NB: Subdivision strategy tries to ensure we get at least 4x4 pixels)
				again = true;
			else if (queryCount == 0)
				// stop if we don't seem to need any samples at all in this square (e.g. because it shows only background)
				again = false;
			else if ((pretraceCoverage != 0) && (reuse / (float)queryCount >= pretraceCoverage))
				// stop if the average number of re-usable samples reaches a certain threshold
				again = false;
			else
				// otherwise do another pass
				again = true;

			pBlockInfo->incompleteSubBlocks.pop_front();
			if (again)
			{
				for (int subDivY = 0; subDivY < subBlockDivideY; subDivY ++)
					for (int subDivX = 0; subDivX < subBlockDivideX; subDivX ++)
						pBlockInfo->incompleteSubBlocks.push_back(RadiositySubBlockInfo(subX * subBlockDivideX + subDivX, subY * subBlockDivideY + subDivY));
			}
		}
		GetViewDataPtr()->Stats()[Number_Of_Pixels] += pixelpositions.size();

		radiosity.AfterTile();

		float progressWeight = pow(4.0f, (float)pBlockInfo->pass) / progressWeightTotal;

		pBlockInfo->pass ++;

		if (pBlockInfo->pass < pretraceStepCount && pBlockInfo->incompleteSubBlocks.size() > 0)
		{
			// run another pass
			pBlockInfo->subBlockCountX *= subBlockDivideX;
			pBlockInfo->subBlockCountY *= subBlockDivideY;
			pBlockInfo->completion += progressWeight;
		}
		else
		{
			// no more passes please
			progressWeight = 1.0 - pBlockInfo->completion;
			delete pBlockInfo;
			pBlockInfo = NULL;
		}

		GetViewDataPtr()->AfterTile();
		if(pixelpositions.size() > 0)
			GetViewData()->CompletedRectangle(rect, serial, pixelpositions, pixelcolors, int(ceil(pretraceSize)), false, progressWeight, pBlockInfo);
		else
			GetViewData()->CompletedRectangle(rect, serial, progressWeight, pBlockInfo);
	}
}

void RadiosityTask::Stopped()
{
	// nothing to do for now [trf]
}

void RadiosityTask::Finish()
{
	GetViewDataPtr()->timeType = SceneThreadData::kRadiosityTime;
	GetViewDataPtr()->realTime = ConsumedRealTime();
	GetViewDataPtr()->cpuTime = ConsumedCPUTime();
}

}
