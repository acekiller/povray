/****************************************************************************
*                povray.cpp
*
*  This module contains the entry routine for the raytracer and the code to
*  parse the parameters on the command line.
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
* $File: //depot/povray/3.5/source/povray.cpp $
* $Revision: #89 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include <ctype.h>
#include <time.h>     /* BP */
#include <algorithm>

#include "frame.h"    /* common to ALL modules in this program */
#include "povproto.h"
#include "bezier.h"
#include "blob.h"
#include "bbox.h"
#include "cones.h"
#include "csg.h"
#include "discs.h"
#include "express.h"
#include "fnpovfpu.h"
#include "fractal.h"
#include "hfield.h"
#include "lathe.h"
#include "lighting.h"
#include "lightgrp.h"
#include "mesh.h"
#include "photons.h"
#include "polysolv.h"
#include "objects.h"
#include "octree.h"
#include "parse.h"
#include "pigment.h"
#include "point.h"
#include "poly.h"
#include "polygon.h"
#include "povray.h"
#include "optin.h"
#include "optout.h"
#include "quadrics.h"
#include "pgm.h"
#include "png_pov.h"
#include "ppm.h"
#include "prism.h"
#include "radiosit.h"
#include "render.h"
#include "sor.h"
#include "spheres.h"
#include "super.h"
#include "targa.h"
#include "texture.h"
#include "tokenize.h"
#include "torus.h"
#include "triangle.h"
#include "truetype.h"
#include "userio.h"
#include "userdisp.h"
#include "lbuffer.h"
#include "vbuffer.h"
#include "povmsend.h"
#include "povmsrec.h"
#include "isosurf.h"
#include "sphsweep.h"
#include "pov_util.h"
#include "renderio.h"
#include "statspov.h"
#include "pov_err.h"
#include "optout.h"


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* Flags for the variable store. */

#define STORE   1
#define RESTORE 2



/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Global variables
******************************************************************************/

/* NK phmap */
extern int backtraceFlag;

extern PHOTON_OPTIONS photonOptions;

extern int disp_elem;
extern int disp_nelems;

extern int warpNormalTextures;
extern int InitBacktraceWasCalled;

// povray_init
int pre_init_flag = 0;

// Used for povray_cooperate tricks.
int Cooperate_Render_Flag = 0;

/* The frame and frame related stuff. */
FRAME Frame;
DBL Clock_Delta;

/* Statistics stuff. */
POV_OSTREAM *stat_file;
COUNTER stats[MaxStat];
COUNTER totalstats[MaxStat];

/* Option stuff. */
Opts opts;
char *Option_String_Ptr;

/* File and parsing stuff. */
int Number_Of_Files;
Image_File_Class *Output_File;
int Num_Echo_Lines;      /* May make user setable later - CEY*/
int Echo_Line_Length;    /* May make user setable later - CEY*/

/* Timing stuff .*/
time_t tstart, tstop;
DBL tparse, tphoton, trender;
DBL tparse_frame, tphoton_frame, trender_frame;
DBL tparse_total, tphoton_total, trender_total;

/* Options and display stuff. */
char Color_Bits;

/* Options and display stuff. */
int Display_Started;
int Abort_Test_Every;
int Experimental_Flag;

/* Current stage of the program. */
int Stage;

/* Stop flag (for abort etc). */
volatile int Stop_Flag;

extern TEXTURE *Default_Texture;
extern CAMERA *Default_Camera;


/*****************************************************************************
* Local variables
******************************************************************************/

char Actual_Output_Name[FILE_NAME_LENGTH];

/* Flag if close_all() was already called. */
static int closed_flag;


/*****************************************************************************
* Static functions
******************************************************************************/

void init_vars(void);

static void destroy_libraries (void);
static void fix_up_rendering_window (void);
static void fix_up_animation_values (void);
static void fix_up_scene_name (void);
static void FrameLoop (void);
static void FrameRender (void);
static void variable_store (int Flag);
static void init_shellouts (void);

void close_all();

/*****************************************************************************
*
* FUNCTION
*
*   main
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

#ifndef ALTMAIN
int main(
#ifndef NOCMDLINE
 int argc, char **argv
#endif
)
{
   int ret = 0;

   argc = GETCOMMANDLINE(argc, argv);

   #ifndef NOCMDLINE
    /* Print help screens. */
    if(argc == 1)
    {
       Print_Help_Screens();
       return 0;
    }
    else if(argc == 2)
    {
       if((pov_stricmp(argv[1], "-h") == 0) ||
          (pov_stricmp(argv[1], "-?") == 0) ||
          (pov_stricmp(argv[1], "--help") == 0) ||
          (pov_stricmp(argv[1], "-help") == 0))
       {
          Print_Help_Screens();
          return 0;
       }
       else if(argv[1][0] == '-')
       {
          if(argv[1][1] == '?')
          {
            Usage(argv[1][2] - '0', 0);
            return 0;
          }
          else if(strlen(argv[1]) == 6)
          {
            if(((argv[1][1] == 'h') || (argv[1][1] == 'H')) &&
               ((argv[1][2] == 'e') || (argv[1][2] == 'E')) &&
               ((argv[1][3] == 'l') || (argv[1][3] == 'L')) &&
               ((argv[1][4] == 'p') || (argv[1][4] == 'P')))
            {
              Usage(argv[1][5] - '0', 0);
              return 0;
            }
          }
       }
    }
   #endif

   /* Init */
   povray_init();

   /* Render */
   #ifdef NOCMDLINE
    ret = povray_render();
   #else
    ret = povray_render(argc, argv);
   #endif

   /* Finish */
   povray_terminate();

   return ret;
}
#endif

/*****************************************************************************
*
* FUNCTION
*
*   povray_init
*
* INPUT -- none
*
* OUTPUT
*
* RETURNS
*
* AUTHOR -- CEY
*
* DESCRIPTION
*
*   This routine does essential initialization that is required before any
*   POV_MALLOC-like routines may be called and before any text streams and
*   the POVMS may be used.
*   
*   If you are not using any built-in main and need access to any part of
*   the generic code before povray_render is called, you MUST call this routine
*   first!  Also note that it is safe to call it twice. If you don't call it,
*   povray_render will. It won't hurt if you both do it.
*   
* CHANGES
*   Nov 1995 : Created by CEY
*
******************************************************************************/

