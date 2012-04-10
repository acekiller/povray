/*******************************************************************************
 * types.h
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
 * $File: //depot/povray/smp/source/base/types.h $
 * $Revision: #35 $
 * $Change: 5408 $
 * $DateTime: 2011/02/21 15:17:08 $
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

#ifndef POVRAY_BASE_TYPES_H
#define POVRAY_BASE_TYPES_H

#include <algorithm>
#include <vector>

#include "base/configbase.h"

namespace pov_base
{

// Get minimum/maximum of three values.
template<typename T>
inline T max3(T x, T y, T z) { return max(x, max(y, z)); }
template<typename T>
inline T min3(T x, T y, T z) { return min(x, min(y, z)); }

template<typename T>
inline T clip(T val, T minv, T maxv);

template<typename T>
inline T clip(T val, T minv, T maxv)
{
	if(val < minv)
		return minv;
	else if(val > maxv)
		return maxv;
	else
		return val;
}

template<typename T>
class GenericRGBColour;

#define RED_INTENSITY   0.297
#define GREEN_INTENSITY 0.589
#define BLUE_INTENSITY  0.114

template<typename T>
class GenericColour
{
	public:
		typedef DBL EXPRESS[5];
		typedef COLC COLOUR[5];
		typedef T DATA[5];

		enum
		{
			RED    = 0,
			GREEN  = 1,
			BLUE   = 2,
			FILTER = 3,
			TRANSM = 4
		};

		GenericColour()
		{
			colour[RED] = 0.0;
			colour[GREEN] = 0.0;
			colour[BLUE] = 0.0;
			colour[FILTER] = 0.0;
			colour[TRANSM] = 0.0;
		}

		explicit GenericColour(T grey)
		{
			colour[RED] = grey;
			colour[GREEN] = grey;
			colour[BLUE] = grey;
			colour[FILTER] = 0.0;
			colour[TRANSM] = 0.0;
		}

		explicit GenericColour(const GenericRGBColour<T>& col);

		explicit GenericColour(const GenericRGBColour<T>& col, T nfilter, T ntransm);

		GenericColour(T nred, T ngreen, T nblue)
		{
			colour[RED] = nred;
			colour[GREEN] = ngreen;
			colour[BLUE] = nblue;
			colour[FILTER] = 0.0;
			colour[TRANSM] = 0.0;
		}

		GenericColour(T nred, T ngreen, T nblue, T nfilter, T ntransm)
		{
			colour[RED] = nred;
			colour[GREEN] = ngreen;
			colour[BLUE] = nblue;
			colour[FILTER] = nfilter;
			colour[TRANSM] = ntransm;
		}

		explicit GenericColour(const COLOUR col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
			colour[FILTER] = col[FILTER];
			colour[TRANSM] = col[TRANSM];
		}

		explicit GenericColour(const EXPRESS col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
			colour[FILTER] = col[FILTER];
			colour[TRANSM] = col[TRANSM];
		}

		GenericColour(const GenericColour& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
			colour[FILTER] = col[FILTER];
			colour[TRANSM] = col[TRANSM];
		}

		template<typename T2>
		explicit GenericColour(const GenericColour<T2>& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
			colour[FILTER] = col[FILTER];
			colour[TRANSM] = col[TRANSM];
		}

		GenericColour(vector<POVMSFloat>::const_iterator& it, bool filter = true, bool transmit = true)
		{
			colour[RED] = *it++;
			colour[GREEN] = *it++;
			colour[BLUE] = *it++;
			if (filter)
				colour[FILTER] = *it++;
			if (transmit)
				colour[TRANSM] = *it++;
		}

		GenericColour& operator=(const GenericColour& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
			colour[FILTER] = col[FILTER];
			colour[TRANSM] = col[TRANSM];
			return *this;
		}

		GenericColour& operator=(const T& col)
		{
			colour[RED] = col;
			colour[GREEN] = col;
			colour[BLUE] = col;
			colour[FILTER] = 0.0f;
			colour[TRANSM] = 0.0f;
			return *this;
		}

		T operator[](int idx) const { return colour[idx]; }
		T& operator[](int idx) { return colour[idx]; }

		const DATA& operator*() const { return colour; }
		DATA& operator*() { return colour; }

		T red() const { return colour[RED]; }
		T& red() { return colour[RED]; }

		T green() const { return colour[GREEN]; }
		T& green() { return colour[GREEN]; }

		T blue() const { return colour[BLUE]; }
		T& blue() { return colour[BLUE]; }

		T filter() const { return colour[FILTER]; }
		T& filter() { return colour[FILTER]; }

		T transm() const { return colour[TRANSM]; }
		T& transm() { return colour[TRANSM]; }

		T greyscale() const { return RED_INTENSITY * colour[RED] + GREEN_INTENSITY * colour[GREEN] + BLUE_INTENSITY * colour[BLUE]; }

		// TODO: find a more correct way of handling alpha <-> filter/transmit
		static void AtoFT(T alpha, T& f, T& t) { f = 0.0f; t = 1.0f - alpha; }
		void AtoFT(T alpha) { colour[FILTER] = 0.0f; colour[TRANSM] = 1.0f - alpha; }
		static T FTtoA(T /*f*/, T t) { return 1.0f - t; }
		T FTtoA() const { return 1.0f - colour[TRANSM]; }

		void clear()
		{
			colour[RED] = 0.0;
			colour[GREEN] = 0.0;
			colour[BLUE] = 0.0;
			colour[FILTER] = 0.0;
			colour[TRANSM] = 0.0;
		}

		void set(T grey)
		{
			colour[RED] = grey;
			colour[GREEN] = grey;
			colour[BLUE] = grey;
			colour[FILTER] = 0.0;
			colour[TRANSM] = 0.0;
		}

		void set(T nred, T ngreen, T nblue)
		{
			colour[RED] = nred;
			colour[GREEN] = ngreen;
			colour[BLUE] = nblue;
			colour[FILTER] = 0.0;
			colour[TRANSM] = 0.0;
		}

		void set(T nred, T ngreen, T nblue, T nfilter, T ntransm)
		{
			colour[RED] = nred;
			colour[GREEN] = ngreen;
			colour[BLUE] = nblue;
			colour[FILTER] = nfilter;
			colour[TRANSM] = ntransm;
		}

		GenericColour clip(T minc, T maxc)
		{
			return GenericColour(pov_base::clip<T>(colour[RED], minc, maxc),
			                     pov_base::clip<T>(colour[GREEN], minc, maxc),
			                     pov_base::clip<T>(colour[BLUE], minc, maxc),
			                     pov_base::clip<T>(colour[FILTER], minc, maxc),
			                     pov_base::clip<T>(colour[TRANSM], minc, maxc));
		}

		GenericRGBColour<T> rgbTransm() const;

		GenericColour operator+(const GenericColour& b) const
		{
			return GenericColour(colour[RED] + b[RED], colour[GREEN] + b[GREEN], colour[BLUE] + b[BLUE], colour[FILTER] + b[FILTER], colour[TRANSM] + b[TRANSM]);
		}

		GenericColour operator-(const GenericColour& b) const
		{
			return GenericColour(colour[RED] - b[RED], colour[GREEN] - b[GREEN], colour[BLUE] - b[BLUE], colour[FILTER] - b[FILTER], colour[TRANSM] - b[TRANSM]);
		}

		GenericColour operator*(const GenericColour& b) const
		{
			return GenericColour(colour[RED] * b[RED], colour[GREEN] * b[GREEN], colour[BLUE] * b[BLUE], colour[FILTER] * b[FILTER], colour[TRANSM] * b[TRANSM]);
		}

		GenericColour operator/(const GenericColour& b) const
		{
			return GenericColour(colour[RED] / b[RED], colour[GREEN] / b[GREEN], colour[BLUE] / b[BLUE], colour[FILTER] / b[FILTER], colour[TRANSM] / b[TRANSM]);
		}

		GenericColour& operator+=(const GenericColour& b)
		{
			colour[RED] += b[RED];
			colour[GREEN] += b[GREEN];
			colour[BLUE] += b[BLUE];
			colour[FILTER] += b[FILTER];
			colour[TRANSM] += b[TRANSM];
			return *this;
		}

		GenericColour& operator-=(const GenericColour& b)
		{
			colour[RED] -= b[RED];
			colour[GREEN] -= b[GREEN];
			colour[BLUE] -= b[BLUE];
			colour[FILTER] -= b[FILTER];
			colour[TRANSM] -= b[TRANSM];
			return *this;
		}

		GenericColour& operator*=(const GenericColour& b)
		{
			colour[RED] *= b[RED];
			colour[GREEN] *= b[GREEN];
			colour[BLUE] *= b[BLUE];
			colour[FILTER] *= b[FILTER];
			colour[TRANSM] *= b[TRANSM];
			return *this;
		}

		GenericColour& operator/=(const GenericColour& b)
		{
			colour[RED] /= b[RED];
			colour[GREEN] /= b[GREEN];
			colour[BLUE] /= b[BLUE];
			colour[FILTER] /= b[FILTER];
			colour[TRANSM] /= b[TRANSM];
			return *this;
		}

		GenericColour operator-() const
		{
			return GenericColour(-colour[RED], -colour[GREEN], -colour[BLUE], -colour[FILTER], -colour[TRANSM]);
		}

		GenericColour operator+(DBL b) const
		{
			return GenericColour(colour[RED] + b, colour[GREEN] + b, colour[BLUE] + b, colour[FILTER] + b, colour[TRANSM] + b);
		}

		GenericColour operator-(DBL b) const
		{
			return GenericColour(colour[RED] - b, colour[GREEN] - b, colour[BLUE] - b, colour[FILTER] - b, colour[TRANSM] - b);
		}

		GenericColour operator*(DBL b) const
		{
			return GenericColour(colour[RED] * b, colour[GREEN] * b, colour[BLUE] * b, colour[FILTER] * b, colour[TRANSM] * b);
		}

		GenericColour operator/(DBL b) const
		{
			return GenericColour(colour[RED] / b, colour[GREEN] / b, colour[BLUE] / b, colour[FILTER] / b, colour[TRANSM] / b);
		}

		GenericColour& operator+=(DBL b)
		{
			colour[RED] += b;
			colour[GREEN] += b;
			colour[BLUE] += b;
			colour[FILTER] += b;
			colour[TRANSM] += b;
			return *this;
		}

		GenericColour& operator-=(DBL b)
		{
			colour[RED] -= b;
			colour[GREEN] -= b;
			colour[BLUE] -= b;
			colour[FILTER] -= b;
			colour[TRANSM] -= b;
			return *this;
		}

		GenericColour& operator*=(DBL b)
		{
			colour[RED] *= b;
			colour[GREEN] *= b;
			colour[BLUE] *= b;
			colour[FILTER] *= b;
			colour[TRANSM] *= b;
			return *this;
		}

		GenericColour& operator/=(DBL b)
		{
			colour[RED] /= b;
			colour[GREEN] /= b;
			colour[BLUE] /= b;
			colour[FILTER] /= b;
			colour[TRANSM] /= b;
			return *this;
		}
	private:
		DATA colour;
};

