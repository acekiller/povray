/*******************************************************************************
 * fileutil.cpp
 *
 * This module implements misc utility functions.
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
 * $File: //depot/povray/smp/source/backend/support/fileutil.cpp $
 * $Revision: #22 $
 * $Change: 5088 $
 * $DateTime: 2010/08/05 17:08:44 $
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

// TODO FIXME - fileutil.cpp code is needed by front- abd backend alike, so should be in "base" directory

#include <ctype.h>
#include <stdarg.h>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/support/fileutil.h"
#include "backend/parser/parse.h" // TODO FIXME HACK - used for hack below, need to remove [trf]

#include "povrayold.h" // TODO

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*
// not used right now
typedef struct
{
	bool read_local;
	bool read_global;
	bool write_local;
	bool write_global;
} POV_File_Restrictions;

POV_File_Restrictions gPOV_File_Restrictions[POV_File_Count] =
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

POV_File_Extensions gPOV_File_Extensions[POV_File_Count] = // TODO FIXME - belongs in backend
{
	{{ "",      "",      "",      ""      }}, // POV_File_Unknown
	{{ ".tga",  ".TGA",  "",      ""      }}, // POV_File_Image_Targa
	{{ ".png",  ".PNG",  "",      ""      }}, // POV_File_Image_PNG
	{{ ".ppm",  ".PPM",  "",      ""      }}, // POV_File_Image_PPM
	{{ ".pgm",  ".PGM",  "",      ""      }}, // POV_File_Image_PGM
	{{ ".gif",  ".GIF",  "",      ""      }}, // POV_File_Image_GIF
	{{ ".iff",  ".IFF",  "",      ""      }}, // POV_File_Image_IFF
	{{ ".jpg",  ".JPG",  ".jpeg", ".JPEG" }}, // POV_File_Image_JPEG
	{{ ".tif",  ".TIF",  ".tiff", ".TIFF" }}, // POV_File_Image_TIFF
	{{ ".bmp",  ".BMP",  "",      ""      }}, // POV_File_Image_BMP
	{{ ".exr",  ".EXR",  "",      ""      }}, // POV_File_Image_EXR
	{{ ".hdr",  ".HDR",  "",      ""      }}, // POV_File_Image_HDR
	{{ POV_IS1, POV_IS2, POV_IS3, POV_IS4 }}, // POV_File_Image_System
	{{ ".pov",  ".POV",  "",      ""      }}, // POV_File_Text_POV
	{{ ".inc",  ".INC",  "",      ""      }}, // POV_File_Text_INC
	{{ ".ini",  ".INI",  "",      ""      }}, // POV_File_Text_INI
	{{ ".csv",  ".CSV",  "",      ""      }}, // POV_File_Text_CSV
	{{ ".txt",  ".TXT",  "",      ""      }}, // POV_File_Text_Stream
	{{ "",      "",      "",      ""      }}, // POV_File_Text_User
	{{ ".df3",  ".DF3",  "",      ""      }}, // POV_File_Data_DF3
	{{ ".rca",  ".RCA",  "",      ""      }}, // POV_File_Data_RCA
	{{ ".log",  ".LOG",  "",      ""      }}, // POV_File_Data_LOG
	{{ ".bak",  ".BAK",  "",      ""      }}, // POV_File_Data_Backup
	{{ ".ttf",  ".TTF",  "",      ""      }}  // POV_File_Font_TTF
};

const int gFile_Type_To_Mask [POV_File_Count] = // TODO FIXME - belongs in backend
{
	NO_FILE,   // POV_File_Unknown
	TGA_FILE,  // POV_File_Image_Targa
	PNG_FILE,  // POV_File_Image_PNG
	PPM_FILE,  // POV_File_Image_PPM
	PGM_FILE,  // POV_File_Image_PGM
	GIF_FILE,  // POV_File_Image_GIF
	IFF_FILE,  // POV_File_Image_IFF
	JPEG_FILE, // POV_File_Image_JPEG
	TIFF_FILE, // POV_File_Image_TIFF
	BMP_FILE,  // POV_File_Image_BMP
	EXR_FILE,  // POV_File_Image_EXR
	HDR_FILE,  // POV_File_Image_HDR
	SYS_FILE,  // POV_File_Image_System
	NO_FILE,   // POV_File_Text_POV
	NO_FILE,   // POV_File_Text_INC
	NO_FILE,   // POV_File_Text_INI
	NO_FILE,   // POV_File_Text_CSV
	NO_FILE,   // POV_File_Text_Stream
	NO_FILE,   // POV_File_Text_User
	NO_FILE,   // POV_File_Data_DF3
	NO_FILE,   // POV_File_Data_RCA
	NO_FILE,   // POV_File_Data_LOG
	NO_FILE,   // POV_File_Data_Backup
	NO_FILE    // POV_File_Font_TTF
};

int InferFileTypeFromExt(const UCS2String& ext) // TODO FIXME - belongs in backend
{
	// TODO FIXME - better compare in the string domain

	string str = UCS2toASCIIString(ext);

	for (int i = 0; i < POV_File_Count; i++)
	{
		// TODO - checking whether extension matches is required at other places, too
		for (int j = 0; j < POV_FILE_EXTENSIONS_PER_TYPE; j ++)
		{
			if ( (strlen(gPOV_File_Extensions[i].ext[j]) > 0) &&
			     (pov_stricmp (gPOV_File_Extensions[i].ext[j], str.c_str()) == 0) )
			{
				return (i);
			}
		}
	}
	return NO_FILE;
}

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

IStream *Locate_File(Parser *p, shared_ptr<SceneData>& sd, const UCS2String& filename, unsigned int stype, UCS2String& buffer, bool err_flag)
{
	UCS2String fn(filename);
	UCS2String foundfile(sd->FindFile(p->GetPOVMSContext(), fn, stype));

	if(foundfile.empty() == true)
	{
		if(err_flag == true)
			p->PossibleError("Cannot find file '%s', even after trying to append file type extension.", UCS2toASCIIString(fn).c_str());

		return NULL;
	}

	if(fn.find('.') == UCS2String::npos)
	{
		// the passed-in filename didn't have an extension, but a file has been found,
		// which means one of the appended extensions worked. we need to work out which
		// one and append it to the original filename so we can store it in the cache
		// (since it's that name that the cache search routine looks for).
		UCS2String ext = GetFileExtension(Path(foundfile));
		if (ext.size() != 0)
			fn += ext;
	}

	// ReadFile will store both fn and foundfile in the cache for next time round
	IStream *result(sd->ReadFile(p->GetPOVMSContext(), fn, foundfile.c_str(), stype));

	if((result == NULL) && (err_flag == true))
		p->PossibleError("Cannot open file '%s'.", UCS2toASCIIString(foundfile).c_str());

	buffer = foundfile;

	return result;
}
/* TODO FIXME - code above should not be there, this is how it should work but this has bugs [trf]
IStream *Locate_File(Parser *p, shared_ptr<SceneData>& sd, const UCS2String& filename, unsigned int stype, UCS2String& buffer, bool err_flag)
{
	UCS2String foundfile(sd->FindFile(p->GetPOVMSContext(), filename, stype));

	if(foundfile.empty() == true)
	{
		if(err_flag == true)
			p->PossibleError("Cannot find file '%s', even after trying to append file type extension.", UCS2toASCIIString(filename).c_str());

		return NULL;
	}

	IStream *result(sd->ReadFile(p->GetPOVMSContext(), foundfile.c_str(), stype));

	if((result == NULL) && (err_flag == true))
		p->PossibleError("Cannot open file '%s'.", UCS2toASCIIString(foundfile).c_str());

	buffer = foundfile;

	return result;
}
*/
}
