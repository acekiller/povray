/*******************************************************************************
 * trace.h
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
 * $File: //depot/povray/smp/source/backend/render/trace.h $
 * $Revision: #97 $
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

#ifndef POVRAY_BACKEND_TRACE_H
#define POVRAY_BACKEND_TRACE_H

#include <vector>

#include <boost/thread.hpp>

#include "backend/frame.h"
#include "backend/povray.h"
#include "backend/scene/atmosph.h"
#include "backend/scene/threaddata.h"
#include "backend/scene/objects.h"
#include "backend/support/bsptree.h"
#include "backend/support/randomsequences.h"
#include "povrayold.h"

namespace pov
{

class SceneData;
class ViewData;
class Task;
class PhotonGatherer;

struct NoSomethingFlagRayObjectCondition : public RayObjectCondition
{
	virtual bool operator()(const Ray& ray, const ObjectBase* object, double) const
	{
		if(ray.IsImageRay() && Test_Flag(object, NO_IMAGE_FLAG))
			return false;
		if(ray.IsReflectionRay() && Test_Flag(object, NO_REFLECTION_FLAG))
			return false;
		if(ray.IsRadiosityRay() && Test_Flag(object, NO_RADIOSITY_FLAG))
			return false;
		if(ray.IsPhotonRay() && Test_Flag(object, NO_SHADOW_FLAG))
			return false;
		return true;
	}
};

struct LitInterval
{
	bool lit;
	double s0, s1, ds;
	size_t l0, l1;

	LitInterval() :
		lit(false), s0(0.0), s1(0.0), ds(0.0), l0(0), l1(0) { }
	LitInterval(bool nlit, double ns0, double ns1, size_t nl0, size_t nl1) :
		lit(nlit), s0(ns0), s1(ns1), ds(ns1 - ns0), l0(nl0), l1(nl1) { }
};

struct MediaInterval
{
	bool lit;
	int samples;
	double s0, s1, ds;
	size_t l0, l1;
	RGBColour od;
	RGBColour te;
	RGBColour te2;

	MediaInterval() :
		lit(false), samples(0), s0(0.0), s1(0.0), ds(0.0), l0(0), l1(0) { }
	MediaInterval(bool nlit, int nsamples, double ns0, double ns1, double nds, size_t nl0, size_t nl1) :
		lit(nlit), samples(nsamples), s0(ns0), s1(ns1), ds(nds), l0(nl0), l1(nl1) { }
	MediaInterval(bool nlit, int nsamples, double ns0, double ns1, double nds, size_t nl0, size_t nl1, const RGBColour& nod, const RGBColour& nte, const RGBColour& nte2) :
		lit(nlit), samples(nsamples), s0(ns0), s1(ns1), ds(nds), l0(nl0), l1(nl1), od(nod), te(nte), te2(nte2) { }

	bool operator<(const MediaInterval& other) const { return (s0 < other.s0); }
};

struct LightSourceIntersectionEntry
{
	double s;
	size_t l;
	bool lit;

	LightSourceIntersectionEntry() :
		s(0.0), l(0), lit(false) { }
	LightSourceIntersectionEntry(double ns, size_t nl, bool nlit) :
		s(ns), l(nl), lit(nlit) { }

	bool operator<(const LightSourceIntersectionEntry& other) const { return (s < other.s); }
};

struct LightSourceEntry
{
	double s0, s1;
	LightSource *light;

	LightSourceEntry() :
		s0(0.0), s1(0.0), light(NULL) { }
	LightSourceEntry(LightSource *nlight) :
		s0(0.0), s1(0.0), light(nlight) { }
	LightSourceEntry(double ns0, double ns1, LightSource *nlight) :
		s0(ns0), s1(ns1), light(nlight) { }

	bool operator<(const LightSourceEntry& other) const { return (s0 < other.s0); }
};

// TODO: these sizes will need tweaking.
typedef FixedSimpleVector<Media *, MEDIA_VECTOR_SIZE> MediaVector; // TODO FIXME - cannot allow this to be fixed size [trf]
typedef FixedSimpleVector<MediaInterval, MEDIA_INTERVAL_VECTOR_SIZE> MediaIntervalVector; // TODO FIXME - cannot allow this to be fixed size [trf]
typedef FixedSimpleVector<LitInterval, LIT_INTERVAL_VECTOR_SIZE> LitIntervalVector; // TODO FIXME - cannot allow this to be fixed size [trf]
typedef FixedSimpleVector<LightSourceIntersectionEntry, LIGHT_INTERSECTION_VECTOR_SIZE> LightSourceIntersectionVector; // TODO FIXME - cannot allow this to be fixed size [trf]
typedef FixedSimpleVector<LightSourceEntry, LIGHTSOURCE_VECTOR_SIZE> LightSourceEntryVector; // TODO FIXME - cannot allow this to be fixed size [trf]

/**
 *  Ray tracing and shading engine.
 *  This class provides the fundamental functionality to trace rays and determine the effective colour.
 */
