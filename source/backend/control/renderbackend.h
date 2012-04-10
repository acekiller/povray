/*******************************************************************************
 * renderbackend.h
 *
 * This file contains ... TODO ...
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
 * $File: //depot/povray/smp/source/backend/control/renderbackend.h $
 * $Revision: #19 $
 * $Change: 5107 $
 * $DateTime: 2010/08/27 14:53:23 $
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

#ifndef POVRAY_BACKEND_RENDERBACKEND_H
#define POVRAY_BACKEND_RENDERBACKEND_H

#include <map>
#include <set>

#include "base/povms.h"
#include "base/povmscpp.h"
#include "backend/povray.h"

namespace pov
{

using namespace pov_base;

class Scene;
class View;

/**
 *	RenderBackend class receives render control messages from the
 *	RenderFrontend class.  Thee control messages are processed and
 *	parsing, rendering or other operations are performed as
 *	requested by the frontend.
 */
class RenderBackend : public POVMS_MessageReceiver
{
	public:
		typedef POVMSInt SceneId;
		typedef POVMSInt ViewId;

		RenderBackend(POVMSContext ctx, bool (*val)(POVMSAddress));
		~RenderBackend();

		static void SendSceneOutput(SceneId sid, POVMSAddress addr, POVMSType ident, POVMS_Object& obj);
		static void SendViewOutput(ViewId vid, POVMSAddress addr, POVMSType ident, POVMS_Object& obj);

		static void SendFindFile(POVMSContext ctx, SceneId sid, POVMSAddress addr, const vector<UCS2String>& filenames, UCS2String& filename);
		static void SendReadFile(POVMSContext ctx, SceneId sid, POVMSAddress addr, const UCS2String& filename, UCS2String& localfile, UCS2String& fileurl);
		static void SendCreatedFile(POVMSContext ctx, SceneId sid, POVMSAddress addr, const UCS2String& filename);

		static void SendSuccessResult(POVMSAddress addr);
		static void SendFailedResult(int error, POVMSAddress addr);
		static void SendFailedResult(const pov_base::Exception& e, POVMSAddress addr);
		static void SendFailedResult(const char *str, POVMSAddress addr);

		static void SendSceneSuccessResult(SceneId sid, POVMSAddress addr);
		static void SendSceneFailedResult(SceneId sid, int error, POVMSAddress addr);
		static void SendSceneFailedResult(SceneId sid, const pov_base::Exception& e, POVMSAddress addr);
		static void SendSceneFailedResult(SceneId sid, const char *str, POVMSAddress addr);

		static void SendViewSuccessResult(ViewId vid, POVMSAddress addr);
		static void SendViewFailedResult(ViewId vid, int error, POVMSAddress addr);
		static void SendViewFailedResult(ViewId vid, const pov_base::Exception& e, POVMSAddress addr);
		static void SendViewFailedResult(ViewId vid, const char *str, POVMSAddress addr);
	protected:
		void CreateScene(POVMS_Message& msg, POVMS_Message& result, int mode);
		void CloseScene(POVMS_Message& msg, POVMS_Message& result, int mode);

		void CreateView(POVMS_Message& msg, POVMS_Message& result, int mode);
		void CloseView(POVMS_Message& msg, POVMS_Message& result, int mode);

		void StartParser(POVMS_Message& msg, POVMS_Message& result, int mode);
		void StopParser(POVMS_Message& msg, POVMS_Message& result, int mode);
		void PauseParser(POVMS_Message& msg, POVMS_Message& result, int mode);
		void ResumeParser(POVMS_Message& msg, POVMS_Message& result, int mode);

		void StartRender(POVMS_Message& msg, POVMS_Message& result, int mode);
		void StopRender(POVMS_Message& msg, POVMS_Message& result, int mode);
		void PauseRender(POVMS_Message& msg, POVMS_Message& result, int mode);
		void ResumeRender(POVMS_Message& msg, POVMS_Message& result, int mode);

		void ReadFile(POVMS_Message& msg, POVMS_Message& result, int mode);
	private:
		static POVMSContext context;

		bool (*validateFrontendAddress)(POVMSAddress);

		SceneId scenecounter;
		ViewId viewcounter;

		map<SceneId, shared_ptr<Scene> > scenes;
		map<ViewId, shared_ptr<View> > views;
		map<SceneId, set<ViewId> > scene2views;
		map<ViewId, SceneId> view2scene;

		bool ValidateFrontendAddress(POVMSAddress addr, POVMS_Message& result);

		void MakeFailedResult(int error, POVMS_Message& result);
		void MakeFailedResult(const pov_base::Exception& e, POVMS_Message& result);
		void MakeFailedResult(const char *str, POVMS_Message& result);
		void MakeDoneResult(POVMS_Message& result);
};

}

#endif // POVRAY_BACKEND_RENDERBACKEND_H
