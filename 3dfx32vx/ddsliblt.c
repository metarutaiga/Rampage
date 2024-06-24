/*
** $Header: ddsliblt.c, 2, 11/5/99 9:34:25 AM PST, Andrew Sobczyk$
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
** File Name:   DDSLIBLT.C
** $Revision: 2$
** $Date: 11/5/99 9:34:25 AM PST$
**
** $History: $
**
** Description:	Internal DDRAW blt functions for SLI/AA mode
**	
*/

/****************************************************************************
*
* INTERNAL FUNCTIONS:
*
* sli_ColorFill       --- fill surface with a color, always use SRCCOPY ROP
* sli_DoBltNoSP       --- blt w/o source/pattern, may have dest. color key
* sli_DoBltS          --- screen to screen blt, also handle stretch/shrink
* sli_SystemToVideo   --- perform host to screen blt
*
****************************************************************************/

#include "precomp.h"

#ifdef SLI
#include "ddsliblt.h"

#pragma intrinsic (memcpy, memset)

/*
 * We use one scanline buffer to read the source because it gains performance,
 * and we do not need to special case overlapping in the x direction.
 *
 * We only use the second buffer if we need to read the destination before
 * writing back. Otherwise, we should not use the second buffer as it is
 * slower.
 */
// maximum x resolution: 2048, maximum color depth: 4 bytes per pixel

BYTE scanBuf1[2048* 4];
BYTE scanBuf2[2048* 4];

#define GET_STRIDE(pSurf) (((FXSURFACEDATA*)pSurf->lpGbl->dwReserved1)->dwPitch)
#define GET_LFBPTR(pSurf) ((((FXSURFACEDATA*)pSurf->lpGbl->dwReserved1)->phantomlfbPtr == 0x0) ? (((FXSURFACEDATA*)pSurf->lpGbl->dwReserved1)->lfbPtr) : (((FXSURFACEDATA*)pSurf->lpGbl->dwReserved1)->phantomlfbPtr))


/***************************************************************************/
/*                             DEFINES                                     */
/***************************************************************************/

// pointers to functions returning voids

typedef void (__stdcall * PBLTFUNC) (BLT_PARAMS);
typedef void (__stdcall * PSBLTFUNC) (LPDDHAL_BLTDATA, BYTE *, BYTE *, DWORD, DWORD, DWORD, DWORD);

typedef enum tag_colorKey { none, src, dst, both } CKEY_TYPE;

// not implemented features

#ifdef DEBUG
 #define NOT_SUPPORTED  _asm { int 3 }
#else
 #define NOT_SUPPORTED
#endif

// macro to convert SSTG_PIX_FMT_* to bytes per pixel

#define GET_BYTE_DEPTH(BytesPerPel, PelFormat) \
        switch(PelFormat) \
        { \
            case SST_WX_PIXFMT_8BPP:  \
                BytesPerPel = 1;    \
                break;              \
            case SST_WX_PIXFMT_16BPP: \
                BytesPerPel = 2;    \
                break;              \
            case SST_WX_PIXFMT_32BPP: \
                BytesPerPel = 4;    \
                break;              \
            default: /* format not supported */ \
                BytesPerPel = 0;    \
                break;              \
        }

// macro to advance to the next scanline

#define NEXT_SCAN { srcPtr += srcPitch; dstPtr += dstPitch; }

/***************************************************************************/
/*                       FUNCTION PROTOTYPES                               */
/***************************************************************************/

// parameter list for blt functions

#define BLT_PARAMS \
    LPDDHAL_BLTDATA  pbd, \
    DWORD dstWidthBytes, DWORD dstHeight, DWORD srcPitch, DWORD dstPitch, \
    BYTE *srcPtr, BYTE *dstPtr, DWORD srcBytesPerPel, DWORD dstBytesPerPel

void __stdcall sBltSrcToScanBuf(LPDDHAL_BLTDATA, BYTE *, BYTE *, DWORD, DWORD, DWORD, DWORD);
void __stdcall xsBltSrcToScanBuf(LPDDHAL_BLTDATA,BYTE *, BYTE *, DWORD, DWORD, DWORD, DWORD);

void __stdcall bltDSa   (BLT_PARAMS);
void __stdcall bltDSan  (BLT_PARAMS);
void __stdcall bltDSna  (BLT_PARAMS);
void __stdcall bltDSno  (BLT_PARAMS);
void __stdcall bltDSo   (BLT_PARAMS);
void __stdcall bltDSon  (BLT_PARAMS);
void __stdcall bltDSx   (BLT_PARAMS);
void __stdcall bltDSxn  (BLT_PARAMS);
void __stdcall bltNop   (BLT_PARAMS);
void __stdcall bltS     (BLT_PARAMS);
void __stdcall bltSDna  (BLT_PARAMS);
void __stdcall bltSDno  (BLT_PARAMS);
void __stdcall bltSn    (BLT_PARAMS);