class Trace
{
	public:
		struct TraceTicket
		{
			/// trace recursion level
			unsigned int traceLevel;

			/// maximum trace recursion level allowed
			unsigned int maxAllowedTraceLevel;

			/// maximum trace recursion level found
			unsigned int maxFoundTraceLevel;

			/// adc bailout
			double adcBailout;

			/// whether background should be rendered all transparent
			bool alphaBackground;

			/// something the radiosity algorithm needs
			unsigned int radiosityRecursionDepth;

			/// something the radiosity algorithm needs
			float radiosityImportanceQueried;
			/// something the radiosity algorithm needs
			float radiosityImportanceFound;
			/// set by radiosity code according to the sample quality encountered (1.0 is ideal, 0.0 really sucks)
			float radiosityQuality;

			/// something the subsurface scattering algorithm needs
			unsigned int subsurfaceRecursionDepth;

			TraceTicket(unsigned int mtl, double adcb, bool ab = true, unsigned int rrd = 0, unsigned int ssrd = 0, float riq = -1.0, float rq = 1.0):
				traceLevel(0), maxAllowedTraceLevel(mtl), maxFoundTraceLevel(0), adcBailout(adcb), alphaBackground(ab), radiosityRecursionDepth(rrd), subsurfaceRecursionDepth(ssrd),
				radiosityImportanceQueried(riq), radiosityImportanceFound(-1.0), radiosityQuality(rq) {}
		};

		class CooperateFunctor
		{
			public:
				virtual void operator()() { }
		};

		class MediaFunctor
		{
			public:
				virtual void ComputeMedia(vector<Media>&, const Ray&, Intersection&, Colour&, Trace::TraceTicket& ticket) { }
				virtual void ComputeMedia(const RayInteriorVector&, const Ray&, Intersection&, Colour&, Trace::TraceTicket& ticket) { }
				virtual void ComputeMedia(MediaVector&, const Ray&, Intersection&, Colour&, Trace::TraceTicket& ticket) { }
		};

		class RadiosityFunctor
		{
			public:
				virtual void ComputeAmbient(const Vector3d& ipoint, const Vector3d& raw_normal, const Vector3d& layer_normal, RGBColour& ambient_colour, double weight, Trace::TraceTicket& ticket) { }
				virtual bool CheckRadiosityTraceLevel(const Trace::TraceTicket& ticket) { return false; }
		};

		Trace(shared_ptr<SceneData> sd, TraceThreadData *td, unsigned int qf,
		      CooperateFunctor& cf, MediaFunctor& mf, RadiosityFunctor& af);
		virtual ~Trace();

		/**
		 *  Trace a ray.
		 *
		 *  @param[in]      ray                 ray
		 *  @param[out]     colour              computed colour
		 *  @param[in]      weight              importance of this computation
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 *  @param[in]      continuedRay        set to true when tracing a ray after it went through some surface
		 *                                      without a change in direction; this governs trace level handling
		 *  @param[in]      maxDepth            objects at or beyond this distance won't be hit by the ray (ignored if < EPSILON)
		 *  @return                             the distance to the nearest object hit
		 */
		virtual double TraceRay(const Ray& ray, Colour& colour, COLC weight, TraceTicket& ticket, bool continuedRay, DBL maxDepth = 0.0);

