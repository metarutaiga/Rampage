/* $Header: ddinit32.c, 27, 12/7/00 12:38:35 PM PST, Brent Burton$ */
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
** File Name: 	DDINIT32.C
**
** Description: 
**
** $Revision: 27$
** $Date: 12/7/00 12:38:35 PM PST$
**
** $History: ddinit32.c $
** 
*/

#include "precomp.h"
#include "ddglobal.h"
#include "ddinit.h"
#if (DIRECT3D_VERSION >= 0x0800)
# include <d3dhalex.h>
# include "PSCompiler.h"
#endif
#include "header.h"
#include "fxglobal.h"
#include "runtime.h"
#include "ddvpe32.h"
#include "ddins.h"

#if (!defined(AGP_CMDFIFO) && defined(CSERVICE))
#include "fxmmsst2.h"
#endif

/*
 *
 * NOTE:  All routines are called with the Win16 lock taken.   This is
 * to prevent anyone from calling the display driver and doing something
 * that could confict with this 32-bit driver.
 *
 * This means that all shared 16-32 memory is safe to use at any time inside
 * either driver.
 */

extern void __stdcall Connect( LPVOID );
extern BOOL __stdcall D3DHALCreateDriver(LPD3DHAL_GLOBALDRIVERDATA* lplpGlobal,
                      LPD3DHAL_CALLBACKS* lplpHALCallbacks);

#ifdef USE_R21_MMGR
DWORD __stdcall GetAvailDriverMemory(LPDDHAL_GETAVAILDRIVERMEMORYDATA lpData);
#endif

/*
 * shared memory area!   16-bit display driver gets this by calling GetDataPtr
 * in the 16-bit DLL
 */
extern HINSTANCE hInstance;

#ifdef DEBUG
#define CHECKSIZE(x) if (lpInput->dwExpectedSize != sizeof(x)) \
        DPF("GetDriverInfo: #x structure size mismatch");
#else
#define CHECKSIZE(x)
#endif

extern DWORD __stdcall ddiGetDriverState( LPDDHAL_GETDRIVERSTATEDATA pgdsd );
extern DWORD __stdcall ddiCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA pcsxd );
extern DWORD __stdcall ddiDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA pdddd );
extern DWORD __stdcall DdUpdateNonLocalHeap ( LPDDHAL_UPDATENONLOCALHEAPDATA lpd );
extern DWORD __stdcall ddiClear2 ( LPD3DHAL_CLEAR2DATA pcd );
extern DWORD __stdcall ddiValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pcd );
extern DWORD __stdcall ddiDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA pcd );

#if (DIRECT3D_VERSION >= 0x0800)
static void BuildD3DCaps8 (NT9XDEVICEDATA *ppdev,
                           D3DCAPS8 *D3DCaps8);
#endif

