/*******************************************************************************
 * fileinputoutput.h
 *
 * This module contains all defines, typedefs, and prototypes for fileinputoutput.cpp.
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
 * $File: //depot/povray/smp/source/base/fileinputoutput.h $
 * $Revision: #31 $
 * $Change: 5094 $
 * $DateTime: 2010/08/07 06:03:14 $
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

#ifndef FILEINPUTOUTPUT_H
#define FILEINPUTOUTPUT_H

#include "base/types.h"
#include "base/path.h"

#include <string>
#include <cstring>

namespace pov_base
{

#ifndef POV_SEEK_SET
	#define POV_SEEK_SET IOBase::seek_set
#endif

#ifndef POV_SEEK_CUR
	#define POV_SEEK_CUR IOBase::seek_cur
#endif

#ifndef POV_SEEK_END
	#define POV_SEEK_END IOBase::seek_end
#endif

// NOTE: Remember to modify gPOV_File_Extensions in fileinputoutput.cpp!
enum
{
	POV_File_Unknown = 0,
	POV_File_Image_Targa,
	POV_File_Image_PNG,
	POV_File_Image_PPM,
	POV_File_Image_PGM,
	POV_File_Image_GIF,
	POV_File_Image_IFF,
	POV_File_Image_JPEG,
	POV_File_Image_TIFF,
	POV_File_Image_BMP,
	POV_File_Image_EXR,
	POV_File_Image_HDR,
	POV_File_Image_System,
	POV_File_Text_POV,
	POV_File_Text_INC,
	POV_File_Text_Macro = POV_File_Text_INC,
	POV_File_Text_INI,
	POV_File_Text_CSV,
	POV_File_Text_Stream,
	POV_File_Text_User,
	POV_File_Data_DF3,
	POV_File_Data_RCA,
	POV_File_Data_LOG,
	POV_File_Data_Backup,
	POV_File_Font_TTF,
	POV_File_Count
};

#define POV_FILE_EXTENSIONS_PER_TYPE 4
typedef struct
{
	char *ext[POV_FILE_EXTENSIONS_PER_TYPE];
} POV_File_Extensions;

class IOBase
{
	public:
		IOBase(unsigned int dir, unsigned int type);
		virtual ~IOBase();

		enum { none = 0, append = 1 };
		enum { input, output, io };
		enum { seek_set = SEEK_SET, seek_cur = SEEK_CUR, seek_end = SEEK_END };

		virtual bool open(const UCS2String& name, unsigned int Flags = 0);
		virtual bool close();
		IOBase& read(void *buffer, size_t count);
		IOBase& write(void *buffer, size_t count);
		IOBase& seekg(POV_LONG pos, unsigned int whence = seek_set);

		inline unsigned int gettype() { return(filetype); }
		inline unsigned int getdirection() { return(direction); }
		inline bool eof() { return(fail ? true : feof(f) != 0); }
		inline POV_LONG tellg() { return(f == NULL ? -1 : ftell(f)); }
		inline IOBase& clearstate() { if(f != NULL) fail = false; return *this; }
		inline const UCS2 *Name() { return(filename.c_str()); }

		inline operator void *() const { return(fail ? 0 :(void *) this); }
		inline bool operator!() const { return(fail); }
	protected:
		bool fail;
		FILE *f;
		IOBase& flush();
		unsigned int filetype;
		unsigned int direction;
		UCS2String filename;
};

class IStream : public IOBase
{
	public:
		IStream(const unsigned int Type);
		virtual ~IStream();

		inline int Read_Byte() { return(fail ? EOF : fgetc(f)); }
		int Read_Short();
		int Read_Int();
		inline IStream& Read_Byte(char& c) { c =(char) Read_Byte(); return *this; }
		inline IStream& Read_Byte(unsigned char& c) { c =(unsigned char) Read_Byte(); return *this; }
		inline IStream& Read_Short(short& n) { n =(short) Read_Short(); return *this; }
		inline IStream& Read_Short(unsigned short& n) { n =(unsigned short) Read_Short(); return *this; }
		inline IStream& Read_Int(int& n) { n = Read_Int(); return *this; }
		inline IStream& Read_Int(unsigned int& n) { n = Read_Int(); return *this; }

		inline IStream& operator>>(int& n) { read(&n, sizeof(n)); return *this; }
		inline IStream& operator>>(short& n) { read(&n, sizeof(n)); return *this; }
		inline IStream& operator>>(char& n) { read(&n, sizeof(n)); return *this; }
		inline IStream& operator>>(unsigned int& n) { read(&n, sizeof(n)); return *this; }
		inline IStream& operator>>(unsigned short& n) { read(&n, sizeof(n)); return *this; }
		inline IStream& operator>>(unsigned char& n) { read(&n, sizeof(n)); return *this; }
		IStream& UnRead_Byte(int c);
		IStream& getline(char *s, size_t buflen);
		IStream& ignore(POV_LONG count) { seekg(count, seek_cur); return *this; }
};

class OStream : public IOBase
{
	public:
		OStream(const unsigned int Type);
		virtual ~OStream();

		void printf(const char *format, ...);

		inline OStream& Write_Byte(unsigned char data) { if(!fail) fail = fputc(data, f) != data; return *this; }
		inline OStream& Write_Short(unsigned short data) { write(&data, sizeof(data)); return *this; }
		inline OStream& Write_Int(unsigned int data) { write(&data, sizeof(data)); return *this; }
		inline OStream& flush(void) { IOBase::flush(); return *this; }

		inline OStream& operator<<(const char *s) { write((void *)s, (size_t) strlen(s)); return *this; }
		inline OStream& operator<<(unsigned char *s) { return operator<<((char *) s); }
		inline OStream& operator<<(char c) { return(Write_Byte(c)); }
		inline OStream& operator<<(unsigned char c) { return operator <<((char) c); }
		inline OStream& operator<<(short n) { return(Write_Short(n)); }
		inline OStream& operator<<(unsigned short n) { return operator <<((short) n); }
		inline OStream& operator<<(int n) { return(Write_Int(n)); }
		inline OStream& operator<<(unsigned int n) { return operator <<((int) n); }
};

IStream *NewIStream(const Path&, const unsigned int);
OStream *NewOStream(const Path&, const unsigned int, const bool);

UCS2String GetFileExtension(const Path& p);
UCS2String GetFileName(const Path& p);

bool CheckIfFileExists(const Path& p);
POV_LONG GetFileLength(const Path& p);

}

namespace pov
{
	int InferFileTypeFromExt(const pov_base::UCS2String& ext); // TODO FIXME - belongs in backend

	extern pov_base::POV_File_Extensions gPOV_File_Extensions[]; // TODO FIXME - belongs in backend
	extern const int gFile_Type_To_Mask[]; // TODO FIXME - belongs in backend
}

#endif
