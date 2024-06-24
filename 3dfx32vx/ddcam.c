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
*/

#include <ddrawi.h>
#include "ddglobal.h"
#include "ddcam.h"                      // self
#include "d3txtr.h"
#include "fifomgr.h"

#ifdef CSERVICE
#pragma message("RYAN@BUG, shouldn't 'camMgr' be part of GLOBALDATA?")
#endif
static CamMgr_t camMgr;                 // this is THE cam manager object

// local CAM management defines
#define START_CAM_RESERVE           0
#define DIB_ENGINE_CAM_ENTRY        (_FF(EndCamEntry) - 1)
#define END_CAM_RESERVE             (_FF(EndCamEntry))
#define NUM_RESERVED_CAM_ENTRIES    (END_CAM_RESERVE-START_CAM_RESERVE+1)

// first entry available for general use
#define START_CAM_ENTRY_POOL        (END_CAM_RESERVE+1)
#define END_CAM_ENTRY_POOL          (SST_CAM_SIZE-1)
 
#define CAM_ENTRY_FREE  0
#define CAM_ENTRY_USED  1

/*----------------------------------------------------------------------
Function name:  FxCamInit

Description:    Initialize CAM object

Return:         none
----------------------------------------------------------------------*/

void FxCamInit(
    CamMgr_t      **ppCamMgr,
    NT9XDEVICEDATA *ppdev)
{
    FxU32           numCAMEntry;
    FxU32           lastCAMaddress;

    // attach global pointer to the real cam manager object
    *ppCamMgr = &camMgr;

    // init CAM usage table
    lastCAMaddress = 0;   // init for greatest address in CAM already alloc'd by display driver
    for (numCAMEntry = START_CAM_RESERVE ; numCAMEntry <= END_CAM_RESERVE; numCAMEntry++)
    {
        camMgr.CAMEntries[numCAMEntry] = CAM_ENTRY_USED;
        // call phantom MM to reserve space used by reserved entr[y/ies], if that space is
        // within the region phantom manages.  As of 7/28, the desktop was before the
        // linear heap and the phantom only managed addresses within the linear heap range
        if (_DD(sst2CRegs)->cam[numCAMEntry].endAddress > lastCAMaddress)
        {
            // this entry at higher address than last, so reset upper address limit
            lastCAMaddress = _DD(sst2CRegs)->cam[numCAMEntry].endAddress;
        }
    }

    // note that CAM endAddress is actually the location following the end (CAM hit if < endAddress)
    lastCAMaddress += _FF(LFBBASE); // CAM addresses are offset only, so add to base

    // make sure memlist exists, then check if need to adjust free space
    if (_DD(phantom_memFree) && (_DD(phantom_memFree)->dwStartAddress < lastCAMaddress))
    {
        // Display driver CAM space overlaps DDraw's phantom region, so adjust start and size
        FxU32 newStart = (lastCAMaddress+0x3)&~0x3;// DWORD align
        _DD(phantom_memFree)->dwMemSize -= newStart - _DD(phantom_memFree)->dwStartAddress;
        _DD(phantom_memFree)->dwStartAddress = newStart;
    }

    for (numCAMEntry = START_CAM_ENTRY_POOL ; numCAMEntry <= END_CAM_ENTRY_POOL; numCAMEntry++)
    {
        // clear the cam entry 
        FxResetCamEntry(&camMgr,ppdev,numCAMEntry);
        camMgr.CAMEntries[numCAMEntry] = CAM_ENTRY_FREE;
    }

    camMgr.camLfbBase = _FF(LFBBASE);
}

/*----------------------------------------------------------------------
Function name:  camMgrAllocEntry

Description:    allocate a cam entry AND/OR virtual memory from phantom mgr

Return:         CAM_SUCCESS        : the request succeeded
                CAM_ERROR_MEMALLOC : virtual memory allocation failed
                
----------------------------------------------------------------------*/

