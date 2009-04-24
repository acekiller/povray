/****************************************************************************
*                   optout.cpp
*
*  This module contains functions for credit, usage and options.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996-2002 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file.
*  If POVLEGAL.DOC is not available it may be found online at -
*
*    http://www.povray.org/povlegal.html.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
* $File: //depot/povray/3.5/source/optout.cpp $
* $Revision: #30 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "atmosph.h"
#include "bezier.h"
#include "blob.h"
#include "bbox.h"
#include "cones.h"
#include "csg.h"
#include "discs.h"
#include "fractal.h"
#include "hfield.h"
#include "lathe.h"
#include "lighting.h"
#include "mesh.h"
#include "polysolv.h"
#include "objects.h"
#include "parse.h"
#include "point.h"
#include "poly.h"
#include "polygon.h"
#include "octree.h"
#include "quadrics.h"
#include "pgm.h"
#include "ppm.h"
#include "prism.h"
#include "radiosit.h"
#include "render.h"
#include "sor.h"
#include "spheres.h"
#include "super.h"
#include "targa.h"
#include "texture.h"
#include "torus.h"
#include "triangle.h"
#include "truetype.h"
#include "userio.h"
#include "userdisp.h"
#include "lbuffer.h"
#include "vbuffer.h"
#include "povray.h"
#include "optin.h"
#include "optout.h"
#include "povmsgid.h"
#include "isosurf.h"

// these are needed for copyright notices and version numbers
#include "zlib.h"
#include "png.h"
#include "jversion.h"

// Including tiffio.h causes the Windows compile to break. As all we need is the
// version function, we just declare it here.
//#define __STDC__
//#include "tiffio.h"

extern "C" const char* TIFFGetVersion(void);

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define NUMBER_LENGTH 19
#define OUTPUT_LENGTH 15

#define NUMBER_OF_AUTHORS_ACROSS  4


/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Local variables
******************************************************************************/


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/

char *Current_Primary_Developers[] =
{
  "Chris Cason",
  "Thorsten Froehlich",
  "Nathan Kopp",
  "Ron Parker",
  NULL
} ;

char *Primary_Developers[] =
{
  "Steve Anger",
  "Dieter Bayer",
  "Chris Dailey",
  "Andreas Dilger",
  "Steve Demlow",
  "Alexander Enzmann",
  "Dan Farmer",
  "Eduard Schwan",
  "Timothy Wegner",
  "Chris Young",
  NULL   /* NULL flags the end of the list */
};

char *Contributing_Authors[] =
{
  "Steve A. Bennett",
  "David K. Buck",
  "Aaron A. Collins",
  "Mark Gordon",
  "Pascal Massimino",
  "Jim McElhiney",
  "Douglas Muir",
  "Bill Pulver",
  "Robert Skinner",
  "Zsolt Szalavari",
  "Scott Taylor",
  "Drew Wells",
  NULL   /* NULL flags the end of the list */
};

