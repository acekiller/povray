/*******************************************************************************
 * fileinputoutput.cpp
 *
 * This module implements the classes handling file input and output.
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
 * $File: //depot/povray/smp/source/base/fileinputoutput.cpp $
 * $Revision: #32 $
 * $Change: 5094 $
 * $DateTime: 2010/08/07 06:03:14 $
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

#include <cstdlib>
#include <cstdarg>
#include <cstring>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"

#include "base/fileinputoutput.h"
#include "base/stringutilities.h"
#include "base/platformbase.h"
#include "base/pointer.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

IOBase::IOBase(unsigned int dir, unsigned int type)
{
	filetype = type;
	direction = dir;
	fail = true;
	f = NULL;
}

IOBase::~IOBase()
{
	close();
}

bool IOBase::open(const UCS2String& name, unsigned int Flags /* = 0 */)
{
	char mode[8];

	close();
	filename = name;

	if((Flags & append) == 0)
	{
		switch(direction)
		{
			case input:
				strcpy(mode, "r");
				break;
			case output:
				strcpy(mode, "w");
				break;
			case io:
				strcpy(mode, "w+");
				break;
			default:
				return false;
		}
	}
	else
	{
		// we cannot use append mode here, since "a" mode is totally incompatible with any
		// output file format that requires in-place updates(i.e. writing to any location
		// other than the end of the file). BMP files are in this category. In theory, "r+"
		// can do anything "a" can do(with appropriate use of seek()) so append mode should
		// not be needed.
		strcpy(mode, "r+");
	}

	strcat(mode, "b");

	f = NULL;
	if(pov_stricmp(UCS2toASCIIString(name).c_str(), "stdin") == 0)
	{
		if(direction != input ||(Flags & append) != 0)
			return false;
		f = stdin;
	}
	else if(pov_stricmp(UCS2toASCIIString(name).c_str(), "stdout") == 0)
	{
		if(direction != output ||(Flags & append) != 0)
			return false;
		f = stdout;
	}
	else if(pov_stricmp(UCS2toASCIIString(name).c_str(), "stderr") == 0)
	{
		if(direction != output ||(Flags & append) != 0)
			return false;
		f = stderr;
	}
	else
	{
		if((f = POV_UCS2_FOPEN(name, mode)) == NULL)
		{
			if((Flags & append) == 0)
				return false;

			// to maintain traditional POV +c(continue) mode compatibility, if
			// the open for append of an existing file fails, we allow a new file
			// to be created.
			mode [0] = 'w';
			if((f = POV_UCS2_FOPEN(name, mode)) == NULL)
				return false;
		}
	}
	fail = false;

	if((Flags & append) != 0)
	{
		if(!seekg(0, seek_end))
		{
			close();
			return false;
		}
	}

	return true;
}

bool IOBase::close(void)
{
	if(f != NULL)
	{
		if (f != stdout && f != stderr && f != stdin)
			fclose(f);
		f = NULL;
	}
	fail = true;
	return true;
}

IOBase& IOBase::flush(void)
{
	if(f != NULL)
		fflush(f);
	return *this;
}

IOBase& IOBase::read(void *buffer, size_t count)
{
	if(!fail && count > 0)
		fail = fread(buffer, count, 1, f) != 1;
	return *this;
}

IOBase& IOBase::write(void *buffer, size_t count)
{
	if(!fail && count > 0)
		fail = fwrite(buffer, count, 1, f) != 1;
	return *this;
}

// Strictly speaking, this should -not- be called seekg, since 'seekg'(an iostreams
// term) applies only to an input stream, and therefore the use of this name here
// implies that only the input stream will be affected on streams opened for I/O
//(which is not the case with fseek, since fseek moves the pointer for output too).
// However, the macintosh code seems to need it to be called seekg, so it is ...
IOBase& IOBase::seekg(POV_LONG pos, unsigned int whence /* = seek_set */)
{
	if(!fail)
		fail = fseek(f, pos, whence) != 0;
	return *this;
}

IStream::IStream(const unsigned int stype) : IOBase(input, stype)
{
}

