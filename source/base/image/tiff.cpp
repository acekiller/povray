/*******************************************************************************
 * tiff.cpp
 *
 * This module contains the code to read and write the TIFF file format.
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
 * $File: //depot/povray/smp/source/base/image/tiff.cpp $
 * $Revision: #26 $
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

#include <vector>
#include <assert.h>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/tiff_pov.h"

#include <boost/scoped_array.hpp>

// this must be the last file included other than tiffio.h
#include "base/povdebug.h"

#ifndef LIBTIFF_MISSING

namespace pov_base
{

namespace Tiff
{

// TIFF Image loader
extern "C"
{
	#ifndef __STDC__
	#define __STDC__        (1)
	#endif
	#ifndef AVOID_WIN32_FILEIO
	#define AVOID_WIN32_FILEIO // this stops the tiff headers from pulling in windows.h on win32/64
	#endif
	#include "tiffio.h"
}

/* Do any of the entries in the color map contain values larger than 255? */
static int checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
	while(n-- > 0)
		if((*r++ >= 256) || (*g++ >= 256) || (*b++ >= 256))
			return 16;
	return 8;
}

static void SuppressTIFFWarnings(const char *, const char *, va_list)
{
}

static tsize_t Tiff_Read(thandle_t fd, tdata_t buf, tsize_t size)
{
	IStream *file = (IStream *)fd;

	if(!file->read(buf, size))
		return 0;

	return (tsize_t)(size);
}

static tsize_t Tiff_Write(thandle_t fd, tdata_t buf, tsize_t size)
{
	return (tsize_t)0;
}

static toff_t Tiff_Seek(thandle_t fd, toff_t off, int whence)
{
	IStream *file = (IStream *)fd;

	file->seekg(off, whence);

	return (toff_t)file->tellg();
}

static int Tiff_Close(thandle_t fd)
{
	IStream *file = (IStream *)fd;

	// we don't close the file here; it's done by the caller
	// delete file;

	return 0;
}

static toff_t Tiff_Size(thandle_t fd)
{
	IStream *file = (IStream *)fd;
	unsigned int pos = 0;
	unsigned int len = 0;

	pos = file->tellg();
	file->seekg(0, IOBase::seek_end);
	len = file->tellg();
	file->seekg(pos, IOBase::seek_set);

	return (toff_t)len;
}

static int Tiff_Map(thandle_t, tdata_t *, toff_t *)
{
	return 0;
}

static void Tiff_Unmap(thandle_t, tdata_t, toff_t)
{
}

/*****************************************************************************
*
* FUNCTION      : Read_Tiff_Image
*
* ARGUMENTS     : IMAGE *Image; char *name;
*
* MODIFIED ARGS : Image
*
* RETURN VALUE  : none
*
* AUTHOR        : Alexander Enzmann
*
* DESCRIPTION
*
*   Reads a TIFF image into an RGB image buffer
*
* CHANGES
*
* New - 6/2000
*
******************************************************************************/

Image *Read (IStream *file, const Image::ReadOptions& options)
{
	int                   nrow;
	int                   result = 0;
	long                  LineSize;
	TIFF                  *tif;
	Image                 *image ;
	uint16                BitsPerSample;
	uint16                BytesPerSample = 1;
	uint16                PhotometricInterpretation;
	uint16                SamplePerPixel;
	uint16                Orientation;
	uint32                RowsPerStrip;
	unsigned int          width;
	unsigned int          height;

	// TODO - TIFF files probably have some gamma info in them by default, but we're currently ignorant about that.
	// Until that is fixed, use whatever the user has chosen as default.
	GammaCurvePtr gamma;
	if (options.gammacorrect && options.defaultGamma)
		gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);

	// [CLi] TIFF is specified to use associated (= premultiplied) alpha, so that's the preferred mode to use for the image container unless the user overrides
	// (e.g. to handle a non-compliant file).
	bool premul = true;
	if (options.premultiplyOverride)
		premul = options.premultiply;

	// Rather than have libTIFF complain about tags it doesn't understand,
	// we just suppress all the warnings.
	TIFFSetWarningHandler(SuppressTIFFWarnings);
	TIFFSetErrorHandler(SuppressTIFFWarnings);

	// Open and do initial processing
	tif = TIFFClientOpen("Dummy File Name", "r", file,
		Tiff_Read, Tiff_Write, Tiff_Seek, Tiff_Close,
		Tiff_Size, Tiff_Map, Tiff_Unmap);
	if (!tif)
		return (NULL) ;

	// Get basic information about the image
	int ExtraSamples, ExtraSampleInfo;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
	TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &RowsPerStrip);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &PhotometricInterpretation);
	TIFFGetField(tif, TIFFTAG_ORIENTATION, &Orientation);
	TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplePerPixel);
	TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES, &ExtraSamples, &ExtraSampleInfo);

	// don't support more than 16 bits per sample
	if (BitsPerSample == 16)
	{
		BytesPerSample = 2 ;
		options.warnings.push_back ("Warning: reading 16 bits/sample TIFF file; components crunched to 8");
	}

	LineSize = TIFFScanlineSize(tif);
	assert (SamplePerPixel == (int) (LineSize / width) / BytesPerSample);
	// SamplePerPixel = (int)(LineSize / width);

