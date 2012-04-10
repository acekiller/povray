/*******************************************************************************
 * openexr.cpp
 *
 * This module contains the code to read and write files via the openexr library.
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
 * $File: //depot/povray/smp/source/base/image/openexr.cpp $
 * $Revision: #28 $
 * $Change: 5510 $
 * $DateTime: 2011/11/11 19:02:11 $
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
#include "base/image/openexr.h"
#include "base/types.h"


#ifndef OPENEXR_MISSING

#ifdef NON_UNIX_OPENEXR_HEADERS

#include "ImfRgbaFile.h"
#include "ImfStringAttribute.h"
#include "ImfMatrixAttribute.h"
#include "ImfArray.h"

#else

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfStringAttribute.h>
#include <OpenEXR/ImfMatrixAttribute.h>
#include <OpenEXR/ImfArray.h>

#endif
#include "metadata.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

namespace OpenEXR
{

using namespace Imf;
using namespace Imath;

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/*****************************************************************************
* Local typedefs
******************************************************************************/
struct Messages
{
	vector<string> warnings;
	string error;
};

///////////////////////////////////////////
// class POV_EXR_OStream
///////////////////////////////////////////
class POV_EXR_OStream : public Imf::OStream
{
	public:
		POV_EXR_OStream(pov_base::OStream& pov_stream) : Imf::OStream(UCS2toASCIIString(pov_stream.Name()).c_str()), os(pov_stream) { }
		virtual ~POV_EXR_OStream() { }

		const char *fileName() { return UCS2toASCIIString(os.Name()).c_str(); }

		void write(const char *c, int n)
		{
			if(os.write((void *) c, n) == false)
				throw POV_EXCEPTION(kFileDataErr, "Error while writing EXR output");
		}

		Int64 tellp()
		{
			unsigned long pos = os.tellg();
			if((int) pos == -1)
				throw POV_EXCEPTION(kFileDataErr, "Error while writing EXR output");
			return(pos);
		}

		void seekp(Int64 pos)
		{
			if(os.seekg((unsigned long)pos) == false)
				throw POV_EXCEPTION(kFileDataErr, "Error when writing EXR output");
		}
	private:
		pov_base::OStream& os;
};

///////////////////////////////////////////
// class POV_EXR_IStream
///////////////////////////////////////////
class POV_EXR_IStream : public Imf::IStream
{
	public:
		POV_EXR_IStream(pov_base::IStream& pov_stream) : Imf::IStream(UCS2toASCIIString(pov_stream.Name()).c_str()), is(pov_stream)
		{
			is.seekg(0, IOBase::seek_end);
			fsize = is.tellg();
			is.seekg(0, IOBase::seek_set);
		}

		virtual ~POV_EXR_IStream() { }

		const char *fileName(void) { return UCS2toASCIIString(is.Name()).c_str(); }
		void clear(void) { is.clearstate(); }

		bool read(char *c, int n)
		{
			if(is.read(c, n) == false)
				throw POV_EXCEPTION(kFileDataErr, "Error while reading EXR file");
			return (is.tellg() < fsize);
		}

		Int64 tellg()
		{
			unsigned long pos = is.tellg();
			if((int)pos == -1)
				throw POV_EXCEPTION(kFileDataErr, "Error while reading EXR file");
			return pos;
		}

		void seekg(Int64 pos)
		{
			if(is.seekg((unsigned long)pos) == false)
				throw POV_EXCEPTION(kFileDataErr, "Error while reading EXR file");
		}
	private:
		pov_base::IStream& is;
		unsigned long fsize;
};

/*****************************************************************************
* Implementation
******************************************************************************/

