/*****************************************************************************
*        png_pov.cpp
*
*  This module contains the code to read and write the PNG output file
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
* Original patch copyright 1995 Andreas Dilger 
* Updated for POV 3.0 by Tim Wegner, August 1995.
* Updated to allow resuming by Andreas Dilger, Sept 1995.
* Updated to support Alpha channel input/output by Andreas Dilger, Sept 1995
* Updated to set the flush distance based on the file buffer size, Dec 1995
* Updated to use the libpng 0.87 messaging functions, Dec 1995
* Updated to use the libpng 0.89 structure interface, Jun 1996
*
* $File: //depot/povray/3.5/source/png_pov.cpp $
* $Revision: #30 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/*****************************************************************************
*  This code requires the use of libpng, Group 42's PNG reference library.
*  libpng is  Copyright (c) 1995 Guy Eric Schalnat, Group 42, Inc.
*
*  This code also requires the use of Zlib,
*  Zlib is Copyright (C) 1995 Jean-loup Gailly and Mark Adler
*
*  The latest version of these libraries are available at ftp.uu.net as
*
*  /graphics/png/libpngXX.tar.gz.
*  /archiver/zlib/zlib-XX.tar.gz.
*
*  where XX is the latest version of the library.
*
*****************************************************************************/

#include "frame.h"
#include "povproto.h"
#include "povray.h"
#include "optout.h"
#include "png.h"
#include "png_pov.h"
#include "file_pov.h"


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* Number of scanlines between output flushes, and hence the maximum number of
 * lines lost for an interrupted render.  Note that making it much smaller
 * than about 10 for a 640x480 image will noticably degrade compression.
 * If a very small buffer is specified, we don't want to flush more than once
 * every 10 lines or so (assuming a 2:1 compression ratio).
 */
/*
#define FLUSH_DIST 10
#define FLUSH_DIST (*width >= 640 ? 10 : 6400 / *width)
*/
#define FLUSH_DIST ((opts.Options & BUFFERED_OUTPUT && \
                     buffer_size > (w * png_stride * 5)) ? \
                    (buffer_size / (w * png_stride)): \
                    (w >= 640 ? 10 : 6400 / w))

#define NTEXT 15     /* Maximum number of tEXt comment blocks */
#define MAXTEXT 1024 /* Maximum length of a tEXt message */


/*****************************************************************************
* Local typedefs
******************************************************************************/


/*****************************************************************************
* Local variables
******************************************************************************/

static png_struct  *png_ptr  = NULL;
static png_info    *info_ptr = NULL;
static png_struct  *o_png_ptr  = NULL;
static png_byte    *row_ptr  = NULL;
static int         png_stride;
static char        tmp_fname[FILE_NAME_LENGTH];
static POV_ISTREAM *tmp_fp = NULL;

int PNG_Image::line_number = 0;
int PNG_Image::buffer_size = 0;
char *PNG_Image::buffer = NULL;
POV_ISTREAM *PNG_Image::in_file = NULL;
POV_OSTREAM *PNG_Image::out_file = NULL;

/*****************************************************************************
* Static functions
******************************************************************************/


extern "C"
{
	/* These are replacement error and warning functions for the libpng code */
	void png_pov_err(png_structp, png_const_charp);
	void png_pov_warn(png_structp, png_const_charp);
	void png_pov_read_data(png_structp, png_bytep, png_size_t);
	void png_pov_write_data(png_structp, png_bytep, png_size_t);
	void png_pov_flush_data(png_structp);

	/* This is an internal function for libpng */
	void png_write_finish_row(png_structp);


	/*****************************************************************************
	*
	* FUNCTION      : png_pov_warn
	*
	* ARGUMENTS     : png_struct *png_ptr; char *msg;
	*
	* MODIFIED ARGS :
	*
	* RETURN VALUE  :
	*
	* AUTHOR        : Andreas Dilger
	*
	* DESCRIPTION
	*
	*   Prints an warning message using the POV I/O functions.  This uses the
	*   png io_ptr to determine whether error messages should be printed or
	*   not.
	*
	* CHANGES
	*
	******************************************************************************/

	void png_pov_warn(png_structp png_ptr, png_const_charp msg)
	{
	  if (png_get_error_ptr(png_ptr))
	    Warning(0,"libpng: %s",msg);
	}


	/*****************************************************************************
	*
	* FUNCTION      : png_pov_err
	*
	* ARGUMENTS     : png_struct *png_ptr; char *msg;
	*
	* MODIFIED ARGS :
	*
	* RETURN VALUE  :
	*
	* AUTHOR        : Andreas Dilger
	*
	* DESCRIPTION
	*
	*   If the png io_ptr is true, this prints an error message using the POV
	*   I/O function.  It will return to the location of the last setjmp call
	*   for this stream in any case.
	*
	* CHANGES
	*
	******************************************************************************/

	void png_pov_err(png_structp png_ptr, png_const_charp msg)
	{
	  if (png_get_error_ptr(png_ptr))
	   PossibleError("libpng: %s",msg);

	  longjmp(png_ptr->jmpbuf,1);
	}


	/*****************************************************************************
	*
	* FUNCTION      : png_pov_read_data
	*
	* ARGUMENTS     : png_structp png_ptr; png_bytep data; png_uint_32 length;
	*
	* MODIFIED ARGS :
	*
	* RETURN VALUE  :
	*
	* AUTHOR        : Thorsten Froehlich
	*
	* DESCRIPTION
	*
	*   Replacement read function.
	*
	* CHANGES
	*
	******************************************************************************/

	void png_pov_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		POV_ISTREAM *file = (POV_ISTREAM *)png_get_io_ptr(png_ptr);

		if (!file->read ((char *)data, length))
			PossibleError("Cannot read PNG data.");
	}


	/*****************************************************************************
	*
	* FUNCTION      : png_pov_write_data
	*
	* ARGUMENTS     : png_structp png_ptr; png_bytep data; png_uint_32 length;
	*
	* MODIFIED ARGS :
	*
	* RETURN VALUE  :
	*
	* AUTHOR        : Thorsten Froehlich
	*
	* DESCRIPTION
	*
	*   Replacement write function.
	*
	* CHANGES
	*
	******************************************************************************/

	void png_pov_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		POV_OSTREAM *file = (POV_OSTREAM *)png_get_io_ptr(png_ptr);

		if (!file->write ((char *)data, length))
			PossibleError("Cannot write PNG data.");
	}


	/*****************************************************************************
	*
	* FUNCTION      : png_pov_flush_data
	*
	* ARGUMENTS     : png_structp png_ptr;
	*
	* MODIFIED ARGS :
	*
	* RETURN VALUE  :
	*
	* AUTHOR        : Thorsten Froehlich
	*
	* DESCRIPTION
	*
	*   Replacement flush function.
	*
	* CHANGES
	*
	******************************************************************************/

	void png_pov_flush_data(png_structp png_ptr)
	{
		POV_OSTREAM *file = (POV_OSTREAM *)png_get_io_ptr(png_ptr);

		file->flush();
	}

}


