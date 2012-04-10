/*******************************************************************************
 * tracetask.h
 *
 * This file contains ... TODO ...
 *
 * from Persistence of Vision(tm) Ray Tracer version 3.7.
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
 * $File: //depot/povray/smp/source/backend/render/tracetask.h $
 * $Revision: #37 $
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

#ifndef POVRAY_BACKEND_TRACETASK_H
#define POVRAY_BACKEND_TRACETASK_H

#include <vector>

#include <boost/thread.hpp>

#include "backend/frame.h"
#include "backend/povray.h"
#include "backend/scene/view.h"
#include "backend/scene/scene.h"
#include "backend/render/rendertask.h"
#include "backend/render/tracepixel.h"

namespace pov
{

#ifdef PROFILE_INTERSECTIONS
	// NB not thread-safe (and not intended to be)
	extern POV_ULONG gIntersectionTime;
	extern vector <vector<POV_ULONG> > gBSPIntersectionTimes;
	extern vector <vector<POV_ULONG> > gBVHIntersectionTimes;
	extern vector <vector<POV_ULONG> > *gIntersectionTimes;
#endif

class TraceTask : public RenderTask
{
	public:
		TraceTask(ViewData *vd, unsigned int tm, DBL js, DBL aat, unsigned int aad, GammaCurvePtr& aag, unsigned int ps, bool psc, bool final, bool hr);
		virtual ~TraceTask();

		virtual void Run();
		virtual void Stopped();
		virtual void Finish();
	private:
		class CooperateFunction : public Trace::CooperateFunctor
		{
			public:
				CooperateFunction(Task& t) : task(t) { }
				virtual void operator()() { task.Cooperate(); }
			private:
				Task& task;
		};

		class SubdivisionBuffer
		{
			public:
				SubdivisionBuffer(size_t s);

				void SetSample(size_t x, size_t y, const Colour& col);
				bool Sampled(size_t x, size_t y);

				Colour& operator()(size_t x, size_t y);

				void Clear();
			private:
				vector<Colour> colors;
				vector<bool> sampled;
				size_t size;
		};

		unsigned int tracingMethod;
		DBL jitterScale;
		DBL aaThreshold;
		unsigned int aaDepth;
		unsigned int previewSize;
		bool previewSkipCorner;
		bool finalTrace;
		bool highReproducibility;
		GammaCurvePtr aaGamma;

		/// tracing core
		TracePixel trace;

		CooperateFunction cooperate;
		MediaFunction media;
		RadiosityFunction radiosity;
		PhotonGatherer photonGatherer;

		void SimpleSamplingM0();
		void SimpleSamplingM0P();
		void NonAdaptiveSupersamplingM1();
		void AdaptiveSupersamplingM2();

		void NonAdaptiveSupersamplingForOnePixel(DBL x, DBL y, Colour& leftcol, Colour& topcol, Colour& curcol, bool& sampleleft, bool& sampletop, bool& samplecurrent);
		void SupersampleOnePixel(DBL x, DBL y, Colour& col);
		void SubdivideOnePixel(DBL x, DBL y, DBL d, size_t bx, size_t by, size_t bstep, SubdivisionBuffer& buffer, Colour& result, int level);
};

}

#endif // POVRAY_BACKEND_TRACETASK_H
