/*******************************************************************************
 * animationprocessing.cpp
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
 * $File: //depot/povray/smp/source/frontend/animationprocessing.cpp $
 * $Revision: #16 $
 * $Change: 5212 $
 * $DateTime: 2010/11/29 14:07:44 $
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

#include <string>
#include <cctype>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/types.h"
#include "base/povmscpp.h"
#include "base/povmsgid.h"

#include "frontend/configfrontend.h"
#include "frontend/animationprocessing.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_frontend
{

AnimationProcessing::AnimationProcessing(POVMS_Object& options) :
	renderOptions(options)
{
	bool cyclic = renderOptions.TryGetBool(kPOVAttrib_CyclicAnimation, false);

	initialFrame = max(renderOptions.TryGetInt(kPOVAttrib_InitialFrame, 1), 0);
	initialClock = renderOptions.TryGetFloat(kPOVAttrib_InitialClock, 0.0);

	finalFrame = max(renderOptions.TryGetInt(kPOVAttrib_FinalFrame, 1), initialFrame);
	finalClock = renderOptions.TryGetFloat(kPOVAttrib_FinalClock, 1.0);

	frameStep = max(renderOptions.TryGetInt(kPOVAttrib_FrameStep, 1), 1);

	if(cyclic == true)
		finalFrame++;

	if(renderOptions.Exist(kPOVAttrib_SubsetStartFrame) == true)
	{
		POVMSFloat subsetStartPercent = max(renderOptions.GetFloat(kPOVAttrib_SubsetStartFrame), POVMSFloat(0.0));
		if((subsetStartPercent == 0.0) || (subsetStartPercent >= 1.0))
			subsetStartFrame = POVMSInt(subsetStartPercent);
		else
			subsetStartFrame = POVMSFloat((finalFrame - initialFrame + 1) * subsetStartPercent);
		if (subsetStartFrame < initialFrame)
			subsetStartFrame = initialFrame; // TODO: should we be issuing a warning or throwing an error here?
	}
	else
		subsetStartFrame = initialFrame;

	if(options.Exist(kPOVAttrib_SubsetEndFrame) == true)
	{
		POVMSFloat subsetEndPercent = max(renderOptions.GetFloat(kPOVAttrib_SubsetEndFrame), POVMSFloat(0.0));
		if((subsetEndPercent == 0.0) || (subsetEndPercent >= 1.0))
			subsetEndFrame = POVMSInt(subsetEndPercent);
		else
			subsetEndFrame = POVMSFloat((finalFrame - initialFrame + 1) * subsetEndPercent);
		if (subsetEndFrame < subsetStartFrame)
			subsetEndFrame = subsetStartFrame; // TODO: should we be issuing a warning or throwing an error here?
	}
	else
	{
		subsetEndFrame = finalFrame;
		if(cyclic == true)
			subsetEndFrame--;
	}

	fieldRenderFlag = renderOptions.TryGetBool(kPOVAttrib_FieldRender, false);
	oddFieldFlag = renderOptions.TryGetBool(kPOVAttrib_OddField, false);

	clockDelta = double(finalClock - initialClock) / double(finalFrame - initialFrame);
	frameNumber = subsetStartFrame;
	clockValue = POVMSFloat(double(clockDelta * double(subsetStartFrame - initialFrame)) + double(initialClock));

	frameNumberDigits = 1;
	for(POVMSInt i = finalFrame; i >= 10; i /= 10)
		frameNumberDigits++;
}

POVMS_Object AnimationProcessing::GetFrameRenderOptions()
{
	POVMS_Object opts(renderOptions);

	opts.SetFloat(kPOVAttrib_Clock, clockValue);

	POVMS_List declares;
	if(opts.Exist(kPOVAttrib_Declare) == true)
		opts.Get(kPOVAttrib_Declare, declares);

	POVMS_Object clock_delta(kPOVMSType_WildCard);
	clock_delta.SetString(kPOVAttrib_Identifier, "clock_delta");
	clock_delta.SetFloat(kPOVAttrib_Value, POVMSFloat(clockDelta));
	declares.Append(clock_delta);

	POVMS_Object final_clock(kPOVMSType_WildCard);
	final_clock.SetString(kPOVAttrib_Identifier, "final_clock");
	final_clock.SetFloat(kPOVAttrib_Value, finalClock);
	declares.Append(final_clock);

	POVMS_Object final_frame(kPOVMSType_WildCard);
	final_frame.SetString(kPOVAttrib_Identifier, "final_frame");
	final_frame.SetFloat(kPOVAttrib_Value, finalFrame);
	declares.Append(final_frame);

	POVMS_Object frame_number(kPOVMSType_WildCard);
	frame_number.SetString(kPOVAttrib_Identifier, "frame_number");
	frame_number.SetFloat(kPOVAttrib_Value, frameNumber);
	declares.Append(frame_number);

	POVMS_Object initial_clock(kPOVMSType_WildCard);
	initial_clock.SetString(kPOVAttrib_Identifier, "initial_clock");
	initial_clock.SetFloat(kPOVAttrib_Value, initialClock);
	declares.Append(initial_clock);

	POVMS_Object initial_frame(kPOVMSType_WildCard);
	initial_frame.SetString(kPOVAttrib_Identifier, "initial_frame");
	initial_frame.SetFloat(kPOVAttrib_Value, initialFrame);
	declares.Append(initial_frame);

	//frameStep not provided on purpose: it should remains invisible to the SDL
	//If a scene really needed it, it would be better tweaking the frame-range & clock

	opts.Set(kPOVAttrib_Declare, declares);

	return opts;
}

void AnimationProcessing::ComputeNextFrame()
{
	frameNumber+= frameStep;
	clockValue = POVMSFloat(double(clockDelta * double(frameNumber - initialFrame)) + double(initialClock));
}

bool AnimationProcessing::MoreFrames()
{
	return (frameNumber+frameStep <= subsetEndFrame);
}

POVMSInt AnimationProcessing::GetFrameNumber()
{
	return frameNumber;
}

POVMSInt AnimationProcessing::GetStartFrame()
{
	return subsetStartFrame;
}

POVMSInt AnimationProcessing::GetEndFrame()
{
	return subsetEndFrame;
}

POVMSInt AnimationProcessing::GetTotalFrames()
{
	return subsetEndFrame - subsetStartFrame + 1;
}

POVMSFloat AnimationProcessing::GetClockValue()
{
	return clockValue;
}

int AnimationProcessing::GetFrameNumberDigits()
{
	return frameNumberDigits;
}

}