#ifdef USE_POVRAY_COOPERATE
void *
#else
void
#endif
povray_init(
#ifdef USE_POVRAY_COOPERATE
void *povmsrootptr
#endif
)
{
   Stage = STAGE_PREINIT;

   if (pre_init_flag == 0)
   {
      int err;

#ifdef USE_POVRAY_COOPERATE
      povmsrootptr = POVMS_Initialize(povmsrootptr);
#else
      (void)POVMS_Initialize(NULL);
#endif

      Cooperate_Render_Flag = 0;

      err = POVMS_InstallReceiver(Receive_RenderOptions, kPOVMsgClass_RenderControl, kPOVMsgIdent_RenderOptions);
      if(err == 0)
         err = POVMS_InstallReceiver(Receive_RenderAll, kPOVMsgClass_RenderControl, kPOVMsgIdent_RenderAll);
      if(err == 0)
         err = POVMS_InstallReceiver(Receive_RenderArea, kPOVMsgClass_RenderControl, kPOVMsgIdent_RenderArea);
      if(err == 0)
         err = POVMS_InstallReceiver(Receive_RenderStop, kPOVMsgClass_RenderControl, kPOVMsgIdent_RenderStop);
      if(err == 0)
         err = POVMS_InstallReceiver(Receive_ReadINIFile, kPOVMsgClass_IniOptions, kPOVMsgIdent_ReadINI);
      if(err == 0)
         err = POVMS_InstallReceiver(Receive_WriteINIFile, kPOVMsgClass_IniOptions, kPOVMsgIdent_WriteINI);

      (void)POVMS_ASSERT(err == 0, "Installing POVMS receive handler functions failed.");

      Send_InitInfo();
   }

   /* Initialize memory. */
   POV_MEM_INIT();

   /* Initialize streams. In USERIO.C */
   Init_Text_Streams();

   init_shellouts();

   pre_init_tokenizer();

   pre_init_flag = 1234;

#ifdef USE_POVRAY_COOPERATE
   return povmsrootptr;
#endif
}

/*****************************************************************************
*
* FUNCTION
*
*   povray_render
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

#ifndef USE_POVRAY_COOPERATE
int povray_render(
#ifndef NOCMDLINE
 int argc, char **argv
#endif
)
{
#ifndef NOCMDLINE
   POVMSObject obj;
#endif
   int i;
   int err = kNoErr;

   /* Call init */
   povray_init();

   /* Startup povray. */
   Stage = STAGE_STARTUP;
   STARTUP_POVRAY

   /* Print banner and credit info. */
   Stage = STAGE_BANNER;
   PRINT_CREDITS
   PRINT_OTHER_CREDITS

   /* Initialize variables. */
   init_vars();

   Stage = STAGE_ENVIRONMENT;

   READ_ENV_VAR

   Stage = STAGE_INI_FILE;

   Cooperate_Render_Flag = 1;

   /* Read parameters from POVRAY.INI */
   PROCESS_POVRAY_INI

#ifndef NOCMDLINE
   /* Parse the command line parameters */
   Stage = STAGE_COMMAND_LINE;

   err = POVMSObject_New(&obj, kPOVObjectClass_ROptions);
   if(POVMS_ASSERT(err == kNoErr, "POVMSObject_New failed, cannot read command line.") == false)
      povray_exit(err);

   for(i = 1 ;i < argc; i++)
   {
      if(povray_addoptionline2object(argv[i], &obj) != kNoErr)
	     Error("Problem with command line or INI file.");
   }

   Stage = STAGE_INI_FILE;
 
   if(Receive_RenderOptions(&obj, NULL, 0) != kNoErr)
	  Error("Problem processing render options.");

   POVMSObject_Delete(&obj);
#endif

   /* Strip path and extension off input name to create scene name */
   fix_up_scene_name ();
   
   /* Redirect text streams [SCD 2/95] */
   Open_Text_Streams();

   /* Write .INI file [SCD 2/95] */
   Write_INI_File();

   ALT_WRITE_INI_FILE

   /* Make sure clock is okay, validate animation parameters */
   fix_up_animation_values();

   /* Fix-up rendering window values if necessary. */
   fix_up_rendering_window();

   /* Set output file handle for options screen. */
   init_output_file_handle();

   /* Print options used. */
   Print_Options();
   Send_RenderOptions();

   /* Enter the frame loop */
   FrameLoop();

   Do_Cooperate(0);

   /* Clean up */
   close_all();
   POV_MEM_RELEASE_ALL();
 
   Send_RenderDone();

   return 0;
}
#endif

/*****************************************************************************
*
* FUNCTION
*
*   povray_terminate
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

void povray_terminate()
{
   close_all();
   POV_MEM_RELEASE_ALL();

   FINISH_POVRAY;
}

/*****************************************************************************
*
* FUNCTION
*
*   povray_exit
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

void povray_exit(int i)
{
   if (Stop_Flag)
   {
     Render_Info("\nAborting render...\n");

     if(POV_SHELLOUT(USER_ABORT_SHL) != FATAL_RET)
     {
       Status_Info("\nUser abort.\n");
     }
     else
     {
       // Do *not* call "Error" here because it would in turn call povray_exit! [trf]
       PossibleError("Fatal error in User_Abort_Command.");
     }
   }

   if ( Stage == STAGE_PARSING ) 
   {
     Terminate_Tokenizer();
     Destroy_Textures(Default_Texture); 
     Destroy_Camera(Default_Camera); 
   }

   Do_Cooperate(0);

   pre_init_flag=1;

   Cooperate_Render_Flag = 3;

   EXIT_POVRAY(i); /* Must call exit(i) or somehow stop */
}

