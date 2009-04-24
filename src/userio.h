/****************************************************************************
*                   userio.h
*
*  This module contains all defines, typedefs, and prototypes for userio.cpp
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
* $File: //depot/povray/3.5/source/userio.h $
* $Revision: #8 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef USERIO_H
#define USERIO_H


/*****************************************************************************
* Global preprocessor defines
******************************************************************************/


/*****************************************************************************
* Global typedefs
******************************************************************************/

struct StageName_Struct
{
  int stage_id;
  char *stage_name;
};

typedef struct StageName_Struct STAGENAME;

enum
{
  BANNER_STREAM = 0,
  STATUS_STREAM,
  DEBUG_STREAM,
  FATAL_STREAM,
  RENDER_STREAM,
  STATISTIC_STREAM,
  WARNING_STREAM,
  ALL_STREAM,
  MAX_STREAMS /* Must be last! */
};

typedef struct Stream_Struct
{
  POV_OSTREAM *handle;
  char *name;
  void (*console)(char *s);
} STREAM_INFO;


/*****************************************************************************
* Global variables
******************************************************************************/

extern STREAM_INFO Stream_Info[MAX_STREAMS];


/*****************************************************************************
* Global functions
******************************************************************************/

void CDECL stream_puts(int stream, char *s);
void CDECL stream_printf(int stream, char *format,...);

void Init_Text_Streams(void);
void Open_Text_Streams(void);
void Destroy_Text_Streams(void);

void POV_Std_Console(char *s);
void POV_Std_DummyConsole(char *s);


#endif /* USERIO_H */
