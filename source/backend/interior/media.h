/*******************************************************************************
 * media.h
 *
 * This module contains all defines, typedefs, and prototypes for MEDIA.CPP.
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
 * $File: //depot/povray/smp/source/backend/interior/media.h $
 * $Revision: #18 $
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


#ifndef MEDIA_H
#define MEDIA_H

#include "backend/render/trace.h"

namespace pov
{

// Scattering types.
enum
{
	ISOTROPIC_SCATTERING            = 1,
	MIE_HAZY_SCATTERING             = 2,
	MIE_MURKY_SCATTERING            = 3,
	RAYLEIGH_SCATTERING             = 4,
	HENYEY_GREENSTEIN_SCATTERING    = 5,
	SCATTERING_TYPES                = 5
};

void Transform_Density(PIGMENT *Density, const TRANSFORM *Trans);

class MediaFunction : public Trace::MediaFunctor
{
	public:
		MediaFunction(TraceThreadData *td, Trace *t, PhotonGatherer *pg);

		virtual void ComputeMedia(vector<Media>& mediasource, const Ray& ray, Intersection& isect, Colour& colour, Trace::TraceTicket& ticket);
		virtual void ComputeMedia(const RayInteriorVector& mediasource, const Ray& ray, Intersection& isect, Colour& colour, Trace::TraceTicket& ticket);
		virtual void ComputeMedia(MediaVector& medias, const Ray& ray, Intersection& isect, Colour& colour, Trace::TraceTicket& ticket);
	protected:
		/// pseudo-random number sequence
		RandomDoubleSequence randomNumbers;
		/// pseudo-random number generator based on random number sequence
		RandomDoubleSequence::Generator randomNumberGenerator;
		/// thread data
		TraceThreadData *threadData;
		/// tracing functions
		Trace *trace;
		/// photon gather functions
		PhotonGatherer *photonGatherer;

		void ComputeMediaRegularSampling(MediaVector& medias, LightSourceEntryVector& lights, MediaIntervalVector& mediaintervals,
		                                 const Ray& ray, Media *IMedia, int minsamples, bool ignore_photons, bool use_scattering,
		                                 bool all_constant_and_light_ray, Trace::TraceTicket& ticket);
		void ComputeMediaAdaptiveSampling(MediaVector& medias, LightSourceEntryVector& lights, MediaIntervalVector& mediaintervals,
		                                  const Ray& ray, Media *IMedia, DBL aa_threshold, int minsamples, bool ignore_photons, bool use_scattering, Trace::TraceTicket& ticket);
		void ComputeMediaColour(MediaIntervalVector& mediaintervals, Colour& colour);
		void ComputeMediaSampleInterval(LitIntervalVector& litintervals, MediaIntervalVector& mediaintervals, Media *media);
		void ComputeMediaLightInterval(LightSourceEntryVector& lights, LitIntervalVector& litintervals, const Ray& ray, Intersection& isect);
		void ComputeOneMediaLightInterval(LightSource *light, LightSourceEntryVector&lights, const Ray& ray, Intersection& isect);
		bool ComputeSpotLightInterval(const Ray &ray, LightSource *Light, DBL *d1, DBL *d2);
		bool ComputeCylinderLightInterval(const Ray &ray, LightSource *Light, DBL *d1, DBL *d2);
		void ComputeOneMediaSample(MediaVector& medias, LightSourceEntryVector& lights, MediaInterval& mediainterval, const Ray &ray, DBL d0, RGBColour& SampCol,
		                           RGBColour& SampOptDepth, int sample_method, bool ignore_photons, bool use_scattering, bool photonPass, Trace::TraceTicket& ticket);
		void ComputeOneMediaSampleRecursive(MediaVector& medias, LightSourceEntryVector& lights, MediaInterval& mediainterval, const Ray& ray,
		                                    DBL d1, DBL d3, RGBColour& Result, const RGBColour& C1, const RGBColour& C3, RGBColour& ODResult, const RGBColour& od1, const RGBColour& od3,
		                                    int depth, DBL Jitter, DBL aa_threshold, bool ignore_photons, bool use_scattering, bool photonPass, Trace::TraceTicket& ticket);
		void ComputeMediaPhotons(MediaVector& medias, RGBColour& Te, const RGBColour& Sc, const Ray& ray, const VECTOR H);
		void ComputeMediaScatteringAttenuation(MediaVector& medias, RGBColour& OutputColor, const RGBColour& Sc, const RGBColour& Light_Colour, const Ray &ray, const Ray &Light_Ray);
};

}

#endif
