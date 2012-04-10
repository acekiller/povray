/*******************************************************************************
 * povms.h
 *
 * This module contains all defines, typedefs, and prototypes for povms.cpp.
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
 * $File: //depot/povray/smp/source/base/povms.h $
 * $Revision: #18 $
 * $Change: 5094 $
 * $DateTime: 2010/08/07 06:03:14 $
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

#ifndef POVMS_H
#define POVMS_H

#ifndef POVMS_NO_DUMP_SUPPORT
	#include <stdio.h>
#endif

/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

/* Note: This low level fatal error text output function is required.
   It defaults to printf if you do not overload it in config.h! */
#ifndef POVMS_ASSERT_OUTPUT
	#define POVMS_ASSERT_OUTPUT(s,f,l) fprintf(stderr, "POVMS_ASSERT failed in %s line %d: %s\n", f, (int)l, s)
#endif

#ifndef POVMS_LOG_OUTPUT
	#define POVMS_LOG_OUTPUT(s)
#endif

/* Note: POVMSType needs to be 32 bit! (unsigned), it will be composed
   of four characters, i.e. 'MyTp', 'any ', '4Mac', etc. */
#ifndef POVMSType
	#define POVMSType      unsigned int
#endif

#ifndef POVMSInt
	#define POVMSInt       int
#endif

#ifndef POVMSLong
	#define POVMSLong      long long

	#define SetPOVMSLong(v,h,l) *v = (((((POVMSLong)(h)) & 0x00000000ffffffff) << 32) | (((POVMSLong)(l)) & 0x00000000ffffffff))
	#define GetPOVMSLong(h,l,v) *h = ((v) >> 32) & 0x00000000ffffffff; *l = (v) & 0x00000000ffffffff

	#define POVMSLongToCDouble(x) double(x)
#endif

#ifndef POVMSFloat
	#define POVMSFloat     float
#endif

#ifndef POVMSBool
	#define POVMSBool      int
#endif

#ifndef POVMSUCS2
	#define POVMSUCS2      unsigned short
#endif

#ifndef POVMSStream
	#define POVMSStream    unsigned char
#endif

#ifndef POVMS_NO_ORDERED_STREAM_DATA
	#ifndef POVMSIEEEFloat
		#define POVMSIEEEFloat float

		#define POVMSFloatToPOVMSIEEEFloat(p, f) f = p
		#define POVMSIEEEFloatToPOVMSFloat(f, p) p = f

		#define HexToPOVMSIEEEFloat(h, f) *((int *)(&f)) = h
	#endif
#else
	// POVMSIEEEFloat does not have to be an IEEE 754 float
	// if POVMS_NO_ORDERED_STREAM_DATA is off.  It only
	// has to be 32 bit in size!
	#ifndef POVMSIEEEFloat
		#define POVMSIEEEFloat POVMSFloat

		#define POVMSFloatToPOVMSIEEEFloat(p, f) f = p
		#define POVMSIEEEFloatToPOVMSFloat(f, p) p = f
	#endif
#endif

/* Note: POVMSAddress needs work with the default copy constructor.
   Adjust it to fit you message addressing needs! */

#ifndef POVMSAddress
	#define POVMSAddress void *
	#define POVMSInvalidAddress NULL
#endif

/* Note: Use POVMS_EXPORT if you need a special export keyword
   in order to use the POVMS functions as part of a library. */
#ifndef POVMS_EXPORT
	#define POVMS_EXPORT
#endif

#ifndef POVMS_CDECL
	#define POVMS_CDECL
#endif

#undef POVMS_VERSION
#define POVMS_VERSION 2


/*****************************************************************************
* Global typedefs
******************************************************************************/

/* Note: All only upper case types are reserved for internal use. */
enum
{
	kPOVMSObjectClassID         = 'OCLA',
	kPOVMSMessageClassID        = 'MCLA',
	kPOVMSMessageIdentID        = 'MIDE',
	kPOVMSSourceAddressID       = 'MSRC',
	kPOVMSDestinationAddressID  = 'MDST',
	kPOVMSMessageTimeoutID      = 'TOUT',
	kPOVMSMessageErrorID        = 'MERR',
	kPOVMSMessageSequenceID     = 'MSEQ',
	kPOVMSResultSequenceID      = 'RSEQ'
};

enum
{
	kPOVMSType_Object           = 'OBJE',
	kPOVMSType_LockedObject     = 'LOCK',
	kPOVMSType_ResultObject     = 'RESU',
	kPOVMSType_Address          = 'ADDR',
	kPOVMSType_Null             = 'NULL',
	kPOVMSType_WildCard         = '****',
	kPOVMSType_CString          = 'CSTR',
	kPOVMSType_UCS2String       = 'U2ST',
	kPOVMSType_Int              = 'INT4',
	kPOVMSType_Long             = 'INT8',
	kPOVMSType_Float            = 'FLT4',
	kPOVMSType_Double           = 'FLT8',
	kPOVMSType_Bool             = 'BOOL',
	kPOVMSType_Type             = 'TYPE',
	kPOVMSType_List             = 'LIST',
	kPOVMSType_VectorInt        = 'VIN4',
	kPOVMSType_VectorLong       = 'VIN8',
	kPOVMSType_VectorFloat      = 'VFL4',
	kPOVMSType_VectorType       = 'VTYP',
};

