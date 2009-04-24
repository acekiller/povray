/****************************************************************************
*                   renderio.h
*
*  This module contains all defines, typedefs, and prototypes for renderio.cpp.
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
* $File: //depot/povray/3.5/source/renderio.h $
* $Revision: #11 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef RENDERIO_H
#define RENDERIO_H

void Read_Rendered_Part(char *New_Fname);
void init_output_file_handle();
void destroy_output_file_handle();
void setup_output_file_name();
void open_output_file();
void output_prev_image_line_and_advance(int y);
void output_single_image_line_with_alpha_correction(COLOUR *Line, int y);
void plot_pixel(int x, int  y, COLOUR Colour);
Image_File_Class *Open_Image(int file_type, char *filename, int w, int h, int m, int l = 0);

#endif
