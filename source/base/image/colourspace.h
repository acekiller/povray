/*******************************************************************************
 * colourspace.h
 *
 * This file contains code for handling colour space conversions.
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
 * $File: //depot/povray/smp/source/base/image/colourspace.h $
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

#ifndef POVRAY_BASE_COLOURSPACE_H
#define POVRAY_BASE_COLOURSPACE_H

#include <vector>

#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>

#include "base/configbase.h"
#include "base/types.h"

namespace pov_base
{

class GammaCurve;
class SimpleGammaCurve;

/**
 *  Class holding a shared reference to a gamma curve.
 *  @note   This is a boost shared_ptr; the assignment operator (=) may not work as expected,
 *          as it is in fact implemented as a swap operation.
 *          To clone a reference, use "GammaCurvePtr myNewPtr(myOldPtr);".
 */
typedef boost::shared_ptr<GammaCurve> GammaCurvePtr;

/**
 *  Class holding a shared reference to a simple gamma curve.
 *  @note   This is a boost shared_ptr; the assignment operator (=) may not work as expected,
 *          as it is in fact implemented as a swap operation.
 *          To clone a reference, use "SimpleGammaCurvePtr myNewPtr(myOldPtr);".
 */
typedef boost::shared_ptr<SimpleGammaCurve> SimpleGammaCurvePtr;

/**
 *  Abstract class representing an encoding gamma curve (or, more generally, transfer function).
 *  In this generic form, the gamma curve may be arbitrarily complex.
 *  @note   To conserve memory, derived classes should prevent duplicates from being instantiated.
 *          This base class provides a caching mechanism to help accomplish this.
 */
class GammaCurve
{
	public:

		/**
		 *  Encoding function.
		 *  This function is to be implemented by subclasses to define the encoding rules of that particular gamma curve.
		 *  @note           Input values 0.0 and 1.0 should be mapped to output values 0.0 and 1.0, respectively.
		 *                  Input values that cannot be mapped should be clipped to the nearest valid value.
		 *  @param[in]  x   Input value.
		 *  @return         Output value.
		 */
		virtual float Encode(float x) const = 0;

		/**
		 *  Decoding function.
		 *  This function is to be implemented by subclasses to define the decoding rules of that particular gamma curve.
		 *  @note           Input values 0.0 and 1.0 should be mapped to output values 0.0 and 1.0, respectively.
		 *                  Input values that cannot be mapped should be clipped to the nearest valid value.
		 *  @param[in]  x   Input value.
		 *  @return         Output value.
		 */
		virtual float Decode(float x) const = 0;

		/**
		 *  Approximated power-law gamma
		 *  This function is to be implemented by subclasses to return an average overall gamma to be used as a
		 *  gamma-law approximation of the particular gamma curve.
		 */
		virtual float ApproximateDecodingGamma() const = 0;

		/**
		 *  Retrieves a lookup table for faster decoding.
		 *  This feature is intended to be used for low-dynamic-range, 8 or 16 bit depth, non-linearly encoded images
		 *  to be kept in encoded format during render to reduce the memory footprint without unnecessarily degrading
		 *  performance.
		 *  @note           The lookup table pointer is only valid during the lifetime of the GammaCurve object.
		 *                  Any entity holding a pointer to the lookup table must therefore also maintain a strong
		 *                  pointer to the GammaCurve object.
		 *  @param[in]  max The maximum encoded value; must be either 255 for 8-bit depth, or 65535 for 16-bit depth.
		 *  @return         Pointer to a table of pre-computed Decode() results for values from 0.0 to 1.0
		 *                  in increments of 1.0/max.
		 */
		float* GetLookupTable(unsigned int max);

		/**
		 *  Convenience function to test whether a gamma curve pointer refers to a neutral curve.
		 *  @param[in]  p   The gamma curve pointer to test.
		 *  @return         @c true if the gamma curve pointer is empty or the gamma curve is neutral, @c false otherwise.
		 */
		static bool IsNeutral(const GammaCurvePtr& p) { return (!p || p->IsNeutral()); }

		/**
		 *  Convenience function to apply encoding according to a given gamma curve pointer.
		 *  @note           If an empty gamma curve pointer is passed, neutral encoding is applied.
		 *  @param[in]  p   The gamma curve pointer to use for encoding.
		 *  @param[in]  x   The value to encode, typically in the range from 0.0 to 1.0.
		 *  @return         The encoded value, typically in the range from 0.0 to 1.0.
		 */
		static float Encode(const GammaCurvePtr& p, float x) { if (IsNeutral(p)) return x; else return p->Encode(x); }

		/**
		 *  Convenience function to apply encoding according to a given gamma curve pointer.
		 *  @note           If an empty gamma curve pointer is passed, neutral encoding is applied.
		 *  @param[in]  p   The gamma curve pointer to use for encoding.
		 *  @param[in]  c   The colour to encode, typically in the range from 0.0 to 1.0.
		 *  @return         The encoded colour, typically in the range from 0.0 to 1.0.
		 */
		static Colour Encode(const GammaCurvePtr& p, const Colour& c)
		{
			if (IsNeutral(p))
				return c;
			else
				return Colour(p->Encode(c.red()), p->Encode(c.green()), p->Encode(c.blue()), c.filter(), c.transm());
		}

