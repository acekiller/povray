/*******************************************************************************
 * atmosph.h
 *
 * This module contains all defines, typedefs, and prototypes for ATMOSPH.CPP.
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
 * $File: //depot/povray/smp/source/backend/scene/atmosph.h $
 * $Revision: #17 $
 * $Change: 5573 $
 * $DateTime: 2011/12/02 21:56:12 $
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


#ifndef ATMOSPH_H
#define ATMOSPH_H

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Define fog types. DMF */

#define ORIG_FOG    1
#define GROUND_MIST 2
#define FOG_TYPES   2

/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct Fog_Struct FOG;
typedef struct Rainbow_Struct RAINBOW;
typedef struct Skysphere_Struct SKYSPHERE;

struct Fog_Struct
{
	int Type;
	DBL Distance;
	DBL Alt;
	DBL Offset;
	Colour colour; // may have a filter/transmit component
	VECTOR Up;
	TURB *Turb;
	SNGL Turb_Depth;
	FOG *Next;
};

struct Rainbow_Struct
{
	DBL Distance;
	DBL Jitter;
	DBL Angle, Width;
	DBL Arc_Angle, Falloff_Angle, Falloff_Width;
	VECTOR Antisolar_Vector;
	VECTOR Up_Vector, Right_Vector;
	PIGMENT *Pigment;
	RAINBOW *Next;
};

struct Skysphere_Struct
{
	int Count;           ///< Number of pigments.
	RGBColour Emission;  ///< Brightness adjustment.
	PIGMENT **Pigments;  ///< Pigment(s) to use.
	TRANSFORM *Trans;    ///< Skysphere transformation.
};

/*****************************************************************************
* Global functions
******************************************************************************/

FOG *Create_Fog (void);
FOG *Copy_Fog (FOG *Fog);
void Destroy_Fog (FOG *Fog);

RAINBOW *Create_Rainbow (void);
RAINBOW *Copy_Rainbow (RAINBOW *Rainbow);
void Destroy_Rainbow (RAINBOW *Rainbow);

SKYSPHERE *Create_Skysphere (void);
SKYSPHERE *Copy_Skysphere (SKYSPHERE *Skysphere);
void Destroy_Skysphere (SKYSPHERE *Skysphere);
void Scale_Skysphere (SKYSPHERE *Skysphere, VECTOR Vector);
void Rotate_Skysphere (SKYSPHERE *Skysphere, VECTOR Vector);
void Translate_Skysphere (SKYSPHERE *Skysphere, VECTOR Vector);
void Transform_Skysphere (SKYSPHERE *Skysphere, TRANSFORM *Trans);

}

#endif
