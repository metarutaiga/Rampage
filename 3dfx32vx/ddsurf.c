/* -*-c++-*- */
/* $Header: ddsurf.c, 87, 12/8/00 9:14:16 AM PST, Ryan Bissell$ */
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
** File Name: 	DDSURF.C
**
** Description: Direct Draw Surface support.
**
** $Revision: 87$
** $Date: 12/8/00 9:14:16 AM PST$
**
** $History: ddsurf.c $
** 
** *****************  Version 90  *****************
** User: Mconrad      Date: 9/09/99    Time: 5:34p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fix typo in packet header declaration.
** 
** *****************  Version 89  *****************
** User: Mconrad      Date: 9/09/99    Time: 5:00p
** Updated in $/devel/sst2/Win95/dx/dd32
** Add mop's on texture locks with DDLOCK_WAIT flag set.
** 
** *****************  Version 88  *****************
** User: Evan         Date: 9/07/99    Time: 4:03p
** Updated in $/devel/sst2/Win95/dx/dd32
** fixes bump map ddunock
** 
** *****************  Version 87  *****************
** User: Evan         Date: 9/07/99    Time: 3:24p
** Updated in $/devel/sst2/Win95/dx/dd32
** use fpVidMem in the call to FxCamFreeEntry in DdUnlock, rather than
** dwReserved1
** 
** *****************  Version 86  *****************
** User: Andrew       Date: 9/07/99    Time: 10:37a
** Updated in $/devel/sst2/Win95/dx/dd32
** Changes for SLI
** 
** *****************  Version 85  *****************
** User: Bburton      Date: 9/03/99    Time: 5:45p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added UVL556 bumpmap texture munging on DdUnlock() for UVL support.
** 
** *****************  Version 84  *****************
** User: Bdaniels     Date: 8/31/99    Time: 1:23p
** Updated in $/devel/sst2/Win95/dx/dd32
** Adding 32 bpp rendering, 24 bit z / 8 bit stencil support.
** 
** *****************  Version 83  *****************
** User: Einkauf      Date: 8/31/99    Time: 2:00p
** Updated in $/devel/sst2/Win95/dx/dd32
** DirectX CMD FIFO
** 
** *****************  Version 82  *****************
** User: Evan         Date: 8/30/99    Time: 10:12a
** Updated in $/devel/sst2/Win95/dx/dd32
** rearranges cam management code to be in one place
** 
** *****************  Version 81  *****************
** User: Xingc        Date: 8/27/99    Time: 7:41p
** Updated in $/devel/sst2/Win95/dx/dd32
** FXSURFACEDATA structure change
** 
** *****************  Version 80  *****************
** User: Mconrad      Date: 8/27/99    Time: 11:01a
** Updated in $/devel/sst2/Win95/dx/dd32
** Setup dwPBytestride in surface data structure.
** 
** *****************  Version 79  *****************
** User: Evan         Date: 8/25/99    Time: 6:13p
** Updated in $/devel/sst2/Win95/dx/dd32
** updates to cam management code
** 
** *****************  Version 78  *****************
** User: Evan         Date: 8/25/99    Time: 10:52a
** Updated in $/devel/sst2/Win95/dx/dd32
** Adds new parameter to FxCamProgram so that a specific cam entry can be
** specified.
** 
** *****************  Version 77  *****************
** User: Evan         Date: 8/24/99    Time: 11:03a
** Updated in $/devel/sst2/Win95/dx/dd32
** fixes ddunlock for texture surface: sends correct address to
** phantom_freeSurface
** 
** *****************  Version 76  *****************
** User: Evan         Date: 8/23/99    Time: 4:23p
** Updated in $/devel/sst2/Win95/dx/dd32
** completes texture locking support
** 
** *****************  Version 75  *****************
** User: Evan         Date: 8/21/99    Time: 1:44p
** Updated in $/devel/sst2/Win95/dx/dd32
** texture lock support
** 
** *****************  Version 74  *****************
** User: Agus         Date: 8/13/99    Time: 2:04p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added save surface pixel format and modified promote_primarytooverlay
** func params and corresponding macros.
** 
** *****************  Version 73  *****************
** User: Mconrad      Date: 8/10/99    Time: 9:43p
** Updated in $/devel/sst2/Win95/dx/dd32
** Enable 3D code, disable texture locks, move old AA and
** automipmap code into deadcode sections. 
** 
** *****************  Version 72  *****************
** User: Xingc        Date: 8/04/99    Time: 6:26p
** Updated in $/devel/sst2/Win95/dx/dd32
** Pass pWidth into CreateOveralySurface()
** 
** *****************  Version 71  *****************
** User: Xingc        Date: 8/04/99    Time: 5:43p
** Updated in $/devel/sst2/Win95/dx/dd32
** Surface support for video port YUY2 surface
** 
** *****************  Version 70  *****************
** User: Xingc        Date: 8/03/99    Time: 5:41p
** Updated in $/devel/sst2/Win95/dx/dd32
** Change FOURCC_VBI into FOURCC_RAW8, delete FOURCC_YUVA
** 
** *****************  Version 69  *****************
** User: Agus         Date: 8/03/99    Time: 12:51p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added check for DDSCAPS2_HINTANTIALIASING in ddCreateSurface for AA
** surface creation.
** 
** *****************  Version 68  *****************
** User: Peterm       Date: 7/30/99    Time: 2:41a
** Updated in $/devel/sst2/Win95/dx/dd32
** modified for updated fxsurfacedata struct
** 
** *****************  Version 67  *****************
** User: Einkauf      Date: 7/29/99    Time: 12:59p
** Updated in $/devel/sst2/Win95/dx/dd32
** remove temp fix to Physical MM which forced phys mem width to be power
** of 2; enable Phantom MM's code that increases region size to compensate
** for non-power of 2 original stride; bug fix in phantomlfbPtr stored;add
** prologs to CAM mgr subroutines
** 
** *****************  Version 66  *****************
** User: Einkauf      Date: 7/28/99    Time: 2:07p
** Updated in $/devel/sst2/Win95/dx/dd32
** added phantom mem mgr - modified existing memmgr to use either physical
** or phantom memory; added interface routines; modified DdLock/DdUnlock
** to interface to phantom mm.
** 
** *****************  Version 65  *****************
** User: Agus         Date: 7/24/99    Time: 5:42p
** Updated in $/devel/sst2/Win95/dx/dd32
** Ddraw clean-up in ddCreateSurface
** 
** *****************  Version 64  *****************
** User: Xingc        Date: 7/23/99    Time: 12:55p
** Updated in $/devel/sst2/Win95/dx/dd32
** delete  a linear surface test case
** 
** *****************  Version 63  *****************
** User: Xingc        Date: 7/23/99    Time: 12:36p
** Updated in $/devel/sst2/Win95/dx/dd32
** Calculate mstride and pstride for all type of surfaces. Increase
** endAddress of CAM for overlay.
** 
** *****************  Version 62  *****************
** User: Xingc        Date: 7/22/99    Time: 6:22p
** Updated in $/devel/sst2/Win95/dx/dd32
** CreateSurface clear up
** 
** *****************  Version 61  *****************
** User: Peterm       Date: 7/22/99    Time: 4:48p
** Updated in $/devel/sst2/Win95/dx/dd32
** cleaned up csim server flip backdoor code, fixed surface allocation for
** overlay surfaces (with Xing C)
** 
** *****************  Version 60  *****************
** User: Agus         Date: 7/22/99    Time: 11:03a
** Updated in $/devel/sst2/Win95/dx/dd32
** Fixes to get flip2d cube and foxbear running, added check for
** PARTOFPRIMARYCHAIN surface creation to be the same as primary's
** tileflag, precalculate pStride, mStride, change CAM programming to use
** pre-calc pStride, mStride during Lock.
** 
** *****************  Version 59  *****************
** User: Agus         Date: 7/19/99    Time: 4:54p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fixed surface's pitch returned to application to be MStride (power of
** two) stride of the surface pitch; this corrects Flip2D application
** stride problem.
** 
** *****************  Version 58  *****************
** User: Agus         Date: 7/19/99    Time: 2:53p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added inlude sst2glob.h
** 
** *****************  Version 57  *****************
** User: Agus         Date: 7/19/99    Time: 2:49p
** Updated in $/devel/sst2/Win95/dx/dd32
** Set offscreen primary surface tile type to be the same as desktop's
** tile flags
** 
** *****************  Version 56  *****************
** User: Peterm       Date: 7/19/99    Time: 12:08p
** Updated in $/devel/sst2/Win95/dx/dd32
** fixed pitch calculation in DDCreateSurface for sst2
** 
** *****************  Version 55  *****************
** User: Agus         Date: 7/16/99    Time: 4:16p
** Updated in $/devel/sst2/Win95/dx/dd32
** Deleted V3 history comments from history block.
** 
** *****************  Version 54  *****************
** User: Agus         Date: 7/16/99    Time: 3:16p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added SST2 device service call connection during exclusive primary
** surface creation, device service release during exclusive mode surface
** destruction, added fxcamfreeentry on non texture surfaces
** 
** *****************  Version 53  *****************
** User: Einkauf      Date: 7/08/99    Time: 7:31p
** Updated in $/devel/sst2/Win95/dx/dd32
** CAM locking for general surfaces, BLT's work better - using proper
** dst/srcFormat settings
** 
** *****************  Version 52  *****************
** User: Agus         Date: 7/06/99    Time: 1:09p
** Updated in $/devel/sst2/Win95/dx/dd32
** Modify ddCreateSurface call to surfMgr_AllocSurface
** 
** *****************  Version 51  *****************
** User: Agus         Date: 7/01/99    Time: 4:43p
** Updated in $/devel/sst2/Win95/dx/dd32
** Comment out invalid HeapID check in ddDestroySurface
** 
** *****************  Version 50  *****************
** User: Agus         Date: 6/25/99    Time: 5:44p
** Updated in $/devel/sst2/Win95/dx/dd32
** Modify the interface to memory manager to use surfMgr_AllocSurface &
** surfMgr_FreeSurface and the passed parameters.
** 
** *****************  Version 49  *****************
** User: Xingc        Date: 6/14/99    Time: 2:52p
** Updated in $/devel/sst2/Win95/dx/dd32
** Put R2.1 overlay and vpe code back. Add mulit-mon support.
** 
** *****************  Version 48  *****************
** User: Peterm       Date: 6/04/99    Time: 2:01a
** Updated in $/devel/sst2/Win95/dx/dd32
** updated for changed inc\sst* and others
** 
** *****************  Version 47  *****************
** User: Peterm       Date: 6/03/99    Time: 11:25p
** Updated in $/devel/sst2/Win95/dx/dd32
** modified to run with H3 tot (adds multimon, various bug fixes, and many
** structural deltas)
** 
*/

#include "precomp.h"
#include "ddovl32.h"
#include "sst2glob.h"
#include "fifomgr.h"
#include "iSST2.h"      // SST2 csim server
#include "iR3.h"        // R3 csim server
#include "iHydra.h"     // Hydra csim server
#include "iRage.h"      // Rage csim server
#include "ddcam.h"      // cam management routines
#include "d3txtr.h"
#include "d3txtr2.h"    // new texture calls
#include "regkeys.h"    // needed for fullscreen aa reg check


#ifdef SLI
#include <ddsli.h>
#endif

#define PROMOTE_PRIMARYTOOVERLAY(arg1,arg2,arg3,arg4) promote_PrimaryToOverlay(arg1,arg2,arg3,arg4)
#define DEMOTE_3DTONONEOVERLAY(arg1) demote_3DToNoneOverlay(arg1)

extern DWORD PROMOTE_PRIMARYTOOVERLAY(NT9XDEVICEDATA *, DWORD,DWORD,FXSURFACEDATA *);
extern DWORD DEMOTE_3DTONONEOVERLAY(NT9XDEVICEDATA *);

#define RNDUP_16KB(size) (((size) + 0x3fffL) & ~0x3fffL)
#define RNDUP_T1_PGWIDTH(width) (((width) + 0x3L) & ~0x3L)

#define FLUSHAGP(arg1) flushAgp(arg1)

#define LEGACY_APPS 1   // !!! fix PRS 2661

#if defined(LEGACY_APPS)

/*
*****************************************************************************
 Fix PRS 2661: MS golf creates a complex/flipping/3ddevice/primary surface
 and a back buffer. It never flips, it does not render 3D images and it uses
 GDI API to render all scenes. When we Alt-tab to desktop and then Alt-tab
 back to the game, MS golf neither locks the primary buffer, nor restores
 any surfaces that are lost during our switch to the desktop.  It calls GDI's
 memory to screen bitblt to restore the scenery as soon as we are back to
 exclusive mode. When we move the yard stick, or swing the club, it then
 restores the back buffer.  At this point, we promote the primary surface to
 tile mode as we do for all 3ddevices, and trash the screen. To remedy such
 ill behaved game, we have to fail the ddCanCreateSurface.

 Credits to Russ's ingenuity.
*****************************************************************************
*/

static BOOL IsLegacyApp(DWORD);
static VOID GetProcessFileName(LPSTR, DWORD);

