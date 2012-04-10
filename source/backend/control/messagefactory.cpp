/*******************************************************************************
 * messagefactory.cpp
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
 * $File: //depot/povray/smp/source/backend/control/messagefactory.cpp $
 * $Revision: #20 $
 * $Change: 5088 $
 * $DateTime: 2010/08/05 17:08:44 $
 * $Author: clipka $
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

#include <boost/thread.hpp>
#include <boost/bind.hpp>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "base/povms.h"
#include "base/povmscpp.h"
#include "base/povmsgid.h"
#include "base/pov_err.h"
#include "backend/control/messagefactory.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

MessageFactory::MessageFactory(unsigned int wl, unsigned int lv, const char *sn, POVMSAddress saddr, POVMSAddress daddr, RenderBackend::SceneId sid, RenderBackend::ViewId vid) :
	warningLevel(wl),
	languageVersion(lv),
	stageName(sn),
	sourceAddress(saddr),
	destinationAddress(daddr),
	sceneId(sid),
	viewId(vid)
{
}

void MessageFactory::Warning(unsigned int level, const char *format,...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	sprintf(localvsbuffer, "%s Warning: ", stageName);

	va_start(marker, format);
	vsnprintf(localvsbuffer + strlen(localvsbuffer), 1023 - strlen(localvsbuffer), format, marker);
	va_end(marker);

	CleanupString(localvsbuffer);

	if((warningLevel < 5) || ((warningLevel < 10) && (level == 0)))
		return;

	if(level >= languageVersion)
		return;

	(void)POVMSObject_New(&msg, kPOVObjectClass_ControlData);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Warning, 0);

	if(viewId != 0)
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_ViewId, viewId);
	else
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_SceneId, sceneId);

	if(viewId != 0)
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Warning);
	else
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Warning);

	(void)POVMSMsg_SetSourceAddress(&msg, sourceAddress);
	(void)POVMSMsg_SetDestinationAddress(&msg, destinationAddress);

	(void)POVMS_Send(NULL, &msg, NULL, kPOVMSSendMode_NoReply);
}

void MessageFactory::WarningAt(unsigned int level, const UCS2 *filename, POV_LONG line, POV_LONG column, POV_LONG offset, const char *format, ...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	sprintf(localvsbuffer, "%s Warning: ", stageName);

	va_start(marker, format);
	vsnprintf(localvsbuffer + strlen(localvsbuffer), 1023 - strlen(localvsbuffer), format, marker);
	va_end(marker);

	CleanupString(localvsbuffer);

	if((warningLevel < 5) || ((warningLevel < 10) && (level == 0)))
		return;

	if(level >= languageVersion)
		return;

	(void)POVMSObject_New(&msg, kPOVObjectClass_ControlData);
	(void)POVMSUtil_SetUCS2String(&msg, kPOVAttrib_FileName, filename);
	(void)POVMSUtil_SetLong(&msg, kPOVAttrib_Line, line);
	(void)POVMSUtil_SetLong(&msg, kPOVAttrib_Column, column);
	(void)POVMSUtil_SetLong(&msg, kPOVAttrib_FilePosition, offset);

	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Warning, 0);

	if(viewId != 0)
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_ViewId, viewId);
	else
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_SceneId, sceneId);

	if(viewId != 0)
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Warning);
	else
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Warning);

	(void)POVMSMsg_SetSourceAddress(&msg, sourceAddress);
	(void)POVMSMsg_SetDestinationAddress(&msg, destinationAddress);

	(void)POVMS_Send(NULL, &msg, NULL, kPOVMSSendMode_NoReply);
}

void MessageFactory::PossibleError(const char *format,...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	sprintf(localvsbuffer, "Possible %s Error: ", stageName);

	va_start(marker, format);
	vsnprintf(localvsbuffer + strlen(localvsbuffer), 1023 - strlen(localvsbuffer), format, marker);
	va_end(marker);

	CleanupString(localvsbuffer);

	if(warningLevel == 0)
		return;

	(void)POVMSObject_New(&msg, kPOVObjectClass_ControlData);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);

	if(viewId != 0)
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_ViewId, viewId);
	else
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_SceneId, sceneId);

	if(viewId != 0)
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Error);
	else
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Error);

	(void)POVMSMsg_SetSourceAddress(&msg, sourceAddress);
	(void)POVMSMsg_SetDestinationAddress(&msg, destinationAddress);

	(void)POVMS_Send(NULL, &msg, NULL, kPOVMSSendMode_NoReply);
}

void MessageFactory::PossibleErrorAt(const UCS2 *filename, POV_LONG line, POV_LONG column, POV_LONG offset, const char *format, ...)
{
	va_list marker;
	POVMSObject msg;
	char localvsbuffer[1024];

	sprintf(localvsbuffer, "Possible %s Error: ", stageName);

	va_start(marker, format);
	vsnprintf(localvsbuffer + strlen(localvsbuffer), 1023 - strlen(localvsbuffer), format, marker);
	va_end(marker);

	CleanupString(localvsbuffer);

	if(warningLevel == 0)
		return;

	(void)POVMSObject_New(&msg, kPOVObjectClass_ControlData);
	(void)POVMSUtil_SetUCS2String(&msg, kPOVAttrib_FileName, filename);
	(void)POVMSUtil_SetLong(&msg, kPOVAttrib_Line, line);
	(void)POVMSUtil_SetLong(&msg, kPOVAttrib_Column, column);
	(void)POVMSUtil_SetLong(&msg, kPOVAttrib_FilePosition, offset);

	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);

	if(viewId != 0)
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_ViewId, viewId);
	else
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_SceneId, sceneId);

	if(viewId != 0)
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Error);
	else
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Error);

	(void)POVMSMsg_SetSourceAddress(&msg, sourceAddress);
	(void)POVMSMsg_SetDestinationAddress(&msg, destinationAddress);

	(void)POVMS_Send(NULL, &msg, NULL, kPOVMSSendMode_NoReply);
}

// filename defaults to NULL, and line, column, and offset default to -1
std::string MessageFactory::SendError(const char *format, va_list arglist, const UCS2 *filename, POV_LONG line, POV_LONG column, POV_LONG offset)
{
	POVMSObject msg;
	char localvsbuffer[1024];

	sprintf(localvsbuffer, "%s Error: ", stageName);
	vsnprintf(localvsbuffer + strlen(localvsbuffer), 1023 - strlen(localvsbuffer), format, arglist);
	CleanupString(localvsbuffer);

	(void)POVMSObject_New(&msg, kPOVObjectClass_ControlData);
	if (filename != NULL)
		(void)POVMSUtil_SetUCS2String(&msg, kPOVAttrib_FileName, filename);
	if (line != -1)
		(void)POVMSUtil_SetLong(&msg, kPOVAttrib_Line, line);
	if (column != -1)
		(void)POVMSUtil_SetLong(&msg, kPOVAttrib_Column, column);
	if (offset != -1)
		(void)POVMSUtil_SetLong(&msg, kPOVAttrib_FilePosition, offset);
	(void)POVMSUtil_SetString(&msg, kPOVAttrib_EnglishText, localvsbuffer);
	(void)POVMSUtil_SetInt(&msg, kPOVAttrib_Error, 0);

	if(viewId != 0)
	{
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_ViewId, viewId);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_ViewOutput, kPOVMsgIdent_FatalError);
	}
	else
	{
		(void)POVMSUtil_SetInt(&msg, kPOVAttrib_SceneId, sceneId);
		(void)POVMSMsg_SetupMessage(&msg, kPOVMsgClass_SceneOutput, kPOVMsgIdent_FatalError);
	}

	(void)POVMSMsg_SetSourceAddress(&msg, sourceAddress);
	(void)POVMSMsg_SetDestinationAddress(&msg, destinationAddress);

	(void)POVMS_Send(NULL, &msg, NULL, kPOVMSSendMode_NoReply);

	return std::string(localvsbuffer);
}

void MessageFactory::Error(const char *format, ...)
{
	va_list marker;

	va_start(marker, format);
	std::string text = SendError(format, marker);
	va_end(marker);

	// Terminate by throwing an exception with the notification flag already set
	pov_base::Exception ex(__FUNCTION__, __FILE__, __LINE__, text);
	ex.frontendnotified(true);
	throw ex;
}

void MessageFactory::Error(const Exception& ex, const char *format, ...)
{
	va_list marker;

	// if the front-end has been told about this exception already, we don't want to tell it again
	// (we presume that the text given by format and its parameters relate to that exception)
	// this form of frontendnotified() doesn't change the state of ex
	if (ex.frontendnotified())
		throw ex;

	va_start(marker, format);
	SendError(format, marker);
	va_end(marker);

	// now take a copy of ex and set its notification flag, then throw that.
	pov_base::Exception local_ex(ex);
	local_ex.frontendnotified(true);
	throw local_ex;
}

void MessageFactory::Error(Exception& ex, const char *format, ...)
{
	va_list marker;

	// if the front-end has been told about this exception already, we don't want to tell it again
	// (we presume that the text given by format and its parameters relate to that exception)
	// this form of frontendnotified() sets the notified state of ex
	if (ex.frontendnotified(true))
		throw ex;

	va_start(marker, format);
	SendError(format, marker);
	va_end(marker);

	// Terminate
	throw ex;
}

void MessageFactory::ErrorAt(const UCS2 *filename, POV_LONG line, POV_LONG column, POV_LONG offset, const char *format, ...)
{
	va_list marker;

	va_start(marker, format);
	std::string text = SendError(format, marker, filename, line, column, offset);
	va_end(marker);

	// Terminate by throwing an exception with the notification flag already set
	pov_base::Exception ex(__FUNCTION__, __FILE__, __LINE__, text);
	ex.frontendnotified(true);

	throw ex;
}

void MessageFactory::ErrorAt(const Exception& ex, const UCS2 *filename, POV_LONG line, POV_LONG column, POV_LONG offset, const char *format, ...)
{
	va_list marker;

	if (ex.frontendnotified())
		throw ex;

	va_start(marker, format);
	SendError(format, marker, filename, line, column, offset);
	va_end(marker);

	pov_base::Exception local_ex(ex);
	local_ex.frontendnotified(true);
	throw local_ex;
}

void MessageFactory::ErrorAt(Exception& ex, const UCS2 *filename, POV_LONG line, POV_LONG column, POV_LONG offset, const char *format, ...)
{
	va_list marker;

	if (ex.frontendnotified(true))
		throw ex;

	va_start(marker, format);
	SendError(format, marker, filename, line, column, offset);
	va_end(marker);

	throw ex;
}

void MessageFactory::CleanupString(char *str)
{
	while(*str != 0)
	{
		if(*str == '\n')
			*str = ' ';
		str++;
	}
}

}
