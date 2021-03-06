/*******************************************************************************
 * view.cpp
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
 * $File: //depot/povray/smp/source/backend/scene/view.cpp $
 * $Revision: #149 $
 * $Change: 5428 $
 * $DateTime: 2011/03/20 07:34:04 $
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
#include <boost/math/common_factor.hpp>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"

#include "base/timer.h"
#include "base/povms.h"
#include "base/povmscpp.h"
#include "base/povmsgid.h"

#include "backend/control/renderbackend.h"
#include "backend/support/octree.h"
#include "backend/math/matrices.h"
#include "backend/math/vector.h"
#include "backend/scene/view.h"
#include "backend/render/tracetask.h"
#include "backend/render/radiositytask.h"
#include "backend/lighting/photons.h"
#include "backend/lighting/radiosity.h"

#include "backend/lighting/photonestimationtask.h"
#include "backend/lighting/photonshootingtask.h"
#include "backend/lighting/photonsortingtask.h"
#include "backend/lighting/photonstrategytask.h"

#include "povray.h" // TODO

// this must be the last file included
#include "base/povdebug.h"

#define DEFAULT_BLOCK_SIZE 32

namespace pov
{

const int QualityValues[10] =
{
	QUALITY_0, QUALITY_1, QUALITY_2, QUALITY_3, QUALITY_4,
	QUALITY_5, QUALITY_6, QUALITY_7, QUALITY_8, QUALITY_9
};

inline unsigned int MakePowerOfTwo(unsigned int i)
{
	unsigned int ii = 0;

	for(i >>= 1; i != 0; i >>= 1, ii++) { }

	return 1 << ii;
}

ViewData::ViewData(shared_ptr<SceneData> sd) :
	nextBlock(0),
	completedFirstPass(false),
	highestTraceLevel(0),
	width(160),
	height(120),
	blockWidth(10),
	blockHeight(8),
	blockSize(DEFAULT_BLOCK_SIZE),
	realTimeRaytracing(false),
	rtrData(NULL),
	renderArea(0, 0, 159, 119),
	radiosityCache(sd->radiositySettings),
	sceneData(sd)
{
	qualitySettings.Quality = 9;
	qualitySettings.Quality_Flags = QUALITY_9;
	qualitySettings.Tracing_Method = 1;
	qualitySettings.AntialiasDepth = 3;
	qualitySettings.Antialias_Threshold = 0.3;
	qualitySettings.JitterScale = 1.0;
}

ViewData::~ViewData()
{
	if (rtrData != NULL)
		delete rtrData;
}

void ViewData::getBlockXY(const unsigned int nb, unsigned int &x, unsigned int &y)
{
	unsigned long neo_nb = nb; /* must be larger, if possible */
	unsigned long sz = blockWidth*blockHeight;

	if (renderBlockStep>1)
	 { /* clock arithmetic: if renderBlockStep & sz are prime (gcd == 1), then 
			**  q := (n * renderBlockStep)% sz , for n in [0,sz[, 
			** q is also in [0,sz[
			** and it's a bijection (every n provides a different q)
			**
			** if they are not pseudo prime, you're stuck!
			*/
		neo_nb *= renderBlockStep;
		neo_nb %= sz;
	}
	switch(renderPattern)
	{
		 case 1:
			 x = neo_nb / blockHeight;
			 y = neo_nb % blockHeight;
			 break;
		 case 2:
			 x = neo_nb % blockWidth;
			 y = neo_nb / blockWidth;
			 if (x & 1)
			 {
				 x = blockWidth-1-x/2;
			 }
			 else
			 {
				 x /= 2;
			 }
			 if (y & 1)
			 {
				 y = blockHeight-1-y/2;
			 }
			 else
			 {
				 y /= 2;
			 }
			 break;
		 case 3:
			 x = (sz-1-neo_nb) % blockWidth;
			 y = (sz-1-neo_nb) / blockWidth;
			 if (x & 1)
			 {
				 x = blockWidth-1-x/2;
			 }
			 else
			 {
				 x /= 2;
			 }
			 if (y & 1)
			 {
				 y = blockHeight-1-y/2;
			 }
			 else
			 {
				 y /= 2;
			 }
			 break;
		 case 4:
			 x = neo_nb / blockHeight;
			 y = neo_nb % blockHeight;
			 if (x & 1)
			 {
				 x = blockWidth-1-x/2;
			 }
			 else
			 {
				 x /= 2;
			 }
			 if (y & 1)
			 {
				 y = blockHeight-1-y/2;
			 }
			 else
			 {
				 y /= 2;
			 }
			 break;
		 case 5:
			 x = (sz-1-neo_nb) / blockHeight;
			 y = (sz-1-neo_nb) % blockHeight;
			 if (x & 1)
			 {
				 x = blockWidth-1-x/2;
			 }
			 else
			 {
				 x /= 2;
			 }
			 if (y & 1)
			 {
				 y = blockHeight-1-y/2;
			 }
			 else
			 {
				 y /= 2;
			 }
			 break;
		 default:
			 x = neo_nb % blockWidth;
			 y = neo_nb / blockWidth;
			 break;
	 }/* all values are covered */
}

bool ViewData::GetNextRectangle(POVRect& rect, unsigned int& serial)
{
	boost::mutex::scoped_lock lock(nextBlockMutex);

	while(true)
	{
		if(nextBlock >= (blockWidth * blockHeight))
			return false;

		unsigned int tempNextBlock = nextBlock; // TODO FIXME - works around nasty gcc (found using 4.0.1) bug failing to honor casting away of volatile on pass by value on template argument lookup [trf]

		if((blockSkipList.empty() == true) || (blockSkipList.find((unsigned int)tempNextBlock) == blockSkipList.end()))
			break;

		blockSkipList.erase((unsigned int)tempNextBlock);
		nextBlock++;
	}

	unsigned int blockX; 
	unsigned int blockY;
	getBlockXY(nextBlock,blockX,blockY);

	rect.left = renderArea.left + (blockX * blockSize);
	rect.right = min(renderArea.left + ((blockX + 1) * blockSize) - 1, renderArea.right);
	rect.top = renderArea.top + (blockY * blockSize);
	rect.bottom = min(renderArea.top + ((blockY + 1) * blockSize) - 1, renderArea.bottom);

	pixelsPending += rect.GetArea();

	serial = nextBlock;
	nextBlock++;

	blockBusyList.insert(serial);

	return true;
}