/*****************************************************************************
*
* FUNCTION      : Open_Png_File
*
* ARGUMENTS     : FILE_HANDLE *handle; char *name; int *width; int *height;
*                 int buffer_size; int mode;
*
* MODIFIED ARGS : handle, width, height
*
* RETURN VALUE  : 1 or 0 for success or failure
*
* AUTHOR        : Andreas Dilger
*
* DESCRIPTION
*
*   Open a PNG file and allocate the needed PNG structure buffers
*
* CHANGES
*
*   Updated for POV-Ray 3.X - [TIW]
*   Updated to handle resuming interrupted traces, Sept 1995 - [AED]
*   Updated to output grayscale heightfield if requested - [AED]
*   Updated to allow grayscale and alpha together, Oct 1995 - [AED]
*   Updated to write gamma differently based on file type, Nov 1995 - [AED]
*   Changed temp file name from TEMP_FILE_BASE to scene name, Feb 1996 - [AED]
*   Changed temp file from scene name to path + scene name, Jun 1996 - [AED]
*
******************************************************************************/

PNG_Image::PNG_Image(char *name, int w, int h, int m, int l)
{
  mode = m;
  filename = name;
  initial_line_number = line_number = l;
  width = w ;
  height = h ;

  switch (mode)
  {
    case READ_MODE:
    
      in_file = NULL;
      out_file = NULL;
      buffer = NULL;

      /* We can't resume from stdout. */
      if (opts.Options & TO_STDOUT)
      {
        Status_Info("\n");
        return;
      }

      /* Initialize PNG output routines using temporary file name.  We
       * need to use the path, or the rename will fail if the temp file
       * is not on the same drive as the output file.
       */
      sprintf(tmp_fname, "%s%s.tpn", opts.Output_Path, opts.Scene_Name);

      /* Move the old output file to a temporary file, so it can be
       * read in and simultaneously written out to the new output file.
       * Note that this can potentially be destructive, but it is
       * impossible to change the output stream in mid-write.  We have
       * to check if a temp file already exists, in case the transfer
       * has been previously aborted.
       */
      if ((tmp_fp = POV_NEW_ISTREAM(tmp_fname, POV_File_Image_PNG)) == NULL)
      {
        /* The temp file doesn't exist.  Try the original file. */
        if ((tmp_fp = POV_NEW_ISTREAM(name, POV_File_Image_PNG)) == NULL)
        {
          Status_Info("\n");
          return;  /* Neither file exists - start from scratch. */
        }
        else /* The original file exists, but the temp file doesn't. */
        {
          POV_DELETE(tmp_fp, POV_ISTREAM);

          if (RENAME_FILE(name,tmp_fname) == RENAME_FILE_ERR)
          {
            Error("Cannot make temporary PNG file for continuing trace.");
          }

          /* Open the original file (now with a new name) for reading */
          if ((tmp_fp = POV_NEW_ISTREAM(tmp_fname, POV_File_Image_PNG)) == NULL)
          {
            RENAME_FILE(tmp_fname,name); /* Try to rename back - not crucial */
            Error("Cannot open temporary PNG file for continuing trace.");
          }
        }
      }
      else if ((in_file = POV_NEW_ISTREAM(name, POV_File_Image_PNG)) != NULL)
      {
        /* The temp file already exists.  If we can open the original file
         * as well, then there is something wrong, and we can't automatically
         * decide which file to delete.
         */
        POV_DELETE(tmp_fp, POV_ISTREAM);
        POV_DELETE(in_file, POV_ISTREAM);

        Error ("Both original and temporary PNG files exist after an interrupted trace.\n"
               "Please delete either %s or %s (preferrably the smaller).",name,tmp_fname);
      }

      /* Try to open the new output file for writing.  If we can't, try
       * to move the old one back so that users don't fret if it's missing.
       * PNG will be able to continue without loss of data either way.
       */
      if ((out_file = POV_NEW_OSTREAM(name, POV_File_Image_PNG, false)) == NULL)
      {
        Status_Info("\n");

        POV_DELETE(tmp_fp, POV_ISTREAM);
        RENAME_FILE(tmp_fname,name);
        return;
      }

      buffer_size = opts.File_Buffer_Size;

      /* The original input file */
      if ((o_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                       (png_voidp)false, png_pov_err, png_pov_warn)) == NULL ||
          (info_ptr = png_create_info_struct(o_png_ptr)) == NULL)
      {
        Error("Cannot allocate PNG data structures");
      }

      if (setjmp(o_png_ptr->jmpbuf))
      {
        /* If we get here, we had a problem reading the file */
        Status_Info("\n");

        if (buffer != NULL)
        {
          POV_FREE(buffer);
          buffer = NULL;
        }

        png_destroy_read_struct(&o_png_ptr, &info_ptr, (png_infopp)NULL);

        POV_DELETE(out_file, POV_OSTREAM);
        out_file = NULL;
        POV_DELETE(tmp_fp, POV_ISTREAM);
        tmp_fp = NULL;

        return;
      }

      /* Set up the compression structure */
      png_set_read_fn(o_png_ptr, tmp_fp, png_pov_read_data);

      /* Read in header info from the file */
      png_read_info(o_png_ptr, info_ptr);

      if (info_ptr->width != w || info_ptr->height != h)
      {
        png_destroy_read_struct(&o_png_ptr, &info_ptr, (png_infopp)NULL);
        POV_DELETE(tmp_fp, POV_ISTREAM);
        tmp_fp = NULL;
        POV_DELETE(out_file, POV_OSTREAM);
        out_file = NULL;
        /* Try to get the original file back */
        if (DELETE_FILE(name) != DELETE_FILE_ERR)
          RENAME_FILE(tmp_fname,name);
        Error("PNG file dimensions do not match render resolution.");
      }

      if (info_ptr->color_type & ~(PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA))
      {
        return;
      }

      Status_Info("\nResuming interrupted trace from %s",filename);

      /* The new output file.  Thank god for re-entrant libpng/libz code! */
      if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                     (png_voidp)true, png_pov_err, png_pov_warn)) == NULL)
      {
        Error("Cannot allocate PNG data structures");
      }

      if (setjmp(png_ptr->jmpbuf))
      {
        /* If we get here, we had a problem writing the file */
        Status_Info("\n");

        if (buffer != NULL)
        {
          POV_FREE(buffer);
          buffer = NULL;
        }

        png_destroy_read_struct(&o_png_ptr, &info_ptr, (png_infopp)NULL);
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

        POV_DELETE(tmp_fp, POV_ISTREAM);
        tmp_fp = NULL;

        if (DELETE_FILE(name) != DELETE_FILE_ERR)
        {
          RENAME_FILE(tmp_fname,name);  /* Try to get the original file back */
        }

        return;
      }

      /* Set up the compression structure */
      png_set_write_fn(png_ptr, out_file, png_pov_write_data, png_pov_flush_data);

      /* Fill in the relevant image information from the resumed file */
      width = info_ptr->width;
      height = info_ptr->height;

      /* Find out if file is a valid format, and if it had Alpha in it */
      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      {
        opts.Options |= OUTPUT_ALPHA;
      }

      if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
      {
        opts.Options |= HF_GRAY_16;
        opts.PaletteOption = GREY;       /* Force grayscale preview */
      }

