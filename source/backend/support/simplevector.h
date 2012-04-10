/*******************************************************************************
 * simplevector.h
 *
 * Very simple, basic vector-like classes containing just enough functionality
 * for their intended uses within POV. Flexibility is sacrificed for performance
 * as these classes will typically be used in places where they may constructed
 * and destroyed many millions of times per render. (For example, mediasky.pov
 * rendered at only 160x120 with no AA results in over 16 million instances of
 * construction of a FixedSimpleVector).
 *
 * These classes were added after extensive profiling pointed to a number of
 * instances of our use of std::vector causing slowdowns, particularly when
 * multiple threads were in use (due to locks in the RTL memory management used
 * to prevent heap corruption). Experiments with non-heap-based allocators (e.g.
 * refpools or thread-local storage) did improve the situation somewhat but weren't
 * enough, hence this file. At the time of writing we get about a 10% improvement
 * as compared to the old code.
 *
 * NOTE NOTE NOTE NOTE
 * -------------------
 * Be aware that these classes do NOT run destructors on contained objects.
 * This is intentional as we currently do not store any objects in them that
 * require this functionality.
 *
 * Author: Christopher J. Cason.
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
 * $File: //depot/povray/smp/source/backend/support/simplevector.h $
 * $Revision: #14 $
 * $Change: 5360 $
 * $DateTime: 2011/01/06 15:22:14 $
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

#ifndef __SIMPLEVECTOR__
#define __SIMPLEVECTOR__

#include <stdexcept>
#include "base/pov_err.h"

namespace pov
{

////////////////////////////////////////////////////////////////////////////
// Works like std::vector in some ways, but very limited and not at all as
// flexible. Does not implement all the methods of std::vector (just what
// is needed in POV). Has different allocation behaviour, which will probably
// need to be tweaked over time for best performance.
//
// Be aware that this class does NOT run destructors on contained objects.
// This is intentional as we currently do not store any objects in it that
// require this functionality. // TODO FIXME
////////////////////////////////////////////////////////////////////////////
template<class ContainerType, class Allocator = std::allocator<ContainerType> >
class SimpleVector
{
public:
	typedef SimpleVector<ContainerType> MyType;
	typedef size_t size_type;
	typedef size_t difference_type;
	typedef ContainerType *pointer;
	typedef ContainerType& reference;
	typedef ContainerType value_type;
	typedef const ContainerType *const_pointer;
	typedef const ContainerType& const_reference;
	typedef const ContainerType *const_iterator;
	typedef ContainerType *iterator;
	typedef Allocator allocator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	SimpleVector()
	{
		m_First = m_Last = m_End = NULL;
	}

	SimpleVector(size_type nItems, const ContainerType& InitialVal)
	{
		m_First = m_Last = m_End = NULL;
		if (nItems)
			allocate (nItems, InitialVal);
	}

	SimpleVector(const MyType& RHS)
	{
		if (RHS.m_First != RHS.m_Last)
		{
			allocate (RHS.capacity());
			for (pointer p = RHS.m_First ; p != RHS.m_Last ; )
				*m_Last++ = *p++;
		}
		else
			m_First = m_Last = m_End = NULL;
	}

	~SimpleVector()
	{
		// we don't call destructors, even if they exist
		if (m_First != NULL)
			deallocate ();
	}

	MyType& operator=(const MyType& RHS)
	{
		if (RHS.size() > capacity())
		{
			if (m_First != NULL)
				deallocate ();
			allocate (RHS.size());
		}
		m_Last = m_First;
		for (pointer p = RHS.m_First ; p != RHS.m_Last ; )
			*m_Last++ = *p++;
		return (*this);
	}

	size_type capacity() const
	{
		return (m_End - m_First);
	}

	iterator begin()
	{
		return (m_First);
	}

	const_iterator begin() const
	{
		return (m_First);
	}

	iterator end()
	{
		return (m_Last);
	}

	const_iterator end() const
	{
		return (m_Last);
	}

	reverse_iterator rbegin()
	{
		return (reverse_iterator (m_Last));
	}

	const_reverse_iterator rbegin() const
	{
		return (const_reverse_iterator (m_Last));
	}

	reverse_iterator rend()
	{
		return (reverse_iterator (m_First));
	}

	const_reverse_iterator rend() const
	{
		return (const_reverse_iterator (m_First));
	}

	size_type size() const
	{
		return (m_Last - m_First);
	}

	size_type max_size() const
	{
		return (alloc.max_size ());
	}

	bool empty() const
	{
		return (m_First == m_Last);
	}

	const_reference at(size_type Index) const
	{
		if (Index > size())
			throw std::out_of_range ("index out of range in SimpleVector::at");
		return (m_First [Index]);
	}

	reference at(size_type Index)
	{
		if (Index > size())
			throw std::out_of_range ("index out of range in SimpleVector::at");
		return (m_First [Index]);
	}

	const_reference operator[](size_type Index) const
	{
		return (m_First [Index]);
	}

	reference operator[](size_type Index)
	{
		return (m_First [Index]);
	}

	reference front()
	{
		return (*m_First);
	}

	const_reference front() const
	{
		return (*m_First);
	}

	reference back()
	{
		return (*(m_Last - 1));
	}

	const_reference back() const
	{
		return (*(m_Last - 1));
	}

	void push_back(const ContainerType& NewVal)
	{
		if (m_Last < m_End)
		{
			*m_Last++ = NewVal;
			return;
		}
		insert(m_Last, NewVal);
	}

	void pop_back()
	{
		if (m_Last > m_First)
			--m_Last;
	}

	iterator insert(iterator Where, const ContainerType& NewVal)
	{
		size_type Index = 0;

		if (m_Last > m_First)
			Index = Where - m_First;

		if (m_Last == m_End)
		{
			size_type c = size() + 1;
			size_type n = capacity();
			size_type nc = n * 2;
			if (nc < 8)
				nc = 8;
			pointer p = alloc.allocate (nc);
			p [Index] = NewVal;
			for (size_type i = 0 ; i < Index ; i++)
				p [i] = m_First [i];
			for (size_type i = Index + 1 ; i < c ; i++)
				p [i] = m_First [i];
			if (m_First != NULL)
				alloc.deallocate (m_First, n);
			m_First = p;
			m_End = m_First + nc;
			m_Last = m_First + c;
			return (m_First + Index);
		}

		if (Index == size())
		{
			*m_Last = NewVal;
			return (m_Last++);
		}

		for (size_type i = size() ; i > Index ; i--)
			m_First [i] = m_First [i - 1];
		m_Last++;
		m_First [Index] = NewVal;
		return (m_First + Index);
	}

	iterator erase(iterator What)
	{
		size_type Index = What - begin();

		if (What == m_Last - 1)
			return (m_Last--);

		for (pointer p1 = What, p2 = What + 1 ; p2 < m_Last ; )
			*p1++ = *p2++;
		m_Last--;
		return (++What);
	}

	void clear()
	{
		m_Last = m_First;
	}

private:
	void allocate (size_type nItems)
	{
		m_Last = m_First = alloc.allocate (nItems);
		m_End = m_First + nItems;
	}

	void allocate (size_type nItems, const ContainerType& InitialVal)
	{
		m_Last = m_First = alloc.allocate (nItems);
		m_End = m_First + nItems;
		while (nItems--)
			*m_Last++ = InitialVal;
	}

	void deallocate ()
	{
		alloc.deallocate (m_First, m_End - m_First);
	}

	allocator alloc;
	pointer m_First;
	pointer m_End;
	pointer m_Last;
};

////////////////////////////////////////////////////////////////////////////
// This template class requires a maximum size (ElementCount) and maintains
// its storage internally (typically therefore this will end up on the stack
// rather than being allocated upon request). The up side to this behaviour
// is that no time is spent obtaining memory from a pool, or for that matter
// copying data if a reallocation is needed. The down side is that firstly
// it cannot expand beyond the maximum size specified, and secondly that
// binary copies of the object take longer because they contain the entire
// storage (even if no entries are allocated).
//
// Be aware that this class does NOT run destructors on contained objects.
// This is intentional as we currently do not store any objects in it that
// require this functionality.
////////////////////////////////////////////////////////////////////////////
template<class ContainerType, int ElementCount>
class FixedSimpleVector
{
public:
	typedef FixedSimpleVector<ContainerType, ElementCount> MyType;
	typedef size_t size_type;
	typedef size_t difference_type;
	typedef ContainerType *pointer;
	typedef ContainerType& reference;
	typedef ContainerType value_type;
	typedef const ContainerType *const_pointer;
	typedef const ContainerType& const_reference;
	typedef const ContainerType *const_iterator;
	typedef ContainerType *iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	FixedSimpleVector() :
		m_Last ((pointer) m_Data),
		m_End (pointer (m_Data) + ElementCount)
	{
	}

	FixedSimpleVector(size_type nItems, const ContainerType& InitialVal) :
		m_Last ((pointer) m_Data),
		m_End (pointer (m_Data) + ElementCount)
	{
		if (nItems > ElementCount)
			throw POV_EXCEPTION(pov_base::kInternalLimitErr, "Internal limit exceeded in FixedSimpleVector");
		while (nItems--)
			*m_Last++ = InitialVal;
	}

	FixedSimpleVector(const MyType& RHS) :
		m_Last ((pointer) m_Data),
		m_End (pointer (m_Data) + ElementCount)
	{
		for (pointer p = pointer (RHS.m_Data) ; p != RHS.m_Last ; )
			*m_Last++ = *p++;
	}

	~FixedSimpleVector()
	{
		// we don't call destructors, even if they exist
	}

	MyType& operator=(const MyType& RHS)
	{
		if (RHS.size() > ElementCount)
			throw POV_EXCEPTION(pov_base::kInternalLimitErr, "Internal limit exceeded in FixedSimpleVector");
		m_Last = pointer (m_Data);
		for (pointer p = pointer (RHS.m_Data) ; p != RHS.m_Last ; )
			*m_Last++ = *p++;
		return (*this);
	}

	size_type capacity() const
	{
		return (ElementCount);
	}

	iterator begin()
	{
		return (pointer (m_Data));
	}

	const_iterator begin() const
	{
		return (pointer (m_Data));
	}

	iterator end()
	{
		return (m_Last);
	}

	const_iterator end() const
	{
		return (m_Last);
	}

	reverse_iterator rbegin()
	{
		return (reverse_iterator (m_Last));
	}

	const_reverse_iterator rbegin() const
	{
		return (const_reverse_iterator (m_Last));
	}

	reverse_iterator rend()
	{
		return (reverse_iterator (pointer (m_Data)));
	}

	const_reverse_iterator rend() const
	{
		return (const_reverse_iterator (pointer (m_Data)));
	}

	size_type size() const
	{
		return (m_Last - pointer (m_Data));
	}

	size_type max_size() const
	{
		return (ElementCount);
	}

	bool empty() const
	{
		return (pointer (m_Data) == m_Last);
	}

	const_reference at(size_type Index) const
	{
		if (Index > size())
			throw std::out_of_range ("index out of range in FixedSimpleVector::at");
		return (pointer (m_Data) [Index]);
	}

	reference at(size_type Index)
	{
		if (Index > size())
			throw std::out_of_range ("index out of range in FixedSimpleVector::at");
		return (pointer (m_Data) [Index]);
	}

	const_reference operator[](size_type Index) const
	{
		return (pointer (m_Data) [Index]);
	}

	reference operator[](size_type Index)
	{
		return (pointer (m_Data) [Index]);
	}

	reference front()
	{
		return (*pointer (m_Data));
	}

	const_reference front() const
	{
		return (*pointer (m_Data));
	}

	reference back()
	{
		return (*(m_Last - 1));
	}

	const_reference back() const
	{
		return (*(m_Last - 1));
	}

	void push_back(const ContainerType& NewVal)
	{
		if (m_Last == m_End)
			throw POV_EXCEPTION(pov_base::kInternalLimitErr, "Internal limit exceeded in FixedSimpleVector");
		*m_Last++ = NewVal;
	}

	void pop_back()
	{
		if (m_Last > pointer (m_Data))
			--m_Last;
	}

	iterator insert(iterator Where, const ContainerType& NewVal)
	{
		if (m_Last == m_End)
			throw POV_EXCEPTION(pov_base::kInternalLimitErr, "Internal limit exceeded in FixedSimpleVector");
		for (pointer p1 = Where, p2 = Where + 1 ; p1 < m_Last ; )
			*p2++ = *p1++;
		m_Last++;
		*Where = NewVal;
		return (Where);
	}

	iterator erase(iterator Where)
	{
		if (Where == m_End)
			throw POV_EXCEPTION(pov_base::kInternalLimitErr, "Attempt to erase past end of vector");
		if (Where == m_Last - 1)
			return (m_Last--);
		for (pointer p1 = Where, p2 = Where + 1 ; p2 < m_Last ; )
			*p1++ = *p2++;
		m_Last--;
		return (++Where);
	}

	void clear()
	{
		m_Last = pointer (m_Data);
	}

private:
	unsigned char m_Data [sizeof (value_type) * ElementCount];
	const pointer m_End;
	pointer m_Last;
};

}

#endif
