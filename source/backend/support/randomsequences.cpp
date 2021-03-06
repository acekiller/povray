/*******************************************************************************
 * randomsequences.cpp
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
 * $File: //depot/povray/smp/source/backend/support/randomsequences.cpp $
 * $Revision: #15 $
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

#include <cassert>
#include <stdexcept>
#include <map>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/support/randomsequences.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

using namespace pov_base;

using boost::uniform_int;
using boost::uniform_real;
using boost::variate_generator;
using boost::mt19937;

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#define PRIME_TABLE_COUNT 25
unsigned int primeTable[PRIME_TABLE_COUNT] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 };

/**********************************************************************************
 *  Legacy Code
 *********************************************************************************/

vector<int> RandomInts(int minval, int maxval, size_t count)
{
	mt19937 generator;
	uniform_int<int> distribution(minval, maxval);
	variate_generator<mt19937, uniform_int<int> > sequence(generator, distribution);
	vector<int> rands(count);

	for(size_t i = 0; i < count; i++)
		rands[i] = sequence();

	return rands;
}

vector<double> RandomDoubles(double minval, double maxval, size_t count)
{
	mt19937 generator;
	uniform_real<double> distribution(minval, maxval);
	variate_generator<mt19937, uniform_real<double> > sequence(generator, distribution);
	vector<double> rands(count);

	for(size_t i = 0; i < count; i++)
		rands[i] = sequence();

	return rands;
}

RandomIntSequence::RandomIntSequence(int minval, int maxval, size_t count) :
	values(RandomInts(minval, maxval, count))
{
}

RandomIntSequence::Generator::Generator(RandomIntSequence *seq, size_t seedindex) :
	sequence(seq),
	index(seedindex)
{
}

int RandomIntSequence::operator()(size_t seedindex)
{
	seedindex = seedindex % values.size();
	return values[seedindex];
}

int RandomIntSequence::Generator::operator()()
{
	index = (index + 1) % sequence->values.size();
	return (*sequence)(index);
}

int RandomIntSequence::Generator::operator()(size_t seedindex)
{
	return (*sequence)(seedindex);
}

size_t RandomIntSequence::Generator::GetSeed()
{
	return index;
}

void RandomIntSequence::Generator::SetSeed(size_t seedindex)
{
	index = seedindex % sequence->values.size();
}

RandomDoubleSequence::RandomDoubleSequence(double minval, double maxval, size_t count) :
	values(RandomDoubles(minval, maxval, count))
{
}

RandomDoubleSequence::Generator::Generator(RandomDoubleSequence *seq, size_t seedindex) :
	sequence(seq),
	index(seedindex)
{
}

double RandomDoubleSequence::operator()(size_t seedindex)
{
	seedindex = seedindex % values.size();
	return values[seedindex];
}

double RandomDoubleSequence::Generator::operator()()
{
	index = (index + 1) % sequence->values.size();
	return (*sequence)(index);
}

double RandomDoubleSequence::Generator::operator()(size_t seedindex)
{
	return (*sequence)(seedindex);
}

size_t RandomDoubleSequence::Generator::GetSeed()
{
	return index;
}

void RandomDoubleSequence::Generator::SetSeed(size_t seedindex)
{
	index = seedindex % sequence->values.size();
}


/**********************************************************************************
 *  Local Types : Abstract Generators
 *********************************************************************************/

/**
 *  Abstract template class representing a generator for numbers that can be accessed both sequentially and by index.
 */
template<class Type>
class HybridNumberGenerator : public SeedableNumberGenerator<Type>, public IndexedNumberGenerator<Type>
{
	public:

		HybridNumberGenerator(size_t size = 0);
		virtual Type operator()();
		virtual shared_ptr<vector<Type> > GetSequence(size_t count);
		virtual size_t MaxIndex() const;
		virtual size_t CycleLength() const;
		virtual void Seed(size_t seed);

	protected:

		const size_t    size;
		size_t          index;
};


/**********************************************************************************
 *  Local Types : Linear Generators
 *********************************************************************************/

/**
 *  Template class representing a generator for uniformly distributed numbers in a given range.
 */
template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH = SIZE_MAX>
class UniformRandomNumberGenerator : public SequentialNumberGenerator<Type>
{
	public:

		struct ParameterStruct {
			ParameterStruct(Type minval, Type maxval);
			Type minval, maxval;
			bool operator< (const ParameterStruct& other) const;
		};

		UniformRandomNumberGenerator(const ParameterStruct& param);
		UniformRandomNumberGenerator(Type minval, Type maxval);
		virtual Type operator()();
		virtual size_t CycleLength() const;

	protected:
		variate_generator<BoostGenerator, UniformType> generator;
};

typedef UniformRandomNumberGenerator<int,    mt19937, uniform_int<int> >        Mt19937IntGenerator;
typedef UniformRandomNumberGenerator<double, mt19937, uniform_real<double> >    Mt19937DoubleGenerator;

/**
 *  Generator for a 1-dimensional Halton sequence (aka van-der-Corput sequence).
 *  This class fulfills the boost UniformRandomNumberGenerator requirements,
 *  except that the numbers generated are actually sub-random.
 */
template<class Type>
class HaltonGenerator : public HybridNumberGenerator<Type>
{
	public:

		struct ParameterStruct {
			ParameterStruct(unsigned int base, Type minval, Type maxval);
			unsigned int base;
			Type minval, maxval;
			bool operator< (const ParameterStruct& other) const;
		};

		HaltonGenerator(const ParameterStruct& param);
		HaltonGenerator(unsigned int base, Type minval, Type maxval);
		/// Returns a particular number from the sequence.
		virtual double operator[](size_t index) const;

	protected:

		unsigned int    base;
		Type            minval;
		Type            scale;
};

typedef HaltonGenerator<int>    HaltonIntGenerator;
typedef HaltonGenerator<double> HaltonDoubleGenerator;


/**********************************************************************************
 *  Local Types : Vector Generators
 *********************************************************************************/

/**
 *  Class generating a cosine-weighted hemispherical direction vector compatible with earlier POV-Ray versions.
 *  This class uses a 1600-element hard-coded directions originally used for radiosity.
 */
class LegacyCosWeightedDirectionGenerator : public HybridNumberGenerator<Vector3d>
{
	public:

		static const int NumEntries = 1600;

		struct ParameterStruct
		{
			bool operator< (const ParameterStruct& other) const;
		};

		LegacyCosWeightedDirectionGenerator(const ParameterStruct& dummy);
		virtual Vector3d operator[](size_t i) const;
};


/**
 *  Abstract template class generating a vector based on a 2D Halton sequence.
 */
template<class Type, class TypeA, class TypeB = TypeA>
class Halton2dBasedGenerator : public HybridNumberGenerator<Type>
{
	public:

		struct ParameterStruct
		{
			ParameterStruct(unsigned int baseA, unsigned int baseB, TypeA minvalA, TypeA maxvalA, TypeB minvalB, TypeB maxvalB);
			unsigned int baseA, baseB;
			TypeA minvalA, maxvalA;
			TypeB minvalB, maxvalB;
			bool operator< (const ParameterStruct& other) const;
		};

		Halton2dBasedGenerator(const ParameterStruct& param);
		virtual Type operator[](size_t i) const = 0;

	protected:

		shared_ptr<HaltonDoubleGenerator> generatorA;
		shared_ptr<HaltonDoubleGenerator> generatorB;
};

/**
 *  Class generating cosine-weighted hemispherical direction vectors, centered around the Y axis, based on a 2D Halton sequence.
 */
class HaltonCosWeightedDirectionGenerator : public Halton2dBasedGenerator<Vector3d, double>
{
	public:

		struct ParameterStruct : Halton2dBasedGenerator<Vector3d, double>::ParameterStruct
		{
			ParameterStruct(unsigned int baseA, unsigned int baseB);
		};

		HaltonCosWeightedDirectionGenerator(const ParameterStruct& param);
		virtual Vector3d operator[](size_t i) const;
};

/**
 *  Class generating uniformly distributed points within the unit circle based on a 2D Halton sequence.
 */
class HaltonOnDiscGenerator : public Halton2dBasedGenerator<Vector2d, double>
{
	public:

		struct ParameterStruct : Halton2dBasedGenerator<Vector2d, double>::ParameterStruct
		{
			ParameterStruct(unsigned int baseA, unsigned int baseB, double radius);
		};