template<typename T>
class GenericRGBColour
{
	public:
		typedef DBL VECTOR[3];
		typedef COLC RGB[3];
		typedef T DATA[3];

		enum
		{
			RED    = 0,
			GREEN  = 1,
			BLUE   = 2
		};

		GenericRGBColour()
		{
			colour[RED] = 0.0;
			colour[GREEN] = 0.0;
			colour[BLUE] = 0.0;
		}

		explicit GenericRGBColour(T grey)
		{
			colour[RED] = grey;
			colour[GREEN] = grey;
			colour[BLUE] = grey;
		}

		GenericRGBColour(T nred, T ngreen, T nblue)
		{
			colour[RED] = nred;
			colour[GREEN] = ngreen;
			colour[BLUE] = nblue;
		}

		explicit GenericRGBColour(const RGB col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
		}

		explicit GenericRGBColour(const VECTOR col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
		}

		explicit GenericRGBColour(const GenericColour<T>& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
		}

		GenericRGBColour(const GenericRGBColour& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
		}

		template<typename T2>
		explicit GenericRGBColour(const GenericRGBColour<T2>& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
		}

		GenericRGBColour(vector<POVMSFloat>::const_iterator& it)
		{
			colour[RED] = *it++;
			colour[GREEN] = *it++;
			colour[BLUE] = *it++;
		}

