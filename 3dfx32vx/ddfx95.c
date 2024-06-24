/* $Header: ddfx95.c, 8, 10/11/00 12:56:49 PM PDT, Brent Burton$ */
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
** File Name: DDFX95.C
**
** Description: Entry point for the 32 bit DDraw Driver DLL, driver init and
**              D3D HAL driver infomation enumeration
**
** $Revision: 8$
** $Date: 10/11/00 12:56:49 PM PDT$
**
** $History: ddfx95.c $
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 9/07/99    Time: 10:37a
** Updated in $/devel/sst2/Win95/dx/dd32
** Changes for SLI
** 
** *****************  Version 16  *****************
** User: Einkauf      Date: 9/03/99    Time: 12:11a
** Updated in $/devel/sst2/Win95/dx/dd32
** add dxfifoprolog stub for convenient breakpoint in fifo debug
** 
** *****************  Version 15  *****************
** User: Einkauf      Date: 8/31/99    Time: 10:57p
** Updated in $/devel/sst2/Win95/dx/dd32
** add FifoEnable routine
** 
** *****************  Version 14  *****************
** User: Evan         Date: 8/30/99    Time: 10:12a
** Updated in $/devel/sst2/Win95/dx/dd32
** rearranges cam management code to be in one place
** 
** *****************  Version 13  *****************
** User: Einkauf      Date: 8/18/99    Time: 9:58a
** Updated in $/devel/sst2/Win95/dx/dd32
** init CAM entry used by texture download
** 
** *****************  Version 12  *****************
** User: Einkauf      Date: 7/30/99    Time: 5:31p
** Updated in $/devel/sst2/Win95/dx/dd32
** buildddhalinfo32: scan HW CAM entries for region display driver has
** mapped and remove it from the Phantom MM free space.
** 
** *****************  Version 11  *****************
** User: Einkauf      Date: 7/28/99    Time: 2:06p
** Updated in $/devel/sst2/Win95/dx/dd32
** added phantom mem mgr - modified existing memmgr to use either physical
** or phantom memory; added interface routines; modified DdLock/DdUnlock
** to interface to phantom mm.
** 
** *****************  Version 10  *****************
** User: Peterm       Date: 6/12/99    Time: 3:18p
** Updated in $/devel/sst2/Win95/dx/dd32
** memmgr merge deltas, mm and other cleanup
** 
** *****************  Version 9  *****************
** User: Peterm       Date: 6/03/99    Time: 11:24p
** Updated in $/devel/sst2/Win95/dx/dd32
** modified to run with H3 tot (adds multimon, various bug fixes, and many
** structural deltas)
** 
** *****************  Version 10  *****************
** User: Andrew       Date: 3/17/99    Time: 2:44p
** Updated in $/devel/h3/Win95/dx/dd32
** reverted to old version to remove swaphang workaround
** 
** *****************  Version 7  *****************
** User: Michael      Date: 12/30/98   Time: 4:48p
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
*/

/*
 *
 * NOTE:  All routines are called with the Win16 lock taken.   This is
 * to prevent anyone from calling the display driver and doing something
 * that could confict with this 32-bit driver.
 *
 * This means that all shared 16-32 memory is safe to use at any time inside
 * either driver.
 */

/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include <ddrawi.h>
#include "d3global.h"
#include "ddcam.h"                  // FxCamInit()
#include "fifomgr.h"

#ifdef K6_2
#include "k6_2.h"
#endif

/***************************************************************************
* D E F I N E S
****************************************************************************/

// we can create a heap in shared memory independent of whether or not
// GLOBALDATA is in the PDEV or not but I'll initialize it so we don't
// create one if GLOBALDATA isn't in the PDEV
// NOTE: this heap is separate from the heap created by MemInit()
#define CREATE_SHARED_HEAP  1

/***************************************************************************
* F U N C T I O N   P R O T O T Y P E S
****************************************************************************/

static BOOL buildDDHALInfo32(NT9XDEVICEDATA *ppdev);

extern BOOL __stdcall
DrvGetDirectDrawInfo(DHPDEV       dhpdev,
                     DDHALINFO    *pHalInfo,
                     DWORD        *pdwNumHeaps,
                     VIDEOMEMORY  *pvmList,
                     DWORD        *pdwNumFourCC,
                     DWORD        *pdwFourCC);
