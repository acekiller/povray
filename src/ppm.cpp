/****************************************************************************
*                ppm.cpp
*
*  This module contains the code to read and write the PPM file format.
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
* $File: //depot/povray/3.5/source/ppm.cpp $
* $Revision: #20 $
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
*    P3              - ASCII data OR
*    P6              - raw binary data
*    # hello         - optional comment(s)
*    wwww hhhh       - Width, Height (ASCII text)
*    # world         - optional comment(s)
*    nnn             - maximum color (nnn = bright, 0 = black)
*
*  (each pixel: one of the following)
*    rr gg bb        - Red, green, blue of intensity 0-nnn (binary byte)
*    RRR GGG BBB     - Red, green, blue of intensity 0-nnn (ASCII number)
*
*****************************************************************************/

#include "frame.h"
#include "povproto.h"
#include "povray.h"
#include "optout.h"
#include "pgm.h"
#include "ppm.h"
#include "file_pov.h"

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/*****************************************************************************
* Local typedefs
******************************************************************************/

/*****************************************************************************
* Local variables
******************************************************************************/

/*****************************************************************************
* Static functions
******************************************************************************/


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

PPM_Image::PPM_Image(char *name, int w, int h, int m, int l)
{
  int  data, c;
  char type;
  int  filetype;
  char junk[512];

  mode = m;
  in_file = NULL;
  out_file = NULL;
  buffer = NULL;
  filename = name;
  line_number = l;

  if (opts.Options & HF_GRAY_16)  /* 16 bit grayscale output */
  {
    filetype = POV_File_Image_PGM;
  } 
  else 
  {
    filetype = POV_File_Image_PPM;
  } 

  switch (mode)
  {
    case READ_MODE:

      /* We can't resume from stdout. */
      if (opts.Options & TO_STDOUT  || 
          (in_file = POV_NEW_ISTREAM(name, filetype)) == NULL)
      {
        Status_Info("\n");
        return;
      }

      if(in_file->Read_Byte () != 'P') 
        return;

      type = in_file->Read_Byte ();
      if(type != '6')
        return;

      /* Ignore any comments (if they are written) */
      while ((data = in_file->Read_Byte ()) == '#')
        in_file->getline (junk, 512) ;
      in_file->UnRead_Byte (data) ;

      *(in_file) >> w;
      if(!(*(in_file) >> h))
         return;
      if(!(*(in_file) >> c))
         return;
      if(c != 255)
         return;

      Status_Info("\nResuming interrupted trace from %s", filename);

      width = w;
      height = h;
      buffer_size = opts.File_Buffer_Size;

      break;

    case WRITE_MODE:


      if (opts.Options & TO_STDOUT)
      {
        buffer_size = 0;
        out_file = POV_NEW_OSTREAM("stdout", filetype, false);
      }
      else
      {
        if ((out_file = POV_NEW_OSTREAM(name, filetype, false)) == NULL)
        {
          return;
        }
      }

      if (opts.Options & HF_GRAY_16)  /* 16 bit grayscale output */
      {
        *(out_file) << "P5\n";

        out_file->printf("%d %d\n65535\n", w, h ); 
      }
      else 
      {
        *(out_file) << "P6\n";

        out_file->printf("%d %d\n%d\n", w, h, (1 << opts.OutputQuality) - 1) ;
      }

      width = w;
      height = h;

      buffer_size = opts.File_Buffer_Size;

      break;

    case APPEND_MODE:

      if (opts.Options & TO_STDOUT)
      {
        buffer_size = 0;
        out_file = POV_NEW_OSTREAM("stdout", filetype, false);
      }
      else
      {
        if ((out_file = POV_NEW_OSTREAM(name, filetype, true)) == NULL)
        {
          return;
        }
      }

      buffer_size = opts.File_Buffer_Size;

      break;
  }

  valid = true;
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

PPM_Image::~PPM_Image()
{
  /* Close the input file (if open) */
  if(in_file != NULL)
    POV_DELETE(in_file, POV_ISTREAM);

  /* Close the output file (if open) */
  if(out_file != NULL)
  {
    out_file->flush();
    POV_DELETE(out_file, POV_OSTREAM);
  }

  if (buffer != NULL)
    POV_FREE(buffer);

  in_file = NULL;
  out_file = NULL;
  buffer = NULL;
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

void PPM_Image::Write_Line(COLOUR *line_data)
{
	if(valid == false)
		Error("Cannot access output image file.");

  unsigned int gray;
  register int x;

  for (x = 0 ; x < width ; x++)
  {
    if (opts.Options & HF_GRAY_16)  /* 16 bit grayscale output */
    {
      gray = ((0.30 * line_data[x][pRED]) +
              (0.59 * line_data[x][pGREEN]) +
              (0.11 * line_data[x][pBLUE])) * 65535;

      out_file->Write_Byte((gray >> 8) & 0xFF);
      if (!out_file->Write_Byte(gray & 0xFF))
        Error("Error writing PPM output data to %s.",filename);
    }
    else                            /* Normal 3*OutputQuality bit pixel coloring */
    {
      // changes for > 8 bits per colour from Mark Weyer
      unsigned long mask = (1 << opts.OutputQuality) - 1 ;
      COLC fac = (COLC) (mask) ;

      unsigned long r=(unsigned long)floor(line_data[x][pRED] * fac) & mask;
      unsigned long g=(unsigned long)floor(line_data[x][pGREEN] * fac) & mask;
      unsigned long b=(unsigned long)floor(line_data[x][pBLUE] * fac) & mask;

      if (opts.OutputQuality>8)             /* two bytes per value */
      {
        out_file->Write_Byte((int)r>>8) ;
        out_file->Write_Byte((int)r&0xFF) ;
        out_file->Write_Byte((int)g>>8) ;
        out_file->Write_Byte((int)g&0xFF) ;
        out_file->Write_Byte((int)b>>8) ;
        if (!out_file->Write_Byte((int)b&0xFF))
        {
          Error("Error writing PPM output data to %s.",filename);
        }
      }
      else                                  /* one byte per value */
      {
        out_file->Write_Byte((int)r&0xFF) ;
        out_file->Write_Byte((int)g&0xFF) ;
        if (!out_file->Write_Byte((int)b&0xFF))
        {
          Error("Error writing PPM output data to %s.",filename);
        }
      }
    }
  }

  line_number++;

  if (buffer_size == 0)
  {
    /* close and reopen file for integrity in case we crash   */
    /* lets not do so,the business of the OS, not ours and it */
    /* creates problems with the new io implemenation - [trf] */

    out_file->flush();
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

int PPM_Image::Read_Line(COLOUR *line_data)
{
	if(valid == false)
		Error("Cannot access output image file.");

  int data, i;

  if (in_file->eof ())
    return 0;

  line_number++;

  for (i = 0 ; i < width ; i++)
  {
    if ((data = in_file->Read_Byte ()) == EOF)
    {
      return -1;
    }

    line_data[i][pRED] = (DBL) data / 255.0;

    if ((data = in_file->Read_Byte ()) == EOF)
    {
      return -1;
    }

    line_data[i][pGREEN] = (DBL) data / 255.0;

    if ((data = in_file->Read_Byte ()) == EOF)
    {
      return -1;
    }

    line_data[i][pBLUE] = (DBL) data / 255.0;
  }

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
* CHANGES
*
******************************************************************************/

void Read_PPM_Image(IMAGE *Image, char *name)
{
  char type;
  int width = 0, height = 0 ;
  int depth;
  char junk[512];
  char *s ;
  int x, y;
  int data;
  int i ;
  IMAGE8_LINE *line_data;
  POV_ISTREAM *infile;

  if ((infile = Locate_File(name,POV_File_Image_PPM,NULL,true)) == NULL)
    Error("Cannot open PPM image %s.", name);

  if(infile->Read_Byte () != 'P')
    Error ("File is not in PPM format.");

  type = infile->Read_Byte ();
  if((type != '3') && (type != '6'))
    Error ("File is not in PPM format.");

  /* Ignore any comments */
  while ((data = infile->Read_Byte ()) == '#')
    infile->getline(junk,512);
  infile->UnRead_Byte (data) ;

  if ((data = infile->Read_Byte ()) != ' ')
    infile->UnRead_Byte (data) ;
  for (i = 0, s = junk ; (i < 8) && ((data = infile->Read_Byte ()) != ' ') ; i++, s++)
    *s = (char) data ;
  *s = '\0' ;
  if (*infile)
    width = atoi (junk) ;

  for (i = 0, s = junk ; (i < 8) && ((data = infile->Read_Byte ()) != ' ') ; i++, s++)
    *s = (char) data ;
  *s = '\0' ;
  if (*infile)
    height = atoi (junk) ;

  if (width == 0 || height == 0)
    Error ("Invalid width or height read from PPM image.");

  /* Ignore any comments */
  while ((data = infile->Read_Byte ()) == '#')
    infile->getline(junk,512);
  infile->UnRead_Byte (data) ;

  infile->getline(junk,512);
  depth = atoi(junk);
  if((depth > 255) || (depth < 1))
    Error ("Unsupported number of colors (%d) in PPM image.", depth);

  Image->width  = (DBL)(Image->iwidth = width);
  Image->height = (DBL)(Image->iheight = height);

  Image->Colour_Map_Size = 0;

  Image->Colour_Map = NULL;

  Image->data.rgb8_lines = (IMAGE8_LINE *) POV_MALLOC(Image->iheight * sizeof (IMAGE8_LINE), "PPM image");

  for (y = 0; y < height; y++)
  {
    line_data = &Image->data.rgb8_lines[y];

    line_data->red   = (unsigned char *)POV_MALLOC(width,"PPM image line");
    line_data->green = (unsigned char *)POV_MALLOC(width,"PPM image line");
    line_data->blue  = (unsigned char *)POV_MALLOC(width,"PPM image line");
    line_data->transm = (unsigned char *)NULL;

    if (type == '3') /* ASCII data to be input */
    {
      for (x = 0; x < width; x++)
      {
        *infile >> data;
        line_data->red[x] = data*255/depth;

        *infile >> data;
        line_data->green[x] = data*255/depth;

        *infile >> data;
        line_data->blue[x] = data*255/depth;

        if (!*infile)
          Error("Cannot read data from PPM image.");
      }
    }
    else /* (type == '6') Raw binary data to be input */
    {
      for (x = 0; x < width; x++)
      {
        if ((data = infile->Read_Byte ()) == EOF)
          Error("Cannot read data from PPM image.");

        line_data->red[x] = data*255/depth;

        if ((data = infile->Read_Byte ()) == EOF)
          Error("Cannot reading data from PPM image.");

        line_data->green[x] = data*255/depth;

        if ((data = infile->Read_Byte ()) == EOF)
          Error("Cannot reading data from PPM image.");

        line_data->blue[x] = data*255/depth;
      }
    }
  }

  POV_DELETE(infile, POV_ISTREAM);
}