INTERSECTION_STATS_INFO intersection_stats[kPOVList_Stat_Last] =
{
	{ kPOVList_Stat_RBezierTest,		Ray_RBezier_Tests, Ray_RBezier_Tests_Succeeded,
	  "Bezier Patch" },
	{ kPOVList_Stat_BicubicTest,		Ray_Bicubic_Tests, Ray_Bicubic_Tests_Succeeded,
	  "Bicubic Patch" },
	{ kPOVList_Stat_BlobTest,			Ray_Blob_Tests, Ray_Blob_Tests_Succeeded,
	  "Blob" },
	{ kPOVList_Stat_BlobCpTest,			Blob_Element_Tests, Blob_Element_Tests_Succeeded,
	  "Blob Component" },
	{ kPOVList_Stat_BlobBdTest,			Blob_Bound_Tests, Blob_Bound_Tests_Succeeded,
	  "Blob Bound" },
	{ kPOVList_Stat_BoxTest,			Ray_Box_Tests, Ray_Box_Tests_Succeeded,
	  "Box" },
	{ kPOVList_Stat_ConeCylTest,		Ray_Cone_Tests, Ray_Cone_Tests_Succeeded,
	  "Cone/Cylinder" },
	{ kPOVList_Stat_CSGIntersectTest,	Ray_CSG_Intersection_Tests, Ray_CSG_Intersection_Tests_Succeeded,
	  "CSG Intersection" },
	{ kPOVList_Stat_CSGMergeTest,		Ray_CSG_Merge_Tests, Ray_CSG_Merge_Tests_Succeeded,
	  "CSG Merge" },
	{ kPOVList_Stat_CSGUnionTest,		Ray_CSG_Union_Tests, Ray_CSG_Union_Tests_Succeeded,
	  "CSG Union" },
	{ kPOVList_Stat_DiscTest,			Ray_Disc_Tests, Ray_Disc_Tests_Succeeded,
	  "Disc" },
	{ kPOVList_Stat_FractalTest,		Ray_Fractal_Tests, Ray_Fractal_Tests_Succeeded,
	  "Fractal" },
	{ kPOVList_Stat_HFTest,				Ray_HField_Tests, Ray_HField_Tests_Succeeded,
	  "Height Field" },
	{ kPOVList_Stat_HFBoxTest,			Ray_HField_Box_Tests, Ray_HField_Box_Tests_Succeeded,
	  "Height Field Box" },
	{ kPOVList_Stat_HFTriangleTest,		Ray_HField_Triangle_Tests, Ray_HField_Triangle_Tests_Succeeded,
	  "Height Field Triangle" },
	{ kPOVList_Stat_HFBlockTest,		Ray_HField_Block_Tests, Ray_HField_Block_Tests_Succeeded,
	  "Height Field Block" },
	{ kPOVList_Stat_HFCellTest,			Ray_HField_Cell_Tests, Ray_HField_Cell_Tests_Succeeded,
	  "Height Field Cell" },
	{ kPOVList_Stat_IsosurfaceTest,		Ray_IsoSurface_Tests, Ray_IsoSurface_Tests_Succeeded,
	  "Isosurface" },
	{ kPOVList_Stat_IsosurfaceBdTest,	Ray_IsoSurface_Bound_Tests, Ray_IsoSurface_Bound_Tests_Succeeded,
	  "Isosurface Container" },
	{ kPOVList_Stat_IsosurfaceCacheTest,Ray_IsoSurface_Cache, Ray_IsoSurface_Cache_Succeeded,
	  "Isosurface Cache" },
	{ kPOVList_Stat_LatheTest,			Ray_Lathe_Tests, Ray_Lathe_Tests_Succeeded,
	  "Lathe" },
	{ kPOVList_Stat_LatheBdTest,		Lathe_Bound_Tests, Lathe_Bound_Tests_Succeeded,
	  "Lathe Bound" },
	{ kPOVList_Stat_MeshTest,			Ray_Mesh_Tests, Ray_Mesh_Tests_Succeeded,
	  "Mesh" },
	{ kPOVList_Stat_PlaneTest,			Ray_Plane_Tests, Ray_Plane_Tests_Succeeded,
	  "Plane" },
	{ kPOVList_Stat_PolygonTest,		Ray_Polygon_Tests, Ray_Polygon_Tests_Succeeded,
	  "Polygon" },
	{ kPOVList_Stat_PrismTest,			Ray_Prism_Tests, Ray_Prism_Tests_Succeeded,
	  "Prism" },
	{ kPOVList_Stat_PrismBdTest,		Prism_Bound_Tests, Prism_Bound_Tests_Succeeded,
	  "Prism Bound" },
	{ kPOVList_Stat_ParametricTest,		Ray_Parametric_Tests, Ray_Parametric_Tests_Succeeded,
	  "Parametric" },
	{ kPOVList_Stat_ParametricBoxTest,	Ray_Par_Bound_Tests, Ray_Par_Bound_Tests_Succeeded,
	  "Parametric Bound" },
	{ kPOVList_Stat_QuardicTest,		Ray_Quadric_Tests, Ray_Quadric_Tests_Succeeded,
	  "Quadric" },
	{ kPOVList_Stat_QuadPolyTest,		Ray_Poly_Tests, Ray_Poly_Tests_Succeeded,
	  "Quartic/Poly" },
	{ kPOVList_Stat_SphereTest,			Ray_Sphere_Tests, Ray_Sphere_Tests_Succeeded,
	  "Sphere" },
	{ kPOVList_Stat_SphereSweepTest,	Ray_Sphere_Sweep_Tests, Ray_Sphere_Sweep_Tests_Succeeded,
	  "Sphere Sweep" },
	{ kPOVList_Stat_SuperellipsTest,	Ray_Superellipsoid_Tests, Ray_Superellipsoid_Tests_Succeeded,
	  "Superellipsoid" },
	{ kPOVList_Stat_SORTest,			Ray_Sor_Tests, Ray_Sor_Tests_Succeeded,
	  "Surface of Revolution" },
	{ kPOVList_Stat_SORBdTest,			Sor_Bound_Tests, Sor_Bound_Tests_Succeeded,
	  "Surface of Rev. Bound" },
	{ kPOVList_Stat_TorusTest,			Ray_Torus_Tests, Ray_Torus_Tests_Succeeded,
	  "Torus" },
	{ kPOVList_Stat_TorusBdTest,		Torus_Bound_Tests, Torus_Bound_Tests_Succeeded,
	  "Torus Bound" },
	{ kPOVList_Stat_TriangleTest,		Ray_Triangle_Tests, Ray_Triangle_Tests_Succeeded,
	  "Triangle" },
	{ kPOVList_Stat_TTFontTest,			Ray_TTF_Tests, Ray_TTF_Tests_Succeeded,
	  "True Type Font" },
	{ kPOVList_Stat_BoundObjectTest,	Bounding_Region_Tests, Bounding_Region_Tests_Succeeded,
	  "Bounding Object" },
	{ kPOVList_Stat_ClipObjectTest,		Clipping_Region_Tests, Clipping_Region_Tests_Succeeded,
	  "Clipping Object" },
	{ kPOVList_Stat_BoundingBoxTest,	nChecked, nEnqueued,
	  "Bounding Box" },
	{ kPOVList_Stat_LightBufferTest,	LBuffer_Tests, LBuffer_Tests_Succeeded,
	  "Light Buffer" },
	{ kPOVList_Stat_VistaBufferTest,	VBuffer_Tests, VBuffer_Tests_Succeeded,
	  "Vista Buffer" },
	{ kPOVList_Stat_Last, MaxStat, MaxStat, NULL }
};

