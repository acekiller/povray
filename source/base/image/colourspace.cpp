/*******************************************************************************
 * colourspace.cpp
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
 * $File: //depot/povray/smp/source/base/image/colourspace.cpp $
 * $Revision: #5 $
 * $Change: 5303 $
 * $DateTime: 2010/12/27 14:22:56 $
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

#include <vector>
#include <algorithm>
#include <cassert>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/colourspace.h"
#include "base/image/encoding.h"
#include "base/povmsgid.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

// definitions of static GammaCurve member variables to satisfy the linker
list<boost::weak_ptr<GammaCurve> > GammaCurve::cache;
boost::mutex GammaCurve::cacheMutex;

// definitions of static GammaCurve-derivatives' member variables to satisfy the linker
SimpleGammaCurvePtr NeutralGammaCurve::instance;
SimpleGammaCurvePtr SRGBGammaCurve::instance;
GammaCurvePtr ITURBT709GammaCurve::instance;
GammaCurvePtr Rec1361GammaCurve::instance;

/*******************************************************************************/

float* GammaCurve::GetLookupTable(unsigned int max)
{
	assert(max == 255 || max == 65535); // shouldn't happen, but it won't hurt to check in debug versions

	// Get a reference to the lookup table pointer we're dealing with, so we don't need to duplicate all the remaining code.
	float*& lookupTable = (max == 255 ? lookupTable8 : lookupTable16);

	// Make sure we're not racing any other thread that might currently be busy creating the LUT.
	boost::mutex::scoped_lock lock(lutMutex);

	// Create the LUT if it doesn't exist yet.
	if (!lookupTable)
	{
		float* tempTable = new float[max+1];
		for (unsigned int i = 0; i <= max; i ++)
			tempTable[i] = Decode(IntDecode(i, max));

		// hook up the table only as soon as it is completed, so that querying the table does not need to
		// care about thread-safety.
		lookupTable = tempTable;
	}

	return lookupTable;
}

GammaCurvePtr GammaCurve::GetMatching(const GammaCurvePtr& newInstance)
{
	GammaCurvePtr oldInstance;
	bool cached = false;

	// See if we have a matching gamma curve in our chache already

	// make sure the cache doesn't get tampered with while we're working on it
	boost::mutex::scoped_lock lock(cacheMutex);

	// Check if we already have created a matching gamma curve object; if so, return that object instead.
	// Also, make sure we get the new object stored (as we're using weak pointers, we may have stale entries;
	// it also won't hurt if we store the new instance, even if we decide to discard it)
	for(list<boost::weak_ptr<GammaCurve> >::iterator i(cache.begin()); i != cache.end(); i++)
	{
		oldInstance = (*i).lock();
		if (!oldInstance)
		{
			// Found a stale entry in the cache where we could store the new instance, in case we don't find any match.
			// As the cache uses weak pointers, we can just as well store the new instance now right away,
			// and leave it up to the weak pointer mechanism to clean up in case we find an existing instance.
			if (!cached)
				(*i) = newInstance;
			cached = true;
		}
		else if (oldInstance->Matches(newInstance))
		{
			// Found a matching curve in the cache, so use that instead, and (as far as we're concerned)
			// just forget that the new instance ever existed (allowing the shared_ptr mechanism to garbage-collect it)
			return oldInstance;
		}
	}

	// No matching gamma curve in the cache yet

	// Store the new entry in the cache if we haven't done so already.
	if (!cached)
		cache.push_back(newInstance);

	return newInstance;
}

/*******************************************************************************/

NeutralGammaCurve::NeutralGammaCurve() {}
SimpleGammaCurvePtr NeutralGammaCurve::Get()
{
	if (!instance)
		instance.reset(new NeutralGammaCurve());
	return SimpleGammaCurvePtr(instance);
}
float NeutralGammaCurve::Encode(float x) const
{
	return x;
}
float NeutralGammaCurve::Decode(float x) const
{
	return x;
}
float NeutralGammaCurve::ApproximateDecodingGamma() const
{
	return 1.0f;
}
int NeutralGammaCurve::GetTypeId() const
{
	return kPOVList_GammaType_Neutral;
}
bool NeutralGammaCurve::Matches(const GammaCurvePtr& p) const
{
	return GammaCurve::IsNeutral(p);
}
bool NeutralGammaCurve::IsNeutral() const
{
	return true;
}

/*******************************************************************************/

