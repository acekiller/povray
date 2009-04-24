/****************************************************************************
*                   optin.cpp
*
*  This module contains functions for ini-file/command line parsing, streams.
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
* $File: //depot/povray/3.5/source/optin.cpp $
* $Revision: #45 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

/****************************************************************************
*
*  This file contains the routines to implement an .INI file parser that can
*  parse options in the form "Variable=value" or traditional POV-Ray
*  command-line switches.  Values can come from POVRAYOPT, command-line,
*  .DEF or .INI files.
*
*  Written by CEY 4/94 based on existing code and INI code from CDW.
*
*  ---
*
*****************************************************************************/

#include <ctype.h>
#include <time.h>
#include "frame.h"
#include "povproto.h"
#include "bbox.h"
#include "lighting.h"
#include "pov_mem.h"        /*POV_FREE*/
#include "octree.h"
#include "povray.h"
#include "pov_util.h"
#include "optin.h"
#include "optout.h"
#include "parse.h"
#include "radiosit.h"
#include "render.h"
#include "tokenize.h"
#include "vlbuffer.h"
#include "ppm.h"
#include "targa.h"
#include "userio.h"
#include "userdisp.h"
#include "png_pov.h"
#include "file_pov.h"
#include "povmsend.h"
#include "pov_err.h"


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#define kUseSpecialHandler	 kPOVMSType_WildCard
#define kNoParameter		 kPOVMSType_Null
#define kUnused				 kPOVMSType_Null


/*****************************************************************************
* Local typedefs
******************************************************************************/

struct Cmd_Parser_Table
{
	char *command;
	POVMSType key;
	POVMSType type;
	POVMSType is_switch;
};


/*****************************************************************************
* Global variables
******************************************************************************/

char *DefaultFile[] =
{
  NULL,
  NULL,
  "debug.out",
  "fatal.out",
  "render.out",
  "stats.out",
  "warning.out",
  "alltext.out"
};


/*****************************************************************************
* Local variables
******************************************************************************/

/* Quality constants */

long Quality_Values[12]=
{
  QUALITY_0, QUALITY_1, QUALITY_2, QUALITY_3, QUALITY_4,
  QUALITY_5, QUALITY_6, QUALITY_7, QUALITY_8, QUALITY_9
};

/* Supported output file types */
char *Output_File_Types = "sStTcCpPnNdDrR";

/*
   Keyword table for the INI-file parser.
   The parser converts the INI-file options into a POVMS object using
   the specifications provided in this table. The first element is the
   INI-file keyword, the second element is the POVMS object attribute
   key, the third is the attribute type.
*/

struct Ini_Parser_Table Option_Variable[] =
{
	{ "All_Console",		 kPOVAttrib_AllConsole,			kPOVMSType_Bool },
	{ "All_File",			 kPOVAttrib_AllFile,			kPOVObjectClass_File },
	{ "Antialias_Depth",	 kPOVAttrib_AntialiasDepth,		kPOVMSType_Int },
	{ "Antialias",			 kPOVAttrib_Antialias,			kPOVMSType_Bool },
	{ "Antialias_Threshold", kPOVAttrib_AntialiasThreshold,	kPOVMSType_Float },

	{ "Bits_Per_Color",		 kPOVAttrib_BitsPerColor,		kPOVMSType_Int },
	{ "Bits_Per_Colour",	 kPOVAttrib_BitsPerColor,		kPOVMSType_Int },
	{ "Bounding",			 kPOVAttrib_Bounding,			kPOVMSType_Bool },
	{ "Bounding_Threshold",	 kPOVAttrib_BoundingThreshold,	kPOVMSType_Int },
	{ "Buffer_Output",		 kPOVAttrib_BufferOutput,		kPOVMSType_Bool },
	{ "Buffer_Size",		 kPOVAttrib_BufferSize,			kPOVMSType_Int },

	{ "Clock",				 kPOVAttrib_Clock,				kPOVMSType_Float },
//	{ "Compression",		 kPOVAttrib_Compression,		kPOVMSType_Int },
	{ "Continue_Trace",		 kPOVAttrib_ContinueTrace,		kPOVMSType_Bool },
	{ "Create_Histogram",	 kPOVAttrib_CreateHistogram,	kPOVMSType_Bool },
	{ "Create_Ini",			 kPOVAttrib_CreateIni,			kPOVObjectClass_File },
	{ "Cyclic_Animation",	 kPOVAttrib_CyclicAnimation,	kPOVMSType_Bool },

	{ "Debug_Console",		 kPOVAttrib_DebugConsole,		kPOVMSType_Bool },
	{ "Debug_File",			 kPOVAttrib_DebugFile,			kPOVObjectClass_File },
	{ "Declare",			 kPOVAttrib_Declare,			kUseSpecialHandler },
	{ "Display",			 kPOVAttrib_Display,			kPOVMSType_Bool },
	{ "Display_Gamma",		 kPOVAttrib_DisplayGamma,		kPOVMSType_Float },
	{ "Draw_Vistas",		 kPOVAttrib_DrawVistas,			kPOVMSType_Bool },

	{ "End_Column",			 kPOVAttrib_Right,				kPOVMSType_Float },
	{ "End_Row",			 kPOVAttrib_Bottom,				kPOVMSType_Float },

	{ "Fatal_Console",		 kPOVAttrib_FatalConsole,		kPOVMSType_Bool },
	{ "Fatal_Error_Command", kPOVAttrib_FatalErrorCommand,	kUseSpecialHandler },
	{ "Fatal_Error_Return",	 kPOVAttrib_FatalErrorCommand,	kUseSpecialHandler },
	{ "Fatal_File",			 kPOVAttrib_FatalFile,			kPOVObjectClass_File },
	{ "Field_Render",		 kPOVAttrib_FieldRender,		kPOVMSType_Bool },
	{ "Final_Clock",		 kPOVAttrib_FinalClock,			kPOVMSType_Float },
	{ "Final_Frame",		 kPOVAttrib_FinalFrame,			kPOVMSType_Int },

	{ "Height",				 kPOVAttrib_Height,				kPOVMSType_Int },
	{ "Histogram_Name",		 kPOVAttrib_HistogramFile,		kPOVObjectClass_File },
	{ "Histogram_Grid_Size", kPOVAttrib_HistogramGridSizeX,	kUseSpecialHandler },
	{ "Histogram_Type",		 kPOVAttrib_HistogramFileType,	kUseSpecialHandler },

