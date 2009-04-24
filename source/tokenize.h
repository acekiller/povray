/****************************************************************************
*                   tokenize.h
*
*  This module contains all defines, typedefs, and prototypes for TOKENIZE.C.
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


#ifndef TOKENIZE_H
#define TOKENIZE_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/




/*****************************************************************************
* Global typedefs
******************************************************************************/




/*****************************************************************************
* Global variables
******************************************************************************/

extern struct Constant_Struct *Constants;
extern int Max_Symbols, Max_Constants;
extern char String[MAX_STRING_INDEX];
extern int Number_Of_Constants;

extern struct Token_Struct Token;

extern struct Reserved_Word_Struct Reserved_Words [LAST_TOKEN];



/*****************************************************************************
* Global functions
******************************************************************************/

void Get_Token PARAMS((void));
void Unget_Token PARAMS((void));
void Where_Error PARAMS((void));
void Parse_Directive PARAMS((int After_Hash));
void Open_Include PARAMS((void));
void pre_init_tokenizer PARAMS((void));
void Initialize_Tokenizer PARAMS((void));
void Terminate_Tokenizer PARAMS((void));


#endif