		GenericRGBColour& operator=(const GenericRGBColour& col)
		{
			colour[RED] = col[RED];
			colour[GREEN] = col[GREEN];
			colour[BLUE] = col[BLUE];
			return *this;
		}

		GenericRGBColour& operator=(const T& col)
		{
			colour[RED] = col;
			colour[GREEN] = col;
			colour[BLUE] = col;
			return *this;
		}

		T operator[](int idx) const { return colour[idx]; }
		T& operator[](int idx) { return colour[idx]; }

		const DATA& operator*() const { return colour; }
		DATA& operator*() { return colour; }

		T red() const { return colour[RED]; }
		T& red() { return colour[RED]; }

		T green() const { return colour[GREEN]; }
		T& green() { return colour[GREEN]; }

		T blue() const { return colour[BLUE]; }
		T& blue() { return colour[BLUE]; }

		T greyscale() const { return RED_INTENSITY * colour[RED] + GREEN_INTENSITY * colour[GREEN] + BLUE_INTENSITY * colour[BLUE]; }

		bool isZero() const { return (colour[RED] == 0) && (colour[GREEN] == 0) && (colour[BLUE] == 0); }

		void clear()
		{
			colour[RED] = 0.0;
			colour[GREEN] = 0.0;
			colour[BLUE] = 0.0;
		}

