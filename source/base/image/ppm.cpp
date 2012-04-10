/*******************************************************************************
 * ppm.cpp
 *
 * This module contains the code to read and write the PPM file format.
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
 * $File: //depot/povray/smp/source/base/image/ppm.cpp $
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

/****************************************************************************
*
*  PPM format according to NetPBM specs (http://netpbm.sourceforge.net/doc/):
*
*  This module implements read support for PPM image maps and
*  write support for PPM output.
*
*  For reading both ASCII and binary files are supported ('P3' and 'P6').
*
*  For writing we use binary files. OutputQuality > 8 leads to 16 bit files.
*  Special handling of Greyscale_Output=on -> 16 bit PGM files ('P5')
*  All formats supported for writing can now also be used in continued trace.
*
*****************************************************************************/

#include <vector>
#include <ctype.h>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/pgm.h"
#include "base/image/ppm.h"
#include "base/types.h"

#include "metadata.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace Ppm
{

void Write (OStream *file, const Image *image, const Image::WriteOptions& options)
{
	int                 file_type = POV_File_Image_PPM;
	int                 width = image->GetWidth() ;
	int                 height = image->GetHeight() ;
	int                 bpcc = options.bpcc;
	bool                grayscale = false;
	unsigned int        rval;
	unsigned int        gval;
	unsigned int        bval;
	unsigned int        gray;
	unsigned int        mask;
	GammaCurvePtr       gamma;
	DitherHandler*      dither = options.dither.get();

	if (bpcc == 0)
		bpcc = image->GetMaxIntValue() == 65535 ? 16 : 8 ;
	else if (bpcc < 5)
		bpcc = 5 ;
	else if (bpcc > 16)
		bpcc = 16 ;

	mask = (1 << bpcc) - 1 ;

	// do we want 16 bit grayscale (PGM) output ?
	// TODO - the check for image container type is here to mimick old code; do we still need it?
	if (image->GetImageDataType () == Image::Gray_Int16 || image->GetImageDataType () == Image::GrayA_Int16 || options.grayscale)
	{
		grayscale   = true;
		bpcc        = 16;
		gamma.reset(); // TODO - this is here to mimick old code, which never did gamma correction for greyscale output; do we want to change that?
		file->printf("P5\n");
	}
	else
	{
		if (options.encodingGamma)
			gamma = TranscodingGammaCurve::Get(options.workingGamma, options.encodingGamma);
		else
			// PPM files may or may not be gamma-encoded; besides the sRGB transfer function, the ITU-R-BT.709 transfer function is said to be common as well.
			// If no encoding gamma is specified, we're defaulting to working gamma space at present, i.e. no gamma correction.
			gamma = NeutralGammaCurve::Get();
#ifndef ASCII_PPM_OUTPUT
		file->printf("P6\n");
#else
		file->printf("P3\n");
#endif
	}

	// Prepare metadata, as comment in the header
	Metadata meta;
	file->printf("# Software: %s\n", meta.getSoftware().c_str());
	file->printf("# Render Date: %s\n" ,meta.getDateTime().c_str()); 
	if (!meta.getComment1().empty())
		file->printf("# %s\n", meta.getComment1().c_str());
	if (!meta.getComment2().empty())
		file->printf("# %s\n", meta.getComment2().c_str());
	if (!meta.getComment3().empty())
		file->printf("# %s\n", meta.getComment3().c_str());
	if (!meta.getComment4().empty())
		file->printf("# %s\n", meta.getComment4().c_str());
	file->printf("%d %d\n%d\n", width, height, mask);

	for (int y = 0 ; y < height ; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (grayscale)
			{
				gray = GetEncodedGrayValue (image, x, y, gamma, mask, *dither) ;

				if (bpcc > 8)
				{
					if (!file->Write_Byte((gray >> 8) & 0xFF))
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PGM output data");
					if (!file->Write_Byte(gray & 0xFF))
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PGM output data");
				}
				else
				{
					if (!file->Write_Byte(gray & 0xFF))
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PGM output data");
				}
			}
			else
			{
				GetEncodedRGBValue (image, x, y, gamma, mask, rval, gval, bval, *dither) ;

				if (bpcc > 8)
				{
					// 16 bit per value
#ifndef ASCII_PPM_OUTPUT
					file->Write_Byte(rval >> 8) ;
					file->Write_Byte(rval & 0xFF) ;
					file->Write_Byte(gval >> 8) ;
					file->Write_Byte(gval & 0xFF) ;
					file->Write_Byte(bval >> 8) ;
					if (!file->Write_Byte(bval & 0xFF))
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PPM output data");
#else
					file->printf("%u ", rval);
					file->printf("%u ", gval);
					file->printf("%u\n", bval);
					if (!file)
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PPM output data");
#endif
				}
				else
				{
					// 8 bit per value
#ifndef ASCII_PPM_OUTPUT
					file->Write_Byte(rval & 0xFF) ;
					file->Write_Byte(gval & 0xFF) ;
					if (!file->Write_Byte(bval & 0xFF))
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PPM output data");
#else
					file->printf("%u ", rval & 0xff);
					file->printf("%u ", gval & 0xff);
					file->printf("%u\n", bval & 0xff);
					if (!file)
						throw POV_EXCEPTION(kFileDataErr, "Cannot write PPM output data");
#endif
				}
			}
		}
	}
}

/*****************************************************************************
*
* FUNCTION
*
*  Read_PPM_Image
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*    Christoph Hormann
*
* DESCRIPTION
*
*    Reads an PPM image file
*
* CHANGES
*
*    August 2003 - New implementation based on targa/png reading code
*
******************************************************************************/

