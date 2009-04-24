/****************************************************************************
*                   matrices.h
*
*  This module contains all defines, typedefs, and prototypes for MATRICES.C.
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


#ifndef MATRICES_H
#define MATRICES_H



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

void MZero PARAMS((MATRIX result));
void MIdentity PARAMS((MATRIX result));
void MTimes PARAMS((MATRIX result, MATRIX matrix1, MATRIX matrix2));
void MAdd PARAMS((MATRIX result, MATRIX matrix1, MATRIX matrix2));
void MSub PARAMS((MATRIX result, MATRIX matrix1, MATRIX matrix2));
void MScale PARAMS((MATRIX result, MATRIX matrix1, DBL amount));
void MTranspose PARAMS((MATRIX result, MATRIX matrix1));
void MTransPoint PARAMS((VECTOR result, VECTOR vector, TRANSFORM *trans));
void MInvTransPoint PARAMS((VECTOR result, VECTOR vector, TRANSFORM *trans));
void MTransDirection PARAMS((VECTOR result, VECTOR vector, TRANSFORM *trans));
void MInvTransDirection PARAMS((VECTOR result, VECTOR vector, TRANSFORM *trans));
void MTransNormal PARAMS((VECTOR result, VECTOR vector, TRANSFORM *trans));
void MInvTransNormal PARAMS((VECTOR result, VECTOR vector, TRANSFORM *trans));
void Compute_Matrix_Transform PARAMS((TRANSFORM *result, MATRIX matrix));
void Compute_Scaling_Transform PARAMS((TRANSFORM *result, VECTOR vector));
void Compute_Inversion_Transform PARAMS((TRANSFORM *result));
void Compute_Translation_Transform PARAMS((TRANSFORM *transform, VECTOR vector));
void Compute_Rotation_Transform PARAMS((TRANSFORM *transform, VECTOR vector));
void Compute_Look_At_Transform PARAMS((TRANSFORM *transform, VECTOR Look_At, VECTOR Up, VECTOR Right));
void Compose_Transforms PARAMS((TRANSFORM *Original_Transform, TRANSFORM *New_Transform));
void Compute_Axis_Rotation_Transform PARAMS((TRANSFORM *transform, VECTOR V1, DBL angle));
void Compute_Coordinate_Transform PARAMS((TRANSFORM *trans, VECTOR origin, VECTOR up, DBL r, DBL len));
TRANSFORM *Create_Transform PARAMS((void));
TRANSFORM *Copy_Transform PARAMS((TRANSFORM *Old));
VECTOR *Create_Vector PARAMS((void));
DBL *Create_Float PARAMS((void));
void MInvers PARAMS((MATRIX r, MATRIX m));



#endif
