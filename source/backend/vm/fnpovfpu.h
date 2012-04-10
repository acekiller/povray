/*******************************************************************************
 * fnpovfpu.h
 *
 * This module contains all defines, typedefs, and prototypes for fnpovfpu.cpp.
 *
 * This module is inspired by code by D. Skarda, T. Bily and R. Suzuki.
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
 * $File: //depot/povray/smp/source/backend/vm/fnpovfpu.h $
 * $Revision: #13 $
 * $Change: 5434 $
 * $DateTime: 2011/04/12 17:57:09 $
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


#ifndef FNPOVFPU_H
#define FNPOVFPU_H

#include <vector>
#include <set>

#include <boost/thread.hpp>

namespace pov
{

class FunctionVM;
struct FPUContext;

}

#include "backend/frame.h"
#include "backend/vm/fncode.h"

namespace pov
{

#define MAX_CALL_STACK_SIZE 1024

enum
{
	ITYPE_R = 0,
	ITYPE_I,
	ITYPE_S,
	ITYPE_J,
	ITYPE_X,
	ITYPE_M
};

struct Opcode
{
	char *name;
	unsigned int code;
	int type;
};

enum
{
	TRAP_SYS1_SIN = 0,
	TRAP_SYS1_COS,
	TRAP_SYS1_TAN,
	TRAP_SYS1_ASIN,
	TRAP_SYS1_ACOS,
	TRAP_SYS1_ATAN,
	TRAP_SYS1_SINH,
	TRAP_SYS1_COSH,
	TRAP_SYS1_TANH,
	TRAP_SYS1_ASINH,
	TRAP_SYS1_ACOSH,
	TRAP_SYS1_ATANH,
	TRAP_SYS1_FLOOR,
	TRAP_SYS1_CEIL,
	TRAP_SYS1_SQRT,
	TRAP_SYS1_EXP,
	TRAP_SYS1_LN,
	TRAP_SYS1_LOG,
	TRAP_SYS1_INT
};

enum
{
	TRAP_SYS2_POW = 0,
	TRAP_SYS2_ATAN2,
	TRAP_SYS2_MOD,
	TRAP_SYS2_DIV
};

typedef SYS_MATH_RETURN (*Sys1)(SYS_MATH_PARAM r0);
typedef SYS_MATH_RETURN (*Sys2)(SYS_MATH_PARAM r0,SYS_MATH_PARAM r1);

class FunctionVM;

// WARNING: Do not change this structure without notice!!!
// Platform specific code may depend on the exact layout and size! [trf]
struct FunctionEntry
{
	union {
		FunctionCode fn;            // valid if reference_count != 0
		FUNCTION next_unreferenced; // valid if reference_count == 0
	};
	unsigned int reference_count;
	SYS_FUNCTION_ENTRY
};

// WARNING: Do not change this structure without notice!!!
// Platform specific code may depend on the exact layout and size! [trf]
struct StackFrame
{
	unsigned int pc;
	FUNCTION fn;
};

// WARNING: Do not change this structure without notice!!!
// Platform specific code may depend on the exact layout and size! [trf]
struct FPUContext
{
	StackFrame *pstackbase;
	DBL *dblstackbase;
	unsigned int maxdblstacksize;
	FunctionVM *functionvm;
	TraceThreadData *threaddata;
	#if (SYS_FUNCTIONS == 1)
	DBL *dblstack;
	#endif

	void SetLocal(unsigned int k, DBL v);
	DBL GetLocal(unsigned int k);
};


#define OPCODE(i,s,d) ((i << 6) | (s << 3) | d)

#define OPCODE_ADD    OPCODE(0,0,0)
#define OPCODE_SUB    OPCODE(1,0,0)
#define OPCODE_MUL    OPCODE(2,0,0)
#define OPCODE_DIV    OPCODE(3,0,0)
#define OPCODE_MOD    OPCODE(4,0,0)
#define OPCODE_MOVE   OPCODE(5,0,0)
#define OPCODE_CMP    OPCODE(6,0,0)
#define OPCODE_NEG    OPCODE(7,0,0)
#define OPCODE_ABS    OPCODE(8,0,0)
#define OPCODE_ADDI   OPCODE(9,0,0)
#define OPCODE_SUBI   OPCODE(9,1,0)
#define OPCODE_MULI   OPCODE(9,2,0)
#define OPCODE_DIVI   OPCODE(9,3,0)
#define OPCODE_MODI   OPCODE(9,4,0)
#define OPCODE_LOADI  OPCODE(9,5,0)
#define OPCODE_CMPI   OPCODE(9,6,0)
#define OPCODE_SEQ    OPCODE(10,0,0)
#define OPCODE_SNE    OPCODE(10,1,0)
#define OPCODE_SLT    OPCODE(10,2,0)
#define OPCODE_SLE    OPCODE(10,3,0)
#define OPCODE_SGT    OPCODE(10,4,0)
#define OPCODE_SGE    OPCODE(10,5,0)
#define OPCODE_TEQ    OPCODE(10,6,0)
#define OPCODE_TNE    OPCODE(10,7,0)
#define OPCODE_LOAD   OPCODE(11,0,0)
#define OPCODE_STORE  OPCODE(12,0,0)
#define OPCODE_BEQ    OPCODE(13,0,0)
#define OPCODE_BNE    OPCODE(13,1,0)
#define OPCODE_BLT    OPCODE(13,2,0)
#define OPCODE_BLE    OPCODE(13,3,0)
#define OPCODE_BGT    OPCODE(13,4,0)
#define OPCODE_BGE    OPCODE(13,5,0)
#define OPCODE_XEQ    OPCODE(14,0,0)
#define OPCODE_XNE    OPCODE(14,1,0)
#define OPCODE_XLT    OPCODE(14,2,0)
#define OPCODE_XLE    OPCODE(14,3,0)
#define OPCODE_XGT    OPCODE(14,4,0)
#define OPCODE_XGE    OPCODE(14,5,0)
#define OPCODE_XDZ    OPCODE(14,6,0)
// #define OPCODE_XINF   OPCODE(14,7,0)
#define OPCODE_JSR    OPCODE(15,0,0)
#define OPCODE_JMP    OPCODE(15,0,1)
#define OPCODE_RTS    OPCODE(15,0,2)
#define OPCODE_CALL   OPCODE(15,0,3)
#define OPCODE_SYS1   OPCODE(15,0,4)
#define OPCODE_SYS2   OPCODE(15,0,5)
#define OPCODE_TRAP   OPCODE(15,0,6)
#define OPCODE_TRAPS  OPCODE(15,0,7)
#define OPCODE_GROW   OPCODE(15,1,0)
#define OPCODE_PUSH   OPCODE(15,1,1)
#define OPCODE_POP    OPCODE(15,1,2)
#define OPCODE_DEBUG  OPCODE(15,1,5)
#define OPCODE_NOP    OPCODE(15,3,7)

extern const Opcode POVFPU_Opcodes[];

extern const Sys1 POVFPU_Sys1Table[];
extern const Sys2 POVFPU_Sys2Table[];

extern const unsigned int POVFPU_Sys1TableSize;
extern const unsigned int POVFPU_Sys2TableSize;

void POVFPU_Exception(FPUContext *context, FUNCTION fn, const char *msg = NULL);
DBL POVFPU_RunDefault(FPUContext *context, FUNCTION k);

class FunctionVM
{
		friend void POVFPU_Exception(FPUContext *, FUNCTION, const char *);
		friend DBL POVFPU_RunDefault(FPUContext *, FUNCTION);
	public:
		FunctionVM();
		~FunctionVM();

		void Reset();

		void SetGlobal(unsigned int k, DBL v);
		DBL GetGlobal(unsigned int k);

		FunctionCode *GetFunction(FUNCTION k);
		FunctionCode *GetFunctionAndReference(FUNCTION k);

		unsigned int AddConstant(DBL v);

		FUNCTION AddFunction(FunctionCode *f);
		void RemoveFunction(FUNCTION fn);

		FPUContext *NewContext(TraceThreadData *);
		void DeleteContext(FPUContext *);
	private:
		vector<FunctionEntry> functions;
		FUNCTION nextUnreferenced;
		set<FPUContext *> contexts;
		vector<DBL> globals;
		vector<DBL> consts;
		boost::recursive_mutex contextMutex;
};

}

#endif
