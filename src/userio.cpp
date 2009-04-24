/****************************************************************************
*                userio.cpp
*
*  This module contains I/O routines.
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
* $File: //depot/povray/3.5/source/userio.cpp $
* $Revision: #29 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include <stdarg.h>
#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "parse.h"
#include "povray.h"
#include "tokenize.h"
#include "userio.h"
#include "userdisp.h"
#include "povms.h"
#include "pov_err.h"
#include "file_pov.h"

STAGENAME Stage_Names[STAGECOUNT] =
{
   { STAGE_PREINIT, "Fatal Init Error" },
   { STAGE_STARTUP, "Startup Error" },
   { STAGE_BANNER, "Banner Error" },
   { STAGE_INIT, "Init Error" },
   { STAGE_ENVIRONMENT, "Environment Error" },
   { STAGE_COMMAND_LINE, "Command Line Error" },
   { STAGE_FILE_INIT, "File Init Error" },
   { STAGE_PARSING, "Parse Error" },
   { STAGE_CONTINUING, "Continue Trace Error" },
   { STAGE_RENDERING, "Rendering Error" },
   { STAGE_SHUTDOWN, "Shutdown Error" },
   { STAGE_INI_FILE, "INI File Error" },
   { STAGE_CLEANUP_PARSE, "Cleanup Parse Error" },
   { STAGE_SLAB_BUILDING, "Slab Building Error" },
   { STAGE_TOKEN_INIT, "Scene File Parser Initialization Error" },
   { STAGE_INCLUDE_ERR, "Parse Error" },
   { STAGE_FOUND_INSTEAD, "Parse Error" }
};

STREAM_INFO Stream_Info[MAX_STREAMS] = 
{
   { NULL, NULL, POV_BANNER },
   { NULL, NULL, POV_STATUS_INFO },
   { NULL, NULL, POV_DEBUG_INFO },
   { NULL, NULL, POV_FATAL },
   { NULL, NULL, POV_RENDER_INFO },
   { NULL, NULL, POV_STATISTICS },
   { NULL, NULL, POV_WARNING },
   { NULL, NULL, POV_CONSOLE }
};

static char vsbuffer[1024];

void CDECL cleanup_string(char *str);

/****************************************************************************/
/* Prints to the stream's console or file destination, as specified by type */
void CDECL stream_puts(int stream, char *s)
{
   if (Stream_Info[stream].console != NULL)
   {
      Stream_Info[stream].console(s);
      Stream_Info[stream].console("\n\f");
   }

   if (Stream_Info[stream].handle != NULL)
   {
      *(Stream_Info[stream].handle) << s << '\n' ;
      Stream_Info[stream].handle->flush();
   }

   if (Stream_Info[ALL_STREAM].handle != NULL)
   {
      *(Stream_Info[ALL_STREAM].handle) << s << '\n' ;
      Stream_Info[ALL_STREAM].handle->flush();
   }
}

void CDECL stream_printf(int stream, char *format,...)
{
   va_list marker;

   va_start(marker, format);
   vsprintf(vsbuffer, format, marker);
   va_end(marker);

   if (Stream_Info[stream].console != NULL)
   {
      Stream_Info[stream].console(vsbuffer);
   }

   if (Stream_Info[stream].handle != NULL)
   {
      *(Stream_Info[stream].handle) << vsbuffer;
      Stream_Info[stream].handle->flush();
   }

   if (Stream_Info[ALL_STREAM].handle != NULL)
   {
      *(Stream_Info[ALL_STREAM].handle) << vsbuffer;
      Stream_Info[ALL_STREAM].handle->flush();
   }
}

/****************************************************************************/
/* Use this routine to replace newlines with spaces in a string. */
void CDECL cleanup_string(char *str)
{
	while(*str != 0)
	{
		if(*str == '\n')
			*str = ' ';
		str++;
	}
}

