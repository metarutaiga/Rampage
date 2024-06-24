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
*/

#include "d3global.h"                   // global d3d data structs
#include "fifomgr.h"                    // cmdfifo access macros
#include "d3txtr2.h"                    // texture-specific data structs
#include "d3npt.h"                      // txtrNptInfo
#include "ddcam.h"                      // cam flags

HwcTexelInfo sst2TexelInfo[] = {
    {  8, 1, 1, 2, 2 },                 // SST_TA_RGB332       0x0
    {  8, 1, 1, 2, 2 },                 // SST_TA_YIQ422       0x1
    {  8, 1, 1, 2, 2 },                 // SST_TA_A8           0x2
    {  8, 1, 1, 2, 2 },                 // SST_TA_I8           0x3
    {  8, 1, 1, 2, 2 },                 // SST_TA_AI44         0x4
    {  8, 1, 1, 2, 2 },                 // SST_TA_P8           0x5
    {  8, 1, 1, 2, 2 },                 // SST_TA_P8_ARGB6666  0x6
    {  0, 0, 0, 0, 0 },                 // Reserved            0x7
    { 16, 1, 1, 2, 2 },                 // SST_TA_ARGB8332     0x8
    { 16, 1, 1, 2, 2 },                 // SST_TA_AYIQ8422     0x9
    { 16, 1, 1, 2, 2 },                 // SST_TA_RGB565       0xa
    { 16, 1, 1, 2, 2 },                 // SST_TA_ARGB1555     0xb
    { 16, 1, 1, 2, 2 },                 // SST_TA_ARGB4444     0xc
    { 16, 1, 1, 2, 2 },                 // SST_TA_AI88         0xd
    { 16, 1, 1, 2, 2 },                 // SST_TA_AP88         0xe
    {  0, 0, 0, 0, 0 },                 // Reserved            0xf
    {  0, 0, 0, 0, 0 },                 // Reserved           0x10
    {  4, 8, 4, 3, 2 },                 // SST_TA_FXT1        0x11
    { 32, 1, 1, 2, 2 },                 // SST_TA_ARGB8888    0x12
    { 16, 2, 1, 2, 2 },                 // SST_TA_YUYV422     0x13
    { 16, 2, 1, 2, 2 },                 // SST_TA_UYVY422     0x14
    { 32, 1, 1, 2, 2 },                 // SST_TA_AVYU444     0x15
    {  4, 4, 4, 2, 2 },                 // SST_TA_DXT1        0x16
    {  8, 4, 4, 2, 2 },                 // SST_TA_DXT2        0x17
    {  8, 4, 4, 2, 2 },                 // SST_TA_DXT3        0x18
    {  8, 4, 4, 2, 2 },                 // SST_TA_DXT4        0x19
    {  8, 4, 4, 2, 2 },                 // SST_TA_DXT5        0x1a
};

//---------------------------------------------------------------------------------------
//
// txtrSurfDataAlloc()
//
//---------------------------------------------------------------------------------------

FxU32 txtrSurfDataAlloc(                    // allocate surface data structure
    NT9XDEVICEDATA     *ppdev,              // IN: needed by DXMALLOCZ
    FXSURFACEDATA     **surfData,           // OUT: returned surf data object
    FxU32               txFormat,           // IN: texture format, in hw reg format
    FxU32               txFormatFlags,      // IN: texture format flags
    FxU32               txBpt)              // IN: texture bits per texel
{
    FXSURFACEDATA      *surfDataTmp;
    txtrDesc           *txtr;
    int                 i, linked_txtrdescs = 0;

    // this allows the txtrDesc type to live in d3txtr2.h and a pointer in shared.h
    surfDataTmp = (FXSURFACEDATA *) DXMALLOCZ(sizeof(FXSURFACEDATA) + sizeof(txtrDesc));
    if (surfDataTmp == NULL)
        goto unwind1;

    memset (surfDataTmp, 0, sizeof(FXSURFACEDATA) + sizeof(txtrDesc));

    // connect surface data's texture descriptor pointer to the txtrDesc struct
    surfDataTmp->pTxtrDesc = txtr = SURFDATA_GET_TXTRDESC(surfDataTmp);

    // fill in some texture descriptor data
    surfDataTmp->pTxtrDesc->txFormat      = txFormat;
    surfDataTmp->pTxtrDesc->txFormatFlags = txFormatFlags;
    surfDataTmp->pTxtrDesc->txBpt         = txBpt;

    // decide whether we need extra texture descriptors for this format
    if (txFormatFlags & TEXFMTFLG_UVL)
        // so far only uvl bumpmapping requires this
        linked_txtrdescs = 2;
    else if (txFormatFlags & TEXFMTFLG_UV)
        linked_txtrdescs = 1;

    // allocate memory for any extra texture descriptors if necessary
    if (linked_txtrdescs) {

        txtr->txtrDescNext = (txtrDesc*)DXMALLOCZ(sizeof(txtrDesc) * linked_txtrdescs);
        if (txtr->txtrDescNext == NULL)
            goto unwind2;

        txtr = txtr->txtrDescNext;

        // connect texture descriptor pointers
        for (i=1; i <= linked_txtrdescs - 1; i++) {
            txtr->txtrDescNext = txtr + 1;
            txtr = txtr->txtrDescNext;
        }
    }

    // terminate the texture desc linked list
    txtr->txtrDescNext = NULL;

    // reset our floating pointer
    txtr = SURFDATA_GET_TXTRDESC(surfDataTmp);

    // handle special-case texture formats, starting with UVL bumpmapping
    if (txFormatFlags & (TEXFMTFLG_UVL | TEXFMTFLG_UV)) {
        txtrDesc *uvtd, *ltd;

        uvtd = txtr->txtrDescNext;

        // Set the texture formats and depths
        uvtd->txFormat      = TEXFMT_ALPHA_INTENSITY_88 << SST_TA_FORMAT_SHIFT;
        uvtd->txFormatFlags = txFormatFlags;
        uvtd->txBpt         = 16;

        if (txtr->txFormatFlags & TEXFMTFLG_UVL)
        {
            ltd  = txtr->txtrDescNext->txtrDescNext;
            ltd->txFormat      = TEXFMT_INTENSITY_8 << SST_TA_FORMAT_SHIFT;
            ltd->txFormatFlags = txFormatFlags;
            ltd->txBpt         = 8;
        }
    }

    return *surfData = surfDataTmp, 0; // exit success

unwind2:
    DXFREE(surfDataTmp);

unwind1:
    return 1; // exit error
}

//---------------------------------------------------------------------------------------
//
// txtrSurfDataTerminate()
//
//---------------------------------------------------------------------------------------

void txtrSurfDataTerminate(
    NT9XDEVICEDATA     *ppdev,
    FXSURFACEDATA      *surfData)
{
    txtrDesc *txtr = surfData->pTxtrDesc;

    if (txtr == NULL)
        return;

    // delete memory for any extra texture descriptors
    if (txtr->txtrDescNext != NULL) {
        DXFREE(txtr->txtrDescNext);
    }

    // free FXSURFACDATA memory
    memset(surfData, 0, sizeof(FXSURFACEDATA) + sizeof(txtrDesc));
    DXFREE(surfData);
}

//---------------------------------------------------------------------------------------
//
// txtrGetFormat
//
//---------------------------------------------------------------------------------------

