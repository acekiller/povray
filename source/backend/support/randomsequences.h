/*******************************************************************************
 * randomsequences.h
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
 * $File: //depot/povray/smp/source/backend/support/randomsequences.h $
 * $Revision: #14 $
 * $Change: 5408 $
 * $DateTime: 2011/02/21 15:17:08 $
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

#ifndef POVRAY_BACKEND_RANDOMSEQUENCES_H
#define POVRAY_BACKEND_RANDOMSEQUENCES_H

#include <vector>
#include <cctype>

#include "base/configbase.h"
#include "base/timer.h"
#include "base/pov_err.h"

namespace pov
{

using namespace pov_base;

vector<int> RandomInts(int minval, int maxval, size_t count);
vector<double> RandomDoubles(int minval, int maxval, size_t count);

// need this to prevent VC++ v8 from thinking that Generator refers to boost::Generator
class Generator;

class RandomIntSequence
{
		friend class Generator;
	public:
		RandomIntSequence(int minval, int maxval, size_t count);

		int operator()(size_t seedindex);

		class Generator
		{
			public:
				Generator(RandomIntSequence *seq, size_t seedindex = 0);
				int operator()();
				int operator()(size_t seedindex);
				size_t GetSeed();
				void SetSeed(size_t seedindex);
			private:
				RandomIntSequence *sequence;
				size_t index;
		};
	private:
		vector<int> values;
};

class RandomDoubleSequence
{
		friend class Generator;
	public:
		RandomDoubleSequence(double minval, double maxval, size_t count);

		double operator()(size_t seedindex);

		class Generator
		{
			public:
				Generator(RandomDoubleSequence *seq, size_t seedindex = 0);
				double operator()();
				double operator()(size_t seedindex);
				size_t GetSeed();
				void SetSeed(size_t seedindex);
			private:
				RandomDoubleSequence *sequence;
				size_t index;
		};
	private:
		vector<double> values;
};


/**
 ***************************************************************************************************************
 *
 *  @name Number Generator Classes
 *
 *  @{
 */

/// Abstract class representing a generator for numbers that can be accessed sequentially.
template<class Type>
class SequentialNumberGenerator
{
	public:
		typedef Type result_type; // defined for compatibility with boost's NumberGenerator concept
		/// Returns the next number from the sequence.
		virtual Type operator()() = 0;
		/// Returns the next N numbers from the sequence.
		virtual shared_ptr<vector<Type> > GetSequence(size_t count)
		{
			shared_ptr<vector<Type> > data(new vector<Type>);
			data->reserve(count);
			for (size_t i = 0; i < count; i ++)
				data->push_back((*this)());
			return data;
		}
		/// Returns the number of values after which the generator must be expected to repeat (SIZE_MAX if unknown or pretty huge).
		virtual size_t CycleLength() const = 0;
};

/// Abstract class representing a generator for numbers that can be accessed sequentially and depend on some seed.
template<class Type>
class SeedableNumberGenerator : public SequentialNumberGenerator<Type>
{
	public:
		/// Seeds the generator.
		virtual void Seed(size_t seed) = 0;
};

/// Abstract class representing a generator for numbers that can be accessed by index.
template<class Type>
class IndexedNumberGenerator
{
	public:
		/// Returns a particular number from the sequence.
		virtual Type operator[](size_t index) const = 0;
		/// Returns a particular subsequence from the sequence.
		virtual shared_ptr<vector<Type> > GetSequence(size_t index, size_t count) const
		{
			shared_ptr<vector<Type> > data(new vector<Type>);
			data->reserve(count);
			for (size_t i = 0; i < count; i ++)
				data->push_back((*this)[index + i]);
			return data;
		}
		/// Returns the maximum reasonable index.
		/// While larger indices are allowed, they may be mapped internally to lower ones.
		virtual size_t MaxIndex() const = 0;
};

/**
 *  @}
 *
 ***************************************************************************************************************
 *
 *  @name Number Generator Pointers
 *
 *  The following types hold shared references to number generators.
 *
 *  @note   These are of type boost::shared_ptr<>; the assignment operator (=) may not work as expected,
 *          as it is in fact implemented as a swap operation.
 *          To clone a reference, use e.g. "SequentialDoubleGeneratorPtr myNewPtr(myOldPtr);".
 *
 *  @{
 */

typedef shared_ptr<SequentialNumberGenerator<int> >         SequentialIntGeneratorPtr;
typedef shared_ptr<SequentialNumberGenerator<double> >      SequentialDoubleGeneratorPtr;
typedef shared_ptr<SequentialNumberGenerator<Vector3d> >    SequentialVectorGeneratorPtr;
typedef shared_ptr<SequentialNumberGenerator<Vector2d> >    SequentialVector2dGeneratorPtr;

typedef shared_ptr<SeedableNumberGenerator<int> >           SeedableIntGeneratorPtr;
typedef shared_ptr<SeedableNumberGenerator<double> >        SeedableDoubleGeneratorPtr;
typedef shared_ptr<SeedableNumberGenerator<Vector3d> >      SeedableVectorGeneratorPtr;
typedef shared_ptr<SeedableNumberGenerator<Vector2d> >      SeedableVector2dGeneratorPtr;

