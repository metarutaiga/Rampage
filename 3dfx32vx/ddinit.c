/* -*-c++-*- */
/* $Header: ddinit.c, 19, 11/2/00 3:59:21 PM PST, Michel Conrad$ */
/*
** Copyright (c) 1995-1998, 3Dfx Interactive, Inc.
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
** File Name:   DDINIT.C
**
** Description: Direct Draw initialization and support functions.
**
** $Revision: 19$
** $Date: 11/2/00 3:59:21 PM PST$
**
** $History: ddinit.c $
** 
** *****************  Version 44  *****************
** User: Bdaniels     Date: 9/04/99    Time: 7:26p
** Updated in $/devel/sst2/Win95/dx/dd32
** Restored Z Bufer DDBD_16 CAPs as some apps are not written / writing to
** new GUID_ZPIXELFORMATs interface.
** 
** *****************  Version 43  *****************
** User: Pratt        Date: 9/02/99    Time: 8:38a
** Updated in $/devel/R3/Win95/dx/dd32
** removed IFDEF R21 and changed IFNDEF r21 to IF(0)
** 
** *****************  Version 42  *****************
** User: Bdaniels     Date: 8/31/99    Time: 1:22p
** Updated in $/devel/sst2/Win95/dx/dd32
** Adding 32 bpp rendering, 24 bit z / 8 bit stencil support.
** 
** *****************  Version 41  *****************
** User: Xingc        Date: 8/11/99    Time: 4:38p
** Updated in $/devel/sst2/Win95/dx/dd32
** Setting DDFXALPHACAPS
** 
** *****************  Version 40  *****************
** User: Mconrad      Date: 8/10/99    Time: 9:45p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fix warnings.
** 
** *****************  Version 39  *****************
** User: Einkauf      Date: 7/08/99    Time: 7:31p
** Updated in $/devel/sst2/Win95/dx/dd32
** CAM locking for general surfaces, BLT's work better - using proper
** dst/srcFormat settings
** 
** *****************  Version 38  *****************
** User: Xingc        Date: 6/14/99    Time: 5:31p
** Updated in $/devel/sst2/Win95/dx/dd32
** Put the code to init sst2VidRegs back
** 
** *****************  Version 37  *****************
** User: Xingc        Date: 6/14/99    Time: 2:52p
** Updated in $/devel/sst2/Win95/dx/dd32
** Put R2.1 overlay and vpe code back. Add mulit-mon support.
** 
** *****************  Version 35  *****************
** User: Peterm       Date: 6/04/99    Time: 2:01a
** Updated in $/devel/sst2/Win95/dx/dd32
** updated for changed inc\sst* and others
** 
** *****************  Version 34  *****************
** User: Peterm       Date: 6/03/99    Time: 11:25p
** Updated in $/devel/sst2/Win95/dx/dd32
** modified to run with H3 tot (adds multimon, various bug fixes, and many
** structural deltas)
** 
** *****************  Version 85  *****************
** User: Adrians      Date: 3/26/99    Time: 11:46a
** Updated in $/devel/h3/Win95/dx/dd32
** Replace DX_VERSION with DX to stay consistant with the D3D code. Make
** DX=6 the default build mode.
** 
** *****************  Version 84  *****************
** User: Russ         Date: 3/22/99    Time: 9:29a
** Updated in $/devel/h3/W2K/Src/Video/Displays/h3
** subtract one from fpEnd of linear heaps reported to MS so that the
** ddhal is available on checked builds
** 
** *****************  Version 83  *****************
** User: Cwilcox      Date: 3/19/99    Time: 10:10a
** Updated in $/devel/h3/Win95/dx/dd32
** Removed ddShrinkSurface hack.
** 
** *****************  Version 82  *****************
** User: Andrew       Date: 3/17/99    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/dd32
** reverted to old version to remove swaphang workaround
** 
** *****************  Version 79  *****************
** User: Andrew       Date: 3/04/99    Time: 9:40a
** Updated in $/devel/h3/Win95/dx/dd32
** Changed Mode X Illegel to be commented out instead of ifdef since we
** are already in a ifdef.
** 
** *****************  Version 78  *****************
** User: Andrew       Date: 3/03/99    Time: 8:07a
** Updated in $/devel/h3/Win95/dx/dd32
** Changed flags to allow mode X for Win '95.  This fixes the issue with
** bug! and mode X in general.
** 
** *****************  Version 77  *****************
** User: Martin       Date: 2/20/99    Time: 5:19p
** Updated in $/devel/h3/Win95/dx/dd32
** initialize agp texture download sync buffers.
** 
** *****************  Version 76  *****************
** User: Stb_srogers  Date: 2/19/99    Time: 12:05p
** Updated in $/devel/h3/win95/dx/dd32
** Added #ifdef WINNT to get the path for WINNT stbperf.in as well as the
** Win95 one
** 
** *****************  Version 75  *****************
** User: Stb_doconnel Date: 2/11/99    Time: 4:44p
** Updated in $/devel/h3/winnt/src/video/displays/h3
** 
** *****************  Version 74  *****************
** User: Xingc        Date: 2/02/99    Time: 11:00a
** Updated in $/devel/h3/Win95/dx/dd32
** Disable STB_DISABLEDOWNSCALE
** 
** *****************  Version 73  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 7:11a
** Updated in $/devel/h3/win95/dx/dd32
** 
** *****************  Version 72  *****************
** User: Russ         Date: 1/18/99    Time: 9:01p
** Updated in $/devel/h3/WinNT/src/Video/Displays/H3
** fix for MTM2 ZBuffer allocation failures on NT5, set
** vmiData.dwZBufferAlign to 16
**
** *****************  Version 71  *****************
** User: Michael      Date: 12/30/98   Time: 4:48p
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
** *****************  Version 70  *****************
** User: Cwilcox      Date: 12/18/98   Time: 12:39p
** Updated in $/devel/h3/Win95/dx/dd32
** Cleaned up dd3dInOverlay values
**
** *****************  Version 69  *****************
** User: Cwilcox      Date: 12/11/98   Time: 11:17a
** Updated in $/devel/h3/Win95/dx/dd32
** Redo previous checkin.
**
** *****************  Version 67  *****************
** User: Russ         Date: 12/09/98   Time: 6:55a
** Updated in $/devel/h3/WinNT/src/Video/Displays/H3
** NT5 D3D changes for Banshee
**
** *****************  Version 66  *****************
** User: Michael      Date: 12/03/98   Time: 3:12p
** Updated in $/devel/h3/Win95/dx/dd32
** References to ClearSwapCount broke the NT build.  Surround them by
** #ifndef WINNT.
**
** *****************  Version 65  *****************
** User: Michael      Date: 12/02/98   Time: 2:06p
** Updated in $/devel/h3/Win95/dx/dd32
** Reenable the ClearSwapCount I backed out in the previous version.
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"
#include "regkeys.h"
#include "ddovl32.h"
#include "ddins.h"

#if defined (INSTRUMENTATION)
#include "dxins.h"
extern INSHEADER insHeader;
#endif

// STB Begin Changes
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
// STB End Changes

/**************************************************************************
* D E F I N E S
***************************************************************************/

