/****************************************************************************
*                povmsend.cpp
*
*  This module contains POVMS received message handling routines.
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
* $File: //depot/povray/3.5/source/povmsend.cpp $
* $Revision: #22 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include <stdarg.h>
#include <float.h>
#include <algorithm>

#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "parse.h"
#include "povray.h"
#include "tokenize.h"
#include "userio.h"
#include "userdisp.h"
#include "povms.h"
#include "povmsend.h"
#include "octree.h"
#include "radiosit.h"
#include "optout.h"
#include "pov_err.h"
#include "pov_util.h"

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
* Local functions
******************************************************************************/


/*****************************************************************************
*
* FUNCTION
*
*   BuildCommand
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int BuildCommand(POVMSObjectPtr msg, POVMSType key, SHELLDATA *data)
{
   POVMSObject obj;
   int err;

   err = POVMSObject_New(&obj, kPOVObjectClass_Command);
   if(err == 0)
      err = POVMSUtil_SetString(&obj, kPOVAttrib_CommandString, data->Command);
   if(err == 0)
   {
      int i;

      switch(data->Ret)
      {
         case IGNORE_RET:
            i = 'I';
            break;
         case QUIT_RET:
            i = 'Q';
            break;
         case USER_RET:
            i = 'U';
            break;
         case FATAL_RET:
            i = 'F';
            break;
         case SKIP_ONCE_RET:
            i = 'S';
            break;
         case ALL_SKIP_RET:
            i = 'A';
            break;
      }

      err = POVMSUtil_SetInt(&obj, kPOVAttrib_ReturnAction, i);
   }
   if(err == 0)
      err = POVMSObject_Set(msg, &obj, key);

   return err;
}


/*****************************************************************************
*
* FUNCTION
*
*   BuildRTime
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int BuildRTime(POVMSObjectPtr msg, POVMSType key, int parse, int photon, int render, int total)
{
   POVMSObject obj;
   int err;

   err = POVMSObject_New(&obj, kPOVObjectClass_RTime);
   if(err == 0)
      err = POVMSUtil_SetInt(&obj, kPOVAttrib_ParseTime, parse);
   if(err == 0)
      err = POVMSUtil_SetInt(&obj, kPOVAttrib_PhotonTime, photon);
   if(err == 0)
      err = POVMSUtil_SetInt(&obj, kPOVAttrib_TraceTime, render);
   if(err == 0)
      err = POVMSUtil_SetInt(&obj, kPOVAttrib_TotalTime, total);
   if(err == 0)
      err = POVMSObject_Set(msg, &obj, key);

   return err;
}


/*****************************************************************************
*
* FUNCTION
*
*   BuildRenderOptions
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int BuildRenderOptions(POVMSObjectPtr msg)
{
   POVMSAttribute attr;
   int err = kNoErr;

   if(msg == NULL)
      return kParamErr;

   if(err == kNoErr)
      err = POVMSAttr_New(&attr);
   if(err == kNoErr)
   {
      err = POVMSAttr_Set(&attr, kPOVMSType_WildCard, (void *)(&opts.Preview_RefCon), sizeof(unsigned long));
      if(err == kNoErr)
         err = POVMSObject_Set(msg, &attr, kPOVAttrib_PreviewRefCon);
   }
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Height, Frame.Screen_Height);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Width, Frame.Screen_Width);
   if(err == kNoErr)
   {
      if (opts.First_Column == -1)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_StartColumn, opts.First_Column_Percent);
      else
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_StartColumn, opts.First_Column);
   }
   if(err == kNoErr)
   {
      if (opts.Last_Column == -1)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_EndColumn, opts.Last_Column_Percent);
      else
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_EndColumn, opts.Last_Column);
   }
   if(err == kNoErr)
   {
      if (opts.First_Line == -1)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_StartRow, opts.First_Line_Percent);
      else
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_StartRow, opts.First_Line);
   }
   if(err == kNoErr)
   {
      if (opts.Last_Line == -1)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_EndRow, opts.Last_Line_Percent);
      else
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_EndRow, opts.Last_Line);
   }
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_TestAbort, (opts.Options & EXITENABLE) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_TestAbortCount, opts.Abort_Test_Counter);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_ContinueTrace, (opts.Options & CONTINUE_TRACE) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetString(msg, kPOVAttrib_CreateIni, opts.Ini_Output_File_Name);
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_Clock, opts.FrameSeq.Clock_Value);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_InitialFrame, max(opts.FrameSeq.InitialFrame, 1));
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_FinalFrame, max(opts.FrameSeq.FinalFrame, 1));
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_InitialClock, opts.FrameSeq.InitialClock);
   if(err == kNoErr)
   {
      if(opts.FrameSeq.FinalFrame <= 1)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_FinalClock, 1.0);
      else
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_FinalClock, opts.FrameSeq.FinalClock);
   }
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_SubsetStartFrame, max(opts.FrameSeq.SubsetStartFrame, 1));
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_SubsetEndFrame, max(opts.FrameSeq.SubsetEndFrame, 1));
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_CyclicAnimation, (opts.Options & CYCLIC_ANIMATION) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_FieldRender, opts.FrameSeq.Field_Render_Flag);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_OddField, opts.FrameSeq.Odd_Field_Flag);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_PauseWhenDone, (opts.Options & PROMPTEXIT) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_Verbose, (opts.Options & VERBOSE) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_DrawVistas, (opts.Options & USE_VISTA_DRAW) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_Display, (opts.Options & DISPLAY) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_VideoMode, opts.DisplayFormat);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Palette, opts.PaletteOption);
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_DisplayGamma, opts.DisplayGamma);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_PreviewStartSize, opts.PreviewGridSize_Start);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_PreviewEndSize, opts.PreviewGridSize_End);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_OutputToFile, (opts.Options & DISKWRITE) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_OutputFileType, opts.OutputFormat);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Compression, opts.OutputQuality);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_OutputAlpha, (opts.Options & OUTPUT_ALPHA) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_BitsPerColor, opts.OutputQuality);
   if(err == kNoErr)
      err = POVMSUtil_SetString(msg, kPOVAttrib_OutputFile, opts.Output_File_Name);
#if PRECISION_TIMER_AVAILABLE
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_CreateHistogram, opts.histogram_on != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_HistogramFileType, opts.histogram_type);
   if(err == kNoErr)
      err = POVMSUtil_SetString(msg, kPOVAttrib_HistogramFile, opts.Histogram_File_Name);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_HistogramGridSizeX, opts.histogram_x);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_HistogramGridSizeY, opts.histogram_y);
#endif /* PRECISION_TIMER_AVAILABLE */
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_BufferOutput, (opts.Options & BUFFERED_OUTPUT) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_BufferSize, opts.File_Buffer_Size / 1024);
   if(err == kNoErr)
      err = BuildCommand(msg, kPOVAttrib_PreSceneCommand, &opts.Shellouts[PRE_SCENE_SHL]);
   if(err == kNoErr)
      err = BuildCommand(msg, kPOVAttrib_PreFrameCommand, &opts.Shellouts[PRE_FRAME_SHL]);
   if(err == kNoErr)
      err = BuildCommand(msg, kPOVAttrib_PostSceneCommand, &opts.Shellouts[POST_SCENE_SHL]);
   if(err == kNoErr)
      err = BuildCommand(msg, kPOVAttrib_PostFrameCommand, &opts.Shellouts[POST_FRAME_SHL]);
   if(err == kNoErr)
      err = BuildCommand(msg, kPOVAttrib_UserAbortCommand, &opts.Shellouts[USER_ABORT_SHL]);
   if(err == kNoErr)
      err = BuildCommand(msg, kPOVAttrib_FatalErrorCommand, &opts.Shellouts[FATAL_SHL]);
   if(err == kNoErr)
      err = POVMSUtil_SetString(msg, kPOVAttrib_InputFile, opts.Input_File_Name);
   if(err == kNoErr)
   {
      POVMSAttributeList list;

      err = POVMSAttrList_New(&list);
      if(err == kNoErr)
      {
         int ii;

         for(ii = 0; ii < opts.Library_Path_Index; ii++)
         {
            err = POVMSAttr_New(&attr);
            if(err == kNoErr)
            {
               err = POVMSAttr_Set(&attr, kPOVMSType_CString, opts.Library_Paths[ii], strlen(opts.Library_Paths[ii]) + 1);
               if(err == kNoErr)
                  err = POVMSAttrList_Append(&list, &attr);
               else
                  err = POVMSAttr_Delete(&attr);
            }
         }
         if(err == kNoErr)
            err = POVMSObject_Set(msg, &list, kPOVAttrib_LibraryPath);
      }
   }
   if(err == kNoErr)
   {
      POVMSFloat f = opts.Language_Version / 100.0;
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_Version, f);
   }
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_DebugConsole, Stream_Info[DEBUG_STREAM].console != NULL);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_FatalConsole, Stream_Info[FATAL_STREAM].console != NULL);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_RenderConsole, Stream_Info[RENDER_STREAM].console != NULL);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_StatisticsConsole, Stream_Info[STATISTIC_STREAM].console != NULL);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_WarningConsole, Stream_Info[WARNING_STREAM].console != NULL);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_AllConsole, Stream_Info[ALL_STREAM].console != NULL);
   if((err == kNoErr) && (Stream_Info[DEBUG_STREAM].name != NULL))
      err = POVMSUtil_SetString(msg, kPOVAttrib_DebugFile, Stream_Info[DEBUG_STREAM].name);
   if((err == kNoErr) && (Stream_Info[FATAL_STREAM].name != NULL))
      err = POVMSUtil_SetString(msg, kPOVAttrib_FatalFile, Stream_Info[FATAL_STREAM].name);
   if((err == kNoErr) && (Stream_Info[RENDER_STREAM].name != NULL))
      err = POVMSUtil_SetString(msg, kPOVAttrib_RenderFile, Stream_Info[RENDER_STREAM].name);
   if((err == kNoErr) && (Stream_Info[STATISTIC_STREAM].name != NULL))
      err = POVMSUtil_SetString(msg, kPOVAttrib_StatisticsFile, Stream_Info[STATISTIC_STREAM].name);
   if((err == kNoErr) && (Stream_Info[WARNING_STREAM].name != NULL))
      err = POVMSUtil_SetString(msg, kPOVAttrib_WarningFile, Stream_Info[WARNING_STREAM].name);
   if((err == kNoErr) && (Stream_Info[ALL_STREAM].name != NULL))
      err = POVMSUtil_SetString(msg, kPOVAttrib_AllFile, Stream_Info[ALL_STREAM].name);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Quality, opts.Quality);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_Bounding, opts.Use_Slabs);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_BoundingThreshold, opts.BBox_Threshold);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_LightBuffer, (opts.Options & USE_LIGHT_BUFFER) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_VistaBuffer, (opts.Options & USE_VISTA_BUFFER) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_RemoveBounds, (opts.Options & REMOVE_BOUNDS) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_SplitUnions, (opts.Options & SPLIT_UNION) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_Antialias, (opts.Options & ANTIALIAS) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_SamplingMethod, opts.Tracing_Method);
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_AntialiasThreshold, opts.Antialias_Threshold);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_AntialiasDepth, opts.AntialiasDepth);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(msg, kPOVAttrib_Jitter, (opts.Options & JITTER) != 0);
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_JitterAmount, opts.JitterScale);
   if(err == kNoErr)
      err = POVMSUtil_SetString(msg, kPOVAttrib_IncludeHeader, opts.Header_File_Name);

   return err;
}


