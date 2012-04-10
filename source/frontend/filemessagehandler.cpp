/*******************************************************************************
 * filemessagehandler.cpp
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
 * $File: //depot/povray/smp/source/frontend/filemessagehandler.cpp $
 * $Revision: #18 $
 * $Change: 4715 $
 * $DateTime: 2009/02/21 10:00:39 $
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

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/types.h"
#include "base/path.h"

#include "frontend/configfrontend.h"
#include "frontend/renderfrontend.h"
#include "frontend/filemessagehandler.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_frontend
{

FileMessageHandler::FileMessageHandler()
{
}

FileMessageHandler::~FileMessageHandler()
{
}

void FileMessageHandler::HandleMessage(const SceneData& sd, POVMSType ident, POVMS_Object& msg, POVMS_Object& result)
{
	switch(ident)
	{
		case kPOVMsgIdent_FindFile:
			(void)FindFile(sd.searchpaths, msg, result);
			break;
		case kPOVMsgIdent_ReadFile:
			if(ReadFile(sd.searchpaths, msg, result) == true)
				sd.readfiles.push_back(result);
			break;
		case kPOVMsgIdent_CreatedFile:
			CreatedFile(msg);
			sd.createdfiles.push_back(msg);
			break;
	}
}

bool FileMessageHandler::FindFile(const list<Path>& lps, POVMS_Object& msg, POVMS_Object& result)
{
	POVMS_List files;
	Path path;

	msg.Get(kPOVAttrib_ReadFile, files);

	for(int i = 1; i <= files.GetListSize(); i++)
	{
		POVMS_Attribute attr;

		files.GetNth(i, attr);

		path = FindFilePath(lps, Path(attr.GetUCS2String()));

		if(path.Empty() == false)
			break;
	}

	result.SetUCS2String(kPOVAttrib_ReadFile, path().c_str());

	return (path.Empty() == false);
}

bool FileMessageHandler::ReadFile(const list<Path>& lps, POVMS_Object& msg, POVMS_Object& result)
{
	Path path(FindFilePath(lps, Path(msg.GetUCS2String(kPOVAttrib_ReadFile))));

	if(path.Empty() == false)
		result.SetUCS2String(kPOVAttrib_LocalFile, path().c_str());

	return (path.Empty() == false);
}

void FileMessageHandler::CreatedFile(POVMS_Object&)
{
}

Path FileMessageHandler::FindFilePath(const list<Path>& lps, const Path& f)
{
	// check the current working dir (or full path if supplied) first
	// note that if the file doesn't have a path and it is found in the
	// CWD, the CWD is not returned as part of the path.
	if(CheckIfFileExists(f) == true)
		return f;

	// if the path is absolute there's no point in checking the include paths;
	// given it wasn't found above we can just return an empty path
	if(f.HasVolume() == true)
		return Path();

	Path path(f);

	for(list<Path>::const_iterator i(lps.begin()); i != lps.end(); i++)
	{
		Path path (*i, f);

		if(CheckIfFileExists(path) == true)
			return path;
	}

	// not found so return empty path
	return Path();
}

}
