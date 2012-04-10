/*******************************************************************************
 * imageprocessing.cpp
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
 * $File: //depot/povray/smp/source/frontend/imageprocessing.cpp $
 * $Revision: #49 $
 * $Change: 5387 $
 * $DateTime: 2011/01/17 15:14:56 $
 * $Author: chrisc $
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

#include <string>
#include <cctype>

#include <boost/scoped_ptr.hpp>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/types.h"
#include "base/image/encoding.h"

#include "frontend/imageprocessing.h"

// this must be the last file included
#include "base/povdebug.h"

// TODO: update ImageProcessing with the means of accepting and caching
// blocks of pixels as opposed to individual ones, with a back-end that
// can serialize completed rows to the final image output file.

namespace pov_frontend
{

using namespace pov;

enum
{
	X = 0,
	Y = 1,
	Z = 2
};

ImageProcessing::ImageProcessing(unsigned int width, unsigned int height)
{
	image = shared_ptr<Image>(Image::Create(width, height, Image::RGBFT_Float));
	toStderr = toStdout = false;

	// TODO FIXME - find a better place for this
	image->SetPremultiplied(true); // POV-Ray uses premultiplied opacity for its math, so that's what will end up in the image container
}

ImageProcessing::ImageProcessing(POVMS_Object& ropts)
{
	unsigned int width(ropts.TryGetInt(kPOVAttrib_Width, 160));
	unsigned int height(ropts.TryGetInt(kPOVAttrib_Height, 120));
	unsigned int blockSize(ropts.TryGetInt(kPOVAttrib_RenderBlockSize, 32));
	unsigned int maxBufferMem(ropts.TryGetInt(kPOVAttrib_MaxImageBufferMem, 128)); // number is megabytes

	image = shared_ptr<Image>(Image::Create(width, height, Image::RGBFT_Float, maxBufferMem, blockSize * blockSize));
	toStdout = OutputIsStdout(ropts);
	toStderr = OutputIsStderr(ropts);

	// TODO FIXME - find a better place for this
	image->SetPremultiplied(true); // POV-Ray uses premultiplied opacity for its math, so that's what will end up in the image container
}

ImageProcessing::ImageProcessing(shared_ptr<Image>& img)
{
	image = img;
	toStderr = toStdout = false;

	// TODO FIXME - find a better place for this
	image->SetPremultiplied(true); // POV-Ray uses premultiplied opacity for its math, so that's what will end up in the image container
}

ImageProcessing::~ImageProcessing()
{
}

UCS2String ImageProcessing::WriteImage(POVMS_Object& ropts, POVMSInt frame, int digits)
{
	if(ropts.TryGetBool(kPOVAttrib_OutputToFile, true) == true)
	{
		Image::WriteOptions wopts;
		Image::ImageFileType imagetype = Image::SYS;
		unsigned int filetype = POV_File_Image_System;

		wopts.bpcc = clip(ropts.TryGetInt(kPOVAttrib_BitsPerColor, 8), 5, 16);
		wopts.alphachannel = ropts.TryGetBool(kPOVAttrib_OutputAlpha, false);
		wopts.compress = clip(ropts.TryGetInt(kPOVAttrib_Compression, 0), 0, 255);
		wopts.grayscale = ropts.TryGetBool(kPOVAttrib_GrayscaleOutput, false);

		switch(ropts.TryGetInt(kPOVAttrib_OutputFileType, DEFAULT_OUTPUT_FORMAT))
		{
			case kPOVList_FileType_Targa:
				imagetype = Image::TGA;
				filetype = POV_File_Image_Targa;
				wopts.compress = 0;
				break;
			case kPOVList_FileType_CompressedTarga:
				imagetype = Image::TGA;
				filetype = POV_File_Image_Targa;
				wopts.compress = 1;
				break;
			case kPOVList_FileType_PNG:
				imagetype = Image::PNG;
				filetype = POV_File_Image_PNG;
				break;
			case kPOVList_FileType_JPEG:
				imagetype = Image::JPEG;
				filetype = POV_File_Image_JPEG;
				wopts.compress = clip(int(wopts.compress), 0, 100);
				break;
			case kPOVList_FileType_PPM:
				imagetype = Image::PPM;
				filetype = POV_File_Image_PPM;
				break;
			case kPOVList_FileType_BMP:
				imagetype = Image::BMP;
				filetype = POV_File_Image_BMP;
				break;
			case kPOVList_FileType_OpenEXR:
				imagetype = Image::EXR;
				filetype = POV_File_Image_EXR;
				break;
			case kPOVList_FileType_RadianceHDR:
				imagetype = Image::HDR;
				filetype = POV_File_Image_HDR;
				break;
			case kPOVList_FileType_System:
				imagetype = Image::SYS;
				filetype = POV_File_Image_System;
				break;
			default:
				throw POV_EXCEPTION_STRING("Invalid file type for output");
		}

		int gammaType = ropts.TryGetInt(kPOVAttrib_FileGammaType, DEFAULT_FILE_GAMMA_TYPE);
		float gamma = ropts.TryGetFloat(kPOVAttrib_FileGamma, DEFAULT_FILE_GAMMA);
		wopts.encodingGamma = GetGammaCurve(gammaType, gamma);
		// NB: RenderFrontend<...>::CreateView should have dealt with kPOVAttrib_LegacyGammaMode already and updated kPOVAttrib_WorkingGammaType and kPOVAttrib_WorkingGamma to fit.
		gammaType = ropts.TryGetInt(kPOVAttrib_WorkingGammaType, DEFAULT_WORKING_GAMMA_TYPE);
		gamma = ropts.TryGetFloat(kPOVAttrib_WorkingGamma, DEFAULT_WORKING_GAMMA);
		wopts.workingGamma = GetGammaCurve(gammaType, gamma);

		bool dither = ropts.TryGetBool(kPOVAttrib_Dither, false);
		int ditherMethod = kPOVList_DitherMethod_None;
		if (dither)
			ditherMethod = ropts.TryGetInt(kPOVAttrib_DitherMethod, kPOVList_DitherMethod_FloydSteinberg);
		wopts.dither = GetDitherHandler(ditherMethod, image->GetWidth());

		// in theory this should always return a filename since the frontend code
		// sets it via a call to GetOutputFilename() before the render starts.
		UCS2String filename = ropts.TryGetUCS2String(kPOVAttrib_OutputFile, "");
		if(filename.empty() == true)
			filename = GetOutputFilename(ropts, frame, digits);

		boost::scoped_ptr<OStream> imagefile(NewOStream(filename.c_str(), filetype, false)); // TODO - check file permissions somehow without macro [ttrf]
		if(imagefile == NULL)
			throw POV_EXCEPTION_CODE(kCannotOpenFileErr);

		Image::Write(imagetype, imagefile.get(), image.get(), wopts);

		return filename;
	}
	else
		return UCS2String();
}

shared_ptr<Image>& ImageProcessing::GetImage()
{
	return image;
}

void ImageProcessing::RGB2XYZ(const COLC *rgb, COLC *xyz)
{
	// assumes D65 white point (slightly rounded sRGB)
	xyz[X] = (0.412453 * rgb[Colour::RED]) + (0.357580 * rgb[Colour::GREEN]) + (0.180423 * rgb[Colour::BLUE]);
	xyz[Y] = (0.212671 * rgb[Colour::RED]) + (0.715160 * rgb[Colour::GREEN]) + (0.072169 * rgb[Colour::BLUE]);
	xyz[Z] = (0.019334 * rgb[Colour::RED]) + (0.119193 * rgb[Colour::GREEN]) + (0.950227 * rgb[Colour::BLUE]);
}

void ImageProcessing::XYZ2RGB(const COLC *xyz, COLC *rgb)
{
	// assumes D65 white point (slightly rounded sRGB)
	rgb[Colour::RED] =    (3.240479 * xyz[X]) + (-1.537150 * xyz[X]) + (-0.498535 * xyz[X]);
	rgb[Colour::GREEN] = (-0.969256 * xyz[Y]) +  (1.875992 * xyz[Y]) +  (0.041556 * xyz[Y]);
	rgb[Colour::BLUE] =   (0.055648 * xyz[Z]) + (-0.204043 * xyz[Z]) +  (1.057311 * xyz[Z]);
}

bool ImageProcessing::OutputIsStdout(POVMS_Object& ropts)
{
	UCS2String path(ropts.TryGetUCS2String(kPOVAttrib_OutputFile, ""));

	toStdout = path == POVMS_ASCIItoUCS2String("-") || path == POVMS_ASCIItoUCS2String("stdout");
	toStderr = path == POVMS_ASCIItoUCS2String("stderr");
	return toStdout;
}

bool ImageProcessing::OutputIsStderr(POVMS_Object& ropts)
{
	OutputIsStdout(ropts);
	return toStderr;
}

UCS2String ImageProcessing::GetOutputFilename(POVMS_Object& ropts, POVMSInt frame, int digits)
{
	Path path(ropts.TryGetUCS2String(kPOVAttrib_OutputFile, ""));
	UCS2String filename = path.GetFile();
	UCS2String ext;
	Image::ImageFileType imagetype;

	switch(ropts.TryGetInt(kPOVAttrib_OutputFileType, DEFAULT_OUTPUT_FORMAT))
	{
		case kPOVList_FileType_Targa:
		case kPOVList_FileType_CompressedTarga:
			ext = ASCIItoUCS2String(".tga");
			imagetype = Image::TGA;
			break;

		case kPOVList_FileType_PNG:
			ext = ASCIItoUCS2String(".png");
			imagetype = Image::PNG;
			break;

		case kPOVList_FileType_JPEG:
			ext = ASCIItoUCS2String(".jpg");
			imagetype = Image::JPEG;
			break;

		case kPOVList_FileType_PPM:
			ext = ASCIItoUCS2String(".ppm"); // TODO FIXME - in case of greyscale output, extension should default to ".pgm"
			imagetype = Image::PPM;
			break;

		case kPOVList_FileType_BMP:
			ext = ASCIItoUCS2String(".bmp");
			imagetype = Image::BMP;
			break;

		case kPOVList_FileType_OpenEXR:
			ext = ASCIItoUCS2String(".exr");
			imagetype = Image::EXR;
			break;

		case kPOVList_FileType_RadianceHDR:
			ext = ASCIItoUCS2String(".hdr");
			imagetype = Image::HDR;
			break;

#ifdef SYS_TO_STANDARD
		case kPOVList_FileType_System:
			ext = ASCIItoUCS2String(POV_SYS_FILE_EXTENSION);
			imagetype = Image::SYS_TO_STANDARD;
			break;
#endif

		default:
			throw POV_EXCEPTION_STRING("Invalid file type for output");
	}

	if (OutputIsStdout(ropts) || OutputIsStderr())
	{
		switch (imagetype)
		{
			case Image::HDR:
			case Image::PNG:
			case Image::TGA:
			case Image::PPM:
			case Image::BMP:
				break;

			default:
				throw POV_EXCEPTION_STRING("Output to STDOUT/STDERR not supported for selected file format");
		}
		return POVMS_ASCIItoUCS2String(OutputIsStdout() ? "stdout" : "stderr");
	}

	// we disallow an output filename that consists purely of the default extension
	// (e.g. Output_File_Name=".png").
	if((filename == ext) || (filename.empty() == true))
	{
		// get the input file name and merge the existing path if need be.
		if (path.Empty() == true)
		{
			path = ropts.TryGetUCS2String(kPOVAttrib_InputFile, "object.pov");
			filename = path.GetFile();
		}
		else
			filename = Path(ropts.TryGetUCS2String(kPOVAttrib_InputFile, "object.pov")).GetFile();

		// if the input file name ends with '.' or '.anything', we remove it
		UCS2String::size_type pos = filename.find_last_of('.');
		if(pos != string::npos)
			filename.erase(pos);
	}
	else if ((path.HasVolume() == false) && (path.Empty() == false))
	{
		// to get here, path must be a relative path with filename
		// if the filename ends with a '.' or with the default extension (case-sensitive),
		// we remove it.
		UCS2String::size_type pos = filename.find_last_of('.');
		if((pos != UCS2String::npos) && ((pos == filename.size() - 1) || (filename.substr(pos) == ext)))
			filename.erase(pos);
	}
	else
	{
		// if there is no path component already, get it from the input file.
		if (path.Empty() == true)
			path = ropts.TryGetUCS2String(kPOVAttrib_InputFile, "object.pov");

		// if the filename ends with a '.' or with the default extension (case-sensitive),
		// we remove it.
		UCS2String::size_type pos = filename.find_last_of('.');
		if((pos != UCS2String::npos) && ((pos == filename.size() - 1) || (filename.substr(pos) == ext)))
			filename.erase(pos);
	}

	if (digits > 0)
	{
		for(int i = 0; i < digits; i++)
			filename += '0';
		for(UCS2String::size_type i = filename.length() - 1; frame > 0; i--, frame /= 10)
			filename[i] = '0' + (frame % 10);
	}

	path.SetFile(filename + ext);

	return path();
}

}