/*****************************************************************************
* Static functions
******************************************************************************/

void rinfo_on(char *string, unsigned value);
const char *Extract_Version(const char *str);



/*****************************************************************************
*
* FUNCTION
*
*   rinfo_on
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

void rinfo_on(char *string, unsigned value)
{
  if (value)
  {
    Render_Info("%s.On",(string));
  }
  else
  {
    Render_Info("%sOff",(string));
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Print_Credits
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

void Print_Credits()
{
  Banner ("Persistence of Vision(tm) Ray Tracer Version %s %s (%s)\n", POV_RAY_VERSION, POVRAY_PLATFORM_NAME, COMPILER_VER);
  Banner ("  %s\n", DISTRIBUTION_MESSAGE_1);
  Banner ("  %s\n", DISTRIBUTION_MESSAGE_2);
  Banner ("  %s\n", DISTRIBUTION_MESSAGE_3);
  Banner ("%s\n", POV_RAY_COPYRIGHT);
}



/*****************************************************************************
*
* FUNCTION
*
*   Print_Help_Screens
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
*   Print all help screens. Use an interactive menu if GET_KEY exists.
*
* CHANGES
*
*   Apr 1995 : Creation.
*
******************************************************************************/

void Print_Help_Screens()
{
#ifdef GET_KEY_EXISTS
  char c;
  int n, x, ok;

  Usage(-1, false);

  for (n = 0; ; )
  {
    Banner("\n");
    Banner("[ Press 0 for general help, 1 to %d for help screen. Press 'q' to quit. ]", MAX_HELP_PAGE);

    do
    {
      ok = false;

      GET_KEY(x);

      c = (char)x;

      if ((c >= '0') && (c <= '0' + MAX_HELP_PAGE))
      {
        ok = true;

        n = (int)c - (int)'0';
      }
      else
      {
        if ((c == 'q') || (c == 'Q'))
        {
          ok = true;
        }
      }
    }
    while(!ok);

    Banner("\n");

    if ((c == 'q') || (c == 'Q'))
    {
      break;
    }

    Usage(n, false);
  }

#else
  int n;

  for (n = -1; n <= MAX_HELP_PAGE; n++)
  {
    Usage(n, (n == MAX_HELP_PAGE));
  }
#endif
}



