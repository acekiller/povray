/****************************************************************************
*                file_pov.cpp
*
*  This module implements the utility functions for handling files.
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
* $File: //depot/povray/3.5/source/file_pov.cpp $
* $Revision: #37 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include <time.h>
#include <stdarg.h>
#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "bbox.h"
#include "chi2.h"
#include "colour.h"
#include "interior.h"
#include "lighting.h"
#include "normal.h"
#include "objects.h"
#include "octree.h"
#include "optout.h"
#include "povray.h"
#include "radiosit.h"
#include "ray.h"
#include "render.h"
#include "targa.h"
#include "texture.h"
#include "vbuffer.h"
#include "userio.h"
#include "userdisp.h"
#include "parse.h"
#include "tokenize.h"
#include "povmsend.h"
#include "file_pov.h"
#include "pov_util.h"

#ifndef POV_IS1
#define POV_IS1 ""
#endif

#ifndef POV_IS2
#define POV_IS2 ""
#endif

#ifndef POV_IS3
#define POV_IS3 ""
#endif

#ifndef POV_IS4
#define POV_IS4 ""
#endif


typedef struct
{
  char *ext[4];
} POV_File_Extensions;


POV_File_Extensions gPOV_File_Extensions[POV_File_Unknown_Count] =
{
  { "",      "",      "",      ""      }, // POV_File_Unknown
  { ".tga",  ".TGA",  "",      ""      }, // POV_File_Image_Targa
  { ".png",  ".PNG",  "",      ""      }, // POV_File_Image_PNG
  { ".ppm",  ".PPM",  "",      ""      }, // POV_File_Image_PPM
  { ".pgm",  ".PGM",  "",      ""      }, // POV_File_Image_PGM
  { ".gif",  ".GIF",  "",      ""      }, // POV_File_Image_GIF
  { ".iff",  ".IFF",  "",      ""      }, // POV_File_Image_IFF
  { ".jpg",  ".JPG",  ".jpeg", ".JPEG" }, // POV_File_Image_JPEG
  { ".tif",  ".TIF",  ".tiff", ".TIFF" }, // POV_File_Image_TIFF
  { POV_IS1, POV_IS2, POV_IS3, POV_IS4 }, // POV_File_Image_System
  { ".pov",  ".POV",  "",      ""      }, // POV_File_Text_POV
  { ".inc",  ".INC",  "",      ""      }, // POV_File_Text_INC
  { ".ini",  ".INI",  "",      ""      }, // POV_File_Text_INI
  { ".csv",  ".CSV",  "",      ""      }, // POV_File_Text_CSV
  { ".txt",  ".TXT",  "",      ""      }, // POV_File_Text_Stream
  { "",      "",      "",      ""      }, // POV_File_Text_User
  { ".df3",  ".DF3",  "",      ""      }, // POV_File_Data_DF3
  { ".rca",  ".RCA",  "",      ""      }, // POV_File_Data_RCA
  { ".log",  ".LOG",  "",      ""      }, // POV_File_Data_LOG
  { ".ttf",  ".TTF",  "",      ""      }  // POV_File_Font_TTF
};

/*
// not used right now
typedef struct
{
  bool read_local;
  bool read_global;
  bool write_local;
  bool write_global;
} POV_File_Restrictions;

POV_File_Restrictions gPOV_File_Restrictions[POV_File_Unknown_Count] =
{
  { false, false, false, false }, // POV_File_Unknown
  { true,  true,  false, false }, // POV_File_Image_Targa
  { true,  true,  false, false }, // POV_File_Image_PNG
  { true,  true,  false, false }, // POV_File_Image_PPM
  { true,  true,  false, false }, // POV_File_Image_PGM
  { true,  true,  false, false }, // POV_File_Image_GIF
  { true,  true,  false, false }, // POV_File_Image_IFF
  { true,  true,  false, false }, // POV_File_Image_JPEG
  { true,  true,  false, false }, // POV_File_Image_TIFF
  { true,  true,  false, false }, // POV_File_Image_System
  { true,  false, false, false }, // POV_File_Text_POV
  { true,  false, false, false }, // POV_File_Text_INC
  { true,  false, false, false }, // POV_File_Text_INI
  { true,  true,  false, false }, // POV_File_Text_CSV
  { true,  false, false, false }, // POV_File_Text_Stream
  { true,  true,  false, false }, // POV_File_Text_User
  { true,  true,  true,  false }, // POV_File_Data_DF3
  { true,  true,  true,  true  }, // POV_File_Data_RCA
  { true,  true,  true,  true  }, // POV_File_Data_LOG
  { true,  false, true,  false }  // POV_File_Font_TTF
};
*/

