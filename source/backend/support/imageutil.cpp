/*******************************************************************************
 * imageutil.cpp
 *
 * This module implements the mapped textures including image map, bump map
 * and material map.
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
 * $File: //depot/povray/smp/source/backend/support/imageutil.cpp $
 * $Revision: #33 $
 * $Change: 5400 $
 * $DateTime: 2011/02/08 14:32:42 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/support/imageutil.h"
#include "backend/math/vector.h"
#include "backend/pattern/pattern.h"
#include "backend/colour/colour.h"
#include "backend/support/fileutil.h"
#include "base/pov_err.h"

#ifdef SYS_IMAGE_HEADER
#include SYS_IMAGE_HEADER
#endif

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Local variables
******************************************************************************/

const DBL DIV_1_BY_65535 = 1.0 / 65535.0;
const DBL DIV_1_BY_255 = 1.0 / 255.0;



/*****************************************************************************
* Static functions
******************************************************************************/

static int cylindrical_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL *v);
static int torus_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL *v);
static int spherical_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL *v);
static int planar_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL *v);
static void no_interpolation(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul);
static void bilinear(DBL *factors, DBL x, DBL y);
static void norm_dist(DBL *factors, DBL x, DBL y);
static void cubic(DBL *factors, DBL x);
static void Interp(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul);
static void InterpolateBicubic(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul);
static void image_colour_at(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index); // TODO ALPHA - caller should decide whether to prefer premultiplied or non-premultiplied alpha
static void image_colour_at(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul);
static int map_pos(const VECTOR EPoint, const TPATTERN *Turb, DBL *xcoor, DBL *ycoor);

/*
 * 2-D to 3-D Procedural Texture Mapping of a Bitmapped Image onto an Object:
 * 
 * A. Simplistic (planar) method of image projection devised by DKB and AAC:
 * 
 * 1. Transform texture in 3-D space if requested. 2. Determine local object 2-d
 * coords from 3-d coords by <X Y Z> triple. 3. Return pixel color value at
 * that position on the 2-d plane of "Image". 3. Map colour value in Image
 * [0..255] to a more normal colour range [0..1].
 * 
 * B. Specialized shape projection variations by Alexander Enzmann:
 * 
 * 1. Cylindrical mapping 2. Spherical mapping 3. Torus mapping
 */

/*****************************************************************************
*
* FUNCTION
*
*   image_map
*
* INPUT
*
*   EPoint   -- 3-D point at which function is evaluated
*   Pigment  -- Pattern containing various parameters
*
* OUTPUT
*
*   Colour   -- color at EPoint
*
* RETURNS
*
*   int - true,  if current point on the image map
*         false, if current point is not on the image map
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION   : Determines color of a 3-D point from a 2-D bitmap
*
* CHANGES
*
******************************************************************************/