		bool FindIntersection(Intersection& isect, const Ray& ray);
		bool FindIntersection(Intersection& isect, const Ray& ray, const RayObjectCondition& precondition, const RayObjectCondition& postcondition);
		bool FindIntersection(ObjectPtr object, Intersection& isect, const Ray& ray, double closest = HUGE_VAL);
		bool FindIntersection(ObjectPtr object, Intersection& isect, const Ray& ray, const RayObjectCondition& postcondition, double closest = HUGE_VAL);

		unsigned int GetHighestTraceLevel();

		bool TestShadow(const LightSource &light, double& depth, Ray& light_source_ray, const Vector3d& p, RGBColour& colour, TraceTicket& ticket); // TODO FIXME - this should not be exposed here

	protected: // TODO FIXME - should be private

		/// structure used to cache reflection information for multi-layered textures
		struct WNRX
		{
			double weight;
			Vector3d normal;
			RGBColour reflec;
			SNGL reflex;

			WNRX(DBL w, const Vector3d& n, const RGBColour& r, SNGL x) :
				weight(w), normal(n), reflec(r), reflex(x) { }
		};

		typedef vector<TEXTURE *> TextureVectorData;
		typedef RefPool<TextureVectorData> TextureVectorPool;
		typedef Ref<TextureVectorData, RefClearContainer<TextureVectorData> > TextureVector;

		typedef vector<WNRX> WNRXVectorData;
		typedef RefPool<WNRXVectorData> WNRXVectorPool;
		typedef Ref<WNRXVectorData, RefClearContainer<WNRXVectorData> > WNRXVector;

		/// structure used to cache shadow test results for complex textures
		struct LightColorCache
		{
			bool        tested;
			RGBColour   colour;
		};

		typedef vector<LightColorCache> LightColorCacheList;
		typedef vector<LightColorCacheList> LightColorCacheListList;

		/// List (well really vector) of lists of LightColorCaches.
		/// Each list is expected to have as many elements as there are global light sources.
		/// The number of lists should be at least that of max trace level.
		LightColorCacheListList lightColorCache;

		/// current index into lightColorCaches
		int lightColorCacheIndex;

		/// scene data
		shared_ptr<SceneData> sceneData;

		/// maximum trace recursion level found
		unsigned int maxFoundTraceLevel;
		/// adc bailout
		unsigned int qualityFlags;

		/// bounding slabs priority queue
		PriorityQueue priorityQueue;
		/// BSP tree mailbox
		BSPTree::Mailbox mailbox;
		/// area light grid buffer
		vector<RGBColour> lightGrid;
		/// fast stack pool
		IStackPool stackPool;
		/// fast texture list pool
		TextureVectorPool texturePool;
		/// fast WNRX list pool
		WNRXVectorPool wnrxPool;
		/// light source shadow cache for shadow tests of first trace level intersections
		vector<ObjectPtr> lightSourceLevel1ShadowCache;
		/// light source shadow cache for shadow tests of higher trace level intersections
		vector<ObjectPtr> lightSourceOtherShadowCache;
		/// pseudo-random number sequence
		RandomDoubleSequence randomNumbers;
		/// pseudo-random number generator based on random number sequence
		RandomDoubleSequence::Generator randomNumberGenerator;
		/// sub-random uniform 3d points on sphere sequence
		vector<SequentialVectorGeneratorPtr> ssltUniformDirectionGenerator;
		/// sub-random uniform numbers sequence
		vector<SequentialDoubleGeneratorPtr> ssltUniformNumberGenerator;
		/// sub-random cos-weighted 3d points on hemisphere sequence
		vector<SequentialVectorGeneratorPtr> ssltCosWeightedDirectionGenerator;
		/// thread data
		TraceThreadData *threadData;

		CooperateFunctor& cooperate;
		MediaFunctor& media;
		RadiosityFunctor& radiosity;

	/**
	 ***************************************************************************************************************
	 *
	 *  @name Texture Computations
	 *
	 *  The following methods compute the effective colour of a given, possibly complex, texture.
	 *
	 *  @{
	 */

