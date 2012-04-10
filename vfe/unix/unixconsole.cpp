/*******************************************************************************
 * unixconsole.cpp
 *
 * Adapted from vfe/win/console/winconsole.cpp
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
 * $File: //depot/povray/smp/vfe/unix/unixconsole.cpp $
 * $Revision: #47 $
 * $Change: 5460 $
 * $DateTime: 2011/07/15 04:26:22 $
 * $Author: jgrimbert $
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

#include <signal.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <boost/shared_ptr.hpp>

#include "vfe.h"
#include "backend/control/benchmark.h"
#include "povray.h"
#include "disp.h"
#include "disp_text.h"
#include "disp_sdl.h"

namespace pov_frontend
{
	boost::shared_ptr<Display> gDisplay;
}

using namespace vfe;
using namespace vfePlatform;

enum DispMode
{
	DISP_MODE_NONE,
	DISP_MODE_TEXT,
	DISP_MODE_SDL
};

DispMode gDisplayMode;

enum ReturnValue
{
	RETURN_OK=0,
	RETURN_ERROR,
	RETURN_USER_ABORT
};

bool gCancelRender = false;

// for handling asynchronous (external) signals
int gSignalNumber = 0;
boost::mutex gSignalMutex;


void SignalHandler (void)
{
	sigset_t sigset;
	int      signum;

	while(true)
	{
		sigfillset(&sigset);
		sigwait(&sigset, &signum);  // wait till a signal is caught
		boost::mutex::scoped_lock lock(gSignalMutex);
		gSignalNumber = signum;
	}
}


void ProcessSignal (void)
{
	boost::mutex::scoped_lock lock(gSignalMutex);

	switch (gSignalNumber)
	{
		case  0:
			break;
#ifdef SIGQUIT
		case SIGQUIT:
			fprintf(stderr, "\n%s: received signal SIGQUIT: Quit; requested render cancel\n", PACKAGE);
			gCancelRender = true;
			break;
#endif
#ifdef SIGTERM
		case SIGTERM:
			fprintf(stderr, "\n%s: received signal SIGTERM: Termination; requested render cancel\n", PACKAGE);
			gCancelRender = true;
			break;
#endif
#ifdef SIGINT
		case SIGINT:
			fprintf(stderr, "\n%s: received signal SIGINT: Interrupt; requested render cancel\n", PACKAGE);
			gCancelRender = true;
			break;
#endif
#ifdef SIGPIPE
		case SIGPIPE:
			fprintf(stderr, "\n%s: received signal SIGPIPE: Broken pipe; requested render cancel\n", PACKAGE);
			gCancelRender = true;
			break;
#endif
		case SIGCHLD:
			// for now, ignore this (side-effect of the shell-out code).
			// once properly implemented, the shell-out code would want to know this has happened, though.
			break;

		default:
			// fprintf(stderr, "\n%s: received signal %d\n", PACKAGE, gSignalNumber);
			break;
	}
	gSignalNumber = 0;
}

vfeDisplay *UnixDisplayCreator (unsigned int width, unsigned int height, GammaCurvePtr gamma, vfeSession *session, bool visible)
{
	UnixDisplay *display = GetRenderWindow () ;
	switch (gDisplayMode)
	{
#ifdef HAVE_LIBSDL
		case DISP_MODE_SDL:
			if (display != NULL && display->GetWidth() == width && display->GetHeight() == height)
			{
				UnixDisplay *p = new UnixSDLDisplay (width, height, gamma, session, false) ;
				if (p->TakeOver (display))
					return p;
				delete p;
			}
			return new UnixSDLDisplay (width, height, gamma, session, visible) ;
			break;
#endif
		case DISP_MODE_TEXT:
			return new UnixTextDisplay (width, height, gamma, session, visible) ;
			break;
		default:
			return NULL;
	}
}

void PrintStatus (vfeSession *session)
{
	string str;
	vfeSession::MessageType type;
	static vfeSession::MessageType lastType = vfeSession::mUnclassified;

	while (session->GetNextCombinedMessage (type, str))
	{
		if (type != vfeSession::mGenericStatus)
		{
			if (lastType == vfeSession::mGenericStatus)
				fprintf (stderr, "\n") ;
			fprintf (stderr, "%s\n", str.c_str());
		}
		else
			fprintf (stderr, "%s\r", str.c_str());
		lastType = type;
	}
}

void PrintStatusChanged (vfeSession *session, State force = kUnknown)
{
	if (force == kUnknown)
		force = session->GetBackendState();
	switch (force)
	{
		case kParsing:
			fprintf (stderr, "==== [Parsing...] ==========================================================\n");
			break;
		case kRendering:
			fprintf (stderr, "==== [Rendering...] ========================================================\n");
			break;
		case kPausedRendering:
			fprintf (stderr, "==== [Paused] ==============================================================\n");
			break;
	}
}

void PrintVersion(void)
{
	fprintf(stderr,
		"%s %s\n\n"
		"%s\n%s\n%s\n"
		"%s\n\n",
		PACKAGE_NAME, POV_RAY_VERSION,
		DISTRIBUTION_MESSAGE_1, DISTRIBUTION_MESSAGE_2, DISTRIBUTION_MESSAGE_3,
		POV_RAY_COPYRIGHT
	);
	fprintf(stderr,
		"Built-in features:\n"
		"  I/O restrictions:          %s\n"
		"  X Window display:          %s\n"
		"  Supported image formats:   %s\n"
		"  Unsupported image formats: %s\n\n",
		BUILTIN_IO_RESTRICTIONS, BUILTIN_XWIN_DISPLAY, BUILTIN_IMG_FORMATS, MISSING_IMG_FORMATS
	);
	fprintf(stderr,
		"Compilation settings:\n"
		"  Build architecture:  %s\n"
		"  Built/Optimized for: %s\n"
		"  Compiler vendor:     %s\n"
		"  Compiler version:    %s\n"
		"  Compiler flags:      %s\n",
		BUILD_ARCH, BUILT_FOR, COMPILER_VENDOR, COMPILER_VERSION, CXXFLAGS
	);
}

void ErrorExit(vfeSession *session)
{
	fprintf(stderr, "%s\n", session->GetErrorString());
	session->Shutdown();
	delete session;
	exit(RETURN_ERROR);
}

void CancelRender(vfeSession *session)
{
	session->CancelRender();  // request the backend to cancel
	PrintStatus (session);
	while (session->GetBackendState() != kReady)  // wait for the render to effectively shut down
		Delay(10);
	PrintStatus (session);
}

void PauseWhenDone(vfeSession *session)
{
	GetRenderWindow()->UpdateScreen(true);
	GetRenderWindow()->PauseWhenDoneNotifyStart();
	while (GetRenderWindow()->PauseWhenDoneResumeIsRequested() == false)
	{
		ProcessSignal();
		if (gCancelRender)
			break;
		else
			Delay(10);
	}
	GetRenderWindow()->PauseWhenDoneNotifyEnd();
}

ReturnValue PrepareBenchmark(vfeSession *session, vfeRenderOptions& opts, string& ini, string& pov, int argc, char **argv)
{
	// parse command-line options
	while (*++argv)
	{
		string s = string(*argv);
		boost::to_lower(s);
		// set number of threads to run the benchmark
		if (boost::starts_with(s, "+wt") || boost::starts_with(s, "-wt"))
		{
			s.erase(0, 3);
			int n = atoi(s.c_str());
			if (n)
				opts.SetThreadCount(n);
			else
				fprintf(stderr, "%s: ignoring malformed '%s' command-line option\n", PACKAGE, *argv);
		}
		// add library path
		else if (boost::starts_with(s, "+l") || boost::starts_with(s, "-l"))
		{
			s.erase(0, 2);
			opts.AddLibraryPath(s);
		}
	}

	int benchversion = pov::Get_Benchmark_Version();
	fprintf(stderr, "\
%s %s%s\n\n\
Entering the standard POV-Ray %s benchmark version %x.%02x.\n\n\
This built-in benchmark requires POV-Ray to be installed on your system\n\
before running it.  There will be neither display nor file output, and\n\
any additional command-line option except setting the number of render\n\
threads (+wtN for N threads) and library paths (+Lpath) will be ignored.\n\
To get an accurate benchmark result you might consider running POV-Ray\n\
with the Unix 'time' command (e.g. 'time povray -benchmark').\n\n\
The benchmark will run using %d render thread(s).\n\
Press <Enter> to continue or <Ctrl-C> to abort.\n\
",
		PACKAGE_NAME, POV_RAY_VERSION, COMPILER_VER,
		VERSION_BASE, benchversion / 256, benchversion % 256,
		opts.GetThreadCount()
	);

	// wait for user input from stdin (including abort signals)
	while (true)
	{
		ProcessSignal();
		if (gCancelRender)
		{
			fprintf(stderr, "Render cancelled by user\n");
			return RETURN_USER_ABORT;
		}

		fd_set readset;
		struct timeval tv = {0,0};  // no timeout
		FD_ZERO(&readset);
		FD_SET(STDIN_FILENO, &readset);
		if (select(STDIN_FILENO+1, &readset, NULL, NULL, &tv) < 0)
			break;
		if (FD_ISSET(STDIN_FILENO, &readset))  // user input is available
		{
			char s[3];
			read(STDIN_FILENO, s, 1);         // read till <ENTER> is hit
			tcflush(STDIN_FILENO, TCIFLUSH);  // discard unread data
			break;
		}
		Delay(20);
	}

	string basename = UCS2toASCIIString(session->CreateTemporaryFile());
	ini = basename + ".ini";
	pov = basename + ".pov";
	if (pov::Write_Benchmark_File(pov.c_str(), ini.c_str()))
	{
		fprintf(stderr, "%s: creating %s\n", PACKAGE, ini.c_str());
		fprintf(stderr, "%s: creating %s\n", PACKAGE, pov.c_str());
		fprintf(stderr, "Running standard POV-Ray benchmark version %x.%02x\n", benchversion / 256, benchversion % 256);
	}
	else
	{
		fprintf(stderr, "%s: failed to write temporary files for benchmark\n", PACKAGE);
		return RETURN_ERROR;
	}

	return RETURN_OK;
}

void CleanupBenchmark(vfeUnixSession *session, string& ini, string& pov)
{
	fprintf(stderr, "%s: removing %s\n", PACKAGE, ini.c_str());
	session->DeleteTemporaryFile(ASCIItoUCS2String(ini.c_str()));
	fprintf(stderr, "%s: removing %s\n", PACKAGE, pov.c_str());
	session->DeleteTemporaryFile(ASCIItoUCS2String(pov.c_str()));
}

int main (int argc, char **argv)
{
	vfeUnixSession   *session;
	vfeStatusFlags    flags;
	vfeRenderOptions  opts;
	ReturnValue       retval = RETURN_OK;
	bool              running_benchmark = false;
	string            bench_ini_name;
	string            bench_pov_name;
	sigset_t          sigset;
	boost::thread    *sigthread;
	char **           argv_copy=argv; /* because argv is updated later */
	int               argc_copy=argc; /* because it might also be updated */

	fprintf(stderr, "%s: This is a RELEASE CANDIDATE version of POV-Ray. General distribution is discouraged.\n", PACKAGE);

	// block some signals for this thread as well as those created afterwards
	sigemptyset(&sigset);