bool image_map(const VECTOR EPoint, const PIGMENT *Pigment, Colour& colour)
{
	// TODO ALPHA - the caller does expect non-premultiplied data, but maybe he could profit from premultiplied data?

	int reg_number;
	DBL xcoor = 0.0, ycoor = 0.0;

	// If outside map coverage area, return clear

	if(map_pos(EPoint, ((const TPATTERN *) Pigment), &xcoor, &ycoor))
	{
		colour = Colour(1.0, 1.0, 1.0, 0.0, 1.0);
		return false;
	}
	else
		image_colour_at(Pigment->Vals.image, xcoor, ycoor, colour, &reg_number, false);

	return true;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Very different stuff than the other routines here. This routine takes an
*   intersection point and a texture and returns a new texture based on the
*   index/color of that point in an image/materials map. CdW 7/91
*
* CHANGES
*
******************************************************************************/

TEXTURE *material_map(const VECTOR EPoint, const TEXTURE *Texture)
{
	int reg_number = -1;
	int Material_Number;
	int numtex;
	DBL xcoor = 0.0, ycoor = 0.0;
	Colour colour;
	TEXTURE *Temp_Tex;

	/*
	 * Now we have transformed x, y, z we use image mapping routine to determine
	 * texture index.
	 */

	if(map_pos(EPoint, ((const TPATTERN *) Texture), &xcoor, &ycoor))
		Material_Number = 0;
	else
	{
		image_colour_at(Texture->Vals.image, xcoor, ycoor, colour, &reg_number); // TODO ALPHA - we should decide whether we prefer premultiplied or non-premultiplied alpha

		if(reg_number == -1)
			Material_Number = (int)(colour[pRED] * 255.0);
		else
			Material_Number = reg_number;
	}

	if(Material_Number > Texture->Num_Of_Mats)
		Material_Number %= Texture->Num_Of_Mats;

	for(numtex = 0, Temp_Tex = Texture->Materials;
	    (Temp_Tex->Next_Material != NULL) && (numtex < Material_Number);
	    Temp_Tex = Temp_Tex->Next_Material, numtex++)
	{
		// do nothing
	}

	return (Temp_Tex);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void bump_map(const VECTOR EPoint, const TNORMAL *Tnormal, VECTOR normal)
{
	DBL xcoor = 0.0, ycoor = 0.0;
	int index = -1, index2 = -1, index3 = -1;
	Colour colour1, colour2, colour3;
	VECTOR p1, p2, p3;
	VECTOR bump_normal;
	VECTOR xprime, yprime, zprime, Temp;
	DBL Length;
	DBL Amount = Tnormal->Amount;
	const ImageData *image = Tnormal->Vals.image;

	// going to have to change this
	// need to know if bump point is off of image for all 3 points

	if(map_pos(EPoint, (const TPATTERN *) Tnormal, &xcoor, &ycoor))
		return;
	else
		image_colour_at(image, xcoor, ycoor, colour1, &index); // TODO ALPHA - we should decide whether we prefer premultiplied or non-premultiplied alpha

	xcoor--;
	ycoor++;

	if(xcoor < 0.0)
		xcoor += (DBL)image->iwidth;
	else if(xcoor >= image->iwidth)
		xcoor -= (DBL)image->iwidth;

	if(ycoor < 0.0)
		ycoor += (DBL)image->iheight;
	else if(ycoor >= (DBL)image->iheight)
		ycoor -= (DBL)image->iheight;

	image_colour_at(image, xcoor, ycoor, colour2, &index2); // TODO ALPHA - we should decide whether we prefer premultiplied or non-premultiplied alpha

	xcoor += 2.0;

	if(xcoor < 0.0)
		xcoor += (DBL)image->iwidth;
	else if(xcoor >= image->iwidth)
		xcoor -= (DBL)image->iwidth;

	image_colour_at(image, xcoor, ycoor, colour3, &index3); // TODO ALPHA - we should decide whether we prefer premultiplied or non-premultiplied alpha

	if(image->Use || (index == -1) || (index2 == -1) || (index3 == -1))
	{
		p1[X] = 0;
		p1[Y] = Amount * colour1.greyscale();
		p1[Z] = 0;

		p2[X] = -1;
		p2[Y] = Amount * colour2.greyscale();
		p2[Z] = 1;

		p3[X] = 1;
		p3[Y] = Amount * colour3.greyscale();
		p3[Z] = 1;
	}
	else
	{
		p1[X] = 0;
		p1[Y] = Amount * index;
		p1[Z] = 0;

		p2[X] = -1;
		p2[Y] = Amount * index2;
		p2[Z] = 1;

		p3[X] = 1;
		p3[Y] = Amount * index3;
		p3[Z] = 1;
	}

	// we have points 1,2,3 for a triangle now we need the surface normal for it

	VSub(xprime, p1, p2);
	VSub(yprime, p3, p2);
	VCross(bump_normal, yprime, xprime);
	VNormalize(bump_normal, bump_normal);

	Assign_Vector(yprime, normal);
	Make_Vector(Temp, 0.0, 1.0, 0.0);
	VCross(xprime, yprime, Temp);
	VLength(Length, xprime);

	if(Length < EPSILON)
	{
		if(fabs(normal[Y] - 1.0) < EPSILON)
		{
			Make_Vector(yprime, 0.0, 1.0, 0.0);
			Make_Vector(xprime, 1.0, 0.0, 0.0);
			Length = 1.0;
		}
		else
		{
			Make_Vector(yprime, 0.0, -1.0, 0.0);
			Make_Vector(xprime, 1.0, 0.0, 0.0);
			Length = 1.0;
		}
	}

	VScaleEq(xprime, 1.0 / Length);
	VCross(zprime, xprime, yprime);
	VNormalizeEq(zprime);
	VScaleEq(xprime, bump_normal[X]);
	VScaleEq(yprime, bump_normal[Y]);
	VScaleEq(zprime, bump_normal[Z]);
	VAdd(Temp, xprime, yprime);
	VScaleEq(zprime, -1);
	VAdd(normal, Temp, zprime);
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR    Nathan Kopp
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

DBL image_pattern(const VECTOR EPoint, const TPATTERN *TPattern)
{
	DBL xcoor = 0.0, ycoor = 0.0;
	int index = -1;
	Colour colour;
	const ImageData *image = TPattern->Vals.image;
	DBL Value;

	colour.clear();

	// going to have to change this
	// need to know if bump point is off of image for all 3 points

	if(map_pos(EPoint, TPattern, &xcoor, &ycoor))
		return 0.0;
	else
		image_colour_at(image, xcoor, ycoor, colour, &index); // TODO ALPHA - we should decide whether we prefer premultiplied or non-premultiplied alpha

	if((index == -1) || image->Use)
	{
		if(image->Use == USE_ALPHA)
		{
			// use alpha channel or red channel
			if(image->data->HasTransparency() == true)
				Value = colour[pTRANSM];
			else
				Value = colour[pRED];   // otherwise, just use the red channel
		}
		else
			// use grey-scaled version of the color
			Value = colour.greyscale();
	}
	else
		Value = index / 255.0;

	if(Value < 0)
		Value = 0;
	else if(Value > 1.0)
		Value = 1.0;

	return Value;
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void image_colour_at(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index)
{
	// TODO ALPHA - caller should decide whether to prefer premultiplied or non-premultiplied alpha

	// As the caller isn't sure whether to prefer premultiplied or non-premultiplied alpha,
	// we'll just give him what the original image source provided.
	image_colour_at(image, xcoor, ycoor, colour, index, image->data->IsPremultiplied());
}

static void image_colour_at(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul)
{
	*index = -1;

	// if either source or destination uses premultiplied alpha, make sure interpolation is done in premultiplied space
	// as it makes the mathematical operations cleaner
	bool getPremul = premul || image->data->IsPremultiplied();

	switch(image->Interpolation_Type)
	{
		case NO_INTERPOLATION:
			no_interpolation(image, xcoor, ycoor, colour, index, getPremul);
			break;
		case BICUBIC:
			InterpolateBicubic(image, xcoor, ycoor, colour, index, getPremul);
			break;
		default:
			Interp(image, xcoor, ycoor, colour, index, getPremul);
			break;
	}

	if (!premul && getPremul)
	{
		// we fetched premultiplied data, but caller expects it non-premultiplied, so we need to fix that
		float alpha = colour.FTtoA();
		if (float(alpha) > EPSILON)
		{
			colour.red()   /= alpha;
			colour.green() /= alpha;
			colour.blue()  /= alpha;
		}
	}
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

HF_VAL image_height_at(const ImageData *image, int x, int y)
{
	// TODO ALPHA - handling of premultiplied vs. non-premultiplied alpha needs to be considered

	float r, g, b;

	// for 8-bit indexed images, use the index (scaled to match short int range)
	if (image->data->IsIndexed())
		return ((HF_VAL)image->data->GetIndexedValue(x, y) * 256); // TODO FIXME - should be *257 to get a max value of 255*257 = 65535

	// for greyscale images, use the float greyscale value (scaled to match short int range)
	if (image->data->IsGrayscale())
		return ((HF_VAL) (image->data->GetGrayValue(x, y) * 65535.0f));

	image->data->GetRGBValue(x, y, r, g, b);

	// for images with high bit depth (>8 bit per color channel), use the float greyscale value (scaled to match short int range)
	if (image->data->GetMaxIntValue() > 255)
		return ((HF_VAL) (GREY_SCALE3 (r, g, b) * 65535.0f));

	// for images with low bit depth (<=8 bit per color channel), compose from red (high byte) and green (low byte) channel.
	return ((HF_VAL) ((r * 255.0 + g) * 255.0)); // TODO FIXME - should be (r*256+g)*255 to properly mimick the high byte / low byte effect
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

bool is_image_opaque(const ImageData *image)
{
	return image->data->IsOpaque();
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Map a point (x, y, z) on a cylinder of radius 1, height 1, that has its axis
*   of symmetry along the y-axis to the square [0,1]x[0,1].
*
* CHANGES
*
******************************************************************************/

static int cylindrical_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL  *v)
{
	DBL len, theta;
	DBL x = EPoint[X];
	DBL y = EPoint[Y];
	DBL z = EPoint[Z];

	if((image->Once_Flag) && ((y < 0.0) || (y > 1.0)))
		return 0;

	*v = fmod(y * image->height, (DBL)image->height);

	// Make sure this vector is on the unit sphere.

	len = sqrt(x * x + y * y + z * z);

	if (len == 0.0)
	{
		return 0;
	}
	else
	{
		x /= len;
		z /= len;
	}

	// Determine its angle from the point (1, 0, 0) in the x-z plane.

	len = sqrt(x * x + z * z);

	if(len == 0.0)
		return 0;
	else
	{
		if(z == 0.0)
		{
			if(x > 0)
				theta = 0.0;
			else
				theta = M_PI;
		}
		else
		{
			theta = acos(x / len);

			if(z < 0.0)
				theta = TWO_M_PI - theta;
		}

		theta /= TWO_M_PI;  // This will be from 0 to 1
	}

	*u = (theta * image->width);

	return 1;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Map a point (x, y, z) on a torus  to a 2-d image.
*
* CHANGES
*
******************************************************************************/

static int torus_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL  *v)
{
	DBL len, phi, theta;
	DBL r0;
	DBL x = EPoint[X];
	DBL y = EPoint[Y];
	DBL z = EPoint[Z];

	r0 = image->Gradient[X];

	// Determine its angle from the x-axis.

	len = sqrt(x * x + z * z);

	if(len == 0.0)
		return 0;
	else if(z == 0.0)
	{
		if(x > 0)
			theta = 0.0;
		else
			theta = M_PI;
	}
	else
	{
		theta = acos(x / len);

		if(z < 0.0)
			theta = TWO_M_PI - theta;
	}

	theta = 0.0 - theta;

	// Now rotate about the y-axis to get the point (x, y, z) into the x-y plane.

	x = len - r0;

	len = sqrt(x * x + y * y);

	phi = acos(-x / len);

	if(y > 0.0)
		phi = TWO_M_PI - phi;

	// Determine the parametric coordinates.

	theta /= TWO_M_PI;

	phi /= TWO_M_PI;

	*u = (-theta * image->width);

	*v = (phi * image->height);

	return 1;
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Map a point (x, y, z) on a sphere of radius 1 to a 2-d image. (Or is it the
*   other way around?)
*
* CHANGES
*
******************************************************************************/

static int spherical_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL *v)
{
	DBL len, phi, theta;
	DBL x = EPoint[X];
	DBL y = EPoint[Y];
	DBL z = EPoint[Z];

	// Make sure this vector is on the unit sphere.

	len = sqrt(x * x + y * y + z * z);

	if(len == 0.0)
		return 0;
	else
	{
		x /= len;
		y /= len;
		z /= len;
	}

	// Determine its angle from the x-z plane.

	phi = 0.5 + asin(y) / M_PI; // This will be from 0 to 1


	// Determine its angle from the point (1, 0, 0) in the x-z plane.

	len = sqrt(x * x + z * z);

	if(len == 0.0)
		// This point is at one of the poles. Any value of xcoord will be ok...
		theta = 0;
	else
	{
		if(z == 0.0)
		{
			if(x > 0)
				theta = 0.0;
			else
				theta = M_PI;
		}
		else
		{
			theta = acos(x / len);

			if(z < 0.0)
				theta = TWO_M_PI - theta;
		}

		theta /= TWO_M_PI;  // This will be from 0 to 1
	}

	*u = (theta * image->width);
	*v = (phi * image->height);

	return 1;
}

/*
 * 2-D to 3-D Procedural Texture Mapping of a Bitmapped Image onto an Object:
 * 
 * Simplistic planar method of object image projection devised by DKB and AAC.
 * 
 * 1. Transform texture in 3-D space if requested. 2. Determine local object 2-d
 * coords from 3-d coords by <X Y Z> triple. 3. Return pixel color value at
 * that position on the 2-d plane of "Image". 3. Map colour value in Image
 * [0..255] to a more normal colour range [0..1].
 */



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Return 0 if there is no color at this point (i.e. invisible), return 1 if a
*   good mapping is found.
*
* CHANGES
*
******************************************************************************/

static int planar_image_map(const VECTOR EPoint, const ImageData *image, DBL *u, DBL  *v)
{
	DBL x = EPoint[X];
	DBL y = EPoint[Y];
	DBL z = EPoint[Z];

	if(image->Gradient[X] != 0.0)
	{
		if((image->Once_Flag) && ((x < 0.0) || (x > 1.0)))
			return 0;

		if(image->Gradient[X] > 0)
			*u = fmod(x * image->width, (DBL) image->width);
		else
			*v = fmod(x * image->height, (DBL) image->height);
	}

	if(image->Gradient[Y] != 0.0)
	{
		if((image->Once_Flag) && ((y < 0.0) || (y > 1.0)))
			return 0;

		if(image->Gradient[Y] > 0)
			*u = fmod(y * image->width, (DBL) image->width);
		else
			*v = fmod(y * image->height, (DBL) image->height);
	}

	if(image->Gradient[Z] != 0.0)
	{
		if((image->Once_Flag) && ((z < 0.0) || (z > 1.0)))
			return 0;

		if(image->Gradient[Z] > 0)
			*u = fmod(z * image->width, (DBL) image->width);
		else
			*v = fmod(z * image->height, (DBL) image->height);
	}

	return 1;
}


/*****************************************************************************
*
* FUNCTION
*
*   map_pos
*
* INPUT
*
*   EPoint   -- 3-D point at which function is evaluated
*   TPattern -- Pattern containing various parameters
*
* OUTPUT
*
*   xcoor, ycoor -- 2-D result
*
* RETURNS
*
*   Map returns 1 if point off of map 0 if on map
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION   : Maps a 3-D point to a 2-D point depending upon map type
*
* CHANGES
*
******************************************************************************/

static int map_pos(const VECTOR EPoint, const TPATTERN *TPattern, DBL *xcoor, DBL  *ycoor)
{
	const ImageData *image = TPattern->Vals.image;

	// Now determine which mapper to use.

	switch(image->Map_Type)
	{
		case PLANAR_MAP:
			if(!planar_image_map(EPoint, image, xcoor, ycoor))
				return (1);
			break;
		case SPHERICAL_MAP:
			if(!spherical_image_map(EPoint, image, xcoor, ycoor))
				return (1);
			break;
		case CYLINDRICAL_MAP:
			if(!cylindrical_image_map(EPoint, image, xcoor, ycoor))
				return (1);
			break;
		case TORUS_MAP:
			if(!torus_image_map(EPoint, image, xcoor, ycoor))
				return (1);
			break;
		default:
			if(!planar_image_map(EPoint, image, xcoor, ycoor))
				return (1);
			break;
	}

	// Now make sure the point is on the image
	// and apply integer repeats and offsets
	*xcoor += image->Offset[U] + EPSILON;
	*ycoor += image->Offset[V] + EPSILON;

	DBL xx=(*xcoor)/(DBL)(image->iwidth);
	DBL yy=(*ycoor)/(DBL)(image->iheight);

	if(image->Once_Flag)
	{
		if((xx>1.0) || (yy>1.0) || (xx<0.0) || (yy<0.0))
			return (1);
	}

	*xcoor -= ((int)xx)*image->iwidth;
	*ycoor -= ((int)yy)*image->iheight;

	// Compensate for y coordinates on the images being upsidedown

	*ycoor = (DBL)image->iheight - *ycoor;

	if(*xcoor < 0.0)
		*xcoor += (DBL)image->iwidth;
	else if(*xcoor >= (DBL)image->iwidth)
		*xcoor -= (DBL)image->iwidth;

	if(*ycoor < 0.0)
		*ycoor += (DBL)image->iheight;
	else if(*ycoor >= (DBL)image->iheight)
		*ycoor -= (DBL)image->iheight;

	if((*xcoor >= (DBL)image->iwidth) ||
	   (*ycoor >= (DBL)image->iheight) ||
	   (*xcoor < 0.0) || (*ycoor < 0.0))
		throw POV_EXCEPTION_STRING("Picture index out of range.");

	return (0);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void no_interpolation(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul)
{
	int iycoor, ixcoor;

	if(image->Once_Flag)
	{
		// image is to be seen only once, so when taking samples for interpolation
		// coordinates should not wrap around; instead, take the samples from the nearest
		// pixel right at the edge of the image

		if(xcoor < 0.0)
			ixcoor = 0;
		else if(xcoor >= (DBL)image->iwidth)
			ixcoor = image->iwidth - 1;
		else
			ixcoor = (int)xcoor;

		if(ycoor < 0.0)
			iycoor = 0;
		else if(ycoor >= (DBL)image->iheight)
			iycoor = image->iheight - 1;
		else
			iycoor = (int)ycoor;
	}
	else
	{
		// image is to be repeated, so when taking samples for interpolation
		// have coordinates wrap around

		while(xcoor < 0.0)
			xcoor += (DBL)image->iwidth;
		while(xcoor >= (DBL)image->iwidth)
			xcoor -= (DBL)image->iwidth;

		while(ycoor < 0.0)
			ycoor += (DBL)image->iheight;
		while(ycoor >= (DBL)image->iheight)
			ycoor -= (DBL)image->iheight;

		ixcoor = (int)xcoor;
		iycoor = (int)ycoor;
	}

	image->data->GetRGBFTValue(ixcoor, iycoor, colour, premul);

	if(image->data->IsIndexed() == false)
	{
		*index = -1;

		// Note: Transmit_all supplements alpha channel
		// TODO ALPHA - check how this affects premultiplied/non-premultiplied alpha considerations
		colour[pTRANSM] += image->AllTransmit;
		colour[pFILTER] += image->AllFilter;
	}
	else
		*index = image->data->GetIndexedValue(ixcoor, iycoor);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

// Interpolate color and filter values when mapping

static void Interp(const ImageData *image, DBL xcoor, DBL ycoor, Colour& colour, int *index, bool premul)
{
	int iycoor, ixcoor, i;
	int Corners_Index[4];
	Colour Corner_Colour[4];
	DBL Corner_Factors[4];

	xcoor += 0.5;
	ycoor += 0.5;

	iycoor = (int)ycoor;
	ixcoor = (int)xcoor;

	no_interpolation(image, (DBL)ixcoor,     (DBL)iycoor,     Corner_Colour[0], &Corners_Index[0], premul);
	no_interpolation(image, (DBL)ixcoor - 1, (DBL)iycoor,     Corner_Colour[1], &Corners_Index[1], premul);
	no_interpolation(image, (DBL)ixcoor,     (DBL)iycoor - 1, Corner_Colour[2], &Corners_Index[2], premul);
	no_interpolation(image, (DBL)ixcoor - 1, (DBL)iycoor - 1, Corner_Colour[3], &Corners_Index[3], premul);

	if(image->Interpolation_Type == BILINEAR)
		bilinear(Corner_Factors, xcoor, ycoor);
	else if(image->Interpolation_Type == NORMALIZED_DIST)
		norm_dist(Corner_Factors, xcoor, ycoor);

	// We're using double precision for the colors here to avoid higher-than-1.0 results due to rounding errors,
	// which would otherwise lead to stray dot artifacts when clamped to [0..1] range for a color_map or similar.
	// (Note that strictly speaking we don't avoid such rounding errors, but rather make them small enough that
	// subsequent rounding to single precision will take care of them.)
	DblColour temp_colour;
	DBL temp_index = 0;
	for (i = 0; i < 4; i ++)
	{
		temp_colour += DblColour(Corner_Colour[i]) * Corner_Factors[i];
		temp_index  += Corners_Index[i]            * Corner_Factors[i];
	}
	colour = Colour(temp_colour);
	*index = (int)temp_index;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

// Interpolate color and filter values when mapping

static void cubic(DBL *factors, DBL x)
{
	DBL p = x-(int)x;
	DBL q = 1-p;
	factors[0] = -0.5 * p * q*q;
	factors[1] =  0.5 * q * ( q * (3*p + 1) + 1 );
	factors[2] =  0.5 * p * ( p * (3*q + 1) + 1 );
	factors[3] = -0.5 * q * p*p;
}

static void InterpolateBicubic(const ImageData *image, DBL xcoor, DBL  ycoor, Colour& colour, int *index, bool premul)
{
	int iycoor, ixcoor;
	int cornerIndex;
	Colour cornerColour;
	DBL factor;
	DBL factorsX[4];
	DBL factorsY[4];

	xcoor += 0.5;
	ycoor += 0.5;

	iycoor = (int)ycoor;
	ixcoor = (int)xcoor;

	cubic(factorsX, xcoor);
	cubic(factorsY, ycoor);

	// We're using double precision for the colors here to avoid higher-than-1.0 results due to rounding errors,
	// which would otherwise lead to stray dot artifacts when clamped to [0..1] range for a color_map or similar.
	// (Note that strictly speaking we don't avoid such rounding errors, but rather make them small enough that
	// subsequent rounding to single precision will take care of them.)
	// (Note that bicubic interpolation may still give values outside the range [0..1] at high-contrast edges;
	// this is an inherent property of this interpolation method, and is therefore accepted here.)
	DblColour tempColour;
	DBL tempIndex = 0;
	for (int i = 0; i < 4; i ++)
	{
		for (int j = 0; j < 4; j ++)
		{
			cornerColour.clear();
			no_interpolation(image, (DBL)ixcoor + i-2, (DBL)iycoor + j-2, cornerColour, &cornerIndex, premul);
			factor = factorsX[i] * factorsY[j];
			tempColour += DblColour(cornerColour) * factor;
			tempIndex  += cornerIndex             * factor;
		}
	}
	colour = Colour(tempColour);
	*index = (int)tempIndex;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

// These interpolation techniques are taken from an article by
// Girish T. Hagan in the C Programmer's Journal V 9 No. 8
// They were adapted for POV-Ray by CdW

static void bilinear(DBL *factors, DBL x, DBL y)
{
	DBL p, q;

	p = x - (int)x;
	q = y - (int)y;

	factors[0] =    p  *    q;
	factors[1] = (1-p) *    q;
	factors[2] =    p  * (1-q);
	factors[3] = (1-p) * (1-q);
}




/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

#define PYTHAGOREAN_SQ(a,b)  ( (a)*(a) + (b)*(b) )

static void norm_dist(DBL *factors, DBL x, DBL y)
{
	register int i;

	DBL p, q;
	DBL wts[4];
	DBL sum_inv_wts = 0.0;
	DBL sum_I = 0.0;

	p = x - (int)x;
	q = y - (int)y;

	wts[0] = 1.0 / PYTHAGOREAN_SQ(1-p, 1-q);
	wts[1] = 1.0 / PYTHAGOREAN_SQ(  p, 1-q);
	wts[2] = 1.0 / PYTHAGOREAN_SQ(1-p,   q);
	wts[3] = 1.0 / PYTHAGOREAN_SQ(  p,   q);

	for(i = 0; i < 4; i++)
		sum_inv_wts += wts[i];

	for(i = 0; i < 4; i++)
		factors[i] = (wts[i] / sum_inv_wts);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Image
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Scott Manley Added repeat vector initialisation
*
******************************************************************************/

ImageData *Create_Image()
{
	ImageData *image;

	image = (ImageData *) POV_CALLOC(1, sizeof(ImageData), "image file");

	image->References = 1;

	image->Map_Type = PLANAR_MAP;

	image->Interpolation_Type = NO_INTERPOLATION;

	image->iwidth = image->iheight = 0;
	image->width = image->height = 0.0;

	image->Once_Flag = false;

	Make_UV_Vector(image->Offset,0.0,0.0);

	image->Use = USE_NONE;

	Make_Vector(image->Gradient, 1.0, -1.0, 0.0);

	image->AllFilter = 0;
	image->AllTransmit = 0;

	image->Object = NULL;

	image->data = NULL;

#ifdef POV_VIDCAP_IMPL
	// beta-test feature
	image->VidCap = NULL;
#endif

	return (image);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Image
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

ImageData *Copy_Image(ImageData *Old)
{
	if(Old != NULL)
		Old->References++;

	return (Old);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Image
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Destroy_Image(ImageData *image)
{
	if((image == NULL) || (--(image->References) > 0))
		return;

#ifdef POV_VIDCAP_IMPL
	// beta-test feature
	if(image->VidCap != NULL)
		delete image->VidCap;
#endif

	delete image->data;

	POV_FREE(image);
}


Image *Read_Image(Parser *p, shared_ptr<SceneData>& sd, int filetype, const UCS2 *filename, const Image::ReadOptions& options)
{
	unsigned int stype;
	Image::ImageFileType type;
	UCS2String ign;

	switch(filetype)
	{
		case GIF_FILE:
			stype = POV_File_Image_GIF;
			type = Image::GIF;
			break;
		case POT_FILE:
			stype = POV_File_Image_GIF;
			type = Image::POT;
			break;
		case SYS_FILE:
			stype = POV_File_Image_System;
			type = Image::SYS;
			break;
		case IFF_FILE:
			stype = POV_File_Image_IFF;
			type = Image::IFF;
			break;
		case TGA_FILE:
			stype = POV_File_Image_Targa;
			type = Image::TGA;
			break;
		case PGM_FILE:
			stype = POV_File_Image_PGM;
			type = Image::PGM;
			break;
		case PPM_FILE:
			stype = POV_File_Image_PPM;
			type = Image::PPM;
			break;
		case PNG_FILE:
			stype = POV_File_Image_PNG;
			type = Image::PNG;
			break;
		case JPEG_FILE:
			stype = POV_File_Image_JPEG;
			type = Image::JPEG;
			break;
		case TIFF_FILE:
			stype = POV_File_Image_TIFF;
			type = Image::TIFF;
			break;
		case BMP_FILE:
			stype = POV_File_Image_BMP;
			type = Image::BMP;
			break;
		case EXR_FILE:
			stype = POV_File_Image_EXR;
			type = Image::EXR;
			break;
		case HDR_FILE:
			stype = POV_File_Image_HDR;
			type = Image::HDR;
			break;
		default:
			throw POV_EXCEPTION(kDataTypeErr, "Unknown file type.");
	}

	boost::scoped_ptr<IStream> file(Locate_File(p, sd, filename, stype, ign, true));

	if(file == NULL)
		throw POV_EXCEPTION(kCannotOpenFileErr, "Cannot find image file.");

	return Image::Read(type, file.get(), options);
}

}