	{ "Initial_Clock",		 kPOVAttrib_InitialClock,		kPOVMSType_Float },
	{ "Initial_Frame",		 kPOVAttrib_InitialFrame,		kPOVMSType_Int },
	{ "Input_File_Name",	 kPOVAttrib_InputFile,			kPOVObjectClass_File },
	{ "Include_Header",		 kPOVAttrib_IncludeHeader,		kPOVObjectClass_File },
	{ "Include_Ini",		 kPOVAttrib_IncludeIni,			kUseSpecialHandler },

	{ "Jitter_Amount",		 kPOVAttrib_JitterAmount,		kPOVMSType_Float },
	{ "Jitter",				 kPOVAttrib_Jitter,				kPOVMSType_Bool },

	{ "Library_Path",		 kPOVAttrib_LibraryPath,		kUseSpecialHandler },
	{ "Light_Buffer",		 kPOVAttrib_LightBuffer,		kPOVMSType_Bool },

	{ "Odd_Field",			 kPOVAttrib_OddField,			kPOVMSType_Bool },
	{ "Output_Alpha",		 kPOVAttrib_OutputAlpha,		kPOVMSType_Bool },
	{ "Output_File_Name",	 kPOVAttrib_OutputFile,			kPOVObjectClass_File },
	{ "Output_File_Type",	 kPOVAttrib_OutputFileType,		kUseSpecialHandler },
	{ "Output_To_File",		 kPOVAttrib_OutputToFile,		kPOVMSType_Bool },

	{ "Palette",			 kPOVAttrib_Palette,			kUseSpecialHandler },
	{ "Pause_When_Done",	 kPOVAttrib_PauseWhenDone,		kPOVMSType_Bool },
	{ "Post_Frame_Command",	 kPOVAttrib_PostFrameCommand,	kUseSpecialHandler },
	{ "Post_Frame_Return",	 kPOVAttrib_PostFrameCommand,	kUseSpecialHandler },
	{ "Post_Scene_Command",	 kPOVAttrib_PostSceneCommand,	kUseSpecialHandler },
	{ "Post_Scene_Return",	 kPOVAttrib_PostSceneCommand,	kUseSpecialHandler },
	{ "Preview_End_Size",	 kPOVAttrib_PreviewEndSize,		kPOVMSType_Int },
	{ "Preview_Start_Size",	 kPOVAttrib_PreviewStartSize,	kPOVMSType_Int },
	{ "Pre_Frame_Command",	 kPOVAttrib_PreFrameCommand,	kUseSpecialHandler },
	{ "Pre_Frame_Return",	 kPOVAttrib_PreFrameCommand,	kUseSpecialHandler },
	{ "Pre_Scene_command",	 kPOVAttrib_PreSceneCommand,	kUseSpecialHandler },
	{ "Pre_Scene_Return",	 kPOVAttrib_PreSceneCommand,	kUseSpecialHandler },

	{ "Quality",			 kPOVAttrib_Quality,			kPOVMSType_Int },

	{ "Radiosity",			 kPOVAttrib_Radiosity,			kPOVMSType_Bool },
	{ "Remove_Bounds",		 kPOVAttrib_RemoveBounds,		kPOVMSType_Bool },
	{ "Render_Console",		 kPOVAttrib_RenderConsole,		kPOVMSType_Bool },
	{ "Render_File",		 kPOVAttrib_RenderFile,			kPOVObjectClass_File },

	{ "Sampling_Method",	 kPOVAttrib_SamplingMethod,		kPOVMSType_Int },
	{ "Split_Unions",		 kPOVAttrib_SplitUnions,		kPOVMSType_Bool },
	{ "Start_Column",		 kPOVAttrib_Left,				kPOVMSType_Float },
	{ "Start_Row",			 kPOVAttrib_Top,				kPOVMSType_Float },
	{ "Statistic_Console",	 kPOVAttrib_StatisticsConsole,	kPOVMSType_Bool },
	{ "Statistic_File",		 kPOVAttrib_StatisticsFile,		kPOVObjectClass_File },
	{ "Subset_End_Frame",	 kPOVAttrib_SubsetEndFrame,		kPOVMSType_Int },
	{ "Subset_Start_Frame",	 kPOVAttrib_SubsetStartFrame,	kPOVMSType_Int },

	{ "Test_Abort_Count",	 kPOVAttrib_TestAbortCount,		kPOVMSType_Int },
	{ "Test_Abort",			 kPOVAttrib_TestAbort,			kPOVMSType_Bool },

	{ "User_Abort_Command",	 kPOVAttrib_UserAbortCommand,	kUseSpecialHandler },
	{ "User_Abort_Return",	 kPOVAttrib_UserAbortCommand,	kUseSpecialHandler },

	{ "Verbose",			 kPOVAttrib_Verbose,			kPOVMSType_Bool },
	{ "Version",			 kPOVAttrib_Version,			kPOVMSType_Float },
	{ "Video_Mode",			 kPOVAttrib_VideoMode,			kUseSpecialHandler },
	{ "Vista_Buffer",		 kPOVAttrib_VistaBuffer,		kPOVMSType_Bool },

	{ "Warning_Console",	 kPOVAttrib_WarningConsole,		kPOVMSType_Bool },
	{ "Warning_File",		 kPOVAttrib_WarningFile,		kPOVObjectClass_File },
	{ "Warning_Level",		 kPOVAttrib_WarningLevel,		kPOVMSType_Int },
	{ "Width",				 kPOVAttrib_Width,				kPOVMSType_Int },

	{ NULL, 0, 0 }
};

/*
   Keyword table for the command line parser.
   The parser converts the command line options into a POVMS object using
   the specifications provided in this table. The first element is the
   command keyword, the second element is the POVMS object attribute key
   of the parameter, the third is the attribute type and the last specifies
   is the +/- switch is used as boolean parameter if an attribute key is
   provided.
*/

struct Cmd_Parser_Table Command_Option[] =
{
	{ "A0",	 kPOVAttrib_AntialiasThreshold,	kPOVMSType_Float,		kPOVAttrib_Antialias },
	{ "AM",	 kPOVAttrib_SamplingMethod,		kPOVMSType_Int,			kNoParameter },
	{ "A",	 kNoParameter,					kNoParameter,			kPOVAttrib_Antialias },

	{ "B",	 kPOVAttrib_BufferSize,			kPOVMSType_Int,			kPOVAttrib_BufferOutput },
	{ "B",	 kNoParameter,					kNoParameter,			kPOVAttrib_BufferOutput },

	{ "C",	 kNoParameter,					kNoParameter,			kPOVAttrib_ContinueTrace },

	{ "D",	 kPOVAttrib_Display,			kUseSpecialHandler,		kPOVAttrib_Display },
	{ "D",	 kNoParameter,					kNoParameter,			kPOVAttrib_Display },