void __stdcall xBltDSa  (BLT_PARAMS);
void __stdcall xBltDSan (BLT_PARAMS);
void __stdcall xBltDSna (BLT_PARAMS);
void __stdcall xBltDSno (BLT_PARAMS);
void __stdcall xBltDSo  (BLT_PARAMS);
void __stdcall xBltDSon (BLT_PARAMS);
void __stdcall xBltDSx  (BLT_PARAMS);
void __stdcall xBltDSxn (BLT_PARAMS);
void __stdcall xBltS    (BLT_PARAMS);
void __stdcall xBltSDna (BLT_PARAMS);
void __stdcall xBltSDno (BLT_PARAMS);
void __stdcall xBltSn   (BLT_PARAMS);

void __stdcall sBltS    (BLT_PARAMS);

/***************************************************************************/
/*                         FUNCTION TABLES                                 */
/***************************************************************************/

// VRAM/system memory to VRAM opaque blt, no stretch/shrink

PBLTFUNC BltFuncTab[16] =
{
    (PBLTFUNC) &bltNop,     // BLACKNESS is supported by sli_DoBltNoSP()
    (PBLTFUNC) &bltDSon,    // NOTSRCERASE
    (PBLTFUNC) &bltDSna,
    (PBLTFUNC) &bltSn,      // NOTSRCCOPY
    (PBLTFUNC) &bltSDna,    // SRCERASE
    (PBLTFUNC) &bltNop,     // DSTINVERT is supported by sli_DoBltNoSP()
    (PBLTFUNC) &bltDSx,     // SRCINVERT
    (PBLTFUNC) &bltDSan,
    (PBLTFUNC) &bltDSa,     // SRCAND
    (PBLTFUNC) &bltDSxn,
    (PBLTFUNC) &bltNop,     // rop D is supported by sli_DoBltNoSP()
    (PBLTFUNC) &bltDSno,    // MERGEPAINT
    (PBLTFUNC) &bltS,       // SRCCOPY
    (PBLTFUNC) &bltSDno,
    (PBLTFUNC) &bltDSo,     // SRCPAINT
    (PBLTFUNC) &bltNop,     // WHITENESS is supported by sli_DoBltNoSP()
};

// VRAM/system memory to VRAM transparent blt, no stretch/shrink

PBLTFUNC xBltFuncTab[16] =
{
    (PBLTFUNC) &bltNop,     // BLACKNESS is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSon,   // NOTSRCERASE
    (PBLTFUNC) &xBltDSna,
    (PBLTFUNC) &xBltSn,     // NOTSRCCOPY
    (PBLTFUNC) &xBltSDna,   // SRCERASE
    (PBLTFUNC) &bltNop,     // DSTINVERT is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSx,    // SRCINVERT
    (PBLTFUNC) &xBltDSan,
    (PBLTFUNC) &xBltDSa,    // SRCAND
    (PBLTFUNC) &xBltDSxn,
    (PBLTFUNC) &bltNop,     // rop D is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSno,   // MERGEPAINT
    (PBLTFUNC) &xBltS,      // SRCCOPY
    (PBLTFUNC) &xBltSDno,
    (PBLTFUNC) &xBltDSo,    // SRCPAINT
    (PBLTFUNC) &bltNop,     // WHITENESS is supported by sli_DoBltNoSP()
};

// VRAM/system memory to VRAM opaque blt, stretch/shrink

PBLTFUNC sBltFuncTab[16] =
{
    (PBLTFUNC) &bltNop,     // BLACKNESS is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSon,   // NOTSRCERASE
    (PBLTFUNC) &xBltDSna,
    (PBLTFUNC) &xBltSn,     // NOTSRCCOPY
    (PBLTFUNC) &xBltSDna,   // SRCERASE
    (PBLTFUNC) &bltNop,     // DSTINVERT is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSx,    // SRCINVERT
    (PBLTFUNC) &xBltDSan,
    (PBLTFUNC) &xBltDSa,    // SRCAND
    (PBLTFUNC) &xBltDSxn,
    (PBLTFUNC) &bltNop,     // rop D is supported by sli_DoBltNoSP()
    (PBLTFUNC) &xBltDSno,   // MERGEPAINT
    (PBLTFUNC) &sBltS,      // SRCCOPY
    (PBLTFUNC) &xBltSDno,
    (PBLTFUNC) &xBltDSo,    // SRCPAINT
    (PBLTFUNC) &bltNop,     // WHITENESS is supported by sli_DoBltNoSP()
};