		/**
		 *  Compute the effective contribution of an intersection point as seen from the ray's origin, or deposits
		 *  photons.
		 *
		 *  Computations include any media effects between the ray's origin and the point of intersection.
		 *
		 *  @remark     The computed contribution is @e added to the value passed in @c colour (does not apply to photon pass).
		 *  @see36      Determine_Apparent_Colour() in lighting.cpp
		 *  @todo       Some input parameters are non-const references.
		 *
		 *  @param[in]      isect               intersection information
		 *  @param[in,out]  colour              computed colour [in,out]; during photon pass: light colour [in]
		 *  @param[in]      ray                 ray
		 *  @param[in]      weight              importance of this computation
		 *  @param[in]      photonpass          whether to deposit photons instead of computing a colour
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 */
		void ComputeTextureColour(Intersection& isect, Colour& colour, const Ray& ray, COLC weight, bool photonpass, TraceTicket& ticket);

		/**
		 *  Compute the effective colour of an arbitrarily complex texture, or deposits photons.
		 *
		 *  @remark     The computed contribution @e overwrites any value passed in @c colour (does not apply to photon pass).
		 *  @remark     Computations do @e not include media effects between the ray's origin and the point of intersection any longer.
		 *  @see36      do_texture_map() in lighting.cpp
		 *  @todo       Some input parameters are non-const references or pointers.
		 *
		 *  @param[in,out]  resultcolour        computed colour [out]; during photon pass: light colour [in]
		 *  @param[in]      texture             texture
		 *  @param[in]      warps               stack of warps to be applied
		 *  @param[in]      ipoint              intersection point (possibly with earlier warps already applied)
		 *  @param[in]      rawnormal           geometric (possibly smoothed) surface normal
		 *  @param[in]      ray                 ray
		 *  @param[in]      weight              importance of this computation
		 *  @param[in]      isect               intersection information
		 *  @param[in]      shadowflag          whether to perform only computations necessary for shadow testing
		 *  @param[in]      photonpass          whether to deposit photons instead of computing a colour
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 */
		void ComputeOneTextureColour(Colour& resultcolour, TEXTURE *texture, vector<TEXTURE *>& warps, const Vector3d& ipoint,
		                             const Vector3d& rawnormal, const Ray& ray, COLC weight, Intersection& isect, bool shadowflag,
		                             bool photonpass, TraceTicket& ticket);

		/**
		 *  Compute the effective colour of an averaged texture, or deposits photons.
		 *
		 *  @remark     The computed contribution @e overwrites any value passed in @c colour (does not apply to photon pass).
		 *  @remark     Computations do @e not include media effects between the ray's origin and the point of intersection
		 *              any longer.
		 *  @todo       Some input parameters are non-const references or pointers.
		 *
		 *  @param[in,out]  resultcolour        computed colour [out]; during photon pass: light colour [in]
		 *  @param[in]      texture             texture
		 *  @param[in]      warps               stack of warps to be applied
		 *  @param[in]      ipoint              intersection point (possibly with earlier warps already applied)
		 *  @param[in]      rawnormal           geometric (possibly smoothed) surface normal
		 *  @param[in]      ray                 ray
		 *  @param[in]      weight              importance of this computation
		 *  @param[in]      isect               intersection information
		 *  @param[in]      shadowflag          whether to perform only computations necessary for shadow testing
		 *  @param[in]      photonpass          whether to deposit photons instead of computing a colour
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 */
		void ComputeAverageTextureColours(Colour& resultcolour, TEXTURE *texture, vector<TEXTURE *>& warps, const Vector3d& ipoint,
		                                  const Vector3d& rawnormal, const Ray& ray, COLC weight, Intersection& isect, bool shadowflag,
		                                  bool photonpass, TraceTicket& ticket);

