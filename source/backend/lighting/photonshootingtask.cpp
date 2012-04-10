/*******************************************************************************
 * photonshootingtask.cpp
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
 * $File: //depot/povray/smp/source/backend/lighting/photonshootingtask.cpp $
 * $Revision: #16 $
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
#include "backend/lighting/photonshootingtask.h"
#include "lightgrp.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

PhotonShootingTask::PhotonShootingTask(ViewData *vd, PhotonShootingStrategy* strategy) :
	RenderTask(vd),
	trace(vd->GetSceneData(), GetViewDataPtr(), vd->GetSceneData()->photonSettings.Max_Trace_Level,
	      vd->GetSceneData()->photonSettings.adcBailout, vd->GetQualityFeatureFlags(), cooperate),
	messageFactory(10, 370, "Photon", vd->GetSceneData()->backendAddress, vd->GetSceneData()->frontendAddress, vd->GetSceneData()->sceneId, 0), // TODO FIXME - Values need to come from the correct place!
	rands(0.0, 1.0, 32768),
	randgen(&rands),
	strategy(strategy),
	cooperate(*this),
	maxTraceLevel(vd->GetSceneData()->photonSettings.Max_Trace_Level),
	adcBailout(vd->GetSceneData()->photonSettings.adcBailout)
{
}

PhotonShootingTask::~PhotonShootingTask()
{
}


PhotonMap* PhotonShootingTask::getMediaPhotonMap()
{
	return GetViewDataPtr()->mediaPhotonMap;
}

PhotonMap* PhotonShootingTask::getSurfacePhotonMap()
{
	return GetViewDataPtr()->surfacePhotonMap;
}

void PhotonShootingTask::SendProgress(void)
{
	if (timer.ElapsedRealTime() > 1000)
	{
		// TODO FIXME PHOTONS
		// with multiple threads shooting photons, the stats messages get confusing on the front-end.
		// this is because each thread sends its own count, and so varying numbers get displayed.
		// the totals should be combined and sent from a single thread.
		timer.Reset();
		POVMS_Object obj(kPOVObjectClass_PhotonProgress);
		obj.SetInt(kPOVAttrib_CurrentPhotonCount, GetViewDataPtr()->surfacePhotonMap->numPhotons + GetViewDataPtr()->mediaPhotonMap->numPhotons);
		RenderBackend::SendViewOutput(GetViewData()->GetViewId(), GetSceneData()->frontendAddress, kPOVMsgIdent_Progress, obj);
	}
}



void PhotonShootingTask::Run()
{
	// quit right away if photons not enabled
	if (!GetSceneData()->photonSettings.photonsEnabled) return;

	Cooperate();

	PhotonShootingUnit* unit = strategy->getNextUnit();
	while(unit)
	{
		//ShootPhotonsAtObject(unit->lightAndObject.target, unit->lightAndObject.light);
		ShootPhotonsAtObject(unit->lightAndObject);
		unit = strategy->getNextUnit();
	}


	// good idea to make sure all warnings and errors arrive frontend now [trf]
	SendProgress();
	Cooperate();
}

void PhotonShootingTask::Stopped()
{
	// nothing to do for now [trf]
}

void PhotonShootingTask::Finish()
{
	GetViewDataPtr()->timeType = SceneThreadData::kPhotonTime;
	GetViewDataPtr()->realTime = ConsumedRealTime();
	GetViewDataPtr()->cpuTime = ConsumedCPUTime();
}






void PhotonShootingTask::ShootPhotonsAtObject(LightTargetCombo& combo)
{
	RGBColour colour;              /* light color */
	Colour PhotonColour;           /* photon color */
	int i;                         /* counter */
	DBL theta, phi;                /* rotation angles */
	DBL dphi;              /* deltas for theta and phi */
	DBL jittheta, jitphi;          /* jittered versions of theta and phi */
	DBL minphi,maxphi;
	                               /* these are minimum and maximum for theta and
	                                   phi for the spiral shooting */
	DBL Attenuation;               /* light attenuation for spotlight */
	TRANSFORM Trans;               /* transformation for rotation */
	int mergedFlags=0;             /* merged flags to see if we should shoot photons */
	int notComputed=true;          /* have the ray containers been computed for this point yet?*/
	int hitAtLeastOnce = false;    /* have we hit the object at least once - for autostop stuff */
	ViewThreadData *renderDataPtr = GetViewDataPtr();

	/* get the light source colour */
	colour = combo.light->colour;

	/* set global variable stuff */
	renderDataPtr->photonSourceLight = combo.light;
	renderDataPtr->photonTargetObject = combo.target;

	/* first, check on various flags... make sure all is a go for this ObjectPtr */
	mergedFlags = combo.computeMergedFlags();

	if (!( ((mergedFlags & PH_RFR_ON_FLAG) && !(mergedFlags & PH_RFR_OFF_FLAG)) ||
	       ((mergedFlags & PH_RFL_ON_FLAG) && !(mergedFlags & PH_RFL_OFF_FLAG)) ))
		/* it is a no-go for this object... bail out now */
		return;

	renderDataPtr->photonSpread = combo.photonSpread;
		
	/* ---------------------------------------------
	       main ray-shooting loop 
	   --------------------------------------------- */
	i = 0;
	notComputed = true;
	for(theta=combo.mintheta; theta<combo.maxtheta; theta+=combo.dtheta)
	{
		Cooperate();
		SendProgress();
		renderDataPtr->hitObject = false;
		
		if (theta<EPSILON)
		{
			dphi=2*M_PI;
		}
		else
		{
			/* remember that for area lights, "theta" really means "radius" */
			if (combo.light->Parallel)
			{
				dphi = combo.dtheta / theta;
			}
			else
			{
				dphi=combo.dtheta/sin(theta);
			}
		}

		// FIXME: should copy from previously computed shootingdirection
		ShootingDirection shootingDirection(combo.light,combo.target);
		shootingDirection.compute();

		minphi = -M_PI + dphi*randgen()*0.5;
		maxphi = M_PI - dphi/2 + (minphi+M_PI);
		for(phi=minphi; phi<maxphi; phi+=dphi)
		{
			int x_samples,y_samples;
			int area_x, area_y;
			/* ------------------- shoot one photon ------------------ */

			/* jitter theta & phi */
			jitphi = phi + (dphi)*(randgen() - 0.5)*1.0*GetSceneData()->photonSettings.jitter;
			jittheta = theta + (combo.dtheta)*(randgen() - 0.5)*1.0*GetSceneData()->photonSettings.jitter;

			/* actually, shoot multiple samples for area light */
			if(combo.light->Area_Light && combo.light->Photon_Area_Light && !combo.light->Parallel)
			{
				x_samples = combo.light->Area_Size1;
				y_samples = combo.light->Area_Size2;
			}
			else
			{
				x_samples = 1;
				y_samples = 1;
			}

			for(area_x=0; area_x<x_samples; area_x++)
			{
				for(area_y=0; area_y<y_samples; area_y++)
				{
					Ray ray;                 /* ray that we shoot */

					Assign_Vector(ray.Origin,combo.light->Center);

					if (combo.light->Area_Light && combo.light->Photon_Area_Light && !combo.light->Parallel)
					{
						shootingDirection.recomputeForAreaLight(ray,area_x,area_y);
						/* we must recompute the media containers (new start point) */
						notComputed = true;
					}

					DBL dist_of_initial_from_center;

					if (combo.light->Parallel)
					{
						DBL a;
						VECTOR v;
						/* assign the direction */
						Assign_Vector(ray.Direction,combo.light->Direction);
					
						/* project ctr onto plane defined by Direction & light location */

						VDot(a,ray.Direction, shootingDirection.toctr);
						VScale(v,ray.Direction, -a*shootingDirection.dist); /* MAYBE NEEDS TO BE NEGATIVE! */

						VAdd(ray.Origin, shootingDirection.ctr, v);

						/* move point along "left" distance theta (remember theta means rad) */
						VScale(v,shootingDirection.left,jittheta);

						/* rotate pt around ray.Direction by phi */
						/* use POV funcitons... slower but easy */
						Compute_Axis_Rotation_Transform(&Trans,combo.light->Direction,jitphi);
						MTransPoint(v, v, &Trans);

						VAddEq(ray.Origin, v);

						// compute the length of "v" if we're going to use it
						if (combo.light->Light_Type == CYLINDER_SOURCE)
						{
							VECTOR initial_from_center;
							VSub(initial_from_center, ray.Origin, combo.light->Center);
							VLength(dist_of_initial_from_center, initial_from_center);
						}
					}
					else
					{
						DBL st,ct;                     /* cos(theta) & sin(theta) for rotation */
						/* rotate toctr by theta around up */
						st = sin(jittheta);
						ct = cos(jittheta);
						/* use fast rotation */
						shootingDirection.v[X] = -st*shootingDirection.left[X] + ct*shootingDirection.toctr[X];
						shootingDirection.v[Y] = -st*shootingDirection.left[Y] + ct*shootingDirection.toctr[Y];
						shootingDirection.v[Z] = -st*shootingDirection.left[Z] + ct*shootingDirection.toctr[Z];

						/* then rotate by phi around toctr */
						/* use POV funcitons... slower but easy */
						Compute_Axis_Rotation_Transform(&Trans,shootingDirection.toctr,jitphi);
						MTransPoint(ray.Direction, shootingDirection.v, &Trans);
					}

					/* ------ attenuation for spot/cylinder (copied from point.c) ---- */
					Attenuation = computeAttenuation(combo.light, ray, dist_of_initial_from_center);

					/* set up defaults for reflection, refraction */
					renderDataPtr->passThruPrev = true;
					renderDataPtr->passThruThis = false;

					renderDataPtr->photonDepth = 0.0;
					// GetViewDataPtr()->Trace_Level = 0;
					// Total_Depth = 0.0;
					renderDataPtr->Stats()[Number_Of_Photons_Shot]++;

					/* attenuate for area light extra samples */
					Attenuation/=(x_samples*y_samples);

					/* compute photon color from light source & attenuation */

					PhotonColour = Colour(colour * Attenuation);

					if (Attenuation<0.00001) continue;

					/* handle the projected_through object if it exists */
					if (combo.light->Projected_Through_Object != NULL)
					{
						/* try to intersect ray with projected-through ObjectPtr */
						Intersection Intersect;

						Intersect.Object = NULL;
						if ( trace.FindIntersection(combo.light->Projected_Through_Object, Intersect, ray) )
						{
							/* we must recompute the media containers (new start point) */
							notComputed = true;

							/* we did hit it, so find the 'real' starting point of the ray */
							/* find the farthest intersection */
							VAddScaledEq(ray.Origin,Intersect.Depth+EPSILON, ray.Direction);
							renderDataPtr->photonDepth += Intersect.Depth+EPSILON;
							while(trace.FindIntersection( combo.light->Projected_Through_Object, Intersect, ray) )
							{
								VAddScaledEq(ray.Origin, Intersect.Depth+EPSILON, ray.Direction);
								renderDataPtr->photonDepth += Intersect.Depth+EPSILON;
							}
						}
						else
						{
							/* we didn't hit it, so stop now */
							continue;
						}

					}

					/* As mike said, "fire photon torpedo!" */
					//Initialize_Ray_Containers(&ray);
					ray.ClearInteriors () ;

					for(vector<ObjectPtr>::iterator object = GetSceneData()->objects.begin(); object != GetSceneData()->objects.end(); object++)
					{
						if((*object)->Inside(ray.Origin, renderDataPtr) && ((*object)->interior != NULL))
							ray.AppendInterior((*object)->interior);
					}

					notComputed = false;
					//disp_elem = 0;   /* for dispersion */
					//disp_nelems = 0; /* for dispersion */

					Trace::TraceTicket ticket(maxTraceLevel, adcBailout);

					ray.SetFlags(Ray::PrimaryRay, false, true);
					trace.TraceRay(ray, PhotonColour, 1.0, ticket, false);

					/* display here */
					if ((i++%100) == 0)
					{
						Cooperate();
						SendProgress();
					}

				} // for(area_y...)
			} // for(area_x...)
		}

		/* if we didn't hit anything and we're past the autostop angle, then
			 we should stop 
			 
			 as per suggestion from Smellenberg, changed autostop to a percentage
			 of the object's bounding sphere. */

		/* suggested by Pabs, we only use autostop if we have it it once */
		if (renderDataPtr->hitObject) hitAtLeastOnce=true;

		if (hitAtLeastOnce && !renderDataPtr->hitObject && renderDataPtr->photonTargetObject)
			if (theta > GetSceneData()->photonSettings.autoStopPercent*combo.maxtheta)
				break;
	} /* end of rays loop */
}