/*----------------------------------------------------------------------
Function name:  DdGetDriverInfo

Description:    Initialize Direct Draw callbacks and data

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpInput)
{
    NT9XDEVICEDATA * ppdev = (NT9XDEVICEDATA * )lpInput->dwContext;
    DWORD dwSize;

    lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;

    if (IsEqualIID(&lpInput->guidInfo, &GUID_Miscellaneous2Callbacks))
    {
        DDHAL_DDMISCELLANEOUS2CALLBACKS Misc2Callbacks;

        DISPDBG((0, "Get Misc2 Callbacks"));

        memset(&Misc2Callbacks, 0, sizeof(Misc2Callbacks));

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS));
        lpInput->dwActualSize = sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS);

        CHECKSIZE(DDHAL_DDMISCELLANEOUS2CALLBACKS);

        Misc2Callbacks.dwSize = dwSize;
        Misc2Callbacks.dwFlags = 0
                               | DDHAL_MISC2CB32_GETDRIVERSTATE
                               | DDHAL_MISC2CB32_CREATESURFACEEX
                               | DDHAL_MISC2CB32_DESTROYDDLOCAL
                               ;

        SETCALLBACK(Misc2Callbacks.GetDriverState,  ddiGetDriverState);
        SETCALLBACK(Misc2Callbacks.CreateSurfaceEx, ddiCreateSurfaceEx);
        SETCALLBACK(Misc2Callbacks.DestroyDDLocal,  ddiDestroyDDLocal);

        memcpy(lpInput->lpvData, &Misc2Callbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
#ifdef AGP_EXECUTE
//#ifdef HW_DVA
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_MotionCompCallbacks))
    {
		extern DWORD __stdcall mcGetMoCompGuids(LPDDHAL_GETMOCOMPGUIDSDATA);
		extern DWORD __stdcall mcGetMoCompFormats(LPDDHAL_GETMOCOMPFORMATSDATA);
		extern DWORD __stdcall mcCreateMoComp(LPDDHAL_CREATEMOCOMPDATA);
		extern DWORD __stdcall mcGetMoCompBuffInfo(LPDDHAL_GETMOCOMPCOMPBUFFDATA);
		extern DWORD __stdcall mcGetInternalMoCompInfo(LPDDHAL_GETINTERNALMOCOMPDATA);
		extern DWORD __stdcall mcBeginMoCompFrame(LPDDHAL_BEGINMOCOMPFRAMEDATA);
		extern DWORD __stdcall mcEndMoCompFrame(LPDDHAL_ENDMOCOMPFRAMEDATA);
		extern DWORD __stdcall mcRenderMoComp(LPDDHAL_RENDERMOCOMPDATA);
		extern DWORD __stdcall mcQueryMoCompStatus(LPDDHAL_QUERYMOCOMPSTATUSDATA);
		extern DWORD __stdcall mcDestroyMoComp(LPDDHAL_DESTROYMOCOMPDATA);
        DDHAL_DDMOTIONCOMPCALLBACKS ddMotionCompCallbacks;
        DISPDBG( (ppdev, 0, "Get MoComp Callbacks"));

        memset(&ddMotionCompCallbacks, 0, sizeof(ddMotionCompCallbacks));

        ddMotionCompCallbacks.dwSize = sizeof(ddMotionCompCallbacks);
        ddMotionCompCallbacks.dwFlags = 
			( DDHAL_MOCOMP32_GETGUIDS
			| DDHAL_MOCOMP32_GETFORMATS     
			| DDHAL_MOCOMP32_CREATE         
			| DDHAL_MOCOMP32_GETCOMPBUFFINFO
			| DDHAL_MOCOMP32_GETINTERNALINFO
			| DDHAL_MOCOMP32_BEGINFRAME     
			| DDHAL_MOCOMP32_ENDFRAME       
			| DDHAL_MOCOMP32_RENDER         
			| DDHAL_MOCOMP32_QUERYSTATUS    
			| DDHAL_MOCOMP32_DESTROY ); 

		ddMotionCompCallbacks.GetMoCompGuids		= mcGetMoCompGuids;
		ddMotionCompCallbacks.GetMoCompFormats		= mcGetMoCompFormats;
		ddMotionCompCallbacks.CreateMoComp			= mcCreateMoComp;
		ddMotionCompCallbacks.GetMoCompBuffInfo		= mcGetMoCompBuffInfo;
		ddMotionCompCallbacks.GetInternalMoCompInfo = mcGetInternalMoCompInfo;
		ddMotionCompCallbacks.BeginMoCompFrame		= mcBeginMoCompFrame;
		ddMotionCompCallbacks.EndMoCompFrame		= mcEndMoCompFrame;
		ddMotionCompCallbacks.RenderMoComp			= mcRenderMoComp;
		ddMotionCompCallbacks.QueryMoCompStatus		= mcQueryMoCompStatus;
		ddMotionCompCallbacks.DestroyMoComp			= mcDestroyMoComp;
        dwSize = min(lpInput->dwExpectedSize, sizeof(ddMotionCompCallbacks));
        lpInput->dwActualSize = sizeof(ddMotionCompCallbacks);
        memcpy(lpInput->lpvData, &ddMotionCompCallbacks, dwSize);

        lpInput->ddRVal = DD_OK;
    }
//#endif	// HW_DVA
#endif

#ifdef CUBEMAP // For CUBEMAPs and more
    else if (IsEqualIID(&(lpInput->guidInfo), &GUID_DDMoreSurfaceCaps) ) 
    {
        DDMORESURFACECAPS   DDMoreSurfaceCaps;
        DDSCAPSEX           ddsCapsEx, ddsCapsExAlt;
        
        DISPDBG((0,"Get GUID_DDMoreSurfaceCaps"));
        
        // fill in everything until expectedsize...
        memset(&DDMoreSurfaceCaps, 0, sizeof(DDMoreSurfaceCaps));
        
        // Caps for heaps 2..n
        memset(&ddsCapsEx, 0, sizeof(ddsCapsEx));
        memset(&ddsCapsExAlt, 0, sizeof(ddsCapsEx));
        
        DDMoreSurfaceCaps.dwSize=lpInput->dwExpectedSize;
        
        DDMoreSurfaceCaps.ddsCapsMore.dwCaps2 = 0
                                    // | DDSCAPS2_HINTDYNAMIC 
                                    // | DDSCAPS2_HINTSTATIC
                                    // | DDSCAPS2_OPAQUE
                                    // | DDSCAPS2_HINTANTIALIASING
                                    | DDSCAPS2_CUBEMAP
                                    | DDSCAPS2_MIPMAPSUBLEVEL
                                    ;
        lpInput->dwActualSize = lpInput->dwExpectedSize;
        
        dwSize = min(sizeof(DDMoreSurfaceCaps),lpInput->dwExpectedSize);
        memcpy(lpInput->lpvData, &DDMoreSurfaceCaps, dwSize);
        
        // now fill in other heaps...
        
        while (dwSize < lpInput->dwExpectedSize)
        {
            memcpy( (PBYTE)lpInput->lpvData+dwSize, 
                    &ddsCapsEx, 
                    sizeof(DDSCAPSEX));
            dwSize += sizeof(DDSCAPSEX);
            memcpy( (PBYTE)lpInput->lpvData+dwSize, 
                    &ddsCapsExAlt, 
                    sizeof(DDSCAPSEX));
            dwSize += sizeof(DDSCAPSEX);
        }
        lpInput->ddRVal = DD_OK;
     }
#endif


#if USE_R21_MMGR  
    if (IsEqualIID(&lpInput->guidInfo, &GUID_MiscellaneousCallbacks))
    {
        DDHAL_DDMISCELLANEOUSCALLBACKS DDMiscCallbacks;

        memset(&DDMiscCallbacks, 0, sizeof(DDMiscCallbacks));
        dwSize = min(lpInput->dwExpectedSize, DDMISCELLANEOUSCALLBACKSSIZE);
        lpInput->dwActualSize = sizeof(DDHAL_DDMISCELLANEOUSCALLBACKS);

        DDMiscCallbacks.dwSize = dwSize;
        DDMiscCallbacks.dwFlags = 0;  // Allow OR selection of callbacks

        DDMiscCallbacks.dwFlags |= DDHAL_MISCCB32_GETAVAILDRIVERMEMORY;
        SETCALLBACK(DDMiscCallbacks.GetAvailDriverMemory, GetAvailDriverMemory);

#ifdef AGP_EXECUTE
        DDMiscCallbacks.dwFlags |= DDHAL_MISCCB32_UPDATENONLOCALHEAP;
        SETCALLBACK(DDMiscCallbacks.UpdateNonLocalHeap, DdUpdateNonLocalHeap);
#endif
        memcpy(lpInput->lpvData, &DDMiscCallbacks, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else
#endif //USE_R21_MMGR
    
    if(VideoGetDriverInfo(lpInput))
    {
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DExtendedCaps) )
    {
        D3DHAL_D3DEXTENDEDCAPS D3DExtendedCaps;

        memset(&D3DExtendedCaps, 0, sizeof(D3DExtendedCaps));
        dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_D3DEXTENDEDCAPS));
        lpInput->dwActualSize = sizeof(D3DHAL_D3DEXTENDEDCAPS);
        CHECKSIZE(D3DHAL_D3DEXTENDEDCAPS);

        D3DExtendedCaps.dwSize = dwSize;
        D3DExtendedCaps.dwMinTextureWidth  = 1;
        D3DExtendedCaps.dwMaxTextureWidth  = 2048;
        D3DExtendedCaps.dwMinTextureHeight = 1;
        D3DExtendedCaps.dwMaxTextureHeight = 2048;

        // MC-FIX-ME, set correct repeat
        D3DExtendedCaps.dwMaxTextureRepeat        = 32768;
        D3DExtendedCaps.dwMaxTextureAspectRatio   = 2048;
        D3DExtendedCaps.dwMaxAnisotropy           = 16;
        D3DExtendedCaps.dvGuardBandLeft           = GUARDBAND_LEFT;     // 
        D3DExtendedCaps.dvGuardBandTop            = GUARDBAND_TOP;      // Need to tweak for best
        D3DExtendedCaps.dvGuardBandRight          = GUARDBAND_RIGHT;    // performance.
        D3DExtendedCaps.dvGuardBandBottom         = GUARDBAND_BOTTOM;   // 
        D3DExtendedCaps.dvExtentsAdjust           = 0.f;

        // MC-CHECK-ME, does expose of these caps wind up dependant
        // on the size of the primary when the HAL is first loaded?

        if ( GETPRIMARYBYTEDEPTH == 2 ) // don't support stencil in 16bpp
        {
            D3DExtendedCaps.dwStencilCaps = 0;
        }
        else
        {
            D3DExtendedCaps.dwStencilCaps = D3DSTENCILCAPS_DECR     |
                                            D3DSTENCILCAPS_DECRSAT  |
                                            D3DSTENCILCAPS_INCR     |
                                            D3DSTENCILCAPS_INCRSAT  |
                                            D3DSTENCILCAPS_INVERT   |
                                            D3DSTENCILCAPS_KEEP     |
                                            D3DSTENCILCAPS_REPLACE  |
                                            D3DSTENCILCAPS_ZERO     |
                                            0;
        }

        D3DExtendedCaps.dwTextureOpCaps = D3DTEXOPCAPS_DISABLE |
                                          D3DTEXOPCAPS_SELECTARG1 |
                                          D3DTEXOPCAPS_SELECTARG2 |
                                          D3DTEXOPCAPS_MODULATE |
                                          D3DTEXOPCAPS_MODULATE2X |
                                          D3DTEXOPCAPS_MODULATE4X |
                                          D3DTEXOPCAPS_ADD |
                                          D3DTEXOPCAPS_ADDSIGNED |
                                          D3DTEXOPCAPS_ADDSIGNED2X |
                                          D3DTEXOPCAPS_SUBTRACT |
                                          D3DTEXOPCAPS_ADDSMOOTH |
                                          D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
                                          D3DTEXOPCAPS_BLENDTEXTUREALPHA |
                                          D3DTEXOPCAPS_BLENDFACTORALPHA |
                                          D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
                                          D3DTEXOPCAPS_BLENDCURRENTALPHA |
                                          D3DTEXOPCAPS_PREMODULATE |
                                          D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
                                          D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
                                          D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
                                          D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
                                          D3DTEXOPCAPS_BUMPENVMAP |
                                          D3DTEXOPCAPS_BUMPENVMAPLUMINANCE |
                                          D3DTEXOPCAPS_DOTPRODUCT3 |
                                          0;

        D3DExtendedCaps.wMaxTextureBlendStages    = 8;
        D3DExtendedCaps.wMaxSimultaneousTextures  = 8;
        D3DExtendedCaps.dwFVFCaps                 = 8 | D3DFVFCAPS_DONOTSTRIPELEMENTS;
        D3DExtendedCaps.dwVertexProcessingCaps    = 0;

        memcpy(lpInput->lpvData, &D3DExtendedCaps, dwSize);
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DCallbacks3) )
    {
        D3DHAL_CALLBACKS3 D3DCallbacks3;
        memset(&D3DCallbacks3, 0, sizeof(D3DCallbacks3));

        dwSize = min(lpInput->dwExpectedSize, sizeof(D3DHAL_CALLBACKS3));
        lpInput->dwActualSize = sizeof(D3DHAL_CALLBACKS3);
        CHECKSIZE( D3DHAL_CALLBACKS3 );

        D3DCallbacks3.dwSize = dwSize;

        D3DCallbacks3.dwFlags = D3DHAL3_CB32_CLEAR2 |
                                D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE |
                                D3DHAL3_CB32_DRAWPRIMITIVES2 |
                                0;

        SETCALLBACK(D3DCallbacks3.Clear2, ddiClear2);
        SETCALLBACK(D3DCallbacks3.ValidateTextureStageState, ddiValidateTextureStageState);
        SETCALLBACK(D3DCallbacks3.DrawPrimitives2, ddiDrawPrimitives2);

        memcpy(lpInput->lpvData, &D3DCallbacks3, dwSize);
        lpInput->ddRVal = DD_OK;
    }
#if (DIRECT3D_VERSION >= 0x0800)
    // Check for calls to GetDriverInfo2
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_GetDriverInfo2) )
    {
        // Make sure this is actually a call to GetDriverInfo2 
        // ( and not a call to DDStereoMode!)
        if (D3DGDI_IS_GDI2(lpInput))
        {
            // Yes, its a call to GetDriverInfo2, fetch the
            // DD_GETDRIVERINFO2DATA data structure.
            DD_GETDRIVERINFO2DATA* pgdi2 = D3DGDI_GET_GDI2_DATA(lpInput);

            // What type of request is this?
            switch (pgdi2->dwType)
            {
              case D3DGDI2_TYPE_GETD3DCAPS8:
              {
                  size_t copySize;
                  D3DCAPS8 D3DCaps8;

                  BuildD3DCaps8 (ppdev, &D3DCaps8);

                  // It should be noted that the dwExpectedSize field
                  // of DD_GETDRIVERINFODATA is not used for
                  // GetDriverInfo2 calls and should be ignored.
                  copySize = min(sizeof(D3DCAPS8), pgdi2->dwExpectedSize);
                  memcpy(lpInput->lpvData, &D3DCaps8, copySize);
                  lpInput->dwActualSize = copySize;
                  lpInput->ddRVal       = DD_OK;
                  break;
              }
              case D3DGDI2_TYPE_GETFORMATCOUNT:
              {
                  extern DWORD d3GetFormatCount(void); // bpb
                  ((DD_GETFORMATCOUNTDATA*)lpInput->lpvData)->dwFormatCount = d3GetFormatCount();
                  lpInput->dwActualSize = min(sizeof(DD_GETFORMATCOUNTDATA), pgdi2->dwExpectedSize);
                  lpInput->ddRVal = DD_OK;
                  break;
              }

              case D3DGDI2_TYPE_GETFORMAT:
                {
                    extern DDPIXELFORMAT *d3GetFormat(DWORD); // bpb
                    DWORD idx = 
                        ((DD_GETFORMATDATA*)lpInput->lpvData)->dwFormatIndex;

                    int gfdsize = sizeof(DD_GETFORMATDATA), expsize = pgdi2->dwExpectedSize;
                    lpInput->dwActualSize = min(sizeof(DD_GETFORMATDATA), pgdi2->dwExpectedSize);
                    memcpy ( &((DD_GETFORMATDATA*)lpInput->lpvData)->format,
                             d3GetFormat (idx),
                             lpInput->dwActualSize);
                    lpInput->ddRVal = DD_OK;
                    break;
                }

              case D3DGDI2_TYPE_DXVERSION:   // Runtime reports its version
                lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;
                break;

              default:
                // Default behavior for any other type.
                lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;
                break;
            }
        }
    }
#endif // DD >= 0x0800
    else if ( IsEqualIID(&lpInput->guidInfo, &GUID_ZPixelFormats) )
    {
        // Notes :  The 16 bit Z buffer is also represented here. This is because DD only recognizes either
        //          the DDBD standard form CAPs, or this GUID_ZPIXELFORMATS form.
        //          24 Bit Z (no stencil) is now supported to handle cases where bpp differs from Z/stencil depth.
        //          Stencil is not available for 16bpp due to fbzMode hardware restriction.

        #define NUMBER_ZPIXELFORMATS 3

        static DDPIXELFORMAT ZPixelFormats [NUMBER_ZPIXELFORMATS] =
        {
            // 16 bit Z, no stencil
            { sizeof(DDPIXELFORMAT),                // ddpfPixelFormat.dwSize
              DDPF_ZBUFFER,                         // ddpfPixelFormat.dwFlags
              0,                                    // FOURCC code
              16,                                   // dwZBufferBitDepth
              0,                                    // dwStencilBitDepth
              0xFFFF,                               // dwZbitMask
              0x0000,                               // dwStencilBitMask
              0,                                    // not used
            },
            // 24 bit Z, no stencil
            { sizeof(DDPIXELFORMAT),                // ddpfPixelFormat.dwSize
              DDPF_ZBUFFER,                         // ddpfPixelFormat.dwFlags
              0,                                    // FOURCC code
              24,                                   // dwZBufferBitDepth
              0,                                    // dwStencilBitDepth
              0x00FFFFFF,                           // dwZbitMask         lower 24 bits
              0x00000000,                           // dwStencilBitMask
              0,                                    // not used
            },
            // 24 bit Z, 8 bit stencil
            { sizeof(DDPIXELFORMAT),                // ddpfPixelFormat.dwSize
              DDPF_ZBUFFER | DDPF_STENCILBUFFER,    // ddpfPixelFormat.dwFlags
              0,                                    // FOURCC code
              32,                                   // dwZBufferBitDepth
              8,                                    // dwStencilBitDepth
              0x00FFFFFF,                           // dwZbitMask         lower 24 bits
              0xFF000000,                           // dwStencilBitMask   upper  8 bits
              0,                                    // not used
            },
        };

        BOOL ZFormatValid [NUMBER_ZPIXELFORMATS] = { // Assume all valid initially
                                                     TRUE,   // 16 bit Z, no stencil
                                                     TRUE,   // 24 bit Z, no stencil
                                                     TRUE    // 24 bit Z, with 8 bit Stencil
                                                   };

        LPBYTE  lpbBuffer = (LPBYTE)lpInput->lpvData;
        DWORD   dwNumberZPixelFormats = NUMBER_ZPIXELFORMATS;
        DWORD   dwSizeLeft;
        int     formatIndex = 0;

        if ( GETPRIMARYBYTEDEPTH == 2 )     // don't support stencil in 16bpp
        {
            ZFormatValid [2] = FALSE;       // Turn off ivalid format for 16 bpp
            dwNumberZPixelFormats--;
        }

        // Return a buffer in which the first DWORD is the number of valid Z buffer
        // DDPIXELFORMATs, followed by the formats themselves.
        if ( lpInput->dwExpectedSize >= sizeof(DWORD) )
        {
            memcpy(lpbBuffer, &dwNumberZPixelFormats, sizeof(DWORD));
            lpbBuffer += sizeof(DWORD);
            dwSizeLeft = min((lpInput->dwExpectedSize - sizeof(DWORD)), (NUMBER_ZPIXELFORMATS * sizeof(DDPIXELFORMAT)) );
            lpInput->dwActualSize = sizeof(DWORD);

            // Copy all the formats to the return buffer, but no more than the expected number of bytes
            while ( dwSizeLeft )
            {
                if ( ZFormatValid [formatIndex] )
                {
                    if ( dwSizeLeft >= sizeof(DDPIXELFORMAT) )
                        dwSize = sizeof(DDPIXELFORMAT);
                    else
                        dwSize = dwSizeLeft;
                    dwSizeLeft -= dwSize;

                    memcpy(lpbBuffer, &ZPixelFormats[formatIndex], dwSize);
                    lpInput->dwActualSize += dwSize;
                    lpbBuffer += sizeof(DDPIXELFORMAT);
                }
                formatIndex++;
            }
        }
        else
            lpInput->dwActualSize = 0;
        lpInput->ddRVal = DD_OK;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_D3DParseUnknownCommandCallback) )
    {
        extern PFND3DPARSEUNKNOWNCOMMAND dp2Callback;

        dp2Callback = (PFND3DPARSEUNKNOWNCOMMAND)lpInput->lpvData;
        lpInput->ddRVal = DD_OK;
    }
    return DDHAL_DRIVER_HANDLED;
}

#if (DIRECT3D_VERSION >= 0x0800)
/*----------------------------------------------------------------
 * BuildD3DCaps8()
 *
 * Set the new DX8 caps, returned in the D3DCAPS8 pointer passed in.
 * No globals changed, no side effects except for setting *d.
 *
 * Many of the values here are taken from setting the
 * D3DExtendedCaps.
 */
