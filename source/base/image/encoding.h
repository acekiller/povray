/*******************************************************************************
 * encoding.h
 *
 * This file contains code for handling image data quantization.
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
 * $File: //depot/povray/smp/source/base/image/encoding.h $
 * $Revision: #2 $
 * $Change: 5233 $
 * $DateTime: 2010/12/04 15:37:56 $
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

#ifndef POVRAY_BASE_IMAGE_ENCODING_H
#define POVRAY_BASE_IMAGE_ENCODING_H

#include <boost/thread.hpp>

#include "base/configbase.h"
#include "base/types.h"
#include "base/image/colourspace.h"

namespace pov_base
{

class Image;

/// Abstract class representing dithering rules and respective state information.
class DitherHandler
{
    public:
        struct OffsetInfo
        {
            union { float red, gray; };  float green, blue, alpha;
			OffsetInfo() : red(0.0f), green(0.0f), blue(0.0f), alpha(0.0f) { }
			OffsetInfo(float r, float g, float b, float a) : red(r), green(g), blue(b), alpha(a) { }
            void clear() { red = 0.0f; green = 0.0f; blue = 0.0f; alpha = 0.0f; }
            void setAll(float v) { red = v; green = v; blue = v; alpha = v; }
            OffsetInfo operator*(float b) const { return OffsetInfo(red*b, green*b, blue*b, alpha*b); }
            OffsetInfo operator+(const OffsetInfo& b) const { return OffsetInfo(red+b.red, green+b.green, blue+b.blue, alpha+b.alpha); }
            void operator+=(const OffsetInfo& b) { red+=b.red; green+=b.green; blue+=b.blue; alpha+=b.alpha; }
        };

        virtual ~DitherHandler() {}

        /// Computes an offset to be added to the pixel value.
        /// @param[in]  x       X coordinate of the pixel (may or may not be relevant to the handler).
        /// @param[in]  y       Y coordinate of the pixel (may or may not be relevant to the handler).
        /// @param[out] offLin  Linear offset to add before any encoding steps
        /// @param[out] offQnt  Offset to add right before quantization (even after scaling)
        virtual void getOffset(unsigned int x, unsigned int y, OffsetInfo& offLin, OffsetInfo& offQnt) = 0;

        /// Informs the handler about the actual quantization error observed for one particular pixel.
        /// @param[in]  x       X coordinate of the pixel (may or may not be relevant to the handler).
        /// @param[in]  y       Y coordinate of the pixel (may or may not be relevant to the handler).
        /// @param[in]  err     Linear quantization error.
        virtual void setError(unsigned int x, unsigned int y, const OffsetInfo& err) {}
};

typedef boost::shared_ptr<DitherHandler> DitherHandlerPtr;

/// Factory class to get a dithering rule and state.
DitherHandlerPtr GetDitherHandler(int method, unsigned int imageWidth);

/// Factory class to get a no-op dithering rule.
DitherHandlerPtr GetNoOpDitherHandler();

/// Computes an offset to be added to a pixel value right before quantization (even after scaling).
/// The value returned by this function is based on a default stateless dithering rule.
float GetDitherOffset(unsigned int x, unsigned int y);

/*******************************************************************************/

/**
 *  Linear decoding function.
 *  This function maps an integer value in the range 0..max linearly to a floating-point value in the range 0..1.
 *  @param  x       value to decode
 *  @param  max     encoded value representing 1.0
 */
inline float IntDecode(unsigned int x, unsigned int max) { return float(x) / float(max); }

/**
 *  Generic decoding function.
 *  This function maps an integer value in the range 0..max to a floating-point value in the range 0..1
 *  using a specific transfer function (gamma curve).
 *  @param  g       transfer function (gamma curve) to use
 *  @param  x       value to decode
 *  @param  max     encoded value representing 1.0
 */
inline float IntDecode(const GammaCurvePtr& g, unsigned int x, unsigned int max) { return GammaCurve::Decode(g,IntDecode(x,max)); }

/**
 *  Linear encoding function.
 *  This function maps a floating-point value in the range 0..1 linearly to in an integer value in the range 0..max.
 *  @note           Floating-point values outside the range 0..1 are clipped, mapping them to 0 or max, respectively.
 *  @param[in]      x       value to encode
 *  @param[in]      max     encoded value representing 1.0
 */
inline unsigned int IntEncode(float x, unsigned int max) { return (unsigned int)floor( clip(x,0.0f,1.0f) * float(max) + 0.5f ); }

/**
 *  Linear encoding function.
 *  This function maps a floating-point value in the range 0..1 linearly to in an integer value in the range 0..max.
 *  @note           Floating-point values outside the range 0..1 are clipped, mapping them to 0 or max, respectively.
 *  @param[in]      x       value to encode
 *  @param[in]      max     encoded value representing 1.0
 *  @param[in]      qOff    offset to add before quantization
 */