	{ "EC",	 kPOVAttrib_Right,				kPOVMSType_Float,		kNoParameter },
	{ "EF0", kPOVAttrib_SubsetEndFrame,		kPOVMSType_Float,		kNoParameter },
	{ "EF",	 kPOVAttrib_SubsetEndFrame,		kPOVMSType_Int,			kNoParameter },
	{ "EP",	 kPOVAttrib_PreviewEndSize,		kPOVMSType_Int,			kNoParameter },
	{ "ER",	 kPOVAttrib_Bottom,				kPOVMSType_Float,		kNoParameter },

	{ "F",	 kPOVAttrib_OutputFileType,		kUseSpecialHandler,		kPOVAttrib_OutputToFile },
	{ "F",	 kNoParameter,					kNoParameter,			kPOVAttrib_OutputToFile },

	{ "GA",	 kPOVAttrib_AllFile,			kPOVObjectClass_File,	kPOVAttrib_AllConsole },
	{ "GA",	 kNoParameter,					kNoParameter,			kPOVAttrib_AllConsole },
	{ "GD",	 kPOVAttrib_DebugFile,			kPOVObjectClass_File,	kPOVAttrib_DebugConsole },
	{ "GD",	 kNoParameter,					kNoParameter,			kPOVAttrib_DebugConsole },
	{ "GF",	 kPOVAttrib_FatalFile,			kPOVObjectClass_File,	kPOVAttrib_FatalConsole },
	{ "GF",	 kNoParameter,					kNoParameter,			kPOVAttrib_FatalConsole },
	{ "GI",	 kPOVAttrib_CreateIni,			kPOVObjectClass_File,	kNoParameter },
	{ "GR",	 kPOVAttrib_RenderFile,			kPOVObjectClass_File,	kPOVAttrib_RenderConsole },
	{ "GR",	 kNoParameter,					kNoParameter,			kPOVAttrib_RenderConsole },
	{ "GS",	 kPOVAttrib_StatisticsFile,		kPOVObjectClass_File,	kPOVAttrib_StatisticsConsole },
	{ "GS",	 kNoParameter,					kNoParameter,			kPOVAttrib_StatisticsConsole },
	{ "GW",	 kPOVAttrib_WarningFile,		kPOVObjectClass_File,	kPOVAttrib_WarningConsole },
	{ "GW",	 kNoParameter,					kNoParameter,			kPOVAttrib_WarningConsole },

	{ "HI",	 kPOVAttrib_IncludeHeader,		kPOVObjectClass_File,	kNoParameter },
	{ "HN",	 kPOVAttrib_HistogramFile,		kPOVObjectClass_File,	kNoParameter },
	{ "HS",	 kPOVAttrib_HistogramGridSizeX,	kUseSpecialHandler,		kNoParameter },
	{ "HT",	 kPOVAttrib_HistogramFileType,	kUseSpecialHandler,		kPOVAttrib_CreateHistogram },
	{ "H",	 kPOVAttrib_Height,				kPOVMSType_Int,			kNoParameter },

	{ "I",	 kPOVAttrib_InputFile,			kPOVObjectClass_File,	kNoParameter },

	{ "J",	 kPOVAttrib_JitterAmount,		kPOVMSType_Float,		kPOVAttrib_Jitter },
	{ "J",	 kNoParameter,					kNoParameter,			kPOVAttrib_Jitter },

	{ "KC",	 kNoParameter,					kNoParameter,			kPOVAttrib_CyclicAnimation },
	{ "KI",	 kPOVAttrib_InitialClock,		kPOVMSType_Float,		kNoParameter },
	{ "KFF", kPOVAttrib_FinalFrame,			kPOVMSType_Int,			kNoParameter },
	{ "KFI", kPOVAttrib_InitialFrame,		kPOVMSType_Int,			kNoParameter },
	{ "KF",	 kPOVAttrib_FinalClock,			kPOVMSType_Float,		kNoParameter },
	{ "K",	 kPOVAttrib_Clock,				kPOVMSType_Float,		kNoParameter },

	{ "L",	 kPOVAttrib_LibraryPath,		kUseSpecialHandler,		kNoParameter },

	{ "MB",	 kPOVAttrib_BoundingThreshold,	kPOVMSType_Int,			kPOVAttrib_Bounding },
	{ "MB",	 kNoParameter,					kNoParameter,			kPOVAttrib_Bounding },
	{ "MV",	 kPOVAttrib_Version,			kPOVMSType_Float,		kNoParameter },

	{ "O",	 kPOVAttrib_OutputFile,			kPOVObjectClass_File,	kNoParameter },

	{ "P",	 kNoParameter,					kNoParameter,			kPOVAttrib_PauseWhenDone },

	{ "QR",	 kNoParameter,					kNoParameter,			kPOVAttrib_Radiosity },
	{ "Q",	 kPOVAttrib_Quality,			kPOVMSType_Int,			kNoParameter },

	{ "R",	 kPOVAttrib_AntialiasDepth,		kPOVMSType_Int,			kNoParameter },

	{ "SC",	 kPOVAttrib_Left,				kPOVMSType_Float,		kNoParameter },
	{ "SF0", kPOVAttrib_SubsetStartFrame,	kPOVMSType_Float,		kNoParameter },
	{ "SF",	 kPOVAttrib_SubsetStartFrame,	kPOVMSType_Int,			kNoParameter },
	{ "SP",	 kPOVAttrib_PreviewStartSize,	kPOVMSType_Int,			kNoParameter },
	{ "SR",	 kPOVAttrib_Top,				kPOVMSType_Float,		kNoParameter },
	{ "SU",	 kNoParameter,					kNoParameter,			kPOVAttrib_SplitUnions },

	{ "UA",	 kNoParameter,					kNoParameter,			kPOVAttrib_OutputAlpha },
	{ "UD",	 kNoParameter,					kNoParameter,			kPOVAttrib_DrawVistas },
	{ "UF",	 kNoParameter,					kNoParameter,			kPOVAttrib_FieldRender },
	{ "UL",	 kNoParameter,					kNoParameter,			kPOVAttrib_LightBuffer },
	{ "UO",	 kNoParameter,					kNoParameter,			kPOVAttrib_OddField },
	{ "UR",	 kNoParameter,					kNoParameter,			kPOVAttrib_RemoveBounds },
	{ "UV",	 kNoParameter,					kNoParameter,			kPOVAttrib_VistaBuffer },

	{ "V",	 kNoParameter,					kNoParameter,			kPOVAttrib_Verbose },

	{ "WL",	 kPOVAttrib_WarningLevel,		kPOVMSType_Int,			kNoParameter },
	{ "W",	 kPOVAttrib_Width,				kPOVMSType_Int,			kNoParameter },