bool ViewData::GetNextRectangle(POVRect& rect, unsigned int& serial, BlockInfo*& blockInfo, unsigned int stride)
{
	boost::mutex::scoped_lock lock(nextBlockMutex);

	set<unsigned int> newPostponedList;

	if (stride != 0)
	{
		unsigned int oldNextBlock = nextBlock;

		bool usePostponed = false;
		for(set<unsigned int>::iterator i = blockPostponedList.begin(); i != blockPostponedList.end(); i ++)
		{
			usePostponed = true;
			for(set<unsigned int>::iterator busy = blockBusyList.begin(); busy != blockBusyList.end(); busy ++)
			{
				if((*i >= *busy) && ((*i - *busy) % stride == 0))
				{
					usePostponed = false;
					break;
				}
			}
			if (usePostponed)
			{
				serial = *i;
				blockPostponedList.erase(i);
				break;
			}
		}

		if(!usePostponed)
		{
			while(true)
			{
				if(nextBlock >= (blockWidth * blockHeight))
				{
					nextBlock = oldNextBlock;
					return false;
				}

				unsigned int tempNextBlock = nextBlock; // TODO FIXME - works around nasty gcc (found using 4.0.1) bug failing to honor casting away of volatile on pass by value on template argument lookup [trf]

				if((blockSkipList.empty() == true) || (blockSkipList.find((unsigned int)tempNextBlock) == blockSkipList.end()))
				{
					bool avoid = false;
					for(set<unsigned int>::iterator busy = blockBusyList.begin(); busy != blockBusyList.end(); busy ++)
					{
						if((tempNextBlock >= *busy) && ((tempNextBlock - *busy) % stride == 0))
						{
							avoid = true;
							break;
						}
					}

					if(avoid)
					{
						newPostponedList.insert(tempNextBlock);
					}
					else
					{
						serial = tempNextBlock;
						nextBlock++;
						break;
					}
				}
				else
				{
					// blockSkipList.erase((unsigned int)tempNextBlock);
				}

				nextBlock++;
			}
		}
	}
	else
	{
		unsigned int oldNextBlock = nextBlock;

		while(true)
		{
			if(nextBlock >= (blockWidth * blockHeight))
			{
				nextBlock = 0;
				completedFirstPass = true;
			}

			unsigned int tempNextBlock = nextBlock; // TODO FIXME - works around nasty gcc (found using 4.0.1) bug failing to honor casting away of volatile on pass by value on template argument lookup [trf]

			if(!completedFirstPass || blockInfoList[tempNextBlock])
			{
				if((blockSkipList.empty() == true) || (blockSkipList.find((unsigned int)tempNextBlock) == blockSkipList.end()))
				{
					if((blockBusyList.empty() == true) || (blockBusyList.find((unsigned int)tempNextBlock) == blockBusyList.end()))
					{
						serial = tempNextBlock;
						nextBlock++;
						break;
					}
				}
				else
				{
					// blockSkipList.erase((unsigned int)tempNextBlock);
				}
			}

			nextBlock++;

			if (oldNextBlock == nextBlock)
				return false;
		}
	}

	unsigned int blockX ;
	unsigned int blockY ;
	getBlockXY(serial,blockX,blockY);

	rect.left = renderArea.left + (blockX * blockSize);
	rect.right = min(renderArea.left + ((blockX + 1) * blockSize) - 1, renderArea.right);
	rect.top = renderArea.top + (blockY * blockSize);
	rect.bottom = min(renderArea.top + ((blockY + 1) * blockSize) - 1, renderArea.bottom);

	pixelsPending += rect.GetArea();

	blockBusyList.insert(serial);
	for(set<unsigned int>::iterator i = newPostponedList.begin(); i != newPostponedList.end(); i ++)
		blockPostponedList.insert(*i);

	blockInfo = blockInfoList[serial];

	return true;
}

void ViewData::CompletedRectangle(const POVRect& rect, unsigned int serial, const vector<Colour>& pixels, unsigned int size, bool final, float completion, BlockInfo* blockInfo)
{
	if (realTimeRaytracing == true)
	{
		assert(pixels.size() == rect.GetArea());
		int y = rect.top - 1;
		int x = rect.right;
		int offset;
		for(vector<Colour>::const_iterator i(pixels.begin()); i != pixels.end(); i++)
		{
			if (++x > rect.right)
				offset = (++y * width + (x = rect.left)) * 5;
			assert(rtrData->rtrPixels.size() >= offset + 5);
			rtrData->rtrPixels[offset++] = i->red();
			rtrData->rtrPixels[offset++] = i->green();
			rtrData->rtrPixels[offset++] = i->blue();
			rtrData->rtrPixels[offset++] = i->filter();
			rtrData->rtrPixels[offset++] = i->transm();
		}
	}
	else
	{
		try
		{
			POVMS_Message pixelblockmsg(kPOVObjectClass_PixelData, kPOVMsgClass_ViewImage, kPOVMsgIdent_PixelBlockSet);
			vector<POVMSFloat> pixelvector;

			pixelvector.reserve(pixels.size() * 5);

			for(vector<Colour>::const_iterator i(pixels.begin()); i != pixels.end(); i++)
			{
				pixelvector.push_back(i->red());
				pixelvector.push_back(i->green());
				pixelvector.push_back(i->blue());
				pixelvector.push_back(i->filter());
				pixelvector.push_back(i->transm());
			}

			POVMS_Attribute pixelattr(pixelvector);

			pixelblockmsg.Set(kPOVAttrib_PixelBlock, pixelattr);
			if(final == true) // only final blocks get a block id (used by continue-trace to identify completely rendered blocks that do not need to be rendered again)
				pixelblockmsg.SetInt(kPOVAttrib_PixelId, serial);
			pixelblockmsg.SetInt(kPOVAttrib_PixelSize, size);
			pixelblockmsg.SetInt(kPOVAttrib_Left, rect.left);
			pixelblockmsg.SetInt(kPOVAttrib_Top, rect.top);
			pixelblockmsg.SetInt(kPOVAttrib_Right, rect.right);
			pixelblockmsg.SetInt(kPOVAttrib_Bottom, rect.bottom);

			pixelblockmsg.SetInt(kPOVAttrib_ViewId, viewId);
			pixelblockmsg.SetSourceAddress(sceneData->backendAddress);
			pixelblockmsg.SetDestinationAddress(sceneData->frontendAddress);

			POVMS_SendMessage(pixelblockmsg);
		}
		catch(pov_base::Exception&)
		{
			// TODO - work out what we should do here. until then, just re-raise the exception.
			throw;
		}
	}

	// update render progress information
	CompletedRectangle(rect, serial, completion, blockInfo);
}

void ViewData::CompletedRectangle(const POVRect& rect, unsigned int serial, const vector<Vector2d>& positions, const vector<Colour>& colors, unsigned int size, bool final, float completion, BlockInfo* blockInfo)
{
	try
	{
		if(positions.size() != colors.size())
			throw POV_EXCEPTION(kInvalidDataSizeErr, "Number of pixel colors and pixel positions does not match!");

		POVMS_Message pixelblockmsg(kPOVObjectClass_PixelData, kPOVMsgClass_ViewImage, kPOVMsgIdent_PixelSet);
		vector<POVMSInt> positionvector;
		vector<POVMSFloat> colorvector;

		positionvector.reserve(positions.size() * 2);
		colorvector.reserve(colors.size() * 5);

		for(vector<Vector2d>::const_iterator i(positions.begin()); i != positions.end(); i++)
		{
			positionvector.push_back(POVMSInt(i->x()));
			positionvector.push_back(POVMSInt(i->y()));
		}

		for(vector<Colour>::const_iterator i(colors.begin()); i != colors.end(); i++)
		{
			colorvector.push_back(i->red());
			colorvector.push_back(i->green());
			colorvector.push_back(i->blue());
			colorvector.push_back(i->filter());
			colorvector.push_back(i->transm());
		}

		POVMS_Attribute pixelposattr(positionvector);
		POVMS_Attribute pixelcolattr(colorvector);

		pixelblockmsg.Set(kPOVAttrib_PixelPositions, pixelposattr);
		pixelblockmsg.Set(kPOVAttrib_PixelColors, pixelcolattr);
		if(final == true) // only final blocks get a block id (used by continue-trace to identify completely rendered blocks that do not need to be rendered again)
			pixelblockmsg.SetInt(kPOVAttrib_PixelId, serial);
		pixelblockmsg.SetInt(kPOVAttrib_PixelSize, size);

		pixelblockmsg.SetInt(kPOVAttrib_ViewId, viewId);
		pixelblockmsg.SetSourceAddress(sceneData->backendAddress);
		pixelblockmsg.SetDestinationAddress(sceneData->frontendAddress);

		POVMS_SendMessage(pixelblockmsg);
	}
	catch(pov_base::Exception&)
	{
		// TODO - work out what we should do here. until then, just re-raise the exception.
		throw;
	}

	// update render progress information
	CompletedRectangle(rect, serial, completion, blockInfo);
}