/****************************************************************************/
/* Use this routine to display debug information. */
int CDECL Debug_Info(char *format,...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	va_start(marker, format);
	vsprintf(localvsbuffer, format, marker);
	va_end(marker);

	stream_printf(DEBUG_STREAM, "%s", localvsbuffer);

	#ifndef DONT_USE_POVMS
	(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_Debug);
	(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
	#endif

	Do_Cooperate(0);

	return 0;
}

/*
 * Use this routine to display non-fatal warning message if
 * opts.Language_Version is greater than level parameter.
 */
int CDECL Warning(unsigned int level, char *format,...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	va_start(marker, format);
	vsprintf(localvsbuffer, format, marker);
	va_end(marker);

	if((opts.Warning_Level < 5) || ((opts.Warning_Level < 10) && (level == 0)))
		return 0;

	if(level >= opts.Language_Version)
		return 0;

	if((Stage == STAGE_PARSING) || (Stage == STAGE_INCLUDE_ERR) || (Stage == STAGE_FOUND_INSTEAD))
	{
		#ifndef DONT_USE_POVMS
		(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
		#endif

		Where_Warning(&msg);

		stream_printf(WARNING_STREAM, "Warning: ");
		stream_puts(WARNING_STREAM, localvsbuffer);

		#ifndef DONT_USE_POVMS
		cleanup_string(localvsbuffer);
		(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Warning, 0);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_Warning);
		(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
		#endif
	}
	else
	{
		stream_printf(WARNING_STREAM, "Warning: ");
		stream_puts(WARNING_STREAM, localvsbuffer);

		#ifndef DONT_USE_POVMS
		cleanup_string(localvsbuffer);
		(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
		(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Warning, 0);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_Warning);
		(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
		#endif
	}

	Do_Cooperate(0);

	return 0;
}

/****************************************************************************/
/* Prints a non-fatal error message and adds file and line number */
int CDECL WarningAt(unsigned int level, char *filename, long line, char *format, ...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	va_start(marker, format);
	vsprintf(localvsbuffer, format, marker);
	va_end(marker);

	if((opts.Warning_Level < 5) || ((opts.Warning_Level < 10) && (level == 0)))
		return 0;

	if(level >= opts.Language_Version)
		return 0;

#ifndef DONT_USE_POVMS
	(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_FileName, filename);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Line, line);
#endif

	stream_printf(WARNING_STREAM, "File: %s  Line: %d\n", filename, line);
	stream_printf(WARNING_STREAM, "Warning: ");
	stream_puts(WARNING_STREAM, localvsbuffer);

#ifndef DONT_USE_POVMS
	cleanup_string(localvsbuffer);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Warning, 0);
	(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_FatalError);
	(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
#endif

	Do_Cooperate(0);

	return 0;
}

/****************************************************************************/
/* Prints a non-fatal error message and adds file and line number if parsing. */
int CDECL PossibleError(char *format,...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	va_start(marker, format);
	vsprintf(localvsbuffer, format, marker);
	va_end(marker);

	if(opts.Warning_Level == 0)
		return 0;

	if((Stage == STAGE_PARSING) || (Stage == STAGE_INCLUDE_ERR) || (Stage == STAGE_FOUND_INSTEAD))
	{
		#ifndef DONT_USE_POVMS
		(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
		#endif

		Where_Warning(&msg);

		stream_printf(WARNING_STREAM, "Error: ");
		stream_puts(WARNING_STREAM, localvsbuffer);

		#ifndef DONT_USE_POVMS
		cleanup_string(localvsbuffer);
		(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_Error);
		(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
		#endif
	}
	else
	{
		stream_printf(WARNING_STREAM, "Possible %s: ", Stage_Names[Stage].stage_name);
		stream_puts(WARNING_STREAM, localvsbuffer);

		#ifndef DONT_USE_POVMS
		cleanup_string(localvsbuffer);
		(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
		(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_Error);
		(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
		#endif
	}

	Do_Cooperate(0);

	return 0;
}

/****************************************************************************/
/* Prints a fatal error message and adds file and line number if parsing, then terminates. */
int CDECL Error(char *format,...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	va_start(marker, format);
	vsprintf(localvsbuffer, format, marker);
	va_end(marker);

	if((Stage == STAGE_PARSING) || (Stage == STAGE_INCLUDE_ERR) || (Stage == STAGE_FOUND_INSTEAD))
	{
		#ifndef DONT_USE_POVMS
		(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
		#endif

		Where_Error(&msg);

		stream_printf(FATAL_STREAM, "%s: ", Stage_Names[Stage].stage_name);
		stream_puts(FATAL_STREAM, localvsbuffer);

		#ifndef DONT_USE_POVMS
		cleanup_string(localvsbuffer);
		(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_FatalError);
		(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
		#endif
	}
	else
	{
		stream_printf(FATAL_STREAM, "%s: ", Stage_Names[Stage].stage_name);
		stream_puts(FATAL_STREAM, localvsbuffer);

		#ifndef DONT_USE_POVMS
		cleanup_string(localvsbuffer);
		(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
		(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_FatalError);
		(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
		#endif
	}

	/* This could be just an "if" but we may add special messages later */
	if(Stage == STAGE_INCLUDE_ERR)
	{
		Warning(0, "Check that the file is in a directory specifed with a +L switch\n"
		           "or 'Library_Path=' .INI item. Standard include files are in the\n"
		           "include directory or folder. Please read your documentation carefully.");
	}

	Terminate_Tokenizer(); /* Closes scene file */

	POV_SHELLOUT(FATAL_SHL);

	povray_exit(1);

	return 0;
}

/****************************************************************************/
/* Prints a fatal error message and adds file and line number, then terminates. */
int CDECL ErrorAt(char *filename, long line, char *format, ...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	va_start(marker, format);
	vsprintf(localvsbuffer, format, marker);
	va_end(marker);

#ifndef DONT_USE_POVMS
	(void)POVMSObject_New(&msg, kPOVObjectClass_FileLoc);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_FileName, filename);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Line, line);
#endif

	stream_printf(FATAL_STREAM, "File: %s  Line: %d\n", filename, line);
	stream_printf(FATAL_STREAM, "Fatal Error: ");
	stream_puts(FATAL_STREAM, localvsbuffer);

#ifndef DONT_USE_POVMS
	cleanup_string(localvsbuffer);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);
	(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_RenderOutput, kPOVMsgIdent_FatalError);
	(void)POVMS_Send(&msg, NULL, kPOVMSSendMode_NoReply);
#endif

	Terminate_Tokenizer(); /* Closes scene file */

	POV_SHELLOUT(FATAL_SHL);

	povray_exit(1);

	return 0;
}

/****************************************************************************/
/* Initialization for streams structure */
void Init_Text_Streams()
{
  int i;
  
  for (i = 0; i < MAX_STREAMS; i++)
  {
    Stream_Info[i].handle = NULL;
    Stream_Info[i].name = NULL;
  }
  Stream_Info[BANNER_STREAM].console = POV_BANNER;
  Stream_Info[STATUS_STREAM].console = POV_STATUS_INFO;
  Stream_Info[DEBUG_STREAM].console = POV_DEBUG_INFO;
  Stream_Info[FATAL_STREAM].console = POV_FATAL;
  Stream_Info[RENDER_STREAM].console = POV_RENDER_INFO;
  Stream_Info[STATISTIC_STREAM].console = POV_STATISTICS;
  Stream_Info[WARNING_STREAM].console = POV_WARNING;
  Stream_Info[ALL_STREAM].console = NULL;
}

/****************************************************************************/
/* Opens stream text output files if necessary. */
void Open_Text_Streams()
{
   int i;

   for(i = 0; i < MAX_STREAMS; i++)
   {
      if(Stream_Info[i].name != NULL)
      {
         if(opts.Options & CONTINUE_TRACE)
         {
            Stream_Info[i].handle = POV_NEW_OSTREAM(Stream_Info[i].name, POV_File_Text_Stream, true);
            if(Stream_Info[i].handle == NULL)
               Warning(0, "Could not append stream to file %s.", Stream_Info[i].name);
         }
         else
         {
            Stream_Info[i].handle = POV_NEW_OSTREAM(Stream_Info[i].name, POV_File_Text_Stream, false);
            if(Stream_Info[i].handle == NULL)
               Warning(0, "Could not write stream to file %s.", Stream_Info[i].name);
         }
      }
   }
}

void Destroy_Text_Streams()
{
   int i;

   for(i = 0; i < MAX_STREAMS; i++)
   {
      if(Stream_Info[i].handle != NULL)
         POV_DELETE(Stream_Info[i].handle, POV_OSTREAM);

      if(Stream_Info[i].name != NULL)
      {
         POV_FREE(Stream_Info[i].name);

         Stream_Info[i].name = NULL;
      }
   }
}

/****************************************************************************/
void POV_Std_Console(char *s)
{
   fprintf(stderr, "%s", s);
   fflush(stderr);
}

void POV_Std_DummyConsole(char *)
{
  // does nothing [trf]
}
