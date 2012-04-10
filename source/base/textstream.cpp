/*******************************************************************************
 * textstream.cpp
 *
 * This module implements the classes handling text file input and output.
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
 * $File: //depot/povray/smp/source/base/textstream.cpp $
 * $Revision: #19 $
 * $Change: 5446 $
 * $DateTime: 2011/06/02 08:31:56 $
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
#include <algorithm>

// configbase.h must always be the first POV file included within base *.cpp files
#include "configbase.h"

#include "textstream.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

ITextStream::ITextStream(const UCS2 *sname, unsigned int stype)
{
	if(sname == NULL)
		throw POV_EXCEPTION_CODE(kParamErr);

	stream = NewIStream(sname, stype);
	if(stream == NULL)
		throw POV_EXCEPTION(kCannotOpenFileErr, string("Cannot open file '") + UCS2toASCIIString(sname) + "' for input.");

	filename = UCS2String(sname);
	lineno = 1;
	bufferoffset = 0;
	maxbufferoffset = 0;
	filelength = 0;
	ungetbuffer = EOF;
	curpos = 0 ;

	stream->seekg(0, IOBase::seek_end);
	filelength = stream->tellg();
	stream->seekg(0, IOBase::seek_set);

	RefillBuffer();
}

ITextStream::ITextStream(const UCS2 *sname, IStream *sstream)
{
	if(sname == NULL)
		throw POV_EXCEPTION_CODE(kParamErr);
	if(sstream == NULL)
		throw POV_EXCEPTION_CODE(kParamErr);

	stream = sstream;
	filename = UCS2String(sname);
	lineno = 1;
	bufferoffset = 0;
	maxbufferoffset = 0;
	filelength = 0;
	ungetbuffer = EOF;
	curpos = 0 ;

	stream->seekg(0, IOBase::seek_end);
	filelength = stream->tellg();
	stream->seekg(0, IOBase::seek_set);

	RefillBuffer();
}

ITextStream::~ITextStream()
{
	delete stream;
	stream = NULL;
}

int ITextStream::getchar()
{
	int chr = 0;

	if(ungetbuffer != EOF)
	{
		chr = ungetbuffer;
		ungetbuffer = EOF;
	}
	else
	{
		if(bufferoffset >= maxbufferoffset)
			chr = EOF;
		else
		{
			chr = buffer[bufferoffset];
			bufferoffset++;
		}
	}

	if(((chr == 10) || (chr == 13)) && (bufferoffset >= maxbufferoffset))
		RefillBuffer();

	if(chr == 10)
	{
		if(buffer[bufferoffset] == 13)
			bufferoffset++;
		chr = '\n';
		lineno++;
	}
	else if(chr == 13)
	{
		if(buffer[bufferoffset] == 10)
			bufferoffset++;
		chr = '\n';
		lineno++;
	}

	if(bufferoffset >= maxbufferoffset)
		RefillBuffer();

	return chr;
}

void ITextStream::ungetchar(int chr)
{
	ungetbuffer = chr;
	if(chr == '\n')
		lineno--;
}

bool ITextStream::eof()
{
	if(ungetbuffer != EOF)
		return false;
	if(bufferoffset >= maxbufferoffset)
		return true;
	return stream->eof();
}

bool ITextStream::seekg(ITextStream::FilePos fp)
{
	bool result = true;

	if((fp.offset < curpos) && ((curpos - fp.offset) < maxbufferoffset))
	{
		bufferoffset = maxbufferoffset - (curpos - fp.offset);
		lineno = fp.lineno;
		ungetbuffer = EOF;
	}
	else
	{
		result = (stream->seekg(fp.offset) != 0);

		if(result == true)
		{
			lineno = fp.lineno;

			bufferoffset = 0;
			maxbufferoffset = 0;
			ungetbuffer = EOF;
			curpos = fp.offset ;

			RefillBuffer();
		}
		else
			curpos = stream->tellg() ;
	}

	return result;
}

ITextStream::FilePos ITextStream::tellg()
{
	FilePos fp;

	fp.lineno = lineno;
	fp.offset = curpos - (maxbufferoffset - bufferoffset);

	if(ungetbuffer != EOF)
		fp.offset--;

	return fp;
}

void ITextStream::RefillBuffer()
{
	if(bufferoffset < maxbufferoffset)
	{
		curpos -= (maxbufferoffset - bufferoffset);
		stream->seekg(curpos, IOBase::seek_set);
	}

	maxbufferoffset = min((POV_ULONG)ITEXTSTREAM_BUFFER_SIZE, filelength - curpos);
	bufferoffset = 0;

	stream->read(buffer, maxbufferoffset);
	if (*stream)
		curpos += maxbufferoffset ;
	else
		curpos = stream->tellg() ;
}

OTextStream::OTextStream(const UCS2 *sname, unsigned int stype, bool append)
{
	if(sname == NULL)
		throw POV_EXCEPTION_CODE(kParamErr);

	stream = NewOStream(sname, stype, append);
	if(stream == NULL)
		throw POV_EXCEPTION(kCannotOpenFileErr, string("Cannot open file '") + UCS2toASCIIString(sname) + "' for output.");

	filename = UCS2String(sname);
}

OTextStream::OTextStream(const UCS2 *sname, OStream *sstream)
{
	if(sname == NULL)
		throw POV_EXCEPTION_CODE(kParamErr);
	if(sstream == NULL)
		throw POV_EXCEPTION_CODE(kParamErr);

	stream = sstream;
	filename = UCS2String(sname);
}

OTextStream::~OTextStream()
{
	delete stream;
	stream = NULL;
}

void OTextStream::putchar(int chr)
{
#ifdef TEXTSTREAM_CRLF
	if (chr == '\n')
		stream->Write_Byte('\r');
#endif

	stream->Write_Byte((unsigned char)chr);
}

void OTextStream::putraw(int chr)
{
	stream->Write_Byte((unsigned char)chr);
}

void OTextStream::printf(const char *format, ...)
{
	va_list marker;
	char buffer[1024];

	va_start(marker, format);
	vsnprintf(buffer, 1023, format, marker);
	va_end(marker);

#ifdef TEXTSTREAM_CRLF
	char *s1 = buffer ;
	char *s2 ;

	while ((s2 = strchr (s1, '\n')) != NULL)
	{
		*s2++ = '\0' ;
		stream->printf("%s\r\n", s1);
		s1 = s2 ;
	}
	if (*s1)
		stream->printf("%s", s1);
#else
	stream->printf("%s", buffer);
#endif
}

void OTextStream::flush()
{
	stream->flush();
}

}
