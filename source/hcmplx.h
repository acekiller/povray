/****************************************************************************
*                   hcmplx.h
*
*  This module contains all defines, typedefs, and prototypes for HCMPLX.C.
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

#ifndef HCMPLX_H
#define HCMPLX_H

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

int F_Bound_HCompl PARAMS((RAY *, FRACTAL *, DBL *, DBL *));
void Normal_Calc_HCompl PARAMS((VECTOR, int, FRACTAL *));
int Iteration_HCompl PARAMS((VECTOR, FRACTAL *));
int D_Iteration_HCompl PARAMS((VECTOR, FRACTAL *, DBL *));

int F_Bound_HCompl_z3 PARAMS((RAY *, FRACTAL *, DBL *, DBL *));
void Normal_Calc_HCompl_z3 PARAMS((VECTOR, int, FRACTAL *));
int Iteration_HCompl_z3 PARAMS((VECTOR, FRACTAL *));
int D_Iteration_HCompl_z3 PARAMS((VECTOR, FRACTAL *, DBL *));

int F_Bound_HCompl_Reciprocal PARAMS((RAY *, FRACTAL *, DBL *, DBL *));
void Normal_Calc_HCompl_Reciprocal PARAMS((VECTOR, int, FRACTAL *));
int Iteration_HCompl_Reciprocal PARAMS((VECTOR, FRACTAL *));
int D_Iteration_HCompl_Reciprocal PARAMS((VECTOR, FRACTAL *, DBL *));

int F_Bound_HCompl_Func PARAMS((RAY *, FRACTAL *, DBL *, DBL *));
void Normal_Calc_HCompl_Func PARAMS((VECTOR, int, FRACTAL *));
int Iteration_HCompl_Func PARAMS((VECTOR, FRACTAL *));
int D_Iteration_HCompl_Func PARAMS((VECTOR, FRACTAL *, DBL *));

void Complex_Exp PARAMS((CMPLX *target, CMPLX *source));
void Complex_Log PARAMS((CMPLX *target, CMPLX *source));
void Complex_Sin PARAMS((CMPLX *target, CMPLX *source));
void Complex_ASin PARAMS((CMPLX *target, CMPLX *source));
void Complex_Sinh PARAMS((CMPLX *target, CMPLX *source));
void Complex_ASinh PARAMS((CMPLX *target, CMPLX *source));
void Complex_Cos PARAMS((CMPLX *target, CMPLX *source));
void Complex_ACos PARAMS((CMPLX *target, CMPLX *source));
void Complex_Cosh PARAMS((CMPLX *target, CMPLX *source));
void Complex_ACosh PARAMS((CMPLX *target, CMPLX *source));
void Complex_Tan PARAMS((CMPLX *target, CMPLX *source));
void Complex_ATan PARAMS((CMPLX *target, CMPLX *source));
void Complex_Tanh PARAMS((CMPLX *target, CMPLX *source));
void Complex_ATanh PARAMS((CMPLX *target, CMPLX *source));
void Complex_Sqrt PARAMS((CMPLX *target, CMPLX *source));
void Complex_Pwr PARAMS((CMPLX *target, CMPLX *source));

void Complex_Mult PARAMS((CMPLX *target, CMPLX *source1, CMPLX *source2));
void Complex_Div PARAMS((CMPLX *target, CMPLX *source1, CMPLX *source2));
void Complex_Power PARAMS((CMPLX *target, CMPLX *source1, CMPLX *source2));

#endif
