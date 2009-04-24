/****************************************************************************
*                   povms.h
*
*  This module contains all defines, typedefs, and prototypes for povms.cpp.
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
* $File: //depot/povray/3.5/source/povms.h $
* $Revision: #13 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include "povmsgid.h"

#ifndef POVMS_H
#define POVMS_H

#ifdef POVMS_DISCONNECTED
#include "cnfpovms.h"
#endif

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Note: Byte order of the system this message was flattened on.
   This should be set in config.h! */
#ifndef POV_BYTE_ORDER
  #define POV_BYTE_ORDER '****'
#endif

/* Note: This low level fatal error text ouput function is required.
   It defaults to printf if you do not overload it in config.h! */
#ifndef POVMS_ASSERT_OUTPUT
  #define POVMS_ASSERT_OUTPUT(s,f,l) fprintf(stderr, "POVMS_ASSERT failed in %s line %ld: %s\n", f, l, s)
#endif

#ifndef POVMS_ASSERT
  #define POVMS_ASSERT(c,s) POVMS_AssertFunction(c, s, __FILE__, __LINE__)
#endif

/* Note: These low level functions allow building POVMS without a
   C library. Usually you do not have to override them! */
#ifndef POVMS_Strlen
  #define POVMS_Strlen(p)       strlen(p)
#endif

#ifndef POVMS_Memmove
  #define POVMS_Memmove(a, b, c)      memmove(a, b, c)
#endif

/* Note: For use between processes you may want to override these
   to use shared memory. Remember that the POVMS cannot use the
   standard POV_MALLOC, POV_CALLOC, POV_REALLOC, POV_FREE calls! */
#ifndef POVMS_Malloc
  #define POVMS_Malloc(s)       malloc(s)
#endif

#ifndef POVMS_Calloc
  #define POVMS_Calloc(m,s)     calloc(m,s)
#endif

#ifndef POVMS_Realloc
  #define POVMS_Realloc(p,s)    realloc(p,s)
#endif

#ifndef POVMS_Free
  #define POVMS_Free(p)         free(p)
#endif

/* Note: For use between processes you may want to override these
   to insure atomic (uninterrupted) operations. */
#ifndef POVMS_BeginAtomic
  #define POVMS_BeginAtomic()
#endif

#ifndef POVMS_EndAtomic
  #define POVMS_EndAtomic()
#endif

/* Note: For use between processes you may want to override these to
   insure receive handler functions are executed in the correct thread. */
#ifndef POVMSThreadID
  #define POVMSThreadID unsigned int
#endif

#ifndef POVMS_InitThreadID
  #define POVMS_InitThreadID() (0)
#endif

#ifndef POVMS_GetCurrentThread
  #define POVMS_GetCurrentThread() (0)
#endif

// return kFalseErr is false and kNoErr if true
#ifndef POVMS_CompareThreads
  #define POVMS_CompareThreads(th1,th2) (kNoErr)
#endif

#ifndef POVMS_SwitchToThread
  #define POVMS_SwitchToThread(th) (0)
#endif

#ifndef POVMS_GetTimeInSeconds
  #define POVMS_GetTimeInSeconds() (0)
#endif

#ifndef POVMS_GetMsgThreadID
  #define POVMS_GetMsgThreadID(object, key, th) (0); *(th) = 1
#endif

#ifndef POVMS_SetMsgThreadID
  #define POVMS_SetMsgThreadID(object, key, th) (0)
#endif

/* Note: POVMSType needs to be 32 bit! (unsigned), it will be composed
   of four characters, i.e. 'MyTp', 'any ', '4Mac', etc. */
#ifndef POVMSType
  #define POVMSType      unsigned long
#endif

#ifndef POVMSInt
  #define POVMSInt       int
#endif

