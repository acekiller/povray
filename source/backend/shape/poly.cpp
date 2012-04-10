/*******************************************************************************
 * poly.cpp
 *
 * This module implements the code for general 3 variable polynomial shapes
 *
 * This file was written by Alexander Enzmann.  He wrote the code for
 * 4th - 6th order shapes and generously provided us these enhancements.
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
 * $File: //depot/povray/smp/source/backend/shape/poly.cpp $
 * $Revision: #31 $
 * $Change: 5276 $
 * $DateTime: 2010/12/20 10:42:42 $
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

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/math/vector.h"
#include "backend/bounding/bbox.h"
#include "backend/math/polysolv.h"
#include "backend/math/matrices.h"
#include "backend/scene/objects.h"
#include "backend/shape/poly.h"
#include "backend/scene/threaddata.h"

// this must be the last file included
#include "base/povdebug.h"

/*
 * Basic form of a quartic equation:
 *
 *  a00*x^4    + a01*x^3*y   + a02*x^3*z   + a03*x^3     + a04*x^2*y^2 +
 *  a05*x^2*y*z+ a06*x^2*y   + a07*x^2*z^2 + a08*x^2*z   + a09*x^2     +
 *  a10*x*y^3  + a11*x*y^2*z + a12*x*y^2   + a13*x*y*z^2 + a14*x*y*z   +
 *  a15*x*y    + a16*x*z^3   + a17*x*z^2   + a18*x*z     + a19*x       + a20*y^4   +
 *  a21*y^3*z  + a22*y^3     + a23*y^2*z^2 + a24*y^2*z   + a25*y^2     + a26*y*z^3 +
 *  a27*y*z^2  + a28*y*z     + a29*y       + a30*z^4     + a31*z^3     + a32*z^2   + a33*z + a34
 *
 */