/*****************************************************************************
*
* FUNCTION
*
*   povray_cooperate
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

#ifdef USE_POVRAY_COOPERATE
void povray_cooperate()
{
   POVRAY_BEGIN_COOPERATE

   switch(Cooperate_Render_Flag)
   {
      case 0:
         Cooperate_Render_Flag = 1;

         // Call init
         (void *)povray_init(NULL);

         // Startup povray
         Stage = STAGE_STARTUP;
         STARTUP_POVRAY

         // Print banner and credit info
         Stage = STAGE_BANNER;
         PRINT_CREDITS
         PRINT_OTHER_CREDITS

         // Initialize variables
         init_vars();

         Stage = STAGE_ENVIRONMENT;
         READ_ENV_VAR

         Stage = STAGE_INI_FILE;
         // Read parameters from POVRAY.INI
         PROCESS_POVRAY_INI
         break;
      case 1:
         // Take option setting message, only render message receive
         // handler switches to next state
         break;
      case 2:
         // Strip path and extension off input name to create scene name
         fix_up_scene_name();

          // Write .INI file [SCD 2/95]
         Write_INI_File();

         // Make sure clock is okay, validate animation parameters
         fix_up_animation_values();
         // Fix-up rendering window values if necessary
         fix_up_rendering_window();

         // Redirect text streams [SCD 2/95]
         Open_Text_Streams();
         // Set output file handle for options screen
         init_output_file_handle();
         // Print options used.
         Print_Options();
         Send_RenderOptions();

         // Enter the frame loop
         FrameLoop();

      case 3:
         // Clean up
         close_all();
         POV_MEM_RELEASE_ALL();

         Send_RenderDone();

         Cooperate_Render_Flag = 1;

         init_vars();

         Stage = STAGE_INI_FILE;
         // Read parameters from POVRAY.INI
         PROCESS_POVRAY_INI
         break;
   }

   POVRAY_END_COOPERATE

   Do_Cooperate(2);
}
#endif

/*****************************************************************************
*
* FUNCTION
*
*   FrameLoop
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

static void FrameLoop()
{
   int Diff_Frame;
   DBL Diff_Clock;
   SHELLRET Pre_Scene_Result, Frame_Result;

   Diff_Clock = opts.FrameSeq.FinalClock - opts.FrameSeq.InitialClock;

   if(opts.Options & CYCLIC_ANIMATION)
      Diff_Frame = opts.FrameSeq.FinalFrame - opts.FrameSeq.InitialFrame + 1;
   else
      Diff_Frame = opts.FrameSeq.FinalFrame - opts.FrameSeq.InitialFrame;

   Clock_Delta = ((Diff_Frame == 0) ? 0 : Diff_Clock/Diff_Frame);

   // Execute the first shell-out command
   Pre_Scene_Result = (POV_SHELLOUT_CAST)POV_SHELLOUT(PRE_SCENE_SHL);

   // Loop over each frame

   if(Pre_Scene_Result != ALL_SKIP_RET)
   {
      if(Pre_Scene_Result != SKIP_ONCE_RET)
      {
         for(opts.FrameSeq.FrameNumber = opts.FrameSeq.InitialFrame,
             opts.FrameSeq.Clock_Value = opts.FrameSeq.InitialClock;

             opts.FrameSeq.FrameNumber <= opts.FrameSeq.FinalFrame;

             // ISO/IEC 14882:1998(E) section 5.18 Comma operator [expr.comma] (page 90) says
             // that comma expressions are evaluated left-to-right, and according to section
             // 6.5.3 The for statement [stmt.for] (page 97) the following is an expression.
             // I just hope all compilers really know about the standard... [trf]
             opts.FrameSeq.FrameNumber++,
             opts.FrameSeq.Clock_Value = opts.FrameSeq.InitialClock +
               (Clock_Delta * (DBL)(opts.FrameSeq.FrameNumber - opts.FrameSeq.InitialFrame)))
         {
            setup_output_file_name();

            // Execute a shell-out command before tracing

            Frame_Result = (POV_SHELLOUT_CAST)POV_SHELLOUT(PRE_FRAME_SHL);
               
            if(Frame_Result == ALL_SKIP_RET)
               break;

            if(Frame_Result != SKIP_ONCE_RET)
            {
               FrameRender();

               // Execute a shell-out command after tracing

               Frame_Result = (POV_SHELLOUT_CAST)POV_SHELLOUT(POST_FRAME_SHL);
           
               if((Frame_Result == SKIP_ONCE_RET) || (Frame_Result == ALL_SKIP_RET))
                  break;
            }

            Do_Cooperate(1);
         }

         // Print total stats ...

         if(opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME)
         {
            opts.FrameSeq.FrameNumber--;

            Send_RenderTime(0, 0, 0);
            PRINT_STATS("Total Statistics", totalstats);
            Send_RenderStatistics();

            opts.FrameSeq.FrameNumber++;
         }
      }

      // Execute the final shell-out command
      POV_SHELLOUT(POST_SCENE_SHL);
   }
}

/*****************************************************************************
*
* FUNCTION
*
*   FrameRender
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
*   Do all that is necessary for rendering a single frame, including parsing
*
* CHANGES
*
*   Feb 1996: Make sure we are displaying when doing a mosaic preview [AED]
*
******************************************************************************/