FxU32 camMgrAllocEntry(                     // allocate cam entry AND/OR virtual mem
    CamMgr_t       *camMgr,                 // self
    GLOBALDATA     *ppdev,                  // needed by lower level routines
    FxU32           camFlags,               // allocation options, including tile mode
    FxU32           useEntry,               // which cam entry to use, if any
    FxU32           useAddr,                // virtual address requested for this entry
    FxU32           hwAddr,                 // hardware address of data
    FxU32           memSize,                // memory in bytes to allocate
    FxU32           strideBytes,            // stride in bytes of surface
    FxU32           pixDepth,               // pixel depth of surface
    FxU32           lfbDepth,               // lfb depth of surface
    FxU32          *lfbAddr)                // OUT: allocated lfb addr into surface
{
    FxU32           rc=CAM_SUCCESS, camState=0, tileMode, mStride, pStride;
    FxU32           addr;
    Sst2CAMEntry    CAMEntry;

    // DEBUG: check for illegal flag combinations

    // allocate virtual address space for surface if requested
    if (camFlags & CAM_ALLOC_PHANTOM_MEM)
    {
        phantom_allocSurface(ppdev, memSize, &addr);
        if (!addr)
        {
            rc = CAM_ERROR_MEMALLOC;
            goto unwind1;
        }
    }
    else
        addr = useAddr;

    // if allocation of cam entry not requested, exit success
    if (!(camFlags & CAM_ALLOC_CAM_ENTRY))
        goto exit_success;

    CAMEntry.baseAddress  = addr   - camMgr->camLfbBase;
    CAMEntry.endAddress   = addr   - camMgr->camLfbBase + memSize;
    CAMEntry.physicalBase = hwAddr - camMgr->camLfbBase;

    // ensure 32-byte alignment of base and end addresses
    if (CAMEntry.baseAddress & 0x1f)
        CAMEntry.baseAddress &= 0xffffffe0;
    if (CAMEntry.endAddress  & 0x1f)
        CAMEntry.endAddress = (CAMEntry.endAddress + 0x20) & 0xffffffe0;

    if (camFlags & CAM_BLOCK_LINEAR)    // assume a certain access method ... ?
    {
        CAMEntry.physicalBase |= SST_CAM_EN_PURE_LINEAR;
        CAMEntry.strideState = (SST_MC_PIXDEPTH_32BPP << SST_CAM_LFB_DEPTH_SHIFT) |
                               (SST_MC_PIXDEPTH_32BPP << SST_CAM_PIXEL_DEPTH_SHIFT) |
                                SST_CAM_LINEAR | SST_CAM_TILE_MODE1;
    }
    else if (camFlags & CAM_PURE_LINEAR)
    {
        CAMEntry.physicalBase |= SST_CAM_EN_PURE_LINEAR;

        switch (pixDepth)
        {
        case 1:
            camState |= SST_MC_PIXDEPTH_8BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            break;
        case 2:
            camState |= SST_MC_PIXDEPTH_16BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            break;
        case 4:
            camState |= SST_MC_PIXDEPTH_32BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            break;
        default:
            rc = CAM_ERROR_ILLEGAL_FLAGS;
            goto unwind2;
        }

        switch (lfbDepth)
        {
        case 1:
            camState |= SST_MC_PIXDEPTH_8BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        case 2:
            camState |= SST_MC_PIXDEPTH_16BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        case 4:
            camState |= SST_MC_PIXDEPTH_32BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        default:
            rc = CAM_ERROR_ILLEGAL_FLAGS;
            goto unwind2;
        }

        // mstride must be a power of two for linear and tiled surfaces
        for (mStride = 1UL; 1UL << mStride < strideBytes; mStride++);
        // pstride is in bytes for linear surfaces
        camState |= (mStride << SST_CAM_MSTRIDE_SHIFT) | 
                    (strideBytes << SST_CAM_PSTRIDE_SHIFT);
        camState |= SST_CAM_LINEAR;        
        CAMEntry.strideState = camState;
    }
    else if (camFlags & CAM_TILED)
    {
        switch (pixDepth)
        {
        case 1:
            camState |= SST_MC_PIXDEPTH_8BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            break;
        case 2:
            camState |= SST_MC_PIXDEPTH_16BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            break;
        case 4:
            camState |= SST_MC_PIXDEPTH_32BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            break;
        default:
            rc = CAM_ERROR_ILLEGAL_FLAGS;
            goto unwind2;
        }

        switch (lfbDepth)
        {
        case 1:
            camState |= SST_MC_PIXDEPTH_8BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        case 2:
            camState |= SST_MC_PIXDEPTH_16BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        case 4:
            camState |= SST_MC_PIXDEPTH_32BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        default:
            rc = CAM_ERROR_ILLEGAL_FLAGS;
            goto unwind2;
        }

        tileMode = camFlags & CAM_TILE_MODE_1 ? 1 : 0;
        camState |= tileMode << SST_CAM_TILE_MODE1_SHIFT;
        for (mStride = 1UL; 1UL << mStride < strideBytes; mStride++);
        pStride = strideBytes / 32;             // TEMP: tile width is 32 bytes
        camState |= (mStride << SST_CAM_MSTRIDE_SHIFT) | (pStride << SST_CAM_PSTRIDE_SHIFT);
        CAMEntry.strideState = camState;
    }

    // check stride state flags
    if (camFlags & CAM_EN_STAGGER)
        CAMEntry.strideState |= SST_CAM_EN_STAGGER;
    if (camFlags & CAM_EN_YUV)
        CAMEntry.strideState |= SST_CAM_EN_YUV;
    if (camFlags & CAM_EN_AA)
        CAMEntry.strideState |= SST_CAM_EN_AA;
    if (camFlags & CAM_EN_PIXEL_PROCESSING)
        CAMEntry.strideState |= SST_CAM_EN_PIXEL_PROCESSING;
    if (camFlags & CAM_EN_DEPTH)
        CAMEntry.strideState |= SST_CAM_EN_DEPTH;
    if (camFlags & CAM_EN_DISTRIBUTED)
        CAMEntry.strideState |= SST_CAM_EN_DISTRIBUTED;

    // check base address flags
    if (camFlags & CAM_EN_BYTE_SWIZZLE)
        CAMEntry.physicalBase |= SST_CAM_EN_BYTE_SWIZZLE;
    if (camFlags & CAM_EN_WORD_SWIZZLE)
        CAMEntry.physicalBase |= SST_CAM_EN_WORD_SWIZZLE;

    // find an available CAM entry and program it
    rc = FxCamProgram(camMgr, ppdev, &CAMEntry, useEntry);

    if (!rc)
    {
        rc = CAM_ERROR_NO_CAM_ENTRIES;
        goto unwind2;
    }
    else
        rc = CAM_SUCCESS;

exit_success:
    return *lfbAddr = addr, rc;

unwind2:
    phantom_freeSurface(ppdev, addr);           // undo memory allocation on error

unwind1:
    return rc;
}