namespace pov
{

/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

const DBL DEPTH_TOLERANCE = 1.0e-4;
const DBL INSIDE_TOLERANCE = 1.0e-4;
const DBL ROOT_TOLERANCE = 1.0e-4;
const DBL COEFF_LIMIT = 1.0e-20;
// const int BINOMSIZE = 40;



/*****************************************************************************
* Local variables
******************************************************************************/

/* The following table contains the binomial coefficients up to 35 */
#if (MAX_ORDER > 35)
#error "Poly.cpp code would break loose due to a too short pascal triangle table."
#endif
/* this is a pascal's triangle : [k][j]=[k-1][j-1]+[k-1][j] ; 
	                               [k][0]=1, [k][k]=1 
 */
/* Max value in [35] is 0x8B18014C, so unsigned int is enough 
 * If you want to go down to from [36] to [69], 
 *  a 64 bits unsigned long must be used
 */
const unsigned int binomials[35][35] =
{

  {          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          2,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          3,          3,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          4,          6,          4,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          5,         10,         10,          5,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          6,         15,         20,         15,          6,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          7,         21,         35,         35,         21,          7,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          8,         28,         56,         70,         56,         28,          8,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,          9,         36,         84,        126,        126,         84,         36,          9,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         10,         45,        120,        210,        252,        210,        120,         45,         10,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         11,         55,        165,        330,        462,        462,        330,        165,         55,         11,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         12,         66,        220,        495,        792,        924,        792,        495,        220,         66,         12,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         13,         78,        286,        715,       1287,       1716,       1716,       1287,        715,        286,         78,         13,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         14,         91,        364,       1001,       2002,       3003,       3432,       3003,       2002,       1001,        364,         91,         14,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         15,        105,        455,       1365,       3003,       5005,       6435,       6435,       5005,       3003,       1365,        455,        105,         15,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         16,        120,        560,       1820,       4368,       8008,      11440,      12870,      11440,       8008,       4368,       1820,        560,        120,         16,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         17,        136,        680,       2380,       6188,      12376,      19448,      24310,      24310,      19448,      12376,       6188,       2380,        680,        136,         17,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         18,        153,        816,       3060,       8568,      18564,      31824,      43758,      48620,      43758,      31824,      18564,       8568,       3060,        816,        153,         18,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         19,        171,        969,       3876,      11628,      27132,      50388,      75582,      92378,      92378,      75582,      50388,      27132,      11628,       3876,        969,        171,         19,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         20,        190,       1140,       4845,      15504,      38760,      77520,     125970,     167960,     184756,     167960,     125970,      77520,      38760,      15504,       4845,       1140,        190,         20,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         21,        210,       1330,       5985,      20349,      54264,     116280,     203490,     293930,     352716,     352716,     293930,     203490,     116280,      54264,      20349,       5985,       1330,        210,         21,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         22,        231,       1540,       7315,      26334,      74613,     170544,     319770,     497420,     646646,     705432,     646646,     497420,     319770,     170544,      74613,      26334,       7315,       1540,        231,         22,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         23,        253,       1771,       8855,      33649,     100947,     245157,     490314,     817190,    1144066,    1352078,    1352078,    1144066,     817190,     490314,     245157,     100947,      33649,       8855,       1771,        253,         23,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         24,        276,       2024,      10626,      42504,     134596,     346104,     735471,    1307504,    1961256,    2496144,    2704156,    2496144,    1961256,    1307504,     735471,     346104,     134596,      42504,      10626,       2024,        276,         24,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         25,        300,       2300,      12650,      53130,     177100,     480700,    1081575,    2042975,    3268760,    4457400,    5200300,    5200300,    4457400,    3268760,    2042975,    1081575,     480700,     177100,      53130,      12650,       2300,        300,         25,          1,          0,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         26,        325,       2600,      14950,      65780,     230230,     657800,    1562275,    3124550,    5311735,    7726160,    9657700,   10400600,    9657700,    7726160,    5311735,    3124550,    1562275,     657800,     230230,      65780,      14950,       2600,        325,         26,          1,          0,          0,          0,          0,          0,          0,          0,          0},
  {          1,         27,        351,       2925,      17550,      80730,     296010,     888030,    2220075,    4686825,    8436285,   13037895,   17383860,   20058300,   20058300,   17383860,   13037895,    8436285,    4686825,    2220075,     888030,     296010,      80730,      17550,       2925,        351,         27,          1,          0,          0,          0,          0,          0,          0,          0},
  {          1,         28,        378,       3276,      20475,      98280,     376740,    1184040,    3108105,    6906900,   13123110,   21474180,   30421755,   37442160,   40116600,   37442160,   30421755,   21474180,   13123110,    6906900,    3108105,    1184040,     376740,      98280,      20475,       3276,        378,         28,          1,          0,          0,          0,          0,          0,          0},
  {          1,         29,        406,       3654,      23751,     118755,     475020,    1560780,    4292145,   10015005,   20030010,   34597290,   51895935,   67863915,   77558760,   77558760,   67863915,   51895935,   34597290,   20030010,   10015005,    4292145,    1560780,     475020,     118755,      23751,       3654,        406,         29,          1,          0,          0,          0,          0,          0},
  {          1,         30,        435,       4060,      27405,     142506,     593775,    2035800,    5852925,   14307150,   30045015,   54627300,   86493225,  119759850,  145422675,  155117520,  145422675,  119759850,   86493225,   54627300,   30045015,   14307150,    5852925,    2035800,     593775,     142506,      27405,       4060,        435,         30,          1,          0,          0,          0,          0},
  {          1,         31,        465,       4495,      31465,     169911,     736281,    2629575,    7888725,   20160075,   44352165,   84672315,  141120525,  206253075,  265182525,  300540195,  300540195,  265182525,  206253075,  141120525,   84672315,   44352165,   20160075,    7888725,    2629575,     736281,     169911,      31465,       4495,        465,         31,          1,          0,          0,          0},
  {          1,         32,        496,       4960,      35960,     201376,     906192,    3365856,   10518300,   28048800,   64512240,  129024480,  225792840,  347373600,  471435600,  565722720,  601080390,  565722720,  471435600,  347373600,  225792840,  129024480,   64512240,   28048800,   10518300,    3365856,     906192,     201376,      35960,       4960,        496,         32,          1,          0,          0},
  {          1,         33,        528,       5456,      40920,     237336,    1107568,    4272048,   13884156,   38567100,   92561040,  193536720,  354817320,  573166440,  818809200, 1037158320, 1166803110, 1166803110, 1037158320,  818809200,  573166440,  354817320,  193536720,   92561040,   38567100,   13884156,    4272048,    1107568,     237336,      40920,       5456,        528,         33,          1,          0},
  {          1,         34,        561,       5984,      46376,     278256,    1344904,    5379616,   18156204,   52451256,  131128140,  286097760,  548354040,  927983760, 1391975640, 1855967520, 2203961430, 2333606220, 2203961430, 1855967520, 1391975640,  927983760,  548354040,  286097760,  131128140,   52451256,   18156204,    5379616,    1344904,     278256,      46376,       5984,        561,         34,          1}

};

/*
*/
bool Poly::Set_Coeff(const unsigned int x, const unsigned int y, const unsigned int z, const DBL value)
{
	int a,b,c;
	unsigned int pos;
	a=Order-x;
	b=Order-x-y;
	c=Order-x-y-z;
	/* a bit overprotective */
	if ((x+y+z>Order)||(a<0)||(b<0)||(c<0))
	{
		return false;
	}
	/* pos = binomials[a+2][3]+binomials[b+1][2]+binomials[c][1];
	 * rewriten to stay in bound (max is "Order", not Order+2)
	 */
	pos = 
		// binomials[a+2][3]
		//       binomials[a+1][3]
		binomials[a][3]+binomials[a][2]
		//      +binomials[a+1][2]
		+binomials[a][2]+binomials[a][1]
		// +binomials[b+1][2]
		+binomials[b][1]+binomials[b][2]
		+binomials[c][1];
	/* It's magic 
	 * Nah... a is the tetraedric sum to jump to get to the power of x index (first entry)
	 * b is then the triangular sum to add to get to the power of y index (also first entry)
	 * and c is the linear sum to add to get to the power of z index (that the one we want)
   *
	 * Notice that binomials[c][1] == c, but the formula would loose its magic use of
	 * pascal triangle everywhere.
	 * triangular sum are in the third ([2] column)
	 * tetraedric sum are in the fourth ([3] column)
	 *
	 * (and yes, the 0 at the start of each column is useful)
	 */
	Coeffs[pos] = value;
	return true;
}


/*****************************************************************************
*
* FUNCTION
*
*   All_Poly_Intersections
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

bool Poly::All_Intersections(const Ray& ray, IStack& Depth_Stack, TraceThreadData *Thread)
{
	DBL Depths[MAX_ORDER];
	DBL len;
	VECTOR  IPoint;
	int cnt, i, j, Intersection_Found, same_root;
	Ray New_Ray;

	/* Transform the ray into the polynomial's space */

