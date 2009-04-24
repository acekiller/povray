/****************************************************************************
*                   atmosph.c
*
*  This module contains all functions for atmospheric effects.
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
#include "atmosph.h"
#include "colour.h"
#include "povray.h"
#include "texture.h"
#include "pigment.h"
#include "objects.h"
#include "lighting.h"
#include "matrices.h"
#include "texture.h"



/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define BLACK_LEVEL 0.0001



/*****************************************************************************
* Local typedefs
******************************************************************************/

typedef struct Light_List_Struct LIGHT_LIST;

struct Light_List_Struct
{
  int active;
  DBL t1, t2;
  LIGHT_SOURCE *Light;
};



/*****************************************************************************
* Local variables
******************************************************************************/

/*
 * Atmosphere stuff.
 */

/* List of active lights. */

static LIGHT_LIST *Light_List;

/* Min. and max. intersetcion depths with light source areas. */

static DBL *tmin, *tmax;



/*****************************************************************************
* Static functions
******************************************************************************/

static DBL constant_fog PARAMS((RAY *Ray, DBL Depth, DBL Width, FOG *Fog, COLOUR Colour));
static DBL ground_fog PARAMS((RAY *Ray, DBL Depth, DBL Width, FOG *Fog, COLOUR Colour));

static void supersample_atmosphere PARAMS((LIGHT_LIST *Light_List,
  int level, RAY *Ray, ATMOSPHERE *Atmosphere,
  DBL d1, COLOUR C1, DBL d3, COLOUR C3));

static void sample_atmosphere PARAMS((LIGHT_LIST *Light_List, DBL dist,
  RAY *Ray, ATMOSPHERE *Atmosphere, COLOUR Col));

static int intersect_spotlight PARAMS((RAY *Ray, LIGHT_SOURCE *Light, DBL *d1, DBL *d2));
static int intersect_cylinderlight PARAMS((RAY *Ray, LIGHT_SOURCE *Light, DBL *d1, DBL *d2));

static void do_atmospheric_scattering PARAMS((RAY *Ray, INTERSECTION *Intersection, COLOUR Colour, int Light_Ray_Flag));
static void do_fog PARAMS((RAY *Ray, INTERSECTION *Intersection, COLOUR Colour, int Light_Ray_Flag));
static void do_rainbow PARAMS((RAY *Ray, INTERSECTION *Intersection, COLOUR Colour));
static void do_skysphere PARAMS((RAY *Ray, COLOUR Colour));



/*****************************************************************************
*
* FUNCTION
*
*   Initialize_Atmosphere_Code
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
*   Initialize atmosphere specific variables.
*
* CHANGES
*
*   Aug 1995 : Creation.
*
******************************************************************************/

