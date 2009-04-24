/****************************************************************************
*                povmscpp.cpp
*
*  This module contains the C++ interface version of povms.cpp
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
* $File: //depot/povray/3.5/source/povmscpp.cpp $
* $Revision: #10 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include "frame.h"
#include "povmscpp.h"
#include "pov_err.h"


/*****************************************************************************
*
* CLASS
*   POVMS_Container
*   
* DESCRIPTION
*   Base class for all other POVMS classes.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_Container::POVMS_Container()
{
	data.type = kPOVMSType_Null;
	data.size = 0;
	data.ptr = NULL;
}

POVMS_Container::~POVMS_Container()
{
	// do nothing for now
}

POVMSType POVMS_Container::Type()
{
	return data.type;
}

long POVMS_Container::Size()
{
	return data.size;
}

void POVMS_Container::DetachData()
{
	data.type = kPOVMSType_Null;
	data.size = 0;
	data.ptr = NULL;
}


/*****************************************************************************
*
* CLASS
*   POVMS_Attribute
*   
* DESCRIPTION
*   Class handling POVMSAttribute.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_Attribute::POVMS_Attribute()
{
	int err;

	err = POVMSAttr_New(&data);
	if(err != kNoErr)
		throw err;
}

POVMS_Attribute::POVMS_Attribute(POVMSAttribute& convert)
{
	data = convert;
}

POVMS_Attribute::POVMS_Attribute(POVMS_Attribute& source)
{
	int err;

	err = POVMSAttr_Copy(&source.data, &data);
	if(err != kNoErr)
		throw err;
}

POVMS_Attribute::~POVMS_Attribute()
{
	int err;

	err = POVMSAttr_Delete(&data);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::Get(POVMSType type, void *data, long *maxdatasize)
{
	int err;

	err = POVMSAttr_Get(&this->data, type, data, maxdatasize);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::Set(POVMSType type, void *data, long datasize)
{
	int err;

	err = POVMSAttr_Set(&this->data, type, data, datasize);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::SetString(POVMSType key, char *str)
{
	int err;

	err = POVMSUtil_SetString(&data, key, str);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::SetInt(POVMSType key, POVMSInt value)
{
	int err;

	err = POVMSUtil_SetInt(&data, key, value);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::SetLong(POVMSType key, POVMSLong value)
{
	int err;

	err = POVMSUtil_SetLong(&data, key, value);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::SetFloat(POVMSType key, POVMSFloat value)
{
	int err;

	err = POVMSUtil_SetFloat(&data, key, value);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::SetBool(POVMSType key, POVMSBool boolvalue)
{
	int err;

	err = POVMSUtil_SetBool(&data, key, boolvalue);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::SetType(POVMSType key, POVMSType typevalue)
{
	int err;

	err = POVMSUtil_SetType(&data, key, typevalue);
	if(err != kNoErr)
		throw err;
}

void POVMS_Attribute::GetString(POVMSType key, char *str, long *maxlen)
{
	int err;

	err = POVMSUtil_GetString(&data, key, str, maxlen);
	if(err != kNoErr)
		throw err;
}

POVMSInt POVMS_Attribute::GetInt(POVMSType key)
{
	POVMSInt ret;
	int err;

	err = POVMSUtil_GetInt(&data, key, &ret);
	if(err != kNoErr)
		throw err;

	return ret;
}

POVMSLong POVMS_Attribute::GetLong(POVMSType key)
{
	POVMSLong ret;
	int err;

	err = POVMSUtil_GetLong(&data, key, &ret);
	if(err != kNoErr)
		throw err;

	return ret;
}

POVMSFloat POVMS_Attribute::GetFloat(POVMSType key)
{
	POVMSFloat ret;
	int err;

	err = POVMSUtil_GetFloat(&data, key, &ret);
	if(err != kNoErr)
		throw err;

	return ret;
}

POVMSBool POVMS_Attribute::GetBool(POVMSType key)
{
	POVMSBool ret;
	int err;

	err = POVMSUtil_GetBool(&data, key, &ret);
	if(err != kNoErr)
		throw err;

	return ret;
}

POVMSType POVMS_Attribute::GetType(POVMSType key)
{
	POVMSType ret;
	int err;

	err = POVMSUtil_GetType(&data, key, &ret);
	if(err != kNoErr)
		throw err;

	return ret;
}


/*****************************************************************************
*
* CLASS
*   POVMS_List
*   
* DESCRIPTION
*   Class handling POVMSAttributeList.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_List::POVMS_List()
{
	int err;

	err = POVMSAttrList_New(&data);
	if(err != kNoErr)
		throw err;
}

POVMS_List::POVMS_List(POVMSAttributeList& convert)
{
	data = convert;
}

POVMS_List::POVMS_List(POVMS_List& source)
{
	int err;

	err = POVMSAttrList_Copy(&source.data, &data);
	if(err != kNoErr)
		throw err;
}

POVMS_List::~POVMS_List()
{
	int err;

	err = POVMSAttrList_Delete(&data);
	if(err != kNoErr)
		throw err;
}

void POVMS_List::Append(POVMS_Container& item)
{
	int err;

	err = POVMSAttrList_Append(&data, &item.data);
	if(err != kNoErr)
		throw err;

	item.DetachData();
}

void POVMS_List::GetNth(long index, POVMS_Container& item)
{
	int err;

	err = POVMSAttrList_GetNth(&data, index, &item.data);
	if(err != kNoErr)
		throw err;
}

void POVMS_List::SetNth(long index, POVMS_Container& item)
{
	int err;

	err = POVMSAttrList_SetNth(&data, index, &item.data);
	if(err != kNoErr)
		throw err;

	item.DetachData();
}

void POVMS_List::RemoveNth(long index)
{
	int err;

	err = POVMSAttrList_RemoveNth(&data, index);
	if(err != kNoErr)
		throw err;
}

void POVMS_List::Clear()
{
	int err;

	err = POVMSAttrList_Clear(&data);
	if(err != kNoErr)
		throw err;
}


/*****************************************************************************
*
* CLASS
*   POVMS_Object
*   
* DESCRIPTION
*   Class handling POVMSObject.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_Object::POVMS_Object(POVMSType objclass)
{
	int err;

	err = POVMSObject_New(&data, objclass);
	if(err != kNoErr)
		throw err;
}

POVMS_Object::POVMS_Object(POVMSObject& convert)
{
	data = convert;
}

POVMS_Object::POVMS_Object(POVMS_Object& source)
{
	int err;

	err = POVMSObject_Copy(&source.data, &data);
	if(err != kNoErr)
		throw err;
}

POVMS_Object::~POVMS_Object()
{
	int err;

	err = POVMSObject_Delete(&data);
	if(err != kNoErr)
		throw err;
}

void POVMS_Object::Get(POVMS_Container& attr, POVMSType key)
{
	int err;

	err = POVMSObject_Get(&data, &attr.data, key);
	if(err != kNoErr)
		throw err;
}

void POVMS_Object::Set(POVMS_Container& attr, POVMSType key)
{
	int err;

	err = POVMSObject_Set(&data, &attr.data, key);
	if(err != kNoErr)
		throw err;

	attr.DetachData();
}

void POVMS_Object::Remove(POVMSType key)
{
	int err;

	err = POVMSObject_Remove(&data, key);
	if(err != kNoErr)
		throw err;
}

void POVMS_Object::Exist(POVMSType key)
{
	int err;

	err = POVMSObject_Exist(&data, key);
	if(err != kNoErr)
		throw err;
}


/*****************************************************************************
*
* CLASS
*   POVMS_Message
*   
* DESCRIPTION
*   Class handling messaages contained in POVMSObjects.
*
* CHANGES
*   -
*
******************************************************************************/

