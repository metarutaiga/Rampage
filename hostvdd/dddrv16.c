/* -*-c++-*- */
/* $Header: dddrv16.c, 10, 11/1/00 9:20:45 AM PST, Ryan Bissell$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** Portions Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
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
*
** File Name:	dddrv16.c
**
** $Revision: 10$
** $Date: 11/1/00 9:20:45 AM PST$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* DDCreateDriverObject  --- Create DirectDraw driver object.
* DDDestroyDriverObject --- Destroy DirectDraw driver object.
* D3notify              --- Notify Direct3d that hardware state has changed,
*
* PRIVATE FUNCTIONS:
*
* buildDDHALInfo16      --- Initialize HALINFO structure.
* DDHalModeInfo         --- Allocate/Lock and initialize the DDHALMODEINFO structure.
* DDHalModeFree         --- Unlocks/Free the DDHALMODEINFO structure and globals.
* EnableTiledHeap       --- Enables tiled heap and moves tiled mark.
* DisableTiledHeap      --- Disables tiled heap and moves tiled mark.
*
*******************************************************************************/

#include "header.h"
#include "modelist.h"

#include "sst2glob.h"  //Must be included int sst2.h when R21 is defined in COP32!

extern   DWORD FAR CDECL   CallProcEx32W(DWORD, DWORD, ...);
extern   DWORD FAR PASCAL  LoadLibraryEx32W(LPVOID,DWORD, DWORD);
extern   DWORD FAR PASCAL  GetProcAddress32W(DWORD, LPVOID);
extern   DWORD FAR PASCAL  FreeLibrary32W(DWORD);


//???extern MODEINFO FAR ModeList[];
//???extern DESKTOPMODEINFO FAR DesktopModeList[];	// STBNW JAC 3-4-99

BOOL buildDDHALInfo16(void);
DDHALMODEINFO FAR * DDHalModeInfo(int nNumModes);
void DDHalModeFree(void);

void EnableTiledHeap();
void DisableTiledHeap();

DWORD fourCC[] = { FOURCC_YUY2, FOURCC_UYVY,FOURCC_RAW8};

// memory manager
// hack, these heap IDs are also defined in fxglobal.h and must match
#define LINEAR_HEAP_ID    0
#define AGP_HEAP_ID       1
#define HEAP_INVALID      0xFFFFFFFFL

#define USE_R21_MMGR      1 // Hack, must match same flag in ddglobal.h

// structure for returning upto two heap templates
static VIDMEM vidMem[] =
{
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};                                                                     


/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  DDCreateDriverObject

Description:    Create DirectDraw driver object.

                Called from Control1 when DirectDraw issues a GDI Escape
                call asking us to create a driver object and register it
                with DirectDraw (bReset == 0).

                Called from Enable1 every time the display mode changes,
                to re-register the mode information and driver capabilities
                with DirectDraw (bReset == 1).

                In both cases we need to call DirectDraw to register our
                driver object.  The function we need to call to register
                with DirectDraw is given to us with the DDNEWCALLBACKFNS
                escape.  If we have not received this escape we should not
                register.

Return:         BOOL (Failure or Success)
----------------------------------------------------------------------*/

BOOL DDCreateDriverObject(BOOL bReset)
{
  DPF(DBGLVL_NORMAL, "DDCreateDriverObject(%d)", bReset);

  // Check that function exists to register with DirectDraw.

  if (_FF(HALCallbacks).lpSetInfo == NULL)
  {
    DPF(DBGLVL_NORMAL, "DDCreateDriverObject: failing because lpSetInfo == NULL");
    return FALSE;
  }

  // Check 32-bit portion of DDHALINFO.

  if (_FF(HALInfo).dwSize != sizeof(DDHALINFO))
  {
    DPF(DBGLVL_NORMAL, "DDCreateDriverObject: failing because 32 driver did not fill in DDHALINFOf");
    return FALSE;
  }

  // Setup 16-bit portion of DDHALINFO.

  buildDDHALInfo16();

  // Do we need vflatd for access to the frame buffer?

#ifdef VFLATD
  if (!bReset && _FF(ScreenAddress) == 0 && _FF(ScreenSel) == VFDQuerySel())
  {
    if (VFDBeginLinearAccess())
    {
      _FF(RealScreenAddress) = VFDQueryBase();
      DPF(DBGLVL_NORMAL,  "Using VFLATD, addr=%08lx", _FF(RealScreenAddress));
    }
  }
  else
  {
      _FF(RealScreenAddress) = _FF(ScreenAddress);
      DPF(DBGLVL_NORMAL,  "RealScreenAddress=%08lx", _FF(RealScreenAddress));
  }
#endif

  // Set the flag for measuring the vertical refresh time.

  if(bReset)
  {
    _FF(fReset) = TRUE;
  }

  // Disable bitmap caching.

  {
    extern void DoAllHost(void);
    extern void DisableDeviceBitmaps(void);

    DoAllHost();
    DisableDeviceBitmaps();
  }
      
  // Call DirectDraw to register our driver object.

  return _FF(HALCallbacks).lpSetInfo(&_FF(HALInfo), bReset);

} /* DDCreateDriverObject */


