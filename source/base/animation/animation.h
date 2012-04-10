/*******************************************************************************
 * animation.h
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
 * $File: //depot/povray/smp/source/base/animation/animation.h $
 * $Revision: #8 $
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

#ifndef POVRAY_BASE_ANIMATION_H
#define POVRAY_BASE_ANIMATION_H

#include <vector>

#include "base/configbase.h"
#include "base/fileinputoutput.h"
#include "base/pov_err.h"
#include "base/types.h"
#include "base/image/image.h"

namespace pov_base
{

class Animation
{
	public:
		enum CodecType
		{
			LosslessCodec = 0, // file format default choice
			LossyCodec = 1, // file format default choice
			PNGCodec,
			BMPCodec,
			JPEGCodec,
			MPEG1Codec, // i-frames only - note that JPEG library DCT can be reused [trf]
			MPEG2Codec // i-frames only - note that JPEG library DCT can be reused [trf]
		};

		enum FileType
		{
			AVI,
			MOV,
			MPEG
		};

		enum ColorEncodingType
		{
			RGB,
			YUV,
			YUV422,
			YUV420,
			YUV411,
			YUV410
		};

		struct ReadOptions
		{
			float gamma;
			bool gammacorrect;

			ReadOptions() : gamma(1.0f), gammacorrect(false) { }
		};

		struct WriteOptions
		{
			ColorEncodingType colorencoding;
			unsigned char compress; // relative quality from 100 best to 0 worst
			unsigned char bpcc; // bits per colour component
			bool alphachannel;
			float gamma;
			float blurradius; // blur radius 0.0 to 1.0 (for better lossy compression without aa) - turned off by 0.0
			float bluredgethreshold; // edge threshold 0.0 to 1.0 (grayscale difference of clipped pixels, only blur if difference larger than threshold) - turned off by 1.0
			float framespersecond; // has to support very odd framerates i.e. for NTSC - defaults to standard 24 fps cinema/movie frame rate

			WriteOptions() : colorencoding(YUV), compress(75), alphachannel(false), gamma(1.0f), blurradius(0.0), bluredgethreshold(1.0), framespersecond(24.0) { }
		};

		virtual ~Animation();

		static Animation *Open(FileType aftype, IStream *file, const ReadOptions& options = ReadOptions()); // reading only
		static Animation *Open(FileType aftype, CodecType codec, OStream *file, unsigned int w, unsigned int h, const WriteOptions& options = WriteOptions()); // writing only

		void AppendFrame(Image *image); // writing only - NOTE: This method reserves the right to *modify* the image passed to it!!! [trf]

		Image *ReadNextFrame(); // reading only

		float GetLengthInSeconds() const;
		unsigned int GetLengthInFrames() const;

		unsigned int GetCurrentFrame() const; // reading only
		void SetCurrentFrame(unsigned int frame); // reading only

		unsigned int GetWidth() const { return width; }
		unsigned int GetHeight() const { return height; }

		const vector<string>& GetWarnings() const;
		void ClearWarnings();
	protected:
		FileType fileType;
		IStream *inFile;
		OStream *outFile;
		unsigned int width;
		unsigned int height;
		ReadOptions readOptions;
		WriteOptions writeOptions;
		vector<string> warnings;
		CodecType codec;
		unsigned int currentFrame;
		unsigned int totalFrames;
		float frameDuration;

		Animation(FileType aftype, IStream *file, const ReadOptions& options);
		Animation(FileType aftype, CodecType codec, OStream *file, unsigned int w, unsigned int h, const WriteOptions& options);

		Image *ReadFrame(IStream *file);
		POV_LONG WriteFrame(OStream *file, const Image *image);

		void ComputeBlurMask(const Image& image, Image& mask);
		void GetBlurredPixel(const Image& image, unsigned int x, unsigned int y, float& red, float& green, float& blue);
	private:
		void *state;
		float blurMatrix[16][16]; // only uses 15 x 15 maximum (16 x 16 for better alignment)
		int blurMatrixRadius;

		/// not available
		Animation();
		/// not available
		Animation(const Animation&);
		/// not available
		Animation& operator=(Animation&);
};

}

#endif // POVRAY_BASE_ANIMATION_H
