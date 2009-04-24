/****************************************************************************
*                statspov.cpp
*
*  This module implements the render statistics gathering and output functions.
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
* $File: //depot/povray/3.5/source/statspov.cpp $
* $Revision: #12 $
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
#include "povms.h"
#include "povmsgid.h"
#include "povmsend.h"
#include "statspov.h"
#include "pov_util.h"

/*****************************************************************************
* Global variables
******************************************************************************/

static char numbers[64][20] =
{
"0000000000000000001",
"0000000000000000002",
"0000000000000000004",
"0000000000000000008",
"0000000000000000016",
"0000000000000000032",
"0000000000000000064",
"0000000000000000128",
"0000000000000000256",
"0000000000000000512",
"0000000000000001024",
"0000000000000002048",
"0000000000000004096",
"0000000000000008192",
"0000000000000016384",
"0000000000000032768",
"0000000000000065536",
"0000000000000131072",
"0000000000000262144",
"0000000000000524288",
"0000000000001048576",
"0000000000002097152",
"0000000000004194304",
"0000000000008388608",
"0000000000016777216",
"0000000000033554432",
"0000000000067108864",
"0000000000134217728",
"0000000000268435456",
"0000000000536870912",
"0000000001073741824",
"0000000002147483648",
"0000000004294967296",
"0000000008589934592",
"0000000017179869184",
"0000000034359738368",
"0000000068719476736",
"0000000137438953472",
"0000000274877906944",
"0000000549755813888",
"0000001099511627776",
"0000002199023255552",
"0000004398046511104",
"0000008796093022208",
"0000017592186044416",
"0000035184372088832",
"0000070368744177664",
"0000140737488355328",
"0000281474976710656",
"0000562949953421312",
"0001125899906842624",
"0002251799813685248",
"0004503599627370496",
"0009007199254740992",
"0018014398509481984",
"0036028797018963968",
"0072057594037927936",
"0144115188075855872",
"0288230376151711744",
"0576460752303423488",
"1152921504606846976",
"2305843009213693952",
"4611686018427387904",
"9223372036854775808"
};

/*****************************************************************************
* Local variables
******************************************************************************/

static char s1[OUTPUT_LENGTH], s2[OUTPUT_LENGTH];


/*****************************************************************************
*
* FUNCTION
*
*   add_numbers
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
*   Add two decimal numbers stored in ASCII strings.
*
* CHANGES
*
*   Mar 1995 : Creation
*
******************************************************************************/