void ViewData::CompletedRectangle(const POVRect& rect, unsigned int serial, float completion, BlockInfo* blockInfo)
{
	{
		boost::mutex::scoped_lock lock(nextBlockMutex);
		blockBusyList.erase(serial);
		blockInfoList[serial] = blockInfo;
	}

	if (realTimeRaytracing == true)
	{
		// nothing to do
	}
	else
	{
		try
		{
			pixelsCompleted += (int)(rect.GetArea() * completion);

			POVMS_Object obj(kPOVObjectClass_RenderProgress);
			// TODO obj.SetLong(kPOVAttrib_RealTime, ElapsedRealTime());
			obj.SetInt(kPOVAttrib_Pixels, renderArea.GetArea());
			obj.SetInt(kPOVAttrib_PixelsPending, pixelsPending - pixelsCompleted + rect.GetArea());
			obj.SetInt(kPOVAttrib_PixelsCompleted, pixelsCompleted);
			RenderBackend::SendViewOutput(viewId, sceneData->frontendAddress, kPOVMsgIdent_Progress, obj);
		}

		catch(pov_base::Exception&)
		{
			// TODO - work out what we should do here. until then, just re-raise the exception.
			throw;
		}
	}
}

void ViewData::SetNextRectangle(const set<unsigned int>& bsl, unsigned int fs)
{
	blockSkipList = bsl;
	blockBusyList.clear(); // safety catch; shouldn't be necessary
	blockPostponedList.clear(); // safety catch; shouldn't be necessary
	nextBlock = fs;
	completedFirstPass = false; // TODO
	pixelsCompleted = 0; // TODO
}

void ViewData::SetHighestTraceLevel(unsigned int htl)
{
	boost::mutex::scoped_lock lock(setDataMutex);

	highestTraceLevel = max(highestTraceLevel, htl);
}

unsigned int ViewData::GetQualityFeatureFlags()
{
	return qualitySettings.Quality_Flags;
}

RadiosityCache& ViewData::GetRadiosityCache()
{
	return radiosityCache;
}

View::View(shared_ptr<SceneData> sd, unsigned int width, unsigned int height, RenderBackend::ViewId vid) :
	viewData(sd),
	stopRequsted(false),
	mailbox(0),
	renderControlThread(NULL)
{
	viewData.viewId = vid;
	viewData.width = width;
	viewData.height = height;

	POV_MEM_STATS_RENDER_BEGIN();

	if(sd->boundingMethod == 2)
		mailbox = BSPTree::Mailbox(sd->numberOfFiniteObjects);
}

View::~View()
{
	stopRequsted = true; // NOTE: Order is important here, set this before stopping the queue!
	renderTasks.Stop();

	if(renderControlThread != NULL)
		renderControlThread->join();
	delete renderControlThread;

	for(vector<ViewThreadData *>::iterator i(viewThreadData.begin()); i != viewThreadData.end(); i++)
		delete (*i);
	viewThreadData.clear();

	// ok to call this more than once (it could be called from the stats method too)
	POV_MEM_STATS_RENDER_END();
}

bool View::CheckCameraHollowObject(const VECTOR point, const BBOX_TREE *node)
{
	// TODO FIXME - duplicate code - remove again!!!

	// Check current node.
	if((node->Infinite == false) && (Inside_BBox(point, node->BBox) == false))
		return false;

	if(node->Entries)
	{
		// This is a node containing leaves to be checked.
		for(int i = 0; i < node->Entries; i++)
			if(CheckCameraHollowObject((double *) point, node->Node[i]))
				return true;
	}
	else
	{
		// This is a leaf so test contained object.
		TraceThreadData threadData(viewData.GetSceneData());
		ObjectPtr object = (ObjectPtr) node->Node;
		if((object->interior != NULL) && (object->Inside((double *) point, &threadData)))
			return true;
	}

	return false;
}

bool View::CheckCameraHollowObject(const VECTOR point)
{
	shared_ptr<SceneData>& sd = viewData.GetSceneData();

	if(sd->boundingMethod == 2)
	{
		HasInteriorPointObjectCondition precond;
		TruePointObjectCondition postcond;
		TraceThreadData threadData(sd); // TODO: avoid the need to construct threadData
		BSPInsideCondFunctor ifn(Vector3d(point), sd->objects, &threadData, precond, postcond);

		mailbox.clear();
		if ((*sd->tree)(Vector3d(point), ifn, mailbox, true))
			return true;

		// test infinite objects
		for(vector<ObjectPtr>::iterator object = sd->objects.begin() + sd->numberOfFiniteObjects; object != sd->objects.end(); object++)
			if(((*object)->interior != NULL) && Inside_BBox(point, (*object)->BBox) && (*object)->Inside((double *) point, &threadData))
				return true;
	}
	else if((sd->boundingMethod == 0) || (sd->boundingSlabs == NULL))
	{
		TraceThreadData threadData(sd); // TODO: avoid the need to construct threadData
		for(vector<ObjectPtr>::const_iterator object = viewData.GetSceneData()->objects.begin(); object != viewData.GetSceneData()->objects.end(); object++)
			if((*object)->interior != NULL)
				if((*object)->Inside((double *) point, &threadData))
					return true;
	}
	else
	{
		return CheckCameraHollowObject(point, sd->boundingSlabs);
	}

	return false;
}

