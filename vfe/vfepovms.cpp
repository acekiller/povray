/*******************************************************************************
 * vfepovms.cpp
 *
 * This module implements POVMS message handling functionality
 *
 * Author: Christopher J. Cason
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 2005-2008 Persistence of Vision Raytracer Pty. Ltd.
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
 * $File: //depot/povray/smp/vfe/vfepovms.cpp $
 * $Revision: #9 $
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

#include "vfe.h"

// this must be the last file included
#include "syspovdebug.h"

namespace vfe
{

using namespace pov;
using namespace pov_base;

class SysQNode
{
  public:
    SysQNode();
    ~SysQNode();

    int Send (void *pData, int Len) ;
    void *Receive (int *pLen, bool Blocking) ;

  private:
    typedef struct _DataNode
    {
      unsigned int              Len;
      void                      *Data;
      _DataNode                 *Next;
    } DataNode ;

    unsigned int                m_Sanity ;
    unsigned int                m_Count ;
    unsigned int                m_ID ;
    DataNode                    *m_First ;
    DataNode                    *m_Last ;
    boost::mutex                m_EventMutex ;
    boost::condition            m_Event ;

    static unsigned int         QueueID ;
} ;

////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////

unsigned int SysQNode::QueueID ;

extern volatile POVMSContext POVMS_Render_Context ;

////////////////////////////////////////////////////////////////////
// memory allocation for POVMS (need to move this to its own heap).
////////////////////////////////////////////////////////////////////

void *vfe_POVMS_Sys_Malloc(size_t size, const char *func, const char *file, int line)
{
  return malloc (size) ;
}

void *vfe_POVMS_Sys_Calloc(size_t nitems, size_t size, const char *func, const char *file, int line)
{
  return calloc (nitems, size) ;
}

void *vfe_POVMS_Sys_Realloc(void *ptr, size_t size, const char *func, const char *file, int line)
{
  return realloc (ptr, size) ;
}

void vfe_POVMS_Sys_Free(void *ptr, const char *func, const char *file, int line)
{
  free (ptr) ;
}

////////////////////////////////////////////////////////////////////
// error handling
////////////////////////////////////////////////////////////////////

void vfeAssert (const char *message, const char *filename, int line)
{
  throw vfeCriticalError(message, filename, line);
}

////////////////////////////////////////////////////////////////////
// class SysQNode
////////////////////////////////////////////////////////////////////

SysQNode::SysQNode (void)
{
  m_Sanity = 0xEDFEEFBE ;
  m_Count = 0 ;
  m_First = NULL ;
  m_Last = NULL ;
  m_ID = QueueID++ ;
}

SysQNode::~SysQNode ()
{
  assert (m_Sanity == 0xEDFEEFBE) ;
  m_Event.notify_all ();
  boost::mutex::scoped_lock lock (m_EventMutex);
  if (m_Count > 0)
  {
    DataNode *current = m_First ;
    DataNode *next = NULL ;

    while (current != NULL)
    {
      next = current->Next ;
      POVMS_Sys_Free (current) ;
      current = next ;
    }
  }
}

int SysQNode::Send (void *pData, int Len)
{
  assert (m_Sanity == 0xEDFEEFBE) ;
  if (m_Sanity == 0xEDFEEFBE)
  {
    DataNode *dNode = (DataNode *) POVMS_Sys_Malloc (sizeof (DataNode)) ;
    if (dNode == NULL)
      return (-3) ;

    dNode->Data = pData ;
    dNode->Len = Len ;
    dNode->Next = NULL ;

    boost::mutex::scoped_lock lock (m_EventMutex) ;

    if (m_Last != NULL)
      m_Last->Next = dNode ;
    if (m_First == NULL)
      m_First = dNode ;
    m_Last = dNode ;
    m_Count++ ;
  }
  else
    return (-2) ;

  m_Event.notify_one ();
  return (0) ;
}

void *SysQNode::Receive (int *pLen, bool Blocking)
{
  boost::mutex::scoped_lock lock (m_EventMutex);

  assert (m_Sanity == 0xEDFEEFBE) ;
  if (m_Sanity != 0xEDFEEFBE)
    throw vfeInvalidDataError("Invalid sanity field in SysQNode::Receive");

  if (m_Count == 0)
  {
    if (Blocking == false)
      return (NULL);

    // TODO: have a shorter wait but loop, and check for system shutdown
    boost::xtime t;
    boost::xtime_get (&t, boost::TIME_UTC);
    t.nsec += 50000000 ;
    m_Event.timed_wait (lock, t);

    if (m_Count == 0)
      return (NULL) ;
  }

  DataNode *dNode = m_First ;
  if (dNode == NULL)
    throw vfeInvalidDataError("NULL data node in SysQNode::Receive");

  void *dPtr = dNode->Data ;
  *pLen = dNode->Len ;
  if (dNode == m_Last)
    m_Last = NULL ;
  m_First = dNode->Next ;
  m_Count-- ;
  assert (m_Count != 0 || (m_First == NULL && m_Last == NULL)) ;
  POVMS_Sys_Free (dNode) ;
  return (dPtr) ;
}

////////////////////////////////////////////////////////////////////
// POVMS queue support code
////////////////////////////////////////////////////////////////////

bool POVMS_Init (void)
{
  return (true) ;
}

void POVMS_Shutdown (void)
{
  // TODO: should keep track of open queues and delete them here
}

POVMSAddress vfe_POVMS_Sys_QueueToAddress (SysQNode *Node)
{
  return ((POVMSAddress) Node) ;
}

SysQNode *vfe_POVMS_Sys_AddressToQueue (POVMSAddress Addr)
{
  return ((SysQNode *) Addr) ;
}

SysQNode *vfe_POVMS_Sys_QueueOpen (void)
{
  return (new SysQNode) ;
}

void vfe_POVMS_Sys_QueueClose (SysQNode *SysQ)
{
  delete SysQ ;
}

int vfe_POVMS_Sys_QueueSend (SysQNode *SysQ, void *pData, int Len)
{
  if (SysQ == NULL)
    return (-1) ;
  return (SysQ->Send (pData, Len)) ;
}

void *vfe_POVMS_Sys_QueueReceive (SysQNode *SysQ, int *pLen, bool Blocking, bool Yielding)
{
  if (pLen == NULL)
    return (NULL) ;
  *pLen = 0 ;
  if (SysQ == NULL)
  {
    if (Yielding)
      Delay (1) ;
    return (NULL) ;
  }
  return (SysQ->Receive (pLen, Blocking)) ;
}

POVMS_Sys_Thread_Type POVMS_GetCurrentThread (void)
{
  return (vfePlatform::GetThreadId ()) ;
}

}