/*----------------------------------------------------------------------
Function name:  DDDestroyDriverObject

Description:    Destroy DirectDraw driver object.

Return:         DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __loadds CALLBACK DDDestroyDriverObject (LPDDHAL_DESTROYDRIVERDATA p)
{
  DPF(DBGLVL_NORMAL, "DDDestroyDriverObject");

  _FF(HALCallbacks).lpSetInfo = 0;

  // Enable bitmap caching.

  {
    extern void EnableDeviceBitmaps(void);

    EnableDeviceBitmaps();
  }

  // If we are using VflatD turn off linear mode.

#ifdef VFLATD
  if (_FF(ScreenAddress) == 0)
  {
    DPF(DBGLVL_NORMAL, "Done using VFlatD");
    _FF(RealScreenAddress) = 0;
    VFDEndLinearAccess();
  }
#endif

  p->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} /* DDDestroyDriverObject */

/*----------------------------------------------------------------------
Function name:  D3notify

Description:    Notify Direct3d that hardware state has changed,

Return:         VOID
----------------------------------------------------------------------*/
void D3notify(void)
{
  // if d3d is loaded
  if (_FF(pD3context) != 0xffffffff)
  {
    h3WRITE(0, (DWORD *)_FF(pD3context), 0);
    h3WRITE(0, (DWORD *)_FF(pD3changed), 1);
    h3WRITE(0, (DWORD *)_FF(pD3colbuff), 0xffffffff);
    h3WRITE(0, (DWORD *)_FF(pD3auxbuff), 0xffffffff); 
  } 
}

/*******************************************************************/
/*                     PRIVATE FUNCTIONS                           */
/*******************************************************************/


/*----------------------------------------------------------------------
Function name:  IsValidDDrawMode

Description:    Determines if passed in mode is a valid DDraw mode.

Information:

Return:         BOOL    TRUE  = Valid DDraw mode
                        FALSE = Invalid mode
----------------------------------------------------------------------*/
BOOL IsValidDDrawMode ( DWORD hres, DWORD vres )
{
    int i;

    for ( i = 0; ModeList[ i ].dwWidth != 0; i++ )
    {
        if ( ModeList[ i ].dwWidth  == hres &&
                ModeList[ i ].dwHeight == vres &&
                ModeList[ i ].dwFlags & IS_DDRAW_MODE )
            return TRUE;
    }  
    return FALSE;
}

