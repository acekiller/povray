/*******************************************************************************
 * pov_err.cpp
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
 * $File: //depot/povray/smp/source/base/pov_err.cpp $
 * $Revision: #20 $
 * $Change: 5230 $
 * $DateTime: 2010/12/03 19:54:28 $
 * $Author: jholsenback $
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

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/povmscpp.h"
#include "base/pov_err.h"
#include <cstring>

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

std::string Exception::lookup_code(int err, const char *file, unsigned int line)
{
	char	str[256];

	// skip over part of filename, if present, to make output more compact.
	if (file != NULL && (strncmp(file, "../../source/", 13) == 0 || strncmp(file, "..\\..\\source\\", 13) == 0))
		file += 13;

	switch(err)
	{
		case kNoErr:
			if (file == NULL)
				return "(Failed to determine error: no code found.)";
			sprintf(str, "(Failed to determine error: no code in exception thrown at %s line %d.)", file, line);
			return std::string(str);

		case kParamErr:
			return "Invalid parameter.";

		case kMemFullErr:
			if (file == NULL)
				return "Out of memory.";
			sprintf(str, "Memory allocation failure exception thrown at %s line %d.", file, line);
			return std::string(str);

		case kInvalidDataSizeErr:
			return "Invalid data size.";

		case kCannotHandleDataErr:
			return "Cannot handle data of this type or kind.";

		case kNullPointerErr:
			return "Unexpected null pointer.";

		case kChecksumErr:
			return "Corrupted data.";

		case kParseErr:
			return "Cannot parse input.";

		case kCannotOpenFileErr:
			return "Cannot open file.";

		case kInvalidDestAddrErr:
			return "Invalid destination address.";

		case kCannotConnectErr:
			return "Cannot establish connection.";

		case kDisconnectedErr:
			return "Disconnection initiated from client.";

		case kHostDisconnectedErr:
			return "Disconnection initiated from host.";

		case kNetworkDataErr:
			return "Network data transmission failed.";

		case kNetworkConnectionErr:
			return "Network connection failed";

		case kObjectAccessErr:
			return "Cannot access object.";

		case kVersionErr:
			return "Invalid version.";

		case kFileDataErr:
			return "Cannot access data in file.";

		case kAuthorisationErr:
			return "Unauthorised access";

		case kDataTypeErr:
			return "Data type or kind does not match.";

		case kTimeoutErr:
			return "Operation timed out.";

		case kInvalidContextErr:
			return "Context is invalid.";

		case kIncompleteDataErr:
			return "Cannot handle data because it is incomplete.";

		case kInvalidIdentifierErr:
			return "Invalid identifier.";

		case kCannotHandleRequestErr:
			return "Cannot handle request.";

		case kFalseErr:
			return "Result was false but true was expected.";

		case kOutOfSyncErr:
			return "Cannot perform this operation now.";

		case kQueueFullErr:
			return "Queue is full.";

		case kUserAbortErr:
			return "Frontend halted render.";

		case kImageAlreadyRenderedErr:
			return "Image already rendered.";

		case kAccessViolationErr:
			return "Memory access violation.";

		case kDivideByZeroErr:
			return "Divide by zero.";

		case kStackOverflowErr:
			return "Stack overflow.";

		case kNativeExceptionErr:
			return "Native operating-system exception.";

		case kInternalLimitErr:
			return "A POV-Ray internal nesting limit was reached.";

		case kUncategorizedError:
			if (file == NULL)
				return "Uncategorized error.";
			sprintf(str, "Uncategorized error thrown at %s line %d.", file, line);
			return std::string(str);
	}

	// default
	sprintf(str, "(Failed to determine error: unidentified code %d in exception thrown at %s line %d. Please report this.)", err, file, line);
	return std::string(str);
}

}