void add_numbers(char *result, char  *c1, char  *c2)
{
  int i;
  char carry, x;

  carry = '0';

  for (i = NUMBER_LENGTH-1; i >= 0; i--)
  {
    x = c1[i] + c2[i] + carry - '0' - '0';

    if (x > '9')
    {
      carry = '1';

      result[i] = x - 10;
    }
    else
    {
      carry = '0';

      result[i] = x;
    }
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   counter_to_string
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
*   Convert a low/high precision counter into a decimal number.
*
* CHANGES
*
*   Mar 1995 : Creation
*
******************************************************************************/

void counter_to_string(COUNTER *counter, char *string, int len)
{
  char n[NUMBER_LENGTH+1];
  int i, j;
  COUNTER c;

  c = *counter;

  for (i = 0; i < NUMBER_LENGTH; i++)
  {
    n[i] = '0';
  }

  n[NUMBER_LENGTH] = '\0';

#if COUNTER_RESOLUTION == HIGH_RESOLUTION

  for (i = 0; i < 32; i++)
  {
    if (c.low & 1)
    {
      add_numbers(n, n, numbers[i]);
    }

    c.low >>= 1;
  }

  for (i = 32; i < 64; i++)
  {
    if (c.high & 1)
    {
      add_numbers(n, n, numbers[i]);
    }

    c.high >>= 1;
  }

#else

  for (i = 0; i < 32; i++)
  {
    if (c & 1)
    {
      add_numbers(n, n, numbers[i]);
    }

    c >>= 1;
  }

#endif

  /* Replace leading zeros. */

  for (i = 0; i < NUMBER_LENGTH-1; i++)
  {
    if (n[i] == '0')
    {
      n[i] = ' ';
    }
    else
    {
      break;
    }
  }

  /* Copy number into result string. */

  if (i >= NUMBER_LENGTH-len)
  {
    for (j = 0; j < i-NUMBER_LENGTH+len-1; j++)
    {
      string[j] = ' ';
    }

    string[j] = '\0';

    string = strcat(string, &n[i]);
  }
  else
  {
    /* Print numbers that don't fit into output string in million units. */

    string = "";

    n[NUMBER_LENGTH-6] = 'm';
    n[NUMBER_LENGTH-5] = '\0';

    string = strcat(string, &n[NUMBER_LENGTH-len+1-6]);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   init_statistics
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
*   Initialize statistics to 0
*
* CHANGES
*
*   -
*
******************************************************************************/

void init_statistics(COUNTER *pstats)
{
  int i;

  for(i=0; i<MaxStat; i++)
    Init_Counter(pstats[i]);
}

/*****************************************************************************
*
* FUNCTION
*
*   sum_statistics
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
*   Add current statistics to total statistics
*
* CHANGES
*
*   -
*
******************************************************************************/

void sum_statistics(COUNTER *ptotalstats, COUNTER *pstats)
{
  int i;
  COUNTER tmp;

  for(i=0; i<MaxStat; i++)
  {
    Add_Counter(tmp,pstats[i],ptotalstats[i]);
    ptotalstats[i]=tmp;
  }
}


/*****************************************************************************
*
* FUNCTION
*
*   check_stats
*
* INPUT
*   y = current row
*   doingMosaic = 1 if doing mosaic preview (show pixWidth)
*   pixWidth = size of pixel
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
*   6/17/95 esp Added display of mosaic pixWidth
*
******************************************************************************/

void check_stats(register int y, register int doingMosaic, register int pixWidth)
{
  DBL time_dif;
  unsigned long hrs, mins;
  DBL secs;

  /* New verbose options CdW */

  if (opts.Options & VERBOSE)
  {
    STOP_TIME

    time_dif = TIME_ELAPSED

    if (time_dif > 0.0)
    {
      SPLIT_TIME(time_dif, &hrs, &mins, &secs);

      Status_Info("%3ld:%02ld:%02ld ", hrs, mins, (long)secs);
      Send_RenderTime(0, 0, time_dif);
      Send_RenderStatus("Rendering");
    }
    else
    {
      Status_Info("  -:--:-- ");
    }

    Status_Info("Rendering line %4d of %4d", y-opts.First_Line+1, opts.Last_Line-opts.First_Line);

    if (doingMosaic)
    {
      Status_Info(" at %dx%d  \b\b", pixWidth, pixWidth);
    }

    if (opts.Options & ANTIALIAS)
    {
      SuperSampleCount = 0;
    }
    else
    {
      Status_Info(". ");
    }

    RadiosityCount = ra_gather_count;
  }
}


/*****************************************************************************
*
* FUNCTION
*
*   print_intersections_stats
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
*   -
*
* CHANGES
*
*   Mar 1995 : Creation
*
******************************************************************************/

void print_intersection_stats(char *text, COUNTER *tests, COUNTER  *succeeded)
{
  DBL t, s, p;

  if (!Test_Zero_Counter(*tests))
  {
    t = DBL_Counter(*tests);
    s = DBL_Counter(*succeeded);

    p = 100.0 * s / t;

    counter_to_string(tests, s1, OUTPUT_LENGTH);
    counter_to_string(succeeded, s2, OUTPUT_LENGTH);

    Statistics("%-22s  %s  %s  %8.2f\n", text, s1, s2, p);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   Print_Stats
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

void Print_Stats(char *title, COUNTER *pstats)
{
  unsigned long hours, minutes;
  DBL seconds, taverage, ttotal;
  long Pixels_In_Image;
  int i;

  Pixels_In_Image = (long)Frame.Screen_Width * (long)Frame.Screen_Height;

  Statistics ("\n%s for %s", title, opts.Input_File_Name);

  if ( Pixels_In_Image > DBL_Counter(pstats[Number_Of_Pixels]) )
    Statistics (" (Partial Image Rendered)");

  Statistics (", Resolution %d x %d\n", Frame.Screen_Width, Frame.Screen_Height);

  Statistics ("----------------------------------------------------------------------------\n");

  Statistics ("Pixels: %15.0f   Samples: %15.0f   Smpls/Pxl: ",
              DBL_Counter(pstats[Number_Of_Pixels]),
              DBL_Counter(pstats[Number_Of_Samples]));

  if (!Test_Zero_Counter(pstats[Number_Of_Pixels]))
  {
    Statistics ("%.2f\n",
              DBL_Counter(pstats[Number_Of_Samples]) /
              DBL_Counter(pstats[Number_Of_Pixels]));
  }
  else
  {
    Statistics ("-\n");
  }

  counter_to_string(&pstats[Number_Of_Rays], s1, OUTPUT_LENGTH);
  counter_to_string(&pstats[ADC_Saves], s2, OUTPUT_LENGTH);

  Statistics ("Rays:    %s   Saved:    %s   Max Level: %d/%d\n",
              s1, s2, Highest_Trace_Level, Max_Trace_Level);

  Statistics ("----------------------------------------------------------------------------\n");

  Statistics ("Ray->Shape Intersection          Tests       Succeeded  Percentage\n");
  Statistics ("----------------------------------------------------------------------------\n");

  for(i = 0; intersection_stats[i].infotext != NULL; i++)
  {
    print_intersection_stats(intersection_stats[i].infotext,
                             &pstats[intersection_stats[i].stat_test_id],
                             &pstats[intersection_stats[i].stat_suc_id]);
  }

  if ((!Test_Zero_Counter(pstats[Ray_IsoSurface_Find_Root])) || (!Test_Zero_Counter(pstats[Ray_Function_VM_Calls])))
    Statistics ("----------------------------------------------------------------------------\n");

  if (!Test_Zero_Counter(pstats[Ray_IsoSurface_Find_Root]))
  {
    counter_to_string(&pstats[Ray_IsoSurface_Find_Root], s1, OUTPUT_LENGTH);

    Statistics ("Isosurface roots:   %s\n", s1);
  }

  if (!Test_Zero_Counter(pstats[Ray_Function_VM_Calls]))
  {
    counter_to_string(&pstats[Ray_Function_VM_Calls], s1, OUTPUT_LENGTH);

    Statistics ("Function VM calls:  %s\n", s1);
  }

  Statistics ("----------------------------------------------------------------------------\n");

  if (!Test_Zero_Counter(pstats[Polynomials_Tested]))
  {
    counter_to_string(&pstats[Polynomials_Tested], s1, OUTPUT_LENGTH);
    counter_to_string(&pstats[Roots_Eliminated], s2, OUTPUT_LENGTH);

    Statistics ("Roots tested:       %s   eliminated:      %s\n", s1, s2);
  }

  counter_to_string(&pstats[Calls_To_Noise], s1, OUTPUT_LENGTH);
  counter_to_string(&pstats[Calls_To_DNoise], s2, OUTPUT_LENGTH);

  Statistics ("Calls to Noise:     %s   Calls to DNoise: %s\n", s1, s2);

  Statistics ("----------------------------------------------------------------------------\n");

  /* Print media samples. */

  if (!Test_Zero_Counter(pstats[Media_Intervals]))
  {
    counter_to_string(&pstats[Media_Intervals], s1, OUTPUT_LENGTH);
    counter_to_string(&pstats[Media_Samples], s2, OUTPUT_LENGTH);

    Statistics ("Media Intervals:    %s   Media Samples:   %s (%4.2f)\n", s1, s2,
      DBL_Counter(pstats[Media_Samples]) / DBL_Counter(pstats[Media_Intervals]));
  }

  if (!Test_Zero_Counter(pstats[Shadow_Ray_Tests]))
  {
    counter_to_string(&pstats[Shadow_Ray_Tests], s1, OUTPUT_LENGTH);
    counter_to_string(&pstats[Shadow_Rays_Succeeded], s2, OUTPUT_LENGTH);

    Statistics ("Shadow Ray Tests:   %s   Succeeded:       %s\n", s1, s2);
  }

  if (!Test_Zero_Counter(pstats[Reflected_Rays_Traced]))
  {
    counter_to_string(&pstats[Reflected_Rays_Traced], s1, OUTPUT_LENGTH);

    Statistics ("Reflected Rays:     %s", s1);

    if (!Test_Zero_Counter(pstats[Internal_Reflected_Rays_Traced]))
    {
      counter_to_string(&pstats[Internal_Reflected_Rays_Traced], s1, OUTPUT_LENGTH);

      Statistics ("   Total Internal:  %s", s1);
    }

    Statistics ("\n");
  }

  if (!Test_Zero_Counter(pstats[Refracted_Rays_Traced]))
  {
    counter_to_string(&pstats[Refracted_Rays_Traced], s1, OUTPUT_LENGTH);

    Statistics ("Refracted Rays:     %s\n", s1);
  }

  if (!Test_Zero_Counter(pstats[Transmitted_Rays_Traced]))
  {
    counter_to_string(&pstats[Transmitted_Rays_Traced], s1, OUTPUT_LENGTH);

    Statistics ("Transmitted Rays:   %s\n", s1);
  }

  if (!Test_Zero_Counter(pstats[Istack_overflows]))
  {
    counter_to_string(&pstats[Istack_overflows], s1, OUTPUT_LENGTH);

    Statistics ("I-Stack overflows:  %s\n", s1);
  }

  /* NK phmap - regular & testing stats */
  if (!Test_Zero_Counter(pstats[Number_Of_Photons_Shot]))
  {
    counter_to_string(&pstats[Number_Of_Photons_Shot], s1, OUTPUT_LENGTH);
    Statistics ("Number of photons shot: %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Number_Of_Photons_Stored]))
  {
    counter_to_string(&pstats[Number_Of_Photons_Stored], s1, OUTPUT_LENGTH);
    Statistics ("Surface photons stored: %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Number_Of_Media_Photons_Stored]))
  {
    counter_to_string(&pstats[Number_Of_Media_Photons_Stored], s1, OUTPUT_LENGTH);
    Statistics ("Media photons stored:   %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Number_Of_Global_Photons_Stored]))
  {
    counter_to_string(&pstats[Number_Of_Global_Photons_Stored], s1, OUTPUT_LENGTH);
    Statistics ("Global photons stored:  %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Priority_Queue_Insert]))
  {
    counter_to_string(&pstats[Priority_Queue_Insert], s1, OUTPUT_LENGTH);
    Statistics ("Priority queue insert:  %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Priority_Queue_Remove]))
  {
    counter_to_string(&pstats[Priority_Queue_Remove], s1, OUTPUT_LENGTH);
    Statistics ("Priority queue remove:  %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Gather_Performed_Count]))
  {
    counter_to_string(&pstats[Gather_Performed_Count], s1, OUTPUT_LENGTH);
    Statistics ("Gather function called: %s\n", s1);
  }
  if (!Test_Zero_Counter(pstats[Gather_Expanded_Count]))
  {
    counter_to_string(&pstats[Gather_Expanded_Count], s1, OUTPUT_LENGTH);
    Statistics ("Gather radius expanded: %s\n", s1);
  }

  if ( ra_reuse_count || ra_gather_count )
  {
    Statistics ("----------------------------------------------------------------------------\n");
    Statistics ("Radiosity samples calculated:  %9ld (%.2f percent)\n", ra_gather_count,
                100.*(DBL) ra_gather_count / ((DBL)(ra_gather_count + ra_reuse_count)));
    /* Note:  don't try to put in a percent sign.  There is basically no way to do this
       which is completely portable using va_start */
    Statistics ("Radiosity samples reused:      %9ld\n", ra_reuse_count);
  }

#if defined(MEM_STATS)
  Statistics ("----------------------------------------------------------------------------\n");

  Long_To_Counter(mem_stats_smallest_alloc(), pstats[MemStat_Smallest_Alloc]);
  counter_to_string(&pstats[MemStat_Smallest_Alloc], s1, OUTPUT_LENGTH);

  Long_To_Counter(mem_stats_largest_alloc(), pstats[MemStat_Largest_Alloc]);
  counter_to_string(&pstats[MemStat_Largest_Alloc],  s2, OUTPUT_LENGTH);

#if (MEM_STATS==1)
  Statistics ("Smallest Alloc:     %s bytes   Largest:   %s\n", s1, s2);
#elif (MEM_STATS>=2)
  Statistics ("Smallest Alloc:     %s bytes @ %s:%d\n", s1, mem_stats_smallest_file(), mem_stats_smallest_line());
  Statistics ("Largest  Alloc:     %s bytes @ %s:%d\n", s2, mem_stats_largest_file(), mem_stats_largest_line());

  Long_To_Counter(mem_stats_total_allocs(), pstats[MemStat_Total_Allocs]);
  counter_to_string(&pstats[MemStat_Total_Allocs], s1, OUTPUT_LENGTH);
  Long_To_Counter(mem_stats_total_frees(), pstats[MemStat_Total_Frees]);
  counter_to_string(&pstats[MemStat_Total_Frees], s2, OUTPUT_LENGTH);
  Statistics ("Total Alloc calls:  %s         Free calls:%s\n", s1, s2);
#endif

  Long_To_Counter(mem_stats_largest_mem_usage(), pstats[MemStat_Largest_Mem_Usage]);
  counter_to_string(&pstats[MemStat_Largest_Mem_Usage], s1, OUTPUT_LENGTH);
  Statistics ("Peak memory used:   %s bytes\n", s1);

#endif

  Statistics ("----------------------------------------------------------------------------\n");

  /* Get time in case the trace was aborted. */

  if (trender == 0.0)
  {
    STOP_TIME

    trender = TIME_ELAPSED
  }

  if (opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME)
  {
    if (tparse_total != 0.0)
    {
      taverage = tparse_total /
          (DBL)(opts.FrameSeq.FrameNumber - opts.FrameSeq.InitialFrame + 1);

      SPLIT_TIME(taverage,&hours,&minutes,&seconds);
      Statistics ("Time For Parse/Frame:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)taverage);

      SPLIT_TIME(tparse_total,&hours,&minutes,&seconds);
      Statistics ("Time For Parse Total:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)tparse_total);
    }

    /* NK photons */
    if (tphoton_total != 0.0)
    {
      taverage = tphoton_total /
          (DBL)(opts.FrameSeq.FrameNumber - opts.FrameSeq.InitialFrame + 1);

      SPLIT_TIME(taverage,&hours,&minutes,&seconds);
      Statistics ("Time For Photon/Frame: %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)taverage);

      SPLIT_TIME(tphoton_total,&hours,&minutes,&seconds);
      Statistics ("Time For Photon Total: %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)tphoton_total);
    }

    if (trender_total != 0.0)
    {
      taverage = trender_total /
          (DBL)(opts.FrameSeq.FrameNumber - opts.FrameSeq.InitialFrame + 1);

      SPLIT_TIME(taverage,&hours,&minutes,&seconds);
      Statistics ("Time For Trace/Frame:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)taverage);

      SPLIT_TIME(trender_total,&hours,&minutes,&seconds);
      Statistics ("Time For Trace Total:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)trender_total);
    }

    ttotal = tparse_total + tphoton_total + trender_total;

    if (ttotal != 0.0)
    {
      SPLIT_TIME(ttotal,&hours,&minutes,&seconds);

      Statistics ("          Total Time:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)ttotal);
    }
  }
  else
  {
    if (tparse_frame != 0.0)
    {
      SPLIT_TIME(tparse_frame,&hours,&minutes,&seconds);

      Statistics ("Time For Parse:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)tparse_frame);
    }

    /* NK photons */
    if (tphoton_frame != 0.0)
    {
      SPLIT_TIME(tphoton_frame,&hours,&minutes,&seconds);

      Statistics ("Time For Photon: %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)tphoton_frame);
    }

    if (trender_frame != 0.0)
    {
      SPLIT_TIME(trender,&hours,&minutes,&seconds);

      Statistics ("Time For Trace:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)trender_frame);
    }

    ttotal = tparse_frame + tphoton_frame + trender_frame;

    if (ttotal != 0.0)
    {
      SPLIT_TIME(ttotal,&hours,&minutes,&seconds);

      Statistics ("    Total Time:  %3ld hours %2ld minutes %5.1f seconds (%ld seconds)\n",
          hours, minutes, seconds, (long)ttotal);
    }
  }
}

/****************************************************************************/
/* Use this routine to display final rendering statistics */
int Statistics(char *format,...)
{
  va_list marker;
  char localvsbuffer[1024];

  va_start(marker, format);
  vsprintf(localvsbuffer, format, marker);
  va_end(marker);
  
  stream_printf(STATISTIC_STREAM, localvsbuffer);
  
  return 0;
}