void View::StartRender(POVMS_Object& renderOptions)
{
	unsigned int tracingmethod = 0;
	DBL jitterscale = 1.0;
	bool jitter = false;
	DBL aathreshold = 0.3;
	unsigned int aadepth = 3;
	DBL aaGammaValue = 1.0;
	GammaCurvePtr aaGammaCurve;
	unsigned int previewstartsize = 0;
	unsigned int previewendsize = 0;
	unsigned int nextblock = 0;
	bool highReproducibility = false;
	shared_ptr<set<unsigned int> > blockskiplist(new set<unsigned int>());

	if(renderControlThread == NULL)
#ifndef USE_OFFICIAL_BOOST
		renderControlThread = new thread(boost::bind(&View::RenderControlThread, this), 1024 * 64);
#else
		renderControlThread = new boost::thread(boost::bind(&View::RenderControlThread, this));
#endif

	viewData.qualitySettings.Quality = clip(renderOptions.TryGetInt(kPOVAttrib_Quality, 9), 0, 9);
	viewData.qualitySettings.Quality_Flags = QualityValues[viewData.qualitySettings.Quality];

	if(renderOptions.TryGetBool(kPOVAttrib_Antialias, false) == true)
		tracingmethod = clip(renderOptions.TryGetInt(kPOVAttrib_SamplingMethod, 1), 0, 2);

	aadepth = clip((unsigned int)renderOptions.TryGetInt(kPOVAttrib_AntialiasDepth, 3), 1u, 9u);
	aathreshold = clip(renderOptions.TryGetFloat(kPOVAttrib_AntialiasThreshold, 0.3f), 0.0f, 1.0f);
	if(renderOptions.TryGetBool(kPOVAttrib_Jitter, true) == true)
		jitterscale = clip(renderOptions.TryGetFloat(kPOVAttrib_JitterAmount, 1.0f), 0.0f, 1.0f);
	else
		jitterscale = 0.0f;
	aaGammaValue = renderOptions.TryGetFloat(kPOVAttrib_AntialiasGamma, 2.5f);
	if (aaGammaValue > 0)
		aaGammaCurve = PowerLawGammaCurve::GetByDecodingGamma(aaGammaValue);
	if (viewData.GetSceneData()->workingGamma)
		aaGammaCurve = TranscodingGammaCurve::Get(viewData.GetSceneData()->workingGamma, aaGammaCurve);

	previewstartsize = MakePowerOfTwo(clip((unsigned int)renderOptions.TryGetInt(kPOVAttrib_PreviewStartSize, 1), 1u, 64u));
	previewendsize = MakePowerOfTwo(clip((unsigned int)renderOptions.TryGetInt(kPOVAttrib_PreviewEndSize, 1), 1u, previewstartsize));
	if((previewendsize == 2) && (tracingmethod == 0)) // optimisation to render all pixels only once
		previewendsize = 1;

	highReproducibility = renderOptions.TryGetBool(kPOVAttrib_HighReproducibility, false);

	// TODO FIXME - [CLi] handle loading, storing (and later optionally deleting) of radiosity cache file for trace abort & continue feature
	// TODO FIXME - [CLi] if high reproducibility is a demand, timing of writing samples to disk is an issue regarding abort & continue
	bool loadRadiosityCache = renderOptions.TryGetBool(kPOVAttrib_RadiosityFromFile, false);
	bool saveRadiosityCache = renderOptions.TryGetBool(kPOVAttrib_RadiosityToFile, false);
	if (loadRadiosityCache || saveRadiosityCache)
	{
		// TODO FIXME - [CLi] I guess the radiosity file name needs more attention than this; probably a frontend job
		Path radiosityFile = Path(renderOptions.TryGetUCS2String(kPOVAttrib_RadiosityFileName, "object.rca"));
		if(loadRadiosityCache)
			loadRadiosityCache = viewData.radiosityCache.Load(radiosityFile);
		if(saveRadiosityCache)
			viewData.radiosityCache.InitAutosave(radiosityFile, loadRadiosityCache); // if we loaded the file, add to existing data
	}

	viewData.GetSceneData()->radiositySettings.vainPretrace = renderOptions.TryGetBool(kPOVAttrib_RadiosityVainPretrace, true);


	 // TODO FIXME - all below is not implemented properly and not threadsafe [trf]


	viewData.GetSceneData()->radiositySettings.radiosityEnabled = viewData.GetSceneData()->radiositySettings.radiosityEnabled && (viewData.qualitySettings.Quality_Flags & Q_ADVANCED_LIGHT);
	viewData.GetSceneData()->photonSettings.photonsEnabled = viewData.GetSceneData()->photonSettings.photonsEnabled && (viewData.qualitySettings.Quality_Flags & Q_ADVANCED_LIGHT);
	viewData.GetSceneData()->useSubsurface = viewData.GetSceneData()->useSubsurface && (viewData.qualitySettings.Quality_Flags & Q_SUBSURFACE);

	if(viewData.GetSceneData()->photonSettings.Max_Trace_Level < 0)
		viewData.GetSceneData()->photonSettings.Max_Trace_Level = viewData.GetSceneData()->parsedMaxTraceLevel;

	if(viewData.GetSceneData()->photonSettings.adcBailout < 0)
		viewData.GetSceneData()->photonSettings.adcBailout = viewData.GetSceneData()->parsedAdcBailout;


	// TODO FIXME - end of todo fixme stuff above


	DBL raleft = renderOptions.TryGetFloat(kPOVAttrib_Left, 1.0f);
	DBL ratop = renderOptions.TryGetFloat(kPOVAttrib_Top, 1.0f);
	DBL raright = renderOptions.TryGetFloat(kPOVAttrib_Right, POVMSFloat(viewData.width));
	DBL rabottom = renderOptions.TryGetFloat(kPOVAttrib_Bottom, POVMSFloat(viewData.height));

	if((raleft >= 0.0) && (raleft < 1.0))
		viewData.renderArea.left = int(DBL(viewData.GetWidth()) * raleft);
	else
		viewData.renderArea.left = int(raleft) - 1;

	if((ratop >= 0.0) && (ratop < 1.0))
		viewData.renderArea.top = int(DBL(viewData.GetHeight()) * ratop);
	else
		viewData.renderArea.top = int(ratop - 1);

	if((raright >= 0.0) && (raright <= 1.0))
		viewData.renderArea.right = int(DBL(viewData.GetWidth()) * raright) - 1;
	else
		viewData.renderArea.right = int(raright - 1);

	if((rabottom >= 0.0) && (rabottom <= 1.0))
		viewData.renderArea.bottom = int(DBL(viewData.GetHeight()) * rabottom) - 1;
	else
		viewData.renderArea.bottom = int(rabottom - 1);

	// TODO FIXME - need to check this in front end, then change this code to set the values to default
	if ((viewData.renderArea.left >= viewData.GetWidth()) || (viewData.renderArea.top >= viewData.GetHeight()))
		throw POV_EXCEPTION(kParamErr, "Invalid start column or row");
	if ((viewData.renderArea.right >= viewData.GetWidth()) || (viewData.renderArea.bottom >= viewData.GetHeight()))
		throw POV_EXCEPTION(kParamErr, "Invalid end column or row");

	viewData.blockSize = renderOptions.TryGetInt(kPOVAttrib_RenderBlockSize, 32);
	if(viewData.blockSize < 4)
		viewData.blockSize = 4;
	if(viewData.blockSize > max(viewData.renderArea.GetWidth(), viewData.renderArea.GetHeight()))
		viewData.blockSize = max(viewData.renderArea.GetWidth(), viewData.renderArea.GetHeight());
	viewData.blockWidth = ((viewData.renderArea.GetWidth() + viewData.blockSize - 1) / viewData.blockSize);
	viewData.blockHeight = ((viewData.renderArea.GetHeight() + viewData.blockSize - 1) / viewData.blockSize);

	viewData.renderPattern = (unsigned int)renderOptions.TryGetInt(kPOVAttrib_RenderPattern,0);
	viewData.renderBlockStep = (unsigned int)renderOptions.TryGetInt(kPOVAttrib_RenderBlockStep,0);
	if (viewData.renderBlockStep > 0)
	{
		// check that the value is prime with blockWidth * blockHeight, 
		// if not reduces it until it does (1 is ok... but boring)
		// when renderBlockStep is positive, the factor (renderBlockStep)
		// must be prime with the modulus (blockWidth * blockHeight),
		// (otherwise, the full range [0.. W*H-1] won't be covered)
		//
		// reduce renderBlockStep to [0;blockWidth * blockHeight-1] range
		// (it's not worth keeping a bigger value)
		// 
		viewData.renderBlockStep %= (viewData.blockWidth*viewData.blockHeight);
		if (!viewData.renderBlockStep)
			viewData.renderBlockStep = (viewData.blockWidth*viewData.blockHeight);
		while(boost::math::gcd((long)viewData.renderBlockStep,(long)(viewData.blockWidth*viewData.blockHeight))>1)
			viewData.renderBlockStep--;
	}

	viewData.blockInfoList.resize(viewData.blockWidth * viewData.blockHeight);

	viewData.pixelsPending = 0;
	viewData.pixelsCompleted = 0;

	// continue trace
	nextblock = renderOptions.TryGetInt(kPOVAttrib_PixelId, 0);

	if(renderOptions.Exist(kPOVAttrib_PixelSkipList) == true)
	{
		vector<POVMSInt> psl(renderOptions.GetIntVector(kPOVAttrib_PixelSkipList));

		for(vector<POVMSInt>::iterator i(psl.begin()); i != psl.end(); i++)
			blockskiplist->insert(*i);
	}

	viewData.SetNextRectangle(*blockskiplist, nextblock);

	// render thread count
	int maxRenderThreads = renderOptions.TryGetInt(kPOVAttrib_MaxRenderThreads, 1);

	viewData.realTimeRaytracing = renderOptions.TryGetBool(kPOVAttrib_RealTimeRaytracing, false); // TODO - experimental code
	if (viewData.realTimeRaytracing)
		viewData.rtrData = new RTRData(viewData, maxRenderThreads);

	// camera changes without parsing
	if(renderOptions.Exist(kPOVAttrib_SceneCamera) == false)
		viewData.camera = viewData.GetSceneData()->parsedCamera;
	else // INCOMPLETE EXPERIMENTAL [trf]
	{
		POVMS_Object camera;

		renderOptions.Get(kPOVAttrib_SceneCamera, camera);

		// TODO FIXME - clear by setting scene's camera, but not sure if this is the way to go in the long run [trf]
		viewData.camera = viewData.GetSceneData()->parsedCamera;

		bool had_location = false;
		bool had_direction = false;
		bool had_up = false;
		bool had_right = false;
		bool had_sky = false;
		bool had_look_at = false;

		if(camera.Exist('cloc') == true)
		{
			std::vector<POVMSFloat> pv = camera.GetFloatVector('cloc');
			Assign_Vector(viewData.camera.Location, *Vector3d(pv[X], pv[Y], pv[Z]));
			had_location = true;
		}

		if(camera.Exist('cdir') == true)
		{
			std::vector<POVMSFloat> pv = camera.GetFloatVector('cdir');
			Assign_Vector(viewData.camera.Direction, *Vector3d(pv[X], pv[Y], pv[Z]));
			had_direction = true;
		}

		if(camera.Exist('cup ') == true)
		{
			std::vector<POVMSFloat> pv = camera.GetFloatVector('cup ');
			Assign_Vector(viewData.camera.Up, *Vector3d(pv[X], pv[Y], pv[Z]));
			had_up = true;
		}

		if(camera.Exist('crig') == true)
		{
			std::vector<POVMSFloat> pv = camera.GetFloatVector('crig');
			Assign_Vector(viewData.camera.Right, *Vector3d(pv[X], pv[Y], pv[Z]));
			had_right = true;
		}

		if(camera.Exist('csky') == true)
		{
			std::vector<POVMSFloat> pv = camera.GetFloatVector('csky');
			Assign_Vector(viewData.camera.Sky, *Vector3d(pv[X], pv[Y], pv[Z]));
			had_sky = true;
		}

		if(camera.Exist('clat') == true)
		{
			std::vector<POVMSFloat> pv = camera.GetFloatVector('clat');
			Assign_Vector(viewData.camera.Look_At, *Vector3d(pv[X], pv[Y], pv[Z]));
			had_look_at = true;
		}

		// apply "look_at"
		if(had_look_at == true)
		{
			DBL Direction_Length = 1.0, Up_Length, Right_Length, Handedness;
			VECTOR tempv;

			VLength (Direction_Length, viewData.camera.Direction);
			VLength (Up_Length,        viewData.camera.Up);
			VLength (Right_Length,     viewData.camera.Right);
			VCross  (tempv,            viewData.camera.Up, viewData.camera.Direction);
			VDot    (Handedness,       tempv,   viewData.camera.Right);

			Assign_Vector(viewData.camera.Direction, viewData.camera.Look_At);

			VSub(viewData.camera.Direction, viewData.camera.Direction, viewData.camera.Location);

			// Check for zero length direction vector.
			if(VSumSqr(viewData.camera.Direction) < EPSILON)
				; // Error("Camera location and look_at point must be different.");

			VNormalize(viewData.camera.Direction, viewData.camera.Direction);

			// Save Right vector
			Assign_Vector (tempv, viewData.camera.Right);

			VCross(viewData.camera.Right, viewData.camera.Sky, viewData.camera.Direction);

			// Avoid DOMAIN error (from Terry Kanakis)
			if((fabs(viewData.camera.Right[X]) < EPSILON) &&
			   (fabs(viewData.camera.Right[Y]) < EPSILON) &&
			   (fabs(viewData.camera.Right[Z]) < EPSILON))
			{
				// Warning(0, "Camera location to look_at direction and sky direction should be different.\n"
				//            "Using default/supplied right vector instead.");

				// Restore Right vector
				Assign_Vector(viewData.camera.Right, tempv);
			}

			VNormalize (viewData.camera.Right,     viewData.camera.Right);
			VCross     (viewData.camera.Up,        viewData.camera.Direction, viewData.camera.Right);
			VScale     (viewData.camera.Direction, viewData.camera.Direction, Direction_Length);

			if (Handedness > 0.0)
			{
				VScaleEq (viewData.camera.Right, Right_Length);
			}
			else
			{
				VScaleEq (viewData.camera.Right, -Right_Length);
			}

			VScaleEq(viewData.camera.Up, Up_Length);
		}
	}

	if(CheckCameraHollowObject(viewData.camera.Location))
	{
		// TODO FIXME
		// Warning(0, "Camera is inside a non-hollow object. Fog and participating media may not work as expected.");
	}

	// check for preview end size
	if((previewendsize > 1) && (tracingmethod == 0))
	{
		// TODO FIXME
		// Warning(0, "In POV-Ray 3.7 and later it is recommended to set the mosaic preview end size to one for\n"
		//              "maximum performance when rendering without anti-aliasing.");
	}

	// do photons
	/*
	if(viewData.GetSceneData()->photonSettings.photonsEnabled)
	{
		viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new PhotonTask(&viewData))));

		// wait for photons to finish
		renderTasks.AppendSync();
	}
	*/
	if(viewData.GetSceneData()->photonSettings.photonsEnabled)
	{
		if (viewData.GetSceneData()->photonSettings.fileName && viewData.GetSceneData()->photonSettings.loadFile)
		{
			vector<PhotonMap*> surfaceMaps;
			vector<PhotonMap*> mediaMaps;

			// when we pass a null parameter for the "strategy" (last parameter),
			// then this will LOAD the photon map
			viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new PhotonSortingTask(&viewData, surfaceMaps, mediaMaps, NULL))));
			// wait for photons to finish
			renderTasks.AppendSync();
		}
		else
		{
			PhotonShootingStrategy* strategy = new PhotonShootingStrategy();

			viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new PhotonEstimationTask(&viewData))));
			// wait for photons to finish
			renderTasks.AppendSync();

			viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new PhotonStrategyTask(&viewData, strategy))));
			// wait for photons to finish
			renderTasks.AppendSync();

			vector<PhotonMap*> surfaceMaps;
			vector<PhotonMap*> mediaMaps;

			for(int i = 0; i < maxRenderThreads; i++)
			{
				PhotonShootingTask* task = new PhotonShootingTask(&viewData, strategy);
				surfaceMaps.push_back(task->getSurfacePhotonMap());
				mediaMaps.push_back(task->getMediaPhotonMap());
				viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(task)));
			}
			// wait for photons to finish
			renderTasks.AppendSync();

			// this merges the maps, sorts, computes gather options, and then cleans up memory
			viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new PhotonSortingTask(&viewData, surfaceMaps, mediaMaps, strategy))));
			// wait for photons to finish
			renderTasks.AppendSync();

		}
	}

	// do radiosity pretrace
	if(viewData.GetSceneData()->radiositySettings.radiosityEnabled)
	{
		// TODO load radiosity data (if applicable)?

		DBL maxWidthHeight  = DBL(max(viewData.GetWidth(), viewData.GetHeight()));
		DBL startSize       = maxWidthHeight * viewData.GetSceneData()->radiositySettings.pretraceStart;
		DBL endSize         = maxWidthHeight * viewData.GetSceneData()->radiositySettings.pretraceEnd;
		if (endSize < 1.0)
		{
			endSize = 1.0;
			// Warning(0, "Radiosity pretrace end too low for selected resolution. Pretrace will be\n"
			//            "stopped early, corresponding to a value of %lf.\n"
			//            "To avoid this warning, increase pretrace_end.", endSize / maxWidthHeight);
		}
		int steps = (int)floor(log(startSize/endSize)/log(2.0) + (1.0 - EPSILON)) + 1;
		if (steps > RadiosityFunction::PRETRACE_MAX - RadiosityFunction::PRETRACE_FIRST - 1)
		{
			steps = RadiosityFunction::PRETRACE_MAX - RadiosityFunction::PRETRACE_FIRST - 1;
			startSize = endSize * pow(2.0, (double)steps);
			// Warning(0, "Too many radiosity pretrace steps. Pretrace will be started late,\n"
			//            "corresponding to a value of %lf", startSize);
			//            "To avoid this warning, decrease pretrace_start or increase pretrace_end.", endSize / maxWidthHeight);
		}

		if (highReproducibility)
		{
			int nominalThreads = 1;
			int actualThreads;
			DBL stepSize = startSize;
			DBL actualSize;
			for(int step = RadiosityFunction::PRETRACE_FIRST; step < RadiosityFunction::PRETRACE_FIRST + steps; step ++)
			{
				actualThreads = min(nominalThreads, maxRenderThreads);
				actualSize = max(stepSize, endSize);

				// do render one pretrace step with current pretrace size
				for(int i = 0; i < actualThreads; i++)
					viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new RadiosityTask(&viewData, actualSize, actualSize, step, 1, nominalThreads))));

				// wait for previous pretrace step to finish
				renderTasks.AppendSync();

				// reset block size counter and block skip list for next pretrace step
				renderTasks.AppendFunction(boost::bind(&View::SetNextRectangle, this, _1, blockskiplist, nextblock));

				// wait for block size counter and block skip list reset to finish
				renderTasks.AppendSync();

				stepSize *= 0.5;
				nominalThreads *= 2;
			}
		}
		else if (steps > 0)
		{
			// do render all pretrace steps
			for(int i = 0; i < maxRenderThreads; i++)
				viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new RadiosityTask(&viewData, startSize, endSize, RadiosityFunction::PRETRACE_FIRST, steps, 0))));

			// wait for pretrace to finish
			renderTasks.AppendSync();

			// reset block size counter and block skip list for final render
			renderTasks.AppendFunction(boost::bind(&View::SetNextRectangle, this, _1, blockskiplist, nextblock));

			// wait for block size counter and block skip list reset to finish
			renderTasks.AppendSync();
		}

		// TODO store radiosity data (if applicable)?
	}

	// do render with mosaic preview
	if(previewstartsize > 1)
	{
		// do render with mosaic preview start size
		for(int i = 0; i < maxRenderThreads; i++)
			viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new TraceTask(&viewData, 0, jitterscale, aathreshold, aadepth, aaGammaCurve, previewstartsize, false, false, highReproducibility))));

		for(unsigned int step = (previewstartsize >> 1); step >= previewendsize; step >>= 1)
		{
			// wait for previous mosaic preview step to finish
			renderTasks.AppendSync();

			// reset block size counter and block skip list
			renderTasks.AppendFunction(boost::bind(&View::SetNextRectangle, this, _1, blockskiplist, nextblock));

			// wait for block size counter and block skip list reset to finish
			renderTasks.AppendSync();

			// do render with current mosaic preview size
			for(int i = 0; i < maxRenderThreads; i++)
				viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new TraceTask(&viewData, 0, jitterscale, aathreshold, aadepth, aaGammaCurve, step, true, ((step == 1) && (tracingmethod == 0)), highReproducibility))));
		}

		// do render everything again if the final mosaic preview block size was not one or anti-aliasing is required
		if((previewendsize > 1) || (tracingmethod > 0))
		{
			// wait for previous mosaic preview step to finish
			renderTasks.AppendSync();

			// reset block size counter and block skip list
			renderTasks.AppendFunction(boost::bind(&View::SetNextRectangle, this, _1, blockskiplist, nextblock));

			// wait for block size counter and block skip list reset to finish
			renderTasks.AppendSync();

			for(int i = 0; i < maxRenderThreads; i++)
				viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new TraceTask(&viewData, tracingmethod, jitterscale, aathreshold, aadepth, aaGammaCurve, 0, false, true, highReproducibility))));
		}
	}
	// do render without mosaic preview
	else
	{
		for(int i = 0; i < maxRenderThreads; i++)
			viewThreadData.push_back(dynamic_cast<ViewThreadData *>(renderTasks.AppendTask(new TraceTask(&viewData, tracingmethod, jitterscale, aathreshold, aadepth, aaGammaCurve, 0, false, true, highReproducibility))));
	}

	// wait for render to finish
	renderTasks.AppendSync();

	// send shutdown messages
	renderTasks.AppendFunction(boost::bind(&View::DispatchShutdownMessages, this, _1));

	// wait for shutdown messages to be sent
	renderTasks.AppendSync();

	// send statistics
	renderTasks.AppendFunction(boost::bind(&View::SendStatistics, this, _1));

	// send done message
	POVMS_Message doneMessage(kPOVObjectClass_ResultData, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Done);
	doneMessage.SetInt(kPOVAttrib_ViewId, viewData.viewId);
	doneMessage.SetSourceAddress(viewData.sceneData->backendAddress);
	doneMessage.SetDestinationAddress(viewData.sceneData->frontendAddress);
	renderTasks.AppendMessage(doneMessage);
}

