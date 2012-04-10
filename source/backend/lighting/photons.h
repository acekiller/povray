/*******************************************************************************
 * photons.h
 *
 * This module contains all defines, typedefs, and prototypes for photons.cpp.
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
 * $File: //depot/povray/smp/source/backend/lighting/photons.h $
 * $Revision: #28 $
 * $Change: 5569 $
 * $DateTime: 2011/12/01 09:21:57 $
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

#ifndef PHOTONS_H
#define PHOTONS_H

#include "base/povms.h"
#include "backend/frame.h"
#include "backend/render/trace.h"
#include "backend/control/messagefactory.h"
#include "backend/colour/colutils.h"
#include "backend/interior/media.h"

namespace pov
{

using namespace pov_base;

#define MEDIA_INTERACTION 1

/* ------------------------------------------------------ */
class ScenePhotonSettings
{
	public:
		ScenePhotonSettings()
		{
			photonsEnabled = false;

			surfaceSeparation = 0.1;
			globalSeparation = 0.1;
			surfaceCount = 0;

			expandTolerance = 0;
			minExpandCount = 0;
			minGatherCount = 10;
			maxGatherCount = 100;

			// trace settings for the photon pre-trace
			Max_Trace_Level = 10;
			adcBailout = 0.001;
			jitter = 0.25;

			// see the POV documentation for info about autostop
			autoStopPercent = 1.0;  // disabled by default

			// these are used for saving or loading the photon map
			// note: save and load are mutually exclusive - there are three states: save, load, neither
			fileName = NULL;
			saveFile = false;
			loadFile = false;

			#ifdef GLOBAL_PHOTONS
			// ---------- global photon map ----------
			int globalCount = 0;  // disabled by default
			#endif

			// settings for media photons
			mediaSpacingFactor = 0;
			maxMediaSteps = 0;  // disabled by default

			// refleciton blur settings
			// to be used with the reflection blur patch
			//photonReflectionBlur = false;
		}
		~ScenePhotonSettings()
		{
			if ( fileName )
			{
				POV_FREE(fileName);
				fileName=NULL;
			}
		}



		bool photonsEnabled;

		// photon separation (a.k.a. spacing)
		DBL surfaceSeparation;
		DBL globalSeparation;
		// ...or photon count
		int surfaceCount;

		// settings for the adaptive search
		// see POV documentation for an explanation
		DBL expandTolerance;
		int minExpandCount;
		int minGatherCount;
		int maxGatherCount;

		// trace settings for the photon pre-trace
		int Max_Trace_Level;
		DBL adcBailout;
		DBL jitter;

		// see the POV documentation for info about autostop
		DBL autoStopPercent;

		// these are used for saving or loading the photon map
		// note: save and load are mutually exclusive - there are three states: save, load, neither
		char* fileName;
		bool saveFile;
		bool loadFile;

		#ifdef GLOBAL_PHOTONS
		// ---------- global photon map ----------
		int globalPhotonsToShoot;      // number of global photons to shoot
		DBL globalGatherRad;           // minimum gather radius
		int globalCount;
		#endif

		// settings for media photons
		DBL mediaSpacingFactor;
		int maxMediaSteps;

		// refleciton blur settings
		// to be used with the reflection blur patch
		//bool photonReflectionBlur;
};

/* ------------------------------------------------------ */
/* photon */
/* ------------------------------------------------------ */
struct Photon
{
	void init(unsigned char _info)
	{
		this->info = _info;
	}

	SNGL_VECT Loc;          /* location */
	SMALL_COLOUR colour;    /* color & intensity (flux) */
	unsigned char info;     /* info byte for kd-tree */
	signed char theta, phi; /* incoming direction */
};

typedef Photon* PhotonBlock;

/* ------------------------------------------------------ */
/* photon map */
/* ------------------------------------------------------ */

/* this is for photon block allocation and array mapping functions */
extern const int PHOTON_BLOCK_POWER;
extern const int PHOTON_BLOCK_SIZE;
extern const int PHOTON_BLOCK_MASK;
extern const int INITIAL_BASE_ARRAY_SIZE;