/***************************************************************************/
/*                       INTERNAL FUNCTIONS                                */
/***************************************************************************/

void __stdcall xBltDSa(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSan(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSna(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSno(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSo(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSon(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSx(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltDSxn(BLT_PARAMS)
{
    NOT_SUPPORTED
}

void __stdcall xBltS(BLT_PARAMS)
{
    CKEY_TYPE colorKey;
    DWORD srcColorkeyMin, srcColorkeyMax;
    DWORD dstColorkeyMin, dstColorkeyMax;
    DWORD dwData, i;
    WORD  wData;
    BYTE *pSrcData, *pDstData, bData, *pData;

    colorKey = none;

    if (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE)
    {
        colorKey = src;
        srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    }

    if (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE)
    {
        colorKey = (src==colorKey)? both : dst;
        dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
        dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    }

    if (src == colorKey)    // do not write if src pixel matches color key
    {
        switch(srcBytesPerPel)
        {
        case 1: // 8bpp
            while (dstHeight--)
            {
                memcpy(scanBuf1, srcPtr, dstWidthBytes);
                pDstData = dstPtr;
                for (i=0; i < dstWidthBytes; i++)
                {
                    bData = scanBuf1[i];
                    if ((bData < srcColorkeyMin) || (bData > srcColorkeyMax))
                    {
                        *((BYTE *)pDstData) = bData;
                    }
                    ((BYTE *)pDstData)++;
                }
                NEXT_SCAN
            }
            break;

        case 2: // 16bpp
            while (dstHeight--)
            {
                memcpy(scanBuf1, srcPtr, dstWidthBytes);
                pDstData = dstPtr;
                pData = scanBuf1;
                for (i=0; i < (dstWidthBytes >> 1); i++)
                {
                    wData = *((WORD *)pData)++;
                    if ((wData < srcColorkeyMin) || (wData > srcColorkeyMax))
                    {
                        *((WORD *)pDstData) = wData;
                    }
                    ((WORD *)pDstData)++;
                }
                NEXT_SCAN
            }
            break;

        case 4: // 32bpp
            while (dstHeight--)
            {
                memcpy(scanBuf1, srcPtr, dstWidthBytes);
                pDstData = dstPtr;
                pData = scanBuf1;
                for (i=0; i < (dstWidthBytes >> 2); i++)
                {
                    dwData = *((DWORD *)pData)++ & 0xffffff;
                    if ((dwData < srcColorkeyMin) || (dwData > srcColorkeyMax))
                    {
                        *((DWORD *)pDstData) = dwData;
                    }
                    ((DWORD *)pDstData)++;
                }
                NEXT_SCAN
            }
            break;
        } // end: switch(bytesPerPel)
    } // endif: use src color key
    else
    {
        if (dst == colorKey ) // update dst. pixel if matches color key
        {
            switch(dstBytesPerPel)
            {
            case 1: // 8bpp
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < dstWidthBytes; i++)
                    {
                        bData = *((BYTE *)pData)++;
                        if ((bData >= dstColorkeyMin) &&
                            (bData <= dstColorkeyMax)
                           )
                        {
                            *((BYTE *)pDstData) = *((BYTE *)pSrcData);
                        }
                        ((BYTE *)pDstData)++;
                        ((BYTE *)pSrcData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 2: // 16bpp
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 1); i++)
                    {
                        wData = *((WORD *)pData)++;
                        if ((wData >= dstColorkeyMin) &&
                            (wData <= dstColorkeyMax)
                           )
                        {
                            *((WORD *)pDstData) = *((WORD *)pSrcData);
                        }
                        ((WORD *)pDstData)++;
                        ((WORD *)pSrcData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 4: // 32bpp
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 2); i++)
                    {
                        dwData = *((DWORD *)pData)++ & 0xffffff;
                        if ((dwData >= dstColorkeyMin) &&
                            (dwData <= dstColorkeyMax)
                           )
                        {
                            *((DWORD *)pDstData) = *((DWORD *)pSrcData);
                        }
                        ((DWORD *)pDstData)++;
                        ((DWORD *)pSrcData)++;
                    }
                    NEXT_SCAN
                }
                break;
            } // end: switch(bytesPerPel)
        } // endif: use dst color key
        else
        {   // use both src and dst color key

            switch(dstBytesPerPel)
            {
            case 1:
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < dstWidthBytes; i++)
                    {
                        bData = *((BYTE *)pSrcData);
                        if ((bData < srcColorkeyMin) || (bData > srcColorkeyMax))
                        {
                            bData = *((BYTE *)pData)++;
                            if ((bData >= dstColorkeyMin) &&
                                (bData <= dstColorkeyMax)
                               )
                            {
                                *((BYTE *)pDstData) = *((BYTE *)pSrcData);
                            }
                         }
                         ((BYTE *)pSrcData)++;
                         ((BYTE *)pDstData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 2:
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 1); i++)
                    {
                        wData = *((WORD *)pSrcData);
                        if ((wData < srcColorkeyMin) || (wData > srcColorkeyMax))
                        {
                            wData = *((WORD *)pData)++;
                            if ((wData >= dstColorkeyMin) &&
                                (wData <= dstColorkeyMax)
                               )
                            {
                                *((WORD *)pDstData) = *((WORD *)pSrcData);
                            }
                         }
                         ((WORD *)pSrcData)++;
                         ((WORD *)pDstData)++;
                    }
                    NEXT_SCAN
                }
                break;

            case 4:
                while (dstHeight--)
                {
                    memcpy(scanBuf1, srcPtr, dstWidthBytes);
                    memcpy(scanBuf2, dstPtr, dstWidthBytes);
                    pSrcData = scanBuf1;
                    pData = scanBuf2;
                    pDstData = dstPtr;
                    for (i=0; i < (dstWidthBytes >> 1); i++)
                    {
                        dwData = *((DWORD *)pSrcData) & 0xffffff;
                        if ((dwData < srcColorkeyMin) || (dwData > srcColorkeyMax))
                        {
                            dwData = *((DWORD *)pData)++ & 0xffffff;
                            if ((dwData >= dstColorkeyMin) &&
                                (dwData <= dstColorkeyMax)
                               )
                            {
                                *((DWORD *)pDstData) = *((DWORD *)pSrcData);
                            }
                         }
                         ((DWORD *)pSrcData)++;
                         ((DWORD *)pDstData)++;
                    }
                    NEXT_SCAN
                }
                break;
            } // end: switch(bytesPerPel)
        } // endif: use src and dst color key
    } // endif: use src color key
}

void __stdcall xBltSDna(BLT_PARAMS)
{
}

void __stdcall xBltSDno(BLT_PARAMS)
{
}

void __stdcall xBltSn(BLT_PARAMS)
{
}

/*--------------------------------------------------------------------*/
void __stdcall bltDSa(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ &= *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ &= *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSan(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ & *((DWORD *)pDstData) );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ & *((BYTE *)pDstData) );
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSna(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ ) & *((DWORD *)pDstData);
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ ) & *((BYTE *)pDstData);
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSno(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ ) | *((DWORD *)pDstData);
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ ) | *((BYTE *)pDstData);
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSo(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ |= *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ |= *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSon(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ | *((DWORD *)pDstData) );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ | *((BYTE *)pDstData) );
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSx(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ ^= *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ ^= *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltDSxn(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ ^ *((DWORD *)pDstData) );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ ^ *((BYTE *)pDstData) );
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltNop(BLT_PARAMS)
{
    /* nothing to do, hurry home */
}

void __stdcall bltS(BLT_PARAMS)
{
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(dstPtr, scanBuf1, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltSDna(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pDstData) ) & *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pDstData) ) & *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltSDno(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        memcpy(scanBuf2, dstPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = scanBuf2;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pDstData) ) | *((DWORD *)pSrcData)++;
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pDstData) ) | *((BYTE *)pSrcData)++;
        }
        memcpy(dstPtr, scanBuf2, dstWidthBytes);
        NEXT_SCAN
    }
}