// I'm leaving the PALETTE_CAPS define because I'm not convinced there is
// any reason to implement the palette callbacks on NT but they are aleady
// implemented for win9x - RL
//
// The OVERLAY_CAPS define is left here for a simple method of disabling
// the overlay support.  Due to the way we are implementing creation of
// overlay surfaces, disabling overlay support is almost the only way to
// debug color conversion blts - RL

#define ENABLE_PALETTE_CAPS       1
#define ENABLE_OVERLAY_CAPS       1

//#define STB_DISABLEHWDOWNSCALE    1   //used for test only

// this define hides the differences between arguments passed
// to the getVideoMemSize function under NT and Win9x
#define GETVIDEOMEMSIZE(arg1)               getVideoMemSize(arg1)
#define D3DHALCREATEDRIVER(arg1,arg2,arg3)  D3DHALCreateDriver(arg1,arg2,arg3)

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

static DWORD __fastcall GETVIDEOMEMSIZE(NT9XDEVICEDATA *ppdev);

extern BOOL __stdcall
D3DHALCREATEDRIVER(NT9XDEVICEDATA *ppdev,
                   LPD3DHAL_GLOBALDRIVERDATA* lplpGlobal,
                   LPD3DHAL_CALLBACKS* lplpHALCallbacks);

#if defined(DEBUG) && defined(RECTMM)
void ddInitDebugLevel(void);
char *getToken(char *, char *);     // declared in d3trace.c
extern BYTE debugLevel[256];        // declared in d3trace.c
#endif

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

