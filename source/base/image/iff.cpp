/*******************************************************************************
 * iff.cpp
 *
 * This file implements a simple IFF format file reader.
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
 * $File: //depot/povray/smp/source/base/image/iff.cpp $
 * $Revision: #18 $
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

#include <vector>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/iff.h"

#include <boost/scoped_array.hpp>

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace Iff
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define FORM 0x464f524dL
#define ILBM 0x494c424dL
#define BMHD 0x424d4844L
#define CAMG 0x43414d47L
#define CMAP 0x434d4150L
#define BODY 0x424f4459L
#define CMPNONE 0

#define HAM 0x800

/*****************************************************************************
* Type definitions
******************************************************************************/

typedef struct Chunk_Header_Struct
{
	int name;
	int size;
} CHUNK_HEADER ;

/*****************************************************************************
* Static functions
******************************************************************************/

static int read_byte(IStream *file)
{
	int c;

	if ((c = file->Read_Byte()) == EOF)
		throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF while reading IFF file");
	return (c);
}

static int read_word(IStream *file)
{
	int result = read_byte(file) * 256;
	result += read_byte(file);
	return (result);
}

static long read_long(IStream *file)
{
	long result = 0;
	for (int i = 0; i < 4; i++)
		result = (result << 8) + read_byte(file);
	return (result);
}

