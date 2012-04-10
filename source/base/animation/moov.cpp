/*******************************************************************************
 * moov.cpp
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
 * $File: //depot/povray/smp/source/base/animation/moov.cpp $
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

#include <limits.h>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/pov_err.h"
#include "base/types.h"
#include "base/animation/animation.h"
#include "base/animation/moov.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

typedef POVMSType Type;

struct PrivateData
{
	unsigned int width;
	unsigned int height;
	Type componenttype;
	bool alphachannel;
	int timescale;
	int frameduration;
	POV_LONG mdatsize;
	vector<int> imagesizes;
};

namespace Moov
{

void WriteAtomHeader(OStream *file, Type type, POV_LONG size);
void WriteType(OStream *file, Type data);
void WriteInt2(OStream *file, POVMSInt data);
void WriteInt4(OStream *file, POVMSInt data);
void WriteInt8(OStream *file, POV_LONG data);
void WriteN(OStream *file, size_t cnt, POVMSInt data);

void *ReadFileHeader(IStream *file, float& lengthinseconds, unsigned int& lengthinframes, Animation::CodecType& codec, unsigned int& w, unsigned int& h, const Animation::ReadOptions& options, vector<string>& warnings)
{
	throw POV_EXCEPTION(kCannotHandleDataErr, "Reading QuickTime movie files is not supported (yet)!");
	return NULL;
}

void PreReadFrame(IStream *file, unsigned int frame, POV_LONG& bytes, Animation::CodecType& codec, const Animation::ReadOptions& options, vector<string>& warnings, void *state)
{
}

void PostReadFrame(IStream *file, unsigned int frame, POV_LONG bytes, Animation::CodecType& codec, const Animation::ReadOptions& options, vector<string>& warnings, void *state)
{
}

void FinishReadFile(IStream *file, vector<string>& warnings, void *state)
{
}

void *WriteFileHeader(OStream *file, Animation::CodecType& codec, unsigned int w, unsigned int h, const Animation::WriteOptions& options, vector<string>& warnings)
{
	PrivateData pd;

	pd.width = w;
	pd.height = h;

	// determine codec equivalent

	switch(codec)
	{
		case Animation::LosslessCodec:
		case Animation::PNGCodec:
			codec = Animation::PNGCodec;
			pd.componenttype = 'png ';
			pd.alphachannel = options.alphachannel;
			break;
		case Animation::LossyCodec:
		case Animation::JPEGCodec:
			codec = Animation::JPEGCodec;
			pd.componenttype = 'jpeg';
			pd.alphachannel = false;
			break;
		case Animation::BMPCodec:
			codec = Animation::BMPCodec;
			pd.componenttype = 'WRLE';
			pd.alphachannel = false;
			break;
		default:
			return NULL; // error - cannot handle format
	}

	if(pd.alphachannel != options.alphachannel)
		warnings.push_back("Alpha channel output not supported for this animation codec. Alpha channel output will be disabled.");

	// compute time scale

	double ts = double(options.framespersecond);
	int m = 1;

	for(m = 1; m <= 1000; m *= 10)
	{
		if((fabs((ts * double(m)) - double(int((ts * double(m))))) < 1.0e-5) && ((ts * double(m)) >= 1.0))
			break;
	}

	pd.timescale = max(int(double(options.framespersecond) * double(m)), 1);

	// frame duration according to time scale

	pd.frameduration = m;

	// movie data atom header

	WriteAtomHeader(file, 'mdat', -1);

	pd.mdatsize = 16;

	// NOTE: This allocation occurs at the end such that there cannot be a memory leak
	// should any of the previous operations fail. If and only if this function returns
	// a state other than NULL shall it be assumed to have been successful!
	return reinterpret_cast<void *>(new PrivateData(pd));
}

void PreWriteFrame(OStream *, const Animation::WriteOptions&, vector<string>&, void *state)
{
	PrivateData *pd = reinterpret_cast<PrivateData *>(state);

	if(pd == NULL)
		throw POV_EXCEPTION_CODE(kNullPointerErr);

	// there really is nothing to do here [trf]
}

void PostWriteFrame(OStream *file, POV_LONG bytes, const Animation::WriteOptions&, vector<string>&, void *state)
{
	PrivateData *pd = reinterpret_cast<PrivateData *>(state);

	if(pd == NULL)
		throw POV_EXCEPTION_CODE(kNullPointerErr);

	// update mdat size

	file->seekg(0, SEEK_END);
	pd->mdatsize = file->tellg() + 16;
	file->seekg(8, SEEK_SET);
	WriteInt8(file, pd->mdatsize);
	file->seekg(0, SEEK_END);

	if(bytes > 2147483647) // 2^31 - 1
		throw POV_EXCEPTION(kInvalidDataSizeErr, "Cannot handle frame data larger than 2^31 bytes!");

	pd->imagesizes.push_back(int(bytes));
}

void FinishWriteFile(OStream *file, const Animation::WriteOptions& options, vector<string>& warnings, void *state)
{
	PrivateData *pd = reinterpret_cast<PrivateData *>(state);

	if(pd == NULL)
		throw POV_EXCEPTION_CODE(kNullPointerErr);

	POV_LONG stsz_size = 20 + (pd->imagesizes.size() * 4);
	POV_LONG stsc_size = 28;
	POV_LONG stts_size = 32;
	POV_LONG stsd_size = 102;
	POV_LONG stbl_size = 8 + stsd_size + stts_size + stsc_size + stsz_size;
	POV_LONG vmhd_size = 20;
	POV_LONG minf_size = 8 + vmhd_size + stbl_size;
	POV_LONG hdlr_size = 32;
	POV_LONG mdhd_size = 32;
	POV_LONG mdia_size = 8 + mdhd_size + hdlr_size + minf_size;
	POV_LONG tkhd_size = 112;
	POV_LONG trak_size = 8 + tkhd_size + mdia_size;
	POV_LONG mvhd_size = 108;
	POV_LONG moov_size = 8 + mvhd_size + trak_size;

	int duration = pd->frameduration * pd->imagesizes.size();

	// write movie atom

	WriteAtomHeader(file, 'moov', moov_size);

	// write movie header atom

	WriteAtomHeader(file, 'mvhd', mvhd_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 0); // creation time
	WriteInt4(file, 0); // modification time
	WriteInt4(file, pd->timescale); // time scale
	WriteInt4(file, duration); // duration
	WriteInt4(file, 1 << 16); // preferred playback rate
	WriteInt2(file, 1 << 8); // preferred sound volume
	WriteN(file, 10, 0); // ten reserved bytes
	WriteInt4(file, 1 << 16); // matrix a
	WriteInt4(file, 0); // matrix b
	WriteInt4(file, 0); // matrix u
	WriteInt4(file, 0); // matrix c
	WriteInt4(file, 1 << 16); // matrix d
	WriteInt4(file, 0); // matrix v
	WriteInt4(file, 0); // matrix tx
	WriteInt4(file, 0); // matrix ty
	WriteInt4(file, 1 << 30); // matrix w
	WriteInt4(file, 0); // preview time
	WriteInt4(file, 0); // preview duration
	WriteInt4(file, 0); // poster time
	WriteInt4(file, 0); // selection time
	WriteInt4(file, 0); // selection duration
	WriteInt4(file, 0); // current time
	WriteInt4(file, 2); // next track id (this code uses track 1)

	// write track atom

	WriteAtomHeader(file, 'trak', trak_size);

	// write track header atom

	WriteAtomHeader(file, 'tkhd', tkhd_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 0); // creation time
	WriteInt4(file, 0); // modification time
	WriteInt4(file, 1); // track id
	WriteN(file, 4, 0); // four reserved bytes
	WriteInt4(file, duration); // duration
	WriteN(file, 8, 0); // eight reserved bytes
	WriteInt2(file, 1); // layer
	WriteInt2(file, 0); // alternate group
	WriteInt2(file, 1 << 8); // sound volume
	WriteN(file, 2, 0); // two reserved bytes
	WriteInt4(file, 1 << 16); // matrix a
	WriteInt4(file, 0); // matrix b
	WriteInt4(file, 0); // matrix u
	WriteInt4(file, 0); // matrix c
	WriteInt4(file, 1 << 16); // matrix d
	WriteInt4(file, 0); // matrix v
	WriteInt4(file, 0); // matrix tx
	WriteInt4(file, 0); // matrix ty
	WriteInt4(file, 1 << 30); // matrix w
	WriteInt4(file, pd->width << 16); // track width
	WriteInt4(file, pd->height << 16); // track height

	// write media atom

	WriteAtomHeader(file, 'mdia', mdia_size);

	// write header media atom

	WriteAtomHeader(file, 'mdhd', mdhd_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 0); // creation time
	WriteInt4(file, 0); // modification time
	WriteInt4(file, pd->timescale); // time scale
	WriteInt4(file, duration); // duration
	WriteInt2(file, 0); // language
	WriteInt2(file, 0); // quality

	// write handler atom

	WriteAtomHeader(file, 'hdlr', hdlr_size);

	WriteInt4(file, 0); // version and flags
	WriteType(file, pd->componenttype); // component type
	WriteType(file, 'vide'); // component subtype (this media is video)
	WriteInt4(file, 0); // reserved
	WriteInt4(file, 0); // reserved
	WriteInt4(file, 0); // reserved

	// write media information atom

	WriteAtomHeader(file, 'minf', minf_size);

	// write video media information header atom

	WriteAtomHeader(file, 'vmhd', vmhd_size);

	WriteInt4(file, 0); // version and flags
	WriteInt2(file, 0); // graphics mode
	WriteInt2(file, 0); // opcolor red
	WriteInt2(file, 0); // opcolor green
	WriteInt2(file, 0); // opcolor blue

	// write sample table atom

	WriteAtomHeader(file, 'stbl', stbl_size);

	// write sample description atom

	WriteAtomHeader(file, 'stsd', stsd_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 1); // number of entries (this code only needs one entry)
	WriteInt4(file, 86); // description size
	WriteType(file, pd->componenttype); // data format
	WriteInt4(file, 0); // reserved
	WriteInt2(file, 0); // reserved
	WriteInt2(file, 0); // data reference index
	WriteInt2(file, 0); // version
	WriteInt2(file, 0); // revision level
	WriteType(file, 'appl'); // vendor
	WriteInt4(file, 0); // temporal quality
	WriteInt4(file, 512); // spacial quality
	WriteInt2(file, pd->width); // width
	WriteInt2(file, pd->height); // height
	WriteInt4(file, 72 << 16); // horizontal resolution
	WriteInt4(file, 72 << 16); // vertical resolution
	WriteInt4(file, 0); // data size (required to be zero according to Apple documentation)
	WriteInt4(file, 1); // frame count
	WriteN(file, 1, 4); // name (32-byte Pascal string, so first byte is length!)
	WriteType(file, pd->componenttype); // name (continued, uses codec type for simplicity)
	WriteN(file, 27, 0); // name (continued, unused)
	WriteInt2(file, options.bpcc * (3 + (pd->alphachannel ? 1 : 0))); // depth
	WriteInt2(file, -1); // color table id

	// write time-to-sample atom

	WriteAtomHeader(file, 'stts', stts_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 1); // number of entries (this code only needs one entry)
	WriteInt4(file, pd->imagesizes.size()); // sample count
	WriteInt4(file, pd->frameduration); // sample duration

	// write sample-to-chunk atom

	WriteAtomHeader(file, 'stsc', stsc_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 1); // number of entries (this code only needs one entry)
	WriteInt4(file, 1); // first chunk
	WriteInt4(file, pd->imagesizes.size()); // samples per chunk
	WriteInt4(file, 1); // sample description id

	// write sample size atom

	WriteAtomHeader(file, 'stsz', stsz_size);

	WriteInt4(file, 0); // version and flags
	WriteInt4(file, 0); // sample size (all samples have different sizes, so this needs to be zero)
	WriteInt4(file, pd->imagesizes.size()); // number of entries

	for(vector<int>::const_iterator i = pd->imagesizes.begin(); i != pd->imagesizes.end(); i++)
		WriteInt4(file, *i); // sample size entry

	delete pd;
}
/*
void ReadAtomHeader(IStream *file, Type& type, POV_LONG& size)
{
	ReadInt4(file, size);

	if(size == 0) // atom goes up to end of file
	{
		ReadType(file, type);

		POV_LONG t = file->tellg();
		file->seekg(0, IOBase::SEEK_END);
		size = file->tellg() - t + 8;
		file->seekg(t, IOBase::SEEK_SET);
	}
	else if(size == 1) // atom sizes is outside 32-bit range
	{
		ReadType(file, type);

		ReadInt8(file, size);
	}
	else
		ReadType(file, type);
}
*/
void WriteAtomHeader(OStream *file, Type type, POV_LONG size)
{
	if(size < 0) // temporary size - always assume 64-bit size
	{
		WriteInt4(file, 1);
		WriteType(file, type);
		WriteInt8(file, 0);
	}
	else if(size > UINT_MAX) // size outside 32-bit range
	{
		WriteInt4(file, 1);
		WriteType(file, type);
		WriteInt8(file, size);
	}
	else // size within 32-bit range
	{
		WriteType(file, type);
		WriteInt4(file, size);
	}
}