		/**
		 *  Compute the effective colour of a simple or layered texture.
		 *
		 *  Computations include secondary rays, as well as any media effects between the ray's origin and the point of intersection.
		 *
		 *  @remark     The computed contribution @e overwrites any value passed in @c colour.
		 *  @remark     Computations do @e not include media effects between the ray's origin and the point of intersection any longer.
		 *  @remark     pov::PhotonTrace overrides this method to deposit photons instead.
		 *  @see36      compute_lighted_texture()
		 *  @todo       Some input parameters are non-const references or pointers.
		 *
		 *  @param[in,out]  resultcolour        computed colour [out]; during photon pass: light colour [in]
		 *  @param[in]      texture             texture
		 *  @param[in]      warps               stack of warps to be applied
		 *  @param[in]      ipoint              intersection point (possibly with earlier warps already applied)
		 *  @param[in]      rawnormal           geometric (possibly smoothed) surface normal
		 *  @param[in]      ray                 ray
		 *  @param[in]      weight              importance of this computation
		 *  @param[in]      isect               intersection information
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 */
		virtual void ComputeLightedTexture(Colour& resultcolour, TEXTURE *texture, vector<TEXTURE *>& warps, const Vector3d& ipoint,
		                                   const Vector3d& rawnormal, const Ray& ray, COLC weight, Intersection& isect, TraceTicket& ticket);