extern BOOL __stdcall

DrvEnableDirectDraw(DHPDEV                    dhpdev,
                    DDHAL_DDCALLBACKS         *pCallBacks,
                    DDHAL_DDSURFACECALLBACKS  *pSurfaceCallBacks,
                    DDHAL_DDPALETTECALLBACKS  *pPaletteCallBacks,
                    DDHAL_DDEXEBUFCALLBACKS   *pExebufCallbacks);

extern BOOL InitOverlay( NT9XDEVICEDATA *ppdev ,DDHALINFO    *pHalInfo);

NT9XDEVICEDATA * pDevices[NUM_DEVICES] =
   {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,}; 

/***************************************************************************
* G L O B A L   V A R I A B L E S
****************************************************************************/
HINSTANCE   hInstance;

/*----------------------------------------------------------------------
Function name: DllMain

Description:   DLL Entry point to 32bit DDraw driver

Return:        BOOL

               TRUE  - DLL is attached
			   FALSE - unable to attach DLL to calling process
----------------------------------------------------------------------*/


BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
  hInstance = hModule;

  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hModule);
      break;

    case DLL_PROCESS_DETACH:
      break;

#ifdef DEBUG
    /*
     * we don't ever want to see thread attach/detach
     */
    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      break;
#endif

    default:
      break;
  }

  return TRUE;
}// DllMain

/*----------------------------------------------------------------------
Function name: DriverInit

Description:   called by DirectDraw on behalf of the main display driver

Return:        DWORD 

               0   - unable to initialize the driver

			   ptr - driver data - ppdev; 
			         shared memory region between 16/32-bit address space
----------------------------------------------------------------------*/

DWORD _stdcall
DriverInit(DWORD ptr)
{
  NT9XDEVICEDATA *ppdev;
  int i;

  ASSERTDD2(ptr);

#ifdef GBLDATA_IN_PDEV
  ppdev = (PDEV *)ptr;
  ASSERTDD2(! IsBadWritePtr((LPVOID)ppdev,sizeof(PDEV)));
  ASSERTDD2(! IsBadReadPtr((LPVOID)ppdev,sizeof(PDEV)));
  if (! buildDDHALInfo32((PDEV *)ptr))
    return 0;
#else
  ppdev = (NT9XDEVICEDATA *)ptr;
  for (i=0; i<NUM_DEVICES; i++)
      {

      // Already got it?
      if (ppdev == pDevices[i])
         break;

      // Found a spare spot then save it
      if (NULL == pDevices[i])
         {
         pDevices[i] = ppdev;
         break;
         }
      }
  ASSERTDD2(! IsBadWritePtr((LPVOID)ppdev,sizeof(GLOBALDATA)));
  ASSERTDD2(! IsBadReadPtr((LPVOID)ppdev,sizeof(GLOBALDATA)));

  if (0 == _DS(hSharedHeap))
      (HANDLE)_DS(hSharedHeap) = HeapCreate(HEAP_SHARED, 0x2000, 0);

  if (! buildDDHALInfo32(ppdev))
    return 0;
#endif

#ifdef K6_2
  // set K6-2 functions pointers so we may enable/disable
  // 3DNow! optimization via registry each time driver inited
  ADJUST3DNOWPOINTERS(ppdev);
#endif

//#pragma message(__FILELINE__, "can we call MemInit here?")
  DISPDBG((ppdev, DEBUG_APIENTRY, "DriverInit called, ptr=%08lx", ptr));

  return (DWORD)ptr;
}// DriverInit

/*----------------------------------------------------------------------
Function name: buildDDHALInfo32

Description:   Enumerate Global data, D3DHal callbacks and DriverInfo

Return:        static BOOL

               TRUE  - 
               FALSE - 
----------------------------------------------------------------------*/