FxU32 txtrGetFormat(
    LPDDRAWI_DDRAWSURFACE_LCL   surfLCL,
    NT9XDEVICEDATA             *ppdev,
    FxU32                      *txFormat,
    FxU32                      *txBpt,
    FxU32                      *txFormatFlags)
{
    LPDDRAWI_DDRAWSURFACE_GBL   surfGBL = surfLCL->lpGbl;
    FxU32                       format, formatFlags, bitsPerTexel;
    FxU32                       ec = 0; // success

    // texel format
    if (surfLCL->dwFlags & DDRAWISURF_HASPIXELFORMAT) {
        if (surfGBL->ddpfSurface.dwFlags & DDPF_BUMPDUDV) // general bumpmapping
        {
            LPDDPIXELFORMAT ppf = &surfGBL->ddpfSurface;

            if (ppf->dwFlags & DDPF_BUMPLUMINANCE) // UVL format
            {
                if (ppf->dwBumpBitCount         == 16           &&
                    ppf->dwBumpDuBitMask        == UVL556_UMASK &&
                    ppf->dwBumpDvBitMask        == UVL556_VMASK &&
                    ppf->dwBumpLuminanceBitMask == UVL556_LMASK)
                {
                    format = TEXFMT_ALPHA_INTENSITY_88 << SST_TA_FORMAT_SHIFT;
                    formatFlags = TEXFMTFLG_INTENSITY | TEXFMTFLG_ALPHA | TEXFMTFLG_UVL;
                    bitsPerTexel = 16;
                }
                else if (ppf->dwBumpBitCount         == 24           &&
                         ppf->dwBumpDuBitMask        == UVL888_UMASK &&
                         ppf->dwBumpDvBitMask        == UVL888_VMASK &&
                         ppf->dwBumpLuminanceBitMask == UVL888_LMASK)
                {
                    format = TEXFMT_ARGB_8888 << SST_TA_FORMAT_SHIFT;
                    formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA | TEXFMTFLG_UVL;
                    bitsPerTexel = 32;
                }
                else
                {
                    D3DPRINT( 255, "Failed creating an unsupported UVL texture");
                    ec = DDERR_INVALIDPIXELFORMAT;
                    goto unwind1;
                }
            }
            else if (ppf->dwBumpBitCount         == 16         &&
                     ppf->dwBumpDuBitMask        == UV88_UMASK &&
                     ppf->dwBumpDvBitMask        == UV88_VMASK &&
                     ppf->dwBumpLuminanceBitMask == 0)
            {
                format = TEXFMT_ALPHA_INTENSITY_88 << SST_TA_FORMAT_SHIFT;
                formatFlags = TEXFMTFLG_INTENSITY | TEXFMTFLG_ALPHA | TEXFMTFLG_UV;
                bitsPerTexel = 16;
            }
            else
            {
                D3DPRINT( 255, "Failed creating an unsupported UV texture");
                ec = DDERR_INVALIDPIXELFORMAT;
                goto unwind1;
            }
        }
        else if (surfGBL->ddpfSurface.dwFlags & DDPF_PALETTEINDEXED8)
        {
            if( surfGBL->ddpfSurface.dwFlags & DDPF_ALPHAPIXELS ) {
                D3DPRINT( 255, "Creating an 8bit ALPHA+PALETTIZED texture surface,");
                format = TEXFMT_ALPHA_P8_RGB << SST_TA_FORMAT_SHIFT;
                formatFlags = TEXFMTFLG_PALETTIZED | TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
                bitsPerTexel = 16;
            }
            else {
                D3DPRINT( 255, "Creating an 8bit PALETTIZED texture surface,");
                format = TEXFMT_P8_RGB << SST_TA_FORMAT_SHIFT;
                formatFlags = TEXFMTFLG_PALETTIZED | TEXFMTFLG_RGB;
                bitsPerTexel = 8;
            }
        }
        // Intensity textures (Luminance)
        else if( surfGBL->ddpfSurface.dwFlags & DDPF_LUMINANCE )
        {
            if( surfGBL->ddpfSurface.dwFlags & DDPF_ALPHAPIXELS )
            {
                if( 16 == surfGBL->ddpfSurface.dwLuminanceBitCount )
                {
                   // Texture mode 16bit - alpha(8), intensity(8)
                   D3DPRINT( 255, "Creating an 16bit ALPHA+LUMINANCE texture surface,");
                   format = TEXFMT_ALPHA_INTENSITY_88 << SST_TA_FORMAT_SHIFT;
                   formatFlags = TEXFMTFLG_INTENSITY | TEXFMTFLG_ALPHA;        
                   bitsPerTexel = 16;
                }
                else {
                   // Texture mode 8bit - alpha(4), intensity(4)
                   D3DPRINT( 255, "Creating an 8bit ALPHA+LUMINANCE texture surface,");
                   format = TEXFMT_ALPHA_INTENSITY_44 << SST_TA_FORMAT_SHIFT;
                   formatFlags = TEXFMTFLG_INTENSITY | TEXFMTFLG_ALPHA;        
                   bitsPerTexel = 8;
                }
            }
            else {
                // Texture mode 8bit - intensity(8)
                D3DPRINT( 255, "Creating an 8bit LUMINANCE texture surface,");
                format = TEXFMT_INTENSITY_8 << SST_TA_FORMAT_SHIFT;
                formatFlags = TEXFMTFLG_INTENSITY;
                bitsPerTexel = 8;
            }
        }
        // support for dx6/s3 compressed texture formats
        else if (SURFACE_IS_DXT(surfGBL->ddpfSurface))
        {
            // width and height must be a multiple of 4 pixels
            if ((surfGBL->wWidth  & 0x3) || (surfGBL->wHeight & 0x3)) {
                D3DPRINT( 255, "Error: DXT surface not a multiple of 4 texels width by height" );
                ec = DDERR_HEIGHTALIGN;
                goto unwind1; // return (DDHAL_DRIVER_HANDLED);
            }
            formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;

            switch (surfGBL->ddpfSurface.dwFourCC)
            {
            case FOURCC_DXT1:
               format = TEXFMT_DXT1 << SST_TA_FORMAT_SHIFT; 
               bitsPerTexel = 4;
               break;
            case FOURCC_DXT2: 
               format = TEXFMT_DXT2 << SST_TA_FORMAT_SHIFT; 
               bitsPerTexel = 8;
               break;
            case FOURCC_DXT3: 
               format = TEXFMT_DXT3 << SST_TA_FORMAT_SHIFT; 
               bitsPerTexel = 8;
               break;
            case FOURCC_DXT4: 
               format = TEXFMT_DXT4 << SST_TA_FORMAT_SHIFT;
               bitsPerTexel = 8;
               break;
            case FOURCC_DXT5: 
               format = TEXFMT_DXT5 << SST_TA_FORMAT_SHIFT; 
               bitsPerTexel = 8;
               break;
            default:
               D3DPRINT( 255, "Error unknown fouecc format" );
               ec = DDERR_INVALIDPIXELFORMAT;
               goto unwind1; // return (DDHAL_DRIVER_HANDLED);
            }
        }
        // support for 3dfx compressed texture formats
        else if (SURFACE_IS_FXT(surfGBL->ddpfSurface)) {
            // width and height must be at least 8x4 pixels
            if ((surfGBL->wWidth & 0x7) || (surfGBL->wHeight & 0x3)) {
                D3DPRINT( 255, "Error: DXT surface not a multiple of 4 texels width by height" );
                ec = DDERR_HEIGHTALIGN;
                goto unwind1; // return (DDHAL_DRIVER_HANDLED);
            }
            formatFlags = TEXFMT_FXT1 | TEXFMTFLG_ALPHA;
            format = TEXFMT_FXT1 << SST_TA_FORMAT_SHIFT;
            bitsPerTexel = 4;
        }

        // rbg texture
        else switch (surfGBL->ddpfSurface.dwRBitMask)
        {
        case RGB4444_RMASK:
            D3DPRINT( 255, "Creating a 16bit 4444 texture surface,");
            format = TEXFMT_ARGB_4444 << SST_TA_FORMAT_SHIFT; 
            formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
            bitsPerTexel = 16;
            break; 

        case RGB8888_RMASK:
            if ( surfGBL->ddpfSurface.dwRGBBitCount == 32 ) {
                D3DPRINT( 255, "Creating a 32bit 8888 texture surface,");
                format = TEXFMT_ARGB_8888 << SST_TA_FORMAT_SHIFT; 
                formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
                bitsPerTexel = 32;
            }
            else {
                // invalid pixelformat specified, exit with error
                D3DPRINT( 255, "error: invalid texture format 888 specified" );
                ec = DDERR_UNSUPPORTEDFORMAT;
                goto unwind1; // return DDHAL_DRIVER_HANDLED;
            }
            break; 

        case RGB1555_RMASK:       // same as case RGB555_RMASK: 
            if( surfGBL->ddpfSurface.dwRGBAlphaBitMask == RGB1555_AMASK ) {
                D3DPRINT( 255, "Creating a 16bit 1555 texture surface,");
                format = TEXFMT_ARGB_1555 << SST_TA_FORMAT_SHIFT; 
                formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
                bitsPerTexel = 16;
            }
            else
            {
                // RGB 555 is not a supported Rampage format.
                D3DPRINT( 255, "error: invalid texture format 555 specified");
                ec = DDERR_UNSUPPORTEDFORMAT;
                goto unwind1;               // return DDHAL_DRIVER_HANDLED
            }
            break;

        case RGB565_RMASK: 
            D3DPRINT( 255, "Creating a 16bit 565 texture surface,");
            format = TEXFMT_RGB_565 << SST_TA_FORMAT_SHIFT; 
            formatFlags = TEXFMTFLG_RGB;
            bitsPerTexel = 16;
            break;

        case RGB332_RMASK:
            if (surfGBL->ddpfSurface.dwRGBAlphaBitMask == RGB8332_AMASK) {
                D3DPRINT( 255, "Creating a 16bit 4444 texture surface,");
                format = TEXFMT_ARGB_8332 << SST_TA_FORMAT_SHIFT; 
                formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
                bitsPerTexel = 16;
            } 
            else {
                D3DPRINT( 255, "Creating an 8-bit 332 texture surface,");
                format = TEXFMT_RGB_332 << SST_TA_FORMAT_SHIFT; 
                formatFlags = TEXFMTFLG_RGB;
                bitsPerTexel = 8;
            }
            break; 

        default:
             // invalid pixelformat specified, exit with error
             D3DPRINT( 255, "error: invalid texture pixel format specified" );
             ec = DDERR_UNSUPPORTEDFORMAT;
             goto unwind1; // return DDHAL_DRIVER_HANDLED;
        }
    }
    else {
        // if no pixelformat specified, use format of primary surface
        D3DPRINT( 255, "No texture format - use primary surface format" );
        bitsPerTexel = (DWORD) GETPRIMARYBYTEDEPTH << 3L;
        formatFlags = TEXFMTFLG_RGB;

        if (bitsPerTexel == 32) {
            format = TEXFMT_ARGB_8888 << SST_TA_FORMAT_SHIFT;
            formatFlags |= TEXFMTFLG_ALPHA;
        }
        else if (bitsPerTexel == 16)
            format = TEXFMT_RGB_565 << SST_TA_FORMAT_SHIFT;
        else {
            ec = DDERR_UNSUPPORTEDFORMAT;
            goto unwind1; // return DDHAL_DRIVER_HANDLED;
        }
    }

#ifdef CUBEMAP
   if (surfLCL->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP)
   { 
      formatFlags |= TEXFMTFLG_CUBEMAP;
   }
#endif

unwind1:
    return *txFormat = format,
           *txBpt = bitsPerTexel,
           *txFormatFlags = formatFlags,
           ec;
}   

//---------------------------------------------------------------------------------------
//
// txtrCalcTextureSize
//
//---------------------------------------------------------------------------------------

FxU32 txtrCalcTextureSize(
    FxU32       txWidth,
    FxU32       txHeight,
    FxU32      *txSlog,
    FxU32      *txTlog)
{
    FxU32       calcSize;
    FxU32       slog, tlog;
    int         isNPT;
    
    slog = tlog = 0;

    if (txWidth == 1)
        isNPT = 0;
    else {
        calcSize = 0;
        while (calcSize < txWidth)
            calcSize = 1 << slog++;

        if (calcSize > txWidth)
            isNPT = 1;
        else
            isNPT = 0;
    }

    if (!isNPT && (txHeight != 1)) {
        calcSize = 0;
        while (calcSize < txHeight)
            calcSize = 1 << tlog++;

        if (calcSize > txHeight)
            isNPT = 1;
    }

    *txSlog = isNPT ? -1 : slog ? slog-1 : 0;
    *txTlog = isNPT ? -1 : tlog ? tlog-1 : 0;
    return isNPT;
}

//----------------------------------------------------------------
// txtrCalcMipmapInfo()
//
// This takes a pointer to the surface information as well as a pointer
// to the txtrDesc structure, and sets the txtrDesc members appropriately
// given the sizes, format, mips (etc) of the DD surface.  Also return the
// number of mip-maps in numMipmaps.
// Return code is 0 (success) or 1 (failure).  If failure occurs, *ddec
// is set to the appropriate DD error code, else is set to DD_OK.
//----------------------------------------------------------------