typedef void * POVMSContext;

typedef struct POVMSData POVMSObject, *POVMSObjectPtr;
typedef struct POVMSData POVMSAttribute, *POVMSAttributePtr;
typedef struct POVMSData POVMSAttributeList, *POVMSAttributeListPtr;
typedef struct POVMSNode POVMSNode;

struct POVMSData
{
	POVMSType type;
	int size;
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
	kPOVMSSendMode_Invalid = 0,
	kPOVMSSendMode_NoReply = 1,
	kPOVMSSendMode_WaitReply = 2,
	kPOVMSSendMode_WantReceipt = 3
};


/*****************************************************************************
* Global variables
******************************************************************************/


/*****************************************************************************
* Global functions
******************************************************************************/

// POVMS context functions
POVMS_EXPORT int POVMS_CDECL POVMS_OpenContext      (POVMSContext *contextrefptr);
POVMS_EXPORT int POVMS_CDECL POVMS_CloseContext     (POVMSContext contextref);
POVMS_EXPORT int POVMS_CDECL POVMS_GetContextAddress(POVMSContext contextref, POVMSAddress *addrptr);

// Message receive handler functions
POVMS_EXPORT int POVMS_CDECL POVMS_InstallReceiver  (POVMSContext contextref, int (*hfunc)(POVMSObjectPtr, POVMSObjectPtr, int, void *), POVMSType hclass, POVMSType hid, void *hpd);
POVMS_EXPORT int POVMS_CDECL POVMS_RemoveReceiver   (POVMSContext contextref, POVMSType hclass, POVMSType hid);

// Message receive functions
POVMS_EXPORT int POVMS_CDECL POVMS_ProcessMessages  (POVMSContext contextref, POVMSBool blocking, POVMSBool yielding);
POVMS_EXPORT int POVMS_CDECL POVMS_Receive          (POVMSContext contextref, POVMSObjectPtr msg, POVMSObjectPtr result, int mode);

// Message send functions
POVMS_EXPORT int POVMS_CDECL POVMS_Send             (POVMSContext contextref, POVMSObjectPtr msg, POVMSObjectPtr result, int mode);

// Message data functions
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetupMessage          (POVMSObjectPtr object, POVMSType msgclass, POVMSType msgid);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetMessageClass       (POVMSObjectPtr object, POVMSType *msgclass);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageClass       (POVMSObjectPtr object, POVMSType msgclass);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetMessageIdentifier  (POVMSObjectPtr object, POVMSType *msgid);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetMessageIdentifier  (POVMSObjectPtr object, POVMSType msgid);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetSourceAddress      (POVMSObjectPtr object, POVMSAddress *value);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetSourceAddress      (POVMSObjectPtr object, POVMSAddress value);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_GetDestinationAddress (POVMSObjectPtr object, POVMSAddress *value);
POVMS_EXPORT int POVMS_CDECL POVMSMsg_SetDestinationAddress (POVMSObjectPtr object, POVMSAddress value);