		void set(T grey)
		{
			colour[RED] = grey;
			colour[GREEN] = grey;
			colour[BLUE] = grey;
		}

		void set(T nred, T ngreen, T nblue)
		{
			colour[RED] = nred;
			colour[GREEN] = ngreen;
			colour[BLUE] = nblue;
		}

		GenericRGBColour clip(T minc, T maxc)
		{
			return GenericRGBColour(pov_base::clip<T>(colour[RED], minc, maxc),
			                        pov_base::clip<T>(colour[GREEN], minc, maxc),
			                        pov_base::clip<T>(colour[BLUE], minc, maxc));
		}

		GenericRGBColour operator+(const GenericRGBColour& b) const
		{
			return GenericRGBColour(colour[RED] + b[RED], colour[GREEN] + b[GREEN], colour[BLUE] + b[BLUE]);
		}

		GenericRGBColour operator-(const GenericRGBColour& b) const
		{
			return GenericRGBColour(colour[RED] - b[RED], colour[GREEN] - b[GREEN], colour[BLUE] - b[BLUE]);
		}

		GenericRGBColour operator*(const GenericRGBColour& b) const
		{
			return GenericRGBColour(colour[RED] * b[RED], colour[GREEN] * b[GREEN], colour[BLUE] * b[BLUE]);
		}

		GenericRGBColour operator/(const GenericRGBColour& b) const
		{
			return GenericRGBColour(colour[RED] / b[RED], colour[GREEN] / b[GREEN], colour[BLUE] / b[BLUE]);
		}

		GenericRGBColour& operator+=(const GenericRGBColour& b)
		{
			colour[RED] += b[RED];
			colour[GREEN] += b[GREEN];
			colour[BLUE] += b[BLUE];
			return *this;
		}

		GenericRGBColour& operator-=(const GenericRGBColour& b)
		{
			colour[RED] -= b[RED];
			colour[GREEN] -= b[GREEN];
			colour[BLUE] -= b[BLUE];
			return *this;
		}

		GenericRGBColour& operator*=(const GenericRGBColour& b)
		{
			colour[RED] *= b[RED];
			colour[GREEN] *= b[GREEN];
			colour[BLUE] *= b[BLUE];
			return *this;
		}

		GenericRGBColour& operator/=(const GenericRGBColour& b)
		{
			colour[RED] /= b[RED];
			colour[GREEN] /= b[GREEN];
			colour[BLUE] /= b[BLUE];
			return *this;
		}

		GenericRGBColour operator-() const
		{
			return GenericRGBColour(-colour[RED], -colour[GREEN], -colour[BLUE]);
		}

		GenericRGBColour operator+(DBL b) const
		{
			return GenericRGBColour(colour[RED] + b, colour[GREEN] + b, colour[BLUE] + b);
		}

		GenericRGBColour operator-(DBL b) const
		{
			return GenericRGBColour(colour[RED] - b, colour[GREEN] - b, colour[BLUE] - b);
		}

		GenericRGBColour operator*(DBL b) const
		{
			return GenericRGBColour(colour[RED] * b, colour[GREEN] * b, colour[BLUE] * b);
		}

		GenericRGBColour operator/(DBL b) const
		{
			return GenericRGBColour(colour[RED] / b, colour[GREEN] / b, colour[BLUE] / b);
		}