FxU32 txtrCalcMipmapInfo(
    NT9XDEVICEDATA             *ppdev,      // [IN]
    txtrDesc                   *txtr,       // [IN/OUT]
    FxU32                       txWidth,
    FxU32                       txHeight,
    FxU32                       ddMipMapCount)
{
    FxU32 txSlog, txTlog, txNpt;
    FxU32 i, lmsStart, lmsEnd, numMipmaps;

    // determine allocation type: tiled, linear (not supported yet), block linear
    switch (txtr->txFormat >> SST_TA_FORMAT_SHIFT)
    {
    case TEXFMT_DXT1:
    case TEXFMT_DXT2:
    case TEXFMT_DXT3:
    case TEXFMT_DXT4:
    case TEXFMT_DXT5:
    case TEXFMT_FXT1:
        txtr->txAllocMode = HWC_TEXTURE_BLOCK_LINEAR; 
        break;
    default:
        txtr->txAllocMode = HWC_TEXTURE_TILED;
        txtr->txFormat |= SST_TA_TEX_IS_TILED;
        break;
    }

    // if we're testing linear textures
    if (txtr->txFormatFlags & TEXFMTFLG_LINEAR) {
        // as long as it's not block-linear
        if (!(txtr->txAllocMode == HWC_TEXTURE_BLOCK_LINEAR)) {
            // force linear textures
            txtr->txAllocMode = HWC_TEXTURE_LINEAR;
            txtr->txFormat &= ~SST_TA_TEX_IS_TILED;
        }
    }

    // determine whether texture is NPT or not
    txNpt = txtrCalcTextureSize(txWidth, txHeight, &txSlog, &txTlog);

    txtr->txFormatFlags |= txNpt ? TEXFMTFLG_NPT : 0;

    // store the original ddraw width and height
    txtr->txWidth  = txWidth;
    txtr->txHeight = txHeight;

    if (txtr->txFormatFlags & TEXFMTFLG_NPT) {
        numMipmaps = 1;
        txtr->txLmsMin = 0;
        txtr->txLmsMax = 0;
        txtr->txMipInfo[0].mipWidth  = txWidth;
        txtr->txMipInfo[0].mipHeight = txHeight;

        // this is expected to work for tiled and block-linear formats
        txtrCalcNptSize(txtr);
    }
    else {
        // compute number of mip levels present
        numMipmaps = ddMipMapCount;
        numMipmaps = numMipmaps == 0 ? 1 : numMipmaps;

        // identify the long edge, compute lar amd lms of largest mip level
        if (txWidth >= txHeight) {
            txtr->txSIsLarger = 1;
            txtr->txLar = txSlog - txTlog;
            txtr->txLmsMax = txSlog;
        } else {
            txtr->txSIsLarger = 0;
            txtr->txLar = txTlog - txSlog;
            txtr->txLmsMax = txTlog;
        }

        txtr->txLmsMin = txtr->txLmsMax - (numMipmaps - 1);
        if (txtr->txLmsMin < 0)
            goto unwind1;

        // compute and store sizes from the smallest present to level 2k by 2k
        for (i=txtr->txLmsMin; i<=SST2_LMS_MAX; i++)
        {
            txtrCalcMipSizes(
                txtr->txAllocMode,
                txtr->txFormat,
                txtr->txBpt,
                i,
                txtr->txLar,
                txtr->txSIsLarger,
                &txtr->txMipInfo[i]);
        }

        txtr->txTotalSize = 0;

        if (txtr->txAllocMode == HWC_TEXTURE_TILED) {
            lmsStart = txtr->txLmsMin;
            lmsEnd   = txtr->txLmsMax < 5 ? 5 : txtr->txLmsMax;
        }
        else if (txtr->txAllocMode == HWC_TEXTURE_BLOCK_LINEAR) {
            lmsStart = txtr->txLmsMin < 5 ? 5 : txtr->txLmsMin;
            lmsEnd   = txtr->txLmsMax < 5 ? 5 : txtr->txLmsMax;
        }
        else { // pure linear
            lmsStart = txtr->txLmsMin < 4 ? 4 : txtr->txLmsMin;
            lmsEnd   = txtr->txLmsMax < 4 ? 4 : txtr->txLmsMax;
        }

        // sum up mip level sizes to compute total hardware texture size
        for (i=lmsStart; i<=lmsEnd; i++)
            txtr->txTotalSize += txtr->txMipInfo[i].mipSize;
    }
	return 0; // exit success

unwind1:
	return 1; // exit error
}

//---------------------------------------------------------------------------------------
//
// txtrCalcMipSizes
//
// Description:
// Side effects:
// Algorithm:
//
//---------------------------------------------------------------------------------------

void txtrCalcMipSizes(
    HwcTxAlloc      texAllocMode,               // tiled, linear, block linear
    FxU32           txFormat,                   // format stored in hw register format
    FxU32           txBpt,                      // bits per texel
    FxU32           txLms,                      // log of map size
    FxU32           txLar,                      // log of aspect ratio
    FxU32           txSIsLarger,                // bool: s edge is longer
    txMipInfo      *MIP)                        // OUT: returned info
{
    FxU32 hMicroTiles, vMicroTiles;
    HwcTexelInfo *texelInfo = &sst2TexelInfo[txFormat >> SST_TA_FORMAT_SHIFT];
    FxU32 blockWidth  = 1 << texelInfo->log2BlockWidth;
    FxU32 blockHeight = 1 << texelInfo->log2BlockHeight;
    FxU32 txSlog, txTlog, i, txtr_bp_block;

    MIP->mipLms = txLms;
    MIP->mipPackedOffset = 0;

    switch (texAllocMode) {
    case HWC_TEXTURE_TILED: 
		if (txLms <= 5) {
            // size includes space for packed maps
			if (txSIsLarger) {
				MIP->mipWidth = 32; // width always 32 texels
                // height must include room for packed maps, stored as texels
   			    MIP->mipHeight = (txLar >= 3) ? 8 : (txLar == 2) ? 16 : (txLar == 1) ? 24 : 48;
			}
			else {
				MIP->mipHeight = 32; // height is always 32 texels
                // width must include room for packed maps, stored as texels
				if (txBpt == 8)
					MIP->mipWidth = 32;
				else if (txBpt == 16)
					MIP->mipWidth = (txLar == 1) ? 32 : 16;
				else
					MIP->mipWidth = (txLar == 1) ? 24 : (txLar == 2) ? 16 : 8;
			}

            if (txLms == 5) {
                MIP->mipSOffset = MIP->mipTOffset = 0;
                MIP->mipPackedOffset = 0;
            }
            else if (txSIsLarger) {
                MIP->mipSOffset = (txLms < 4) ? (32 - (1 << (txLms + 1))) : 0;
                MIP->mipTOffset = (txLar > 3) ? 4 : (1 << (5 - txLar));
                // it's (total bytes of map 5) + (s byte offset of the given map)
                MIP->mipPackedOffset = 32 * (1 << (5 - txLar)) * (txBpt >> 3) +
                                       MIP->mipSOffset * (txBpt >> 3);
            }
            else {
                MIP->mipSOffset = (txLar > 3) ? 4 : (1 << (5 - txLar));
                MIP->mipTOffset = (txLms < 4) ? (32 - (1 << (txLms + 1))) : 0;
                // it's (byte width of map 5) * (t texel offset of the given map)
                MIP->mipPackedOffset = (1 << (5 - txLar)) * txBpt * MIP->mipTOffset;
            }
		} 
		else { // tiled texture, lms > 5
			if (txSIsLarger) {
				MIP->mipWidth  = 0x0001 << txLms;
				MIP->mipHeight = 0x0001 << ((txLms >= txLar) ? txLms - txLar : 0);
			}
			else {
				MIP->mipHeight = 0x0001 << txLms;
				MIP->mipWidth  = 0x0001 << ((txLms >= txLar) ? txLms - txLar : 0);
			}
            MIP->mipSOffset = MIP->mipTOffset = 0;
            MIP->mipPackedOffset = 0;
        }

        // snap to tile sizes, compute tile mode, map stride & size in bytes
		switch (txBpt) {
		case 8: // tiled surface, at 8 bpt microtile = 32x4
   			if (MIP->mipWidth < 32)
   				MIP->mipWidth = 32;
   			if (MIP->mipHeight < 8)
   				MIP->mipHeight = 8;
		    hMicroTiles = (MIP->mipWidth + 31) >> 5;
		    vMicroTiles = (MIP->mipHeight + 3) >> 2;
            break;
		case 16: // tiled surface, at 16 bpt microtile = 16x4
   			if (MIP->mipWidth < 16)
   				MIP->mipWidth = 16;
   			if (MIP->mipHeight < 8)
   				MIP->mipHeight = 8;
		    hMicroTiles = (MIP->mipWidth + 15) >> 4;
		    vMicroTiles = (MIP->mipHeight + 3) >> 2;
            break;
		case 32: // tiled surface, at 32 bpt microtile = 8x4 
   			if (MIP->mipWidth < 8)
   				MIP->mipWidth = 8;
   			if (MIP->mipHeight < 8)
   				MIP->mipHeight = 8;
            hMicroTiles = (MIP->mipWidth + 7) >> 3;
            vMicroTiles = (MIP->mipHeight + 3) >> 2;
            break;
        default:
            __asm {int 3};
        }
    
        MIP->mipTileMode  = (txLms <= 5 ) ? 0 : ((vMicroTiles >= 8) ? 1 : 0);
        MIP->mipAlignment = 7; // 128B alignment
        MIP->mipStride    = hMicroTiles * SST_MC_MICRO_TILE_WIDTH;
        MIP->mipHuTiles   = hMicroTiles;
        MIP->mipVuTiles   = vMicroTiles;
        MIP->mipSize      = txLms < 5 ? 0 : hMicroTiles * vMicroTiles * SST_MC_MICRO_TILE_SIZE;
        break;

    // compute raw width and height based on lms and lar
    case HWC_TEXTURE_BLOCK_LINEAR: 
        if (txSIsLarger) {
            MIP->mipWidth  = 0x0001 << txLms;
            MIP->mipHeight = 0x0001 << ((txLms >= txLar) ? txLms - txLar : 0);
            txSlog = txLms;
            txTlog = (txLms >= txLar) ? txLms - txLar : 0;
        }
        else {
            MIP->mipHeight = 0x0001 << txLms;
            MIP->mipWidth  = 0x0001 << ((txLms >= txLar) ? txLms - txLar : 0);
            txTlog = txLms;
            txSlog = (txLms >= txLar) ? txLms - txLar : 0;
        }

        // all levels must be an integral number of blocks
        if ( MIP->mipWidth < blockWidth ) 
            MIP->mipWidth = blockWidth;
        if ( MIP->mipHeight < blockHeight ) 
            MIP->mipHeight = blockHeight;

        // compute map stride & size in bytes
        MIP->mipAlignment = 5; // 32 byte alignment for block linear textures
        MIP->mipStride = (MIP->mipWidth * texelInfo->bitsPerTexel ) >> 3;
        MIP->mipSize = MIP->mipHeight * MIP->mipStride;

        txtr_bp_block = txBpt == 4 ? 8 : 16;

        if (txLms < 5) { // compute packed offsets in bytes for small mipmap levels
            for (i = txLms; i < 5; i++) {
                // compute and add the distance from the given lms to lms 5
                MIP->mipPackedOffset += (1 << (txSlog + 1)) *   // s txls in lod+1
                                        (1 << (txTlog + 1)) /   // t txls in lod+1
                                         16                 *   // texels per block
                                         txtr_bp_block;         // bytes per block
                txSlog++; txTlog++;   
            }
        }
        break;

    case HWC_TEXTURE_LINEAR: 
        if (txLms < 4) {
            MIP->mipSOffset = 32 - (1 << (txLms + 1));
            MIP->mipTOffset = 0;
            MIP->mipPackedOffset = MIP->mipSOffset * (txBpt >> 3);
		    if (txSIsLarger) {
			    MIP->mipWidth  = 1 << 4;
   			    MIP->mipHeight = 1 << ((4 >= txLar) ? 4 - txLar : 0);
		    }
		    else {
                MIP->mipWidth  = 1 << ((4 >= txLar) ? 4 - txLar : 0);
			    MIP->mipHeight = 1 << 4;
		    }
        }
        else {
            MIP->mipSOffset = MIP->mipTOffset = MIP->mipPackedOffset = 0;
            if (txSIsLarger) {
                MIP->mipWidth  = 1 << txLms;
                MIP->mipHeight = 1 << ((txLms >= txLar) ? txLms - txLar : 0);
                txSlog = txLms;
                txTlog = (txLms >= txLar) ? txLms - txLar : 0;
            } else {
                MIP->mipHeight = 1 << txLms;
                MIP->mipWidth  = 1 << ((txLms >= txLar) ? txLms - txLar : 0);
                txTlog = txLms;
                txSlog = (txLms >= txLar) ? txLms - txLar : 0;
            }
        }
        MIP->mipWidth = (txLms <= 4) ? MAX(MIP->mipWidth, 32) : MAX(MIP->mipWidth, 16);
        MIP->mipAlignment = 5;
        MIP->mipStride = MIP->mipWidth * (txBpt >> 3);
        MIP->mipSize = MIP->mipHeight * MIP->mipStride;
        break;
    }
}