POV_ISTREAM *Opts_Locate_File(char *filename, u_int32 stype, char *buffer, int err_flag, POVMSObjectPtr obj);


/*****************************************************************************
*
* FUNCTION
*
*   Locate_File
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
*   Find a file in the search path.
*
* CHANGES
*
*   Apr 1996: Don't add trailing FILENAME_SEPARATOR if we are immediately
*             following DRIVE_SEPARATOR because of Amiga probs.  [AED]
*
******************************************************************************/

POV_ISTREAM *Locate_File(char *filename, u_int32 stype, char *buffer, int err_flag)
{
  POV_ISTREAM *result;
  char *qualified_name = Locate_Filename(filename, stype, err_flag);

  if (qualified_name != NULL) {
        POV_GET_FULL_PATH(f, qualified_name, buffer);
        result = POV_NEW_ISTREAM(qualified_name, stype);
    POV_FREE(qualified_name);
  } else {
    /* Any error was already reported in Locate_Filename(...) */
    result = NULL;
  }

  return result;
}

POV_ISTREAM *Opts_Locate_File(char *filename, u_int32 stype, char *buffer, int err_flag, POVMSObjectPtr obj)
{
  int i,ii,l[4];
  char pathname[FILE_NAME_LENGTH];
  char file[FILE_NAME_LENGTH];
  char file_x[4][FILE_NAME_LENGTH];
  long cnt = 0;
  long ll;
  POVMSAttribute attr, item;

  if(Has_Extension(filename))
  {
    for(i = 0; i < 4; i++)
    l[i]=0;
  }
  else
  {
    for(i = 0; i < 4; i++)
    {
      if((l[i] = strlen(gPOV_File_Extensions[stype].ext[i])) > 0)
      {
        strcpy(file_x[i], filename);
        strcat(file_x[i], gPOV_File_Extensions[stype].ext[i]);
      }
    }
  }

  /* Check the current directory first. */
  for(i = 0; i < 4; i++)
  {
    if(l[i])
    {
      if(EXIST_FILE(file_x[i]) == true)
      {
        POV_GET_FULL_PATH(f,file_x[i],buffer);
        return POV_NEW_ISTREAM(file_x[i], stype);
      }
    }
  }
  if(EXIST_FILE(filename) == true)
  {
    POV_GET_FULL_PATH(f,filename,buffer);
    return POV_NEW_ISTREAM(filename, stype);
  }

  if(POVMSObject_Get(obj, &attr, kPOVAttrib_LibraryPath) != 0)
    return NULL;

  if(POVMSAttrList_Count(&attr, &cnt) != 0)
    return NULL;

  for (i = 1; i <= cnt; i++)
  {
    (void)POVMSAttr_New(&item);
    if(POVMSAttrList_GetNth(&attr, i, &item) != 0)
      continue;
    ll = 0;
    if(POVMSAttr_Size(&item, &ll) != 0)
    {
      (void)POVMSAttr_Delete(&item);
      continue;
    }
    if(ll <= 0)
    {
      (void)POVMSAttr_Delete(&item);
      continue;
    }
    if(POVMSAttr_Get(&item, kPOVMSType_CString, file, &ll) != 0)
    {
      (void)POVMSAttr_Delete(&item);
      continue;
    }
    (void)POVMSAttr_Delete(&item);

    file[strlen(file)+1] = '\0';
    if(file[strlen(file) - 1] != DRIVE_SEPARATOR)
      file[strlen(file)] = FILENAME_SEPARATOR;

    for(ii = 0; ii < 4; ii++)
    {
      if(l[ii])
      {
        strcpy(pathname, file);
        strcat(pathname, file_x[ii]);
        if(EXIST_FILE(pathname) == true)
        {
          POV_GET_FULL_PATH(f,pathname,buffer);
          return POV_NEW_ISTREAM(pathname, stype);
        }
      }
    }
    strcpy(pathname, file);
    strcat(pathname, filename);
    if(EXIST_FILE(pathname) == true)
    {
      POV_GET_FULL_PATH(f,pathname,buffer);
      return POV_NEW_ISTREAM(pathname, stype);
    }
  }
 
  if(err_flag)
  {
    if(l[0])
    {
      PossibleError("Could not find file '%s%s'",filename,gPOV_File_Extensions[stype].ext[0]);
    }
    else
    {
      PossibleError("Could not find file '%s'",filename);
    }
  }
  
  return NULL;
}