/*****************************************************************************
*
* FUNCTION
*
*   Usage
*
* INPUT
*
*   n - Number of usage screen
*   f - Flag to terminate
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
*   Print out usage messages.
*
* CHANGES
*
*   Dec 1994 : Changed to show options depending on parameter n. [DB]
*
*   Feb 1995 : Changed to terminate only if f != 0. [DB]
*
******************************************************************************/

void Usage(int n, int  f)
{
  switch (n)
  {
    /* Help screen. */

    case 0:

      Banner("\n");
      Banner("Usage: POVRAY [+/-]Option1 [+/-]Option2 ... (-h or -? for help)\n");
      Banner("\n");
      Banner("  Example: POVRAY scene.ini +Iscene.pov +Oscene.tga +W320 +H200\n");
      Banner("  Example: POVRAY +Iscene.pov +Oscene.tga +W160 +H200 +V -D +X\n");
      Banner("\n");
      Banner("The help screen is divided into several parts. To access one part\n");
      Banner("just enter the number of the screen after the -? option or the\n");
      Banner("-help option.\n");
      Banner("\n");
      Banner("E.g. use -?5 or -help5 to see the help screen about the tracing\n");
      Banner("options.\n");
      Banner("\n");
      Banner("  Number  Part\n");
      Banner("    1     Parsing Options\n");
      Banner("    2     Output Options\n");
      Banner("    3     Output Options - display related\n");
      Banner("    4     Output Options - file related\n");
      Banner("    5     Tracing Options\n");
      Banner("    6     Animation Options\n");
      Banner("    7     Redirecting Options\n");

      break;

    /* Parsing options. */

    case 1:

      Banner("\n");
      Banner("Parsing options\n");
      Banner("\n");
      Banner("  I<name> = input file name\n");
      Banner("  HI<name>= header include file name\n");
      Banner("  L<name> = library path prefix\n");
      Banner("  MVn.n   = set compability to version n.n\n");
      Banner("  SU      = split bounded unions if children are finite\n");
      Banner("  UR      = remove unnecessary bounding objects\n");

      break;

    /* Output options. */

    case 2:

      Banner("\n");
      Banner("Output options\n");
      Banner("\n");
      Banner("  Hnnn    = image height\n");
      Banner("  Wnnn    = image width\n");
      Banner("\n");
      Banner("  SRnn    = start at row nn | SR0.nn start row at nn percent of screen\n");
      Banner("  ERnn    = end   at row nn | ER0.nn end   row at nn percent of screen\n");
      Banner("  SCnn    = start at col nn | SC0.nn start col at nn percent of screen\n");
      Banner("  ECnn    = end   at col nn | EC0.nn end   col at nn percent of screen\n");
      Banner("\n");
      Banner("  C       = continue aborted trace\n");
      Banner("  P       = pause before exit\n");
      Banner("  V       = verbose messages on\n");
      Banner("  Xnnn    = enable early exit by key hit (every nnn pixels)\n");

      break;

    case 3:

      Banner("\n");
      Banner("Output options - display related\n");
      Banner("\n");
      Banner("  Dxy     = display in format x, using palette option y\n");
      Banner("  SPnnn   = Mosaic Preview display, Start grid size = 2, 4, 8, 16, ...\n");
      Banner("  EPnnn   = Mosaic Preview display, End grid size   = 2, 4, 8, 16, ...\n");
      Banner("  UD      = draw vista rectangles\n");

      break;

    /* Output options - file related. */

    case 4:

      Banner("\n");
      Banner("Output options - file related\n");
      Banner("\n");
      Banner("  Bnnn    = Use nnn KB for output file buffer\n");
      Banner("  Fx      = write output file in format x\n");
      Banner("            FC  - Compressed Targa with 24 or 32 bpp\n");
      Banner("            FNn - PNG (n bits/color, n = 5 to 16, default is 8)\n");
//      Banner("            FJn - JPEG (n compression quality, n = 0 to 100, default is 100)\n");
      Banner("            FP  - PPM\n");
      Banner("            FS  - System specific\n");
      Banner("            FT  - Uncompressed Targa with 24 or 32 bpp\n");
      Banner("  O<name> = output file name\n");
#if PRECISION_TIMER_AVAILABLE
      Banner("\n");
      Banner("  HTx     = write CPU utilization histogram in format x\n");
      Banner("            HTC - Comma separated values (CSV - spreadsheet)\n");
      Banner("            HTN - PNG grayscale\n");
      Banner("            HTP - PPM heightfield\n");
      Banner("            HTS - System specific\n");
      Banner("            HTT - Uncompressed TGA heightfield\n");
      Banner("            HTX - No histogram output\n");
      Banner("  HN<name>= histogram filename\n");
      Banner("  HSx.y   = histogram grid number of x, y divisions\n");
#endif

      break;

    /* Tracing options. */

    case 5:

      Banner("\n");
      Banner("Tracing options\n");
      Banner("\n");
      Banner("  MBnnn   = use slabs if more than nnn objects\n");
      Banner("  Qn      = image quality (0 = rough, 9 = full, R = radiosity)\n");
      Banner("  QR      = enable radiosity calculations for ambient light\n");
      Banner("\n");
      Banner("  A0.n    = perform antialiasing\n");
      Banner("  AMn     = use non-adaptive (n=1) or adaptive (n=2) supersampling\n");
      Banner("  Jn.n    = set antialiasing-jitter amount\n");
      Banner("  Rn      = set antialiasing-depth (use n X n rays/pixel)\n");
      Banner("\n");
      Banner("  UL      = use light buffer\n");
      Banner("  UV      = use vista buffer\n");

      break;

    /* Animation options. */

    case 6:

      Banner("\n");
      Banner("Animation options\n");
      Banner("\n");
      Banner("  Kn.n      = set frame clock to n.n\n");
      Banner("  KFInnn    = Initial frame number\n");
      Banner("  KFFnnn    = Final frame number\n");
      Banner("  KInnn.nn  = Initial clock value\n");
      Banner("  KFnnn.nn  = Final clock value\n");
      Banner("  SFnn      = Start subset at frame nn\n");
      Banner("  SF0.nn    = Start subset nn percent into sequence\n");
      Banner("  EFnn      = End subset at frame nn\n");
      Banner("  EF0.n     = End subset nn percent into sequence\n");
      Banner("  KC        = Calculate clock value for cyclic animation\n");
      Banner("\n");
      Banner("  UF        = use field rendering\n");
      Banner("  UO        = use odd lines in odd frames\n");

      break;

    /* Redirecting options. */

    case 7:

      Banner("\n");
      Banner("Redirecting options\n");
      Banner("\n");
      Banner("  GI<name>= write all .INI parameters to file name\n");
      Banner("  Gx<name>= write stream x to console and/or file name\n");
      Banner("            GA - All streams (except status)\n");
      Banner("            GD - Debug stream\n");
      Banner("            GF - Fatal stream\n");
      Banner("            GR - Render stream\n");
      Banner("            GS - Statistics stream\n");
      Banner("            GW - Warning stream\n");

      break;

    /* Usage ... */

    default:

      Print_Authors();
  }

#if defined(WAIT_FOR_KEYPRESS_EXISTS) && !defined(GET_KEY_EXISTS)
  Banner("\n");
  Banner("[ Paused for keypress... ]");

  WAIT_FOR_KEYPRESS;

  Banner("\n");
#endif
}


