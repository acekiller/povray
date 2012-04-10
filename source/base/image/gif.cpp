/*******************************************************************************
 * gif.cpp
 *
 * This module contains the code to read the GIF file format.
 *
 * NOTE:  Portions of this module were written by Steve Bennett and are used
 *        here with his permission.
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
 * $File: //depot/povray/smp/source/base/image/gif.cpp $
 * $Revision: #22 $
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

/*
 * The following routines were borrowed freely from FRACTINT, and represent
 * a generalized GIF file decoder.
 *
 * Swiped and converted to entirely "C" coded routines by AAC for the most
 * in future portability!
 */

#include <vector>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/gif.h"

#include <boost/thread.hpp>

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace Gif
{

// TODO: make sure we don't leak an image object if we throw an exception.
Image *Read (IStream *file, const Image::ReadOptions& options, bool IsPOTFile)
{
	int                             data ;
	int                             width;
	int                             height;
	Image                           *image ;
	unsigned char                   buffer[256];
	vector<Image::RGBAMapEntry>     colormap ;
	int                             alphaIdx = -1; // assume no transparency color

	// GIF files used to have no clearly defined gamma by default, but a W3C recommendation exists for them to use sRGB.
	// Anyway, use whatever the user has chosen as default.
	GammaCurvePtr gamma;
	if (options.gammacorrect)
	{
		if (options.defaultGamma)
			gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);
		else
			gamma = TranscodingGammaCurve::Get(options.workingGamma, SRGBGammaCurve::Get());
	}

	int status = 0;

	/* Get the screen description. */
	if (!file->read (buffer, 13))
		throw POV_EXCEPTION(kFileDataErr, "Cannot read GIF file header");

	/* Use updated GIF specs. */
	if (memcmp ((char *) buffer, "GIF", 3) != 0)
		throw POV_EXCEPTION(kFileDataErr, "File is not in GIF format");

	if (buffer[3] != '8' || (buffer[4] != '7' && buffer[4] != '9') || buffer[5] < 'A' || buffer[5] > 'z')
		throw POV_EXCEPTION(kFileDataErr, "Unsupported GIF version");

	int planes = ((unsigned) buffer [10] & 0x0F) + 1;
	int colourmap_size = (1 << planes);

	/* Color map (better be!) */
	if ((buffer[10] & 0x80) == 0)
		throw POV_EXCEPTION(kFileDataErr, "Error in GIF color map");

	for (int i = 0; i < colourmap_size ; i++)
	{
		Image::RGBAMapEntry entry;
		if (!file->read (buffer, 3))
			throw POV_EXCEPTION(kFileDataErr, "Cannot read GIF colormap");
		entry.red   = IntDecode(gamma, buffer[0], 255);
		entry.green = IntDecode(gamma, buffer[1], 255);
		entry.blue  = IntDecode(gamma, buffer[2], 255);
		entry.alpha = 1.0f;
		colormap.push_back(entry);
	}

	/* Now read one or more GIF objects. */
	bool finished = false;

	while (!finished)
	{
		switch (file->Read_Byte())
		{
			case EOF:
				throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF reading GIF file");
				finished = true;
				break ;

			case ';': /* End of the GIF dataset. */
				finished = true;
				status = 0;
				break;

			case '!': /* GIF Extension Block. */
				/* Read (and check) the ID. */
				if (file->Read_Byte() == 0xF9)
				{
					if ((data = file->Read_Byte()) > 0)
					{
						if (!file->read (buffer, data))
							throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF reading GIF file");
						// check transparency flag, and set transparency color index if appropriate
						if (data >= 3 && buffer[0] & 0x01)
						{
							int alphaIdx = buffer[3];
							if (alphaIdx < colourmap_size)
								colormap[alphaIdx].alpha = 0.0f;
						}
					}
					else
						break;
				}
				while ((data = file->Read_Byte()) > 0)
					if (!file->read (buffer, data))
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF reading GIF file");
				break;

			case ',': /* Start of image object. Get description. */
				for (int i = 0; i < 9; i++)
				{
					if ((data = file->Read_Byte()) == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF reading GIF file");
					buffer[i] = (unsigned char) data;
				}

				/* Check "interlaced" bit. */
				if ((buffer[9] & 0x40) != 0)
					throw POV_EXCEPTION(kFileDataErr, "Interlacing in GIF image unsupported");

				width  = (int) buffer[4] | ((int) buffer[5] << 8);
				height = (int) buffer[6] | ((int) buffer[7] << 8);
				image = Image::Create (width, height, Image::Colour_Map, colormap) ;
				// [CLi] GIF only uses full opacity or full transparency, so premultiplied vs. non-premultiplied alpha is not an issue

				/* Get bytes */
				Decode (file, image);
				finished = true;
				break;

			default:
				status = -1;
				finished = true;
				break;
		}
	}

	if (IsPOTFile == false)
	{
		if (!image)
			throw POV_EXCEPTION(kFileDataErr, "Cannot find GIF image data block");
		return (image);
	}

	// POT files are GIF files where the right half of the image contains
	// a second byte for each pixel on the left, thus allowing 16-bit
	// indexes. In this case the palette data is ignored and we convert
	// the image into a 16-bit grayscale version.
	if ((width & 0x01) != 0)
		throw POV_EXCEPTION(kFileDataErr, "Invalid width for POT file");
	int newWidth = width / 2 ;
	Image *newImage = Image::Create (newWidth, height, Image::Gray_Int16) ;
	for (int y = 0 ; y < height ; y++)
		for (int x = 0 ; x < newWidth ; x++)
			newImage->SetGrayValue (x, y, (unsigned int) image->GetIndexedValue (x, y) << 8 | image->GetIndexedValue (x + newWidth, y)) ;
			// NB: POT files don't use alpha, so premultiplied vs. non-premultiplied is not an issue
			// NB: No gamma adjustment happening here!
	delete image ;
	return (newImage) ;
}

} // end of namespace Gif

}

