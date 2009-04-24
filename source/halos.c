/****************************************************************************
*                   halo.c
*
*  This module contains all functions for halo effects.
*
*  This file was written by Zsolt Szalavari. He wrote the code for
*  halos and generously provided us these enhancements.
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

/*
  ===========================================================================
  Copyright 1995 Zsolt Szalavari.
  Vienna University of Technology - Institute of Computer Graphics
  All Rights Reserved.
  ===========================================================================
   NAME:       halos.c
   TYPE:       c code
   PROJECT:    Halo Visualisation Project
   CONTENT:
   VERSION:    1.0.0
  ===========================================================================
    AUTHOR:     zss     Zsolt Szalavari
  ===========================================================================
   HISTORY:

   29-May-95           zss     last modification
   10-Mar-95           zss     created
  ===========================================================================
*/

#include "frame.h"
#include "povproto.h"
#include "vector.h"
#include "povray.h"
#include "atmosph.h"
#include "pattern.h"
#include "texture.h"
#include "halos.h"
#include "objects.h"
#include "matrices.h"
#include "lighting.h"
#include "colour.h"


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define SMALLSTEP 0.1



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Local variables
******************************************************************************/



/*****************************************************************************
* Static functions
******************************************************************************/

static DBL determine_density PARAMS((HALO *Halo, VECTOR Place));
static void compute_halo_colour PARAMS((COLOUR Colour, HALO *Halo, DBL value));
static void sample_halos PARAMS((RAY *Ray, HALO *Halo, VECTOR In_Point, DBL dist, VECTOR One_Step, COLOUR Halo_Colour, int Light_Ray_Flag));
static void supersample_halos PARAMS((RAY *Ray, int level, HALO *Halo,
  DBL d1, COLOUR C1, DBL d3, COLOUR C3, VECTOR In_Point, VECTOR One_Step,
  int Light_Ray_FLag));



/*****************************************************************************
*
* FUNCTION
*
*   Do_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   This one does it !!!
*
*   It is the main calculation routine, which is called whenever a
*   transparent object was hit and it was a halo by fortune. Roughly said
*   it gets the info of the entry and the exit point of the ray in a halo,
*   and more info about what object was hit. Performing the calculations
*   it passes a colour value back which represents the appearance of the
*   halo combined with everything behind. Detailed explanation inline.
*
* CHANGES
*
*  May 1995 : Creation.
*
*  Aug 1995 : Modified sampling loop. Added jitter. [DB]
*
******************************************************************************/