static void FrameRender()
{
   unsigned long hours, minutes;
   DBL seconds, t_total;

   // Store start time for parse.
   START_TIME

   // use_slabs can be turned off by Build_Bounding_Slabs after a parse
   opts.Use_Slabs=true;

   tparse_frame = tphoton_frame = trender_frame = 0.0;

   // Parse the scene file.
   Status_Info("\n\nParsing...");
   Send_RenderStatus("Parsing");
   Send_RenderTime(0, 0, 0);

   opts.Do_Stats = false;

   // Set up noise-tables
   Initialize_Noise();

   // Set up function VM
   POVFPU_Init();

   // Create blob queue.
   Init_Blob_Queue();

   // Init module specific stuff.
   Initialize_Mesh_Code();

   Parse();

   opts.Do_Stats = true;

   if (opts.Radiosity_Enabled)
      Experimental_Flag |= EF_RADIOS;

   if (Experimental_Flag)
   {
      char str[512] = "" ;

      if (Experimental_Flag & EF_SPLINE)
        strcat (str, str [0] ? ", spline" : "spline") ;
      if (Experimental_Flag & EF_RADIOS)
        strcat (str, str [0] ? ", radiosity" : "radiosity") ;
      if (Experimental_Flag & EF_SLOPEM)
        strcat (str, str [0] ? ", slope pattern" : "slope pattern") ;
      if (Experimental_Flag & EF_ISOFN) 
        strcat (str, str [0] ? ", function '.hf'" : "function '.hf'") ;
      if (Experimental_Flag & EF_TIFF) 
        strcat (str, str [0] ? ", TIFF image support" : "TIFF image support") ;

      Warning(0, "This rendering uses the following experimental feature(s): %s.\n"
                 "The design and implementation of these features is likely to change in future versions\n"
                 "of POV-Ray. Full backward compatibility with the current implementation is NOT guaranteed.",
                 str);
   }

   Experimental_Flag = 0;

   // Switch off standard anti-aliasing.

   if((Frame.Camera->Aperture != 0.0) && (Frame.Camera->Blur_Samples > 0))
   {
      opts.Options &= ~ANTIALIAS;

      Warning(0, "Focal blur is used. Standard antialiasing is switched off.");
   }

   // Create the bounding box hierarchy.

   Stage = STAGE_SLAB_BUILDING;

   if(opts.Use_Slabs)
   {
      Status_Info("\nCreating bounding slabs.");
      Send_RenderStatus("Creating bounding slabs");
   }

   // Init module specific stuff.
   Initialize_Atmosphere_Code();
   Initialize_BBox_Code();
   Initialize_Lighting_Code();
   Initialize_VLBuffer_Code();
   Initialize_Radiosity_Code();

   // Always call this to print number of objects.
   Build_Bounding_Slabs(&Root_Object);

   // Create the vista buffer.
   Build_Vista_Buffer();

   // Create the light buffers.
   Build_Light_Buffers();

   // Save variable values.
   variable_store(STORE);

   /* Get the parsing time. */
   STOP_TIME
   tparse = TIME_ELAPSED

   if (photonOptions.photonsEnabled)
   {
     /* Store start time for photons. */
     START_TIME

     /* now backwards-trace the scene and build the photon maps */
     InitBacktraceEverything();
     BuildPhotonMaps();

     /* Get the photon-shooting time. */
     STOP_TIME
     tphoton = TIME_ELAPSED

     /* Get total parsing time. */
     tphoton_total += tphoton;
     tphoton_frame = tphoton;
     tphoton = 0;
   }

   /* Store start time for the rest of parsing. */
   START_TIME

   // Open output file and if we are continuing an interrupted trace,
   // read in the previous file settings and any data there.  This has to
   // be done before any image-size related allocations, since the settings
   // in a resumed file take precedence over that specified by the user. [AED]
   open_output_file();

   // Start the display.
   if(opts.Options & DISPLAY)
   {
      Status_Info ("\nDisplaying...");
      Send_RenderStatus("Displaying");

      Display_Started = POV_DISPLAY_INIT(opts.Preview_RefCon, Frame.Screen_Width, Frame.Screen_Height);

      // Display vista tree.
      Draw_Vista_Buffer();
   }
   else
   {
      Display_Started = false;
   }

   // Get things ready for ray tracing (misc init, mem alloc)
   Initialize_Renderer();

   // This had to be taken out of open_output_file() because we don't have
   // the final image size until the output file has been opened, so we can't
   // initialize the display until we know this, which in turn means we can't
   // read the rendered part before the display is initialized. [AED]
   if((opts.Options & DISKWRITE) && (opts.Options & CONTINUE_TRACE))
   {
      Read_Rendered_Part(Actual_Output_Name);

      if (opts.Last_Line > Frame.Screen_Height)
         opts.Last_Line = Frame.Screen_Height;

      if (opts.Last_Column > Frame.Screen_Width)
         opts.Last_Column = Frame.Screen_Width;
   }

   // Get the rest of the parsing time.
   STOP_TIME
   tparse += TIME_ELAPSED

   Send_RenderTime(0, 0, 0);

   // Store start time for trace.
   START_TIME

   if(opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME)
   {
      t_total=tparse_total+tphoton_total+trender_total;
      SPLIT_TIME(t_total,&hours,&minutes,&seconds);
      Render_Info("\n %02ld:%02ld:%02.0f so far, ",hours,minutes,seconds);
      Render_Info("Rendering frame %d, going to %d.",
                  opts.FrameSeq.FrameNumber, opts.FrameSeq.FinalFrame);
      Send_RenderStatus("Rendering");
      Send_RenderTime(0, 0, 0);
   }

   // Get total parsing time.
   tparse_total += tparse;
   tparse_frame = tparse;
   tparse = 0;

   // Start tracing.
   Stage = STAGE_RENDERING;

   POV_PRE_RENDER

   Status_Info("\nRendering...\r");
   Send_RenderStatus("Rendering");

   // Macro for setting up any special FP options
   CONFIG_MATH

   // Ok, go for it - trace the picture.

   // If radiosity preview has been done, we are continuing a trace, so it
   // is important NOT to do the preview, even if the user requests it, as it
   // will cause discontinuities in radiosity shading by (probably) calculating
   // a few more radiosity values.

   // Note that radiosity REQUIRES a mosaic preview prior to main scan
   if ( opts.Radiosity_Enabled && !opts.Radiosity_Preview_Done)
      Start_Tracing_Radiosity_Preview(opts.PreviewGridSize_Start, opts.PreviewGridSize_End);

   else if((opts.Options & PREVIEW) && (opts.Options & DISPLAY))
      Start_Tracing_Mosaic_Preview(opts.PreviewGridSize_Start, opts.PreviewGridSize_End);

   switch(opts.Tracing_Method)
   {
      case 2:
         Start_Adaptive_Tracing();
         break;
      case 1:
      default:
         Start_Non_Adaptive_Tracing();
   }

   // We're done.

   // Record time so well spent before file close so it can be in comments
   STOP_TIME
   trender = TIME_ELAPSED

   // shutdown (freeing memory) does not get included in the time!

   // Get total render time.
   trender_total += trender;
   trender_frame = trender;
   trender = 0;

   // Close out our file
   if(Output_File != NULL)
   {
      POV_DELETE(Output_File, Image_File_Class);
      Output_File = NULL;
   }

   // For all those who never rtfm [trf]
   if((Highest_Trace_Level >= Max_Trace_Level) && (Had_Max_Trace_Level == false))
      PossibleError("Maximum trace level reached! If your scene contains black spots\nread more about the max_trace_level setting in the documentation!");

   Stage = STAGE_SHUTDOWN;

   POV_PRE_SHUTDOWN

   // DESTROY lots of stuff
   /* NK phmap */
   FreeBacktraceEverything();
   Deinitialize_Atmosphere_Code();
   Deinitialize_BBox_Code();
   Deinitialize_Lighting_Code();
   Deinitialize_Mesh_Code();
   Deinitialize_VLBuffer_Code();
   Deinitialize_Radiosity_Code();
   Destroy_Blob_Queue();
   Destroy_Light_Buffers();
   Destroy_Vista_Buffer();
   Destroy_Bounding_Slabs();
   Destroy_Frame();
   Terminate_Renderer();
   FreeFontInfo();
   Free_Iteration_Stack();
   Free_Noise_Tables();

   POVFPU_Terminate();

   POV_POST_SHUTDOWN

   if((opts.Options & DISPLAY) && Display_Started)
   {
      POV_DISPLAY_FINISHED(opts.Preview_RefCon);

      POV_DISPLAY_CLOSE(opts.Preview_RefCon);

      Display_Started = false;
   }

   if(opts.histogram_on)
      write_histogram(opts.Histogram_File_Name);

   Status_Info("\nDone Tracing");
   Send_RenderStatus("Done Tracing");
   Send_RenderTime(0, 0, 0);

   // Print stats ...
   PRINT_STATS("Statistics", stats);
   Send_RenderStatistics();

   if(opts.FrameSeq.FrameType == FT_MULTIPLE_FRAME)
   {
      // Add them up
      sum_statistics(totalstats, stats);

      // ... and then clear them for the next frame
      init_statistics(stats);
   }

   // Restore variable values.
   variable_store(RESTORE);
}

/*****************************************************************************
*
* FUNCTION
*
*   Do_Cooperate
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

void Do_Cooperate(int level)
{
   int maxMsgCalls = 20;

   switch(level)
   {
      case 2:
      default:
         COOPERATE_2
      case 0:
#ifndef DONT_USE_POVMS
         do
         {
            if(POVMS_ProcessMessages() == 0)
               break;
            maxMsgCalls--;
         } while(maxMsgCalls > 0);
#endif
         COOPERATE_0
         break;
      case 1:
         COOPERATE_1
         break;
   };
}

/*****************************************************************************
*
* FUNCTION
*
*   fix_up_rendering_window
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
*   Fix wrong window and mosaic preview values.
*
* CHANGES
*
*   -
*
******************************************************************************/