void Print_Authors()
{
  int h, i, j;

  Banner("\n");
  Banner("POV-Ray is based on DKBTrace 2.12 by David K. Buck & Aaron A. Collins.\n");
  Banner("\n");
  Banner("Primary POV-Ray 3.5 Developers: (Alphabetically)\n");

  for (i = h = 0; Current_Primary_Developers[h] != NULL; i++)
  {
    for (j = 0; (j < NUMBER_OF_AUTHORS_ACROSS) && (Current_Primary_Developers[h] != NULL); j++)
    {
        Banner(" %-19s", Current_Primary_Developers[h++]);
    }

    Banner("\n");
  }

  Banner("\n");
  Banner("Primary POV-Ray 3.0 & 3.1 Developers: (Alphabetically)\n");

  for (i = h = 0; Primary_Developers[h] != NULL; i++)
  {
    for (j = 0; (j < NUMBER_OF_AUTHORS_ACROSS) && (Primary_Developers[h] != NULL); j++)
    {
        Banner(" %-19s", Primary_Developers[h++]);
    }

    Banner("\n");
  }

  Banner("\n");
  Banner("Major Contributing Authors: (Alphabetically)\n");

  for (i = h = 0; Contributing_Authors[h] != NULL; i++)
  {
    for (j = 0; (j < NUMBER_OF_AUTHORS_ACROSS) && (Contributing_Authors[h] != NULL); j++)
    {
        Banner(" %-19s", Contributing_Authors[h++]);
    }

    Banner("\n");
  }

  Banner("\n");
  Banner("Other contributors listed in the documentation.\n");

  Banner("\n");
  Banner("Support libraries used by POV-Ray:\n");
  // ZLib library version and copyright notice
  Banner(" ZLib %s, Copyright 1995-1998 Jean-loup Gailly and Mark Adler\n", Extract_Version(zlibVersion()));
  // LibPNG library version and copyright notice
  Banner(" LibPNG %s, Copyright 2000, 2001 Glenn Randers-Pehrson\n", Extract_Version(png_get_libpng_ver(NULL)));
  // LibJPEG library version and copyright notice
  Banner(" LibJPEG %s, Copyright 1998 Thomas G. Lane\n", Extract_Version(JVERSION));
  // LibTIFF library version and copyright notice
  Banner(" LibTIFF %s, Copyright 1988-1997 Sam Leffler, 1991-1997 Silicon Graphics, Inc.\n", Extract_Version(TIFFGetVersion()));
}