//---------------------------------------------------------------------------------------
//
// txtrAllocMemory()
//
//---------------------------------------------------------------------------------------

FxU32 txtrAllocMemory(
    NT9XDEVICEDATA             *ppdev,      // IN
    LPDDRAWI_DDRAWSURFACE_LCL   surfLCL,    // IN
    FxU32                       size,       // IN
    FxU32                       flags,      // IN
    FxU32                      *HwStart,    // OUT
    FxU32                      *VaStart,    // OUT
    FxU32                      *AgpStart)   // OUT
{
    FxU32 tileFlag, pitch, HwEnd, HwVidMem;
    FxU32 rc, surfCaps;

    if (flags & TXTR_ALLOC_FRAMEBUF_MEM) {

        // surfMgr_allocSurface expects no caps for local surfaces
        surfCaps = surfLCL->ddsCaps.dwCaps;
        if (!(surfCaps & DDSCAPS_NONLOCALVIDMEM))
            surfCaps = 0;

        // we always tell the memmgr to allocate linear memory even if we use it as tiled
        tileFlag = MEM_IN_LINEAR;           // for some reason the routine wants a ptr

        rc = surfMgr_allocSurface(          // allocate frame buffer memory
            surfLCL->lpGbl->lpDD,           // direct draw vidmemalloc need this
            surfCaps,                       // user's surface
            0,                              // unused
            size,                           // width in linear space
            1,                              // height in pixel (linear space)
            0xFF,                           // linear address mask
            HwStart,                        // returned lfb start address of allocation
            &HwEnd,                         // unused return value
            &HwVidMem,                      // AGP only (?)
            &pitch,                         // pitch
            &tileFlag);                     // MEM_IN_LINEAR 

        if (rc == DDERR_OUTOFVIDEOMEMORY) {
            *HwStart = 0;
            goto unwind1;
        }
    }
    else
        *HwStart = 0;

    if (flags & TXTR_ALLOC_VIRTUAL_MEM) {
        rc = camMgrAllocEntry(              // allocate virtual memory
            _DD(camMgr),                    // use the global cam manager
            ppdev,                          // device data
            CAM_ALLOC_PHANTOM_MEM,          // options: vm only, no cam entry
            0,                              // parameter not used in this case
            0,                              // parameter not used in this case
            *HwStart,            		    // hardware address of data
            size,          				    // virtual memory in bytes to allocate
            0,                              // parameter not used in this case
            0,                              // parameter not used in this case
            0,                              // parameter not used in this case
            VaStart);                       // OUT: returned lfb addr into surface

        if (rc != CAM_SUCCESS)
            goto unwind2;
    }
    else
        *VaStart = 0;

    return *AgpStart = HwVidMem, 0;         // TEMP: this needs to be clearer

unwind2:
    surfMgr_freeSurface(                    // free the hardware memory
        surfLCL->lpGbl->lpDD,               // direct draw object
        0,                                  // there is no texture virtual address
        *HwStart - _FF(LFBBASE),            // the texture hardware addr (offset)
        surfLCL->ddsCaps.dwCaps);           // surface caps

unwind1:
    return 1;
}

//---------------------------------------------------------------------------------------
//
// txtrFreeMemory()
// Free the HW video memory allocated to *txtr, as well as any
// CAM entry.
//
//---------------------------------------------------------------------------------------

void txtrFreeMemory(
    NT9XDEVICEDATA             *ppdev,
    LPDDRAWI_DDRAWSURFACE_LCL   surfLCL,
    txtrDesc                   *txtr)
{
    FxU32 rc;
    FxU32 txHwFreeAddr = txtr->txHwStart - _FF(LFBBASE);
    FxU32 txVaFreeAddr = 0;

    if (TXTR_IS_NLVM(txtr)) {
        txHwFreeAddr = txtr->txHwGartPhysStart;
        txVaFreeAddr = txtr->txVaGartLinStart;
    }

    // double-check that there's something to free
    if (txtr->txHwStart) {
        surfMgr_freeSurface(                    // free frame buffer memory
            surfLCL->lpGbl->lpDD,               // direct draw object
            txVaFreeAddr,                       // TEMP: the texture virtual addr?
            txHwFreeAddr,                       // the texture hardware addr (offset)
            surfLCL->ddsCaps.dwCaps);           // surface caps

        txtr->txHwStart = 0;
    }

    if (TXTR_IS_NLVM(txtr))
        txtr->txVaStart = 0;                    // No CAM for AGP surfaces

    if (txtr->txVaStart) {
        rc = camMgrFreeEntry(                   // free virtual memory and/or cam entry
            _DD(camMgr),                        // the cam manager
            ppdev,                              // global device data
            CAM_FREE_PHANTOM_MEM,               // freeing flags
            txtr->txVaStart);                   // virtual address to free

        if (rc != CAM_SUCCESS) {
            D3DPRINT(0, "camMgrFreeEntry failure");
            __asm int 3;
        }

        txtr->txVaStart = 0;
    }
}

//---------------------------------------------------------------------------------------
//
// txtrCalcMipPointers()
//
//---------------------------------------------------------------------------------------

void txtrCalcMipPointers(txtrDesc *txtr)
{
    FxU32 accum_lod_size=0;
    FxI32 i, lmsStart;

    if (txtr->txFormatFlags & TEXFMTFLG_NPT) {
        // set offsets and base addr
        txtr->txMipInfo[0].mipHwStart = txtr->txHwStart;
        txtr->txMipInfo[0].mipVaStart = txtr->txVaStart;
    }
    else {
        if (txtr->txAllocMode == HWC_TEXTURE_LINEAR)
            lmsStart = txtr->txLmsMax < 4 ? 4 : txtr->txLmsMax;
        else
            lmsStart = txtr->txLmsMax < 5 ? 5 : txtr->txLmsMax;

        // mip level offsets calculation
        for (i=lmsStart; i>=txtr->txLmsMin; i--) {

            // set pointers to each actual level
            txtr->txMipInfo[i].mipHwStart = txtr->txHwStart + accum_lod_size;

            // certain textures (AGP for one) don't have a virtual address
            if (txtr->txVaStart != 0)
                txtr->txMipInfo[i].mipVaStart = txtr->txVaStart + accum_lod_size +
                                                txtr->txMipInfo[i].mipPackedOffset;
            else
                txtr->txMipInfo[i].mipVaStart = 0;

            if (txtr->txAllocMode == HWC_TEXTURE_LINEAR)
                accum_lod_size += i > 4 ? txtr->txMipInfo[i].mipSize : 0;
            else
                accum_lod_size += i > 5 ? txtr->txMipInfo[i].mipSize : 0;
        }
    }
}

//---------------------------------------------------------------------------------------
//
// txtrCalcRegisterValues()
//
//---------------------------------------------------------------------------------------

