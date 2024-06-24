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
**
*/

#include <ddrawi.h>
#include <d3dhal.h>
#include "d3global.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3txtr2.h"
#include "d3npt.h"                          // self

extern HwcTexelInfo sst2TexelInfo[];

//---------------------------------------------------------------------------------------
//
// txtrCalcNptSize()
//
//---------------------------------------------------------------------------------------

FxBool txtrCalcNptSize(txtrDesc *txtr)
{
    HwcTexelInfo *texelInfo = &sst2TexelInfo[txtr->txFormat >> SST_TA_FORMAT_SHIFT];
    FxU32 vAlign;
    FxBool linearInBlocks = FXFALSE;
    FxI32 blockWidth  = 1 << texelInfo->log2BlockWidth;
    FxI32 blockHeight = 1 << texelInfo->log2BlockHeight;

    // since there's no mipmapping for npt textures, just use lms 0 to store info
    txMipInfo *MIP = &txtr->txMipInfo[0];

    MIP->mipSOffset = 0;
    MIP->mipTOffset = 0;

	switch ( txtr->txFormat )
	{
	case SST_TA_FXT1:
	case SST_TA_DXT1:
	case SST_TA_DXT2:
	case SST_TA_DXT3:
	case SST_TA_DXT4:
	case SST_TA_DXT5:
	    // FXT1 & DXn compressed textures are always block linear
	    linearInBlocks = FXTRUE;
	    break;

    default:
        switch ( txtr->txBpt ) {
        case 8:
            // tiled surface, at 8bbp micro tile = 32x4
            if ( txtr->txAllocMode == HWC_TEXTURE_TILED ) {
                  MIP->mipHuTiles = (MIP->mipWidth  + 31) >> 5;
                  MIP->mipVuTiles = (MIP->mipHeight + 3)  >> 2;
            } 
            break;
        case 16:
            // tiled surface, at 16 bpp micro tile = 16x4
            if ( txtr->txAllocMode == HWC_TEXTURE_TILED ) {
                MIP->mipHuTiles = (MIP->mipWidth  + 15) >> 4;
                MIP->mipVuTiles = (MIP->mipHeight + 3)  >> 2;
            }
            break;
        case 32:
            if ( txtr->txAllocMode == HWC_TEXTURE_TILED ) {
                MIP->mipHuTiles = (MIP->mipWidth  + 7) >> 3;
                MIP->mipVuTiles = (MIP->mipHeight + 3) >> 2;
            }
            break;
        default:
            return FXFALSE;
        }
    break;
    }
    switch ( txtr->txAllocMode ) {
    case HWC_TEXTURE_TILED:
        MIP->mipAlignment = 7; // 128 byte alignment
        if ( MIP->mipVuTiles >= 16 )
            MIP->mipTileMode = 1;
        else
            MIP->mipTileMode = 0;
        // number of micro tiles in vertical direction must be a multiple of 2 for tile mode 0 and 8 for tile mode 1
        vAlign = MIP->mipTileMode == 1 ? 7 : 1;
        MIP->mipVuTiles = ( MIP->mipVuTiles + vAlign ) & ~vAlign;
        txtr->txTotalSize = MIP->mipHuTiles * MIP->mipVuTiles * SST_MC_MICRO_TILE_SIZE;
        MIP->mipStride = MIP->mipHuTiles * SST_MC_MICRO_TILE_WIDTH;
        break;
    case HWC_TEXTURE_LINEAR: 
        // width and height are already calculated
        MIP->mipAlignment = 5; // for now just make them 32-byte aligned
        MIP->mipStride = (((MIP->mipWidth * txtr->txBpt) >> 3) + 31) & ~SST_MASK(5);
        MIP->mipSize = txtr->txTotalSize = MIP->mipStride * MIP->mipHeight;
        break;
    case HWC_TEXTURE_BLOCK_LINEAR:
        // align width and height to the block size
        MIP->mipWidth  = (MIP->mipWidth  + blockWidth  - 1) & ~(blockWidth  - 1);
        MIP->mipHeight = (MIP->mipHeight + blockHeight - 1) & ~(blockHeight - 1);
        MIP->mipAlignment = 5; // alignment for block linear is 32 bytes
        MIP->mipStride = (MIP->mipWidth * txtr->txBpt ) >> 3;
        txtr->txTotalSize = MIP->mipStride * MIP->mipHeight;
        break;
    }
    return FXTRUE;
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadNptLfb : entry point for npt download, uses lfb writes
//
//---------------------------------------------------------------------------------------

void txtrDownloadNptLfb(
    NT9XDEVICEDATA *ppdev,                  // global data
    long            lfbAddr,                // address to download data TO
    long            pitch,                  // pitch of source surface
    txtrDesc       *txtr,                   // texture descriptor structure
    void           *pData)                  // pointer to texture data to download
{
    HwcTexelInfo   *texelInfo = &sst2TexelInfo[txtr->txFormat >> SST_TA_FORMAT_SHIFT];
    txMipInfo      *hwMapInfo = &txtr->txMipInfo[0];
    FxU32           s, t;
    FxU32          *src32, *ptr32;
    FxU16          *src16, *ptr16;
    FxU8           *src8,  *ptr8;
    FxU32           bytesWidePerTile;
    FxU32           surfStrideBytes, txtrDest;
    FxU8           *baseptr;
    FxU32           MStride;

    CMDFIFO_PROLOG(cmdFifo);

    // TEMP: enable the following if the lfb access converted to blt
    // HW_ACCESS_ENTRY(cmdFifo,ACCESS_2D);

    // TEMP: should only need MOP if the downloaded area is in the active texture
    CMDFIFO_CHECKROOM( cmdFifo, 2*MOP_SIZE );
    SETMOP( cmdFifo, SST_MOP_STALL_2D );
    SETMOP( cmdFifo,SST_MOP_STALL_3D | 
            (SST_MOP_STALL_3D_TD << SST_MOP_STALL_3D_SEL_SHIFT) | 
            SST_MOP_FLUSH_TCACHE);

    bytesWidePerTile = 32;

    // calculate graphics stride as log base two of the width
    for (MStride = 1; 1 << MStride < (FxI32)hwMapInfo->mipHuTiles * 32; MStride++);

    surfStrideBytes = 1 << MStride; // LFB view of mem must have pot stride
    baseptr = (FxU8*)pData;

    // TEMP:
    // call routine to flush and disable fifo - until download converted to blt packets
    // BE SURE to uncomment the HW_ACCESS macros near CMDFIFO_PROLOG and CMDFIFO_EPILOG
    //  if fifo packets are used
#ifdef CMDFIFO
    CMDFIFO_EPILOG(cmdFifo);
    {
    extern void FifoEnable(NT9XDEVICEDATA *ppdev, FxBool enable);
    FifoEnable(ppdev,FXFALSE);
    }
#endif

    for (t = 0; t < txtr->txMipInfo[0].mipHeight; t++)
    {
        src8  = (FxU8 *)baseptr;
        src16 = (FxU16*)baseptr;
        src32 = (FxU32*)baseptr;

        for (s = 0; s < txtr->txMipInfo[0].mipWidth; s++)
        {
            txtrDest = lfbAddr + (t * surfStrideBytes) + (s * (txtr->txBpt >> 3));

            // We don't want/need pkt5 download since that's 3D lfb's only
            // Leave as regular LFB's, or convert to BLT packets
            
            switch (txtr->txBpt)
            {
            case 8:
                ptr8   = (FxU8*)txtrDest;
                *ptr8  = *src8++;
                break;
            case 16:
                ptr16  = (FxU16*)txtrDest;
                *ptr16 = *src16++;
                break;
            case 32:
                ptr32  = (FxU32*)txtrDest;
                *ptr32 = *src32++;
                break;
            case 4:
            default:
                // no support for s3tc right now
                __asm { int 3 }
                break;
            }
        }
        baseptr += pitch;
    }

    // TEMP:
    // call routine to re-enable fifo - until download converted to blt packets
    // BE SURE to uncomment the HW_ACCESS macros near CMDFIFO_PROLOG and CMDFIFO_EPILOG
    //  if fifo packets are used
#ifdef CMDFIFO
    {
    extern void FifoEnable(NT9XDEVICEDATA *ppdev, FxBool enable);
    FifoEnable(ppdev,FXTRUE);
    }
#endif

    // TEMP enable the following if the lfb access converted to blt
    // HW_ACCESS_EXIT(ACCESS_2D);

    CMDFIFO_EPILOG(cmdFifo);
}

//---------------------------------------------------------------------------------------
//
// txtrSetNptHwRegs : programs the hardware registers for npt textures
//
//---------------------------------------------------------------------------------------

// TEMP: defines from glide I needed here
#define GR_LOD_LOG2_2048        0xB

void txtrSetNptHwRegs(
    int                 stage,
    RC                 *pRc,
    txtrDesc           *txtr)
{
    FxU32 taMode, baseAddress, texTiled, SStride, newTexFormat;
    SETUP_PPDEV(pRc)

    newTexFormat = txtr->txFormat >> SST_TA_FORMAT_SHIFT;
    texTiled = (txtr->txAllocMode == HWC_TEXTURE_TILED);

    // Update Texture Mode
    switch (newTexFormat) {
    case TEXFMT_YIQ_422:
        newTexFormat = TEXFMT_P8_RGB;
        break;
    case TEXFMT_AYIQ_8422:
        newTexFormat = TEXFMT_P8_RGBA;
        break;
    default:
        break;
    }

    taMode = pRc->sst.TMU[stage].taMode.vFxU32;      
    taMode &= ~(SST_TA_FORMAT | SST_TA_TEX_IS_TILED);
    taMode |= (SST_TA_CLAMPW |
                (texTiled << SST_TA_TEX_IS_TILED_SHIFT) |
                (newTexFormat << SST_TA_FORMAT_SHIFT));

    // Are we already npt-ing?
    // if ((pRc->sst.TMU[stage].taLMS.vFxU32 & SST_TA_EN_NPT) == 0x00UL) {

    // Constant field values set down in the programming model
    pRc->sst.TMU[stage].taLMS.vFxU32 =
        ((GR_LOD_LOG2_2048 << (SST_TA_LMS_FRACBITS + SST_TA_LMS_MIN_SHIFT)) |
         (GR_LOD_LOG2_2048 << (SST_TA_LMS_FRACBITS + SST_TA_LMS_MAX_SHIFT)) |
         (0x00UL           << SST_TA_LMS_BIAS_SHIFT)                         |
         (0x00UL           << SST_TA_LMS_LAR_SHIFT)                          |
         (0x00UL           << SST_TA_LMS_TSPLIT)                             |
         (0x00UL           << SST_TA_LMS_MBA_MODE_SHIFT)                     |
                              SST_TA_EN_NPT);
    UPDATE_HW_STATE( reg3D.TMU[stage].taLMS.group );

    // update shadows
    {
        const FxU32 maxS = txtr->txMipInfo[0].mipWidth  - 0x01UL;
        const FxU32 maxT = txtr->txMipInfo[0].mipHeight - 0x01UL;

        baseAddress = txtr->txTaBaseAddr0;

        // TEMP:
        // if ( gc->texAGP )
        //    baseAddress |= SST_TA_TEX_AGP;

        // TEMP:
        // if ( gc->texStagger )
        //    baseAddress |= SST_TA_TEX_STAGGERED;

        if (texTiled) {
            SStride = txtr->txMipInfo[0].mipHuTiles - 1;
            baseAddress |= txtr->txMipInfo[0].mipTileMode << SST_TA_NPT_TM_SHIFT;
        }
        else
            SStride = (txtr->txMipInfo[0].mipStride >> 4) - 1;;

        pRc->sst.TMU[stage].taBaseAddr0.vFxU32 = baseAddress;
        pRc->sst.TMU[stage].taMode.vFxU32      = taMode; 
        pRc->sst.TMU[stage].taNPT.vFxU32       =
                    ((maxS << SST_TA_NPT_S_MAX_SHIFT) |
                     (maxT << SST_TA_NPT_T_MAX_SHIFT) |
                     ((SStride) << SST_TA_NPT_S_STRIDE_SHIFT));

        if ( txtr->txAllocMode == HWC_TEXTURE_BLOCK_LINEAR ) 
            pRc->sst.TMU[stage].taNPT.vFxU32 |=  SST_BLOCK_LINEAR; 

        UPDATE_HW_STATE( reg3D.TMU[stage].taBaseAddr0.group | reg3D.TMU[stage].taMode.group | reg3D.TMU[stage].taNPT.group );
    }

    // for NPT, always set s,t scaling to 11; encoded as vpSTScale0.s0, vpSTScale0.t0
    txtr->txSTscale = 0xbb;
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadNptBackdoor : old debug npt texture download entry point
//
//---------------------------------------------------------------------------------------

#if 0 // remove this for now

void txtrDownloadNptBackdoor(
    NT9XDEVICEDATA *ppdev, 
    FxU32           startAddress, 
    FxU32           pitch,
    txtrDesc       *txtr,
    void           *data)
{
    HwcTextureAllocation texAllocMethod;
    txMipInfo hwMapInfo;

    texAllocMethod = txtrNptInfo(txtr, FXTRUE, &hwMapInfo);

    txtrDownloadNptInternal(
        ppdev,
        startAddress,
        pitch,
        &hwMapInfo, 
        txtr,
        &sst2TexelInfo[txtr->format >> SST_TA_FORMAT_SHIFT],
        data,
        0,
        txtr->nptWidth-1,
        0,
        txtr->nptHeight-1,
        0,                 // TEMP: isAGP
        texAllocMethod);
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadNptInternal : old debug npt texture download main routine
//
//---------------------------------------------------------------------------------------
        
static void txtrDownloadNptInternal(
    NT9XDEVICEDATA *ppdev, 
    FxU32           startAddress, 
    FxU32           pitch,
    txMipInfo      *hwMapInfo,
    txtrDesc       *txtr,
    HwcTexelInfo   *texelInfo, 
    void           *data, 
    int             min_s,
    int             max_s,
    int             min_t,
    int             max_t, 
    FxBool          isAGP,
    HwcTextureAllocation texAllocMethod)
{
    int t, sinc, tinc;
    const FxU32 *src32 = (const FxU32 *) data;
    const FxU16 *src16 = (const FxU16 *) data;
    const FxU8 *src8 = (const FxU8 *) data;
    FxU8 *baseptr;
    FxU32 ndwords;
    FxU32 bpt;
    FxU32 blockSize;
    FxU32 minAddr = 0xffffffff, maxAddr = 0;
    FxU32 offset;

    CMDFIFO_PROLOG(cmdFifo);

    // TEMP: enable the following if the lfb access converted to blt
    // HW_ACCESS_ENTRY(cmdFifo,ACCESS_2D);
  
    // TEMP: should only need MOP if the downloaded area is in the active texture
    CMDFIFO_CHECKROOM( cmdFifo, 2*MOP_SIZE );
    SETMOP( cmdFifo, SST_MOP_STALL_2D );
    SETMOP( cmdFifo, SST_MOP_STALL_3D | 
                    (SST_MOP_STALL_3D_TD << SST_MOP_STALL_3D_SEL_SHIFT) | 
                        SST_MOP_FLUSH_TCACHE);

    bpt  = texelInfo->bitsPerTexel;
    sinc = texelInfo->minWidth;
    tinc = texelInfo->minHeight;

    // compute number of bytes in block
    blockSize = (sinc * tinc * bpt) >> 3;
    ndwords = ((max_s-min_s+1) * bpt) >> 5;
    if (ndwords == 0)
        ndwords = 1;

    baseptr = data;

    // TEMP: memSize = isAGP ? gc->hwc->agpSizeInBytes : gc->hwc->memSizeInBytes;

    for (t = min_t; t <= max_t; t += tinc) {
        FxI32 s;
        src8  = (const FxU8  *) baseptr;
        src16 = (const FxU16 *) baseptr;
        src32 = (const FxU32 *) baseptr;

        for (s = min_s; s <= max_s; s += sinc) {
            switch ( texAllocMethod ) {
            case HWC_TEXTURE_LINEAR:
                offset = startAddress+t*hwMapInfo->stride+((s*tinc*bpt)>>3);
                break;
            case HWC_TEXTURE_TILED:
            case HWC_TEXTURE_BLOCK_LINEAR:
                break;
            }

            if ( minAddr > offset ) minAddr = offset;
            if ( maxAddr < ( offset+blockSize )) maxAddr = ( offset+blockSize );

            switch (blockSize) {
            case 1:
            default:
                D3DPRINT(13, "memory offset 0x%x(%d) too large\n", offset, offset);
                break;
            }
        }
        baseptr += pitch;
    }

    // TEMP: enable the following if the lfb access converted to blt
    // HW_ACCESS_EXIT(ACCESS_2D);

    CMDFIFO_EPILOG(cmdFifo);
}

#endif
