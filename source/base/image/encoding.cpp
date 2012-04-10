/*******************************************************************************
 * encoding.cpp
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
 * $File: //depot/povray/smp/source/base/image/encoding.cpp $
 * $Revision: #1 $
 * $Change: 5232 $
 * $DateTime: 2010/12/04 14:24:46 $
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

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/image/encoding.h"
#include "base/image/image.h"
#include "base/povmsgid.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_base
{

/*******************************************************************************/

static const unsigned int MaxBayerMatrixSize = 4;
typedef float BayerMatrix[MaxBayerMatrixSize][MaxBayerMatrixSize];

static const BayerMatrix BayerMatrices[MaxBayerMatrixSize+1] = 
{
    // dummy for 0x0
    { { 0 } },
    // 1x1 (of little use, but here it is)
    { { 1/2.0-0.5 } },
    // 2x2
    { { 1/4.0-0.5, 3/4.0-0.5 },
      { 4/4.0-0.5, 2/4.0-0.5 } },
    // 3x3
    { { 3/9.0-0.5, 7/9.0-0.5, 4/9.0-0.5 },
      { 6/9.0-0.5, 1/9.0-0.5, 9/9.0-0.5 },
      { 2/9.0-0.5, 8/9.0-0.5, 5/9.0-0.5 } },
    // 4x4
    { {  1/16.0-0.5,  9/16.0-0.5,  3/16.0-0.5, 11/16.0-0.5 },
      { 13/16.0-0.5,  5/16.0-0.5, 15/16.0-0.5,  7/16.0-0.5 },
      {  4/16.0-0.5, 12/16.0-0.5,  2/16.0-0.5, 10/16.0-0.5 },
      { 16/16.0-0.5,  8/16.0-0.5, 14/16.0-0.5,  6/16.0-0.5 } }
};

/*******************************************************************************/

/// Class representing "no-op" dithering rules.
class NoDither : public DitherHandler
{
    public:
        virtual void getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt);
};

/// Class representing bayer dithering rules, generating a regular pattern.
class BayerDither : public DitherHandler
{
    public:
        BayerDither(unsigned int mxSize);
        virtual void getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt);
        static inline float getOffset(unsigned int x, unsigned int y, unsigned int ms) { return BayerMatrices[ms][x%ms][y%ms]; }
    protected:
        OffsetInfo  lastErr;
        int         matrixSize;
};

/// Class representing simple 1D error diffusion dithering rules, carrying over the error from one pixel to the next.
class DiffusionDither1D : public DitherHandler
{
    public:
        virtual void getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt);
        virtual void setError(unsigned int x, unsigned int y, const OffsetInfo& err);
    protected:
        OffsetInfo lastErr;
};

/// Class representing simple 2D error diffusion dithering rules, carrying over the error from one pixel to the right, as well as the two pixels below.
/// @note   This implementation uses an additional 1-line pixel buffer to avoid manipulating the original image.
class DiffusionDither2D : public DitherHandler
{
    public:
        DiffusionDither2D(unsigned int width);
        virtual ~DiffusionDither2D();
        virtual void getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt);
        virtual void setError(unsigned int x, unsigned int y, const OffsetInfo& err);
    protected:
        unsigned int imageWidth;
        OffsetInfo* nextRowOffset;
        OffsetInfo* thisRowOffset;
};

/// Class representing Floyd-Steinberg dithering rules, carrying over the error from one pixel to the right, as well as the three pixels below.
/// @note   This implementation uses an additional 1-line pixel buffer to avoid manipulating the original image.
class FloydSteinbergDither : public DitherHandler
{
    public:
        FloydSteinbergDither(unsigned int width);
        virtual ~FloydSteinbergDither();
        virtual void getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt);
        virtual void setError(unsigned int x, unsigned int y, const OffsetInfo& err);
    protected:
        unsigned int imageWidth;
        OffsetInfo* nextRowOffset;
        OffsetInfo* thisRowOffset;
};

/*******************************************************************************/

void NoDither::getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt)
{
    offLin.clear();
    offQnt.clear();
}

/*******************************************************************************/

BayerDither::BayerDither(unsigned int mxSize) :
    matrixSize(min(mxSize,MaxBayerMatrixSize))
{
    ;
}

void BayerDither::getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt)
{
    offLin.clear();
    offQnt.setAll(getOffset(x, y, matrixSize));
}

/*******************************************************************************/