static void fix_up_rendering_window()
{
  int temp;
  
  if (opts.First_Column_Percent > 0.0)
    opts.First_Column = (int) (Frame.Screen_Width * opts.First_Column_Percent);

  if (opts.First_Line_Percent > 0.0)
    opts.First_Line = (int) (Frame.Screen_Height * opts.First_Line_Percent);

  /* The decrements are a fudge factor that used to be in OPTIN.C
   * but it messed up Write_INI_File so its moved here.
   */

  if (opts.First_Column <= 0)
    opts.First_Column = 0;
  else
    opts.First_Column--;

  if (opts.First_Line <= 0)
    opts.First_Line = 0;
  else
    opts.First_Line--;
  
  if ((opts.Last_Column == -1) && (opts.Last_Column_Percent <= 1.0))
    opts.Last_Column = (int) (Frame.Screen_Width * opts.Last_Column_Percent);

  if ((opts.Last_Line == -1) && (opts.Last_Line_Percent <= 1.0))
    opts.Last_Line = (int) (Frame.Screen_Height * opts.Last_Line_Percent);

  if (opts.Last_Line == -1)
    opts.Last_Line = Frame.Screen_Height;

  if (opts.Last_Column == -1)
    opts.Last_Column = Frame.Screen_Width;

  if (opts.Last_Column < 0 || opts.Last_Column > Frame.Screen_Width)
    opts.Last_Column = Frame.Screen_Width;

  if (opts.Last_Line > Frame.Screen_Height)
    opts.Last_Line = Frame.Screen_Height;

  /* Fix up Mosaic Preview values */
  opts.PreviewGridSize_Start=max(1,opts.PreviewGridSize_Start);
  opts.PreviewGridSize_End=max(1,opts.PreviewGridSize_End);

  if ((temp=closest_power_of_2((unsigned)opts.PreviewGridSize_Start))!=opts.PreviewGridSize_Start)
  {
     Warning(0,"Preview_Start_Size must be a power of 2. Changing to %d.",temp);
     opts.PreviewGridSize_Start=temp;
  }

  if ((temp=closest_power_of_2((unsigned)opts.PreviewGridSize_End))!=opts.PreviewGridSize_End)
  {
     Warning(0,"Preview_End_Size must be a power of 2. Changing to %d.",temp);
     opts.PreviewGridSize_End=temp;
  }

  /* End must be less than or equal to start */
  if (opts.PreviewGridSize_End > opts.PreviewGridSize_Start)
    opts.PreviewGridSize_End = opts.PreviewGridSize_Start;
    
  if (opts.PreviewGridSize_Start > 1)
  {
     opts.PreviewGridSize_End=max(opts.PreviewGridSize_End,2);
     opts.Options |= PREVIEW;
  }
  else
  {
     opts.Options &= ~PREVIEW;
  }

  /* Set histogram size here so it is available for Print_Options, and
   * make sure that it has an integer number of pixels/bucket. */
  if (opts.histogram_on)
  {
    if (opts.histogram_x == 0 || opts.histogram_x > Frame.Screen_Width)
      opts.histogram_x = Frame.Screen_Width;
    else if (opts.histogram_x < Frame.Screen_Width)
      opts.histogram_x = Frame.Screen_Width / ((Frame.Screen_Width +
                         opts.histogram_x - 1) / opts.histogram_x);

    if (opts.histogram_y == 0 || opts.histogram_y > Frame.Screen_Height)
      opts.histogram_y = Frame.Screen_Height;
    else if (opts.histogram_y < Frame.Screen_Height)
      opts.histogram_y = Frame.Screen_Height / ((Frame.Screen_Height +
                         opts.histogram_y - 1) /opts.histogram_y);
  }
}

/*****************************************************************************
*
* FUNCTION
*
*   fix_up_animation_values
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
*   Validate animation parameters, compute subset values
*
* CHANGES
*
*   -
*
******************************************************************************/
static void fix_up_animation_values()
{
  DBL ClockDiff;
  int FrameDiff;
  int FrameIncr;
  DBL ClockPerFrameIncr;
  int NumFrames;

  /*
   * Added because that is no animation. [trf]
   */
  if ((opts.FrameSeq.InitialFrame == opts.FrameSeq.FinalFrame) &&
      ((opts.FrameSeq.FinalFrame == 0) || (opts.FrameSeq.FinalFrame == 1)))
  {
     opts.FrameSeq.InitialFrame = -1;
     opts.FrameSeq.FinalFrame = -1;
  }

  if (opts.FrameSeq.FinalFrame != -1)
  {
    opts.FrameSeq.FrameType = FT_MULTIPLE_FRAME;

    if (opts.FrameSeq.Clock_Value != 0.0)
    {
       Warning(0,"Attempted to set single clock value in multi frame\n"
                 "animation. Clock value overridden.");
    }
  }
  else
  {
    if (opts.FrameSeq.Clock_Value != 0.0)
    {
       opts.FrameSeq.FrameType = FT_SINGLE_FRAME;
    }
  }

  if (opts.FrameSeq.FrameType == FT_SINGLE_FRAME)
  {
    /*
     * These are dummy values that will work for single_frame,
     * even in an animation loop.
     */

    opts.FrameSeq.InitialFrame = 0;
    opts.FrameSeq.FinalFrame   = 0;
    opts.FrameSeq.InitialClock = opts.FrameSeq.Clock_Value;
    opts.FrameSeq.FinalClock   = 0.0;
  }
  else
  {
    /* FrameType==FT_MULTIPLE_FRAME */

    if(opts.FrameSeq.InitialFrame == -1)
    {
      opts.FrameSeq.InitialFrame = 1;
    }

    if (opts.FrameSeq.FinalFrame < opts.FrameSeq.InitialFrame)
    {
      Error("Final frame %d is less than Start Frame %d.",
            opts.FrameSeq.FinalFrame, opts.FrameSeq.InitialFrame);
    }

    ClockDiff = opts.FrameSeq.FinalClock-opts.FrameSeq.InitialClock;

    if (opts.Options & CYCLIC_ANIMATION)
    {
      FrameDiff = opts.FrameSeq.FinalFrame-opts.FrameSeq.InitialFrame+1;
    }
    else
    {
      FrameDiff = opts.FrameSeq.FinalFrame-opts.FrameSeq.InitialFrame;
    }

    ClockPerFrameIncr = (FrameDiff == 0) ? 0 : (ClockDiff/FrameDiff);

    /* Calculate width, which is an integer log10 */

    NumFrames = opts.FrameSeq.FinalFrame;

    opts.FrameSeq.FrameNumWidth = 1;

    while (NumFrames >= 10)
    {
      opts.FrameSeq.FrameNumWidth++;

      NumFrames = NumFrames / 10;
    }

    if (opts.FrameSeq.FrameNumWidth > POV_NAME_MAX-1)
    {
      Error("Cannot render %d frames requiring %d chars with %d width filename.",
          opts.FrameSeq.FinalFrame - opts.FrameSeq.InitialFrame + 1,
          opts.FrameSeq.FrameNumWidth, POV_NAME_MAX);
    }

    /* STARTING FRAME SUBSET */

    if (opts.FrameSeq.SubsetStartPercent != DBL_VALUE_UNSET)
    {
      FrameIncr = FrameDiff * opts.FrameSeq.SubsetStartPercent + 0.5; /* w/rounding */

      opts.FrameSeq.SubsetStartFrame = opts.FrameSeq.InitialFrame + FrameIncr;
    }

    if (opts.FrameSeq.SubsetStartFrame != INT_VALUE_UNSET)
    {
      NumFrames = opts.FrameSeq.SubsetStartFrame - opts.FrameSeq.InitialFrame;

      opts.FrameSeq.InitialFrame = opts.FrameSeq.SubsetStartFrame;
      opts.FrameSeq.InitialClock = opts.FrameSeq.InitialClock + NumFrames * ClockPerFrameIncr;
    }

    /* ENDING FRAME SUBSET */

    if (opts.FrameSeq.SubsetEndPercent != DBL_VALUE_UNSET)
    {
      /*
       * By this time, we have possibly lost InitialFrame, so we calculate
       * it via FinalFrame-FrameDiff
       */

      FrameIncr = FrameDiff * opts.FrameSeq.SubsetEndPercent + 0.5; /* w/rounding */

      opts.FrameSeq.SubsetEndFrame = (opts.FrameSeq.FinalFrame - FrameDiff) + FrameIncr;
    }

    if (opts.FrameSeq.SubsetEndFrame != INT_VALUE_UNSET)
    {
      NumFrames = opts.FrameSeq.FinalFrame - opts.FrameSeq.SubsetEndFrame;

      opts.FrameSeq.FinalFrame = opts.FrameSeq.SubsetEndFrame;
      opts.FrameSeq.FinalClock = opts.FrameSeq.FinalClock - NumFrames * ClockPerFrameIncr;
    }

    /*
     * Now that we have everything calculated, we check FinalFrame
     * and InitialFrame one more time, in case the subsets messed them up
     */

    if (opts.FrameSeq.FinalFrame < opts.FrameSeq.InitialFrame)
    {
      Error("Final frame %d is less than Start Frame %d due to bad subset specification.",
            opts.FrameSeq.FinalFrame, opts.FrameSeq.InitialFrame);
    }
  }

  /* Needed for pre-render shellout fixup */

  opts.FrameSeq.FrameNumber = opts.FrameSeq.InitialFrame;
  opts.FrameSeq.Clock_Value = opts.FrameSeq.InitialClock;
}