/*****************************************************************************
*
* FUNCTION
*
*   ExtractVersion
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

char LibVersionStringBuffer[20];

const char *Extract_Version(const char *str)
{
	int pos = 0;

	for(; *str != 0; str++)
	{
		if(isdigit(*str))
		{
			while(((isalnum(*str)) || (*str == '.')) && (pos < 10))
			{
				LibVersionStringBuffer[pos] = *str;
				str++;
				pos++;
			}
			break;
		}
	}

	LibVersionStringBuffer[pos] = 0;

	return LibVersionStringBuffer;
}


/*****************************************************************************
*
* FUNCTION
*
*   Print_Options
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

void Print_Options()
{
  int i, j;

  /* Print parsing options. */

  Render_Info("Parsing Options\n");

  Render_Info("  Input file: %s", opts.Input_File_Name);
  Render_Info(" (compatible to version %g)\n", (DBL)(opts.Language_Version / 100.0));

  rinfo_on   ("  Remove bounds.......", (opts.Options & REMOVE_BOUNDS));
  rinfo_on   ("  Split unions........", (opts.Options & SPLIT_UNION));
  Render_Info("\n");

  j = 17;

  Render_Info("  Library paths:");

  for (i = 0; i < opts.Library_Path_Index; i++)
  {
    j += strlen(opts.Library_Paths[i])+2;

    if (j > 77)
    {
      Render_Info("\n   ");

      j = strlen(opts.Library_Paths[i]) + 5;
    }

    Render_Info(" %s", opts.Library_Paths[i]);
  }

  Render_Info("\n");

  /* Print output options. */

  Render_Info("Output Options\n");

  Render_Info("  Image resolution %d by %d",
                                    Frame.Screen_Width, Frame.Screen_Height);
  Render_Info(" (rows %d to %d, columns %d to %d).\n",
                       opts.First_Line+1, opts.Last_Line,
                       opts.First_Column+1, opts.Last_Column);

  if (opts.Options & DISKWRITE)
  {
    Render_Info("  Output file: %s%s, ", opts.Output_Path, opts.Output_File_Name);

    if (toupper(opts.OutputFormat) == 'N' && opts.Options & HF_GRAY_16)
      Render_Info("%d bpp", opts.OutputQuality);
	else if(toupper(opts.OutputFormat) == 'J')
      Render_Info("%d bpp, quality %d%s", ((opts.Options & OUTPUT_ALPHA) ? 4 : 3) * 8, opts.OutputQuality, "%");
    else
      Render_Info("%d bpp", ((opts.Options & OUTPUT_ALPHA) ? 4 : 3) * opts.OutputQuality);

    Render_Info(" ");

    switch (toupper(opts.OutputFormat))
    {
      case 'C': Render_Info("RLE Targa");       break;
      case 'N': Render_Info("PNG");             break;
      case 'J': Render_Info("JPEG");            break;
      case 'P': Render_Info("PPM");             break;
      case 'S': Render_Info("(system format)"); break;
      case 'T': Render_Info("Targa");           break;
    }

    if (opts.Options & HF_GRAY_16)
    {
      if (toupper(opts.OutputFormat) == 'N')
      {
        Render_Info(" grayscale");
      }
      else if (opts.Options & HF_GRAY_16)
      {
        Render_Info(" POV heightfield");
      }
    }

    if (opts.Options & OUTPUT_ALPHA)
    {
      Render_Info(" with alpha");
    }

    if ((opts.Options & BUFFERED_OUTPUT) && (opts.File_Buffer_Size != 0))
    {
      Render_Info(", %d KByte buffer\n", opts.File_Buffer_Size/1024);
    }
    else
    {
      Render_Info("\n");
    }
  }
  else if (opts.Options & DISPLAY)
  {
    if (opts.histogram_on == true)
    {
      Warning(0, "  Rendering to screen and histogram file only.");
    }
    else
    {
      Warning(0, "  Rendering to screen only. No file output.");
    }
  }
  else if (opts.histogram_on == true)
  {
    Warning(0, "  Rendering to histogram file only.");
  }
  else
  {
    PossibleError("  Rendering to nothing! No screen or file output.");
  }

  rinfo_on   ("  Graphic display.....", (opts.Options & DISPLAY));
  if (opts.Options & DISPLAY)
  {
    Render_Info("  (type: %c, palette: %c, gamma: %4.2g)",
               toupper(opts.DisplayFormat), toupper(opts.PaletteOption),
               opts.DisplayGamma);
  }
  Render_Info("\n");

  rinfo_on   ("  Mosaic preview......", (opts.Options & PREVIEW));
  if (opts.Options & PREVIEW)
  {
    Render_Info("  (pixel sizes %d to %d)",
               opts.PreviewGridSize_Start, opts.PreviewGridSize_End);
  }
  Render_Info("\n");

