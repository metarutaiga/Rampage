/* $Header: memmgr32.c, 2, 11/1/00 9:20:31 AM PST, Ryan Bissell$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File Name: 	MEMMGR32.C
**
** Description: DirectDraw communication with 16-bit memory manager.
**
** $Revision: 2$
** $Date: 11/1/00 9:20:31 AM PST$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* mmEvict32    --- Calls 16-bit memory manager to evict memory from linear heap.
* mmReclaim32  --- Calls 16-bit memory manager to reclaim memory from linear heap.
*
*******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "ddraw.h"
#include "ddrawi.h"
#include "3dfx.h"
#include "shared.h"

//
// The compiler warns about the return statements below (e.g. "mmEvict32 must
// return a value").  The return value is actually in eax, but there is no
// way to pass it between the inline assembly code and the C return statement
// without introducing a local variable.  The inline assembly code prevents
// the compiler to use register vars.  The most efficient way is to do a
// return without specifying the value.  That's the reason we turn off the
// warning message here.
//
#pragma warning (disable : 4033)
#pragma hdrstop

void __cdecl QT_Thunk (void);

#define NT9XDEVICEDATA GLOBALDATA

/*----------------------------------------------------------------------
Function name: mmEvict32

Description:   Calls 16-bit memory manager to evict memory from linear heap.

Return:        FALSE - failure
               TRUE  - success
----------------------------------------------------------------------*/

DWORD mmEvict32 (NT9XDEVICEDATA * ppdev, DWORD fpVidMemStart32, DWORD fpVidMemEnd32, DWORD dwBlockSize)
{
   // QT_Thunk is documented in Windows 95 System Programming
   //   Secrets, Pietrek.  pp. 191-195, 208-211.
   // NOTE that QT_Thunk will die without fixing up EBP.  I
   //   don't know exactly how big the stack should be but
   //   it appears the more the 16:16 uses, the larger it must be!

   DWORD Addr1616;

   // Get 16:16 function address from shared data
   Addr1616 = _FF(mmEvictAddr2D);

   // QT_Thunk will do all the fixups and pass control to
   //   the 16:16 in edx.
   _asm
      {
      push  DWORD PTR fpVidMemStart32
      push  DWORD PTR fpVidMemEnd32
      push  DWORD PTR dwBlockSize
      mov   edx,Addr1616     ; QT_Thunk expects 16:16 proc addr in EDX
      sub   ebp,200h         ; QT_Thunk builds convoluted 16:16 stack frame
      call  QT_Thunk
      add   ebp,200h         ; QT_Thunk stack stuff
      and   eax,0ffffh       ; mask off high word
      }

    return;                  // return code in eax
}


/*----------------------------------------------------------------------
Function name: mmReclaim32

Description:   Calls 16-bit memory manager to reclaim memory from linear heap.

Return:        FALSE - failure
               TRUE  - success
----------------------------------------------------------------------*/

DWORD mmReclaim32 (NT9XDEVICEDATA * ppdev, DWORD *fpVidMemAddr32)
{
   DWORD Addr1616;

   // Get 16:16 function address from shared data
   Addr1616 = _FF(mmReclaimAddr2D);

   // QT_Thunk will do all the fixups and pass control to
   //   the 16:16 in edx.
   _asm
      {
      push  DWORD PTR fpVidMemAddr32   ; pass our parms
      mov   edx,Addr1616     ; QT_Thunk expects 16:16 proc addr in EDX
      sub   ebp,200h         ; QT_Thunk builds convoluted 16:16 stack frame
      call  QT_Thunk
      add   ebp,200h         ; QT_Thunk stack stuff
      and   eax,0ffffh       ; mask off high word
      }

    return;                  // return code in eax
}