	MInvTransPoint(New_Ray.Origin, ray.Origin, Trans);
	MInvTransDirection(New_Ray.Direction, ray.Direction, Trans);

	VLength(len, New_Ray.Direction);
	VInverseScaleEq(New_Ray.Direction, len);

	Intersection_Found = false;

	Thread->Stats()[Ray_Poly_Tests]++;

	switch (Order)
	{
		case 1:

			cnt = intersect_linear(New_Ray, Coeffs, Depths);

			break;

		case 2:

			cnt = intersect_quadratic(New_Ray, Coeffs, Depths);

			break;

		default:

			cnt = intersect(New_Ray, Order, Coeffs, Test_Flag(this, STURM_FLAG), Depths, Thread);
	}

	if (cnt > 0)
	{
		Thread->Stats()[Ray_Poly_Tests_Succeeded]++;
	}

	for (i = 0; i < cnt; i++)
	{
		if (Depths[i] > DEPTH_TOLERANCE)
		{
			same_root = false;

			for (j = 0; j < i; j++)
			{
				if (Depths[i] == Depths[j])
				{
					same_root = true;

					break;
				}
			}

			if (!same_root)
			{
				VEvaluateRay(IPoint, New_Ray.Origin, Depths[i], New_Ray.Direction);

				/* Transform the point into world space */

				MTransPoint(IPoint, IPoint, Trans);

				if (Clip.empty() || Point_In_Clip(IPoint, Clip, Thread))
				{
					Depth_Stack->push(Intersection(Depths[i] / len,IPoint,this));

					Intersection_Found = true;
				}
			}
		}
	}