#if PRECISION_TIMER_AVAILABLE
  rinfo_on ("  CPU usage histogram.", opts.histogram_on);
  if (opts.histogram_on)
  {
    char *type;

    switch (opts.histogram_type)
    {
      case CSV:   type = "CSV"; break;
      case TARGA: type = "TGA"; break;
      case PNG:   type = "PNG"; break;
      case PPM:   type = "PPM"; break;
      case SYS:   type = "(system format)"; break;
      case NONE:                     /* Just to stop warning messages */
      default:    type = "None"; break;
    }

    Render_Info("  (name: %s type: %s, grid: %dx%d)",
             opts.Histogram_File_Name,type,opts.histogram_x,opts.histogram_y);
  }
  Render_Info("\n");
#endif /* PRECISION_TIMER_AVAILABLE */

  rinfo_on   ("  Continued trace.....", (opts.Options & CONTINUE_TRACE));
  rinfo_on   ("  Allow interruption..", (opts.Options & EXITENABLE));
  rinfo_on   ("  Pause when done.....", (opts.Options & PROMPTEXIT));
  Render_Info("\n");

  rinfo_on   ("  Verbose messages....", (opts.Options & VERBOSE));

  Render_Info("\n");

  /* Print tracing options. */

  Render_Info("Tracing Options\n");

  Render_Info("  Quality: %2d\n", opts.Quality);

  rinfo_on   ("  Bounding boxes......", opts.Use_Slabs);
  if (opts.Use_Slabs)
  {
    Render_Info("  Bounding threshold: %d ", opts.BBox_Threshold);
  }
  Render_Info("\n");

  rinfo_on("  Light Buffer........", (opts.Options & USE_LIGHT_BUFFER));
  rinfo_on("  Vista Buffer........", (opts.Options & USE_VISTA_BUFFER));
  if (opts.Options & USE_VISTA_BUFFER && opts.Options & DISPLAY)
  {
    rinfo_on("  Draw Vista Buffer...", (opts.Options & USE_VISTA_DRAW));
  }
  Render_Info("\n");

  rinfo_on   ("  Antialiasing........", (opts.Options & ANTIALIAS));
  if (opts.Options & ANTIALIAS)
  {
    Render_Info("  (Method %d, ", opts.Tracing_Method);
    Render_Info("Threshold %.3f, ", opts.Antialias_Threshold);
    Render_Info("Depth %ld, ", opts.AntialiasDepth);
    Render_Info("Jitter %.2f)", opts.JitterScale);
  }
  Render_Info("\n");

  /* Print animation options. */

  Render_Info("Animation Options\n");

  if (opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME)
  {
    Render_Info("  Initial Frame..%8d", opts.FrameSeq.InitialFrame);
    Render_Info("  Final Frame....%8d\n", opts.FrameSeq.FinalFrame);
    Render_Info("  Initial Clock..%8.3f", opts.FrameSeq.InitialClock);
    Render_Info("  Final Clock....%8.3f\n", opts.FrameSeq.FinalClock);
    rinfo_on   ("  Cyclic Animation....", (opts.Options & CYCLIC_ANIMATION));
    rinfo_on   ("  Field render........", (opts.FrameSeq.Field_Render_Flag));
    rinfo_on   ("  Odd lines/frames....", (opts.FrameSeq.Odd_Field_Flag));
  }
  else
  {
    Render_Info("  Clock value....%8.3f", opts.FrameSeq.Clock_Value);
    Render_Info("  (Animation off)");
  }

  Render_Info("\n");

  /* Print redirecting options. */

  Render_Info("Redirecting Options\n");

  rinfo_on   ("  All Streams to console.........", Stream_Info[ALL_STREAM].console != NULL);
  if (Stream_Info[ALL_STREAM].name == NULL)
  {
    Render_Info("\n");
  }
  else
  {
    Render_Info("  and file %s\n", Stream_Info[ALL_STREAM].name);
  }

  rinfo_on   ("  Debug Stream to console........", Stream_Info[DEBUG_STREAM].console != NULL);
  if (Stream_Info[DEBUG_STREAM].name == NULL)
  {
    Render_Info("\n");
  }
  else
  {
    Render_Info("  and file %s\n", Stream_Info[DEBUG_STREAM].name);
  }

  rinfo_on   ("  Fatal Stream to console........", Stream_Info[FATAL_STREAM].console != NULL);
  if (Stream_Info[FATAL_STREAM].name == NULL)
  {
    Render_Info("\n");
  }
  else
  {
    Render_Info("  and file %s\n", Stream_Info[FATAL_STREAM].name);
  }

  rinfo_on   ("  Render Stream to console.......", Stream_Info[RENDER_STREAM].console != NULL);
  if (Stream_Info[RENDER_STREAM].name == NULL)
  {
    Render_Info("\n");
  }
  else
  {
    Render_Info("  and file %s\n", Stream_Info[RENDER_STREAM].name);
  }

  rinfo_on   ("  Statistics Stream to console...", Stream_Info[STATISTIC_STREAM].console != NULL);
  if (Stream_Info[STATISTIC_STREAM].name == NULL)
  {
    Render_Info("\n");
  }
  else
  {
    Render_Info("  and file %s\n", Stream_Info[STATISTIC_STREAM].name);
  }

  rinfo_on   ("  Warning Stream to console......", Stream_Info[WARNING_STREAM].console != NULL);
  if (Stream_Info[WARNING_STREAM].name == NULL)
  {
    Render_Info("\n");
  }
  else
  {
    Render_Info("  and file %s\n", Stream_Info[WARNING_STREAM].name);
  }
}


