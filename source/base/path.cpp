/*******************************************************************************
 * path.cpp
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
 * $File: //depot/povray/smp/source/base/path.cpp $
 * $Revision: #15 $
 * $Change: 5449 $
 * $DateTime: 2011/06/02 10:41:38 $
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

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/types.h"
#include "base/path.h"
#include "base/pov_err.h"

#ifdef USE_SYSPROTO
#include "syspovprotobase.h" // TODO FIXME - need to resolve structural dependencies between config.h, configbase.h, frame.h and sysproto.h
#endif

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

const char *BIN2HEX = "0123456789ABCDEF";

Path::Path()
{
}

Path::Path(const char *p, Encoding e)
{
	if(e == ASCII)
		ParsePathString(ASCIItoUCS2String(p));
	else
		ParsePathString(URLToUCS2String(p));
}

Path::Path(const string& p, Encoding e)
{
	if(e == ASCII)
		ParsePathString(ASCIItoUCS2String(p.c_str()));
	else
		ParsePathString(URLToUCS2String(p));
}

Path::Path(const UCS2 *p)
{
	ParsePathString(UCS2String(p));
}

Path::Path(const UCS2String& p)
{
	ParsePathString(p);
}

Path::Path(const Path& p1, const Path& p2)
{
	// TODO: Handle case p2.HasVolume()==true

	vector<UCS2String> f1(p1.GetAllFolders());
	vector<UCS2String> f2(p2.GetAllFolders());

	volume = p1.GetVolume();

	for(vector<UCS2String>::iterator i(f1.begin()); i != f1.end(); i++)
		folders.push_back(*i);
	for(vector<UCS2String>::iterator i(f2.begin()); i != f2.end(); i++)
		folders.push_back(*i);

	file = p2.GetFile();
}

bool Path::operator==(const Path& p) const
{
	if(volume != p.GetVolume())
		return false;

	if(folders != p.GetAllFolders())
		return false;

	if(file != p.GetFile())
		return false;

	return true;
}

bool Path::operator!=(const Path& p) const
{
	if(volume != p.GetVolume())
		return true;

	if(folders != p.GetAllFolders())
		return true;

	if(file != p.GetFile())
		return true;

	return false;
}

UCS2String Path::operator()() const
{
	UCS2String p;

	p += volume;

	for(vector<UCS2String>::const_iterator i(folders.begin()); i != folders.end(); i++)
	{
		p += *i;
		p += POV_FILE_SEPARATOR;
	}

	p += file;

	return p;
}

bool Path::HasVolume() const
{
	return (volume.length() > 0);
}

UCS2String Path::GetVolume() const
{
	return volume;
}

UCS2String Path::GetFolder() const
{
	if(folders.empty() == false)
		return folders.back();
	else
		return UCS2String();
}

vector<UCS2String> Path::GetAllFolders() const
{
	return folders;
}

UCS2String Path::GetFile() const
{
	return file;
}

void Path::SetVolume(const char *p)
{
	volume = ASCIItoUCS2String(p);
}

void Path::SetVolume(const string& p)
{
	volume = ASCIItoUCS2String(p.c_str());
}

void Path::SetVolume(const UCS2 *p)
{
	volume = UCS2String(p);
}

void Path::SetVolume(const UCS2String& p)
{
	volume = p;
}

void Path::AppendFolder(const char *p)
{
	folders.push_back(ASCIItoUCS2String(p));
}

void Path::AppendFolder(const string& p)
{
	folders.push_back(ASCIItoUCS2String(p.c_str()));
}

void Path::AppendFolder(const UCS2 *p)
{
	folders.push_back(UCS2String(p));
}

void Path::AppendFolder(const UCS2String& p)
{
	folders.push_back(p);
}

void Path::RemoveFolder()
{
	folders.pop_back();
}

void Path::RemoveAllFolders()
{
	folders.clear();
}

void Path::SetFile(const char *p)
{
	file = ASCIItoUCS2String(p);
}

void Path::SetFile(const string& p)
{
	file = ASCIItoUCS2String(p.c_str());
}

void Path::SetFile(const UCS2 *p)
{
	file = UCS2String(p);
}

void Path::SetFile(const UCS2String& p)
{
	file = p;
}

void Path::Clear()
{
	volume.clear();
	folders.clear();
	file.clear();
}

bool Path::Empty() const
{
	return (volume.empty() && folders.empty() && file.empty());
}

void Path::ParsePathString(const UCS2String& p)
{
	if(POV_PARSE_PATH_STRING(p, volume, folders, file) == false)
		throw POV_EXCEPTION_STRING("Invalid path."); // TODO FIXME - properly report path string [trf]
}

UCS2String Path::URLToUCS2String(const char *p) const
{
	return URLToUCS2String(string(p));
}

UCS2String Path::URLToUCS2String(const string& p) const
{
	return ASCIItoUCS2String(p.c_str()); // TODO FIXME
}

}