void   __stdcall Set_FSAAMode( LPDDHAL_CREATESURFACEDATA pcsd, DWORD dwCaps2,  
                               DWORD dwCaps, LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl );  

typedef struct tag_LegacyAppID
{
  PSTR  pszProcFileName;    // legacy app's module filename
  DWORD dwSurfaceCaps;      // surface caps request
} LEGACYAPPID;

static LEGACYAPPID LegacyApps[] =
{
  { "GAME.EXE",
    DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX |
    DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE
  },
};


/*----------------------------------------------------------------------
Function name: Set_WindowedAAMode

Description:    Sets a global flag for Windowed AA mode based on registry or Hint AA flag
                Valid modes for the registry to set are 0,1 or 2
                Setting mode 0 in the registry disables windowed AA even if the app requests it.
                Setting mode 1 in the registry allows the app to turn windowed AA on and off with Hint flag
                Setting mode 2 forces 4 sample AA on for 3D windowed apps 

                This should work for either DX7 or DX8.


Return          void
----------------------------------------------------------------------*/
void __stdcall Set_WindowedAAMode( LPDDHAL_CREATESURFACEDATA pcsd, DWORD dwCaps, DWORD dwCaps2 )
{
    LPSTR   lpStr = NULL;
    DWORD   dwValue = 0;
    DWORD   WNAARequested;
    DWORD   nSamples = 4;   //default number of AA samples

    DD_ENTRY_SETUP(pcsd->lpDD);


    //only check registry once - during primary surface creation.

    if( dwCaps & DDSCAPS_PRIMARYSURFACE )
    {

        // check registry to see if we want to force windowed AA on 
        lpStr = GETENV(FORCE_WN_AA_NAME); 
        if (NULL != lpStr)
        {
          dwValue = ddatoi(lpStr);
          if (dwValue < 0)
              dwValue = 1;
          if (dwValue > WNAA_MODE_MAX )  
              dwValue = 1;                

          _DD(ddAARegistryMode) = dwValue; 
        }
        else
        {
          _DD(ddAARegistryMode) = WNAA_MODE_APP_CONTROLLED; // default to normal ( application requested only ) AA
        }
        
    }

    // look at Ddraw surface AA hint flag 
    if( dwCaps2 && DDSCAPS2_HINTANTIALIASING)
    {
       WNAARequested = TRUE;
    }
    else
    {
       WNAARequested = FALSE; 
    }


#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    // check dx8 multisample info to see if AA is requested and how many samples app wants
    {
        // there may be a cleaner way to extract this value...
        nSamples = ((DDSURFACEDESC2 *) pcsd->lpDDSurfaceDesc)->ddsCaps.dwCaps3 & DDSCAPS3_MULTISAMPLE_MASK;
    }
#endif



    switch( _DD(ddAARegistryMode) )
    {
        case WNAA_MODE_NONE:
           _DD(ddWindowedAAMode) = WNAA_MODE_NONE;    // Registry said force windowed AA off
           break;
        case WNAA_MODE_APP_CONTROLLED:                // if Registry says AA is OK, let app chose mode
           if( WNAARequested && ( nSamples == 4 ) )
               _DD(ddWindowedAAMode) = WNAA_MODE_4X;
           else if( WNAARequested && ( nSamples == 16) )
               _DD(ddWindowedAAMode) = WNAA_MODE_16X;
           else
               _DD(ddWindowedAAMode) = WNAA_MODE_NONE; // we only do 4 or 16 right now
           break;
         case WNAA_MODE_4X:
           _DD(ddWindowedAAMode) = WNAA_MODE_4X;      // Registry said force 4 sample AA
           break;
         case WNAA_MODE_16X:
           _DD(ddWindowedAAMode) = WNAA_MODE_16X;     // Registry said force 16 sample AA
           break;
        default:
           _DD(ddWindowedAAMode) = WNAA_MODE_NONE;    // Registry is confused, turn aa off
           break;
    }

}



/*----------------------------------------------------------------------
Function name:  Set_FSAAMode

Description:    Sets a global flag for FSAA mode based on registry or Hint AA flag
                Valid modes for the registry to set are 0,1,2 or 3.
                Setting mode 0 in the registry disables FSAA even if the app requests it.
                Setting mode 1 in the registry allows the app to turn FSAA on and off with Hint flag
                Setting mode 2 forces FSAA on in the normal mode
                Setting mode 3 forces FSAA on in the "memory saver" AA blt mode.

                The fallback mechanizm for dropping out of AA mode if we don't have enough
                memory is not entirely finished. I am checking the total amount of VRAM
                available but I do not yet have a means of checking whether that memory 
                is available in "surface" sized chunks. It is still possible at this time
                that we could set an AA mode that will not actually have enough memory
                to create the back and z-buffers. When the fxmm_queryheap() function is finished that
                should take care of this problem.

                Note also that currently if the FSAA registry key is set to 1, and the app sets the AA hint
                flag, we will first attempt to use flip AA mode as a default. 
                     
Return:         void
----------------------------------------------------------------------*/
void __stdcall Set_FSAAMode( LPDDHAL_CREATESURFACEDATA pcsd, DWORD dwCaps2, DWORD dwCaps,
                             LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl) 
{
    LPSTR   lpStr = NULL;
    DWORD   dwValue = 0;
    DWORD   tileInX;
    DWORD   tileInY;
    DWORD   screenSize;
    DWORD   FreeSpace;
    DWORD   height;
    DWORD   newAAMode;
    DWORD   FSAARequested;    
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    DWORD   nSamples = 0; 
#endif

    DD_ENTRY_SETUP(pcsd->lpDD);

    // look at DX AA hint flag - this does not necessarily mean the app will get FSAA, just that he wants it.
    if( dwCaps2 && DDSCAPS2_HINTANTIALIASING)
    {
       FSAARequested = TRUE;  
    }
    else
    {
       FSAARequested = FALSE; 
    }

    // check registry to see if we want to force fs AA on 
    lpStr = GETENV(FORCE_FS_AA_NAME); 
    if (NULL != lpStr)
    {
      dwValue = ddatoi(lpStr);
      if (dwValue < 0)
          dwValue = 0;
      if (dwValue > FSAA_MODE_MAX )  
          dwValue = 0;               // mode not supported 

      _DD(ddAARegistryMode) = FSAA_MODE_NONE;
      newAAMode = dwValue; 
    }
    else
    {
        _DD(ddAARegistryMode) = FSAA_MODE_NONE;
        _DD(ddFSAAMode) = FSAA_MODE_NONE;
        return; // leave now if we're not going to FSAA
    }
 
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    // this code is not entirely necessary since at the moment we only support 4 sample AA
    // if at some point we wanted to support 8 sample or 16 sample through some form of trickery,
    // then appropriate selection code would be needed here to set the mode to "8XFlIP" or whatever.
     nSamples = ((DDSURFACEDESC2 *) pcsd->lpDDSurfaceDesc)->ddsCaps.dwCaps3 & DDSCAPS3_MULTISAMPLE_MASK;
#endif

    // If registry sets mode 1, follow the state of the Hint flag...
    if( newAAMode == FSAA_MODE_ONE    )
    {
        
        if(   !FSAARequested  )   
        {
            _DD(ddFSAAMode) = FSAA_MODE_NONE;
            return; // app did not request AA so we're done
        }
        else
        {
            // if reg key is 1 we use 4XFLIP as the default aa method ( if it fits in memory )
            newAAMode = FSAA_MODE_4XFLIP;
        }
    }
    
    switch( newAAMode )
    {
        case  FSAA_MODE_4XFLIP:
            // If FSAA app is not going to flip then force it into blt AA mode
            // I put this in primarily for MS flight simulator.
            // By default Flight Simulator 2000 doesn't flip and doesn't set the primary as an overlay -
            // unless you tell it to flip in the .cfg file
            if( (dwCaps & DDSCAPS_PRIMARYSURFACE) && !(dwCaps & DDSCAPS_FLIP)  )
            {
                newAAMode = FSAA_MODE_4XBLT;
            }
            break;
        default:
            break;
    }


    height =  psurf_gbl->wHeight;                     
    tileInX = (psurf_gbl->wWidth * GETPRIMARYBYTEDEPTH + SST2_TILE_WIDTH - 1) >> SST2_TILE_WIDTH_BITS;
    tileInX = RNDUP_T1_PGWIDTH(tileInX);
    tileInY = (height + SST2_TILE1_HEIGHT - 1) >> SST2_TILE1_HEIGHT_BITS;
    screenSize = (tileInX << SST2_TILE_WIDTH_BITS) * (tileInY << SST2_TILE1_HEIGHT_BITS);                     

    screenSize = RNDUP_16KB(screenSize);                    
    // calculate the absolute maximum amount of vram we could get.
    FreeSpace = _FF(TotalVRAM) - _FF(mmPersistentHeapSize); 

    // flip mode FSAA takes memory equal to 12x the requested primary size
    if(  newAAMode == FSAA_MODE_4XFLIP )
    {
        if( FreeSpace < screenSize*12)
        {
            newAAMode = FSAA_MODE_4XBLT; // switch to blt mode if we have to
        }
        else 
        {
//  tbd...    
//            if( fxmm_queryheap( &_FF(mmTransientHeap) , 3, screenSize, 0, 0 ) )
//            {
//                 _DD(ddFSAAMode) = FSAA_MODE_NONE;
//                 return; // Not enough memory for AA - no point in continuing
//            }
        }


    }

    if( newAAMode == FSAA_MODE_4XBLT )
    {
        // blt mode FSAA takes memory equal to 9x the requested primary size
        if ( FreeSpace < screenSize*9 )
        {
         _DD(ddFSAAMode) = FSAA_MODE_NONE;
         return; // Not enough memory for AA - no point in continuing
        }
        // if we get this far we have enough total memory in theory for our blt AA mode,
        // now see if we can actually get it in big enough contiguous chunks to create
        // 1 render buffer and a z-buffer
  //  tbd...         if( fxmm_queryheap( &_FF(mmTransientHeap) , 2, screenSize, 0, 0 ) )
  //           {
  //              _DD(ddFSAAMode) = FSAA_MODE_NONE;
  //              return; // Not enough memory for AA - no point in continuing
  //           }

    }
    
   
    // WORK STILL NEEDED...
    // mls note - need some way to reclaim nml heap if only in blt mode.
    // also need to add test for available memory in SLI modes.


    
    // if we are using a fullscreen aa mode,
    // initialize the aa buffer addresses .
    _DD(ddAAZBufferStart) = 0;          
    _DD(ddAASecondaryBuffer1Start) = 0;  
    _DD(ddAASecondaryBuffer2Start) = 0; 
    _DD(ddFSAAMode) = newAAMode;

}






/*----------------------------------------------------------------------
Function name:  GetProcessFileName

Description:    Get the file name of the calling process.

                If the file name is retreived, set dwSize to the
                number of characters in the file name and lpBuf
                to the file name.

                Otherwise, set lpBuf to NULL.

Return:         NONE
----------------------------------------------------------------------*/
static VOID GetProcessFileName(LPSTR lpBuf, DWORD dwSize)
{
  HANDLE hFile;
  DWORD  i;

  // GetModuleFileName returns length of filename (exclude null terminated
  // char) if successful, else returns zero

  hFile = GetModuleHandle(NULL);
  if ( (i = GetModuleFileName(hFile, lpBuf, dwSize)) )
  {
    // locate first char of filename, excluding pathname

    i--;                    // index starts from zero, points at last char
    while ( (i >=0) && (lpBuf[i] != '\\') )
    {
        i--;
    }
    i++;                    // lpBuf[0] points to first char of filename
    strcpy(lpBuf, &lpBuf[i]);
  }
  else {
    lpBuf[0] = '\0';        // indicate we did not get the process name
  }
}// GetProcessFileName


/*----------------------------------------------------------------------
Function name:  IsLegacyApp

Description:    determine if the process calling this driver is
                a "legacy application"

Return:         BOOL
				TRUE  - if calling process is legacy app
				FALSE - if not calling process legacy app
----------------------------------------------------------------------*/
static BOOL IsLegacyApp(DWORD dwSurfaceCaps)
{
  CHAR  szBuffer[MAX_PATH];
  LEGACYAPPID *pLegacyApp;

  GetProcessFileName(szBuffer, sizeof(szBuffer));
  if ('\0' == szBuffer[0])
    return FALSE;

  for (pLegacyApp = &LegacyApps[0];
       pLegacyApp < &LegacyApps[sizeof(LegacyApps)/sizeof(LegacyApps[0])];
       pLegacyApp++)
  {
    if ( (!strcmp(pLegacyApp->pszProcFileName, szBuffer)) &&
         (pLegacyApp->dwSurfaceCaps & dwSurfaceCaps)
       )
      return TRUE;
  }

  return FALSE;
}

#endif    // LEGACY_APPS