/*----------------------------------------------------------------------
Function name:  DrvGetDirectDrawInfo

Description:    NT style function called to fill in ddraw caps, etc.

Return:         BOOL

                TRUE  - ddraw caps enumerated
				FALSE - unable to fill ddraw caps
----------------------------------------------------------------------*/

BOOL __stdcall
DrvGetDirectDrawInfo(DHPDEV       dhpdev,
                     DDHALINFO    *pHalInfo,
                     DWORD        *pdwNumHeaps,
                     VIDEOMEMORY  *pvmList,
                     DWORD        *pdwNumFourCC,
                     DWORD        *pdwFourCC)
{
  NT9XDEVICEDATA *ppdev = (NT9XDEVICEDATA *)dhpdev;

  // fill in the HALINFO
  memset(pHalInfo, 0, sizeof(DDHALINFO));

  pHalInfo->dwSize = sizeof(DDHALINFO);

  pHalInfo->ddCaps.ddsCaps.dwCaps = 0
                                  | DDSCAPS_FLIP
                                  | DDSCAPS_OFFSCREENPLAIN
                                  | DDSCAPS_BACKBUFFER
                                  | DDSCAPS_COMPLEX
                                  | DDSCAPS_FRONTBUFFER
                                  | DDSCAPS_VIDEOMEMORY
#if ENABLE_PALETTE_CAPS
                                  | DDSCAPS_PALETTE
#endif
                                  | DDSCAPS_PRIMARYSURFACE
                                  | DDSCAPS_TEXTURE
                                  | DDSCAPS_3DDEVICE
                                  | DDSCAPS_ZBUFFER
                                  | DDSCAPS_MIPMAP

#if ENABLE_VIDEOPORT
                                  | DDSCAPS_VIDEOPORT
#endif
                                  ;


  pHalInfo->lpDDCallbacks        = &(_FF(DDCallbacks));
  pHalInfo->lpDDSurfaceCallbacks = &(_FF(DDSurfaceCallbacks));
  pHalInfo->lpDDPaletteCallbacks = &(_FF(DDPaletteCallbacks));

  if ((IS_SAGE_ACTIVE) && _FF(DDExebufCallbacks.dwFlags))
  {
        pHalInfo->lpDDExeBufCallbacks  = &_FF(DDExebufCallbacks);
        pHalInfo->ddCaps.ddsCaps.dwCaps  |= DDSCAPS_EXECUTEBUFFER;
  }
  else
        pHalInfo->lpDDExeBufCallbacks = NULL;

  pHalInfo->ddCaps.dwNumFourCCCodes = 0;
  pHalInfo->lpdwFourCC              = NULL;

  pHalInfo->hInstance = (DWORD) hInstance;
  pHalInfo->lpPDevice = (LPVOID)_FF(lpPDevice);

  pHalInfo->dwFlags = (0
                    // | DDHALINFO_MODEXILLEGAL // Fixes Bug!
                       | DDHALINFO_GETDRIVERINFOSET
#if (DIRECTDRAW_VERSION >= 0x0800)
                       | DDHALINFO_GETDRIVERINFO2
#endif
                       );
  pHalInfo->GetDriverInfo = DdGetDriverInfo;  //Callback for Misc, D3D, Videoport callbacks & caps

  pHalInfo->ddCaps.dwCaps = 0
                          | DDCAPS_BLT
                          //| DDCAPS_BLTQUEUE
                          | DDCAPS_BLTFOURCC
                          | DDCAPS_BLTSTRETCH
                          | DDCAPS_GDI
#if ENABLE_PALETTE_CAPS
                          | DDCAPS_PALETTE
#endif
                          | DDCAPS_READSCANLINE
                          | DDCAPS_COLORKEY
                          //| DDCAPS_ALPHA
                          | DDCAPS_COLORKEYHWASSIST
                          | DDCAPS_BLTCOLORFILL
                          //| DDCAPS_CANCLIP
                          //| DDCAPS_CANCLIPSTRETCHED
#ifndef PERF_DISABLE_CANBLTSYSMEM
                          | DDCAPS_CANBLTSYSMEM
#endif
                        
                          | DDCAPS_3D
                          | DDCAPS_ZBLTS
                          | DDCAPS_BLTDEPTHFILL
                          ;

  pHalInfo->ddCaps.dwSVBCaps = 0
                             | DDCAPS_BLT
                             //| DDCAPS_BLTQUEUE
                             //| DDCAPS_BLTFOURCC
                             | DDCAPS_BLTSTRETCH
                             | DDCAPS_GDI
                             | DDCAPS_COLORKEY
                             //| DDCAPS_ALPHA
                             | DDCAPS_COLORKEYHWASSIST
                             | DDCAPS_BLTCOLORFILL
                             //| DDCAPS_CANCLIP
                             //| DDCAPS_CANCLIPSTRETCHED
#ifndef PERF_DISABLE_CANBLTSYSMEM
                             | DDCAPS_CANBLTSYSMEM
#endif

                             | DDCAPS_3D
                             | DDCAPS_ZBLTS
                             | DDCAPS_BLTDEPTHFILL
                             ;

  pHalInfo->ddCaps.dwCaps2 = 0
                           | DDCAPS2_WIDESURFACES
                           | DDCAPS2_COPYFOURCC
#if ENABLE_VIDEOPORT
                           | DDCAPS2_VIDEOPORT
#endif
                           ;

#ifdef AGP_EXECUTE
  if( _FF( enableAGPEM ))
  {
    pHalInfo->ddCaps.dwCaps2 |= DDCAPS2_NONLOCALVIDMEM;
  }
#endif

  pHalInfo->ddCaps.dwFXCaps = 0
                            //| DDFXCAPS_BLTARITHSTRETCHY
                            //| DDFXCAPS_BLTARITHSTRETCHYN
                            | DDFXCAPS_BLTSHRINKX
                            | DDFXCAPS_BLTSHRINKY
                            //| DDFXCAPS_BLTSHRINKXN
                            //| DDFXCAPS_BLTSHRINKYN
                            | DDFXCAPS_BLTSTRETCHX
                            //| DDFXCAPS_BLTSTRETCHXN
                            | DDFXCAPS_BLTSTRETCHY
                            //| DDFXCAPS_BLTSTRETCHYN
                            ;

  pHalInfo->ddCaps.dwFXAlphaCaps = 0;

  pHalInfo->ddCaps.dwSVBFXCaps = 0
                                 //| DDFXCAPS_BLTARITHSTRETCHY
                                 //| DDFXCAPS_BLTARITHSTRETCHYN
                                 //| DDFXCAPS_BLTSHRINKX
                                 //| DDFXCAPS_BLTSHRINKY
                                 //| DDFXCAPS_BLTSHRINKXN
                                 //| DDFXCAPS_BLTSHRINKYN
                                 | DDFXCAPS_BLTSTRETCHX
                                 //| DDFXCAPS_BLTSTRETCHXN
                                 | DDFXCAPS_BLTSTRETCHY
                                 //| DDFXCAPS_BLTSTRETCHYN
                                 ;

#if ENABLE_PALETTE_CAPS
  pHalInfo->ddCaps.dwPalCaps = 0
                             | DDPCAPS_8BIT
                             | DDPCAPS_ALLOW256
                             ;
#endif

  pHalInfo->ddCaps.dwCKeyCaps = 0
                              | DDCKEYCAPS_DESTBLT
                              | DDCKEYCAPS_DESTBLTCLRSPACE
                              | DDCKEYCAPS_SRCBLT
                              | DDCKEYCAPS_SRCBLTCLRSPACE
                              ;

  pHalInfo->ddCaps.dwSVBCKeyCaps = 0
                                 | DDCKEYCAPS_DESTBLT
                                 | DDCKEYCAPS_DESTBLTCLRSPACE
                                 | DDCKEYCAPS_SRCBLT
                                 | DDCKEYCAPS_SRCBLTCLRSPACE
                                 ;
#if ENABLE_VIDEOPORT
  pHalInfo->ddCaps.dwMaxVideoPorts =  1;      //maximum number of usable video ports
  pHalInfo->ddCaps.dwCurrVideoPorts = 0;      //current number of video ports used
#else
  pHalInfo->ddCaps.dwMaxVideoPorts =  0;      //maximum number of usable video ports
  pHalInfo->ddCaps.dwCurrVideoPorts = 0;      //current number of video ports used
#endif

#if ENABLE_OVERLAY_CAPS
  InitOverlay( ppdev, pHalInfo);           //Xing
#endif

  // support all 16 two operand rops
  pHalInfo->ddCaps.dwRops[0] = 0x00020001;  // 0    & DSon
  pHalInfo->ddCaps.dwRops[1] = 0x00080004;  // DSna & Sn
  pHalInfo->ddCaps.dwRops[2] = 0x00200010;  // SDna & Dn
  pHalInfo->ddCaps.dwRops[3] = 0x00800040;  // DSx  & DSan
  pHalInfo->ddCaps.dwRops[4] = 0x02000100;  // DSa  & DSxn
  pHalInfo->ddCaps.dwRops[5] = 0x08000400;  // D    & DSno
  pHalInfo->ddCaps.dwRops[6] = 0x20001000;  // S    & SDno
  pHalInfo->ddCaps.dwRops[7] = 0x80004000;  // DSo  & 1

  pHalInfo->ddCaps.dwSVBRops[0] = 0x00020001;
  pHalInfo->ddCaps.dwSVBRops[1] = 0x00080004;
  pHalInfo->ddCaps.dwSVBRops[2] = 0x00200010;
  pHalInfo->ddCaps.dwSVBRops[3] = 0x00800040;
  pHalInfo->ddCaps.dwSVBRops[4] = 0x02000100;
  pHalInfo->ddCaps.dwSVBRops[5] = 0x08000400;
  pHalInfo->ddCaps.dwSVBRops[6] = 0x20001000;
  pHalInfo->ddCaps.dwSVBRops[7] = 0x80004000;

  pHalInfo->ddCaps.dwZBufferBitDepths  = DDBD_16 |  // Although handled by GUID_ZPIXELFORMATS, some apps
                                         DDBD_24;   // don't follow new conventions and rely on old norms

  // 16bit driver will fill this out
  pHalInfo->dwModeIndex = DDUNSUPPORTEDMODE;

  // Here we initialize the mode so that sst1init code maps
  // the card and returns addreses that can be passed to ddraw
  if (! fxinit(ppdev, FALSE))
    return FALSE;

  if (!D3DHALCREATEDRIVER(ppdev,
                          (LPD3DHAL_GLOBALDRIVERDATA*)&(pHalInfo->lpD3DGlobalDriverData),
                          (LPD3DHAL_CALLBACKS*)&(pHalInfo->lpD3DHALCallbacks)))
    return FALSE;

  // initialise instrumentation
  #if defined( INSTRUMENTATION )
  insInit();
  ppdev->insHeader = (DWORD *)&insHeader;
  #endif

  return TRUE;
}// DrvGetDirectDrawInfo