		GenericRGBColour& operator+=(DBL b)
		{
			colour[RED] += b;
			colour[GREEN] += b;
			colour[BLUE] += b;
			return *this;
		}

		GenericRGBColour& operator-=(DBL b)
		{
			colour[RED] -= b;
			colour[GREEN] -= b;
			colour[BLUE] -= b;
			return *this;
		}

		GenericRGBColour& operator*=(DBL b)
		{
			colour[RED] *= b;
			colour[GREEN] *= b;
			colour[BLUE] *= b;
			return *this;
		}

		GenericRGBColour& operator/=(DBL b)
		{
			colour[RED] /= b;
			colour[GREEN] /= b;
			colour[BLUE] /= b;
			return *this;
		}
	private:
		DATA colour;
};

template<typename T>
inline GenericColour<T>::GenericColour(const GenericRGBColour<T>& col)
{
	colour[RED] = col[RED];
	colour[GREEN] = col[GREEN];
	colour[BLUE] = col[BLUE];
	colour[FILTER] = 0.0;
	colour[TRANSM] = 0.0;
}

template<typename T>
inline GenericColour<T>::GenericColour(const GenericRGBColour<T>& col, T nfilter, T ntransm)
{
	colour[RED] = col[RED];
	colour[GREEN] = col[GREEN];
	colour[BLUE] = col[BLUE];
	colour[FILTER] = nfilter;
	colour[TRANSM] = ntransm;
}

template<typename T>
inline GenericRGBColour<T> GenericColour<T>::rgbTransm() const
{
	return GenericRGBColour<T>( colour[RED]   * colour[FILTER] + colour[TRANSM],
	                            colour[GREEN] * colour[FILTER] + colour[TRANSM],
	                            colour[BLUE]  * colour[FILTER] + colour[TRANSM] );
}

template<typename T>
inline GenericColour<T> operator* (double a, const GenericColour<T>& b) { return b * a; }
template<typename T>
inline GenericRGBColour<T> operator* (double a, const GenericRGBColour<T>& b) { return b * a; }

template<typename T>
inline double colourDistance (const GenericColour<T>& a, const GenericColour<T>& b) { return fabs(a.red() - b.red()) + fabs(a.green() - b.green()) + fabs(a.blue() - b.blue()); }
template<typename T>
inline double colourDistance (const GenericRGBColour<T>& a, const GenericRGBColour<T>& b) { return fabs(a.red() - b.red()) + fabs(a.green() - b.green()) + fabs(a.blue() - b.blue()); }

template<typename T>
inline GenericRGBColour<T> Sqr(const GenericRGBColour<T>& a) { return a * a; }
template<typename T>
inline GenericRGBColour<T> exp(const GenericRGBColour<T>& a) { return GenericRGBColour<T>(::exp(a.red()), ::exp(a.green()), ::exp(a.blue())); }
template<typename T>
inline GenericRGBColour<T> sqrt(const GenericRGBColour<T>& a) { return GenericRGBColour<T>(::sqrt(a.red()), ::sqrt(a.green()), ::sqrt(a.blue())); }

typedef GenericColour<COLC>    Colour;
typedef GenericColour<DBL>     DblColour;
typedef GenericRGBColour<COLC> RGBColour;
typedef GenericRGBColour<DBL>  DblRGBColour;

struct POVRect
{
	unsigned int top;
	unsigned int left;
	unsigned int bottom;
	unsigned int right;

	POVRect() : top(0), left(0), bottom(0), right(0) { }
	POVRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) :
		top(y1), left(x1), bottom(y2), right(x2) { }

	unsigned int GetArea() const { return ((bottom - top + 1) * (right - left + 1)); }
	unsigned int GetWidth() const { return (right - left + 1); }
	unsigned int GetHeight() const { return (bottom - top + 1); }
};

class GenericSetting
{
	public:
		explicit GenericSetting(bool set = false): set(set) {}
		void Unset() { set = false; }
		bool isSet() const { return set; }
	protected:
		bool set;
};

class FloatSetting : public GenericSetting
{
	public:
		explicit FloatSetting(double data = 0.0, bool set = false): data(data), GenericSetting(set) {}
		double operator=(double b)      { data = b; set = true; return data; }
		operator double() const         { return data; }
		double operator()(double def)   { if (set) return data; else return def; }
	private:
		double  data;
};

}

#endif // POVRAY_BASE_TYPES_H