void txtrCalcRegisterValues(
    NT9XDEVICEDATA 	*ppdev,		// IN
    txtrDesc        *txtr)
{
    FxU32 base_addr_offset, lmsEnd, i;

    if (txtr->txFormatFlags & TEXFMTFLG_NPT)
        txtr->txTaBaseAddr0 = txtr->txHwStart - _FF(LFBBASE);
    else {
        // base address calculation
        base_addr_offset = 0;

        if (txtr->txAllocMode == HWC_TEXTURE_LINEAR)
            lmsEnd = txtr->txLmsMax < 4 ? 4 : txtr->txLmsMax;
        else
            lmsEnd = txtr->txLmsMax < 5 ? 5 : txtr->txLmsMax;

        // don't count the size of the 1st map in the chain
        for (i=lmsEnd+1; i<=SST2_LMS_MAX; i++)
            base_addr_offset += txtr->txMipInfo[i].mipSize;

        // base addr must point to where lms 11 would be
        txtr->txTaBaseAddr0 = txtr->txHwStart - base_addr_offset - _FF(LFBBASE);
    }

    // reset texture base address and mark for AGP
    if (TXTR_IS_NLVM(txtr)) {
        txtr->txTaBaseAddr0 += _FF(LFBBASE);
        txtr->txTaBaseAddr0 |= SST_TA_TEX_AGP;
    }

    txtr->txTaShiftBias = 0; // always 0 since texel centers always off for now

    txtr->txTaLMS = (txtr->txSIsLarger <<  SST_TA_LMS_S_IS_LARGER_SHIFT)              |
                    (txtr->txLar       <<  SST_TA_LMS_LAR_SHIFT)                      |
                    (txtr->txLmsMin    << (SST_TA_LMS_FRACBITS+SST_TA_LMS_MIN_SHIFT)) |
                    (txtr->txLmsMax    << (SST_TA_LMS_FRACBITS+SST_TA_LMS_MAX_SHIFT)); 

    txtr->txTaLMS_noMipMaps = 
                    (txtr->txSIsLarger <<  SST_TA_LMS_S_IS_LARGER_SHIFT)              |
                    (txtr->txLar       <<  SST_TA_LMS_LAR_SHIFT)                      |
                    (txtr->txLmsMax    << (SST_TA_LMS_FRACBITS+SST_TA_LMS_MIN_SHIFT)) |
                    (txtr->txLmsMax    << (SST_TA_LMS_FRACBITS+SST_TA_LMS_MAX_SHIFT)); 

    // scale factor assuming this is the only texture
    if (txtr->txSIsLarger) 
        txtr->txSTscale = txtr->txLar << SST_VP_T0_SCALE_SHIFT; // Scale T
    else
        txtr->txSTscale = txtr->txLar << SST_VP_S0_SCALE_SHIFT; // Scale S
}

//---------------------------------------------------------------------------------------
//
// txtrAllocLinearAddr()
//
//---------------------------------------------------------------------------------------

FxU8* txtrAllocLinearAddr(
    NT9XDEVICEDATA      *ppdev,
    txtrDesc            *txtr,
    FxU32                width,
    FxU32                height)
{
    FxU32 txSlog, txTlog, txLms, camLms;
    FxU32 camFlags, camEntrySize, appAddr, rc;

    // get the slog, tlog of the level being locked
    txtrCalcTextureSize(width, height, &txSlog, &txTlog);

    // for npt textures use txLms == 0 to index to the correct txMipInfo
    txLms = txtr->txFormatFlags & TEXFMTFLG_NPT ? 0 : txSlog >= txTlog ? txSlog : txTlog;

    camFlags = CAM_ALLOC_CAM_ENTRY;
    camLms = txLms < 5 ? 5 : txLms;

    if (txtr->txAllocMode == HWC_TEXTURE_BLOCK_LINEAR)
        camFlags |= CAM_BLOCK_LINEAR;
    else if (txtr->txAllocMode == HWC_TEXTURE_LINEAR)
        camFlags |= CAM_PURE_LINEAR;
    else {
        camFlags |= CAM_TILED;
        camFlags |= txtr->txMipInfo[camLms].mipTileMode == 1 ? CAM_TILE_MODE_1 : 0;
    }

    // for packed maps, the cam entry must span all the packed lods
    camEntrySize = txLms <= 5 ? txtr->txTotalSize : txtr->txMipInfo[camLms].mipSize;

    rc = camMgrAllocEntry(                  // allocate a cam entry but not virtual mem
        _DD(camMgr),                        // use the global cam manager
        ppdev,                              // device data
        camFlags,                           // allocation options, including tile mode
        CAM_ENTRY_ANY,                      // choose any cam entry available        
        txtr->txMipInfo[camLms].mipVaStart, // virtual address of texture
        txtr->txMipInfo[camLms].mipHwStart, // hardware address of texture
        camEntrySize,                       // size in bytes to make the cam entry
        txtr->txMipInfo[camLms].mipStride,  // stride in bytes of surface
        txtr->txBpt >> 3,                   // pixel depth of surface, bytes
        txtr->txBpt >> 3,                   // lfb depth of surface, bytes
        &appAddr);                          // returned lfb addr into surface

    if (rc != CAM_SUCCESS)
        return 0;

    txtr->txHasCamEntry = 1;
        
    return (FxU8 *) txtr->txMipInfo[txLms].mipVaStart;
}

//---------------------------------------------------------------------------------------
//
// txtrFreeLinearAddr()
//
//---------------------------------------------------------------------------------------

void txtrFreeLinearAddr(
    NT9XDEVICEDATA     *ppdev,
    txtrDesc           *txtr,
    FxU32               width,
    FxU32               height)
{
    FxU32 txLms, txSlog, txTlog, rc;

    // get the slog, tlog of level being unlocked
    txtrCalcTextureSize(width, height, &txSlog, &txTlog);

    txLms = txtr->txFormatFlags & TEXFMTFLG_NPT ? 
            0 : txSlog >= txTlog ? txSlog : txTlog;

    txLms = txLms < 5 ? 5 : txLms;

    rc = camMgrFreeEntry(                   // free cam entry back to cam manager
        _DD(camMgr),                        // the cam manager
        ppdev,                              // global device data
        CAM_FREE_CAM_ENTRY,                 // cam flags
        txtr->txMipInfo[txLms].mipVaStart); // virtual address (not freed)

    if (rc != CAM_SUCCESS) {
        D3DPRINT(0, "camMgrFreeEntry failure %s %d", __FILE__, __LINE__);
        __asm int 3;
    }

    txtr->txHasCamEntry = 0;
}

//---------------------------------------------------------------------------------------
//
// txtrUnmungeUVLSurface()
// Break out the original UVL data and save it into the
// UV and L surfaces separately.  The original texture data
// is 8-bit or 5-bit 2's-complement which is converted here
// into a biased 8-bit value (biased by 128).  When accessed
// in the VTA, ATEX and CTEX should use the SST_TA_INV_MINUS_HALF
// flag to remap these values to -0.5..0.5.  Then scale them by
// 2 to get the full range.
//
//---------------------------------------------------------------------------------------

void txtrUnmungeUVLSurface(
    NT9XDEVICEDATA             *ppdev,
    txtrDesc                   *txtr,
    FxU8                       *uvlData,
    FxU32                       width,
    FxU32                       height,
    FxU32                       pitch
    )
{
    txtrDesc *uvtd = NULL, *ltd = NULL;
    FxU8 *uvData, *lData;
    BOOL bIsUVL = TXTR_IS_UVL(txtr);

    uvtd = txtr->txtrDescNext;
    if (bIsUVL) ltd  = uvtd->txtrDescNext;

    // uvlData points to the address of the texture
    // data.  We need to read from uvlData and unmunge into
    // newly-locked UV and L textures.

    // 1) Get linear pointer for UV surface thru CAM
    // 2) Get linear pointer for L  surface thru CAM
    if ((uvData = txtrAllocLinearAddr (ppdev, uvtd, width, height)) == NULL)
        return;

    if (bIsUVL && (lData  = txtrAllocLinearAddr (ppdev,  ltd, width, height)) == NULL)
    {
        txtrFreeLinearAddr (ppdev, uvtd, width, height);
        return;
    }

    // 3) Unmunge UVL data into UV and L components
    if (txtr->txBpt == 16 &&                // UVL556
        (txtr->txFormatFlags & TEXFMTFLG_UVL))
    {
        FxU8 *start = uvlData;
        FxU16 *p, word;
        FxI8 u,v;
        FxU32 w, h = height;                // h,w: local width/height iterators

        while (h--)
        {
            p = (FxU16*)start;
            w = width;
            while (w--)
            {
                word = *p;
                *lData++ = ((word & UVL556_LMASK) >> 8) & 0x00ff;
                // Add 128 to map 2's-complement into 0..255 and subtract 0.5 in VTA.
                u = (word & UVL556_UMASK) << 3;
                v = (word & UVL556_VMASK) >> 2;
                *((FxU16*)uvData)++ = ((128 + u) | ((128 + v) << 8));
                p++;
            }
            start += pitch;
        }
    }
    else if (txtr->txBpt == 32 &&       // UVL888
             (txtr->txFormatFlags & TEXFMTFLG_UVL))
    {
        FxI8 *start = (FxI8*)uvlData;
        FxI8 *p;                        // signed 8-bit
        FxU32 w, h = height;            // h,w: local width/height iterators

        while (h--)
        {
            p = start;
            w = width;
            while (w--)
            {
                // Add 128 to map 2's-complement into 0..255 and subtract 0.5 in VTA.
                *uvData++ = 128 + *p++; // U - access by byte for correct alignment
                *uvData++ = 128 + *p++; // V
                *lData++  = *p++;       // L
            }
            start += pitch;
        }
    }
    else if (txtr->txBpt == 16 &&       // UV88
             (txtr->txFormatFlags & TEXFMTFLG_UV))
    {
        FxI8 *start = (FxI8*)uvlData;
        FxI8 *p;
        FxU32 w, h = height;            // h,w: local width/height iterators

        while (h--)
        {
            p = start;
            w = width;
            while (w--)
            {
                // Add 128 to map 2's-complement into 0..255 and subtract 0.5 in VTA.
                *uvData++ = 128 + *p++; // U - access by byte for correct alignment
                *uvData++ = 128 + *p++; // V
            }
            start += pitch;
        }
    }

    // 4) Release UV and L CAM entries.
    txtrFreeLinearAddr (ppdev, uvtd, width, height);
    if (bIsUVL) txtrFreeLinearAddr (ppdev,  ltd, width, height);

    return;
} // txtrUnmungeUVLSurface()


//---------------------------------------------------------------------------------------
//
// txtrDownloadLevelHblt
//
//---------------------------------------------------------------------------------------

// TEMP: consistent with ddglobal.h without including all that
#define ghw2D   _DD(sst22DRegs)

