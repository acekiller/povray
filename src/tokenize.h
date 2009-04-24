/****************************************************************************
*                   tokenize.h
*
*  This module contains all defines, typedefs, and prototypes for TOKENIZE.CPP.
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
* $File: //depot/povray/3.5/source/tokenize.h $
* $Revision: #15 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "frame.h"
#include "povms.h"


/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define MAX_NUMBER_OF_TABLES 100



/*****************************************************************************
* Global typedefs
******************************************************************************/


struct Token_Struct
{
  TOKEN Token_Id;
  TOKEN Function_Id;
  int Token_Line_No,Table_Index;
  char *Token_String;
  DBL Token_Float;
  int Unget_Token, End_Of_File;
  char *Filename;
  void *Data;
  int *NumberPtr;
  void **DataPtr;
  bool is_array_elem;
};

#define MAX_PARAMETER_LIST 56

typedef struct Pov_Macro_Struct POV_MACRO;

struct Pov_Macro_Struct
{
  char *Macro_Name;
  char *Macro_Filename;
  long Macro_Pos,Macro_Line_No,Macro_End;
  int Num_Of_Pars;
  char *Par_Name[MAX_PARAMETER_LIST];
};

typedef struct Pov_Array_Struct POV_ARRAY;

struct Pov_Array_Struct
{
   int Dims, Type, Total;
   int Sizes[5];
   int Mags[5];
   void **DataPtrs;
};

typedef struct Pov_Param_Struct POV_PARAM;

struct Pov_Param_Struct
{
   int *NumberPtr;
   void **DataPtr;
   int Table_Index;
};



/*****************************************************************************
* Global variables
******************************************************************************/

extern struct Token_Struct Token;

extern struct Reserved_Word_Struct Reserved_Words [LAST_TOKEN];
extern int Table_Index;
extern int token_count;



/*****************************************************************************
* Global functions
******************************************************************************/

void Get_Token (void);
void Unget_Token (void);
void Parse_String_Literal(void);
void Where_Error (POVMSObjectPtr msg);
void Where_Warning (POVMSObjectPtr msg);
void Parse_Directive (int After_Hash);
int Get_Include_File_Depth (void);
DATA_FILE * Get_Include_File_Array (void);
void Open_Include (void);
void IncludeHeader(char *temp);
void pre_init_tokenizer (void);
void Initialize_Tokenizer (void);
void Terminate_Tokenizer (void);
SYM_ENTRY *Add_Symbol (int Index,char *Name,TOKEN Number);
void Destroy_Macro (POV_MACRO *PMac);
POV_ARRAY *Parse_Array_Declare (void);
SYM_ENTRY *Create_Entry (int Index,char *Name,TOKEN Number);
SYM_ENTRY *Destroy_Entry (int Index,SYM_ENTRY *Entry);
int Parse_Ifdef_Param (void);

#endif