class PhotonMap
{
	public:
		Photon **head;
		int numBlocks;        /* number of blocks in base array */
		int numPhotons;       /* total number of photons used */

		DBL minGatherRad;       /* minimum gather radius */
		DBL minGatherRadMult;   /* minimum gather radius multiplier (for speed adjustments) */
		DBL gatherRadStep;      /* step size for gather expansion */
		int gatherNumSteps;     /* maximum times to perform 'gather' */

		PhotonMap();
		~PhotonMap();

		void swapPhotons(int a, int b);
		void insertSort(int start, int end, int d);
		void quickSortRec(int left, int right, int d);
		void halfSortRec(int left, int right, int d, int mid);
		void sortAndSubdivide(int start, int end, int /*sorted*/);
		void buildTree();

		void setGatherOptions(ScenePhotonSettings& photonSettings, int mediaMap);

		Photon* AllocatePhoton();
		//mutex allocatePhotonMutex;

		void mergeMap(PhotonMap* map);
};


/* ------------------------------------------------------ */
/*
  Photon array mapping function

  This converts a one-dimensional index into a two-dimensional address
  in the photon array.

  Photon memory looks like this:

    # -> **********
    # -> **********  <- blocks of photons
    # -> **********
    # -> /
    # -> /
    :
    ^
    |
    Base array.

  The base array (which is dynamically resized as needed) contians pointers
  to blocks of photons.  Blocks of photons (of fixed size of a power of two)
  are allocated as needed.

  This mapping function converts a one-dimensional index and into a two-
  dimensional address consisting of a block index and an offset within
  that block.

  Note that, while this data structure allows easy allocation of photons,
  it does not easily permit deallocation of photons.  Once photons are placed
  into the photon map, they are not removed.
*/
// if this is changed, you must also change swapPhotons() and
// allocatePhoton, both in photons.c
#define PHOTON_AMF(map, idx)   map[(idx)>>PHOTON_BLOCK_POWER][(idx) & PHOTON_BLOCK_MASK]


/* ------------------------------------------------------ */
/* photon gatherer */
/* ------------------------------------------------------ */
class GatheredPhotons
{
	public:
		// priority queue arrays
		Photon** photonGatherList;  // photons
		DBL *photonDistances;       // priorities
		int numFound;  // number of photons found

		void swapWith(GatheredPhotons& theOther);
	
		GatheredPhotons(int maxGatherCount);
		~GatheredPhotons();
};

class PhotonGatherer
{
	public:
		ScenePhotonSettings& photonSettings;
		PhotonMap *map;

		DBL size_sq_s;   // search radius squared
		DBL Size_s;      // search radius (static)
		DBL sqrt_dmax_s, dmax_s;      // dynamic search radius... current maximum
		int TargetNum_s; // how many to gather
		const DBL *pt_s;       // point around which we are gathering
		const DBL *norm_s;     // surface normal
		DBL flattenFactor; // amount to flatten the spher to make it
		                   // an ellipsoid when gathering photons
		                   // zero = no flatten, one = regular
		bool gathered;
		DBL alreadyGatheredRadius;

		GatheredPhotons gatheredPhotons;

		PhotonGatherer(PhotonMap *map, ScenePhotonSettings& photonSettings);

		void gatherPhotonsRec(int start, int end);
		int gatherPhotons(const VECTOR pt, DBL Size, DBL *r, const VECTOR norm, bool flatten);
		DBL gatherPhotonsAdaptive(const VECTOR pt, const VECTOR norm, bool flatten);

		void PQInsert(Photon *photon, DBL d);
		void FullPQInsert(Photon *photon, DBL d);
};

class PhotonMediaFunction : public MediaFunction
{
	public:
		PhotonMediaFunction(shared_ptr<SceneData> sd, TraceThreadData *td, Trace *t, PhotonGatherer *pg);

		void ComputeMediaAndDepositPhotons(MediaVector& medias, const Ray& ray, Intersection& isect, Colour& colour, Trace::TraceTicket& ticket);
	protected:
		void DepositMediaPhotons(Colour& colour, MediaVector& medias, LightSourceEntryVector& lights, MediaIntervalVector& mediaintervals,
		                         const Ray& ray, Media *IMedia, int minsamples, bool ignore_photons, bool use_scattering, bool all_constant_and_light_ray, Trace::TraceTicket& ticket);
	private:
		shared_ptr<SceneData> sceneData;

