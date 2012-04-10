/*******************************************************************************
 * animation.cpp
 *
 * This file contains ... TODO ...
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
 * $File: //depot/povray/smp/source/base/animation/animation.cpp $
 * $Revision: #10 $
 * $Change: 5009 $
 * $DateTime: 2010/06/05 10:39:30 $
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

#include <boost/scoped_ptr.hpp>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/image.h"
#include "base/image/png_pov.h"
#include "base/image/jpeg_pov.h"
#include "base/image/bmp.h"
#include "base/animation/animation.h"
//#include "base/animation/avi.h"
#include "base/animation/moov.h"
//#include "base/animation/mpeg.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

Animation::Animation(FileType aftype, IStream *file, const ReadOptions& options) :
	fileType(aftype),
	inFile(file),
	outFile(NULL),
	readOptions(options)
{
	float seconds = 0.0f;

	currentFrame = 0;

	switch(fileType)
	{
		case AVI:
		//	state = Avi::ReadFileHeader(inFile, seconds, totalFrames, codec, width, height, readOptions, warnings);
			break;
		case MOV:
			state = Moov::ReadFileHeader(inFile, seconds, totalFrames, codec, width, height, readOptions, warnings);
			break;
		case MPEG:
		//	state = Mpeg::ReadFileHeader(inFile, seconds, totalFrames, codec, width, height, readOptions, warnings);
			break;
	}

	if(state == NULL)
		throw POV_EXCEPTION(kCannotHandleDataErr, "Cannot read animation file header in the specified format!");

	frameDuration = seconds / float(totalFrames);
}

Animation::Animation(FileType aftype, CodecType c, OStream *file, unsigned int w, unsigned int h, const WriteOptions& options) :
	fileType(aftype),
	inFile(NULL),
	outFile(file),
	width(w),
	height(h),
	writeOptions(options),
	codec(c)
{
	totalFrames = 0;
	frameDuration = 1.0f / options.framespersecond;

	blurMatrixRadius = 7;

	switch(fileType)
	{
		case AVI:
		//	state = Avi::WriteFileHeader(outFile, codec, width, height, writeOptions, warnings);
			break;
		case MOV:
			state = Moov::WriteFileHeader(outFile, codec, width, height, writeOptions, warnings);
			break;
		case MPEG:
		//	state = Mpeg::WriteFileHeader(outFile, codec, width, height, writeOptions, warnings);
			break;
	}

	if(state == NULL)
		throw POV_EXCEPTION(kCannotHandleDataErr, "Cannot write animation file with the specified format and codec!");

	// TODO FIXME - build blur matrix (this code only builds an identity matrix)
	for(size_t y = 0; y < 15; y++)
	{
		for(size_t x = 0; x < 15; x++)
			blurMatrix[x][y] = 0.0f;
	}
	blurMatrix[blurMatrixRadius + 1][blurMatrixRadius + 1] = 1.0f;
}

Animation::~Animation()
{
	if(outFile != NULL)
	{
		switch(fileType)
		{
			case AVI:
			//	Avi::FinishWriteFile(outFile, writeOptions, warnings, state);
				break;
			case MOV:
				Moov::FinishWriteFile(outFile, writeOptions, warnings, state);
				break;
			case MPEG:
			//	Mpeg::FinishWriteFile(outFile, writeOptions, warnings, state);
				break;
		}
	}
	else if(inFile != NULL)
	{
		switch(fileType)
		{
			case AVI:
			//	Avi::FinishReadFile(inFile, warnings, state);
				break;
			case MOV:
				Moov::FinishReadFile(inFile, warnings, state);
				break;
			case MPEG:
			//	Mpeg::FinishReadFile(inFile, warnings, state);
				break;
		}
	}

	state = NULL;
}

Animation *Animation::Open(FileType aftype, IStream *file, const ReadOptions& options) // reading only
{
	return new Animation(aftype, file, options);
}

Animation *Animation::Open(FileType aftype, CodecType codec, OStream *file, unsigned int w, unsigned int h, const WriteOptions& options) // writing only
{
	return new Animation(aftype, codec, file, w, h, options);
}

void Animation::AppendFrame(Image *image) // writing only - NOTE: This method reserves the right to *modify* the image passed to it!!! [trf]
{
	if(writeOptions.blurradius > 0.0f)
	{
		boost::scoped_ptr<Image> mask(Image::Create(image->GetWidth(), image->GetHeight(), Image::Bit_Map));
		float r, g, b, f, t;

		mask->FillBitValue(false);

		if(writeOptions.bluredgethreshold < 1.0f)
			ComputeBlurMask(*image, *mask.get());

		for(int y = 0; y < image->GetHeight(); y++)
		{
			for(int x = 0; x < image->GetWidth(); x++)
			{
				if(mask->GetBitValue(x, y) == true)
				{
					image->GetRGBFTValue(x, y, r, g, b, f, t);
					GetBlurredPixel(*image, x, y, r, g, b);
					image->SetRGBFTValue(x, y, r, g, b, f, t);
				}
			}
		}
	}

	WriteFrame(outFile, image);

	totalFrames++;
}

Image *Animation::ReadNextFrame() // reading only
{
	currentFrame++;

	return ReadFrame(inFile);
}

float Animation::GetLengthInSeconds() const
{
	return frameDuration * totalFrames;
}

unsigned int Animation::GetLengthInFrames() const
{
	return totalFrames;
}

unsigned int Animation::GetCurrentFrame() const // reading only
{
	return currentFrame;
}

void Animation::SetCurrentFrame(unsigned int frame) // reading only
{
	currentFrame = frame;
}

const vector<string>& Animation::GetWarnings() const
{
	return warnings;
}

void Animation::ClearWarnings()
{
	warnings.clear();
}

Image *Animation::ReadFrame(IStream *file)
{
	POV_LONG bytes = 0;
	Image *image = NULL;
	Image::ReadOptions options;

	options.defaultGamma = PowerLawGammaCurve::GetByDecodingGamma(readOptions.gamma);
	options.gammacorrect = readOptions.gammacorrect;
	options.itype = Image::RGBFT_Float;

	switch(fileType)
	{
		case AVI:
		//	Avi::PreReadFrame(file, currentFrame, bytes, codec, readOptions, warnings, state);
			break;
		case MOV:
			Moov::PreReadFrame(file, currentFrame, bytes, codec, readOptions, warnings, state);
			break;
	}

	POV_LONG prepos = file->tellg();

	switch(codec)
	{
		case PNGCodec:
			image = Png::Read(file, options);
			break;
		case BMPCodec:
			image = Bmp::Read(file, options);
			break;
		case JPEGCodec:
			image = Jpeg::Read(file, options);
			break;
		case MPEG1Codec:
		case MPEG2Codec:
		//	image = Mpeg::ReadFrame(file, currentFrame, codec, readOptions, warnings, state);
			break;
	}

	if(file->tellg() < (prepos + bytes))
		warnings.push_back("Frame decompressor read fewer bytes than expected.");
	else if(file->tellg() > (prepos + bytes))
		throw POV_EXCEPTION(kInvalidDataSizeErr, "Frame decompressor read more bytes than expected. The input file may be corrupted!");

	file->seekg(prepos + bytes, SEEK_END);

	switch(fileType)
	{
		case AVI:
		//	Avi::PostReadFrame(file, currentFrame, bytes, codec, readOptions, warnings, state);
			break;
		case MOV:
			Moov::PostReadFrame(file, currentFrame, bytes, codec, readOptions, warnings, state);
			break;
	}

	return image;
}

POV_LONG Animation::WriteFrame(OStream *file, const Image *image)
{
	Image::WriteOptions options;

	options.bpcc = writeOptions.bpcc;
	options.alphachannel = writeOptions.alphachannel;
	options.compress = writeOptions.compress;
	// options.gamma = writeOptions.gamma;
	options.encodingGamma = PowerLawGammaCurve::GetByEncodingGamma(writeOptions.gamma);

	switch(fileType)
	{
		case AVI:
		//	Avi::PreWriteFrame(file, writeOptions, warnings, state);
			break;
		case MOV:
			Moov::PreWriteFrame(file, writeOptions, warnings, state);
			break;
	}

	POV_LONG bytes = file->tellg();

	switch(codec)
	{
		case PNGCodec:
			Png::Write(file, image, options);
			break;
		case BMPCodec:
			Bmp::Write(file, image, options);
			break;
		case JPEGCodec:
			// TODO FIXME Jpeg::Write(file, image, options);
			break;
		case MPEG1Codec:
		case MPEG2Codec:
		//	Mpeg::WriteFrame(file, image, codec, writeOptions, warnings, state);
			break;
	}

	bytes = (file->tellg() - bytes);

	switch(fileType)
	{
		case AVI:
		//	Avi::PostWriteFrame(file, bytes, writeOptions, warnings, state);
			break;
		case MOV:
			Moov::PostWriteFrame(file, bytes, writeOptions, warnings, state);
			break;
	}

	return bytes;
}

void Animation::ComputeBlurMask(const Image& image, Image& mask)
{
	for(int y = 0; y < image.GetHeight(); y++)
	{
		for(int x = 0; x < image.GetWidth(); x++)
		{
			for(int yy = -1; yy <= 1; yy++)
			{
				for(int xx = -1; xx <= 1; xx++)
				{
					if(fabs(image.GetGrayValue(clip(x + xx, 0, int(image.GetWidth() - 1)), clip(y + yy, 0, int(image.GetHeight() - 1))) - image.GetGrayValue(x, y)) >= writeOptions.bluredgethreshold)
						mask.SetBitValue(x, y, true);
				}
			}
		}
	}
}

void Animation::GetBlurredPixel(const Image& image, unsigned int x, unsigned int y, float& red, float& green, float& blue)
{
	red = green = blue = 0.0f;

	for(int yy = -blurMatrixRadius; yy <= blurMatrixRadius; yy++)
	{
		for(int xx = -blurMatrixRadius; xx <= blurMatrixRadius; xx++)
		{
			float scale = blurMatrix[blurMatrixRadius + xx][blurMatrixRadius + yy];
			float r = 0.0f, g = 0.0f, b = 0.0f;

			image.GetRGBValue(clip(int(x) + xx, 0, int(image.GetWidth() - 1)), clip(int(y) + yy, 0, int(image.GetHeight() - 1)), r, g, b);

			red += r * scale;
			green += g * scale;
			blue += b * scale;
		}
	}
}

}