		HaltonOnDiscGenerator(const ParameterStruct& param);
		virtual Vector2d operator[](size_t i) const;
};

/**
 *  Class generating uniformly distributed points on the unit sphere based on a 2D Halton sequence.
 */
class HaltonUniformDirectionGenerator : public Halton2dBasedGenerator<Vector3d, double>
{
	public:

		struct ParameterStruct : Halton2dBasedGenerator<Vector3d, double>::ParameterStruct
		{
			ParameterStruct(unsigned int baseA, unsigned int baseB);
		};

		HaltonUniformDirectionGenerator(const ParameterStruct& param);
		virtual Vector3d operator[](size_t i) const;
};

/**
 *  Class generating uniformly distributed points within a square based on a 2D Halton sequence.
 */
class Halton2dGenerator : public Halton2dBasedGenerator<Vector2d, double>
{
	public:
		Halton2dGenerator(const ParameterStruct& param);
		virtual Vector2d operator[](size_t i) const;
};


/**********************************************************************************
 *  Local Types : Auxiliary
 *********************************************************************************/

/**
 *  Template class representing a factory for pre-computed number tables.
 */
template<class Type>
class NumberSequenceFactory
{
	public:

		/// Sets up the factory to use a given sequence.
		NumberSequenceFactory(shared_ptr<vector<Type> const> masterSequence);
		/// Sets up the factory to use a given number source.
		NumberSequenceFactory(shared_ptr<SequentialNumberGenerator<Type> > master);
		/// Sets up the factory to use a given number source, pre-computing a given number of elements.
		NumberSequenceFactory(shared_ptr<SequentialNumberGenerator<Type> > master, size_t count);
		/// Gets a reference to a table of pre-computed numbers having at least the given size.
		/// @note The vector returned may contain more elements than requested.
		shared_ptr<vector<Type> const> operator()(size_t count);

	protected:

		typedef SequentialNumberGenerator<Type> Generator;
		typedef shared_ptr<Generator>           GeneratorPtr;
		typedef vector<Type>                    Sequence;
		typedef shared_ptr<Sequence>            SequencePtr;
		typedef shared_ptr<Sequence const>      SequenceConstPtr;

		GeneratorPtr        master;
		SequenceConstPtr    masterSequence;
		boost::mutex        masterMutex;
};

typedef NumberSequenceFactory<int>      IntSequenceFactory;
typedef NumberSequenceFactory<double>   DoubleSequenceFactory;
typedef NumberSequenceFactory<Vector3d> VectorSequenceFactory;


/**
 *  Template class representing a meta-factory for factories for pre-computed number tables.
 */
template<class ValueType, class GeneratorType>
class NumberSequenceMetaFactory
{
	public:

		static shared_ptr<NumberSequenceFactory<ValueType> > GetFactory(const typename GeneratorType::ParameterStruct& param);

	protected:

		typedef NumberSequenceFactory<ValueType>    Factory;
		typedef shared_ptr<Factory>                 FactoryPtr;
		typedef boost::weak_ptr<Factory>            FactoryWeakPtr;
		typedef std::map<typename GeneratorType::ParameterStruct, FactoryWeakPtr> FactoryTable;

		static  FactoryTable*   lookupTable;
		static  boost::mutex    lookupMutex;
};

typedef NumberSequenceMetaFactory<int,      Mt19937IntGenerator>                    Mt19937IntMetaFactory;
typedef NumberSequenceMetaFactory<double,   Mt19937DoubleGenerator>                 Mt19937DoubleMetaFactory;
typedef NumberSequenceMetaFactory<Vector3d, LegacyCosWeightedDirectionGenerator>    LegacyCosWeightedDirectionMetaFactory;
typedef NumberSequenceMetaFactory<Vector3d, HaltonCosWeightedDirectionGenerator>    HaltonCosWeightedDirectionMetaFactory;
typedef NumberSequenceMetaFactory<double,   HaltonDoubleGenerator>                  HaltonUniformDoubleMetaFactory;
typedef NumberSequenceMetaFactory<Vector3d, HaltonUniformDirectionGenerator>        HaltonUniformDirectionMetaFactory;
typedef NumberSequenceMetaFactory<Vector2d, HaltonOnDiscGenerator>                  HaltonOnDiscMetaFactory;
typedef NumberSequenceMetaFactory<Vector2d, Halton2dGenerator>                      Halton2dMetaFactory;


