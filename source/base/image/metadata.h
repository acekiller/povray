/*******************************************************************************
 * metadata.h
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
 * $File: //depot/povray/smp/source/base/image/metadata.h $
 * $Revision: #5 $
 * $Change: 5251 $
 * $DateTime: 2010/12/09 08:48:36 $
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

#ifndef POVRAY_BASE_METADATA_H
#define POVRAY_BASE_METADATA_H

#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "backend/povray.h"

namespace pov_base
{

/// Generates metadata to write into output images.
class Metadata
{
	public:
		Metadata()
		{
			boost::posix_time::ptime timestamp = boost::posix_time::second_clock::universal_time();
			date = timestamp.date();
			time = timestamp.time_of_day();
		}

		virtual ~Metadata(){}

		/**
		 *  Get software string.
		 *  @note   This method should return at most 40 ascii bytes, otherwise it may become truncated by some file formats.
		 *          non-printable characters, line feeds and tabs are not allowed.
		 */
		static string getSoftware() { return "POV-Ray " POV_RAY_VERSION;}

		/* Up to 4 comments, each at most 80 ascii bytes, no line feed, no tab 
		 * if it's longer, it can either fit anyway or get truncated, it's the
		 * image format which choose what happen.
		 */

		/**
		 *  Get comment string #1.
		 *  @note   This method should return at most 80 ascii bytes, otherwise it may become truncated by some file formats.
		 *          non-printable characters, line feeds and tabs are not allowed.
		 */
		static string getComment1()
		{
#ifdef METADATA_PLATFORM_STRING
			// METADATA_PLATFORM_STRING should be in a form similar to 'i686-pc-linux-gnu', 'i386-pc-win', etc.
			return string("Platform: ") + METADATA_PLATFORM_STRING;
#else
			return string();
#endif
		}

		/**
		 *  Get comment string #2.
		 *  @note   This method should return at most 80 ascii bytes, otherwise it may become truncated by some file formats.
		 *          non-printable characters, line feeds and tabs are not allowed.
		 */
		static string getComment2()
		{
#ifdef METADATA_COMPILER_STRING
			// METADATA_COMPILER_STRING should be in a form similar to 'g++ 4.4.3', 'msvc 10.0', etc.
			return string("Compiler: ") + METADATA_COMPILER_STRING;
#else
			return string();
#endif
		}

		/**
		 *  Get comment string #3.
		 *  @note   This method should return at most 80 ascii bytes, otherwise it may become truncated by some file formats.
		 *          non-printable characters, line feeds and tabs are not allowed.
		 */
		static string getComment3()
		{
#ifdef METADATA_COMMENT_3
			// NB it is legal for METADATA_COMMENT_3 to be a function returning string
			// Note that it may be called more than once
			return string(METADATA_COMMENT_3);
#else
			return string();
#endif
		}

		/**
		 *  Get comment string #4.
		 *  @note   This method should return at most 80 ascii bytes, otherwise it may become truncated by some file formats.
		 *          non-printable characters, line feeds and tabs are not allowed.
		 */
		static string getComment4()
		{
#ifdef METADATA_COMMENT_4
			// NB it is legal for METADATA_COMMENT_4 to be a function returning string
			// Note that it may be called more than once
			return string(METADATA_COMMENT_4);
#else
			return string();
#endif
		}

		/// Get date string in ISO 8601 format.
		string getDateTime() { return to_iso_extended_string(date) + " " + to_simple_string(time) + "Z"; }

		/// Get year (including full century)
		int getYear() { return date.year(); }
		/// Get month (1..12)
		int getMonth() { return date.month(); }
		/// Get day of month (1..31)
		int getDay() { return date.day(); }
		/// Get hour of day (0..23)
		int getHour() { return time.hours(); }
		/// Get minute of hour (0..59)
		int getMin() { return time.minutes(); }
		/// Get second of minute (0..59)
		int getSec() { return time.seconds(); }

	protected:
		boost::gregorian::date              date;
		boost::posix_time::time_duration    time;
};

}
#endif
