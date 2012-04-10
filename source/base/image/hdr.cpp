/*******************************************************************************
 * hdr.cpp
 *
 * This module contains the code to read and write files in Radiance HDRI format
 * (sometimes known as 'RGBE' format).
 *
 * Author: Christopher Cason
 * Based on MegaPOV HDR code written by Mael and Christoph Hormann
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
 * $File: //depot/povray/smp/source/base/image/hdr.cpp $
 * $Revision: #23 $
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

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

#include <string>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/fileinputoutput.h"
#include "base/image/hdr.h"
#include "base/types.h"

#include "metadata.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace HDR
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define  MINELEN        8       /* minimum scanline length for encoding */
#define  MAXELEN        0x7fff  /* maximum scanline length for encoding */
#define  MINRUN         4       /* minimum run length */

/*****************************************************************************
* Local typedefs
******************************************************************************/

struct Messages
{
	vector<string> warnings;
	string error;
};

typedef unsigned char RGBE[4]; // red, green, blue, exponent

void GetRGBE(RGBE rgbe, const Image *image, int col, int row, const GammaCurvePtr& gamma, DitherHandler* dither);
void SetRGBE(unsigned char *scanline, Image *image, int row, int width, const GammaCurvePtr& gamma);
void ReadOldLine(unsigned char *scanline, int width, IStream *file);

/*****************************************************************************
* Code
******************************************************************************/
void GetRGBE(RGBE rgbe, const Image *image, int col, int row, const GammaCurvePtr& gamma, DitherHandler* dh)
{
	float r, g, b, d;
	int e;

	DitherHandler::OffsetInfo linOff, encOff;
	dh->getOffset(col,row,linOff,encOff);

	GetEncodedRGBValue(image, col, row, gamma, r, g, b);
	r += linOff.red;
	g += linOff.green;
	b += linOff.blue;

	if((d = max3(r, g, b)) <= 1.0e-32)
	{
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
		return;
	}

	d = frexp(d, &e) * 256.0 / d;

	rgbe[0] = (unsigned char)(clip(r * d + encOff.red,   0.0f, 255.0f));
	rgbe[1] = (unsigned char)(clip(g * d + encOff.green, 0.0f, 255.0f));
	rgbe[2] = (unsigned char)(clip(b * d + encOff.blue,  0.0f, 255.0f));
	rgbe[3] = (unsigned char)(clip(e + 128.0f, 0.0f, 255.0f));

	linOff.red   = r - (double(rgbe[0]) + 0.5) / d;
	linOff.green = g - (double(rgbe[1]) + 0.5) / d;
	linOff.blue  = b - (double(rgbe[2]) + 0.5) / d;
	dh->setError(col,row,linOff);
}

void SetRGBE(unsigned char *scanline, Image *image, int row, int width, const GammaCurvePtr& gamma)
{
	for(int i = 0; i < width; i++)
	{
		double v = ldexp(1.0, int(scanline[3]) - (128 + 8));
		float r = (double(scanline[0]) + 0.5) * v;
		float g = (double(scanline[1]) + 0.5) * v;
		float b = (double(scanline[2]) + 0.5) * v;

		SetEncodedRGBValue(image, i, row, gamma, r, g, b);
		scanline += 4;
	}
}

void ReadOldLine(unsigned char *scanline, int width, IStream *file)
{
	int rshift = 0;
	unsigned char b;

	while(width > 0)
	{
		scanline[0] = file->Read_Byte();
		scanline[1] = file->Read_Byte();
		scanline[2] = file->Read_Byte();

		// NB EOF won't be set at this point even if the last read obtained the
		// final byte in the file (we need to read another byte for that to happen).
		if(*file == false)
			throw POV_EXCEPTION(kFileDataErr, "Invalid HDR file (unexpected EOF)");

		if(file->Read_Byte(b).eof())
			return;

		scanline[3] = b;

		if((scanline[0] == 1) && (scanline[1] == 1) && (scanline[2] == 1))
		{
			for(int i = scanline[3] << rshift; i > 0; i--)
			{
				memcpy(scanline, scanline - 4, 4);
				scanline += 4;
				width--;
			}
			rshift += 8;
		}
		else
		{
			scanline += 4;
			width--;
			rshift = 0;
		}
	}
}

