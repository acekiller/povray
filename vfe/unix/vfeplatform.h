/*******************************************************************************
 * vfeplatform.h
 *
 * Defines a *nix platform-specific session class derived from vfeSession.
 *
 * Based on vfe/win/vfeplatform.h by Christopher J. Cason
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
 * $File: //depot/povray/smp/vfe/unix/vfeplatform.h $
 * $Revision: #9 $
 * $Change: 5016 $
 * $DateTime: 2010/06/13 07:01:12 $
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

#ifndef __VFEPLATFORM_H__
#define __VFEPLATFORM_H__

#include <boost/shared_ptr.hpp>

namespace vfePlatform
{
	using namespace vfe;

	class UnixOptionsProcessor;

	class UnixShelloutProcessing: public ShelloutProcessing
	{
		public:
			UnixShelloutProcessing(POVMS_Object& opts, const string& scene, uint width, uint height);
			~UnixShelloutProcessing();

			virtual int ProcessID(void);
			virtual bool ShelloutsSupported(void) { return true; }

		protected:
			virtual bool ExecuteCommand(const string& cmd, const string& params);
			virtual bool KillCommand(int timeout, bool force = false);
			virtual bool CommandRunning(void);
			virtual int CollectCommand(string& output);
			virtual int CollectCommand(void);
			virtual bool CommandPermitted(const string& command, const string& parameters);

			bool m_ProcessRunning;
			string m_Command;
			string m_Params;
			unsigned long m_ExitCode;
			unsigned long m_LastError;
			unsigned long m_ProcessId;

		private:
			UnixShelloutProcessing();
	};

	///////////////////////////////////////////////////////////////////////
	// most of the methods in vfeUnixSession are derived from vfeSession.
	// see vfeSession for documentation for those cases.
	class vfeUnixSession : public vfeSession
	{
		public:
			vfeUnixSession(int id = 0);
			virtual ~vfeUnixSession() {}

			virtual UCS2String GetTemporaryPath(void) const;
			virtual UCS2String CreateTemporaryFile(void) const;
			virtual void DeleteTemporaryFile(const UCS2String& filename) const;
			virtual POV_LONG GetTimestamp(void) const ;
			virtual void NotifyCriticalError(const char *message, const char *file, int line);
			virtual int RequestNewOutputPath(int CallCount, const string& Reason, const UCS2String& OldPath, UCS2String& NewPath);
			virtual bool TestAccessAllowed(const Path& file, bool isWrite) const;
			virtual ShelloutProcessing *CreateShelloutProcessing(POVMS_Object& opts, const string& scene, uint width, uint height)
				{ return new UnixShelloutProcessing(opts, scene, width, height); }

			boost::shared_ptr<UnixOptionsProcessor> GetUnixOptions(void) { return m_OptionsProc; }

		protected:
			virtual void WorkerThreadStartup();
			virtual void WorkerThreadShutdown();

			///////////////////////////////////////////////////////////////////////
			// return true if the path component of file is equal to the path component
			// of path. will also return true if recursive is true and path is a parent
			// of file. does not support relative paths, and will convert UCS2 paths to
			// ASCII and perform case-insensitive comparisons.
			virtual bool TestPath(const Path& path, const Path& file, bool recursive) const;

			///////////////////////////////////////////////////////////////////////
			// perform case-sensitive UCS2 string comparison. does not take code-
			// page into account.
			virtual bool StrCompare (const UCS2String& lhs, const UCS2String& rhs) const;

			///////////////////////////////////////////////////////////////////////
			// used to detect wall clock changes to prevent GetTimeStamp()
			// returning a value less than that of a previous call during the
			// current session.
			mutable POV_LONG m_LastTimestamp;
			mutable POV_LONG m_TimestampOffset;

			// platform specific configuration options
			boost::shared_ptr<UnixOptionsProcessor> m_OptionsProc;
	} ;

	///////////////////////////////////////////////////////////////////////
	// return a number that uniquely identifies the calling thread amongst
	// all other running threads in the process (and preferably in the OS).
	POVMS_Sys_Thread_Type GetThreadId();
}

#endif