#ifdef SIGQUIT
	sigaddset(&sigset, SIGQUIT);
#endif
#ifdef SIGTERM
	sigaddset(&sigset, SIGTERM);
#endif
#ifdef SIGINT
	sigaddset(&sigset, SIGINT);
#endif
#ifdef SIGPIPE
	sigaddset(&sigset, SIGPIPE);
#endif
#ifdef SIGCHLD
	sigaddset(&sigset, SIGCHLD);
#endif

	pthread_sigmask(SIG_BLOCK, &sigset, NULL);

	// create the signal handling thread
	sigthread = new boost::thread(SignalHandler);

	session = new vfeUnixSession();
	if (session->Initialize(NULL, NULL) != vfeNoError)
		ErrorExit(session);

	// display mode registration
#ifdef HAVE_LIBSDL
	if (UnixSDLDisplay::Register(session))
		gDisplayMode = DISP_MODE_SDL;
	else
#endif
	if (UnixTextDisplay::Register(session))
		gDisplayMode = DISP_MODE_TEXT;
	else
		gDisplayMode = DISP_MODE_NONE;

	// default number of work threads: number of CPUs or 4
	int nthreads = 1;
#ifdef _SC_NPROCESSORS_ONLN  // online processors
	nthreads = sysconf(_SC_NPROCESSORS_ONLN);
