/*******************************************************************************
 * unixoptions.h
 *
 * Written by Christoph Hormann <chris_hormann@gmx.de>
 * based on unix.cpp Elements by Nicolas Calimet
 *
 * Processing system for options in povray.conf, command line
 * and environment variables.
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
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
 * $File: //depot/povray/smp/vfe/unix/unixoptions.h $
 * $Revision: #8 $
 * $Change: 5325 $
 * $DateTime: 2011/01/01 08:06:05 $
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

#ifndef _OPTIONS_H
#define _OPTIONS_H

#include "vfe.h"

#include <list>
#include <vector>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>

using boost::to_lower_copy;

/*
 * [NC]
 * Default values for the location of the povray library and configuration.
 * These constants don't have to be in config.h .
 */
#ifndef POVLIBDIR
# define POVLIBDIR  "/usr/local/share/" PACKAGE "-" VERSION_BASE
#endif

#ifndef POVCONFDIR
# define POVCONFDIR  "/usr/local/etc/" PACKAGE "/" VERSION_BASE
#endif

#ifndef POVCONFDIR_BACKWARD
# define POVCONFDIR_BACKWARD  "/usr/local/etc"
#endif


namespace vfePlatform
{
	using namespace std;

	/**
		@brief Processing system for options in povray.conf, command line and environment variables.

		@author Christoph Hormann <chris_hormann@gmx.de>

		@date June 2006

		This class wraps the processing of platform specific options set via

			- povray.conf files in the form:

				[section]
				name=value

			- command line options (which have to be removed before the options vector 
				is passed to the core code for further processing)

			- environment variables

		It provides access to the settings made with those options to other parts of 
		the code and allows subsystems like the display classes to register their own
		options.

		Registering custom options:

			even options not registered are read from povray.conf when present and 
			their values are made available.  To register an options the public method
			Register() is provided.  It takes a vector of Option_Info structs containing
			the option information.  See disp_text.cpp for an (empty) example.

		The option values are currently stored as strings and the system does not 
		distinguish between different types.

		Furthermore it processes the IO-restrictions settings and manages the povray.ini
		locations.

		Inner workings:

			The options are stored in the list m_user_options.

			The constructor sets up the locations of the configuration files
			(previously unix_create_globals()), adds the standard options to the list
			and processes the povray.conf files.  The io-restrictions settings from 
			those files are read with the old proven system and stored in their own 
			list (m_permitted_paths).  At the end these are transferred into the 
			corresponding vfeSession fields.  All unknown options in the povray.conf 
			files are read into a temporary list (m_custom_conf_options).

			The actual options processing is done in ProcessOptions(). There all
			options from the povray.conf files are transferred into the list.  Then
			the values of all options which can be set via command line and environment 
			variables are determined.  Environment variables override povray.conf 
			settings and command line options override environment variables.  The read
			command line options are removed from the argument list like previously in 
			the xwindow code.

		An instance of this class is part of vfeUnixSession and can be accesses through
		vfeUnixSession::GetUnixOptions().

	*/
	class UnixOptionsProcessor
	{
	  public:
		/*
		 * [NC] structures to handle configuration-related (povray.conf) code.
		 */
		enum FileIO {IO_UNSET, IO_NONE, IO_READONLY, IO_RESTRICTED, IO_UNKNOWN};
		enum ShellOut {SHL_UNSET, SHL_ALLOWED, SHL_FORBIDDEN, SHL_UNKNOWN};

		/// permission path for IO restrictions settings
		struct UnixPath
		{
			string str;
			bool descend, writable;

			UnixPath(const string &s, bool desc = false, bool wrt = false) : str(s), descend(desc), writable(wrt) { }
		};

		/**
			 Option of a povray.conf file of the form
			 [Section]
			 Name=Value
		*/
		struct Conf_Option
		{
			string Section;
			string Name;
			string Value;

			Conf_Option(const string &Sect, const string &Nm, const string &Val = "") : Section(Sect), Name(Nm), Value(Val) { }
		};

		/**
			 A platform specific configuration option
			 with configuration file settings, 
			 Command line option (optional) and 
			 Environment variable (optional)

			 This stucture is used by the Display classes to 
			 provide their own options and by the options 
			 processor to store the option values
		*/
		struct Option_Info
		{
			string Section;
			string Name;
			string Value;
			string CmdOption;
			string EnvVariable;
			string Comment;
			bool has_param;

			Option_Info(const string &Sect ,
									const string &Nm, 
									const string &Val = "", 
									bool par = false, 
									const string &Cmd = "", 
									const string &Env = "", 
									const string &Comm = "")
				: Section(Sect), Name(Nm), Value(Val), has_param(par), CmdOption(Cmd), EnvVariable(Env), Comment(Comm) { }