inline unsigned int IntEncode(float x, unsigned int max, float qOff)
{
    return IntEncode(x+qOff/float(max),max);
}

/**
 *  Linear encoding function.
 *  This function maps a floating-point value in the range 0..1 linearly to in an integer value in the range 0..max.
 *  @note           Floating-point values outside the range 0..1 are clipped, mapping them to 0 or max, respectively.
 *  @param[in]      x       value to encode
 *  @param[in]      max     encoded value representing 1.0
 *  @param[in]      qOff    offset to add before quantization
 *  @param[out]     err     quantization error (including effects due to adding qOff)
 */
inline unsigned int IntEncode(float x, unsigned int max, float qOff, float& err)
{
    unsigned int v = IntEncode(x,max,qOff);
    err = clip(x,0.0f,1.0f) - IntDecode(v,max);
    return v;
}

/**
 *  Generic encoding function.
 *  This function maps a floating-point value in the range 0..1 to in an integer value in the range 0..max.
 *  @note           Floating-point values outside the range 0..1 (after applying the transfer function) are clipped,
 *                  mapping them to 0 or max, respectively.
 *  @param[in]      g       transfer function (gamma curve) to use
 *  @param[in]      x       value to encode
 *  @param[in]      max     encoded value representing 1.0
 */
inline unsigned int IntEncode(const GammaCurvePtr& g, float x, unsigned int max) { return IntEncode(GammaCurve::Encode(g,x),max); }

/**
 *  Generic encoding function.
 *  This function maps a floating-point value in the range 0..1 to in an integer value in the range 0..max.
 *  @note           Floating-point values outside the range 0..1 (after applying the transfer function) are clipped,
 *                  mapping them to 0 or max, respectively.
 *  @param[in]      g       transfer function (gamma curve) to use
 *  @param[in]      x       value to encode
 *  @param[in]      max     encoded value representing 1.0
 *  @param[in]      qOff    offset to add before quantization
 */
inline unsigned int IntEncode(const GammaCurvePtr& g, float x, unsigned int max, float qOff)
{
    return IntEncode(GammaCurve::Encode(g,x)+qOff/float(max),max);
}

/**
 *  Generic encoding function.
 *  This function maps a floating-point value in the range 0..1 to in an integer value in the range 0..max.
 *  @note           Floating-point values outside the range 0..1 (after applying the transfer function) are clipped,
 *                  mapping them to 0 or max, respectively.
 *  @param[in]      g       transfer function (gamma curve) to use
 *  @param[in]      x       value to encode
 *  @param[in]      max     encoded value representing 1.0
 *  @param[in]      qOff    offset to add before quantization
 *  @param[out]     err     quantization error (including effects due to adding qOff)
 */
inline unsigned int IntEncode(const GammaCurvePtr& g, float x, unsigned int max, float qOff, float& err)
{
    unsigned int v = IntEncode(g,x,max,qOff);
    err = clip(x,0.0f,1.0f) - IntDecode(g,v,max);
    return v;
}

/*******************************************************************************/

// convenience functions for image file decoding

void SetEncodedGrayValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int gray);
void SetEncodedGrayAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int gray, unsigned int alpha, bool premul = false);
void SetEncodedRGBValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int red, unsigned int green, unsigned int blue);
void SetEncodedRGBAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha, bool premul = false);
void SetEncodedGrayValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float gray);
void SetEncodedGrayAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float gray, float alpha, bool premul = false);
void SetEncodedRGBValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float red, float green, float blue);
void SetEncodedRGBAValue(Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float red, float green, float blue, float alpha, bool premul = false);

// convenience functions for image file encoding
unsigned int GetEncodedGrayValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, DitherHandler& dh);
void GetEncodedGrayAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int& gray, unsigned int& alpha, DitherHandler& dh, bool premul = false);
void GetEncodedRGBValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int& red, unsigned int& green, unsigned int& blue, DitherHandler& dh);
void GetEncodedRGBAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, unsigned int max, unsigned int& red, unsigned int& green, unsigned int& blue, unsigned int& alpha, DitherHandler& dh, bool premul = false);
float GetEncodedGrayValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&);
void GetEncodedGrayAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float& gray, float& alpha, bool premul = false);
void GetEncodedRGBValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float& red, float& green, float& blue);
void GetEncodedRGBAValue(const Image* img, unsigned int x, unsigned int y, const GammaCurvePtr&, float& red, float& green, float& blue, float& alpha, bool premul = false);

}

#endif // POVRAY_BASE_IMAGE_ENCODING_H
