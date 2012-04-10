/*******************************************************************************
 * tracepixel.h
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
 * $File: //depot/povray/smp/source/backend/render/tracepixel.h $
 * $Revision: #23 $
 * $Change: 5401 $
 * $DateTime: 2011/02/08 21:06:55 $
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

#ifndef POVRAY_BACKEND_TRACEPIXEL_H
#define POVRAY_BACKEND_TRACEPIXEL_H

#include <vector>

#include <boost/thread.hpp>

#include "backend/frame.h"
#include "backend/povray.h"
#include "backend/scene/view.h"
#include "backend/scene/scene.h"
#include "backend/render/trace.h"

namespace pov
{

class BSPIntersectFunctor : public BSPTree::Intersect
{
	public:
		BSPIntersectFunctor(Intersection& bi, const Ray& r, vector<ObjectPtr>& objs, TraceThreadData *t) :
			found(false),
			bestisect(bi),
			ray(r),
			objects(objs),
			traceThreadData(t)
		{
			Vector3d tmp(1.0 / ray.GetDirection()[X], 1.0 / ray.GetDirection()[Y], 1.0 /ray.GetDirection()[Z]);
			Assign_Vector(origin, ray.Origin);
			Assign_Vector(invdir, *tmp);
			variant = (ObjectBase::BBoxDirection)((int(invdir[X] < 0.0) << 2) | (int(invdir[Y] < 0.0) << 1) | int(invdir[Z] < 0.0));
		}

		virtual bool operator()(unsigned int index, double& maxdist)
		{
			ObjectPtr object = objects[index];
			Intersection isect;

			if(Find_Intersection(&isect, object, ray, variant, origin, invdir, traceThreadData) && (isect.Depth <= maxdist))
			{
				if(isect.Depth < bestisect.Depth)
				{
					bestisect = isect;
					found = true;
					maxdist = bestisect.Depth;
				}
			}

			return found;
		}

		virtual bool operator()() const { return found; }
	private:
		bool found;
		vector<ObjectPtr>& objects;
		Intersection& bestisect;
		const Ray& ray;
		BBOX_VECT origin;
		BBOX_VECT invdir;
		ObjectBase::BBoxDirection variant;
		TraceThreadData *traceThreadData;
};

class BSPIntersectCondFunctor : public BSPTree::Intersect
{
	public:
		BSPIntersectCondFunctor(Intersection& bi, const Ray& r, vector<ObjectPtr>& objs, TraceThreadData *t,
		                        const RayObjectCondition& prec, const RayObjectCondition& postc) :
			found(false),
			bestisect(bi),
			ray(r),
			objects(objs),
			traceThreadData(t),
			precondition(prec),
			postcondition(postc)
		{
			Vector3d tmp(1.0 / ray.GetDirection()[X], 1.0 / ray.GetDirection()[Y], 1.0 /ray.GetDirection()[Z]);
			Assign_Vector(origin, ray.Origin);
			Assign_Vector(invdir, *tmp);
			variant = (ObjectBase::BBoxDirection)((int(invdir[X] < 0.0) << 2) | (int(invdir[Y] < 0.0) << 1) | int(invdir[Z] < 0.0));
		}

		virtual bool operator()(unsigned int index, double& maxdist)
		{
			ObjectPtr object = objects[index];

			if(precondition(ray, object, 0.0) == true)
			{
				Intersection isect;

				if(Find_Intersection(&isect, object, ray, variant, origin, invdir, postcondition, traceThreadData) && (isect.Depth <= maxdist))
				{
					if(isect.Depth < bestisect.Depth)
					{
						bestisect = isect;
						found = true;
						maxdist = bestisect.Depth;
					}
				}
			}

			return found;
		}

		virtual bool operator()() const { return found; }
	private:
		bool found;
		vector<ObjectPtr>& objects;
		Intersection& bestisect;
		const Ray& ray;
		BBOX_VECT origin;
		BBOX_VECT invdir;
		ObjectBase::BBoxDirection variant;
		TraceThreadData *traceThreadData;
		const RayObjectCondition& precondition;
		const RayObjectCondition& postcondition;
};

class BSPInsideCondFunctor : public BSPTree::Inside
{
	public:
		BSPInsideCondFunctor(Vector3d o, vector<ObjectPtr>& objs, TraceThreadData *t,
		                     const PointObjectCondition& prec, const PointObjectCondition& postc) :
			found(false),
			origin(o),
			objects(objs),
			precondition(prec),
			postcondition(postc),
			threadData(t)
		{
		}

		virtual bool operator()(unsigned int index)
		{
			ObjectPtr object = objects[index];
			if(precondition(origin, object))
				if(Inside_BBox(*origin, object->BBox) && object->Inside(*origin, threadData))
					if(postcondition(origin, object))
						found = true;
			return found;
		}

		virtual bool operator()() const { return found; }
	private:
		bool found;
		vector<ObjectPtr>& objects;
		Vector3d origin;
		const PointObjectCondition& precondition;
		const PointObjectCondition& postcondition;
		TraceThreadData *threadData;
};

struct HasInteriorPointObjectCondition : public PointObjectCondition
{
	virtual bool operator()(const Vector3d& point, const ObjectBase* object) const
	{
		return object->interior != NULL;
	}
};

struct ContainingInteriorsPointObjectCondition : public PointObjectCondition
{
	ContainingInteriorsPointObjectCondition(RayInteriorVector& ci) : containingInteriors(ci) {}
	virtual bool operator()(const Vector3d& point, const ObjectBase* object) const
	{
		containingInteriors.push_back(object->interior);
		return true;
	}
	RayInteriorVector &containingInteriors;
};

class TracePixel : public Trace
{
	public:
		TracePixel(ViewData *vd, TraceThreadData *td, unsigned int mtl, DBL adcb, unsigned int qf,
		           CooperateFunctor& cf, MediaFunctor& mf, RadiosityFunctor& af, bool pt = false);
		virtual ~TracePixel();
		void SetupCamera(const Camera& cam);

		void operator()(DBL x, DBL y, DBL width, DBL height, Colour& colour);
	private:
		// Focal blur data
		class FocalBlurData
		{
		public:
			FocalBlurData(const Camera& camera, TraceThreadData* threadData);
			~FocalBlurData();

			// Direction to focal plane. 
			DBL Focal_Distance;
			// Array of threshold for confidence test. 
			DBL *Sample_Threshold;
			// Array giving number of samples to take before next confidence test. 
			const int *Current_Number_Of_Samples;
			// Array of sample locations. 
			Vector2d *Sample_Grid;
			// Maximum amount of jitter to use. 
			DBL Max_Jitter;
			// Vectors in the viewing plane. 
			VECTOR XPerp, YPerp;

		};

		bool useFocalBlur;
		FocalBlurData *focalBlurData;

		bool precomputeContainingInteriors;
		RayInteriorVector containingInteriors;

		Vector3d cameraDirection;
		Vector3d cameraRight;
		Vector3d cameraUp;
		Vector3d cameraLocation;
		/// length of current camera's 'right' vector prior to normalisation
		DBL cameraLengthRight;
		/// length of current camera's 'up' vector prior to normalisation
		DBL cameraLengthUp;
		/// aspect ratio for current camera
		DBL aspectRatio;
		/// camera
		Camera camera;
		/// scene data
		shared_ptr<SceneData> sceneData;
		/// thread data
		TraceThreadData *threadData;

		/// maximum trace recursion level allowed
		unsigned int maxTraceLevel;
		/// adc bailout
		DBL adcBailout;
		/// whether this is just a pretrace, allowing some computations to be skipped
		bool pretrace;

		bool CreateCameraRay(Ray& ray, DBL x, DBL y, DBL width, DBL height, size_t ray_number);

		void InitRayContainerState(Ray& ray, bool compute = false);
		void InitRayContainerStateTree(Ray& ray, BBOX_TREE *node);

		void TraceRayWithFocalBlur(Colour& colour, DBL x, DBL y, DBL width, DBL height);
		void JitterCameraRay(Ray& ray, DBL x, DBL y, size_t ray_number);
};

}

#endif // POVRAY_BACKEND_TRACEPIXEL_H
