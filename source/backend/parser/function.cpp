/*******************************************************************************
 * function.cpp
 *
 * This module implements the the function type used by iso surfaces and
 * the function pattern.
 *
 * This module is based on code by D. Skarda, T. Bily and R. Suzuki.
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
 * $File: //depot/povray/smp/source/backend/parser/function.cpp $
 * $Revision: #16 $
 * $Change: 5179 $
 * $DateTime: 2010/11/01 13:56:10 $
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

#include <limits.h>

// frame.h must always be the first POV file included (pulls in platform config)
#include "backend/frame.h"
#include "backend/parser/parse.h"
#include "backend/vm/fnpovfpu.h"
#include "backend/math/splines.h"
#include "backend/texture/pigment.h"

// this must be the last file included
#include "base/povdebug.h"

namespace pov
{

/*****************************************************************************
 *
 * FUNCTION
 *
 *   Destroy_Function
 *
 * INPUT
 *
 * OUTPUT
 *
 * RETURNS
 *
 * AUTHOR
 *
 * DESCRIPTION
 *
 *   -
 *
 * CHANGES
 *   
 *   -
 *
 ******************************************************************************/

void Parser::Destroy_Function(FUNCTION_PTR Function)
{
	if(Function != NULL)
	{
		sceneData->functionVM->RemoveFunction(*Function);
		POV_FREE(Function);
	}
}

void Parser::Destroy_Function(FunctionVM *functionVM, FUNCTION_PTR Function)
{
	if(Function != NULL)
	{
		functionVM->RemoveFunction(*Function);
		POV_FREE(Function);
	}
}


/*****************************************************************************
 *
 * FUNCTION
 *
 *   Copy_Function
 *
 * INPUT
 *
 * OUTPUT
 *
 * RETURNS
 *
 * AUTHOR
 *
 * DESCRIPTION
 *
 *   -
 *
 * CHANGES
 *   
 *   -
 *
 ******************************************************************************/

FUNCTION_PTR Parser::Copy_Function(FUNCTION_PTR Function)
{
	FUNCTION_PTR ptr = (FUNCTION_PTR)POV_MALLOC(sizeof(FUNCTION), "Function ID");

	(void)sceneData->functionVM->GetFunctionAndReference(*Function); // increase the reference count
	*ptr = *Function;

	return ptr;
}

FUNCTION_PTR Parser::Copy_Function(FunctionVM *functionVM, FUNCTION_PTR Function)
{
	FUNCTION_PTR ptr = (FUNCTION_PTR)POV_MALLOC(sizeof(FUNCTION), "Function ID");

	functionVM->GetFunctionAndReference(*Function); // increase the reference count
	*ptr = *Function;

	return ptr;
}


/*****************************************************************************
*
* FUNCTION
*
*   Parse_Function
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   FUNCTION - parsed and compiled function reference number
*
* AUTHOR
*
*   Thorsten Froehlich
*   
* DESCRIPTION
*
*   Parse and compile a function and add it to the global function table.
*
* CHANGES
*
*   -
*
******************************************************************************/

FUNCTION_PTR Parser::Parse_Function(void)
{
	FUNCTION_PTR ptr = (FUNCTION_PTR)POV_MALLOC(sizeof(FUNCTION), "Function ID");
	ExprNode *expression = NULL;
	FunctionCode function;

	Parse_Begin();

	FNCode f(this, &function, false, NULL);

	expression = FNSyntax_ParseExpression();
	f.Compile(expression);
	FNSyntax_DeleteExpression(expression);

	Parse_End();

	*ptr = sceneData->functionVM->AddFunction(&function);

	return ptr;
}


/*****************************************************************************
*
* FUNCTION
*
*   Parse_FunctionContent
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   FUNCTION - parsed and compiled function reference number
*
* AUTHOR
*
*   Thorsten Froehlich
*   
* DESCRIPTION
*
*   Parse and compile a function and add it to the global function table.
*
* CHANGES
*
*   -
*
******************************************************************************/

FUNCTION_PTR Parser::Parse_FunctionContent(void)
{
	FUNCTION_PTR ptr = (FUNCTION_PTR)POV_MALLOC(sizeof(FUNCTION), "Function ID");
	ExprNode *expression = NULL;
	FunctionCode function;

	FNCode f(this, &function, false, NULL);

	expression = FNSyntax_ParseExpression();
	f.Compile(expression);
	FNSyntax_DeleteExpression(expression);

	*ptr = sceneData->functionVM->AddFunction(&function);

	return ptr;
}


/*****************************************************************************
*
* FUNCTION
*
*   Parse_DeclareFunction
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   FUNCTION - parsed and compiled function reference number
*
* AUTHOR
*
*   Thorsten Froehlich
*   
* DESCRIPTION
*
*   Parse and compile a function and add it to the global function table.
*   Additionally, this function takes an optional list of parameter names.
*
* CHANGES
*
*   -
*
******************************************************************************/