void View::StopRender()
{
	renderTasks.Stop();

	RenderBackend::SendViewFailedResult(viewData.viewId, kUserAbortErr, viewData.sceneData->frontendAddress);
}

void View::GetStatistics(POVMS_Object& renderStats)
{
	RenderStatistics    stats;

	for(vector<ViewThreadData *>::iterator i(viewThreadData.begin()); i != viewThreadData.end(); i++)
		stats += (*i)->Stats();

	// object intersection stats
	POVMS_List isectStats;

	for(size_t index = 0; intersection_stats[index].infotext != NULL; index++)
	{
		POVMS_Object isectStat(kPOVObjectClass_IsectStat);

		isectStat.SetString(kPOVAttrib_ObjectName, intersection_stats[index].infotext);
		isectStat.SetInt(kPOVAttrib_ObjectID, intersection_stats[index].povms_id);
		isectStat.SetLong(kPOVAttrib_ISectsTests, stats[intersection_stats[index].stat_test_id]);
		isectStat.SetLong(kPOVAttrib_ISectsSucceeded, stats[intersection_stats[index].stat_suc_id]);

		isectStats.Append(isectStat);
	}

	renderStats.Set(kPOVAttrib_ObjectIStats, isectStats);

	// general stats
	renderStats.SetInt(kPOVAttrib_Height, viewData.GetHeight());
	renderStats.SetInt(kPOVAttrib_Width, viewData.GetWidth());

	// basic tracing stats
	renderStats.SetLong(kPOVAttrib_Pixels, stats[Number_Of_Pixels]);
	renderStats.SetLong(kPOVAttrib_PixelSamples, stats[Number_Of_Samples]);
	renderStats.SetLong(kPOVAttrib_Rays, stats[Number_Of_Rays]);
	renderStats.SetLong(kPOVAttrib_RaysSaved, stats[ADC_Saves]);

	// detailed tracing stats
	renderStats.SetLong(kPOVAttrib_ShadowTest, stats[Shadow_Ray_Tests]);
	renderStats.SetLong(kPOVAttrib_ShadowTestSuc, stats[Shadow_Rays_Succeeded]);
	renderStats.SetLong(kPOVAttrib_ShadowCacheHits, stats[Shadow_Cache_Hits]);
	renderStats.SetLong(kPOVAttrib_MediaSamples, stats[Media_Samples]);
	renderStats.SetLong(kPOVAttrib_MediaIntervals, stats[Media_Intervals]);
	renderStats.SetLong(kPOVAttrib_ReflectedRays, stats[Reflected_Rays_Traced]);
	renderStats.SetLong(kPOVAttrib_InnerReflectedRays, stats[Internal_Reflected_Rays_Traced]);
	renderStats.SetLong(kPOVAttrib_RefractedRays, stats[Refracted_Rays_Traced]);
	renderStats.SetLong(kPOVAttrib_TransmittedRays, stats[Transmitted_Rays_Traced]);

	// other tracing-related stats
	renderStats.SetLong(kPOVAttrib_IsoFindRoot, stats[Ray_IsoSurface_Find_Root]);
	renderStats.SetLong(kPOVAttrib_FunctionVMCalls, stats[Ray_Function_VM_Calls]);
	renderStats.SetLong(kPOVAttrib_FunctionVMInstrEst, stats[Ray_Function_VM_Instruction_Est]);
	renderStats.SetLong(kPOVAttrib_PolynomTest, stats[Polynomials_Tested]);
	renderStats.SetLong(kPOVAttrib_RootsEliminated, stats[Roots_Eliminated]);
	renderStats.SetLong(kPOVAttrib_CallsToNoise, stats[Calls_To_Noise]);
	renderStats.SetLong(kPOVAttrib_CallsToDNoise, stats[Calls_To_DNoise]);

	renderStats.SetLong(kPOVAttrib_CrackleCacheTest, stats[CrackleCache_Tests]);
	renderStats.SetLong(kPOVAttrib_CrackleCacheTestSuc, stats[CrackleCache_Tests_Succeeded]);

	POV_LONG current;
	POV_ULONG allocs(0), frees(0), peak(0), smallest(0), largest(0);
	POV_MEM_STATS_RENDER_END();
	if (POV_GLOBAL_MEM_STATS(allocs, frees, current, peak, smallest, largest))
	{
		if (allocs)
			renderStats.SetLong(kPOVAttrib_CallsToAlloc, allocs);
		if (frees)
			renderStats.SetLong(kPOVAttrib_CallsToFree, frees);
		if (peak)
			renderStats.SetLong(kPOVAttrib_PeakMemoryUsage, peak);
		if (smallest)
			renderStats.SetLong(kPOVAttrib_MinAlloc, smallest);
		if (largest)
			renderStats.SetLong(kPOVAttrib_MaxAlloc, largest);
	}

	renderStats.SetInt(kPOVAttrib_TraceLevel, viewData.highestTraceLevel);
	renderStats.SetInt(kPOVAttrib_MaxTraceLevel, viewData.sceneData->parsedMaxTraceLevel);

	renderStats.SetLong(kPOVAttrib_RadGatherCount, stats[Radiosity_GatherCount]);
	renderStats.SetLong(kPOVAttrib_RadUnsavedCount, stats[Radiosity_UnsavedCount]);
	renderStats.SetLong(kPOVAttrib_RadReuseCount, stats[Radiosity_ReuseCount]);
	renderStats.SetLong(kPOVAttrib_RadRayCount, stats[Radiosity_RayCount]);
	renderStats.SetLong(kPOVAttrib_RadTopLevelGatherCount, stats[Radiosity_TopLevel_GatherCount]);
	renderStats.SetLong(kPOVAttrib_RadTopLevelReuseCount, stats[Radiosity_TopLevel_ReuseCount]);
	renderStats.SetLong(kPOVAttrib_RadTopLevelRayCount, stats[Radiosity_TopLevel_RayCount]);
	renderStats.SetLong(kPOVAttrib_RadFinalGatherCount, stats[Radiosity_Final_GatherCount]);
	renderStats.SetLong(kPOVAttrib_RadFinalReuseCount, stats[Radiosity_Final_ReuseCount]);
	renderStats.SetLong(kPOVAttrib_RadFinalRayCount, stats[Radiosity_Final_RayCount]);
	renderStats.SetLong(kPOVAttrib_RadOctreeNodes, stats[Radiosity_OctreeNodes]);
	renderStats.SetLong(kPOVAttrib_RadOctreeLookups, stats[Radiosity_OctreeLookups]);
	renderStats.SetLong(kPOVAttrib_RadOctreeAccepts0, stats[Radiosity_OctreeAccepts0]);
	renderStats.SetLong(kPOVAttrib_RadOctreeAccepts1, stats[Radiosity_OctreeAccepts1]);
	renderStats.SetLong(kPOVAttrib_RadOctreeAccepts2, stats[Radiosity_OctreeAccepts2]);
	renderStats.SetLong(kPOVAttrib_RadOctreeAccepts3, stats[Radiosity_OctreeAccepts3]);
	renderStats.SetLong(kPOVAttrib_RadOctreeAccepts4, stats[Radiosity_OctreeAccepts4]);
	renderStats.SetLong(kPOVAttrib_RadOctreeAccepts5, stats[Radiosity_OctreeAccepts5]);

	for (int recursion = 0; recursion < 5; recursion ++)
	{
		long id;
		long queryCount = stats[IntStatsIndex(Radiosity_QueryCount_R0 + recursion)];
		id = kPOVAttrib_RadQueryCountR0 + recursion;
		renderStats.SetLong(id, queryCount);
		for(int pass = 1; pass <= 5; pass ++)
		{
			id = kPOVAttrib_RadSamplesP1R0 + (pass-1)*256 + recursion;
			renderStats.SetLong(id, stats[IntStatsIndex(Radiosity_SamplesTaken_PTS1_R0 + (pass-1)*5 + recursion)]);
		}
		id = kPOVAttrib_RadSamplesFR0 + recursion;
		renderStats.SetLong(id, stats[IntStatsIndex(Radiosity_SamplesTaken_Final_R0 + recursion)]);
		id = kPOVAttrib_RadWeightR0 + recursion;
		renderStats.SetFloat(id, stats[FPStatsIndex(Radiosity_Weight_R0 + recursion)] / (double)queryCount);
	}

	// photon stats // TODO FIXME - move to photon pass? [trf]
	renderStats.SetLong(kPOVAttrib_PhotonsShot, stats[Number_Of_Photons_Shot]);
	renderStats.SetLong(kPOVAttrib_PhotonsStored, stats[Number_Of_Photons_Stored]);
	renderStats.SetLong(kPOVAttrib_GlobalPhotonsStored, stats[Number_Of_Global_Photons_Stored]);
	renderStats.SetLong(kPOVAttrib_MediaPhotonsStored, stats[Number_Of_Media_Photons_Stored]);
	renderStats.SetLong(kPOVAttrib_PhotonsPriQInsert, stats[Priority_Queue_Add]);
	renderStats.SetLong(kPOVAttrib_PhotonsPriQRemove, stats[Priority_Queue_Remove]);
	renderStats.SetLong(kPOVAttrib_GatherPerformedCnt, stats[Gather_Performed_Count]);
	renderStats.SetLong(kPOVAttrib_GatherExpandedCnt, stats[Gather_Expanded_Count]);

	struct TimeData
	{
		POV_LONG cpuTime;
		POV_LONG realTime;
		size_t samples;

		TimeData() : cpuTime(0), realTime(0), samples(0) { }
	};

	TimeData timeData[SceneThreadData::kMaxTimeType];

	for(vector<ViewThreadData *>::iterator i(viewThreadData.begin()); i != viewThreadData.end(); i++)
	{
		timeData[(*i)->timeType].realTime = max(timeData[(*i)->timeType].realTime, (*i)->realTime);
		timeData[(*i)->timeType].cpuTime += (*i)->cpuTime;
		timeData[(*i)->timeType].samples++;
	}

	for(size_t i = SceneThreadData::kUnknownTime; i < SceneThreadData::kMaxTimeType; i++)
	{
		if(timeData[i].samples > 0)
		{
			POVMS_Object elapsedTime(kPOVObjectClass_ElapsedTime);

			elapsedTime.SetLong(kPOVAttrib_RealTime, timeData[i].realTime);
			elapsedTime.SetLong(kPOVAttrib_CPUTime, timeData[i].cpuTime);
			elapsedTime.SetInt(kPOVAttrib_TimeSamples, (int) timeData[i].samples);

			switch(i)
			{
				case SceneThreadData::kPhotonTime:
					renderStats.Set(kPOVAttrib_PhotonTime, elapsedTime);
					break;
				case SceneThreadData::kRadiosityTime:
					renderStats.Set(kPOVAttrib_RadiosityTime, elapsedTime);
					break;
				case SceneThreadData::kRenderTime:
					renderStats.Set(kPOVAttrib_TraceTime, elapsedTime);
					break;
			}
		}
	}
}