/**
 *  Template class representing a generator for pre-computed numbers using a shared values table.
 */
template<class Type>
class PrecomputedNumberGenerator : public HybridNumberGenerator<Type>
{
	public:

		/// Construct from a sequence factory.
		PrecomputedNumberGenerator(shared_ptr<NumberSequenceFactory<Type> > master, size_t size) :
			HybridNumberGenerator<Type>(size),
			values((*master)(size)),
			startIndex(startIndex)
		{}

		/// Returns a particular number from the sequence.
		virtual Type operator[](size_t i) const
		{
			// According to C++ standard, template classes cannot refer to parent template classes' members by unqualified name
			const size_t& size = HybridNumberGenerator<Type>::size;
			return (*values)[i % size];
		}
		/// Returns a particular subset from the sequence.
		virtual shared_ptr<vector<Type> > GetSequence(size_t index, size_t count) const
		{
			// According to C++ standard, template classes cannot refer to parent template classes' members by unqualified name
			const size_t& size = HybridNumberGenerator<Type>::size;
			shared_ptr<vector<Type> > data(new vector<Type>);
			data->reserve(count);
			size_t i = index % size;
			while (count >= size - i) // handle wrap-around
			{
				data->insert(data->end(), values->begin() + i, values->begin() + size);
				count -= (size - i);
				i = 0;
			}
			data->insert(data->end(), values->begin() + i, values->begin() + i + count);
			return data;
		}

	protected:

		shared_ptr<vector<Type> const> values;
		size_t startIndex;

};

typedef PrecomputedNumberGenerator<int>         PrecomputedIntGenerator;
typedef PrecomputedNumberGenerator<double>      PrecomputedDoubleGenerator;
typedef PrecomputedNumberGenerator<Vector3d>    PrecomputedVectorGenerator;
typedef PrecomputedNumberGenerator<Vector2d>    PrecomputedVector2dGenerator;


/**********************************************************************************
 *  HybridNumberGenerator implementation
 *********************************************************************************/

template<class Type>
HybridNumberGenerator<Type>::HybridNumberGenerator(size_t size) :
	size(size),
	index(0)
{}

template<class Type>
Type HybridNumberGenerator<Type>::operator()()
{
	const Type& data = (*this)[index ++];
	if (size != 0)
		index = index % size;
	return data;
}

template<class Type>
shared_ptr<vector<Type> > HybridNumberGenerator<Type>::GetSequence(size_t count)
{
	shared_ptr<vector<Type> > data(IndexedNumberGenerator<Type>::GetSequence(index, count));
	index += count;
	if (size != 0)
		index = index % size;
	return data;
}

template<class Type>
size_t HybridNumberGenerator<Type>::MaxIndex() const
{
	return size - 1;
}

template<class Type>
size_t HybridNumberGenerator<Type>::CycleLength() const
{
	return size;
}

template<class Type>
void HybridNumberGenerator<Type>::Seed(size_t seed)
{
	index = seed % size;
}


/**********************************************************************************
 *  UniformRandomNumberGenerator implementation
 *********************************************************************************/

template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH>
UniformRandomNumberGenerator<Type,BoostGenerator,UniformType,CYCLE_LENGTH>::ParameterStruct::ParameterStruct(Type minval, Type maxval) :
	minval(minval), maxval(maxval)
{}

template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH>
bool UniformRandomNumberGenerator<Type,BoostGenerator,UniformType,CYCLE_LENGTH>::ParameterStruct::operator< (const ParameterStruct& other) const
{
	if (minval != other.minval)
		return (minval < other.minval);
	else
		return (maxval < other.maxval);
}

template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH>
UniformRandomNumberGenerator<Type,BoostGenerator,UniformType,CYCLE_LENGTH>::UniformRandomNumberGenerator(const ParameterStruct& param) :
	generator(BoostGenerator(), UniformType(param.minval, param.maxval))
{}

template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH>
UniformRandomNumberGenerator<Type,BoostGenerator,UniformType,CYCLE_LENGTH>::UniformRandomNumberGenerator(Type minval, Type maxval) :
	generator(BoostGenerator(), UniformType(minval, maxval))
{}