/*****************************************************************************
*
* FUNCTION
*
*   Locate_Filename
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*  Fully expanded filename, including drive, path, ...
*   
* AUTHOR
*
*   Alexander R. Enzmann
*   
* DESCRIPTION
*
*   Find a file in the search path.
*
* CHANGES
*
*
******************************************************************************/

char *Locate_Filename(char *filename, u_int32 stype, int err_flag)
{
  int i,ii,l[4];
  char pathname[FILE_NAME_LENGTH];
  char file[FILE_NAME_LENGTH];
  char file_x[4][FILE_NAME_LENGTH];
  char *result = NULL;

  if (Has_Extension(filename)) {
    for(i = 0; i < 4; i++)
      l[i]=0;
  } else {
    for(i = 0; i < 4; i++) {
      if ((l[i] = strlen(gPOV_File_Extensions[stype].ext[i])) > 0) {
        strcpy(file_x[i], filename);
        strcat(file_x[i], gPOV_File_Extensions[stype].ext[i]);
      }
    }
  }

  /* Check the current directory first. */
  for(i = 0; i < 4; i++) {
    /* Try appending the variations of the file extension */
    if(l[i]) {
      if (EXIST_FILE(file_x[i]) == true) {
        result = (char *)POV_MALLOC(sizeof(char) * strlen(file_x[i]) + 1, "filename");
        POV_GET_FULL_PATH(f, file_x[i], result);
        return result;
      }
    }
  }
  /* Try the filename without any modifications */
  if (EXIST_FILE(filename) == true) {
    result = (char *)POV_MALLOC(sizeof(char) * strlen(filename) + 1, "filename");
    POV_GET_FULL_PATH(f, filename, result);
    return result;
  }
  
  /* Walk through the library paths, trying with and without file extensions */
  for (i = 0; i < opts.Library_Path_Index; i++) {
    strcpy(file, opts.Library_Paths[i]);
    file[strlen(file)+1] = '\0';
    if (file[strlen(file) - 1] != DRIVE_SEPARATOR)
      file[strlen(file)] = FILENAME_SEPARATOR;
    
    for(ii = 0; ii < 4; ii++) {
      if(l[ii]) {
        strcpy(pathname, file);
        strcat(pathname, file_x[ii]);
        if (EXIST_FILE(pathname) == true) {
          result = (char *)POV_MALLOC(sizeof(char) * strlen(pathname) + 1, "pathname");
          POV_GET_FULL_PATH(f, pathname, result);
          return result;
        }
      }
    }
    
    strcpy(pathname, file);
    strcat(pathname, filename);
    if (EXIST_FILE(pathname) == true) {
      result = (char *)POV_MALLOC(sizeof(char) * strlen(pathname) + 1, "pathname");
      POV_GET_FULL_PATH(f, pathname, result);
      return result;
    }
  }

  // Allow system specific access of font files:
  // Obviously this requires POV_NEW_ISTREAM
  // to be platform specific as well! [trf]
  if(stype == POV_File_Font_TTF)
  {
    if(EXIST_FONT_FILE(filename))
       return FONT_FILE_PATH(filename);
  }

  if (err_flag) {
    if (l[0]) {
      PossibleError("Could not find file '%s%s'",filename,gPOV_File_Extensions[stype].ext[0]);
    } else {
      PossibleError("Could not find file '%s'",filename);
    }
  }
  
  return NULL;
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

int Has_Extension(char *name)
{
   char *p;

   if (name!=NULL)
   {
     p=strrchr(name, '.');

     if (p!=NULL)
     {
        if ((strlen(name)-(p-name))<=4)
        {
           return (true);
        }
     }
   }
   return (false);
}

void POV_Split_Path(char *s, char *p, char *f)
{
  char *l;

  strcpy(p,s);

  if ((l=strrchr(p,FILENAME_SEPARATOR))==NULL)
  {
     if ((l=strrchr(p,DRIVE_SEPARATOR))==NULL)
     {
        strcpy(f,s);
        p[0]='\0';
        return;
     }
  }
  
  l++;
  strcpy(f,l);
  *l='\0';

}

bool POV_File_Exist(char *name)
{
  FILE *file = fopen(name, "r");

  if(file != NULL)
    fclose(file);
  else
    return false;

  return true;
}

pov_istream_class::pov_istream_class (const u_int32 stype) : pov_io_base (input, stype)
{
}

pov_istream_class::~pov_istream_class ()
{
}

int32 pov_istream_class::Read_Short (void)
{
  int16 result ;
  read ((char *) &result, 2) ;
  return (result) ;
}

int32 pov_istream_class::Read_Long (void)
{
  int32 result ;
  read ((char *) &result, 4) ;
  return (result) ;
}

pov_ostream_class::pov_ostream_class (const u_int32 stype) : pov_io_base (output, stype)
{
}

pov_ostream_class::~pov_ostream_class ()
{
}

void pov_ostream_class::printf (char *format, ...)
{
  va_list marker;
  char buffer[1024];

  va_start(marker, format);
  vsprintf (buffer, format, marker);
  va_end(marker);

  *this << buffer ;
}

/*
POV_ISTREAM *POV_New_IStream_UCS2(const UCS2 *sname, const u_int32 stype)
{
  char *str = UCS2_To_String(sname);
  POV_ISTREAM *stream = POV_New_IStream(str, stype);

  POV_FREE(str);

  return stream;
}
*/

pov_istream_class *POV_New_IStream (const char *sname, const u_int32 stype)
{
  if (pov_stricmp (sname, "stdin") != 0)
  {
    if (!POV_ALLOW_FILE_READ (sname, stype))
    {
#ifndef NO_GENERIC_IO_ERROR
      Error("I/O restriction prohibits read access to file '%s'.\n"
            "Refer to the platform specific documentation for details.", sname);
#endif
      return (NULL) ;
    }
  }

  pov_istream_class *istreamptr = POV_NEW (pov_istream_class) (stype) ;

  if (istreamptr == NULL)
    return (NULL) ;

  if (istreamptr->open (sname) == 0)
  {
    POV_DELETE (istreamptr, pov_istream_class) ;
    return (NULL) ;
  }

  return (istreamptr) ;
}

/*
POV_ISTREAM *POV_New_OStream_UCS2(const UCS2 *sname, const u_int32 stype, const bool sappend)
{
  char *str = UCS2_To_String(sname);
  POV_OSTREAM *stream = POV_New_OStream(str, stype, sappend);

  POV_FREE(str);

  return stream;
}
*/

pov_ostream_class *POV_New_OStream(const char *sname, const u_int32 stype, const bool sappend)
{
  u_int32 Flags = pov_io_base::none;

  if (pov_stricmp (sname, "stdout") != 0)
  {
    if (pov_stricmp (sname, "stderr") != 0)
    {
      if (!POV_ALLOW_FILE_WRITE (sname, stype))
      {
#ifndef NO_GENERIC_IO_ERROR
        Error("I/O restriction prohibits write access to file '%s'.\n"
              "Refer to the platform specific documentation for details.", sname);
#endif
        return (NULL) ;
      }
    }
  }

  pov_ostream_class *ostreamptr = POV_NEW (pov_ostream_class) (stype) ;

  if (ostreamptr == NULL)
    return (NULL) ;

  if (sappend) {
    Flags |= pov_io_base::append;
  }

  switch (stype) {
  case POV_File_Text_POV:
  case POV_File_Text_INC:
  case POV_File_Text_INI:
  case POV_File_Text_CSV:
  case POV_File_Text_Stream:
  case POV_File_Text_User:
  case POV_File_Data_LOG:
    Flags |= pov_io_base::textMode;
  }

  if (ostreamptr->open (sname, Flags) == 0)
  {
    POV_DELETE (ostreamptr, POV_OSTREAM) ;
    return (NULL) ;
  }

  return (ostreamptr) ;
}

pov_io_base::pov_io_base (u_int32 dir, u_int32 type)
{
  filetype = type ;
  direction = dir ;
  fail = true ;
  f = NULL ;
  filename = NULL ;
}

pov_io_base::~pov_io_base ()
{
  close () ;
}

bool pov_io_base::open (const char *Name, u_int32 Flags /* = 0 */)
{
  char        mode [8] ;

  close () ;

  if ((Flags & append) == 0)
  {
    switch (direction)
    {
      case input :
           strcpy (mode, "r") ;
           break ;

      case output :
           strcpy (mode, "w") ;
           break ;

      case io :
           strcpy (mode, "w+") ;
           break ;

      default :
        return (false) ;
    }
  }
  else
  {
    // we cannot use append mode here, since "a" mode is totally incompatible with any
    // output file format that requires in-place updates (i.e. writing to any location
    // other than the end of the file). BMP files are in this category. In theory, "r+"
    // can do anything "a" can do (with appropriate use of seek()) so append mode should
    // not be needed.
    strcpy (mode, "r+") ;
  }

  if ((Flags & textMode) == 0)
    strcat (mode, "b") ;

  f = NULL ;
  if (pov_stricmp (Name, "stdin") == 0)
  {
    if (direction != input || (Flags & append) != 0)
      return (false) ;
    f = stdin ;
  }
  else if (pov_stricmp (Name, "stdout") == 0)
  {
    if (direction != output || (Flags & append) != 0)
      return (false) ;
    f = stdout ;
  }
  else if ((f = fopen (Name, mode)) == NULL)
  {
    if ((Flags & append) == 0)
      return (false) ;

    // to maintain traditional POV +c (continue) mode compatibility, if
    // the open for append of an existing file fails, we allow a new file
    // to be created.
    mode [0] = 'w' ;
    if ((f = fopen (Name, mode)) == NULL)
      return (false) ;
  }
  fail = false ;

  if ((Flags & append) != 0)
  {
    if (!seekg (0, seek_end))
    {
      close () ;
      return (false) ;
    }
  }

  filename = (char *) POV_MALLOC (strlen (Name) + 1, "filename") ;
  strcpy (filename, Name) ;
  
  return (true) ;
}

bool pov_io_base::close (void)
{
  if (f != NULL)
  {
    fclose (f) ;
    f = NULL ;
  }
  if (filename != NULL)
  {
    POV_FREE (filename) ;
    filename = NULL ;
  }
  fail = true ;
  return (true) ;
}

pov_io_base& pov_io_base::flush (void)
{
  if (f != NULL)
    fflush (f) ;
  return (*this) ;
}

pov_io_base& pov_io_base::read (void *buffer, u_int32 count)
{
  if (!fail && count > 0)
    fail = fread (buffer, count, 1, f) != 1 ;
  return (*this) ;
}

pov_io_base& pov_io_base::write (void *buffer, u_int32 count)
{
  if (!fail && count > 0)
    fail = fwrite (buffer, count, 1, f) != 1 ;
  return (*this) ;
}

// Strictly speaking, this should -not- be called seekg, since 'seekg' (an iostreams
// term) applies only to an input stream, and therefore the use of this name here
// implies that only the input stream will be affected on streams opened for I/O
// (which is not the case with fseek, since fseek moves the pointer for output too).
// However, the macintosh code seems to need it to be called seekg, so it is ...
pov_io_base& pov_io_base::seekg (u_int32 pos, u_int32 whence /* = seek_set */)
{
  if (!fail)
    fail = fseek (f, pos, whence) != 0 ;
  return (*this) ;
}

pov_istream_class& pov_istream_class::UnRead_Byte (int32 c)
{
  if (!fail)
    fail = ungetc (c, f) != c ;
  return (*this) ;
}

pov_istream_class& pov_istream_class::getline (char *s, u_int32 buflen)
{
  int chr = 0;

  if(feof(f) != 0)
    fail = true;

  if (!fail && buflen > 0)
  {
    while(buflen > 1)
    {
      chr = fgetc(f);
      if(chr == EOF)
        break;
      else if(chr == 10)
      {
        chr = fgetc(f);
        if(chr != 13)
          ungetc(chr, f);
        break;
      }
      else if(chr == 13)
      {
        chr = fgetc(f);
        if(chr != 10)
          ungetc(chr, f);
        break;
      }
      *s = chr;
      s++;
      buflen--;
    }
    *s = 0;
  }

  return (*this) ;
}