/*----------------------------------------------------------------------
Function name:  camMgrFreeEntry

Description:    Free a specified CAM entry AND/OR phantom memory

Return:         none
----------------------------------------------------------------------*/

FxU32 camMgrFreeEntry(                          // free a previously allocated cam entry
    CamMgr_t       *camMgr,                     // self
    NT9XDEVICEDATA *ppdev,                      // device data needed by phantom
    FxU32           camFlags,                   // deallocation options
    FxU32           lfbAddr)                    // virtual address of surface to free
{
    FxU32 tmp, rc = CAM_SUCCESS;

    if (camFlags & CAM_FREE_PHANTOM_MEM)
        phantom_freeSurface(ppdev, lfbAddr);

    if (camFlags & CAM_FREE_CAM_ENTRY)
    {
        tmp = FxCamFreeEntry(camMgr, ppdev, lfbAddr - camMgr->camLfbBase);

        if (tmp != TRUE)
            rc = CAM_ERROR_ENTRY_NOT_FOUND;
    }

    return rc;
}

/*----------------------------------------------------------------------
Function name:  FxResetCamEntry

Description:    write the specified CAM entry registers to all 0

Return:         none
----------------------------------------------------------------------*/

void FxResetCamEntry(
    CamMgr_t       *camMgr,
    NT9XDEVICEDATA *ppdev,
    FxU32           entry)
{
    SETDW( _DD(sst2CRegs)->cam[entry].baseAddress, (FxU32)NULL );
    SETDW( _DD(sst2CRegs)->cam[entry].endAddress,  (FxU32)NULL );
    SETDW( _DD(sst2CRegs)->cam[entry].physicalBase,(FxU32)NULL );   
    SETDW( _DD(sst2CRegs)->cam[entry].strideState, (FxU32)NULL );
}