#if defined(PNG_READ_sBIT_SUPPORTED)
      if (info_ptr->valid & PNG_INFO_sBIT)
      {
        if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
        {
          opts.OutputQuality = info_ptr->sig_bit.red;
        }
        else
        {
          opts.OutputQuality = info_ptr->sig_bit.gray;
        }
      }

#else /* !PNG_READ_sBIT_SUPPORTED */
      if (info_ptr->bit_depth == 8 && opts.OutputQuality > 8 ||
          info_ptr->bit_depth == 16 && opts.OutputQuality <= 8)
      {
        Error("Specified color depth +fn%d not the same as depth %d in %s",
              opts.OutputQuality, info_ptr->bit_depth, name);
      }
#endif /* !PNG_READ_sBIT_SUPPORTED */

#if defined(PNG_READ_oFFs_SUPPORTED)
      opts.First_Column = info_ptr->x_offset;
      opts.First_Line   = info_ptr->y_offset;
#endif /* PNG_READ_oFFs_SUPPORTED */

      png_write_info(png_ptr, info_ptr);

      png_stride = info_ptr->color_type & PNG_COLOR_MASK_COLOR ? 3 : 1;

      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
        png_stride++;

      png_stride *= (opts.OutputQuality + 7) / 8;

      row_ptr = (png_byte *)POV_MALLOC(w*png_stride,"PNG read row buffer");
      break;

    case WRITE_MODE:

      in_file = NULL;
      out_file = NULL;
      buffer = NULL;

      if (opts.Options & TO_STDOUT)
      {
        buffer_size = 0;
        out_file = POV_NEW_OSTREAM("stdout", POV_File_Image_PNG, false);
      }
      else if ((out_file = POV_NEW_OSTREAM(name, POV_File_Image_PNG, false)) == NULL)
      {
        return;
      }

      buffer_size = opts.File_Buffer_Size;

      if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                     (png_voidp)true, png_pov_err, png_pov_warn)) == NULL ||
          (info_ptr = png_create_info_struct(png_ptr)) == NULL)
      {
        Error("Cannot allocate PNG data structures");
      }

      if (setjmp(png_ptr->jmpbuf))
      {
        /* If we get here, we had a problem writing the file */
        if (buffer != NULL)
        {
          POV_FREE(buffer);
          buffer = NULL;
        }

        png_destroy_write_struct(&png_ptr, &info_ptr);

        POV_DELETE(out_file, POV_OSTREAM);
        out_file = NULL;

        return;
      }

      /* Set up the compression structure */

      // png_set_read_fn(png_ptr, out_file, png_pov_read_data);
      png_set_write_fn(png_ptr, out_file, png_pov_write_data, png_pov_flush_data);

      /* Fill in the relevant image information */

      info_ptr->width = width = w;
      info_ptr->height = height = h;

      info_ptr->bit_depth = 8 * ((opts.OutputQuality + 7) / 8);

      if (opts.Options & HF_GRAY_16)
      {
        if ( info_ptr->bit_depth < 16 ) {
          info_ptr->bit_depth = 16;
          opts.OutputQuality = 16;
        }
        info_ptr->color_type = PNG_COLOR_TYPE_GRAY;
      }
      else
      {
        info_ptr->color_type = PNG_COLOR_TYPE_RGB;
      }

      if (opts.Options & OUTPUT_ALPHA)
      {
        info_ptr->color_type |= PNG_COLOR_MASK_ALPHA;
      }

#if defined(PNG_WRITE_sBIT_SUPPORTED)
      if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
      {
        info_ptr->sig_bit.red =
        info_ptr->sig_bit.green =
        info_ptr->sig_bit.blue = opts.OutputQuality;
      }
      else
      {
        info_ptr->sig_bit.gray = opts.OutputQuality;
      }

      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      {
        info_ptr->sig_bit.alpha = opts.OutputQuality;
      }

      info_ptr->valid |= PNG_INFO_sBIT;
#endif /* PNG_WRITE_sBIT_SUPPORTED */

#if defined(PNG_WRITE_gAMA_SUPPORTED)
      if (!opts.histogram_on)
      {
        info_ptr->gamma = 1.0/opts.DisplayGamma;
        info_ptr->valid |= PNG_INFO_gAMA;
      }
      else
      {
        info_ptr->gamma = 1.0;
        info_ptr->valid |= PNG_INFO_gAMA;
      }
#endif /* PNG_WRITE_gAMA_SUPPORTED */

#if defined(PNG_WRITE_oFFs_SUPPORTED)
      if (opts.First_Column != 0 || opts.First_Line != 0)
      {
        info_ptr->x_offset = opts.First_Column;
        info_ptr->y_offset = opts.First_Line;

        info_ptr->offset_unit_type = PNG_OFFSET_PIXEL;

        info_ptr->valid |= PNG_INFO_oFFs;
      }
#endif /* PNG_WRITE_oFFs_SUPPORTED */

      if (opts.histogram_on)
      {
      /* If we are writing a histogram file, we could potentially output
       * a pCAL chunk with the max histogram value, to allow recovery of
       * the original timing data.  However, pCAL is not yet official at
       * the time of this writing.
       */
      }

      png_write_info(png_ptr, info_ptr);

      png_stride = info_ptr->color_type & PNG_COLOR_MASK_COLOR ? 3 : 1;

      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
        png_stride++;

      png_stride *= (opts.OutputQuality + 7) / 8;

      row_ptr = (png_byte *)POV_MALLOC(w*png_stride, "PNG write row buffer");

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
      /* Set libpng to flush the output buffers every few lines, so that
       * in case of a rude crash we don't lose very much data.
       */
      png_set_flush(png_ptr, FLUSH_DIST);
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

      break;

    case APPEND_MODE:

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
      if (setjmp(png_ptr->jmpbuf))
      {
        /* If we get here, we had a problem writing the file */

        if (buffer != NULL)
        {
          POV_FREE(buffer);
          buffer = NULL;
        }

        png_destroy_write_struct(&png_ptr, &info_ptr);

        POV_DELETE(out_file, POV_OSTREAM);
        out_file = NULL;

        return;
      }

      /* Write out the data in the PNG/zlib buffers, and set automatic
       * flushing for every few scanlines, in case of a rude crash.
       */
      png_write_flush(png_ptr);
      png_set_flush(png_ptr, FLUSH_DIST);