#ifndef POVMSLong
  typedef struct SignedInt64_Struct SIGNEDINTEGER64;

  struct SignedInt64_Struct
  {
    long high;
    unsigned long low;
  };

  #define POVMSLong      SIGNEDINTEGER64

  #define SetPOVMSLong(v,h,l) (v)->high = h; (v)->low = l
  #define GetPOVMSLong(h,l,v) *h = v.high; *l = v.low
#endif

#ifndef POVMSFloat
  #define POVMSFloat     float
#endif

#ifndef POVMSBool
  #define POVMSBool      int
#endif

/* Note: Use POVMS_EXPORT if you need a special export keyword
   in order to use the POVMS functions as part of a library. */
#ifndef POVMS_EXPORT
  #define POVMS_EXPORT
#endif

#ifndef POVMS_CDECL
  #define POVMS_CDECL
#endif

#ifndef POVMS_THREADDATAROOT
  #define POVMS_THREADDATAROOT NULL
#endif

#undef POVMS_VERSION
#define POVMS_VERSION 1


/*****************************************************************************
* Global typedefs
******************************************************************************/

/* Note: All only upper case types are reserved for internal use. */
enum
{
    kPOVMSObjectClassID      = 'OCLA',
    kPOVMSMessageClassID     = 'MCLA',
    kPOVMSMessageIdentID     = 'MIDE',
    kPOVMSTargetAddressID    = 'MTRG',
    kPOVMSSourceAddressID    = 'MSRC',
    kPOVMSMessageTimeoutID   = 'TOUT',
    kPOVMSMessageErrorID     = 'MERR'
};

enum
{
    kPOVMSType_Object        = 'OBJE',
    kPOVMSType_LockedObject  = 'LOCK',
    kPOVMSType_Address       = 'ADDR',
    kPOVMSType_Null          = 'NULL',
    kPOVMSType_WildCard      = '****',
    kPOVMSType_CString       = 'CSTR',
    kPOVMSType_UnicodeText   = 'UTXT',
    kPOVMSType_Int           = 'INT4',
    kPOVMSType_Long          = 'INT8',
    kPOVMSType_Float         = 'SING',
    kPOVMSType_Bool          = 'BOOL',
    kPOVMSType_Type          = 'TYPE',
    kPOVMSType_List          = 'LIST'
};

typedef struct POVMSData POVMSObject, *POVMSObjectPtr;
typedef struct POVMSData POVMSAttribute, *POVMSAttributePtr;
typedef struct POVMSData POVMSAttributeList, *POVMSAttributeListPtr;
typedef struct POVMSNode POVMSNode;

struct POVMSData
{
    POVMSType type;
    long size;
    union
    {
        void *ptr;
        struct POVMSData *items;
        struct POVMSNode *root;
    };
};

struct POVMSNode
{
    struct POVMSNode *last;
    struct POVMSNode *next;
    POVMSType key;
    struct POVMSData data;
};

enum
{
    kPOVMSSendMode_NoReply = 1,
    kPOVMSSendMode_QueueReply = 2,
    kPOVMSSendMode_WaitReply = 3,
    kPOVMSSendMode_WantReceipt = 4
};


/*****************************************************************************
* Global variables
******************************************************************************/


/*****************************************************************************
* Global functions
******************************************************************************/

// POVMS setup and destroy functions
POVMS_EXPORT void * POVMS_CDECL POVMS_Initialize    (void *rootptr);
POVMS_EXPORT int POVMS_CDECL POVMS_Deinitialize     (void);

// Message send and receive functions
POVMS_EXPORT int POVMS_CDECL POVMS_InstallReceiver  (int (*hfunc)(POVMSObjectPtr, POVMSObjectPtr, int), POVMSType hclass, POVMSType hid);
POVMS_EXPORT int POVMS_CDECL POVMS_RemoveReceiver   (POVMSType hclass, POVMSType hid);
POVMS_EXPORT int POVMS_CDECL POVMS_ProcessMessages  (void);
POVMS_EXPORT int POVMS_CDECL POVMS_Send             (POVMSObjectPtr msg, POVMSObjectPtr result, int mode);