/*****************************************************************************
*
* FUNCTION
*
*   fix_up_scene_name
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
*   Strip path and extention of input file to create scene name
*
* CHANGES
*
******************************************************************************/

static void fix_up_scene_name()
{
  int i, l;
  char temp[FILE_NAME_LENGTH];
  
  if ((l=strlen(opts.Input_File_Name)-1)<1)
  {
     strcpy(opts.Scene_Name,opts.Input_File_Name);
     return;
  }

  strcpy(temp,opts.Input_File_Name);
  for (i=l;i>0;i--)
  {
     if (temp[i]==FILENAME_SEPARATOR)
     {
        break;
     }
     if (temp[i]=='.')
     {
        temp[i]=0;
        break;
     }
  }

  i=strlen(temp)-1;
  
  while ((i>0) && (temp[i]!=FILENAME_SEPARATOR))
    i--;
  if (temp[i]==FILENAME_SEPARATOR)
    i++;
  strcpy(opts.Scene_Name,&(temp[i]));

  if (opts.Language_Version > OFFICIAL_VERSION_NUMBER)
  {
     Error("Your scene file requires POV-Ray version %g or later!\n", (DBL)(opts.Language_Version / 100.0));
  }
}

/*****************************************************************************
*
* FUNCTION
*
*   init_vars
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
*   Initialize all global variables.
*
* CHANGES
*
*   -
*
******************************************************************************/