		void addMediaPhoton(const VECTOR Point, const VECTOR Origin, const RGBColour& LightCol, DBL depthDiff);
};

class PhotonTrace : public Trace
{
	public:
		PhotonTrace(shared_ptr<SceneData> sd, TraceThreadData *td, unsigned int mtl, DBL adcb, unsigned int qf, Trace::CooperateFunctor& cf);
		~PhotonTrace();

		virtual DBL TraceRay(const Ray& ray, Colour& colour, COLC weight, Trace::TraceTicket& ticket, bool continuedRay, DBL maxDepth = 0.0);
	protected:
		virtual void ComputeLightedTexture(Colour& LightCol, TEXTURE *Texture, vector<TEXTURE *>& warps, const Vector3d& ipoint, const Vector3d& rawnormal, const Ray& ray, COLC weight, Intersection& isect, Trace::TraceTicket& ticket);
		bool ComputeRefractionForPhotons(const FINISH* finish, Interior *interior, const Vector3d& ipoint, const Ray& ray, const Vector3d& normal, const Vector3d& rawnormal, Colour& colour, COLC weight, Trace::TraceTicket& ticket);
		bool TraceRefractionRayForPhotons(const FINISH* finish, const Vector3d& ipoint, const Ray& ray, Ray& nray, DBL ior, DBL n, const Vector3d& normal, const Vector3d& rawnormal, const Vector3d& localnormal, Colour& colour, COLC weight, Trace::TraceTicket& ticket);
	private:
		PhotonMediaFunction mediaPhotons;
		RadiosityFunctor noRadiosity;

		void addSurfacePhoton(const VECTOR Point, const VECTOR Origin, const RGBColour& LightCol);
};

// foward declaration
class PhotonShootingStrategy;
class LightTargetCombo;

/* ------------------------------------------------------ */
/* photon map builder */
/* ------------------------------------------------------ */

class ShootingDirection
{
	public:
		ShootingDirection(LightSource* light, ObjectPtr target):light(light),target(target) {}

		LightSource* light;
		ObjectPtr target;
		VECTOR up, left, ctr, toctr, v; /* vectors to determine direction of shot */
		DBL dist;                      /* distance from light to center of bounding sphere */
		DBL rad;                       /* radius of bounding sphere */

		void compute();
		void recomputeForAreaLight(Ray& ray, int area_x, int area_y);
	};


class LightTargetCombo
{
	public:
		LightTargetCombo(LightSource *light, ObjectPtr target):light(light),target(target),shootingDirection(light,target) {}
		LightSource *light;
		ObjectPtr target;
		int estimate;
		DBL mintheta;
		DBL maxtheta;
		DBL dtheta;
		DBL photonSpread;
		ShootingDirection shootingDirection;

		int computeMergedFlags();
		void computeAnglesAndDeltas(ViewThreadData* renderDataPtr, shared_ptr<SceneData> sceneData);
};


class PhotonShootingUnit
{
	public:
		PhotonShootingUnit(LightSource* light, ObjectPtr target):lightAndObject(light,target) {}
		LightTargetCombo lightAndObject;
};




class SinCosOptimizations
{
	public:
		// speed optimization data - sin/cos stored in two arrays
		// these are only created if photon mapping is used
		// TODO move these to their own class
		// these are thread safe - used many times but not modified after initialization
		DBL *cosTheta;
		DBL *sinTheta;
		SinCosOptimizations();
		~SinCosOptimizations();
};

extern SinCosOptimizations sinCosData;

/* ------------------------------------------------------ */
/* global functions */
/* for documentation of these functions, see photons.c */
/* ------------------------------------------------------ */
void ChooseRay(Ray &NewRay, const VECTOR Normal, const Ray &Ray, const VECTOR Raw_Normal, int WhichRay);
int GetPhotonStat(POVMSType a);

}

#endif
