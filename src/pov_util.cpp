/****************************************************************************
*                pov_util.cpp
*
*  This module implements misc utility functions.
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
* $File: //depot/povray/3.5/source/pov_util.cpp $
* $Revision: #9 $
* $Change: 1817 $
* $DateTime: 2002/07/27 10:45:37 $
* $Author: chrisc $
* $Log$
*
*****************************************************************************/

#include <ctype.h>
#include <stdarg.h>

#include "frame.h"
#include "pov_util.h"



/*****************************************************************************
*
* FUNCTION
*
*   pov_stricmp
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   Since the stricmp function isn't available on all systems, we've
*   provided a simplified version of it here.
*
* CHANGES
*
*   -
*
******************************************************************************/

int pov_stricmp(const char *s1, const char *s2)
{
  char c1, c2;

  while ((*s1 != '\0') && (*s2 != '\0'))
  {
    c1 = *s1++;
    c2 = *s2++;

    c1 = (char)toupper(c1);
    c2 = (char)toupper(c2);

    if (c1 < c2)
    {
      return(-1);
    }

    if (c1 > c2)
    {
      return(1);
    }
  }

  if (*s1 == '\0')
  {
    if (*s2 == '\0')
    {
      return(0);
    }
    else
    {
      return(-1);
    }
  }
  else
  {
    return(1);
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   POV_Std_Split_Time
*
* INPUT
*
* OUTPUT
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   Split time into hours, minutes and seconds.
*
* CHANGES
*
*   -
*
******************************************************************************/

void POV_Std_Split_Time(DBL time_dif, unsigned long *hrs, unsigned long *mins, DBL *secs)
{
  *hrs = (unsigned long)(time_dif / 3600.0);

  *mins = (unsigned long)((time_dif - (DBL)(*hrs * 3600)) / 60.0);

  *secs = time_dif - (DBL)(*hrs * 3600 + *mins * 60);
}



/*****************************************************************************
*
* FUNCTION
*
*   closest_power_of_2
*
* INPUT
*
*   theNumber - the value to determine closest power of 2 for.
*
* OUTPUT
*
* RETURNS
*
*   The closest power of two is returned, or zero if the
*   argument is less than or equal to zero.
*
* AUTHOR
*
*   Eduard Schwan
*
* DESCRIPTION
*
*   Decription: Find the highest positive power of 2 that is
*   less than or equal to the number passed.
*
*   Input  Output
*   -----  ------
*     0      0
*     1      1
*     2      2
*     3      2
*     8      8
*     9      8
*
* CHANGES
*
*   Aug 1994 : Created by Eduard.
*
******************************************************************************/

unsigned closest_power_of_2(unsigned theNumber)
{
  int PowerOf2Counter;

  /* do not handle zero or negative numbers for now */

  if (theNumber <= 0)
  {
    return(0);
  }

  /* count the number in question down as we count up a power of 2 */

  PowerOf2Counter = 1;

  while (theNumber > 1)
  {
    /* move our power of 2 counter bit up... */

    PowerOf2Counter <<= 1;

    /* and reduce our test number by a factor of 2 two */

    theNumber >>= 1;
  }

  return(PowerOf2Counter);
}



/*****************************************************************************
*
* FUNCTION
*   POVMSUtil_SetFormatString
*   
* DESCRIPTION
*   Stores a string with format information in the given attribute.
*
* CHANGES
*   -
*
******************************************************************************/

int POVMSUtil_SetFormatString(POVMSObjectPtr object, POVMSType key, char *format, ...) // Note: Strings may not contain \0 characters codes!
{
	va_list marker;
	char buffer[1024];

	va_start(marker, format);
	vsprintf(buffer, format, marker);
	va_end(marker);

	return POVMSUtil_SetString(object, key, buffer);
}
