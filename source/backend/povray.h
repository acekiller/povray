/*******************************************************************************
 * povray.h
 *
 * This file contains the interface to initialise and terminate all
 * POV-Ray threads. Beyond the functions in this file, no other
 * functions need to be called to run POV-Ray.
 * Rendering is controlled by the classes provided in the frontend
 * files.
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
 * $File: //depot/povray/smp/source/backend/povray.h $
 * $Revision: #80 $
 * $Change: 5485 $
 * $DateTime: 2011/09/06 09:31:37 $
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

#ifndef POVRAY_BACKEND_POVRAY_H
#define POVRAY_BACKEND_POVRAY_H

// Please put everything that isn't a preprocessor directive in this
// file into SKIP_COMPLEX_OPTOUT_H sections like the one below! [trf]
#ifndef SKIP_COMPLEX_OPTOUT_H

#include "base/povms.h"
#include <boost/thread.hpp>
#include <boost/function.hpp>

/**
 *	This function does essential initialisation that is required before
 *	POV-Ray can be used. It also starts the main render thread that
 *	receives and processes all messages received from the frontend.
 *	@param  addr  If not NULL, backend address on return.
 *  @return       Pointer to the thread resource created.
 */
boost::thread *povray_init(const boost::function0<void>& threadExit, POVMSAddress *addr = NULL);

/**
 *	This function shuts down the main render thread and after it has
 *	been called, all memory allocated by POV-Ray has been freed and
 *	all threads created by POV-Ray have been terminated.
 */
void povray_terminate();

/**
 *	Returns true if the main thread has terminated. It will return
 *	false if the main thread is not running because it has not yet
 *	been started.
 */
bool povray_terminated();

#endif // SKIP_COMPLEX_OPTOUT_H

#define DAYS(n)         (86400 * n)

// POV-Ray version and copyright message macros

#define POV_RAY_IS_OFFICIAL 1
#define POV_RAY_IS_RELEASE_CANDIDATE

#define POV_RAY_VERSION "3.7.0.RC4"
#define POV_RAY_COPYRIGHT "Copyright 1991-2003 Persistence of Vision Team\nCopyright 2003-2011 Persistence of Vision Raytracer Pty. Ltd."
#define OFFICIAL_VERSION_NUMBER 370
#define OFFICIAL_VERSION_NUMBER_HEX 0x0370

#if POV_RAY_IS_OFFICIAL == 1

#ifdef DISTRIBUTION_MESSAGE_2
#undef DISTRIBUTION_MESSAGE_2
#endif

#define DISTRIBUTION_MESSAGE_1 "This is a release candidate of POV-Ray version 3.7.0."
#define DISTRIBUTION_MESSAGE_2 "General distribution is strongly discouraged."
#define DISTRIBUTION_MESSAGE_3 ""

/*
#define DISTRIBUTION_MESSAGE_1 "This is an official version prepared by the POV-Ray Team. See the"
#define DISTRIBUTION_MESSAGE_2 " documentation on how to contact the authors or visit us on the"
#define DISTRIBUTION_MESSAGE_3 " internet at http://www.povray.org/."
*/

#else

// Please set DISTRIBUTION_MESSAGE_2 to your real name to make
// unofficial versions distinguishable from each other. We also
// recommend including an email address. Remove the #error directive
// afterwards to proceed with the build.

#define DISTRIBUTION_MESSAGE_1 "This is an unofficial version compiled by:"
#ifndef DISTRIBUTION_MESSAGE_2
#error You must complete the following DISTRIBUTION_MESSAGE_2 definition
#define DISTRIBUTION_MESSAGE_2 " FILL IN NAME HERE........................."
#endif
#define DISTRIBUTION_MESSAGE_3 " The POV-Ray Team(tm) is not responsible for supporting this version."

#endif

#endif // POVRAY_BACKEND_POVRAY_H