// TODO: make sure we destroy the image if we throw an exception
Image *Read (IStream *file, const Image::ReadOptions& options)
{
	int                   width;
	int                   height;
	unsigned int          data_hi;
	unsigned int          data_lo;
	char                  line[1024];
	char                  *ptr;
	Image                 *image = NULL;
	unsigned int          depth;
	unsigned int          r;
	unsigned int          g;
	unsigned int          b;
	unsigned char         header[2];

	// PPM files may or may not be gamma-encoded.
	GammaCurvePtr gamma;
	if (options.gammacorrect && options.defaultGamma)
		gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);

	// --- Read Header ---
	if (!file->read((char *)header, 2))
		throw POV_EXCEPTION(kFileDataErr, "Cannot read header of PPM image");

	if(header[0] != 'P')
		throw POV_EXCEPTION(kFileDataErr, "File is not in PPM format");

	if ((header[1] != '3') && (header[1] != '6'))
		throw POV_EXCEPTION(kFileDataErr, "File is not in PPM format");

	// TODO FIXME - Some valid PPM files may have a different header layout regarding line breaks

	do
	{
		file->getline (line, 1024);
		line[1023] = '\0';
		if ((ptr = strchr(line, '#')) != NULL)
			*ptr = '\0';  // remove comment
	} while (line[0]=='\0');  // read until line without comment from beginning

	// --- First: two numbers: width and height ---
	if (sscanf(line,"%d %d",&width, &height) != 2)
		throw POV_EXCEPTION(kFileDataErr, "Cannot read width and height from PPM image");

	if (width <= 0 || height <= 0)
		throw POV_EXCEPTION(kFileDataErr, "Invalid width or height read from PPM image");

	do
	{
		file->getline (line, 1024) ;
		line[1023] = '\0';
		if ((ptr = strchr(line, '#')) != NULL)
			*ptr = '\0';  // remove comment
	} while (line[0]=='\0');  // read until line without comment from beginning

	// --- Second: one number: color depth ---
	if (sscanf(line,"%d",&depth) != 1)
		throw POV_EXCEPTION(kFileDataErr, "Cannot read color depth from PPM image");

	if ((depth > 65535) || (depth < 1))
		throw POV_EXCEPTION(kFileDataErr, "Unsupported number of colors in PPM image");

	if (depth < 256)
	{
		// We'll be using an image container that provides for automatic decoding if possible - unless there's no such decoding to do.
		gamma = ScaledGammaCurve::GetByDecoding(255.0f/depth, gamma); // Note that we'll apply the scaling even if we don't officially gamma-correct
		Image::ImageDataType imagetype = options.itype;
		if (imagetype == Image::Undefined)
			imagetype = ( GammaCurve::IsNeutral(gamma) ? Image::RGB_Int8 : Image::RGB_Gamma8);
		image = Image::Create (width, height, imagetype) ;
		// NB: PPM files don't use alpha, so premultiplied vs. non-premultiplied is not an issue
		image->TryDeferDecoding(gamma, 255); // try to have gamma adjustment being deferred until image evaluation.

		for (int i = 0; i < height; i++)
		{
			if (header[1] == '3') // --- ASCII PPM file (type 3) ---
			{
				for (int x = 0; x < width; x++)
				{
					r = Pgm::Read_ASCII_File_Number(file);
					g = Pgm::Read_ASCII_File_Number(file);
					b = Pgm::Read_ASCII_File_Number(file);
					SetEncodedRGBValue (image, x, i, gamma, 255, r, g, b) ;
				}
			}
			else                  // --- binary PPM file (type 6) ---
			{
				for (int x = 0; x < width; x++)
				{
					if ((r = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					if ((g = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					if ((b = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					SetEncodedRGBValue (image, x, i, gamma, 255, r, g, b) ;
				}
			}
		}
	}
	else // --- 16 bit PPM (binary or ASCII) ---
	{
		// We'll be using an image container that provides for automatic decoding if possible - unless there's no such decoding to do.
		gamma = ScaledGammaCurve::GetByDecoding(65535.0f/depth, gamma); // Note that we'll apply the scaling even if we don't officially gamma-correct
		Image::ImageDataType imagetype = options.itype;
		if (imagetype == Image::Undefined)
			imagetype = ( GammaCurve::IsNeutral(gamma) ? Image::RGB_Int16 : Image::RGB_Gamma16);
		image = Image::Create (width, height, imagetype) ;
		// NB: PPM files don't use alpha, so premultiplied vs. non-premultiplied is not an issue
		image->TryDeferDecoding(gamma, 65535); // try to have gamma adjustment being deferred until image evaluation.

		for (int i = 0; i < height; i++)
		{
			if (header[1] == '3') // --- ASCII PPM file (type 3) ---
			{
				for (int x = 0; x < width; x++)
				{
					r = Pgm::Read_ASCII_File_Number(file);
					g = Pgm::Read_ASCII_File_Number(file);
					b = Pgm::Read_ASCII_File_Number(file);
					SetEncodedRGBValue (image, x, i, gamma, 65535, r, g, b) ;
				}
			}
			else                  // --- binary PPM file (type 6) ---
			{
				for (int x = 0; x < width; x++)
				{
					if ((data_hi = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					if ((data_lo = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					r = (256*data_hi + data_lo);

					if ((data_hi = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					if ((data_lo = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					g = (256*data_hi + data_lo);

					if ((data_hi = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					if ((data_lo = file->Read_Byte ()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF in PPM file");
					b = (256*data_hi + data_lo);

					SetEncodedRGBValue (image, x, i, gamma, 65535, r, g, b) ;
				}
			}
		}
	}

	return (image) ;
}

} // end of namespace Ppm

}

