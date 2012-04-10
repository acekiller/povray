/*******************************************************************************
 * pov_err.h
 *
 * This module contains all error numbers.
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
 * $File: //depot/povray/smp/source/base/pov_err.h $
 * $Revision: #29 $
 * $Change: 5218 $
 * $DateTime: 2010/12/01 07:10:22 $
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

#ifndef POV_ERR_H
#define POV_ERR_H

#include "base/configbase.h"

namespace pov_base
{

// fatal errors
enum
{
	kNoError                  = 0,
	kNoErr                    = kNoError,
	kParamErr                 = -1,
	kMemFullErr               = -2,
	kOutOfMemoryErr           = kMemFullErr,
	kInvalidDataSizeErr       = -3,
	kCannotHandleDataErr      = -4,
	kNullPointerErr           = -5,
	kChecksumErr              = -6,
	kParseErr                 = -7,
	kCannotOpenFileErr        = -8,
	kInvalidDestAddrErr       = -9,
	kCannotConnectErr         = -10,
	kDisconnectedErr          = -11,
	kHostDisconnectedErr      = -12,
	kNetworkDataErr           = -13,
	kNetworkConnectionErr     = -14,
	kObjectAccessErr          = -15,
	kVersionErr               = -16,
	kFileDataErr              = -17,
	kAuthorisationErr         = -18,
	kDataTypeErr              = -19,
	kTimeoutErr               = -20,
	kInvalidContextErr        = -21,
	kIncompleteDataErr        = -22,
	kInvalidIdentifierErr     = -23,
	kCannotHandleRequestErr   = -24,
	kImageAlreadyRenderedErr  = -25,
	kAccessViolationErr       = -26,
	kDivideByZeroErr          = -27,
	kStackOverflowErr         = -28,
	kNativeExceptionErr       = -29,
	kInternalLimitErr         = -30,
	kUncategorizedError       = -31,
};

// non fatal errors
enum
{
	kFalseErr                 = 1,
	kOutOfSyncErr             = 2,
	kNotNowErr                = kOutOfSyncErr,
	kQueueFullErr             = 3,
	kUserAbortErr             = 4
};

/**
 *	Macro which builds an exception with file name, line and function.
 *	Requires an error message text string to be specified.
 */
#define POV_EXCEPTION_STRING(str) pov_base::Exception(__FUNCTION__, __FILE__, __LINE__, (str))

/**
 *	Macro which builds an exception with file name, line and function.
 *	Requires an error code to be specified.
 *	The string will be derived from the error code.
 */
#define POV_EXCEPTION_CODE(err) pov_base::Exception(__FUNCTION__, __FILE__, __LINE__, (err))

/**
 *	Macro which builds an exception with file name, line and function.
 *	Requires an error code and string to be specified.
 */
#define POV_EXCEPTION(err,str) pov_base::Exception(__FUNCTION__, __FILE__, __LINE__, (err), (str))

/**
 *  POV-Ray exception class.
 */
class Exception : public runtime_error
{
	public:
		/**
		 *	Create a new exception without location information.
		 *	@param	str			Error message string.
		 */
		Exception(const char *str) throw() : runtime_error(str), xfunction(NULL), xfile(NULL), xline(0), xcode(0), xcodevalid(false), xfrontendnotified(false) { }

		/**
		 *	Create a new exception with location information; looks up message from code.
		 *	@param	fn			__FUNCTION__ or NULL
		 *	@param	fi			__FILE__
		 *	@param	li			__LINE__
		 *	@param	err			Error number.
		 */
		Exception(const char *fn, const char *fi, unsigned int li, int err) throw() : runtime_error(Exception::lookup_code(err, fi, li)), xfunction(fn), xfile(fi), xline(li), xcode(err), xcodevalid(true), xfrontendnotified(false) { }