#else  /* !PNG_WRITE_FLUSH_SUPPORTED */
      fflush(file);
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

      if (!(opts.Options & TO_STDOUT))
      {
       	POV_DELETE(out_file, POV_OSTREAM);
      	out_file = POV_NEW_OSTREAM(name, POV_File_Image_PNG, true);
        if (out_file == NULL)
        {
          if (buffer != NULL)
          {
            POV_FREE(buffer);
            buffer = NULL;
          }

          png_destroy_write_struct(&png_ptr, &info_ptr);

          return;
        }
        png_set_write_fn(png_ptr, out_file, png_pov_write_data, png_pov_flush_data);
      }

      /* Delete the temporary data file.  Note that the new output file
       * is all ready to go - nothing needs to be done here.
       */
      if (tmp_fp != NULL)
      {
        POV_DELETE(tmp_fp, POV_ISTREAM);
        tmp_fp = NULL;

        if (DELETE_FILE(tmp_fname) == DELETE_FILE_ERR)
        {
          Warning(0,"Cannot delete temporary PNG file %s.\nPlease delete it.",tmp_fname);
        }
      }
  }

  valid = true;
}


/*****************************************************************************
*
* FUNCTION      : Close_Png_File
*
* ARGUMENTS     : FILE_HANDLE *handle
*
* MODIFIED ARGS : handle
*
* RETURN VALUE  : none
*
* AUTHOR        : Andreas Dilger
*
* DESCRIPTION
*
*   Write any chunks coming after image (eg comments), and free all the
*   memory associated with the PNG IO streams.  Will output some rendering
*   stats and info into tEXt chunks if POV_COMMENTS is #defined, and will
*   also record the rendering time if CTIME is #defined.
*
* CHANGES
*
*   Updated for POV-Ray 3.X - [TIW]
*
******************************************************************************/