/*----------------------------------------------------------------------
Function name:  buildDDHALInfo16

Description:    Initialize HALINFO structure.

                Most of HALINFO is initialized by 32-bit DirectDraw driver.
                This functions initializes 16-bit specific HALINFO fields,
                including information which can change on every mode switch.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL buildDDHALInfo16()
{
  int                 i;
  LPDDHALMODEINFO     lpMode;
  DWORD dwWidth, dwHeight, dwBPP;
  int nCount;         
  DWORD dwDDHALAddr;
  DWORD dwDDProcAddr;


  /*
   * modify the structures in our shared window with the 32bit driver
   */
  #define cbDDCallbacks        _FF(DDCallbacks)
  #define cbDDSurfaceCallbacks _FF(DDSurfaceCallbacks)
  #define cbDDPaletteCallbacks _FF(DDPaletteCallbacks)
  #define ddHALInfo            _FF(HALInfo)
  #define vmiData              ddHALInfo.vmiData

  /*
   * count the number of modes in the mode table, the table lives in
   * setmode.c not this file.
   */
  dwWidth = 0x0;
  dwHeight = 0x0;
  dwBPP = 0x0;
  nCount = 0;
  for (i=0; ModeList[i].dwWidth != 0; i++)
      if ( (ModeList[i].dwFlags & IS_VALID_MODE) &&
           IsValidDDrawMode( ModeList[i].dwWidth, ModeList[i].dwHeight ) )  // STBNW JAC 3-4-99
         {
         if ((dwWidth != ModeList[i].dwWidth) ||
             (dwHeight != ModeList[i].dwHeight) ||
             (dwBPP != ModeList[i].dwBPP))
            {
            nCount++;
            dwWidth = ModeList[i].dwWidth;
            dwHeight = ModeList[i].dwHeight;
            dwBPP = ModeList[i].dwBPP;
            }
         }

  ddHALInfo.dwNumModes = nCount;
  ddHALInfo.lpModeInfo = DDHalModeInfo(nCount);

  if (NULL == ddHALInfo.lpModeInfo)
      {
      DPF(DBGLVL_NORMAL, "DDHalModeInfo called failed\n");
      return FALSE;
      }

  /*
   * current video mode
   */
  // Translate Mode number into compressed mode list for DirectDraw
  dwWidth = ModeList[_FF(ModeNumber)].dwWidth;
  dwHeight = ModeList[_FF(ModeNumber)].dwHeight;
  dwBPP = ModeList[_FF(ModeNumber)].dwBPP;
  for (i=0; i<nCount; i++)
      if ((dwWidth == ddHALInfo.lpModeInfo[i].dwWidth) &&
          (dwHeight == ddHALInfo.lpModeInfo[i].dwHeight) &&
          (dwBPP == ddHALInfo.lpModeInfo[i].dwBPP))
         break;         

  ddHALInfo.dwModeIndex = i;
  lpMode = &ddHALInfo.lpModeInfo[ddHALInfo.dwModeIndex];

  //ddPrimarySurfaceData struct is initialized in setmode
  vmiData.fpPrimary = _FF(ddPrimarySurfaceData).lfbPtr;

  /*
   * fill in the pixel format
   */
  vmiData.ddpfDisplay.dwSize  = sizeof (DDPIXELFORMAT);
  vmiData.ddpfDisplay.dwFlags = DDPF_RGB;
  vmiData.ddpfDisplay.dwRGBBitCount = lpMode->dwBPP;

  if (lpMode->wFlags & DDMODEINFO_PALETTIZED )
  {
      vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
  }

  // Banshee does not support FOURCC Blts to 8bpp destination.

  if (vmiData.ddpfDisplay.dwRGBBitCount == 8)
  {
    ddHALInfo.ddCaps.dwCaps &= ~DDCAPS_BLTFOURCC;
  }

  vmiData.ddpfDisplay.dwRBitMask = lpMode->dwRBitMask;
  vmiData.ddpfDisplay.dwGBitMask = lpMode->dwGBitMask;
  vmiData.ddpfDisplay.dwBBitMask = lpMode->dwBBitMask;
  vmiData.ddpfDisplay.dwRGBAlphaBitMask = lpMode->dwAlphaBitMask;

  ddHALInfo.lpdwFourCC = fourCC;
  ddHALInfo.ddCaps.dwNumFourCCCodes = sizeof( fourCC ) / sizeof( fourCC[0] );

  //AS 6/15/99: May need to modify these alignment info for Rampage
  vmiData.dwOffscreenAlign = 32;
  vmiData.dwOverlayAlign = 32;
  vmiData.dwTextureAlign = 32; //128 for tiled texture?
  vmiData.dwAlphaAlign = 32;
  vmiData.dwZBufferAlign = 32; //16KB stagger mode Z?

  vmiData.pvmList = vidMem;
  vmiData.dwDisplayHeight =  lpMode->dwHeight;
  vmiData.dwDisplayWidth = lpMode->dwWidth;
  vmiData.lDisplayPitch = lpMode->lPitch;  //This should be the same value being
                                           //used for the MStride of desktop CAM0