FUNCTION_PTR Parser::Parse_DeclareFunction(int *token_id, char *fn_name, bool is_local)
{
	FUNCTION_PTR ptr = (FUNCTION_PTR)POV_MALLOC(sizeof(FUNCTION), "Function ID");
	ExprNode *expression = NULL;
	FunctionCode function;

	// default type is float function
	*token_id = FUNCT_ID_TOKEN;

	FNCode f(this, &function, is_local, fn_name);
	f.Parameter();

	Parse_Begin();

	Get_Token();
	if(Token.Token_Id == INTERNAL_TOKEN)
	{
		GET(LEFT_PAREN_TOKEN);

		Get_Token();
		if(Token.Function_Id != FLOAT_TOKEN)
			Expectation_Error("internal function identifier");
		expression = FNSyntax_GetTrapExpression((unsigned int)(Token.Token_Float));

		function.flags = FN_INLINE_FLAG;

		GET(RIGHT_PAREN_TOKEN);
	}
	else if(Token.Token_Id == TRANSFORM_TOKEN)
	{
		if(function.parameter_cnt != 0)
			Error("Function parameters for transform functions are not allowed.");

		expression = FNSyntax_GetTrapExpression(1); // 1 refers to POVFPU_TrapSTable[1] = f_transform [trf]

		function.private_copy_method = (FNCODE_PRIVATE_COPY_METHOD)Copy_Transform;
		function.private_destroy_method = (FNCODE_PRIVATE_DESTROY_METHOD)Destroy_Transform;

		function.private_data = (void *)Parse_Transform_Block();

		function.return_size = 3; // returns a 3d vector!!!

		// function type is vector function
		*token_id = VECTFUNCT_ID_TOKEN;
	}
	else if(Token.Token_Id == SPLINE_TOKEN)
	{
		if(function.parameter_cnt != 0)
			Error("Function parameters for spline functions are not allowed.");

		Experimental_Flag |= EF_SPLINE;

		expression = FNSyntax_GetTrapExpression(2); // 2 refers to POVFPU_TrapSTable[2] = f_spline [trf]

		function.private_copy_method = (FNCODE_PRIVATE_COPY_METHOD)Copy_Spline;
		function.private_destroy_method = (FNCODE_PRIVATE_DESTROY_METHOD)Destroy_Spline;

		Parse_Begin();
		function.private_data = (void *)Parse_Spline();
		Parse_End();

		function.return_size = ((SPLINE *)(function.private_data))->Terms; // returns a 2d, 3d, 4d or 5d vector!!!

		// function type is vector function
		*token_id = VECTFUNCT_ID_TOKEN;
	}
	else if(Token.Token_Id == PIGMENT_TOKEN)
	{
		if(function.parameter_cnt != 0)
			Error("Function parameters for pigment functions are not allowed.");

		expression = FNSyntax_GetTrapExpression(0); // 0 refers to POVFPU_TrapSTable[0] = f_pigment [trf]

		function.private_copy_method = (FNCODE_PRIVATE_COPY_METHOD)Copy_Pigment;
		function.private_destroy_method = (FNCODE_PRIVATE_DESTROY_METHOD)Destroy_Pigment;

		Parse_Begin();
		function.private_data = (void *)Create_Pigment();
		Parse_Pigment((PIGMENT **)(&function.private_data));
		Parse_End();
		Post_Pigment((PIGMENT *)(function.private_data));

		function.return_size = 5; // returns a color!!!

		// function type is vector function
		*token_id = VECTFUNCT_ID_TOKEN;
	}
	else if(Token.Token_Id == PATTERN_TOKEN)
	{
		if(function.parameter_cnt != 0)
			Error("Function parameters for pattern functions are not allowed.");

		expression = FNSyntax_GetTrapExpression(77); // 77 refers to POVFPU_TrapTable[77] = f_pattern [trf]

		function.private_copy_method = (FNCODE_PRIVATE_COPY_METHOD)Copy_Pigment;
		function.private_destroy_method = (FNCODE_PRIVATE_DESTROY_METHOD)Destroy_Pigment;

		Parse_Begin();
		function.private_data = (void *)Create_Pigment(); // Yes, this is a pigment! [trf]
		Parse_PatternFunction((TPATTERN *)(function.private_data));
		Parse_End();
		Post_Pigment((PIGMENT *)(function.private_data));
	}
	else if(Token.Token_Id == STRING_LITERAL_TOKEN)
	{
		f.SetFlag(2, Token.Token_String);
		Get_Token();
		if(Token.Token_Id == COMMA_TOKEN)
		{
			Get_Token();
			if(Token.Token_Id != STRING_LITERAL_TOKEN)
				Expectation_Error("valid function expression");
			f.SetFlag(1, Token.Token_String);
		}
		else
		{
			Unget_Token();
			expression = FNSyntax_ParseExpression();
		}
	}
	else
	{
		Unget_Token();
		expression = FNSyntax_ParseExpression();
	}

	f.Compile(expression);
	FNSyntax_DeleteExpression(expression);

	Parse_End();

	*ptr = sceneData->functionVM->AddFunction(&function);

	return ptr;
}

}