PNG_Image::~PNG_Image()
{
#ifdef POV_COMMENTS
  int n, index = - 1;
  png_text *text_ptr = NULL;
  char allocated[NTEXT];        /* Boolean array if text is MALLOCed */
  char bigtext[MAXTEXT];        /* Large temporary string to print into */
# ifdef CAMERA
  CAMERA *Camera = Frame.Camera;
# endif
#ifdef CTIME
  char months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
#endif
#endif

  /* Why are we here? */

  if (mode == WRITE_MODE || mode == APPEND_MODE)
  {
    /* If no line was written to the file */

    if(initial_line_number == line_number)
    {
      if(png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, &info_ptr);
      png_ptr = NULL;
    }

    if (png_ptr != NULL)
    {
      if (setjmp(png_ptr->jmpbuf))
      {
        /* If we get here, we had a problem writing the file */

        png_destroy_write_struct(&png_ptr, &info_ptr);

        if (row_ptr != NULL)
        {
          POV_FREE(row_ptr);
          row_ptr = NULL;
        }

        if (out_file != NULL)
        {
          POV_DELETE(out_file, POV_OSTREAM);
          out_file = NULL;
        }

        if (buffer != NULL)
        {
          POV_FREE(buffer);
          buffer = NULL;
        }

        Error("Cannot write PNG file.");
      }

      // have to check png_ptr again as it can be set to NULL by png_destroy_write_struct above.
      if (png_ptr->row_number < png_ptr->num_rows)
      {
         /* finished prematurely - trick into thinking done*/
         png_ptr->num_rows = png_ptr->row_number;
         png_write_finish_row(png_ptr);
      }

#ifdef POV_COMMENTS /* temporarily skip comment writing code */
      if (info_ptr != NULL)
      {
#if defined(PNG_WRITE_tIME_SUPPORTED)
        png_convert_from_time_t(&info_ptr->mod_time, tstart);
        info_ptr->valid = PNG_INFO_tIME;
#endif /* PNG_WRITE_tIME_SUPPORTED */

#if defined(PNG_WRITE_tEXt_SUPPORTED)
        text_ptr = (png_text *)POV_MALLOC(NTEXT*sizeof(png_text), "PNG comment structure");

        /* Init allocation flags. */
        for (n = 0; n < NTEXT; n++)
        {
          allocated[n] = false;
          text_ptr[n].compression = - 1;
        }

#ifdef TRACER
        text_ptr[++index].key = "Author";
        text_ptr[index].text = TRACER;
        text_ptr[index].text_length = strlen(text_ptr[index].text);
#endif

#ifdef COPYRIGHT
        text_ptr[++index].key = "Copyright";
        /* 0xA9 is the ISO-8859-1 (used in PNG tEXt) copyright character */
        sprintf(bigtext, "Copyright %c %d %s", 0xA9, info_ptr->mod_time.year,
                                               COPYRIGHT);
        text_ptr[index].text_length = strlen(bigtext);
        text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
        strcpy(text_ptr[index].text, bigtext);
        allocated[index] = true;
        if (text_ptr[index].text_length > 200) /* Compress if long copyright */
          text_ptr[index].compression = 0;
#endif

#ifdef CTIME
        /* Print the image "creation" time in RFC 1123 format */
        text_ptr[++index].key = "Creation Time";
        sprintf(bigtext, "%02d %3s %4d %02d:%02d:%02d GMT",
                info_ptr->mod_time.day, months[info_ptr->mod_time.month],
                info_ptr->mod_time.year, info_ptr->mod_time.hour, 
                info_ptr->mod_time.minute, info_ptr->mod_time.second);
        text_ptr[index].text_length = strlen(bigtext);
        text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
        strcpy(text_ptr[index].text, bigtext);
        allocated[index] = true;
#endif

        text_ptr[++index].key = "Source";
        sprintf(bigtext, "Persistence of Vision(tm) Ray Tracer v%s%s",
                POV_RAY_VERSION, COMPILER_VER);
        text_ptr[index].text_length = strlen(bigtext);
        text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
        strcpy(text_ptr[index].text, bigtext);
        allocated[index] = true;

        if (!(opts.Options & FROM_STDIN))
        {
          text_ptr[++index].key = "Input File";
          text_ptr[index].text = opts.Input_File_Name;
          text_ptr[index].text_length = strlen(text_ptr[index].text);
        }

#ifdef CAMERA
        text_ptr[++index].key = "POV Camera";
        sprintf(bigtext, "Location:   %7g %7g %7g\n"
          "           Direction:  %7g %7g %7g\n"
          "           Up:         %7g %7g %7g\n"
          "           Right:      %7g %7g %7g\n"
          "           Sky:        %7g %7g %7g",
          Camera->Location[X], Camera->Location[Y], Camera->Location[Z],
          Camera->Direction[X], Camera->Direction[Y], Camera->Direction[Z],
          Camera->Up[X], Camera->Up[Y], Camera->Up[Z],
          Camera->Right[X], Camera->Right[Y], Camera->Right[Z],
          Camera->Sky[X], Camera->Sky[Y], Camera->Sky[Z]);
        text_ptr[index].text_length = strlen(bigtext);
        text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
        strcpy(text_ptr[index].text, bigtext);
        allocated[index] = true;
#endif

        if (opts.FrameSeq.Clock_Value != 0)
        {
          text_ptr[++index].key = "POV Clock";
          sprintf(bigtext, "%g", opts.FrameSeq.Clock_Value);
          text_ptr[index].text_length = strlen(bigtext);
          text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
          strcpy(text_ptr[index].text, bigtext);
          allocated[index] = true;
        }

        if (opts.Quality != 9)
        {
          text_ptr[++index].key = "Rendering Quality";
          sprintf(bigtext, "%d", opts.Quality);
          text_ptr[index].text_length = strlen(bigtext);
          text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
          strcpy(text_ptr[index].text, bigtext);
          allocated[index] = true;
        }

        text_ptr[++index].key = "Rendering Time";
        sprintf(bigtext, "%g s", trender);
        text_ptr[index].text_length = strlen(bigtext);
        text_ptr[index].text = (char *)POV_MALLOC(text_ptr[index].text_length + 1, "PNG comment");
        strcpy(text_ptr[index].text, bigtext);
        allocated[index] = true;

        info_ptr->num_text = index + 1;
        info_ptr->max_text = NTEXT;
        info_ptr->text = text_ptr;
#endif  /* PNG_WRITE_tEXt_SUPPORTED */
      }
#endif  /* POV_COMMENTS */ 

      png_write_end(png_ptr, info_ptr);
      png_destroy_write_struct(&png_ptr, &info_ptr);

#ifdef POV_COMMENTS
      if (text_ptr != NULL)
      {
        for (n = 0; n <= index; n++)
        {
          if (allocated[n])
          {
            POV_FREE(text_ptr[n].text);
          }
        }

        POV_FREE(text_ptr);
        text_ptr = NULL;
      }
#endif /* POV_COMMENTS */

    }

    if (row_ptr != NULL)
    {
      POV_FREE(row_ptr);
      row_ptr = NULL;
    }

    if (out_file != NULL && !(opts.Options & TO_STDOUT))
    {
      POV_DELETE(out_file, POV_OSTREAM);
      out_file = NULL;
    }

    if (buffer != NULL)
    {
      POV_FREE(buffer);
      buffer = NULL;
    }
  }
  else /* READ_MODE */
  {
    if (in_file != NULL)
    {
      POV_DELETE(in_file, POV_ISTREAM);
      in_file = NULL;
    }

    if (o_png_ptr != NULL)
    {
      png_destroy_read_struct(&o_png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    }
  }
}


/*****************************************************************************
*
* FUNCTION      : Write_Png_Line
*
* ARGUMENTS     : handle, line_data, line_number
*
* MODIFIED ARGS : none
*
* RETURN VALUE  : none
*
* AUTHOR        : Andreas Dilger
*
* DESCRIPTION
*
*   Write a line of data to the PNG file
*
* CHANGES
*
*   Updated for POV-Ray 3.X - [TIW]
*   Updated to do flush output to reduce data loss - [AED]
*   Updated to output Alpha channel if requested - [AED]
*   Updated to output grayscale heightfield if requested - [AED]
*   Updated to allow grayscale in 5-8 bpp if desired, Oct 1995 - [AED]
*   Updated to allow grayscale and alpha together, Oct 1995 - [AED]
*   Changed how bit-depths 9-15 get promoted to 16 bits based on new
*     recommendations from the PNG Group,  Nov 1995 - [AED]
*
******************************************************************************/

void PNG_Image::Write_Line(COLOUR *line_data)
{
	if(valid == false)
		Error("Cannot access output image file.");

  register int col, j;
  int himask;
  int color;


  /*
   * We must copy all the values because PNG expects RGBRGB bytes, but
   * POV-Ray stores RGB components in separate arrays as floats.  In
   * order to use the full scale values at the lower bit depth, PNG
   * recommends filling the low-order bits with a copy of the high-order
   * bits.  However, studies have shown that filling the low order bits
   * with constant bits significantly improves compression, which I'm
   * doing here.  Note that since the true bit depth is stored in the
   * sBIT chunk, the extra packed bits are not important.
   */

  switch (opts.OutputQuality)
  {
    case 5:
    case 6:
    case 7:
      /* Handle shifting for arbitrary output bit depth */

      himask = 0xFF ^ ((1 << (8 - opts.OutputQuality)) - 1);

      if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          color = (png_byte)floor((line_data[col][pRED]*0.30 +
                                   line_data[col][pGREEN]*0.59 +
                                   line_data[col][pBLUE]*0.11) * 255.0);

          /* Use left-bit replication (LBR) for bit depths < 8 */
          row_ptr[j] = color & himask;
          row_ptr[j] |= color >> opts.OutputQuality;

          /* Handle Alpha here if needed - must use exact bit replication
           * instead of truncation or 100... termination
           */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            color = 255 - (int)floor(line_data[col][pTRANSM] * 255.0);

            row_ptr[j + 1] = color & himask;
            row_ptr[j + 1] |= color >> opts.OutputQuality;
          }
        }
      }
      else
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          color = (int)floor(line_data[col][pRED]   * 255.0);

          row_ptr[j] = color & himask;
          row_ptr[j] |= color >> opts.OutputQuality;

          color = (int)floor(line_data[col][pGREEN] * 255.0);

          row_ptr[j + 1] = color & himask;
          row_ptr[j + 1] |= color >> opts.OutputQuality;

          color = (int)floor(line_data[col][pBLUE]  * 255.0);

          row_ptr[j + 2] = color & himask;
          row_ptr[j + 2] |= color >> opts.OutputQuality;

          /* Handle Alpha here if needed */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            color = 255 - (int)floor(line_data[col][pTRANSM] * 255.0);

            row_ptr[j + 3] = color & himask;
            row_ptr[j + 3] |= color >> opts.OutputQuality;
          }
        }
      }
      break;

    case 8:
      if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          row_ptr[j] = (png_byte)floor((line_data[col][pRED]*0.30 +
                                        line_data[col][pGREEN]*0.59 +
                                        line_data[col][pBLUE]*0.11) * 255.0);

          /* Handle Alpha here if needed */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            row_ptr[j+1] = (png_byte)(255-floor(line_data[col][pTRANSM]*255.0));
          }
        }
      }
      else
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          row_ptr[j] = (png_byte)floor(line_data[col][pRED]   * 255.0);
          row_ptr[j + 1] = (png_byte)floor(line_data[col][pGREEN] * 255.0);
          row_ptr[j + 2] = (png_byte)floor(line_data[col][pBLUE]  * 255.0);

          /* Handle Alpha here if needed */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            row_ptr[j+3] = (png_byte)(255-floor(line_data[col][pTRANSM]*255.0));
          }
        }
      }
      break;

    case 16:
      if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          color = (int)floor((line_data[col][pRED]*0.30 +
                              line_data[col][pGREEN]*0.59 +
                              line_data[col][pBLUE]*0.11) * 65535.0);

          row_ptr[j] = color >> 8;
          row_ptr[j+1] = color & 0xff;

          /* Handle Alpha here if needed */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            color = 65535 - (int)floor(line_data[col][pTRANSM]  * 65535.0);

          row_ptr[j+2] = color >> 8;
          row_ptr[j+3] = color & 0xff;
          }
        }
      }
      else
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          color = (int)floor(line_data[col][pRED]   * 65535.0);

          row_ptr[j] = color >> 8;
          row_ptr[j + 1] = color & 0xFF;

          color = (int)floor(line_data[col][pGREEN] * 65535.0);

          row_ptr[j + 2] = color >> 8;
          row_ptr[j + 3] = color & 0xFF;

          color = (int)floor(line_data[col][pBLUE]  * 65535.0);

          row_ptr[j + 4] = color >> 8;
          row_ptr[j + 5] = color & 0xFF;

          /* Handle Alpha here if needed */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            color = 65535 - (int)floor(line_data[col][pTRANSM]  * 65535.0);

            row_ptr[j + 6] = color >> 8;
            row_ptr[j + 7] = color & 0xFF;
          }
        }
      }
      break;

    default:  /* OutputQuality 9 - 15 */
      /* Handle shifting for arbitrary output bit depth */
      himask = 0xFF ^ ((1 << (16 - opts.OutputQuality)) - 1);

      if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          color = (int)floor((line_data[col][pRED]*0.30 +
                              line_data[col][pGREEN]*0.59 +
                              line_data[col][pBLUE]*0.11) * 65535.0);

          row_ptr[j] = color >> 8;
          row_ptr[j + 1] = color & himask;
          row_ptr[j + 1] |= color >> opts.OutputQuality;

          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            color = 65535 - (int)floor(line_data[col][pTRANSM] * 65535.0);

            row_ptr[j + 2] = color >> 8;
            row_ptr[j + 3] = color & himask;
            row_ptr[j + 3] |= color >> opts.OutputQuality;
          }
        }
      }
      else
      {
        for (col = j = 0; col < width; col++, j += png_stride)
        {
          color = (int)floor(line_data[col][pRED]   * 65535.0);

          row_ptr[j] = color >> 8;
          row_ptr[j + 1] = color & himask;
          row_ptr[j + 1] |= color >> opts.OutputQuality;

          color = (int)floor(line_data[col][pGREEN] * 65535.0);

          row_ptr[j + 2] = color >> 8;
          row_ptr[j + 3] = color & himask;
          row_ptr[j + 3] |= color >> opts.OutputQuality;

          color = (int)floor(line_data[col][pBLUE]  * 65535.0);

          row_ptr[j + 4] = color >> 8;
          row_ptr[j + 5] = color & himask;
          row_ptr[j + 5] |= color >> opts.OutputQuality;

          /* Handle Alpha here if needed */
          if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            color = 65535 - (int)floor(line_data[col][pTRANSM]  * 65535.0);

            row_ptr[j + 6] = color >> 8;
            row_ptr[j + 7] = color & himask;
            row_ptr[j + 7] |= color >> opts.OutputQuality;
          }
        }
      }
  }

  if (setjmp(png_ptr->jmpbuf))
  {
    /* If we get here, we had a problem writing the file */
    POV_DELETE(out_file, POV_OSTREAM);
    out_file = NULL;

    Error("Cannot write PNG output data to %s.", filename);
  }

  /* Write out a scanline */
  png_write_row(png_ptr, row_ptr);
  line_number++;

  /* close and reopen file for integrity in case we crash   */
  /* lets not do so,the business of the OS, not ours and it */
  /* creates problems with the new io implemenation - [trf] */
  if (buffer_size == 0 && !(opts.Options & TO_STDOUT))
  {
#ifndef PNG_WRITE_FLUSH
    out_file->flush();
#endif
  }
}


