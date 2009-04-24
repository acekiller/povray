/****************************************************************************
*                povms.cpp
*
*  This module contains POVMS data type handling and utility functions.
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
* $File: //depot/povray/3.5/source/povms.cpp $
* $Revision: #18 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#ifdef POVMS_DISCONNECTED
#include "cnfpovms.h"
#else
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "frame.h"
#include "userio.h"
#endif

#include "povms.h"
#include "pov_err.h"


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

#undef _DEBUG_POVMS_
// #define _DEBUG_POVMS_

#define kMaxQueueSize     16
#define kDefaultTimeout   30

/*****************************************************************************
* Local typedefs
******************************************************************************/

typedef struct POVMSObjectQueueEntry POVMSObjectQueueEntry;
typedef struct POVMSObjectQueue POVMSObjectQueue;
typedef struct POVMSReceiveHandlerNode POVMSReceiveHandlerNode;
typedef struct POVMSThreadDataNode POVMSThreadDataNode;
typedef struct POVMSThreadDataNodeRoot POVMSThreadDataNodeRoot;

struct POVMSObjectQueueEntry
{
	POVMSThreadID sender;
	POVMSObject msg;
	POVMSObject result;
	int mode;
};

struct POVMSObjectQueue
{
	int offset;
	int count;
	POVMSObjectQueueEntry queue[kMaxQueueSize];
};

struct POVMSReceiveHandlerNode
{
	POVMSReceiveHandlerNode *last;
	POVMSReceiveHandlerNode *next;
	POVMSType handledclass;
	POVMSType handledid;
	int (*handler)(POVMSObjectPtr, POVMSObjectPtr, int);
};

struct POVMSThreadDataNode
{
	POVMSThreadDataNode *last;
	POVMSThreadDataNode *next;
	POVMSThreadID threadid;
	POVMSObjectQueue msgqueue;
	POVMSObjectQueueEntry *waitmsg;
	POVMSReceiveHandlerNode *receivehandlerroot;
};

struct POVMSThreadDataNodeRoot
{
	POVMSInt version;
	POVMSThreadDataNode *root;
};


/*****************************************************************************
* Local variables
******************************************************************************/

volatile POVMSThreadDataNodeRoot *gPOVMSThreadDataRoot = NULL;
volatile int gPOVMSIsInitialized = 0;

/*****************************************************************************
* Local functions
******************************************************************************/

POVMSThreadDataNode *POVMS_AddThreadDataNode  ();
int POVMS_RemoveThreadDataNode                (POVMSThreadDataNode *tdn);
POVMSThreadDataNode *POVMS_FindThreadDataNode (POVMSThreadID tid);

POVMSReceiveHandlerNode *POVMS_AddReceiveHandlerNode  (POVMSThreadDataNode *tdn);
int POVMS_RemoveReceiveHandlerNode                    (POVMSThreadDataNode *tdn, POVMSReceiveHandlerNode *thn);
POVMSReceiveHandlerNode *POVMS_FindReceiveHandlerNode (POVMSThreadDataNode *tdn, POVMSType hclass, POVMSType hid);

int POVMS_Receive              (POVMSObjectPtr msg, POVMSObjectPtr result, int mode);

POVMSNode *POVMSObject_Find    (POVMSObjectPtr msg, POVMSType key);

#ifndef POVMS_NO_DUMP_SUPPORT
int POVMSObject_DumpSpace      (FILE *file);
#endif


/*****************************************************************************
*
* FUNCTION
*   POVMS_Initialize
*   
* DESCRIPTION
*   Call to init internal data structures before any other call to the POVMS.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT void * POVMS_CDECL POVMS_Initialize(void *rootptr)
{
	POVMS_BeginAtomic();

	gPOVMSThreadDataRoot = (POVMSThreadDataNodeRoot *)rootptr;
	if(gPOVMSThreadDataRoot == NULL)
	{
		// NOTE: This memory will not be freed when terminating POVMS via POVMS_Deinitialize in
		// order to still keep it available to other threads that have their own POVMS running!
		gPOVMSThreadDataRoot = (POVMSThreadDataNodeRoot *)POVMS_Malloc(sizeof(POVMSThreadDataNodeRoot));
		if(POVMS_ASSERT(gPOVMSThreadDataRoot != NULL, "POVMS_Initialize failed, out of memory") == false)
		{
			POVMS_EndAtomic();
			return NULL;
		}

		gPOVMSThreadDataRoot->version = POVMS_VERSION;
		gPOVMSThreadDataRoot->root = NULL;
	}
	else
	{
		// make sure the POVMS versions are identical
		if(POVMS_ASSERT(gPOVMSThreadDataRoot->version == POVMS_VERSION, "POVMS_Initialize failed, POVMS versions not compatible") == false)
		{
			gPOVMSThreadDataRoot = NULL;
			POVMS_EndAtomic();
			return NULL;
		}
	}

	gPOVMSIsInitialized++;

	POVMS_EndAtomic();

	return (void *)gPOVMSThreadDataRoot;
}

/*****************************************************************************
*
* FUNCTION
*   POVMS_Deinitialize
*   
* DESCRIPTION
*   Call to destroy internal data structures and POVMS_Free all POVMS related data.
*   Never call this inside a POVMS function!
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMS_Deinitialize()
{
	POVMSThreadDataNode *curtdn = NULL;

	POVMS_BeginAtomic();

	if(POVMS_ASSERT(gPOVMSIsInitialized > 0, "POVMS_Deinitialize called but POVMS is not initialize") == false)
	{
		POVMS_EndAtomic();
		return kParamErr;
	}

	POVMS_EndAtomic();

	for(curtdn = POVMS_FindThreadDataNode(POVMS_GetCurrentThread());
	    curtdn != NULL;
	    curtdn = POVMS_FindThreadDataNode(POVMS_GetCurrentThread()))
	{
		POVMS_RemoveThreadDataNode(curtdn);
	}

	POVMS_BeginAtomic();

	if(gPOVMSIsInitialized > 0)
		gPOVMSIsInitialized--;
	else
		gPOVMSIsInitialized = 0;

	POVMS_EndAtomic();

	return kNoErr;
}

/*****************************************************************************
*
* FUNCTION
*   POVMS_ASSERTFunction
*   
* DESCRIPTION
*   Low level fatal error checking and output function.
*   Note: Currently it will not and may not terminate execution!
*
* CHANGES
*   -
*
******************************************************************************/

POVMSBool POVMS_AssertFunction(int cond, char *str, char *filename, long line)
{
	if(cond == false)
	{
		POVMS_ASSERT_OUTPUT(str,filename,line);
		return false;
	}

	return true;
}