	return (Intersection_Found);
}



/*****************************************************************************
*
* FUNCTION
*
*   evaluate_linear
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

/* For speedup of low order polynomials, expand out the terms
   involved in evaluating the poly. */
/* unused
DBL evaluate_linear(VECTOR P, DBL *a)
{
	return(a[0] * P[X]) + (a[1] * P[Y]) + (a[2] * P[Z]) + a[3];
}
*/



/*****************************************************************************
*
* FUNCTION
*
*   evaluate_quadratic
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

/*
DBL evaluate_quadratic(VECTOR P, DBL *a)
{
	DBL x, y, z;

	x = P[X];
	y = P[Y];
	z = P[Z];

	return(a[0] * x * x + a[1] * x * y + a[2] * x * z +
	       a[3] * x     + a[4] * y * y + a[5] * y * z +
	       a[6] * y     + a[7] * z * z + a[8] * z     + a[9]);
}
*/



/*****************************************************************************
*
* FUNCTION
*
*   factor_out
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Remove all factors of i from n.
*
* CHANGES
*
*   -
*
******************************************************************************/
/*
int Poly::factor_out(int n, int i, int *c, int *s)
{
	while (!(n % i))
	{
		n /= i;

		s[(*c)++] = i;
	}

	return(n);
}
*/


/*****************************************************************************
*
* FUNCTION
*
*   factor1
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Find all prime factors of n. (Note that n must be less than 2^15.
*
* CHANGES
*
*   -
*
******************************************************************************/
#if 0
void Poly::factor1(int n, int *c, int *s)
{
	int i,k;

	/* First factor out any 2s. */

	n = factor_out(n, 2, c, s);

	/* Now any odd factors. */

	k = (int)sqrt((DBL)n) + 1;

	for (i = 3; (n > 1) && (i <= k); i += 2)
	{
		if (!(n % i))
		{
			n = factor_out(n, i, c, s);
			k = (int)sqrt((DBL)n)+1;
		}
	}

	if (n > 1)
	{
		s[(*c)++] = n;
	}
}
#endif


/*****************************************************************************
*
* FUNCTION
*
*   binomial
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Calculate the binomial coefficent of n, r.
*
* CHANGES
*
*   -
*
******************************************************************************/
#if 0
int Poly::binomial(int n, int  r)
{
	int h,i,j,k,l;
	unsigned int result;
	int stack1[BINOMSIZE], stack2[BINOMSIZE];

	if ((n < 0) || (r < 0) || (r > n))
	{
		result = 0L;
	}
	else
	{
		if (r == n)
		{
			result = 1L;
		}
		else
		{
			if ((r < 16) && (n < 16))
			{
				result = (int)binomials[n][r];
			}
			else
			{
				j = 0;

				for (i = r + 1; i <= n; i++)
				{
					stack1[j++] = i;
				}

				for (i = 2; i <= (n-r); i++)
				{
					h = 0;

					factor1(i, &h, stack2);

					for (k = 0; k < h; k++)
					{
						for (l = 0; l < j; l++)
						{
							if (!(stack1[l] % stack2[k]))
							{
								stack1[l] /= stack2[k];

								goto l1;
							}
						}
					}

					// Error if we get here
//					Debug_Info("Failed to factor\n");
l1:;
				}

				result = 1;

				for (i = 0; i < j; i++)
				{
					result *= stack1[i];
				}
			}
		}
	}

	return(result);
}
#endif