void WriteType(OStream *file, Type data)
{
	file->Write_Byte((data >> 24) & 255);
	file->Write_Byte((data >> 16) & 255);
	file->Write_Byte((data >> 8) & 255);
	file->Write_Byte(data & 255);
}

void WriteInt2(OStream *file, POVMSInt data)
{
	file->Write_Byte((data >> 8) & 255);
	file->Write_Byte(data & 255);
}

void WriteInt4(OStream *file, POVMSInt data)
{
	file->Write_Byte((data >> 24) & 255);
	file->Write_Byte((data >> 16) & 255);
	file->Write_Byte((data >> 8) & 255);
	file->Write_Byte(data & 255);
}

void WriteInt8(OStream *file, POV_LONG data)
{
	file->Write_Byte((data >> 56) & 255);
	file->Write_Byte((data >> 48) & 255);
	file->Write_Byte((data >> 40) & 255);
	file->Write_Byte((data >> 32) & 255);
	file->Write_Byte((data >> 24) & 255);
	file->Write_Byte((data >> 16) & 255);
	file->Write_Byte((data >> 8) & 255);
	file->Write_Byte(data & 255);
}

void WriteN(OStream *file, size_t cnt, POVMSInt data)
{
	for(size_t i = 0; i < cnt; i++)
		file->Write_Byte(data & 255);
}

}

}