/*----------------------------------------------------------------------
Function name:  DdCanCreateSurface

Description:    DDRAW callback CanCreateSurface

                Called by DirectDraw to determine if the driver can
                create a particular off-screen surface type.

				This entry point is called after parameter validation
                but  before any object creation.   You can decide
                here if it is possible for you to create this surface.

				pccsd->bIsDifferentPixelFormat tells us if the pixel
                format of the surface being created matches that of
                the primary surface.

				The surfaces we allow:

				  texture:      rgb 8, 16, power of 2
				  z:            16 bit, 24 bit, 32 bit (8stencil, 24z)
				  overlay:      yuy2, uyvy, rgb 16
				  primary       rgb 8, 16, 24, 32
				  back buffers: rgb 8, 16, 24, 32
				  offscreen:    yuy2, uyvy, rgb 8, 16, 24, 32

				 notes:
				  1. overlay has to be checked before color buffer.
                     I use DDSCAPS_COMPLEX to check for tripple
                     buffer.  an overlay can be DDSCAPS_COMPLEX and
                     we don't want overlay to fall into color buffer
                     catagory.
				  2. color buffers has to be checked before offscreen.
                     DDSCAPS_VIDEO is sometimes use as a
                     DDSCAPS_OFFSCREEN and we don't want the color
                     buffers fall into DDSCAPS_OFFSCREEN catagory.
				  3. 3DDEVICE is not checked even though we can only
                     support 16bpp for 3D render.  This is to work
                     around some games, i.e. Jedi Knight

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
DdCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pccsd )
{
  LPDDSURFACEDESC      lpddsd;
  DWORD                dwFlags, dwBpp, dwCaps, pixelBitDepth;
  DWORD dwUnsupported = DDSCAPS_LIVEVIDEO | DDSCAPS_VIDEOMEMORY | DDSCAPS_OVERLAY;
  DD_ENTRY_SETUP(pccsd->lpDD);

  #ifdef FXTRACE
    DISPDBG((ppdev, DEBUG_APIENTRY, "CanCreate Surface32" ));
    DUMP_SURFACEDESC(ppdev, DEBUG_DDGORY, pccsd->lpDDSurfaceDesc );
  #endif

  lpddsd = pccsd->lpDDSurfaceDesc;
  dwFlags = lpddsd->dwFlags;
  dwCaps = lpddsd->ddsCaps.dwCaps;

#if defined(LEGACY_APPS)

  if ( ((DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP |
         DDSCAPS_VIDEOMEMORY | DDSCAPS_COMPLEX) == dwCaps) ||
       ((DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY)
         == dwCaps)
     )
  {
     if ( IsLegacyApp(dwCaps) )
     {
        #ifdef DEBUG
        _asm { int 3 };   // we want to know which app is labeled as legacy
        #endif

        pccsd->ddRVal = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
     }
  }
#endif  // LEGACY_APPS

	/*
	Two tuner cards (TurboTv and STB) try to allocate a very wide surface and
	then ask us to shrink it appropriately whilst they squirt LIVEVIDEO over the
	PCI bus.  We really can't support this, so deny a surface if it's LIVEVID.
	*/
#if 0 //Tiny TV-Tuner requires this to be disabled
	if ((dwCaps & dwUnsupported) == dwUnsupported)
	{
		pccsd->ddRVal = DDERR_INVALIDPARAMS;
	    return DDHAL_DRIVER_HANDLED;
	}
#endif

  if ( dwCaps & DDSCAPS_TEXTURE )
  {
      pccsd->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
  }

#ifdef AGP_EXECUTE
  if (dwCaps & DDSCAPS_NONLOCALVIDMEM) 
  {
    if ( (_DD(agpMode) & EXECUTE_MODE_BLTS) && 
         (dwCaps & DDSCAPS_OFFSCREENPLAIN ) )
    {
      pccsd->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
    }
    else
    {
      pccsd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
    }
  }
#endif


  // z : 16 bit, 24 bit and 24Z/8 Stencil = 32 bit
  if (dwCaps & DDSCAPS_ZBUFFER) // Zbuffer surfaces
  {
    if (pccsd->bIsDifferentPixelFormat)             // Determine bits per pixel requested
    {
        dwBpp = lpddsd->ddpfPixelFormat.dwRGBBitCount;
    }
    else
    {
        dwBpp = (DWORD) GETPRIMARYBYTEDEPTH << 3L;  // primary surface
    }

    if (lpddsd->dwFlags & DDSD_ZBUFFERBITDEPTH)
    {
        dwBpp = lpddsd->dwZBufferBitDepth;
    }

    if ((dwBpp == 16) || (dwBpp == 24) || (dwBpp == 32)) // 16bpp/24bpp/32bpp zbuffer
    {
        pccsd->ddRVal = DD_OK;
    }
  }

  pixelBitDepth = GETPRIMARYBYTEDEPTH << 3L;
  if((pccsd->bIsDifferentPixelFormat) && (dwFlags & DDSD_PIXELFORMAT))
  {
      // on NT, this sets pixelBitDepth to zero for YUY2 & UYVY
      // but since it's not used later for these format's it's ok
      if(lpddsd->ddpfPixelFormat.dwFlags & DDPF_RGB)
        pixelBitDepth = lpddsd->ddpfPixelFormat.dwRGBBitCount;
  }

  if(dwCaps & DDSCAPS_OVERLAY)
  {
    if(dwFlags & DDSD_PIXELFORMAT )
    {
        pccsd->ddRVal= CanCreateOverlaySurface(ppdev,
                 &(lpddsd->ddpfPixelFormat), pixelBitDepth, dwCaps);
    }
    else
        pccsd->ddRVal=
            CanCreateOverlaySurface(ppdev, 0,pixelBitDepth, dwCaps);

       return DDHAL_DRIVER_HANDLED;
  }

  if(dwCaps & DDSCAPS_VIDEOPORT)
  {
    //video port only support UYVY and RAW8
    if((dwFlags & DDSD_PIXELFORMAT ) &&
       (lpddsd->ddpfPixelFormat.dwFlags & DDPF_FOURCC) &&
       ((lpddsd->ddpfPixelFormat.dwFourCC  == FOURCC_UYVY) &&
        (lpddsd->ddpfPixelFormat.dwYUVBitCount  == 16)) ||
       ((lpddsd->ddpfPixelFormat.dwFourCC  == FOURCC_RAW8) &&
        (lpddsd->ddpfPixelFormat.dwYUVBitCount  == 8))) 
    {
       pccsd->ddRVal = DD_OK;

    }
    else
    {
       pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
       
    }

    return DDHAL_DRIVER_HANDLED;

  }

  // all color buffers: 8, 16, 24, 32 bpp.
  // I hope DDSCAPS_COMPLEX will include tripple buffer
  // this come after overlay, so ovelay complex is already taken care of -SS
  if((dwCaps & DDSCAPS_PRIMARYSURFACE) || (dwCaps & DDSCAPS_COMPLEX)
     || (dwCaps & DDSCAPS_FRONTBUFFER) || (dwCaps & DDSCAPS_BACKBUFFER))
  {
    if(dwFlags & DDSD_PIXELFORMAT )
    {
      if(!(lpddsd->ddpfPixelFormat.dwFlags & DDPF_RGB))
      {
        pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        return DDHAL_DRIVER_HANDLED;
      }
    }

    if( pixelBitDepth == 8 || pixelBitDepth == 16 ||
          pixelBitDepth == 24 || pixelBitDepth==32)
    {
      pccsd->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
    }

    pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
    return DDHAL_DRIVER_HANDLED;
  }


  // offscreen or video memory: yuy2, uyuv, 8, 16, 24, 32 bpp rgbs
  // video memory is done last since we want to check the other flags first
  // at this point, videomemory is considered the same as offscreen -SS
  if((dwCaps & DDSCAPS_OFFSCREENPLAIN)
      || (dwCaps & DDSCAPS_VIDEOMEMORY))
  {
    if(dwFlags & DDSD_PIXELFORMAT )
    {
      if(lpddsd->ddpfPixelFormat.dwFlags & DDPF_FOURCC)
      {
        if (   (lpddsd->ddpfPixelFormat.dwFourCC == FOURCC_YUY2)
            || (lpddsd->ddpfPixelFormat.dwFourCC == FOURCC_UYVY))
        {
            pccsd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }
        pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
        return DDHAL_DRIVER_HANDLED;
      }
    }

    if( pixelBitDepth == 8 || pixelBitDepth == 16 ||
           pixelBitDepth == 24 || pixelBitDepth==32)
    {
      pccsd->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
    }

    pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
    return DDHAL_DRIVER_HANDLED;

  }

  pccsd->ddRVal = DDERR_UNSUPPORTED;
  return DDHAL_DRIVER_HANDLED;
}


/*----------------------------------------------------------------------
Function name:  Set_Busy

Description:

Return:         WORD
----------------------------------------------------------------------*/
WORD  Set_Busy(WORD * pFlags);


/*----------------------------------------------------------------------
Function name:  DdCreateSurface2

Description:    DDRAW callback CreateSurface

                This callback is invoked once the surface objects
                have been created.

				You can:
				  - compute the size of the block, by returning
				    DDHAL_PLEASEALLOC_BLOCKSIZE in fpVidMem, and
                    putting the size in dwBlockSizeX and
                    dwBlockSizeY
				  - override some fields in the surface structure,
                    like the pitch.  (you must specifiy the pitch
                    if you are computing the size)

				  If we return handled, then it is assumed that
                  we did SOMETHING with the surface structures to
                  indicate either what size of block or a new
                  pitch or some modification; or we are returning
                  an error.

                Note: Overlay surface count keeps track of how many overlays.
                When count goes to zero, disable overlay hardware.

                PROMOTION/DEMOTION:

                - all promotions happen in DdCreateSurface.
                - all demotions happen in DdDestroySurface.
                - promote primary from desktop to overlay if:

                  1. 16bpp surface
                  2. in exclusive mode
				  3. not overlay surface
                  4. 3D surface
				  5. primary surface or back buffer

----------------------------------------------------------------------*/
DWORD __stdcall
DdCreateSurface2( LPDDHAL_CREATESURFACEDATA pcsd )
{
  LPDDRAWI_DDRAWSURFACE_LCL   psurf;
  LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl;
  DWORD                       ddReturnVal;
  int                         i;
  DWORD                       pWidth, bWidth, height, pixelByteDepth;
  DWORD                       dwCaps, dwCaps2, ddstype, ddstype2;
  FXSURFACEDATA               *surfaceData;
  FxU32                       byte_width;
  DWORD                       sHeight; 

  DD_ENTRY_SETUP(pcsd->lpDD);
  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "CreateSurface32" ));
  DUMP_SURFACEDESC(ppdev, DEBUG_DDGORY, pcsd->lpDDSurfaceDesc );
  #endif

  dwCaps  = pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps;
  dwCaps2 = 0;


#if ( DX >= 6 )
  if ( pcsd->lpDDSurfaceDesc->dwSize == sizeof(DDSURFACEDESC2) )
    dwCaps2 = ((DDSURFACEDESC2*)pcsd->lpDDSurfaceDesc)->ddsCaps.dwCaps2;
#endif
    
  // Texture surface
  if (dwCaps & DDSCAPS_TEXTURE)
  {
    DISPDBG((ppdev, DEBUG_DDDETAILS,"Surface Create - Texture"));
    ddReturnVal = txtrSurfaceCreate(ppdev, pcsd->lplpSList[0], &pcsd->ddRVal);
    DISPDBG((ppdev, DEBUG_DDDETAILS,"return from texture create = %d\n", ddReturnVal));
    return(ddReturnVal);
  }

#ifdef AGP_EXECUTE
  if (dwCaps & DDSCAPS_NONLOCALVIDMEM) 
  {
    // Only allow offscreen plain nonlocal, textures handled in d3d code

    if ( (_DD(agpMode) & EXECUTE_MODE_BLTS) && 
         !(dwCaps & DDSCAPS_OFFSCREENPLAIN) )
    {
      pcsd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
    }
  }
#endif

  //If overlay surface to be created and overlay h/w is already being used, fail it
  if ( (dwCaps & DDSCAPS_OVERLAY) && !(dwCaps & DDSCAPS_BACKBUFFER) && _DD(overlaySurfaceCnt) )
  {
      pcsd->ddRVal = DDERR_NOOVERLAYHW;
      return DDHAL_DRIVER_HANDLED;
  }


  // See into the future
  // The whole point of this loop is to determine if we are going to 
  // eventually promote into SLI/AA mode.  If we are then we want to allocate
  // the surfaces different.  For SLI the size of the surface is divided into
  // bandheight sections and distributed across the devices
