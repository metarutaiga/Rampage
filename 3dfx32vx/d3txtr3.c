/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Log: 
**  28   Rampage   1.27        9/6/00   Michel Conrad   Fixed a typo in the 
**       lfbptrend calculation. Update setting of txDirtyFlags on locks of 
**       cubemaps.
**  27   Rampage   1.26        8/3/00   Don Fowler      Added code to handle 
**       legacy ALPHA MODULATE state. Alpha modulate will now get alpha 
**       information from the iterator if there is no alpha information in the 
**       pixel format.    Added support for texture SRC colorkeying    Fixed Z 
**       bias to support Rampage 24 bit bias and added an arbitrary multiplier 
**       to "fixup" the value of 0-15 passed by ddraw. The number I chose was 
**       entirely arbitrary and was just enough to get the tests to pass.    
**       Coupled the AlphaDepthTest with the AlphaColorTest because of a bug in 
**       CSIM and possibly in the hardware.
**  26   Rampage   1.25        7/10/00  Michel Conrad   Add cubemap support. 
**       Phase out AGP_EXECUTE ifdefs.
**  25   Rampage   1.24        5/11/00  Evan Leland     dx7 structure cleanup 
**       effort complete
**  24   Rampage   1.23        5/3/00   Ryan Bissell    Fixed PRS #12821: 
**       "Verdict DirectDraw Mem Allocation Stress test crashes."
**  23   Rampage   1.22        4/24/00  Evan Leland     preliminary changes, 
**       getting ready to removing all ddraw structure pointers stored in the 
**       driver
**  22   Rampage   1.21        4/13/00  Evan Leland     Force agp textures to 
**       linear; converge to minimal texture download routines.
**  21   Rampage   1.20        4/12/00  Evan Leland     Added 
**       txtrSurfaceCreateEx to initialize texture-specific data on behalf of 
**       ddiCreateSurfaceEx.
**  20   Rampage   1.19        4/10/00  Evan Leland     Forgot to remove a few 
**       debug variables.
**  19   Rampage   1.18        4/10/00  Evan Leland     Added linear texture 
**       support.
**  18   Rampage   1.17        4/5/00   Evan Leland     modification to the code
**       that calculates surfData structures to make sure the values are 
**       correct; some minor code streamlining
**  17   Rampage   1.16        4/4/00   Evan Leland     Add partial download for
**       NPT textures.
**  16   Rampage   1.15        4/4/00   Evan Leland     fix for partial texture 
**       load bug in txtrLoadDp2
**  15   Rampage   1.14        3/31/00  Evan Leland     Support for partial 
**       texture loading; some streamlining of txtrLoadDp2.
**  14   Rampage   1.13        3/16/00  Brent Burton    Changed arguments to 
**       txtrUnmungeUVLSurface() to make it more generic.  Also added calls to 
**       same from Blt code to handle DX7 TEXBLT updating of bumpmaps.
**  13   Rampage   1.12        3/14/00  Evan Leland     added support for 
**       COPYFOURCC through calls to txtrDownloadLevelDXT
**  12   Rampage   1.11        3/8/00   Brent Burton    Added unmunging of UV 
**       textures to txtrUnlock().
**  11   Rampage   1.10        2/24/00  Evan Leland     just added comments to 
**       function headers
**  10   Rampage   1.9         2/7/00   Michel Conrad   Tweaks to restore AGP 
**       execute mode functionality.
**  9    Rampage   1.8         2/4/00   Evan Leland     Added AGP code back in; 
**       modified texture management to support multiple texture descriptors; 
**       fixed problem where multiple txtrSurfaceDelete calls per single 
**       txtrSurfaceCreate call was causing trouble
**  8    Rampage   1.7         1/29/00  Brent Burton    Dorked with a typecast 
**       to clean up a compile warning.
**  7    Rampage   1.6         1/29/00  Brent Burton    Fixed some error 
**       detection for UVL and removed dead code from txtrSurfaceDelete().
**  6    Rampage   1.5         1/28/00  Brent Burton    Added new UVL texture 
**       code, made a few functions more modular, added new functions, and 
**       removed TXTRDESC references.
**  5    Rampage   1.4         1/28/00  Evan Leland     Adds support for 
**       txtrLoadDp2 -- support for ddiDrawPrim2 texture download
**  4    Rampage   1.3         1/27/00  Evan Leland     DX7 changes
**  3    Rampage   1.2         1/25/00  Evan Leland     updates for dx7
**  2    Rampage   1.1         1/25/00  Brent Burton    Created 
**       txtrSetupTxtrDesc() function in d3txhlp.[ch].  Removed this body of 
**       code from txtrSurfaceCreate() to make it more modular.
**  1    Rampage   1.0         1/25/00  Evan Leland     
** $
*/