/*----------------------------------------------------------------------
Function name:  FxCamReserveEntry

Description:    search CAM management table for free entry.  If found, reserve it

Return:         table index of reserved entry, or NULL if reserve failed
----------------------------------------------------------------------*/

FxU32 FxCamReserveEntry(
    CamMgr_t       *camMgr,
    NT9XDEVICEDATA *ppdev,
    FxU32           baseAddr)
{
    FxU32           entry;    

    for (entry = START_CAM_ENTRY_POOL ; entry <= END_CAM_ENTRY_POOL; entry++)
    {
        // if available, take it
        if (camMgr->CAMEntries[entry] == CAM_ENTRY_FREE) 
        {
            // save address in CAM pool table - nonzero will reserve the entry
            camMgr->CAMEntries[entry] = baseAddr;
            return (entry);
        } 
    }
    return ((FxU32)NULL);
}

/*----------------------------------------------------------------------
Function name:  FxCamFreeEntry

Description:    Free CAM entry corresponding to given base address, and reset HW CAM entry

Return:         TRUE if entry found and freed, FALSE otherwise
----------------------------------------------------------------------*/

FxBool FxCamFreeEntry(
    CamMgr_t       *camMgr,
    NT9XDEVICEDATA *ppdev,
    FxU32           baseAddr)
{
    FxU32           entry;    
    FxBool          found=FALSE;

    for (entry = START_CAM_ENTRY_POOL ; !found && (entry <= END_CAM_ENTRY_POOL); entry++)
    {
        // if address in CAM pool table - this is the entry to free
        if (camMgr->CAMEntries[entry] == baseAddr) 
        {
            found=TRUE;
            break;
        } 
    }

    if (found)
    {
        // set hw to known benign state 
        FxResetCamEntry(camMgr, ppdev, entry);
        camMgr->CAMEntries[entry] = CAM_ENTRY_FREE;
    }

    return (found);
}

/*----------------------------------------------------------------------
Function name:  FxCamProgram

Description:    Find a free CAM entry, reserve it, and program it with given data.
                If useEntry is a legal cam entry, use that entry instead of finding
                a free entry.

Return:         If successful, Base address programmed into CAM entry; NULL if fails
----------------------------------------------------------------------*/

FxU32 FxCamProgram(
    CamMgr_t       *camMgr,
    NT9XDEVICEDATA *ppdev,
    Sst2CAMEntry   *lpCAMEntry,
    FxU32           useEntry)
{
    FxU32           entry;

    // if a specific entry is requested
    if (useEntry != CAM_ENTRY_ANY)
    {
        entry = useEntry;

        // if the entry requested is not use, claim it
        if (camMgr->CAMEntries[entry] == CAM_ENTRY_FREE) 
            camMgr->CAMEntries[entry] = lpCAMEntry->baseAddress;
        // else flag an error
        else
            __asm int 3;
    }
    else
        entry = FxCamReserveEntry(camMgr, ppdev, lpCAMEntry->baseAddress);

    if ((entry >= START_CAM_RESERVE) && (entry <= END_CAM_ENTRY_POOL))
    {
        SETDW( _DD(sst2CRegs)->cam[entry].baseAddress, lpCAMEntry->baseAddress );
        SETDW( _DD(sst2CRegs)->cam[entry].endAddress,  lpCAMEntry->endAddress );
        SETDW( _DD(sst2CRegs)->cam[entry].physicalBase,lpCAMEntry->physicalBase );   
        SETDW( _DD(sst2CRegs)->cam[entry].strideState, lpCAMEntry->strideState );
    }
    else
        __asm int 3;

    return lpCAMEntry->baseAddress;
}