POVMS_Message::POVMS_Message(POVMSType objclass, POVMSType msgclass, POVMSType msgid) : POVMS_Object(objclass)
{
	int err;

	err = POVMSMsg_SetupMessage(&data, msgclass, msgid);
	if(err != kNoErr)
		throw err;
}

POVMS_Message::POVMS_Message(POVMSObject& convert)
{
	data = convert;
}

POVMS_Message::POVMS_Message(POVMS_Message& /*source*/)
{
	// do nothing for now
}

POVMSType POVMS_Message::GetClass()
{
	POVMSType type;
	int err;

	err = POVMSMsg_GetMessageClass(&data, &type);
	if(err != kNoErr)
		throw err;

	return type;
}

POVMSType POVMS_Message::GetIdentifier()
{
	POVMSType type;
	int err;

	err = POVMSMsg_GetMessageIdentifier(&data, &type);
	if(err != kNoErr)
		throw err;

	return type;
}


/*****************************************************************************
*
* FUNCTION
*   POVMS_Send
*   
* DESCRIPTION
*   POVMS_SendMessage same as POVMS_Send in povms.c, but takes
*   POVMS_Messages objects as arguments.
*
* CHANGES
*   -
*
******************************************************************************/

void POVMS_SendMessage(POVMS_Message& msg, POVMS_Message *result, int mode)
{
	int err;

	if(result != NULL)
		err = POVMS_Send(&msg.data, &result->data, mode);
	else
		err = POVMS_Send(&msg.data, NULL, mode);

	if(err != kNoErr)
		throw err;

	msg.DetachData();
}