DWORD txtrDownloadLevelHblt(            // download texture using host blt
    NT9XDEVICEDATA *ppdev,              // needed by mop macros, _DD()
    txtrDesc       *txtr,               // the texture structure
    FxU32           lms,                // which mip level
    RECTL          *srcRect,            // subrectangle describing download source
    POINT          *dstPoint,           // destination point of download
    FxU32          *srcData,            // pointer to texture data to download
    FxU32           srcPitch)           // pitch in bytes of source
{
    FxU32           s, t, numDwWide, *ptr32, agp=0;
    FxU32           stride, bltDstFormat, bltSrcFormat, srcDelta;
    FxU32           width, height, dstX, dstY, tileMode, linear;
    txMipInfo      *MIP;

    CMDFIFO_PROLOG(cmdFifo);
    HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);
    CMDFIFO_CHECKROOM(cmdFifo, MOP_SIZE );

    // Why mop 3D when it's not involved? Leaving alone for now. -MC-
    SETMOP(cmdFifo, SST_MOP_FLUSH_ALL_3D);

    MIP = &txtr->txMipInfo[lms];

    // compute blt adjustments based on subrectangle
    srcDelta = srcRect->top * srcPitch + srcRect->left * (txtr->txBpt >> 3);
    srcData += srcDelta / 4;
    width  = srcRect->right  - srcRect->left;
    height = srcRect->bottom - srcRect->top;
    dstX = MIP->mipSOffset + dstPoint->x;
    dstY = MIP->mipTOffset + dstPoint->y;

    if (TXTR_IS_NLVM(txtr))
        agp = 1;

    if (txtr->txFormatFlags & TEXFMTFLG_LINEAR) {
        stride = MIP->mipStride;
        tileMode = 0;
        linear = 1;
    }
    else { // stride is in tiles for tiled surfaces
        stride = MIP->mipStride / SST_MC_MICRO_TILE_WIDTH;
        tileMode = MIP->mipTileMode;
        linear = 0;
    }
      
    switch (txtr->txBpt) {
    default:
    case 0:
    case 4:
        _asm int 3;
        break;
    case 8:
        bltSrcFormat = (SST_WX_PIXFMT_8BPP << SST_WX_SRC_FORMAT_SHIFT);
        bltDstFormat = (SST_WX_PIXFMT_8BPP << SST_WX_DST_FORMAT_SHIFT)    |
                       (tileMode           << SST_WX_DST_TILE_MODE_SHIFT) |
                       (linear             << SST_WX_DST_LINEAR_SHIFT)    |
                       (agp                << SST_WX_DST_AGP_SHIFT)       |
                        stride;
        break;
    case 16:
        bltSrcFormat = (SST_WX_PIXFMT_16BPP << SST_WX_SRC_FORMAT_SHIFT);
        bltDstFormat = (SST_WX_PIXFMT_16BPP << SST_WX_DST_FORMAT_SHIFT)    |
                       (tileMode            << SST_WX_DST_TILE_MODE_SHIFT) |
                       (linear              << SST_WX_DST_LINEAR_SHIFT)    |
                       (agp                 << SST_WX_DST_AGP_SHIFT)       |
                        stride;
        break;
    case 32:
        bltSrcFormat = (SST_WX_PIXFMT_32BPP << SST_WX_SRC_FORMAT_SHIFT);
        bltDstFormat = (SST_WX_PIXFMT_32BPP << SST_WX_DST_FORMAT_SHIFT)    |
                       (tileMode            << SST_WX_DST_TILE_MODE_SHIFT) |
                       (linear              << SST_WX_DST_LINEAR_SHIFT)    |
                       (agp                 << SST_WX_DST_AGP_SHIFT)       |
                        stride;
        break;
    }

    CMDFIFO_CHECKROOM(cmdFifo, PH2_SIZE + 9); 
    SETPH(cmdFifo, CMDFIFO_BUILD_PK2( clip0minBit | clip0maxBit | dstBaseAddrBit
                                    | dstFormatBit | srcFormatBit | srcXYBit
                                    | dstSizeBit | dstXYBit | commandBit ) );

    SETPD( cmdFifo, ghw2D->clip0min,    0 );
    SETPD( cmdFifo, ghw2D->clip0max,    0xffffffff );
    if( agp ) {
      SETPD( cmdFifo, ghw2D->dstBaseAddr, MIP->mipHwStart );
    }
    else {
      SETPD( cmdFifo, ghw2D->dstBaseAddr, MIP->mipHwStart - _FF(LFBBASE) );
    }
    SETPD( cmdFifo, ghw2D->dstFormat,   bltDstFormat );
    SETPD( cmdFifo, ghw2D->srcFormat,   bltSrcFormat );
    SETPD( cmdFifo, ghw2D->srcXY,       0 );
    SETPD( cmdFifo, ghw2D->dstSize,     (height << SST_WX_DST_HEIGHT_SHIFT) | width );
    SETPD( cmdFifo, ghw2D->dstXY,       (dstY << SST_WX_DST_Y_SHIFT) | dstX);

    // writing this register will launch the host blt
    SETPD(cmdFifo, ghw2D->command, (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_HOST_BLT);
    BUMP( PH2_SIZE + 9 );

    // compute dwords per line in the texture, round up to send whole dwords
    numDwWide = (width * (txtr->txBpt >> 3) + 3) / 4;

    for (t = 0; t < height; t++) {
        ptr32 = srcData;

        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + numDwWide );
        SETPH(cmdFifo, (SSTCP_PKT1 | SSTCP_PKT1_2D |
                        (((FxU32)regoffsetof(Sst2WxRegs, launch[0])) << SSTCP_REGBASE_SHIFT)|
                        ((numDwWide) << SSTCP_PKT1_2D_NWORDS_SHIFT)));

        for (s = 0; s < numDwWide; s++)
            SETPD( cmdFifo, ghw2D->launch[0], *ptr32++ );

        CMDFIFO_BUMP( cmdFifo );
        srcData += srcPitch >> 2;
    }

    if( agp ) {
      // Is the 2D stall really needed? -MC-
      CMDFIFO_CHECKROOM(cmdFifo, MOP_SIZE*2 );
      SETMOP(cmdFifo, SST_MOP_STALL_2D);
      SETMOP(cmdFifo, SST_MOP_AGP_FLUSH);
      HW_ACCESS_EXIT(ACCESS_2D);
    }
    else {
      // Why mop 3D when it's not involved? Leaving alone for now. -MC-
      CMDFIFO_CHECKROOM(cmdFifo, MOP_SIZE*2 );
      SETMOP(cmdFifo, SST_MOP_STALL_2D);
      SETMOP(cmdFifo, SST_MOP_STALL_3D |
                   (SST_MOP_STALL_3D_TD << SST_MOP_STALL_3D_SEL_SHIFT) | 
                    SST_MOP_FLUSH_TCACHE);
      HW_ACCESS_EXIT(ACCESS_3D);
    } 
    CMDFIFO_EPILOG(cmdFifo);
    return 0;
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadLevelDXT
//
//---------------------------------------------------------------------------------------

// #define SSTCP_PKT1_MAX_2D_XFER_SIZE pow(2,19) // note: hardcoded packet 1 transfer size
#define PKT1_MAX_2D_XFER_SIZE ((FxU32)0x1000)  // TEMP: why? because of fifo size limits?
#define MAX_LINEAR_BLT_SIZE 0x4000

DWORD txtrDownloadLevelDXT(             // download dxt texture using host blt
    NT9XDEVICEDATA *ppdev,              // needed by mop macros, _DD()
    txtrDesc       *txtr,               // the texture object
    FxU32           lms,                // mip level
    FxU32           width,              // texture width, texels
    FxU32           height,             // texture height, texels
    FxU32          *srcData,            // pointer to texture data to download
    FxU32           srcLength)          // length in bytes of source data
{
    FxU32          *ptr32, bltDstFormat, bltSrcFormat;
    FxU32           pktLength, copyLength;
    int             pktNum, pktRem, i, j;
    txMipInfo      *MIP;                // info struct per mip level

    CMDFIFO_PROLOG(cmdFifo);
    HW_ACCESS_ENTRY( cmdFifo, ACCESS_3D); // 3d mop writes 3d register if direct writes
    CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );
    SETMOP( cmdFifo, (SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
          ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << 
            SST_MOP_STALL_3D_SEL_SHIFT)));

    MIP = &txtr->txMipInfo[lms];
    height = txtr->txBpt == 4 ? height / 2 : height;

    bltSrcFormat = (SST_WX_PIXFMT_8BPP << SST_WX_SRC_FORMAT_SHIFT);
    bltDstFormat = (SST_WX_PIXFMT_8BPP << SST_WX_DST_FORMAT_SHIFT) |
                   (1 << SST_WX_DST_LINEAR_SHIFT)                  |
                   width;

    CMDFIFO_CHECKROOM(cmdFifo, PH2_SIZE + 9); 
    SETPH(cmdFifo, CMDFIFO_BUILD_PK2( clip0minBit | clip0maxBit | dstBaseAddrBit |
                                      dstFormatBit | srcFormatBit | srcXYBit |
                                      dstSizeBit | dstXYBit | commandBit ));

    SETPD( cmdFifo, ghw2D->clip0min,    0 );
    SETPD( cmdFifo, ghw2D->clip0max,    0xffffffff );
    SETPD( cmdFifo, ghw2D->dstBaseAddr, MIP->mipHwStart + MIP->mipPackedOffset - _FF(LFBBASE));
    SETPD( cmdFifo, ghw2D->dstFormat,   bltDstFormat );
    SETPD( cmdFifo, ghw2D->srcFormat,   bltSrcFormat );
    SETPD( cmdFifo, ghw2D->srcXY,       0 );
    SETPD( cmdFifo, ghw2D->dstSize,     (height << SST_WX_DST_HEIGHT_SHIFT) | width);
    SETPD( cmdFifo, ghw2D->dstXY,       0 );

    // writing this register will launch the host blt
    SETPD(cmdFifo, ghw2D->command, (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_HOST_BLT);
    BUMP( PH2_SIZE + 9 );

    ptr32 = srcData;
    copyLength = srcLength/4;

    if (copyLength > PKT1_MAX_2D_XFER_SIZE) {
        pktLength = PKT1_MAX_2D_XFER_SIZE;
        pktNum = copyLength / PKT1_MAX_2D_XFER_SIZE;
        pktRem = copyLength % PKT1_MAX_2D_XFER_SIZE;
    }
    else {
        pktLength = copyLength;
        pktNum = 1;
        pktRem = 0;
    }

    for (i = 0; i < pktNum; i++) {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + pktLength );
        SETPH(cmdFifo, (SSTCP_PKT1 | SSTCP_PKT1_2D |
                       (((FxU32)regoffsetof(Sst2WxRegs,launch[0]))
                       << SSTCP_REGBASE_SHIFT) |
                       (pktLength << SSTCP_PKT1_2D_NWORDS_SHIFT)));

        for (j = 0; j < (int) pktLength; j++) {
            SETPD( cmdFifo, ghw2D->launch[0], *ptr32++ );
            CMDFIFO_BUMP( cmdFifo );
        }
    }

    if (pktRem) {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + pktRem );
        SETPH(cmdFifo, (SSTCP_PKT1 | SSTCP_PKT1_2D |
                        (((FxU32)regoffsetof(Sst2WxRegs,launch[0]))<<SSTCP_REGBASE_SHIFT)|
                        (pktRem << SSTCP_PKT1_2D_NWORDS_SHIFT)));

        for (j = 0; j < (int) pktRem; j++) {
            SETPD( cmdFifo, ghw2D->launch[0], *ptr32++ );
            CMDFIFO_BUMP( cmdFifo );
        }
    }

    CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE*2 );
    SETMOP(cmdFifo, SST_MOP_STALL_2D);
    SETMOP(cmdFifo, SST_MOP_STALL_3D |
                   (SST_MOP_STALL_3D_TD << SST_MOP_STALL_3D_SEL_SHIFT) |
                    SST_MOP_FLUSH_TCACHE);
    HW_ACCESS_EXIT( ACCESS_3D);
    CMDFIFO_EPILOG(cmdFifo);

    return 0;
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadLevelLfb
//
//---------------------------------------------------------------------------------------