void DiffusionDither1D::getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt)
{
    offLin = lastErr; lastErr.clear(); offQnt.clear();
}

void DiffusionDither1D::setError(unsigned int x, unsigned int y, const OffsetInfo& err)
{
    lastErr = err;
}

/*******************************************************************************/

DiffusionDither2D::DiffusionDither2D(unsigned int width) :
    imageWidth(width),
    thisRowOffset(new OffsetInfo[width+1]),
    nextRowOffset(new OffsetInfo[width+1])
{
    ;
}

DiffusionDither2D::~DiffusionDither2D()
{
    delete[] thisRowOffset;
    delete[] nextRowOffset;
}

void DiffusionDither2D::getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt)
{
    offLin = thisRowOffset[x];
    offQnt.clear();
}

void DiffusionDither2D::setError(unsigned int x, unsigned int y, const OffsetInfo& err)
{
    if (x == 0)
    {
        OffsetInfo* tmp = nextRowOffset;
        nextRowOffset = thisRowOffset;
        thisRowOffset = tmp;
        for (unsigned int i = 0; i < imageWidth+1; i ++)
            nextRowOffset[i].clear();
    }
    thisRowOffset[x+1] += err * (2/4.0); // pixel to the right
    nextRowOffset[x]   += err * (1/4.0); // pixel below
    nextRowOffset[x+1] += err * (1/4.0); // pixel below right
}

/*******************************************************************************/

FloydSteinbergDither::FloydSteinbergDither(unsigned int width) :
    imageWidth(width),
    thisRowOffset(new OffsetInfo[width+2]),
    nextRowOffset(new OffsetInfo[width+2])
{
    ;
}

FloydSteinbergDither::~FloydSteinbergDither()
{
    delete[] thisRowOffset;
    delete[] nextRowOffset;
}

void FloydSteinbergDither::getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt)
{
    offLin = thisRowOffset[x+1];
    offQnt.clear();
}

void FloydSteinbergDither::setError(unsigned int x, unsigned int y, const OffsetInfo& err)
{
    if (x == 0)
    {
        OffsetInfo* tmp = nextRowOffset;
        nextRowOffset = thisRowOffset;
        thisRowOffset = tmp;
        for (unsigned int i = 0; i < imageWidth+2; i ++)
            nextRowOffset[i].clear();
    }
    thisRowOffset[x+2] += err * (7/16.0); // pixel to the right
    nextRowOffset[x]   += err * (3/16.0); // pixel below left
    nextRowOffset[x+1] += err * (5/16.0); // pixel below
    nextRowOffset[x+2] += err * (1/16.0); // pixel below right
}

/*******************************************************************************/

DitherHandlerPtr GetDitherHandler(int method, unsigned int imageWidth)
{
    switch (method)
    {
        case kPOVList_DitherMethod_None:            return DitherHandlerPtr(new NoDither());
        case kPOVList_DitherMethod_Diffusion1D:     return DitherHandlerPtr(new DiffusionDither1D());
        case kPOVList_DitherMethod_Diffusion2D:     return DitherHandlerPtr(new DiffusionDither2D(imageWidth));
        case kPOVList_DitherMethod_FloydSteinberg:  return DitherHandlerPtr(new FloydSteinbergDither(imageWidth));
        case kPOVList_DitherMethod_Bayer2x2:        return DitherHandlerPtr(new BayerDither(2));
        case kPOVList_DitherMethod_Bayer3x3:        return DitherHandlerPtr(new BayerDither(3));
        case kPOVList_DitherMethod_Bayer4x4:        return DitherHandlerPtr(new BayerDither(4));
        default:                                    throw POV_EXCEPTION_STRING("Invalid dither method for output");
    }
}

DitherHandlerPtr GetNoOpDitherHandler()
{
    return DitherHandlerPtr(new NoDither());
}

/*******************************************************************************/

float GetDitherOffset(unsigned int x, unsigned int y)
{
    return BayerDither::getOffset(x,y,4);
}

/*******************************************************************************/

void SetEncodedGrayValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int gray)
{
	if (!img->IsIndexed() && img->GetMaxIntValue() == max && GammaCurve::IsNeutral(g))
		// avoid potential re-quantization in case we have a pretty match between encoded data and container
		img->SetGrayValue(x, y, gray);
	else
		img->SetGrayValue(x, y, IntDecode(g,gray,max));
}
void SetEncodedGrayAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int gray, unsigned int alpha, bool premul)
{
	bool doPremultiply   = (alpha != max) && !premul && (img->IsPremultiplied() || !img->HasTransparency()); // need to apply premultiplication if encoded data isn't PM'ed but container content should be
	bool doUnPremultiply = (alpha != max) && premul && !img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	if (!doPremultiply && !doUnPremultiply && !img->IsIndexed() && img->GetMaxIntValue() == max && GammaCurve::IsNeutral(g))
		// avoid potential re-quantization in case we have a pretty match between encoded data and container
		img->SetGrayAValue(x, y, gray, alpha);
	else
	{
		float fAlpha = IntDecode(alpha,max);
		float fGray  = IntDecode(g,gray,max);
		if (doPremultiply)
			fGray *= fAlpha;
		else if (doUnPremultiply && alpha != 0)
			fGray /= fAlpha;
		// else no need to worry about premultiplication (or can't compensate anyway)
		img->SetGrayAValue(x, y, fGray, fAlpha);
	}
}
void SetEncodedRGBValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int red, unsigned int green, unsigned int blue)
{
	if (!img->IsIndexed() && img->GetMaxIntValue() == max && GammaCurve::IsNeutral(g))
		// avoid potential re-quantization in case we have a pretty match between encoded data and container
		img->SetRGBValue(x, y, red, green, blue);
	else
		img->SetRGBValue(x, y, IntDecode(g,red,max), IntDecode(g,green,max), IntDecode(g,blue,max));
}
void SetEncodedRGBAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha, bool premul)
{
	bool doPremultiply   = (alpha != max) && !premul && (img->IsPremultiplied() || !img->HasTransparency()); // need to apply premultiplication if encoded data isn't PM'ed but container content should be
	bool doUnPremultiply = (alpha != max) && premul && !img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	if (!doPremultiply && !doUnPremultiply && !img->IsIndexed() && img->GetMaxIntValue() == max && GammaCurve::IsNeutral(g))
		// avoid potential re-quantization in case we have a pretty match between encoded data and container
		img->SetRGBAValue(x, y, red, green, blue, alpha);
	else
	{
		float fAlpha = IntDecode(alpha,  max);
		float fRed   = IntDecode(g,red,  max);
		float fGreen = IntDecode(g,green,max);
		float fBlue  = IntDecode(g,blue, max);
		if (doPremultiply)
		{
			fRed   *= fAlpha;
			fGreen *= fAlpha;
			fBlue  *= fAlpha;
		}
		else if (doUnPremultiply && alpha != 0)
		{
			fRed   /= fAlpha;
			fGreen /= fAlpha;
			fBlue  /= fAlpha;
		}
		// else no need to worry about premultiplication (or can't compensate anyway)
		img->SetRGBAValue(x, y, fRed, fGreen, fBlue, fAlpha);
	}
}
void SetEncodedGrayValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float gray)
{
	img->SetGrayValue(x, y, GammaCurve::Decode(g,gray));
}
void SetEncodedGrayAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float gray, float alpha, bool premul)
{
	bool doPremultiply   = !premul && (img->IsPremultiplied() || !img->HasTransparency()); // need to apply premultiplication if encoded data isn't PM'ed but container content should be
	bool doUnPremultiply = premul && !img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	gray = GammaCurve::Decode(g,gray);
	if (doPremultiply)
		gray *= alpha;
	else if (doUnPremultiply && alpha != 0) // TODO maybe use some epsilon?!
		gray /= alpha;
	// else no need to worry about premultiplication (or can't compensate anyway)
	img->SetGrayAValue(x, y, gray, alpha);
}
void SetEncodedRGBValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float red, float green, float blue)
{
	img->SetRGBValue(x, y, GammaCurve::Decode(g,red), GammaCurve::Decode(g,green), GammaCurve::Decode(g,blue));
}
void SetEncodedRGBAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float red, float green, float blue, float alpha, bool premul)
{
	bool doPremultiply   = !premul && (img->IsPremultiplied() || !img->HasTransparency()); // need to apply premultiplication if encoded data isn't PM'ed but container content should be
	bool doUnPremultiply = premul && !img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	red   = GammaCurve::Decode(g,red);
	green = GammaCurve::Decode(g,green);
	blue  = GammaCurve::Decode(g,blue);
	if (doPremultiply)
	{
		red   *= alpha;
		green *= alpha;
		blue  *= alpha;
	}
	else if (doUnPremultiply && alpha != 0.0) // TODO maybe use some epsilon?!
	{
		red   /= alpha;
		green /= alpha;
		blue  /= alpha;
	}
	// else no need to worry about premultiplication (or can't compensate anyway)
	img->SetRGBAValue(x, y, red, green, blue, alpha);
}