static void
BuildD3DCaps8 (NT9XDEVICEDATA *ppdev, D3DCAPS8 *d8)
{
    D3DDEVICEDESC_V1 *dd = &(_D3(d3d).hwCaps);
    D3DPRIMCAPS      *tc = &dd->dpcTriCaps;

    d8->DeviceType     = 0;                  // 0 per spec
    d8->AdapterOrdinal = 0;                  // 0 per spec

    d8->Caps           = 0;
    d8->Caps2          = D3DCAPS2_CANRENDERWINDOWED;
    d8->Caps3          = 0;
    d8->PresentationIntervals = 0;

    d8->CursorCaps        = 0;

    d8->DevCaps           = dd->dwDevCaps;
    d8->PrimitiveMiscCaps = tc->dwMiscCaps;
    d8->RasterCaps        = tc->dwRasterCaps;
    d8->ZCmpCaps          = tc->dwZCmpCaps;
    d8->SrcBlendCaps      = tc->dwSrcBlendCaps;
    d8->DestBlendCaps     = tc->dwDestBlendCaps;
    d8->AlphaCmpCaps      = tc->dwAlphaCmpCaps;
    d8->ShadeCaps         = tc->dwShadeCaps;;
    d8->TextureCaps       = tc->dwTextureCaps;
    d8->TextureFilterCaps = tc->dwTextureFilterCaps;
    d8->CubeTextureFilterCaps = 0;           // bpb - needs fixing
    d8->VolumeTextureFilterCaps = 0;
    d8->TextureAddressCaps = tc->dwTextureAddressCaps;
    d8->VolumeTextureAddressCaps = 0;

    d8->LineCaps = 0;

    d8->MaxTextureWidth = 2048;
    d8->MaxTextureHeight = 2048;
    d8->MaxVolumeExtent = 0;

    d8->MaxTextureRepeat = 32768;
    d8->MaxTextureAspectRatio = 2048;
    d8->MaxAnisotropy = 16;
    d8->MaxVertexW = 0;

    d8->GuardBandLeft = GUARDBAND_LEFT;
    d8->GuardBandTop = GUARDBAND_TOP;
    d8->GuardBandRight = GUARDBAND_RIGHT;
    d8->GuardBandBottom = GUARDBAND_BOTTOM;

    d8->ExtentsAdjust = 0.0f;
    d8->StencilCaps = (D3DSTENCILCAPS_DECR     |
                       D3DSTENCILCAPS_DECRSAT  |
                       D3DSTENCILCAPS_INCR     |
                       D3DSTENCILCAPS_INCRSAT  |
                       D3DSTENCILCAPS_INVERT   |
                       D3DSTENCILCAPS_KEEP     |
                       D3DSTENCILCAPS_REPLACE  |
                       D3DSTENCILCAPS_ZERO     |
                       0);

    d8->TextureOpCaps = (D3DTEXOPCAPS_DISABLE |
                         D3DTEXOPCAPS_SELECTARG1 |
                         D3DTEXOPCAPS_SELECTARG2 |
                         D3DTEXOPCAPS_MODULATE |
                         D3DTEXOPCAPS_MODULATE2X |
                         D3DTEXOPCAPS_MODULATE4X |
                         D3DTEXOPCAPS_ADD |
                         D3DTEXOPCAPS_ADDSIGNED |
                         D3DTEXOPCAPS_ADDSIGNED2X |
                         D3DTEXOPCAPS_SUBTRACT |
                         D3DTEXOPCAPS_ADDSMOOTH |
                         D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
                         D3DTEXOPCAPS_BLENDTEXTUREALPHA |
                         D3DTEXOPCAPS_BLENDFACTORALPHA |
                         D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
                         D3DTEXOPCAPS_BLENDCURRENTALPHA |
                         D3DTEXOPCAPS_PREMODULATE |
                         D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
                         D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
                         D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
                         D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
                         D3DTEXOPCAPS_BUMPENVMAP |
                         D3DTEXOPCAPS_BUMPENVMAPLUMINANCE |
                         D3DTEXOPCAPS_DOTPRODUCT3 |
                         0);
    d8->FVFCaps = 8 | D3DFVFCAPS_DONOTSTRIPELEMENTS;

    d8->MaxTextureBlendStages = 8;
    d8->MaxSimultaneousTextures = 8;

    d8->VertexProcessingCaps = 0;
    d8->MaxActiveLights = 0;
    d8->MaxUserClipPlanes = 0;
    d8->MaxVertexBlendMatrices = 0;
    d8->MaxVertexBlendMatrixIndex = 0;

    d8->MaxPointSize = 64.0f;

    d8->MaxPrimitiveCount = 0xffff;         //no D3DMAXNUMPRIMITIVES in dx8
    d8->MaxVertexIndex = 0xffff;            //no D3DMAXNUMVERTICES in dx8
    d8->MaxStreams = 1;
    d8->MaxStreamStride = 256;

    d8->VertexShaderVersion  = 0;           //D3DVS_VERSION(1, 0);
    d8->MaxVertexShaderConst = 0;           // # of vert shader const regs. 96?

    {
        PSC_CAPS psc;                       // Pixel Shader Compiler caps

        PSCGetCaps (&psc);
        d8->PixelShaderVersion  = psc.pixelShaderVersion;
        d8->MaxPixelShaderValue = (float)psc.maxPixelShaderValue;

        D3DPRINT( DL_DX8_PS, "DX8PS: version = 0x%08x  maxValue = %f",
                  psc.pixelShaderVersion, psc.maxPixelShaderValue);
    }

} // BuildD3DCaps8()
#endif // DX >= 8