SRGBGammaCurve::SRGBGammaCurve() {}
SimpleGammaCurvePtr SRGBGammaCurve::Get()
{
	if (!instance)
		instance.reset(new SRGBGammaCurve());
	return SimpleGammaCurvePtr(instance);
}
float SRGBGammaCurve::Encode(float x) const
{
	// (the threshold of 0.00304 occasionally found on the net was from an older draft)
	if (x <= 0.0031308f) return x * 12.92f;
	else                 return 1.055f * pow(x, 1.0f/2.4f) - 0.055f;
}
float SRGBGammaCurve::Decode(float x) const
{
	// (the threshold of 0.03928 occasionally found on the net was from an older draft)
	if (x < 0.04045f) return x / 12.92f;
	else              return pow((x + 0.055f) / 1.055f, 2.4f);
}
float SRGBGammaCurve::ApproximateDecodingGamma() const
{
	return 2.2f;
}
int SRGBGammaCurve::GetTypeId() const
{
	return kPOVList_GammaType_SRGB;
}

/*******************************************************************************/

ITURBT709GammaCurve::ITURBT709GammaCurve() {}
GammaCurvePtr ITURBT709GammaCurve::Get()
{
	if (!instance)
		instance.reset(new ITURBT709GammaCurve());
	return GammaCurvePtr(instance);
}
float ITURBT709GammaCurve::Encode(float x) const
{
	if (x < 0.018f) return x * 4.5f;
	else            return 1.099f * pow(x, 0.45f) - 0.099f;
}
float ITURBT709GammaCurve::Decode(float x) const
{
	if (x < 0.081f) return x / 4.5f;
	else            return pow((x + 0.099f) / 1.099f, 1.0f/0.45f);
}
float ITURBT709GammaCurve::ApproximateDecodingGamma() const
{
	return 1.9f; // very rough approximation
}

/*******************************************************************************/

Rec1361GammaCurve::Rec1361GammaCurve() {}
GammaCurvePtr Rec1361GammaCurve::Get()
{
	if (!instance)
		instance.reset(new Rec1361GammaCurve());
	return GammaCurvePtr(instance);
}
float Rec1361GammaCurve::Encode(float x) const
{
	if      (x < -0.0045f) return (1.099f * pow(-4*x, 0.45f) - 0.099f) / 4;
	else if (x <  0.018f)  return x * 4.5f;
	else                   return 1.099f * pow(x,0.45f) - 0.099f;
}
float Rec1361GammaCurve::Decode(float x) const
{
	if      (x < -0.02025f) return pow((4*x + 0.099f) / 1.099f, 1.0f/0.45f) / -4;
	else if (x <  0.081f)   return x / 4.5f;
	else                    return pow((x + 0.099f) / 1.099f, 1.0f/0.45f);
}
float Rec1361GammaCurve::ApproximateDecodingGamma() const
{
	return 1.9f; // very rough approximation of the x>0 section
}

/*******************************************************************************/

PowerLawGammaCurve::PowerLawGammaCurve(float gamma) :
	encGamma(gamma)
{}
SimpleGammaCurvePtr PowerLawGammaCurve::GetByEncodingGamma(float gamma)
{
	if (IsNeutral(gamma))
		return NeutralGammaCurve::Get();
	return boost::dynamic_pointer_cast<SimpleGammaCurve,GammaCurve>(GetMatching(GammaCurvePtr(new PowerLawGammaCurve(gamma))));
}
SimpleGammaCurvePtr PowerLawGammaCurve::GetByDecodingGamma(float gamma)
{
	return GetByEncodingGamma(1.0f/gamma);
}
float PowerLawGammaCurve::Encode(float x) const
{
	return pow(max(x,0.0f), encGamma);
}
float PowerLawGammaCurve::Decode(float x) const
{
	return pow(max(x,0.0f), 1.0f/encGamma);
}
float PowerLawGammaCurve::ApproximateDecodingGamma() const
{
	return 1.0f/encGamma;
}
int PowerLawGammaCurve::GetTypeId() const
{
	return kPOVList_GammaType_PowerLaw;
}
float PowerLawGammaCurve::GetParam() const
{
	return 1.0f/encGamma;
}
bool PowerLawGammaCurve::Matches(const GammaCurvePtr& p) const
{
	PowerLawGammaCurve* other = dynamic_cast<PowerLawGammaCurve*>(p.get());
	if (!other) return false;
	return IsNeutral(this->encGamma / other->encGamma);
}
bool PowerLawGammaCurve::IsNeutral(float gamma)
{
	return fabs(1.0 - gamma) <= 0.01;
}

/*******************************************************************************/

