/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/**********************************************************************
   module: DPMI.C

   author: James R. Dose
   date:   April 8, 1994

   Functions for performing DPMI calls.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "dpmi.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

/*---------------------------------------------------------------------
   Function: DPMI_GetRealModeVector

   Returns the vector of a real mode interrupt.
---------------------------------------------------------------------*/

unsigned long DPMI_GetRealModeVector
   (
   int num
   )

   {
   return 0;
   }


/*---------------------------------------------------------------------
   Function: DPMI_SetRealModeVector

   Sets the vector of a real mode interrupt.
---------------------------------------------------------------------*/

void DPMI_SetRealModeVector
   (
   int num,
   unsigned long vector
   )

   {
   }


/*---------------------------------------------------------------------
   Function: DPMI_CallRealModeFunction

   Performs a call to a real mode function.
---------------------------------------------------------------------*/

int DPMI_CallRealModeFunction
   (
   dpmi_regs *callregs
   )

   {
   return( DPMI_Ok );
   }

int DPMI_GetDOSMemory( void **ptr, long *descriptor, unsigned length )
{
	/* Lovely... */
	
	*ptr = (void *)malloc(length);
	
	*descriptor = (long) *ptr;
	
	return (descriptor == 0) ? DPMI_Error : DPMI_Ok;
}

int DPMI_FreeDOSMemory( long descriptor )
{
	free((void *)descriptor);
	
	return (descriptor == 0) ? DPMI_Error : DPMI_Ok;
}