template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH>
Type UniformRandomNumberGenerator<Type,BoostGenerator,UniformType,CYCLE_LENGTH>::operator()()
{
	return generator();
}

template<class Type, class BoostGenerator, class UniformType, size_t CYCLE_LENGTH>
size_t UniformRandomNumberGenerator<Type,BoostGenerator,UniformType,CYCLE_LENGTH>::CycleLength() const
{
	return CYCLE_LENGTH;
}


/**********************************************************************************
 *  HaltonGenerator implementation
 *********************************************************************************/

template<class Type>
HaltonGenerator<Type>::ParameterStruct::ParameterStruct(unsigned int base, Type minval, Type maxval) :
	base(base), minval(minval), maxval(maxval)
{}

template<class Type>
bool HaltonGenerator<Type>::ParameterStruct::operator< (const ParameterStruct& other) const
{
	if (base != other.base)
		return (base < other.base);
	else if (minval != other.minval)
		return (minval < other.minval);
	else
		return (maxval < other.maxval);
}

template<class Type>
HaltonGenerator<Type>::HaltonGenerator(const ParameterStruct& param) :
	base(param.base),
	minval(param.minval),
	scale(param.maxval-param.minval)
{
}

template<class Type>
HaltonGenerator<Type>::HaltonGenerator(unsigned int base, Type minval, Type maxval) :
	base(base),
	minval(minval),
	scale(maxval-minval)
{
}

template<class Type>
double HaltonGenerator<Type>::operator[](size_t index) const
{
	size_t i = 1 + index; // index starts at 0, but halton sequence as implemented here starts at 1

	double h = 0;
	double q = 1.0/base;
	unsigned int digit;

	while (i > 0)
	{
		digit = (unsigned int)(i % base);
		h = h + digit * q;
		i /= base;
		q /= base;
	}

	return minval + (Type)(h * scale);
}


/**********************************************************************************
 *  NumberSequenceFactory implementation
 *********************************************************************************/

template<class Type>
NumberSequenceFactory<Type>::NumberSequenceFactory(shared_ptr<vector<Type> const> masterSequence) :
	masterSequence(masterSequence)
{}

template<class Type>
NumberSequenceFactory<Type>::NumberSequenceFactory(shared_ptr<SequentialNumberGenerator<Type> > master) :
	master(master)
{}

template<class Type>
NumberSequenceFactory<Type>::NumberSequenceFactory(shared_ptr<SequentialNumberGenerator<Type> > master, size_t count) :
	master(master)
{
	(*this)(count); // force initial sequence to be generated
}

template<class Type>
shared_ptr<vector<Type> const> NumberSequenceFactory<Type>::operator()(size_t count)
{
	boost::mutex::scoped_lock lock(masterMutex);
	if (!masterSequence)
	{
		// No values pre-computed yet; do it now.
		masterSequence = SequenceConstPtr(master->GetSequence(count));
	}
	else if ((masterSequence->size() < count) && master)
	{
		// Not enough values pre-computed; release the current values list and build a larger one.
		// NB: We're not simply appending to the current values list, because that might require re-allocation
		// and interfere with other threads trying to read from the list. To avoid having to synchronize
		// all read accesses, we're going for the less memory-efficient approach.
		size_t newCount = count;
		if (masterSequence->size() > newCount / 2)
		{
			// make sure to pre-compute at least twice the already-computed size, so we don't waste too much space with
			if (masterSequence->size() > SIZE_MAX / 2) // play it safe (though that'll have us run out of memory anyway)
				newCount = SIZE_MAX;
			else
				newCount = masterSequence->size() * 2;
		}
		// Pull more data from our master generator.
		// NB: We're using a temporary pointer to the new values list, so we can keep the master list const,
		// lest anyone might accidently modify it while other threads are reading it.
		SequenceConstPtr newSequence(master->GetSequence(newCount - masterSequence->size()));
		SequencePtr mergedSequence(new Sequence(*masterSequence));
		mergedSequence->insert(mergedSequence->end(), newSequence->begin(), newSequence->end());
		masterSequence = mergedSequence;
	}
	return masterSequence;
}