/*****************************************************************************
*
* FUNCTION
*   POVMS_InstallReceiver
*   
* DESCRIPTION
*   Installs a function which will be called when a message with the given
*   class and id is received. You may specifiy kPOVMSType_WildCard as id, then
*   the function will get all messages of the class. Note that you may not
*   define other handlers for a class if you registered a handler function
*   with the kPOVMSType_WildCard id for that class. However, there is currently
*   no checking if a specified class and id are valid.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMS_InstallReceiver(int (*hfunc)(POVMSObjectPtr, POVMSObjectPtr, int), POVMSType hclass, POVMSType hid)
{
	POVMSReceiveHandlerNode *currhn = NULL;
	POVMSThreadDataNode *curtdn = NULL;

	if(hfunc == NULL)
		return kParamErr;

	POVMS_BeginAtomic();

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
	{
		POVMS_EndAtomic();
		return kParamErr;
	}

	curtdn = POVMS_FindThreadDataNode(POVMS_GetCurrentThread());
	if(curtdn == NULL)
	{
		curtdn = POVMS_AddThreadDataNode();
		if(curtdn == NULL)
		{
			POVMS_EndAtomic();
			return kMemFullErr;
		}
	}

	currhn = POVMS_AddReceiveHandlerNode(curtdn);
	if(currhn == NULL)
	{
		POVMS_EndAtomic();
		return kMemFullErr;
	}

	currhn->handler = hfunc;
	currhn->handledclass = hclass;
	currhn->handledid = hid;

	POVMS_EndAtomic();

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_RemoveReceiver
*

* DESCRIPTION
*   Removes a handler function for the specified class and id.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMS_RemoveReceiver(POVMSType hclass, POVMSType hid)
{
	POVMSThreadDataNode *tdn = NULL;
	int err = kNoErr;

	POVMS_BeginAtomic();

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
	{
		POVMS_EndAtomic();
		return kParamErr;
	}

	tdn = POVMS_FindThreadDataNode(POVMS_GetCurrentThread());
	if(tdn == NULL)
		err = kParamErr;

	if(err == kNoErr)
		err = POVMS_RemoveReceiveHandlerNode(tdn, POVMS_FindReceiveHandlerNode(tdn, hclass, hid));

	if(err == kNoErr)
	{
		if(tdn->receivehandlerroot == NULL)
			err = POVMS_RemoveThreadDataNode(tdn);
	}

	POVMS_EndAtomic();

	return err;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_ProcessMessages
*   
* DESCRIPTION
*   Processes a message in the queue if available. Returns kFalseErr if more
*   messages *may* still be waiting in the queue.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMS_ProcessMessages(void)
{
	POVMSObject msg = { 0, 0, NULL };
	POVMSObject result = { 0, 0, NULL };
	POVMSObjectPtr resultptr = NULL;
	POVMSThreadDataNode *tdn = NULL;
	POVMSThreadID tid = POVMS_GetCurrentThread();
	int mode = kPOVMSSendMode_NoReply;
	int ret = kNoErr;

	POVMS_BeginAtomic();

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
	{
		POVMS_EndAtomic();
		return kParamErr;
	}

	tdn = POVMS_FindThreadDataNode(tid);

	if((tdn->msgqueue.count > 0) || (tdn->waitmsg != NULL))
	{
		if(tdn->waitmsg != NULL)
		{
			msg = tdn->waitmsg->msg;
			mode = tdn->waitmsg->mode;
			if(mode != kPOVMSSendMode_NoReply)
			{
				result = tdn->waitmsg->result;
				resultptr = &result;
			}
		}
		else
		{
			msg = tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].msg;
			mode = tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].mode;
			if(mode != kPOVMSSendMode_NoReply)
			{
				result = tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].result;
				resultptr = &result;
			}
			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].msg.type = kPOVMSType_Object;
			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].msg.size = 0;
			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].msg.root = NULL;

			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].result.type = kPOVMSType_Object;
			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].result.size = 0;
			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].result.root = NULL;

			tdn->msgqueue.queue[tdn->msgqueue.offset % kMaxQueueSize].mode = kPOVMSSendMode_NoReply;

			tdn->msgqueue.offset = ((tdn->msgqueue.offset + 1) % kMaxQueueSize);
			tdn->msgqueue.count--;
		}

		POVMS_EndAtomic();

		if(msg.root != NULL)
		{
			// NOTE: If by POVMS_Receive result is not 0 an error occured and maybe an error should automatically be send somehow...
			(void)POVMS_Receive(&msg, resultptr, mode);
			(void)POVMSObject_Delete(&msg);
		}

		POVMS_BeginAtomic();

		if(tdn->waitmsg != NULL)
			tdn->waitmsg->sender = POVMS_InitThreadID(); // mark as processed

		if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		{
			POVMS_EndAtomic();
			return kParamErr;
		}

		if(tdn->msgqueue.count != 0)
			ret = kFalseErr;
	}

	POVMS_EndAtomic();

	return ret;
}

/*****************************************************************************
*
* FUNCTION
*   POVMS_Send
*   
* DESCRIPTION
*   Sends a message by adding it to a queue of message which will be handled
*   when POVMS_ProcessMessages is called. The current limit of messages in the
*   queue is kMaxQueueSize, if the queue is full POVMS__Send will not add the
*   message. It will return the error code kQueueFullErr, which is no fatal
*   err - as soonas there is space available again in the queue you can send
*   the message.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMS_Send(POVMSObjectPtr msg, POVMSObjectPtr result, int mode)
{
	POVMSThreadDataNode *tdn = NULL;
	POVMSThreadID tid = POVMS_GetCurrentThread();
	int err = kNoErr;

	if(msg == NULL)
		return kParamErr;
	if(msg->type == kPOVMSType_LockedObject)
		return kNotNowErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	switch(mode)
	{
		case kPOVMSSendMode_NoReply:
		case kPOVMSSendMode_WantReceipt:
			result = NULL;
			break;
		case kPOVMSSendMode_QueueReply:
		case kPOVMSSendMode_WaitReply:
			if(result == NULL)
				err = kParamErr;
			break;
		default:
			err = kParamErr;
	}

	if(err == kNoErr)
		err = POVMS_GetMsgThreadID(msg, kPOVMSTargetAddressID, &tid);

	if(err == kNoErr)
	{
		if((mode == kPOVMSSendMode_WaitReply) || (mode == kPOVMSSendMode_WantReceipt))
		{
			if(POVMS_CompareThreads(POVMS_GetCurrentThread(), tid) == kNoErr)
			{
				err = POVMS_Receive(msg, result, mode);
			}
			else if(POVMS_CompareThreads(POVMS_InitThreadID(), tid) == kNoErr)
			{
				// remote target
			}
			else
			{
				unsigned long curtime = POVMS_GetTimeInSeconds();
				int maxtime = kDefaultTimeout;

				if(POVMSUtil_GetInt(msg, kPOVMSMessageTimeoutID, &maxtime) != kNoErr)
					maxtime = kDefaultTimeout; // kDefaultTimeout seconds is the default timeout

				while(err == kNoErr)
				{
					POVMS_BeginAtomic();

					tdn = POVMS_FindThreadDataNode(tid);
					if(tdn == NULL)
						err = kCannotConnectErr;
					else
					{
						if(tdn->waitmsg == NULL)
							break;
					}

					POVMS_EndAtomic();

					if(POVMS_GetTimeInSeconds() - curtime >= maxtime)
						err = kTimeoutErr;

					err = POVMS_SwitchToThread(tid);
				}

				if(err == kNoErr)
				{
					tdn->waitmsg = (POVMSObjectQueueEntry *)POVMS_Malloc(sizeof(POVMSObjectQueueEntry));
					if(tdn->waitmsg == NULL)
						err = kOutOfMemoryErr;
					else
					{
						tdn->waitmsg->sender = POVMS_GetCurrentThread();
						tdn->waitmsg->msg = *msg;
						if(result != NULL)
							tdn->waitmsg->result = *result;
						else
						{
							tdn->waitmsg->result.type = kPOVMSType_Null;
							tdn->waitmsg->result.size = 0;
							tdn->waitmsg->result.root = NULL;
						}
						tdn->waitmsg->mode = mode;

						POVMS_EndAtomic();
					}
				}

				while(err == kNoErr)
				{
					POVMS_BeginAtomic();

					tdn = POVMS_FindThreadDataNode(tid);
					if(tdn == NULL)
						err = kDisconnectedErr;
					else
					{
						if(tdn->waitmsg == NULL)
							err = kDisconnectedErr;
						else
						{
							if(POVMS_CompareThreads(POVMS_InitThreadID(), tdn->waitmsg->sender) == kNoErr) // marker that msg has been handled
							{
								*result = tdn->waitmsg->result;
								if(POVMSUtil_GetInt(result, kPOVMSMessageErrorID, &err) != kNoErr) // no result code, assume no error
									err = kNoErr;

								POVMS_Free((void *)(tdn->waitmsg));
								tdn->waitmsg = NULL;

								POVMS_EndAtomic();
								break;
							}
						}
					}

					POVMS_EndAtomic();

					if(POVMS_GetTimeInSeconds() - curtime >= maxtime)
						err = kTimeoutErr;

					err = POVMS_SwitchToThread(tid);
				}
			}
		}
		else
		{
			POVMS_BeginAtomic();

			tdn = POVMS_FindThreadDataNode(tid);
			if(tdn == NULL)
				err = kParamErr;

			if(err == kNoErr)
			{
				if(tdn->msgqueue.count >= kMaxQueueSize)
					err = kQueueFullErr;
				else
				{
					tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].sender = POVMS_GetCurrentThread();
					tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].msg = *msg;
					if(result != NULL)
						tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].result = *result;
					else
					{
						tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].result.type = kPOVMSType_Null;
						tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].result.size = 0;
						tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].result.root = NULL;
					}
					tdn->msgqueue.queue[(tdn->msgqueue.offset + tdn->msgqueue.count) % kMaxQueueSize].mode = mode;
					tdn->msgqueue.count++;
				}
			}

			POVMS_EndAtomic();
		}
	}

#ifdef _DEBUG_POVMS_
	puts("");
	(void)POVMSObject_Dump(stdout, msg);
	puts("");
#endif

	return err;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_Receive
*   
* DESCRIPTION
*   Receive a message and dispatch it to the handler function. It returns
*   0 if everything is ok, kNotNowErr if the message object was locked,
*   kCannotHandleDataErr if no message receive handler for the message was
*   found and any other value is a return value of the receive handler
*   function.
*
* CHANGES
*   -
*
******************************************************************************/

