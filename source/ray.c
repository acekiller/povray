/****************************************************************************
*                ray.c
*
*  This module implements the code pertaining to rays.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other 
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If 
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/

#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "povray.h"
#include "ray.h"
#include "texture.h"



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
*   Initialize_Ray_Containers
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

void Initialize_Ray_Containers(Ray)
RAY *Ray;
{
  Ray->Containing_Index = - 1;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Ray_Containers
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

void Copy_Ray_Containers(Dest_Ray, Source_Ray)
RAY *Dest_Ray, *Source_Ray;
{
  register int i;

  Dest_Ray->Containing_Index = Source_Ray->Containing_Index;

  for (i = 0 ; i <= Source_Ray->Containing_Index; i++)
  {
    Dest_Ray->Containing_Textures[i] = Source_Ray->Containing_Textures[i];
    Dest_Ray->Containing_Objects[i]  = Source_Ray->Containing_Objects[i];
    Dest_Ray->Containing_IORs[i]     = Source_Ray->Containing_IORs[i];
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Ray_Enter
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
*   Oct 1995 : Fixed bug with IOR assignment (only valid for plain textures) [DB]
*
******************************************************************************/

void Ray_Enter(Ray, texture, object)
RAY *Ray;
TEXTURE *texture;
OBJECT *object;
{
  register int index;

  if ((index = ++(Ray->Containing_Index)) >= MAX_CONTAINING_OBJECTS)
  {
    Error("Too many nested refracting objects.");
  }

  Ray->Containing_Textures[index] = texture;

  if ((texture->Type == PLAIN_PATTERN) && (texture->Finish != NULL))
  {
    Ray->Containing_IORs[index] = texture->Finish->Index_Of_Refraction;
  }
  else
  {
    Ray->Containing_IORs[index] = Frame.Atmosphere_IOR;
  }

  Ray->Containing_Objects[index] = object;
}



/*****************************************************************************
*
* FUNCTION
*
*   Ray_Exit
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
*   Remove given entry from given ray's container.
*
* CHANGES
*
*   -
*
******************************************************************************/

void Ray_Exit(Ray, nr)
RAY *Ray;
int nr;
{
  int i;

  for (i = nr; i < Ray->Containing_Index; i++)
  {
    Ray->Containing_Textures[i] = Ray->Containing_Textures[i+1];
    Ray->Containing_Objects[i]  = Ray->Containing_Objects[i+1];
    Ray->Containing_IORs[i]     = Ray->Containing_IORs[i+1];
  }

  if (--(Ray->Containing_Index) < - 1)
  {
    Error("Too many exits from refractions.");
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Texture_In_Ray_Container
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Test if a given texture is in the container of a given ray.
*
* CHANGES
*
*   Mar 1996 : Creation.
*
******************************************************************************/

int Texture_In_Ray_Container(Ray, Texture)
RAY *Ray;
TEXTURE *Texture;
{
  int i, found;

  found = -1;

  for (i = 0; i <= Ray->Containing_Index; i++)
  {
    if (Texture == Ray->Containing_Textures[i])
    {
      found = i;

      break;
    }
  }

  return(found);
}