void init_vars()
{
  Stage=STAGE_INIT;
  opts.Abort_Test_Counter = Abort_Test_Every ;
  Abort_Test_Every = 1;
  opts.AntialiasDepth = 3;
  opts.Antialias_Threshold = 0.3;
  opts.BBox_Threshold = 25;
  Color_Bits = 8;
  opts.DisplayFormat = '0';
  Display_Started = false;
  opts.File_Buffer_Size = 0;
  opts.First_Column = 0;
  opts.First_Column_Percent = 0.0;
  opts.First_Line = 0;
  opts.First_Line_Percent = 0.0;
  Frame.Screen_Height = 100;
  Frame.Screen_Width  = 100;
  Root_Object = NULL;
  free_istack = NULL;
  opts.JitterScale = 1.0;
  opts.Language_Version = OFFICIAL_VERSION_NUMBER;
  opts.Last_Column = -1;
  opts.Last_Column_Percent = 1.0;
  opts.Last_Line = -1;
  opts.Last_Line_Percent = 1.0;
  opts.PreviewGridSize_Start = 1;
  opts.PreviewGridSize_End   = 1;
  opts.Library_Paths[0] = NULL;
  opts.Library_Path_Index = 0;
  Max_Intersections = 64; /*128*/
  Number_Of_Files = 0;
  Number_of_istacks = 0;

  opts.Options = USE_VISTA_BUFFER + USE_LIGHT_BUFFER + JITTER +
                 DISKWRITE + REMOVE_BOUNDS;
  opts.OutputFormat = DEFAULT_OUTPUT_FORMAT;
  opts.OutputQuality = 8;
  Output_File = NULL;
  opts.Output_Numbered_Name[0]='\0';
  opts.Output_File_Name[0]='\0';
  opts.Output_Path[0]='\0';
  opts.Output_File_Type=0;
  opts.PaletteOption = '3';
  opts.Quality = 9;
  opts.Quality_Flags = QUALITY_9;
  opts.DisplayGamma = DEFAULT_DISPLAY_GAMMA;

  opts.Header_File_Name[0] = '\0';

  /* 
   * If DisplayGamma == 2.2, then GammaFactor == .45, which is what we want.
   */
  opts.GammaFactor = DEFAULT_ASSUMED_GAMMA/opts.DisplayGamma;

  opts.FrameSeq.FrameType = FT_SINGLE_FRAME;
  opts.FrameSeq.Clock_Value = 0.0;
  opts.FrameSeq.InitialFrame = 1;
  opts.FrameSeq.InitialClock = 0.0;
  opts.FrameSeq.FinalFrame = INT_VALUE_UNSET;
  opts.FrameSeq.FrameNumWidth = 0;
  opts.FrameSeq.FinalClock = 1.0;
  opts.FrameSeq.SubsetStartFrame = INT_VALUE_UNSET;
  opts.FrameSeq.SubsetStartPercent = DBL_VALUE_UNSET;
  opts.FrameSeq.SubsetEndFrame = INT_VALUE_UNSET;
  opts.FrameSeq.SubsetEndPercent = DBL_VALUE_UNSET;
  opts.FrameSeq.Field_Render_Flag = false;
  opts.FrameSeq.Odd_Field_Flag = false;

  /* NK rad - these default settings are low quality
  for relatively high quality, use

  opts.Radiosity_Nearest_Count = 8;
  opts.Radiosity_Count = 100;
  opts.Radiosity_Recursion_Limit = 5;

  Only these variables should need adjustment
  */

  opts.Radiosity_Brightness = 1.0;
  opts.Radiosity_Count = 35;
  opts.Radiosity_Dist_Max = 0.0;   /* NK rad - dist_max is always computed on the fly now - FYI */
  opts.Radiosity_Error_Bound = 1.8;
  opts.Radiosity_Gray = 0.0;       /* degree to which gathered light is grayed */
  opts.Radiosity_Low_Error_Factor = 0.5;
  opts.Radiosity_Min_Reuse = 0.015;
  opts.Radiosity_Nearest_Count = 5;
  opts.Radiosity_Recursion_Limit = 3;
  opts.Radiosity_Quality = 6;     /* Q-flag value for light gathering */
  opts.Radiosity_File_ReadOnContinue = 1;
  opts.Radiosity_File_SaveWhileRendering = 1;
  opts.Radiosity_File_AlwaysReadAtStart = 0;
  opts.Radiosity_File_KeepOnAbort = 1;
  opts.Radiosity_File_KeepAlways = 0;
  opts.Maximum_Sample_Brightness = -1.0;  /* default max brightness allows any */
  opts.Radiosity_ADC_Bailout = 0.01;     /* use a fairly high default adc_bailout for rad */
  opts.Radiosity_Use_Normal = false;
  opts.Radiosity_Use_Media = false;
  opts.radPretraceStart = 0.08;
  opts.radPretraceEnd = 0.04;
  opts.Radiosity_Load_File_Name = NULL;
  opts.Radiosity_Save_File_Name = NULL;
  opts.Radiosity_Add_On_Final_Trace = true;
  opts.Radiosity_Enabled = false;

  Current_Line_Number = 0;

  init_statistics(stats);
  init_statistics(totalstats);

  strcpy (opts.Input_File_Name, "OBJECT.POV");
  opts.Scene_Name[0]='\0';
  opts.Ini_Output_File_Name[0]='\0';
  opts.Use_Slabs=true;
  Num_Echo_Lines = POV_NUM_ECHO_LINES;   /* May make user setable later - CEY*/
  Echo_Line_Length = POV_ECHO_LINE_LENGTH;   /* May make user setable later - CEY*/

  closed_flag = false;
  Stop_Flag = false;

  trender = trender_frame = trender_total = 0.0;
  tparse  = tparse_frame  = tparse_total  = 0.0;
  tphoton = tphoton_frame = tphoton_total = 0.0;

  histogram_grid = NULL;
  opts.histogram_on = false;
  opts.histogram_type = NONE;
  opts.histogram_file_type=0;
  opts.Histogram_File_Name[0] = '\0';
  Histogram_File = NULL;
  /*
   * Note that late initialization of the histogram_x and histogram_y
   * variables is done in fix_up_rendering_window, if they aren't specified
   * on the command line.  This is because they are based on the image
   * dimensions, and we can't be certain that we have this info at the
   * time we parse the histogram options in optin.c. [AED]
   */
  opts.histogram_x = opts.histogram_y = 0;
  max_histogram_value = 0;

  opts.Tracing_Method = 1;
  Experimental_Flag = 0;
  Make_Pigment_Entries();

  opts.Preview_RefCon = 0;

  opts.Warning_Level = 10; // all warnings

  opts.String_Encoding = 0; // ASCII

  (void)POVMSAttrList_New(&opts.Declared_Variables); // we have to be careful... [trf]

  /* NK phmap */
  backtraceFlag=0;
  photonOptions.photonsEnabled = 0;
  InitBacktraceWasCalled=false;

  photonOptions.photonMap.head = NULL;
  photonOptions.photonMap.numPhotons  = 0;
  photonOptions.photonMap.numBlocks  = 0;

  photonOptions.photonMap.gatherNumSteps = 2;
  photonOptions.photonMap.minGatherRad = -1.0;
  photonOptions.photonMap.minGatherRadMult = 1.0;
#ifdef GLOBAL_PHOTONS
  photonOptions.globalPhotonMap.gatherNumSteps = 1;
  photonOptions.globalPhotonMap.minGatherRad = -1.0;
  photonOptions.globalPhotonMap.minGatherRadMult = 1.0;
#endif
  photonOptions.mediaPhotonMap.gatherNumSteps = 1;
  photonOptions.mediaPhotonMap.minGatherRad = -1.0;
  photonOptions.mediaPhotonMap.minGatherRadMult = 1.0;

  photonOptions.minGatherCount = 20;
  photonOptions.maxGatherCount = 100;

  photonOptions.ADC_Bailout = -1;  /* use the normal adc bailout */
  photonOptions.Max_Trace_Level = -1; /* use the normal max_trace_level */

  photonOptions.jitter = 0.4;
  photonOptions.autoStopPercent = 0.5;

  photonOptions.expandTolerance = 0.2;
  photonOptions.minExpandCount = 35;

  photonOptions.fileName = NULL;
  photonOptions.loadFile = false;

  disp_elem = 0; /* for dispersion */
  disp_nelems = 0;   /* reset this for next pixel's tracing */

  photonOptions.photonGatherList = NULL;
  photonOptions.photonDistances = NULL;

#ifdef GLOBAL_PHOTONS
  /* global photon map */
  photonOptions.globalGatherRad = 10.0;
  photonOptions.globalPhotonsToShoot = 0;
#endif

  photonOptions.surfaceSeparation = 1.0;
  photonOptions.globalSeparation = 1.0;

  photonOptions.photonMap.head = NULL;
  photonOptions.photonMap.numPhotons = 0;
  photonOptions.photonMap.numBlocks = 0;
#ifdef GLOBAL_PHOTONS
  photonOptions.globalPhotonMap.head = NULL;
  photonOptions.globalPhotonMap.numPhotons = 0;
  photonOptions.globalPhotonMap.numBlocks = 0;
#endif
  photonOptions.mediaPhotonMap.head = NULL;
  photonOptions.mediaPhotonMap.numPhotons = 0;
  photonOptions.mediaPhotonMap.numBlocks = 0;

  photonOptions.maxMediaSteps = 0;  /* disable media photons by default */
  photonOptions.mediaSpacingFactor = 1.0;

  photonOptions.photonReflectionBlur = false; /* off by default */

  photonOptions.surfaceCount = 0;
  photonOptions.globalCount = 0;

  Highest_Trace_Level = 0 ;

  /* NK 1999 - bugfix */
  Trace_Level = 0;
  // [trf] Total_Depth = 0.0;
  Radiosity_Trace_Level = 1;

  warpNormalTextures = 0;

  opts.Noise_Generator = 2; /* default is the range-corrected noise, since the perlin noise (gen 3) seems buggy */
}


/*****************************************************************************
*
* FUNCTION
*
*   variable_store
*
* INPUT
*
*   flag - flag telling wether to store or restore variables.
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
*   Store or restore variables whose value has to be the same for all
*   frames of an animation and who are changed during every frame.
*
* CHANGES
*
*   May 1995 : Creation.
*
******************************************************************************/

