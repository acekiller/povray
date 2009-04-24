/****************************************************************************
*                   express.h
*
*  This module contains all defines, typedefs, and prototypes for EXPRESS.C.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copright 1995 Persistence of Vision Team
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


#ifndef EXPRESS_H
#define EXPRESS_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/




/*****************************************************************************
* Global typedefs
******************************************************************************/




/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void Parse_Colour PARAMS((COLOUR Colour));
BLEND_MAP *Parse_Blend_Map PARAMS((int Blend_Type, int Pat_Type));
BLEND_MAP *Parse_Colour_Map PARAMS((void));
BLEND_MAP *Parse_Blend_List PARAMS((int Count, BLEND_MAP *Def_Map, int Blend_Type));
DBL Parse_Raw_Number PARAMS((void));
DBL Parse_Float PARAMS((void));
DBL Allow_Float PARAMS((DBL defval));
void Parse_UV_Vect PARAMS((UV_VECT UV_Vect));
void Parse_Vector PARAMS((VECTOR Vector));
void Parse_Vector4D PARAMS((VECTOR Vector));
void Parse_Vector_Float PARAMS((VECTOR Vector));
void Parse_Scale_Vector PARAMS((VECTOR Vector));
DBL Parse_Float_Param PARAMS((void));
void Parse_Float_Param2 PARAMS((DBL *Val1, DBL *Val2));
char *Parse_String PARAMS((void));
char *Parse_Formatted_String PARAMS((void));
void Init_Random_Generators PARAMS((void));
void Destroy_Random_Generators PARAMS((void));

#endif