void __stdcall bltSn(BLT_PARAMS)
{
    DWORD rightEdge, i;
    BYTE *pSrcData, *pDstData;

    rightEdge = dstWidthBytes & 3;
    while (dstHeight--)
    {
        memcpy(scanBuf1, srcPtr, dstWidthBytes);
        pSrcData = scanBuf1;
        pDstData = dstPtr;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pDstData)++ =
                ~( *((DWORD *)pSrcData)++ );
        }
        for (i=0; i < rightEdge; i++)
        {
            *((BYTE *)pDstData)++ =
                ~( *((BYTE *)pSrcData)++ );
        }
        NEXT_SCAN
    }
}
/*--------------------------------------------------------------------*/


/*----------------------------------------------------------------------
Function name:  sli_ColorFill

Description:    Fill a rectangular region of a surface.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
sli_ColorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD BytesPerPel)
{
    FxU32 color;
    DWORD dstWidth, dstHeight, dstPitch, dstWidthBytes;
    DWORD i;
    BYTE  *dstPtr, *pData;
    LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf;

    FXBUSYWAIT(ppdev);

    dstWidth = pbd->rDest.right - pbd->rDest.left;
    dstHeight = pbd->rDest.bottom - pbd->rDest.top;
    lpDstSurf = pbd->lpDDDestSurface;
    dstWidthBytes = dstWidth * BytesPerPel;

    dstPitch = GET_STRIDE(lpDstSurf);

    dstPtr = (BYTE *)
        ( ((FXSURFACEDATA*)pbd->lpDDDestSurface->lpGbl->dwReserved1)->phantomlfbPtr +
          (pbd->rDest.top * dstPitch) + (pbd->rDest.left * BytesPerPel)
        );

    color = pbd->bltFX.dwFillColor;

    switch(BytesPerPel)
    {
    case 1: // 8bpp
        memset(scanBuf1, color, dstWidthBytes);
        break;

    case 2: // 16bpp
        pData = scanBuf1;
        for (i=0; i < (dstWidthBytes >> 1); i++)
        {
            *((WORD *)pData)++ = (WORD)color;
        }
        break;

    case 4: // 32bpp
        pData = scanBuf1;
        for (i=0; i < (dstWidthBytes >> 2); i++)
        {
            *((DWORD *)pData)++ = (DWORD)color;
        }
        break;
    }

    while (dstHeight--)
    {
        memcpy(dstPtr, scanBuf1, dstWidthBytes);
        dstPtr += dstPitch; // next scan
    }

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // sli_ColorFill


/*----------------------------------------------------------------------
Function name:  sli_DoBltNoSP

Description:    Handle blts that don't use a source or pattern.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall
sli_DoBltNoSP(NT9XDEVICEDATA   *ppdev,
              LPDDHAL_BLTDATA   pbd,
              DWORD             rop3,
              DWORD             BytesPerPel)
{
  DWORD dstWidth, dstHeight, dstPitch, dstWidthBytes;
  DWORD rightEdge, i;
  BYTE  *dstPtr, *pData;
  LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf;

  FXBUSYWAIT(ppdev);

  if (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE)
  {
    NOT_SUPPORTED   // destination color key
  }
  else
  {
    switch(rop3)
    {
    case ( HIWORD(BLACKNESS) ):
        pbd->bltFX.dwFillColor = rop3;
        sli_ColorFill(ppdev, pbd, BytesPerPel);
        break;

    case ( HIWORD(WHITENESS) ):
        pbd->bltFX.dwFillColor = 0xffffffff;
        sli_ColorFill(ppdev, pbd, BytesPerPel);
        break;

    case ( HIWORD(DSTINVERT) ):
        dstWidth = pbd->rDest.right - pbd->rDest.left;
        dstHeight = pbd->rDest.bottom - pbd->rDest.top;
        lpDstSurf = pbd->lpDDDestSurface;
        dstWidthBytes = dstWidth * BytesPerPel;

        dstPitch = GET_STRIDE(lpDstSurf);

        dstPtr = (BYTE *)
            ( ((FXSURFACEDATA*)pbd->lpDDDestSurface->lpGbl->dwReserved1)->phantomlfbPtr +
              (pbd->rDest.top * dstPitch) + (pbd->rDest.left * BytesPerPel)
            );

        rightEdge = dstWidthBytes & 3;

        while (dstHeight--)
        {
            memcpy(scanBuf1, dstPtr, dstWidthBytes);
            pData = scanBuf1;
            for (i=0; i < (dstWidthBytes >> 2); i++)
            {
                *((DWORD *)pData)++ = ~(*((DWORD *)pData));
            }
            for (i=0; i < rightEdge; i++)       // invert right edge
            {
                *((BYTE *)pData)++ = ~(*((BYTE *)pData));
            }
            memcpy(dstPtr, scanBuf1, dstWidthBytes);
            dstPtr += dstPitch;                 // next scan
        }
        break;

    case (0xaa):
        break;
    } // end: switch(rop3)
  } // endif: dst color key specified

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // sli_DoBltNoSP

/*----------------------------------------------------------------------
Function name:  xsBltSrcToScanBuf

Description:    Apply color keys, then performs pixel replication in X,
                put result in stretch buffer.

Return:         void
----------------------------------------------------------------------*/
void __stdcall xsBltSrcToScanBuf(
    LPDDHAL_BLTDATA pbd, BYTE *srcPtr, BYTE *dstPtr,
    DWORD srcWidth, DWORD dstWidth,
    DWORD srcBytesPerPel, DWORD dstBytesPerPel)
{
    CKEY_TYPE colorKey;
    DWORD srcColorkeyMin, srcColorkeyMax;
    DWORD dstColorkeyMin, dstColorkeyMax, dwData;
    WORD  wData;
    int   twoSrcX, twoDstX, xErr;
    BYTE  *pSrc, *pDst, bData;
    DWORD srcWidthBytes, dstWidthBytes;

    colorKey = none;

    if (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE)
    {
        colorKey = src;
        srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    }

    if (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE)
    {
        colorKey = (src==colorKey)? both : dst;
        dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
        dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
    }

    // replicate in X, result is in scan buffer 2

    srcWidthBytes = srcWidth * srcBytesPerPel;
    dstWidthBytes = dstWidth * dstBytesPerPel;
    twoSrcX = (int) (srcWidth + srcWidth);
    twoDstX = (int) (dstWidth + dstWidth);
    xErr = twoSrcX + (int)srcWidth - twoDstX;

    pSrc = (BYTE *)scanBuf1;
    pDst = (BYTE *)scanBuf2;
    memcpy(scanBuf1, srcPtr, srcWidthBytes); // read source into buffer 1
    memcpy(scanBuf2, dstPtr, dstWidthBytes); // read dst into buffer 2

    if (src == colorKey)    // do not write if src pixel matches color key
    {
        switch(srcBytesPerPel)
        {
        case 1: // 8bpp
            bData = *((BYTE *)pSrc);
            while (dstWidth--)
            {
                if ((bData < srcColorkeyMin) || (bData > srcColorkeyMax))
                {
                    *((BYTE *)pDst) = bData;
                }
                ((BYTE *)pDst)++;

                while (xErr >= 0)
                {
                    bData = * (++((BYTE *)pSrc));
                    xErr -= twoDstX;
                }
                xErr += twoSrcX;
            }
            break;

        case 2: // 16bpp
            wData = *((WORD *)pSrc);
            while (dstWidth--)
            {
                if ((wData < srcColorkeyMin) || (wData > srcColorkeyMax))
                {
                    *((WORD *)pDst) = wData;
                }
                ((WORD *)pDst)++;

                while (xErr >= 0)
                {
                    wData = * (++((WORD *)pSrc));
                    xErr -= twoDstX;
                }
                xErr += twoSrcX;
            }
            break;

        case 4: // 32bpp
            dwData = *((DWORD *)pSrc) & 0xffffff;
            while (dstWidth--)
            {
                if ((dwData < srcColorkeyMin) || (dwData > srcColorkeyMax))
                {
                    *((DWORD *)pDst) = dwData;
                }
                ((DWORD *)pDst)++;

                while (xErr >= 0)
                {
                    dwData = * (++((DWORD *)pSrc)) & 0xffffff;
                    xErr -= twoDstX;
                }
                xErr += twoSrcX;
            }
            break;

        } // end: switch(srcBytesPerPel)
    } // endif: use src color key
    else
    {
        if (dst == colorKey ) // update dst. pixel if matches color key
        {
            switch(srcBytesPerPel)
            {
            case 1: // 8bpp
                while (dstWidth--)
                {
                    bData = *((BYTE *)pDst);
                    if ((bData >= dstColorkeyMin) &&
                        (bData <= dstColorkeyMax)
                       )
                    {
                        *((BYTE *)pDst) = *((BYTE *)pSrc);
                    }
                    ((BYTE *)pDst)++;

                    while (xErr >= 0)
                    {
                        ((BYTE *)pSrc)++;
                        xErr -= twoDstX;
                    }
                    xErr += twoSrcX;
                }
                break;

            case 2: // 16bpp
                while (dstWidth--)
                {
                    wData = *((WORD *)pDst);
                    if ((wData >= dstColorkeyMin) &&
                        (wData <= dstColorkeyMax)
                       )
                    {
                        *((WORD *)pDst) = *((WORD *)pSrc);
                    }
                    ((WORD *)pDst)++;

                    while (xErr >= 0)
                    {
                        ((WORD *)pSrc)++;
                        xErr -= twoDstX;
                    }
                    xErr += twoSrcX;
                }
                break;

            case 4: // 32bpp
                while (dstWidth--)
                {
                    dwData = *((DWORD *)pDst);
                    if ((dwData >= dstColorkeyMin) &&
                        (dwData <= dstColorkeyMax)
                       )
                    {
                        *((DWORD *)pDst) = *((DWORD *)pSrc);
                    }
                    ((DWORD *)pDst)++;

                    while (xErr >= 0)
                    {
                        ((DWORD *)pSrc)++;
                        xErr -= twoDstX;
                    }
                    xErr += twoSrcX;
                }
                break;

            } // end: switch(bytesPerPel)
        } // endif: use dst color key
    } // endif: use src color key
}