/*****************************************************************************
*
* FUNCTION
*
*   BuildRenderStatus
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int BuildRenderStatus(POVMSObjectPtr msg)
{
   int err = kNoErr;

   if(err == kNoErr)
   {
      if(Stage == STAGE_RENDERING)
         err = POVMSUtil_SetInt(msg, kPOVAttrib_CurrentLine, Current_Line_Number - opts.First_Line + 1);
      else
         err = POVMSUtil_SetInt(msg, kPOVAttrib_CurrentLine, 0);
   }

   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_LineCount, opts.Last_Line - opts.First_Line);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_AbsoluteCurrentLine, Current_Line_Number);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_FiniteObjects, numberOfFiniteObjects);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_InfiniteObjects, numberOfInfiniteObjects);

   if(opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME)
   {
      if(err == kNoErr)
         err = POVMSUtil_SetInt(msg, kPOVAttrib_CurrentFrame, opts.FrameSeq.FrameNumber - opts.FrameSeq.InitialFrame + 1);
      if(err == kNoErr)
         err = POVMSUtil_SetInt(msg, kPOVAttrib_FrameCount, opts.FrameSeq.FinalFrame - opts.FrameSeq.InitialFrame + 1);
      if(err == kNoErr)
         err = POVMSUtil_SetInt(msg, kPOVAttrib_AbsoluteCurFrame, opts.FrameSeq.FrameNumber);
      if(err == kNoErr)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_FirstClock, opts.FrameSeq.InitialClock);
      if(err == kNoErr)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_CurrentClock, opts.FrameSeq.Clock_Value);
      if(err == kNoErr)
         err = POVMSUtil_SetFloat(msg, kPOVAttrib_LastClock, opts.FrameSeq.FinalClock);
   }

   return err;
}


/*****************************************************************************
*
* FUNCTION
*
*   BuildRenderStatistics
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int BuildRenderStatistics(POVMSObjectPtr msg, COUNTER *pstats)
{
   POVMSAttributeList list;
   int err = kNoErr;

   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Height, Frame.Screen_Height);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_Width, Frame.Screen_Width);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_Pixels, &pstats[Number_Of_Pixels]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PixelSamples, &pstats[Number_Of_Samples]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_Rays, &pstats[Number_Of_Rays]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_RaysSaved, &pstats[ADC_Saves]);
   if(err == kNoErr)
      err = POVMSAttrList_New(&list);
   if(err == kNoErr)
   {
      int i;

      for(i = 0; intersection_stats[i].infotext != NULL; i++)
      {
         err = AddOIStatistic(&list, i, pstats);
         if(err != kNoErr)
            break;
      }
   }
   if(err == kNoErr)
      err = POVMSObject_Set(msg, &list, kPOVAttrib_ObjectIStats);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_ShadowTest, &pstats[Shadow_Ray_Tests]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_ShadowTestSuc, &pstats[Shadow_Rays_Succeeded]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_IsoFindRoot, &pstats[Ray_IsoSurface_Find_Root]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_FunctionVMCalls, &pstats[Ray_Function_VM_Calls]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PolynomTest, &pstats[Polynomials_Tested]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_RootsEliminated, &pstats[Roots_Eliminated]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_CallsToNoise, &pstats[Calls_To_Noise]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_CallsToDNoise, &pstats[Calls_To_DNoise]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_MediaSamples, &pstats[Media_Samples]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_MediaIntervals, &pstats[Media_Intervals]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_ReflectedRays, &pstats[Reflected_Rays_Traced]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_InnerReflectedRays, &pstats[Internal_Reflected_Rays_Traced]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_RefractedRays, &pstats[Refracted_Rays_Traced]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_TransmittedRays, &pstats[Transmitted_Rays_Traced]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_IStackOverflow, &pstats[Istack_overflows]);
#if defined(MEM_STATS)
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_MinAlloc, &pstats[MemStat_Smallest_Alloc]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_MaxAlloc, &pstats[MemStat_Largest_Alloc]);
#if (MEM_STATS>=2)
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_CallsToAlloc, &pstats[MemStat_Total_Allocs]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_CallsToFree, &pstats[MemStat_Total_Frees]);
#endif
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PeakMemoryUsage, &pstats[MemStat_Largest_Mem_Usage]);
#endif

   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_TraceLevel, Highest_Trace_Level);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(msg, kPOVAttrib_MaxTraceLevel, Max_Trace_Level);
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_RadGatherCount, (POVMSFloat)ra_gather_count);
   if(err == kNoErr)
      err = POVMSUtil_SetFloat(msg, kPOVAttrib_RadReuseCount, (POVMSFloat)ra_reuse_count);

   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PhotonsShot, &pstats[Number_Of_Photons_Shot]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PhotonsStored, &pstats[Number_Of_Photons_Stored]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_GlobalPhotonsStored, &pstats[Number_Of_Global_Photons_Stored]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_MediaPhotonsStored, &pstats[Number_Of_Media_Photons_Stored]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PhotonsPriQInsert, &pstats[Priority_Queue_Insert]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_PhotonsPriQRemove, &pstats[Priority_Queue_Remove]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_GatherPerformedCnt, &pstats[Gather_Performed_Count]);
   if(err == kNoErr)
      err = AddStatistic(msg, kPOVAttrib_GatherExpandedCnt, &pstats[Gather_Expanded_Count]);

   return 0;
}


/*****************************************************************************
*
* FUNCTION
*
*   AddStatistic
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int AddStatistic(POVMSObjectPtr msg, POVMSType key, COUNTER *counter)
{
   POVMSLong cnt;

#if COUNTER_RESOLUTION == HIGH_RESOLUTION
   SetPOVMSLong(&cnt, counter->high, counter->low);
#else
   SetPOVMSLong(&cnt, 0, counter);
#endif

   return POVMSUtil_SetLong(msg, key, cnt);
}


/*****************************************************************************
*
* FUNCTION
*
*   AddOIStatistic
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int AddOIStatistic(POVMSAttributeListPtr list, int index, COUNTER *pstats)
{
   POVMSObject obj;
   POVMSLong tests,succeeded;
   int err;

#if COUNTER_RESOLUTION == HIGH_RESOLUTION
   SetPOVMSLong(&tests, pstats[intersection_stats[index].stat_test_id].high, 
                        pstats[intersection_stats[index].stat_test_id].low);
   SetPOVMSLong(&succeeded, pstats[intersection_stats[index].stat_suc_id].high,
                            pstats[intersection_stats[index].stat_suc_id].low);
#else
   SetPOVMSLong(&tests, 0, pstats[intersection_stats[index].stat_test_id]);
   SetPOVMSLong(succeeded, 0, pstats[intersection_stats[index].stat_suc_id]);
#endif

   err = POVMSObject_New(&obj, kPOVObjectClass_OIStat);
   if(err == kNoErr)
      err = POVMSUtil_SetString(&obj, kPOVAttrib_ObjectName, intersection_stats[index].infotext);
   if(err == kNoErr)
      err = POVMSUtil_SetInt(&obj, kPOVAttrib_ObjectID, intersection_stats[index].povms_id);
   if(err == kNoErr)
      err = POVMSUtil_SetLong(&obj, kPOVAttrib_ISectsTests, tests);
   if(err == kNoErr)
      err = POVMSUtil_SetLong(&obj, kPOVAttrib_ISectsSucceeded, succeeded);
   if(err == kNoErr)
      err = POVMSAttrList_Append(list, &obj);

   return err;
}


/*****************************************************************************
*
* FUNCTION
*
*   Send_InitInfo
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

void Send_InitInfo()
{
#ifndef DONT_USE_POVMS
   POVMSObject msg;
   int err = kNoErr;

   if(err == kNoErr)
      err = POVMSObject_New(&msg, kPOVMSType_WildCard);

   if(err == kNoErr)
      err = POVMSUtil_SetString(&msg, kPOVAttrib_PlatformName, POVRAY_PLATFORM_NAME);
   if(err == kNoErr)
      err = POVMSUtil_SetFormatString(&msg, kPOVAttrib_CoreVersion, "Persistence of Vision(tm) Ray Tracer Version %s %s", POV_RAY_VERSION, COMPILER_VER);
   if(err == kNoErr)
      err = POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, DISTRIBUTION_MESSAGE_1 "\n" DISTRIBUTION_MESSAGE_2 "\n" DISTRIBUTION_MESSAGE_3);
   if(err == kNoErr)
      err = POVMSUtil_SetBool(&msg, kPOVAttrib_Official, POV_RAY_IS_OFFICIAL);

   if(err == kNoErr)
      err = POVMSMsg_SetupMessage(&msg, kPOVMsgClass_Miscellaneous, kPOVMsgIdent_InitInfo);

   if(err == kNoErr)
      err = POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);

   (void)POVMS_ASSERT(err == kNoErr, "Sending InitInfo failed!");
#endif
}


/*****************************************************************************
*
* FUNCTION
*
*   Send_RenderStatus
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int Send_RenderStatus(char *statusString)
{
#ifdef DONT_USE_POVMS
   return 0;
#else
   POVMSObject msg;
   int err = kNoErr;

   if(err == kNoErr)
      err = POVMSObject_New(&msg, kPOVObjectClass_RInfo);

   if(err == kNoErr)
      err = POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, statusString);

   if(err == kNoErr)
      err = BuildRenderStatus(&msg);

   if(err == kNoErr)
      err = POVMSMsg_SetupMessage(&msg, kPOVMsgClass_Miscellaneous, kPOVMsgIdent_RenderStatus);

   if(err == kNoErr)
      err = POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);

   return err;
#endif
}


/*****************************************************************************
*
* FUNCTION
*
*   Send_RenderTime
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int Send_RenderTime(int tp, int th, int tr)
{
#ifdef DONT_USE_POVMS
   return 0;
#else
   POVMSObject msg;
   int err;

   if(tp <= 0)
     tp = tparse_frame;
   if(th <= 0)
     th = tphoton_frame;
   if(tr <= 0)
     tr = trender_frame;

   err = POVMSObject_New(&msg, kPOVMSType_WildCard);
   if(err == kNoErr)
      BuildRTime(&msg, kPOVAttrib_FrameTime, tp, th, tr, tp + th + tr);
   if((opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME) && (err == kNoErr))
      err = BuildRTime(&msg, kPOVAttrib_AnimationTime,
                       tparse_total + tp, tphoton_total + th, trender_total + tr,
                       tparse_total + tphoton_total + trender_total + tp + th + tr);
   if(err == kNoErr)
      err = POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_RenderTime);
   if(err == kNoErr)
      err = POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);

   return err;
#endif
}


/*****************************************************************************
*
* FUNCTION
*
*   Send_RenderStatistics
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int Send_RenderStatistics()
{
#ifdef DONT_USE_POVMS
   return 0;
#else
   POVMSObject msg;
   int err = kNoErr;

   if(err == kNoErr)
      err = POVMSObject_New(&msg, kPOVObjectClass_RStats);
   if(err == kNoErr)
      err = BuildRenderStatistics(&msg, stats);
   if(err == kNoErr)
      err = POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_RenderStatistics);
   if(err == kNoErr)
      err = POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);

   return err;
#endif
}


/*****************************************************************************
*
* FUNCTION
*
*   Send_RenderOptions
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int Send_RenderOptions()
{
#ifdef DONT_USE_POVMS
   return 0;
#else
   POVMSObject msg;
   int err = kNoErr;

   if(err == kNoErr)
      err = POVMSObject_New(&msg, kPOVObjectClass_ROptions);
   if(err == kNoErr)
      err = BuildRenderOptions(&msg);
   if(err == kNoErr)
      err = POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_RenderOptions);
   if(err == kNoErr)
      err = POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);

   return err;
#endif
}


/*****************************************************************************
*
* FUNCTION
*
*   Send_RenderDone
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
* AUTHOR
*
*   Thorsten Froehlich
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

int Send_RenderDone()
{
#ifdef DONT_USE_POVMS
   return 0;
#else
   POVMSObject msg;
   int err = kNoErr;
   int tp, th, tr;

   if (trender == 0.0)
   {
     STOP_TIME

     trender = TIME_ELAPSED
   }

   tp = tparse_total;
   if(tp <= 0)
     tp = tparse;
   th = tphoton_total;
   if(th <= 0)
     th = tphoton;
   tr = trender_total;
   if(tr <= 0)
     tr = trender;

   err = POVMSObject_New(&msg, kPOVMSType_WildCard);
   if(err == kNoErr)
      BuildRTime(&msg, kPOVAttrib_AnimationTime, tp, th, tr, tp + th + tr);
   if(err == kNoErr)
      err = POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_RenderDone);
   if(err == kNoErr)
      err = POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);

   return err;
#endif
}
