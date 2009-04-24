/****************************************************************************
*                   ppm.h
*
*  This module contains all defines, typedefs, and prototypes for PPM.CPP.
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
* $File: //depot/povray/3.5/source/ppm.h $
* $Revision: #10 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#ifndef PPM_H
#define PPM_H

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/



/*****************************************************************************
* Global typedefs
******************************************************************************/

class PPM_Image : public Image_File_Class
{
	public:
		PPM_Image(char *filename, int width, int height, int mode, int line = 0);
		~PPM_Image();

		void Write_Line(COLOUR *line_data);
		int Read_Line(COLOUR *line_data);

		int Line() { return line_number; };
		int Width() { return width; };
		int Height() { return height; };
	private:
		char *filename;
		int mode;
		int width, height;
		int line_number;
		int buffer_size;
		char *buffer;
		POV_ISTREAM *in_file;
		POV_OSTREAM *out_file;
};


/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void Read_PPM_Image(IMAGE *Image, char *filename);

#endif