/*----------------------------------------------------------------------
Function name:  sBltSrcToScanBuf

Description:    Performs pixel replication in X, put result in
                stretch buffer.

Return:         void
----------------------------------------------------------------------*/
void __stdcall sBltSrcToScanBuf(
    LPDDHAL_BLTDATA  pbd, BYTE *srcPtr, BYTE *dstPtr,
    DWORD srcWidth, DWORD dstWidth,
    DWORD srcBytesPerPel, DWORD dstBytesPerPel)
{
    int   twoSrcX, twoDstX, xErr;
    BYTE  *pSrc, *pDst;
    DWORD srcWidthBytes;

    // replicate in X, result is in scan buffer 2

    srcWidthBytes = srcWidth * srcBytesPerPel;
    twoSrcX = (int) (srcWidth + srcWidth);
    twoDstX = (int) (dstWidth + dstWidth);
    xErr = twoSrcX + (int)srcWidth - twoDstX;

    pSrc = (BYTE *)scanBuf1;
    pDst = (BYTE *)scanBuf2;
    memcpy(scanBuf1, srcPtr, srcWidthBytes);

    switch(srcBytesPerPel)
    {
    case 1: // 8bpp
        while (dstWidth--)
        {
            *((BYTE *)pDst)++ = *((BYTE *)pSrc);

            while (xErr >= 0)
            {
                ((BYTE *)pSrc)++;
                xErr -= twoDstX;
            }
            xErr += twoSrcX;
        }
        break;

    case 2: // 16bpp
        while (dstWidth--)
        {
            *((WORD *)pDst)++ = *((WORD *)pSrc);

            while (xErr >= 0)
            {
                ((WORD *)pSrc)++;
                xErr -= twoDstX;
            }
            xErr += twoSrcX;
        }
        break;

    case 4: // 32bpp
        while (dstWidth--)
        {
            *((DWORD *)pDst)++ = *((DWORD *)pSrc);

            while (xErr >= 0)
            {
                ((DWORD *)pSrc)++;
                xErr -= twoDstX;
            }
            xErr += twoSrcX;
        }
        break;

    } // end: switch(srcBytesPerPel)
}