		/**
		 *  Compute the effective filtering effect of a simple or layered texture.
		 *
		 *  @remark     The computed contribution @e overwrites any value passed in @c colour.
		 *  @remark     Computations do @e not include media effects between the ray's origin and the point of intersection any longer.
		 *  @todo       Some input parameters are non-const references or pointers.
		 *
		 *  @param[in,out]  filtercolour        computed filter colour [out]; during photon pass: light colour [in]
		 *  @param[in]      texture             texture
		 *  @param[in]      warps               stack of warps to be applied
		 *  @param[in]      ipoint              intersection point (possibly with earlier warps already applied)
		 *  @param[in]      rawnormal           geometric (possibly smoothed) surface normal
		 *  @param[in]      ray                 ray
		 *  @param[in]      isect               intersection information
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 */
		void ComputeShadowTexture(Colour& filtercolour, TEXTURE *texture, vector<TEXTURE *>& warps, const Vector3d& ipoint,
		                          const Vector3d& rawnormal, const Ray& ray, Intersection& isect, TraceTicket& ticket);

	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 *
	 *  @name Reflection and Refraction Computations
	 *
	 *  The following methods compute the contribution of secondary (reflected and refracted) rays.
	 *
	 *  @{
	 */

		void ComputeReflection(const FINISH* finish, const Vector3d& ipoint, const Ray& ray, const Vector3d& normal, const Vector3d& rawnormal, Colour& colour, COLC weight, TraceTicket& ticket);
		bool ComputeRefraction(const FINISH* finish, Interior *interior, const Vector3d& ipoint, const Ray& ray, const Vector3d& normal, const Vector3d& rawnormal, Colour& colour, COLC weight, TraceTicket& ticket);
		bool TraceRefractionRay(const FINISH* finish, const Vector3d& ipoint, const Ray& ray, Ray& nray, double ior, double n, const Vector3d& normal, const Vector3d& rawnormal, const Vector3d& localnormal, Colour& colour, COLC weight, TraceTicket& ticket);

	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 *
	 *  @name Classic Light Source Computations
	 *
	 *  The following methods compute the (additional) contribution of classic lighting.
	 *
	 *  @{
	 */

		/// @todo The name is misleading, as it computes all contributions of classic lighting, including highlights.
		void ComputeDiffuseLight(const FINISH *finish, const Vector3d& ipoint, const  Ray& eye, const Vector3d& layer_normal, const RGBColour& layer_pigment_colour,
		                         RGBColour& colour, double attenuation, ObjectPtr object, TraceTicket& ticket);
		/// @todo The name is misleading, as it computes all contributions of classic lighting, including highlights.
		void ComputeOneDiffuseLight(const LightSource &lightsource, const Vector3d& reye, const FINISH *finish, const Vector3d& ipoint, const Ray& eye,
		                            const Vector3d& layer_normal, const RGBColour& Layer_Pigment_Colour, RGBColour& colour, double Attenuation, ObjectPtr Object, TraceTicket& ticket, int light_index = -1);
		/// @todo The name is misleading, as it computes all contributions of classic lighting, including highlights.
		void ComputeFullAreaDiffuseLight(const LightSource &lightsource, const Vector3d& reye, const FINISH *finish, const Vector3d& ipoint, const Ray& eye,
		                                 const Vector3d& layer_normal, const RGBColour& layer_pigment_colour, RGBColour& colour, double attenuation,
		                                 double lightsourcedepth, Ray& lightsourceray, const RGBColour& lightcolour,
		                                 bool isDoubleIlluminated); // JN2007: Full area lighting

		/**
		 *  Compute the direction, distance and unshadowed brightness of an unshadowed light source.
		 *
		 *  Computations include spotlight falloff and distance-based attenuation.
		 *
		 *  @param[in]      lightsource         light source
		 *  @param[out]     lightsourcedepth    distance to the light source
		 *  @param[in,out]  lightsourceray      ray to the light source
		 *  @param[in]      ipoint              intersection point
		 *  @param[out]     lightcolour         effective brightness
		 */
		void ComputeOneLightRay(const LightSource &lightsource, double& lightsourcedepth, Ray& lightsourceray, const Vector3d& ipoint, RGBColour& lightcolour);

		void TraceShadowRay(const LightSource &light, double depth, Ray& lightsourceray, const Vector3d& point, RGBColour& colour, TraceTicket& ticket);
		void TracePointLightShadowRay(const LightSource &lightsource, double& lightsourcedepth, Ray& lightsourceray, RGBColour& lightcolour, TraceTicket& ticket);
		void TraceAreaLightShadowRay(const LightSource &lightsource, double& lightsourcedepth, Ray& lightsourceray,
		                             const Vector3d& ipoint, RGBColour& lightcolour, TraceTicket& ticket);
		void TraceAreaLightSubsetShadowRay(const LightSource &lightsource, double& lightsourcedepth, Ray& lightsourceray,
		                                   const Vector3d& ipoint, RGBColour& lightcolour, int u1, int  v1, int  u2, int  v2, int level, const Vector3d& axis1, const Vector3d& axis2,
		                                   TraceTicket& ticket);

		/**
		 *  Compute the filtering effect of an object on incident light from a particular light source.
		 *
		 *  Computations include any media effects between the ray's origin and the point of intersection.
		 *
		 *  @todo       Some input parameters are non-const references.
		 *
		 *  @param[in]      lightsource         light source
		 *  @param[in]      isect               intersection information
		 *  @param[in,out]  lightsourceray      ray to the light source
		 *  @param[in,out]  colour              computed effect on the incident light
		 *  @param[in,out]  ticket              additional information passed through to/from secondary rays
		 */
		void ComputeShadowColour(const LightSource &lightsource, Intersection& isect, Ray& lightsourceray, RGBColour& colour, TraceTicket& ticket);

		/**
		 *  Compute the direction and distance of a single light source to a given intersection point.
		 *
		 *  @remark     The @c Origin and @c Direction member of @c lightsourceray are updated, all other members are left unchanged;
		 *              the distance is returned in a separate parameter. For cylindrical light sources, the values are set accordingly.
		 *  @todo       The name is misleading, as it just computes direction and distance.
		 *
		 *  @param[in]      lightsource         light source
		 *  @param[out]     lightsourcedepth    distance to the light source
		 *  @param[in,out]  lightsourceray      ray to the light source
		 *  @param[in]      ipoint              intersection point
		 *  @param[in]      jitter              jitter to apply to the light source
		 */
		void ComputeOneWhiteLightRay(const LightSource &lightsource, double& lightsourcedepth, Ray& lightsourceray, const Vector3d& ipoint, const Vector3d& jitter = Vector3d());

	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 *
	 *  @name Photon Light Source Computations
	 *
	 *  The following methods compute the (additional) contribution of photon-based lighting.
	 *
	 *  @{
	 */

		/// @todo The name is misleading, as it computes all contributions of classic lighting, including highlights.
		void ComputePhotonDiffuseLight(const FINISH *Finish, const Vector3d& IPoint, const Ray& Eye, const Vector3d& Layer_Normal, const Vector3d& Raw_Normal,
		                               const RGBColour& Layer_Pigment_Colour, RGBColour& colour, double Attenuation,
		                               ObjectPtr Object, PhotonGatherer& renderer);

	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 *
	 *  @name Material Finish Computations
	 *
	 *  The following methods compute the contribution of a finish illuminated by light from a given direction.
	 *
	 *  @{
	 */

		/**
		 *  Compute the diffuse contribution of a finish illuminated by light from a given direction.
		 *
		 *  @remark     The computed contribution is @e added to the value passed in @c colour.
		 *
		 *  @param[in]      finish              finish
		 *  @param[in]      lightsourceray      ray from intersection to light source
		 *  @param[in]      layer_normal        effective (possibly pertubed) surface normal
		 *  @param[in,out]  colour              effective surface colour
		 *  @param[in]      light_colour        effective light colour
		 *  @param[in]      layer_pigment_colour  nominal pigment colour
		 *  @param[in]      attenuation         attenuation factor to account for partial transparency
		 *  @param[in]      backside            whether to use backside instead of frontside diffuse brightness factor
		 */
		void ComputeDiffuseColour(const FINISH *finish, const Ray& lightsourceray, const Vector3d& layer_normal, RGBColour& colour,
		                          const RGBColour& light_colour, const RGBColour& layer_pigment_colour, double attenuation, bool backside);

		/**
		 *  Compute the iridescence contribution of a finish illuminated by light from a given direction.
		 *
		 *  @remark     The computed contribution is @e added to the value passed in @c colour.
		 *  @bug        The math behind this is utterly bogus.
		 *
		 *  @param[in]      finish              finish
		 *  @param[in]      lightsourceray      ray from intersection to light source
		 *  @param[in]      layer_normal        effective (possibly pertubed) surface normal
		 *  @param[in]      ipoint              intersection point (possibly with earlier warps already applied)
		 *  @param[in,out]  colour              effective surface colour
		 */
		void ComputeIridColour(const FINISH *finish, const Vector3d& lightsource, const Vector3d& eye, const Vector3d& layer_normal, const Vector3d& ipoint, RGBColour& colour);

		/**
		 *  Compute the Phong highlight contribution of a finish illuminated by light from a given direction.
		 *
		 *  Computation uses the classic Phong highlight model
		 *
		 *  @remark     The model used is @e not energy-conserving
		 *  @remark     The computed contribution is @e added to the value passed in @c colour.
		 *
		 *  @param[in]      finish              finish
		 *  @param[in]      lightsourceray      ray from intersection to light source
		 *  @param[in]      layer_normal        effective (possibly pertubed) surface normal
		 *  @param[in,out]  colour              effective surface colour
		 *  @param[in]      light_colour        effective light colour
		 *  @param[in]      layer_pigment_colour  nominal pigment colour
		 */
		void ComputePhongColour(const FINISH *finish, const Ray& lightsourceray, const Vector3d& eye, const Vector3d& layer_normal, RGBColour& colour,
		                        const RGBColour& light_colour, const RGBColour& layer_pigment_colour);

		/**
		 *  Compute the specular highlight contribution of a finish illuminated by light from a given direction.
		 *
		 *  Computation uses the Blinn-Phong highlight model
		 *
		 *  @remark     The model used is @e not energy-conserving
		 *  @remark     The computed contribution is @e added to the value passed in @c colour.
		 *
		 *  @param[in]      finish              finish
		 *  @param[in]      lightsourceray      ray from intersection to light source
		 *  @param[in]      eye                 vector from intersection to observer
		 *  @param[in]      layer_normal        effective (possibly pertubed) surface normal
		 *  @param[in,out]  colour              effective surface colour
		 *  @param[in]      light_colour        effective light colour
		 *  @param[in]      layer_pigment_colour  nominal pigment colour
		 */
		void ComputeSpecularColour(const FINISH *finish, const Ray& lightsourceray, const Vector3d& eye, const Vector3d& layer_normal, RGBColour& colour,
		                           const RGBColour& light_colour, const RGBColour& layer_pigment_colour);

	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 */

		void ComputeRelativeIOR(const Ray& ray, const Interior* interior, double& ior);

		/**
		 *  Compute Reflectivity.
		 *
		 *  @remark     In Fresnel mode, light is presumed to be unpolarized on average, using
		 *              @f$ R = \frac{1}{2} \left( R_s + R_p \right) @f$.
		 */
		void ComputeReflectivity(double& weight, RGBColour& reflectivity, const RGBColour& reflection_max, const RGBColour& reflection_min,
		                         int reflection_type, double reflection_falloff, double cos_angle, const Ray& ray, Interior *interior);

		void ComputeSky(const Ray& ray, Colour& colour, TraceTicket& ticket);
		void ComputeFog(const Ray& ray, Intersection& isect, Colour& colour);
		double ComputeConstantFogColour(const Ray &ray, double depth, double width, FOG *fog, Colour& colour);
		double ComputeGroundFogColour(const Ray& ray, double depth, double width, FOG *fog, Colour& colour);
		void ComputeRainbow(const Ray& ray, Intersection& isect, Colour& colour);

		/**
		 *  Compute media effect on traversing light rays.
		 *
		 *  @note       This computes two things:
		 *                  - media and fog attenuation of the shadow ray (optional)
		 *                  - entry/exit of interiors
		 *                  .
		 *              In other words, you can't skip this whole thing, because the entry/exit is important.
		 */
		void ComputeShadowMedia(Ray& light_source_ray, Intersection& isect, RGBColour& resultcolour, bool media_attenuation_and_interaction, TraceTicket& ticket);

		/**
		 *  Test whether an object is part of (or identical to) a given other object.
		 *
		 *  @todo       The name is misleading, as the object to test against (@c parent) does not necessarily have
		 *              to be a CSG compound object, but can actually be of any type. In that case, the function
		 *              serves to test for identity.
		 *
		 *  @param[in]      object              the object to test
		 *  @param[in]      parent              the object to test against
		 *  @return                             true if @c object is part of, or identical to, @c parent
		 */
		bool IsObjectInCSG(const ObjectBase *object, const ObjectBase *parent);


	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 *
	 *  @name Subsurface Light Transport
	 *
	 *  The following methods implement the BSSRDF approximation as outlined by Jensen et al.
	 *
	 *  @{
	 */

		double ComputeFt(double phi, double eta);
		void ComputeSurfaceTangents(const Vector3d& normal, Vector3d& u, Vector3d& v);
		void ComputeSSLTNormal (Intersection& Ray_Intersection);
		bool IsSameSSLTObject(const ObjectBase* obj1, const ObjectBase* obj2);
		void ComputeDiffuseSampleBase(Vector3d& basePoint, const Intersection& out, const Vector3d& vOut, double avgFreeDist);
		void ComputeDiffuseSamplePoint(ObjectPtr obj, const Vector3d& basePoint, Intersection& in, double& sampleArea, TraceTicket& ticket);
		void ComputeDiffuseContribution(const Intersection& out, const Vector3d& vOut, const Vector3d& pIn, const Vector3d& nIn, const Vector3d& vIn, double& sd, double sigma_prime_s, double sigma_a, double eta);
		void ComputeDiffuseContribution1(const LightSource& lightsource, const Intersection& out, const Vector3d& vOut, const Intersection& in, RGBColour& Total_Colour, const DblRGBColour& sigma_prime_s, const DblRGBColour& sigma_a, double eta, double weight, TraceTicket& ticket);
		void ComputeDiffuseAmbientContribution1(const Intersection& out, const Vector3d& vOut, const Intersection& in, RGBColour& Total_Colour, const DblRGBColour& sigma_prime_s, const DblRGBColour& sigma_a, double eta, double weight, TraceTicket& ticket);
		void ComputeOneSingleScatteringContribution(const LightSource& lightsource, const Intersection& out, double sigma_t_xo, double sigma_s, double s_prime_out, RGBColour& Lo, double eta, const Vector3d& bend_point, double phi_out, double cos_out_prime, TraceTicket& ticket);
		void ComputeSingleScatteringContribution(const Intersection& out, double dist, double cos_out, const Vector3d& refractedREye, double sigma_prime_t, double sigma_prime_s, RGBColour& Lo, double eta, TraceTicket& ticket);
		void ComputeSubsurfaceScattering (const FINISH *Finish, const RGBColour& layer_pigment_colour, const Intersection& isect, const Ray& Eye, const Vector3d& Layer_Normal, RGBColour& Colour, double Attenuation, TraceTicket& ticket);
		bool SSLTComputeRefractedDirection(const Vector3d& v, const Vector3d& n, double eta, Vector3d& refracted);

	/**
	 *  @}
	 *
	 ***************************************************************************************************************
	 */

};

}

#endif // POVRAY_BACKEND_TRACE_H
