/*****************************************************************************
*        tiff_pov.cpp
*
*  This module contains the code to read and write the TIFF output file
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996-2002 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file.
*  If POVLEGAL.DOC is not available it may be found online at -
*
*    http://www.povray.org/povlegal.html.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
* Original patch copyright 2000 Alexander R. Enzmann 
*
* $File: //depot/povray/3.5/source/tiff_pov.cpp $
* $Revision: #12 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include "frame.h"
#include <assert.h>

#include "povproto.h"
#include "povray.h"
#include "tiff_pov.h"
#include "file_pov.h"

// TIFF Image loader
extern "C" {
#ifndef __STDC__
#define __STDC__        (1)
#endif
#include "tiffio.h"
}

/* 16 to 8 bit conversion for those things that POV-Ray can't handle (e.g., color map
   entries are limited to 8 bits per pixel). */
#define CVT(x)      (((x) * 255L) / ((1L<<16)-1))

/* Do any of the entries in the color map contain values larger than 255? */
static int
checkcmap(int n, uint16* r, uint16* g, uint16* b)
{
    while (n-- > 0)
        if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
			return (16);
		
		return (8);
}

static void
SuppressTIFFWarnings(const char*, const char*, va_list)
{
}


/*****************************************************************************
*
* FUNCTION      : Read_Tiff_Image
*
* ARGUMENTS     : IMAGE *Image; char *name;
*
* MODIFIED ARGS : Image
*
* RETURN VALUE  : none
*
* AUTHOR        : Alexander Enzmann
*
* DESCRIPTION
*
*   Reads a TIFF image into an RGB image buffer
*
* CHANGES
*
* New - 6/2000
*
******************************************************************************/

void Read_Tiff_Image(IMAGE *Image, char *name)
{
	unsigned int width, height;
	char *filename = Locate_Filename(name, POV_File_Image_TIFF, 1);
	
    long LineSize;
    uint16 BitsPerSample, PhotometricInterpretation;
	uint16 SamplePerPixel, Orientation;
	uint32 RowsPerStrip;
    int row, nrow, i, j, l;
    TIFF* tif;
	int result = 0;
	
	if (filename == NULL) {
		Error("Cannot read TIFF image.");
	}
	
	// Rather than have libTIFF complain about tags it doesn't understand,
	// we just suppress all the warnings.
	TIFFSetWarningHandler(SuppressTIFFWarnings);
	TIFFSetErrorHandler(SuppressTIFFWarnings);
	
	// Open and do initial processing
	tif = TIFFOpen(filename, "r");
	if (!tif)
		return;
	
	// Get basic information about the image
	int ExtraSamples, ExtraSampleInfo;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);  
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
	TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &RowsPerStrip);   
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &PhotometricInterpretation);
	TIFFGetField(tif, TIFFTAG_ORIENTATION, &Orientation);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplePerPixel);
    TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES, &ExtraSamples, &ExtraSampleInfo);
	
	Image->iwidth = width;
	Image->iheight = height;
	Image->width = (DBL)width;
	Image->height = (DBL)height;
	
    LineSize = TIFFScanlineSize(tif);
	assert(SamplePerPixel == (int)(LineSize / width));
    // SamplePerPixel = (int)(LineSize / width);
	
#if 0
	/* For now we are ignoring the orientation of the image... */
	switch (Orientation) {
	case ORIENTATION_TOPLEFT:
		break;
	case ORIENTATION_TOPRIGHT:
		break;
	case ORIENTATION_BOTRIGHT:
		break;
	case ORIENTATION_BOTLEFT:
		break;
	case ORIENTATION_LEFTTOP:
		break;
	case ORIENTATION_RIGHTTOP:
		break;
	case ORIENTATION_RIGHTBOT:
		break;
	case ORIENTATION_LEFTBOT:
		break;
	default:
		break;
	}