int POVMS_Receive(POVMSObjectPtr msg, POVMSObjectPtr result, int mode)
{
	POVMSThreadDataNode *tdn = NULL;
	POVMSReceiveHandlerNode *cur;
	POVMSType hclass,hid;
	long l = 0;
	int ret = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(msg == NULL)
		return kParamErr;
	if(msg->type == kPOVMSType_LockedObject)
		return kNotNowErr;

	ret = POVMSUtil_GetType(msg, kPOVMSMessageClassID, &hclass);
	if(ret == kNoErr)
		ret = POVMSUtil_GetType(msg, kPOVMSMessageIdentID, &hid);
	if(ret == kNoErr)
	{
		tdn = POVMS_FindThreadDataNode(POVMS_GetCurrentThread());
		if(tdn == NULL)
			ret = kCannotConnectErr;
	}
	if(ret == kNoErr)
	{
		POVMS_BeginAtomic();

		for(cur = tdn->receivehandlerroot; cur != NULL; cur = cur->next)
		{
			if((cur->handledclass == hclass) && (cur->handledid == hid))
				break;
			else if((cur->handledclass == hclass) && (cur->handledid == kPOVMSType_WildCard))
				break;
		}

		if(cur == NULL)
			ret = kCannotHandleDataErr;
		else
		{
			if(cur->handler == NULL)
				ret = kNullPointerErr;
			else
			{
				ret = cur->handler(msg, result, mode);
				if(result != NULL)
					(void)POVMSUtil_SetInt(result, kPOVMSMessageErrorID, ret);

				if(mode == kPOVMSSendMode_QueueReply)
				{
					POVMS_EndAtomic();
					ret = POVMS_Send(result, NULL, kPOVMSSendMode_NoReply);
					POVMS_BeginAtomic(); // Insure that Begin...End calls are matched, so this is needed! [trf]
				}
			}
		}

		POVMS_EndAtomic();
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_AddThreadDataNode
*   
* DESCRIPTION
*   Create a new thread node and insert it into the list of thread nodes.
*   This function requires mutual exclusive execution.
*
* CHANGES
*   -
*
******************************************************************************/

POVMSThreadDataNode *POVMS_AddThreadDataNode()
{
	POVMSThreadDataNode *out;
	int i;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return NULL;

	out = (POVMSThreadDataNode *)POVMS_Malloc(sizeof(POVMSThreadDataNode));
	if(POVMS_ASSERT(out != NULL, "POVMS_AddThreadDataNode failed, out of memory") == false)
		return NULL;

	for(i = 0; i < kMaxQueueSize; i++)
	{
		out->msgqueue.queue[i].sender = POVMS_InitThreadID();
		out->msgqueue.queue[i].msg.type = kPOVMSType_Object;
		out->msgqueue.queue[i].msg.size = 0;
		out->msgqueue.queue[i].msg.root = NULL;
		out->msgqueue.queue[i].result.type = kPOVMSType_Object;
		out->msgqueue.queue[i].result.size = 0;
		out->msgqueue.queue[i].result.root = NULL;
		out->msgqueue.queue[i].mode = kPOVMSSendMode_NoReply;
	}

	out->waitmsg = NULL;
	out->msgqueue.offset = 0;
	out->msgqueue.count = 0;
	out->receivehandlerroot = NULL;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
	{
		POVMS_Free((void *)out);
		return NULL;
	}

	out->last = NULL;
	out->next = gPOVMSThreadDataRoot->root;

	gPOVMSThreadDataRoot->root = out;
	if(out->next != NULL)
		out->next->last = out;

	return out;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_RemoveThreadDataNode
*   
* DESCRIPTION
*   Removes a thread node from the list of thread nodes and frees the memory.
*   This function requires mutual exclusive execution.
*
* CHANGES
*   -
*
******************************************************************************/

int POVMS_RemoveThreadDataNode(POVMSThreadDataNode *tdn)
{
	int ret = kNoErr;
	int i;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(tdn == NULL)
	{
		ret = kNullPointerErr;
	}
	else
	{
		for(i = 0; i < kMaxQueueSize; i++)
		{
			tdn->msgqueue.queue[i].sender = POVMS_InitThreadID();

			if(tdn->msgqueue.queue[i].msg.root != NULL)
				(void)POVMSObject_Delete(&(tdn->msgqueue.queue[i].msg));
			tdn->msgqueue.queue[i].msg.type = kPOVMSType_Object;
			tdn->msgqueue.queue[i].msg.size = 0;
			tdn->msgqueue.queue[i].msg.root = NULL;

			if(tdn->msgqueue.queue[i].result.root != NULL)
				(void)POVMSObject_Delete(&(tdn->msgqueue.queue[i].result));
			tdn->msgqueue.queue[i].result.type = kPOVMSType_Object;
			tdn->msgqueue.queue[i].result.size = 0;
			tdn->msgqueue.queue[i].result.root = NULL;

			tdn->msgqueue.queue[i].mode = kPOVMSSendMode_NoReply;
		}

		if(tdn->waitmsg != NULL)
		{
			if(tdn->waitmsg->msg.root != NULL)
				(void)POVMSObject_Delete(&(tdn->waitmsg->msg));
			if(tdn->waitmsg->result.root != NULL)
				(void)POVMSObject_Delete(&(tdn->waitmsg->result));
			POVMS_Free((void *)(tdn->waitmsg));
		}

		tdn->waitmsg = NULL;
		tdn->msgqueue.offset = 0;
		tdn->msgqueue.count = 0;

		while(tdn->receivehandlerroot != NULL)
			(void)POVMS_RemoveReceiveHandlerNode(tdn, tdn->receivehandlerroot);

		if(tdn == gPOVMSThreadDataRoot->root)
			gPOVMSThreadDataRoot->root = (gPOVMSThreadDataRoot->root)->next;

		if(tdn->last != NULL)
			tdn->last->next = tdn->next;
		if(tdn->next != NULL)
			tdn->next->last = tdn->last;

		POVMS_Free((void *)tdn);
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_FindThreadDataNode
*   
* DESCRIPTION
*   Search for a thread node by thread id in the list of thread nodes.
*   This function requires mutual exclusive execution.
*
* CHANGES
*   -
*
******************************************************************************/

POVMSThreadDataNode *POVMS_FindThreadDataNode(POVMSThreadID tid)
{
	POVMSThreadDataNode *tdn;
	int ret = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return NULL;

	for(tdn = gPOVMSThreadDataRoot->root; tdn != NULL ; tdn = tdn->next)
	{
		if(POVMS_CompareThreads(tdn->threadid, tid) == false)
			break;
	}

	return tdn;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_AddReceiveHandlerNode
*   
* DESCRIPTION
*   Create a new receive handler node and insert it into the thread nodes
*   list of receive handlers.
*   This function requires mutual exclusive execution.
*
* CHANGES
*   -
*
******************************************************************************/

POVMSReceiveHandlerNode *POVMS_AddReceiveHandlerNode(POVMSThreadDataNode *tdn)
{
	POVMSReceiveHandlerNode *out;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return NULL;

	out = (POVMSReceiveHandlerNode *)POVMS_Malloc(sizeof(POVMSReceiveHandlerNode));
	if(POVMS_ASSERT(out != NULL, "POVMS_AddReceiveHandlerNode failed, out of memory") == false)
		return NULL;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
	{
		POVMS_Free((void *)out);
		return NULL;
	}

	out->last = NULL;
	out->next = tdn->receivehandlerroot;
	out->handledclass = kPOVMSType_Null;
	out->handledid = kPOVMSType_Null;
	out->handler = NULL;

	tdn->receivehandlerroot = out;
	if(out->next != NULL)
		out->next->last = out;

	return out;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_RemoveReceiveHandlerNode
*   
* DESCRIPTION
*   Removes a receive handler node from the thread nodes list of receive
*   handlers and frees the memory.
*   This function requires mutual exclusive execution.
*
* CHANGES
*   -
*
******************************************************************************/

int POVMS_RemoveReceiveHandlerNode(POVMSThreadDataNode *tdn, POVMSReceiveHandlerNode *thn)
{
	int ret = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(tdn == NULL)
	{
		ret = kNullPointerErr;
	}
	else
	{
		if(thn == tdn->receivehandlerroot)
			tdn->receivehandlerroot = tdn->receivehandlerroot->next;

		if(thn->last != NULL)
			thn->last->next = thn->next;
		if(thn->next != NULL)
			thn->next->last = thn->last;

		POVMS_Free((void *)thn);
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_FindReceiveHandlerNode
*   
* DESCRIPTION
*   Search for a receive handler node by handles message class and id in the
*   list of thread nodes.
*   This function requires mutual exclusive execution.
*
* CHANGES
*   -
*
******************************************************************************/

POVMSReceiveHandlerNode *POVMS_FindReceiveHandlerNode(POVMSThreadDataNode *tdn, POVMSType hclass, POVMSType hid)
{
	POVMSReceiveHandlerNode *thn;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return NULL;

	for(thn = tdn->receivehandlerroot; thn != NULL; thn = thn->next)
	{
		if((thn->handledclass == hclass) && (thn->handledid == hid))
			break;
	}

	return thn;
}



/*****************************************************************************
*
* FUNCTION
*   POVMSMsg_SetupMessage
*   
* DESCRIPTION
*   Sets the message class and identifier for a given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetupMessage(POVMSObjectPtr object, POVMSType msgclass, POVMSType msgid)
{
	int ret;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	ret = POVMSMsg_SetMessageClass(object, msgclass);
	if(ret == kNoErr)
		ret = POVMSMsg_SetMessageIdentifier(object, msgid);
	if(ret == kNoErr)
		ret = POVMSMsg_SetMessageTargetThread(object, POVMS_GetCurrentThread());

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSMsg_GetMessageClass
*   
* DESCRIPTION
*   Gets the message class for a given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetMessageClass(POVMSObjectPtr object, POVMSType *msgclass)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;

	if(msgclass == NULL)
		return kParamErr;

	return POVMSUtil_GetType(object, kPOVMSMessageClassID, msgclass);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSMsg_SetMessageClass
*   
* DESCRIPTION
*   Sets the message class for a given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageClass(POVMSObjectPtr object, POVMSType msgclass)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;

	return POVMSUtil_SetType(object, kPOVMSMessageClassID, msgclass);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSMsg_GetMessageIdentifier
*   
* DESCRIPTION
*   Gets the message identifier for a given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetMessageIdentifier(POVMSObjectPtr object, POVMSType *msgid)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;

	if(msgid == NULL)
		return kParamErr;

	return POVMSUtil_GetType(object, kPOVMSMessageIdentID, msgid);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSMsg_SetMessageIdentifier
*   
* DESCRIPTION
*   Sets the message identifier for a given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageIdentifier(POVMSObjectPtr object, POVMSType msgid)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;

	return POVMSUtil_SetType(object, kPOVMSMessageIdentID, msgid);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSMsg_SetMessageTargetThread
*   
* DESCRIPTION
*   Sets the message target using the target thread id.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageTargetThread(POVMSObjectPtr object, POVMSThreadID tid)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;

	return POVMS_SetMsgThreadID(object, kPOVMSTargetAddressID, tid);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_New
*   
* DESCRIPTION
*   Setup of a new and empty object.
*   Remember that no data for the POVMSObject is allocated.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_New(POVMSObjectPtr object, POVMSType objclass)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;

	object->type = kPOVMSType_LockedObject;

	object->size = 0;
	object->root = NULL;

	object->type = kPOVMSType_Object;

	return POVMSUtil_SetType(object, kPOVMSObjectClassID, objclass);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Delete
*   
* DESCRIPTION
*   Deletes the given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Delete(POVMSObjectPtr object)
{
	POVMSNode *cur,*del;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;
	if(object->type == kPOVMSType_LockedObject)
		return kNotNowErr;

	object->type = kPOVMSType_LockedObject;

	for(cur = object->root; cur != NULL;)
	{
		del = cur;
		cur = cur->next;
		POVMSAttr_Delete(&del->data);

		POVMS_Free((void *)del);
	}

	object->type = kPOVMSType_Object;
	object->size = 0;
	object->root = NULL;

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Copy
*   
* DESCRIPTION
*   Copies the given object into the second given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Copy(POVMSObjectPtr sourceobject, POVMSObjectPtr targetobject)
{
	POVMSNode *cur = NULL;
	POVMSAttribute attr;
	POVMSType t;
	int ret = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(sourceobject == NULL)
		return kParamErr;
	if(targetobject == NULL)
		return kParamErr;

	if(POVMSUtil_GetType(sourceobject, kPOVMSObjectClassID, &t) != kNoErr)
		return kObjectAccessErr;

	if(POVMSObject_New(targetobject, t) != kNoErr)
		return kObjectAccessErr;

	for(cur = sourceobject->root; cur != NULL; cur = cur->next)
	{
		if(POVMS_ASSERT(POVMSAttr_Copy(&(cur->data), &attr) == kNoErr, "POVMSObject_Copy failed, out of memory") == false)
		{
			ret = kOutOfMemoryErr;
			break;
		}

		if(POVMS_ASSERT(POVMSObject_Set(targetobject, &attr, cur->key) == kNoErr, "POVMSObject_Copy failed, out of memory") == false)
		{
			ret = kOutOfMemoryErr;
			break;
		}
	}

	if(ret != kNoErr)
		(void)POVMSObject_Delete(targetobject);

	return ret;
}

/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Get
*   
* DESCRIPTION
*   Gets the attribute with the given key from the given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Get(POVMSObjectPtr object, POVMSAttributePtr attr, POVMSType key)
{
	POVMSNode *in = NULL;
	int ret;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kNoErr;

	if(object == NULL)
		return kNoErr;
	if(object->type == kPOVMSType_LockedObject)
		return kNotNowErr;
	if(attr == NULL)
		return kParamErr;

	object->type = kPOVMSType_LockedObject;

	in = POVMSObject_Find(object, key);
	if(in == NULL)
		ret = kParamErr;
	else
		ret = POVMSAttr_Copy(&(in->data), attr);

	object->type = kPOVMSType_Object;

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Set
*   
* DESCRIPTION
*   Sets the attribute with the given key of the given object. If the
*   attribute is not yet part of the object it will be added.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Set(POVMSObjectPtr object, POVMSAttributePtr attr, POVMSType key)
{
	POVMSNode *in = NULL;
	int ret;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;
	if(object->type == kPOVMSType_LockedObject)
		return kNotNowErr;
	if(attr == NULL)
		return kParamErr;

	object->type = kPOVMSType_LockedObject;

	in = POVMSObject_Find(object, key);
	if(in != NULL)
	{
		POVMSAttr_Delete(&in->data);

		in->data = *attr;

		ret = kNoErr;
	}
	else
	{
		in = (POVMSNode *)POVMS_Malloc(sizeof(POVMSNode));
		if(POVMS_ASSERT(in != NULL, "POVMSObject_Set failed, out of memory") == false)
			ret = kOutOfMemoryErr;
		else
		{
			in->last = NULL;
			in->next = object->root;
			in->data = *attr;
			in->key = key;
			if(in->next != NULL)
				in->next->last = in;

			object->root = in;

			object->size++;

			ret = kNoErr;
		}
	}

	object->type = kPOVMSType_Object;

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Remove
*   
* DESCRIPTION
*   Removes the attribute with the given key from the given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Remove(POVMSObjectPtr object, POVMSType key)
{
	POVMSNode *del;
	int ret = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;
	if(object->type == kPOVMSType_LockedObject)
		return kNotNowErr;

	object->type = kPOVMSType_LockedObject;

	del = POVMSObject_Find(object, key);
	if(del == NULL)
		ret = kParamErr;
	else
	{
		if(del->data.ptr != NULL)
			POVMSAttr_Delete(&del->data);

		if(del == object->root)
			object->root = object->root->next;
		if(del->last != NULL)
			del->last->next = del->next;
		if(del->next != NULL)
			del->next->last = del->last;

		object->size--;

		POVMS_Free((void *)del);
	}

	object->type = kPOVMSType_Object;

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Exist
*   
* DESCRIPTION
*   Determines if an attribute with the specified key is part of the given
*   object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Exist(POVMSObjectPtr object, POVMSType key)
{
	int ret = false;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;
	if(object->type == kPOVMSType_LockedObject)
		return kNotNowErr;

	object->type = kPOVMSType_LockedObject;

	if(POVMSObject_Find(object, key) != NULL)
		ret = kNoErr;
	else
		ret = kFalseErr;

	object->type = kPOVMSType_Object;

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Count
*   
* DESCRIPTION
*   Counts the attributes of the given object.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSObject_Count(POVMSObjectPtr object, long *cnt)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(object == NULL)
		return kParamErr;
	if(object->type == kPOVMSType_LockedObject)
		return kNotNowErr;
	if(cnt == NULL)
		return kParamErr;

	object->type = kPOVMSType_LockedObject;

	*cnt = object->size;

	object->type = kPOVMSType_Object;

	return kNoErr;
}



/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Find
*
* DESCRIPTION
*   Search for a given key in the given object and return a pointer to its node.
*
* CHANGES
*   -
*
******************************************************************************/

POVMSNode *POVMSObject_Find(POVMSObjectPtr object, POVMSType key)
{
	POVMSNode *cur = NULL;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return NULL;

	if(object == NULL)
		return NULL;

	for(cur = object->root; cur != NULL; cur = cur->next)
	{
		if(cur->key == key)
			return cur;
	}

	return NULL;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_Dump
*
* DESCRIPTION
*   Write the complete given object to the given file.
*
* CHANGES
*   -
*
******************************************************************************/

#ifndef POVMS_NO_DUMP_SUPPORT

int gPOVMSDumpLevel = 0;

POVMS_EXPORT int POVMS_CDECL POVMSObject_Dump(FILE *file, POVMSObjectPtr object)
{
	POVMSNode *cur = NULL;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(file == NULL)
		return kParamErr;
	if(object == NULL)
		return kParamErr;

	POVMSObject_DumpSpace(file); fprintf(file, "Object ");

	fprintf(file, "%c%c%c%c\n", (char)((object->type) >> 24)
							  , (char)((object->type) >> 16)
							  , (char)((object->type) >> 8)
							  , (char)((object->type)));

	POVMSObject_DumpSpace(file); fprintf(file, "{\n");

	gPOVMSDumpLevel++;

	for(cur = object->root; cur != NULL; cur = cur->next)
	{
		if((cur->data.type != kPOVMSType_Object) && (cur->data.type != kPOVMSType_LockedObject))
		{
			POVMSObject_DumpSpace(file);
			fprintf(file, "%c%c%c%c = ", (char)((cur->key) >> 24)
									   , (char)((cur->key) >> 16)
									   , (char)((cur->key) >> 8)
									   , (char)((cur->key)));

			(void)POVMSObject_DumpAttr(file, &(cur->data));
		}
		else
			(void)POVMSObject_Dump(file, &(cur->data));
	}

	gPOVMSDumpLevel--;

	POVMSObject_DumpSpace(file); fprintf(file, "}\n");

	return kNoErr;
}

#endif


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_DumpSpace
*
* DESCRIPTION
*   Write the given number of spaces to the given file.
*
* CHANGES
*   -
*
******************************************************************************/

#ifndef POVMS_NO_DUMP_SUPPORT

int POVMSObject_DumpSpace(FILE *file)
{
	int i;

	for(i = 0; i < gPOVMSDumpLevel; i++)
		fprintf(file, "  ");

	return kNoErr;
}

#endif


/*****************************************************************************
*
* FUNCTION
*   POVMSObject_DumpAttr
*
* DESCRIPTION
*   Write the given attribute to the given file.
*
* CHANGES
*   -
*
******************************************************************************/

#ifndef POVMS_NO_DUMP_SUPPORT

POVMS_EXPORT int POVMS_CDECL POVMSObject_DumpAttr(FILE *file, POVMSAttributePtr attr)
{
	POVMSNode *cur = NULL;
	int cnt;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(file == NULL)
		return kParamErr;
	if(attr == NULL)
		return kParamErr;

	fprintf(file, "(%c%c%c%c) ", (char)((attr->type) >> 24)
							   , (char)((attr->type) >> 16)
							   , (char)((attr->type) >> 8)
							   , (char)((attr->type)));

	switch(attr->type)
	{
		case kPOVMSType_List:
			fprintf(file, "List\n");
			POVMSObject_DumpSpace(file); fprintf(file, "{\n");

			gPOVMSDumpLevel++;

			for(cnt = 0; cnt < attr->size; cnt++)
			{
				if((attr->items[cnt].type != kPOVMSType_Object) && (attr->items[cnt].type != kPOVMSType_LockedObject))
				{
					POVMSObject_DumpSpace(file);
					(void)POVMSObject_DumpAttr(file, &(attr->items[cnt]));
				}
				else
					(void)POVMSObject_Dump(file, &(attr->items[cnt]));
			}

			gPOVMSDumpLevel--;

			POVMSObject_DumpSpace(file); fprintf(file, "}\n");
			break;
		case kPOVMSType_CString:
			fprintf(file, "\"%s\"\n", (char *)(attr->ptr));
			break;
		case kPOVMSType_Int:
			fprintf(file, "%d\n", (int)*((POVMSInt *)(attr->ptr)));
			break;
		case kPOVMSType_Long:
			unsigned long l;
			long h;

			GetPOVMSLong(&h, &l, (*((POVMSLong *)(attr->ptr))));
			fprintf(file, "%.8x%.8x\n", h, l);
			break;
		case kPOVMSType_Float:
			fprintf(file, "%f\n", (float)*((POVMSFloat *)(attr->ptr)));
			break;
		case kPOVMSType_Bool:
			if((*((unsigned char *)(attr->ptr))) != 0x00)
				fprintf(file, "true\n");
			else
				fprintf(file, "false\n");
			break;
		case kPOVMSType_Type:
			fprintf(file, "\'%c%c%c%c\'\n", (char)((*((unsigned long *)(attr->ptr))) >> 24)
										  , (char)((*((unsigned long *)(attr->ptr))) >> 16)
										  , (char)((*((unsigned long *)(attr->ptr))) >> 8)
										  , (char)((*((unsigned long *)(attr->ptr)))));
			break;
	}

	return kNoErr;
}

#endif


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_New
*   
* DESCRIPTION
*   Setup of a new and empty attribute.
*   Remember that no data for the POVMSAttribute is allocated.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_New(POVMSAttributePtr attr)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;

	attr->type = kPOVMSType_Null;
	attr->size = 0;
	attr->ptr = NULL;

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_Delete
*   
* DESCRIPTION
*   Deletes the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_Delete(POVMSAttributePtr attr)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;

	if(attr->type == kPOVMSType_Object)
	{
		POVMSObject_Delete(attr);
	}
	else if(attr->type == kPOVMSType_List)
	{
		POVMSAttrList_Delete(attr);
	}
	else if(attr->type == kPOVMSType_Address)
	{
		// NetPOVMS_DeleteAddress((POVMSAddressPtr)(attr->ptr));
		POVMS_Free((void *)(attr->ptr));
	}
	else if(attr->ptr != NULL)
		POVMS_Free((void *)(attr->ptr));

	attr->type = kPOVMSType_Null;
	attr->size = 0;
	attr->ptr = NULL;

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_Copy
*   
* DESCRIPTION
*   Copies the given attribute into the second given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_Copy(POVMSAttributePtr sourceattr, POVMSAttributePtr targetattr)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(sourceattr == NULL)
		return kParamErr;
	if(targetattr == NULL)
		return kParamErr;
	if(sourceattr->size < 0)
		return kParamErr;

	if(sourceattr->type == kPOVMSType_Object)
	{
		return POVMSObject_Copy(sourceattr, targetattr);
	}
	else if(sourceattr->type == kPOVMSType_List)
	{
		return POVMSAttrList_Copy(sourceattr, targetattr);
	}
	else
	{
		*targetattr = *sourceattr;

		if(sourceattr->ptr != NULL)
		{
			targetattr->ptr =(void *)POVMS_Malloc(sourceattr->size);
			if(POVMS_ASSERT(targetattr->ptr != NULL, "POVMSAttr_Copy failed, out of memory") == false)
				return kMemFullErr;

			POVMS_Memmove(targetattr->ptr, sourceattr->ptr, sourceattr->size);

			//if(sourceattr->type == kPOVMSType_Address)
			//	NetPOVMS_CopyAddress((POVMSAddressPtr)(sourceattr->ptr), (POVMSAddressPtr)(targetattr->ptr));
		}
	}

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_Get
*   
* DESCRIPTION
*   Gets the data of the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_Get(POVMSAttributePtr attr, POVMSType type, void *data, long *maxdatasize)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(data == NULL)
		return kParamErr;
	if(maxdatasize == NULL)
		return kParamErr;
	if(*maxdatasize < 0)
		return kParamErr;

	if(*maxdatasize < attr->size)
	{
		*maxdatasize = attr->size;
		return kInvalidDataSizeErr;
	}

	if(attr->type != type)
		return kDataTypeErr;

	POVMS_Memmove(data, attr->ptr, attr->size);
	*maxdatasize = attr->size;

	//if(attr->type == kPOVMSType_Address)
	//	return NetPOVMS_CopyAddress((POVMSAddressPtr)(attr->ptr), (POVMSAddressPtr)(data));

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_Set
*   
* DESCRIPTION
*   Sets the data of the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_Set(POVMSAttributePtr attr, POVMSType type, void *data, long datasize)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(data == NULL)
		return kParamErr;
	if(datasize < 0)
		return kParamErr;
	if(attr->ptr != NULL)
		return kParamErr;
	if(attr->size != 0)
		return kParamErr;

	attr->ptr = (void *)POVMS_Malloc(datasize);
	if(POVMS_ASSERT(attr->ptr != NULL, "POVMSAttr_Set failed, out of memory") == false)
		return kMemFullErr;

	//if(attr->type == kPOVMSType_Address)
	//	(void)NetPOVMS_DeleteAddress((POVMSAddressPtr)(attr->ptr));

	POVMS_Memmove(attr->ptr, data, datasize);
	attr->type = type;
	attr->size = datasize;

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_Size
*   
* DESCRIPTION
*   Gets the data size of the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_Size(POVMSAttributePtr attr, long *size)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(size == NULL)
		return kParamErr;

	*size = attr->size;

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttr_Type
*   
* DESCRIPTION
*   Gets the data type of the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttr_Type(POVMSAttributePtr attr, POVMSType *type)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(type == NULL)
		return kParamErr;

	*type = attr->type;

	return kNoErr;
}



/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_New
*   
* DESCRIPTION
*   Setup of a new and empty attribute list. Remember that no data for the
*   POVMSAttribute is allocated.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_New(POVMSAttributeListPtr attr)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;

	attr->type = kPOVMSType_List;
	attr->size = 0;
	attr->items = NULL;

	return kNoErr;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_Delete
*   
* DESCRIPTION
*   Deletes the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Delete(POVMSAttributeListPtr attr)
{
	int ret = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;

	if(attr->items != NULL)
		ret = POVMSAttrList_Clear(attr);

	attr->type = kPOVMSType_Null;
	attr->size = 0;
	attr->items = NULL;

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_Delete
*   
* DESCRIPTION
*   Deletes the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Copy(POVMSAttributeListPtr sourcelist, POVMSAttributeListPtr targetlist)
{
	long cnt;
	int err = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(sourcelist == NULL)
		return kParamErr;
	if(targetlist == NULL)
		return kParamErr;
	if(sourcelist->size < 0)
		return kParamErr;
	if(sourcelist->type != kPOVMSType_List)
		return kDataTypeErr;
	if((sourcelist->items == NULL) && (sourcelist->size > 0))
		return kParamErr;

	*targetlist = *sourcelist;

	if(sourcelist->size > 0)
	{
		if(sourcelist->ptr != NULL)
		{
			targetlist->ptr = (void *)POVMS_Malloc(sizeof(POVMSData) * sourcelist->size);
			if(POVMS_ASSERT(targetlist->ptr != NULL, "POVMSAttrList_Copy failed, out of memory") == false)
				return -1;
		}

		POVMS_Memmove(targetlist->ptr, sourcelist->ptr, sourcelist->size);

		for(cnt = 0; cnt < sourcelist->size; cnt++)
		{
			err = POVMSAttr_Copy(&(sourcelist->items[cnt]), &(targetlist->items[cnt]));
			if(err != kNoErr)
				break;
		}
		if(err != kNoErr)
		{
			for(cnt--; cnt >= 0; cnt--)
			{
				err = POVMSAttr_Delete(&(targetlist->items[cnt]));
				POVMS_ASSERT(err == kNoErr, "POVMSAttr_Delete in POVMSAttrList_Copy failed. Possible memory leak.");
			}
			POVMS_Free((void *)(targetlist->items));
			err = kObjectAccessErr;
		}
	}

	return err;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_Append
*   
* DESCRIPTION
*   Adds the data of the item to the end of the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Append(POVMSAttributeListPtr attr, POVMSAttributePtr item)
{
	POVMSData *temp_items;
	int err = kNoErr;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;
	if(item == NULL)
		return kNoErr;

	temp_items = (POVMSData *)POVMS_Realloc((void *)(attr->items), sizeof(POVMSData) * (attr->size + 1));
	if(POVMS_ASSERT(temp_items != NULL, "POVMSAttrList_Append failed, out of memory") == false)
	{
		err = kNoErr;
	}
	else
	{
		attr->items = temp_items;
		attr->items[attr->size] = *item;
		attr->size++;
	}

	return err;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_Remove
*   
* DESCRIPTION
*   Removes the last item in the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Remove(POVMSAttributeListPtr attr)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;
	if(attr->size <= 0)
		return kParamErr;
	if(attr->items == NULL)
		return kParamErr;

	return POVMSAttrList_RemoveNth(attr, attr->size);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_GetNth
*   
* DESCRIPTION
*   Gets the data of the item with the given index in the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_GetNth(POVMSAttributeListPtr attr, long index, POVMSAttributePtr item)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;
	if(item == NULL)
		return kParamErr;
	if(attr->items == NULL)
		return kParamErr;
	if(index < 1)
		return kParamErr;
	if(index > attr->size)
		return kParamErr;

	return POVMSAttr_Copy(&(attr->items[index - 1]), item);
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_SetNth
*   
* DESCRIPTION
*   Sets the data of the item with the given index in the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_SetNth(POVMSAttributeListPtr attr, long index, POVMSAttributePtr item)
{
	int err;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return -1;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;
	if(item == NULL)
		return kParamErr;
	if(attr->items == NULL)
		return kParamErr;
	if(index < 1)
		return kParamErr;
	if(index > attr->size)
		return kParamErr;

	err = POVMSAttr_Delete(&(attr->items[index - 1]));
	if(err == kNoErr)
		attr->items[index - 1] = *item;

	return err;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_RemoveNth
*   
* DESCRIPTION
*   Removes the item with the given index in the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_RemoveNth(POVMSAttributeListPtr attr, long index)
{
	POVMSData *temp_items;
	int err;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return -1;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;
	if(attr->items == NULL)
		return kParamErr;
	if(index < 1)
		return kParamErr;
	if(index > attr->size)
		return kParamErr;

	err = POVMSAttr_Delete(&(attr->items[index - 1]));
	if(err == kNoErr)
	{
		if(attr->size < index)
			POVMS_Memmove((void *)(&(attr->items[index - 1])), (void *)(&(attr->items[index])), sizeof(POVMSData) * (attr->size - index));
		temp_items = (POVMSData *)POVMS_Realloc((void *)(attr->items), sizeof(POVMSData) * (attr->size - 1));
		if(POVMS_ASSERT(attr->items != NULL, "POVMSAttrList_RemoveNth failed, out of memory") == false)
			err = kOutOfMemoryErr;
		else
			attr->items = temp_items;
		attr->size--;
	}

	return err;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_Clear
*   
* DESCRIPTION
*   Clears the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Clear(POVMSAttributeListPtr attr)
{
	long cnt;
	int err;

	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kDataTypeErr;
	if(attr->items == NULL)
		return kParamErr;
	if(attr->size <= 0)
		return kParamErr;

	for(cnt = attr->size - 1; cnt >= 0; cnt--)
	{
		err = POVMSAttr_Delete(&(attr->items[cnt]));
		POVMS_ASSERT(err == kNoErr, "POVMSAttr_Delete in POVMSAttrList_Clear failed. Possible memory leak.");
	}

	if(attr->items != NULL)
		POVMS_Free((void *)(attr->items));

	attr->type = kPOVMSType_Null;
	attr->size = 0;
	attr->items = NULL;

	return kNoErr; // don't return err
}


/*****************************************************************************
*
* FUNCTION
*   POVMSAttrList_Count
*   
* DESCRIPTION
*   Gets the number of items in the given attribute list.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Count(POVMSAttributeListPtr attr, long *cnt)
{
	if(POVMS_ASSERT(gPOVMSIsInitialized, "POVMS is not initialized") == false)
		return kParamErr;

	if(attr == NULL)
		return kParamErr;
	if(attr->type != kPOVMSType_List)
		return kParamErr;
	if(cnt == NULL)
		return kParamErr;

	*cnt = attr->size;

	return kNoErr;
}



/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetString
*   
* DESCRIPTION
*   Stores a string in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetString(POVMSObjectPtr object, POVMSType key, char *str) // Note: Strings may not contain \0 characters codes!
{
	POVMSAttribute attr;
	int ret;

	if(object == NULL)
		return kParamErr;
	if(str == NULL)
		return kParamErr;

	ret = POVMSAttr_New(&attr);
	if(ret == kNoErr)
		ret = POVMSAttr_Set(&attr, kPOVMSType_CString, (void *)str, POVMS_Strlen(str) + 1);
	if(ret == kNoErr)
		ret = POVMSObject_Set(object, &attr, key);

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetInt
*   
* DESCRIPTION
*  Stores an integer value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetInt(POVMSObjectPtr object, POVMSType key, POVMSInt value)
{
	POVMSAttribute attr;
	int ret;

	if(object == NULL)
		return kParamErr;

	ret = POVMSAttr_New(&attr);
	if(ret == kNoErr)
		ret = POVMSAttr_Set(&attr, kPOVMSType_Int, (void *)(&value), sizeof(POVMSInt));
	if(ret == kNoErr)
		ret = POVMSObject_Set(object, &attr, key);

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetLong
*   
* DESCRIPTION
*  Stores a long integer value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetLong(POVMSObjectPtr object, POVMSType key, POVMSLong value)
{
	POVMSAttribute attr;
	int ret;

	if(object == NULL)
		return kParamErr;

	ret = POVMSAttr_New(&attr);
	if(ret == kNoErr)
		ret = POVMSAttr_Set(&attr, kPOVMSType_Long, (void *)(&value), sizeof(POVMSLong));
	if(ret == kNoErr)
		ret = POVMSObject_Set(object, &attr, key);

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetFloat
*   
* DESCRIPTION
*   Stores a float value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetFloat(POVMSObjectPtr object, POVMSType key, POVMSFloat value)
{
	POVMSAttribute attr;
	int ret;

	if(object == NULL)
		return kParamErr;

	ret = POVMSAttr_New(&attr);
	if(ret == kNoErr)
		ret = POVMSAttr_Set(&attr, kPOVMSType_Float, (void *)(&value), sizeof(POVMSFloat));
	if(ret == kNoErr)
		ret = POVMSObject_Set(object, &attr, key);

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetBool
*   
* DESCRIPTION
*   Stores a bool value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetBool(POVMSObjectPtr object, POVMSType key, POVMSBool boolvalue)
{
	POVMSAttribute attr;
	int ret;
	unsigned char c;

	if(object == NULL)
		return kParamErr;

	if(boolvalue == false)
		c = 0x00;
	else
		c = 0xFF;

	ret = POVMSAttr_New(&attr);
	if(ret == kNoErr)
		ret = POVMSAttr_Set(&attr, kPOVMSType_Bool, (void *)(&c), sizeof(unsigned char));
	if(ret == kNoErr)
		ret = POVMSObject_Set(object, &attr, key);

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetType
*   
* DESCRIPTION
*  Stores an type value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetType(POVMSObjectPtr object, POVMSType key, POVMSType typevalue)
{
	POVMSAttribute attr;
	int ret;

	if(object == NULL)
		return kParamErr;

	ret = POVMSAttr_New(&attr);
	if(ret == kNoErr)
		ret = POVMSAttr_Set(&attr, kPOVMSType_Type, (void *)(&typevalue), sizeof(POVMSType));
	if(ret == kNoErr)
		ret = POVMSObject_Set(object, &attr, key);

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_GetString
*   
* DESCRIPTION
*   Accesses a string in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetString(POVMSObjectPtr object, POVMSType key, char *str, long *maxlen)
{
	POVMSAttribute attr;
	int ret,temp_ret;

	if(str == NULL)
		return kParamErr;
	if(maxlen == NULL)
		return kParamErr;

	ret = POVMSObject_Get(object, &attr, key);
	if(ret == kNoErr)
	{
		ret = POVMSAttr_Get(&attr, kPOVMSType_CString, (void *)str, maxlen);
		temp_ret = POVMSAttr_Delete(&attr);
		if(ret == kNoErr)
			ret = temp_ret;
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_GetInt
*   
* DESCRIPTION
*   Accesses an integer value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetInt(POVMSObjectPtr object, POVMSType key, POVMSInt *value)
{
	POVMSAttribute attr;
	long l = sizeof(POVMSInt);
	int ret,temp_ret;

	if(value == NULL)
		return kParamErr;

	ret = POVMSObject_Get(object, &attr, key);
	if(ret == kNoErr)
	{
		ret = POVMSAttr_Get(&attr, kPOVMSType_Int, (void *)value, &l);
		temp_ret = POVMSAttr_Delete(&attr);
		if(ret == 0)
			ret = temp_ret;
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_GetLong
*   
* DESCRIPTION
*   Accesses an integer value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetLong(POVMSObjectPtr object, POVMSType key, POVMSLong *value)
{
	POVMSAttribute attr;
	long l = sizeof(POVMSLong);
	int ret,temp_ret;

	if(value == NULL)
		return kParamErr;

	ret = POVMSObject_Get(object, &attr, key);
	if(ret == kNoErr)
	{
		ret = POVMSAttr_Get(&attr, kPOVMSType_Long, (void *)value, &l);
		temp_ret = POVMSAttr_Delete(&attr);
		if(ret == kNoErr)
			ret = temp_ret;
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_GetFloat
*   
* DESCRIPTION
*   Accesses a float value in the given attribute.
*   NOTE: It is legal to access an integer as float, it will be converted
*   automatically!
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetFloat(POVMSObjectPtr object, POVMSType key, POVMSFloat *value)
{
	POVMSAttribute attr;
	long l = sizeof(POVMSFloat);
	int ret,temp_ret;

	if(value == NULL)
		return kParamErr;

	ret = POVMSObject_Get(object, &attr, key);
	if(ret == kNoErr)
	{
		ret = POVMSAttr_Get(&attr, kPOVMSType_Float, (void *)value, &l);
		if(ret == kDataTypeErr)
		{
			POVMSInt i = 0;
			l = sizeof(POVMSInt);
			ret = POVMSAttr_Get(&attr, kPOVMSType_Int, (void *)(&i), &l);
			*value = POVMSFloat(i);
		}
		temp_ret = POVMSAttr_Delete(&attr);
		if(ret == kNoErr)
			ret = temp_ret;
	}

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_GetBool
*   
* DESCRIPTION
*   Accesses a bool value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetBool(POVMSObjectPtr object, POVMSType key, POVMSBool *boolvalue)
{
	POVMSAttribute attr;
	long l = sizeof(unsigned char);
	int ret,temp_ret;
	unsigned char c = 0x00;

	if(boolvalue == NULL)
		return kParamErr;

	ret = POVMSObject_Get(object, &attr, key);
	if(ret == kNoErr)
	{
		ret = POVMSAttr_Get(&attr, kPOVMSType_Bool, (void *)(&c), &l);
		temp_ret = POVMSAttr_Delete(&attr);
		if(ret == kNoErr)
			ret = temp_ret;
	}

	if(c == 0x00)
		*boolvalue = false;
	else
		*boolvalue = true;

	return ret;
}


/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_GetType
*   
* DESCRIPTION
*   Accesses a type value in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetType(POVMSObjectPtr object, POVMSType key, POVMSType *typevalue)
{
	POVMSAttribute attr;
	long l = sizeof(POVMSType);
	int ret,temp_ret;

	if(typevalue == NULL)
		return kParamErr;

	ret = POVMSObject_Get(object, &attr, key);
	if(ret == kNoErr)
	{
		ret = POVMSAttr_Get(&attr, kPOVMSType_Type, (void *)typevalue, &l);
		temp_ret = POVMSAttr_Delete(&attr);
		if(ret == kNoErr)
			ret = temp_ret;
	}

	return ret;
}