IStream::~IStream()
{
}

int IStream::Read_Short(void)
{
	short result;
	read((char *) &result, sizeof(short));
	return result;
}

int IStream::Read_Int(void)
{
	int result;
	read((char *) &result, sizeof(int));
	return result;
}

IStream& IStream::UnRead_Byte(int c)
{
	if(!fail)
		fail = ungetc(c, f) != c;
	return *this;
}

IStream& IStream::getline(char *s, size_t buflen)
{
	int chr = 0;

	if(feof(f) != 0)
		fail = true;

	if(!fail && buflen > 0)
	{
		while(buflen > 1)
		{
			chr = fgetc(f);
			if(chr == EOF)
				break;
			else if(chr == 10)
			{
				chr = fgetc(f);
				if(chr != 13)
					ungetc(chr, f);
				break;
			}
			else if(chr == 13)
			{
				chr = fgetc(f);
				if(chr != 10)
					ungetc(chr, f);
				break;
			}
			*s = chr;
			s++;
			buflen--;
		}
		*s = 0;
	}

	return *this;
}

OStream::OStream(const unsigned int stype) : IOBase(output, stype)
{
}

OStream::~OStream()
{
}

void OStream::printf(const char *format, ...)
{
	va_list marker;
	char buffer[1024];

	va_start(marker, format);
	vsnprintf(buffer, 1023, format, marker);
	va_end(marker);

	*this << buffer;
}

IStream *NewIStream(const Path& p, const unsigned int stype)
{
	Pointer<IStream> istreamptr(POV_PLATFORM_BASE.CreateIStream(stype));

	if(istreamptr == NULL)
		return NULL;

	if (POV_ALLOW_FILE_READ(p().c_str(), stype) == false) // TODO FIXME - this is handled by the frontend, but that code isn't completely there yet [trf]
	{
		string str ("IO Restrictions prohibit read access to '") ;
		str += UCS2toASCIIString(p());
		str += "'";
		throw POV_EXCEPTION(kCannotOpenFileErr, str);
	}
	if(istreamptr->open(p().c_str()) == 0)
		return NULL;

	return istreamptr.release();
}

OStream *NewOStream(const Path& p, const unsigned int stype, const bool sappend)
{
	Pointer<OStream> ostreamptr(POV_PLATFORM_BASE.CreateOStream(stype));
	unsigned int Flags = IOBase::none;

	if(ostreamptr == NULL)
		return NULL;

	if(sappend)
		Flags |= IOBase::append;

	if (POV_ALLOW_FILE_WRITE(p().c_str(), stype) == false) // TODO FIXME - this is handled by the frontend, but that code isn't completely there yet [trf]
	{
		string str ("IO Restrictions prohibit write access to '") ;
		str += UCS2toASCIIString(p());
		str += "'";
		throw POV_EXCEPTION(kCannotOpenFileErr, str);
	}
	if(ostreamptr->open(p().c_str(), Flags) == 0)
		return NULL;

	return ostreamptr.release();
}

UCS2String GetFileExtension(const Path& p)
{
	UCS2String::size_type pos = p.GetFile().find_last_of('.');

	if(pos != UCS2String::npos)
		return UCS2String(p.GetFile(), pos);

	return UCS2String();
}

UCS2String GetFileName(const Path& p)
{
	UCS2String::size_type pos = p.GetFile().find_last_of('.');

	if(pos != UCS2String::npos)
		return UCS2String(p.GetFile(), 0, pos);

	return p.GetFile();
}

bool CheckIfFileExists(const Path& p)
{
	FILE *tempf = POV_UCS2_FOPEN(p().c_str(), "r");

	if(tempf != NULL)
		fclose(tempf);
	else
		return false;

	return true;
}

POV_LONG GetFileLength(const Path& p)
{
	FILE *tempf = POV_UCS2_FOPEN(p().c_str(), "rb");
	POV_LONG result = -1;

	if(tempf != NULL)
	{
		fseek(tempf, 0, SEEK_END);
		result = ftell(tempf);
		fclose(tempf);
	}

	return result;
}

}