#if USE_R21_MMGR  
/*----------------------------------------------------------------------
Function name:  GetAvailDriverMemory

Description:    Determines how much free memory is in the drivers' private heap
				(that is, heaps that the driver did not expose to DirectDraw).

Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED    -
                DDHAL_DRIVER_NOTHANDLED	-
----------------------------------------------------------------------*/
DWORD __stdcall GetAvailDriverMemory(LPDDHAL_GETAVAILDRIVERMEMORYDATA lpData)
{
   DD_ENTRY_SETUP(lpData->lpDD)

   lpData->ddRVal = DDERR_CURRENTLYNOTAVAIL;

   if( lpData->DDSCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
   {
#ifdef AGP_EXECUTE
        DPF("(GetAvailDriverMemory) AGP memory punting to ddraw");
#else
        DPF("(GetAvailDriverMemory) AGP memory not handled");
#endif
        lpData->ddRVal = DD_OK;
        return DDHAL_DRIVER_NOTHANDLED;

   } else if (
      ((   DDSCAPS_LOCALVIDMEM
        | DDSCAPS_VIDEOMEMORY
        | DDSCAPS_BACKBUFFER
        | DDSCAPS_ZBUFFER
        | DDSCAPS_3DDEVICE
        | DDSCAPS_TEXTURE )
        & lpData->DDSCaps.dwCaps)
   || ((  DDSCAPS_COMPLEX & lpData->DDSCaps.dwCaps) &&
       (  DDSCAPS_FLIP & lpData->DDSCaps.dwCaps))
   || (0 == lpData->DDSCaps.dwCaps))
   {
        lpData->dwTotal = _FF(mmTransientHeapSize);
        lpData->dwFree  =  memMgr_GetFreeMemSize(lpData->lpDD);

#if (!defined(AGP_CMDFIFO) && defined(CSERVICE))
        lpData->dwTotal += FXMM_SST2_LOANBOUNDARY;  //take into account what can be borrowed
#endif

        DPF("(GetAvailDriverMemory LOCAL) free = %li, total= %li",
               lpData->dwFree, lpData->dwTotal);

        lpData->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }
    return DDHAL_DRIVER_NOTHANDLED;
}
#endif // USE_R21_MMGR


#ifdef AGP_EXECUTE
//Needed for running on CSimServer 
#include "iSST2.h"
#include "iR3.h"
#include "iHydra.h"
#include "iRage.h"

void __stdcall csimUpdateAgpConfig( DWORD physBase, DWORD linBase, DWORD sizeInBytes )
{
    DWORD dwCount;
    DWORD dwDevNode;

    {
        SST2INTERFACE iSST2;
   
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

        SST2_Enable_Device_AGP(iSST2, 
                               dwDevNode,
                               physBase,
                               linBase,
                               sizeInBytes );
 
        SST2_Release_Interface(iSST2);

        goto ConnectDone;
    }
SST2ConnectFail: ;

    {
        R3INTERFACE iR3;
        DWORD dwCount;
        DWORD dwDevNode;
   
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

        R3_Enable_Device_AGP(iR3, 
                             dwDevNode,
                             physBase,
                             linBase,
                             sizeInBytes );
 
        R3_Release_Interface(iR3);

        goto ConnectDone;
    }
R3ConnectFail: ;

    {
        HYDRAINTERFACE iHydra;
        DWORD dwCount;
        DWORD dwDevNode;
   
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

        Hydra_Enable_Device_AGP(iHydra, 
                                dwDevNode,
                                physBase,
                                linBase,
                                sizeInBytes );
 
        Hydra_Release_Interface(iHydra);

        goto ConnectDone;
    }
HydraConnectFail: ;

    {
        RAGEINTERFACE iRage;
        DWORD dwCount;
        DWORD dwDevNode;
   
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

        Rage_Enable_Device_AGP(iRage, 
                               dwDevNode,
                               physBase,
                               linBase,
                               sizeInBytes );
 
        Rage_Release_Interface(iRage);

        goto ConnectDone;
    }
RageConnectFail: ;

ConnectDone: ;
}


/*----------------------------------------------------------------------
Function name:  DdUpdateNonLocalHeap

Description:    
Return:         DWORD DDRAW result
                DDHAL_DRIVER_HANDLED    -
				    DDHAL_DRIVER_NOTHANDLED	-
----------------------------------------------------------------------*/

DWORD __stdcall DdUpdateNonLocalHeap( LPDDHAL_UPDATENONLOCALHEAPDATA lpd )
{
   FxU32 agpStart;
   FxU32 agpFinish;
   FxU32 agpLinear;

   DD_ENTRY_SETUP(lpd->lpDD)

#if defined(CSERVICE) //[

   //steal a pointer to the AGP heap, so CS can play by DDRAW's rules.
   _FF(csDirectDrawHeaps) = (DWORD)lpd->lpDD->vmiData.pvmList;

#endif //] CSERVICE


   if( lpd->dwHeap == AGP_HEAP_ID )  // Assuming one linear and one agp heap
   {
      if( _FF(enableAGPEM) )
      {
         _DD(agpMode) = EXECUTE_MODE_BLTS | EXECUTE_MODE_TEXTURES;
         agpLinear    = _FF(agpHeapLinBaseAddr)  = lpd->fpGARTLin;
         agpStart     = _FF(agpHeapPhysBaseAddr) = lpd->fpGARTDev;
         agpFinish    = agpStart + _FF(agpHeapSize)*1024*1024 -1;

         if (!_FF(isHardware))  //WinSIM stuff
         {
            if (_FF(enableAGPCF))
            {
               //RYAN: presently, CSIM only supports one AGP range, so we'll
               //just glom our fifo and our heap into one big honkin' block,
               //even though there may be a "no man's land" between them.

               agpLinear = (FxU32)NULL; //NULL means, "map it yourself"
               agpStart  = __min(agpStart,  _FF(agpMain.physAddr));
               agpFinish = __max(agpFinish, _FF(agpMain.physAddr)+_FF(agpMain.sizeInB)-1);
            }
             
            csimUpdateAgpConfig( agpStart, 
                                 agpLinear,
                                 agpFinish-agpStart+1 );
         }
      }
      else
      {
         _DD(agpMode) = EXECUTE_MODE_DISABLED;
         _FF(agpHeapLinBaseAddr) = 0;
         _FF(agpHeapPhysBaseAddr) = 0;
      }
      lpd->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
   }

   return DDHAL_DRIVER_HANDLED;   // Doc says we cannot fail this call
} 
#endif // AGP_EXECUTE