/*****************************************************************************
*
* FUNCTION      : Read_Png_Line
*
* ARGUMENTS     : FILE_HANDLE *handle; COLOUR *line_data; int *line_number;
*
* MODIFIED ARGS : none
*
* RETURN VALUE  : 1 if no error exit
*
* AUTHOR        : Andreas Dilger
*
* DESCRIPTION
*
*   Read a line of PNG data
*
* CHANGES
*
*   Updated for POV-Ray 3.X - [TIW]
*   Updated to handle interrupted file resuming Sept 1995 - [AED]
*   Updated to support grayscale and alpha together, Oct 1995 - [AED]
*
******************************************************************************/

int PNG_Image::Read_Line(COLOUR *line_data)
{
	if(valid == false)
		Error("Cannot access output image file.");

  register int col, j, step;

  if (setjmp(o_png_ptr->jmpbuf))
  {
    /* If we get here, we had a problem reading the file, which probably
     * means that we have read all the available data, rather than a real
     * error, but there is no sure way to know.
     */
    Status_Info("\n");
    return 0;
  }

  if (setjmp(png_ptr->jmpbuf))
  {
    /* If we get here, we had a problem writing the new file */
    Status_Info("\n");

    POV_DELETE(in_file, POV_ISTREAM);
    in_file = NULL;
    POV_DELETE(tmp_fp, POV_ISTREAM);
    tmp_fp = NULL;

    if (DELETE_FILE(filename) != DELETE_FILE_ERR)
    {
      RENAME_FILE(tmp_fname,filename); /* Move original file back */
    }

    return -1;
  }

  /* Read in another row if available */
  png_read_row(o_png_ptr, row_ptr, NULL);

  /* We won't get here if there was a read error */
  png_write_row(png_ptr, row_ptr);

  if (line_number % 32 == 31)
    Status_Info(".");
  
  /*
   * We must copy all the values because PNG supplies RGBRGB, but POV-Ray
   * stores RGB components in separate arrays.  Note that since we have
   * already written the data out to the temporary file, we only need to
   * use the top 8 bits for the line_data info as it is only used for
   * potential screen output.
   */

  /* How many bytes in a sample */
  step = (info_ptr->bit_depth <= 8) ? 1 : 2;

  if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
  {
    for (col = j = 0; col < width; col++, j += png_stride)
    {
      line_data[col][pRED] = (DBL)row_ptr[j] / 255.0;
      line_data[col][pGREEN] = (DBL)row_ptr[j] / 255.0;
      line_data[col][pBLUE] = (DBL)row_ptr[j] / 255.0;

      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      {
        line_data[col][pTRANSM] = (DBL)(255 - row_ptr[j + step]) / 255.0;
      }
    }
  }
  else
  {
    for (col = j = 0; col < width; col++, j += png_stride)
    {
      line_data[col][pRED] = (DBL)row_ptr[j] / 255.0;
      line_data[col][pGREEN] = (DBL)row_ptr[j + step] / 255.0;
      line_data[col][pBLUE] = (DBL)row_ptr[j + 2*step] / 255.0;

      if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      {
        line_data[col][pTRANSM] = (DBL)(255 - row_ptr[j + 3*step]) / 255.0;
      }
    }
  }

  /* Note that line_number is that of the first blank (i.e. missing) row */
#if defined(PNG_READ_oFFS_SUPPORTED)
  line_number = info_ptr->y_offset + png_ptr->row_number ;
#else
  line_number = png_ptr->row_number ;
#endif

  return 1;
}


