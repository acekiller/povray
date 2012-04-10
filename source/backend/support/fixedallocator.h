/*******************************************************************************
 * fixedallocator.h
 *
 * Simple allocator using a fixed-size pool.
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
 * $File: //depot/povray/smp/source/backend/support/fixedallocator.h $
 * $Revision: #8 $
 * $Change: 5218 $
 * $DateTime: 2010/12/01 07:10:22 $
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

#if !defined __FIXED_ALLOCATOR_H__
#define __FIXED_ALLOCATOR_H__

#include <cassert>

namespace pov
{
	template <typename T, int MaxElements>
	class FixedAllocator
	{
	public:
		typedef T               value_type ;
		typedef unsigned short  size_type ;
		typedef ptrdiff_t       difference_type;
		typedef T&              reference ;
		typedef const T&        const_reference ;
		typedef T               *pointer ;
		typedef const T         *const_pointer ;

	private:
		size_type byteCount (unsigned char nItems) { return ((size_type) nItems * sizeof (T)) ; }
		typedef struct
		{
			unsigned char nItems ;
			unsigned char allocated ;
		} BlockHeader ;

		void defrag (void)
		{
			unsigned char current = m_FirstFreeBlock ;
			while (current < MaxElements)
			{
				unsigned char next = current + m_Headers [current].nItems ;
				if (next >= MaxElements)
					break ;
				if (m_Headers [current].allocated || m_Headers [next].allocated)
				{
					current = next ;
					continue ;
				}
				m_Headers [current].nItems += m_Headers [next].nItems ;
				m_Headers [next].nItems = 0 ;
			}
		}

		unsigned char allocateBlocks (unsigned char nBlocks, unsigned char current)
		{
			assert (nBlocks > 0) ;
			while (current < MaxElements)
			{
				unsigned char next = current + m_Headers [current].nItems ;
				if (m_Headers [current].allocated == false && m_Headers [current].nItems >= nBlocks)
				{
					if (current + nBlocks < MaxElements && m_Headers [current].nItems > nBlocks)
					{
						m_Headers [current + nBlocks].nItems = m_Headers [current].nItems - nBlocks ;
						m_Headers [current + nBlocks].allocated = false ;
					}
					m_Headers [current].nItems = nBlocks ;
					m_Headers [current].allocated = true ;
					if (m_FirstFreeBlock == current)
						m_FirstFreeBlock = current + nBlocks ;
					return (current) ;
				}
				current = next ;
			}
			return (0xff) ;
		}

	public:
		// construct an allocator for the supplied new type.
		template <typename N>
		struct rebind { typedef FixedAllocator<N, MaxElements> other ; } ;

		size_type max_size() const { return (MaxElements) ; }
		pointer address (reference what) const { return (&what) ; }
		const_pointer address (const_reference what) const { return (&what) ; }
		FixedAllocator ()
		{
			if (MaxElements > 127)
				throw POV_EXCEPTION_CODE(kInternalLimitErr, "Internal limit exceeded in fixedallocator.h") ;
			m_FirstFreeBlock = 0 ;
			m_Headers [0].allocated = false ;
			m_Headers [0].nItems = MaxElements ;
		}
		~FixedAllocator () { }
		void construct (pointer where, T const& what) { new (where) T (what) ; }
		void destroy (pointer what) { what->~T() ; }

		pointer allocate (size_type nItems, std::allocator<void>::const_pointer hint = 0)
		{
			if (((unsigned char *) hint >= m_Data) && ((unsigned char *) hint < m_Data + sizeof (T) * MaxElements))
			{
				unsigned char slot = ((unsigned char *) hint - m_Data) / sizeof (T) ;
				if ((slot = allocateBlocks (nItems, slot)) != 0xff)
					return ((pointer) (m_Data + byteCount (slot))) ;
			}
			unsigned char slot = allocateBlocks (nItems, m_FirstFreeBlock) ;
			if (slot == 0xff)
			{
				defrag () ;
				if ((slot = allocateBlocks (nItems, m_FirstFreeBlock)) == 0xff)
					throw POV_EXCEPTION_CODE(kInternalLimitErr, "Internal limit exceeded in fixedallocator.h") ;
			}
			return ((pointer) (m_Data + byteCount (slot))) ;
		}

		void deallocate (pointer what, unsigned char nItems)
		{
			if (((unsigned char *) what >= m_Data) && ((unsigned char *) what < m_Data + sizeof (T) * MaxElements))
			{
				unsigned char slot = ((unsigned char *) what - m_Data) / sizeof (T) ;
				if (m_Headers [slot].allocated && m_Headers [slot].nItems == nItems)
				{
					m_Headers [slot].allocated = false ;
					if (m_FirstFreeBlock > slot)
					{
						m_FirstFreeBlock = slot ;
						assert (m_Headers [slot].nItems > 0) ;
					}
					unsigned char next = slot + nItems ;
					while (next < MaxElements && m_Headers [next].allocated == false)
					{
						m_Headers [slot].nItems += m_Headers [next].nItems ;
						m_Headers [next].nItems = 0 ;
						next = slot + m_Headers [slot].nItems ;
					}
				}
			}
		}

	private:
		unsigned char m_FirstFreeBlock ;
		unsigned char m_Data [sizeof (T) * MaxElements] ;
		BlockHeader m_Headers [MaxElements] ;
	} ;

	template <typename T>
	class MallocAllocator
	{
	public:
		typedef T               value_type ;
		typedef size_t          size_type ;
		typedef ptrdiff_t       difference_type;
		typedef T&              reference ;
		typedef const T&        const_reference ;
		typedef T               *pointer ;
		typedef const T         *const_pointer ;

	public:
		// construct an allocator for the supplied new type.
		template <typename N>
		struct rebind { typedef MallocAllocator<N> other ; } ;

		size_type max_size() const { return (0xffffffff) ; }
		pointer address (reference what) const { return (&what) ; }
		const_pointer address (const_reference what) const { return (&what) ; }
		MallocAllocator () { }
		~MallocAllocator () { }
		void construct (pointer where, T const& what) { new (where) T (what) ; }
		void destroy (pointer what) { what->~T() ; }

		pointer allocate (size_type nItems, std::allocator<void>::const_pointer hint = 0)
		{
			return ((pointer) malloc (sizeof (T) * nItems)) ;
		}

		void deallocate (pointer what, size_type nItems)
		{
			free (what) ;
		}
	} ;
} // end namespace pov

#endif