/**********************************************************************************
 *  NumberSequenceMetaFactory implementation
 *********************************************************************************/

template<class ValueType, class GeneratorType>
std::map<typename GeneratorType::ParameterStruct, boost::weak_ptr<NumberSequenceFactory<ValueType> > >* NumberSequenceMetaFactory<ValueType, GeneratorType>::lookupTable;

template<class ValueType, class GeneratorType>
boost::mutex NumberSequenceMetaFactory<ValueType, GeneratorType>::lookupMutex;

template<class ValueType, class GeneratorType>
shared_ptr<NumberSequenceFactory<ValueType> > NumberSequenceMetaFactory<ValueType, GeneratorType>::GetFactory(const typename GeneratorType::ParameterStruct& param)
{
	boost::mutex::scoped_lock lock(lookupMutex);
	if (!lookupTable)
		lookupTable = new FactoryTable();
	shared_ptr<NumberSequenceFactory<ValueType> > factory = (*lookupTable)[param].lock();
	if (!factory)
	{
		shared_ptr<GeneratorType> masterGenerator(new GeneratorType(param));
		factory = shared_ptr<NumberSequenceFactory<ValueType> >(new NumberSequenceFactory<ValueType>(masterGenerator));
		(*lookupTable)[param] = factory;
	}
	return factory;
}


/**********************************************************************************
 *  LegacyCosWeightedDirectionGenerator implementation
 *********************************************************************************/

extern BYTE_XYZ rad_samples[]; // defined in rad_data.cpp

bool LegacyCosWeightedDirectionGenerator::ParameterStruct::operator< (const ParameterStruct& other) const
{
	return false; // all instances are equal
}

LegacyCosWeightedDirectionGenerator::LegacyCosWeightedDirectionGenerator(const ParameterStruct& dummy)
{}

Vector3d LegacyCosWeightedDirectionGenerator::operator[](size_t i) const
{
	Vector3d result;
	VUnpack(result, &(rad_samples[i % NumEntries]));
	return result;
}


/**********************************************************************************
 *  Halton2dBasedGenerator implementation
 *********************************************************************************/

template<class Type, class TypeA, class TypeB>
Halton2dBasedGenerator<Type, TypeA, TypeB>::ParameterStruct::ParameterStruct(unsigned int baseA, unsigned int baseB, TypeA minvalA, TypeA maxvalA, TypeB minvalB, TypeB maxvalB) :
	baseA(baseA), baseB(baseB),
	minvalA(minvalA), maxvalA(maxvalA),
	minvalB(minvalB), maxvalB(maxvalB)
{}

template<class Type, class TypeA, class TypeB>
bool Halton2dBasedGenerator<Type, TypeA, TypeB>::ParameterStruct::operator< (const ParameterStruct& other) const
{
	if (baseA != other.baseA)
		return (baseA < other.baseA);
	else if (baseB != other.baseB)
		return (baseB < other.baseB);
	else if (minvalA != other.minvalA)
		return (minvalA < other.minvalA);
	else if (maxvalA != other.maxvalA)
		return (maxvalA < other.maxvalA);
	else if (minvalB != other.minvalB)
		return (minvalB < other.minvalB);
	else
		return (maxvalB < other.maxvalB);
}

template<class Type, class TypeA, class TypeB>
Halton2dBasedGenerator<Type, TypeA, TypeB>::Halton2dBasedGenerator(const ParameterStruct& param) :
	generatorA(new HaltonDoubleGenerator(param.baseA, param.minvalA, param.maxvalA)),
	generatorB(new HaltonDoubleGenerator(param.baseB, param.minvalB, param.maxvalB))
{}


/**********************************************************************************
 *  HaltonCosWeightedDirectionGenerator implementation
 *********************************************************************************/

HaltonCosWeightedDirectionGenerator::HaltonCosWeightedDirectionGenerator(const ParameterStruct& param) :
	Halton2dBasedGenerator<Vector3d,double,double>(param)
{}

HaltonCosWeightedDirectionGenerator::ParameterStruct::ParameterStruct(unsigned int baseA, unsigned int baseB) :
	Halton2dBasedGenerator<Vector3d,double,double>::ParameterStruct(baseA, baseB, 0.0, 1.0, 0.0, 2*M_PI)
{}

