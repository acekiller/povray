/*******************************************************************************
 * threaddata.h
 *
 * This file contains ... TODO ...
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
 * $File: //depot/povray/smp/source/backend/scene/threaddata.h $
 * $Revision: #69 $
 * $Change: 5451 $
 * $DateTime: 2011/06/04 08:55:36 $
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

#ifndef POVRAY_BACKEND_THREADDATA_H
#define POVRAY_BACKEND_THREADDATA_H

#include <vector>
#include <stack>

#include "base/types.h"
#include "backend/frame.h"
#include "backend/support/task.h"
#include "backend/support/statistics.h"
#include "backend/shape/mesh.h"
#include "backend/pattern/pattern.h"

namespace pov
{

using namespace pov_base;

class SceneData;
class ViewData;
class FunctionVM;
struct FPUContext;
struct ISO_ThreadData;

class PhotonMap;
struct Blob_Interval_Struct;

/**
 *	Class holding parser thread specific data.
 */
class SceneThreadData : public Task::TaskData
{
		friend class Scene;
		friend class Trace;
		friend class View; // TODO FIXME - needed only to access TraceThreadData for CheckCameraHollowObject()
	public:
		/**
		 *	Create thread local data.
		 *	@param	sd				Scene data defining scene attributes.
		 */
		SceneThreadData(shared_ptr<SceneData> sd);

		/**
		 *	Get the statistics.
		 *	@return					Reference to statistic counters.
		 */
		RenderStatistics& Stats(void) { return renderStats; }

		DBL *Fractal_IStack[4];
		PriorityQueue Mesh_Queue;
		void **Blob_Queue;
		unsigned int Max_Blob_Queue_Size;
		DBL *Blob_Coefficients;
		Blob_Interval_Struct *Blob_Intervals;
		int Blob_Coefficient_Count;
		int Blob_Interval_Count;
		ISO_ThreadData *isosurfaceData;
		void *BCyl_Intervals;
		void *BCyl_RInt;
		void *BCyl_HInt;
		IStackPool stackPool;
		FPUContext *functionContext;
		vector<FPUContext *> functionPatternContext;
		int Facets_Last_Seed;
		int Facets_CVC;
		VECTOR Facets_Cube[81];

		// TODO FIXME - thread-local copy of lightsources. we need this
		// because various parts of the lighting code seem to make changes
		// to the lightsource object passed to them (this is not confined
		// just to the area light shadow code). This code ought to be fixed
		// to treat the lightsource as const, after which this can go away.
		vector<LightSource *> lightSources;

		// all of these are for photons
		// most of them should be refactored into parameters, return values, or other objects
		LightSource *photonSourceLight;
		ObjectPtr photonTargetObject;
		bool litObjectIgnoresPhotons;
		RGBColour GFilCol;
		int hitObject;    // did we hit the target object? (for autostop)
		DBL photonSpread; // photon spread (in radians)
		DBL photonDepth;  // total distance from light to intersection
		int passThruThis;           // is this a pass-through object encountered before the target?
		int passThruPrev;           // was the previous object a pass-through object encountered before the target?
		bool Light_Is_Global;       // is the current light global? (not part of a light_group?)
		PhotonMap* surfacePhotonMap;
		PhotonMap* mediaPhotonMap;

		Crackle_Cache_Type Crackle_Cache;

		// data for waves and ripples pattern
		unsigned int numberOfWaves;
		vector<double> waveFrequencies;
		vector<Vector3d> waveSources;

		/**
		 * called after a rectangle is finished
		 * used for crackle cache expiry
		 */
		void AfterTile();

		/**
		 * @returns the index of the current rectangle rendered
		 * used by the crackle pattern to indicate age of cache entries
		 */
		inline size_t ProgressIndex() const { return progress_index; }

		enum TimeType
		{
			kUnknownTime,
			kParseTime,
			kBoundingTime,
			kPhotonTime,
			kRadiosityTime,
			kRenderTime,
			kMaxTimeType
		};

		TimeType timeType;
		POV_LONG cpuTime;
		POV_LONG realTime;
		unsigned int qualityFlags; // TODO FIXME - remove again

		inline shared_ptr<const SceneData> GetSceneData() const { return sceneData; }

	protected:
		/// scene data
		shared_ptr<SceneData> sceneData;
		/// render statistics
		RenderStatistics renderStats;

	private:
		/// not available
		SceneThreadData();

		/// not available
		SceneThreadData(const SceneThreadData&);

		/// not available
		SceneThreadData& operator=(const SceneThreadData&);

		/// current number of Tiles to expire crackle cache entries after
		size_t CrCache_MaxAge;
		/// current tile index (for crackle cache expiry)
		size_t progress_index;

	public: // TODO FIXME - temporary workaround [trf]

		/**
		 *	Destructor.
		 */
		~SceneThreadData();
};

/**
 *	Class holding render thread specific data.
 */
class ViewThreadData : public SceneThreadData
{
		friend class Scene;
	public:
		/**
		 *	Create thread local data.
		 *	@param	vd				View data defining view attributes
		 *							as well as view output.
		 */
		ViewThreadData(ViewData *vd);

		/**
		 *	Get width of view.
		 *	@return					Width.
		 */
		unsigned int GetWidth() const;

		/**
		 *	Get height of view.
		 *	@return					Height.
		 */
		unsigned int GetHeight() const;

		/**
		 *	Get area of view to be rendered.
		 *	@return					Area rectangle.
		 */
		const POVRect& GetRenderArea();
	protected:
		/// view data
		ViewData *viewData;
	private:
		/// not available
		ViewThreadData();

		/// not available
		ViewThreadData(const ViewThreadData&);

		/// not available
		ViewThreadData& operator=(const ViewThreadData&);
	public: // TODO FIXME - temporary workaround [trf]
		/**
		 *	Destructor.
		 */
		~ViewThreadData();
};

}

#endif // POVRAY_BACKEND_THREADDATA_H
