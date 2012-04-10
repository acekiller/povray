/*******************************************************************************
 * lightgrp.cpp
 *
 * Implements light group utility functions.
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
 * $File: //depot/povray/smp/source/lightgrp.cpp $
 * $Revision: #20 $
 * $Change: 5095 $
 * $DateTime: 2010/08/07 07:51:37 $
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

#include "backend/scene/objects.h"
#include "backend/shape/csg.h"
#include "backend/lighting/point.h"

#include "lightgrp.h" // TODO

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

void Promote_Local_Lights_Recursive(CompoundObject *Object, vector<LightSource *>& Lights);


/*****************************************************************************
*
* FUNCTION
*
*   Promote_Local_Lights
*
* INPUT
*
*   Object - CSG union object
*
* OUTPUT
*
*   Modified CSG union object with local lights added to each object
*
* RETURNS
*   
* AUTHOR
*
*   Thorsten Froehlich [trf]
*   
* DESCRIPTION
*
*   Collects all light sources in CSG union object (only those who are direct
*   children of the object, not childrens children light sources - this was
*   taken care of before) and adds them to local light list of every object
*   in the CSG union.
*   NOTE: Only pointers are changed, the original light source is still the
*   one in the CSG union, and once this light source has been deallocated,
*   the pointers will be invalid.  Note that because of this we do not need
*   to (and we may not!) free the LLight lights of each object!
*
* CHANGES
*
*   Jun 2000 : Creation.
*
******************************************************************************/

void Promote_Local_Lights(CSG *Object)
{
	vector<LightSource *> lights;

	if(Object == NULL)
		return;

	// find all light sources in the light group and connect them to form a list
	int light_counter = 0;
	int object_counter = 0;
	for(vector<ObjectPtr>::iterator curObject = Object->children.begin();
	    curObject != Object->children.end();
	    curObject++)
	{
		if(((*curObject)->Type & LIGHT_GROUP_LIGHT_OBJECT) == LIGHT_GROUP_LIGHT_OBJECT)
		{
			lights.push_back((LightSource *)(*curObject));
			light_counter++;
		}
		else
			object_counter++;
	}

	// if no lights have been found in the light group we don't need to continue, but the
	// user should know there are no lights (also it will continue to work as a union)
	if(light_counter <= 0)
	{
;// TODO MESSAGE		Warning(0, "No light source(s) found in light group.");
		return;
	}
	// if no objects have been found nothing will happen at all (the light group is only wasting memory)
	if(object_counter <= 0)
	{
;// TODO MESSAGE		Warning(0, "No object(s) found in light group.");
		return;
	}

	// allow easy promotion of lights (if this is part of another light group)
	Object->LLights = lights;

	// promote light recursively to all other objects in the CSG union
	Promote_Local_Lights_Recursive((CompoundObject *)Object, lights);
}

/*****************************************************************************
*
* FUNCTION
*
*   Promote_Local_Lights_Recursive
*
* INPUT
*
*   Object - compound object
*   Lights - local lights to add to children objects
*
* OUTPUT
*
*   Modified compound object with local lights added to each object
*
* RETURNS
*   
* AUTHOR
*
*   Thorsten Froehlich [trf]
*   
* DESCRIPTION
*
*   Adds input list of light sources to local light list of every object in
*   the compound object, recursively if there are other compound objects.
*   NOTE: Only pointers are changed and because of this we do not need to
*   (and we may not!) free the LLight lights of each object!
*
* CHANGES
*
*   Jun 2000 : Creation.
*
******************************************************************************/

void Promote_Local_Lights_Recursive(CompoundObject *Object, vector<LightSource *>& Lights)
{
	ObjectPtr curObject = NULL;

	for(vector<ObjectPtr>::iterator curObject = Object->children.begin();
	    curObject != Object->children.end();
	    curObject++)
	{
		if(!(*curObject)->LLights.empty())
		{
			for(vector<LightSource *>::iterator i = Lights.begin(); i != Lights.end(); i++)
				(*curObject)->LLights.push_back(*i);
		}
		else if(((*curObject)->Type & IS_COMPOUND_OBJECT) == IS_COMPOUND_OBJECT)
		{
			// allow easy promotion of lights (if this is part of another light group)
			(*curObject)->LLights = Lights;

			Promote_Local_Lights_Recursive((CompoundObject *)(*curObject), Lights);
		}
		else
		{
			(*curObject)->LLights = Lights;
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Check_Photon_Light_Group
*
* INPUT
*
*   Object - any object
*
* OUTPUT
*
* RETURNS
*
*   True if this object is lit by the photon light (according to the light_group rules)
*
*
* AUTHOR
*
*   Nathan Kopp [NK]
*   
* DESCRIPTION
*
*   If the photon light is a global light (not in a light group) as determined by
*   the photonOptions object, then we just check to see if the object interacts
*   with global lights.
*
*   Otherwise...
*
*   Checks to see if Light is one of Object's local lights (part of the light
*   group).
*   
* CHANGES
*
*   Apr 2002 : Creation.
*
******************************************************************************/

bool Check_Photon_Light_Group(ObjectPtr Object)
{
/* TODO FIXME	if(photonOptions.Light_Is_Global)
	{
		if((Object->Flags & NO_GLOBAL_LIGHTS_FLAG) == NO_GLOBAL_LIGHTS_FLAG)
			return false;
		else
			return true;
	}
	else
	{
		for(vector<LightSource *>::iterator Test_Light = Object->LLights.begin(); Test_Light != Object->LLights.end(); Test_Light++)
		{
			if(*Test_Light == photonOptions.Light)
				return true;
		} */
		return true;
// 	}
}

}
