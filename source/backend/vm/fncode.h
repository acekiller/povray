/*******************************************************************************
 * fncode.h
 *
 * This module contains all defines, typedefs, and prototypes for fncode.cpp.
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
 * $File: //depot/povray/smp/source/backend/vm/fncode.h $
 * $Revision: #12 $
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


#ifndef FNCODE_H
#define FNCODE_H

#include "backend/frame.h"
#include "backend/parser/parse.h"
#include "backend/math/matrices.h"
#include "backend/math/vector.h"
#include "base/textstream.h"


namespace pov
{

#ifndef DEBUG_FLOATFUNCTION
 #define DEBUG_FLOATFUNCTION 0
#endif

#define FN_INLINE_FLAG 1
#define FN_LOCAL_FLAG  2

#define MAX_K ((unsigned int)0x000fffff)
#define MAX_FN MAX_K

#define MAKE_INSTRUCTION(op, k) ((((k) << 12) & 0xfffff000) | ((op) & 0x00000fff))

#define GET_OP(w) ((w) & 0x00000fff)
#define GET_K(w) (((w) >> 12) & 0x000fffff)


typedef void *(*FNCODE_PRIVATE_COPY_METHOD)(void *);
typedef void (*FNCODE_PRIVATE_DESTROY_METHOD)(void *);

typedef unsigned int Instruction;

struct FunctionCode
{
	Instruction *program;
	unsigned int program_size;
	unsigned char return_size;
	unsigned char parameter_cnt;
	unsigned char localvar_cnt;
	unsigned int localvar_pos[MAX_PARAMETER_LIST];
	char *localvar[MAX_PARAMETER_LIST];
	char *parameter[MAX_PARAMETER_LIST];
	char *name;
	UCS2 *filename;
	pov_base::ITextStream::FilePos filepos;
	unsigned int flags;
	FNCODE_PRIVATE_COPY_METHOD private_copy_method;
	FNCODE_PRIVATE_DESTROY_METHOD private_destroy_method;
	void *private_data;
};

void FNCode_Copy(FunctionCode *, FunctionCode *);
void FNCode_Delete(FunctionCode *);

class FNCode
{
	public:
		FNCode(Parser *, FunctionCode *, bool, char *);

		void Parameter();
		void Compile(Parser::ExprNode *);
		void SetFlag(unsigned int, char *);
	private:
		FunctionCode *function;
		Parser *parser;
		FunctionVM *functionVM;

		unsigned int max_program_size;
		unsigned int max_stack_size;
		unsigned int stack_pointer;
		unsigned int parameter_stack_pointer;
		int level;

		#if (DEBUG_FLOATFUNCTION == 1)

		char *asm_input;
		char *asm_output;
		char *asm_error;

		#endif

		FNCode();
		FNCode(FNCode&);

		void compile_recursive(Parser::ExprNode *expr);
		void compile_member(char *name);
		void compile_call(Parser::ExprNode *expr, FUNCTION fn, int token, char *name);
		void compile_select(Parser::ExprNode *expr);
		void compile_seq_op(Parser::ExprNode *expr, unsigned int op, DBL neutral);
		void compile_float_function_call(Parser::ExprNode *expr, FUNCTION fn, char *name);
		void compile_vector_function_call(Parser::ExprNode *expr, FUNCTION fn, char *name);
		void compile_inline(FunctionCode *function);
		void compile_variable(char *name);
		void compile_parameters();
		unsigned int compile_push_result();
		void compile_pop_result(unsigned int local_k);
		unsigned int compile_instruction(unsigned int, unsigned int, unsigned int, unsigned int);
		unsigned int compile_instruction(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

		#if (DEBUG_FLOATFUNCTION == 1)

		int assemble(char *filename);
		int disassemble(char *filename);
		void disassemble_instruction(FILE *f, Instruction& i);

		unsigned int parse_instruction(FILE *f);
		unsigned int parse_reg(FILE *f);
		DBL parse_float(FILE *f);
		unsigned int parse_integer(FILE *f);
		unsigned int parse_move(FILE *f, unsigned int& r, unsigned int& k);
		void parse_comma(FILE *f);
		bool parse_comment(FILE *f);
		void skip_space(FILE *f);
		void skip_newline(FILE *f);

		#endif
};

}

#endif