// Object functions
POVMS_EXPORT int POVMS_CDECL POVMSObject_New        (POVMSObjectPtr object, POVMSType objclass);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Delete     (POVMSObjectPtr object);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Copy       (POVMSObjectPtr sourceobject, POVMSObjectPtr destobject);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Merge      (POVMSObjectPtr sourceobject, POVMSObjectPtr destobject);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Get        (POVMSObjectPtr object, POVMSAttributePtr attr, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Set        (POVMSObjectPtr object, POVMSAttributePtr attr, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Remove     (POVMSObjectPtr object, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Exist      (POVMSObjectPtr object, POVMSType key);
POVMS_EXPORT int POVMS_CDECL POVMSObject_Count      (POVMSObjectPtr object, int  *cnt);
POVMS_EXPORT int POVMS_CDECL POVMSObject_GetClass   (POVMSObjectPtr object, POVMSType *objclass);

#ifndef POVMS_NO_DUMP_SUPPORT
// Object debug functions
POVMS_EXPORT int POVMS_CDECL POVMSObject_Dump       (FILE *file, POVMSObjectPtr object);
POVMS_EXPORT int POVMS_CDECL POVMSObject_DumpAttr   (FILE *file, POVMSAttributePtr attr);
#endif

// Object streaming functions
POVMS_EXPORT int POVMS_CDECL POVMSObjectStream_Size (POVMSObjectPtr object, int *streamsize);
POVMS_EXPORT int POVMS_CDECL POVMSObjectStream_Read (POVMSObjectPtr object, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSObjectStream_Write(POVMSObjectPtr object, POVMSStream *stream, int *maxstreamsize);

// Attribute functions
POVMS_EXPORT int POVMS_CDECL POVMSAttr_New          (POVMSAttributePtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Delete       (POVMSAttributePtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Copy         (POVMSAttributePtr sourceattr, POVMSAttributePtr destattr);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Get          (POVMSAttributePtr attr, POVMSType type, void *data, int *maxdatasize);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Set          (POVMSAttributePtr attr, POVMSType type, const void *data, int datasize);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Size         (POVMSAttributePtr attr, int *size);
POVMS_EXPORT int POVMS_CDECL POVMSAttr_Type         (POVMSAttributePtr attr, POVMSType *type);

// Attribute list functions
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_New      (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Delete   (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Copy     (POVMSAttributeListPtr sourcelist, POVMSAttributeListPtr destlist);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Append   (POVMSAttributeListPtr attr, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_AppendN  (POVMSAttributeListPtr attr, int cnt, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Remove   (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_GetNth   (POVMSAttributeListPtr attr, int index, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_SetNth   (POVMSAttributeListPtr attr, int index, POVMSAttributePtr item);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_RemoveNth(POVMSAttributeListPtr attr, int index);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Clear    (POVMSAttributeListPtr attr);
POVMS_EXPORT int POVMS_CDECL POVMSAttrList_Count    (POVMSAttributeListPtr attr, int *cnt);

// Utility functions
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetString    (POVMSObjectPtr object, POVMSType key, const char *str); // Note: Strings may not contain \0 characters codes!
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetUCS2String(POVMSObjectPtr object, POVMSType key, const POVMSUCS2 *str); // Note: Strings may not contain \0 characters codes!
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetInt       (POVMSObjectPtr object, POVMSType key, POVMSInt value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetLong      (POVMSObjectPtr object, POVMSType key, POVMSLong value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetFloat     (POVMSObjectPtr object, POVMSType key, POVMSFloat value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetBool      (POVMSObjectPtr object, POVMSType key, POVMSBool boolvalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_SetType      (POVMSObjectPtr object, POVMSType key, POVMSType typevalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetStringLength(POVMSObjectPtr object, POVMSType key, int *len); // Note: Includes trailing \0 character code!
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetString    (POVMSObjectPtr object, POVMSType key, char *str, int *maxlen);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetUCS2StringLength(POVMSObjectPtr object, POVMSType key, int *len); // Note: Includes trailing \0 character code!
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetUCS2String(POVMSObjectPtr object, POVMSType key, POVMSUCS2 *str, int *maxlen);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetInt       (POVMSObjectPtr object, POVMSType key, POVMSInt *value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetLong      (POVMSObjectPtr object, POVMSType key, POVMSLong *value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetFloat     (POVMSObjectPtr object, POVMSType key, POVMSFloat *value);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetBool      (POVMSObjectPtr object, POVMSType key, POVMSBool *boolvalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_GetType      (POVMSObjectPtr object, POVMSType key, POVMSType *typevalue);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_TempAlloc    (void **ptr, int datasize);
POVMS_EXPORT int POVMS_CDECL POVMSUtil_TempFree     (void *ptr);

// Memory debug functions
#ifdef _DEBUG_POVMS_TRACE_MEMORY_
POVMS_EXPORT int POVMS_TraceDump                    ();
#endif

#endif /* POVMS_H */

#ifdef POVMSCPP_H
	#define POVMS_EXPORT_STREAM_FUNCTIONS
#endif

// Stream functions only available to the C++ interface
#ifdef POVMS_EXPORT_STREAM_FUNCTIONS

// Stream reading functions
POVMS_EXPORT int POVMS_CDECL POVMSStream_ReadString         (char *data, POVMSStream *stream, int datasize, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_ReadUCS2String     (POVMSUCS2 *data, POVMSStream *stream, int datasize, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_ReadInt            (POVMSInt *data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_ReadLong           (POVMSLong *data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_ReadFloat          (POVMSFloat *data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_ReadType           (POVMSType *data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_Read               (struct POVMSData *data, POVMSStream *stream, int *maxstreamsize);

// Stream writing functions
POVMS_EXPORT int POVMS_CDECL POVMSStream_WriteString        (const char *data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_WriteUCS2String    (const POVMSUCS2 *data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_WriteInt           (POVMSInt data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_WriteLong          (POVMSLong data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_WriteFloat         (POVMSFloat data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_WriteType          (POVMSType data, POVMSStream *stream, int *maxstreamsize);
POVMS_EXPORT int POVMS_CDECL POVMSStream_Write              (struct POVMSData *data, POVMSStream *stream, int *maxstreamsize);

// Stream utility functions
POVMS_EXPORT int POVMS_CDECL POVMSStream_Size               (struct POVMSData *data);
POVMS_EXPORT int POVMS_CDECL POVMSStream_CheckMessageHeader (POVMSStream *stream, int streamsize, int *totalsize);

#endif