		/**
		 *	Create a new exception with code, location information, and an explicit message.
		 *	@param	fn			__FUNCTION__ or NULL
		 *	@param	fi			__FILE__
		 *	@param	li			__LINE__
		 *	@param	str			Error message string.
		 */
		Exception(const char *fn, const char *fi, unsigned int li, const char *str) throw() : runtime_error(str), xfunction(fn), xfile(fi), xline(li), xcode(0), xcodevalid(false), xfrontendnotified(false) { }

		/**
		 *	Create a new exception with code, location information, and an explicit message.
		 *	@param	fn			__FUNCTION__ or NULL
		 *	@param	fi			__FILE__
		 *	@param	li			__LINE__
		 *	@param	str			Error message string.
		 */
		Exception(const char *fn, const char *fi, unsigned int li, const string& str) throw() : runtime_error(str), xfunction(fn), xfile(fi), xline(li), xcode(0), xcodevalid(false), xfrontendnotified(false) { }

		/**
		 *	Create a new exception with code, location information, and an explicit message.
		 *	@param	fn			__FUNCTION__ or NULL
		 *	@param	fi			__FILE__
		 *	@param	li			__LINE__
		 *	@param	err			Error number.
		 *	@param	str			Error message string.
		 */
		Exception(const char *fn, const char *fi, unsigned int li, int err, const char *str) throw() : runtime_error(str), xfunction(fn), xfile(fi), xline(li), xcode(err), xcodevalid(true), xfrontendnotified(false) { }

		/**
		 *	Create a new exception with code, location information, and an explicit message.
		 *	@param	fn			__FUNCTION__ or NULL
		 *	@param	fi			__FILE__
		 *	@param	li			__LINE__
		 *	@param	err			Error number.
		 *	@param	str			Error message string.
		 */
		Exception(const char *fn, const char *fi, unsigned int li, int err, const string& str) throw() : runtime_error(str), xfunction(fn), xfile(fi), xline(li), xcode(err), xcodevalid(true), xfrontendnotified(false) { }

		/**
		 *	Destructor.
		 */
		virtual ~Exception() throw() { }

		/**
		 *	Determine the function name where the exception occured.
		 *	@return				Function name or NULL.
		 */
		const char *function() const { return xfunction; }

		/**
		 *	Determine the name of the source file where the exception occured.
		 *	@return				File name or NULL.
		 */
		const char *file() const { return xfile; }

		/**
		 *	Determine the line number in the source file where the exception occured.
		 *	@return				File line or 0.
		 */
		unsigned int line() const { return xline; }

		/**
		 *	Determine the error code.
		 *	@return				Error code (n.b. 0 is a legal errorcode)
		 */
		int code() const { return xcode; }

		/**
		 *	Determine the error code or default value if code not supplied at construction.
		 *	@return				Error code (n.b. 0 is a legal errorcode)
		 */
		int code(int defval) const { return xcodevalid ? xcode : defval; }

		/**
		 *	Find out if an errorcode was supplied
		 *	@return				true if this exception was created with an int param
		 */
		bool codevalid() const { return xcodevalid; }

		/**
		 *	Find out if the front-end has been notified about this exception
		 *	@return				true if the front-end has been told about the exception
		 */
		bool frontendnotified(void) const { return xfrontendnotified; }

		/**
		 *	Set the front-end notification flag
		 *	@param	yes			true to indicate notification has been passed on
		 *	@return				previous value of notification flag
		 */
		bool frontendnotified(bool yes) { bool oldval = xfrontendnotified; xfrontendnotified = yes; return oldval; }

		/**
		 *	Return the error description for the given code.
		 *	@param	err			Error code.
		 *	@return				Error string for error code.
		 */
		static std::string lookup_code(int err, const char *file = NULL, unsigned int line = 0);

	private:
		/// Function where the exception occured
		const char *xfunction;
		/// File name of the source file where the exception occured
		const char *xfile;
		// Line number in the source file where the exception occured
		unsigned int xline;
		// Error code
		int xcode;
		// flag so we can tell if this exception was created with an errorcode
		bool xcodevalid;
		// flag that tells us if this exception has been notified to the front-end
		bool xfrontendnotified;
};

}

#endif