void Initialize_Atmosphere_Code()
{
  Light_List = NULL;

  tmin = NULL;
  tmax = NULL;

  /* Allocate memory for atmosphere. */

  if ((Frame.Atmosphere != NULL) && 
      (fabs(Frame.Atmosphere->Distance) > EPSILON) &&
      (Frame.Number_Of_Light_Sources > 0))
  {
    Light_List = (LIGHT_LIST *)POV_MALLOC(Frame.Number_Of_Light_Sources * sizeof(LIGHT_LIST), "atmosphere sampling lists");

    tmin = (DBL *)POV_MALLOC(Frame.Number_Of_Light_Sources * sizeof(DBL), "atmosphere sampling lists");

    tmax = (DBL *)POV_MALLOC(Frame.Number_Of_Light_Sources * sizeof(DBL), "atmosphere sampling lists");
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Deinitialize_Atmosphere_Code
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
*   Deinitialize atmosphere specific variables.
*
* CHANGES
*
*   Aug 1995 : Creation.
*
******************************************************************************/

void Deinitialize_Atmosphere_Code()
{
  /* Free memory used by atmosphere. */

  if (Light_List != NULL)
  {
    POV_FREE(Light_List);
    POV_FREE(tmin);
    POV_FREE(tmax);
  }

  Light_List = NULL;

  tmin = NULL;
  tmax = NULL;
}



/*****************************************************************************
*
* FUNCTION
*
*   Do_Infinite_Atmosphere
*
* INPUT
*
*   Ray    - Current ray
*
* OUTPUT
*
*   Colour - Color of the current ray
*
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Apply atmospheric effects to an infinite ray.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
*   Jun 1995 : Added code for alpha channel support. [DB]
*
******************************************************************************/

void Do_Infinite_Atmosphere(Ray, Colour)
RAY *Ray;
COLOUR Colour;
{
  /* Set background color. */

  Assign_Colour(Colour, Frame.Background_Colour);
  
  Colour[FILTER] = 0.0;
  Colour[TRANSM] = 1.0;

  /* Determine atmospheric effects for infinite ray. */

  do_skysphere(Ray, Colour);
}



/*****************************************************************************
*
* FUNCTION
*
*   Do_Finite_Atmosphere
*
* INPUT
*
*   Ray            - Current ray
*   Intersection   - Current intersection
*   Light_Ray_Flag - TRUE if ray is a light source ray
*
* OUTPUT
*
*   Colour         - Color of the current ray
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Apply atmospheric effects to a finite ray.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

void Do_Finite_Atmosphere(Ray, Intersection, Colour, Light_Ray_Flag)
RAY *Ray;
INTERSECTION *Intersection;
COLOUR Colour;
int Light_Ray_Flag;
{
  if (!Light_Ray_Flag)
  {
    do_rainbow(Ray, Intersection, Colour);
  }

  do_atmospheric_scattering(Ray, Intersection, Colour, Light_Ray_Flag);

  do_fog(Ray, Intersection, Colour, Light_Ray_Flag);
}



/*****************************************************************************
*
* FUNCTION
*
*   do_atmospheric_scattering
*
* INPUT
*
*   Ray            - Current ray
*   Intersection   - Current intersection
*   Light_Ray_Flag - TRUE if ray is a light source ray
*
* OUTPUT
*
*   Colour         - Color of the current ray
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Simulate atmospheric scattering using volume sampling.
*
*   Ideas for the atmospheric scattering were taken from:
*
*     - M. Inakage, "An Illumination Model for Atmospheric Environments", ..
*
*     - Nishita, T., Miyawaki, Y. and Nakamae, E., "A Shading Model for
*       Atmospheric Scattering Considering Luminous Intensity Distribution
*       of Light Sources", Computer Graphics, 21, 4, 1987, 303-310
*
* CHANGES
*
*   Nov 1994 : Creation.
*
*   Jan 1995 : Added support of cylindrical light sources. [DB]
*
*   Jun 1995 : Added code for alpha channel support. [DB]
*
******************************************************************************/

static void do_atmospheric_scattering(Ray, Intersection, Colour, Light_Ray_Flag)
RAY *Ray;
INTERSECTION *Intersection;
COLOUR Colour;
int Light_Ray_Flag;
{
  int i, j;
  int active, interval, intervals, insert;
  DBL k, jdist, t1, t2, f, fi;
  DBL dist, dist_prev;
  DBL step;
  COLOUR Col_total, Col, Col_prev, Max_Colour, C1, C2;
  LIGHT_SOURCE *Light;
  ATMOSPHERE *Atmosphere;

  /* Why are we here? */

  if (((Atmosphere = Frame.Atmosphere) == NULL) || 
      (Frame.Atmosphere->Distance == 0.0) ||
      (Frame.Number_Of_Light_Sources == 0))
  {
    return;
  }

  /*
   * If we have a light source ray we only have to attenuate
   * the light source color due to the distance traveled.
   */

  if (Light_Ray_Flag)
  {
    k = exp(-Intersection->Depth / Atmosphere->Distance);

    /* Check for minimum transmittance. */

    if (k < Atmosphere->Colour[TRANSM])
    {
      k = Atmosphere->Colour[TRANSM];
    }

    VScaleEq(Colour, k);

    return;
  }

  /* Init light list and sampling intervals. */

  for (i = 0; i < Frame.Number_Of_Light_Sources; i++)
  {
    Light_List[i].active = FALSE;
    Light_List[i].t1     = 0.0;
    Light_List[i].t2     = Max_Distance;
    Light_List[i].Light  = NULL;

    tmin[i] =  BOUND_HUGE;
    tmax[i] = -BOUND_HUGE;
  }

  /* Get depths for all light sources and disconnected sampling intervals. */

  active = 0;

  intervals = 0;

  t1 = t2 = 0.0;

  Make_Colour(Max_Colour, 0.0, 0.0, 0.0);

  for (i = 0, Light = Frame.Light_Sources; Light != NULL; Light = Light->Next_Light_Source, i++)
  {
    insert = FALSE;

    Light_List[i].Light = Light;

    if (!Light->Atmosphere_Interaction)
    {
      continue;
    }

    switch (Light->Light_Type)
    {
      case CYLINDER_SOURCE:

        if (intersect_cylinderlight(Ray, Light, &t1, &t2))
        {
          if ((t1 < Intersection->Depth) && (t2 > Small_Tolerance))
          {
            insert = TRUE;
          }
        }

        break;

      case POINT_SOURCE:

        t1 = 0.0;
        t2 = Intersection->Depth;

        insert = TRUE;

        break;

      case SPOT_SOURCE:

        if (intersect_spotlight(Ray, Light, &t1, &t2))
        {
          if ((t1 < Intersection->Depth) && (t2 > Small_Tolerance))
          {
            insert = TRUE;
          }
        }

        break;
    }

    /* Insert distances into sampling interval list. */

    if (insert)
    {
      /* Add light color to maximum color. */

      VAddEq(Max_Colour, Light->Colour);

      /* Number of active light sources. */

      active ++;

      /* Insert light source intersections into light list. */

      t1 = max(t1, 0.0);
      t2 = min(t2, Intersection->Depth);

      Light_List[i].active = TRUE;
      Light_List[i].t1 = t1;
      Light_List[i].t2 = t2;

      /* Test if there's an overlapping interval. */

      for (j = 0; j < intervals; j++)
      {
        if (!((t2 < tmin[j]) || (t1 > tmax[j])))
        {
          /* The intervals are overlapping. */

          break;
        }
      }

      if (j >= intervals)
      {
        j = intervals;

        intervals++;
      }

      /* Adjust interval. */

      tmin[j] = min(tmin[j], t1);
      tmax[j] = max(tmax[j], t2);
    }
  }

  /* Initialize maximum color. */

  Max_Colour[RED]   = (Max_Colour[RED]   > EPSILON) ? 1.0 : 0.0;
  Max_Colour[GREEN] = (Max_Colour[GREEN] > EPSILON) ? 1.0 : 0.0;
  Max_Colour[BLUE]  = (Max_Colour[BLUE]  > EPSILON) ? 1.0 : 0.0;

  /*
   * If there are no active lights, we can skip ahead because no
   * light from any light source will reach the viewer.
   */

  Make_Colour(Col_total, 0.0, 0.0, 0.0);

  if (active > 0)
  {
    for (interval = 0; interval < intervals; interval++)
    {
      /* Get sampling step distance. */

      step = min((tmax[interval] - tmin[interval]), Atmosphere->Distance) / (DBL)Atmosphere->Samples;

      /* Set up distance. */

      dist = dist_prev = 0.0;

      /* Sample along the ray. */

      Make_Colour(Col, 0.0, 0.0, 0.0);

      for (i = 0; dist < tmax[interval]; i++)
      {
        /* Step to next sample point. */

        dist = tmin[interval] + ((DBL)i + 0.5) * step;

        /* Get distance of current sampling point. */

        jdist = dist + Atmosphere->Jitter * step * (FRAND() - 0.5);

        /* If we're behind the intersection point we can quit. */

        if (jdist >= Intersection->Depth)
        {
          break;
        }

        sample_atmosphere(Light_List, jdist, Ray, Atmosphere, Col);

        /* Add previous result to the total color. */

        if (i)
        {
          /* Do the current and previous colours differ too much? */

          if ((Atmosphere->AA_Level > 0) && (Colour_Distance(Col, Col_prev) >= Atmosphere->AA_Threshold))
          {
            /* Supersample between current and previous point. */

            supersample_atmosphere(Light_List, 1, Ray, Atmosphere, dist_prev, Col_prev, dist, Col);

            Col[RED]   = Col_prev[RED]   = 0.5 * (Col[RED]   + Col_prev[RED]);
            Col[GREEN] = Col_prev[GREEN] = 0.5 * (Col[GREEN] + Col_prev[GREEN]);
            Col[BLUE]  = Col_prev[BLUE]  = 0.5 * (Col[BLUE]  + Col_prev[BLUE]);
          }

          /* Get attenuation due to distance from view point. */

          k = step * exp(-dist_prev / Atmosphere->Distance);

          /* If the contribution is too small we can stop. */

          if (k < BLACK_LEVEL)
          {
            break;
          }

          Col_total[RED]   += k * Col_prev[RED];
          Col_total[GREEN] += k * Col_prev[GREEN];
          Col_total[BLUE]  += k * Col_prev[BLUE];

          /* If the total color is larger than max. value we can stop now. */

          if ((Col_total[RED]   >= Max_Colour[RED])   &&
              (Col_total[GREEN] >= Max_Colour[GREEN]) &&
              (Col_total[BLUE]  >= Max_Colour[BLUE]))
          {
            break;
          }
        }

        dist_prev = dist;

        Assign_Colour(Col_prev, Col);
      }

      /* Add last result to the total color. */

      /* Get attenuation due to distance from view point. */

      k = step * exp(-dist / Atmosphere->Distance);

      Col_total[RED]   += k * Col_prev[RED];
      Col_total[GREEN] += k * Col_prev[GREEN];
      Col_total[BLUE]  += k * Col_prev[BLUE];

      /* If the total color is white already we can stop now. */

      if ((Col_total[RED] >= 1.0) && (Col_total[GREEN] >= 1.0) && (Col_total[BLUE] >= 1.0))
      {
        break;
      }
    }
  }

  /* Add attenuated background color. */

  k = exp(-Intersection->Depth / Atmosphere->Distance);

  /* Check for minimum transmittance. */

  if (k < Atmosphere->Colour[TRANSM])
  {
    k = Atmosphere->Colour[TRANSM];
  }

/*
  ki = 1.0 - k;

  VLinComb3(Colour, 1.0, Col_total, k, Colour, ki, Atmosphere->Colour);
*/

  /* Attenuate color due to atmosphere color. */

  f = Atmosphere->Colour[FILTER];

  fi = 1.0 - f;

  C1[RED]   = Col_total[RED]   * (fi + f * Atmosphere->Colour[RED]);
  C1[GREEN] = Col_total[GREEN] * (fi + f * Atmosphere->Colour[GREEN]);
  C1[BLUE]  = Col_total[BLUE]  * (fi + f * Atmosphere->Colour[BLUE]);

  C2[RED]   = Colour[RED]   * (fi + f * Atmosphere->Colour[RED]);
  C2[GREEN] = Colour[GREEN] * (fi + f * Atmosphere->Colour[GREEN]);
  C2[BLUE]  = Colour[BLUE]  * (fi + f * Atmosphere->Colour[BLUE]);

  VLinComb2(Colour, 1.0, C1, k, C2);

  Colour[FILTER] = k * Colour[FILTER];
  Colour[TRANSM] = k * Colour[TRANSM];
}



/*****************************************************************************
*
* FUNCTION
*
*   supersample_atmosphere
*
* INPUT
*
*   Light_List   - array containing light source information
*   level        - level of recursion
*   Ray          - pointer to ray
*   Atmosphere   - pointer to atmosphere to use
*   d1           - distance to lower sample
*   d3           - distance to upper sample
*
* OUTPUT
*
*   C1           - Color of lower sample
*   C3           - Color of upper sample
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
*   Nov 1994 : Creation.
*
******************************************************************************/

static void supersample_atmosphere(Light_List, level, Ray, Atmosphere, d1, C1, d3, C3)
LIGHT_LIST *Light_List;
int level;
RAY *Ray;
ATMOSPHERE *Atmosphere;
DBL d1, d3;
COLOUR C1, C3;
{
  DBL d2, jdist;
  COLOUR C2;

  Increase_Counter(stats[Atmosphere_Supersamples]);

  /* Sample between lower and upper point. */

  d2 = 0.5 * (d1 + d3);

  jdist = d2 + Atmosphere->Jitter * 0.5 * (d3 - d1) * (FRAND() - 0.5);

  sample_atmosphere(Light_List, jdist, Ray, Atmosphere, C2);

  /* Test for further supersampling. */

  if (level < Atmosphere->AA_Level)
  {
    if (Colour_Distance(C1, C2) >= Atmosphere->AA_Threshold)
    {
      /* Supersample between lower and middle point. */

      supersample_atmosphere(Light_List, level+1, Ray, Atmosphere, d1, C1, d2, C2);
    }

    if (Colour_Distance(C2, C3) >= Atmosphere->AA_Threshold)
    {
      /* Supersample between current and higher point. */

      supersample_atmosphere(Light_List, level+1, Ray, Atmosphere, d2, C2, d3, C3);
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
*   sample_atmosphere
*
* INPUT
*
*   Light_List   - array containing light source information
*   dist         - distance of current sample
*   Ray          - pointer to ray
*   Atmosphere   - pointer to atmosphere to use
*
* OUTPUT
*
*   Col          - color of current sample
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate the color of the current atmosphere sample, i.e. the
*   sample at distance dist, by summing the light reaching that point
*   from all light sources.
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static void sample_atmosphere(Light_List, dist, Ray, Atmosphere, Col)
LIGHT_LIST *Light_List;
DBL dist;
RAY *Ray;
ATMOSPHERE *Atmosphere;
COLOUR Col;
{
  int i;
  DBL alpha, len, k, g, g2;
  VECTOR P;
  COLOUR Light_Colour;
  RAY Light_Ray;

  Increase_Counter(stats[Atmosphere_Samples]);

  /* Get current sample point. */

  VEvaluateRay(P, Ray->Initial, dist, Ray->Direction);

  /* Process all light sources. */

  Make_Colour(Col, 0.0, 0.0, 0.0);

  for (i = 0; i < Frame.Number_Of_Light_Sources; i++)
  {
    /* Use light only if active and within it's boundaries. */

    if (Light_List[i].active && (dist >= Light_List[i].t1) && (dist <= Light_List[i].t2))
    {
      if (!(Test_Shadow(Light_List[i].Light, &len, &Light_Ray, Ray, P, Light_Colour)))
      {
        VDot(alpha, Light_Ray.Direction, Ray->Direction);

        /* Get attenuation due to scattering. */

        switch (Atmosphere->Type)
        {
          case RAYLEIGH_SCATTERING:

            k = (1.0 + Sqr(alpha)) / 2.0;

            break;

          case MIE_HAZY_SCATTERING:

            k = 0.1 * (1.0 + 0.03515625 * pow(1.0 + alpha, 8.0));

            break;

          case MIE_MURKY_SCATTERING:

            k = 0.019607843 * (1.0 + 1.1641532e-8 * pow(1.0 + alpha, 32.0));

            break;

          case HENYEY_GREENSTEIN_SCATTERING:

            g = Atmosphere->Eccentricity;

            g2 = Sqr(g);

            k = (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * alpha, 1.5);

            break;

          case ISOTROPIC_SCATTERING:
          default:

            k = 1.0;

            break;
        }

        k *= Atmosphere->Scattering;

        VAddScaledEq(Col, k, Light_Colour);
      }
    }
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   do_fog
*
* INPUT
*
*   Ray            - current ray
*   Intersection   - current intersection
*   Light_Ray_Flag - TRUE if ray is a light source ray
*
* OUTPUT
*
*   Colour         - color of current ray
*
* RETURNS
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*
*   Evaluate all fogs for the current ray and intersection.
*
* CHANGES
*
*   Dec 1994 : Rewritten to allow multiple fogs. [DB]
*
*   Apr 1995 : Added transmittance threshold and filtering. [DB]
*
*   Jun 1995 : Added code for alpha channel support. [DB]
*
******************************************************************************/

static void do_fog(Ray, Intersection, Colour, Light_Ray_Flag)
RAY *Ray;
INTERSECTION *Intersection;
COLOUR Colour;
int Light_Ray_Flag;
{
  DBL att, att_inv, width;
  COLOUR Col_Fog;
  COLOUR sum_att;  /* total attenuation. */
  COLOUR sum_col;  /* total color.       */
  FOG *Fog;

  /* Why are we here. */

  if (Frame.Fog == NULL)
  {
    return;
  }

  /* Init total attenuation and total color. */

  Make_ColourA(sum_att, 1.0, 1.0, 1.0, 1.0, 1.0);
  Make_ColourA(sum_col, 0.0, 0.0, 0.0, 0.0, 0.0);

  /* Loop over all fogs. */

  for (Fog = Frame.Fog; Fog != NULL; Fog = Fog->Next)
  {
    /* Don't care about fogs with zero distance. */

    if (fabs(Fog->Distance) > EPSILON)
    {
      width = Intersection->Depth;

      switch (Fog->Type)
      {
        case GROUND_MIST:

          att = ground_fog(Ray, 0.0, width, Fog, Col_Fog);

          break;

        default:

          att = constant_fog(Ray, 0.0, width, Fog, Col_Fog);

          break;
      }

      /* Check for minimum transmittance. */

      if (att < Col_Fog[TRANSM])
      {
        att = Col_Fog[TRANSM];
      }

      /* Get attenuation sum due to filtered/unfiltered translucency. */

      sum_att[RED]    *= att * ((1.0 - Col_Fog[FILTER]) + Col_Fog[FILTER] * Col_Fog[RED]);
      sum_att[GREEN]  *= att * ((1.0 - Col_Fog[FILTER]) + Col_Fog[FILTER] * Col_Fog[GREEN]);
      sum_att[BLUE]   *= att * ((1.0 - Col_Fog[FILTER]) + Col_Fog[FILTER] * Col_Fog[BLUE]);
      sum_att[FILTER] *= att * Col_Fog[FILTER];
      sum_att[TRANSM] *= att * Col_Fog[TRANSM];

      if (!Light_Ray_Flag)
      {
        att_inv = 1.0 - att;

        VAddScaledEq(sum_col, att_inv, Col_Fog);
      }
    }
  }

  /* Add light coming from background. */

  sum_col[RED]    += sum_att[RED]    * Colour[RED];
  sum_col[GREEN]  += sum_att[GREEN]  * Colour[GREEN];
  sum_col[BLUE]   += sum_att[BLUE]   * Colour[BLUE];
  sum_col[FILTER] += sum_att[FILTER] * Colour[FILTER];
  sum_col[TRANSM] += sum_att[TRANSM] * Colour[TRANSM];

  Assign_Colour(Colour, sum_col);
}



/*****************************************************************************
*
* FUNCTION
*
*   do_rainbow
*
* INPUT
*
*   Ray          - Current ray
*   Intersection - Cuurent intersection
*
* OUTPUT
*
*   Colour       - Current colour
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a rainbow using an impressionistic model.
*
*   The model was taken from:
*
*     Musgrave, F. Kenton, "Prisms and Rainbows: a Dispersion Model
*     for Computer Graphics", Proceedings of Graphics Interface '89 -
*     Vision Interface '89, p. 227-234.
*
* CHANGES
*
*   Jul 1994 : Creation.
*
*   Dec 1994 : Modified to allow multiple rainbows. [DB]
*
*   Apr 1995 : Added rainbow arcs and filtering. [DB]
*
*   Jun 1995 : Added code for alpha channel support. [DB]
*
******************************************************************************/

static void do_rainbow(Ray, Intersection, Colour)
RAY *Ray;
INTERSECTION *Intersection;
COLOUR Colour;
{
  int n;
  DBL dot, k, ki, index, x, y, l, angle, fade, f;
  VECTOR Temp;
  COLOUR Cr, Ct;
  RAINBOW *Rainbow;

  /* Why are we here. */

  if (Frame.Rainbow == NULL)
  {
    return;
  }

  Make_ColourA(Ct, 0.0, 0.0, 0.0, 1.0, 1.0);

  n = 0;

  for (Rainbow = Frame.Rainbow; Rainbow != NULL; Rainbow = Rainbow->Next)
  {
    if ((Rainbow->Pigment != NULL) && (Rainbow->Distance != 0.0) && (Rainbow->Width != 0.0))
    {
      /* Get angle between ray direction and rainbow's up vector. */

      VDot(x, Ray->Direction, Rainbow->Right_Vector);
      VDot(y, Ray->Direction, Rainbow->Up_Vector);

      l = Sqr(x) + Sqr(y);

      if (l > 0.0)
      {
        l = sqrt(l);

        y /= l;
      }

      angle = fabs(acos(y));

      if (angle <= Rainbow->Arc_Angle)
      {
        /* Get dot product between ray direction and antisolar vector. */

        VDot(dot, Ray->Direction, Rainbow->Antisolar_Vector);

        if (dot >= 0.0)
        {
          /* Get index ([0;1]) into rainbow's colour map. */

          index = (acos(dot) - Rainbow->Angle) / Rainbow->Width;

          /* Jitter index. */

          if (Rainbow->Jitter > 0.0)
          {
            index += (2.0 * FRAND() - 1.0) * Rainbow->Jitter;
          }

          if ((index >= 0.0) && (index <= 1.0 - EPSILON))
          {
            /* Get colour from rainbow's colour map. */

            Make_Vector(Temp, index, 0.0, 0.0);

            Compute_Pigment(Cr, Rainbow->Pigment, Temp);

            /* Get fading value for falloff. */

            if ((Rainbow->Falloff_Width > 0.0) && (angle > Rainbow->Falloff_Angle))
            {
              fade = (angle - Rainbow->Falloff_Angle) / Rainbow->Falloff_Width;

              fade = (3.0 - 2.0 * fade) * fade * fade;
            }
            else
            {
              fade = 0.0;
            }

            /* Get attenuation factor due to distance. */

            k = exp(-Intersection->Depth / Rainbow->Distance);

            /* Colour's transm value is used as minimum attenuation value. */

            k = max(k, fade * (1.0 - Cr[TRANSM]) + Cr[TRANSM]);

            /* Now interpolate the colours. */

            ki = 1.0 - k;

            /* Attenuate filter value. */

            f = Cr[FILTER] * ki;

            Ct[RED]    += k * Colour[RED]   * ((1.0 - f) + f * Cr[RED])   + ki * Cr[RED];
            Ct[GREEN]  += k * Colour[GREEN] * ((1.0 - f) + f * Cr[GREEN]) + ki * Cr[GREEN];
            Ct[BLUE]   += k * Colour[BLUE]  * ((1.0 - f) + f * Cr[BLUE])  + ki * Cr[BLUE];
            Ct[FILTER] *= k * Cr[FILTER];
            Ct[TRANSM] *= k * Cr[TRANSM];

            n++;
          }
        }
      }
    }
  }

  if (n > 0)
  {
    VInverseScale(Colour, Ct, (DBL)n);

    Colour[FILTER] *= Ct[FILTER];
    Colour[TRANSM] *= Ct[TRANSM];
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   do_skysphere
*
* INPUT
*
*   Ray    - Current ray
*
* OUTPUT
*
*   Colour - Current color
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate color of the sky.
*
*   Use the ray direction as a point on the skysphere. Thus the sky can
*   easily be colored with all kinds of pigments.
*
* CHANGES
*
*   Jul 1994 : Creation.
*
*   Dec 1994 : Modified to allow layered pigments. [DB]
*
*   Jun 1995 : Added code for alpha channel support. [DB]
*
******************************************************************************/

static void do_skysphere(Ray, Colour)
RAY *Ray;
COLOUR Colour;
{
  int i;
  DBL att, trans;
  COLOUR Col, Col_Temp, Filter;
  VECTOR P;
  SKYSPHERE *Skysphere;

  /* Why are we here. */

  if (Frame.Skysphere == NULL)
  {
    return;
  }

  Make_ColourA(Col, 0.0, 0.0, 0.0, 0.0, 0.0);

  if (((Skysphere = Frame.Skysphere) != NULL) && (Skysphere->Pigments != NULL))
  {
    Make_ColourA(Filter, 1.0, 1.0, 1.0, 1.0, 1.0);

    trans = 1.0;

    /* Transform point on unit sphere. */

    if (Skysphere->Trans != NULL)
    {
      MInvTransPoint(P, Ray->Direction, Skysphere->Trans);
    }
    else
    {
      Assign_Vector(P, Ray->Direction);
    }

    for (i = Skysphere->Count-1; i >= 0; i--)
    {
      /* Compute sky colour from colour map. */

      Compute_Pigment(Col_Temp, Skysphere->Pigments[i], P);

      att = trans * (1.0 - Col_Temp[FILTER] - Col_Temp[TRANSM]);

      VAddScaledEq(Col, att, Col_Temp);

      Filter[RED]    *= Col_Temp[RED];
      Filter[GREEN]  *= Col_Temp[GREEN];
      Filter[BLUE]   *= Col_Temp[BLUE];
      Filter[FILTER] *= Col_Temp[FILTER];
      Filter[TRANSM] *= Col_Temp[TRANSM];

      trans = fabs(Filter[FILTER]) + fabs(Filter[TRANSM]);
    }

    Colour[RED]    = Col[RED]    + Colour[RED]   * (Filter[RED]   * Filter[FILTER] + Filter[TRANSM]);
    Colour[GREEN]  = Col[GREEN]  + Colour[GREEN] * (Filter[GREEN] * Filter[FILTER] + Filter[TRANSM]);
    Colour[BLUE]   = Col[BLUE]   + Colour[BLUE]  * (Filter[BLUE]  * Filter[FILTER] + Filter[TRANSM]);
    Colour[FILTER] = Colour[FILTER] * Filter[FILTER];
    Colour[TRANSM] = Colour[TRANSM] * Filter[TRANSM];
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   constant_fog
*
* INPUT
*
*   Ray    - current ray
*   Depth  - intersection depth with fog's boundary
*   Width  - width of the fog along the ray
*   Fog    - current fog
*
* OUTPUT
*
*   Colour - color of the fog
*
* RETURNS
*
* AUTHOR
*
*   POV-Ray Team
*
* DESCRIPTION
*
*   Apply distance attenuated fog.
*
* CHANGES
*
*   Dec 1994 : Modified to work with multiple fogs. [DB]
*
******************************************************************************/

static DBL constant_fog(Ray, Depth, Width, Fog, Colour)
RAY *Ray;
DBL Depth, Width;
FOG *Fog;
COLOUR Colour;
{
  DBL k;
  VECTOR P;

  if (Fog->Turb != NULL)
  {
    Depth += Width / 2.0;

    VEvaluateRay(P, Ray->Initial, Depth, Ray->Direction);

    VEvaluateEq(P, Fog->Turb->Turbulence);

    /* The further away the less influence turbulence has. */

    k = exp(-Width / Fog->Distance);

    Width *= 1.0 - k * min(1.0, Turbulence(P, Fog->Turb)*Fog->Turb_Depth);
  }

  Assign_Colour(Colour, Fog->Colour);

  return (exp(-Width / Fog->Distance));
}



/*****************************************************************************
*
* FUNCTION
*
*   ground_fog
*
* INPUT
*
*   Ray   - current ray
*   Depth - intersection depth with fog's boundary
*   Width - width of the fog along the ray
*   Fog   - current fog
*
* OUTPUT
*
*   Colour - color of the fog
*
* RETURNS
*
* AUTHOR
*
*   Eric Barish
*
* DESCRIPTION
*
*   Here is an ascii graph of the ground fog density, it has a maximum
*   density of 1.0 at Y <= 0, and approaches 0.0 as Y goes up:
*
*   ***********************************
*        |           |            |    ****
*        |           |            |        ***
*        |           |            |           ***
*        |           |            |            | ****
*        |           |            |            |     *****
*        |           |            |            |          *******
*   -----+-----------+------------+------------+-----------+-----
*       Y=-2        Y=-1         Y=0          Y=1         Y=2
*
*   ground fog density is 1 / (Y*Y+1) for Y >= 0 and equals 1.0 for Y <= 0.
*   (It behaves like regular fog for Y <= 0.)
*
*   The integral of the density is atan(Y) (for Y >= 0).
*
* CHANGES
*
*   Feb 1996 : Changed to behave like normal fog for Y <= 0.
*              Fixed bug with reversed offset effect. [DB]
*
******************************************************************************/

static DBL ground_fog(Ray, Depth, Width, Fog, Colour)
RAY *Ray;
DBL Depth, Width;
FOG *Fog;
COLOUR Colour;
{
  DBL fog_density, delta;
  DBL start, end;
  DBL y1, y2, k;
  VECTOR P, P1, P2;

  /* Get start point. */

  VEvaluateRay(P1, Ray->Initial, Depth, Ray->Direction);

  /* Get end point. */

  VLinComb2(P2, 1.0, P1, Width, Ray->Direction);

  /*
   * Could preform transfomation here to translate Start and End
   * points into ground fog space.
   */

  VDot(y1, P1, Fog->Up);
  VDot(y2, P2, Fog->Up);

  start = (y1 - Fog->Offset) / Fog->Alt;
  end   = (y2 - Fog->Offset) / Fog->Alt;

  /* Get integral along y-axis from start to end. */

  if (start <= 0.0)
  {
    if (end <= 0.0)
    {
      fog_density = 1.0;
    }
    else
    {
      fog_density = (atan(end) - start) / (end - start);
    }
  }
  else
  {
    if (end <= 0.0)
    {
      fog_density = (atan(start) - end) / (start - end);
    }
    else
    {
      delta = start - end;

      if (fabs(delta) > EPSILON)
      {
        fog_density = (atan(start) - atan(end)) / delta;
      }
      else
      {
        fog_density = 1.0 / (Sqr(start) + 1.0);
      }
    }
  }

  /* Apply turbulence. */

  if (Fog->Turb != NULL)
  {
    VHalf(P, P1, P2);

    VEvaluateEq(P, Fog->Turb->Turbulence);

    /* The further away the less influence turbulence has. */

    k = exp(-Width / Fog->Distance);

    Width *= 1.0 - k * min(1.0, Turbulence(P, Fog->Turb)*Fog->Turb_Depth);
  }

  Assign_Colour(Colour, Fog->Colour);

  return (exp(-Width * fog_density / Fog->Distance));
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_spotlight
*
* INPUT
*
*   Ray    - current ray
*   Light  - current light source
*
* OUTPUT
*
*   d1, d2 - intersection depths
*
* RETURNS
*
*   int - TRUE, if hit
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Intersect a ray with the light cone of a spotlight.
*
* CHANGES
*
*   Nov 1994 : Creation.
*
******************************************************************************/

static int intersect_spotlight(Ray, Light, d1, d2)
RAY *Ray;
LIGHT_SOURCE *Light;
DBL *d1, *d2;
{
  int viewpoint_is_in_cone;
  DBL a, b, c, d, m, l, l1, l2, t, t1, t2, k1, k2, k3, k4;
  VECTOR V1;

  /* Get cone's slope. Note that cos(falloff) is stored in Falloff! */

  a = acos(Light->Falloff);

  /* This only works for a < 180 degrees! */

  m = tan(a);

  m = 1.0 + Sqr(m);

  VSub(V1, Ray->Initial, Light->Center);

  VDot(k1, Ray->Direction, Light->Direction);

  VDot(k2, V1, Light->Direction);

  VLength(l, V1);

  if (l > EPSILON)
  {
    viewpoint_is_in_cone = (k2 / l >= Light->Falloff);
  }
  else
  {
    viewpoint_is_in_cone = FALSE;
  }

  if ((k1 <= 0.0) && (k2 < 0.0))
  {
    return (FALSE);
  }

  VDot(k3, V1, Ray->Direction);

  VDot(k4, V1, V1);

  a = 1.0 - Sqr(k1) * m;

  b = k3 - k1 * k2 * m;

  c = k4 - Sqr(k2) * m;

  if (a != 0.0)
  {
    d = Sqr(b) - a * c;

    if (d > EPSILON)
    {
      d = sqrt(d);

      t1 = (-b + d) / a;
      t2 = (-b - d) / a;

      if (t1 > t2)
      {
        t = t1; t1 = t2; t2 = t;
      }

      l1 = k2 + t1 * k1;
      l2 = k2 + t2 * k1;

      if ((l1 <= 0.0) && (l2 <= 0.0))
      {
        return (FALSE);
      }

      if ((l1 <= 0.0) || (l2 <= 0.0))
      {
        if (l1 <= 0.0)
        {
          if (viewpoint_is_in_cone)
          {
            t1 = 0.0;
            t2 = (t2 > 0.0) ? (t2) : (Max_Distance);
          }
          else
          {
            t1 = t2;
            t2 = Max_Distance;
          }
        }
        else
        {
          if (viewpoint_is_in_cone)
          {
            t2 = t1;
            t1 = 0.0;
          }
          else
          {
            t2 = Max_Distance;
          }
        }
      }

      *d1 = t1;
      *d2 = t2;

      return (TRUE);
    }
    else
    {
      if (d > -EPSILON)
      {
        if (viewpoint_is_in_cone)
        {
          *d1 = 0.0;
          *d2 = -b / a;
        }
        else
        {
          *d1 = -b / a;
          *d2 = Max_Distance;
        }

        return(TRUE);
      }
    }
  }
  else
  {
    if (viewpoint_is_in_cone)
    {
      *d1 = 0.0;
      *d2 = -c/b;

      return(TRUE);
    }
  }

  return (FALSE);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_cylinderlight
*
* INPUT
*
*   Ray    - current ray
*   Light  - current light source
*
* OUTPUT
*
*   d1, d2 - intersection depths
*
* RETURNS
*
*   int - TRUE, if hit
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Intersect a ray with the light cylinder of a cylinderlight.
*
* CHANGES
*
*   Jan 1995 : Creation.
*
******************************************************************************/

static int intersect_cylinderlight(Ray, Light, d1, d2)
RAY *Ray;
LIGHT_SOURCE *Light;
DBL *d1, *d2;
{
  DBL a, b, c, d, l1, l2, t, t1, t2, k1, k2, k3, k4;
  VECTOR V1;

  VSub(V1, Ray->Initial, Light->Center);

  VDot(k1, Ray->Direction, Light->Direction);

  VDot(k2, V1, Light->Direction);

  if ((k1 <= 0.0) && (k2 < 0.0))
  {
    return (FALSE);
  }

  a = 1.0 - Sqr(k1);

  if (a != 0.0)
  {
    VDot(k3, V1, Ray->Direction);

    VDot(k4, V1, V1);

    b = k3 - k1 * k2;

    c = k4 - Sqr(k2) - Sqr(Light->Falloff);

    d = Sqr(b) - a * c;

    if (d > EPSILON)
    {
      d = sqrt(d);

      t1 = (-b + d) / a;
      t2 = (-b - d) / a;

      if (t1 > t2)
      {
        t = t1; t1 = t2; t2 = t;
      }

      l1 = k2 + t1 * k1;
      l2 = k2 + t2 * k1;

      if ((l1 <= 0.0) && (l2 <= 0.0))
      {
        return (FALSE);
      }

      if ((l1 <= 0.0) || (l2 <= 0.0))
      {
        if (l1 <= 0.0)
        {
          t1 = 0.0;
        }
        else
        {
          t2 = (Max_Distance - k2) / k1;
        }
      }

      *d1 = t1;
      *d2 = t2;

      return (TRUE);
    }
  }

  return (FALSE);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Atmosphere
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   ATMOSPHERE * - created atmosphere
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create an atmosphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

ATMOSPHERE *Create_Atmosphere()
{
  ATMOSPHERE *New;

  New = (ATMOSPHERE *)POV_MALLOC(sizeof(ATMOSPHERE), "fog");

  New->Type = ISOTROPIC_SCATTERING;

  New->Samples            = 100;
  New->Distance           = 0.0;
  New->Distance_Threshold = 0.005;
  New->Scattering         = 1.0;
  New->Eccentricity       = 0.0;

  Make_Colour(New->Colour, 0.0, 0.0, 0.0);

  New->AA_Level     = 0;
  New->AA_Threshold = 0.3;
  New->Jitter       = 0.0;

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Atmosphere
*
* INPUT
*
*   Old - atmosphere to copy
*
* OUTPUT
*
* RETURNS
*
*   ATMOSPHERE * - new atmosphere
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy an atmosphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void *Copy_Atmosphere(Old)
ATMOSPHERE *Old;
{
  ATMOSPHERE *New;

  New = Create_Atmosphere();

  *New = *Old;

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Atmosphere
*
* INPUT
*
*   Atmosphere - atmosphere to destroy
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
*   Destroy an atmosphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Atmosphere(Atmosphere)
ATMOSPHERE *Atmosphere;
{
  if (Atmosphere != NULL)
  {
    POV_FREE(Atmosphere);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Fog
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   FOG * - created fog
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a fog.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

FOG *Create_Fog()
{
  FOG *New;

  New = (FOG *)POV_MALLOC(sizeof(FOG), "fog");

  New->Type = ORIG_FOG;

  New->Distance = 0.0;
  New->Alt      = 0.0;
  New->Offset   = 0.0;

  Make_ColourA(New->Colour, 0.0, 0.0, 0.0, 0.0, 0.0);

  Make_Vector(New->Up, 0.0, 1.0, 0.0);

  New->Turb = NULL;
  New->Turb_Depth = 0.5;

  New->Next = NULL;

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Fog
*
* INPUT
*
*   Old - fog to copy
*
* OUTPUT
*
* RETURNS
*
*   FOG * - new fog
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy a fog.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void *Copy_Fog(Old)
FOG *Old;
{
  FOG *New;

  New = Create_Fog();

  *New = *Old;

  New->Turb = (TURB *)Copy_Warps(((WARP *)Old->Turb));

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Fog
*
* INPUT
*
*   Fog - fog to destroy
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
*   Destroy a fog.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Fog(Fog)
FOG *Fog;
{
  if (Fog != NULL)
  {
    Destroy_Turb(Fog->Turb);

    POV_FREE(Fog);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Rainbow
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   RAINBOW * - created rainbow
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a rainbow.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

RAINBOW *Create_Rainbow()
{
  RAINBOW *New;

  New = (RAINBOW *)POV_MALLOC(sizeof(RAINBOW), "fog");

  New->Distance = Max_Distance;
  New->Jitter   = 0.0;
  New->Angle    = 0.0;
  New->Width    = 0.0;

  New->Falloff_Width  = 0.0;
  New->Arc_Angle      = 180.0;
  New->Falloff_Angle  = 180.0;

  New->Pigment = NULL;

  Make_Vector(New->Antisolar_Vector, 0.0, 0.0, 0.0);

  Make_Vector(New->Right_Vector, 1.0, 0.0, 0.0);
  Make_Vector(New->Up_Vector, 0.0, 1.0, 0.0);

  New->Next = NULL;

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Rainbow
*
* INPUT
*
*   Old - rainbow to copy
*
* OUTPUT
*
* RETURNS
*
*   RAINBOW * - new rainbow
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy a rainbow.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void *Copy_Rainbow(Old)
RAINBOW *Old;
{
  RAINBOW *New;

  New = Create_Rainbow();

  *New = *Old;

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Rainbow
*
* INPUT
*
*   Rainbow - rainbow to destroy
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
*   Destroy a rainbow.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Rainbow(Rainbow)
RAINBOW *Rainbow;
{
  if (Rainbow != NULL)
  {
    Destroy_Pigment(Rainbow->Pigment);

    POV_FREE(Rainbow);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Skysphere
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   SKYSPHERE * - created skysphere
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Create a skysphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

SKYSPHERE *Create_Skysphere()
{
  SKYSPHERE *New;

  New = (SKYSPHERE *)POV_MALLOC(sizeof(SKYSPHERE), "fog");

  New->Count = 0;

  New->Pigments = NULL;

  New->Trans = Create_Transform();

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Skysphere
*
* INPUT
*
*   Old - skysphere to copy
*
* OUTPUT
*
* RETURNS
*
*   SKYSPHERE * - copied skysphere
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Copy a skysphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void *Copy_Skysphere(Old)
SKYSPHERE *Old;
{
  int i;
  SKYSPHERE *New;

  New = Create_Skysphere();

  Destroy_Transform(New->Trans);

  *New = *Old;

  New->Trans = Copy_Transform(Old->Trans);

  if (New->Count > 0)
  {
    New->Pigments = (PIGMENT **)POV_MALLOC(New->Count*sizeof(PIGMENT *), "skysphere pigment");

    for (i = 0; i < New->Count; i++)
    {
      New->Pigments[i] = Copy_Pigment(Old->Pigments[i]);
    }
  }

  return (New);
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Skysphere
*
* INPUT
*
*   Skysphere - skysphere to destroy
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
*   Destroy a skysphere.
*
* CHANGES
*
*   Dec 1994 : Creation.
*
******************************************************************************/

void Destroy_Skysphere(Skysphere)
SKYSPHERE *Skysphere;
{
  int i;

  if (Skysphere != NULL)
  {
    for (i = 0; i < Skysphere->Count; i++)
    {
      Destroy_Pigment(Skysphere->Pigments[i]);
    }

    POV_FREE(Skysphere->Pigments);

    Destroy_Transform(Skysphere->Trans);

    POV_FREE(Skysphere);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Skysphere
*
* INPUT
*
*   Vector - Rotation vector
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Rotate a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Rotate_Skysphere(Skysphere, Vector)
SKYSPHERE *Skysphere;
VECTOR Vector;
{
  TRANSFORM Trans;

  Compute_Rotation_Transform(&Trans, Vector);

  Transform_Skysphere(Skysphere, &Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Skysphere
*
* INPUT
*
*   Vector - Scaling vector
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Scale a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Scale_Skysphere(Skysphere, Vector)
SKYSPHERE *Skysphere;
VECTOR Vector;
{
  TRANSFORM Trans;

  Compute_Scaling_Transform(&Trans, Vector);

  Transform_Skysphere(Skysphere, &Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Skysphere
*
* INPUT
*
*   Vector - Translation vector
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Translate a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Translate_Skysphere(Skysphere, Vector)
SKYSPHERE *Skysphere;
VECTOR Vector;
{
  TRANSFORM Trans;

  Compute_Translation_Transform(&Trans, Vector);

  Transform_Skysphere(Skysphere, &Trans);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Skysphere
*
* INPUT
*
*   Trans  - Pointer to transformation
*
* OUTPUT
*
*   Skysphere - Pointer to skysphere structure
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Transform a skysphere.
*
* CHANGES
*
*   Feb 1996 : Creation.
*
******************************************************************************/

void Transform_Skysphere(Skysphere, Trans)
SKYSPHERE *Skysphere;
TRANSFORM *Trans;
{
  if (Skysphere->Trans == NULL)
  {
    Skysphere->Trans = Create_Transform();
  }

  Compose_Transforms(Skysphere->Trans, Trans);
}