//---------------------------------------------------------------------------------------
//
// txtrComputeCAMEntry
//
//---------------------------------------------------------------------------------------

void FxCamComputeTxtrEntry(                 // compute cam entry for textures
    CamMgr_t       *camMgr,                 // self
    txtrDesc       *txtr,                   // texture struct
    FxU32           level,                  // which level 
    FxU32           levelSize,              // size of current level
    FxU32           levelAddr,              // hardware address of current level
    FxU32           lfbAddr,                // lfb address of current level
    FxU32           lfbBase,                // base of cam lfb space
    Sst2CAMEntry   *CAMEntry)               // OUT: returned cam entry
{
    FxU32           mStride=0, pStride=0, camState=0;
    txMipInfo      *txMipInfo = &txtr->txMipInfo[level];

    if (TEXTURE_IS_DXT(txtr->txFormat >> SST_TA_FORMAT_SHIFT) ||
        TEXTURE_IS_FXT(txtr->txFormat >> SST_TA_FORMAT_SHIFT))
    {
        CAMEntry->baseAddress  = lfbAddr - lfbBase;
        CAMEntry->endAddress   = lfbAddr - lfbBase + levelSize;
        CAMEntry->physicalBase = (levelAddr - lfbBase) | SST_CAM_EN_PURE_LINEAR;
        CAMEntry->strideState  = 
            (SST_MC_PIXDEPTH_32BPP << SST_CAM_LFB_DEPTH_SHIFT) |
            (SST_MC_PIXDEPTH_32BPP << SST_CAM_PIXEL_DEPTH_SHIFT) |
            SST_CAM_LINEAR |
            SST_CAM_TILE_MODE1;
    }
    else // any other format tiled power-of-two texture
    {
        CAMEntry->baseAddress  = lfbAddr - lfbBase;
        CAMEntry->endAddress   = lfbAddr - lfbBase + levelSize;
        CAMEntry->physicalBase = levelAddr - lfbBase;

        switch (txtr->txBpt)
        {
        case 0:
            break;                              // TEMP
        case 1:
            camState |= SST_MC_PIXDEPTH_8BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            camState |= SST_MC_PIXDEPTH_8BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        case 2:
            camState |= SST_MC_PIXDEPTH_16BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            camState |= SST_MC_PIXDEPTH_16BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        case 4:
            camState |= SST_MC_PIXDEPTH_32BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
            camState |= SST_MC_PIXDEPTH_32BPP << SST_CAM_LFB_DEPTH_SHIFT;
            break;
        }

        camState |= txMipInfo->mipTileMode << SST_CAM_TILE_MODE1_SHIFT;

        for (mStride = 1;
            1 << mStride < (FxI32) txMipInfo->mipHuTiles * 32; // TEMP: 32=tile width
            mStride++);

        pStride = txMipInfo->mipHuTiles;

        camState |= (mStride << SST_CAM_MSTRIDE_SHIFT) |
                    (pStride << SST_CAM_PSTRIDE_SHIFT);

        CAMEntry->strideState = camState;
    }
}

//---------------------------------------------------------------------------------------
//
// FxCamComputeNPTTxtrEntry
//
//---------------------------------------------------------------------------------------