#ifdef SLI
  if (_DS(ddExclusiveMode) && !(_FF(dd3DInOverlay) & D3D_USING_OVERLAY))
      {
      for (i=0; i<(int)pcsd->dwSCnt; i++)
         {
         psurf = pcsd->lplpSList[i];
         psurf_gbl = psurf->lpGbl;
         dwCaps = psurf->ddsCaps.dwCaps;

         // Note: This implies that we will over allocate our Zbuffer if it comes first
                   if ((dwCaps & DDSCAPS_3DDEVICE) &&
                       (!(dwCaps & DDSCAPS_OVERLAY)) &&
                    ((dwCaps & DDSCAPS_PRIMARYSURFACE) || (dwCaps & DDSCAPS_FLIP)))
            {
            if (FALSE == _DD(dwComputeMode))
               {
               ComputeConfig(ppdev);
               _DD(dwComputeMode) = TRUE;
               if (SLI_MODE_ENABLED == _DD(sliMode))
                  {
                  if (MEMORY_MODE_DISABLED != _DD(sliMemoryMode))
                     {
                     DWORD tileInX;
                     DWORD tileInY;
                     DWORD screenSize;
                     DWORD FreeSpace;

                     // Return the Extra Memory at the End of the Primary Surface
                     // This kinda of sucks cause we need to move anything that exists here on delete
                     height = ((psurf_gbl->wHeight + ((1 << _DD(dwLog2GroupHeight)) - 1)) >> _DD(dwLog2GroupHeight)) << _DD(dwLog2BandHeight);                     
                     tileInX = (psurf_gbl->wWidth * GETPRIMARYBYTEDEPTH + SST2_TILE_WIDTH - 1) >> SST2_TILE_WIDTH_BITS;
                     // should be a check for stagger mode here? mls
                     tileInX = RNDUP_T1_PGWIDTH(tileInX);
                     tileInY = (height + SST2_TILE1_HEIGHT - 1) >> SST2_TILE1_HEIGHT_BITS;
                     screenSize = (tileInX << SST2_TILE_WIDTH_BITS) * (tileInY << SST2_TILE1_HEIGHT_BITS);                     
                     
                     screenSize = RNDUP_16KB(screenSize);                                 
                     FreeSpace = _FF(mmReclamationHeapStart) - (_FF(gdiDesktopStart) + screenSize -1);
                     AddSLISpace(ppdev, FreeSpace);
                     }
                  }
               }
            }   
         }
      }