void View::PauseRender()
{
	renderTasks.Pause();
}

void View::ResumeRender()
{
	renderTasks.Resume();
}

bool View::IsRendering()
{
	return renderTasks.IsRunning();
}

bool View::IsPaused()
{
	return renderTasks.IsPaused();
}

bool View::Failed()
{
	return renderTasks.Failed();
}

void View::DispatchShutdownMessages(TaskQueue&)
{
	MessageFactory messageFactory(10, 370, "Shutdown", viewData.sceneData->backendAddress, viewData.sceneData->frontendAddress, viewData.sceneData->sceneId, viewData.viewId);

	for (vector<ObjectPtr>::iterator it = viewData.sceneData->objects.begin(); it != viewData.sceneData->objects.end(); it++)
		(*it)->DispatchShutdownMessages(messageFactory);
}

void View::SendStatistics(TaskQueue&)
{
	POVMS_Message renderStats(kPOVObjectClass_RenderStatistics, kPOVMsgClass_ViewOutput, kPOVMsgIdent_RenderStatistics);

	GetStatistics(renderStats);

	renderStats.SetInt(kPOVAttrib_ViewId, viewData.viewId);
	renderStats.SetSourceAddress(viewData.sceneData->backendAddress);
	renderStats.SetDestinationAddress(viewData.sceneData->frontendAddress);

	POVMS_SendMessage(renderStats);

	for(vector<ViewThreadData *>::iterator i(viewThreadData.begin()); i != viewThreadData.end(); i++)
		delete (*i);
	viewThreadData.clear();
}