	{ "X",	 kPOVAttrib_TestAbortCount,		kUseSpecialHandler,		kPOVAttrib_TestAbort },
	{ "X",	 kNoParameter,					kNoParameter,			kPOVAttrib_TestAbort },

	{ NULL, 0, 0, 0 }
};


static char temp_string[3]="\0\0";
static char ret_string[7]="IQUFSA";

/*****************************************************************************
* static functions
******************************************************************************/

int output_ini_option(struct Ini_Parser_Table *option, POVMSObjectPtr obj, POV_OSTREAM *file);
int parse_ini_option_line(char *Option_Line, POVMSObjectPtr obj);
int parse_switch(char *Option_Line, POVMSObjectPtr obj);
int extract_filename(char **param);
int process_ini_option(struct Ini_Parser_Table *option, char *param, POVMSObjectPtr obj);
int process_switch(struct Cmd_Parser_Table *option, char *param, POVMSObjectPtr obj, bool is_on);

static bool matches(char *v1, char *v2);

POV_ISTREAM *Opts_Locate_File(char *filename, unsigned long stype, char *buffer, int err_flag, POVMSObjectPtr obj);


/*****************************************************************************
*
* FUNCTION
*
*   Write_INI_File
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   SCD, 2/95
*   
* DESCRIPTION
*
*   Writes all options to an INI file with the current input filename's name
*   (by default), or using a specified filename.
*
* CHANGES
*
*   -
*
******************************************************************************/

void Write_INI_File()
{
	char ini_name[FILE_NAME_LENGTH];
	POVMSObject obj;
	int err = 0;

	if (opts.Ini_Output_File_Name[0]=='\0')
		return;

	if (isfalse(opts.Ini_Output_File_Name))
		return;

	Status_Info("\nWriting INI file...");

	if (istrue(opts.Ini_Output_File_Name))
	{
		strcpy(ini_name,opts.Scene_Name);
		strcat(ini_name,".ini");
	}
	else
		strcpy(ini_name,opts.Ini_Output_File_Name);

	err = POVMSObject_New(&obj, kPOVObjectClass_ROptions);
	if(err != 0)
		return;
	if(err == 0)
		err = BuildRenderOptions(&obj);
	if(err == 0)
		err = povray_object2inifile(ini_name, &obj);
	if(err == kCannotOpenFileErr)
		Warning(0,"Error opening .INI output file '%s' - no file written.", ini_name);
	else if(err != kNoErr)
		Warning(0,"Error generating .INI output file '%s'.", ini_name);
	POVMSObject_Delete(&obj);		
}

int output_ini_option(struct Ini_Parser_Table *option, POVMSObjectPtr obj, POV_OSTREAM *file)
{
	POVMSAttributeList list;
	POVMSFloat floatval;
	POVMSBool b;
	POVMSInt intval,intval2;
	int err = 0;
	long l;
	int i,imax;
	POVMSAttribute item;
	char *bufptr;
	char chr;

	switch(option->type)
	{
		case kPOVMSType_Int:
			if(POVMSUtil_GetInt(obj, option->key, &intval) == 0)
				file->printf("%s=%d\n", option->keyword, (int)intval);
			break;
		case kPOVMSType_Float:
			if(POVMSUtil_GetFloat(obj, option->key, &floatval) == 0)
				file->printf("%s=%g\n", option->keyword, (float)floatval);
			break;
		case kPOVMSType_Bool:
			if(POVMSUtil_GetBool(obj, option->key, &b) == 0)
			{
				if(b == true)
					file->printf("%s=On\n", option->keyword);
				else
					file->printf("%s=Off\n", option->keyword);
			}
			break;
		case kPOVObjectClass_File:
			err = POVMSObject_Get(obj, &item, option->key);
			if(err != 0)
				break;
			// get the file name and path string
			l = 0;
			err = POVMSAttr_Size(&item, &l);
			if(l > 0)
			{
				bufptr = (char *)POV_MALLOC(l, "file name buffer");
				bufptr[0] = 0;
				if(POVMSAttr_Get(&item, kPOVMSType_CString, bufptr, &l) == 0)
					file->printf("%s=\"%s\"\n", option->keyword, bufptr);
				POV_FREE(bufptr);
			} 
			(void)POVMSAttr_Delete(&item);
			break;
		case kUseSpecialHandler:
			switch(option->key)
			{
				case kPOVAttrib_HistogramGridSizeX:
					if(POVMSUtil_GetInt(obj, kPOVAttrib_HistogramGridSizeX, &intval) == 0)
					{
						if(POVMSUtil_GetInt(obj, kPOVAttrib_HistogramGridSizeY, &intval2) == 0)
							file->printf("%s=%d.%d\n", option->keyword, intval, intval2);
					}
					break;
				case kPOVAttrib_Palette:
				case kPOVAttrib_VideoMode:
				case kPOVAttrib_OutputFileType:
				case kPOVAttrib_HistogramFileType:
					if(POVMSUtil_GetInt(obj, option->key, &intval) == 0)
					{
						chr = intval;
						if(chr > 32)
							file->printf("%s=%c\n", option->keyword, chr);
					}
					break;
				case kPOVAttrib_IncludeIni:
					break;
				case kPOVAttrib_Declare:
					POVMSObject decobj;

					err = POVMSObject_Get(obj, &list, option->key);
					if(err != 0)
						break;

					l = 0;
					err = POVMSAttrList_Count(&list, &l);
					if(err != 0)
						break;
					if(l == 0)
						break;

					imax = l;
					for(i = 1; i <= imax; i++)
					{
						err = POVMSAttrList_GetNth(&list, i, &decobj);
						if(err == 0)
							err = POVMSObject_Get(&decobj, &item, kPOVAttrib_Identifier);
						if(err == 0)
						{
							l = 0;
							err = POVMSAttr_Size(&item, &l);
							if(l > 0)
							{
								bufptr = (char *)POV_MALLOC(l, "declare");
								bufptr[0] = 0;
								if((POVMSUtil_GetFloat(&decobj, kPOVAttrib_Value, &floatval) == 0) &&
								   (POVMSAttr_Get(&item, kPOVMSType_CString, bufptr, &l) == 0))
									file->printf("%s=%s=%g\n", option->keyword, bufptr, (float)floatval);
								POV_FREE(bufptr);
							}
							(void)POVMSAttr_Delete(&item);
						}
					}
					break;
				case kPOVAttrib_LibraryPath:
					err = POVMSObject_Get(obj, &list, option->key);
					if(err != 0)
						break;

					l = 0;
					err = POVMSAttrList_Count(&list, &l);
					if(err != 0)
						break;
					if(l == 0)
						break;

					imax = l;
					for(i = 1; i <= imax; i++)
					{
						err = POVMSAttrList_GetNth(&list, i, &item);
						if(err == 0)
						{
							l = 0;
							err = POVMSAttr_Size(&item, &l);
							if(l > 0)
							{
								bufptr = (char *)POV_MALLOC(l, "library path buffer");
								bufptr[0] = 0;
								if(POVMSAttr_Get(&item, kPOVMSType_CString, bufptr, &l) == 0)
									file->printf("%s=\"%s\"\n", option->keyword, bufptr);
								POV_FREE(bufptr);
							}
							(void)POVMSAttr_Delete(&item);
						}
					}
					break;
				case kPOVAttrib_FatalErrorCommand:
				case kPOVAttrib_PostFrameCommand:
				case kPOVAttrib_PostSceneCommand:
				case kPOVAttrib_PreFrameCommand:
				case kPOVAttrib_PreSceneCommand:
				case kPOVAttrib_UserAbortCommand:
					POVMSObject cmdobj;

					err = POVMSObject_Get(obj, &cmdobj, option->key);
					if(err != 0)
						break;

					err = POVMSObject_Get(&cmdobj, &item, kPOVAttrib_CommandString);
					if(err == 0)
					{
						if(toupper(*(option->keyword + strlen(option->keyword) - 1)) == 'D')
						{
							l = 0;
							err = POVMSAttr_Size(&item, &l);
							if(l > 0)
							{
								bufptr = (char *)POV_MALLOC(l, "command buffer");
								bufptr[0] = 0;
								if(POVMSAttr_Get(&item, kPOVMSType_CString, bufptr, &l) == 0)
									file->printf("%s=%s\n", option->keyword, bufptr);
								POV_FREE(bufptr);
							}
						}
						else
						{
							if(POVMSUtil_GetInt(&cmdobj, kPOVAttrib_ReturnAction, &intval) == 0)
							{
								chr = intval;
								file->printf("%s=%c\n", option->keyword, chr);
							}
						}
					}
					if(err == 0)
						err = POVMSObject_Delete(&cmdobj);
					break;
			}
			break;
		default:
			Error("Ignoring unknown INI option in Write_INI.");
			break;
	}

	return err;
}