unsigned int GetEncodedGrayValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, DitherHandler& dh)
{
	float fGray;
	if (!img->IsPremultiplied() && img->HasTransparency())
	{
		// data has transparency and is stored non-premultiplied; precompose against a black background
		float fAlpha;
		img->GetGrayAValue(x, y, fGray, fAlpha);
		fGray *= fAlpha;
	}
	else
	{
		// no need to worry about premultiplication
		fGray = img->GetGrayValue(x, y);
	}
	DitherHandler::OffsetInfo linOff, encOff;
	dh.getOffset(x,y,linOff,encOff);
	unsigned int iGray = IntEncode(g,fGray+linOff.gray,max,encOff.gray,linOff.gray);
	dh.setError(x,y,linOff);
	return iGray;
}
void GetEncodedGrayAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int& gray, unsigned int& alpha, DitherHandler& dh, bool premul)
{
	bool doPremultiply   = premul && !img->IsPremultiplied() && img->HasTransparency(); // need to apply premultiplication if encoded data should be premul'ed but container content isn't
	bool doUnPremultiply = !premul && img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	float fGray, fAlpha;
	img->GetGrayAValue(x, y, fGray, fAlpha);
	if (doPremultiply)
		fGray *= fAlpha;
	else if (doUnPremultiply && fAlpha != 0) // TODO maybe use some epsilon?!
		fGray /= fAlpha;
	// else no need to worry about premultiplication
	if (!premul)
	{
		// Data is to be encoded un-premultiplied, so clipping will happen /before/ multiplying with alpha (because the latter is done in the viewer),
		// which is equivalent to clipping pre-multiplied components to be no greater than alpha; compensate for this by increasing opacity
		// of any exceptionally bright pixels.
		if (fGray > 1.0)
		{
			float fFactor = fGray;
			if (fFactor * fAlpha > 1.0)
				fFactor = 1.0/fAlpha;
			// this keeps the product of alpha*color constant
			fAlpha *= fFactor;
			fGray  /= fFactor;
		}
	}
	DitherHandler::OffsetInfo linOff, encOff;
	dh.getOffset(x,y,linOff,encOff);
	gray  = IntEncode(g, fGray + linOff.gray,  max, encOff.gray,  linOff.gray);
	alpha = IntEncode(fAlpha   + linOff.alpha, max, encOff.alpha, linOff.alpha);
	dh.setError(x,y,linOff);
}
void GetEncodedRGBValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int& red, unsigned int& green, unsigned int& blue, DitherHandler& dh)
{
	float fRed, fGreen, fBlue;
	if (!img->IsPremultiplied() && img->HasTransparency())
	{
		float fAlpha;
		// data has transparency and is stored non-premultiplied; precompose against a black background
		img->GetRGBAValue(x, y, fRed, fGreen, fBlue, fAlpha);
		fRed   *= fAlpha;
		fGreen *= fAlpha;
		fBlue  *= fAlpha;
	}
	else
	{
		// no need to worry about premultiplication
		img->GetRGBValue(x, y, fRed, fGreen, fBlue);
	}
	DitherHandler::OffsetInfo linOff, encOff;
	dh.getOffset(x,y,linOff,encOff);
	red   = IntEncode(g,fRed   + linOff.red,   max, encOff.red,   linOff.red);
	green = IntEncode(g,fGreen + linOff.green, max, encOff.green, linOff.green);
	blue  = IntEncode(g,fBlue  + linOff.blue,  max, encOff.blue,  linOff.blue);
	dh.setError(x,y,linOff);
}
void GetEncodedRGBAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, unsigned int max, unsigned int& red, unsigned int& green, unsigned int& blue, unsigned int& alpha, DitherHandler& dh, bool premul)
{
	bool doPremultiply   = premul && !img->IsPremultiplied() && img->HasTransparency(); // need to apply premultiplication if encoded data should be premul'ed but container content isn't
	bool doUnPremultiply = !premul && img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	float fRed, fGreen, fBlue, fAlpha;
	img->GetRGBAValue(x, y, fRed, fGreen, fBlue, fAlpha);
	if (doPremultiply)
	{
		fRed   *= fAlpha;
		fGreen *= fAlpha;
		fBlue  *= fAlpha;
	}
	else if (doUnPremultiply && fAlpha != 0) // TODO maybe use some epsilon?!
	{
		fRed   /= fAlpha;
		fGreen /= fAlpha;
		fBlue  /= fAlpha;
	}
	// else no need to worry about premultiplication
	if (!premul)
	{
		// Data is to be encoded un-premultiplied, so clipping will happen /before/ multiplying with alpha (because the latter is done in the viewer),
		// which is equivalent to clipping pre-multiplied components to be no greater than alpha; compensate for this by increasing opacity
		// of any exceptionally bright pixels.
		float fBright = min(fRed, min(fGreen, fBlue));
		if (fBright > 1.0)
		{
			float fFactor = fBright;
			if (fFactor * fAlpha > 1.0)
				fFactor = 1.0/fAlpha;
			// this keeps the product of alpha*color constant
			fAlpha *= fFactor;
			fRed   /= fFactor;
			fGreen /= fFactor;
			fBlue  /= fFactor;
		}
	}
	DitherHandler::OffsetInfo linOff, encOff;
	dh.getOffset(x,y,linOff,encOff);
	red   = IntEncode(g,fRed   + linOff.red,   max, encOff.red,   linOff.red);
	green = IntEncode(g,fGreen + linOff.green, max, encOff.green, linOff.green);
	blue  = IntEncode(g,fBlue  + linOff.blue,  max, encOff.blue,  linOff.blue);
	alpha = IntEncode(fAlpha   + linOff.alpha, max, encOff.alpha, linOff.alpha);
	dh.setError(x,y,linOff);
}

float GetEncodedGrayValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g)
{
	float gray;
	if (!img->IsPremultiplied() && img->HasTransparency())
	{
		// data has transparency and is stored non-premultiplied; precompose against a black background
		float alpha;
		img->GetGrayAValue(x, y, gray, alpha);
		gray *= alpha;
	}
	else
	{
		// no need to worry about premultiplication
		gray = img->GetGrayValue(x, y);
	}
	return GammaCurve::Encode(g,gray);
}
void GetEncodedGrayAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float& gray, float& alpha, bool premul)
{
	bool doPremultiply   = premul && !img->IsPremultiplied() && img->HasTransparency(); // need to apply premultiplication if encoded data should be premul'ed but container content isn't
	bool doUnPremultiply = !premul && img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	img->GetGrayAValue(x, y, gray, alpha);
	if (doPremultiply)
		gray *= alpha;
	else if (doUnPremultiply && alpha != 0) // TODO maybe use some epsilon?!
		gray /= alpha;
	// else no need to worry about premultiplication
	gray = GammaCurve::Encode(g,gray);
}
void GetEncodedRGBValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float& red, float& green, float& blue)
{
	if (!img->IsPremultiplied() && img->HasTransparency())
	{
		// data has transparency and is stored non-premultiplied; precompose against a black background
		float alpha;
		img->GetRGBAValue(x, y, red, green, blue, alpha);
		red   *= alpha;
		green *= alpha;
		blue  *= alpha;
	}
	else
	{
		// no need to worry about premultiplication
		img->GetRGBValue(x, y, red, green, blue);
	}
	red   = GammaCurve::Encode(g,red);
	green = GammaCurve::Encode(g,green);
	blue  = GammaCurve::Encode(g,blue);
}
void GetEncodedRGBAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr& g, float& red, float& green, float& blue, float& alpha, bool premul)
{
	bool doPremultiply   = premul && !img->IsPremultiplied() && img->HasTransparency(); // need to apply premultiplication if encoded data should be premul'ed but container content isn't
	bool doUnPremultiply = !premul && img->IsPremultiplied() && img->HasTransparency(); // need to undo premultiplication if other way round
	img->GetRGBAValue(x, y, red, green, blue, alpha);
	if (doPremultiply)
	{
		red   *= alpha;
		green *= alpha;
		blue  *= alpha;
	}
	else if (doUnPremultiply && alpha != 0) // TODO maybe use some epsilon?!
	{
		red   /= alpha;
		green /= alpha;
		blue  /= alpha;
	}
	// else no need to worry about premultiplication
	red   = GammaCurve::Encode(g,red);
	green = GammaCurve::Encode(g,green);
	blue  = GammaCurve::Encode(g,blue);
}

}