Vector3d HaltonCosWeightedDirectionGenerator::operator[](size_t i) const
{
	double r = sqrt((*generatorA)[i]);
	double theta = (*generatorB)[i];
	double x = r * cos(theta);
	double z = r * sin(theta);
	double y = sqrt (1 - x*x - z*z);
	return Vector3d(x, y, z);
}


/**********************************************************************************
 *  HaltonOnDiscGenerator implementation
 *********************************************************************************/

HaltonOnDiscGenerator::HaltonOnDiscGenerator(const ParameterStruct& param) :
	Halton2dBasedGenerator<Vector2d,double,double>(param)
{}

HaltonOnDiscGenerator::ParameterStruct::ParameterStruct(unsigned int baseA, unsigned int baseB, double radius) :
	Halton2dBasedGenerator<Vector2d,double,double>::ParameterStruct(baseA, baseB, 0.0, radius*radius, 0.0, 2*M_PI)
{}

Vector2d HaltonOnDiscGenerator::operator[](size_t i) const
{
	double r = sqrt((*generatorA)[i]);
	double theta = (*generatorB)[i];
	double x = r * cos(theta);
	double y = r * sin(theta);
	return Vector2d(x, y);
}


/**********************************************************************************
 *  Halton2dGenerator implementation
 *********************************************************************************/

Halton2dGenerator::Halton2dGenerator(const ParameterStruct& param) :
	Halton2dBasedGenerator<Vector2d,double,double>(param)
{}

Vector2d Halton2dGenerator::operator[](size_t i) const
{
	double x = (*generatorA)[i];
	double y = (*generatorB)[i];
	return Vector2d(x, y);
}


/**********************************************************************************
 *  HaltonUniformDirectionGenerator implementation
 *********************************************************************************/

HaltonUniformDirectionGenerator::HaltonUniformDirectionGenerator(const ParameterStruct& param) :
	Halton2dBasedGenerator<Vector3d,double,double>(param)
{}

HaltonUniformDirectionGenerator::ParameterStruct::ParameterStruct(unsigned int baseA, unsigned int baseB) :
	Halton2dBasedGenerator<Vector3d,double,double>::ParameterStruct(baseA, baseB, -1.0, 1.0, 0.0, 2*M_PI)
{}

Vector3d HaltonUniformDirectionGenerator::operator[](size_t i) const
{
	double x = (*generatorA)[i];
	double r = sqrt(1 - x*x);
	double theta = (*generatorB)[i];
	double y = r * cos(theta);
	double z = r * sin(theta);
	return Vector3d(x, y, z);
}


/**********************************************************************************
 *  Factory Functions
 *********************************************************************************/

SeedableIntGeneratorPtr GetRandomIntGenerator(int minval, int maxval, size_t count)
{
	assert (count > 0);
	Mt19937IntGenerator::ParameterStruct param(minval, maxval);
	shared_ptr<NumberSequenceFactory<int> > factory = Mt19937IntMetaFactory::GetFactory(param);
	SeedableIntGeneratorPtr generator(new PrecomputedIntGenerator(factory, count));
	(void)(*generator)(); // legacy fix
	return generator;
}

SeedableDoubleGeneratorPtr GetRandomDoubleGenerator(double minval, double maxval, size_t count)
{
	assert (count > 0);
	Mt19937DoubleGenerator::ParameterStruct param(minval, maxval);
	shared_ptr<NumberSequenceFactory<double> > factory(Mt19937DoubleMetaFactory::GetFactory(param));
	SeedableDoubleGeneratorPtr generator(new PrecomputedDoubleGenerator(factory, count));
	(void)(*generator)(); // legacy fix
	return generator;
}

SequentialDoubleGeneratorPtr GetRandomDoubleGenerator(double minval, double maxval)
{
	Mt19937DoubleGenerator::ParameterStruct param(minval, maxval);
	SequentialDoubleGeneratorPtr generator(new Mt19937DoubleGenerator(param));
	(void)(*generator)(); // legacy fix
	return generator;
}

