/*******************************************************************************
 * simplefrontend.h
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
 * $File: //depot/povray/smp/source/frontend/simplefrontend.h $
 * $Revision: #38 $
 * $Change: 5385 $
 * $DateTime: 2011/01/17 07:14:45 $
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

#ifndef POVRAY_FRONTEND_SIMPLEFRONTEND_H
#define POVRAY_FRONTEND_SIMPLEFRONTEND_H

#include "syspovconfigfrontend.h"

#include "frontend/renderfrontend.h"
#include "frontend/imageprocessing.h"
#include "frontend/animationprocessing.h"
#include "frontend/shelloutprocessing.h"

#include <boost/scoped_ptr.hpp>

namespace pov_frontend
{

using namespace pov_base;

enum State
{
	kUnknown,
	kReady,
	kStarting,
	kPreSceneShellout,
	kPreFrameShellout,
	kParsing,
	kPausedParsing,
	kRendering,
	kPausedRendering,
	kPostFrameShellout,
	kPostSceneShellout,
	kPostShelloutPause,
	kStopping,
	kStopped,
	kFailed,
	kDone
};

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
class SimpleFrontend
{
	public:
		SimpleFrontend(POVMSContext ctx, POVMSAddress addr, POVMS_Object& msg,
		               boost::function<Console *()> cfn,
		               boost::function<Display *(unsigned int, unsigned int, GammaCurvePtr)> dfn,
		               POVMS_Object *result = NULL, shared_ptr<Console> console = shared_ptr<Console>());
		~SimpleFrontend();

		bool Start(POVMS_Object& opts, shared_ptr<Image> img = shared_ptr<Image>());
		bool Stop();
		bool Pause();
		bool Resume();

		State Process();

		State GetState() const;

		shared_ptr<Console> GetConsole();
		shared_ptr<Image> GetImage();
		shared_ptr<Display> GetDisplay();
	private:
		RenderFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH> renderFrontend;
		POVMSAddress backendAddress;
		State state;
		POVMS_Object options;
		RenderFrontendBase::SceneId sceneId;
		RenderFrontendBase::ViewId viewId;
		shared_ptr<ImageProcessing> imageProcessing;
		shared_ptr<AnimationProcessing> animationProcessing;
		shared_ptr<ShelloutProcessing> shelloutProcessing;
		boost::function<Console *()> createConsole;
		boost::function<Display *(unsigned int, unsigned int, GammaCurvePtr)> createDisplay;
};

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::SimpleFrontend(POVMSContext ctx, POVMSAddress addr, POVMS_Object& msg,
                                                                        boost::function<Console *()> cfn,
                                                                        boost::function<Display *(unsigned int, unsigned int, GammaCurvePtr)> dfn,
                                                                        POVMS_Object *result, shared_ptr<Console> console) :
	renderFrontend(ctx),
	backendAddress(addr),
	state(kReady),
	createConsole(cfn),
	createDisplay(dfn)
{
	renderFrontend.ConnectToBackend(backendAddress, msg, result, console);
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::~SimpleFrontend()
{
	renderFrontend.DisconnectFromBackend(backendAddress);
	state = kUnknown;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
bool SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::Start(POVMS_Object& opts, shared_ptr<Image> img)
{
	int width;
	int height;

	if(state != kReady)
		return false;

	animationProcessing.reset();

	POVMS_List declares;
	if(opts.Exist(kPOVAttrib_Declare) == true)
		opts.Get(kPOVAttrib_Declare, declares);

	POVMS_Object image_width(kPOVMSType_WildCard);
	image_width.SetString(kPOVAttrib_Identifier, "image_width");
	image_width.SetFloat(kPOVAttrib_Value, width = opts.TryGetInt(kPOVAttrib_Width, 160));
	declares.Append(image_width);

	POVMS_Object image_height(kPOVMSType_WildCard);
	image_height.SetString(kPOVAttrib_Identifier, "image_height");
	image_height.SetFloat(kPOVAttrib_Value, height = opts.TryGetInt(kPOVAttrib_Height, 120));
	declares.Append(image_height);

	opts.Set(kPOVAttrib_Declare, declares);

	if(opts.TryGetInt(kPOVAttrib_FinalFrame, 0) > 0)
		animationProcessing = shared_ptr<AnimationProcessing>(new AnimationProcessing(opts));

	options = opts;

	if(opts.TryGetBool(kPOVAttrib_OutputToFile, true))
	{
		if(img != NULL)
			imageProcessing = shared_ptr<ImageProcessing>(new ImageProcessing(img));
		else
			imageProcessing = shared_ptr<ImageProcessing>(new ImageProcessing(options));
	}

	Path ip (opts.TryGetString(kPOVAttrib_InputFile, ""));
	shelloutProcessing.reset(new ShelloutProcessing(opts, UCS2toASCIIString(ip.GetFile()), width, height));

	state = kStarting;

	return true;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
bool SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::Stop()
{
	switch(state)
	{
		case kStarting:
			state = kStopped;
			return true;

		case kPreSceneShellout:
		case kPostFrameShellout:
		case kPostSceneShellout:
		case kPreFrameShellout:
			// TODO: add support for stopping shellouts, then halting render
			throw POV_EXCEPTION(kCannotHandleRequestErr, "Shellout code not active yet (how did we get here?)");

		case kParsing:
			renderFrontend.StopParser(sceneId);
			state = kStopping;
			return true;
		case kRendering:
			renderFrontend.StopRender(viewId);
			state = kStopping;
			return true;
	}

	return false;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
bool SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::Pause()
{
	switch(state)
	{
		case kPreSceneShellout:
		case kPreFrameShellout:
		case kPostFrameShellout:
			// TODO: if we are running a shellout, we should allow request of a pause, which will
			// take effect after the shellout returns (i.e. it won't pause the program being run
			// but will pause POV-Ray afterwards).
			throw POV_EXCEPTION(kCannotHandleRequestErr, "Shellout code not active yet (how did we get here?)");
			break;

		case kParsing:
			renderFrontend.PauseParser(sceneId);
			state = kPausedParsing;
			return true;

		case kRendering:
			renderFrontend.PauseRender(viewId);
			state = kPausedRendering;
			return true;
	}

	return false;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
bool SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::Resume()
{
	switch(state)
	{
		case kPausedParsing:
			renderFrontend.ResumeParser(sceneId);
			state = kParsing;
			return true;
		case kPausedRendering:
			renderFrontend.ResumeRender(viewId);
			state = kRendering;
			return true;

		case kPreSceneShellout:
		case kPreFrameShellout:
		case kPostFrameShellout:
			// TODO: clear any pause that was requested while a shellout was running
			// (would not have been acted on yet)
			throw POV_EXCEPTION(kCannotHandleRequestErr, "Shellout code not active yet (how did we get here?)");
	}

	return false;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
State SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::Process()
{
	switch(state)
	{
		case kReady:
			return kReady;
		case kStarting:
			try
			{
				if(animationProcessing != NULL)
				{
					options = animationProcessing->GetFrameRenderOptions();
					if (imageProcessing != NULL);
						options.SetUCS2String(kPOVAttrib_OutputFile, imageProcessing->GetOutputFilename(options, animationProcessing->GetFrameNumber(), animationProcessing->GetFrameNumberDigits()).c_str());
				}
				else
					if (imageProcessing != NULL);
						options.SetUCS2String(kPOVAttrib_OutputFile, imageProcessing->GetOutputFilename(options, 0, 0).c_str());
			}
			catch(pov_base::Exception&)
			{
				state = kFailed;
				// TODO - output failure message
				return kFailed;
			}

			try { sceneId = renderFrontend.CreateScene(backendAddress, options, createConsole); }
			catch(pov_base::Exception&)
			{
				state = kFailed;
				// TODO - output failure message
				return kFailed;
			}

			try { renderFrontend.StartParser(sceneId, options); }
			catch(pov_base::Exception&)
			{
				state = kFailed;
				// TODO - output failure message
				return kFailed;
			}

			state = kParsing;

			return kParsing;

		case kPreSceneShellout:
		case kPostFrameShellout:
		case kPostSceneShellout:
		case kPreFrameShellout:
			// TODO: add support for shellouts
			throw POV_EXCEPTION(kCannotHandleRequestErr, "Shellout code not active yet (how did we get here?)");

		case kParsing:
			switch(renderFrontend.GetSceneState(sceneId))
			{
				case SceneData::Scene_Failed:
					state = kStopped;
					return kStopped;
				case SceneData::Scene_Stopping:
					state = kStopping;
					return kStopping;
				case SceneData::Scene_Ready:
					try { viewId = renderFrontend.CreateView(sceneId, options, imageProcessing, createDisplay); }
					catch(pov_base::Exception&)
					{
						state = kFailed;
						// TODO - output failure message
						return kFailed;
					}

					try { renderFrontend.StartRender(viewId, options); }
					catch(pov_base::Exception& e)
					{
						if(e.codevalid() && (e.code() == kImageAlreadyRenderedErr)) // TODO FIXME - This can be done much simpler: Just do nothing!
						{
							// this is not a failure; continue has been requested and
							// the file has already been rendered, so we skip it.
							if ((animationProcessing != NULL) && (animationProcessing->MoreFrames() == true))
							{
								animationProcessing->ComputeNextFrame();
								state = kStarting;
								return kStarting;
							}
							else
							{
								state = kDone;
								return kDone;
							}
						}
						state = kFailed;
						// TODO - output failure message
						return kFailed;
					}

					state = kRendering;

					return kRendering;
			}

			return kParsing;
		case kRendering:
			switch(renderFrontend.GetViewState(viewId))
			{
				case ViewData::View_Failed:
					state = kStopped;
					return kStopped;
				case ViewData::View_Stopping:
					state = kStopping;
					return kStopping;
				case ViewData::View_Rendered:
					if (imageProcessing != NULL)
					{
						try
						{
							if(animationProcessing != NULL)
								imageProcessing->WriteImage(options, animationProcessing->GetFrameNumber(), animationProcessing->GetFrameNumberDigits());
							else
								imageProcessing->WriteImage(options);
						}
						catch(...)
						{
							state = kFailed;
							// TODO - output failure message
							return kFailed;
						}
					}

					if((animationProcessing != NULL) && (animationProcessing->MoreFrames() == true))
					{
						try { renderFrontend.CloseView(viewId); } catch(...) { } // Ignore any error here!
						try { renderFrontend.CloseScene(sceneId); } catch(...) { } // Ignore any error here!
						animationProcessing->ComputeNextFrame();
						state = kStarting;
						return kStarting;
					}
					else
					{
						state = kDone;
						return kDone;
					}
			}

			return kRendering;
		case kStopping:
			if(renderFrontend.GetSceneState(sceneId) == SceneData::Scene_Ready || renderFrontend.GetSceneState(sceneId) == SceneData::Scene_Failed)
			{
				state = kStopped;
				return kStopped;
			}
			else if(renderFrontend.GetViewState(viewId) == ViewData::View_Rendered)
			{
				state = kStopped;
				return kStopped;
			}
			return kRendering;
		case kStopped:
		case kFailed:
		case kDone:
			try { renderFrontend.CloseView(viewId); } catch(...) { } // Ignore any error here!
			try { renderFrontend.CloseScene(sceneId); } catch(...) { } // Ignore any error here!
			animationProcessing.reset();

			state = kReady;

			return kReady;
	}

	return state;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
State SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::GetState() const
{
	return state;
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
shared_ptr<Console> SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::GetConsole()
{
	return renderFrontend.GetConsole(sceneId);
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
shared_ptr<Image> SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::GetImage()
{
	return renderFrontend.GetImage(viewId);
}

template<class PARSER_MH, class FILE_MH, class RENDER_MH, class IMAGE_MH>
shared_ptr<Display> SimpleFrontend<PARSER_MH, FILE_MH, RENDER_MH, IMAGE_MH>::GetDisplay()
{
	return renderFrontend.GetDisplay(viewId);
}

}

#endif // POVRAY_FRONTEND_SIMPLEFRONTEND_H
