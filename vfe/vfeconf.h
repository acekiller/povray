/*******************************************************************************
 * vfeconf.h
 *
 * This file contains vfe specific defines.
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
 * $File: //depot/povray/smp/vfe/vfeconf.h $
 * $Revision: #8 $
 * $Change: 4733 $
 * $DateTime: 2009/03/14 08:15:24 $
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

#ifndef __VFECONF_H__
#define __VFECONF_H__

//////////////////////////////////////////////////////////////
// POVMS support
/////////////////////////////////////////////////////////////

#define POVMSLong                             POV_LONG
#define POVMSBool                             bool
#define POVMSAddress                          void *
#define POVMSInvalidAddress                   NULL
#define POVMS_Sys_Thread_Type                 unsigned long

#include <string>
#include <cstdio>

namespace vfe
{
  typedef struct SysQDataNode POVMS_Sys_QueueDataNode ;
  typedef class SysQNode POVMS_Sys_QueueNode ;
  POVMSAddress vfe_POVMS_Sys_QueueToAddress (POVMS_Sys_QueueNode *q) ;
  POVMS_Sys_QueueNode *vfe_POVMS_Sys_AddressToQueue (POVMSAddress a) ;
  POVMS_Sys_QueueNode *vfe_POVMS_Sys_QueueOpen (void) ;
  void vfe_POVMS_Sys_QueueClose (POVMS_Sys_QueueNode *q) ;
  void *vfe_POVMS_Sys_QueueReceive (POVMS_Sys_QueueNode *q, int *l, bool, bool) ;
  int vfe_POVMS_Sys_QueueSend(POVMS_Sys_QueueNode *q, void *p, int l) ;
  int Allow_File_Write (const char *Filename, const unsigned int FileType);
  int Allow_File_Read (const char *Filename, const unsigned int FileType);
  int Allow_File_Read (const unsigned short *Filename, const unsigned int FileType);
  int Allow_File_Write (const unsigned short *Filename, const unsigned int FileType);
  POVMS_Sys_Thread_Type POVMS_GetCurrentThread();
  void vfeAssert (const char *message, const char *filename, int line) ;
  FILE *vfeFOpen (const std::basic_string<unsigned short>& name, const char *mode);
  bool vfeRemove (const std::basic_string<unsigned short>& name);

#if defined _DEBUG
  void *vfe_POVMS_Sys_Malloc(size_t size, const char *func, const char *file, int line) ;
  void *vfe_POVMS_Sys_Calloc(size_t nitems, size_t size, const char *func, const char *file, int line) ;
  void *vfe_POVMS_Sys_Realloc(void *ptr, size_t size, const char *func, const char *file, int line) ;
  void vfe_POVMS_Sys_Free(void *ptr, const char *func, const char *file, int line) ;
  void *vfe_POVMS_Sys_Malloc(size_t size) ;
  void *vfe_POVMS_Sys_Calloc(size_t nitems, size_t size) ;
  void *vfe_POVMS_Sys_Realloc(void *ptr, size_t size) ;
  void vfe_POVMS_Sys_Free(void *ptr) ;
#endif
}

// TODO: move these POVMS_Sys_* memory functions to their own heap
#ifndef _DEBUG
  #define POVMS_Sys_Malloc(s)                 malloc(s)
  #define POVMS_Sys_Calloc(m,s)               calloc(m,s)
  #define POVMS_Sys_Realloc(p,s)              realloc(p,s)
  #define POVMS_Sys_Free(p)                   free(p)
#else
  // a long timeout so we can break into the debugger
  #define kDefaultTimeout                     100

  #define POVMS_Sys_Malloc(s)                 vfe::vfe_POVMS_Sys_Malloc(s, __FUNCTION__, __FILE__, __LINE__)
  #define POVMS_Sys_Calloc(m,s)               vfe::vfe_POVMS_Sys_Calloc(m,s, __FUNCTION__, __FILE__, __LINE__)
  #define POVMS_Sys_Realloc(p,s)              vfe::vfe_POVMS_Sys_Realloc(p,s, __FUNCTION__, __FILE__, __LINE__)
  #define POVMS_Sys_Free(p)                   vfe::vfe_POVMS_Sys_Free(p, __FUNCTION__, __FILE__, __LINE__)
#endif

#define USE_SYSPROTO                          1
#define POV_DELAY_IMPLEMENTED                 1
#define POV_TIMER                             pov_base::vfeTimer
#define POV_SYS_THREAD_STARTUP                pov_base::vfeSysThreadStartup();
#define POV_SYS_THREAD_CLEANUP                pov_base::vfeSysThreadCleanup();
#define POV_PARSE_PATH_STRING(p,v,c,f)        pov_base::vfeParsePathString(p,v,c,f)

#define POVMS_ASSERT_OUTPUT                   vfe::vfeAssert
#define POVMS_Sys_Queue_Type                  vfe::POVMS_Sys_QueueNode *
#define POVMS_Sys_Queue_Type                  vfe::POVMS_Sys_QueueNode *
#define POVMS_Sys_QueueToAddress              vfe::vfe_POVMS_Sys_QueueToAddress
#define POVMS_Sys_AddressToQueue              vfe::vfe_POVMS_Sys_AddressToQueue
#define POVMS_Sys_QueueOpen                   vfe::vfe_POVMS_Sys_QueueOpen
#define POVMS_Sys_QueueClose                  vfe::vfe_POVMS_Sys_QueueClose
#define POVMS_Sys_QueueReceive                vfe::vfe_POVMS_Sys_QueueReceive
#define POVMS_Sys_QueueSend                   vfe::vfe_POVMS_Sys_QueueSend
#define POVMS_Sys_GetCurrentThread            vfe::POVMS_GetCurrentThread
#define POV_ALLOW_FILE_READ(f,t)              vfe::Allow_File_Read(f,t)
#define POV_ALLOW_FILE_WRITE(f,t)             vfe::Allow_File_Write(f,t)
#define POV_UCS2_FOPEN(n,m)                   vfe::vfeFOpen(n,m)
#define POV_UCS2_REMOVE(n)                    vfe::vfeRemove(n)

#ifndef SetPOVMSLong
  #define SetPOVMSLong(v,h,l)                 *v = (((((POVMSLong)(h)) & 0x00000000ffffffff) << 32) | (((POVMSLong)(l)) & 0x00000000ffffffff))
#endif
#ifndef GetPOVMSLong
  #define GetPOVMSLong(h,l,v)                 *h = ((v) >> 32) & 0x00000000ffffffff; *l = (v) & 0x00000000ffffffff
#endif
#ifndef POVMSLongToCDouble
  #define POVMSLongToCDouble(x)               double(x)
#endif

#endif // __VFECONF_H__