/*----------------------------------------------------------------------
Function name:  sBltS

Description:    Performs pixel replication for stretch blt.

Return:         void
----------------------------------------------------------------------*/
void __stdcall sBltS(BLT_PARAMS)
{
    int   twoSrcY, twoDstY, yErr;
    DWORD srcWidth, dstWidth, srcHeight;
    PSBLTFUNC pfn;

    srcWidth = pbd->rSrc.right - pbd->rSrc.left;
    dstWidth = pbd->rDest.right - pbd->rDest.left;
    srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

    twoSrcY = (int) (srcHeight + srcHeight);
    twoDstY = (int) (dstHeight + dstHeight);
    yErr = twoSrcY + (int)srcHeight - twoDstY;

    if ( (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE) ||
         (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE) )
    {
        pfn = (PSBLTFUNC) &xsBltSrcToScanBuf;   // apply color keys
    }
    else
    {
        pfn = (PSBLTFUNC) &sBltSrcToScanBuf;    // no color keys
    }

    // stretch or shrink one scan, store result in scanline buffer

    (*pfn)(pbd, srcPtr, dstPtr, srcWidth, dstWidth, srcBytesPerPel, dstBytesPerPel);

    while (dstHeight--)
    {
        memcpy(dstPtr, scanBuf2, dstWidthBytes); // replicate in Y
        dstPtr += dstPitch;

        while (yErr >= 0)
        {
            srcPtr += srcPitch; // advance to next source scan
            yErr -= twoDstY;

            // stretch or shrink next scan, store result in scanline buffer

            (*pfn)(pbd, srcPtr, dstPtr, srcWidth, dstWidth, srcBytesPerPel, dstBytesPerPel);
        }
        yErr += twoSrcY;

    } // while (dstHeight--)
}