#ifndef NOLOWRESFIX
  {
    extern FxU32 lowreshack, lowresheight;

    if (lowreshack)
    {
      vmiData.dwDisplayHeight = lowresheight;
      vmiData.dwDisplayWidth = 320;
      vmiData.lDisplayPitch /= 2;
    }
  }
#endif // #ifndef NOLOWRESFIX

  // Setup the heap templates. Note that when the R21 memory manager is used
  // for local memory, setting the heap flag to zero allows us to return a
  // template with caps and addresses, but GetAvailDriverMemory manages
  // all reporting of local memory. Previously 2X local memory was being
  // reported due to the template. Tried using the VIDMEM_ISHEAP flag to 
  // indicate that the driver had preallocated the memory but ddraw would 
  // fail the heap and shut off all video memory.

  vidMem[LINEAR_HEAP_ID].dwFlags = 0
#ifndef USE_R21_MMGR
                                 | VIDMEM_ISLINEAR
#endif
                                 ;

  vidMem[LINEAR_HEAP_ID].fpStart = _FF(mmTransientHeapStart) + _FF(LFBBASE);
  vidMem[LINEAR_HEAP_ID].fpEnd   = vidMem[LINEAR_HEAP_ID].fpStart + _FF(mmTransientHeapSize) - 1;
  _FF(ddHeap0Start) = vidMem[LINEAR_HEAP_ID].fpStart;
#ifndef USE_R21_MMGR
  vmiData.dwNumHeaps = 0L;
#else
  vmiData.dwNumHeaps = 1L;
#endif

#ifdef AGP_EXECUTE
  if( _FF(enableAGPEM ))
  {
    // Set size and UpdateNonLocalHeap will get actual addresses

    vidMem[AGP_HEAP_ID].dwFlags = 0
                                  | VIDMEM_ISLINEAR 
                                  | VIDMEM_ISNONLOCAL 
                                  | VIDMEM_ISWC
                                  ;
    vidMem[AGP_HEAP_ID].fpStart = 0;
    vidMem[AGP_HEAP_ID].fpEnd   = (_FF(agpHeapSize)*1024*1024)-1;
    vidMem[AGP_HEAP_ID].ddsCaps.dwCaps = 0 
                                  | DDSCAPS_OVERLAY
                                  | DDSCAPS_ZBUFFER
                                  | DDSCAPS_BACKBUFFER
                                  | DDSCAPS_FRONTBUFFER
                                  | DDSCAPS_LOCALVIDMEM
                                  ;
                       
    vidMem[AGP_HEAP_ID].ddsCapsAlt.dwCaps = 0
                                  | DDSCAPS_OVERLAY
                                  | DDSCAPS_ZBUFFER
                                  | DDSCAPS_BACKBUFFER
                                  | DDSCAPS_FRONTBUFFER
                                  | DDSCAPS_LOCALVIDMEM
                                  ;
    vmiData.dwNumHeaps++;
  }
#endif

  _FF(ddNumHeap) = vmiData.dwNumHeaps;
  
  DPF(DBGLVL_NORMAL, "TotalVRAM     = %08lX", _FF(TotalVRAM));
  DPF(DBGLVL_NORMAL, "ScreenAddress = %08lX", _FF(ScreenAddress));
  DPF(DBGLVL_NORMAL, "ScreenSize    = %08lX", lpMode->dwHeight * lpMode->lPitch);

  /*
   * callback functions (give DDRAW 16:16 pointers)
   */
  
  ddHALInfo.lpDDCallbacks        = &cbDDCallbacks;
  ddHALInfo.lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
  ddHALInfo.lpDDPaletteCallbacks = &cbDDPaletteCallbacks;

  if ((IS_SAGE_ACTIVE) && _FF(DDExebufCallbacks.dwFlags))
  {
    ddHALInfo.lpDDExeBufCallbacks  = &_FF(DDExebufCallbacks);
    ddHALInfo.ddCaps.ddsCaps.dwCaps  |= DDSCAPS_EXECUTEBUFFER;
  }
  else
    ddHALInfo.lpDDExeBufCallbacks = NULL;

  /*
   * Only 16-bit callback, all others are undefined or 32-bit.
   */
  cbDDCallbacks.DestroyDriver = DDDestroyDriverObject;


  dwDDHALAddr = LoadLibraryEx32W("3dfx32vx.dll", 0L, 0L);
  if (dwDDHALAddr)
  {
      // get proc address of rmm_bEnableOffscreenHeap in DDraw HAL
       dwDDProcAddr = GetProcAddress32W(dwDDHALAddr, "updateDDInfo32");
        if (dwDDProcAddr)
        {
          // call the DDraw HAL's heap initialization function
          CallProcEx32W(1L,                  // number of parameters
                        1L,                  // 16:16 addr conversion mask
                        dwDDProcAddr,
                        (DWORD)&DriverData);        // parameters...
        }
        // unload the DDraw HAL
        FreeLibrary32W(dwDDHALAddr);
  }

  return TRUE;

} /* buildDDHALInfo16 */