ScaledGammaCurve::ScaledGammaCurve(const GammaCurvePtr& gamma, float factor) :
	baseGamma(gamma), encFactor(factor)
{
	ScaledGammaCurve* other = dynamic_cast<ScaledGammaCurve*>(baseGamma.get());
	if (other) // if base gamma curve is a scaled one as well, compute a combined scaling factor instead of nesting
	{
		baseGamma = other->baseGamma;
		encFactor *= other->encFactor;
	}
}
GammaCurvePtr ScaledGammaCurve::GetByEncoding(const GammaCurvePtr& gamma, float factor)
{
	if (IsNeutral(factor))
		return GammaCurvePtr(gamma);
	return GetMatching(GammaCurvePtr(new ScaledGammaCurve(
		GammaCurve::IsNeutral(gamma) ? NeutralGammaCurve::Get() : GammaCurvePtr(gamma),
		factor)));
}
GammaCurvePtr ScaledGammaCurve::GetByDecoding(float factor, const GammaCurvePtr& gamma)
{
	return GetByEncoding(gamma, 1.0f/factor);
}
float ScaledGammaCurve::Encode(float x) const
{
	return baseGamma->Encode(x) * encFactor;
}
float ScaledGammaCurve::Decode(float x) const
{
	return baseGamma->Decode(x / encFactor);
}
float ScaledGammaCurve::ApproximateDecodingGamma() const
{
	return baseGamma->ApproximateDecodingGamma();
}
bool ScaledGammaCurve::Matches(const GammaCurvePtr& p) const
{
	ScaledGammaCurve* other = dynamic_cast<ScaledGammaCurve*>(p.get());
	if (!other) return false;
	return (this->baseGamma == other->baseGamma) && IsNeutral(this->encFactor / other->encFactor);
}
bool ScaledGammaCurve::IsNeutral(float scale) { return fabs(1.0 - scale) <= 1e-6; }

/*******************************************************************************/

TranscodingGammaCurve::TranscodingGammaCurve(const GammaCurvePtr& working, const GammaCurvePtr& encoding) :
	workGamma(working), encGamma(encoding)
{}
GammaCurvePtr TranscodingGammaCurve::Get(const GammaCurvePtr& working, const GammaCurvePtr& encoding)
{
	// if the working gamma space is linear, we only need the encoding gamma
	if (GammaCurve::IsNeutral(working))
		return GammaCurvePtr(encoding);
	// if both gamma spaces are the same, we can replace them with a neutral gamma curve
	if (working->Matches(encoding))
		return NeutralGammaCurve::Get();
	// check if we can replace the combination of gamma curves with a single power-law gamma curve
	PowerLawGammaCurve* powerLawWork = dynamic_cast<PowerLawGammaCurve*>(working.get());
	if (powerLawWork)
	{
		// if the encoding gamma space is linear, we only need the inverse of the working gamma
		if (GammaCurve::IsNeutral(encoding))
			return PowerLawGammaCurve::GetByEncodingGamma(powerLawWork->ApproximateDecodingGamma());
		// if both gamma spaces are based on a simple power-law, we only need to combine them into a single one
		PowerLawGammaCurve* powerLawEnc  = dynamic_cast<PowerLawGammaCurve*>(encoding.get());
		if (powerLawEnc)
			return PowerLawGammaCurve::GetByEncodingGamma(powerLawWork->ApproximateDecodingGamma() / powerLawEnc->ApproximateDecodingGamma());
	}
	// we really need a combo of two gamma curves
	return GetMatching(GammaCurvePtr(new TranscodingGammaCurve(working, encoding ? encoding : NeutralGammaCurve::Get())));
}
float TranscodingGammaCurve::Encode(float x) const
{
	return encGamma->Encode(workGamma->Decode(x));
}
float TranscodingGammaCurve::Decode(float x) const
{
	return workGamma->Encode(encGamma->Decode(x));
}
float TranscodingGammaCurve::ApproximateDecodingGamma() const
{
	return encGamma->ApproximateDecodingGamma() / workGamma->ApproximateDecodingGamma();
}
bool TranscodingGammaCurve::Matches(const GammaCurvePtr& p) const
{
	TranscodingGammaCurve* other = dynamic_cast<TranscodingGammaCurve*>(p.get());
	if (!other) return false;
	return (this->encGamma->Matches(other->encGamma) && this->workGamma->Matches(other->workGamma));
}

/*******************************************************************************/

SimpleGammaCurvePtr GetGammaCurve(int type, float param)
{
	switch (type)
	{
		case kPOVList_GammaType_Neutral:    return NeutralGammaCurve::Get();
		case kPOVList_GammaType_PowerLaw:   return PowerLawGammaCurve::GetByDecodingGamma(param);
		case kPOVList_GammaType_SRGB:       return SRGBGammaCurve::Get();
		default:                            return PowerLawGammaCurve::GetByDecodingGamma(DEFAULT_FILE_GAMMA);
	}
}

}
