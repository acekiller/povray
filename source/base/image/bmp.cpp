/*******************************************************************************
 * bmp.cpp
 *
 * This module contains the code to read and write the BMP file format.
 *
 * Author: Wlodzimierz ABX Skiba (abx@abx.art.pl)
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
 * $File: //depot/povray/smp/source/base/image/bmp.cpp $
 * $Revision: #25 $
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

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define WIN_NEW     40
#define WIN_OS2_OLD 12
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

#include <vector>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/bmp.h"
#include "base/types.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace Bmp
{

/*****************************************************************************
*
* FUNCTION
*
*   Read_Safe_Char
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*   Jan 2004 : Added exception to allow cleanup on error [CJC]
*
******************************************************************************/

static inline unsigned char Read_Safe_Char (IStream& in)
{
	unsigned char ch;

	in >> ch;
	if (!in)
		throw POV_EXCEPTION(kFileDataErr, "Error reading data from BMP image.") ;

	return (ch) ;
}

// skip forward without using seekg
static bool Skip (IStream *file, int bytes)
{
	while (*file && bytes--)
		file->Read_Byte () ;
	return (*file) ;
}

// skip forward without using seekg
static bool Skip (OStream *file, int bytes)
{
	while (*file && bytes--)
		file->Write_Byte (0) ;
	return (*file) ;
}

// write a long to a stream in little-endian (e.g. x86) format
static void Write_Long (OStream *file, unsigned long val)
{
	file->Write_Byte (val & 0xff) ;
	file->Write_Byte ((val >> 8) & 0xff) ;
	file->Write_Byte ((val >> 16) & 0xff) ;
	file->Write_Byte ((val >> 24) & 0xff) ;
}

// write a short to a stream in little-endian (e.g. x86) format
static void Write_Short (OStream *file, unsigned short val)
{
	file->Write_Byte (val & 0xff) ;
	file->Write_Byte ((val >> 8) & 0xff) ;
}

// read a long from a stream in little-endian (e.g. x86) format
static unsigned long Read_Long (IStream *file)
{
	unsigned long ch1 = Read_Safe_Char (*file) ;
	unsigned long ch2 = Read_Safe_Char (*file) ;
	unsigned long ch3 = Read_Safe_Char (*file) ;
	unsigned long ch4 = Read_Safe_Char (*file) ;
	return ((ch4 << 24) | (ch3 << 16) | (ch2 << 8) | ch1) ;
}

// read a short from a stream in little-endian (e.g. x86) format
static unsigned short Read_Short (IStream *file)
{
	unsigned short ch1 = Read_Safe_Char (*file) ;
	unsigned short ch2 = Read_Safe_Char (*file) ;
	return ((ch2 << 8) | ch1) ;
}

/*****************************************************************************
*
* FUNCTION
*
*   Read_BMP_1b
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*
******************************************************************************/

