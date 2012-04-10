/*******************************************************************************
 * imagemessagehandler.cpp
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
 * $File: //depot/povray/smp/source/frontend/imagemessagehandler.cpp $
 * $Revision: #38 $
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
#include "base/types.h"

#include "frontend/configfrontend.h"
#include "frontend/renderfrontend.h"
#include "frontend/imagemessagehandler.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_frontend
{

ImageMessageHandler::ImageMessageHandler()
{
}

ImageMessageHandler::~ImageMessageHandler()
{
}

void ImageMessageHandler::HandleMessage(const SceneData& sd, const ViewData& vd, POVMSType ident, POVMS_Object& msg)
{
	switch(ident)
	{
		case kPOVMsgIdent_PixelSet:
			DrawPixelSet(sd, vd, msg);
			break;
		case kPOVMsgIdent_PixelBlockSet:
			DrawPixelBlockSet(sd, vd, msg);
			break;
		case kPOVMsgIdent_PixelRowSet:
			DrawPixelRowSet(sd, vd, msg);
			break;
		case kPOVMsgIdent_RectangleFrameSet:
			DrawRectangleFrameSet(sd, vd, msg);
			break;
		case kPOVMsgIdent_FilledRectangleSet:
			DrawFilledRectangleSet(sd, vd, msg);
			break;
	}
}

void ImageMessageHandler::DrawPixelSet(const SceneData& sd, const ViewData& vd, POVMS_Object& msg)
{
	POVMS_Attribute pixelposattr;
	POVMS_Attribute pixelcolattr;
	unsigned int psize(msg.GetInt(kPOVAttrib_PixelSize));

	msg.Get(kPOVAttrib_PixelPositions, pixelposattr);
	msg.Get(kPOVAttrib_PixelColors, pixelcolattr);

	vector<POVMSInt> pixelpositions(pixelposattr.GetIntVector());
	vector<POVMSFloat> pixelcolors(pixelcolattr.GetFloatVector());

	if((pixelpositions.size() / 2) != (pixelcolors.size() / 5))
		throw POV_EXCEPTION(kInvalidDataSizeErr, "Number of pixel colors and pixel positions does not match!");

	GammaCurvePtr gamma;
	if (vd.display != NULL)
		gamma = vd.display->GetGamma();

	for(int i = 0, ii = 0; (i < pixelcolors.size()) && (ii < pixelpositions.size()); i += 5, ii += 2)
	{
		Colour col(pixelcolors[i], pixelcolors[i + 1], pixelcolors[i + 2], pixelcolors[i + 3], pixelcolors[i + 4]);
		Colour gcol(col);
		unsigned int x(pixelpositions[ii]);
		unsigned int y(pixelpositions[ii + 1]);
		Display::RGBA8 rgba;
		float dither = GetDitherOffset(x, y);

		if(vd.display != NULL)
		{
			// TODO ALPHA - display may profit from receiving the data in its original, premultiplied form
			// Premultiplied alpha was good for the math, but the display expects non-premultiplied alpha, so fix this if possible.
			float alpha = gcol.FTtoA();
			if (alpha != 1.0 && fabs(alpha) > 1e-6) // TODO FIXME - magic value
			{
				gcol.red()   /= alpha;
				gcol.green() /= alpha;
				gcol.blue()  /= alpha;
			}
		}

		rgba.red   = IntEncode(gamma, gcol.red(),   255, dither);
		rgba.green = IntEncode(gamma, gcol.green(), 255, dither);
		rgba.blue  = IntEncode(gamma, gcol.blue(),  255, dither);
		rgba.alpha = IntEncode(       gcol.FTtoA(), 255, dither);

		if(psize == 1)
		{
			if(vd.display != NULL)
				vd.display->DrawPixel(x, y, rgba);

			if((vd.image != NULL) && (x < vd.image->GetWidth()) && (y < vd.image->GetHeight()))
				vd.image->SetRGBAValue(x, y, col.red(), col.green(), col.blue(), col.FTtoA());
		}
		else
		{
			if(vd.display != NULL)
				vd.display->DrawFilledRectangle(x, y, x + psize - 1, y + psize - 1, rgba);

			if(vd.image != NULL)
			{
				for(unsigned int py = 0; (py < psize) && (y + py < vd.image->GetHeight()); py++)
				{
					for(unsigned int px = 0; (px < psize) && (x + px < vd.image->GetWidth()); px++)
						vd.image->SetRGBAValue(x + px, y + py, col.red(), col.green(), col.blue(), col.FTtoA());
				}
			}
		}
	}

	if(vd.imageBackup != NULL)
	{
		msg.Write(*vd.imageBackup);
		vd.imageBackup->flush();
	}
}

void ImageMessageHandler::DrawPixelBlockSet(const SceneData& sd, const ViewData& vd, POVMS_Object& msg)
{
	POVRect rect(msg.GetInt(kPOVAttrib_Left), msg.GetInt(kPOVAttrib_Top), msg.GetInt(kPOVAttrib_Right), msg.GetInt(kPOVAttrib_Bottom));
	POVMS_Attribute pixelattr;
	vector<Colour> cols;
	vector<Display::RGBA8> rgbas;
	unsigned int psize(msg.GetInt(kPOVAttrib_PixelSize));
	int i = 0;

	msg.Get(kPOVAttrib_PixelBlock, pixelattr);

	vector<POVMSFloat> pixelvector(pixelattr.GetFloatVector());

	cols.reserve(rect.GetArea());
	rgbas.reserve(rect.GetArea());

	GammaCurvePtr gamma;

	if (vd.display != NULL)
		gamma = vd.display->GetGamma();

	for(i = 0; i < rect.GetArea() *  5; i += 5)
	{
		Colour col(pixelvector[i], pixelvector[i + 1], pixelvector[i + 2], pixelvector[i + 3], pixelvector[i + 4]);
		Colour gcol(col);
		Display::RGBA8 rgba;
		unsigned int x(rect.left + (i/5) % rect.GetWidth());
		unsigned int y(rect.top  + (i/5) / rect.GetWidth());
		float dither = GetDitherOffset(x, y);

		if(vd.display != NULL)
		{
			// TODO ALPHA - display may profit from receiving the data in its original, premultiplied form
			// Premultiplied alpha was good for the math, but the display expects non-premultiplied alpha, so fix this if possible.
			float alpha = gcol.FTtoA();
			if (alpha != 1.0 && fabs(alpha) > 1e-6) // TODO FIXME - magic value
			{
				gcol.red()   /= alpha;
				gcol.green() /= alpha;
				gcol.blue()  /= alpha;
			}
		}

		rgba.red   = IntEncode(gamma, gcol.red(),   255, dither);
		rgba.green = IntEncode(gamma, gcol.green(), 255, dither);
		rgba.blue  = IntEncode(gamma, gcol.blue(),  255, dither);
		rgba.alpha = IntEncode(       gcol.FTtoA(), 255, dither);

		cols.push_back(col);
		rgbas.push_back(rgba);
	}

	if(vd.display != NULL)
	{
		if(psize == 1)
			vd.display->DrawPixelBlock(rect.left, rect.top, rect.right, rect.bottom, &rgbas[0]);
		else
		{
			for(unsigned int y = rect.top, i = 0; y <= rect.bottom; y += psize)
			{
				for(unsigned int x = rect.left; x <= rect.right; x += psize, i++)
					vd.display->DrawFilledRectangle(x, y, x + psize - 1, y + psize - 1, rgbas[0]);
			}
		}
	}

	if(vd.image != NULL)
	{
		for(unsigned int y = rect.top, i = 0; y <= rect.bottom; y += psize)
		{
			for(unsigned int x = rect.left; x <= rect.right; x += psize, i++)
			{
				for(unsigned int py = 0; py < psize; py++)
				{
					for(unsigned int px = 0; px < psize; px++)
						vd.image->SetRGBAValue(x + px, y + py, cols[i].red(), cols[i].green(), cols[i].blue(), cols[i].FTtoA());
				}
			}
		}
	}

	if(vd.imageBackup != NULL)
	{
		msg.Write(*vd.imageBackup);
		vd.imageBackup->flush();
	}
}

void ImageMessageHandler::DrawPixelRowSet(const SceneData& sd, const ViewData& vd, POVMS_Object& msg)
{
}

void ImageMessageHandler::DrawRectangleFrameSet(const SceneData& sd, const ViewData& vd, POVMS_Object& msg)
{
}

void ImageMessageHandler::DrawFilledRectangleSet(const SceneData& sd, const ViewData& vd, POVMS_Object& msg)
{
}

}