		/**
		 *  Applies decoding according to a given gamma curve pointer.
		 *  @note           If an empty gamma curve pointer is passed, neutral decoding is applied.
		 *  @param[in]  p   The gamma curve pointer to use for decoding.
		 *  @param[in]  x   The value to decode, typically in the range from 0.0 to 1.0.
		 *  @return         The decoded value, typically in the range from 0.0 to 1.0.
		 */
		static float Decode(const GammaCurvePtr& p, float x) { if (IsNeutral(p)) return x; else return p->Decode(x); }

	protected:

		/**
		 *  Cached lookup table for 8-bit lookup.
		 *  This member variable caches the pointer returned by a first call to @c GetLookupTable(255) to avoid creating
		 *  multiple copies of the table.
		 */
		float* lookupTable8;

		/**
		 *  Cached lookup table for 16-bit lookup.
		 *  This member variable caches the pointer returned by a first call to @c GetLookupTable(65535) to avoid creating
		 *  multiple copies of the table.
		 */
		float* lookupTable16;

		/**
		 *  Mutex to guard access to @c lookupTable8 and @c lookupTable16.
		 */
		boost::mutex lutMutex;

		/**
		 *  Constructor.
		 */
		GammaCurve() : lookupTable8(NULL), lookupTable16(NULL) {}

		/**
		 *  Destructor.
		 */
		virtual ~GammaCurve() { if (lookupTable8) delete[] lookupTable8; if (lookupTable16) delete[] lookupTable16; }

		/**
		 *  Function to test whether two gamma curves match.
		 *  This function is to be implemented by subclasses to define how to test for matching gamma curves.
		 *  @param[in]  p   Pointer to the gamma curve to compare with.
		 *  @return         @c true if the gamma curve will produce the same result as this instance, @c false otherwise.
		 */
		virtual bool Matches(const GammaCurvePtr& p) const { return this == p.get(); }

		/**
		 *  Function to test whether the gamma curve is neutral.
		 *  This function is to be implemented by subclasses to define how to test for neutral gamma curves.
		 *  @return         @c true if this gamma curve is neutral (i.e. maps any value to itself), @c false otherwise.
		 */
		virtual bool IsNeutral() const { return false; }

		/**
		 *  Cache of all gamma curves currently in use.
		 *  This static member variable caches pointers of gamma curve instances currently in use, forming the basis
		 *  of the @c GetMatching() mechanism to avoid duplicate instances.
		 */
		static list<boost::weak_ptr<GammaCurve> > cache;

		/**
		 *  Mutex to guard access to @c cache.
		 */
		static boost::mutex cacheMutex;

		/**
		 *  Function to manage the gamma curve cache.
		 *  This static function will look up a gamma curve from the cache to match the supplied one, or encache the
		 *  supplied one if no match has been found.
		 *  @note           Derived classes allowing for multiple instances can pass any newly created instance
		 *                  through this function to make sure no duplicate gamma curve instances are ever in use.
		 *                  For this purpose, no references to the instance supplied shall be retained; instead,
		 *                  only the instance returned by this function shall be kept.
		 *  @param      p   Pointer to the gamma curve to look up or encache.
		 *  @return         A matching encached gamma curve (possibly, but not necessarily, the instance supplied).
		 */
		static GammaCurvePtr GetMatching(const GammaCurvePtr& p);

		friend class TranscodingGammaCurve;
};

/**
 *  Abstract class representing a simple transfer function having at most one float parameter.
 */
class SimpleGammaCurve : public GammaCurve
{
	public:

		/**
		 *  Get type identifier.
		 *  This function is to be implemented by subclasses to return the type identifier of the gamma curve subclass.
		 */
		virtual int GetTypeId() const = 0;

		/**
		 *  Get parameter.
		 *  This function is to be implemented by subclasses to return the type-specific parameter of the gamma curve subclass.
		 */
		virtual float GetParam() const = 0;

	protected:

		/**
		 *  Function to test whether two gamma curves match.
		 *  This function is to be implemented by subclasses to define how to test for matching gamma curves.
		 *  @param[in]  p   Pointer to the gamma curve to compare with.
		 *  @return         @c true if the gamma curve will produce the same result as this instance, @c false otherwise.
		 */
		virtual bool Matches(const GammaCurvePtr& p) const
		{
			SimpleGammaCurve* simpleP = dynamic_cast<SimpleGammaCurve*>(p.get());
			if (simpleP)
				return ((simpleP->GetTypeId() == this->GetTypeId()) && (simpleP->GetParam() == this->GetParam()));
			else
				return false;
		}
};

/**
 *  Abstract class representing a simple transfer function having no parameters.
 */