typedef shared_ptr<IndexedNumberGenerator<int> const>       IndexedIntGeneratorPtr;
typedef shared_ptr<IndexedNumberGenerator<double> const>    IndexedDoubleGeneratorPtr;
typedef shared_ptr<IndexedNumberGenerator<Vector3d> const>  IndexedVectorGeneratorPtr;
typedef shared_ptr<IndexedNumberGenerator<Vector2d> const>  IndexedVector2dGeneratorPtr;

/**
 *  @}
 *
 ***************************************************************************************************************
 *
 *  @name Pseudo-Random Number Generator Factories
 *
 *  The following global functions provide sources for pseudo-random number sequences, that is, reproducible
 *  sequences of numbers that are intended to appear non-correlated and... well, pretty random.
 *
 *  @note       For some purposes, sub-random number generators may be better suited.
 *
 *  @{
 */

/**
 *  Gets a source for integer pseudo-random numbers satisfying the given properties.
 *  The object returned is intended for sequential access.
 *
 *  @param[in]  minval          Lower bound of value interval (inclusive).
 *  @param[in]  maxval          Upper bound of value interval (inclusive).
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SeedableIntGeneratorPtr GetRandomIntGenerator(int minval, int maxval, size_t count);

/**
 *  Gets a source for floating-point pseudo-random numbers satisfying the given properties.
 *  The object returned is intended for sequential access.
 *
 *  @param[in]  minval          Lower bound of value interval (inclusive).
 *  @param[in]  maxval          Upper bound of value interval (inclusive).
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SeedableDoubleGeneratorPtr GetRandomDoubleGenerator(double minval, double maxval, size_t count);

/**
 *  Gets a source for floating-point pseudo-random numbers satisfying the given properties.
 *  The object returned is intended for sequential access.
 *
 *  @param[in]  minval          Lower bound of value interval (inclusive).
 *  @param[in]  maxval          Upper bound of value interval (inclusive).
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SequentialDoubleGeneratorPtr GetRandomDoubleGenerator(double minval, double maxval);

/**
 *  Gets a source for floating-point pseudo-random numbers satisfying the given properties.
 *  The object returned is intended for access by index.
 *
 *  @param[in]  minval          Lower bound of value interval (inclusive).
 *  @param[in]  maxval          Upper bound of value interval (inclusive).
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
IndexedDoubleGeneratorPtr GetIndexedRandomDoubleGenerator(double minval, double maxval, size_t count);

/**
 *  @}
 *
 ***************************************************************************************************************
 *
 *  @name Sub-Random Number Generator Factories
 *
 *  The following global functions provide sources for low-discrepancy sequences (aka sub-random or quasi-random
 *  number sequences - not to be confused with pseudo-random number sequences), that is, reproducible sequences
 *  of values that cover an interval (or N-dimensional space) pretty uniformly, regardless how many consecutive
 *  values are used from the sequence.
 *
 *  @{
 */

/**
 *  Gets a source for sub-random (low discrepancy) floating-point numbers in the specified interval.
 *
 *  @param[in]  id              Selects one of multiple sources.
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SequentialDoubleGeneratorPtr GetSubRandomDoubleGenerator(unsigned int id, double minval, double maxval, size_t count = 0);

/**
 *  Gets a source for cosine-weighted sub-random (low discrepancy) vectors on the unit hemisphere centered around +Y.
 *
 *  @note       If count is smaller than 1600, this function will return a generator for the hard-coded
 *              radiosity sampling direction sequence used in POV-Ray 3.6.
 *
 *  @param[in]  id              Selects one of multiple sources.
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SequentialVectorGeneratorPtr GetSubRandomCosWeightedDirectionGenerator(unsigned int id, size_t count = 0);

/**
 *  Gets a source for sub-random (low discrepancy) vectors on the unit sphere.
 *
 *  @param[in]  id              Selects one of multiple sources.
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SequentialVectorGeneratorPtr GetSubRandomDirectionGenerator(unsigned int id, size_t count = 0);

/**
 *  Gets a source for sub-random (low discrepancy) 2D vectors on a disc.
 *
 *  @param[in]  id              Selects one of multiple sources.
 *  @param[in]  radius          Radius of the disc.
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SequentialVector2dGeneratorPtr GetSubRandomOnDiscGenerator(unsigned int id, double radius, size_t count = 0);

/**
 *  Gets a source for sub-random (low discrepancy) 2D vectors within a square.
 *
 *  @param[in]  id              Selects one of multiple sources.
 *  @param[in]  minX            Lower bound of X coordinate.
 *  @param[in]  maxX            Upper bound of X coordinate.
 *  @param[in]  minY            Lower bound of Y coordinate.
 *  @param[in]  maxY            Upper bound of Y coordinate.
 *  @param[in]  count           Number of values to provide.
 *  @return                     A shared pointer to a corresponding number generator.
 */
SequentialVector2dGeneratorPtr GetSubRandom2dGenerator(unsigned int id, double minX, double maxX, double minY, double maxY, size_t count = 0);

/**
 *  @}
 *
 ***************************************************************************************************************
 */

} // end of namespace

#endif // POVRAY_BACKEND_RANDOMSEQUENCES_H