void View::SetNextRectangle(TaskQueue&, shared_ptr<set<unsigned int> > bsl, unsigned int fs)
{
	viewData.SetNextRectangle(*bsl, fs);
}

void View::RenderControlThread()
{
	bool sentFailedResult = false;

	while(stopRequsted == false)
	{
		while((renderTasks.Process() == true) && (stopRequsted == false)) { }

		if((renderTasks.IsDone() == true) && (renderTasks.Failed() == true) && (sentFailedResult == false))
		{
			RenderBackend::SendViewFailedResult(viewData.viewId, renderTasks.FailureCode(kUncategorizedError), viewData.sceneData->frontendAddress);
			sentFailedResult = true;
		}

		if(stopRequsted == false)
		{
			boost::thread::yield();
			Delay(50);
		}
	}
}

RTRData::RTRData(ViewData& v, int mrt) :
	viewData(v),
	numRTRframes(0),
	numRenderThreads(mrt),
	numRenderThreadsCompleted(0),
	numPixelsCompleted(0)
{
	width = viewData.GetWidth();
	height = viewData.GetHeight();
	rtrPixels.resize(width * height * 5);
}

RTRData::~RTRData()
{
	event.notify_all();
};

// TODO: it will be more efficient using atomic operators on numRenderThreadsCompleted.
// as it stands, we only use counterMutex to avoid a race condition that exists (or
// if it's not that, then for some reason notify_all() isn't releasing all of the
// waiting threads on win32). currently the below code is slightly sub-optimal in
// that we only get about 95% CPU utilization, as compared to the 99-100% we were
// getting with the code as of rev #110 (see change #4275). however for now we put
// up with a few percent idle CPU time in order to avoid said race condition (which
// lead to all threads ending up waiting on the condition).
const Camera *RTRData::CompletedFrame()
{
	boost::mutex::scoped_lock lock (counterMutex);

	vector<Camera>& cameras = viewData.GetSceneData()->cameras;
	bool ca = viewData.GetSceneData()->clocklessAnimation;

	if(true) // yes I know it's not needed, but I prefer this over headless code blocks
	{
		// test >= in case of weirdness due to the timed wait we use with the boost::condition
		if (++numRenderThreadsCompleted >= numRenderThreads)
		{
			viewData.SetNextRectangle(set<unsigned int>(), 0);
			try
			{
				POVMS_Message pixelblockmsg(kPOVObjectClass_PixelData, kPOVMsgClass_ViewImage, kPOVMsgIdent_PixelBlockSet);
				POVMS_Attribute pixelattr(rtrPixels);

				// we can release the other threads now.
				numRenderThreadsCompleted = 0;
				numRTRframes++;

				event.notify_all();

				pixelblockmsg.Set(kPOVAttrib_PixelBlock, pixelattr);
				pixelblockmsg.SetInt(kPOVAttrib_PixelSize, 1);
				pixelblockmsg.SetInt(kPOVAttrib_Left, 0);
				pixelblockmsg.SetInt(kPOVAttrib_Top, 0);
				pixelblockmsg.SetInt(kPOVAttrib_Right, width - 1);
				pixelblockmsg.SetInt(kPOVAttrib_Bottom, height - 1);

				pixelblockmsg.SetInt(kPOVAttrib_ViewId, viewData.GetViewId());
				pixelblockmsg.SetSourceAddress(viewData.GetSceneData()->backendAddress);
				pixelblockmsg.SetDestinationAddress(viewData.GetSceneData()->frontendAddress);

				POVMS_SendMessage(pixelblockmsg);

				numPixelsCompleted += width * height;
				POVMS_Object obj(kPOVObjectClass_RenderProgress);
				obj.SetInt(kPOVAttrib_Pixels, width * height);
				obj.SetInt(kPOVAttrib_PixelsPending, 0);
				obj.SetInt(kPOVAttrib_PixelsCompleted, numPixelsCompleted);
				RenderBackend::SendViewOutput(viewData.GetViewId(), viewData.GetSceneData()->frontendAddress, kPOVMsgIdent_Progress, obj);

				return(ca ? &cameras[numRTRframes % cameras.size()] : NULL);
			}
			catch(pov_base::Exception&)
			{
				// TODO - work out what we should do here. until then, just re-raise the exception.
				// oh yeah, might want to release any waiting threads first ...
				event.notify_all();
				throw;
			}
		}
	}

	boost::xtime t;
	boost::xtime_get (&t, boost::TIME_UTC);
	t.sec += 3;

	// this will cause us to wait until the other threads are done.
	// we use a timed lock so that we eventually pick up a render cancel request.
	// if we do exit as a result of a timeout, and there is not a cancel pending,
	// things could get out of whack.
	if (!event.timed_wait(lock, t))
		numRenderThreadsCompleted--;

	return(ca ? &cameras[numRTRframes % cameras.size()] : NULL);
}

}
