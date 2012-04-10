/*******************************************************************************
 * matrices.h
 *
 * This module contains all defines, typedefs, and prototypes for MATRICES.CPP.
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 1991-2003 Persistence of Vision Team
 * Copyright 2003-2009 Persistence of Vision Raytracer Pty. Ltd.
 * ---------------------------------------------------------------------------
 * NOTICE: This source code file is provided so that users may experiment
 * with enhancements to POV-Ray and to port the software to platforms other
 * than those supported by the POV-Ray developers. There are strict rules
 * regarding how you are permitted to use this file. These rules are contained
 * in the distribution and derivative versions licenses which should have been
 * provided with this file.
 *
 * These licences may be found online, linked from the end-user license
 * agreement that is located at http://www.povray.org/povlegal.html
 * ---------------------------------------------------------------------------
 * POV-Ray is based on the popular DKB raytracer version 2.12.
 * DKBTrace was originally written by David K. Buck.
 * DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
 * ---------------------------------------------------------------------------
 * $File: //depot/povray/smp/source/backend/math/matrices.h $
 * $Revision: #10 $
 * $Change: 5009 $
 * $DateTime: 2010/06/05 10:39:30 $
 * $Author: chrisc $
 *******************************************************************************/

/*********************************************************************************
 * NOTICE
 *
 * This file is part of a BETA-TEST version of POV-Ray version 3.7. It is not
 * final code. Use of this source file is governed by both the standard POV-Ray
 * licences referred to in the copyright header block above this notice, and the
 * following additional restrictions numbered 1 through 4 below:
 *
 *   1. This source file may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd.
 *
 *   2. This notice may not be altered or removed.
 *   
 *   3. Binaries generated from this source file by individuals for their own
 *      personal use may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd. Such personal-use binaries
 *      are not required to have a timeout, and thus permission is granted in
 *      these circumstances only to disable the timeout code contained within
 *      the beta software.
 *   
 *   4. Binaries generated from this source file for use within an organizational
 *      unit (such as, but not limited to, a company or university) may not be
 *      distributed beyond the local organizational unit in which they were made,
 *      unless written permission is obtained from Persistence of Vision Raytracer
 *      Pty. Ltd. Additionally, the timeout code implemented within the beta may
 *      not be disabled or otherwise bypassed in any manner.
 *
 * The following text is not part of the above conditions and is provided for
 * informational purposes only.
 *
 * The purpose of the no-redistribution clause is to attempt to keep the
 * circulating copies of the beta source fresh. The only authorized distribution
 * point for the source code is the POV-Ray website and Perforce server, where
 * the code will be kept up to date with recent fixes. Additionally the beta
 * timeout code mentioned above has been a standard part of POV-Ray betas since
 * version 1.0, and is intended to reduce bug reports from old betas as well as
 * keep any circulating beta binaries relatively fresh.
 *
 * All said, however, the POV-Ray developers are open to any reasonable request
 * for variations to the above conditions and will consider them on a case-by-case
 * basis.
 *
 * Additionally, the developers request your co-operation in fixing bugs and
 * generally improving the program. If submitting a bug-fix, please ensure that
 * you quote the revision number of the file shown above in the copyright header
 * (see the '$Revision:' field). This ensures that it is possible to determine
 * what specific copy of the file you are working with. The developers also would
 * like to make it known that until POV-Ray 3.7 is out of beta, they would prefer
 * to emphasize the provision of bug fixes over the addition of new features.
 *
 * Persons wishing to enhance this source are requested to take the above into
 * account. It is also strongly suggested that such enhancements are started with
 * a recent copy of the source.
 *
 * The source code page (see http://www.povray.org/beta/source/) sets out the
 * conditions under which the developers are willing to accept contributions back
 * into the primary source tree. Please refer to those conditions prior to making
 * any changes to this source, if you wish to submit those changes for inclusion
 * with POV-Ray.
 *
 *********************************************************************************/


#ifndef MATRICES_H
#define MATRICES_H

namespace pov
{

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

void MZero (MATRIX result);
void MIdentity (MATRIX result);
void MTimesA (MATRIX result, const MATRIX matrix2);
void MTimesB (const MATRIX matrix1, MATRIX result);
void MTimesC (MATRIX result, const MATRIX matrix1, const MATRIX matrix2);
void MAdd (MATRIX result, const MATRIX matrix1, const MATRIX matrix2);
void MSub (MATRIX result, const MATRIX matrix1, const MATRIX matrix2);
void MScale (MATRIX result, const MATRIX matrix1, DBL amount);
void MTranspose (MATRIX result);
void MTranspose (MATRIX result, const MATRIX matrix1);
void MTransPoint (VECTOR result, const VECTOR vector, const TRANSFORM *trans);
void MInvTransPoint (VECTOR result, const VECTOR vector, const TRANSFORM *trans);
void MTransDirection (VECTOR result, const VECTOR vector, const TRANSFORM *trans);
void MInvTransDirection (VECTOR result, const VECTOR vector, const TRANSFORM *trans);
void MTransNormal (VECTOR result, const VECTOR vector, const TRANSFORM *trans);
void MInvTransNormal (VECTOR result, const VECTOR vector, const TRANSFORM *trans);
void Compute_Matrix_Transform (TRANSFORM *result, const MATRIX matrix);
void Compute_Scaling_Transform (TRANSFORM *result, const VECTOR vector);
void Compute_Inversion_Transform (TRANSFORM *result);
void Compute_Translation_Transform (TRANSFORM *transform, const VECTOR vector);
void Compute_Rotation_Transform (TRANSFORM *transform, const VECTOR vector);
void Compute_Look_At_Transform (TRANSFORM *transform, const VECTOR Look_At, const VECTOR Up, const VECTOR Right);
void Compose_Transforms (TRANSFORM *Original_Transform, const TRANSFORM *New_Transform);
void Compute_Axis_Rotation_Transform (TRANSFORM *transform, const VECTOR AxisVect, DBL angle);
void Compute_Coordinate_Transform (TRANSFORM *trans, const VECTOR origin, VECTOR up, DBL r, DBL len);
TRANSFORM *Create_Transform (void);
TRANSFORM *Copy_Transform (const TRANSFORM *Old);
void Destroy_Transform (TRANSFORM *Trans);
UV_VECT *Create_UV_Vect (void);
VECTOR *Create_Vector (void);
VECTOR_4D *Create_Vector_4D (void);
DBL *Create_Float (void);
void MInvers (MATRIX r, const MATRIX m);
int MInvers3(const VECTOR inM[3], VECTOR outM[3]);
void MTransUVPoint(const DBL p[2], const DBL m[3][3], DBL t[2]);
void MSquareQuad(const UV_VECT st[4], DBL sq[3][3]);

}

#endif