IndexedDoubleGeneratorPtr GetIndexedRandomDoubleGenerator(double minval, double maxval, size_t count)
{
	assert (count > 0);
	Mt19937DoubleGenerator::ParameterStruct param(minval, maxval);
	shared_ptr<NumberSequenceFactory<double> > factory(Mt19937DoubleMetaFactory::GetFactory(param));
	return IndexedDoubleGeneratorPtr(new PrecomputedDoubleGenerator(factory, count));
}

SequentialVectorGeneratorPtr GetSubRandomCosWeightedDirectionGenerator(unsigned int id, size_t count)
{
	if ((id == 0) && count && (count < LegacyCosWeightedDirectionGenerator::NumEntries))
	{
		LegacyCosWeightedDirectionGenerator::ParameterStruct param;
		shared_ptr<NumberSequenceFactory<Vector3d> > factory(LegacyCosWeightedDirectionMetaFactory::GetFactory(param));
		return SequentialVectorGeneratorPtr(new PrecomputedVectorGenerator(factory, count));
	}
	else
	{
		HaltonCosWeightedDirectionGenerator::ParameterStruct param(primeTable[id % PRIME_TABLE_COUNT], primeTable[(id+1) % PRIME_TABLE_COUNT]);
		if (count)
		{
			shared_ptr<NumberSequenceFactory<Vector3d> > factory(HaltonCosWeightedDirectionMetaFactory::GetFactory(param));
			return SequentialVectorGeneratorPtr(new PrecomputedVectorGenerator(factory, count));
		}
		else
			return SequentialVectorGeneratorPtr(new HaltonCosWeightedDirectionGenerator(param));
	}
}

SequentialDoubleGeneratorPtr GetSubRandomDoubleGenerator(unsigned int id, double minval, double maxval, size_t count)
{
	HaltonDoubleGenerator::ParameterStruct param(primeTable[id % PRIME_TABLE_COUNT], minval, maxval);
	if (count)
	{
		shared_ptr<NumberSequenceFactory<double> > factory(HaltonUniformDoubleMetaFactory::GetFactory(param));
		return SequentialDoubleGeneratorPtr(new PrecomputedDoubleGenerator(factory, count));
	}
	else
		return SequentialDoubleGeneratorPtr(new HaltonDoubleGenerator(param));
}

SequentialVectorGeneratorPtr GetSubRandomDirectionGenerator(unsigned int id, size_t count)
{
	HaltonUniformDirectionGenerator::ParameterStruct param(primeTable[id % PRIME_TABLE_COUNT], primeTable[(id+1) % PRIME_TABLE_COUNT]);
	if (count)
	{
		shared_ptr<NumberSequenceFactory<Vector3d> > factory(HaltonUniformDirectionMetaFactory::GetFactory(param));
		return SequentialVectorGeneratorPtr(new PrecomputedVectorGenerator(factory, count));
	}
	else
		return SequentialVectorGeneratorPtr(new HaltonUniformDirectionGenerator(param));
}

SequentialVector2dGeneratorPtr GetSubRandomOnDiscGenerator(unsigned int id, double radius, size_t count)
{
	HaltonOnDiscGenerator::ParameterStruct param(primeTable[id % PRIME_TABLE_COUNT], primeTable[(id+1) % PRIME_TABLE_COUNT], radius);
	if (count)
	{
		shared_ptr<NumberSequenceFactory<Vector2d> > factory(HaltonOnDiscMetaFactory::GetFactory(param));
		return SequentialVector2dGeneratorPtr(new PrecomputedVector2dGenerator(factory, count));
	}
	else
		return SequentialVector2dGeneratorPtr(new HaltonOnDiscGenerator(param));
}

SequentialVector2dGeneratorPtr GetSubRandom2dGenerator(unsigned int id, double minX, double maxX, double minY, double maxY, size_t count)
{
	Halton2dGenerator::ParameterStruct param(primeTable[id % PRIME_TABLE_COUNT], primeTable[(id+1) % PRIME_TABLE_COUNT], minX, maxX, minY, maxY);
	if (count)
	{
		shared_ptr<NumberSequenceFactory<Vector2d> > factory(Halton2dMetaFactory::GetFactory(param));
		return SequentialVector2dGeneratorPtr(new PrecomputedVector2dGenerator(factory, count));
	}
	else
		return SequentialVector2dGeneratorPtr(new Halton2dGenerator(param));
}

} // end of namespace pov