/*****************************************************************************
*
* FUNCTION
*
*   povray_object2inifile
*
* INPUT
*
*   op - the .ini option's index
*   libind - if op = LIBRARY_PATH_OP, the library's index
*   
* OUTPUT
*   
* RETURNS
*
*   char * pointing to a static string representation of the
*   option's value.
*   
* AUTHOR
*   
* DESCRIPTION
*
*   Returns a static string representation of an option's value.
*
* CHANGES
*
*   -
*
******************************************************************************/

int povray_object2inifile(char *File_Name, POVMSObjectPtr obj)
{
	struct Ini_Parser_Table *table = Option_Variable;
	POV_OSTREAM *ini_file;
	int err = 0;

	ini_file = POV_NEW_OSTREAM(File_Name, POV_File_Text_INI, false);
	if(ini_file == NULL)
		return kCannotOpenFileErr;

	// find the keyword
	while(table->keyword != NULL)
	{
		/* So that we don't get both Bits_Per_Color and Bits_Per_Colour in
		 * the INI file. */
		if(strcmp(table->keyword, "Bits_Per_Colour") != 0)
			output_ini_option(table, obj, ini_file);

		table++;
	}

	POV_DELETE(ini_file, POV_OSTREAM);

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*
*   povray_inifile2object
*
* INPUT
*
*   op - the .ini option's index
*   libind - if op = LIBRARY_PATH_OP, the library's index
*   
* OUTPUT
*   
* RETURNS
*
*   char * pointing to a static string representation of the
*   option's value.
*   
* AUTHOR
*   
* DESCRIPTION
*
*   Returns a static string representation of an option's value.
*
* CHANGES
*
*   -
*
******************************************************************************/

int povray_inifile2object(char *File_Name, POVMSObjectPtr obj)
{
	char Option_Line[512];
	char INI_Name[FILE_NAME_LENGTH];
	char Desired_Section[FILE_NAME_LENGTH];
	char Current_Section[FILE_NAME_LENGTH];
	char *source, *dest;
	POV_ISTREAM *ini_file;
	bool Matched, Never_Matched;
	int err;
 
	/* File_Name can be of the for "POV_FILE.INI[Section]" where everything
	 * before the '[' is the actual name and "[Section]" is the title of
	 * a section within that file that starts with the [Section] heading.  
	 * Only the specified section of the INI file is processed.  If no
	 * section is specified then only parts of the file without a section
	 * header are processed.
	 */
   
	/* Copy the file name part */
	source = File_Name;
	dest = INI_Name;
	while((*source != '\0') && (*source != '['))
		*(dest++) = *(source++);
	*dest = '\0';

	/* Copy the section name part */
	dest = Desired_Section;
	while((*source != '\0') && (*source != ']'))
		*(dest++) = *(source++);
	*dest = *source;
	*(++dest) = '\0';

	if((ini_file = Opts_Locate_File(INI_Name, POV_File_Text_INI, NULL, false, obj)) == NULL)
	{
		PossibleError("Could not open INI file '%s'.", INI_Name);
		return kCannotOpenFileErr;
	}

	*Current_Section = '\0';

	Matched = (*Desired_Section == '\0');
	Never_Matched = true;

	while (ini_file->getline (Option_Line, 512))
	{
		if (*Option_Line == '[')
		{
			source = Option_Line;
			dest = Current_Section;
			while((*source != '\0') && (*source != ']'))
				*(dest++) = *(source++);
			*dest = *source;
			*(++dest) = '\0';

			Matched = (pov_stricmp(Current_Section, Desired_Section) == 0);
		}
		else
		{
			if(Matched == true)
			{
				err = povray_addoptionline2object(Option_Line, obj);
				if(err != kNoErr)
				{
					PossibleError("Could not read INI file '%s'.", INI_Name);
					POV_DELETE(ini_file, POV_ISTREAM);
					return err;
				}
				Never_Matched = false;
			}
		}
	}

	POV_DELETE(ini_file, POV_ISTREAM);

	if(Never_Matched == true)
	{
		Warning(0, "Never found section '%s' in file '%s'.", Desired_Section, INI_Name);
		return kFalseErr;
	}

	return kNoErr;
}

/*****************************************************************************
*
* FUNCTION
*
*   parse_option_line
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
*   Given a string containing a line of text, split it into individual
*   switches or options and then pass them off to be parsed by parse_switch
*   or parse_ini_option_line.
*
* CHANGES
*
*   -
*
******************************************************************************/

int povray_addoptionline2object(char *Option_Line, POVMSObjectPtr obj)
{
	POVMSAttributeList list;
	POVMSAttribute attr;
	struct Ini_Parser_Table *table = Option_Variable;
	char *source, *dest, *ptr;
	char strbuf[512];
	char *Option_String = strbuf;
	int err = 0;

	source = Option_Line;

	while(true)
	{
		// skip leading white space
		while(((int)*source > 0) && ((int)*source < 33))
			source++;

		// Quit when finished or ignore if commented
		if((*source == '\0') || (*source == ';'))
			return kNoErr;
		
		if((*source == '=') || (*source == '#'))
		{
			PossibleError("'=' or '#' must be preceded by a keyword.");
			return kParseErr;
		}

		// Copy everything that is not a space, an equals or a comment into Option_String 
		dest = Option_String;
		while((isprint((int)*source)) && (*source != ' ') && (*source != '=') && (*source != '#') && (*source != ';'))
			*(dest++) = *(source++);
		*dest = '\0';

		// If its a +/- style switch then just do it
		if((*Option_String == '+') || (*Option_String == '-'))
		{
			err = parse_switch(Option_String, obj);
			if(err != kNoErr)
				return err;
			continue;
		}

		// If we make it this far, then it must be an .INI-style setting
		// or a file name if -3 is returned. Any file names that appear
		// at this point are .INI/.DEF files to be parsed.

		// remove spaces
		while(isspace(*source))
			source++;

		// check for equal sign
		if(*source == '=')
		{
			// the parameter starts here
			source++;

			// remove spaces
			while(isspace(*source))
				source++;

			// remove trailing control characters (newline) and spaces
			// if there is something left.
			if (strlen(source))
			{
				while (*(source+strlen(source)-1) <= ' ')
				{
					*(source+strlen(source)-1) = '\0';
				}
			}

			// if there is no parameter, return now
			// to make sure a valid default is used
			if((*source) == '\0')
				return kNoErr;

			// find the keyword
			while(table->keyword != NULL)
			{
				if(pov_stricmp(table->keyword, Option_String) == 0)
					return process_ini_option(table, source, obj);

				table++;
			}

			PossibleError("Unknown option '%s' in INI file.", Option_String);

			return kParseErr;
		}

		// get the file name and path string
		err = extract_filename(&Option_String);

		if(err == kNoErr)
		{
			// see if it is a .pov file
			ptr = strrchr(Option_String, '.');
			if(ptr != NULL)
			{
				if(pov_stricmp(ptr, ".pov") == 0)
				{
					err = POVMSUtil_SetString(obj, kPOVAttrib_InputFile, Option_String);
					return err;
				}
			}

			// create list if it isn't there
			if(POVMSObject_Exist(obj, kPOVAttrib_IncludeIni) == kFalseErr)
				err = POVMSAttrList_New(&list);
			else if(POVMSObject_Exist(obj, kPOVAttrib_IncludeIni) != kNoErr)
				err = kObjectAccessErr;
			else
				err = POVMSObject_Get(obj, &list, kPOVAttrib_IncludeIni);
		}

		// parse INI file (recursive)
		if(err == kNoErr)
			err = povray_inifile2object(Option_String, obj);

		// add path or file to list
		if(err == kNoErr)
			err = POVMSAttr_New(&attr);
		if(err == kNoErr)
		{
			err = POVMSAttr_Set(&attr, kPOVMSType_CString, Option_String, strlen(Option_String) + 1);
			if(err == kNoErr)
				err = POVMSAttrList_Append(&list, &attr);
			else
				err = POVMSAttr_Delete(&attr);
		}
		if(err == kNoErr)
			err = POVMSObject_Set(obj, &list, kPOVAttrib_IncludeIni);
	}

	return err;
}

int parse_ini_option_line(char *Option_Line, POVMSObjectPtr obj)
{
	struct Ini_Parser_Table *table = Option_Variable;
	char *paramstr;

	// get the option keyword
	paramstr = Option_Line;
	while(isalnum(*paramstr))
		paramstr++;
	*paramstr = '\0';
	paramstr++;

	// remove spaces
	while(isspace(*paramstr))
		paramstr++;

	// check for equal sign
	if(*paramstr != '=')
	{
		PossibleError("'=' expected, '%s' found instead.", paramstr);
		return kParseErr;
	}

	// the parameter starts here
	paramstr++;

	// remove spaces
	while(isspace(*paramstr))
		paramstr++;

	// find the keyword
	while(table->keyword != NULL)
	{
		if(pov_stricmp(table->keyword, Option_Line) == 0)
			return process_ini_option(table, paramstr, obj);

		table++;
	}

	return -3;
}

/*****************************************************************************
*
* FUNCTION
*
*   parse_switch
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
*   Parses a traditional POV-Ray command-line switch that starts
*   with + or -.  Always calls process_variable to perform the function
*   rather than doing so itself.  Although this requires another pass
*   through a switch{case, case...}, it insures that command-line switches
*   and variable=value options get treated identically.
*
* CHANGES
*
*   -
*
******************************************************************************/

int parse_switch(char *Option_Line, POVMSObjectPtr obj)
{
	struct Cmd_Parser_Table *table = Command_Option;
	char *srcptr,*dstptr;
	bool is_on;

	if(*Option_Line == '+')
		is_on = true;
	else
		is_on = false;

	Option_Line++;

	// find the command switch
	while(table->command != NULL)
	{
		srcptr = Option_Line;
		dstptr = table->command;
		while((toupper(*srcptr) == toupper(*dstptr)) && (*srcptr != 0) && (*dstptr != 0))
		{
			srcptr++;
			dstptr++;
		}
		if(((*dstptr) == 0) &&
		   (((*srcptr > ' ') && (table->type != kNoParameter)) ||
		    ((*srcptr <= ' ') && (table->type == kNoParameter))))
			return process_switch(table, srcptr, obj, is_on);

		table++;
	}

	if(SYS_PROCESS_SWITCH(Option_Line) == false)
		PossibleError("Unknown switch '%s'.", Option_Line);
	else
		return kNoErr;

	return kParseErr;
}

int process_ini_option(struct Ini_Parser_Table *option, char *param, POVMSObjectPtr obj)
{
	POVMSAttributeList list;
	DBL floatval = 0.0;
	int intval = 0;
	int intval2 = 0;
	int err = kNoErr;

	switch(option->type)
	{
		case kPOVMSType_Int:
			if(sscanf(param, "%d", &intval) == 1)
				err = POVMSUtil_SetInt(obj, option->key, intval);
			else
			{
				PossibleError("Integer parameter expected for option '%s', found '%s'.", option->keyword, param);
				err = kParseErr;
			}
			break;
		case kPOVMSType_Float:
			if(sscanf(param, DBL_FORMAT_STRING, &floatval) == 1)
				err = POVMSUtil_SetFloat(obj, option->key, floatval);
			else
			{
				PossibleError("Floating-point parameter expected for option '%s', found '%s'.", option->keyword, param);
				err = kParseErr;
			}
			break;
		case kPOVMSType_Bool:
			err = POVMSUtil_SetBool(obj, option->key, istrue(param));
			break;
		case kPOVObjectClass_File:
			// get the file name and path string
			err = extract_filename(&param);

			// make the file object
			if(err == kNoErr)
				err = POVMSUtil_SetString(obj, option->key, param);
			else
			{
				PossibleError("File name or path parameter expected for option '%s', found '%s'.", option->keyword, param);
				err = kParseErr;
			}
			break;
		case kUseSpecialHandler:
			switch(option->key)
			{
				case kPOVAttrib_HistogramGridSizeX:
					if(sscanf(param, "%d.%d", &intval, &intval2) == 2)
					{
						err = POVMSUtil_SetInt(obj, kPOVAttrib_HistogramGridSizeX, intval);
						if(err == kNoErr)
							err = POVMSUtil_SetInt(obj, kPOVAttrib_HistogramGridSizeY, intval2);
					}
					else
					{
						PossibleError("Invalid histogram grid size '%s'.", param);
						err = kParseErr;
					}
					break;
				case kPOVAttrib_Palette:
				case kPOVAttrib_VideoMode:
					while(isspace(*param))
						param++;
					err = POVMSUtil_SetInt(obj, option->key, tolower(*param));
					break;
				case kPOVAttrib_HistogramFileType:
				case kPOVAttrib_OutputFileType:
					while(isspace(*param))
						param++;
					if(strchr(Output_File_Types, *param) == NULL)
						Error("Unrecognized output file format %c.", *param);
					err = POVMSUtil_SetInt(obj, option->key, tolower(*param));
					break;
				case kPOVAttrib_IncludeIni:
				case kPOVAttrib_LibraryPath:
					POVMSAttribute attr;

					// get the file name and path string
					err = extract_filename(&param);

					if(err == kNoErr)
					{
						// parse INI file (recursive)
						if(option->key == kPOVAttrib_IncludeIni)
							err = povray_inifile2object(param, obj);

						// create list if it isn't there
						if(err == kNoErr)
						{
							if(POVMSObject_Exist(obj, option->key) == kFalseErr)
								err = POVMSAttrList_New(&list);
							else if(POVMSObject_Exist(obj, option->key) != kNoErr)
								err = kObjectAccessErr;
							else
								err = POVMSObject_Get(obj, &list, option->key);
						}
					}
					else
					{
						PossibleError("File name or path parameter expected for option '%s', found '%s'.", option->keyword, param);
						err = -4;
					}

					// add path or file to list
					if(err == kNoErr)
						err = POVMSAttr_New(&attr);
					if(err == kNoErr)
					{
						err = POVMSAttr_Set(&attr, kPOVMSType_CString, param, strlen(param) + 1);
						if(err == kNoErr)
							err = POVMSAttrList_Append(&list, &attr);
						else
							err = POVMSAttr_Delete(&attr);
					}
					if(err == kNoErr)
						err = POVMSObject_Set(obj, &list, option->key);
					break;
				case kPOVAttrib_Declare:
					POVMSObject decobj;

					// create list if it isn't there
					if(POVMSObject_Exist(obj, option->key) == kFalseErr)
						err = POVMSAttrList_New(&list);
					else if(POVMSObject_Exist(obj, option->key) != kNoErr)
						err = kObjectAccessErr;
					else
						err = POVMSObject_Get(obj, &list, option->key);

					// add value to list
					if(err == kNoErr)
						err = POVMSObject_New(&decobj, kPOVMSType_WildCard);
					if(err == kNoErr)
					{
						char *ptr = NULL;

						err = POVMSUtil_SetString(&decobj, kPOVAttrib_Identifier, strtok(param, "="));
						if(err == kNoErr)
						{
							ptr = strtok(NULL, "");
							if(ptr == NULL)
								err = kParseErr;
						}
						if(err == kNoErr)
						{
							if(strchr(ptr, '"') != NULL)
							{
								ptr = strchr(ptr, '"') + 1;
								strtok(ptr, "\"");
								err = POVMSUtil_SetString(&decobj, kPOVAttrib_Value, ptr);
							}
							else
								err = POVMSUtil_SetFloat(&decobj, kPOVAttrib_Value, atof(ptr));
						}
						if(err == kNoErr)
							err = POVMSAttrList_Append(&list, &decobj);
						else
							err = POVMSObject_Delete(&decobj);
					}
					if(err == kNoErr)
						err = POVMSObject_Set(obj, &list, option->key);
					break;
				case kPOVAttrib_FatalErrorCommand:
				case kPOVAttrib_PostFrameCommand:
				case kPOVAttrib_PostSceneCommand:
				case kPOVAttrib_PreFrameCommand:
				case kPOVAttrib_PreSceneCommand:
				case kPOVAttrib_UserAbortCommand:
					POVMSObject cmdobj;

					err = POVMSObject_New(&cmdobj, kPOVMSType_WildCard);
					if(toupper(*(option->keyword + strlen(option->keyword) - 1)) == 'D')
					{
						if(err == kNoErr)
							err = POVMSUtil_SetString(&cmdobj, kPOVAttrib_CommandString, param);
					}
					else
					{
						if(err == kNoErr)
							err = POVMSUtil_SetInt(&cmdobj, kPOVAttrib_ReturnAction, tolower(*param));
					}
					if(err == kNoErr)
						err = POVMSObject_Set(obj, &cmdobj, option->key);
					break;
			}
			break;
		default:
			err = kParseErr;
			break;
	}

	return err;
}

int process_switch(struct Cmd_Parser_Table *option, char *param, POVMSObjectPtr obj, bool is_on)
{
	DBL floatval = 0.0;
	int intval = 0;
	int intval2 = 0;
	int err = 0;
	char chr = 0;

	if(option->is_switch != kNoParameter)
	{
		err = POVMSUtil_SetBool(obj, option->is_switch, is_on);
		if(err != kNoErr)
			return err;
	}

	switch(option->type)
	{
		case kPOVMSType_Int:
			if(sscanf(param, "%d", &intval) == 1)
				err = POVMSUtil_SetInt(obj, option->key, intval);
			else
			{
				PossibleError("Integer parameter expected for switch '%s', found '%s'.", option->command, param);
				err = kParseErr;
			}
			break;
		case kPOVMSType_Float:
			if(sscanf(param, DBL_FORMAT_STRING, &floatval) == 1)
				err = POVMSUtil_SetFloat(obj, option->key, floatval);
			else
			{
				PossibleError("Floating-point parameter expected for switch '%s', found '%s'.", option->command, param);
				err = kParseErr;
			}
			break;
		case kPOVMSType_Bool:
			err = POVMSUtil_SetBool(obj, option->key, istrue(param));
			break;
		case kPOVObjectClass_File:
			// get the file name and path string
			err = extract_filename(&param);

			// make the file object
			if(err == kNoErr)
				err = POVMSUtil_SetString(obj, option->key, param);
			else
			{
				PossibleError("File name or path parameter expected for switch '%s', found '%s'.", option->command, param);
				err = kParseErr;
			}
			break;
		case kUseSpecialHandler:
			switch(option->key)
			{
				case kPOVAttrib_Display:
					if(param[0] != '\0')
					{
						err = POVMSUtil_SetInt(obj, kPOVAttrib_VideoMode, (int)toupper(param[0]));
						if((param[1] != '\0') && (err == 0))
							err = POVMSUtil_SetInt(obj, kPOVAttrib_Palette, (int)toupper(param[1]));
					}
					break;
				case kPOVAttrib_HistogramGridSizeX:
					if(sscanf(param, "%d.%d", &intval, &intval2) == 2)
					{
						err = POVMSUtil_SetInt(obj, kPOVAttrib_HistogramGridSizeX, intval);
						if(err == kNoErr)
							err = POVMSUtil_SetInt(obj, kPOVAttrib_HistogramGridSizeY, intval2);
					}
					else
					{
						PossibleError("Invalid histogram grid size '%s'.", param);
						err = kParseErr;
					}
					break;
				case kPOVAttrib_OutputFileType:
					if(strchr(Output_File_Types, *param) == NULL)
						Error("Unrecognized output file format %c.", *param);
					err = POVMSUtil_SetInt(obj, option->key, tolower(*param));
					param++;
					if((err == kNoErr) && (*param > ' '))
					{
						if(isdigit(*param) != 0)
						{
							if(sscanf(param, "%d", &intval) == 1)
								err = POVMSUtil_SetInt(obj, kPOVAttrib_BitsPerColor, intval);
							else
								Error("Invalid bits per color '%s'.", param);
						}
						else
							Error("Missing bits per color, '%s' found instead.", param);
					}
					break;
				case kPOVAttrib_HistogramFileType:
					if(strchr(Output_File_Types, *param) == NULL)
						Error("Unrecognized output file format %c.", *param);
					chr = tolower(*param);
					err = POVMSUtil_SetInt(obj, option->key, chr);
					break;
				case kPOVAttrib_LibraryPath:
					POVMSAttributeList list;
					POVMSAttribute attr;

					// get the file name and path string
					err = extract_filename(&param);

					if(err == kNoErr)
					{
						// create list if it isn't there
						if(POVMSObject_Exist(obj, option->key) == kFalseErr)
							err = POVMSAttrList_New(&list);
						else if(POVMSObject_Exist(obj, option->key) != kNoErr)
							err = kObjectAccessErr;
						else
							err = POVMSObject_Get(obj, &list, option->key);
					}
					else
					{
						PossibleError("File name or path parameter expected for switch '%s', found '%s'.", option->command, param);
						err = kParseErr;
					}

					// add path or file to list
					if(err == kNoErr)
						err = POVMSAttr_New(&attr);
					if(err == kNoErr)
					{
						err = POVMSAttr_Set(&attr, kPOVMSType_CString, param, strlen(param) + 1);
						if(err == kNoErr)
							err = POVMSAttrList_Append(&list, &attr);
						else
							err = POVMSAttr_Delete(&attr);
					}
					if(err == kNoErr)
						err = POVMSObject_Set(obj, &list, option->key);
					break;
				case kPOVAttrib_TestAbortCount:
					if((*param) == 0)
						break;
					if(sscanf(param, "%d", &intval) == 1)
						err = POVMSUtil_SetInt(obj, option->key, intval);
					else
					{
						PossibleError("No or integer parameter expected for switch '%s', found '%s'.", option->command, param);
						err = kParseErr;
					}
					break;
			}
			break;
		case kNoParameter:
			break;
		default:
			err = kParseErr;
			break;
	}

	return err;
}

int extract_filename(char **param)
{
	unsigned char stop = '\0';
	unsigned char *end;

	// allow quotation marks around file path and name
	if((**param == '"') || (**param == '\''))
	{
		stop = (unsigned char)(**param);
		(*param)++;
	}

	// find the end of the file path and name
	end = (unsigned char *)(*param);
	while((*end >= ' ') &&
	      (((*end != ';')  && (stop == '\0')) ||
	       ((*end != stop) && (stop != '\0'))))
	{
		end++;
	}

	// check for invalid end of line
	if((stop != '\0') && (*end < ' '))
		return kParseErr;

	// mark the end
	*end = '\0';

	return kNoErr;
}

static bool matches(char *v1, char *v2)
{
   int i = 0;
   int ans = 1;

   while ((ans) && (v1[i] != '\0') && (v2[i] != '\0'))
   {
      ans = ans && (int)(v1[i] == tolower(v2[i]));
      i++;
   }

   return (ans != 0);
}

bool istrue(char *value)
{
   return (matches("on",value)  || matches("true",value) || 
           matches("yes",value) || matches("1",value));
}

bool isfalse(char *value)
{
   return (matches("off",value)  || matches("false",value) || 
           matches("no",value)   || matches("0",value));
}

/*
int povray_read_all_options(POVMSObjectPtr obj)
{
	POVMSAttributeList list;
	POVMSAttribute item;
	char *bufptr;
	long l;
	int err = 0;

	if(POVMSObject_Exist(obj, kPOVAttrib_IncludeIni) == (int)false)
		return 0;

	err = POVMSObject_Get(obj, &list, kPOVAttrib_IncludeIni);
	if(err == 0)
	{
		while(true)
		{
			l = 0;
			err = POVMSAttrList_Count(&list, &l);
			if(err != 0)
				break;
			if(l == 0)
				break;

			err = POVMSAttrList_GetNth(&list, 1, &item);
			if(err == 0)
			{
				l = 0;
				err = POVMSAttr_Size(&item, &l);
				if(l > 0)
				{
					bufptr = (char *)POV_MALLOC(l, "INI path buffer");
					bufptr[0] = 0;
					if(POVMSAttr_Get(&item, kPOVMSType_CString, bufptr, &l) == 0)
						povray_inifile2object(bufptr, obj);
					POV_FREE((void *)bufptr);
				}
				(void)POVMSAttr_Delete(&item);
				if(err == 0)
					err = POVMSAttrList_RemoveNth(&list, 1);
			}
			if(err != 0)
				break;
		}
	}

	return err;
}
*/
