/*******************************************************************************
 * configbase.h
 *
 * This header file defines all types that can be configured by platform
 * specific code for base-layer use. It further allows insertion of platform
 * specific function prototypes making use of those types.
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
 * $File: //depot/povray/smp/source/base/configbase.h $
 * $Revision: #14 $
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

#ifndef CONFIGBASE_H
#define CONFIGBASE_H

#include "syspovconfigbase.h"

#ifndef DBL
	#define DBL double
#endif

#ifndef SNGL
	#define SNGL float
#endif

#ifndef COLC
	#define COLC float
#endif

#ifndef UCS2
	#define UCS2 unsigned short
#endif

#ifndef UCS4
	#define UCS4 unsigned int
#endif

#ifndef POV_LONG
	#define POV_LONG long long
	#define POV_ULONG unsigned long long
#endif

#ifndef POV_ULONG
	#define POV_ULONG unsigned POV_LONG
#endif

#ifndef M_PI
	#define M_PI   3.1415926535897932384626
#endif

#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif

#ifndef TWO_M_PI
	#define TWO_M_PI 6.283185307179586476925286766560
#endif

#ifndef M_PI_180
	#define M_PI_180 0.01745329251994329576
#endif

#ifndef M_PI_360
	#define M_PI_360 0.00872664625997164788
#endif

#ifndef POV_SYS_FILE_EXTENSION
	#define POV_SYS_FILE_EXTENSION ".tga"
#endif

#ifndef POV_FILE_SEPARATOR
	#define POV_FILE_SEPARATOR '/'
#endif

#ifndef POV_UCS2_FOPEN
	#define POV_UCS2_FOPEN(name, mode) fopen(UCS2toASCIIString(UCS2String(name)).c_str(), mode)
#endif

#ifndef POV_UCS2_REMOVE
	#define POV_UCS2_REMOVE(name) unlink(UCS2toASCIIString(UCS2String(name)).c_str())
#endif

#ifndef EXIST_FONT_FILE
	#define EXIST_FONT_FILE(name) (0)
#endif

#ifndef DEFAULT_ITEXTSTREAM_BUFFER_SIZE
	#define DEFAULT_ITEXTSTREAM_BUFFER_SIZE 512
#endif

#ifndef POV_ALLOW_FILE_READ
	#define POV_ALLOW_FILE_READ(f,t) (1)
#endif

#ifndef POV_ALLOW_FILE_WRITE
	#define POV_ALLOW_FILE_WRITE(f,t) (1)
#endif

#ifndef POV_TRACE_THREAD_PREINIT
	#define POV_TRACE_THREAD_PREINIT
#endif

#ifndef POV_TRACE_THREAD_POSTINIT
	#define POV_TRACE_THREAD_POSTINIT
#endif

// these should not be changed by platform-specific config
#define DEFAULT_WORKING_GAMMA_TYPE  kPOVList_GammaType_Neutral
#define DEFAULT_WORKING_GAMMA       1.0
#define DEFAULT_WORKING_GAMMA_TEXT  "1.0"


#include "base/povms.h"
#include "base/povmscpp.h"

namespace pov_base
{

typedef POVMSUCS2String UCS2String;

inline UCS2String ASCIItoUCS2String(const char *s)
{
	return POVMS_ASCIItoUCS2String(s);
}

inline string UCS2toASCIIString(const UCS2String& s)
{
	return POVMS_UCS2toASCIIString(s);
}

}

#include "syspovprotobase.h"

#endif
