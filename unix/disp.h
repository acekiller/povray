/*******************************************************************************
 * disp.h
 *
 * Written by Christoph Hormann <chris_hormann@gmx.de>
 *
 * Abstract base class for Unix systems render preview displays
 * Based on Windows pvdisplay.h.
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 2005-2009 Persistence of Vision Raytracer Pty. Ltd.
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
 * $File: //depot/povray/smp/unix/disp.h $
 * $Revision: #9 $
 * $Change: 5232 $
 * $DateTime: 2010/12/04 14:24:46 $
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

#ifndef _DISP_H
#define _DISP_H

#include "vfe.h"

#include <boost/shared_ptr.hpp>

namespace pov_frontend
{
	using namespace std;
	using namespace vfe;

	extern boost::shared_ptr<Display> gDisplay;

	class UnixDisplay : public vfeDisplay
	{
		public:
			UnixDisplay(unsigned int w, unsigned int h, GammaCurvePtr gamma, vfeSession *session, bool visible) :
				vfeDisplay(w, h, gamma, session, visible) {};
			virtual ~UnixDisplay() {} ;
			virtual void Initialise() = 0;
			virtual void Close() = 0;
			virtual void Show() = 0;
			virtual void Hide() = 0;
			virtual bool TakeOver(UnixDisplay *display) = 0;

			/**
				 To read all pending events in an interfactive display system
				 and interpret them.

				 @returns true if an abort request has been made, false otherwise
			 */
			virtual bool HandleEvents() = 0;

			/**
				 Called regularly by the main event loop.

				 @param Force  Indicates the Update should be immediate
				 otherwise the display does not actually have to be updated
				 during every call.
			 */
			virtual void UpdateScreen(bool Force = false) = 0;

			/**
				 The following methods are sequentially called when pausing after
				 a frame has been rendered, e.g. waiting for a keypress to continue.
				 The first can be used to print a message that the display will wait
				 for user input.  The second must poll (with no wait) for user input 
				 and returns whether the pause is to be resumed; the method is called
				 within a loop by the main thread.  The last method allows to notify
				 that the pause is finished.
			*/
			virtual void PauseWhenDoneNotifyStart() = 0;
			virtual bool PauseWhenDoneResumeIsRequested() = 0;
			virtual void PauseWhenDoneNotifyEnd() = 0;
	};

	inline UnixDisplay *GetRenderWindow (void)
	{
		Display *p = gDisplay.get();
		if (p == NULL)
			return NULL;
		return dynamic_cast<UnixDisplay *>(p) ;
	}
}

#endif