#if 0
	// For now we are ignoring the orientation of the image...
	switch (Orientation)
	{
	case ORIENTATION_TOPLEFT:
		break;
	case ORIENTATION_TOPRIGHT:
		break;
	case ORIENTATION_BOTRIGHT:
		break;
	case ORIENTATION_BOTLEFT:
		break;
	case ORIENTATION_LEFTTOP:
		break;
	case ORIENTATION_RIGHTTOP:
		break;
	case ORIENTATION_RIGHTBOT:
		break;
	case ORIENTATION_LEFTBOT:
		break;
	default:
		break;
	}
#endif

	//PhotometricInterpretation = 2 image is RGB
	//PhotometricInterpretation = 3 image have a color palette
	if (PhotometricInterpretation == PHOTOMETRIC_PALETTE && (TIFFIsTiled(tif) == 0))
	{
		uint16 *red, *green, *blue;

		//load the palette
		int cmap_len = (1 << BitsPerSample);

		TIFFGetField(tif, TIFFTAG_COLORMAP, &red, &green, &blue);

		vector<Image::RGBMapEntry> colormap ;
		Image::RGBMapEntry entry;

		// I may be mistaken, but it appears that alpha/opacity information doesn't
		// appear in a Paletted Tiff image.  Well - if it does, it's not as easy to
		// get at as RGB.
		// Read the palette
		// Is the palette 16 or 8 bits ?
		if (checkcmap(cmap_len, red, green, blue) == 16)
		{
			for (int i=0;i<cmap_len;i++)
			{
				entry.red   = IntDecode(gamma, red[i],   65535);
				entry.green = IntDecode(gamma, green[i], 65535);
				entry.blue  = IntDecode(gamma, blue[i],  65535);
				colormap.push_back (entry);
			}
		}
		else
		{
			for (int i=0;i<cmap_len;i++)
			{
				entry.red   = IntDecode(gamma, red[i],   255);
				entry.green = IntDecode(gamma, green[i], 255);
				entry.blue  = IntDecode(gamma, blue[i],  255);
				colormap.push_back (entry);
			}
		}

		Image::ImageDataType imagetype = options.itype;
		if (imagetype == Image::Undefined)
			imagetype = Image::Colour_Map;
		image = Image::Create (width, height, imagetype, colormap) ;
		image->SetPremultiplied(premul); // specify whether the color map data has premultiplied alpha

		boost::scoped_array<unsigned char> buf (new unsigned char [TIFFStripSize(tif)]);

		//read the tiff lines and save them in the image
		//with RGB mode, we have to change the order of the 3 samples RGB <=> BGR
		for (int row=0;row<height;row+=RowsPerStrip)
		{
			nrow = (row + (int)RowsPerStrip > height ? height - row : RowsPerStrip);
			TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 0), buf.get(), nrow * LineSize);
			for (int l = 0, offset = 0; l < nrow ; l++, offset += LineSize)
				for (int x = 0 ; x < width ; x++)
					image->SetIndexedValue (x, row+l, buf[offset+x]) ;
		}
	}
	else
	{
		// Allocate the row buffers for the image
		boost::scoped_array<uint32> buf (new uint32 [width * height]) ;

		Image::ImageDataType imagetype = options.itype;
		if (imagetype == Image::Undefined)
			imagetype = ( GammaCurve::IsNeutral(gamma) ? Image::RGBA_Int8 : Image::RGBA_Gamma8 );
		image = Image::Create (width, height, imagetype) ;
		image->SetPremultiplied(premul); // set desired storage mode regarding alpha premultiplication
		image->TryDeferDecoding(gamma, 255); // try to have gamma adjustment being deferred until image evaluation.

		TIFFReadRGBAImage(tif, width, height, buf.get(), 0);
		uint32 abgr, *tbuf = buf.get();
		for (int i=height-1;i>=0;i--)
		{
			for (int j=0;j<width;j++)
			{
				abgr = *tbuf++;
				unsigned int b = (unsigned char)TIFFGetB(abgr);
				unsigned int g = (unsigned char)TIFFGetG(abgr);
				unsigned int r = (unsigned char)TIFFGetR(abgr);
				unsigned int a = (unsigned char)TIFFGetA(abgr);
				SetEncodedRGBAValue(image, j, i, gamma, 255, r, g, b, a, premul) ;
			}
		}
	}

	TIFFClose(tif);

	return (image) ;
}

} // end of namespace Tiff

}

#endif  // LIBTIFF_MISSING
