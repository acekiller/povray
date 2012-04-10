/*******************************************************************************
 * textstreambuffer.cpp
 *
 * This module contains the basic C++ text stream buffer.
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
 * $File: //depot/povray/smp/source/base/textstreambuffer.cpp $
 * $Revision: #17 $
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

#include <algorithm>
#include <cstring>

// configbase.h must always be the first POV file included within base *.cpp files
#include "configbase.h"

#include "textstreambuffer.h"

#include "povms.h"
#include "pov_err.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

TextStreamBuffer::TextStreamBuffer(size_t buffersize, unsigned int wrapwidth)
{
	boffset = 0;
	bsize = buffersize;
	wrap = wrapwidth;
	curline = 0;
	if(POVMSUtil_TempAlloc((void **)&buffer, bsize) != kNoErr)
		throw POV_EXCEPTION_CODE(kOutOfMemoryErr);
}

TextStreamBuffer::~TextStreamBuffer()
{
	boffset = 0;
	bsize = 0;
	wrap = 0;
	curline = 0;
	if(buffer != NULL)
		(void)POVMSUtil_TempFree((void *)buffer);
	buffer = NULL;
}

void TextStreamBuffer::printf(const char *format, ...)
{
	va_list marker;

	va_start(marker, format);
	vsnprintf(&buffer[boffset], bsize - boffset - 1, format, marker);
	va_end(marker);

	// direct output
	directflush(&buffer[boffset], strlen(&buffer[boffset]));

	boffset = strlen(buffer);

	// line buffered output
	lineflush();
}

void TextStreamBuffer::print(const char *str)
{
	printf("%s", str);
}

void TextStreamBuffer::puts(const char *str)
{
	printf("%s\n", str);
}

void TextStreamBuffer::putc(int chr)
{
	printf("%c", chr);
}

void TextStreamBuffer::printfile(const char *filename, POV_LONG offset, POV_LONG lines)
{
	FILE *file = fopen(filename, "r");

	if(file != NULL)
	{
		fseek(file, offset, SEEK_SET);
		printfile(file, lines);
		fclose(file);
	}
}

void TextStreamBuffer::printfile(FILE *file, POV_LONG lines)
{
	if(file != NULL)
	{
		bool stopposset = (lines < 0); // only if walking backwards stop at current position
		POV_LONG stoppos = (POV_LONG)(ftell(file));
		int chr = 0;

		if(lines < 0)
		{
			POV_LONG lineoffset = lines;

			// NOTE: This will walk back one line too far! However, it only walks
			// back to the end of that line. Thus, the next step will walk forward
			// again to the beginning of the right line, which is the desired
			// position. Do not change this behavior without testing! [trf]
			for(POV_LONG pos = (POV_LONG)(ftell(file)) - 1; (lineoffset < 1) && (pos >= 0); pos--)
			{
				// WARNING: Expensive way to walk backward through a file, but will only
				// be used when problems are encountered anyway, and then it most likely
				// does not matter if the output of the message takes a tiny bit longer!
				fseek(file, pos, SEEK_SET);

				chr = fgetc(file);

				if((chr == 10) || (chr == 13))
				{
					chr = fgetc(file);
					if(!((chr == 10) || (chr == 13)))
						ungetc(chr, file);
					lineoffset++;
				}
				else if(chr == EOF)
					break;
			}

			// beginning of file was previously reached
			if(lineoffset < 1)
				fseek(file, 0, SEEK_SET);

			while(lineoffset > 0)
			{
				chr = fgetc(file);

				if((chr == 10) || (chr == 13))
				{
					chr = fgetc(file);
					if(!((chr == 10) || (chr == 13)))
						ungetc(chr, file);
					lineoffset--;
				}
				else if(chr == EOF)
					break;
			}

			// make number of lines to output positive for next step
			lines = -lines;
		}

		while(lines > 0)
		{
			chr = fgetc(file);

			if((stopposset == true) && (stoppos == ((POV_LONG)(ftell(file)) - 1))) // only if walking backwards stop at initial position
				break;

			// count newlines in file and replace newlines with system specific newline charcater
			if((chr == 10) || (chr == 13))
			{
				chr = fgetc(file);
				if(!((chr == 10) || (chr == 13)))
					ungetc(chr, file);
				else
				{
					if((stopposset == true) && (stoppos == ((POV_LONG)(ftell(file)) - 1))) // only if walking backwards stop at initial position
						break;
				}
				printf("\n");
				lines--;
			}
			else if(chr == EOF)
				break;
			else
				printf("%c", chr);
		}
	}
}

void TextStreamBuffer::flush()
{
	if(curline > 0)
		directoutput("\n", 1);
	curline = 0;

	lineflush();
	if(boffset > 0)
		lineoutput(buffer, boffset);
	boffset = 0;
}

void TextStreamBuffer::lineoutput(const char *str, unsigned int chars)
{
	// by default output to stdout
	fwrite(str, sizeof(char), chars, stdout);
	fprintf(stdout, "\n");
	fflush(stdout);
}

void TextStreamBuffer::directoutput(const char *, unsigned int)
{
	// does nothing by default
}

void TextStreamBuffer::rawoutput(const char *, unsigned int)
{
	// does nothing by default
}

void TextStreamBuffer::lineflush()
{
	unsigned int lasti = 0;
	unsigned int ii = 0;
	unsigned int i = 0;

	// output all complete lines in the buffer
	while(i < boffset)
	{
		if((buffer[i] == '\n') || (buffer[i] == '\r'))
		{
			lineoutput(&buffer[lasti], i - lasti);
			lasti = i + 1;
		}
		else if(i - lasti >= wrap)
		{
			// track back to last space up to 1/4 in the line to wrap
			for(ii = 0; ii < min((wrap / 4), i); ii++)
			{
				if(isspace(buffer[i - ii]))
					break;
			}

			// if no space was found in the last 1/4 of the line to wrap, split it at the end anyway
			if(ii == min((wrap / 4), i))
				ii = 0;
			i -= ii;

			lineoutput(&buffer[lasti], i - lasti);
			lasti = i;
			continue;
		}
		i++;
	}

	if(lasti > 0)
	{
		// remove all completely output lines
		boffset -= lasti;
		memmove(buffer, &buffer[lasti], boffset);
	}
}

void TextStreamBuffer::directflush(const char *str, unsigned int chars)
{
	unsigned int ii = 0;
	unsigned int i = 0;

	rawoutput(str, chars);

	for(i = 0; i < chars; i++)
	{
		if((str[i] == '\n') || (str[i] == '\r'))
		{
			i++;
			directoutput(str, i);
			str += i;
			chars -= i;
			i = 0;
			curline = 0;
		}
		else if(curline + i >= wrap)
		{
			// track back to last space up to 1/4 in the line to wrap
			for(ii = 0; ii < min((wrap / 4), i); ii++)
			{
				if(isspace(str[i - ii]))
					break;
			}

			// if no space was found in the last 1/4 of the line to wrap, split it at the end anyway
			if(ii == min((wrap / 4), i))
				ii = 0;
			i -= ii;

			directoutput(str, i);
			directoutput("\n", 1);

			str += i;
			chars -= i;
			i = 0;
			curline = 0;
		}
	}

	if(chars > 0)
	{
		directoutput(str, chars);
		curline += chars;
	}
}

}
