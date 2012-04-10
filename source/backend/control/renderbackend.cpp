/*******************************************************************************
 * renderbackend.cpp
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
 * $File: //depot/povray/smp/source/backend/control/renderbackend.cpp $
 * $Revision: #35 $
 * $Change: 5269 $
 * $DateTime: 2010/12/15 06:02:17 $
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
#include "base/povms.h"
#include "base/povmscpp.h"
#include "base/povmsgid.h"
#include "base/pov_err.h"
#include "backend/povray.h"
#include "backend/control/renderbackend.h"
#include "backend/scene/scene.h"
#include "backend/scene/view.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

using namespace pov_base;

POVMSContext RenderBackend::context = NULL;

RenderBackend::RenderBackend(POVMSContext ctx,  bool (*val)(POVMSAddress)) :
	POVMS_MessageReceiver(ctx),
	validateFrontendAddress(val),
	scenecounter(0),
	viewcounter(0)
{
	context = ctx;

	InstallFront(kPOVMsgClass_BackendControl, kPOVMsgIdent_CreateScene, this, &RenderBackend::CreateScene);
	InstallFront(kPOVMsgClass_BackendControl, kPOVMsgIdent_CloseScene, this, &RenderBackend::CloseScene);

	InstallFront(kPOVMsgClass_SceneControl, kPOVMsgIdent_CreateView, this, &RenderBackend::CreateView);
	InstallFront(kPOVMsgClass_SceneControl, kPOVMsgIdent_CloseView, this, &RenderBackend::CloseView);

	InstallFront(kPOVMsgClass_SceneControl, kPOVMsgIdent_StartParser, this, &RenderBackend::StartParser);
	InstallFront(kPOVMsgClass_SceneControl, kPOVMsgIdent_StopParser, this, &RenderBackend::StopParser);
	InstallFront(kPOVMsgClass_SceneControl, kPOVMsgIdent_PauseParser, this, &RenderBackend::PauseParser);
	InstallFront(kPOVMsgClass_SceneControl, kPOVMsgIdent_ResumeParser, this, &RenderBackend::ResumeParser);

	InstallFront(kPOVMsgClass_ViewControl, kPOVMsgIdent_StartRender, this, &RenderBackend::StartRender);
	InstallFront(kPOVMsgClass_ViewControl, kPOVMsgIdent_StopRender, this, &RenderBackend::StopRender);
	InstallFront(kPOVMsgClass_ViewControl, kPOVMsgIdent_PauseRender, this, &RenderBackend::PauseRender);
	InstallFront(kPOVMsgClass_ViewControl, kPOVMsgIdent_ResumeRender, this, &RenderBackend::ResumeRender);

	InstallFront(kPOVMsgClass_FileAccess, kPOVMsgIdent_ReadFile, this, &RenderBackend::ReadFile);
}

RenderBackend::~RenderBackend()
{
}

void RenderBackend::SendSceneOutput(SceneId sid, POVMSAddress addr, POVMSType ident, POVMS_Object& obj)
{
	POVMS_Message msg(obj, kPOVMsgClass_SceneOutput, ident);

	msg.SetInt(kPOVAttrib_SceneId, sid);
	msg.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, msg, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendViewOutput(ViewId vid, POVMSAddress addr, POVMSType ident, POVMS_Object& obj)
{
	POVMS_Message msg(obj, kPOVMsgClass_ViewOutput, ident);

	msg.SetInt(kPOVAttrib_ViewId, vid);
	msg.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, msg, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendFindFile(POVMSContext ctx, SceneId sid, POVMSAddress addr, const vector<POVMSUCS2String>& filenames, POVMSUCS2String& filename)
{
	POVMS_Message msg(kPOVObjectClass_FileData, kPOVMsgClass_FileAccess, kPOVMsgIdent_FindFile);
	POVMS_Message result(kPOVObjectClass_FileData, kPOVMsgClass_FileAccess, kPOVMsgIdent_FindFile);
	POVMS_List files;

	for(vector<POVMSUCS2String>::const_iterator i(filenames.begin()); i != filenames.end(); i++)
	{
		POVMS_Attribute attr(i->c_str());
		files.Append(attr);
	}

	msg.Set(kPOVAttrib_ReadFile, files);

	msg.SetInt(kPOVAttrib_SceneId, sid);
	msg.SetDestinationAddress(addr);

	POVMS_SendMessage(ctx, msg, &result, kPOVMSSendMode_WaitReply);

	filename = result.TryGetUCS2String(kPOVAttrib_ReadFile, "");
}

void RenderBackend::SendReadFile(POVMSContext ctx, SceneId sid, POVMSAddress addr, const POVMSUCS2String& filename, POVMSUCS2String& localfile, POVMSUCS2String& fileurl)
{
	POVMS_Message msg(kPOVObjectClass_FileData, kPOVMsgClass_FileAccess, kPOVMsgIdent_ReadFile);
	POVMS_Message result(kPOVObjectClass_FileData, kPOVMsgClass_FileAccess, kPOVMsgIdent_ReadFile);

	msg.SetUCS2String(kPOVAttrib_ReadFile, filename.c_str());

	msg.SetInt(kPOVAttrib_SceneId, sid);
	msg.SetDestinationAddress(addr);

	POVMS_SendMessage(ctx, msg, &result, kPOVMSSendMode_WaitReply);

	localfile = result.TryGetUCS2String(kPOVAttrib_LocalFile, "");
	fileurl = result.TryGetUCS2String(kPOVAttrib_FileURL, "");
}

void RenderBackend::SendCreatedFile(POVMSContext ctx, SceneId sid, POVMSAddress addr, const POVMSUCS2String& filename)
{
	POVMS_Message msg(kPOVObjectClass_FileData, kPOVMsgClass_FileAccess, kPOVMsgIdent_CreatedFile);

	msg.SetUCS2String(kPOVAttrib_CreatedFile, filename.c_str());

	msg.SetInt(kPOVAttrib_SceneId, sid);
	msg.SetDestinationAddress(addr);

	POVMS_SendMessage(ctx, msg, NULL, kPOVMSSendMode_NoReply);
}

void RenderBackend::SendSuccessResult(POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_BackendControl, kPOVMsgIdent_Done);

	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendFailedResult(int error, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_BackendControl, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_ErrorNumber, error);
	result.SetString(kPOVAttrib_EnglishText, pov_base::Exception::lookup_code(error).c_str());
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendFailedResult(const pov_base::Exception& e, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_BackendControl, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_ErrorNumber, e.code(kCannotHandleRequestErr));
	// pov_base::Exception(...) does a code->string lookup if a string isn't supplied
	result.SetString(kPOVAttrib_EnglishText, e.what());
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendFailedResult(const char *str, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_BackendControl, kPOVMsgIdent_Failed);

	result.SetString(kPOVAttrib_EnglishText, str);
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendSceneSuccessResult(SceneId sid, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Done);

	result.SetInt(kPOVAttrib_SceneId, sid);
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendSceneFailedResult(SceneId sid, int error, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_SceneId, sid);
	result.SetInt(kPOVAttrib_ErrorNumber, error);
	result.SetString(kPOVAttrib_EnglishText, pov_base::Exception::lookup_code(error).c_str());
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendSceneFailedResult(SceneId sid, const pov_base::Exception& e, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_SceneId, sid);
	result.SetInt(kPOVAttrib_ErrorNumber, e.code(kCannotHandleRequestErr));
	// pov_base::Exception(...) does a code->string lookup if a string isn't supplied
	result.SetString(kPOVAttrib_EnglishText, e.what());
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendSceneFailedResult(SceneId sid, const char *str, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_SceneOutput, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_SceneId, sid);
	result.SetString(kPOVAttrib_EnglishText, str);
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendViewSuccessResult(ViewId vid, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Done);

	result.SetInt(kPOVAttrib_ViewId, vid);
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendViewFailedResult(ViewId vid, int error, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_ViewId, vid);
	result.SetInt(kPOVAttrib_ErrorNumber, error);
	result.SetString(kPOVAttrib_EnglishText, pov_base::Exception::lookup_code(error).c_str());
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendViewFailedResult(ViewId vid, const pov_base::Exception& e, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_ViewId, vid);
	result.SetInt(kPOVAttrib_ErrorNumber, e.code(kCannotHandleRequestErr));
	// pov_base::Exception(...) does a code->string lookup if a string isn't supplied
	result.SetString(kPOVAttrib_EnglishText, e.what());
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::SendViewFailedResult(ViewId vid, const char *str, POVMSAddress addr)
{
	POVMS_Message result(kPOVObjectClass_ResultData, kPOVMsgClass_ViewOutput, kPOVMsgIdent_Failed);

	result.SetInt(kPOVAttrib_ViewId, vid);
	result.SetString(kPOVAttrib_EnglishText, str);
	result.SetDestinationAddress(addr);

	POVMS_SendMessage(RenderBackend::context, result, NULL, kPOVMSSendMode_NoReply); // POVMS context provide for source address access only!
}

void RenderBackend::CreateScene(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		POVMSAddress backendAddress = POVMSInvalidAddress;
		int err = POVMS_GetContextAddress(context, &backendAddress);

		if(err != kNoErr)
			throw POV_EXCEPTION_CODE (err);

		shared_ptr<Scene> scene(new Scene(backendAddress, msg.GetSourceAddress(), scenecounter + 1));

		scenecounter++;

		POVMS_Message newresult(result, result.GetClass(), kPOVMsgIdent_Done);
		result = newresult;
		result.SetInt(kPOVAttrib_SceneId, scenecounter);

		scenes[scenecounter] = scene;
		try
		{
			scene2views[scenecounter] = set<ViewId>();
		}
		catch(std::exception&)
		{
			scenes.erase(scenecounter);
			throw;
		}
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::CloseScene(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		SceneId sid = msg.GetInt(kPOVAttrib_SceneId);

		map<SceneId, shared_ptr<Scene> >::iterator i(scenes.find(sid));

		if(i == scenes.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if(scene2views[sid].size() > 0)
			throw POV_EXCEPTION_CODE(kNotNowErr);

		try { scenes.erase(sid); } catch(pov_base::Exception&) { }
		try { scene2views.erase(sid); } catch(pov_base::Exception&) { }

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::CreateView(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		SceneId sid = msg.GetInt(kPOVAttrib_SceneId);

		map<SceneId, shared_ptr<Scene> >::iterator i(scenes.find(sid));

		if(i == scenes.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		shared_ptr<View> view(i->second->NewView(msg.TryGetInt(kPOVAttrib_Width, 160), msg.TryGetInt(kPOVAttrib_Height, 120), viewcounter + 1));

		viewcounter++;

		POVMS_Message newresult(result, result.GetClass(), kPOVMsgIdent_Done);
		result = newresult;
		result.SetInt(kPOVAttrib_ViewId, viewcounter);

		views[viewcounter] = view;
		try { view2scene[viewcounter] = sid; } catch(std::exception&) { views.erase(viewcounter); throw; }
		try { scene2views[sid].insert(viewcounter); } catch(std::exception&) { views.erase(viewcounter); view2scene.erase(viewcounter); throw; }
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::CloseView(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		ViewId vid = msg.GetInt(kPOVAttrib_ViewId);

		map<ViewId, shared_ptr<View> >::iterator i(views.find(vid));

		if(i == views.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		try { views.erase(vid); } catch(pov_base::Exception&) { }
		try { scene2views[view2scene[vid]].erase(vid); } catch(pov_base::Exception&) { }
		try { view2scene.erase(vid); } catch(pov_base::Exception&) { }

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::StartParser(POVMS_Message& msg, POVMS_Message&, int)
{
	try
	{
		SceneId sid = msg.GetInt(kPOVAttrib_SceneId);

		try
		{
			if(validateFrontendAddress(msg.GetSourceAddress()) == false)
				throw POV_EXCEPTION_CODE(kAuthorisationErr);

			map<SceneId, shared_ptr<Scene> >::iterator i(scenes.find(sid));

			if(i == scenes.end())
				throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

			i->second->StartParser(msg);
		}
		catch(pov_base::Exception& e)
		{
			SendSceneFailedResult(sid, e, msg.GetSourceAddress());
		}
	}
	catch(pov_base::Exception& e)
	{
		SendFailedResult(e, msg.GetSourceAddress());
	}
	catch(std::runtime_error& e)
	{
		SendFailedResult(e.what(), msg.GetSourceAddress());
	}
	catch(std::bad_alloc&)
	{
		SendFailedResult(kOutOfMemoryErr, msg.GetSourceAddress());
	}
}

void RenderBackend::StopParser(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		SceneId sid = msg.GetInt(kPOVAttrib_SceneId);

		map<SceneId, shared_ptr<Scene> >::iterator i(scenes.find(sid));

		if(i == scenes.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if((i->second->IsParsing() == false) && (i->second->IsPaused() == false))
			throw POV_EXCEPTION_CODE(kNotNowErr);

		i->second->StopParser();

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::PauseParser(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		SceneId sid = msg.GetInt(kPOVAttrib_SceneId);

		map<SceneId, shared_ptr<Scene> >::iterator i(scenes.find(sid));

		if(i == scenes.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if((i->second->IsParsing() == false) && (i->second->IsPaused() == false))
			throw POV_EXCEPTION_CODE(kNotNowErr);

		i->second->PauseParser();

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::ResumeParser(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		SceneId sid = msg.GetInt(kPOVAttrib_SceneId);

		map<SceneId, shared_ptr<Scene> >::iterator i(scenes.find(sid));

		if(i == scenes.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if((i->second->IsParsing() == false) && (i->second->IsPaused() == false))
			throw POV_EXCEPTION_CODE(kNotNowErr);

		i->second->ResumeParser();

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::StartRender(POVMS_Message& msg, POVMS_Message&, int)
{
	try
	{
		ViewId vid = msg.GetInt(kPOVAttrib_ViewId);

		try
		{
			if(validateFrontendAddress(msg.GetSourceAddress()) == false)
				throw POV_EXCEPTION_CODE(kAuthorisationErr);

			map<ViewId, shared_ptr<View> >::iterator i(views.find(vid));

			if(i == views.end())
				throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

			if(i->second->IsRendering() == true)
				throw POV_EXCEPTION_CODE(kNotNowErr);

			i->second->StartRender(msg);
		}
		catch(pov_base::Exception& e)
		{
			SendViewFailedResult(vid, e, msg.GetSourceAddress());
		}
	}
	catch(pov_base::Exception& e)
	{
		SendFailedResult(e, msg.GetSourceAddress());
	}
	catch(std::runtime_error& e)
	{
		SendFailedResult(e.what(), msg.GetSourceAddress());
	}
	catch(std::bad_alloc&)
	{
		SendFailedResult(kOutOfMemoryErr, msg.GetSourceAddress());
	}
}

void RenderBackend::StopRender(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		ViewId vid = msg.GetInt(kPOVAttrib_ViewId);

		map<ViewId, shared_ptr<View> >::iterator i(views.find(vid));

		if(i == views.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if((i->second->IsRendering() == false) && (i->second->IsPaused() == false))
			throw POV_EXCEPTION_CODE(kNotNowErr);

		i->second->StopRender();

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::PauseRender(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		ViewId vid = msg.GetInt(kPOVAttrib_ViewId);

		map<ViewId, shared_ptr<View> >::iterator i(views.find(vid));

		if(i == views.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if((i->second->IsRendering() == false) && (i->second->IsPaused() == false))
			throw POV_EXCEPTION_CODE(kNotNowErr);

		i->second->PauseRender();

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::ResumeRender(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	try
	{
		ViewId vid = msg.GetInt(kPOVAttrib_ViewId);

		map<ViewId, shared_ptr<View> >::iterator i(views.find(vid));

		if(i == views.end())
			throw POV_EXCEPTION_CODE(kInvalidIdentifierErr);

		if((i->second->IsRendering() == false) && (i->second->IsPaused() == false))
			throw POV_EXCEPTION_CODE(kNotNowErr);

		i->second->ResumeRender();

		MakeDoneResult(result);
	}
	catch(pov_base::Exception& e)
	{
		MakeFailedResult(e, result);
	}
	catch(std::runtime_error& e)
	{
		MakeFailedResult(e.what(), result);
	}
	catch(std::bad_alloc&)
	{
		MakeFailedResult(kOutOfMemoryErr, result);
	}
}

void RenderBackend::ReadFile(POVMS_Message& msg, POVMS_Message& result, int)
{
	if(ValidateFrontendAddress(msg.GetSourceAddress(), result) == false)
		return;

	MakeFailedResult(kCannotHandleRequestErr, result);
}

bool RenderBackend::ValidateFrontendAddress(POVMSAddress addr, POVMS_Message& result)
{
	if(validateFrontendAddress(addr) == false)
	{
		MakeFailedResult(kAuthorisationErr, result);

		return false;
	}
	else
		return true;
}

void RenderBackend::MakeFailedResult(int error, POVMS_Message& result)
{
	POVMS_Message newmsg(result, result.GetClass(), kPOVMsgIdent_Failed);
	result = newmsg;
	result.SetInt(kPOVAttrib_ErrorNumber, error);
	result.SetString(kPOVAttrib_EnglishText, pov_base::Exception::lookup_code(error).c_str());
}

void RenderBackend::MakeFailedResult(const pov_base::Exception& e, POVMS_Message& result)
{
	POVMS_Message newmsg(result, result.GetClass(), kPOVMsgIdent_Failed);
	result = newmsg;
	result.SetInt(kPOVAttrib_ErrorNumber, e.code(kCannotHandleRequestErr));
	// pov_base::Exception(...) does a code->string lookup if a string isn't supplied
	result.SetString(kPOVAttrib_EnglishText, e.what());
}

void RenderBackend::MakeFailedResult(const char *str, POVMS_Message& result)
{
	POVMS_Message newmsg(result, result.GetClass(), kPOVMsgIdent_Failed);
	result = newmsg;
	result.SetString(kPOVAttrib_EnglishText, str);
}

void RenderBackend::MakeDoneResult(POVMS_Message& result)
{
	POVMS_Message newmsg(result, result.GetClass(), kPOVMsgIdent_Done);
	result = newmsg;
}

}