Image *Read (IStream *file, const Image::ReadOptions& options)
{
	int                             nPlanes = 0;
	int                             compression = 0;
	int                             mask;
	int                             byte_index;
	int                             count;
	int                             viewmodes=0;
	int                             Previous_Red=0;
	int                             Previous_Green=0;
	int                             Previous_Blue=0;
	int                             colourmap_size = 0;
	int                             width = -1;
	int                             height = -1;
	Image                           *image = NULL;
	CHUNK_HEADER                    Chunk_Header;
	unsigned int                    r;
	unsigned int                    g;
	unsigned int                    b;
	unsigned long                   creg;
	Image::RGBMapEntry              entry;
	vector<Image::RGBMapEntry>      colormap;

	while (true)
	{
		Chunk_Header.name = read_long(file);
		Chunk_Header.size = read_long(file);

		switch (IFF_SWITCH_CAST Chunk_Header.name)
		{
			case FORM:
				if (read_long(file) != ILBM)
					throw POV_EXCEPTION(kFileDataErr, "Expected ILBM while reading IFF file");
				break;

			case BMHD:
				width = read_word(file);
				height = read_word(file);
				read_word(file); /* x position ignored */
				read_word(file); /* y position ignored */
				nPlanes = read_byte(file);
				colourmap_size = 1 << nPlanes;
				read_byte(file); /* masking ignored */
				compression = read_byte(file); /* masking ignored */
				read_byte(file); /* pad */
				read_word(file); /* Transparent colour ignored */
				read_word(file); /* Aspect ratio ignored */
				read_word(file); /* page width ignored */
				read_word(file); /* page height ignored */
				break;

			case CAMG:
				viewmodes = (int)read_long(file);  /* Viewmodes */
				if (viewmodes & HAM)
					colourmap_size = 16;
				break;

			case CMAP:
				colourmap_size = (int) Chunk_Header.size / 3;
				Previous_Red = read_byte(file);
				Previous_Green = read_byte(file);
				Previous_Blue = read_byte(file);
				entry.red = Previous_Red / 255.0f;
				entry.green = Previous_Green / 255.0f;
				entry.blue = Previous_Blue / 255.0f;
				colormap.push_back (entry);
				for (int i = 1; i < colourmap_size; i++)
				{
					entry.red = read_byte(file) / 255.0f;
					entry.green = read_byte(file) / 255.0f;
					entry.blue = read_byte(file) / 255.0f;
					colormap.push_back (entry);
					// TODO FIXME - gamma!
				}

				for (int i = colourmap_size * 3; (long)i < Chunk_Header.size; i++)
					read_byte(file);

				break;

			case BODY:
				if (width > 0 && height > 0)
				{
					Image::ImageDataType imagetype = options.itype;
					if (imagetype == Image::Undefined)
						imagetype = ((viewmodes & HAM) != 0 || nPlanes == 24) ? Image::RGB_Int8 : Image::Colour_Map;
					if ((viewmodes & HAM) != 0 || nPlanes == 24)
						image = Image::Create (width, height, imagetype);
					else
						image = Image::Create (width, height, imagetype, colormap);
					// NB: IFF files don't use alpha, so premultiplied vs. non-premultiplied is not an issue

					int rowlen = ((width + 15) / 16) * 2 ;
					boost::scoped_array<unsigned char> row_bytes (new unsigned char [nPlanes * rowlen]);

					for (int row = 0; row < height; row++)
					{
						for (int plane = 0; plane < nPlanes; plane++)
						{
							if (compression != CMPNONE)
							{
								int nBytes = 0;

								while (nBytes != ((width + 15) / 16) * 2)
								{
									unsigned char c = read_byte(file);

									if ((c >= 0) && (c <= 127))
									{
										for (int k = 0; k <= c; k++)
											row_bytes[plane * rowlen + nBytes++] = (unsigned char)read_byte(file);
									}
									else
									{
										if ((c >= 129) && (c <= 255))
										{
											count = 257 - c;
											c = read_byte(file);
											for (int k = 0; k < count; k++)
												row_bytes[plane * rowlen + nBytes++] = (unsigned char)c;
										}
									}
								}
							}
							else
								for (int k = 0; k < (((width + 15) / 16) * 2); k++)
									row_bytes[plane * rowlen + k] = (unsigned char)read_byte(file);
						}

						mask = 0x80;
						byte_index = 0;

						for (int col = 0; col < width; col++)
						{
							creg = 0;

							for (int plane = nPlanes - 1; plane >= 0; plane--)
							{
								creg *= 2;
								if (row_bytes[plane * rowlen + byte_index] & mask)
									creg++;
							}

							if (viewmodes & HAM)
							{
								switch ((int)(creg >> 4))
								{
									case 0:
										Previous_Red = r = (unsigned char)(colormap[creg].red*255.0);
										Previous_Green = g = (unsigned char)(colormap[creg].green*255.0);
										Previous_Blue = b = (unsigned char)(colormap[creg].blue*255.0);
										break;

									case 1:
										r = (unsigned char)Previous_Red;
										g = (unsigned char)Previous_Green;
										Previous_Blue = b = (unsigned char)(((creg & 0xf) << 4) + (creg & 0xf));
										break;

									case 2:
										Previous_Red = r = (unsigned char)(((creg & 0xf) << 4) + (creg & 0xf));
										g = (unsigned char)Previous_Green;
										b = (unsigned char)Previous_Blue;
										break;

									case 3:
										r = (unsigned char)Previous_Red;
										Previous_Green = g = (unsigned char)(((creg & 0xf) << 4) + (creg & 0xf));
										b = (unsigned char)Previous_Blue;
										break;
								}
								image->SetRGBValue (col, row, r, g, b); // TODO FIXME - gamma!
							}
							else
							{
								if (nPlanes == 24)
								{
									r = (unsigned char)((creg >> 16) & 0xFF);
									g = (unsigned char)((creg >> 8) & 0xFF);
									b = (unsigned char)(creg & 0xFF);
									image->SetRGBValue (col, row, r, g, b); // TODO FIXME - gamma!
								}
								else
								{
									if (creg >= colormap.size())
										throw POV_EXCEPTION(kFileDataErr, "IFF color out of range in image");
									image->SetIndexedValue (col, row, creg);
								}
							}

							mask >>= 1;

							if (mask == 0)
							{
								mask = 0x80;
								byte_index++;
							}
						}
					}
				}
				else
					throw POV_EXCEPTION(kFileDataErr, "Invalid IFF file");
				break;

			default:
				for (int i = 0; (long)i < Chunk_Header.size; i++)
					if (file->Read_Byte() == EOF)
						throw POV_EXCEPTION(kFileDataErr, "Unexpected EOF while reading IFF file");
				break;
		}
	}
	return (image);
}

} // end of namespace Iff

}