void Do_Halo(Halo, Ray, Ray_Intersection, Colour, Light_Ray_Flag)
HALO *Halo;
RAY *Ray;
INTERSECTION *Ray_Intersection;
COLOUR Colour;
int Light_Ray_Flag;
{
  int i;
  DBL dist, dist_prev, jdist;
  DBL Distance, Stepsize;
  DBL Halo_Factor, Halo_Factor_Inverse;
  DBL Total_Density, Temp_Density;
  VECTOR In_Point, Out_Point, P1, P2, D;
  VECTOR One_Step;
  COLOUR Halo_Colour, Halo_Colour_prev, Temp_Colour;

  /*
   * Get start and end point.
   *
   * YOU SHOULD ALWAYS USE UNIT SIZE GENERATED OBJECTS, WHICH ARE
   * SCALED, ROTATED AND TRANSLATED TO THEIR FINAL POSITION AND SHAPE!!!
   * THUS YOU WILL BE SAVE FROM ANY HOLY CRASHES!!!
   */

  Assign_Vector(In_Point,  Ray->Initial);
  Assign_Vector(Out_Point, Ray_Intersection->IPoint);

  Distance = Ray_Intersection->Depth;

  /*
   * Get step size based on number of samples.
   */

  Stepsize = Distance / Halo->Samples;

  /*
   * Get step vector, i.e. the vetor pointing in the direction of the
   * current ray and having a length that is equal to one sample step.
   */

  VScale(One_Step, Ray->Direction, Stepsize);

  /*
   * Resize Stepsize to remove dependencies due to container object
   * size. This assumes that the halo is designed for a unit size
   * object (that's normally the case because the halo density map
   * works on unit size objects).
   */

  MInvTransPoint(P1, In_Point,  Halo->Container_Trans);
  MInvTransPoint(P2, Out_Point, Halo->Container_Trans);

  VSub(D, P2, P1);

  VLength(dist, D);

  /* 
   * We can resize the Stepsize because it's only used for weighting 
   * the halo samples, not for stepping through the halo.
   */

  Stepsize = dist / Halo->Samples;

  /*
   * Here it happens. The main stepping loop to step through the halo space.
   */

  Make_Colour(Temp_Colour, 0.0, 0.0, 0.0);

  Make_Colour(Halo_Colour_prev, 0.0, 0.0, 0.0);

  dist_prev = 0.0;

  Total_Density = 0.0;

  for (i = 0; i <= Halo->Samples; i++)
  {
    /* Get distance to next sample point. */

    dist = (DBL)i + 0.5;

    /* Sample at current location if last sample hasn't been traced yet. */

    if (i < Halo->Samples)
    {
      /* Get jittered distance. */

      jdist = dist + (FRAND() - 0.5) * Halo->Jitter;

      sample_halos(Ray, Halo, In_Point, jdist, One_Step, Halo_Colour, Light_Ray_Flag);
    }

    if (i)
    {
      /* If the last sample has been traced we just want to add it. */

      if (i < Halo->Samples)
      {
        /* Do the current and previous colours differ too much? */

        if ((Halo->AA_Level > 0) && (Colour_Distance(Halo_Colour, Halo_Colour_prev) >= Halo->AA_Threshold))
        {
          /* Supersample between current and previous point. */

          supersample_halos(Ray, 1, Halo, dist_prev, Halo_Colour_prev, dist, Halo_Colour, In_Point, One_Step, Light_Ray_Flag);

          Halo_Colour[RED]    = Halo_Colour_prev[RED]    = 0.5 * (Halo_Colour[RED]    + Halo_Colour_prev[RED]);
          Halo_Colour[GREEN]  = Halo_Colour_prev[GREEN]  = 0.5 * (Halo_Colour[GREEN]  + Halo_Colour_prev[GREEN]);
          Halo_Colour[BLUE]   = Halo_Colour_prev[BLUE]   = 0.5 * (Halo_Colour[BLUE]   + Halo_Colour_prev[BLUE]);
          Halo_Colour[FILTER] = Halo_Colour_prev[FILTER] = 0.5 * (Halo_Colour[FILTER] + Halo_Colour_prev[FILTER]);
          Halo_Colour[TRANSM] = Halo_Colour_prev[TRANSM] = 0.5 * (Halo_Colour[TRANSM] + Halo_Colour_prev[TRANSM]);
        }
      }

      Total_Density = min(1.0, Total_Density + Stepsize * Halo_Colour_prev[TRANSM]);

      /*
       * After we stored the total density we sum up the colour values.
       * For GLOWING each color contribution is attenuated by the total
       * density up to this point (i.e. light emerging from a point in a
       * GLOW is attenuated by the particles lying in front of it).
       */

      switch (Halo->Rendering_Type)
      {
        case HALO_DUST:
        case HALO_EMITTING:

          Temp_Colour[RED]    += Halo_Colour_prev[RED]    * Halo_Colour_prev[TRANSM];
          Temp_Colour[GREEN]  += Halo_Colour_prev[GREEN]  * Halo_Colour_prev[TRANSM];
          Temp_Colour[BLUE]   += Halo_Colour_prev[BLUE]   * Halo_Colour_prev[TRANSM];
          Temp_Colour[FILTER] += Halo_Colour_prev[FILTER] * Halo_Colour_prev[TRANSM];
          Temp_Colour[TRANSM] += Halo_Colour_prev[TRANSM] * Halo_Colour_prev[TRANSM];

          break;

        case HALO_GLOWING:

          Temp_Density = 1.0 - Total_Density;

          Temp_Colour[RED]    += Temp_Density * Halo_Colour_prev[RED]    * Halo_Colour_prev[TRANSM];
          Temp_Colour[GREEN]  += Temp_Density * Halo_Colour_prev[GREEN]  * Halo_Colour_prev[TRANSM];
          Temp_Colour[BLUE]   += Temp_Density * Halo_Colour_prev[BLUE]   * Halo_Colour_prev[TRANSM];
          Temp_Colour[FILTER] += Temp_Density * Halo_Colour_prev[FILTER] * Halo_Colour_prev[TRANSM];
          Temp_Colour[TRANSM] += Temp_Density * Halo_Colour_prev[TRANSM] * Halo_Colour_prev[TRANSM];

          break;
      }
    }

    Assign_Colour(Halo_Colour_prev, Halo_Colour);

    dist_prev = dist;
  }

  /*
   * Scale all the values by the stepsize for sampling rate independency.
   */

  Temp_Colour[RED]    *= Stepsize;
  Temp_Colour[GREEN]  *= Stepsize;
  Temp_Colour[BLUE]   *= Stepsize;
  Temp_Colour[FILTER] *= Stepsize;
  Temp_Colour[TRANSM] *= Stepsize;

  /*
   * Use the different rendering methods to determine the final colour.
   */

  Halo_Factor = 1.0 - Total_Density;

  if (!Light_Ray_Flag)
  {
    switch (Halo->Rendering_Type)
    {
      case HALO_ATTENUATING:

        compute_halo_colour(Halo_Colour, Halo, Total_Density);

        Halo_Factor_Inverse = 1.0 - Halo_Factor;

        Colour[RED]    = Colour[RED]    * Halo_Factor + Halo_Colour[RED]    * Halo_Factor_Inverse;
        Colour[GREEN]  = Colour[GREEN]  * Halo_Factor + Halo_Colour[GREEN]  * Halo_Factor_Inverse;
        Colour[BLUE]   = Colour[BLUE]   * Halo_Factor + Halo_Colour[BLUE]   * Halo_Factor_Inverse;
        Colour[FILTER] = Colour[FILTER] * Halo_Factor + Halo_Colour[FILTER] * Halo_Factor_Inverse;
        Colour[TRANSM] = Colour[TRANSM] * Halo_Factor + Halo_Colour[TRANSM] * Halo_Factor_Inverse;

        break;

      case HALO_EMITTING:
      case HALO_DUST:
      case HALO_GLOWING:

        Colour[RED]    = Colour[RED]    * Halo_Factor + Temp_Colour[RED];
        Colour[GREEN]  = Colour[GREEN]  * Halo_Factor + Temp_Colour[GREEN];
        Colour[BLUE]   = Colour[BLUE]   * Halo_Factor + Temp_Colour[BLUE];
        Colour[FILTER] = Colour[FILTER] * Halo_Factor + Temp_Colour[FILTER];
        Colour[TRANSM] = Colour[TRANSM] * Halo_Factor + Temp_Colour[TRANSM];

        break;
    }
  }

  Increase_Counter(stats[Halo_Rays_Traced]);
}