static void variable_store(int Flag)
{
  static int STORE_First_Line;

  switch (Flag)
  {
    case STORE:

      STORE_First_Line = opts.First_Line;

      break;

    case RESTORE:

      opts.First_Line = STORE_First_Line;

      break;

    default:

      Error("Unknown flag in variable_store().");
  }
}

/*****************************************************************************
*
* FUNCTION
*
*   destroy_libraries
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
*   Free library path memory.
*
* CHANGES
*
*   -
*
******************************************************************************/

static void destroy_libraries()
{
  int i;

  for (i = 0; i < opts.Library_Path_Index; i++)
  {
    POV_FREE(opts.Library_Paths[i]);
    
    opts.Library_Paths[i] = NULL;
  }

  opts.Library_Path_Index = 0;
}



/*****************************************************************************
*
* FUNCTION
*
*   close_all
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
*   Close all the stuff that has been opened and free all allocated memory.
*
* CHANGES
*
*   -
*
******************************************************************************/

void close_all()
{
  /* Only close things once */

  if (closed_flag)
  {
    return;
  }

  FreeBacktraceEverything();

  // Close out our file
  if(Output_File != NULL)
  {
    POV_DELETE(Output_File, Image_File_Class);
    Output_File = NULL;
  }

  destroy_libraries();
  Terminate_Renderer();
  Destroy_Bounding_Slabs();
  Destroy_Blob_Queue();
  Destroy_Vista_Buffer();
  Destroy_Light_Buffers();
  Destroy_Random_Generators();
  Deinitialize_Radiosity_Code();
  Free_Iteration_Stack();
  Free_Noise_Tables();
  destroy_histogram();
  Deinitialize_Atmosphere_Code();
  Deinitialize_BBox_Code();
  Deinitialize_Lighting_Code();
  Deinitialize_Mesh_Code();
  Deinitialize_VLBuffer_Code();
  Destroy_Frame();
  Destroy_IStacks();
  FreeFontInfo();

  POVFPU_Terminate();

  if ((opts.Options & DISPLAY) && Display_Started)
  {
    POV_DISPLAY_CLOSE(opts.Preview_RefCon);
  }

  (void)POVMSAttrList_Delete(&opts.Declared_Variables);

  init_shellouts();
  Destroy_Text_Streams();
  Init_Text_Streams(); /* make sure the text streams are valid again because POVMS may still use them, it 
                          assumes Init_Text_Streams allocates no memory and doesn't not open anything */
  closed_flag = true;
}



/*****************************************************************************
*
* FUNCTION
*
*   pov_shellout
*
* INPUT
*
*   template_command - the template command string to execute
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
*   Execute the command line described by the string being passed in
*
* CHANGES
*
*   -
*
******************************************************************************/

SHELLRET pov_shellout (SHELLTYPE Type)
{
  char real_command[POV_MAX_CMD_LENGTH];
  int i, j, l = 0;
  int length;
  SHELLRET Return_Code;
  char *s = NULL;
  char *template_command;


  if ( opts.Shellouts == NULL ) return(IGNORE_RET);

  template_command=opts.Shellouts[Type].Command;

  if ((length = strlen(template_command)) == 0)
  {
    return(IGNORE_RET);
  }

  switch(Type)
  {
    case PRE_SCENE_SHL:  s="pre-scene";   break;
    case PRE_FRAME_SHL:  s="pre-frame";   break;
    case POST_FRAME_SHL: s="post-frame";  break;
    case POST_SCENE_SHL: s="post-scene";  break;
    case USER_ABORT_SHL: s="user about";  break;
    case FATAL_SHL:      s="fatal error"; break;
    case MAX_SHL: /* To remove warnings*/ break;
  }

  Status_Info("\nPerforming %s shell-out command",s);

  /* First, find the real command */

  for (i = 0, j = 0; i < length; )
  {
    if (template_command[i] == '%')
    {
      switch (toupper(template_command[i+1]))
      {
         case 'O':

          strncpy(&real_command[j], opts.Output_Numbered_Name, 
               (unsigned)(l=strlen(opts.Output_Numbered_Name)));

          break;

         case 'P':

          strncpy(&real_command[j], opts.Output_Path,(unsigned)(l=strlen(opts.Output_Path)));

          break;

         case 'S':

          strncpy(&real_command[j], opts.Scene_Name, (unsigned)(l=strlen(opts.Scene_Name)));

          break;

         case 'N':

          sprintf(&real_command[j],"%d",opts.FrameSeq.FrameNumber);
          l = strlen(&real_command[j]);

          break;

         case 'K':

          sprintf(&real_command[j],"%f",opts.FrameSeq.Clock_Value);
          l = strlen(&real_command[j]);

          break;

         case 'H':

          sprintf(&real_command[j],"%d",Frame.Screen_Height);
          l = strlen(&real_command[j]);

          break;

         case 'W':

          sprintf(&real_command[j],"%d",Frame.Screen_Width);
          l = strlen(&real_command[j]);

          break;

         case '%':

          real_command[j]='%';

          l=1;

          break;
       }

       j+=l;

       i+=2; /* we used 2 characters of template_command */
    }
    else
    {
      real_command[j++]=template_command[i++];
    }
  }

  real_command[j]='\0';

  Return_Code=(POV_SHELLOUT_CAST)POV_SYSTEM(real_command);

  if (opts.Shellouts[Type].Inverse)
  {
    Return_Code=(POV_SHELLOUT_CAST)(!((int)Return_Code));
  }

  if (Return_Code)
  {
    if (Type < USER_ABORT_SHL)
    {
      switch(opts.Shellouts[Type].Ret)
      {
        case FATAL_RET:

          Error("Fatal error returned from shellout command.");

          break;

        case USER_RET:

          Check_User_Abort(true); /* the true forces user abort */

          break;

        case QUIT_RET:

          povray_exit(0);

          break;

        case IGNORE_RET:
        case SKIP_ONCE_RET:
        case ALL_SKIP_RET: /* Added to remove warnings */
          break;
      }
    }

    return(opts.Shellouts[Type].Ret);
  }

  return(IGNORE_RET);
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
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void init_shellouts()
{
  int i;

  for (i=0; i < MAX_SHL; i++)
  {
    opts.Shellouts[i].Ret=IGNORE_RET;
    opts.Shellouts[i].Inverse=false;
    opts.Shellouts[i].Command[0]='\0';
  }
}

#ifdef NEED_INVHYP
DBL asinh(DBL x)
{
	return (x < 0 ? -1 : (x > 0 ? 1 : 0)) * log(fabs(x) + sqrt(1 + x * x));
}

DBL acosh(DBL x)
{
	if(x < 1.0)
		return 0;
	return log(x + sqrt(x * x - 1));
}

DBL atanh(DBL x)
{
	if(fabs(x) >= 1)
		return 0;
	return 0.5 * log((1 + x) / (1 - x));
}
#endif