static void Read_BMP_1b(Image *image, IStream& in, unsigned width, unsigned height)
{
	int c = 0;
	unsigned pwidth = ((width+31)>>5)<<5; /* clear bits to get 4 byte boundary */

	for (int y=height - 1; y >= 0; y--)
	{
		for (int x=0; x<pwidth; x++)
		{
			if ((x&7) == 0)
				c = Read_Safe_Char (in);
			if (x<width)
			{
				image->SetBitValue (x, y, (c & 0x80) ? 1 : 0);
				c <<= 1;
			}
		}
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Read_BMP_4b_RGB
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*
******************************************************************************/

static void Read_BMP_4b_RGB(Image *image, IStream& in, unsigned width, unsigned height)
{
	int c = 0;
	unsigned pwidth = ((width+7)>>3)<<3;

	for (int y=height - 1; y >= 0; y--)
	{
		for (int x=0; x<pwidth; x++)
		{
			if ((x&1)==0)
				c = Read_Safe_Char(in);
			if (x<width)
			{
				image->SetIndexedValue (x, y, (c&0xf0)>>4) ;
				c <<= 4;
			}
		}
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Read_BMP_4b_RLE
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*
******************************************************************************/

static void Read_BMP_4b_RLE(Image *image, IStream& in, unsigned width, unsigned height)
{
	int c, cc = 0;
	unsigned x = 0;
	unsigned y = height-1;

	while (1)
	{
		c = Read_Safe_Char (in);
		if (c)
		{
			cc = Read_Safe_Char (in);
			for (int i=0; i<c; i++, x++)
				if ((y<height) && (x<width))
					image->SetIndexedValue (x, y, (i&1) ? (cc &0x0f) : ((cc>>4)&0x0f));
		}
		else
		{
			c = Read_Safe_Char (in);
			if (c==0)
			{
				x=0;
				y--;
			}
			else if (c==1)
				return;
			else if (c==2)
			{
				x += Read_Safe_Char (in);
				y -= Read_Safe_Char (in);
			}
			else
			{
				for (int i=0; i<c; i++, x++)
				{
					if ((i&1)==0)
						cc = Read_Safe_Char (in);
					if ((y<height) && (x<width))
						image->SetIndexedValue (x, y, ((i&1)?cc:(cc>>4))&0x0f) ;
				}
				if (((c&3)==1) || ((c&3)==2))
					Read_Safe_Char (in);
			}
		}
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Read_BMP_8b_RGB
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*
******************************************************************************/

static void Read_BMP_8b_RGB(Image *image, IStream& in, unsigned width, unsigned height)
{
	int c;
	unsigned pwidth = ((width+3)>>2)<<2;

	for (unsigned y=height; (--y)<height;)
		for (unsigned x=0; x<pwidth; x++)
		{
			c = Read_Safe_Char (in);
			if (x<width)
				image->SetIndexedValue (x, y, c);
		}
}

/*****************************************************************************
*
* FUNCTION
*
*   Read_BMP_8b_RLE
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*
******************************************************************************/

static void Read_BMP_8b_RLE(Image *image, IStream& in, unsigned width, unsigned height)
{
	int c, cc;
	unsigned x = 0;
	unsigned y = height-1;

	while (1)
	{
		c = Read_Safe_Char (in);
		if (c)
		{
			cc = Read_Safe_Char (in);
			for (int i=0; i<c; i++, x++)
				if ((y<height) && (x<width))
					image->SetIndexedValue (x, y, cc);
		}
		else
		{
			c = Read_Safe_Char (in);
			switch(c)
			{
				case 0:
					x = 0;
					y--;
					break;
				case 1:
					return;
					break;
				case 2:
					x += Read_Safe_Char (in);
					y -= Read_Safe_Char (in);
					break;
				default:
					for (int i=0; i<c; i++, x++)
						if ((y<height) && (x<width))
							image->SetIndexedValue (x, y, Read_Safe_Char (in));
					if (c & 1)
						Read_Safe_Char (in); /* "absolute mode" runs are word-aligned */
			}
		}
	}
}

/*****************************************************************************
*
* FUNCTION
*
*   Open_BMP_File
*
* INPUT
*
* *filePUT
*
* RETURNS
*
* AUTHOR
*
*   Wlodzimierz ABX Skiba
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 2003 : Creation.
*   Jan 2004 : Added exception handling to allow cleanup on error [CJC]
*
******************************************************************************/

void Write (OStream *file, const Image *image, const Image::WriteOptions& options)
{
	int             width = image->GetWidth();
	int             height = image->GetHeight();
	int             pad = (4 - ((width * 3) % 4)) & 0x03 ;
	bool            alpha = image->HasTransparency() && options.alphachannel;
	unsigned int    r ;
	unsigned int    g ;
	unsigned int    b ;
	unsigned int    a ;
	GammaCurvePtr   gamma;
	DitherHandler*  dither = options.dither.get();

	if (options.encodingGamma)
		gamma = TranscodingGammaCurve::Get(options.workingGamma, options.encodingGamma);
	else
		// BMP files used to have no clearly defined gamma by default, but a Microsoft recommendation exists to assume sRGB.
		gamma = TranscodingGammaCurve::Get(options.workingGamma, SRGBGammaCurve::Get());

	// TODO ALPHA - check if BMP should really keep presuming non-premultiplied alpha
	// We presume non-premultiplied alpha, unless the user overrides
	// (e.g. to handle a non-compliant file).
	bool premul = false;
	if (options.premultiplyOverride)
		premul = options.premultiply;

	int count = (width * (alpha ? 32 : 24) + 31) / 32 * 4 * height;

	*file << 'B' << 'M' ;
	Write_Long (file, 14 + 40 + count) ;
	Write_Short (file, 0) ;
	Write_Short (file, 0) ;
	Write_Long (file, 14 + 40) ;
	Write_Long (file, 40) ;
	Write_Long (file, width) ;
	Write_Long (file, height) ;
	Write_Short (file, 1) ;
	Write_Short (file, alpha ? 32 : 24) ;
	Write_Long (file, BI_RGB) ;
	Write_Long (file, count) ;
	Write_Long (file, 0) ;
	Write_Long (file, 0) ;
	Write_Long (file, 0) ;
	Write_Long (file, 0) ;

	for (int y = height - 1 ; y >= 0 ; y--)
	{
		for (int x = 0 ; x < width ; x++)
		{
			if (alpha)
				GetEncodedRGBAValue (image, x, y, gamma, 255, r, g, b, a, *dither, premul);
			else
				GetEncodedRGBValue (image, x, y, gamma, 255, r, g, b, *dither) ;
			*file << (unsigned char) b;
			*file << (unsigned char) g;
			*file << (unsigned char) r;
			if (alpha)
				*file << (unsigned char) a;
		}
		if (!alpha)
			for (int i = 0 ; i < pad; i++)
				*file << (unsigned char) 0 ;
	}

	if (!*file)
		throw POV_EXCEPTION(kFileDataErr, "Error writing to BMP file") ;
}

Image *Read (IStream *file, const Image::ReadOptions& options)
{
	unsigned file_width, file_height;
	unsigned file_depth, file_colors;
	unsigned data_location, planes, compression;
	unsigned info;
	Image *image = NULL ;

	// BMP files used to have no clearly defined gamma by default, but a Microsoft recommendation exists to assume sRGB.
	// Since ~1995, a header extension (BITMAPV4HEADER) with gamma metadata exists, which could be used if present.
	// However, as of now (2009), such information seems to be rarely included, if at all. (Same goes for BITMAPV5HEADER,
	// which could include a full colorimetric profile.)
	GammaCurvePtr gamma;
	if (options.gammacorrect)
	{
		if (options.defaultGamma)
			gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);
		else
			gamma = TranscodingGammaCurve::Get(options.workingGamma, SRGBGammaCurve::Get());
	}

	// TODO ALPHA - check if BMP should really keep presuming non-premultiplied alpha
	// We presume non-premultiplied alpha, so that's the preferred mode to use for the image container unless the user overrides
	// (e.g. to handle a non-compliant file).
	bool premul = false;
	if (options.premultiplyOverride)
		premul = options.premultiply;

	if ((file->Read_Byte () != 'B') || (file->Read_Byte () !='M'))
		throw POV_EXCEPTION(kFileDataErr, "Error reading magic number of BMP image");

	// skip file size and reserved fields
	Skip (file, 8) ;
	data_location = Read_Long (file) ;

	// read properties
	if ((info = Read_Long (file)) != WIN_OS2_OLD)
	{
		file_width = Read_Long (file) ;
		file_height = Read_Long (file) ;
		planes = Read_Short (file) ;
		file_depth = Read_Short (file) ;
		compression = Read_Long (file) ;
		Skip (file, 12) ; // skip image size in bytes, H&V pixels per meter
		file_colors = Read_Long (file) ;
		Skip (file, 4) ; // skip needed colors
	}
	else  /* info == WIN_OS2_OLD */
	{
		file_width = Read_Short (file) ;
		file_height = Read_Short (file) ;
		planes = Read_Short (file) ;
		file_depth = Read_Short (file) ;
		compression = BI_RGB ;
		file_colors = 0 ;
	}

	bool has_alpha = file_depth == 32 ;

	/* do not allow other subtypes */
	if (((file_depth!=1) && (file_depth!=4) && (file_depth!=8) && (file_depth!=24) && (file_depth!=32)) ||
	    (planes!=1) || (compression>BI_RLE4) ||
	    (((file_depth==1) || (file_depth==24) || (file_depth==32)) && (compression!=BI_RGB)) ||
	    ((file_depth==4) && (compression==BI_RLE8)) ||
	    ((file_depth==8) && (compression==BI_RLE4)))
		throw POV_EXCEPTION(kFileDataErr, "Invalid or unsupported BMP file");

	/* seek to colormap */
	if (info != WIN_OS2_OLD)
		Skip (file, info - 40) ;

	if (file_depth < 24)
	{
		int color_map_length = file_colors ? file_colors : 1<<file_depth ;
		vector<Image::RGBMapEntry> colormap ;
		Image::RGBMapEntry entry;

		for (int i=0; i<color_map_length; i++)
		{
			entry.blue  = IntDecode(gamma, file->Read_Byte (), 255);
			entry.green = IntDecode(gamma, file->Read_Byte (), 255);
			entry.red   = IntDecode(gamma, file->Read_Byte (), 255);
			if (info != WIN_OS2_OLD)
				file->Read_Byte() ;
			colormap.push_back (entry) ;
		}
		gamma.reset(); // gamma has been taken care of by transforming the color table.

		if (file->eof ())
			throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF while reading BMP file");

		image = Image::Create (file_width, file_height, Image::Colour_Map, colormap) ;
		image->SetPremultiplied(premul); // specify whether the color map data has premultiplied alpha

		switch (file_depth)
		{
			case 1:
				Read_BMP_1b(image, *file, file_width, file_height);
				break;
			case 4:
				switch(compression)
				{
					case BI_RGB:
						Read_BMP_4b_RGB(image, *file, file_width, file_height);
						break;
					case BI_RLE4:
						Read_BMP_4b_RLE(image, *file, file_width, file_height);
						break;
					default:
						throw POV_EXCEPTION(kFileDataErr, "Unknown compression scheme in BMP file");
				}
				break;
			case 8:
				switch(compression)
				{
					case BI_RGB:
						Read_BMP_8b_RGB(image, *file, file_width, file_height);
						break;
					case BI_RLE8:
						Read_BMP_8b_RLE(image, *file, file_width, file_height);
						break;
					default:
						throw POV_EXCEPTION(kFileDataErr, "Unknown compression scheme in BMP file");
				}
				break;
			default:
				throw POV_EXCEPTION(kFileDataErr, "Unknown depth in BMP file");
		}
	}
	else
	{
		/* includes update from stefan maierhofer for 32bit */
		Image::ImageDataType imagetype = options.itype ;
		if (imagetype == Image::Undefined)
		{
			if (GammaCurve::IsNeutral(gamma))
				// No gamma correction required, raw values can be stored "as is".
				imagetype = has_alpha ? Image::RGBA_Int8 : Image::RGB_Int8 ;
			else
				// Gamma correction required; use an image container that will take care of that.
				imagetype = has_alpha ? Image::RGBA_Gamma8 : Image::RGB_Gamma8 ;
		}
		image = Image::Create (file_width, file_height, imagetype) ;
		image->SetPremultiplied(premul); // set desired storage mode regarding alpha premultiplication
		image->TryDeferDecoding(gamma, 255); // try to have gamma adjustment being deferred until image evaluation.

		int pad = has_alpha ? 0 : (4 - ((file_width * 3) % 4)) & 0x03 ;
		unsigned int a = 255 ; // value to use for files that don't have an alpha channel (full opacity)

		for (int y = file_height - 1 ; y >= 0 ; y--)
		{
			for (int x = 0 ; x < file_width ; x++)
			{
				unsigned int b = Read_Safe_Char (*file);
				unsigned int g = Read_Safe_Char (*file);
				unsigned int r = Read_Safe_Char (*file);
				if (has_alpha)
					a = Read_Safe_Char (*file);
				SetEncodedRGBAValue (image, x, y, gamma, 255, r, g, b, a, premul);
			}
			if (pad && !Skip (file, pad))
				throw POV_EXCEPTION(kFileDataErr, "Error reading data from BMP image.") ;
		}
	}

	return (image) ;

}

} // end of namespace Bmp

}
