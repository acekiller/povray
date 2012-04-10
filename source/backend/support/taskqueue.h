/*******************************************************************************
 * taskqueue.h
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
 * $File: //depot/povray/smp/source/backend/support/taskqueue.h $
 * $Revision: #16 $
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

#ifndef POVRAY_BACKEND_TASKQUEUE_H
#define POVRAY_BACKEND_TASKQUEUE_H

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include "base/povmscpp.h"
#include "backend/support/task.h"

namespace pov
{

using boost::recursive_mutex;

class TaskQueue
{
		class TaskEntry
		{
			public:
				enum EntryType
				{
					kTask,
					kSync,
					kMessage,
					kFunction,
				};

				TaskEntry(EntryType et) : entryType(et) { }
				TaskEntry(shared_ptr<Task> rt) : entryType(kTask), task(rt) { }
				TaskEntry(POVMS_Message& m) : entryType(kMessage), msg(m) { }
				TaskEntry(const boost::function1<void, TaskQueue&>& f) : entryType(kFunction), fn(f) { }
				~TaskEntry() { }

				shared_ptr<Task> GetTask() { return task; }
				POVMS_Message& GetMessage() { return msg; }
				boost::function1<void, TaskQueue&>& GetFunction() { return fn; }

				EntryType GetEntryType() { return entryType; }
			private:
				EntryType entryType;
				shared_ptr<Task> task;
				POVMS_Message msg;
				boost::function1<void, TaskQueue&> fn;
		};
	public:
		TaskQueue();
		~TaskQueue();

		void Stop();
		void Pause();
		void Resume();

		bool IsPaused();
		bool IsRunning();
		bool IsDone();
		bool Failed();

		int FailureCode(int defval = kNoError);

		Task::TaskData *AppendTask(Task *task);
		void AppendSync();
		void AppendMessage(POVMS_Message& msg);
		void AppendFunction(const boost::function1<void, TaskQueue&>& fn);

		bool Process();

		void Notify();
	private:
		/// queued task list
		std::queue<TaskEntry> queuedTasks;
		/// active task list
		list<TaskEntry> activeTasks;
		/// queue mutex
		boost::recursive_mutex queueMutex;
		/// failed code
		int failed;
		/// wait for data in queue or related operation to be processed
		boost::condition processCondition;
		/// not available
		TaskQueue(const TaskQueue&);

		/// not available
		TaskQueue& operator=(const TaskQueue&);
};

}

#endif // POVRAY_BACKEND_TASKQUEUE_H