/****************************************************************************/
/* Use this routine to display opening banner & copyright info */
int Banner(char *format,...)
{
  va_list marker;
  char localvsbuffer[1024];

  va_start(marker, format);
  vsprintf(localvsbuffer, format, marker);
  va_end(marker);
  
  stream_printf(BANNER_STREAM, "%s", localvsbuffer);
  
  return 0;
}

/****************************************************************************/
/*
 * Use this routine to display general information messages about the current
 * rendering if that information applies to the entire render.  Items such as
 * "Options in effect" or when animation is added it would display frame number
 * or clock value. In a windowed environment this info might stay static on the
 * screen during the whole session. Status messages such as "Parsing..." or
 * "Building slabs, please wait" etc should use Status_Info below.
 */
int Render_Info(char *format,...)
{
  va_list marker;
  char localvsbuffer[1024];

  va_start(marker, format);
  vsprintf(localvsbuffer, format, marker);
  va_end(marker);
  
  stream_printf(RENDER_STREAM, "%s", localvsbuffer);
  
  return 0;
}

/****************************************************************************/
/*
 * Use this routine to display information messages such as "Parsing..." or
 * "Building slabs, please wait" etc   Windowed environments might implement
 * one or two status lines at the bottom of the screen.
 */
int Status_Info(char *format,...)
{
  va_list marker;
  char localvsbuffer[1024];

  va_start(marker, format);
  vsprintf(localvsbuffer, format, marker);
  va_end(marker);
  
  stream_printf(STATUS_STREAM, "%s", localvsbuffer);
  
  return 0;
}
