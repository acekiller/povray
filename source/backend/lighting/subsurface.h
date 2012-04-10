/*******************************************************************************
 * subsurface.h
 *
 * This module contains all defines, typedefs, and prototypes for subsurface.cpp.
 *
 * Author: Christoph Lipka
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
 * $File: //depot/povray/smp/source/backend/lighting/subsurface.h $
 * $Revision: #2 $
 * $Change: 5409 $
 * $DateTime: 2011/02/22 14:58:18 $
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

#ifndef SUBSURFACE_H
#define SUBSURFACE_H

#include <boost/version.hpp>
#if BOOST_VERSION >= 103800 // flyweight is unavailable prior to boost 1.38
#include <boost/flyweight.hpp>
#include <boost/flyweight/key_value.hpp>
#endif // BOOST_VERSION

namespace pov
{

//using namespace pov_base;
#if BOOST_VERSION >= 103800 // flyweight is unavailable prior to boost 1.38
using boost::flyweights::flyweight;
using boost::flyweights::key_value;
#endif // BOOST_VERSION

/// Class storing SSLT data precomputed based on index of refraction.
class SubsurfaceInterior {

	public:

		SubsurfaceInterior(double ior);
		DblRGBColour GetReducedAlbedo(const RGBColour& diffuseReflectance);

	protected:

		static const int ReducedAlbedoSamples = 100;

		// precomputed reduced albedo for selected values of diffuse reflectance
		struct PrecomputedReducedAlbedo {
			float reducedAlbedo[ReducedAlbedoSamples+1];
			PrecomputedReducedAlbedo(double ior);
			double operator()(double diffuseReflectance) const;
		};

#if BOOST_VERSION >= 103800 // flyweight is unavailable prior to boost 1.38
		flyweight<key_value<double,PrecomputedReducedAlbedo> > precomputedReducedAlbedo;
#else // BOOST_VERSION
		// TODO - if flyweight is unavailable, we're currently resorting to a simple but memory-hungry fallback solution
		PrecomputedReducedAlbedo precomputedReducedAlbedo;
#endif // BOOST_VERSION
};

/// Approximation to the Fresnel diffuse reflectance.
inline double FresnelDiffuseReflectance(double eta)
{
#if 0
	// This is the original formula as per the 2001 Jensen et al. paper;
	// however, this breaks down for large values of eta, or values < 1.0,
	// and comes with some other bogosities.
	return clip( -1.440/Sqr(eta) + 0.710/eta + 0.668 + 0.0636*eta, 0.0, 1.0-EPSILON );
#else
	// My own approximation; maybe it's utterly wrong, but at least it is stable.
	if (eta < 1.0)
		return Sqr(eta)-pow(eta,2.25);
	else
		return ( (1.0-1.0/eta) + 3*pow(1.0-1.0/eta,4.5) ) / 4.0;
#endif
}

} // end of namespace

#endif // SUBSURFACE_H
