/*******************************************************************************
 * povrayold.h
 *
 * Obsolete data structures that have not been cleaned up yet!
 * Do not add anything new to this file.  Clean up stuff you need
 * as soon as possible.
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
 * $File: //depot/povray/smp/source/povrayold.h $
 * $Revision: #31 $
 * $Change: 5428 $
 * $DateTime: 2011/03/20 07:34:04 $
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


#ifndef POVRAY_H
#define POVRAY_H

#include <time.h>
#include <string>

#include "backend/frame.h"

namespace pov
{

using namespace pov_base;

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define STAGE_PREINIT         0  // set in POVRAY.C
#define STAGE_STARTUP         1  // set in POVRAY.C
#define STAGE_BANNER          2  // set in POVRAY.C
#define STAGE_INIT            3  // set in POVRAY.C
#define STAGE_FILE_INIT       4  // set in POVRAY.C
#define STAGE_PARSING         5  // set in PARSE.C
#define STAGE_CONTINUING      6  // set in POVRAY.C
#define STAGE_RENDERING       7  // set in POVRAY.C
#define STAGE_SHUTDOWN        8  // set in POVRAY.C
#define STAGE_CLEANUP_PARSE   9  // set in PARSE.C
#define STAGE_SLAB_BUILDING  10  // set in POVRAY.C
#define STAGE_TOKEN_INIT     11  // set in TOKENIZE.C
#define STAGE_INCLUDE_ERR    12  // set in TOKENIZE.C
#define STAGE_FOUND_INSTEAD  13  // set in TOKENIZE.C
#define STAGECOUNT           14  // number of stages

//#define DISPLAY           0x000001L
//#define VERBOSE           0x000002L
//#define DISKWRITE         0x000004L
//#define PROMPTEXIT        0x000008L
//#define ANTIALIAS         0x000010L
//#define RGBSEPARATE       0x000020L
//#define EXITENABLE        0x000040L
//#define CONTINUE_TRACE    0x000080L
#define JITTER            0x000100L
//#define PREVIEW           0x000200L
#define SPLIT_UNION       0x000400L
#define USE_VISTA_BUFFER  0x000800L
#define USE_LIGHT_BUFFER  0x001000L
#define USE_VISTA_DRAW    0x002000L
#define REMOVE_BOUNDS     0x004000L
//#define CYCLIC_ANIMATION  0x008000L
//#define OUTPUT_ALPHA      0x010000L
//#define HF_GRAY_16        0x020000L
//#define GAMMA_CORRECT     0x040000L
//#define FROM_STDIN        0x080000L
//#define TO_STDOUT         0x100000L

#define Q_FULL_AMBIENT    0x000001L
#define Q_QUICKC          0x000002L
#define Q_SHADOW          0x000004L
#define Q_AREA_LIGHT      0x000008L
#define Q_REFRACT         0x000010L
#define Q_REFLECT         0x000020L
#define Q_NORMAL          0x000040L
#define Q_VOLUME          0x000080L
#define Q_ADVANCED_LIGHT  0x000100L
#define Q_SUBSURFACE      0x000200L

#define EF_SSLT    1
#define EF_SLOPEM  2
#define EF_ISOFN   4
#define EF_SPLINE  8
#define EF_TIFF    16
#define EF_BACKILL 32
#define EF_MESHCAM 64

#define BF_VIDCAP  1
#define BF_RTR     2

#define QUALITY_0  Q_QUICKC+Q_FULL_AMBIENT
#define QUALITY_1  QUALITY_0
#define QUALITY_2  QUALITY_1-Q_FULL_AMBIENT
#define QUALITY_3  QUALITY_2
#define QUALITY_4  QUALITY_3+Q_SHADOW
#define QUALITY_5  QUALITY_4+Q_AREA_LIGHT
#define QUALITY_6  QUALITY_5-Q_QUICKC+Q_REFRACT
#define QUALITY_7  QUALITY_6
#define QUALITY_8  QUALITY_7+Q_REFLECT+Q_NORMAL
#define QUALITY_9  QUALITY_8+Q_VOLUME+Q_ADVANCED_LIGHT+Q_SUBSURFACE


/*****************************************************************************
* Global typedefs
******************************************************************************/

struct RadiositySettings
{
	int Quality;  // Q-flag value for light gathering

	int File_ReadOnContinue;
	int File_SaveWhileRendering;
	int File_AlwaysReadAtStart;
	int File_KeepOnAbort;
	int File_KeepAlways;
	char *Load_File_Name; //[FILE_NAME_LENGTH];
	char *Save_File_Name; //[FILE_NAME_LENGTH];
};

struct QualitySettings
{
	int Quality;
	unsigned int Quality_Flags;
	int Tracing_Method;
	int AntialiasDepth;
	DBL Antialias_Threshold;
	DBL JitterScale;
};

typedef struct OPTIONS_STRUCT
{
	unsigned int Options;

	int Abort_Test_Counter;

	char *Ini_Output_File_Name;

	char *Header_File_Name;
} Opts;

}

/*****************************************************************************
* Global functions
******************************************************************************/

namespace pov
{

void Do_Cooperate(int level);

}

#endif