#include "d3global.h"                       // global structure defns
#include "ddcam.h"                          // FxCam..., cam manager defines
#include "d3txtr2.h"                        // internal texture data structures
#include "d3npt.h"                          // txtrNptInfo
#include "fifomgr.h"                        // CMDFIFO_PROLOG, etc

//---------------------------------------------------------------------------------------
//
// txtrSurfaceCreate()
//
// Description: Single entry point for texture surface creation. Constructor for
//  texture surfaces.
//
// Side effects: 
//  Allocates from heap for texture data structures.
//  Allocates from frame buffer and phantom memory manager.
//  Writes data back to direct draw surface structures.
//
// Algorithm:
//  check for early exit conditions
//  map from D3D texture format to sst2 format, determine bpt
//  allocate driver data structures (FXSURFACEDATA, txtrDesc) for this texture
//  for each texture descriptor needed to describe this texture
//      calculate size info for each mip level present
//      allocate frame buffer and virtual memory for texture (including mip levels)
//      calculate frame buffer and virtual memory pointers at each mip level
//      calculate sst2 register values for this texture
//  compute generic surface data stored by driver for all surfaces
//  return info about this surface to DirectDraw
// 
//---------------------------------------------------------------------------------------

FxU32 __stdcall txtrSurfaceCreate(          // create a texture surface
    NT9XDEVICEDATA             *ppdev,      // IN: global driver data
    LPDDRAWI_DDRAWSURFACE_LCL   surfLCL,    // IN: direct draw surface local struct
    HRESULT                    *retCode)    // OUT: direct draw return value
{
    FxU32 tlod, txFormat, txBpt, txFormatFlags, lms;
    FxU32 txHwStart, txVaStart;
    txtrDesc *txtr;
    FXSURFACEDATA *surfData;
    LPDDRAWI_DDRAWSURFACE_LCL next;
    HRESULT ddec = DD_OK;   // direct draw error code: plan for success
    FxU32 ddrc;             // direct draw return code
    FxU32 rc;

    // check for system memory texture
    if (surfLCL->lpGbl->dwGlobalFlags & DDRAWISURFGBL_SYSMEMREQUESTED) {
        ddec = 0;
        ddrc = DDHAL_DRIVER_NOTHANDLED;
        goto unwind1;
    }

    // check for texture size out of range
    if ((surfLCL->lpGbl->wWidth > 2048) || (surfLCL->lpGbl->wHeight > 2048)) {
        surfLCL->lpGbl->fpVidMem = (DWORD) NULL;
        ddec = DDERR_OUTOFVIDEOMEMORY; // no good return code defined 
        ddrc = DDHAL_DRIVER_HANDLED;
        goto unwind1;
    }

#ifdef CUBEMAP
    if (surfLCL->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP)
    { 
        HRESULT  retCodeCEM;

        // cube map texture creation requires enough special case stuff
        // that it seemed best to hanlde it all seperately

        *retCode = txtrCreateCubeMap( ppdev, surfLCL, &retCodeCEM );
        return retCodeCEM;
    }
#endif

    // determine sst2 texture format, texel depth
    rc = txtrGetFormat(surfLCL, ppdev, &txFormat, &txBpt, &txFormatFlags);
    if (rc != 0) {
        ddec = DDERR_INVALIDPIXELFORMAT;
        ddrc = DDHAL_DRIVER_HANDLED;
        goto unwind1;
    }

    if (surfLCL->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        txFormatFlags |= TEXFMTFLG_NLVM;

    // call the FXSURFACEDATA and txtrDesc constructor
    rc = txtrSurfDataAlloc(ppdev, &surfData, txFormat, txFormatFlags, txBpt);
    if (rc != 0) {
        ddec = DDERR_OUTOFVIDEOMEMORY;
        ddrc = DDHAL_DRIVER_HANDLED;
        goto unwind1;
    }

    // obtain local pointer to the texture descriptor
    txtr = surfData->pTxtrDesc;

    // for all the texture descriptors present (in most cases only 1)
    while (txtr != NULL) {
        FxU32 txAGPGartPhys, flags, mipCount = surfLCL->lpSurfMore->dwMipMapCount;
        FxU32 width  = surfLCL->lpGbl->wWidth;
        FxU32 height = surfLCL->lpGbl->wHeight;

        #ifdef LINEAR_TEXTURES // compile-flag forces all textures to linear
        txtr->txFormatFlags |= TEXFMTFLG_LINEAR;
        #endif

        // force agp textures to be linear
        if (surfLCL->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
            txtr->txFormatFlags |= TEXFMTFLG_LINEAR;

        // initialize info for each mipmap levels for this txtrDesc
        rc = txtrCalcMipmapInfo(ppdev, txtr, width, height, mipCount); 
        if (rc != 0) {
            ddec = DDERR_INVALIDPARAMS;
            ddrc = DDHAL_DRIVER_HANDLED;
            goto unwind2;
        }

        // AGP surfaces don't require cam virtual memory
        if (surfLCL->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
            flags = TXTR_ALLOC_FRAMEBUF_MEM;
        else
            flags = TXTR_ALLOC_FRAMEBUF_MEM | TXTR_ALLOC_VIRTUAL_MEM;

        // allocate frame buffer memory and virtual memory as needed
        rc = txtrAllocMemory(ppdev, surfLCL, txtr->txTotalSize, flags,
                             &txHwStart, &txVaStart, &txAGPGartPhys);
        if (rc != 0) {
            ddec = DDERR_OUTOFVIDEOMEMORY;
            ddrc = DDHAL_DRIVER_HANDLED;
            goto unwind2;
        }

        // set top-level pointers to texture memory
        if (surfLCL->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) {
            txtr->txHwGartPhysStart = txAGPGartPhys;
            txtr->txVaGartLinStart  = txHwStart;
        }
        else {
            txtr->txHwStart = txHwStart;
            txtr->txVaStart = txVaStart;
        }

        // calculate mipmap frame buffer and virtual memory addresses
        txtrCalcMipPointers(txtr);

        // calculate VTA register values that are stored with the txtrDesc
        txtrCalcRegisterValues(ppdev, txtr);

        txtr = txtr->txtrDescNext;
    }

    // set local pointer to the top-level texture descriptor
    txtr = surfData->pTxtrDesc;
    lms = txtr->txLmsMax;

    // fill in texture's surfaceData structure
    surfLCL->lpGbl->dwReserved1 = (DWORD)surfData;
    surfData->dwFlags          |= FXSURFACE_IS_BROADCAST;
    surfData->dwPixelFormat     = DDPF_RGB;
    surfData->dwBytesPerPixel   = txtr->txBpt >> 3;
    surfData->inFlipChain       = FALSE;
    surfData->surfaceLevel      = _FF(ddCurrentSurfaceLevel);
    surfData->dwWidth           = txtr->txWidth;
    surfData->dwHeight          = txtr->txHeight;
    surfData->dwSurfLclFlags    = surfLCL->dwFlags;
    surfData->dwColorSpaceLowValue = surfLCL->ddckCKSrcBlt.dwColorSpaceLowValue;
    surfData->dwColorSpaceHighValue = surfLCL->ddckCKSrcBlt.dwColorSpaceHighValue;

    // video surface addresses are stored as offsets for our DDraw driver
    surfData->hwPtr = txtr->txMipInfo[lms].mipHwStart - _FF(LFBBASE);
    if (TXTR_IS_NLVM(txtr))
        // agp surfaces use an absolute system memory address
        surfData->hwPtr = txtr->txMipInfo[lms].mipHwStart;
    surfData->lfbPtr    = txtr->txMipInfo[lms].mipVaStart;
    surfData->endlfbPtr = txtr->txMipInfo[lms].mipVaStart + txtr->txTotalSize;

    if (txtr->txAllocMode == HWC_TEXTURE_TILED) {
        if (txtr->txMipInfo[lms].mipTileMode)
            surfData->tileFlag = MEM_IN_TILE1;
        else
            surfData->tileFlag = MEM_IN_TILE0;

        // pstride = tiles; mstride = tiles * tile width
        surfData->dwPStride = txtr->txMipInfo[lms].mipStride / SST_MC_MICRO_TILE_WIDTH;
        surfData->dwStride  = txtr->txMipInfo[lms].mipStride;
        surfData->dwL2MStride = 0;
        surfData->dwMStride   = 1;

        while ( surfData->dwMStride < surfData->dwPStride ) {
            surfData->dwMStride <<= 1;
            surfData->dwL2MStride++;
        }
    }
    else
        surfData->tileFlag = MEM_IN_LINEAR;

    // set up to return data to DDraw
    tlod = txtr->txFormatFlags & TEXFMTFLG_NPT ? 0 : txtr->txLmsMax;
    next = surfLCL;

    // return pitch & vidmem pointer at every level or DDraw will allocate memory
    while (next != NULL) {

        next->lpGbl->lPitch      = txtr->txMipInfo[tlod].mipStride;
        next->lpGbl->fpVidMem    = txtr->txMipInfo[tlod].mipVaStart;
        next->lpGbl->dwReserved1 = (DWORD)surfData;

        if ((next->lpAttachList != NULL) && 
            (next->lpAttachList->lpAttached != NULL)) {
            next = next->lpAttachList->lpAttached;
            tlod--;
        }
        else
            next = NULL;
    }

    if (surfLCL->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) {
        DDRAWI_DDRAWSURFACE_GBL_MORE *psurf_gbl_more;
        psurf_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(surfLCL->lpGbl);
        psurf_gbl_more->fpPhysicalVidMem = txtr->txVaGartLinStart;
    }

    // exit success
    return *retCode = ddec, DDHAL_DRIVER_HANDLED;

unwind2:
    txtrSurfDataTerminate(ppdev, surfData);
unwind1:
    return *retCode = ddec, ddrc;
}

//---------------------------------------------------------------------------------------
//
// txtrSurfaceCreateEx()
//
// Description:
//
// Side effects: 
//
// Algorithm:
// 
//---------------------------------------------------------------------------------------

FxU32 txtrSurfaceCreateEx(                  //
    NT9XDEVICEDATA             *ppdev,      // IN
    LPDDRAWI_DDRAWSURFACE_LCL   pDDSLcl,    // IN
    TXHNDLSTRUCT               *thstruct,   // IN
    txtrDesc                  **txDesc)     // OUT
{
    FxU32 numMipmaps, txSlog, txTlog, txNpt, txWidth, txHeight, rc;
    int mipLevel;
    FXSURFACEDATA *surfData;
    txtrDesc *txtr;
    FxU32 txBpt = pDDSLcl->lpGbl->ddpfSurface.dwRGBBitCount;

    // if this is a system memory (non-agp) texture
    if (pDDSLcl->lpGbl->dwReserved1 == 0) {
        LPDDRAWI_DDRAWSURFACE_LCL next;

        // create FXSURFACEDATA struct which defines this surface for the driver
        rc = txtrSurfDataAlloc(ppdev, &surfData, 0, 0, txBpt);
        if (rc)
            return FALSE;

        // store FXSURFACEDATA ptr in the globally reachable tx handle struct
        thstruct->surfData = surfData;
        thstruct->surfData->dwSurfLclFlags = pDDSLcl->dwFlags;
        thstruct->surfData->ddsDwCaps      = pDDSLcl->ddsCaps.dwCaps;
        thstruct->surfData->ddsDwCaps2     = pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2;

        txWidth  = pDDSLcl->lpGbl->wWidth;
        txHeight = pDDSLcl->lpGbl->wHeight;
        txNpt    = txtrCalcTextureSize(txWidth, txHeight, &txSlog, &txTlog);

        mipLevel = txSlog > txTlog ? txSlog : txTlog;
        numMipmaps = pDDSLcl->lpSurfMore->dwMipMapCount;
        numMipmaps = numMipmaps == 0 ? 1 : numMipmaps;

        txtr = SURFDATA_GET_TXTRDESC(surfData);
        txtr->txLmsMax = mipLevel;
        txtr->txLmsMin = mipLevel - (numMipmaps - 1);
        txtr->txFormatFlags |= txNpt ? TEXFMTFLG_NPT : 0;

        if (txNpt)
            mipLevel = 0;

        // loop the mip levels and store info to describe a system mem texture
        next = pDDSLcl;
        while (next != NULL) {
            txtr->txMipInfo[mipLevel].mipHwStart = next->lpGbl->fpVidMem;
            txtr->txMipInfo[mipLevel].mipStride  = next->lpGbl->lPitch;
            txtr->txMipInfo[mipLevel].mipWidth   = next->lpGbl->wWidth;
            txtr->txMipInfo[mipLevel].mipHeight  = next->lpGbl->wHeight;
            mipLevel--;

                if (txNpt)
                    next = NULL;
                else if ((next->lpAttachList != NULL) && (next->lpAttachList->lpAttached != NULL))
                    next = next->lpAttachList->lpAttached;
                else
                    next = NULL;
        }
    }
    else
        txtr = SURFDATA_TO_TXTRDESC(pDDSLcl->lpGbl->dwReserved1);

    return TRUE;
}

//---------------------------------------------------------------------------------------
//
// txtrSurfaceDelete()
//
// Description: Single entry point for deleting texture surfaces. Destructor for
//  texture surfaces.
//
// Side effects:
//  frees frame buffer and virtual memory (if any) allocated to this texture
//  frees driver-specific data structures (FXSURFACEDATA, txtrDesc) for this texture
//
// Algorithm:
//  if this surface is NOT the largest mip level for this texture, return
//  if this is a system memory surface
//      delete driver-specific data structures for this texture surface
//  else
//      free frame buffer and virtual memory (if any) allocated to this texture
//      delete driver-specific data structures for this texture surface
//
// NOTE: This routine only frees the texture surface and all its accompanying memory
// and data structures when it receives the request to delete the TOP-LEVEL direct
// draw surface that represents the largest mip level in the mip chain. Otherwise it
// ignores the request. So far direct draw always deletes all attached surfaces at
// the same time, with the largest surface last, so this works.
//
//---------------------------------------------------------------------------------------

void __stdcall txtrSurfaceDelete(           // delete a texture surface
    NT9XDEVICEDATA             *ppdev,      // IN: global driver data
    LPDDRAWI_DDRAWSURFACE_LCL   surfLCL)    // IN: direct draw surface local struct
{
    FXSURFACEDATA *surfData;
    txtrDesc *txtr;

    if (surfLCL->lpGbl->dwReserved1 == 0)
        return;

    surfData = (FXSURFACEDATA *) surfLCL->lpGbl->dwReserved1;
    txtr = surfData->pTxtrDesc;

    // if we are not deleting the top-level map, return
    if ((surfLCL->lpGbl->wWidth  != txtr->txWidth) ||
        (surfLCL->lpGbl->wHeight != txtr->txHeight))
        return;

    if (surfLCL->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) {
        // free local data structure memory, the FXSURFACEDATA and txtrDesc(s)
        txtrSurfDataTerminate(ppdev, surfData);
    }
    else {
        while (txtr != NULL) {
            // free frame buffer and virtual memory allocated with the texture
            txtrFreeMemory(ppdev, surfLCL, txtr);
            txtr = txtr->txtrDescNext;
        }

        if (surfLCL->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) {
            DDRAWI_DDRAWSURFACE_GBL_MORE *psurf_gbl_more;

            psurf_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(surfLCL->lpGbl);
            psurf_gbl_more->fpPhysicalVidMem = 0L;
        }

        // free local data structure memory, the FXSURFACEDATA and txtrDesc(s)
        txtrSurfDataTerminate(ppdev, surfData);
    }

    surfLCL->lpGbl->dwReserved1 = 0;
}

//---------------------------------------------------------------------------------------
//
// txtrLock()
//
// Description: Entry point for Direct Draw lock of a texture surface.
//
// Side effects:
//  allocates and programs a CAM entry
//
// Algorithm:
//  check for errors
//  retrieve texture width and height from direct draw
//  call txtrAllocLinearAddr() to allocate and program cam entry
//  issue mops and idles the hardware to ensure prior rendering is complete
//  return texture virtual address to direct draw
//
//---------------------------------------------------------------------------------------

void __stdcall txtrLock(                    // lock a texture surface
    NT9XDEVICEDATA             *ppdev,      // IN: global driver data
    LPDDHAL_LOCKDATA            pld)        // IN: direct draw lock data
{
    FXSURFACEDATA *surfData;
    txtrDesc *txtr;
    FxU8 *appAddr;
    FxU32 width, height;

    CMDFIFO_PROLOG(cmdFifo);

    // check for null vidmem pointer, null surface data
    if (((LPVOID)pld->lpDDSurface->lpGbl->fpVidMem == NULL) ||
        (pld->lpDDSurface->lpGbl->dwReserved1 == 0)) {
        pld->ddRVal = DDERR_UNSUPPORTED;
        return;
    }

    surfData = (FXSURFACEDATA *) pld->lpDDSurface->lpGbl->dwReserved1;
    txtr = surfData->pTxtrDesc;

    width  = pld->lpDDSurface->lpGbl->wWidth;
    height = pld->lpDDSurface->lpGbl->wHeight;

    // set up a cam entry and retrieve the linear address
    appAddr = txtrAllocLinearAddr(ppdev, txtr, width, height);

    // flush the texture cache, stall 3d and idle the hardware
    CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE * 2 );
    SETMOP( cmdFifo, SST_MOP_STALL_2D);
    SETMOP( cmdFifo, SST_MOP_STALL_3D | 
                     (SST_MOP_STALL_3D_TD << SST_MOP_STALL_3D_SEL_SHIFT) |
                     SST_MOP_FLUSH_TCACHE);
    CMDFIFO_EPILOG( cmdFifo );
    while (FXGETBUSYSTATUS(ppdev));

    // return virtual addresses to DDraw
    pld->lpSurfData = (void *) appAddr;
    pld->lpDDSurface->lpGbl->fpVidMem = (unsigned long) pld->lpSurfData; // TEMP: ?
    pld->ddRVal = DD_OK;
}

//---------------------------------------------------------------------------------------
//
// txtrUnlock()
//
// Description: Entry point for direct draw unlock for a texture surface.
//
// Side effects:
//  Frees a previously allocated cam entry for this surface
//
// Algorithm:
//  check for errors
//  if this is a UVL surface, call special routine to handle this format
//  if there is a linear (virtual) address allocated
//      call txtrFreeLinearAddr to free the cam entry
//
//---------------------------------------------------------------------------------------

void __stdcall txtrUnlock(                  // unlock a texture surface
    NT9XDEVICEDATA             *ppdev,      // IN: global driver data
    LPDDHAL_UNLOCKDATA          puld)       // IN: direct draw unlock data
{
    txtrDesc *txtr;
    FXSURFACEDATA *surfData;
    FxU32 width  = puld->lpDDSurface->lpGbl->wWidth;
    FxU32 height = puld->lpDDSurface->lpGbl->wHeight;

    if (puld->lpDDSurface->lpGbl->dwReserved1 == 0)
        return;

    surfData = (FXSURFACEDATA*)(puld->lpDDSurface->lpGbl->dwReserved1);
    txtr = surfData->pTxtrDesc;

#ifdef CUBEMAP
    if (txtr->txFormatFlags & TEXFMTFLG_CUBEMAP)
    {
        // Mark face as dirty. Could refine to only mark the mip level
        // for the face that was touched.
        txtr->txDirtyBits |= surfData->ddsDwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES;
    }
#endif

    // check for special case texture surface
    if (txtr->txFormatFlags & (TEXFMTFLG_UVL | TEXFMTFLG_UV))
        txtrUnmungeUVLSurface (ppdev,
                               txtr,
                               (FxU8*)puld->lpDDSurface->lpGbl->fpVidMem,
                               width, height,
                               puld->lpDDSurface->lpGbl->lPitch);

    // free cam entry/linear address if we have one
    if (txtr->txVaStart)
        txtrFreeLinearAddr(ppdev, txtr, width, height);
}

//---------------------------------------------------------------------------------------
//
// txtrLoad()
//
// Description: Entry point to load a single mip level from DdBlt for a texture surface
//
// Side effects:
//  copies texture data to the frame buffer from system memory
//
// Algorithm:
//  check for errors
//  compute the slog and tlog for the texture
//  if npt texture, call npt-specific download routine txtrDownloadNptHblt
//  if AGP texture, call agp-specific download routine
//  else download the level using host blt by calling txtrDownloadLevelHblt
//
//---------------------------------------------------------------------------------------

FxU32 __stdcall txtrLoad(                   // load a texture (entry point from DdBlt)
    NT9XDEVICEDATA             *ppdev,      // IN: global driver data
    LPDDRAWI_DDRAWSURFACE_LCL   srcLCL,     // IN: direct draw surface local for source
    RECTL                      *prSrc,      // IN: rect describing source data to copy
    LPDDRAWI_DDRAWSURFACE_LCL   dstLCL,     // IN: direct draw surface local for dest
    RECTL                      *prDst)      // IN: destination point of load
{
    txtrDesc *txtr;
    FXSURFACEDATA *surfData;
    FxU32 slog, tlog, lms, w, h, npt;
    FxU32 *pData, rc=DD_OK, stride;
    POINT DstPt;

    if (dstLCL->lpGbl->dwReserved1 == 0) {
        rc = DDERR_GENERIC; // TEMP: what to return?
        goto unwind1;
    }

    // retrieve texture handle as supplied by DDraw
    surfData = (FXSURFACEDATA*) dstLCL->lpGbl->dwReserved1;
    txtr = surfData->pTxtrDesc;

    w = srcLCL->lpGbl->wWidth;
    h = srcLCL->lpGbl->wHeight;

    // determine which level is being downloaded
    npt = txtrCalcTextureSize(w, h, &slog, &tlog);
    lms = npt ? 0 : slog >= tlog ? slog : tlog;

    // obtain data from source surface
    pData  = (unsigned long *) srcLCL->lpGbl->fpVidMem;
    stride = srcLCL->lpGbl->lPitch;

    if (txtr->txAllocMode == HWC_TEXTURE_BLOCK_LINEAR) {
        rc = txtrDownloadLevelDXT(ppdev, txtr, lms, w, h, pData, stride);
    }
    else {
        // download the level using host blt
        DstPt.x = prDst->left; // convert rect to point
        DstPt.y = prDst->top;
        rc = txtrDownloadLevelHblt(ppdev, txtr, lms, prSrc, &DstPt, pData, stride);
    }
unwind1:
    return rc; // TEMP: what should we return?
}

//---------------------------------------------------------------------------------------
//
// txtrLoadDp2()
//
// Description: Entry point for texture download from ddiDrawPrimitives2 when the
//  TEXBLT token is received.
//
// Side effects:
//  Copies all common mip levels between the direct draw system-memory surface and
//  the driver's texture surface
//
// Algorithm:
//  (no error checks are made since ddiDrawPrimitves2 is a trusted caller)
//  determine the smallest mip level in common between source and dest
//  for each level that is present in the source and destination
//      call txtrDownloadLevelHblt to download the given level
//
//---------------------------------------------------------------------------------------

DWORD __stdcall txtrLoadDp2(
    NT9XDEVICEDATA             *ppdev,      // IN: global driver data
    txtrDesc                   *ptxSrc,     // IN: source texture descriptor
    RECTL                      *rSrc,       // IN: subrectangle to load
    txtrDesc                   *ptxDst,     // IN: destination texture descriptor
    POINT                      *pDst)       // IN: destination point to load
{
    int lms, lmsMin;
    FxU32 *pData, stride, w, h;

    // choose the largest of the two minimum levels
    if (ptxSrc->txLmsMin >= ptxDst->txLmsMin)
        lmsMin = ptxSrc->txLmsMin;
    else
        lmsMin = ptxDst->txLmsMin;

    // download all levels that are present in both textures
    for (lms = ptxDst->txLmsMax; lms >= lmsMin; lms--) {

        pData  = (FxU32*) ptxSrc->txMipInfo[lms].mipHwStart;
        stride = ptxSrc->txMipInfo[lms].mipStride;
        w      = ptxSrc->txMipInfo[lms].mipWidth;
        h      = ptxSrc->txMipInfo[lms].mipHeight;

        if (ptxDst->txAllocMode == HWC_TEXTURE_BLOCK_LINEAR)
        {
            txtrDownloadLevelDXT(ppdev, ptxDst, lms, w, h, pData, stride);
        }
        else
        {
            txtrDownloadLevelHblt(ppdev, ptxDst, lms, rSrc, pDst, pData, stride);
        }

        if (ptxDst->txFormatFlags & (TEXFMTFLG_UVL | TEXFMTFLG_UV))
        {
            txtrUnmungeUVLSurface(ppdev, ptxDst, (FxU8*)pData, w, h, stride);
        }

        // adjust subrectangles by half
        rSrc->top >>= 1; rSrc->left >>= 1;
        pDst->x   >>= 1; pDst->y    >>= 1;
        rSrc->bottom = (rSrc->bottom + 1) >> 1;
        rSrc->right  = (rSrc->right  + 1) >> 1;
        rSrc->bottom += (rSrc->bottom - rSrc->top) > 1 ? 0 : 1;
        rSrc->right  += (rSrc->right - rSrc->left) > 1 ? 0 : 1;
    }
    return 0;
}