/*----------------------------------------------------------------------
Function name:  DDHalModeInfo

Description:    Allocate/Lock and initialize the DDHALMODEINFO structure.

Return:         DDHALMODEINFO pointer if success, NULL if failure
----------------------------------------------------------------------*/

HGLOBAL hglbDDModeInfo = 0x0;
DDHALMODEINFO FAR * lpHalModeInfo = NULL;
int nOldModes = 0;

DDHALMODEINFO FAR * DDHalModeInfo(int nNumModes)
{
   DWORD dwWidth=0;
   DWORD dwHeight=0;
   DWORD dwBPP=0;
   int i;
   int j;		// STBNW JAC 3-4-99 Moved outside of #ifdef NO_REFRESH
   DWORD dwPitch, dwTiledPitch;

   // Check to see if the number of modes has changed.
   // if it has then we need to reallocated our Mode Information
   if (NULL != lpHalModeInfo)
      {
      if (nNumModes == nOldModes)
         GlobalUnlock(hglbDDModeInfo);
      else
         DDHalModeFree();
      }
   
   nOldModes = nNumModes;
   if (NULL == lpHalModeInfo)
      {
      if (0x0 == (hglbDDModeInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, nNumModes * sizeof(DDHALMODEINFO))))
         {
         DPF(DBGLVL_NORMAL, "  GlobalAlloc for DDModeList failed");
         return NULL;
         }
      }

   if (NULL == (lpHalModeInfo = (DDHALMODEINFO FAR *)GlobalLock(hglbDDModeInfo)))
      {
      DPF(DBGLVL_NORMAL, "  GlobalLock for DDModeList failed");
      GlobalFree(hglbDDModeInfo);
      return NULL;
      }

   // Fill in the Mode Table
  dwWidth = 0x0;
  dwHeight = 0x0;
  dwBPP = 0x0;
  j = 0;
  for (i=0; ModeList[i].dwWidth != 0; i++)
      {
      
      // Will current monitor not support mode?
      if (0x0 == (ModeList[i].dwFlags & IS_VALID_MODE))
         continue;

      // Same mode different refresh ?
      if ((dwWidth == ModeList[i].dwWidth) &&
          (dwHeight == ModeList[i].dwHeight) &&
          (dwBPP == ModeList[i].dwBPP))
         continue;

      // STBNW JAC 3-4-99
      if ( !IsValidDDrawMode ( ModeList[i].dwWidth, ModeList[i].dwHeight ) )
         continue;
      // STBNW end

      dwWidth = ModeList[i].dwWidth;
      dwHeight = ModeList[i].dwHeight;
      dwBPP = ModeList[i].dwBPP;
      lpHalModeInfo[j].dwWidth = ModeList[i].dwWidth;
      lpHalModeInfo[j].dwHeight = ModeList[i].dwHeight;
      lpHalModeInfo[j].dwBPP = ModeList[i].dwBPP;

      dwPitch = ModeList[i].lPitch;
      if ( _FF(ddPrimaryInTile) == TRUE )
      {
         dwTiledPitch = (dwPitch + SST2_TILE_WIDTH - 1) >> SST2_TILE_WIDTH_BITS;
         //Needs to know if primary in stagger mode and do adjustment to dwTiledPitch here
         //if ( _FF(dwGdiMiscFlags) & T1_STAGGER )
         //    dwTiledPitch = RNDUP_T1_PGWIDTH(dwTiledPitch);
         //else if ( _FF(dwGdiMiscFlags) & T0_STAGGER )
         //    dwTiledPitch = RNDUP_T0_PGWIDTH(dwTiledPitch);
                  
         dwPitch = dwTiledPitch << SST2_TILE_WIDTH_BITS;
      }
      //The mode's pitch reported should be the same as the value set in the desktop CAM0's MStride 
      if      (dwPitch  <= 1024) lpHalModeInfo[j].lPitch = 1024L;
      else if (dwPitch  <= 2048) lpHalModeInfo[j].lPitch = 2048L;
      else if (dwPitch  <= 4096) lpHalModeInfo[j].lPitch = 4096L;
      else if (dwPitch  <= 8192) lpHalModeInfo[j].lPitch = 8192L;
      else                       lpHalModeInfo[j].lPitch = 16384L;
    
      // Tell DirectDraw latter what the refresh will be
      lpHalModeInfo[j].wRefreshRate = 0x0;
      if (8 == ModeList[i].dwBPP)
         {
         lpHalModeInfo[j].wFlags = DDMODEINFO_PALETTIZED;
         lpHalModeInfo[j].dwRBitMask = 0x00000000L; 
         lpHalModeInfo[j].dwGBitMask = 0x00000000L;
         lpHalModeInfo[j].dwBBitMask = 0x00000000L;
         lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
         }
      else if (16 == ModeList[i].dwBPP)
         {
         lpHalModeInfo[j].wFlags = 0x0;
         lpHalModeInfo[j].dwRBitMask = 0x0000F800L; 
         lpHalModeInfo[j].dwGBitMask = 0x000007E0L;
         lpHalModeInfo[j].dwBBitMask = 0x0000001FL;
         lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
         }
      else if (24 == ModeList[i].dwBPP)
         {
         lpHalModeInfo[j].wFlags = 0x0;
         lpHalModeInfo[j].dwRBitMask = 0x00FF0000L; 
         lpHalModeInfo[j].dwGBitMask = 0x0000FF00L;
         lpHalModeInfo[j].dwBBitMask = 0x000000FFL;
         lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
         }
      else 
         {
         lpHalModeInfo[j].wFlags = 0x0;
         lpHalModeInfo[j].dwRBitMask = 0x00FF0000L; 
         lpHalModeInfo[j].dwGBitMask = 0x0000FF00L;
         lpHalModeInfo[j].dwBBitMask = 0x000000FFL;
         lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
         }
      j++;
      }

   return lpHalModeInfo;
}


