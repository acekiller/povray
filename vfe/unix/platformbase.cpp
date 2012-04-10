/*******************************************************************************
 * platformbase.cpp
 *
 * This file implements code that is needed (and linked to) the base code.
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 2006-2009 Persistence of Vision Raytracer Pty. Ltd.
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
 * $File: //depot/povray/smp/vfe/unix/platformbase.cpp $
 * $Revision: #12 $
 * $Change: 4846 $
 * $DateTime: 2009/07/20 10:11:14 $
 * $Author: calimet $
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

// must come first
#include "syspovconfig.h"

#ifdef HAVE_TIME_H
# include <time.h>
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#include "vfe.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{
	////////////////////////////////////////////////////////////////////////////////////////
	//
	// needed if we define POV_DELAY_IMPLEMENTED in config.h
	//
	////////////////////////////////////////////////////////////////////////////////////////
	void Delay(unsigned int msec)
	{
#ifdef HAVE_NANOSLEEP
		timespec ts;
		ts.tv_sec = msec / 1000;
		ts.tv_nsec = (POV_ULONG) (1000000) * (msec % 1000);
		nanosleep(&ts, NULL);
#else
		// taken from source/base/timer.cpp
		boost::xtime t;
		boost::xtime_get(&t, boost::TIME_UTC);
		POV_ULONG ns = (POV_ULONG)(t.sec) * (POV_ULONG)(1000000000) + (POV_ULONG)(t.nsec) + (POV_ULONG)(msec) * (POV_ULONG)(1000000);
		t.sec = (boost::xtime::xtime_sec_t)(ns / (POV_ULONG)(1000000000));
		t.nsec = (boost::xtime::xtime_nsec_t)(ns % (POV_ULONG)(1000000000));
		boost::thread::sleep(t);
#endif
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// thread support
	//
	////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	// called by the base code each time a worker thread is created (the call
	// is made in the context of the new thread).
	void vfeSysThreadStartup(void)
	{
	}

	/////////////////////////////////////////////////////////////////////////
	// called by a worker thread just before it exits.
	void vfeSysThreadCleanup(void)
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// class vfeTimer (OPTIONAL)
	//
	// if you don't want to supply this class, remove the definition for POV_TIMER from
	// config.h. see the base code for documentation on the implementation requirements.
	//
	////////////////////////////////////////////////////////////////////////////////////////

	vfeTimer::vfeTimer (bool CPUTimeIsThreadOnly)
	{
		m_ThreadTimeOnly = CPUTimeIsThreadOnly;
		Reset();
	}

	vfeTimer::~vfeTimer ()
	{
	}

	unsigned POV_LONG vfeTimer::GetWallTime (void) const
	{
#ifdef HAVE_CLOCK_GETTIME
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
			return (unsigned POV_LONG) (1000)*ts.tv_sec + ts.tv_nsec/1000000;
#endif
#ifdef HAVE_GETTIMEOFDAY
		struct timeval tv;  // seconds + microseconds since the Epoch (1970-01-01)
		if (gettimeofday(&tv, NULL) == 0)
			return (unsigned POV_LONG) (1000)*tv.tv_sec + tv.tv_usec/1000;
#endif
		return 0;  // FIXME: add fallback, using ftime(), or time() + a counter for ms
	}

	unsigned POV_LONG vfeTimer::GetCPUTime (void) const
	{
#ifdef HAVE_CLOCK_GETTIME
		struct timespec ts;
#if defined (__FreeBSD__)
		if (clock_gettime(m_ThreadTimeOnly ? CLOCK_THREAD_CPUTIME_ID : CLOCK_REALTIME, &ts) == 0)
#else
		if (clock_gettime(m_ThreadTimeOnly ? CLOCK_THREAD_CPUTIME_ID : CLOCK_PROCESS_CPUTIME_ID, &ts) == 0)
#endif
			return (unsigned POV_LONG) (1000)*ts.tv_sec + ts.tv_nsec/1000000;
#endif
#ifdef HAVE_GETRUSAGE
		struct rusage ru;
#if defined(__sun)
		if (getrusage(m_ThreadTimeOnly ? RUSAGE_LWP : RUSAGE_SELF, &ru) == 0)
#else
		if (getrusage(RUSAGE_SELF, &ru) == 0)
#endif
			return (unsigned POV_LONG) (1000)*(ru.ru_utime.tv_sec + ru.ru_stime.tv_sec)
				+ (unsigned POV_LONG)(ru.ru_utime.tv_usec + ru.ru_stime.tv_usec)/1000;
#endif
		return GetWallTime();
	}

	POV_LONG vfeTimer::ElapsedRealTime (void) const
	{
		return GetWallTime() - m_WallTimeStart;
	}

	POV_LONG vfeTimer::ElapsedCPUTime (void) const
	{
		return GetCPUTime() - m_CPUTimeStart;
	}

	void vfeTimer::Reset (void)
	{
		m_WallTimeStart = GetWallTime();
		m_CPUTimeStart = GetCPUTime();
	}

	bool vfeTimer::HasValidCPUTime() const
	{
#ifdef HAVE_CLOCK_GETTIME
		struct timespec ts;
		if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) == 0)
			return true;
#endif
#ifdef HAVE_GETRUSAGE
#if defined(__sun)
		struct rusage ru;
		if (getrusage(RUSAGE_LWP, &ru) == 0)
			return true;
#endif
#endif
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//
	// path parsing
	//
	////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////
	// The first argument is the input, a UCS2 string.
	//
	// The second argument is the string you are supposed to return the "volume"
	// name with.  For DOS-style paths this implies i.e. "A:\" is the "volume".
	// Note that it is essential that the first "path" separator is also part of
	// the volume name.  If the path is relative, the "volume" name shall be empty.
	//
	// This trick is necessary so the code can account for the lack of volume names
	// in Unix-style paths: In Unix, the POV_PARSE_PATH_STRING function will have
	// to take a reading "/" or "~/" as "volume" name.  This makes it possible to
	// determine if a string is absolute or relative based on this 'virtual'
	// "volume" name rather than some flags.
	//
	// The third is a vector of strings you have to return, each has to contain the
	// folder name from left to right, without the path separator, of course.
	//
	// The fourth argument shall contain the filename, if any was given in the
	// source string. By definition if the source string does not contain a
	// trailing path separator, whatever comes after the last path separator
	// (or the start of the string if there is none) must be considered a filename,
	// even if it could be a directory (in other words, don't call a system function
	// to find out if it is a dir or not - see below).
	//
	// Please note that the function must not attempt to determine the validity of
	// a string by accessing the filesystem.  It has to parse anything that it is
	// given.  If the string provided cannot be parsed for some reason (that is if
	// you can determine that a given path cannot possibly be valid i.e. because it
	// contains invalid characters), the function has to return false.  It may not
	// throw exceptions.  The return value for success is true.
	////////////////////////////////////////////////////////////////////////////////////////

	bool vfeParsePathString (const UCS2String& path, UCS2String& volume, vector<UCS2String>& components, UCS2String& filename)
	{
		UCS2String q;

		if(path.empty() == true)
			return true;

		if(path[0] == '/')
			volume = '/';

		for(size_t i = 0; i < path.length(); ++i)
		{
			if(path[i] == '/')
			{
				if(q.empty() == false)
					components.push_back(q);
				q.clear();
			}
			else
				q += path[i];
		}

		filename = q;

		return true;
	}
}