Image *Read(IStream *file, const Image::ReadOptions& options)
{
	char line[2048];
	char *s;
	char s1[3];
	char s2[3];
	unsigned char b;
	unsigned char val;
	float e;
	float exposure = 1.0;
	unsigned int width;
	unsigned int height;
	Image *image = NULL;
	Image::ImageDataType imagetype = options.itype;

	// Radiance HDR files store linear color values by default, so never convert unless the user overrides
	// (e.g. to handle a non-compliant file).
	GammaCurvePtr gamma;
	if (options.gammacorrect)
	{
		if (options.gammaOverride)
			gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);
		else
			gamma = TranscodingGammaCurve::Get(options.workingGamma, NeutralGammaCurve::Get());
	}

	while(*file)
	{
		if((file->getline(line, sizeof(line)) == false) || (line[0] == '-') || (line[0] == '+'))
			break;

		// TODO: what do we do with exposure?
		if(strncmp(line, "EXPOSURE", 8) == 0)
		{
			if((s = strchr(line, '=')) != NULL)
			{
				if(sscanf(s + 1, "%f", &e) == 1)
					exposure *= e;
			}
		}
	}

	if(sscanf(line, "%2[+-XY] %d %2[+-XY] %d\n", s1, &height, s2, &width) != 4)
		throw POV_EXCEPTION(kFileDataErr, "Bad HDR file header");

	if(imagetype == Image::Undefined)
		imagetype = Image::RGBFT_Float;

	image = Image::Create(width, height, imagetype);
	// NB: HDR files don't use alpha, so premultiplied vs. non-premultiplied is not an issue

	boost::scoped_array<unsigned char> scanline(new unsigned char[4 * width]);
	for(int row = 0; row < height; row++)
	{
		// determine scanline type
		if((width < MINELEN) | (width > MAXELEN))
		{
			ReadOldLine(scanline.get(), width, file);
			SetRGBE(scanline.get(), image, row, width, gamma);
			continue;
		}

		if(file->Read_Byte(b) == false)
			throw POV_EXCEPTION(kFileDataErr, "Incomplete HDR file");

		if(b != 2)
		{
			file->UnRead_Byte(b);

			ReadOldLine(scanline.get(), width, file);
			SetRGBE(scanline.get(), image, row, width, gamma);
			continue;
		}

		scanline[1] = file->Read_Byte();
		scanline[2] = file->Read_Byte();

		if(file->Read_Byte(b) == false)
			throw POV_EXCEPTION(kFileDataErr, "Incomplete or invalid HDR file");

		if((scanline[1] != 2) || ((scanline[2] & 128) != 0))
		{
			scanline[0] = 2;
			scanline[3] = b;

			ReadOldLine(scanline.get() + 4, width - 1, file);
			SetRGBE(scanline.get(), image, row, width, gamma);
			continue;
		}

		if((((int) scanline[2] << 8) | b) != width)
			throw POV_EXCEPTION(kFileDataErr, "Invalid HDR file (length mismatch)");

		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < width; )
			{
				if(file->Read_Byte(b) == false)
					throw POV_EXCEPTION(kFileDataErr, "Invalid HDR file (unexpected EOF)");

				if(b > 128)
				{
					// run
					b &= 127;

					if(file->Read_Byte(val) == false)
						throw POV_EXCEPTION(kFileDataErr, "Invalid HDR file (unexpected EOF)");

					while(b--)
						scanline[j++ * 4 + i] = (unsigned char) val;
				}
				else
				{
					while(b--)
					{
						if(file->Read_Byte(val) == false)
							throw POV_EXCEPTION(kFileDataErr, "Invalid HDR file (unexpected EOF)");

						scanline[j++ * 4 + i] = (unsigned char) val;
					}
				}
			}
		}

		SetRGBE(scanline.get(), image, row, width, gamma);
	}

	return image;
}

