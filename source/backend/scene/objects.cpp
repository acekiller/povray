/*******************************************************************************
 * objects.cpp
 *
 * This module implements the methods for objects and composite objects.
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
 * $File: //depot/povray/smp/source/backend/scene/objects.cpp $
 * $Revision: #51 $
 * $Change: 5410 $
 * $DateTime: 2011/02/24 16:46:18 $
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

#include <cassert>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/scene/objects.h"
#include "backend/texture/texture.h"
#include "backend/interior/interior.h"
#include "backend/math/matrices.h"
#include "backend/scene/threaddata.h"
#include "backend/shape/csg.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

template<int BX, int BY, int BZ>
FORCEINLINE bool Intersect_BBox_Dir(const BBOX& bbox, const BBOX_VECT& origin, const BBOX_VECT& invdir, BBOX_VAL mind, BBOX_VAL maxd);

/*****************************************************************************
 * ObjectDebugHelper class support code
 *****************************************************************************/

int ObjectDebugHelper::ObjectIndex = 0;

std::string& ObjectDebugHelper::SimpleDesc(std::string& result)
{
	char str[256];

	sprintf(str, "%u: ", Index);
	result = str;
	if(IsCopy)
		result += "Copy of ";
	if(Tag == "")
		result += "Unnamed object";
	else
		result += Tag;

	return result;
}

