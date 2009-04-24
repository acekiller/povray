/****************************************************************************
*                   file_pov.h
*
*  This module contains all defines, typedefs, and prototypes for FILE_POV.CPP.
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
* $File: //depot/povray/3.5/source/file_pov.h $
* $Revision: #18 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef FILE_POV_H
#define FILE_POV_H

#ifndef POV_SEEK_SET
#define POV_SEEK_SET pov_io_base::seek_set
#endif

#ifndef POV_SEEK_CUR
#define POV_SEEK_CUR pov_io_base::seek_cur
#endif

#ifndef POV_SEEK_END
#define POV_SEEK_END pov_io_base::seek_end
#endif

enum
{
  POV_File_Unknown = 0,
  POV_File_Image_Targa = 1,
  POV_File_Image_PNG = 2,
  POV_File_Image_PPM = 3,
  POV_File_Image_PGM = 4,
  POV_File_Image_GIF = 5,
  POV_File_Image_IFF = 6,
  POV_File_Image_JPEG = 7,
  POV_File_Image_TIFF = 8,
  POV_File_Image_System = 9,
  POV_File_Text_POV = 10,
  POV_File_Text_INC = 11,
  POV_File_Text_Macro = POV_File_Text_INC, 
  POV_File_Text_INI = 12,
  POV_File_Text_CSV = 13,
  POV_File_Text_Stream = 14,
  POV_File_Text_User = 15,
  POV_File_Data_DF3 = 16,
  POV_File_Data_RCA = 17,
  POV_File_Data_LOG = 18,
  POV_File_Font_TTF = 19,
  POV_File_Unknown_Count = 20
};

class pov_io_base
{
  public:
    pov_io_base (u_int32 dir, u_int32 type) ;
    virtual ~pov_io_base () ;

    enum {none = 0, append = 1, textMode = 2 } ;
    enum {input, output, io} ;
    enum {seek_set = SEEK_SET, seek_cur = SEEK_CUR, seek_end = SEEK_END} ;

    bool open (const char *Name, u_int32 Flags = 0) ;
    bool close (void) ;
    pov_io_base& read (void *buffer, u_int32 count) ;
    pov_io_base& write (void *buffer, u_int32 count) ;
    pov_io_base& seekg (u_int32 pos, u_int32 whence = seek_set) ;

    inline u_int32 gettype (void) { return (filetype) ; }
    inline u_int32 getdirection (void) { return (direction) ; }
    inline bool eof (void) { return (fail ? true : feof (f) != 0) ; }
    inline u_int32 tellg (void) { return (f == NULL ? -1 : ftell (f)) ; }
    inline pov_io_base& clearstate (void) { if (f != NULL) fail = false ; return (*this) ; }
    inline const char *Name (void) { return (filename) ; }

    inline operator void *() const { return (fail ? 0 : (void *) this) ; }
    inline bool operator!() const { return (fail) ; }

  protected:
    bool fail ;
    FILE *f ;
    pov_io_base& flush (void) ;
    u_int32 filetype ;
    u_int32 direction ;
    char *filename ;
} ;

class pov_istream_class : public pov_io_base
{
  public:
    pov_istream_class (const u_int32 Type) ;
    virtual ~pov_istream_class () ;

    inline int32 Read_Byte (void) { return (fail ? EOF : fgetc (f)) ; }
    int32 Read_Short (void) ;
    int32 Read_Long (void) ;
    inline pov_istream_class& Read_Byte (char& c) { c = (char) Read_Byte () ; return (*this) ; }
    inline pov_istream_class& Read_Byte (unsigned char& c) { c = (unsigned char) Read_Byte () ; return (*this) ; }
    inline pov_istream_class& Read_Short (short& n) { n = (short) Read_Short () ; return (*this) ; }
    inline pov_istream_class& Read_Short (unsigned short& n) { n = (unsigned short) Read_Short () ; return (*this) ; }
    inline pov_istream_class& Read_Long (long& n) { n = Read_Long () ; return (*this) ; }
    inline pov_istream_class& Read_Long (unsigned long& n) { n = Read_Long () ; return (*this) ; }

    inline pov_istream_class& operator>>(long& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(int& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(short& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(char& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(unsigned long& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(unsigned int& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(unsigned short& n) { read (&n, sizeof (n)) ; return (*this) ; }
    inline pov_istream_class& operator>>(unsigned char& n) { read (&n, sizeof (n)) ; return (*this) ; }
    pov_istream_class& UnRead_Byte (int32 c) ;
    pov_istream_class& getline (char *s, u_int32 buflen) ;
    pov_istream_class& ignore (u_int32 count) { seekg (count, seek_cur) ; return (*this) ; }
} ;

class pov_ostream_class : public pov_io_base
{
  public:
    pov_ostream_class (const u_int32 Type) ;
    virtual ~pov_ostream_class() ;

    void printf (char *format, ...) ;

    inline pov_ostream_class& Write_Byte (byte data) { if (!fail) fail = fputc (data, f) != data ; return (*this) ; }
    inline pov_ostream_class& Write_Short (u_int16 data) { write (&data, sizeof (data)) ; return (*this) ; }
    inline pov_ostream_class& Write_Int (u_int32 data) { write (&data, sizeof (data)) ; return (*this) ; }
    inline pov_ostream_class& Write_Long (u_int32 data) { write (&data, sizeof (data)) ; return (*this) ; }
    inline pov_ostream_class& flush (void) { pov_io_base::flush () ; return (*this) ; }

    inline pov_ostream_class& operator<<(char *s) { write (s, strlen ((char *) s)) ; return *this ; }
    inline pov_ostream_class& operator<<(unsigned char *s) { return operator<<((char *) s) ; }
    inline pov_ostream_class& operator<<(char c) { return (Write_Byte (c)) ; }
    inline pov_ostream_class& operator<<(unsigned char c) { return operator <<((char) c) ; }
    inline pov_ostream_class& operator<<(short n) { return (Write_Short (n)) ; }
    inline pov_ostream_class& operator<<(unsigned short n) { return operator <<((short) n) ; }
    inline pov_ostream_class& operator<<(int n) { return (Write_Int (n)) ; }
    inline pov_ostream_class& operator<<(unsigned int n) { return operator <<((int) n) ; }
    inline pov_ostream_class& operator<<(long n) { return (Write_Long (n)) ; }
    inline pov_ostream_class& operator<<(unsigned long n) { return operator <<((long) n) ; }
} ;

//typedef pov_istream_class POV_ISTREAM ;
//typedef pov_ostream_class POV_OSTREAM ;

pov_istream_class *POV_New_IStream (const char *sname, const u_int32 stype) ;
pov_ostream_class *POV_New_OStream (const char *sname, const u_int32 stype, const bool sappend) ;

// --------------------------------------------------------------------------------

char *Locate_Filename (char *filename, u_int32 stype, int err_flag) ;
POV_ISTREAM *Locate_File (char *filename, u_int32 stype, char *buffer, int err_flag) ;
int Has_Extension (char *name) ;
void POV_Split_Path (char *s, char *p, char *f) ;
bool POV_File_Exist (char *name) ;

#endif
