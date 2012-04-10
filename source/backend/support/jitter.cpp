/*******************************************************************************
 * jitter.cpp
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
 * $File: //depot/povray/smp/source/backend/support/jitter.cpp $
 * $Revision: #8 $
 * $Change: 4715 $
 * $DateTime: 2009/02/21 10:00:39 $
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
#include "backend/support/jitter.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

const float JitterTable[256] =
{
	-0.500000,  0.005890,  0.011749, -0.490234,  0.023468, -0.470703, -0.480469,  0.017609,
	 0.046906, -0.447266, -0.441406,  0.056671, -0.460938,  0.044952,  0.035187, -0.466797,
	 0.093781, -0.400391, -0.394531,  0.103546, -0.382813,  0.123077,  0.113312, -0.388672,
	-0.421875,  0.084015,  0.089874, -0.412109,  0.070343, -0.423828, -0.433594,  0.064484,
	 0.187531, -0.306641, -0.300781,  0.197296, -0.289063,  0.216827,  0.207062, -0.294922,
	-0.265625,  0.240265,  0.246124, -0.255859,  0.226593, -0.267578, -0.277344,  0.220734,
	-0.343750,  0.162140,  0.167999, -0.333984,  0.179718, -0.314453, -0.324219,  0.173859,
	 0.140656, -0.353516, -0.347656,  0.150421, -0.367188,  0.138702,  0.128937, -0.373047,
	 0.375031, -0.119141, -0.113281,  0.384796, -0.101563,  0.404327,  0.394562, -0.107422,
	-0.078125,  0.427765,  0.433624, -0.068359,  0.414093, -0.080078, -0.089844,  0.408234,
	-0.031250,  0.474640,  0.480499, -0.021484,  0.492218, -0.001953, -0.011719,  0.486359,
	 0.453156, -0.041016, -0.035156,  0.462921, -0.054688,  0.451202,  0.441437, -0.060547,
	-0.187500,  0.318390,  0.324249, -0.177734,  0.335968, -0.158203, -0.167969,  0.330109,
	 0.359406, -0.134766, -0.128906,  0.369171, -0.148438,  0.357452,  0.347687, -0.154297,
	 0.281281, -0.212891, -0.207031,  0.291046, -0.195313,  0.310577,  0.300812, -0.201172,
	-0.234375,  0.271515,  0.277374, -0.224609,  0.257843, -0.236328, -0.246094,  0.251984,
	-0.249969,  0.255859,  0.261719, -0.240204,  0.273438, -0.220673, -0.230438,  0.267578,
	 0.296875, -0.197235, -0.191376,  0.306641, -0.210907,  0.294922,  0.285156, -0.216766,
	 0.343750, -0.150360, -0.144501,  0.353516, -0.132782,  0.373047,  0.363281, -0.138641,
	-0.171844,  0.333984,  0.339844, -0.162079,  0.320313, -0.173798, -0.183563,  0.314453,
	 0.437500, -0.056610, -0.050751,  0.447266, -0.039032,  0.466797,  0.457031, -0.044891,
	-0.015594,  0.490234,  0.496094, -0.005829,  0.476563, -0.017548, -0.027313,  0.470703,
	-0.093719,  0.412109,  0.417969, -0.083954,  0.429688, -0.064423, -0.074188,  0.423828,
	 0.390625, -0.103485, -0.097626,  0.400391, -0.117157,  0.388672,  0.378906, -0.123016,
	 0.125000, -0.369110, -0.363251,  0.134766, -0.351532,  0.154297,  0.144531, -0.357391,
	-0.328094,  0.177734,  0.183594, -0.318329,  0.164063, -0.330048, -0.339813,  0.158203,
	-0.281219,  0.224609,  0.230469, -0.271454,  0.242188, -0.251923, -0.261688,  0.236328,
	 0.203125, -0.290985, -0.285126,  0.212891, -0.304657,  0.201172,  0.191406, -0.310516,
	-0.437469,  0.068359,  0.074219, -0.427704,  0.085938, -0.408173, -0.417938,  0.080078,
	 0.109375, -0.384735, -0.378876,  0.119141, -0.398407,  0.107422,  0.097656, -0.404266,
	 0.031250, -0.462860, -0.457001,  0.041016, -0.445282,  0.060547,  0.050781, -0.451141,
	-0.484344,  0.021484,  0.027344, -0.474579,  0.007813, -0.486298, -0.496063,  0.001953,
};

}