extern void FifoEnable(NT9XDEVICEDATA *ppdev, FxBool enable);

// TEMP: this routine not verified to work recently!

DWORD txtrDownloadLevelLfb(
    txtrDesc       *txtr,                   // texture descriptor structure
    FxI32           level,                  // which level is being loaded
    FxU32           levelAddr,              // hardware address of this level
    FxU32           slog,                   // size in s log2
    FxU32           tlog,                   // size in t log2
    NT9XDEVICEDATA *ppdev,                  // needed for cam programming
    LPDDRAWI_DDRAWSURFACE_LCL surfLCL,      // needed by phantom alloc routines
    FxU32          *srcData)                // pointer to texture data to download
{
    txMipInfo      *txMipInfo = &txtr->txMipInfo[level];
    HwcTexelInfo   *texelInfo = &sst2TexelInfo[txtr->txFormat >> SST_TA_FORMAT_SHIFT];
    FxU32           levelSize;
    FxU32           lfbAddr, prevAddr, txtrDest;
    FxU32           srcPitch = surfLCL->lpGbl->lPitch;
    FxU32          *src32, *ptr32;
    FxU16          *src16, *ptr16;
    FxU8           *src8,  *ptr8;
    FxU8           *baseptr;
    FxU32           bitsPerTexel;
    int             s, t, height, width;
    FxU32           camFlags;

    #ifdef CMDFIFO
    FifoEnable(ppdev, FXFALSE);
    #endif

    // idle the hardware so this will work whether fifo is on or off
    while (FXGETBUSYSTATUS(ppdev));

    SETDW( ghw0->mopCMD, (SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
                        ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << 
                          SST_MOP_STALL_3D_SEL_SHIFT)));

    // TEMP: may allocate too much for small map sizes
    levelSize = level <= 4 ? txtr->txMipInfo[5].mipSize : txMipInfo->mipSize;
    camFlags  = CAM_TILED | CAM_ALLOC_CAM_ENTRY;
    camFlags |= txMipInfo->mipTileMode == 1 ? CAM_TILE_MODE_1 : 0;

    // don't allocate phantom memory for the largest mip level, it's already done
    if (txtr->txLmsMax != level) {
        camFlags |= CAM_ALLOC_PHANTOM_MEM;
        prevAddr = 0;
    }
    else
        prevAddr = txtr->txVaStart;

    camMgrAllocEntry(                       // allocate a cam entry, may alloc vm also
        _DD(camMgr),                        // self
        ppdev,                              // needed by lower level routines
        camFlags,                           // allocation options, including tile mode
        CAM_ENTRY_ANY,                      // which cam entry to use
        prevAddr,                           // previously allocated virtual address
        levelAddr,                          // hardware address of data
        levelSize,                          // memory in bytes to allocate
        txMipInfo->mipStride,               // stride in bytes of surface
        txtr->txBpt >> 3,                   // pixel depth of surface
        txtr->txBpt >> 3,                   // lfb depth of surface
        &lfbAddr);                          // OUT: allocated lfb addr into surface

    width  = 1 << slog;
    height = 1 << tlog;
    bitsPerTexel = texelInfo->bitsPerTexel;
    baseptr = (FxU8*) srcData;

    for (t = 0; t < height; t++)
    {
        src8  = (FxU8  *) baseptr;
        src16 = (FxU16 *) baseptr;
        src32 = (FxU32 *) baseptr;

        for (s = 0; s < width; s++)
        {
            txtrDest = lfbAddr + (t + txMipInfo->mipTOffset) * txMipInfo->mipStride + 
                                 (s + txMipInfo->mipSOffset) * (bitsPerTexel >> 3);
            switch (bitsPerTexel)
            {
            case 8:
                ptr8  = (FxU8*)txtrDest;
                *ptr8 = *src8++;
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
        baseptr += srcPitch;
    }

    // if we allocated phantom memory, free it back
    camFlags = camFlags & CAM_ALLOC_PHANTOM_MEM ? CAM_FREE_PHANTOM_MEM : 0;
    camFlags |= CAM_FREE_CAM_ENTRY;

    camMgrFreeEntry(                        // free the cam entry and vm back
        _DD(camMgr),                        // the cam manager
        ppdev,                              // device data
        camFlags,                           // deallocation flags
        lfbAddr);                           // virtual address to free back to phantom

    #ifdef CMDFIFO        
    FifoEnable(ppdev, FXTRUE);
    #endif

    return 0;       // TEMP: means success
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadLevelDbg - special debugging mode: downloads into linear memory but
//                        arranges the data in tiled format; eliminates CAM or blt
//                        doing the work of tiling the data
//
//---------------------------------------------------------------------------------------

#define BF(_val, _msb, _lsb) (((_val)>>(_lsb))&SST_MASK(_msb-_lsb+1))

DWORD txtrDownloadLevelDbg(
    txtrDesc       *txtr,               // texture descriptor structure
    FxU32           lms,                // log of map size
    FxU32           width,              // texture width, texels
    FxU32           height,             // texture height, texels
    FxU32          *srcData,            // pointer to texture data to download
    FxU32           srcPitch)           // pitch in bytes of source
{
    FxU32          *src32, *ptr32;
    FxU16          *src16, *ptr16;
    FxU8           *src8, *ptr8;
    FxU8           *baseptr;
    FxU32           i, j, s, t;
    FxU32           tmp1, tmp2, addr;
    FxU32           tileMode, baseAlign, strideAlign;
    FxBool          odd_row, stagger = FXFALSE;
    FxU32           tileStride, levelAddr;
    txMipInfo      *MIP;                // info struct per mip level

    MIP = &txtr->txMipInfo[lms];
    tileMode = MIP->mipTileMode;
    tileStride = MIP->mipStride / 32;
    levelAddr = MIP->mipHwStart;

    if (TXTR_IS_NLVM(txtr))
       levelAddr = MIP->mipVaGartLinStart;

    baseptr = (FxU8*) srcData;

    for (j = 0; j < height; j++)
    {
        src8  = (FxU8  *) baseptr;
        src16 = (FxU16 *) baseptr;
        src32 = (FxU32 *) baseptr;

        for (i = 0; i < width; i++)
        {
            baseAlign = stagger ? 14 : 7;
            strideAlign = tileMode ? 10 : 7;

            s = i + MIP->mipSOffset;
            t = j + MIP->mipTOffset;

            switch ( txtr->txBpt ) {
            case 32:
                addr = ( BF(t, 2, 2) << 7 ) |
                       ( BF(s, 2, 1) << 5 ) |
                       ( BF(t, 1, 0) << 3 ) |
                       ( BF(s, 0, 0) << 2 );
                break;
            case 16:
                addr = ( BF(t, 2, 2) << 7 ) |
                       ( BF(s, 3, 2) << 5 ) |
                       ( BF(t, 1, 0) << 3 ) |
                       ( BF(s, 1, 0) << 1 );
                break;
            case  8:
                addr = ( BF(t, 2, 2) << 7 ) |
                       ( BF(s, 4, 3) << 5 ) |
                       ( BF(t, 1, 0) << 3 ) |
                       ( BF(s, 2, 0) );
                break;
            case  4:
                addr = ( BF(t, 3, 3) << 7 ) |
                       ( BF(s, 4, 3) << 5 ) |
                       ( BF(t, 2, 2) << 4 ) |
                       ( BF(s, 2, 2) << 3 );
                break;
            default:
                _asm int 3;
            }

            switch ( tileMode ) {
            case 0:
                switch ( txtr->txBpt ) {
                case 32: 
                    tmp1 = BF(t, 11, 3) * tileStride + BF(s, 11, 3); 
                    odd_row = BF(t, 3, 3);
                    break;
                case 16: 
                    tmp1 = BF(t, 11, 3) * tileStride + BF(s, 11, 4);
                    odd_row = BF(t, 3, 3);
                    break;
                case 8: 
                    tmp1 = BF(t, 11, 3) * tileStride + BF(s, 11, 5);
                    odd_row = BF(t, 3, 3);
                    break;
                case 4: 
                    tmp1 = BF(t, 11, 4) * tileStride + BF(s, 11, 5);
                    odd_row = BF(t, 4, 4);
                    break;
   
                    }
                break;
            case 1:
                switch ( txtr->txBpt ) {
                case 32: 
                    tmp1 = BF(t, 11, 5) * tileStride + BF(s, 11, 3); 
                    tmp2 = BF(t, 4, 3);
                    odd_row = BF(t, 5, 5);
				    break;
                case 16: 
                    tmp1 = BF(t, 11, 5) * tileStride + BF(s, 11, 4); 
                    tmp2 = BF(t, 4, 3);
                    odd_row = BF(t, 5, 5);
				    break;
                case 8: 
                    tmp1 = BF(t, 11, 5) * tileStride + BF(s, 11, 5); 
                    tmp2 = BF(t, 4, 3);
                    odd_row = BF(t, 5, 5);
				    break;
                case 4: 
                    tmp1 = BF(t, 11, 6) * tileStride + BF(s, 11, 5); 
                    tmp2 = BF(t, 5, 4);
                    odd_row = BF(t, 6, 6);
				    break;
                }
                tmp1 = (tmp1 << 2) + tmp2;
                break;
            default:
                break;
            }

            if ( tmp1 & BIT(24))
                _asm int 3;

            addr += tmp1 << 8;
            addr += levelAddr;

            switch ( txtr->txBpt ) {
            case 32:
                ptr32  = (FxU32*)addr;
                *ptr32 = *src32++;
                break;
            case 16:
                ptr16  = (FxU16*)addr;
                *ptr16 = *src16++;
                break;
            case 8:
                ptr8   = (FxU8*)addr;
                *ptr8  = *src8++;
                break;
            case 4:
                // add this case?
                break;
            }
        }
        baseptr += srcPitch;
    }
    return 0;
}

#ifdef AGP_EXECUTE
//---------------------------------------------------------------------------------------
//
// txtrDownloadLevelAGP
//
// This was a WAX hostblt to download textures in AGP. Not used because 
// WAX doesn't handle blts to tile mode agp and performance probably
// less than using the host.
//
//---------------------------------------------------------------------------------------

DWORD txtrDownloadLevelAGP(                 // download AGP texture using host blt
    NT9XDEVICEDATA *ppdev,                  // needed by mop macros, _DD()
    txtrDesc       *txtr,                   // texture object
    FxU32           lms,                    // specifies the mip level being loaded
    FxU32           width,                  // texture width, texels
    FxU32           height,                 // texture height, texels
    FxU32          *srcData,                // pointer to texture data to download
    FxU32           srcPitch)               // pitch in bytes of source
{
    FxU32          *ptr32;
    FxU32           s, t, numDwWide;
    FxU32           bltDstFormat, bltSrcFormat, tileStride;
    txMipInfo      *MIP;                    // info struct per mip level

    CMDFIFO_PROLOG(cmdFifo);
    CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );
    SETMOP( cmdFifo, (SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
          ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << 
            SST_MOP_STALL_3D_SEL_SHIFT)));
      
    MIP = &txtr->txMipInfo[lms];

    // assuming tiled-only textures for the moment
    tileStride = MIP->mipStride / SST_MC_MICRO_TILE_WIDTH;
      
    // compute source, dest format registers: pretend everything is 32-bit
    switch (txtr->txBpt) {
    default:
    case 0:
    case 4:
        break;
    case 8:
        bltSrcFormat = (SST_WX_PIXFMT_8BPP << SST_WX_SRC_FORMAT_SHIFT);
        bltDstFormat = (SST_WX_PIXFMT_8BPP << SST_WX_DST_FORMAT_SHIFT)    |
                        SST_WX_DST_AGP                                    |
                       (MIP->mipTileMode   << SST_WX_DST_TILE_MODE_SHIFT) |
                        tileStride;
        break;
    case 16:
        bltSrcFormat = (SST_WX_PIXFMT_16BPP << SST_WX_SRC_FORMAT_SHIFT);
        bltDstFormat = (SST_WX_PIXFMT_16BPP << SST_WX_DST_FORMAT_SHIFT)    |
                        SST_WX_DST_AGP                                     |
                       (MIP->mipTileMode    << SST_WX_DST_TILE_MODE_SHIFT) |
                        tileStride;
        break;
    case 32:
        bltSrcFormat = (SST_WX_PIXFMT_32BPP << SST_WX_SRC_FORMAT_SHIFT);
        bltDstFormat = (SST_WX_PIXFMT_32BPP << SST_WX_DST_FORMAT_SHIFT)    |
                        SST_WX_DST_AGP                                     |
                       (MIP->mipTileMode    << SST_WX_DST_TILE_MODE_SHIFT) |
                        tileStride;
        break;
    }

    CMDFIFO_CHECKROOM(cmdFifo, PH2_SIZE + 9); 
    SETPH(cmdFifo, CMDFIFO_BUILD_PK2( clip0minBit | clip0maxBit | dstBaseAddrBit |
                                      dstFormatBit | srcFormatBit | srcXYBit |
                                      dstSizeBit | dstXYBit | commandBit ) );

    SETPD( cmdFifo, ghw2D->clip0min,    0 );
    SETPD( cmdFifo, ghw2D->clip0max,    0xffffffff );
    SETPD( cmdFifo, ghw2D->dstBaseAddr, MIP->mipHwGartPhysStart - _FF(LFBBASE));
    SETPD( cmdFifo, ghw2D->dstFormat,   bltDstFormat );
    SETPD( cmdFifo, ghw2D->srcFormat,   bltSrcFormat );
    SETPD( cmdFifo, ghw2D->srcXY,       0 );
    SETPD( cmdFifo, ghw2D->dstSize,     (height << SST_WX_DST_HEIGHT_SHIFT) | width );
    SETPD( cmdFifo, ghw2D->dstXY,       (MIP->mipTOffset << SST_WX_DST_Y_SHIFT) |
                                         MIP->mipSOffset );

    // writing this register will launch the host blt
    SETPD( cmdFifo, ghw2D->command, (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_HOST_BLT);
    BUMP( PH2_SIZE + 9 );

    // compute dwords per line in the texture, round up to send whole dwords
    numDwWide = (width * (txtr->txBpt >> 3) + 3) / 4;

    for (t = 0; t < height; t++) {
        ptr32 = srcData;

        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + numDwWide );
        SETPH(cmdFifo, (SSTCP_PKT1 |
                        SSTCP_PKT1_2D |
                        (((FxU32)regoffsetof(Sst2WxRegs, launch[0])) << SSTCP_REGBASE_SHIFT)|
                        ((numDwWide) << SSTCP_PKT1_2D_NWORDS_SHIFT)));

        for (s = 0; s < numDwWide; s++)
            SETPD( cmdFifo, ghw2D->launch[0], *ptr32++ );

        BUMP( PH1_SIZE + numDwWide);
        srcData += srcPitch >> 2;
    }

    CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE*2 );
    SETMOP(cmdFifo, SST_MOP_STALL_2D);
    SETMOP(cmdFifo, SST_MOP_AGP_FLUSH );

    CMDFIFO_EPILOG(cmdFifo);

    return 0;
}

