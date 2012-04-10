/*******************************************************************************
* shelloutprocessing.h
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
* $File: //depot/povray/smp/source/frontend/shelloutprocessing.h $
* $Revision: #7 $
* $Change: 5095 $
* $DateTime: 2010/08/07 07:51:37 $
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

#ifndef POVRAY_FRONTEND_SHELLOUTPROCESSING_H
#define POVRAY_FRONTEND_SHELLOUTPROCESSING_H

#include <string>

#include "base/povmscpp.h"
#include "base/povmsgid.h"

namespace pov_frontend
{

class ShelloutProcessing;

class ShelloutAction
{
public:
	typedef enum
	{
		ignore = 'i',
		skipOne = 's',
		skipAll = 'a',
		quit = 'q',
		abort = 'u',
		fatal ='f'
	} Action;

	ShelloutAction(const ShelloutProcessing *sp, unsigned int attribID, POVMS_Object& opts);
	~ShelloutAction() {}

	Action ReturnAction(void) const { return returnAction; }
	bool IsSet(void) const { return isSet; }
	const string& RawCommand(void) const { return rawCommand; }
	const string& Command(void) const { return command; }
	const string& RawParameters(void) const { return rawParameters; }
	const string& Parameters(void) const { return parameters; }
	bool ReturnNegate(void) const { return returnNegate; }
	void ExpandParameters(const string& scene, const string& ofn, unsigned int w, unsigned int h, float clock, unsigned int frame);

private:
	bool          isSet;
	bool          returnNegate;
	string        command;
	string        rawCommand;
	string        parameters;
	string        rawParameters;
	Action        returnAction;

	ShelloutAction();
};

class ShelloutProcessing
{
	friend class ShelloutAction;

public:
	typedef boost::shared_ptr<ShelloutAction> ShelloutPtr;

	typedef enum
	{
		preScene,
		postScene,
		preFrame,
		postFrame,
		userAbort,
		fatalError,
		lastShelloutEvent
	} shelloutEvent;

	// we use strings rather than UCS2Strings for the scene name and parameters since the passed
	// parameters (via POVMS) are also strings.
	ShelloutProcessing(POVMS_Object& opts, const string& scene, unsigned int width, unsigned int height);

	// you should reap any processes here as needed, and forcefully terminate ones still running.
	~ShelloutProcessing();

	// true if a shellout command was specified for the given phase
	bool IsSet(shelloutEvent which) const { return shellouts[which]->IsSet(); }

	// retrieve details for each type of shellout.
	ShelloutAction::Action ReturnAction(shelloutEvent which) const { return shellouts[which]->ReturnAction(); }

	// return the command string as passed from the option parser; i.e. complete with parameters
	string RawCommand(shelloutEvent which) const { return shellouts[which]->RawCommand(); }

	// the command itself, separated from its parameters. quotes around the command will have been removed.
	string Command(shelloutEvent which) const { return shellouts[which]->Command(); }

	// the raw parameters after separation from the command. any quotes will remain in place.
	string RawParameters(shelloutEvent which) const { return shellouts[which]->RawParameters(); }

	// the parameters after expansion of terms; e.g. %s to scene name. SetOutputFile() and
	// SetFrameClock() (if relevant) must be called prior to calling this method.
	string Parameters(shelloutEvent which) const { return shellouts[which]->Parameters(); }

	// returns true if all frames should be skipped. if so, any subsequent calls for
	// pre-frame and post-frame actions will be ignored (and preferebly should not be
	// made). the post-scene action should always be called; internal logic will determine
	// if it will do anything.
	bool SkipAllFrames(void) const { return skipAllFrames; }

	// returns true if next frame should be skipped. if so, pre-frame and post-frame actions
	// will be ignored. the internal skip frame flag is only reset when the frame number is
	// updated. NB skip all frames does not imply skip next frame since they are handled differently.
	bool SkipNextFrame(void) const { return skipNextFrame; }

	// returns the exit code that POV-Ray should return at the end of the render.
	// 0 means normal exit, 1 means fatal error, and 2 means user abort.
	int ExitCode(void) const { return exitCode; }

	// returns a string representation of the exit code; e.g. 'user abort'
	string ExitDesc(void) const { return exitCode == 0 ? "SUCCESS" : exitCode == 2 ? "USER ABORT" : "FATAL ERROR"; }

	// returns true if the render should be halted.
	bool RenderCancelled(void) const { return cancelRender; }

	// return true if the pre-scene event has been seen
	bool HadPreScene(void) const { return hadPreScene; }

	// return true if the post-scene event has been seen
	bool HadPostScene(void) const { return hadPostScene; }

	// if there is no output file, it is not required to call SetOutputFile().
	// if there is an output file, this method must be called prior to the pre-scene
	// action with the value of the first output file, and prior to pre-frame for each
	// subsequent output file if the render is an animation.
	void SetOutputFile(const string& filename) { outputFile = filename; }

	// if the render is not an animation, there is no need to call SetFrameClock().
	// if it is an animation, it must be called prior to the pre-scene action, and
	// then prior to pre-frame for each frame of the animation. note that if an action
	// returns the 'skip next frame' option, the SkipNextFrame() method will continue
	// to return true until the frame number supplied via this method has changed.
	void SetFrameClock(unsigned int frame, float clock) { if (frameNo != frame) skipNextFrame = false; frameNo = frame; clockVal = clock; }

	// shutdown any currently-running shellouts. if force is true, force them to exit.
	// in either case, don't wait more than timeout seconds. return true if there are
	// no more processes running afterwards.
	bool KillShellouts(int timeout, bool force = false);

	// the message is constructed as per the documentation for the boost::format class.
	// the positional parameters are as follows:
	//   1: the event causing the cancel (as a string), e.g. "pre-scene"
	//   2: the POV-Ray return code (as an integer)
	//   3: the return code (as an upper-case string), e.g. "USER ABORT"
	//   4: the return code (as a lower-case string).
	//   5: the reason for the cancel (as a string), e.g. "generate a user abort"
	//   6: the command name that generated the cancel
	//   7: the command parameters (CAUTION: may contain escape codes)
	//   8: the command return code (as an integer)
	//   9: output text from the command, as returned by LastShelloutResult()
	virtual string GetCancelMessage(void);
	virtual void SetCancelMessage(const string& format) { cancelFormat.parse(format); }

	// the positional parameters are as follows:
	//   1: the event causing the skip (as a string), e.g. "pre-scene"
	//   2: the type of the skip (as a string); e.g. "skip frame 11" or "skip all remaining frames"
	//   3: the command name that generated the skip
	//   4: the command parameters (CAUTION: may contain escape codes)
	//   5: the command return code (as an integer)
	//   6: output text from the command, as returned by LastShelloutResult()
	virtual string GetSkipMessage(void);
	virtual void SetSkipMessage(const string& format) { skipFormat.parse(format); }

	// advise the code that a particular event should be handled now; e.g. pre-scene, post-scene
	// and so forth. this method should be called even if the platform indicates it does not
	// support shellouts; if a render defines one, the code will thrown a kCannotOpenFileErr
	// exception at the time (which should be handled by the caller). this method does not block
	// the caller during processing of shellouts as they are run in the background. the return
	// value indicates whether or not rendering should be cancelled as a result of a shellout:
	// generally this won't be known at the time of return, however it could be set early if
	// a shellout could not be started and the INI file indicated that render should be halted
	// on failure.
	virtual bool ProcessEvent(shelloutEvent event) { return HandleProcessEvent(event, false); }

	// returns true if a shellout is currently running. if this method is being called in an
	// event loop to wait until a shellout has completed, it is the responsibility of the event
	// loop to perform appropriate sleeps to avoid wasting CPU time. platforms that implement
	// shellout support MUST override this method to return an appropriate value by actually
	// checking the process each time it's called.
	virtual bool ShelloutRunning(void);

	// return the name of the currently running shellout (without parameters)
	// if no shellout is running, an empty string should be returned.
	virtual string ProcessName(void) { return ShelloutRunning() ? runningProcessName : string(); }

	// return the PID of the currently running shellout (or equivalent thereof).
	// returns 0 if no process is running, and -1 of the platform has no PID equivalent
	// or this method is not implemented.
	virtual int ProcessID(void) { return -1; }

	// return a descriptive string detailing the result of the last shellout command
	// in a form suitable for display on the console or UI message log - preferably
	// no more than a single line (width unimportant). if not implemented, an empty
	// string should be returned.
	virtual string LastShelloutResult(void) { return string(); }

	// return true if this platform supports shellouts.
	virtual bool ShelloutsSupported(void) { return false; }

protected:
	int exitCode;
	int cancelReturn;
	int skipReturn;
	bool skipAllFrames;
	bool skipNextFrame;
	bool cancelRender;
	bool skipCallouts;
	bool killRequested;
	bool hadPreScene;
	bool hadPostScene;
	bool hadUserAbort;
	bool hadFatalError;
	bool commandProhibited;
	unsigned int frameNo;
	unsigned int imageWidth;
	unsigned int imageHeight;
	float clockVal;
	string sceneName;
	string outputFile;
	string runningProcessName;
	string cancelPhase;
	string cancelReason;
	string cancelCommand;
	string cancelParameters;
	string cancelOutput;
	string skipPhase;
	string skipReason;
	string skipCommand;
	string skipParameters;
	string skipOutput;
	boost::format cancelFormat;
	boost::format skipFormat;
	ShelloutPtr shellouts[lastShelloutEvent];

	// helper method
	string GetPhaseName(shelloutEvent event);

	// execute the given command with the supplied parameters, which have already
	// been expanded as per the docs, and immediately return true without waiting
	// for completion of the process. if the command can't be run other than for
	// one of the reasons documented below, return false (in which case CollectCommand
	// should return -2 if called later on).
	//
	// if shellouts are not supported, or access to the executable is prohibited by
	// POV-Ray internal rules (not the OS), throw a kCannotOpenFile exception with an
	// appropriate message. you should also throw a (different) exception if a process
	// is still running. any exception thrown will cancel a render (including remaining
	// frames of an animation) and the fatal error shellout (if defined) will not be
	// called.
	//
	// you should reap any processes in your destructor in case CollectCommand doesn't
	// get called.
	//
	// if the platform implemeting a subclass of this method has the equivalent of a
	// system log (e.g. syslog on unix, event log on windows), the implementation should
	// consider providing a user-controllable option to log any commands using such.
	virtual bool ExecuteCommand(const string& cmd, const string& params);

	// shutdown any currently-running shellouts. if force is true, force them to exit.
	// in either case, don't wait more than timeout milliseconds. return true if there
	// are no more processes running afterwards.
	virtual bool KillCommand(int timeout, bool force = false) { return true; }

	// returns true if a shellout is currently running. if this method is being called in an
	// event loop to wait until a shellout has completed, it is the responsibility of the event
	// loop to perform appropriate sleeps to avoid wasting CPU time. platforms that implement
	// shellout support MUST override this method to return an appropriate value by actually
	// checking the process each time it's called.
	virtual bool CommandRunning(void) { return false; }

	// if no process is running or has already been reaped, return -2. if a process
	// is still running, return -1. if the process is complete, place the output into
	// output then return the process's exit code. if the process failed, it would help
	// if the output string included (or was only) stderr, but this is not a requirement.
	//
	// if the platform does not support capturing output of processes (or the
	// processes are GUI-based), there is no requirement to return any output.
	virtual int CollectCommand(string& output) { return -2; }
	virtual int CollectCommand(void) { return -2; }

	// return true if the requested shellout command is permitted. this method is
	// called just before a shellout runs. if it fails, an exception will generally
	// be thrown by the caller (the method itself should not throw an exception).
	virtual bool CommandPermitted(const string& command, const string& parameters) { return true; }

	// called by the internal parser during construction to separate commands from parameters.
	// given a raw string in the form returned from the POV INI file, extract the command and any parameters.
	// the default version of this method should suffice for most implementations, however some platforms
	// may need different treatment of quotes or escapes. for example, the windows platform may wish to
	// provide a special-case for strings that look like windows paths, and exempt them from escaping of
	// the backslash.
	//
	// the default method will trim the source and then search it for the first whitespace character
	// that is both outside of a quoted string and not escaped. this forms the boundary between the
	// command and the parameters (if not found, the entire source is considered the command). the code
	// accepts single and double quotes as acceptable delimiters. if quotes are found around the command,
	// they are removed. no other quotes (including any within the parameters) will be altered.
	//
	// when parsing the command portion of the string, any backslashes found are considered to be escapes
	// which remove the special meaning of the following character. the escape is removed and the next
	// character will not be subject to special interpretation (this affects single quote, double quote,
	// backslashes, and any whitespace characters. any escapes in the text after the point where the
	// parameters start will not be removed.
	//
	// this method should return true if the command is non-empty upon completion.
	virtual bool ExtractCommand(const string& src, string& command, string& parameters) const;

private:
	bool processStartRequested;
	shelloutEvent postProcessEvent;

	bool HandleProcessEvent(shelloutEvent which, bool internalCall);
	bool PostProcessEvent(void);
};

}

#endif // POVRAY_FRONTEND_SHELLOUTPROCESSING_H