/*----------------------------------------------------------------------
Function name:  sli_DoBltS

Description:    Handle blts that use a source only, no pattern. Source
                can be from video memory or system memory. Destination
                is currently only in video memory.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
sli_DoBltS(NT9XDEVICEDATA   *ppdev,
           LPDDHAL_BLTDATA  pbd,
           DWORD            rop3,
           DWORD            srcPixelFormat,
           DWORD            dstPixelFormat,
           BLT_TYPE         blt_type)
{
  DWORD dstWidth, dstHeight, srcWidth, srcHeight;
  DWORD srcWidthBytes, dstWidthBytes;
  DWORD srcBytesPerPel, dstBytesPerPel, srcPitch, dstPitch, i;
  BYTE  *dstPtr, *srcPtr;
  LPDDRAWI_DDRAWSURFACE_LCL lpDstSurf, lpSrcSurf;

  FXBUSYWAIT(ppdev);

  GET_BYTE_DEPTH(srcBytesPerPel, srcPixelFormat);
  GET_BYTE_DEPTH(dstBytesPerPel, dstPixelFormat);

#ifdef DEBUG
  if (srcBytesPerPel != dstBytesPerPel)
  {
    NOT_SUPPORTED   // we will add support as required
  }
#endif

  srcWidth = pbd->rSrc.right - pbd->rSrc.left;
  srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;
  lpSrcSurf = pbd->lpDDSrcSurface;
  srcWidthBytes = srcWidth * srcBytesPerPel;

  if (vid2vid == blt_type)
  {
    srcPitch = GET_STRIDE(lpSrcSurf); // valid only if source is from video mem.
  }
  else
  {
    srcPitch = lpSrcSurf->lpGbl->lPitch;
  }

  dstWidth = pbd->rDest.right - pbd->rDest.left;
  dstHeight = pbd->rDest.bottom - pbd->rDest.top;
  lpDstSurf = pbd->lpDDDestSurface;
  dstWidthBytes = dstWidth * dstBytesPerPel;

  dstPitch = GET_STRIDE(lpDstSurf);

  // Check for overlaps. Since we are reading source into a scanline buffer
  // for performance reasons, we do not need to worry about overlaping in
  // the x direction.

  if ( (pbd->rDest.top >= pbd->rSrc.top) && (pbd->rDest.top <= pbd->rSrc.bottom) )
  {
    // copy from bottom up

    srcPtr = (BYTE *)
        ( GET_LFBPTR(pbd->lpDDSrcSurface) + ((pbd->rSrc.bottom - 1) * srcPitch) +
          (pbd->rSrc.left * srcBytesPerPel)
        );
    dstPtr = (BYTE *)
        ( GET_LFBPTR(pbd->lpDDDestSurface) + ((pbd->rDest.bottom - 1) * dstPitch) +
          (pbd->rDest.left * dstBytesPerPel)
        );
    srcPitch = 0 - srcPitch;
    dstPitch = 0 - dstPitch;
  }
  else
  {
    srcPtr = (BYTE *)
        ( GET_LFBPTR(pbd->lpDDSrcSurface) +
          (pbd->rSrc.top * srcPitch) + (pbd->rSrc.left * srcBytesPerPel)
        );
    dstPtr = (BYTE *)
        ( GET_LFBPTR(pbd->lpDDDestSurface) +
          (pbd->rDest.top * dstPitch) + (pbd->rDest.left * dstBytesPerPel)
        );
  }

  // check for stretch/shrink blt

  i = rop3 & 0xf;
  if ( (srcWidth == dstWidth) && (srcHeight == dstHeight) )
  {
    if ( (pbd->dwFlags & DDBLT_KEYSRCOVERRIDE) ||
         (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE) )
    {
        (* xBltFuncTab[i])(pbd, dstWidthBytes, dstHeight, srcPitch, dstPitch,
            srcPtr, dstPtr, srcBytesPerPel, dstBytesPerPel);
    }
    else
    {
        (* BltFuncTab[i])(pbd, dstWidthBytes, dstHeight, srcPitch, dstPitch,
            srcPtr, dstPtr, srcBytesPerPel, dstBytesPerPel);
    }
  }
  else // stretch/shrink blt
  {
        (* sBltFuncTab[i])(pbd, dstWidthBytes, dstHeight, srcPitch, dstPitch,
            srcPtr, dstPtr, srcBytesPerPel, dstBytesPerPel);

  } // endif: non stretch/shrink blt

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // sli_DoBltS

#endif

