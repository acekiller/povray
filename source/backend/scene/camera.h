/*******************************************************************************
 * camera.h
 *
 * This module contains all defines, typedefs, and prototypes for CAMERA.CPP.
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
 * $File: //depot/povray/smp/source/backend/scene/camera.h $
 * $Revision: #19 $
 * $Change: 5136 $
 * $DateTime: 2010/09/02 11:36:18 $
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


#ifndef CAMERA_H
#define CAMERA_H

namespace pov
{

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

// Available camera types. [DB 8/94]

#define PERSPECTIVE_CAMERA      1
#define ORTHOGRAPHIC_CAMERA     2
#define FISHEYE_CAMERA          3
#define ULTRA_WIDE_ANGLE_CAMERA 4
#define OMNIMAX_CAMERA          5
#define PANORAMIC_CAMERA        6
#define CYL_1_CAMERA            7
#define CYL_2_CAMERA            8
#define CYL_3_CAMERA            9
#define CYL_4_CAMERA           10
#define SPHERICAL_CAMERA       11
#define MESH_CAMERA            12

/*****************************************************************************
* Global typedefs
******************************************************************************/

class Camera
{
public:
	VECTOR Location;
	VECTOR Direction;
	VECTOR Up;
	VECTOR Right;
	VECTOR Sky;
	VECTOR Look_At;                 // Used only to record the user's preference
	VECTOR Focal_Point;             // Used only to record the user's preference
	DBL Focal_Distance, Aperture;   // ARE 9/92 for focal blur.
	int Blur_Samples;               // ARE 9/92 for focal blur.
	int Blur_Samples_Min;           // Minimum number of blur samples to take regardless of confidence settings.
	DBL Confidence;                 // Probability for confidence test.
	DBL Variance;                   // Max. variance for confidence test.
	int Type;                       // Camera type.
	DBL Angle;                      // Viewing angle.
	DBL H_Angle;                    // Spherical horizontal viewing angle
	DBL V_Angle;                    // Spherical verticle viewing angle
	TNORMAL *Tnormal;               // Primary ray pertubation.
	TRANSFORM *Trans;               // Used only to record the user's input
	PIGMENT *Bokeh;                 // Pigment to use for the bokeh

	// the following declarations are used for the mesh camera
	unsigned int Face_Distribution_Method;  // how to associate a pixel to a face within a mesh
	unsigned int Rays_Per_Pixel;            // cast this many rays per pixel; never less than 1
	bool Smooth;                            // if true, interpolate normals for dist #3
	vector<ObjectPtr> Meshes;               // list of the meshes to be used as the camera
	vector<unsigned int> Mesh_Index;        // used with distribution #1 to keep track of accumulated meshes
	vector<unsigned int> U_Xref[10];        // used to speed up location of a matching face for distribution #3
	vector<unsigned int> V_Xref[10];        // used to speed up location of a matching face for distribution #3
	DBL Max_Ray_Distance;					// if not 0.0, then maximum distance to look along the ray for an intersection
	// end of mesh camera declarations

	Camera();
	Camera(const Camera& src);
	~Camera();
	Camera& operator=(const Camera& rhs);
	void Transform(const TRANSFORM *Trans);
	void Scale(const VECTOR Vector);
	void Rotate(const VECTOR Vector);
	void Translate(const VECTOR Vector);

private:
	void Init();
};

}

#endif