/*----------------------------------------------------------------------
Function name:  DrvEnableDirectDraw

Description:    NT style fucntion called to obtain pointers to the
                DirectDraw callbacks that the driver supports

Return:         BOOL

                TRUE -
----------------------------------------------------------------------*/
BOOL __stdcall
DrvEnableDirectDraw(DHPDEV                    dhpdev,
                    DDHAL_DDCALLBACKS         *pCallBacks,
                    DDHAL_DDSURFACECALLBACKS  *pSurfaceCallBacks,
                    DDHAL_DDPALETTECALLBACKS  *pPaletteCallBacks,
                    DDHAL_DDEXEBUFCALLBACKS   *pExebufCallbacks)
{
  NT9XDEVICEDATA *ppdev = (NT9XDEVICEDATA *)dhpdev;

  // fill in DirectDraw object callbacks
  memset(pCallBacks, 0, sizeof(DDHAL_DDCALLBACKS));
  pCallBacks->dwSize  = sizeof(DDHAL_DDCALLBACKS);

  pCallBacks->dwFlags = 0
                      | DDHAL_CB32_CREATESURFACE
                      | DDHAL_CB32_WAITFORVERTICALBLANK
                      | DDHAL_CB32_CANCREATESURFACE
#if ENABLE_PALETTE_CAPS
                      | DDHAL_CB32_CREATEPALETTE
#endif
                      | DDHAL_CB32_GETSCANLINE
                      | DDHAL_CB32_SETEXCLUSIVEMODE
                      | DDHAL_CB32_FLIPTOGDISURFACE
                      ;

  SETCALLBACK(pCallBacks->SetColorKey, DdSetDrvColorKey);
  pCallBacks->dwFlags |= DDHAL_CB32_SETCOLORKEY;

  SETCALLBACK(pCallBacks->CreateSurface,        DdCreateSurface);
  SETCALLBACK(pCallBacks->WaitForVerticalBlank, DdWaitForVerticalBlank);
  SETCALLBACK(pCallBacks->CanCreateSurface,     DdCanCreateSurface);
#if ENABLE_PALETTE_CAPS
  SETCALLBACK(pCallBacks->CreatePalette,        DdCreatePalette);
#endif
  SETCALLBACK(pCallBacks->GetScanLine,          DdGetScanLine);
  SETCALLBACK(pCallBacks->SetExclusiveMode,     DdSetExclusiveMode);
  SETCALLBACK(pCallBacks->FlipToGDISurface,     DdFlipToGDISurface);

  // fill in DirectDrawSurface object callbacks
  memset(pSurfaceCallBacks, 0, sizeof(DDHAL_DDSURFACECALLBACKS));
  pSurfaceCallBacks->dwSize  = sizeof(DDHAL_DDSURFACECALLBACKS);

  pSurfaceCallBacks->dwFlags = 0
                             | DDHAL_SURFCB32_DESTROYSURFACE
                             | DDHAL_SURFCB32_FLIP
                             | DDHAL_SURFCB32_LOCK
                             | DDHAL_SURFCB32_UNLOCK
                             | DDHAL_SURFCB32_BLT
#if ENABLE_OVERLAY_CAPS
                             | DDHAL_SURFCB32_SETCOLORKEY
#endif
                             | DDHAL_SURFCB32_ADDATTACHEDSURFACE
                             | DDHAL_SURFCB32_GETBLTSTATUS
                             | DDHAL_SURFCB32_GETFLIPSTATUS
#if ENABLE_OVERLAY_CAPS
                             | DDHAL_SURFCB32_UPDATEOVERLAY
                             | DDHAL_SURFCB32_SETOVERLAYPOSITION
#endif
#if ENABLE_PALETTE_CAPS
                             | DDHAL_SURFCB32_SETPALETTE
#endif
                             ;

  SETCALLBACK(pSurfaceCallBacks->DestroySurface,    DdDestroySurface);
  SETCALLBACK(pSurfaceCallBacks->Flip,              DdFlip);
  SETCALLBACK(pSurfaceCallBacks->Lock,              DdLock);
  SETCALLBACK(pSurfaceCallBacks->Unlock,            DdUnlock);
  SETCALLBACK(pSurfaceCallBacks->Blt,               DdBlt);
#if ENABLE_OVERLAY_CAPS
  SETCALLBACK(pSurfaceCallBacks->SetColorKey,       DdSetSurfaceColorKey);
#endif
  SETCALLBACK(pSurfaceCallBacks->AddAttachedSurface,DdAddAttachedSurface);
  SETCALLBACK(pSurfaceCallBacks->GetBltStatus,      DdGetBltStatus);
  SETCALLBACK(pSurfaceCallBacks->GetFlipStatus,     DdGetFlipStatus);
#if ENABLE_OVERLAY_CAPS
  SETCALLBACK(pSurfaceCallBacks->UpdateOverlay,     UpdateOverlay32);
  SETCALLBACK(pSurfaceCallBacks->SetOverlayPosition,SetOverlayPosition32);
#endif
#if ENABLE_PALETTE_CAPS
  SETCALLBACK(pSurfaceCallBacks->SetPalette,        DdSetPalette);
#endif

  // fill in DirectDrawPalette object callbacks
  memset(pPaletteCallBacks, 0, sizeof(DDHAL_DDPALETTECALLBACKS));
  pPaletteCallBacks->dwSize  = sizeof(DDHAL_DDPALETTECALLBACKS);

#if ENABLE_PALETTE_CAPS
  pPaletteCallBacks->dwFlags = 0
                             | DDHAL_PALCB32_DESTROYPALETTE
                             | DDHAL_PALCB32_SETENTRIES
                             ;

  SETCALLBACK(pPaletteCallBacks->DestroyPalette,    DdDestroyPalette);
  SETCALLBACK(pPaletteCallBacks->SetEntries,        DdSetEntries);
#endif

  if (IS_SAGE_ACTIVE) {
      pExebufCallbacks->dwSize = sizeof(DDHAL_DDEXEBUFCALLBACKS);
      SETCALLBACK(pExebufCallbacks->CanCreateExecuteBuffer,   CanCreateExecuteBuffer32);
      SETCALLBACK(pExebufCallbacks->CreateExecuteBuffer,      CreateExecuteBuffer32);
      SETCALLBACK(pExebufCallbacks->DestroyExecuteBuffer,     DestroyExecuteBuffer32);
      SETCALLBACK(pExebufCallbacks->LockExecuteBuffer,        LockExecuteBuffer32);
      SETCALLBACK(pExebufCallbacks->UnlockExecuteBuffer,      UnlockExecuteBuffer32);
      pExebufCallbacks->dwFlags =
          (pExebufCallbacks->CanCreateExecuteBuffer ? DDHAL_EXEBUFCB32_CANCREATEEXEBUF : 0) |
          (pExebufCallbacks->CreateExecuteBuffer    ? DDHAL_EXEBUFCB32_CREATEEXEBUF    : 0) |
          (pExebufCallbacks->DestroyExecuteBuffer   ? DDHAL_EXEBUFCB32_DESTROYEXEBUF   : 0) |
          (pExebufCallbacks->LockExecuteBuffer      ? DDHAL_EXEBUFCB32_LOCKEXEBUF      : 0) |
          (pExebufCallbacks->UnlockExecuteBuffer    ? DDHAL_EXEBUFCB32_UNLOCKEXEBUF    : 0) ;            
  }

  return TRUE;
}// DrvEnableDirectDraw