class UniqueGammaCurve : public SimpleGammaCurve
{
	public:
		/**
		 *  Get parameter.
		 *  This function is to be implemented by subclasses to return the type-specific parameter of the gamma curve subclass.
		 */
		virtual float GetParam() const { return 0.0; }

	protected:

		/**
		 *  Function to test whether two gamma curves match.
		 *  This function is to be implemented by subclasses to define how to test for matching gamma curves.
		 *  @param[in]  p   Pointer to the gamma curve to compare with.
		 *  @return         @c true if the gamma curve will produce the same result as this instance, @c false otherwise.
		 */
		virtual bool Matches(const GammaCurvePtr& p) const
		{
			UniqueGammaCurve* uniqueP = dynamic_cast<UniqueGammaCurve*>(p.get());
			if (uniqueP)
				return (uniqueP->GetTypeId() == this->GetTypeId());
			else
				return false;
		}
};

/**
 *  Class representing a neutral gamma curve.
 */
class NeutralGammaCurve : public UniqueGammaCurve
{
	public:
		static SimpleGammaCurvePtr Get();
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
		virtual int GetTypeId() const;
	private:
		static SimpleGammaCurvePtr instance;
		virtual bool Matches(const GammaCurvePtr&) const;
		virtual bool IsNeutral() const;
		NeutralGammaCurve();
};

/**
 *  Class representing the IEC 61966-2-1 sRGB transfer function.
 *  @note   While the sRGB transfer functionn can be approximated with a classic power-law curve
 *          having a constant gamma of 1/2.2, the two are not identical. This class represents
 *          the exact function as specified in IEC 61966-2-1.
 */
class SRGBGammaCurve : public UniqueGammaCurve
{
	public:
		static SimpleGammaCurvePtr Get();
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
		virtual int GetTypeId() const;
	private:
		static SimpleGammaCurvePtr instance;
		SRGBGammaCurve();
};

/**
 *  Class representing the ITU-R BT.709 transfer function.
 *  @note   This class does @e not account for the "black digital count" and "white digital count" being defined
 *          as 16/255 and 235/255, respectively.
 */
class ITURBT709GammaCurve : public GammaCurve // TODO we could make this a UniqueGammaCurve if we assign it a type ID
{
	public:
		static GammaCurvePtr Get();
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
	private:
		static GammaCurvePtr instance;
		ITURBT709GammaCurve();
};

/**
 *  Class representing the Rec1361 transfer function.
 *  This transfer function is a wide-gamut extension to that specified in ITU-R BT.709.
 */
class Rec1361GammaCurve : public GammaCurve // TODO we could make this a UniqueGammaCurve if we assign it a type ID
{
	public:
		static GammaCurvePtr Get();
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
	private:
		static GammaCurvePtr instance;
		Rec1361GammaCurve();
};

/**
 *  Class representing a classic constant-gamma (power-law) gamma encoding curve.
 */
class PowerLawGammaCurve : public SimpleGammaCurve
{
	public:
		static SimpleGammaCurvePtr GetByEncodingGamma(float gamma);
		static SimpleGammaCurvePtr GetByDecodingGamma(float gamma);
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
		virtual int GetTypeId() const;
		virtual float GetParam() const;
	protected:
		float encGamma;
		PowerLawGammaCurve(float encGamma);
		virtual bool Matches(const GammaCurvePtr&) const;
		static bool IsNeutral(float gamma);
};

/**
 *  Class representing a scaled-encoding variant of another gamma curves.
 */
class ScaledGammaCurve : public GammaCurve
{
	public:
		static GammaCurvePtr GetByEncoding(const GammaCurvePtr&, float encodingFactor);
		static GammaCurvePtr GetByDecoding(float decodingFactor, const GammaCurvePtr&);
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
	protected:
		GammaCurvePtr baseGamma;
		float encFactor;
		ScaledGammaCurve(const GammaCurvePtr&, float);
		virtual bool Matches(const GammaCurvePtr&) const;
		static bool IsNeutral(float factor);
};

/**
 *  Class representing a transformation between different (non-linear) "gamma spaces".
 *  @note   This class is only required for backward compatibility with POV-Ray 3.6.
 */
class TranscodingGammaCurve : public GammaCurve
{
	public:
		static GammaCurvePtr Get(const GammaCurvePtr& working, const GammaCurvePtr& encoding);
		virtual float Encode(float x) const;
		virtual float Decode(float x) const;
		virtual float ApproximateDecodingGamma() const;
	protected:
		GammaCurvePtr workGamma;
		GammaCurvePtr encGamma;
		TranscodingGammaCurve();
		TranscodingGammaCurve(const GammaCurvePtr&, const GammaCurvePtr&);
		virtual bool Matches(const GammaCurvePtr&) const;
};

/**
 *  Generic transfer function factory.
 *  @param  typeId  transfer function type (one of kPOVList_GammaType_*)
 *  @param  param   parameter for parameterized transfer function (e.g. gamma of power-law function)
 */
SimpleGammaCurvePtr GetGammaCurve(int typeId, float param);

}

#endif // POVRAY_BASE_COLOURSPACE_H