// Message data functions
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetupMessage          (POVMSObjectPtr object, POVMSType msgclass, POVMSType msgid);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetMessageClass       (POVMSObjectPtr object, POVMSType *msgclass);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageClass       (POVMSObjectPtr object, POVMSType msgclass);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetMessageIdentifier  (POVMSObjectPtr object, POVMSType *msgid);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageIdentifier  (POVMSObjectPtr object, POVMSType msgid);

// Message target address
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageTargetThread(POVMSObjectPtr object, POVMSThreadID tid);

// Object functions
POVMS_EXPORT int POVMS_CDECL POVMSObject_New        (POVMSObjectPtr object, POVMSType objclass);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Delete     (POVMSObjectPtr object);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Copy       (POVMSObjectPtr sourceobject, POVMSObjectPtr targetobject);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Get        (POVMSObjectPtr object, POVMSAttributePtr attr, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Set        (POVMSObjectPtr object, POVMSAttributePtr attr, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Remove     (POVMSObjectPtr object, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Exist      (POVMSObjectPtr object, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Count      (POVMSObjectPtr object, long *cnt);
POVMS_EXPORT int POVMS_CDECL POVMSObject_GetClass   (POVMSObjectPtr object, POVMSType *objclass);

#ifndef POVMS_NO_DUMP_SUPPORT
// Object debug functions
POVMS_EXPORT int POVMS_CDECL POVMSObject_Dump       (FILE *file, POVMSObjectPtr object);
POVMS_EXPORT int POVMS_CDECL POVMSObject_DumpAttr   (FILE *file, POVMSAttributePtr attr);
#endif

// Attribute functions
POVMS_EXPORT int POVMS_CDECL POVMSAttr_New          (POVMSAttributePtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Delete       (POVMSAttributePtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Copy         (POVMSAttributePtr sourceattr, POVMSAttributePtr targetattr);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Get          (POVMSAttributePtr attr, POVMSType type, void *data, long *maxdatasize);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Set          (POVMSAttributePtr attr, POVMSType type, void *data, long datasize);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Size         (POVMSAttributePtr attr, long *size);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Type         (POVMSAttributePtr attr, POVMSType *type);

// Attribute list functions
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_New      (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Delete   (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Copy     (POVMSAttributeListPtr sourcelist, POVMSAttributeListPtr targetlist);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Append   (POVMSAttributeListPtr attr, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Remove   (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_GetNth   (POVMSAttributeListPtr attr, long index, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_SetNth   (POVMSAttributeListPtr attr, long index, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_RemoveNth(POVMSAttributeListPtr attr, long index);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Clear    (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Count    (POVMSAttributeListPtr attr, long *cnt);

// Utility functions
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetString    (POVMSObjectPtr object, POVMSType key, char *str); // Note: Strings may not contain \0 characters codes!
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetInt       (POVMSObjectPtr object, POVMSType key, POVMSInt value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetLong      (POVMSObjectPtr object, POVMSType key, POVMSLong value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetFloat     (POVMSObjectPtr object, POVMSType key, POVMSFloat value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetBool      (POVMSObjectPtr object, POVMSType key, POVMSBool boolvalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetType      (POVMSObjectPtr object, POVMSType key, POVMSType typevalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetString    (POVMSObjectPtr object, POVMSType key, char *str, long *maxlen);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetInt       (POVMSObjectPtr object, POVMSType key, POVMSInt *value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetLong      (POVMSObjectPtr object, POVMSType key, POVMSLong *value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetFloat     (POVMSObjectPtr object, POVMSType key, POVMSFloat *value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetBool      (POVMSObjectPtr object, POVMSType key, POVMSBool *boolvalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetType      (POVMSObjectPtr object, POVMSType key, POVMSType *typevalue);

// POVMS low level fatal error checking and handling, not for export!
POVMSBool POVMS_AssertFunction(int cond, char *str, char *filename, long line);

#endif /* POVMS_H */
