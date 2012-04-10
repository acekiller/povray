/*******************************************************************************
 * task.h
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
 * $File: //depot/povray/smp/source/backend/support/task.h $
 * $Revision: #32 $
 * $Change: 5277 $
 * $DateTime: 2010/12/20 12:24:04 $
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

#ifndef POVRAY_BACKEND_TASK_H
#define POVRAY_BACKEND_TASK_H

#include <vector>
#include <queue>

#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "backend/frame.h"

#include "base/timer.h"
#include "base/pov_err.h"

namespace pov
{

using namespace pov_base;

class Task;

class Task
{
		struct StopThreadException { };
	public:
		class TaskData
		{
				friend class Task;
			public:
				TaskData() { }
				virtual ~TaskData() { }
			private:
				Task *task;
		};

		Task(TaskData *td, const boost::function1<void, Exception&>& f);
		virtual ~Task();

		inline bool IsPaused() { return !done && paused; }
		inline bool IsRunning() { return !done && !paused && !stopRequested && (taskThread != NULL); }
		inline bool IsDone() { return done; }
		inline bool Failed() { return done && (failed != kNoError); }

		int FailureCode(int defval = kNoError);

		POV_LONG ConsumedRealTime() const;
		POV_LONG ConsumedCPUTime() const;

		void Start(const boost::function0<void>& completion);
		void RequestStop();
		void Stop();
		void Pause();
		void Resume();

		inline void Cooperate()
		{
			if(stopRequested == true)
				throw StopThreadException();
			else if(paused == true)
			{
				while(paused == true)
				{
					boost::thread::yield();
					Delay(100);
					if(stopRequested == true)
						throw StopThreadException();
				}
			}
		}

		inline static void CurrentTaskCooperate()
		{
			TaskData *td = Task::GetTLSDataPtr();

			if((td != NULL) && (td->task != NULL))
				td->task->Cooperate();
		}

		inline static TaskData *GetTLSDataPtr() { return GET_THREAD_LOCAL_PTR(taskDataPtr); }

		inline TaskData *GetDataPtr() { return taskData; }

		inline POVMSContext GetPOVMSContext() { return povmsContext; }
	protected:
		virtual void Run() = 0;
		virtual void Stopped() = 0;
		virtual void Finish() = 0;

		POV_LONG ElapsedRealTime() const;
		POV_LONG ElapsedCPUTime() const;
	private:
		/// task data pointer
		TaskData *taskData;
		/// task data pointer - TLS version
		static DECLARE_THREAD_LOCAL_PTR(TaskData, taskDataPtr);
		/// task fatal error handler
		boost::function1<void, Exception&> fatalErrorHandler;
		/// stop request flag
		volatile bool stopRequested;
		/// paused flag
		volatile bool paused;
		/// done flag
		volatile bool done;
		/// failed code
		volatile int failed;
		// pointer to timer or NULL
		Timer *timer;
		// real time spend in task
		POV_LONG realTime;
		// CPU time spend in task
		POV_LONG cpuTime;
		/// task thread
		boost::thread *taskThread;
		/// POVMS message receiving context
		POVMSContext povmsContext;

		inline void FatalErrorHandler(const Exception& e)
		{
			Exception pe(e);
			fatalErrorHandler(pe);
		}

		inline void FatalErrorHandler(Exception& e) { fatalErrorHandler(e); }

		/// not available
		Task();

		/// not available
		Task(const Task&);

		/// not available
		Task& operator=(const Task&);

		void TaskThread(const boost::function0<void>& completion);

		static void TaskDataCleanup(Task::TaskData *) { }
};

}

#endif // POVRAY_BACKEND_TASK_H