#if defined(DEBUG) && defined(RECTMM)
/*----------------------------------------------------------------------
Function name:  ddInitDebugLevel

Description:    Set debug output messages level.
				We are using D3D's scheme where multiple levels can be
				specified in the profile string.  We also try to preserve
				the way DDRAW's debug level is defined, see examples below.

				Examples:
				[3dfx]
				DD_DebugLevel = 4     ; enable level 1,2,3,4
				DD_DebugLevel = 24 4  ; enable level 24 and 1,2,3,4
				DD_DebugLevel = 0     ; no debug output
				_DD(DD_DebugLevel) is set to the last token value

Return:         NONE
----------------------------------------------------------------------*/

void ddInitDebugLevel(void)
{
  char  seps[] = " ,\t\n";
  char  *token;
  int   idx, iLevel;
  LPSTR lpStr;

  // initialize level array
  memset(debugLevel, 0, sizeof(debugLevel));
  debugLevel[0] = 0xff;                 // first entry (level 0) is always on

  lpStr = GETENV(DDDEBUGLEVEL);        // get profile string
  if (NULL == lpStr)
  {
    _DD(DD_DebugLevel) = 0;             // debug output off
  }
  else
  {
    token = getToken(lpStr, seps);      // read first token

    while( token != NULL )
    {
        iLevel = atoi(token);           // convert token to integer
        _DD(DD_DebugLevel) = iLevel;
        debugLevel[iLevel] = 0xff;

        if( iLevel >= DEBUG_APIENTRY  && iLevel <= DEBUG_D3DGORY )
        {
            // special case to handle DDRAW's debug level, enable lesser
            // levels up to iLevel

            for (idx = DEBUG_APIENTRY; idx < iLevel; idx++)
            {
                debugLevel[idx] = 0xff; // enable debug flag
            }
        }
        token = getToken( NULL, seps ); // read next token

     }  // endwhile: parse all tokens
  }     // elseif: profile string is not empty
}// ddInitDebugLevel
#endif  // DEBUG and RECTMM