#endif

  for( i=0;i<(int)pcsd->dwSCnt;i++ )
  {
    psurf = pcsd->lplpSList[i];
    psurf_gbl = psurf->lpGbl;
    dwCaps = psurf->ddsCaps.dwCaps;

    if ( (dwCaps & DDSCAPS_3DDEVICE|DDSCAPS_ZBUFFER) &&
         (psurf->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_HINTANTIALIASING) )
    {
        dwCaps2 |= DDSCAPS2_HINTANTIALIASING;
    }

    // handle fullscreen AA mode selection
    // either comes from registry or from DX7 hint flag
    if ( _DS(ddExclusiveMode) && ( dwCaps & DDSCAPS_PRIMARYSURFACE ) ) 
    {
        //make sure windowed AA mode is off
        _DD(ddWindowedAAMode) = WNAA_MODE_NONE; 
        // Only set fs aa flags during primary surface creation
        Set_FSAAMode( pcsd, dwCaps2, dwCaps, psurf_gbl );
        if( _DD(ddFSAAMode) == FSAA_MODE_NONE )  
        {
            // if we're not in AA ( mode 0 ) make sure the hint flag is not on
            // for primary surface
            dwCaps2 &= ~DDSCAPS2_HINTANTIALIASING;
        }
    } 
    else if( !_DS(ddExclusiveMode) ) 
    {
        // we can't set a FSAA mode if we aren't in Fullscreen ...
        _DD(ddFSAAMode)     = FSAA_MODE_NONE; // (=0)

        if( !(dwCaps & DDSCAPS_ZBUFFER) ) // check this on primary and backbuffer creation
        {
            Set_WindowedAAMode( pcsd, dwCaps, dwCaps2 );
        }
        
    
        // If this is the primary surface in windowed mode ( desktop ) it will not be flagged as an AA surface.
        // ( You can blit from an AA surface to the non-AA desktop )
        if( _DD(ddWindowedAAMode) && !(dwCaps & DDSCAPS_PRIMARYSURFACE) )
        {
            dwCaps2 |= DDSCAPS2_HINTANTIALIASING;
        }
        else
        {
            dwCaps2 &= ~DDSCAPS2_HINTANTIALIASING;
        }
        
    }


    if(!(dwCaps & DDSCAPS_PRIMARYSURFACE))
    {
      surfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));
      if( _DS(ddExclusiveMode) && ( _DD(ddFSAAMode) == FSAA_MODE_NONE ) )
      {
         // if we're not in AA (mode 0) make sure the hint flag is not on
         // for secondary surfaces
         dwCaps2 &= ~DDSCAPS2_HINTANTIALIASING;
      }
    }
    else
    {
      surfaceData = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));
    }

    if(!surfaceData)
    {
      pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
      return DDHAL_DRIVER_HANDLED;
    }

    // save info from the ddraw surface into our local surface descriptor
    surfaceData->dwFlags         |= FXSURFACE_IS_EXCLUSIVE;
    surfaceData->ddsDwCaps        = dwCaps;
    surfaceData->ddsDwCaps2       = dwCaps2;
    surfaceData->dwSurfLclFlags   = pcsd->lplpSList[0]->dwFlags;
    surfaceData->pTxtrDesc        = 0;
    surfaceData->dwStencilBitMask = 0; // initialize to zero, might change later
    surfaceData->dwZBitMask       = 0;
    surfaceData->dwWidth          = psurf_gbl->wWidth;
    surfaceData->dwHeight         = psurf_gbl->wHeight;
    surfaceData->dwColorSpaceLowValue = psurf->ddckCKSrcBlt.dwColorSpaceLowValue;
    surfaceData->dwColorSpaceHighValue = psurf->ddckCKSrcBlt.dwColorSpaceHighValue;

    if( _DD(ddFSAAMode) && (dwCaps & DDSCAPS_ZBUFFER) )
    {
        //make sure the aa caps flag is set for the clear routine.
        surfaceData->ddsDwCaps2 |= DDSCAPS2_HINTANTIALIASING;
    }
    else
    {
        switch( _DD(ddFSAAMode) )
        {
            case  FSAA_MODE_4XFLIP:
                // double stride for primary surface in AA flip mode
                surfaceData->dwPStride   = surfaceData->dwPStride*2; 
                surfaceData->dwStride    = surfaceData->dwStride*2;
                surfaceData->dwL2MStride = surfaceData->dwL2MStride+1;
                surfaceData->dwMStride   = surfaceData->dwMStride*2;
                // make sure the aa caps flag is set for the clear routine.
                surfaceData->ddsDwCaps2 |= DDSCAPS2_HINTANTIALIASING;
                // set correct width and height in local data
                surfaceData->dwWidth     = psurf_gbl->wWidth*2;
                surfaceData->dwHeight    = psurf_gbl->wHeight*2;
                break;
            case FSAA_MODE_4XBLT:
                if( (DWORD)surfaceData == _DD(ddAASecondaryBuffer1SurfaceData)) 
                {
                    //if app is "recreating" the backbuffer with the same addresses, 
                    // we need to reset the Buffer1Start value ( MS flight sim 2000 does this) 
                    _DD(ddAASecondaryBuffer1Start) = 0;   
                }

                // for blt mode  double surface stride only if this is back buffer 1
                if ( !(dwCaps & DDSCAPS_PRIMARYSURFACE) && (_DD(ddAASecondaryBuffer1Start) == (DWORD) NULL ) )   
                {
                    surfaceData->dwPStride   = surfaceData->dwPStride*2; 
                    //make sure the aa caps flag is set for the clear routine.
                    surfaceData->ddsDwCaps2 |= DDSCAPS2_HINTANTIALIASING;
                    // set correct width and height in local data
                    surfaceData->dwWidth     = psurf_gbl->wWidth*2;
                    surfaceData->dwHeight    = psurf_gbl->wHeight*2;
                }
                break;
            default:
                break;
        }   // end switch
    } // endif

    if ( !(dwCaps & DDSCAPS_PRIMARYSURFACE ) && _DD(ddWindowedAAMode) )
    {
        // double local width and height of offscreen surface in windowed AA mode
        surfaceData->dwWidth     = psurf_gbl->wWidth*2;
        surfaceData->dwHeight    = psurf_gbl->wHeight*2;
    }


    if(dwCaps & DDSCAPS_FLIP)
    {
      // Initialize surface flipping variables.
      _DD(ddSurfaceFlippedFrom) = 0;
      _DD(ddSurfaceFlippedTo)   = 0;
      _DD(ddAcceleratorUsed)    = 0;
      surfaceData->inFlipChain = TRUE;
#ifdef SLI
      surfaceData->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
      surfaceData->dwFlags |= (SLI_MODE_ENABLED == _DD(sliMode)) ? FXSURFACE_IS_DISTRIBUTED : FXSURFACE_IS_EXCLUSIVE;
#endif
    }
    else
    {
      surfaceData->inFlipChain = FALSE;
    }
    surfaceData->surfaceLevel = _FF(ddCurrentSurfaceLevel);

    psurf_gbl->dwReserved1 = (DWORD)surfaceData;

    #ifdef FXTRACE
    DUMP_DDRAWSURFACE_LCL(ppdev, DEBUG_DDGORY, psurf );
    #endif

    if( dwCaps == DDSCAPS_SYSTEMMEMORY )
    {
      pcsd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
    }

    pWidth = psurf_gbl->dwBlockSizeX = psurf_gbl->wWidth;
    height = psurf_gbl->dwBlockSizeY = psurf_gbl->wHeight;

    switch( _DD(ddFSAAMode)  )
    {   // primary surface is enlarged for aa flip mode
        case  FSAA_MODE_4XFLIP:
            if ( dwCaps & DDSCAPS_PRIMARYSURFACE )  
            {
                // only change local sizes - not global info
                pWidth *= 2; 
                height *= 2;
            }
            break;
        default:
            break;
    }


    if (dwCaps & DDSCAPS_ZBUFFER)
    {
#ifdef SLI
        surfaceData->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
        surfaceData->dwFlags |= FXSURFACE_IS_EXCLUSIVE;
#endif
        pixelByteDepth = GETPRIMARYBYTEDEPTH;                     // Primary and Zbuffer depth are equal
        if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT)
        {
            pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwZBufferBitDepth)>>3;
        }

        // When 24bpp zbuffer is requested, 32bpp zbuffer must be allocated.
        if (pixelByteDepth == 3) pixelByteDepth = 4;

        if ((pixelByteDepth != 2) && (pixelByteDepth != 4)) // 16bpp/32bpp zbuffer (Napalm)
        {
            pcsd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_HANDLED;
        }

        // Very dangerous code to change the zbuffer depth out from under the application,
        // since Napalm cannot handle a different depth zbuffer and primary! Borrowed from Napalm & CGW
        if ((pixelByteDepth == 2) && (GETPRIMARYBYTEDEPTH == 4)) // 32bpp rendering, 16bpp zbuffer requested
        {
            pixelByteDepth = 4; // Force 32bpp allocation
        }

        if ((pixelByteDepth == 4) && (GETPRIMARYBYTEDEPTH == 2)) // 16bpp rendering, 32bpp zbuffer requested
        {
            pixelByteDepth = 2; // Force 16bpp allocation
        }

        // Very dangerous code to ensure that the pixel format of the zbuffer is correct, since we
        // may have changed the zbuffer depth in the code above! Borrowed from Napalm & CGW
        if ((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT)
        {
            if (pixelByteDepth == 2)
            {
                psurf_gbl->ddpfSurface.dwZBufferBitDepth = 16; // 16bpp zbuffer
                psurf_gbl->ddpfSurface.dwZBitMask = 0x0000FFFF;
            }
            else if (pixelByteDepth == 4)
            {
                psurf_gbl->ddpfSurface.dwZBufferBitDepth = 32; // 32bpp zbuffer
                psurf_gbl->ddpfSurface.dwZBitMask = 0x00FFFFFF;
            }
        }
        surfaceData->dwStencilBitMask = psurf_gbl->ddpfSurface.dwStencilBitMask;
        surfaceData->dwZBitMask = psurf_gbl->ddpfSurface.dwZBitMask;
    } // if zbuffer
    else

    if ((dwCaps & (DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE)) &&
        !(dwCaps & DDSCAPS_OVERLAY))
    {
      if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
      {
        if(psurf_gbl->ddpfSurface.dwFlags & DDPF_RGB)
        {
          surfaceData->dwPixelFormat = DDPF_RGB;
          pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
        }   // RGB
        else
        {
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return DDHAL_DRIVER_HANDLED;
        }
      } // HASPIXELFORMAT
      else
      {
        surfaceData->dwPixelFormat = DDPF_RGB;
        pixelByteDepth = GETPRIMARYBYTEDEPTH;
      } // !DDRAWISURF_HASPIXELFORMAT
    }
    else  if(dwCaps & DDSCAPS_OVERLAY )
    {

       pcsd->ddRVal =  CreateOverlaySurface( ppdev, psurf, dwCaps,
        pWidth, height,&pixelByteDepth,i);

       if( pcsd->ddRVal != DD_OK )
           return DDHAL_DRIVER_HANDLED;

    }
    else  if(dwCaps & DDSCAPS_VIDEOPORT)
    {
       //video port only support UYVY,YUYV and RAW8
        if(!(psurf->dwFlags & DDRAWISURF_HASPIXELFORMAT ) 
           || !(psurf_gbl->ddpfSurface.dwFlags & DDPF_FOURCC)
           || ((psurf_gbl->ddpfSurface.dwFourCC != FOURCC_UYVY) 
               && (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YUY2) 
               && (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_RAW8) ))

        {
           pcsd->ddRVal = DDERR_INVALIDPIXELFORMAT;
           return DDHAL_DRIVER_HANDLED;
       
        }


        if((psurf->dwFlags & DDRAWISURF_HASPIXELFORMAT ) 
           &&(psurf_gbl->ddpfSurface.dwFlags & DDPF_FOURCC)
           && (psurf_gbl->ddpfSurface.dwFourCC == FOURCC_RAW8))
             pixelByteDepth = 1;
         else
             pixelByteDepth = 2;

    }
    else if(dwCaps & DDSCAPS_VIDEOMEMORY
        || dwCaps & DDSCAPS_OFFSCREENPLAIN)
    {
      if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
      {
        if(psurf_gbl->ddpfSurface.dwFlags & DDPF_FOURCC)
        {
            if( (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YUY2)
                && (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_UYVY))
            {
                pcsd->ddRVal = DDERR_UNSUPPORTED;
                return DDHAL_DRIVER_HANDLED;
            }
            surfaceData->dwPixelFormat = psurf_gbl->ddpfSurface.dwFourCC;
            pixelByteDepth = 2;
            psurf_gbl->ddpfSurface.dwYUVBitCount = 16;
         
        }   // FOURCC
        else if(psurf_gbl->ddpfSurface.dwFlags & DDPF_RGB)
        {
          surfaceData->dwPixelFormat = DDPF_RGB;
          pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
        }   // RGB
        else
        {
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return DDHAL_DRIVER_HANDLED;
        }
      } // HASPIXELFORMAT
      else
      {
        surfaceData->dwPixelFormat = DDPF_RGB;
        pixelByteDepth = GETPRIMARYBYTEDEPTH;
      } // !DDRAWISURF_HASPIXELFORMAT

    }
    else
    {
      pcsd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
    }

    if ( !(dwCaps & DDSCAPS_OVERLAY) && (pWidth == (DWORD)_FF(bi).biWidth) && (height == (DWORD)_FF(bi).biHeight))
      bWidth = _DS(ddPrimarySurfaceData.dwStride);
    else
      bWidth = pWidth * pixelByteDepth;

    if (psurf->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
      psurf_gbl->fpVidMem = surfaceData->lfbPtr;

       switch( _DD(ddFSAAMode)  )
      {
          case  FSAA_MODE_4XBLT:
              // double global pitch of primary for AA blt modes 
              // (even though primary is normal size )
              psurf_gbl->lPitch   = surfaceData->dwMStride*2; 
              break;
          default:
              psurf_gbl->lPitch   = surfaceData->dwMStride;
              break;
      }


      // CAM entry already setup for Primary by display driver - no "phantom" space needed
      surfaceData->phantom_blksize = 0;
      surfaceData->phantomlfbPtr =   0;
#ifdef SLI
      surfaceData->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
      surfaceData->dwFlags |= (SLI_MODE_ENABLED == _DD(sliMode)) ? FXSURFACE_IS_DISTRIBUTED : FXSURFACE_IS_EXCLUSIVE;
#endif
    }
    else
    {
#ifdef AGP_EXECUTE
        if ( dwCaps & DDSCAPS_NONLOCALVIDMEM )
        {
            ddstype  = dwCaps;
            ddstype2 = dwCaps2;
            surfaceData->tileFlag = MEM_IN_LINEAR; // Use linear for other surface types
            bWidth = (bWidth+31) & ~31;          // Make it multiple of 32bytes
            surfaceData->dwPStride   = bWidth;
            surfaceData->dwStride = bWidth;
            surfaceData->dwL2MStride = 0;

            // AGP has no CAM so report linear pitch
            surfaceData->dwMStride   = bWidth;   // needs 1 for tiled
        }
        else
#endif 
        //If standard surface types then set the ddstype with dwCaps
        if ( (dwCaps & DDSCAPS_3DDEVICE) || (dwCaps & DDSCAPS_ZBUFFER) ||
             (dwCaps & (DDSCAPS_OVERLAY|DDSCAPS_VIDEOPORT)) )
        {
            ddstype  = dwCaps;
            ddstype2 = dwCaps2;
            surfaceData->tileFlag = 0;
#ifdef SLI
           if (!(dwCaps & DDSCAPS_ZBUFFER))
               {
               surfaceData->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
               surfaceData->dwFlags |= (SLI_MODE_ENABLED == _DD(sliMode)) ? FXSURFACE_IS_DISTRIBUTED : FXSURFACE_IS_EXCLUSIVE;
               }
#endif
            if(dwCaps & DDSCAPS_OVERLAY)
            {
                 if(psurf_gbl->ddpfSurface.dwFourCC == FOURCC_YV12)
                 {
                 //adjust width and height
                    bWidth >>= 1;
                    //surface width must be power of 2 to match
                    //MSstride, otherwise V and U data will have
                    //wrong half picth to copy to

                    surfaceData->dwMStride   = 1;
                
                    while ( surfaceData->dwMStride < bWidth )
                    {
                        surfaceData->dwMStride <<= 1;
                    }
                    bWidth = surfaceData->dwMStride;
                    height = (height +1 ) * 3 /2;
                    pixelByteDepth =1;
                 }
                 else if(_DD(dwOVLFlags) & (PRE_DXVA| DXVA_ON))
                 {

                    surfaceData->tileFlag = MEM_IN_TILE1 | MEM_AT_16K;
                    ddstype  = 0;

                 }

            }
            bWidth = (bWidth+31) & ~31;  // Make it multiple of 32bytes
        }
        else //If non standard surface type 
        {
            // Set ddstype to 0 and determine the surface tile mode and alignment type
            ddstype = ddstype2 = 0;

            if ( psurf->dwFlags & DDRAWISURF_PARTOFPRIMARYCHAIN )
            {
                // Set surface's tile flag to primary's if part of primary chain
                surfaceData->tileFlag    = _FF(ddPrimarySurfaceData).tileFlag;
                surfaceData->dwPStride   = _FF(ddPrimarySurfaceData).dwPStride;
                surfaceData->dwStride = _FF(ddPrimarySurfaceData).dwStride;
                surfaceData->dwL2MStride = _FF(ddPrimarySurfaceData).dwL2MStride;
                surfaceData->dwMStride   = _FF(ddPrimarySurfaceData).dwMStride;
#ifdef SLI
                surfaceData->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
                surfaceData->dwFlags |= (SLI_MODE_ENABLED == _DD(sliMode)) ? FXSURFACE_IS_DISTRIBUTED : FXSURFACE_IS_EXCLUSIVE;

#endif

                //Made surface width in bytes (bWidth) equals its physical stride (PStride)
                bWidth = surfaceData->dwStride;
            }
            else
            {
                surfaceData->tileFlag = MEM_IN_LINEAR; // Use linear for other surface types
                bWidth = (bWidth+31) & ~31;          // Make it multiple of 32bytes
                surfaceData->dwPStride   = bWidth;
                surfaceData->dwStride = bWidth;
                surfaceData->dwL2MStride = 0;
                surfaceData->dwMStride   = 1;
                
                while ( surfaceData->dwMStride < surfaceData->dwPStride )
                {
                    surfaceData->dwMStride <<= 1;
                    surfaceData->dwL2MStride++;
                }
            }
        }
        
        // Note this is where we Crunch the height
        //
#ifdef SLI
        if (FXSURFACE_IS_DISTRIBUTED == (surfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED))
           sHeight = ((height + ((1 << _DD(dwLog2GroupHeight)) - 1)) >> _DD(dwLog2GroupHeight)) << _DD(dwLog2BandHeight);            
         
        else
#endif
            sHeight = height;            

        pcsd->ddRVal = surfMgr_allocSurface(
                psurf_gbl->lpDD,          // direct draw vidmemalloc need this
                ddstype,                  // type of surface (can use the standard DD surface flags) [IN]
                ddstype2,                 // type of surface extension [IN]
                bWidth,                   // width in bytes (linear space) [IN]
                sHeight,                   // height in pixel (linear space) [IN]
                0,                        // linear address mask
                &(surfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
                &(surfaceData->endlfbPtr),// host lfb end address of allocation [OUT]
                &(surfaceData->hwPtr),    // hw vidmem address
                &(surfaceData->dwStride), // pitch [OUT]
                &(surfaceData->tileFlag)  // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                );

        if ( pcsd->ddRVal != DD_OK )
        {
            pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
            return DDHAL_DRIVER_HANDLED;
        }

        // if we successfully allocated a secondary buffer - 
        // and we are in fullscreen aa mode, save the buffer address
        if( _DD(ddFSAAMode) )
        {
            if (dwCaps & DDSCAPS_ZBUFFER)
            {
                _DD(ddAAZBufferStart) = surfaceData->lfbPtr;
            }
            else if( _DD(ddAASecondaryBuffer1Start) == (DWORD) NULL )
            {
                _DD(ddAASecondaryBuffer1Start) = surfaceData->lfbPtr;
                _DD(ddAASecondaryBuffer1HW)    = surfaceData->hwPtr;
                // save this pointer for AA mode 3 use
                _DD(ddAASecondaryBuffer1SurfaceData) = (DWORD)surfaceData;
               }
            else
            {            
                _DD(ddAASecondaryBuffer2Start) = surfaceData->lfbPtr;
                _DD(ddAASecondaryBuffer2HW)    = surfaceData->hwPtr;
            }
        }





#ifdef AGP_EXECUTE
        if ( dwCaps & DDSCAPS_NONLOCALVIDMEM )
        {
          // Set fpPhysicalVidMem which is not visible surface manager
          #define DWORD_PTR   DWORD    // ddrawi.h issue for DX7, not in W9X DX DDK

          DDRAWI_DDRAWSURFACE_GBL_MORE  *psurf_gbl_more;

          psurf_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(psurf_gbl);
          psurf_gbl_more->fpPhysicalVidMem = surfaceData->hwPtr;
        }
        else
#endif 

        //For standard surface types, we'll need to know allocated pitch to determine PStride and MStride
        if ( (dwCaps & DDSCAPS_3DDEVICE) || (dwCaps & DDSCAPS_ZBUFFER) ||
             (dwCaps & (DDSCAPS_OVERLAY|DDSCAPS_VIDEOPORT)) )
        {
            //fist adjust overlay start address to make sure
            //the overlay right edge is at 32 byte boundary -- fix HW bug
            if( (dwCaps & DDSCAPS_OVERLAY) && !(dwCaps & DDSCAPS_VIDEOPORT)
                &&(psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YV12))
            {
               // save the original data first
               DWORD dwExtra = (pWidth * pixelByteDepth) & 31;

               if(dwExtra)
                dwExtra = 32 - dwExtra;
               surfaceData->dwOvlHOffset = dwExtra / pixelByteDepth;
               surfaceData->dwOvlExtraOffset = dwExtra;

            }

            if ( surfaceData->tileFlag & MEM_IN_LINEAR )
            {
               surfaceData->dwPStride = surfaceData->dwStride; //Better be multiple of 32bytes
            }
            else
            {
               surfaceData->dwPStride = surfaceData->dwStride >> SST2_TILE_WIDTH_BITS; //PStride in tiles
            }   
            surfaceData->dwL2MStride = 0;
            surfaceData->dwMStride   = 1;
            while ( surfaceData->dwMStride < surfaceData->dwStride )
            {
               surfaceData->dwMStride <<= 1;
               surfaceData->dwL2MStride++;
            }
        }
        
        psurf_gbl->fpVidMem = surfaceData->lfbPtr;
        psurf_gbl->lPitch = surfaceData->dwMStride;   //Report MStride power of two pitch to application!

        // allocate host(linear) address for eventual lock

        // if pitch is power of 2, alloc with current size
        // if pitch not a power of 2, alloc additional to allow for mstride rounded up 
        surfaceData->phantom_blksize = surfaceData->endlfbPtr-surfaceData->lfbPtr; // real size is end-start
        // Regrow blksize to actual size
#ifdef SLI
        if (surfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
            {
            surfaceData->phantom_blksize *= _FF(dwNumSlaves + 1);
            }
#endif

        if (surfaceData->tileFlag & (MEM_IN_TILE0|MEM_IN_TILE1))
        {
            //PStride is in tile units - convert to bytes
            byte_width = surfaceData->dwPStride * SST2_TILE_WIDTH;
        }
        else
        {
            byte_width = surfaceData->dwPStride;
        }

        if (surfaceData->dwMStride > byte_width)
        {
            // pitch was rounded up - bump size accordingly
            FxU32 height = surfaceData->phantom_blksize/byte_width;// num rows
            surfaceData->phantom_blksize += height*(surfaceData->dwMStride - byte_width);// add height*extra row length
        }

        // memmgr aligns start on 32B so align size so end also 32B aligned
        surfaceData->phantom_blksize = (surfaceData->phantom_blksize+0x1f) & ~(0x1f);

#ifdef AGP_EXECUTE
        if ( !( dwCaps & DDSCAPS_NONLOCALVIDMEM ))
#endif 
        {
          phantom_allocSurface(GET_DEVICE_DATA(psurf_gbl->lpDD), surfaceData->phantom_blksize,(DWORD *)&surfaceData->phantomlfbPtr);

          if (!surfaceData->phantomlfbPtr)
          {
            // can't find suitable phantom address
            pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
            return DDHAL_DRIVER_HANDLED;
          }
        }
    }

    // save for CAM later
    if (3 == pixelByteDepth)
      pixelByteDepth = 1;
    surfaceData->dwBytesPerPixel = pixelByteDepth;

    // surfaceData->tileFlag,surfaceData->lPitch will also be used

    // Bug: Single Buffer Fullscreen promotes to overlay but runtime
    // doesn't call Destroy surface so dd3DInOverlay=1 and subsequant
    // promtion never happens.
    // Possible Fix: In SetExclusive Mode call demote

    if ( _DS(ddExclusiveMode) &&                       //Exclusive mode only
         (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY)) &&//Overlay hw already used by 3D app or other video app?
		   (psurf->ddsCaps.dwCaps & DDSCAPS_3DDEVICE) &&
		   (!(psurf->ddsCaps.dwCaps & DDSCAPS_OVERLAY)) &&
		   ((psurf->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) || (psurf->ddsCaps.dwCaps & DDSCAPS_FLIP))
       )
    {
      if ( _DD(ddFSAAMode) )   
      {      
        // start this counter now
        _DD(ddAAFlipCount) = 0; 
      }
      PROMOTE_PRIMARYTOOVERLAY(ppdev, pWidth, height , surfaceData);
    }        


  }   // Surface Creation Loop

   // If we are in Low Power mode then we want to
   // not spin on the hardware
   #define BUSY 0x10
   if (BUSY != (*_FF(lpDeFlags) & BUSY))
   	FXBUSYWAIT(ppdev);
  pcsd->ddRVal = DD_OK;

  return DDHAL_DRIVER_HANDLED;

}// DdCreateSurface2


