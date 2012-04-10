/*******************************************************************************
 * texture.h
 *
 * This file contains defines and variables for the txt*.cpp files
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
 * $File: //depot/povray/smp/source/backend/texture/texture.h $
 * $Revision: #18 $
 * $Change: 5597 $
 * $DateTime: 2011/12/24 21:06:16 $
 * $Author: chrisc $
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

/* NOTE: FRAME.H contains other texture stuff. */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "backend/pattern/pattern.h"
#include "backend/pattern/warps.h"

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/*
 * Macro to create random number in the [0; 1] range.
 */

#define FLOOR(x)  ((x) >= 0.0 ? floor(x) : (0.0 - floor(0.0 - (x)) - 1.0))

#define Hash3d(a,b,c) \
	hashTable[(int)(hashTable[(int)(hashTable[(int)((a) & 0xfff)] ^ ((b) & 0xfff))] ^ ((c) & 0xfff))]



/*****************************************************************************
* Global typedefs
******************************************************************************/

/*****************************************************************************
* Global variables
******************************************************************************/

#ifdef DYNAMIC_HASHTABLE
extern unsigned short *hashTable;
#else
extern ALIGN16 unsigned short hashTable[8192];
#endif


/*****************************************************************************
* Global functions
******************************************************************************/

void Initialize_Noise (void);
void Initialize_Waves(vector<double>& waveFrequencies, vector<Vector3d>& waveSources, unsigned int numberOfWaves);
void Free_Noise_Tables (void);
#if defined(USE_AVX_FMA4_FOR_NOISE)
extern DBL (*Noise) (const VECTOR EPoint, int noise_generator);
extern void (*DNoise) (VECTOR result, const VECTOR EPoint);
void Initialise_NoiseDispatch();
DBL AVX_FMA4_Noise(const VECTOR EPoint, int noise_generator);
void AVX_FMA4_DNoise(VECTOR result, const VECTOR EPoint);

#else
INLINE_NOISE DBL Noise (const VECTOR EPoint, int noise_generator);
INLINE_NOISE void DNoise (VECTOR result, const VECTOR EPoint);
#endif
DBL Turbulence (const VECTOR EPoint, const TURB *Turb, int noise_generator);
void DTurbulence (VECTOR result, const VECTOR EPoint, const TURB *Turb);
DBL cycloidal (DBL value);
DBL Triangle_Wave (DBL value);
void Transform_Textures (TEXTURE *Textures, const TRANSFORM *Trans);
void Destroy_Textures (TEXTURE *Textures);
void Post_Textures (TEXTURE *Textures);
FINISH *Create_Finish (void);
FINISH *Copy_Finish (const FINISH *Old);
TEXTURE *Copy_Texture_Pointer (TEXTURE *Texture);
TEXTURE *Copy_Textures (const TEXTURE *Textures);
TEXTURE *Create_Texture (void);
int Test_Opacity (TEXTURE *Texture);
TURB *Create_Turb (void);

}

#endif
