/****************************************************************************
*                   povmsend.h
*
*  This module contains all defines, typedefs, and prototypes for povmsend.cpp
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996-2002 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file.
*  If POVLEGAL.DOC is not available it may be found online at -
*
*    http://www.povray.org/povlegal.html.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
* $File: //depot/povray/3.5/source/povmsend.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/


#ifndef POVMSSEND_H
#define POVMSSEND_H


/*****************************************************************************
* Global preprocessor defines
******************************************************************************/


/*****************************************************************************
* Global typedefs
******************************************************************************/


/*****************************************************************************
* Global variables
******************************************************************************/


/*****************************************************************************
* Global functions
******************************************************************************/

int BuildCommand(POVMSObjectPtr msg, POVMSType key, SHELLDATA *data);
int BuildRTime(POVMSObjectPtr msg, POVMSType key, int parse, int photon, int render, int total);
int BuildRenderOptions(POVMSObjectPtr msg);
int BuildRenderStatus(POVMSObjectPtr msg);
int BuildRenderStatistics(POVMSObjectPtr msg, COUNTER *pstats);
int AddStatistic(POVMSObjectPtr msg, POVMSType key, COUNTER *counter);
int AddOIStatistic(POVMSAttributeListPtr list, int index, COUNTER *pstats);

void Send_InitInfo();
int Send_RenderStatus(char *statusString);
int Send_RenderTime(int tp, int th, int tr);
int Send_RenderStatistics();
int Send_RenderOptions();
int Send_RenderDone();

#endif /* POVMSSEND_H */