/*****************************************************************************
*
* FUNCTION      : Read_Png_Image
*
* ARGUMENTS     : IMAGE *Image; char *name;
*
* MODIFIED ARGS : Image
*
* RETURN VALUE  : none
*
* AUTHOR        : Andreas Dilger
*
* DESCRIPTION
*
*   Reads a PNG image into an RGB image buffer
*
* CHANGES
*
*   Updated for POV-Ray 3.X - [TIW]
*   Updated to allow grayscale and alpha together, Oct 1995 - [AED]
*   Fixed palette size for grayscale images with bit-depth <= 8, Nov 1995 [AED]
*   Changed how grayscale images > 8bpp are stored based on use, Nov 1995 [AED]
*
******************************************************************************/

void Read_Png_Image(IMAGE *Image, char *name)
{
  unsigned int width, height;
  int row, col, j;
  int stride;
  POV_ISTREAM *filep;
  IMAGE8_LINE *line_data;
  png_struct *r_png_ptr;
  png_info *r_info_ptr;
  png_byte **row_ptrs;

  /* Start by trying to open the file */

  if ((filep = Locate_File(name,POV_File_Image_PNG,NULL,true)) == NULL)
  {
    Error("Cannot open PNG file.");
  }

  if ((r_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                   (png_voidp)true, png_pov_err, png_pov_warn)) == NULL ||
      (r_info_ptr = png_create_info_struct(r_png_ptr)) == NULL)
  {
    Error("Cannot allocate PNG data structures");
  }

  if (setjmp(r_png_ptr->jmpbuf))
  {
    /* If we get here, we had a problem reading the file */

    png_destroy_read_struct(&r_png_ptr, &r_info_ptr, (png_infopp)NULL);
    Error("Cannot read PNG image.");
  }

  /* set up the input control */

  png_set_read_fn(r_png_ptr, filep, png_pov_read_data);

  /* read the file information */

  png_read_info(r_png_ptr, r_info_ptr);

  width = r_info_ptr->width;
  height = r_info_ptr->height;

  Image->iwidth = width;
  Image->iheight = height;
  Image->width = (DBL)width;
  Image->height = (DBL)height;

  /* Allocate buffers for the image */
  stride = 1;

  if (r_info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
  {
    IMAGE_COLOUR *cmap;
    png_color *png_cmap;
    int cmap_len = r_info_ptr->num_palette;
    int index;

    Image->Colour_Map_Size = cmap_len;

    cmap = (IMAGE_COLOUR *)POV_MALLOC(cmap_len*sizeof(IMAGE_COLOUR), "PNG image color map");

    Image->Colour_Map = cmap;
    png_cmap = r_info_ptr->palette;

    for (index = 0; index < cmap_len; index++)
    {
      cmap[index].Red = png_cmap[index].red;
      cmap[index].Green = png_cmap[index].green;
      cmap[index].Blue = png_cmap[index].blue;
      cmap[index].Filter = 0;
      cmap[index].Transmit = 0;
    }

    if (r_info_ptr->valid & PNG_INFO_tRNS)
    {
      for (index = 0; index < r_info_ptr->num_trans; index++)
        cmap[index].Transmit = 255 - r_info_ptr->trans[index];
    }

    Image->data.map_lines = (unsigned char **)
      POV_MALLOC(height * sizeof(unsigned char *), "PNG image");

    /* tellg pnglib to expand data to 1 pixel/byte */
    png_set_packing(r_png_ptr);
  }
  else if (r_info_ptr->color_type == PNG_COLOR_TYPE_GRAY &&
           r_info_ptr->bit_depth <= 8)
  {
    IMAGE_COLOUR *cmap;
    int cmap_len;
    int index;
    
    Image->Colour_Map_Size = cmap_len = 1 << r_info_ptr->bit_depth;
    
    cmap = (IMAGE_COLOUR *)POV_MALLOC(cmap_len*sizeof(IMAGE_COLOUR), "PNG image color map");

    Image->Colour_Map = cmap;

    for (index = 0; index < cmap_len; index++)
    {
      cmap[index].Red =
      cmap[index].Green =
      cmap[index].Blue = index;
      cmap[index].Filter = 0;
      cmap[index].Transmit = 0;
    }

    if (r_info_ptr->valid & PNG_INFO_tRNS)
    {
      for (index = 0; index < r_info_ptr->num_trans; index++)
        cmap[index].Transmit = 255 - r_info_ptr->trans[index];
    }

    Image->data.map_lines = (unsigned char **)
      POV_MALLOC(height * sizeof(unsigned char *), "PNG image");
    
    /* tellg pnglib to expand data to 1 pixel/byte */
    png_set_packing(r_png_ptr);
  }
  else if (r_info_ptr->color_type == PNG_COLOR_TYPE_GRAY ||
           r_info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    Image->Colour_Map = NULL;
    
    Image->data.rgb8_lines = (IMAGE8_LINE *)
      POV_MALLOC(height * sizeof(IMAGE8_LINE), "PNG image");

    if (r_info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
    {
      stride = 2;
    }
    else if (r_info_ptr->bit_depth <= 8) /* PNG_COLOR_TYPE_GRAY_ALPHA */
    {
      /* tellg pnglib to expand data to 1 pixel/byte */
      png_set_packing(r_png_ptr);
      stride = 2;
    }
    else                                  /* PNG_COLOR_TYPE_GRAY_ALPHA */
    {
      stride = 4;
    }
  }
  else if (r_info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
      r_info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
  {
    Image->Colour_Map = NULL;
    
    /* tellg pnglib to strip 16 bit depth files down to 8 bits */
    if (r_info_ptr->bit_depth > 8)
    {
      if (opts.Options & VERBOSE)
        Warning(0,"Converting PNG image map to 8 bits/sample from higher bit depth.");
      png_set_strip_16(r_png_ptr);
    }
    
    Image->data.rgb8_lines = (IMAGE8_LINE *)
    POV_MALLOC(height * sizeof(IMAGE8_LINE), "PNG image");

    if (r_info_ptr->color_type == PNG_COLOR_TYPE_RGB)
      stride = 3;
    else                               /* PNG_COLOR_TYPE_RGB_ALPHA */
      stride = 4;
  }
  else                                 /* Unknown PNG type */
  {
    Error("Unsupported color type %d in PNG image.", r_info_ptr->color_type);
  }
  
  /* tellg pnglib to handle the gamma conversion for you.  Note that
   * GammaFactor * DisplayFactor = assumed_gamma, so we are converting
   * images into the "internal gamma" space of POV (rather than to a
   * gamma of 1.0) to avoid doing gamma correction on image maps twice for
   * those scene files which don't have a gamma of 1.0.  For POV 3.0,
   * we will only do input gamma conversion on those files which will be
   * used as image maps, and the other types will load the raw pixel values.
   */
#if defined(PNG_READ_GAMMA_SUPPORTED) && defined(PNG_READ_gAMA_SUPPORTED)
  if (r_info_ptr->valid & PNG_INFO_gAMA && (Image->Image_Type & IMAGE_FTYPE))
  {
    png_set_gamma(r_png_ptr, opts.GammaFactor*opts.DisplayGamma,
                                                          r_info_ptr->gamma);
  }
#endif /* PNG_READ_GAMMA_SUPPORTED and PNG_READ_gAMA_SUPPORTED */
    
  png_set_interlace_handling(r_png_ptr);
  png_read_update_info(r_png_ptr, r_info_ptr);

  /* Allocate row buffers for the input */
  row_ptrs = (png_byte **)POV_MALLOC(height*sizeof(png_byte *), "PNG image");
  
  for (row = 0; row < height; row++)
  {
    row_ptrs[row] = (png_byte *)POV_MALLOC(r_info_ptr->rowbytes, "PNG image line");
  }
  
  /* Read in the entire image */
  png_read_image(r_png_ptr, row_ptrs);
  
  /* We must copy all the values because PNG supplies RGBRGB, but POV-Ray
   * stores RGB components in separate arrays
   */
  for (row = 0; row < height; row++)
  {
    if (Image->Colour_Map == NULL)
    {
      line_data = &Image->data.rgb8_lines[row];

      line_data->red = (unsigned char *)POV_MALLOC(width, "PNG image line");
      line_data->green = (unsigned char *)POV_MALLOC(width, "PNG image line");
      line_data->blue = (unsigned char *)POV_MALLOC(width, "PNG image line");

      if (r_info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      {
        line_data->transm = (unsigned char *)POV_MALLOC(width,"PNG image line");
      }
      else
      {
        line_data->transm = NULL;
      }
      
      /* 8-bit grayscale image with a full alpha channel. Since the paletted
       * images don't support different transparencies for each pixel, we
       * have to make this a full-color image.
       */
      if (r_info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA &&
          r_info_ptr->bit_depth <= 8)
      {
        for (col = j = 0; col < width; col ++, j += stride)
        {
          line_data->red[col] =
          line_data->green[col] =
          line_data->blue[col] = row_ptrs[row][j];
          line_data->transm[col] = 255 - row_ptrs[row][j + 1];
        }
      }
      /* For 16 bit PNG heightfields, we need to know if the map will be used
       * for an image (in which case we want to store the values as grays, or
       * if we want to use it as a heightfield (in which case we need to store
       * it in the MSB-read, LSB-green format that POV uses to store 16-bit
       * heightfields.
       */
      else if ((r_info_ptr->color_type & PNG_COLOR_MASK_COLOR) ==
                                                          PNG_COLOR_TYPE_GRAY)
      {
        if (Image->Image_Type & HF_FTYPE)
        {
          for (col = j = 0; col < width; col ++, j += stride)
          {
            int red = row_ptrs[row][j];
            int green = row_ptrs[row][j + 1];

            line_data->red[col] = red;
            line_data->green[col] = green;
            line_data->blue[col] = 0;

            if (r_png_ptr->color_type & PNG_COLOR_MASK_ALPHA)
            {
              line_data->transm[col] = 255 - row_ptrs[row][j + 2];
            }
          }
        }
        else
        {
          for (col = j = 0; col < width; col ++, j += stride)
          {
            int red = row_ptrs[row][j];
  
            line_data->red[col] = red;
            line_data->green[col] = red;
            line_data->blue[col] = red;

            if (r_png_ptr->color_type & PNG_COLOR_MASK_ALPHA)
            {
              line_data->transm[col] = 255 - row_ptrs[row][j + 2];
            }
          }
        }
      }
      else /* r_info_ptr->color_type & PNG_COLOR_MASK_COLOR */
      {
        for (col = j = 0; col < width; col ++, j += stride)
        {
          line_data->red[col] = row_ptrs[row][j];
          line_data->green[col] = row_ptrs[row][j + 1];
          line_data->blue[col] = row_ptrs[row][j + 2];

          if (r_png_ptr->color_type & PNG_COLOR_MASK_ALPHA)
          {
            line_data->transm[col] = 255 - row_ptrs[row][j + 3];
          }
        }
      }
      POV_FREE(row_ptrs[row]);
    }
    else
    {
      Image->data.map_lines[row] = row_ptrs[row];
    }
  }

  /* Clean up the rest of the PNG memory and such */

  POV_FREE(row_ptrs);

  /* read the rest of the file, getting any additional chunks in png_info */

  png_read_end(r_png_ptr, r_info_ptr);

  /* clean up after the read, and free any memory allocated */

  png_destroy_read_struct(&r_png_ptr, &r_info_ptr, (png_infopp)NULL);

  POV_DELETE(filep, POV_ISTREAM);
}