Image *Read(IStream *file, const Image::ReadOptions& options)
{
	unsigned int width;
	unsigned int height;
	Image  *image = NULL;

	// OpenEXR files store linear color values by default, so never convert unless the user overrides
	// (e.g. to handle a non-compliant file).
	GammaCurvePtr gamma;
	if (options.gammacorrect)
	{
		if (options.gammaOverride)
			gamma = TranscodingGammaCurve::Get(options.workingGamma, options.defaultGamma);
		else
			gamma = TranscodingGammaCurve::Get(options.workingGamma, NeutralGammaCurve::Get());
	}

	// OpenEXR officially uses premultiplied alpha, so that's the preferred mode to use for the image container unless the user overrides
	// (e.g. to handle a non-compliant file).
	bool premul = true;
	if (options.premultiplyOverride)
		premul = options.premultiply;

	// TODO: code this to observe the request for alpha in the input file type.
	POV_EXR_IStream is(*file);
	try
	{
		RgbaInputFile rif(is);
		Array2D<Rgba> pixels;
		Box2i dw = rif.dataWindow();
		Image::ImageDataType imagetype = options.itype;

		width = dw.max.x - dw.min.x + 1;
		height = dw.max.y - dw.min.y + 1;
		pixels.resizeErase(height, width);
		rif.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
		rif.readPixels(dw.min.y, dw.max.y);

		if(imagetype == Image::Undefined)
			imagetype = Image::RGBFT_Float;

		image = Image::Create(width, height, imagetype);
		image->SetPremultiplied(premul); // set desired storage mode regarding alpha premultiplication

		for(int row = 0; row < height; row++)
		{
			for(int col = 0; col < width; col++)
			{
				struct Rgba &rgba = pixels [row][col];
				SetEncodedRGBAValue(image, col, row, gamma, (float)rgba.r, (float)rgba.g, (float)rgba.b, (float)rgba.a, premul);
			}
		}
	}
	catch(const std::exception& e)
	{
		throw POV_EXCEPTION(kFileDataErr, e.what());
	}

	return image;
}

void Write(OStream *file, const Image *image, const Image::WriteOptions& options)
{
	int width = image->GetWidth();
	int height = image->GetHeight();
	bool use_alpha = image->HasTransparency() && options.alphachannel;
	float pixelAspect = 1.0;
	Header hdr(width, height, pixelAspect, Imath::V2f(0, 0), 1.0, INCREASING_Y, ZIP_COMPRESSION);
	boost::scoped_array<Rgba> pixels(new Rgba[width * height]);
	Rgba *p = pixels.get();
	GammaCurvePtr gamma = TranscodingGammaCurve::Get(options.workingGamma, NeutralGammaCurve::Get());

	// OpenEXR officially uses premultiplied alpha, so that's the way we do it unless the user overrides
	// (e.g. to handle a non-compliant file).
	bool premul = true;
	if (options.premultiplyOverride)
		premul = options.premultiply;

	for(int row = 0; row < height; row++)
	{
		for(int col = 0; col < width; col++)
		{
			float r, g, b, a;
			GetEncodedRGBAValue(image, col, row, gamma, r, g, b, a, premul);
			*p++ = Rgba(r, g, b, use_alpha ? a : 1.0f);
		}
	}

	POV_EXR_OStream os(*file);
	try
	{
		Imf::RgbaChannels channels;
		if (options.grayscale)
			if (use_alpha)
				channels = WRITE_YA; // only write luminance & alpha
			else
				channels = WRITE_Y; // only write luminance
		else
			if (use_alpha)
				channels = WRITE_RGBA; // write RGB & alpha
			else
				channels = WRITE_RGB; // write RGB

		Metadata meta;
		string comments;
		if (!meta.getComment1().empty())
			comments += meta.getComment1() + "\n";
		if (!meta.getComment2().empty())
			comments += meta.getComment2() + "\n";
		if (!meta.getComment3().empty())
			comments += meta.getComment3() + "\n";
		if (!meta.getComment4().empty())
			comments += meta.getComment4() + "\n";
		
		if (!comments.empty())
			hdr.insert("comments",StringAttribute(comments.c_str()));

		string software= meta.getSoftware();
		string datetime= meta.getDateTime();
		hdr.insert("software",StringAttribute(software.c_str()));
		hdr.insert("creation",StringAttribute(datetime.c_str()));

		RgbaOutputFile rof(os, hdr, channels);
		rof.setFrameBuffer(pixels.get(), 1, width);
		rof.writePixels(height);
	}
	catch(const std::exception& e)
	{
		throw POV_EXCEPTION(kFileDataErr, e.what());
	}
}

} // end of namespace OpenEXR

} // end of namespace pov_base

#endif  // OPENEXR_MISSING