/*----------------------------------------------------------------------
Function name:  fxinit

Description:    initialize relevant information in PDEV from the hardware

Return:         BOOL

                FXTRUE  -
----------------------------------------------------------------------*/

BOOL __stdcall fxinit
(
  NT9XDEVICEDATA *ppdev,
  BOOL setMode
)
{
  FxBool  fxStatus = FXTRUE;
  FxU32   width, height; 

  _DD(sst22DRegs) = (Sst2WxRegs*)(_FF(regBase) + SST_2D_OFFSET);
  _DD(sst2IORegs) = (Sst2IORegs*)(_FF(regBase) + SST_IO_OFFSET);
  _DD(sst23DPalette) = (Sst2Regs*)(_FF(regBase) + SST_3D_OFFSET + SST_PALETTE );
  _DD(sst23DRegs) = (Sst2Regs*)(_FF(regBase) + SST_3D_OFFSET + SST_FBI );
  _DD(sst2CRegs) = (Sst2CRegs*)(_FF(regBase) + SST_CMD_OFFSET);
  _DD(sst2VidRegs) = (Sst2VidRegs*)(_FF(regBase) + SST_VID_OFFSET );

  _DD(overlaySurfaceCnt) = 0;
  _DD(fbiMemorySize) = GETVIDEOMEMSIZE(ppdev) << 20;
  _DD(fxCaps) |= FXCAPS_PALETTIZED_TEXTURES;

  width = _DD(fbiWidth) = _FF(HALInfo).vmiData.dwDisplayWidth;
  height = _DD(fbiHeight) = _FF(HALInfo).vmiData.dwDisplayHeight;

  _DD(bufferUsage) = 0;
  _FF(ddVisibleOverlaySurf) = 0;

#ifdef DEBUG
 #ifdef RECTMM
  ddInitDebugLevel();
 #else
  // System Environment
  if (NULL == GETENV(DDDEBUGLEVEL))
    _DD(DD_DebugLevel) = 0;
  else
    _DD(DD_DebugLevel) = atoi(GETENV(DDDEBUGLEVEL));
 #endif
#endif  // DEBUG

  if (NULL == GETENV(SWAPINTERVAL))
    _DD(WaitOnVsync) = 1;
  else
    _DD(WaitOnVsync) = atoi(GETENV(SWAPINTERVAL));

  // Initialize surface flipping variables.

  _DD(ddSurfaceFlippedFrom) = 0;
  _DD(ddSurfaceFlippedTo)   = 0;
  _DD(ddAcceleratorUsed)    = 0;
  _DD(ddLastSwapCount) = READSWAPCOUNT(ppdev);

  if (NULL == GETENV(OVERLAYMODE))
    _DD(overlayFilter) = 0;
  else
    _DD(overlayFilter) = atoi(GETENV(OVERLAYMODE));

  // Initially no locks outstanding
  _FX(openLockCount) = 0;
  _FX(openLockIndex) = 0;

  return FXTRUE;
}// fxinit

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

/*----------------------------------------------------------------------
Function name:  getVideoMemSize

Description:

Return:         DWORD memSize -
----------------------------------------------------------------------*/

static DWORD __fastcall
GETVIDEOMEMSIZE(NT9XDEVICEDATA *ppdev)
{
  DWORD memSize = 8;
  DWORD dramInit0 = 0;


  // dramInit0 = GET(ghwIO->dramInit0);     !! SST2
// !! SST2  if(dramInit0 & SST_SGRAM_TYPE_16MBIT)
// !! SST2    memSize = 16;

// !! SST2  if(dramInit0 & SST_SGRAM_NUM_CHIPSETS)
// !! SST2    memSize *= 2;

  return memSize;
}// getVideoMemSize