DBL PhotonShootingTask::computeAttenuation(LightSource* Light, Ray& ray, DBL dist_of_initial_from_center)
{
	DBL costheta_spot;
	DBL Attenuation = 1.0;

	/* ---------- spot light --------- */
	if (Light->Light_Type == SPOT_SOURCE)
	{
		VDot(costheta_spot, ray.Direction, Light->Direction);

		if (costheta_spot > 0.0)
		{
			Attenuation = pow(costheta_spot, Light->Coeff);

			if (Light->Radius > 0.0)
				Attenuation *= cubic_spline(Light->Falloff, Light->Radius, costheta_spot);

		}
		else
			Attenuation = 0.0;
	}
	/* ---------- cylinder light ----------- */
	else if (Light->Light_Type == CYLINDER_SOURCE)
	{
		DBL k, len;

		VDot(k, ray.Direction, Light->Direction);

		if (k > 0.0)
		{
			len = dist_of_initial_from_center;

			if (len < Light->Falloff)
			{
				DBL dist = 1.0 - len / Light->Falloff;
				Attenuation = pow(dist, Light->Coeff);

				if (Light->Radius > 0.0 && len > Light->Radius)
					Attenuation *= cubic_spline(0.0, 1.0 - Light->Radius / Light->Falloff, dist);

			}
			else
				Attenuation = 0.0;
		}
		else
			Attenuation = 0.0;
	}
	return Attenuation;
}

}
