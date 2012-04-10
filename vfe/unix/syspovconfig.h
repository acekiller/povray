/*******************************************************************************
 * syspovconfig.h
 *
 * This file contains most unix-specific defines for compiling the VFE.
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
 * $File: //depot/povray/smp/vfe/unix/syspovconfig.h $
 * $Revision: #13 $
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

#ifndef __SYSPOVCONFIG_H__
#define __SYSPOVCONFIG_H__

#define _FILE_OFFSET_BITS	64

#define fseek64(stream,offset,whence) fseeko(stream,offset,whence)

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# error "!!!!! config.h is required !!!!!"
#endif


#include <algorithm>
#include <stdarg.h>
#include <math.h>

#include "../vfeconf.h"


using std::max;
using std::min;

#ifndef STD_TYPES_DECLARED
#define STD_TYPES_DECLARED

// the following types are used extensively throughout the POV source and hence are
// included and named here for reasons of clarity and convenience.

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>

// when we say 'string' we mean std::string
using std::string;

// and vector is a std::vector
using std::vector;

// yup, list too
using std::list;

// runtime_error is the base of our Exception class, plus is referred
// to in a few other places.
using std::runtime_error;

// C++0x has a shared_ptr, but we currently use the boost one.
using boost::shared_ptr;

#endif // STD_POV_TYPES_DECLARED

// After Stroustrop in _The C++ Programming Language, 3rd Ed_ p. 88
#ifndef NULL
const int NULL=0;
#endif

#define POV_LONG long long

#define DELETE_FILE(name)  unlink(name)

#if defined (PATH_MAX)
# define FILE_NAME_LENGTH   PATH_MAX
#elif defined (_POSIX_PATH_MAX)
# define FILE_NAME_LENGTH   _POSIX_PATH_MAX
#else
# define FILE_NAME_LENGTH   200
#endif
#define MAX_PATH FILE_NAME_LENGTH  // FIXME: remove later

#define DEFAULT_OUTPUT_FORMAT       kPOVList_FileType_PNG
#define DEFAULT_DISPLAY_GAMMA_TYPE  kPOVList_GammaType_SRGB
#define DEFAULT_DISPLAY_GAMMA       2.2
#define DEFAULT_FILE_GAMMA_TYPE     kPOVList_GammaType_SRGB
#define DEFAULT_FILE_GAMMA          2.2

#define METADATA_PLATFORM_STRING BUILD_ARCH
#define METADATA_COMPILER_STRING COMPILER_VERSION

#define DECLARE_THREAD_LOCAL_PTR(ptrType, ptrName)                __thread ptrType *ptrName
#define IMPLEMENT_THREAD_LOCAL_PTR(ptrType, ptrName, ignore)      __thread ptrType *ptrName
#define GET_THREAD_LOCAL_PTR(ptrName)                             (ptrName)
#define SET_THREAD_LOCAL_PTR(ptrName, ptrValue)                   (ptrName = ptrValue)

#endif
