/*******************************************************************************
 * taskqueue.cpp
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
 * $File: //depot/povray/smp/source/backend/support/taskqueue.cpp $
 * $Revision: #26 $
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
#include "backend/support/task.h"
#include "backend/support/taskqueue.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

TaskQueue::TaskQueue() : failed(kNoError)
{
}

TaskQueue::~TaskQueue()
{
	Stop();
}

void TaskQueue::Stop()
{
	recursive_mutex::scoped_lock lock(queueMutex);

	// we pass through this list twice; the first time through only sets the cancel
	// flag, and the second time through waits for the threads to exit. if we only
	// the the one pass (and wait), the cancel flag for later threads would remain
	// unset whilst we wait for earlier threads to reach a point where they check
	// the status of the flag, which tends to cause the shutdown to take longer
	// (particularly with a large number of threads vs a small number of CPU's).
	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		i->GetTask()->RequestStop();
	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		i->GetTask()->Stop();

	activeTasks.clear();
	while(queuedTasks.empty() == false)
		queuedTasks.pop();

	Notify();
}

void TaskQueue::Pause()
{
	recursive_mutex::scoped_lock lock(queueMutex);

	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		i->GetTask()->Pause();
}

void TaskQueue::Resume()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		i->GetTask()->Resume();
}

bool TaskQueue::IsPaused()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	bool paused = false;

	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		paused = paused || i->GetTask()->IsPaused();

	return paused;
}

bool TaskQueue::IsRunning()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	bool running = !queuedTasks.empty();

	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		running = running || i->GetTask()->IsRunning();

	return running;
}

bool TaskQueue::IsDone()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	bool done = queuedTasks.empty();

	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end(); i++)
		done = done && i->GetTask()->IsDone();

	return done;
}

bool TaskQueue::Failed()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	return (failed != kNoError);
}

int TaskQueue::FailureCode(int defval)
{
	recursive_mutex::scoped_lock lock(queueMutex);

	if(failed == kNoError)
		return defval;
	else
		return failed;
}

Task::TaskData *TaskQueue::AppendTask(Task *task)
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	failed = false;

	queuedTasks.push(TaskEntry(shared_ptr<Task>(task)));

	Notify();

	return task->GetDataPtr();
}

void TaskQueue::AppendSync()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	queuedTasks.push(TaskEntry::kSync);

	Notify();
}

void TaskQueue::AppendMessage(POVMS_Message& msg)
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	queuedTasks.push(TaskEntry(msg));

	Notify();
}

void TaskQueue::AppendFunction(const boost::function1<void, TaskQueue&>& fn)
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	queuedTasks.push(TaskEntry(fn));

	Notify();
}

bool TaskQueue::Process()
{
	boost::recursive_mutex::scoped_lock lock(queueMutex);

	for(list<TaskEntry>::iterator i(activeTasks.begin()); i != activeTasks.end();)
	{
		if(failed == kNoError)
			failed = i->GetTask()->FailureCode();

		if(i->GetTask()->IsDone() == true)
		{
			list<TaskEntry>::iterator e(i);
			i++;
			activeTasks.erase(e);
		}
		else
			i++;
	}

	if(failed != kNoError)
	{
		Stop();
		return false;
	}

	if(queuedTasks.empty() == false)
	{
		switch(queuedTasks.front().GetEntryType())
		{
			case TaskEntry::kTask:
			{
				activeTasks.push_back(queuedTasks.front());
				queuedTasks.front().GetTask()->Start(boost::bind(&TaskQueue::Notify, this));
				queuedTasks.pop();
				break;
			}
			case TaskEntry::kSync:
			{
				if(activeTasks.empty() == true)
					queuedTasks.pop();
				else
					return false;
				break;
			}
			case TaskEntry::kMessage:
			{
				try { POVMS_SendMessage(queuedTasks.front().GetMessage()); } catch(pov_base::Exception&) { }
				queuedTasks.pop();
				break;
			}
			case TaskEntry::kFunction:
			{
				try { queuedTasks.front().GetFunction()(*this); } catch(pov_base::Exception&) { }
				queuedTasks.pop();
				break;
			}
		}
	}

	if(queuedTasks.empty() == true)
		processCondition.wait(lock);

	return (queuedTasks.empty() == false);
}

void TaskQueue::Notify()
{
	processCondition.notify_one();
}

}