/*****************************************************************************
*
* FUNCTION
*
*   Find_Intersection
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

bool Find_Intersection(Intersection *isect, ObjectPtr object, const Ray& ray, TraceThreadData *threadData)
{
	if(object != NULL)
	{
		DBL closest = HUGE_VAL;
		BBOX_VECT origin;
		BBOX_VECT invdir;
		ObjectBase::BBoxDirection variant;

		Vector3d tmp(1.0 / ray.GetDirection()[X], 1.0 / ray.GetDirection()[Y], 1.0 /ray.GetDirection()[Z]);
		Assign_Vector(origin, ray.Origin);
		Assign_Vector(invdir, *tmp);
		variant = (ObjectBase::BBoxDirection)((int(invdir[X] < 0.0) << 2) | (int(invdir[Y] < 0.0) << 1) | int(invdir[Z] < 0.0));

		if(object->Intersect_BBox(variant, origin, invdir, closest) == false)
			return false;

		if(object->Bound.empty() == false)
		{
			if(Ray_In_Bound(ray, object->Bound, threadData) == false)
				return false;
		}

		IStack depthstack(threadData->stackPool);
		assert(depthstack->empty()); // verify that the IStack pulled from the pool is in a cleaned-up condition

		if(object->All_Intersections(ray, depthstack, threadData))
		{
			bool found = false;
			double tmpDepth = 0;

			while(depthstack->size() > 0)
			{
				tmpDepth = depthstack->top().Depth;
				// TODO FIXME - This was SMALL_TOLERANCE, but that's too rough for some scenes [cjc] need to check what it was in the old code [trf]
				if(tmpDepth < closest && (ray.IsSubsurfaceRay() || tmpDepth >= MIN_ISECT_DEPTH))
				{
					*isect = depthstack->top();
					closest = tmpDepth;
					found = true;
				}

				depthstack->pop();
			}

			return (found == true);
		}

		assert(depthstack->empty()); // verify that the IStack is in a cleaned-up condition (again)
	}

	return false;
}

bool Find_Intersection(Intersection *isect, ObjectPtr object, const Ray& ray, const RayObjectCondition& postcondition, TraceThreadData *threadData)
{
	if(object != NULL)
	{
		DBL closest = HUGE_VAL;
		BBOX_VECT origin;
		BBOX_VECT invdir;
		ObjectBase::BBoxDirection variant;

		Vector3d tmp(1.0 / ray.GetDirection()[X], 1.0 / ray.GetDirection()[Y], 1.0 /ray.GetDirection()[Z]);
		Assign_Vector(origin, ray.Origin);
		Assign_Vector(invdir, *tmp);
		variant = (ObjectBase::BBoxDirection)((int(invdir[X] < 0.0) << 2) | (int(invdir[Y] < 0.0) << 1) | int(invdir[Z] < 0.0));

		if(object->Intersect_BBox(variant, origin, invdir, closest) == false)
			return false;

		if(object->Bound.empty() == false)
		{
			if(Ray_In_Bound(ray, object->Bound, threadData) == false)
				return false;
		}

		IStack depthstack(threadData->stackPool);
		assert(depthstack->empty()); // verify that the IStack pulled from the pool is in a cleaned-up condition

		if(object->All_Intersections(ray, depthstack, threadData))
		{
			bool found = false;
			double tmpDepth = 0;

			while(depthstack->size() > 0)
			{
				tmpDepth = depthstack->top().Depth;
				// TODO FIXME - This was SMALL_TOLERANCE, but that's too rough for some scenes [cjc] need to check what it was in the old code [trf]
				if(tmpDepth < closest && (ray.IsSubsurfaceRay() || tmpDepth >= MIN_ISECT_DEPTH) && postcondition(ray, object, tmpDepth))
				{
					*isect = depthstack->top();
					closest = tmpDepth;
					found = true;
				}

				depthstack->pop();
			}

			return (found == true);
		}

		assert(depthstack->empty()); // verify that the IStack is in a cleaned-up condition (again)
	}

	return false;
}

bool Find_Intersection(Intersection *isect, ObjectPtr object, const Ray& ray, ObjectBase::BBoxDirection variant, const BBOX_VECT& origin, const BBOX_VECT& invdir, TraceThreadData *threadData)
{
	if(object != NULL)
	{
		DBL closest = HUGE_VAL;

		if(object->Intersect_BBox(variant, origin, invdir, closest) == false)
			return false;

		if(object->Bound.empty() == false)
		{
			if(Ray_In_Bound(ray, object->Bound, threadData) == false)
				return false;
		}

		IStack depthstack(threadData->stackPool);
		assert(depthstack->empty()); // verify that the IStack pulled from the pool is in a cleaned-up condition

		if(object->All_Intersections(ray, depthstack, threadData))
		{
			bool found = false;
			double tmpDepth = 0;

			while(depthstack->size() > 0)
			{
				tmpDepth = depthstack->top().Depth;
				// TODO FIXME - This was SMALL_TOLERANCE, but that's too rough for some scenes [cjc] need to check what it was in the old code [trf]
				if(tmpDepth < closest && (ray.IsSubsurfaceRay() || tmpDepth >= MIN_ISECT_DEPTH))
				{
					*isect = depthstack->top();
					closest = tmpDepth;
					found = true;
				}

				depthstack->pop();
			}

			return (found == true);
		}

		assert(depthstack->empty()); // verify that the IStack is in a cleaned-up condition (again)
	}

	return false;
}

bool Find_Intersection(Intersection *isect, ObjectPtr object, const Ray& ray, ObjectBase::BBoxDirection variant, const BBOX_VECT& origin, const BBOX_VECT& invdir, const RayObjectCondition& postcondition, TraceThreadData *threadData)
{
	if(object != NULL)
	{
		DBL closest = HUGE_VAL;

		if(object->Intersect_BBox(variant, origin, invdir, closest) == false)
			return false;

		if(object->Bound.empty() == false)
		{
			if(Ray_In_Bound(ray, object->Bound, threadData) == false)
				return false;
		}

		IStack depthstack(threadData->stackPool);
		assert(depthstack->empty()); // verify that the IStack pulled from the pool is in a cleaned-up condition

		if(object->All_Intersections(ray, depthstack, threadData))
		{
			bool found = false;
			double tmpDepth = 0;

			while(depthstack->size() > 0)
			{
				tmpDepth = depthstack->top().Depth;
				// TODO FIXME - This was SMALL_TOLERANCE, but that's too rough for some scenes [cjc] need to check what it was in the old code [trf]
				if(tmpDepth < closest && (ray.IsSubsurfaceRay() || tmpDepth >= MIN_ISECT_DEPTH) && postcondition(ray, object, tmpDepth))
				{
					*isect = depthstack->top();
					closest = tmpDepth;
					found = true;
				}

				depthstack->pop();
			}

			return (found == true);
		}

		assert(depthstack->empty()); // verify that the IStack is in a cleaned-up condition (again)
	}

	return false;
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Object
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

bool Inside_Object (const VECTOR IPoint, ObjectPtr Object, TraceThreadData *Thread)
{
	for (vector<ObjectPtr>::iterator Sib = Object->Clip.begin(); Sib != Object->Clip.end(); Sib++)
	{
		if(!Inside_Object(IPoint, *Sib, Thread))
			return false;
	}

	return (Object->Inside(IPoint, Thread));
}



/*****************************************************************************
*
* FUNCTION
*
*   Ray_In_Bound
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

bool Ray_In_Bound (const Ray& ray, const vector<ObjectPtr>& Bounding_Object, TraceThreadData *Thread)
{
	Intersection Local;

	for(vector<ObjectPtr>::const_iterator Bound = Bounding_Object.begin(); Bound != Bounding_Object.end(); Bound++)
	{
		Thread->Stats()[Bounding_Region_Tests]++;

		if((!Find_Intersection (&Local, *Bound, ray, Thread)) && (!Inside_Object(ray.Origin, *Bound, Thread)))
			return false;

		Thread->Stats()[Bounding_Region_Tests_Succeeded]++;
	}

	return true;
}



/*****************************************************************************
*
* FUNCTION
*
*   Point_In_Clip
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

bool Point_In_Clip (const VECTOR IPoint, const vector<ObjectPtr>& Clip, TraceThreadData *Thread)
{
	for(vector<ObjectPtr>::const_iterator Local_Clip = Clip.begin(); Local_Clip != Clip.end(); Local_Clip++)
	{
		Thread->Stats()[Clipping_Region_Tests]++;

		if(!Inside_Object(IPoint, *Local_Clip, Thread))
			return false;

		Thread->Stats()[Clipping_Region_Tests_Succeeded]++;
	}

	return true;
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Object
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

void Translate_Object (ObjectPtr Object, const VECTOR Vector, const TRANSFORM *Trans)
{
	if(Object == NULL)
		return;

	for(vector<ObjectPtr>::iterator Sib = Object->Bound.begin(); Sib != Object->Bound.end(); Sib++)
	{
		Translate_Object(*Sib, Vector, Trans);
	}

	if(Object->Clip != Object->Bound)
	{
		for(vector<ObjectPtr>::iterator Sib = Object->Clip.begin(); Sib != Object->Clip.end(); Sib++)
			Translate_Object(*Sib, Vector, Trans);
	}

	/* NK 1998 added if */
	if(!Test_Flag(Object, UV_FLAG))
	{
		Transform_Textures(Object->Texture, Trans);
		Transform_Textures(Object->Interior_Texture, Trans);
	}

	if(Object->UV_Trans == NULL)
		Object->UV_Trans = Create_Transform();
	Compose_Transforms(Object->UV_Trans, Trans);

	if(Object->interior != NULL)
		Object->interior->Transform(Trans);

	Object->Translate(Vector, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Object
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

void Rotate_Object (ObjectPtr Object, const VECTOR Vector, const TRANSFORM *Trans)
{
	if (Object == NULL)
		return;

	for(vector<ObjectPtr>::iterator Sib = Object->Bound.begin(); Sib != Object->Bound.end(); Sib++)
	{
		Rotate_Object(*Sib, Vector, Trans);
	}

	if (Object->Clip != Object->Bound)
	{
		for(vector<ObjectPtr>::iterator Sib = Object->Clip.begin(); Sib != Object->Clip.end(); Sib++)
		{
			Rotate_Object(*Sib, Vector, Trans);
		}
	}

	/* NK 1998 added if */
	if (!Test_Flag(Object, UV_FLAG))
	{
		Transform_Textures(Object->Texture, Trans);
		Transform_Textures(Object->Interior_Texture, Trans);
	}

	if (Object->UV_Trans == NULL)
		Object->UV_Trans = Create_Transform();
	Compose_Transforms(Object->UV_Trans, Trans);

	if(Object->interior != NULL)
		Object->interior->Transform(Trans);

	Object->Rotate(Vector, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Object
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

void Scale_Object (ObjectPtr Object, const VECTOR Vector, const TRANSFORM *Trans)
{
	if (Object == NULL)
		return;

	for(vector<ObjectPtr>::iterator Sib = Object->Bound.begin(); Sib != Object->Bound.end(); Sib++)
	{
		Scale_Object(*Sib, Vector, Trans);
	}

	if (Object->Clip != Object->Bound)
	{
		for(vector<ObjectPtr>::iterator Sib = Object->Clip.begin(); Sib != Object->Clip.end(); Sib++)
			Scale_Object(*Sib, Vector, Trans);
	}

	/* NK 1998 added if */
	if (!Test_Flag(Object, UV_FLAG))
	{
		Transform_Textures(Object->Texture, Trans);
		Transform_Textures(Object->Interior_Texture, Trans);
	}

	if (Object->UV_Trans == NULL)
		Object->UV_Trans = Create_Transform();
	Compose_Transforms(Object->UV_Trans, Trans);

	if(Object->interior != NULL)
		Object->interior->Transform(Trans);

	Object->Scale(Vector, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Object
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

void Transform_Object (ObjectPtr Object, const TRANSFORM *Trans)
{
	if (Object == NULL)
		return;

	for(vector<ObjectPtr>::iterator Sib = Object->Bound.begin(); Sib != Object->Bound.end(); Sib++)
	{
		Transform_Object(*Sib, Trans);
	}

	if (Object->Clip != Object->Bound)
	{
		for(vector<ObjectPtr>::iterator Sib = Object->Clip.begin(); Sib != Object->Clip.end(); Sib++)
		{
			Transform_Object(*Sib, Trans);
		}
	}

	/* NK 1998 added if */
	if (!Test_Flag(Object, UV_FLAG))
	{
		Transform_Textures(Object->Texture, Trans);
		Transform_Textures(Object->Interior_Texture, Trans);
	}

	if (Object->UV_Trans == NULL)
		Object->UV_Trans = Create_Transform();
	Compose_Transforms(Object->UV_Trans, Trans);

	if(Object->interior != NULL)
		Object->interior->Transform(Trans);

	Object->Transform(Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Object
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

// this function must not be called on CSG objects
void Invert_Object(ObjectPtr Object)
{
	if (Object == NULL)
		return;
	assert((Object->Type & IS_CSG_OBJECT) == 0);
	Object->Invert();
}

// Invert_CSG_Object deletes the original object and returns a new one
// we force use of a separate function for this to help ensure that calling code handles this
// we also set the passed pointer to NULL
// (yes, this is ugly and needs to be fixed)
ObjectPtr Invert_CSG_Object(ObjectPtr& Object)
{
	CSG     *csg_object;

	if(Object == NULL)
		return NULL;
	Object->Invert();

	csg_object = dynamic_cast<CSG *>(Object);
	assert(csg_object != NULL);

	// Morph will delete the old object
	csg_object = csg_object->Morph();
	Object = NULL;

	return csg_object;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Object
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

ObjectPtr Copy_Object (ObjectPtr Old)
{
	ObjectPtr New;

	if(Old == NULL)
		return NULL;

	New = (ObjectPtr)Old->Copy();

	/*
	 * The following copying of OBJECT_FIELDS is redundant if Copy
	 * did *New = *Old but we cannot assume it did. It is safe for
	 * Copy to do *New = *Old but it should not otherwise
	 * touch OBJECT_FIELDS.
	 */

	New->Type    = Old->Type;
	New->Bound   = Old->Bound;
	New->Clip    = Old->Clip;
	New->BBox    = Old->BBox;
	New->Flags   = Old->Flags;

	New->Ph_Density             = Old->Ph_Density;
	New->RadiosityImportance    = Old->RadiosityImportance;

	// TODO FIXME - An explanation WHY this is important would be nice [CLi]
	New->LLights.clear(); // important

	New->Texture = Copy_Textures (Old->Texture);
	New->Interior_Texture = Copy_Textures (Old->Interior_Texture);
	if(Old->interior != NULL)
		New->interior = new Interior(*(Old->interior));
	else
		New->interior = NULL;

	/* NK 1998 */
	New->UV_Trans = Copy_Transform(Old->UV_Trans);
	/* NK ---- */

	// TODO: we really ought to decide whether or not it's useful to maintain
	//       the overhead of having multiple clip and bound objects ... it is
	//       after all possible for the user to use CSG and give us one object
	//       meaning we could use a plain pointer here.
	if (Old->Bound.empty() == false)
		New->Bound = Copy_Objects(Old->Bound);
	if (Old->Clip.empty() == false)
	{
		// note that in this case the objects are shared and should only be
		// destroyed the once !!! ... to be frank POV really needs a reference-
		// counted system for sharing objects with copy-on-write semantics.
		if(Old->Bound != Old->Clip)
			New->Clip = Copy_Objects(Old->Clip);
		else
			New->Clip = New->Bound;
	}

	return New;
}

vector<ObjectPtr> Copy_Objects (vector<ObjectPtr>& Src)
{
	vector<ObjectPtr> Dst ;

	for(vector<ObjectPtr>::iterator it = Src.begin(); it != Src.end(); it++)
		Dst.push_back(Copy_Object(*it)) ;
	return (Dst) ;
}

/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Object
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

void Destroy_Single_Object (ObjectPtr *objectPtr)
{
	ObjectPtr object = *objectPtr;

	Destroy_Textures(object->Texture);

	Destroy_Object(object->Bound);

	Destroy_Interior((Interior *)object->interior);

	/* NK 1998 */
	Destroy_Transform(object->UV_Trans);

	Destroy_Object(object->Bound);
	Destroy_Interior((Interior *)object->interior);

	if(object->Bound != object->Clip)
		Destroy_Object(object->Clip);

	delete object;
}

void Destroy_Object(vector<ObjectPtr>& Objects)
{
	for(vector<ObjectPtr>::iterator Sib = Objects.begin(); Sib != Objects.end(); Sib++)
		Destroy_Object (*Sib);
	Objects.clear();
}

void Destroy_Object(ObjectPtr Object)
{
	if(Object != NULL)
	{
		bool DestroyClip = true ;
		if (!Object->Bound.empty() && !Object->Clip.empty())
			if (*Object->Bound.begin() == *Object->Clip.begin())
				DestroyClip = false ;
		Destroy_Textures(Object->Texture);
		Destroy_Textures(Object->Interior_Texture);
		Destroy_Object(Object->Bound);

		Destroy_Interior((Interior *)Object->interior);
		Destroy_Transform(Object->UV_Trans);

		if (DestroyClip)
			Destroy_Object(Object->Clip);

		if (dynamic_cast<CompoundObject *> (Object) != NULL)
			Destroy_Object (((CompoundObject *) Object)->children);

		delete Object;
	}
}


/*****************************************************************************
*
* FUNCTION
*
*   UVCoord
*
* INPUT
*
*   Object  - Pointer to blob structure
*   Inter   - Pointer to intersection
*
* OUTPUT
*
*
* RETURNS
*
* AUTHOR
*
*   Nathan Kopp
*
* DESCRIPTION
*   This is used as a default UVCoord function for objects where UVCoordinates
*   are not defined.  It instead returns the XY coordinates of the intersection.
*
* CHANGES
*
*
******************************************************************************/

void ObjectBase::UVCoord(UV_VECT Result, const Intersection *Inter, TraceThreadData *) const
{
	Result[U] = Inter->IPoint[X];
	Result[V] = Inter->IPoint[Y];
}

void ObjectBase::Determine_Textures(Intersection *isect, bool hitinside, WeightedTextureVector& textures, TraceThreadData *threaddata)
{
	if((Interior_Texture != NULL) && (hitinside == true))
		textures.push_back(WeightedTexture(1.0, Interior_Texture));
	else if(Texture != NULL)
		textures.push_back(WeightedTexture(1.0, Texture));
	else if(isect->Csg != NULL)
		isect->Csg->Determine_Textures(isect, hitinside, textures, threaddata);
}

bool ObjectBase::Intersect_BBox(BBoxDirection variant, const BBOX_VECT& origin, const BBOX_VECT& invdir, BBOX_VAL maxd) const
{
	// TODO FIXME - This was SMALL_TOLERANCE, but that's too rough for some scenes [cjc] need to check what it was in the old code [trf]
	switch(variant)
	{
		case BBOX_DIR_X0Y0Z0: // 000
			return Intersect_BBox_Dir<0, 0, 0>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X0Y0Z1: // 001
			return Intersect_BBox_Dir<0, 0, 1>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X0Y1Z0: // 010
			return Intersect_BBox_Dir<0, 1, 0>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X0Y1Z1: // 011
			return Intersect_BBox_Dir<0, 1, 1>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X1Y0Z0: // 100
			return Intersect_BBox_Dir<1, 0, 0>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X1Y0Z1: // 101
			return Intersect_BBox_Dir<1, 0, 1>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X1Y1Z0: // 110
			return Intersect_BBox_Dir<1, 1, 0>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
		case BBOX_DIR_X1Y1Z1: // 111
			return Intersect_BBox_Dir<1, 1, 1>(BBox, origin, invdir, MIN_ISECT_DEPTH, maxd);
	}

	return false; // unreachable
}

template<int BX, int BY, int BZ>
FORCEINLINE bool Intersect_BBox_Dir(const BBOX& bbox, const BBOX_VECT& origin, const BBOX_VECT& invdir, BBOX_VAL mind, BBOX_VAL maxd)
{
	BBOX_VAL tmin, tmax, tymin, tymax, tzmin, tzmax;
	BBOX_VECT bounds[2];

	Make_min_max_from_BBox(bounds[0], bounds[1], bbox);

	tmin = (bounds[BX][X] - origin[X]) * invdir[X];
	tmax = (bounds[1 - BX][X] - origin[X]) * invdir[X];
	tymin = (bounds[BY][Y] - origin[Y]) * invdir[Y];
	tymax = (bounds[1 - BY][Y] - origin[Y]) * invdir[Y];

	if((tmin > tymax) || (tymin > tmax))
		return false;

	if(tymin > tmin)
		tmin = tymin;

	if(tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[BZ][Z] - origin[Z]) * invdir[Z];
	tzmax = (bounds[1 - BZ][Z] - origin[Z]) * invdir[Z];

	if((tmin > tzmax) || (tzmin > tmax))
		return false;

	if(tzmin > tmin)
		tmin = tzmin;

	if(tzmax < tmax)
		tmax = tzmax;

	return ((tmin < maxd) && (tmax > mind));
}

}