#endif // AGP_EXECUTE

//---------------------------------------------------------------------------------------
//
// txPrintMipInfo
//
//---------------------------------------------------------------------------------------

txtrPrintMipInfo(txMipInfo *txMipInfo)
{
    D3DPRINT(0, "mipLms          %d", txMipInfo->mipLms);
    D3DPRINT(0, "mipWidth        %d", txMipInfo->mipWidth);
    D3DPRINT(0, "mipHeight       %d", txMipInfo->mipHeight);
    D3DPRINT(0, "mipSize         %x", txMipInfo->mipSize);
    D3DPRINT(0, "mipPackedOffset %x", txMipInfo->mipPackedOffset);
    D3DPRINT(0, "mipSOffset      %d", txMipInfo->mipSOffset);
    D3DPRINT(0, "mipTOffset      %d", txMipInfo->mipTOffset);
    D3DPRINT(0, "mipStride       %d", txMipInfo->mipStride);
    D3DPRINT(0, "mipHuTiles      %d", txMipInfo->mipHuTiles);
    D3DPRINT(0, "mipVuTiles      %d", txMipInfo->mipVuTiles);
    D3DPRINT(0, "mipAlignment    %d", txMipInfo->mipAlignment);
    D3DPRINT(0, "mipHwStart      %x", txMipInfo->mipHwStart);
    D3DPRINT(0, "mipVaStart      %x", txMipInfo->mipVaStart);
    D3DPRINT(0, "mipTileMode     %d", txMipInfo->mipTileMode);
}

#if 0

DWORD txtrDownloadLevelDXT(             // download dxt texture using fifo writes
    NT9XDEVICEDATA *ppdev,              // needed by mop macros, _DD()
    txtrDesc       *txtr,               // the texture object
    FxU32           lms,                // lms of the mip level being loaded
    FxU32           width,              // texture width, texels
    FxU32           height,             // texture height, texels
    FxU32          *srcData,            // pointer to texture data to download
    FxU32           srcPitch)           // pitch in bytes of source
{
    LPDDRAWI_DDRAWSURFACE_LCL ddraw_surf_lcl; // ddraw-specific vars
    DWORD          *ddraw_txtr_ptr;
    DWORD           txtr_handle;        // driver txtr-specific vars
    txtrDesc       *txtr_desc;
    long            txtr_slog;
    long            txtr_tlog;
    long            txtr_lod_count;
    long            txtr_large_lms;
    long            csim_tmu_addr;
    long            csim_map_offset;
    long            csim_pack_offset;
    DWORD           dxt_num_blocks;     // DXT-related only
    DWORD           dxt_bp_block;       // = bytes per block
    DWORD           dxt_size_bytes;    
    DWORD           dxt_bytes_copied;
    txMipInfo      *MIP,                // info struct per mip level

    dxt_bp_block = txtr->txBpt == 4 ? 8 : 16;
    dxt_bytes_copied = 0;
    csim_pack_offset = 0;

    // if this is one of the packed maps, lms 5-0
    if (MIP->mipLms < 5) {   
        int lms;
        int slog = txtr_slog;
        int tlog = txtr_tlog;

        // compute packed offset if lod < 5
        for (lms = MIP->mipLms; lms < 5; lms++)
        {
            // compute and add the distance from the given lms to lms 5
            csim_pack_offset += (1 << (slog + 1)) *     // s txls in next bigger lod
                                (1 << (tlog + 1)) /     // t txls in next bigger lod
                                 16               *     // texels per s3tc block
                                 dxt_bp_block;          // bytes per s3tc block
            slog++; tlog++;       
        }
    }

    // compute size of current dxt lod: a dxt block is always 16 texels
    dxt_num_blocks = (1 << txtr_slog) * (1 << txtr_tlog) / 16;
    dxt_size_bytes = dxt_num_blocks * dxt_bp_block;

    // perform linear memory texture download
    while (dxt_bytes_copied < dxt_size_bytes) {
        FxU32 temp = (csim_tmu_addr + csim_map_offset + csim_pack_offset);
        // use CSIM-specific memory-write macro to send each dword
        SETPD(cmdFifo, temp, *ddraw_txtr_ptr);

        dxt_bytes_copied += 4;
        csim_map_offset  += 4;
        ddraw_txtr_ptr   += 1;
    }
  
unwind:
    return;
}

#endif