void FxCamComputeNPTTxtrEntry(                      // compute npt cam entry
    CamMgr_t       *camMgr,                         // self
    txtrDesc       *txtr,                           // texture struct
    FxU32           lfbAddr,                        // virtual address of map
    FxU32           lfbBase,                        // virtual address of texture base
    Sst2CAMEntry   *CAMEntry)                       // OUT: returned cam entry
{
    txMipInfo *mipInfo = &txtr->txMipInfo[0];       // just to look better
    FxU32 mStride, state = 0;

    // note: VIRTUAL (lfb) address range must be power-of-two in stride
    CAMEntry->baseAddress  = lfbAddr - lfbBase;
    CAMEntry->endAddress   = lfbAddr - lfbBase + mipInfo->mipHeight * mipInfo->mipStride;
    CAMEntry->physicalBase = mipInfo->mipHwStart - lfbBase;

    switch (txtr->txBpt)
    {
    case 4:
        // TEMP: not ready for 4-bit textures
        break;
    case 8:
        state |= SST_MC_PIXDEPTH_8BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
        state |= SST_MC_PIXDEPTH_8BPP << SST_CAM_LFB_DEPTH_SHIFT;
        break;
    case 16:
        state |= SST_MC_PIXDEPTH_16BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
        state |= SST_MC_PIXDEPTH_16BPP << SST_CAM_LFB_DEPTH_SHIFT;
        break;
    case 32:
        state |= SST_MC_PIXDEPTH_32BPP << SST_CAM_PIXEL_DEPTH_SHIFT;
        state |= SST_MC_PIXDEPTH_32BPP << SST_CAM_LFB_DEPTH_SHIFT;
        break;
    }

    state |= mipInfo->mipTileMode << SST_CAM_TILE_MODE1_SHIFT;

    for (mStride = 1;
         1 << mStride < (FxI32) mipInfo->mipHuTiles * 32; // TEMP: 32=tile width
         mStride++);

    CAMEntry->strideState = state                                          |
                            (mStride             << SST_CAM_MSTRIDE_SHIFT) |
                            (mipInfo->mipHuTiles << SST_CAM_PSTRIDE_SHIFT);
}

// for s3tc:
//      s3nptStride = ( hBlocks*texelInfo->minWidth*bitsPerTexel) >> 3;
// for linear npt:
//      mapData->state |= SST_CAM_LINEAR; ? no support for linear
//      mapData->stride = ((txtr->nptWidth * bitsPerTexel + kNPTBitAlignMask) & ~kNPTBitAlignMask) >> 3;
//      nptStride = mapData->stride;
//      mapData->stride = ( mapData->stride >> 4)-1;

FxU32 FxCamComputeSurfaceEntry()
{
    return 0;
}


#ifdef CSERVICE


__declspec(dllexport) FxU32 cs9x_CamAllocator(NT9XDEVICEDATA * ppdev, FxU32 camFlags, FxU32 offset, FxU32 size, FxU32 stride, FxU32 pixDepth, FxU32 lfbDepth, FxU32* plinAddress)
{
  camFlags |= (CAM_ALLOC_PHANTOM_MEM | CAM_ALLOC_CAM_ENTRY);
  offset += _FF(LFBBASE);  //for some silly reason, camMgrAllocEntry wants it this way.

  pixDepth >>= 3;  //convert from bpp to bytes
  lfbDepth >>= 3;  //convert from bpp to bytes
  return (CAM_SUCCESS == camMgrAllocEntry(&camMgr, ppdev, camFlags, CAM_ENTRY_ILLEGAL, 0, offset, size, stride, pixDepth, lfbDepth, plinAddress));
}



__declspec(dllexport) FxU32 cs9x_CamLiberator(NT9XDEVICEDATA * ppdev, FxU32 linAddress)
{
  FxU32 camFlags = CAM_FREE_CAM_ENTRY;

  if (linAddress)
    camFlags |= CAM_FREE_PHANTOM_MEM;

  return (CAM_SUCCESS ==  camMgrFreeEntry(&camMgr, ppdev, camFlags, linAddress));
}


#endif //CSERVICE