/*----------------------------------------------------------------------
Function name:  DDHalModeFree

Description:    Unlocks/frees memory associated with Direct Draw HAL.

Return:         VOID
----------------------------------------------------------------------*/
void DDHalModeFree(void)
{
  // Unlock Pointer

  GlobalUnlock(hglbDDModeInfo);

  // Pointer is no longer valid

  lpHalModeInfo = NULL;  

  // Handle is no longer valid

  if (GlobalFree(hglbDDModeInfo))
  {
    DPF(DBGLVL_NORMAL, "GlobalFree Failed\n");
  }  
  hglbDDModeInfo = 0x0;
}


/*----------------------------------------------------------------------
Function name:  EnableTiledHeap

Description:	Enables tiled heap and moves tiled mark.

Return:         NONE
----------------------------------------------------------------------*/

void EnableTiledHeap()
{
  // Set tiled heap active flag.

  _FF(ddTiledHeapActive) = TRUE;

  // Move tiled mark to start of tiled heap.

  _FF(ddTileMark) = _FF(ddTiledHeapStart);

  // Update DIB engine pointer, since tile mark moved.
  
  _FF(lpPDevice)->deBitsOffset = _FF(ddPrimarySurfaceData).lfbPtr;
}

/*----------------------------------------------------------------------
Function name:  DisableTiledHeap

Description:	Disables tiled heap and moves tiled mark.

Return:         NONE
----------------------------------------------------------------------*/
void DisableTiledHeap()
{
  // Clear tiled heap active flag.

  _FF(ddTiledHeapActive) = FALSE;

  // Move tiled mark to start of desktop.

  _FF(ddTileMark) = _FF(gdiDesktopStart);

  // Update DIB engine pointer, since tile mark moved.
  
  _FF(lpPDevice)->deBitsOffset = _FF(ScreenAddress);
}