static BOOL buildDDHALInfo32(NT9XDEVICEDATA *ppdev)
{

#if CREATE_SHARED_HEAP
  //allocate memory for ddglobal
  if (NULL == (LPVOID)_FF(pddglobal))
  {
    (LPVOID)_FF(pddglobal) = DXMALLOCZ(sizeof(DDGLOBAL));
    if (NULL == (LPVOID)_FF(pddglobal))
      return FALSE;
  }

  //allocate memory for fxglobal
  if (NULL == (LPVOID)_FF(pfxglobal))
  {
    (LPVOID)_FF(pfxglobal) = DXMALLOCZ(sizeof(fxGlobal));
    if (NULL == (LPVOID)_FF(pfxglobal))
      return FALSE;
  }

#if ENABLE_3D
  //allocate memory for d3global
  if (NULL == (LPVOID)_FF(pd3global))
  {
    (LPVOID)_FF(pd3global) = DXMALLOCZ(sizeof(d3Global));
    if (NULL == (LPVOID)_FF(pd3global))
      return FALSE;
  }
#endif
#else
  //allocate memory for ddglobal
  ppdev->pddglobal = (DWORD)(&mytempddglobal);
#endif

  // initialize the memory lists
  if (!memMgr_initLists(ppdev))
      return FALSE;


  // if we wanted to emulate NT, we'd call DrvGetDirectDrawInfo twice
  // the first time with pvmList and pdwFourCC set to NULL, after the
  // first call we'd use the returned dwNumHeaps and dwNumFourCC to
  // allocate two memory chunks, one for the heap array and one for
  // the fourcc array then call DrvGetDirectDrawInfo again with these
  // pointers filled in but this isn't necessary on win9x since the 16
  // bit side will fill in the heap array and fourcc array
  // So we just call DrvGetDirectDrawInfo once on winx9x
  if (FALSE == DrvGetDirectDrawInfo((DHPDEV)ppdev,
                                    &(_FF(HALInfo)),
                                    &(_FF(HALInfo).vmiData.dwNumHeaps),
                                    NULL,
                                    &(_FF(HALInfo).ddCaps.dwNumFourCCCodes),
                                    NULL))
    return FALSE;

  if (FALSE == DrvEnableDirectDraw((DHPDEV)ppdev,
                                   &(_FF(DDCallbacks)),
                                   &(_FF(DDSurfaceCallbacks)),
                                   &(_FF(DDPaletteCallbacks)),
                                   &(_FF(DDExebufCallbacks))))
    return FALSE;

  // initialize the cam manager
  FxCamInit(&(_DD(camMgr)), ppdev);

#ifdef SLI
   _DD(sliMode) = SLI_MODE_DISABLED;
   _DD(sliMemoryMode) = MEMORY_MODE_DISABLED;
   _DD(dwComputeMode) = FALSE;
#endif

#ifdef AGP_EXECUTE
  _DD(agpMode) = EXECUTE_MODE_DISABLED;
#endif

  return TRUE;
}// buildDDHALInfo32


/*----------------------------------------------------------------------
Function name: updateDDInfo32

Description:   update some variables when display driver is updating
               DDHALInfo ( during mode change...)
               This function is called by 16 display driver 

Return:        static BOOL

               TRUE  - 
               FALSE - 
----------------------------------------------------------------------*/

BOOL _stdcall updateDDInfo32(NT9XDEVICEDATA *ppdev)
{
  if (NULL == (LPVOID)_FF(pddglobal))
  {
      return FALSE;
  }

#if CREATE_SHARED_HEAP
  if (NULL == (LPVOID)_FF(pfxglobal))
  {
      return FALSE;
  }

#if ENABLE_3D
  if (NULL == (LPVOID)_FF(pd3global))
  {
      return FALSE;
  }
#endif
#endif

  InitOverlay( ppdev , &(_FF(HALInfo)));

  // initialize the memory lists
  if (!memMgr_initLists(ppdev))
      return FALSE;

  // initialize the cam manager
  FxCamInit(&(_DD(camMgr)), ppdev);

  return TRUE;

}

void FifoEnable(NT9XDEVICEDATA *ppdev, FxBool enable)
{
	FxU32 basesize;

	// wait for fifo to empty before changing Fifo enable mode
    while (FXGETBUSYSTATUS(ppdev));

	basesize = GET(ghwAC->PRIMARY_CMDFIFO.baseSize);

	if (enable)
	{
		basesize |= SST_EN_CMDFIFO;
	}
	else 
	{
		basesize &= ~SST_EN_CMDFIFO;
	}

	SETDW(ghwAC->PRIMARY_CMDFIFO.baseSize, basesize);
}

void dxfifobreak()
{
    // just for break point for dx fifo debug
}