/*----------------------------------------------------------------------
Function name:  DdCreateSurface

Description:    DDRAW callback CreateSurface

                This callback is invoked once the surface objects
                have been created.

				You can:
				  - compute the size of the block, by returning
				    DDHAL_PLEASEALLOC_BLOCKSIZE in fpVidMem, and
                    putting the size in dwBlockSizeX and
                    dwBlockSizeY
				  - override some fields in the surface structure,
                    like the pitch.  (you must specifiy the pitch
                    if you are computing the size)

				  If we return handled, then it is assumed that
                  we did SOMETHING with the surface structures to
                  indicate either what size of block or a new
                  pitch or some modification; or we are returning
                  an error.

                Note: Overlay surface count keeps track of how many overlays.
                When count goes to zero, disable overlay hardware.

                PROMOTION/DEMOTION:

                - all promotions happen in DdCreateSurface.
                - all demotions happen in DdDestroySurface.
                - promote primary from desktop to overlay if:

                  1. 16bpp surface
                  2. in exclusive mode
				  3. not overlay surface
                  4. 3D surface
				  5. primary surface or back buffer

----------------------------------------------------------------------*/
DWORD __stdcall
DdCreateSurface( LPDDHAL_CREATESURFACEDATA pcsd )
{
  int i;
  DWORD dwCaps;
  DWORD ddReturnVal;
  DD_ENTRY_SETUP(pcsd->lpDD);

  dwCaps  = pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps;

  if (dwCaps & DDSCAPS_VIDEOMEMORY)
  {
    if ((dwCaps & (DDSCAPS_TEXTURE | DDSCAPS_OFFSCREENPLAIN)) &&
       !(dwCaps & (DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM)))
    {
      //RYAN@MEM, Oops, the caller wants video memory, but didn't specify what
      //flavor.  So we need to try all flavors, in the order of preference.

      //modify given parameters to specify LOCAL video memory
      pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps = (dwCaps | DDSCAPS_LOCALVIDMEM);
      for(i=0;i<(int)pcsd->dwSCnt;i++) 
        pcsd->lplpSList[i]->ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM;

      ddReturnVal = DdCreateSurface2(pcsd);  //restart with the modified parameters

#ifdef AGP_EXECUTE
      if(pcsd->ddRVal != DD_OK)
      {
        //LOCAL didn't work, so modify given parameters to specify NONLOCAL video memory
        pcsd->ddRVal = DD_OK;
        pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps = (dwCaps | DDSCAPS_NONLOCALVIDMEM);
        for(i=0;i<(int)pcsd->dwSCnt;i++) 
          pcsd->lplpSList[i]->ddsCaps.dwCaps ^= (DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM);

        ddReturnVal = DdCreateSurface2(pcsd);  //try again with the new parameters
      }
#endif
      
      //undo our parameter modifications, if all attempts failed.
      if (pcsd->ddRVal != DD_OK)
      {
        pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps = dwCaps;
        for(i=0;i<(int)pcsd->dwSCnt;i++) 
          pcsd->lplpSList[i]->ddsCaps.dwCaps &= ~(DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM);
      }
      return ddReturnVal;
    }
  }

  return DdCreateSurface2(pcsd);
}





/*----------------------------------------------------------------------
Function name:  DdDestroySurface

Description:    DDRAW callback DestroySurface

                This callback destroys a specified surface.

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED	-
				DDHAL_DRIVER_NOTHANDLED -
----------------------------------------------------------------------*/
DWORD __stdcall
DdDestroySurface( LPDDHAL_DESTROYSURFACEDATA pdsd )
{
  DWORD                dwCaps;
  LPDDRAWI_DDRAWSURFACE_LCL psurf;
  LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl;
  FXSURFACEDATA        *surfaceData;
  DWORD hwPtr;

  DD_ENTRY_SETUP(pdsd->lpDD);

  psurf = pdsd->lpDDSurface;
  psurf_gbl = psurf->lpGbl;
  dwCaps = psurf->ddsCaps.dwCaps;

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "DestroySurface32" ));
  DUMP_DDHAL_DESTROYSURFACEDATA(ppdev, DEBUG_DDGORY, pdsd );
  #endif

  if ( dwCaps & DDSCAPS_TEXTURE )
  {
    txtrSurfaceDelete(ppdev, pdsd->lpDDSurface);
    pdsd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
  }

  if ( dwCaps == DDSCAPS_SYSTEMMEMORY)
  {
    pdsd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_HANDLED;
  }

  // Demote when primary is using overlay and destroyed
  
  if (psurf->dwFlags & DDRAWISURF_PARTOFPRIMARYCHAIN) // DDSCAPS_PRIMARYSURFACE is not always set!
  {
    if(_FF(dd3DInOverlay) & D3D_USING_OVERLAY)
    {
#ifdef SLI
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         RemoveSLISpace(ppdev, _DD(dwSLISize));
         }
#endif
      DEMOTE_3DTONONEOVERLAY(ppdev);
    }

    _FF(ddVisibleOverlaySurf) = 0;
  }


  if ( dwCaps & DDSCAPS_OVERLAY )
  {
     DestroyOverlaySurface(pdsd);
  }

  // free buffer if it is not the primary surface
  if(psurf_gbl->fpVidMem != GETPRIMARY )
  {
    surfaceData = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
    if(surfaceData != (FXSURFACEDATA*) NULL)
    {
      hwPtr = surfaceData->hwPtr;

      if(hwPtr != 0)
      {
#ifdef AGP_EXECUTE
        if( dwCaps & DDSCAPS_NONLOCALVIDMEM )
        {
           DDRAWI_DDRAWSURFACE_GBL_MORE *psurf_gbl_more;

           // Cleanup what surface manger can't get to, skip
           // CAM and phantom address cleanup
           psurf_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(psurf_gbl);
           psurf_gbl_more->fpPhysicalVidMem = 0L;
        }
        else
#endif
        {
          // since we're freeing physical memory, free phantom representation as well
          phantom_freeSurface(GET_DEVICE_DATA(pdsd->lpDD), surfaceData->phantomlfbPtr);
          // Free associated CAM entry - if surface locked (following is NOP if surface not locked)
          FxCamFreeEntry(_DD(camMgr), ppdev, surfaceData->phantomlfbPtr-_FF(LFBBASE));
        }

        if( ( _D3( zBufferHwPtr ) != 0 ) && ( dwCaps & DDSCAPS_ZBUFFER ) )
        {
          surfMgr_freeSurface(
            psurf_gbl->lpDD,          // direct draw vidmemalloc need this
            surfaceData->lfbPtr,      // host lfb start address of allocation [OUT]
            _D3( zBufferHwPtr ),      // hw start address of allocation [OUT]
            dwCaps                    // flags local/nonlocal type
            );

            _D3( zBufferHwPtr ) = 0;
        }
        else
        {
          {
            surfMgr_freeSurface(
              psurf_gbl->lpDD,          // direct draw vidmemalloc need this
              surfaceData->lfbPtr,      // host lfb start address of allocation [OUT]
              surfaceData->hwPtr,       // hw start address of allocation [OUT]
              dwCaps                    // flags local/nonlocal type
              );
           }
        }
      }

      DXFREE((void*)surfaceData);
    }
  }
  else
  {
    surfaceData = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
    surfaceData->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
    surfaceData->dwFlags |= FXSURFACE_IS_EXCLUSIVE;
    // in case desktop is still pointing at the back buffer
    DISPDBG((ppdev, 0, "***Destroy Surface for Primary!!!!" ));

    pdsd->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
  }

  psurf_gbl->dwReserved1 = 0;
  psurf_gbl->fpVidMem = 0;

  // Csim Server stuff
  // Set Render_Enable to 1 -- causes GuiUpdateRect to happen for every prim instead of just on flip
  {
        DWORD dwCount;
        DWORD dwDevNode;

        {
            SST2INTERFACE iSST2;
            SST2HOSTINFO hostInfo;
            // SST2DISPLAYMASK mask;
     
            if(SST2_Get_Interface(&iSST2, (DWORD) SST2_VXD_NAME) != SST2RET_OK)
                goto SST2ConnectFail;

            dwCount = 1;

            if(SST2_Get_Device_Devnodes(iSST2, &dwCount, &dwDevNode) != SST2RET_OK)
            {
                SST2_Release_Interface(iSST2);
                goto SST2ConnectFail;
            }

            if(dwCount != 1)
            {
                SST2_Release_Interface(iSST2);
                goto SST2ConnectFail;
            }

            if ( SST2_Get_Device_Host(iSST2, dwDevNode, &hostInfo) != SST2RET_OK)
            {
                SST2_Release_Interface(iSST2);
                goto SST2ConnectFail;
            }

            hostInfo.Flags.Render_Enabled = 1;
         
            if(SST2_Set_Device_Host(iSST2, dwDevNode, &hostInfo) != SST2RET_OK)
            {
                SST2_Release_Interface(iSST2);
                goto SST2ConnectFail;
            }
         
            SST2_Release_Interface(iSST2);

            goto ConnectDone;
        }
SST2ConnectFail: ;

        //-------------------------------------------------------------
        // Now try R3 interface
        //-------------------------------------------------------------
        {
            R3INTERFACE iR3;
            R3HOSTINFO hostInfo;
         
            if(R3_Get_Interface(&iR3, (DWORD) R3_VXD_NAME) != R3RET_OK)
                goto R3ConnectFail;

            dwCount = 1;

            if(R3_Get_Device_Devnodes(iR3, &dwCount, &dwDevNode) != R3RET_OK)
            {
                R3_Release_Interface(iR3);
                goto R3ConnectFail;
            }

            if(dwCount != 1)
            {
                R3_Release_Interface(iR3);
                goto R3ConnectFail;
            }

            if ( R3_Get_Device_Host(iR3, dwDevNode, &hostInfo) != R3RET_OK)
            {
                R3_Release_Interface(iR3);
                goto R3ConnectFail;
            }

            hostInfo.Flags.Render_Enabled = 1;
             
            if(R3_Set_Device_Host(iR3, dwDevNode, &hostInfo) != R3RET_OK)
            {
                R3_Release_Interface(iR3);
                goto R3ConnectFail;
            }
             
            R3_Release_Interface(iR3);

            goto ConnectDone;
        }
R3ConnectFail: ;

        //-------------------------------------------------------------
        // Now try Hydra interface
        //-------------------------------------------------------------
        {
            HYDRAINTERFACE iHydra;
            HYDRAHOSTINFO hostInfo;
         
            if(Hydra_Get_Interface(&iHydra, (DWORD) HYDRA_VXD_NAME) != HYDRARET_OK)
                goto HydraConnectFail;

            dwCount = 1;

            if(Hydra_Get_Device_Devnodes(iHydra, &dwCount, &dwDevNode) != HYDRARET_OK)
            {
                Hydra_Release_Interface(iHydra);
                goto HydraConnectFail;
            }

            if(dwCount != 1)
            {
                Hydra_Release_Interface(iHydra);
                goto HydraConnectFail;
            }

            if ( Hydra_Get_Device_Host(iHydra, dwDevNode, &hostInfo) != HYDRARET_OK)
            {
                Hydra_Release_Interface(iHydra);
                goto HydraConnectFail;
            }

            hostInfo.Flags.Render_Enabled = 1;
             
            if(Hydra_Set_Device_Host(iHydra, dwDevNode, &hostInfo) != HYDRARET_OK)
            {
                Hydra_Release_Interface(iHydra);
                goto HydraConnectFail;
            }
             
            Hydra_Release_Interface(iHydra);

            goto ConnectDone;
        }
HydraConnectFail: ;

        //-------------------------------------------------------------
        // Now try Rage interface
        //-------------------------------------------------------------
        {
            RAGEINTERFACE iRage;
            RAGEHOSTINFO hostInfo;
         
            if(Rage_Get_Interface(&iRage, (DWORD) RAGE_VXD_NAME) != RAGERET_OK)
                goto RageConnectFail;

            dwCount = 1;

            if(Rage_Get_Device_Devnodes(iRage, &dwCount, &dwDevNode) != RAGERET_OK)
            {
                Rage_Release_Interface(iRage);
                goto RageConnectFail;
            }

            if(dwCount != 1)
            {
                Rage_Release_Interface(iRage);
                goto RageConnectFail;
            }

            if ( Rage_Get_Device_Host(iRage, dwDevNode, &hostInfo) != RAGERET_OK)
            {
                Rage_Release_Interface(iRage);
                goto RageConnectFail;
            }

            hostInfo.Flags.Render_Enabled = 1;
             
            if(Rage_Set_Device_Host(iRage, dwDevNode, &hostInfo) != RAGERET_OK)
            {
                Rage_Release_Interface(iRage);
                goto RageConnectFail;
            }
             
            Rage_Release_Interface(iRage);

            goto ConnectDone;
        }
RageConnectFail: ;

ConnectDone: ;
    }

    pdsd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}// DdDestroySurface