void Write(OStream *file, const Image *image, const Image::WriteOptions& options)
{
	int width = image->GetWidth();
	int height = image->GetHeight();
	int cnt = 1;
	int c2;
	RGBE rgbe;
	GammaCurvePtr gamma = TranscodingGammaCurve::Get(options.workingGamma, NeutralGammaCurve::Get());
	DitherHandler* dither = options.dither.get();

	Metadata meta;
	file->printf("#?RADIANCE\n");
	file->printf("SOFTWARE=%s\n", meta.getSoftware().c_str());
	file->printf("CREATION_TIME=%s\n" ,meta.getDateTime().c_str());
	if (!meta.getComment1().empty())
		file->printf("COMMENT=%s\n", meta.getComment1().c_str());
	if (!meta.getComment2().empty())
		file->printf("COMMENT=%s\n", meta.getComment2().c_str());
	if (!meta.getComment3().empty())
		file->printf("COMMENT=%s\n", meta.getComment3().c_str());
	if (!meta.getComment4().empty())
		file->printf("COMMENT=%s\n", meta.getComment4().c_str());

	file->printf("FORMAT=32-bit_rle_rgbe\n");
	file->printf("\n");
	file->printf("-Y %d +X %d\n", height, width);

	boost::scoped_array<RGBE> scanline(new RGBE[width]);

	for(int row = 0; row < height; row++)
	{
		if((width < MINELEN) | (width > MAXELEN))
		{
			for(int col = 0; col < width; col++)
			{
				GetRGBE(rgbe, image, col, row, gamma, dither);

				if(file->write(&rgbe, sizeof(RGBE)) == false)
					throw POV_EXCEPTION(kFileDataErr, "Failed to write data to HDR file");
			}
		}
		else
		{
			// put magic header
			file->Write_Byte(2);
			file->Write_Byte(2);
			file->Write_Byte(width >> 8);
			file->Write_Byte(width & 255);

			// convert pixels
			for(int col = 0; col < width; col++)
				GetRGBE(scanline[col], image, col, row, gamma, dither);

			// put components seperately
			for(int i = 0; i < 4; i++)
			{
				for(int col = 0; col < width; col += cnt)
				{
					int beg = 0;

					// find next run
					for(beg = col; beg < width; beg += cnt)
					{
						cnt = 1;
						while((cnt < 127) && (beg + cnt < width) && (scanline[beg + cnt][i] == scanline[beg][i]))
							cnt++;

						// long enough ?
						if(cnt >= MINRUN)
							break;
					}

					if(beg - col > 1 && beg - col < MINRUN)
					{
						c2 = col + 1;
						while(scanline[c2++][i] == scanline[col][i])
						{
							if(c2 == beg)
							{
								// short run
								file->Write_Byte(128 + beg - col);

								if(file->Write_Byte(scanline[col][i]) == false)
									throw POV_EXCEPTION(kFileDataErr, "Failed to write data to HDR file");

								col = beg;
								break;
							}
						}
					}
					while(col < beg)
					{
						// write non-run
						if((c2 = beg - col) > 128)
							c2 = 128;

						file->Write_Byte(c2);

						while(c2--)
						{
							if(file->Write_Byte(scanline[col++][i]) == false)
								throw POV_EXCEPTION(kFileDataErr, "Failed to write data to HDR file");
						}
					}
					if(cnt >= MINRUN)
					{
						// write run
						file->Write_Byte(128 + cnt);

						if(file->Write_Byte(scanline[beg][i]) == false)
							throw POV_EXCEPTION(kFileDataErr, "Failed to write data to HDR file");
					}
					else
						cnt = 0;
				}
			}
		}
	}
}

} // end of namespace HDR

} // end of namespace pov_base