			/// only checks identity of the option, not of the selected value.
			bool operator==(Option_Info const& other) const
			{
				return to_lower_copy(Section) == to_lower_copy(other.Section) && to_lower_copy(Name) == to_lower_copy(other.Name);
			}

			/// compares sections (for sorting)
			bool operator<(Option_Info const& other) const
			{
				if (to_lower_copy(other.Section) < to_lower_copy(Section))
					return true;
				if (to_lower_copy(other.Section) == to_lower_copy(Section))
					if (to_lower_copy(other.Name) < to_lower_copy(Name))
						return true;
				return false;
			}
		};

		UnixOptionsProcessor(vfeSession *session);
		virtual ~UnixOptionsProcessor() {} ;

		/**
			 called by the Display classes to register their custom options

			 @param options Vector of Option_Info structs containing the options
		*/
		void Register(const Option_Info options[]);
 
		/**
			 Converts a file path to standard form replacing 
			 relative notations.
		*/
		string CanonicalizePath(const string &path);
 
		/**
			 Finds out the default location for temporary files.
			 Set by an option, alternatively '/tmp/'.

			 @returns temporary path including a trailing slash
		*/
		string GetTemporaryPath(void);

		/**
			 Finds the value of a certain option from
			 either configuration file, command line
			 or environment variable.

			 @param option The option to query

			 The value is returned in option.Value.
		*/
		void QueryOption(Option_Info &option);

		/**
			 Finds the value of a certain option via
			 section and option name and returns it as
			 a string.
		*/
		string QueryOptionString(const string &section, const string &name);

		/**
			 Finds the value of a certain option via
			 section and option name and returns it as
			 an int.  If the options value is not convertible 
			 to int dflt is returned instead.
		*/
		int QueryOptionInt(const string &section, const string &name, const int dflt = 0);

		/**
			 Finds the value of a certain option via
			 section and option name and returns it as
			 a float  If the options value is not convertible 
			 to float dflt is returned instead..
		*/
		float QueryOptionFloat(const string &section, const string &name, const float dflt = 0.0);

		/**
			 Check if a certain option has been set
			 this is the case if a parameter free option is present of
			 if the parameter is 'on', 'yes', 'true' or '1'.

			 @returns true if set, false otherwise
		*/
		bool isOptionSet(const Option_Info &option);
		bool isOptionSet(const string &section, const string &name);

		/**
			 Adds the custom povray.conf options with their values
			 to the main options list and reads the environment 
			 variables and command line options where advised.

			 The argument list is replaced with a version
			 with the custom options removed.
		*/
		void ProcessOptions(int *argc, char **argv[]);

		/**
			 Search for povray.ini at standard locations
			 and add it to opts.
		*/
		void Process_povray_ini(vfeRenderOptions &opts);

		/**
			 Print all currrently registered command line 
			 options in an option summary.
		*/
		void PrintOptions(void);

		/**
			 Determines if IO restrictions are enabled at compile
			 time and via povray.conf settings.

			 @param write if false and permissions are 'free read' returns false.
		*/
		bool isIORestrictionsEnabled(bool write);

		/**
			 Prints an approriate error message if IO restrictions
			 prevent access to a file.

			 @param fnm File not permitted to access
			 @param write If write acccess was requested
			 @param is_user_setting if denial was due to user setting				 
		*/
		void IORestrictionsError(const string &fnm, bool write, bool is_user_setting);

		bool ShelloutPermitted(const string& command, const string& parameters) const { return m_shellout == SHL_ALLOWED; }

	 protected:
		/// list of standard options
		static const Option_Info Standard_Options[];

		string unix_getcwd(void);
		string basename(const string &path);
		string dirname(const string &path);
		string unix_readlink(const string &path);
		string pre_process_conf_line(const string &input);
		void add_permitted_path(list<UnixPath> &paths, const string &input, const string &conf_name, unsigned long line_number);
		void parse_conf_file(std::istream &Stream, const string &conf_name, bool user_mode);
		void process_povray_conf(void);
		void remove_arg(int *argc, char *argv[], int index);
		bool file_exist(const string &name);

		vfeSession *m_Session;
		string  m_home;
		string  m_user_dir;
		string  m_sysconf;                // system conf filename
		string  m_userconf;               // user conf filename
		string  m_conf;                   // selected conf file
		string  m_sysini, m_sysini_old;   // system ini filename
		string  m_userini, m_userini_old; // user ini filename
		FileIO  m_file_io;
		ShellOut m_shellout;
		list<UnixPath> m_permitted_paths;
		list<Conf_Option> m_custom_conf_options;
		list<Option_Info> m_user_options;
	};
}

#endif