/*----------------------------------------------------------------------
Function name:  DdLock

Description:    DDRAW callback Lock

                This callback is invoked whenever a surface is
                about to bedirectly accessed by the user.  This
                is where you need tomake sure that a surface can
                be safely accessed by the user.

				If your memory cannot be accessed while in
                accelerator mode, you should take either take
                the card out of accelerator mode or else return
                DDERR_SURFACEBUSY.

				If someone is accessing a surface that was just
                flipped away from, make sure that the old surface
                (what was the primary) has finished being displayed.

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED	-
				DDHAL_DRIVER_NOTHANDLED -
----------------------------------------------------------------------*/
DWORD __stdcall
DdLock( LPDDHAL_LOCKDATA pld )
{
  DWORD               dwCaps, ptr;
  DWORD               BytesPerPixel;
  FXSURFACEDATA       *surfaceData;
  DD_ENTRY_SETUP(pld->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "Lock32" ));
  DUMP_LOCKDATA(ppdev, DEBUG_DDGORY, pld );
  #endif

  dwCaps = pld->lpDDSurface->ddsCaps.dwCaps;
  if ( dwCaps & DDSCAPS_SYSTEMMEMORY)
  {
    pld->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
  }

  if (dwCaps & DDSCAPS_TEXTURE)
  {
    DISPDBG((ppdev,DEBUG_DDDETAILS,"DdLock - Texture"));

#ifdef AGP_CMDFIFO
    {
       void  FLUSHAGP(NT9XDEVICEDATA * ppdev);

       FLUSHAGP(ppdev);
    }
#endif // #ifdef AGP_CMDFIFO

#ifdef AGP_EXECUTE
    if ( dwCaps & DDSCAPS_NONLOCALVIDMEM )
    {
      // support for tiled surface unswizzling could be called
      // here, for now assume grat linear in fpVidMem is ok
      return DDHAL_DRIVER_HANDLED;
    }
    else
#endif
      txtrLock(ppdev, pld);
    return DDHAL_DRIVER_HANDLED;
  }


#ifdef AGP_CMDFIFO
   {
       void  FLUSHAGP(NT9XDEVICEDATA * ppdev);

       FLUSHAGP(ppdev);
   }
#endif // #ifdef AGP_CMDFIFO

  /* Avoid (non-texture) surfaces which have been invalidated by a mode change. - CGW */

   switch( _DD(ddFSAAMode) )
   {
        case  FSAA_MODE_4XBLT:
        case  FSAA_MODE_DEMO:
            // we must use the surfaceData from the 1st backbuffer here 
            // because that is always the rendering surface in blit AA mode
            surfaceData = (FXSURFACEDATA *)_DD( ddAASecondaryBuffer1SurfaceData ); 
            break;
        default:        
            // otherwise use the data from the surface global info
            surfaceData = (FXSURFACEDATA*) (pld->lpDDSurface->lpGbl->dwReserved1);
            break;
   }
   
   
   if ((surfaceData != NULL) && (surfaceData->surfaceLevel != _FF(ddCurrentSurfaceLevel)))
  {
      pld->ddRVal = DDERR_SURFACELOST;
      return DDHAL_DRIVER_HANDLED;
  }

  if (_DD(ddAcceleratorUsed) || _FF(dd3DInOverlay) || (dwCaps & DDSCAPS_3DDEVICE))
  {
    /* Must flush pipeline when surface is locked, in order  */
    /* to process all drawing commands.  A side effect is to */
    /* is to flush previous swap buffer commands, which ends */
    /* up causing a wait on vertical retrace.  Some apps may */
    /* not like this behavior, but the flush is unavoidable  */
    /* for a pipelined architecture. -CGW-                   */

    if (pld->dwFlags & DDLOCK_WAIT)
    {
      CMDFIFO_PROLOG(cmdFifo);
      // D3D-Bringup, be more seletive later
      CMDFIFO_CHECKROOM( cmdFifo, (3*MOP_SIZE) + PH1_SIZE + 1 );

      SETMOP( cmdFifo, SST_MOP_STALL_2D );
  
      SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
             (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));

#ifdef AGP_EXECUTE
      if ( dwCaps & DDSCAPS_NONLOCALVIDMEM )
        SETMOP( cmdFifo, SST_MOP_AGP_FLUSH );
#endif

      CMDFIFO_EPILOG(cmdFifo);// subroutines may output to fifo, but this routine itself is done

      while (FXGETBUSYSTATUS(ppdev));
    }
    else if (FXGETBUSYSTATUS(ppdev))
    {
      pld->ddRVal = DDERR_WASSTILLDRAWING;
      return(DDHAL_DRIVER_HANDLED);
    }
  }
  else
  {
    /* Avoid pipeline flush when surface is locked, if the   */
    /* the rendering surface is not 3D, or if Blt has not    */
    /* been called since the flipping chain was created.     */
    /* The intention is to allow unacclerated applications   */
    /* to avoid flushing.  Must still check the flip status, */
	/* if the surface being locked is involved. -CGW-        */

    /* Return flip status only for surface flipped from or to. */

    if (( pld->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedFrom)) ||
        ( pld->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedTo)))
    {
      /* Check primary surface or video overlay flipping status. */

      if (pld->dwFlags & DDLOCK_WAIT)
      {
        while (FXGETFLIPSTATUS(ppdev));
      }
      else if (FXGETFLIPSTATUS(ppdev))
      {
        pld->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
      }
    }
  }

  surfaceData->dwFlags |= SURFACE_UPDATE;

#ifdef AGP_EXECUTE
  if( dwCaps & DDSCAPS_NONLOCALVIDMEM )
  { 
     // support for tiled surface unswizzling could go here
     // since no phantom spacce this should follow primary surface path
  }
