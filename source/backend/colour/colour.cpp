/*******************************************************************************
 * colour.cpp
 *
 * This module implements routines to manipulate colours.
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
 * $File: //depot/povray/smp/source/backend/colour/colour.cpp $
 * $Revision: #15 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/colour/colour.h"
#include "backend/texture/texture.h"
#include "backend/texture/pigment.h"
#include "backend/texture/normal.h"
#include "backend/math/vector.h"

#include <algorithm>

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

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
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

COLOUR *Create_Colour ()
{
	COLOUR *New;

	New = (COLOUR *)POV_MALLOC(sizeof (COLOUR), "color");

	Make_ColourA (*New, 0.0, 0.0, 0.0, 0.0, 0.0);

	return (New);
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
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

COLOUR *Copy_Colour (const COLOUR Old)
{
	COLOUR *New;

	if (Old != NULL)
	{
		New = Create_Colour ();

		Assign_Colour(*New,Old);
	}
	else
	{
		New = NULL;
	}

	return (New);
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
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Aug 1995 : Use POV_CALLOC to initialize entries. [DB]
*
******************************************************************************/

BLEND_MAP_ENTRY *Create_BMap_Entries (int Map_Size)
{
	BLEND_MAP_ENTRY *New;

	New = (BLEND_MAP_ENTRY *)POV_CALLOC(Map_Size, sizeof (BLEND_MAP_ENTRY), "blend map entry");

	return (New);
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
*   POV-Ray Team
*   
* DESCRIPTION
*
*
* CHANGES
*
******************************************************************************/

BLEND_MAP_ENTRY *Copy_BMap_Entries (const BLEND_MAP_ENTRY *Old, int Map_Size, int Type)
{
	int i;
	BLEND_MAP_ENTRY *New;

	if (Old != NULL)
	{
		New = Create_BMap_Entries (Map_Size);

		for (i = 0; i < Map_Size; i++)
		{
			switch (Type)
			{
				case PIGMENT_TYPE:

					New[i].Vals.Pigment = Copy_Pigment(Old[i].Vals.Pigment);

					break;

				case NORMAL_TYPE:

					New[i].Vals.Tnormal = Copy_Tnormal(Old[i].Vals.Tnormal);

					break;

				case TEXTURE_TYPE:

					New[i].Vals.Texture = Copy_Textures(Old[i].Vals.Texture);

					break;

				case COLOUR_TYPE:
				case SLOPE_TYPE:

					New[i] = Old[i];

					break;
			}
		}
	}
	else
	{
		New = NULL;
	}

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Blend_Map
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

BLEND_MAP *Create_Blend_Map ()
{
	BLEND_MAP *New;

	New = (BLEND_MAP *)POV_MALLOC(sizeof (BLEND_MAP), "blend map");

	New->Users = 1;

	New->Number_Of_Entries = 0;

	New->Type = COLOUR_TYPE;

	New->Blend_Map_Entries = NULL;

	New->Transparency_Flag = false;

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Blend_Map
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

BLEND_MAP *Copy_Blend_Map (BLEND_MAP *Old)
{
	BLEND_MAP *New;

	New = Old;

	/* 
	 * Do not increase the users field if it is negative.
	 *
	 * A negative users field incicates a reference to a static
	 * or global memory area in the data segment, not on the heap!
	 * Thus it must not be deleted later.
	 */

	if ((New != NULL) && (New->Users >= 0))
	{
		New->Users++;
	}

	return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Colour_Distance_RGBT
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

DBL Colour_Distance_RGBT (const Colour& colour1, const Colour& colour2)
{
	return (fabs(colour1[pRED]    - colour2[pRED]) +
	        fabs(colour1[pGREEN]  - colour2[pGREEN]) +
	        fabs(colour1[pBLUE]   - colour2[pBLUE]) +
	        fabs(colour1[pTRANSM] - colour2[pTRANSM]));
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Blend_Map
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
*   -
*
* CHANGES
*
*   -
*
******************************************************************************/

void Destroy_Blend_Map (BLEND_MAP *BMap)
{
	int i;
	
	if (BMap != NULL)
	{
		if ((BMap->Users > 0) && (--(BMap->Users) == 0))
		{
			for (i = 0; i < BMap->Number_Of_Entries; i++)
			{
				switch (BMap->Type)
				{
					case PIGMENT_TYPE:
					case DENSITY_TYPE:
						Destroy_Pigment(BMap->Blend_Map_Entries[i].Vals.Pigment);
						break;

					case NORMAL_TYPE:
						Destroy_Tnormal(BMap->Blend_Map_Entries[i].Vals.Tnormal);
						break;

					case TEXTURE_TYPE:
						Destroy_Textures(BMap->Blend_Map_Entries[i].Vals.Texture);
				}
			}

			POV_FREE (BMap->Blend_Map_Entries);

			POV_FREE (BMap);
		}
	}
}

}