#endif
	
	//PhotometricInterpretation = 2 image is RGB
	//PhotometricInterpretation = 3 image have a color palette              
	if (PhotometricInterpretation == PHOTOMETRIC_PALETTE) {
		uint16 *red, *green, *blue;
		int16 i;
		int Palette16Bits;
		IMAGE_COLOUR *cmap;
		
		//load the palette
		Image->data.rgb8_lines = NULL;
		int cmap_len = (1 << BitsPerSample);
		Image->Colour_Map_Size = cmap_len;
		
		cmap = (IMAGE_COLOUR *)POV_MALLOC(cmap_len*sizeof(IMAGE_COLOUR), "TIFF image color map");
		Image->Colour_Map = cmap;
		
		TIFFGetField(tif, TIFFTAG_COLORMAP, &red, &green, &blue); 
		
		/* Is the palette 16 or 8 bits ? */
		if (checkcmap(cmap_len, red, green, blue) == 16) 
			Palette16Bits = true;
		else
			Palette16Bits = false;
		
		/* Read the palette */
		for (i=0,j=0;i<cmap_len;i++)  {
			if (Palette16Bits) {
				cmap[i].Red   = CVT(red[i]);
				cmap[i].Green = CVT(green[i]);
				cmap[i].Blue  = CVT(blue[i]);
			} else {
				cmap[i].Red   = red[i];
				cmap[i].Green = green[i];
				cmap[i].Blue  = blue[i];
			}
			/* I may be mistaken, but it appears that alpha/opacity information doesn't
			appear in a Paletted Tiff image.  Well - if it does, it's not as easy to
			get at as RGB.  */
			cmap[i].Filter   = 0;
			cmap[i].Transmit = 0;
		}
		
		Image->data.map_lines = (unsigned char **)
			POV_MALLOC(height * sizeof(unsigned char *), "TIFF image");
		unsigned char *buf = (unsigned char *)POV_MALLOC(sizeof(unsigned char) * TIFFStripSize(tif), "TIFF row");
		
		//read the tiff lines and save them in the image
		//with RGB mode, we have to change the order of the 3 samples RGB <=> BGR
		for (row=0;row<height;row+=RowsPerStrip) {     
			nrow = (row + (int)RowsPerStrip > height ? height - row : RowsPerStrip);
			TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 0), buf, nrow * LineSize);
			for (l=0;l<nrow;l++) {
				Image->data.map_lines[row+l] = (unsigned char *)POV_MALLOC(width, "TIFF Image line");
				memcpy(Image->data.map_lines[row+l], &buf[l*LineSize], (int)width); 
			}
		}
		
		POV_FREE(buf);
	}
	else
	{
		// Allocate the row buffers for the image
		Image->Colour_Map_Size = 0;
		Image->Colour_Map = NULL;
		Image->data.rgb8_lines = (IMAGE8_LINE *)
			POV_MALLOC(height * sizeof(IMAGE8_LINE), "TIFF image");
		uint32 *buf = (uint32 *)POV_MALLOC(sizeof(uint32) * width * height, "TIIF image data");
		
		TIFFReadRGBAImage(tif, width, height, buf, 0);
		uint32 abgr, *tbuf = buf;
		for (i=0;i<height;i++)
		{
			Image->data.rgb8_lines[i].blue   = (unsigned char *)POV_MALLOC(width, "TIFF Image line");
			Image->data.rgb8_lines[i].green  = (unsigned char *)POV_MALLOC(width, "TIFF Image line");
			Image->data.rgb8_lines[i].red    = (unsigned char *)POV_MALLOC(width, "TIFF Image line");
			Image->data.rgb8_lines[i].transm = (unsigned char *)POV_MALLOC(width, "TIFF Image line");
			for (j=0,l=0;j<width;j++)
			{
				abgr = *tbuf++;
				Image->data.rgb8_lines[i].blue[j]   = (unsigned char)TIFFGetB(abgr);
				Image->data.rgb8_lines[i].green[j]  = (unsigned char)TIFFGetG(abgr);
				Image->data.rgb8_lines[i].red[j]    = (unsigned char)TIFFGetR(abgr);
				Image->data.rgb8_lines[i].transm[j] = 255 - (unsigned char)TIFFGetA(abgr);
			}
		}
        POV_FREE(buf);
	}
	
	TIFFClose(tif);
}