#endif
#ifdef _SC_NPROCESSORS_CONF  // configured processors
	if (nthreads < 2)
		nthreads = sysconf(_SC_NPROCESSORS_CONF);
#endif
	if (nthreads < 2)
		nthreads = 4;
	opts.SetThreadCount(nthreads);

	// process command-line options
	session->GetUnixOptions()->ProcessOptions(&argc, &argv);
	if (session->GetUnixOptions()->isOptionSet("general", "help"))
	{
		session->Shutdown() ;
		PrintStatus (session) ;
		// TODO: general usage display (not yet in core code)
		session->GetUnixOptions()->PrintOptions();
		delete sigthread;
		delete session;
		return RETURN_OK;
	}
	else if (session->GetUnixOptions()->isOptionSet("general", "version"))
	{
		session->Shutdown() ;
		PrintVersion();
		delete sigthread;
		delete session;
		return RETURN_OK;
	}
	else if (session->GetUnixOptions()->isOptionSet("general", "benchmark"))
	{
		retval = PrepareBenchmark(session, opts, bench_ini_name, bench_pov_name, argc, argv);
		if (retval == RETURN_OK)
			running_benchmark = true;
		else
		{
			session->Shutdown();
			delete sigthread;
			delete session;
			return retval;
		}
	}

	// process INI settings
	if (running_benchmark)
	{
		// read only the provided INI file and set minimal lib paths
		opts.AddLibraryPath(string(POVLIBDIR "/include"));
		opts.AddINI(bench_ini_name.c_str());
		opts.SetSourceFile(bench_pov_name.c_str());
	}
	else
	{
		char *s = getenv ("POVINC");
		session->SetDisplayCreator(UnixDisplayCreator);
		session->GetUnixOptions()->Process_povray_ini(opts);
		if (s != NULL)
			opts.AddLibraryPath (s);
		while (*++argv)
			opts.AddCommand (*argv);
	}

	// set all options and start rendering
	if (session->SetOptions(opts) != vfeNoError)
	{
		fprintf(stderr,"\nProblem with option setting\n");
		for(int loony=0;loony<argc_copy;loony++)
		{
      fprintf(stderr,"%s%c",argv_copy[loony],loony+1<argc_copy?' ':'\n');
		}
		ErrorExit(session);
	}
	if (session->StartRender() != vfeNoError)
		ErrorExit(session);

	// set inter-frame pause for animation
	if (session->RenderingAnimation() && session->GetBoolOption("Pause_When_Done", false))
		session->PauseWhenDone(true);

	// main render loop
	session->SetEventMask(stBackendStateChanged);  // immediatly notify this event
	while (((flags = session->GetStatus(true, 200)) & stRenderShutdown) == 0)
	{
		ProcessSignal();
		if (gCancelRender)
		{
			CancelRender(session);
			break;
		}

		if (flags & stAnimationStatus)
			fprintf(stderr, "\nRendering frame %d of %d\n", session->GetCurrentFrame(), session->GetTotalFrames());
		if (flags & stAnyMessage)
			PrintStatus (session);
		if (flags & stBackendStateChanged)
			PrintStatusChanged (session);

		if (GetRenderWindow() != NULL)
		{
			// early exit
			if (GetRenderWindow()->HandleEvents())
			{
				gCancelRender = true;  // will set proper return value
				CancelRender(session);
				break;
			}

			GetRenderWindow()->UpdateScreen();

			// inter-frame pause
			if (session->GetCurrentFrame() < session->GetTotalFrames()
			&& session->GetPauseWhenDone()
			&& (flags & stAnimationFrameCompleted) != 0
			&& session->Failed() == false)
			{
				PauseWhenDone(session);
				if (! gCancelRender)
					session->Resume();
			}
		}
	}

	// pause when done for single or last frame of an animation
	if (session->Failed() == false && GetRenderWindow() != NULL && session->GetBoolOption("Pause_When_Done", false))
	{
		PrintStatusChanged(session, kPausedRendering);
		PauseWhenDone(session);
		gCancelRender = false;
	}

	if (running_benchmark)
		CleanupBenchmark(session, bench_ini_name, bench_pov_name);

	if (session->Succeeded() == false)
		retval = gCancelRender ? RETURN_USER_ABORT : RETURN_ERROR;
	session->Shutdown();
	PrintStatus (session);
	delete sigthread;
	delete session;

	return retval;
}
