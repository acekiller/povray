/****************************************************************************
*                   povmscpp.h
*
*  This module contains all defines, typedefs, and prototypes for the
*  C++ interface version of povms.cpp.
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
* $File: //depot/povray/3.5/source/povmscpp.h $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include "povms.h"

#ifndef POVMSCPP_H
#define POVMSCPP_H

/*****************************************************************************
* Global classes
******************************************************************************/

class POVMS_Container;
class POVMS_Attribute;
class POVMS_List;
class POVMS_Object;
class POVMS_Message;

class POVMS_Container
{
		friend void POVMS_SendMessage(POVMS_Message&, POVMS_Message *, int);
		friend class POVMS_List;
		friend class POVMS_Object;
	public:
		POVMS_Container();
		virtual ~POVMS_Container();

		POVMSType Type();
		long Size();
	protected:
		POVMSData data;

		void DetachData();
};

class POVMS_Attribute : public POVMS_Container
{
	public:
		POVMS_Attribute();
		POVMS_Attribute(POVMSAttribute& convert);
		POVMS_Attribute(POVMS_Attribute& source);
		virtual ~POVMS_Attribute();

		void Get(POVMSType type, void *data, long *maxdatasize);
		void Set(POVMSType type, void *data, long datasize);

		void SetString(POVMSType key, char *str); // Note: Strings may not contain \0 characters codes!
		void SetInt(POVMSType key, POVMSInt value);
		void SetLong(POVMSType key, POVMSLong value);
		void SetFloat(POVMSType key, POVMSFloat value);
		void SetBool(POVMSType key, POVMSBool boolvalue);
		void SetType(POVMSType key, POVMSType typevalue);

		void GetString(POVMSType key, char *str, long *maxlen);
		POVMSInt GetInt(POVMSType key);
		POVMSLong GetLong(POVMSType key);
		POVMSFloat GetFloat(POVMSType key);
		POVMSBool GetBool(POVMSType key);
		POVMSType GetType(POVMSType key);
};

class POVMS_List : public POVMS_Container
{
	public:
		POVMS_List();
		POVMS_List(POVMSAttributeList& convert);
		POVMS_List(POVMS_List& source);
		virtual ~POVMS_List();

		void Append(POVMS_Container& item);
		void GetNth(long index, POVMS_Container& item);
		void SetNth(long index, POVMS_Container& item);
		void RemoveNth(long index);
		void Clear();
};

class POVMS_Object : public POVMS_Container
{
	public:
		POVMS_Object(POVMSType objclass = kPOVMSType_WildCard);
		POVMS_Object(POVMSObject& convert);
		POVMS_Object(POVMS_Object& source);
		~POVMS_Object();

		void Get(POVMS_Container& attr, POVMSType key);
		void Set(POVMS_Container& attr, POVMSType key);
		void Remove(POVMSType key);
		void Exist(POVMSType key);
};

class POVMS_Message : public POVMS_Object
{
	public:
		POVMS_Message(POVMSType objclass = kPOVMSType_Null, POVMSType msgclass = kPOVMSType_WildCard, POVMSType msgid = kPOVMSType_WildCard);
		POVMS_Message(POVMSObject& convert);
		POVMS_Message(POVMS_Message& source);

		POVMSType GetClass();
		POVMSType GetIdentifier();
};

void POVMS_SendMessage(POVMS_Message& msg, POVMS_Message *result, int mode);

#endif