#endif

  if ( !surfaceData->phantomlfbPtr )
  {
        DWORD AAStrideState = 0;

        switch( _DD(ddFSAAMode) )
        {
            case  FSAA_MODE_4XFLIP:

                // if not FSAA, primary will use existing DibEngine CAM entry
                // otherwise we need to update the cam entry for 2x AA surface
                ptr = surfaceData->lfbPtr;


                AAStrideState  = SST_CAM_TILE_MODE1 | SST_CAM_EN_AA;
                AAStrideState |= (surfaceData->dwPStride >> 1) << SST_CAM_PSTRIDE_SHIFT;     // PStride is adjusted for FSAA
                AAStrideState |= surfaceData->dwL2MStride      << SST_CAM_MSTRIDE_SHIFT;
                AAStrideState |= surfaceData->dwBytesPerPixel  << SST_CAM_LFB_DEPTH_SHIFT;   // pixel depth
                AAStrideState |= surfaceData->dwBytesPerPixel  << SST_CAM_PIXEL_DEPTH_SHIFT;
                if (surfaceData->tileFlag & MEM_STAGGER)                                     // check for stagger
                    AAStrideState |= SST_CAM_EN_STAGGER;

#ifdef SLI
                // Enable SLI for the surface
                if (SLI_MODE_ENABLED == _DD(sliMode))
                {
                    if (surfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
                        AAStrideState |= SST_CAM_EN_DISTRIBUTED;
                }
#endif
                
                // the other 2 values in this cam entry should be OK
                _DD(sst2CRegs)->cam[_FF(EndCamEntry)].strideState = AAStrideState;
                _DD(sst2CRegs)->cam[_FF(EndCamEntry)].endAddress  = _DD(sst2CRegs)->cam[_FF(EndCamEntry)].baseAddress + 
                                                                          (surfaceData->dwMStride*surfaceData->dwHeight);
                break;
            case  FSAA_MODE_4XBLT:
            case  FSAA_MODE_DEMO:
                // always return the same surface ptr in mode 3 
                ptr = _DD(ddAASecondaryBuffer1Start); 
                break;

            default:
                // if primary is not AA (2X) use existing DibEngine CAM entry
                ptr = surfaceData->lfbPtr; 
                break;

        }  // end ddFSAAmode switch

  }
  else
  {
      // Program CAM for non-primary surface
      Sst2CAMEntry CAMEntry;

      CAMEntry.strideState  =0;

      // tile/linear modes
      if (surfaceData->tileFlag & MEM_IN_TILE1)      
          CAMEntry.strideState |= SST_CAM_TILE_MODE1;
      else if (surfaceData->tileFlag & MEM_IN_TILE0) 
          CAMEntry.strideState &= ~SST_CAM_TILE_MODE1;
      else                                           
          CAMEntry.strideState |= SST_CAM_LINEAR;

      // stride in physical memory
      switch( _DD(ddFSAAMode) )
      {
      case  FSAA_MODE_4XFLIP:
      case  FSAA_MODE_4XBLT:
            CAMEntry.strideState |= SST_CAM_EN_AA;  
            // must enter "non-AA" size stride here even though AA surface is double sized
            CAMEntry.strideState |= (surfaceData->dwPStride >> 1)   << SST_CAM_PSTRIDE_SHIFT;
            break;
      default:
            CAMEntry.strideState |= surfaceData->dwPStride   <<SST_CAM_PSTRIDE_SHIFT;
            break;
      }
      switch( _DD(ddWindowedAAMode) )
      {
            case  WNAA_MODE_4X:
                CAMEntry.strideState |= SST_CAM_EN_AA;  
                // must enter "non-AA" size stride here even though AA surface is double sized
                CAMEntry.strideState |= (surfaceData->dwPStride >> 1)   << SST_CAM_PSTRIDE_SHIFT;
                break;
            default:
                break; // stridestate should already be set
      }

      CAMEntry.strideState |= surfaceData->dwL2MStride << SST_CAM_MSTRIDE_SHIFT;

      // pixel depth
      CAMEntry.strideState |= surfaceData->dwBytesPerPixel << SST_CAM_LFB_DEPTH_SHIFT;
      CAMEntry.strideState |= surfaceData->dwBytesPerPixel << SST_CAM_PIXEL_DEPTH_SHIFT;

      // Stagger
      if (surfaceData->tileFlag & MEM_STAGGER)      
          CAMEntry.strideState |= SST_CAM_EN_STAGGER;

      // Depth buffer?

      // YUV ??

      // alloc LFB space given stride==power of 2 restriction

#ifdef SLI
      // Enable SLI for the surface
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         if (surfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
            CAMEntry.strideState |= SST_CAM_EN_DISTRIBUTED;
         }
#endif
      ptr = surfaceData->phantomlfbPtr; // pointer returned to Host caller

      CAMEntry.baseAddress = ptr - _FF(LFBBASE);    // CAM needs offset from top of membase1
      CAMEntry.endAddress  = CAMEntry.baseAddress+surfaceData->phantom_blksize;
      CAMEntry.physicalBase= surfaceData->hwPtr;

      if (!FxCamProgram(_DD(camMgr),ppdev,&CAMEntry,CAM_ENTRY_ILLEGAL))
      {
          // CAM programming failed - is there a better error condition?
          pld->ddRVal = DDERR_CANTLOCKSURFACE;
          return DDHAL_DRIVER_HANDLED;
      }

  }

  if( pld->bHasRect )
  {
      if (pld->lpDDSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
      {
        BytesPerPixel = pld->lpDDSurface->lpGbl->ddpfSurface.dwRGBBitCount >> 3;
      }
      else
      {
        BytesPerPixel = pld->lpDDSurface->lpGbl->lpDD->vmiData.ddpfDisplay.dwRGBBitCount >> 3;
      }
      pld->lpSurfData = (LPVOID)( ptr + ((pld->rArea.top * pld->lpDDSurface->lpGbl->lPitch)
                                      + (pld->rArea.left * BytesPerPixel)));
  }
  else
  {
      if ( (dwCaps & DDSCAPS_OVERLAY) && !(dwCaps & DDSCAPS_VIDEOPORT) &&
        (pld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC
	    	!= FOURCC_YV12))
          pld->lpSurfData = (LPVOID)( ptr + surfaceData->dwOvlExtraOffset);
      else
          pld->lpSurfData = (LPVOID) ptr;
  }

  // NEED TO TURN OFF PIXEL PIPELINE?  -SS

  pld->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

}// DdLock

/*----------------------------------------------------------------------
Function name:  DdUnlock

Description:    DDRAW callback Unlock
                Unlocks the specified surface

Information:    This function does nothing useful on nt.  It
                doesn't do anything useful on win9x either

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED	-
				DDHAL_DRIVER_NOTHANDLED -
----------------------------------------------------------------------*/
DWORD __stdcall
DdUnlock( LPDDHAL_UNLOCKDATA puld )
{
  DWORD dwCaps;
  FXSURFACEDATA  *surfaceData;
  DD_ENTRY_SETUP(puld->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "UnLock32" ));
  #endif

  dwCaps = puld->lpDDSurface->ddsCaps.dwCaps;
  puld->ddRVal = DD_OK;


  if ( dwCaps & DDSCAPS_TEXTURE)
  {

#ifdef AGP_EXECUTE
     if( dwCaps & DDSCAPS_NONLOCALVIDMEM )
     { 
       // support for tiled texture swizzling could go here
     }
     else
#endif
        // allow texture code to clean up the lock on a texture surface
        txtrUnlock(ppdev, puld);
  }
  else  // non-texture surface
  {
     surfaceData = (FXSURFACEDATA*) (puld->lpDDSurface->lpGbl->dwReserved1);

#ifdef AGP_EXECUTE
     if( dwCaps & DDSCAPS_NONLOCALVIDMEM )
     { 
       // support for tiled surface swizzling could go here
       // since no phantom spacce this should follow primary surface path
     }
#endif
    if ( !surfaceData->phantomlfbPtr )
    {
        // No phantom space allocated for surface, so this must be Primary
        // which uses CAM entry setup by display driver for DIB engine,so don't free it

        // however... if this is an AA primary in the normal AA mode ( 2x width/height surfaces ),
        // we need to adjust the L2MStride to get the "PrintScreen" key to work properly.
        // When you press the PrintScreen key while an app is running, it reads the primary without
        // doing a lock. The only way I could get that to work properly (in mode 2) is to restore both PStride
        // and L2MStride to the values they would have in a non-AA surface case.
        // In blit AA mode we don't need to change anything since the primary remains as a normal sized surface. (mls 9/29/00)

        DWORD AAStrideState = 0;

        switch( _DD(ddFSAAMode) )
        {
            case  FSAA_MODE_4XFLIP:

                AAStrideState  = SST_CAM_TILE_MODE1 | SST_CAM_EN_AA;
                AAStrideState |= (surfaceData->dwPStride    >> 1) << SST_CAM_PSTRIDE_SHIFT;     // PStride is adjusted for FSAA
                AAStrideState |= (surfaceData->dwL2MStride  - 1) << SST_CAM_MSTRIDE_SHIFT;
                AAStrideState |= surfaceData->dwBytesPerPixel  << SST_CAM_LFB_DEPTH_SHIFT;   // pixel depth
                AAStrideState |= surfaceData->dwBytesPerPixel  << SST_CAM_PIXEL_DEPTH_SHIFT;
                if (surfaceData->tileFlag & MEM_STAGGER)                                     // check for stagger
                    AAStrideState |= SST_CAM_EN_STAGGER;

#ifdef SLI
                // Enable SLI for the surface
                if (SLI_MODE_ENABLED == _DD(sliMode))
                {
                    if (surfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
                        AAStrideState |= SST_CAM_EN_DISTRIBUTED;
                }
#endif
                
                // the other 2 values in this cam entry should be OK
                _DD(sst2CRegs)->cam[_FF(EndCamEntry)].strideState = AAStrideState;
                _DD(sst2CRegs)->cam[_FF(EndCamEntry)].endAddress  = _DD(sst2CRegs)->cam[_FF(EndCamEntry)].baseAddress + 
                                                                          (surfaceData->dwMStride*surfaceData->dwHeight);
                break;
            default:
                break;

        }  // end ddFSAAmode switch
    
   
    }
    else
    {
        // non-texture, non-primary surfaces, free associated CAM entry
        // Note that host (phantom) address space will be freed when surface destroyed
        if (!FxCamFreeEntry(_DD(camMgr), ppdev, surfaceData->phantomlfbPtr-_FF(LFBBASE)))
        {
            // could not unlock surface 
            puld->ddRVal = DDERR_NOTLOCKED;
        }
    }
  }

  return DDHAL_DRIVER_HANDLED;

}// DdUnlock

/*----------------------------------------------------------------------
Function name:  DdAddAttachedSurface

Description:    DDRAW surface callback AddAtachedSurface
                Attaches a surface to another surface

Information:    This function does nothing useful without 3D
                enabled.

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED    -
				DDHAL_DRIVER_NOTHANDLED	-
----------------------------------------------------------------------*/
DWORD __stdcall
DdAddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA pasd )
{
  DWORD dwCap1, dwCap2;
  FXSURFACEDATA* surfaceData1, * surfaceData2;
#ifdef SLI
  DWORD bWidth;
  DWORD sHeight; 
#endif

  DD_ENTRY_SETUP(pasd->lpDD);

  #ifdef FXTRACE
    DISPDBG((ppdev, DEBUG_APIENTRY, "AddAttachSurface32" ));
  #endif

  dwCap1 = pasd->lpDDSurface->ddsCaps.dwCaps;
  dwCap2 = pasd->lpSurfAttached->ddsCaps.dwCaps;

  if(dwCap1 & DDSCAPS_OVERLAY)
  {
    if(dwCap2 & DDSCAPS_OVERLAY)
    {
      surfaceData1 = (FXSURFACEDATA*) (pasd->lpDDSurface->lpGbl->dwReserved1);
      surfaceData2 = (FXSURFACEDATA*) (pasd->lpSurfAttached->lpGbl->dwReserved1);

      surfaceData1->inFlipChain = TRUE;
      surfaceData2->inFlipChain = TRUE;
    }
    else
    {
      pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
      return DDHAL_DRIVER_HANDLED;
    }
  }

  /* attaching system memory with video zbuffer is not allowed */
  if( dwCap1 & DDSCAPS_SYSTEMMEMORY )
  {
    if( ( dwCap2 & DDSCAPS_ZBUFFER ) && ( dwCap2 & DDSCAPS_VIDEOMEMORY ) )
    {
      pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
      return DDHAL_DRIVER_HANDLED;
    }
  }
  else if( dwCap2 & DDSCAPS_SYSTEMMEMORY )
  {
    if( ( dwCap1 & DDSCAPS_ZBUFFER ) && ( dwCap1 & DDSCAPS_VIDEOMEMORY ) )
    {
      pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
      return DDHAL_DRIVER_HANDLED;
    }
  }

#ifdef SLI
  if (SLI_MODE_ENABLED == _DD(sliMode))
      {
      // If surface has not been allocated as a distributed then allocate it as one
      surfaceData1 = (FXSURFACEDATA*) (pasd->lpDDSurface->lpGbl->dwReserved1);
      surfaceData2 = (FXSURFACEDATA*) (pasd->lpSurfAttached->lpGbl->dwReserved1);
      // Check to see if we have allocated a Full-Size Z-Buffer that we can reduce at this time
      if ((FXSURFACE_IS_DISTRIBUTED == (surfaceData1->dwFlags & FXSURFACE_IS_DISTRIBUTED)) &&
          (FXSURFACE_IS_DISTRIBUTED != (surfaceData2->dwFlags & FXSURFACE_IS_DISTRIBUTED)))
         {
         surfaceData2->dwFlags &= ~(FXSURFACE_TYPE_BITS); 
         surfaceData2->dwFlags |= (SLI_MODE_ENABLED == _DD(sliMode)) ? FXSURFACE_IS_DISTRIBUTED : FXSURFACE_IS_EXCLUSIVE;

         // Free the Surface Physical HwPtr
         surfMgr_freeSurface(pasd->lpDD, surfaceData2->lfbPtr, surfaceData2->hwPtr, dwCap1);

         // Reallocate using a smaller footprint distributed across the devices
         bWidth = _DS(ddPrimarySurfaceData.dwStride);
         sHeight = ((pasd->lpSurfAttached->lpGbl->wHeight + (1 << _DD(dwLog2GroupHeight))) >> _DD(dwLog2GroupHeight)) << _DD(dwLog2BandHeight);                     
         surfaceData2->tileFlag = _FF(ddPrimarySurfaceData).tileFlag;
         surfaceData2->dwPStride = _FF(ddPrimarySurfaceData).dwPStride;
         surfaceData2->dwStride = _FF(ddPrimarySurfaceData).dwStride;
         surfaceData2->dwL2MStride = _FF(ddPrimarySurfaceData).dwL2MStride;
         surfaceData2->dwMStride = _FF(ddPrimarySurfaceData).dwMStride;
         surfMgr_allocSurface(pasd->lpDD, dwCap1, dwCap2, bWidth, sHeight, 0, &(surfaceData2->lfbPtr),   
            &(surfaceData2->endlfbPtr), &(surfaceData2->hwPtr), &(surfaceData2->dwStride), &(surfaceData2->tileFlag));
         }
      }
#endif

  pasd->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}


/*----------------------------------------------------------------------
Function name:  DdSetSurfaceColorKey

Description:

Return:         DWORD DDRAW result
                DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
DdSetSurfaceColorKey( LPDDHAL_SETCOLORKEYDATA pssck)
{
  FXSURFACEDATA *surfData;
  DD_ENTRY_SETUP(pssck->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "SetSurfaceColorKey" ));
  DUMP_SETSURFACECOLORKEY32(ppdev, DEBUG_DDGORY, pssck );
  #endif

  // if DDRAW is setting the source colorkey value then store the flags and the colorkey data 
  // in the surface descriptor. my scope on this is limited to D3D texture colorkeys. i believe 
  // there will always be a valid surfData pointer if this driver created the surface and this function
  // should not be called for surfaces that were created by ddraw. - DWF
  if( pssck->dwFlags & DDCKEY_SRCBLT )
  {
    // get a pointer to the surface data stored in the ddraw surface descriptor by textureSurfaceCreate
    surfData = ( FXSURFACEDATA * )pssck->lpDDSurface->lpGbl->dwReserved1;
    surfData->dwSurfLclFlags    = pssck->lpDDSurface->dwFlags;
    surfData->dwColorSpaceLowValue = pssck->ckNew.dwColorSpaceLowValue;
    surfData->dwColorSpaceHighValue = pssck->ckNew.dwColorSpaceHighValue;
  }

  pssck->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
}// DdSetSurfaceColorKey

/*----------------------------------------------------------------------
Function name:  FxGetBusyStatus

Description:    Determine if the pipeline is busy.

Return:         HRESULT DDRAW result
                DD_OK				  -
                DDERR_WASSTILLDRAWING -
----------------------------------------------------------------------*/
#pragma optimize("",off)
HRESULT FxGetBusyStatus(NT9XDEVICEDATA *ppdev)
{
    
   DWORD dwStatus;
#ifdef SLI
   DWORD i;
   DWORD dwChipMask;
   DWORD dwsliBroadcast;

   if (_DD(sliMode))
      {
      dwsliBroadcast = GET(_DD(sst2IORegs)->sliBroadcast);
      dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
      dwStatus = 0;
      for (i=0; i<_FF(dwNumUnits); i++)
         {
         if (dwChipMask & BIT(i))
            {
            // First Set the Broadcast Register for Each Device !!!!
            SETDW(_DD(sst2IORegs)->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
            dwStatus |= GET(_DD(sst2IORegs)->status); 
            }
         }
      SETDW (_DD(sst2IORegs)->sliBroadcast, dwsliBroadcast);
      }
   else
#endif
      dwStatus = GET(_DD(sst2IORegs)->status);

    // Byte access of status register not allowed thus optimize is off
    if (dwStatus & SST2_BUSY)
        return DDERR_WASSTILLDRAWING;
    else
    {
        _DD(ddAcceleratorUsed) = 0; // Pipeline flushed
        return DD_OK;
    }
}//  FxGetBusyStatus

#pragma optimize("",on)