/*****************************************************************************
*
* FUNCTION
*
*   inside
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

DBL Poly::inside(const VECTOR IPoint, int Order, const DBL *Coeffs)
{
	DBL x[MAX_ORDER+1], y[MAX_ORDER+1], z[MAX_ORDER+1];
	DBL c, Result;
	int i, j, k, term;

	x[0] = 1.0;       y[0] = 1.0;       z[0] = 1.0;
	x[1] = IPoint[X]; y[1] = IPoint[Y]; z[1] = IPoint[Z];

	for (i = 2; i <= Order; i++)
	{
		x[i] = x[1] * x[i-1];
		y[i] = y[1] * y[i-1];
		z[i] = z[1] * z[i-1];
	}

	Result = 0.0;

	term = 0;

	for (i = Order; i >= 0; i--)
	{
		for (j=Order-i;j>=0;j--)
		{
			for (k=Order-(i+j);k>=0;k--)
			{
				if ((c = Coeffs[term]) != 0.0)
				{
					Result += c * x[i] * y[j] * z[k];
				}
				term++;
			}
		}
	}

	return(Result);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Intersection of a ray and an arbitrary polynomial function.
*
* CHANGES
*
*   -
*
******************************************************************************/

int Poly::intersect(const Ray &ray, int Order, const DBL *Coeffs, int Sturm_Flag, DBL *Depths, TraceThreadData *Thread)
{
	DBL eqn_v[3][MAX_ORDER+1], eqn_vt[3][MAX_ORDER+1];
	DBL eqn[MAX_ORDER+1];
	DBL t[3][MAX_ORDER+1];
	VECTOR  P, D;
	DBL val;
	int h, i, j, k, i1, j1, k1, term;
	int offset;

	/* First we calculate the values of the individual powers
	   of x, y, and z as they are represented by the ray */

	Assign_Vector(P,ray.Origin);
	Assign_Vector(D,ray.Direction);

	for (i = 0; i < 3; i++)
	{
		eqn_v[i][0]  = 1.0;
		eqn_vt[i][0] = 1.0;
	}

	eqn_v[0][1] = P[X];
	eqn_v[1][1] = P[Y];
	eqn_v[2][1] = P[Z];

	eqn_vt[0][1] = D[X];
	eqn_vt[1][1] = D[Y];
	eqn_vt[2][1] = D[Z];

	for (i = 2; i <= Order; i++)
	{
		for (j=0;j<3;j++)
		{
			eqn_v[j][i]  = eqn_v[j][1] * eqn_v[j][i-1];
			eqn_vt[j][i] = eqn_vt[j][1] * eqn_vt[j][i-1];
		}
	}

	for (i = 0; i <= Order; i++)
	{
		eqn[i] = 0.0;
	}

	/* Now walk through the terms of the polynomial.  As we go
	   we substitute the ray equation for each of the variables. */

	term = 0;

	for (i = Order; i >= 0; i--)
	{
		for (h = 0; h <= i; h++)
		{
			t[0][h] = binomials[i][h] * eqn_vt[0][i-h] * eqn_v[0][h];
		}

		for (j = Order-i; j >= 0; j--)
		{
			for (h = 0; h <= j; h++)
			{
				t[1][h] = binomials[j][h] * eqn_vt[1][j-h] * eqn_v[1][h];
			}

			for (k = Order-(i+j); k >= 0; k--)
			{
				if (Coeffs[term] != 0)
				{
					for (h = 0; h <= k; h++)
					{
						t[2][h] = binomials[k][h] * eqn_vt[2][k-h] * eqn_v[2][h];
					}

					/* Multiply together the three polynomials. */

					offset = Order - (i + j + k);

					for (i1 = 0; i1 <= i; i1++)
					{
						for (j1=0;j1<=j;j1++)
						{
							for (k1=0;k1<=k;k1++)
							{
								h = offset + i1 + j1 + k1;
								val = Coeffs[term];
								val *= t[0][i1];
								val *= t[1][j1];
								val *= t[2][k1];
								eqn[h] += val;
							}
						}
					}
				}

				term++;
			}
		}
	}

	for (i = 0, j = Order; i <= Order; i++)
	{
		if (eqn[i] != 0.0)
		{
			break;
		}
		else
		{
			j--;
		}
	}

	if (j > 1)
	{
		return(Solve_Polynomial(j, &eqn[i], Depths, Sturm_Flag, ROOT_TOLERANCE, Thread));
	}
	else
	{
		return(0);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_linear
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Intersection of a ray and a quadratic.
*
* CHANGES
*
*   -
*
******************************************************************************/

int Poly::intersect_linear(const Ray &ray, const DBL *Coeffs, DBL *Depths)
{
	DBL t0, t1;
	const DBL *a = Coeffs;

	t0 = a[0] * ray.Origin[X] + a[1] * ray.Origin[Y] + a[2] * ray.Origin[Z];
	t1 = a[0] * ray.Direction[X] + a[1] * ray.Direction[Y] +

	a[2] * ray.Direction[Z];

	if (fabs(t1) < EPSILON)
	{
		return(0);
	}

	Depths[0] = -(a[3] + t0) / t1;

	return(1);
}



/*****************************************************************************
*
* FUNCTION
*
*   intersect_quadratic
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Intersection of a ray and a quadratic.
*
* CHANGES
*
*   -
*
******************************************************************************/

int Poly::intersect_quadratic(const Ray &ray, const DBL *Coeffs, DBL *Depths)
{
	DBL x, y, z, x2, y2, z2;
	DBL xx, yy, zz, xx2, yy2, zz2, ac, bc, cc, d, t;
	const DBL *a;

	x  = ray.Origin[X];
	y  = ray.Origin[Y];
	z  = ray.Origin[Z];

	xx = ray.Direction[X];
	yy = ray.Direction[Y];
	zz = ray.Direction[Z];

	x2  = x * x;    y2  = y * y;    z2 = z * z;
	xx2 = xx * xx;  yy2 = yy * yy;  zz2 = zz * zz;

	a = Coeffs;

	/*
	 * Determine the coefficients of t^n, where the line is represented
	 * as (x,y,z) + (xx,yy,zz)*t.
	 */

	ac = (a[0]*xx2 + a[1]*xx*yy + a[2]*xx*zz + a[4]*yy2 + a[5]*yy*zz + a[7]*zz2);

	bc = (2*a[0]*x*xx + a[1]*(x*yy + xx*y) + a[2]*(x*zz + xx*z) +
	      a[3]*xx + 2*a[4]*y*yy + a[5]*(y*zz + yy*z) + a[6]*yy +
	      2*a[7]*z*zz + a[8]*zz);

	cc = a[0]*x2 + a[1]*x*y + a[2]*x*z + a[3]*x + a[4]*y2 +
	     a[5]*y*z + a[6]*y + a[7]*z2 + a[8]*z + a[9];

	if (fabs(ac) < COEFF_LIMIT)
	{
		if (fabs(bc) < COEFF_LIMIT)
		{
			return(0);
		}

		Depths[0] = -cc / bc;

		return(1);
	}

	/*
	 * Solve the quadratic formula & return results that are
	 * within the correct interval.
	 */

	d = bc * bc - 4.0 * ac * cc;

	if (d < 0.0)
	{
		return(0);
	}

	d = sqrt(d);

	bc = -bc;

	t = 2.0 * ac;

	Depths[0] = (bc + d) / t;
	Depths[1] = (bc - d) / t;

	return(2);
}



/*****************************************************************************
*
* FUNCTION
*
*   normal0
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Normal to a polynomial (used for polynomials with order > 4).
*
* CHANGES
*
*   -
*
******************************************************************************/

void Poly::normal0(VECTOR Result, int Order, const DBL *Coeffs, const VECTOR IPoint)
{
	int i, j, k, term;
	DBL x[MAX_ORDER+1], y[MAX_ORDER+1], z[MAX_ORDER+1];
	DBL val;
	const DBL *a;

	x[0] = 1.0;
	y[0] = 1.0;
	z[0] = 1.0;

	x[1] = IPoint[X];
	y[1] = IPoint[Y];
	z[1] = IPoint[Z];

	for (i = 2; i <= Order; i++)
	{
		x[i] = IPoint[X] * x[i-1];
		y[i] = IPoint[Y] * y[i-1];
		z[i] = IPoint[Z] * z[i-1];
	}

	a = Coeffs;

	term = 0;

	Make_Vector(Result, 0.0, 0.0, 0.0);

	for (i = Order; i >= 0; i--)
	{
		for (j = Order-i; j >= 0; j--)
		{
			for (k = Order-(i+j); k >= 0; k--)
			{
				if (i >= 1)
				{
					val = x[i-1] * y[j] * z[k];
					Result[X] += i * a[term] * val;
				}

				if (j >= 1)
				{
					val = x[i] * y[j-1] * z[k];
					Result[Y] += j * a[term] * val;
				}

				if (k >= 1)
				{
					val = x[i] * y[j] * z[k-1];
					Result[Z] += k * a[term] * val;
				}

				term++;
			}
		}
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   nromal1
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Normal to a polynomial (for polynomials of order <= 4).
*
* CHANGES
*
*   -
*
******************************************************************************/

void Poly::normal1(VECTOR Result, int Order, const DBL *Coeffs, const VECTOR IPoint)
{
	DBL x, y, z, x2, y2, z2, x3, y3, z3;
	const DBL *a;

	a = Coeffs;

	x = IPoint[X];
	y = IPoint[Y];
	z = IPoint[Z];

	switch (Order)
	{
		case 1:

			/* Linear partial derivatives */

			Make_Vector(Result, a[0], a[1], a[2]);

			break;

		case 2:

			/* Quadratic partial derivatives */

			Result[X] = 2*a[0]*x+a[1]*y+a[2]*z+a[3];
			Result[Y] = a[1]*x+2*a[4]*y+a[5]*z+a[6];
			Result[Z] = a[2]*x+a[5]*y+2*a[7]*z+a[8];

			break;

		case 3:

			x2 = x * x;  y2 = y * y;  z2 = z * z;

			/* Cubic partial derivatives */

			Result[X] = 3*a[0]*x2 + 2*x*(a[1]*y + a[2]*z + a[3]) + a[4]*y2 +
			            y*(a[5]*z + a[6]) + a[7]*z2 + a[8]*z + a[9];
			Result[Y] = a[1]*x2 + x*(2*a[4]*y + a[5]*z + a[6]) + 3*a[10]*y2 +
			            2*y*(a[11]*z + a[12]) + a[13]*z2 + a[14]*z + a[15];
			Result[Z] = a[2]*x2 + x*(a[5]*y + 2*a[7]*z + a[8]) + a[11]*y2 +
			            y*(2*a[13]*z + a[14]) + 3*a[16]*z2 + 2*a[17]*z + a[18];

			break;

		case 4:

			/* Quartic partial derivatives */

			x2 = x * x;  y2 = y * y;  z2 = z * z;
			x3 = x * x2; y3 = y * y2; z3 = z * z2;

			Result[X] = 4*a[ 0]*x3+3*x2*(a[ 1]*y+a[ 2]*z+a[ 3])+
			            2*x*(a[ 4]*y2+y*(a[ 5]*z+a[ 6])+a[ 7]*z2+a[ 8]*z+a[ 9])+
			            a[10]*y3+y2*(a[11]*z+a[12])+y*(a[13]*z2+a[14]*z+a[15])+
			            a[16]*z3+a[17]*z2+a[18]*z+a[19];
			Result[Y] = a[ 1]*x3+x2*(2*a[ 4]*y+a[ 5]*z+a[ 6])+
			            x*(3*a[10]*y2+2*y*(a[11]*z+a[12])+a[13]*z2+a[14]*z+a[15])+
			            4*a[20]*y3+3*y2*(a[21]*z+a[22])+2*y*(a[23]*z2+a[24]*z+a[25])+
			            a[26]*z3+a[27]*z2+a[28]*z+a[29];
			Result[Z] = a[ 2]*x3+x2*(a[ 5]*y+2*a[ 7]*z+a[ 8])+
			            x*(a[11]*y2+y*(2*a[13]*z+a[14])+3*a[16]*z2+2*a[17]*z+a[18])+
			            a[21]*y3+y2*(2*a[23]*z+a[24])+y*(3*a[26]*z2+2*a[27]*z+a[28])+
			            4*a[30]*z3+3*a[31]*z2+2*a[32]*z+a[33];
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Inside_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

bool Poly::Inside(const VECTOR IPoint, TraceThreadData *Thread) const
{
	VECTOR  New_Point;
	DBL Result;

	/* Transform the point into polynomial's space */

	MInvTransPoint(New_Point, IPoint, Trans);

	Result = inside(New_Point, Order, Coeffs);

	if (Result < INSIDE_TOLERANCE)
	{
		return ((int)(!Test_Flag(this, INVERTED_FLAG)));
	}
	else
	{
		return ((int)Test_Flag(this, INVERTED_FLAG));
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Poly_Normal
*
* INPUT
*   
* OUTPUT
*
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Normal to a polynomial.
*
* CHANGES
*
*   -
*
******************************************************************************/

void Poly::Normal(VECTOR Result, Intersection *Inter, TraceThreadData *Thread) const
{
	DBL val;
	VECTOR  New_Point;

	/* Transform the point into the polynomials space. */

	MInvTransPoint(New_Point, Inter->IPoint, Trans);

	if (Order > 4)
	{
		normal0(Result, Order, Coeffs, New_Point);
	}
	else
	{
		normal1(Result, Order, Coeffs, New_Point);
	}

	/* Transform back to world space. */

	MTransNormal(Result, Result, Trans);

	/* Normalize (accounting for the possibility of a 0 length normal). */

	VDot(val, Result, Result);

	if (val > 0.0)
	{
		val = 1.0 / sqrt(val);

		VScaleEq(Result, val);
	}
	else
	{
		Make_Vector(Result, 1.0, 0.0, 0.0);
	}
}



/*****************************************************************************
*
* FUNCTION
*
*   Translate_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

void Poly::Translate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Rotate_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

void Poly::Rotate(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Scale_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

void Poly::Scale(const VECTOR, const TRANSFORM *tr)
{
	Transform(tr);
}



/*****************************************************************************
*
* FUNCTION
*
*   Transform_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

void Poly::Transform(const TRANSFORM *tr)
{
	Compose_Transforms(Trans, tr);

	Compute_BBox();
}



/*****************************************************************************
*
* FUNCTION
*
*   Invert_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

void Poly::Invert()
{
	Invert_Flag(this, INVERTED_FLAG);
}



/*****************************************************************************
*
* FUNCTION
*
*   Create_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

Poly::Poly(int order) : ObjectBase(POLY_OBJECT)
{
	Order = order;

	Trans = Create_Transform();

	Coeffs = (DBL *)POV_MALLOC(term_counts(Order) * sizeof(DBL), "coefficients for POLY");

	for (int i = 0; i < term_counts(Order); i++)
		Coeffs[i] = 0.0;
}



/*****************************************************************************
*
* FUNCTION
*
*   Copy_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
*   
* DESCRIPTION
*
*   Make a copy of a polynomial object.
*
* CHANGES
*
*   -
*
******************************************************************************/

ObjectPtr Poly::Copy()
{
	Poly *New = new Poly(Order);
	DBL *tmpCoeffs = New->Coeffs;
	int i;

	Destroy_Transform(New->Trans);
	*New = *this;
	New->Coeffs = tmpCoeffs;
	New->Trans = Copy_Transform(Trans);

	for(i = 0; i < term_counts(New->Order); i++)
		New->Coeffs[i] = Coeffs[i];

	return New;
}



/*****************************************************************************
*
* FUNCTION
*
*   Destroy_Poly
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   Alexander Enzmann
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

Poly::~Poly()
{
	Destroy_Transform(Trans);

	POV_FREE(Coeffs);
}



/*****************************************************************************
*
* FUNCTION
*
*   Compute_Poly_BBox
*
* INPUT
*
*   Poly - Poly
*   
* OUTPUT
*
*   Poly
*
* RETURNS
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Calculate the bounding box of a poly.
*
* CHANGES
*
*   Aug 1994 : Creation.
*
******************************************************************************/

void Poly::Compute_BBox()
{
	Make_BBox(BBox, -BOUND_HUGE/2, -BOUND_HUGE/2, -BOUND_HUGE/2, BOUND_HUGE, BOUND_HUGE, BOUND_HUGE);

	if(!Clip.empty())
		BBox = Clip[0]->BBox; // FIXME - does not seem to support more than one bounding object? [trf]
}

bool Poly::Intersect_BBox(BBoxDirection, const BBOX_VECT&, const BBOX_VECT&, BBOX_VAL) const
{
	return true;
}

}
