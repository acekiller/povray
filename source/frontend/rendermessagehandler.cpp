/*******************************************************************************
 * rendermessagehandler.cpp
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
 * $File: //depot/povray/smp/source/frontend/rendermessagehandler.cpp $
 * $Revision: #28 $
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

#include <sstream>

// configbase.h must always be the first POV file included within base *.cpp files
#include "base/configbase.h"
#include "base/types.h"

#include "frontend/configfrontend.h"
#include "frontend/renderfrontend.h"
#include "frontend/filemessagehandler.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov_frontend
{

RenderMessageHandler::RenderMessageHandler()
{
}

RenderMessageHandler::~RenderMessageHandler()
{
}

void RenderMessageHandler::HandleMessage(const SceneData& sd, const ViewData&, POVMSType ident, POVMS_Object& obj)
{
	if(ident != kPOVMsgIdent_Progress)
	{
		sd.console->flush();
		if(sd.streams[STATUS_STREAM] != NULL)
			sd.streams[STATUS_STREAM]->flush();
	}

	switch(ident)
	{
		case kPOVMsgIdent_RenderOptions:
			Options(sd.console.get(), obj, sd.consoleoutput[RENDER_STREAM]);
			if(sd.streams[RENDER_STREAM].get() != NULL)
				Message2Console::RenderOptions(obj, sd.streams[RENDER_STREAM].get());
			if(sd.streams[ALL_STREAM].get() != NULL)
				Message2Console::RenderOptions(obj, sd.streams[ALL_STREAM].get());
			break;
		case kPOVMsgIdent_RenderStatistics:
			Statistics(sd.console.get(), obj, sd.consoleoutput[STATISTIC_STREAM]);
			if(sd.streams[STATISTIC_STREAM].get() != NULL)
			{
				Message2Console::RenderStatistics(obj, sd.streams[STATISTIC_STREAM].get());
				Message2Console::RenderTime(obj, sd.streams[STATISTIC_STREAM].get());
			}
			if(sd.streams[ALL_STREAM].get() != NULL)
			{
				Message2Console::RenderStatistics(obj, sd.streams[ALL_STREAM].get());
				Message2Console::RenderTime(obj, sd.streams[ALL_STREAM].get());
			}
			break;
		case kPOVMsgIdent_Progress:
			Progress(sd.console.get(), obj, sd.verbose);
//			if(sd.streams[ALL_STREAM].get() != NULL)
//				Message2Console::Progress(obj, sd.streams[ALL_STREAM]);
			break;
		case kPOVMsgIdent_Warning:
			Warning(sd.console.get(), obj, sd.consoleoutput[WARNING_STREAM]);
			if(sd.streams[WARNING_STREAM].get() != NULL)
				Message2Console::Warning(obj, sd.streams[WARNING_STREAM].get());
			if(sd.streams[ALL_STREAM].get() != NULL)
				Message2Console::Warning(obj, sd.streams[ALL_STREAM].get());
			break;
		case kPOVMsgIdent_Error:
			Error(sd.console.get(), obj, sd.consoleoutput[WARNING_STREAM]);
			if(sd.streams[WARNING_STREAM].get() != NULL)
				Message2Console::Error(obj, sd.streams[WARNING_STREAM].get());
			if(sd.streams[ALL_STREAM].get() != NULL)
				Message2Console::Error(obj, sd.streams[ALL_STREAM].get());
			break;
		case kPOVMsgIdent_FatalError:
			FatalError(sd.console.get(), obj, sd.consoleoutput[FATAL_STREAM]);
			if(sd.streams[FATAL_STREAM].get() != NULL)
				Message2Console::FatalError(obj, sd.streams[FATAL_STREAM].get());
			if(sd.streams[ALL_STREAM].get() != NULL)
				Message2Console::FatalError(obj, sd.streams[ALL_STREAM].get());
			break;
	}
}

void RenderMessageHandler::Options(Console *console, POVMS_Object& obj, bool conout)
{
	if(conout == true)
		Message2Console::RenderOptions(obj, console);
}

void RenderMessageHandler::Statistics(Console *console, POVMS_Object& obj, bool conout)
{
	if(conout == true)
	{
		Message2Console::RenderStatistics(obj, console);
		Message2Console::RenderTime(obj, console);
	}
}

void RenderMessageHandler::Progress(Console *console, POVMS_Object& obj, bool verbose)
{
	ostringstream sstr;

	switch(obj.GetType(kPOVMSObjectClassID))
	{
		case kPOVObjectClass_PhotonProgress:
		{
			int cpc(obj.GetInt(kPOVAttrib_CurrentPhotonCount));

			sstr << Message2Console::GetProgressTime(obj, kPOVAttrib_RealTime)
			     << " Photons " << cpc << "    \r";
			break;
		}
		case kPOVObjectClass_RadiosityProgress:
		{
			int pt(obj.GetInt(kPOVAttrib_Pixels));
			int pc(obj.GetInt(kPOVAttrib_PixelsCompleted));
			int percent = 0;

			if(pt > 0)
				percent = (pc * 100) / pt;

			sstr << Message2Console::GetProgressTime(obj, kPOVAttrib_RealTime)
			     << " Radiosity pretrace completed " << pc << " of " << pt << " pixels (" << percent << "%)    \r";
			break;
		}
		case kPOVObjectClass_RenderProgress:
		{
			int pt(obj.GetInt(kPOVAttrib_Pixels));
			int pp(obj.GetInt(kPOVAttrib_PixelsPending));
			int pc(obj.GetInt(kPOVAttrib_PixelsCompleted));
			int percent = 0;

			if(pt > 0)
				percent = (pc * 100) / pt;

			sstr << Message2Console::GetProgressTime(obj, kPOVAttrib_RealTime)
			     << "Rendering completed " << pc << " of " << pt << " pixels (" << percent << "%) and " << pp << " pixels pending    \r";
			break;
		}
	}

	console->Output(sstr.str());
}

void RenderMessageHandler::Warning(Console *console, POVMS_Object& obj, bool conout)
{
	if(conout == true)
		Message2Console::Warning(obj, console);
}

void RenderMessageHandler::Error(Console *console, POVMS_Object& obj, bool conout)
{
	if(conout == true)
		Message2Console::Error(obj, console);
}

void RenderMessageHandler::FatalError(Console *console, POVMS_Object& obj, bool conout)
{
	if(conout == true)
		Message2Console::FatalError(obj, console);
}

}
