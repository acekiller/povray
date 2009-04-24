/****************************************************************************
*                pgm.cpp
*
*  This module contains the code to read the PGM file format.
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
* Original patch copyright 1994 Tim Rowley
* Updated for POV 3.0 by Chris Cason, Jan '95.
*
* $File: //depot/povray/3.5/source/pgm.cpp $
* $Revision: #13 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/****************************************************************************
*  The format is as follows:
*
*  (header:)
*    P2              - ASCII data in file OR
*    P5              - binary data in file
*    # hello         - comment (optional, may be multiple)
*    wwww hhhh       - Width, Height (ASCII text)
*    # world         - comment (optional, may be multiple)
*    nnn             - maximum color (nnn = white, 0 = black)
*
*  (each pixel: one of the following)
*    xx               - Intensity 0-nnn (binary byte)
*    AAA              - Intensity 0-nnn (ASCII number)
*
*****************************************************************************/

#include "frame.h"
#include "povproto.h"
#include "povray.h"
#include "pgm.h"
#include "ppm.h"
#include "file_pov.h"
#include "ctype.h"

/*****************************************************************************
*
* FUNCTION
*
*  Read_PGM_Image
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
*     Modified to support ASCII files, comments, and arbitrary bit depth [AED]
*
******************************************************************************/

void Read_PGM_Image(IMAGE *Image, char *name)
{
  char type;
  int width, height;
  unsigned depth;
  char junk[512];
  char *s ;
  int x, y;
  int data;
  int i ;
  POV_ISTREAM *infile;

  if((infile = Locate_File(name,POV_File_Image_PGM,NULL,true)) == NULL)
    Error ("Cannot open PGM image %s.", name);

  if(infile->Read_Byte () != 'P')
    Error ("File is not in PGM format.");

  type = infile->Read_Byte ();
  if((type != '2') && (type != '5'))
    Error ("File is not in PGM format.");

  /* Ignore any comments */
  while ((data = infile->Read_Byte ()) == '#')
    infile->getline (junk, 512) ;
  infile->UnRead_Byte (data) ;

  while (isspace(data = infile->Read_Byte ()));
  infile->UnRead_Byte (data) ;
 
  for (i = 0, s = junk ; (i < 8) && (!isspace(data = infile->Read_Byte ())) ; i++, s++)
    *s = (char) data ;
  *s = '\0' ;
  if (*infile)
    width = atoi (junk) ;

  while (isspace(data = infile->Read_Byte ()));
  infile->UnRead_Byte (data) ;
  
  for (i = 0, s = junk ; (i < 8) && (!isspace(data = infile->Read_Byte ())) ; i++, s++)
    *s = (char) data ;
  *s = '\0' ;
  if (*infile)
    height = atoi (junk) ;

  if (width == 0 || height == 0)
    Error ("Invalid width or height read from PPM image.");

  if (!infile)
    Error ("Cannot read width or height from PGM image.");

  /* Ignore any comments */
  while ((data = infile->Read_Byte ()) == '#')
    infile->getline(junk,512);
  infile->UnRead_Byte (data) ;

  while (isspace(data = infile->Read_Byte ()));
  infile->UnRead_Byte (data) ;

  if (!infile->getline (junk, 512))
    Error ("Premature EOF in PGM image") ;
  depth = atoi (junk) ;
  if (((depth > 255) && (type == '5')) || (depth > 65535) || (depth < 1))
    Error ("Unsupported number of colors (%d) in PGM image.", depth);

  Image->width  = (DBL)(Image->iwidth  = width);
  Image->height = (DBL)(Image->iheight = height);

  if (depth < 256)
  {
    Image->Colour_Map_Size = depth;

    Image->Colour_Map = (IMAGE_COLOUR *)POV_MALLOC(depth*sizeof(IMAGE_COLOUR), "PGM color map");

    for (x = 0; x < depth; x++)
    {
      Image->Colour_Map[x].Red =
      Image->Colour_Map[x].Green =
      Image->Colour_Map[x].Blue = x*255/depth;
      Image->Colour_Map[x].Filter =0;
      Image->Colour_Map[x].Transmit =0;
    }

    Image->data.map_lines = (unsigned char **)POV_MALLOC(height*sizeof(unsigned char *), "PGM image");

    for (y = 0; y < height; y++)
    {
      Image->data.map_lines[y] = (unsigned char *)POV_MALLOC(width,"PGM image line");

      if (type == '2') /* ASCII data to be input */
      {
        for (x = 0; x < width; x++)
        {
          for (i = 0, s = junk ; (i < 8) && (!isspace(data = infile->Read_Byte ())) ; i++, s++)
            *s = (char) data ;
          *s = '\0' ;
          if (*infile)
          {
            while (isspace(data = infile->Read_Byte()));
            infile->UnRead_Byte(data);
            data = atoi (junk) ;
          }
          else
            Error ("Cannot read data from PGM image.");
          Image->data.map_lines[y][x] = data;
        }
      }
      else /* (type == '5') Raw binary data to be input */
      {
        for (x = 0; x < width; x++)
        {
          if ((data = infile->Read_Byte ()) == EOF)
            Error ("Cannot read data from PGM image.");
          Image->data.map_lines[y][x] = data;
        }
      }
    }
  }
  else /* if(depth < 65536) the data MUST be in ASCII format */
  {
    IMAGE8_LINE *line_data;

    Image->Colour_Map_Size = 0;
    Image->Colour_Map = (IMAGE_COLOUR *)NULL;

    Image->data.rgb8_lines = (IMAGE8_LINE *)POV_MALLOC(height * sizeof(IMAGE8_LINE), "PGM image");
    for (y = 0; y < height; y++)
    {
      line_data = &Image->data.rgb8_lines[y];

      line_data->red   = (unsigned char *)POV_MALLOC(width, "PGM image line");
      line_data->green = (unsigned char *)POV_MALLOC(width, "PGM image line");
      line_data->blue  = (unsigned char *)POV_MALLOC(width, "PGM image line");
      line_data->transm = (unsigned char *)NULL;

      for (x = 0; x < width; x++)
      {
        for (i = 0, s = junk ; (i < 8) && (!isspace(data = infile->Read_Byte ())) ; i++, s++)
          *s = (char) data ;
        *s = '\0' ;
        if (*infile) {
          while (isspace(data = infile->Read_Byte()));
          infile->UnRead_Byte(data);
          data = atoi (junk) ;
        }
        else
          Error("Cannot read data from PGM image.");
        data = (int)((DBL)data*65535/depth);

        line_data->red[x] = (data >> 8) & 0xFF;
        line_data->green[x] = data & 0xFF;
        line_data->blue[x] = 0;
      }
    }
  }

  POV_DELETE(infile, POV_ISTREAM);
}