/*****************************************************************************
*
* FUNCTION
*
*   sample_halos
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
* CHANGES
*
*   Aug 1995 : Moved here from Do_Halo. [DB]
*
******************************************************************************/

static void sample_halos(Ray, Halo, In_Point, dist, One_Step, Halo_Colour, Light_Ray_Flag)
RAY *Ray;
HALO *Halo;
VECTOR In_Point, One_Step;
DBL dist;
COLOUR Halo_Colour;
int Light_Ray_Flag;
{
  DBL Density, Light_Source_Depth, Cos_Ray_Light, Phase, g, f, fi;
  VECTOR Place, Local_Place;
  COLOUR Temp_Colour, Temp2_Colour;
  COLOUR Light_Colour;
  RAY Light_Source_Ray;
  LIGHT_SOURCE *Light_Source;
  HALO *Local_Halo;

  /* Get sample point in world coordinates. */

  VEvaluateRay(Place, In_Point, dist, One_Step);

  /* Look for multiple halos and get their density & colour contribution. */

  Make_Colour(Halo_Colour, 0.0, 0.0, 0.0);

  for (Local_Halo = Halo; Local_Halo != NULL; Local_Halo = Local_Halo->Next_Halo)
  {
    Make_Colour(Temp_Colour, 0.0, 0.0, 0.0);

    MInvTransPoint(Local_Place, Place, Local_Halo->Trans);

    Density = determine_density(Local_Halo, Local_Place);

    compute_halo_colour(Temp_Colour, Local_Halo, Density);

    /*
     * If the halo happens to be DUST, we have to look for lights.
     * The routine looks for all light sources, checks whether they are
     * blocked or attenuated (spot lights). Finally it calculates the
     * phase depending on the cosine of the angle between eye-ray and
     * light-ray.
     *
     * If the current ray is a light source ray, i.e. it is not a viewing
     * ray, do not determine attenuation for dust.
     */

    if ((!Light_Ray_Flag) && (Local_Halo->Rendering_Type == HALO_DUST))
    {
      /* Do not change the filter and transmittance channels. */

/*
      Temp_Colour[RED] = Temp_Colour[GREEN] = Temp_Colour[BLUE] = 0.0;
*/
      Make_Colour(Temp2_Colour, 0.0, 0.0, 0.0);

      for (Light_Source = Frame.Light_Sources; Light_Source != NULL; Light_Source = Light_Source->Next_Light_Source)
      {
        if (!Test_Shadow(Light_Source, &Light_Source_Depth, &Light_Source_Ray, Ray, Place, Light_Colour))
        {
          VDot(Cos_Ray_Light, Ray->Direction, Light_Source_Ray.Direction);

          switch (Halo->Dust_Type)
          {
            case MIE_HAZY_SCATTERING:

              Phase = 0.1 * (1.0 + 0.03515625 * pow((1.0 + Cos_Ray_Light), 8.0));

              break;

            case MIE_MURKY_SCATTERING:

              Phase = 0.019607843 * (1.0 + 1.1641532e-8 * pow(1.0 + Cos_Ray_Light, 32.0));

              break;

            case RAYLEIGH_SCATTERING:

              Phase = (1.0 + Sqr(Cos_Ray_Light)) / 2.0;

              break;

            case HENYEY_GREENSTEIN_SCATTERING:

              g = Local_Halo->Eccentricity;

              Phase = (1.0 - Sqr(g)) / pow((1.0 + Sqr(g) - 2.0*g*Cos_Ray_Light), 1.5);

              break;

            case ISOTROPIC_SCATTERING:
            default:

              Phase = 1.0;
          }

          Temp2_Colour[RED]   += Light_Colour[RED]   * Phase;
          Temp2_Colour[GREEN] += Light_Colour[GREEN] * Phase;
          Temp2_Colour[BLUE]  += Light_Colour[BLUE]  * Phase;
        }
      }

      f = Temp_Colour[FILTER];

      fi = 1.0 - f;

      Temp_Colour[RED]   = Temp2_Colour[RED]   * (fi + f * Temp_Colour[RED]);
      Temp_Colour[GREEN] = Temp2_Colour[GREEN] * (fi + f * Temp_Colour[GREEN]);
      Temp_Colour[BLUE]  = Temp2_Colour[BLUE]  * (fi + f * Temp_Colour[BLUE]);
    }

    /* The contribution of multiple halos is added up here. */

    Halo_Colour[RED]    += Temp_Colour[RED]    - Halo_Colour[RED]    * Temp_Colour[RED];
    Halo_Colour[GREEN]  += Temp_Colour[GREEN]  - Halo_Colour[GREEN]  * Temp_Colour[GREEN];
    Halo_Colour[BLUE]   += Temp_Colour[BLUE]   - Halo_Colour[BLUE]   * Temp_Colour[BLUE];
    Halo_Colour[FILTER] += Temp_Colour[FILTER] - Halo_Colour[FILTER] * Temp_Colour[FILTER];
    Halo_Colour[TRANSM] += Temp_Colour[TRANSM] - Halo_Colour[TRANSM] * Temp_Colour[TRANSM];
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   supersample_halos
*
* INPUT
*
*   level - level of recursion
*   Halo  - pointer to first halo
*   d1    - distance to lower sample
*   d3    - distance to upper sample
*
* OUTPUT
*
*   C1    - Color of lower sample
*   C3    - Color of upper sample
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Recursevily supersample between two points on the ray.
*
* CHANGES
*
*   Aug 1995 : Creation.
*
******************************************************************************/

static void supersample_halos(Ray, level, Halo, d1, C1, d3, C3, In_Point, One_Step, Light_Ray_Flag)
RAY *Ray;
int level, Light_Ray_Flag;
HALO *Halo;
DBL d1, d3;
COLOUR C1, C3;
VECTOR In_Point, One_Step;
{
  DBL d2, jdist;
  COLOUR C2;

  Increase_Counter(stats[Halo_Supersamples]);

  /* Sample between lower and upper point. */

  d2 = 0.5 * (d1 + d3);

  jdist = d2 + Halo->Jitter * 0.5 * (d3 - d1) * (FRAND() - 0.5);

  sample_halos(Ray, Halo, In_Point, jdist, One_Step, C2, Light_Ray_Flag);

  /* Test for further supersampling. */

  if (level < Halo->AA_Level)
  {
    if (Colour_Distance(C1, C2) >= Halo->AA_Threshold)
    {
      /* Supersample between lower and middle point. */

      supersample_halos(Ray, level+1, Halo, d1, C1, d2, C2, In_Point, One_Step, Light_Ray_Flag);
    }

    if (Colour_Distance(C2, C3) >= Halo->AA_Threshold)
    {
      /* Supersample between current and higher point. */

      supersample_halos(Ray, level+1, Halo, d2, C2, d3, C3, In_Point, One_Step, Light_Ray_Flag);
    }
  }

  /* Add supersampled colors. */

  VLinComb2(C1, 0.75, C1, 0.25, C2);
  VLinComb2(C3, 0.25, C2, 0.75, C3);
}



/*****************************************************************************
*
* FUNCTION
*
*   compute_halo_colour
*
* INPUT
*
*   Halo   - Current halo
*   value  - Current value of the scalar field
*
* OUTPUT
*
*   Colour - Color of halo for given density
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   This one will calculate the Halo_Colour, depending on the estimated
*   density. It's actually a mirror of the Compute_Colour routine in
*   texture.c, modified to work with halos. It interpolates the colour
*   from the colour_map.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

static void compute_halo_colour (Colour, Halo, value)
COLOUR Colour;
HALO *Halo;
DBL value;
{
  DBL fraction;
  COLOUR C1, C2;
  BLEND_MAP *Blend_Map = Halo->Blend_Map;
  BLEND_MAP_ENTRY *Curr, *Prev;

  value = fmod(value * Halo->Frequency + Halo->Phase, 1.00001);

  /* allow negative Frequency */

  if (value < 0.0)
  {
    value -= floor(value);
  }

  Search_Blend_Map(value, Blend_Map, &Prev, &Curr);

  if (Prev != Curr)
  {
    fraction = (value - Prev->value) / (Curr->value - Prev->value);

    Assign_Colour(C1, Curr->Vals.Colour);
    Assign_Colour(C2, Prev->Vals.Colour);

    Colour[RED]    = C2[RED]    + fraction * (C1[RED]    - C2[RED]);
    Colour[GREEN]  = C2[GREEN]  + fraction * (C1[GREEN]  - C2[GREEN]);
    Colour[BLUE]   = C2[BLUE]   + fraction * (C1[BLUE]   - C2[BLUE]);
    Colour[FILTER] = C2[FILTER] + fraction * (C1[FILTER] - C2[FILTER]);
    Colour[TRANSM] = C2[TRANSM] + fraction * (C1[TRANSM] - C2[TRANSM]);
  }
  else
  {
    Assign_Colour(Colour, Curr->Vals.Colour);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   determine_density
*
* INPUT
*
*   Halo  - Current halo
*   Place - Current location of evaluation (in halo space)
*
* OUTPUT
*
* RETURNS
*
*   DBL - Density in current location
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   This routine is dedicated to estimate the density, OOPS, the change in
*   the density at a given point in halo space. First it adds turbulence
*   (if there is some to add) to the vector pointing at a place in halo
*   space. Then it determines a 'radius', depending on the spatial mapping
*   of the density vals. After that we get the density as a constant, linear
*   or whatever function of this radius. At the end we cut the value to a
*   tiny unit intervall.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

static DBL determine_density(Halo, Place)
HALO *Halo;
VECTOR Place;
{
  DBL Radius, Solution;
  VECTOR Local_Place, Temp;

  /* Add turbulence. */

  if (Halo->Turb != NULL)
  {
    DTurbulence(Temp, Place, Halo->Turb);

    Local_Place[X] = Place[X] + Temp[X] * Halo->Turb->Turbulence[X];
    Local_Place[Y] = Place[Y] + Temp[Y] * Halo->Turb->Turbulence[Y];
    Local_Place[Z] = Place[Z] + Temp[Z] * Halo->Turb->Turbulence[Z];
  }
  else
  {
    Assign_Vector(Local_Place, Place);
  }

  /* Estimate radius, depending on mapping type. */

  switch (Halo->Mapping_Type)
  {
    case HALO_CYLINDRICAL_MAP:

      Radius = sqrt(Sqr(Local_Place[X]) + Sqr(Local_Place[Z]));

      break;

    case HALO_PLANAR_MAP:

      Radius = fabs(Local_Place[Y]);

      break;

    case HALO_BOX_MAP:

      Radius = max(fabs(Local_Place[X]), max(fabs(Local_Place[Y]), fabs(Local_Place[Z])));

      break;

    case HALO_SPHERICAL_MAP:
    default:

      VLength(Radius, Local_Place);

      break;
  }

  /* Clip density. */

  if (Radius < 0.0)
  {
    Radius = 0.0;
  }
  else
  {
    if (Radius > 1.0)
    {
      Radius = 1.0;
    }
  }

  /* Calculate density by choosen formula. */

  switch (Halo->Type)
  {
    case HALO_LINEAR:

      Solution = Halo->Max_Value * (1.0 - Radius);

      break;

    case HALO_CUBIC:

      Solution = Halo->Max_Value * ((2.0 * Radius - 3.0 ) * Sqr(Radius) + 1.0);

      break;

    case HALO_POLY:

      Solution = Halo->Max_Value * pow((1.0 - Radius), Halo->Exponent);

      break;

    case HALO_CONSTANT:
    default:

      Solution = Halo->Max_Value;

      break;
  }

  Increase_Counter(stats[Halo_Samples]);

  return(Solution);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   In the beginning there was a simple halo, beeing a default for everything!
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

HALO *Create_Halo ()
{
  HALO *New;

  New = (HALO *)POV_MALLOC(sizeof(HALO), "halo");

  Init_TPat_Fields((TPATTERN *)New);

  New->Type  = HALO_NO_HALO;
  New->Flags = 0;

  New->Rendering_Type = HALO_EMITTING;
  New->Mapping_Type   = HALO_SPHERICAL_MAP;
  New->Dust_Type      = ISOTROPIC_SCATTERING;

  New->Max_Value      = 1.0;
  New->Exponent       = 1.0;
  New->Eccentricity   = 0.0;
  New->Samples        = 10;
  New->Jitter         = 0.0;
  New->Frequency      = 1.0;
  New->Phase          = 0.0;
  New->Blend_Map      = NULL;
  New->Next_Halo      = NULL;
  New->Turb           = NULL;

  New->AA_Level     = 0;
  New->AA_Threshold = 0.3;

  New->Trans = Create_Transform();

  New->Container_Trans = Create_Transform();

  return(New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   Designing multiply halos requires copying the old data fields to the new
*   halo. Exactly that's the reason why this routine has been written.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

HALO *Copy_Halo (Old)
HALO *Old;
{
  HALO *New, *First, *Previous, *Local_Halo;

  Previous = First = NULL;

  if (Old != NULL)
  {
    for (Local_Halo = Old; Local_Halo != NULL; Local_Halo = Local_Halo->Next_Halo)
    {
      New = Create_Halo();

      Destroy_Transform(New->Trans);
      Destroy_Transform(New->Container_Trans);

      *New = *Local_Halo;

      New->Trans = Copy_Transform (Local_Halo->Trans);
      New->Container_Trans = Copy_Transform (Local_Halo->Container_Trans);

      New->Blend_Map = Copy_Blend_Map (Local_Halo->Blend_Map);

      New->Turb = (TURB *)Copy_Warps((WARP *)(Local_Halo->Turb));

      if (First == NULL)
      {
        First = New;
      }

      if (Previous != NULL)
      {
        Previous->Next_Halo = New;
      }

      Previous = New;
    }
  }
  else
  {
    First = NULL;
  }

  return(First);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   To the ashes with the remainals of a data structure. It's hard.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Destroy_Halo (Halo)
HALO *Halo;
{
  HALO *Local_Halo, *Temp;

  if (Halo == NULL)
  {
    return;
  }

  Local_Halo = Halo;

  while (Local_Halo != NULL)
  {
    Destroy_Blend_Map(Local_Halo->Blend_Map);
    Destroy_Transform(Local_Halo->Trans);
    Destroy_Transform(Local_Halo->Container_Trans);
    Destroy_Warps((WARP *)(Local_Halo->Turb));

    Temp = Local_Halo->Next_Halo;

    POV_FREE(Local_Halo);

    Local_Halo = Temp;
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_One_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   Compute the translation transform and transform the halo.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Translate_One_Halo(Halo, Trans)
HALO *Halo;
TRANSFORM *Trans;
{
  if (Halo != NULL)
  {
    Transform_One_Halo(Halo, Trans);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_One_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   Compute the rotation transform and transform the halo.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Rotate_One_Halo(Halo, Trans)
HALO *Halo;
TRANSFORM *Trans;
{
  if (Halo != NULL)
  {
    Transform_One_Halo(Halo, Trans);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_One_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   Compute the scaling transform and transform the halo.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Scale_One_Halo(Halo, Trans)
HALO *Halo;
TRANSFORM *Trans;
{
  if (Halo != NULL)
  {
    Transform_One_Halo(Halo, Trans);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_One_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   Transform one halo.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Transform_One_Halo(Halo,Trans)
HALO *Halo;
TRANSFORM *Trans;
{
  if (Halo != NULL)
  {
    Compose_Transforms(Halo->Trans, Trans);
  }
}


/*****************************************************************************
*
* FUNCTION
*
*   Transform_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   Transform halo and apply the trafo to all of it's multiply friends.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Transform_Halo(Halo,Trans)
HALO *Halo;
TRANSFORM *Trans;
{
  HALO *Local_Halo;

  if (Halo != NULL)
  {
    for (Local_Halo = Halo; Local_Halo != NULL; Local_Halo = Local_Halo->Next_Halo)
    {
      Compose_Transforms(Local_Halo->Trans, Trans);
    }
  }
}


/*****************************************************************************
*
* FUNCTION
*
*   Post_Halo
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Zsolt Szalavari
*
* DESCRIPTION
*
*   To get everything correctly caught by the routines looking for a
*   halo, some parameters must be set previously. Care is taken of
*   eventually bad links.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

void Post_Halo(Texture)
TEXTURE *Texture;
{
  if (Texture->Halo == NULL)
  {
    return;
  }

  if (Texture->Halo->Flags & POST_DONE)
  {
    return;
  }

  Texture->Halo->Flags |= POST_DONE;
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Halo_Container
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
*   Transform halo container.
*
* CHANGES
*
*   Mar 1996 : Creation.
*
******************************************************************************/

void Rotate_Halo_Container(Textures, Trans)
TEXTURE *Textures;
TRANSFORM *Trans;
{
  Transform_Halo_Container(Textures, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Halo_Container
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
*   Transform halo container.
*
* CHANGES
*
*   Mar 1996 : Creation.
*
******************************************************************************/

void Scale_Halo_Container(Textures, Trans)
TEXTURE *Textures;
TRANSFORM *Trans;
{
  Transform_Halo_Container(Textures, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Halo_Container
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
*   Transform halo container.
*
* CHANGES
*
*   Mar 1996 : Creation.
*
******************************************************************************/

void Translate_Halo_Container(Textures, Trans)
TEXTURE *Textures;
TRANSFORM *Trans;
{
  Transform_Halo_Container(Textures, Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Halo_Container
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
*   Transform halo container.
*
* CHANGES
*
*   Mar 1996 : Creation.
*
******************************************************************************/

void Transform_Halo_Container(Textures, Trans)
TEXTURE *Textures;
TRANSFORM *Trans;
{
  HALO *Local_Halo;
  TEXTURE *Layer;

  for (Layer = Textures; Layer != NULL; Layer = (TEXTURE *)Layer->Next)
  {
    if (Layer->Type == PLAIN_PATTERN)
    {
      if (Layer->Halo != NULL)
      {
        for (Local_Halo = Layer->Halo; Local_Halo != NULL; Local_Halo = Local_Halo->Next_Halo)
        {
          Compose_Transforms(Local_Halo->Container_Trans, Trans);
        }
      }
    }
  }
}


