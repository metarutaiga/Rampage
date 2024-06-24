/* -*-c++-*- */
/* $Header: ddsli2d.c, 12, 12/7/00 9:43:22 AM PST, Ryan Bissell$ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** File Name: 	ddsli2d.c
**
** Description: This file has all of the 2D routines to work on distributed
**  surfaces aka Sli surfaces.
**
** $Revision: 12$
** $Date: 12/7/00 9:43:22 AM PST$
**
** $History: $
**
*/

#include "precomp.h"
#include "sst2glob.h"
#include "sst2bwe.h"
#include "fifomgr.h"
#include "regkeys.h"
#include "ddcam.h"

#ifdef SLI
#include <ddsli2d.h>
#endif

#define YMAX()          (_DD(dwMaxHeight))
#define YCRUNCH(Y)       (((Y) >> _DD(dwLog2GroupHeight) << _DD(dwLog2BandHeight)) + ((Y) & (_DD(dwBandHeight) - 1)))
#define YUNIT(Y)         (((Y) >> _DD(dwLog2BandHeight)) & _FF(dwNumSlaves))
#define SAVECAM(CAM)       \
   { \
      CAM.baseAddress = GET(_FF(lpCRegs)->cam[0].baseAddress); \
      CAM.endAddress = GET(_FF(lpCRegs)->cam[0].endAddress); \
      CAM.physicalBase = GET(_FF(lpCRegs)->cam[0].physicalBase); \
      CAM.strideState = GET(_FF(lpCRegs)->cam[0].strideState); \
      SETDW(_FF(lpCRegs)->cam[0].baseAddress, 0x0); \
      SETDW(_FF(lpCRegs)->cam[0].endAddress, 0x0); \
      SETDW(_FF(lpCRegs)->cam[0].physicalBase, 0x0); \
      SETDW(_FF(lpCRegs)->cam[0].strideState, 0x0); \
   }   

#define RESTORECAM(CAM)       \
   { \
      SETDW(_FF(lpCRegs)->cam[0].baseAddress, CAM.baseAddress); \
      SETDW(_FF(lpCRegs)->cam[0].endAddress, CAM.endAddress); \
      SETDW(_FF(lpCRegs)->cam[0].physicalBase, CAM.physicalBase); \
      SETDW(_FF(lpCRegs)->cam[0].strideState, CAM.strideState); \
   }   

#ifdef CMDFIFO
#define SLI_UNIT_SELECT(SaveSLIBroadcast, ChipMask, CMDFIFO) \
   { \
      P6FENCE; \
      SaveSLIBroadcast = GET(ghwIO->sliBroadcast); \
      if ((DWORD)CMDFIFO != CMDFIFOSTART) \
         { \
         CMDFIFO_CHECKROOM(CMDFIFO, CMDFIFOSPACE + 1); \
         } \
      SAVECAM(CAM); \
      SETDW(ghwIO->sliBroadcast, (ChipMask << SST_SLI_EN0_MEMBASE1_SHIFT | SST_SLI_ENDBG_MEMBASE1)); \
   };
#else
#define SLI_UNIT_SELECT(SaveSLIBroadcast, ChipMask, CMDFIFO) \
   { \
      SaveSLIBroadcast = GET(ghwIO->sliBroadcast); \
      SETDW(ghwIO->sliBroadcast, (ChipMask << SST_SLI_WEN0_MEMBASE0_SHIFT)); \
   };
#endif

#ifdef CMDFIFO
#define SLI_UNIT_DESELECT(SaveSLIBroadcast, CMDFIFO) \
   { \
      P6FENCE; \
      if ((DWORD)CMDFIFO != CMDFIFOSTART) \
         { \
         CMDFIFO_CHECKROOM(CMDFIFO, CMDFIFOSPACE + 1); \
         } \
      SETDW(ghwIO->sliBroadcast, SaveSLIBroadcast); \
      RESTORECAM(CAM); \
   };
#else
#define SLI_UNIT_DESELECT(SaveSLIBroadcast, CMDFIFO) \
   { \
      SETDW(ghwIO->sliBroadcast, SaveSLIBroadcast); \
   };
#endif

/*----------------------------------------------------------------------
Function name: SliClear

Description:   This is the high level routine to clear Sli surfaces.

Notes:         Make sure to add clean stuff around this call....
Return:        None  

----------------------------------------------------------------------*/
HRESULT SliClear(RC * pRc, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth, DWORD dwFillStencil, RECT *pRects, DWORD count)
{
   SETUP_PPDEV(pRc)
   DWORD i;
   FxU32 pixelByteDepth;
   DWORD dstPixelFormat;
   long dstPitch;
   DWORD fillData1;
   DWORD fillData2;
   SOLIDCOLORPARAMS SolidColorParams; 
   FXSURFACEDATA * pSurfaceData;
   FXSURFACEDATA * pSurfaceDataZ;


   for (i = 0; i < count; ++i)
      {
      SolidColorParams.dstLeft = pRects[i].left;
      SolidColorParams.dstTop = pRects[i].top;
      SolidColorParams.dstRight = pRects[i].right;
      SolidColorParams.dstBottom = pRects[i].bottom;

      // If this is AA surface it will be double width and height
      if (pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING)
      {
        SolidColorParams.dstRight <<= 1;
        SolidColorParams.dstBottom <<= 1;
      }         

      if (((dwFlags & D3DCLEAR_ZBUFFER) | (dwFlags & D3DCLEAR_STENCIL)) && (pRc->DDSZHndl != 0))
      {
         pSurfaceDataZ = TXTRHNDL_TO_SURFDATA( pRc, pRc->DDSZHndl );
         pRc->lpSurfDataZ = pSurfaceDataZ;

         if (pSurfaceDataZ->dwFlags & FXSURFACE_IS_DISTRIBUTED)
            SolidColorParams.dwSliSurface = TRUE;
         else
            SolidColorParams.dwSliSurface = FALSE;

         SolidColorParams.bltDstBaseAddr = pSurfaceDataZ->hwPtr;

         pixelByteDepth = pSurfaceDataZ->dwBytesPerPixel;

         GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
         BLTFMT_STRIDESTATE(pRc->lpSurfDataZ, dstPitch);
         BLTFMT_DST(dstPitch, dstPixelFormat, SolidColorParams.bltDstFormat);

         if ((pRc->stencilInUse) && ((D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL) != (dwFlags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))))
         {
            // This is a two step operation
            // we are going to do a AND and then a OR
            // the AND mask will be either NOT STENCIL or NOT Z-Buffer and the
            // OR mask will be STENCIL or Z-BUFFER Data
            if (dwFlags & D3DCLEAR_ZBUFFER)
            {
               fillData1 = ((0xFF << 24) & pRc->lpSurfDataZ->dwZBitMask);
               fillData2 = (float2int(dvFillDepth * pRc->lpSurfDataZ->dwZBitMask));
            }
            else
            {
               fillData1 = ((0x00FFFFFF) & pRc->lpSurfDataZ->dwZBitMask);
               fillData2 = ((dwFillStencil << 24) & pRc->lpSurfDataZ->dwZBitMask);
            }

            // Clear the Stencil or Z-Buffer
            SolidColorParams.fillData =  fillData1;
            SolidColorParams.bltRop = (SST_WX_ROP_AND << 16) | (SST_WX_ROP_AND << 8) | (SST_WX_ROP_AND);
            DdSli2DSolidColor(ppdev, &SolidColorParams);
      
            // Fill the Stencil or Z-Buffer
            SolidColorParams.fillData =  fillData2;
            SolidColorParams.bltRop = (SST_WX_ROP_OR << 16) | (SST_WX_ROP_OR << 8) | (SST_WX_ROP_OR);
            DdSli2DSolidColor(ppdev, &SolidColorParams);
         }
         else
         {
            SolidColorParams.bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | (SST_WX_ROP_SRC );
            SolidColorParams.fillData =  ((dwFillStencil << 24) & pRc->lpSurfDataZ->dwZBitMask) |   // Stencil value
                         (float2int(dvFillDepth * pRc->lpSurfDataZ->dwZBitMask));         // Z value

            // Doit the operation
            DdSli2DSolidColor(ppdev, &SolidColorParams);
         }    
      }

      if( (dwFlags & D3DCLEAR_TARGET) && (pRc->DDSHndl != 0))
      {

         pSurfaceData = pRc->lpSurfData;
         if (pSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
            SolidColorParams.dwSliSurface = TRUE;
         else
            SolidColorParams.dwSliSurface = FALSE;

         SolidColorParams.bltDstBaseAddr = pSurfaceData->hwPtr;

         pixelByteDepth = GETPRIMARYBYTEDEPTH;
         if (pixelByteDepth == 2)            // if 16 bpp
         {
            SolidColorParams.fillData = dwFillColor;  // Assume fill color is rgb888 always
		      SolidColorParams.fillData = (((SolidColorParams.fillData & 0xF80000) >> 8) | ((SolidColorParams.fillData & 0xFC00) >> 5) | ((SolidColorParams.fillData & 0xF8) >> 3));
         }
         else if (pixelByteDepth == 4)       // if 32 bpp
         {
            SolidColorParams.fillData = dwFillColor;
         }
         else                                // An error exists - non supported color format
         {
            return D3DERR_UNSUPPORTEDCOLOROPERATION;
         }

         GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
         BLTFMT_STRIDESTATE(pRc->lpSurfData, dstPitch);
         BLTFMT_DST(dstPitch, dstPixelFormat, SolidColorParams.bltDstFormat);
         // Doit the operation
         SolidColorParams.bltRop = (SST_WX_ROP_SRC << 16) | (SST_WX_ROP_SRC << 8) | (SST_WX_ROP_SRC);
         DdSli2DSolidColor(ppdev, &SolidColorParams);
         }

      }

   return D3D_OK;
}


/*----------------------------------------------------------------------
Function name: Sli_ColorFill

Description:   This is the high level routine to colorfill Sli surfaces.

Return:        None  

----------------------------------------------------------------------*/
#define BLT32_COLORFILL(arg1,arg2,arg3)               blt32_colorFill(arg1,arg2,arg3)
DWORD __stdcall BLT32_COLORFILL(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat);
DWORD Sli_ColorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat)
{
   long      dstPitch;
   SOLIDCOLORPARAMS SolidColorParams;   
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwReturn;

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

   if (pDstSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
      {
      SolidColorParams.dstTop = pbd->rDest.top;
      SolidColorParams.dstRight = pbd->rDest.right;
      SolidColorParams.dstBottom = pbd->rDest.bottom;
      SolidColorParams.dstLeft = pbd->rDest.left;
   
      SolidColorParams.bltDstBaseAddr = pDstSurfaceData->hwPtr;
     
      SolidColorParams.fillData = pbd->bltFX.dwFillColor;
      BLTFMT_STRIDESTATE(pDstSurfaceData, dstPitch);
      BLTFMT_DST(dstPitch, dstPixelFormat, SolidColorParams.bltDstFormat);
      SolidColorParams.bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | (SST_WX_ROP_SRC );
      SolidColorParams.dwSliSurface = TRUE;
   
      DdSli2DSolidColor(ppdev, &SolidColorParams);
   
      pbd->ddRVal = DD_OK;
      dwReturn = DDHAL_DRIVER_HANDLED;
      }
   else
      {
      dwReturn = BLT32_COLORFILL(ppdev, pbd, dstPixelFormat);
      }
   
   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: DdSli2DSolidColor

Description:   This is a low level routine to fill a Distributed
surface using solid color fill.

There are two cases:

1.) Easy Case --  Rectangle to fill is the whole surface.  In this case
we just need to crunch Y to account for Sli.
2.) Hard Case -- Rectangle to fill is part of the whole surface.  This
case breaks down to the following:

   a.) Fill Partial Start <a partial Fill is where the Height of Blit
   is less then BandHeight>.  There can be only one of these on one chip.
   b.) Fill Wholes.  There can be several of these and if conditions are right
   more then one chip can work in parallel.
   c.) Fill Partial End <only one on one chip>.

Return:        None  

----------------------------------------------------------------------*/
int DdSli2DSolidColor(NT9XDEVICEDATA * ppdev, PSOLIDCOLORPARAMS pParams)
{
   long  dstSliBottom;
   long  dstRawSliBottom;
   long dstRawSLITop;
   long dstSliCurrent;
   long dstSliTop;
   DWORD SaveSLIBroadCast;
   DWORD clip1min;
   DWORD clip1max;
   DWORD packetHeader;
   long dstWidth;
   long dstHeight;
   DWORD bltDstSize;
   DWORD bltDstXY;
   DWORD dwChunk;
   DWORD dwMask;
   DWORD dwGroup;
   DWORD i;
   Sst2CAMEntry CAM;

   CMDFIFO_PROLOG(cmdFifo);

   CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );
   SETMOP( cmdFifo,SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
         ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));

   //Calculate some loop invariants
   dstWidth = pParams->dstRight - pParams->dstLeft;

   if (pParams->dwSliSurface)
      {
      // Start with the most common and easist case
      // This is the case where the Blt is Full Screen so we just need to munge the
      // Height of the Blt
      if ((0x0 == pParams->dstTop) && (ppdev->bi.biHeight == pParams->dstBottom))
         {
   	  CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 );

         // write to hw
         packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;

         BLTCLIP(pParams->dstLeft, pParams->dstTop, clip1min);
         BLTCLIP(pParams->dstRight, YMAX(), clip1max);
         dstHeight = YMAX();
         BLTSIZE(dstWidth, dstHeight, bltDstSize);
         BLTXY(pParams->dstLeft, pParams->dstTop, bltDstXY);
         SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
         SETPD(cmdFifo, ghw2D->dstBaseAddr,  pParams->bltDstBaseAddr);
         SETPD(cmdFifo, ghw2D->dstFormat,    pParams->bltDstFormat);
         SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
         SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
         SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
         SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
         SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
         SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
         SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                  | ((pParams->bltRop & 0xFF) << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);

         }
      else
         {
         // There is a partial chunk at the beginning
         dstSliTop = pParams->dstTop;
         if ((pParams->dstTop & (_DD(dwBandHeight) - 1)) || (pParams->dstBottom - pParams->dstTop < (long)_DD(dwBandHeight)))
            {
            // Align on Chunk Boundary
            dstRawSLITop = pParams->dstTop & ~(_DD(dwBandHeight) - 1);
         
            SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(pParams->dstTop)), cmdFifo);

   	      CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 );
   
            // write to hw
            packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;

            dstRawSliBottom = MIN(dstRawSLITop + ((long)_DD(dwBandHeight)), pParams->dstBottom);
            dstSliCurrent = dstRawSliBottom;
            dstSliTop = YCRUNCH(pParams->dstTop);
            dstSliBottom = dstSliTop + (dstRawSliBottom - pParams->dstTop);
            BLTCLIP(pParams->dstLeft, dstSliTop, clip1min);
            BLTCLIP(pParams->dstRight, dstSliBottom, clip1max);
            dstHeight = dstSliBottom - dstSliTop;
            BLTSIZE(dstWidth, dstHeight, bltDstSize);
            BLTXY(pParams->dstLeft, dstSliTop, bltDstXY);
            SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  pParams->bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    pParams->bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
            SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
            SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
            SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
            SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);

            SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
            }
         else
            {
            dstSliCurrent = dstSliTop;
            }         

         // Here is where we need to group up the wholes Chunks.....
         dwChunk = (pParams->dstBottom - dstSliCurrent) >> _DD(dwLog2BandHeight);
         for (i=0; i<dwChunk; )
            {
            dwMask = BIT(YUNIT(dstSliCurrent));
            dstSliTop = YCRUNCH(dstSliCurrent);
            // if we start at the Master Unit then we can group dwChunk/(nSlaves+1) chunks together
            dwGroup = (dwChunk - i)/(_FF(dwNumSlaves) + 1);
            if ((BIT(_FF(dwChipID)) == dwMask) && (dwGroup))
               {
               dstSliCurrent = dwGroup * (_FF(dwNumSlaves) + 1) * _DD(dwBandHeight) + dstSliCurrent;
               dwMask = _FF(dwSlaveMask) | dwMask;
               i = i + dwGroup * (_FF(dwNumSlaves) + 1);
               dstHeight = dwGroup << _DD(dwLog2BandHeight);
               }
            else
               {
               dstSliCurrent += _DD(dwBandHeight);
               i += 1;
               while (((DWORD)dstSliTop == YCRUNCH(dstSliCurrent)) && (i < dwChunk))
                  {
                  dwMask |= BIT(YUNIT(dstSliCurrent));
                  dstSliCurrent += _DD(dwBandHeight);
                  i += 1;
                  }
               dstHeight = _DD(dwBandHeight);
               }
         
            SLI_UNIT_SELECT(SaveSLIBroadCast, dwMask, cmdFifo);

   	      CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 );

            // write to hw
            packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;
   
            BLTCLIP(pParams->dstLeft, dstSliTop, clip1min);
            BLTCLIP(pParams->dstRight, dstSliTop + dstHeight, clip1max);
            BLTSIZE(dstWidth, dstHeight, bltDstSize);
            BLTXY(pParams->dstLeft, pParams->dstTop, bltDstXY);
            SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  pParams->bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    pParams->bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
            SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
            SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
            SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
            SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                   | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);

            SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
            }

         // There is a partial chunk at the end
         if (dstSliCurrent <= pParams->dstBottom - 1)
            {
            SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(dstSliCurrent)), cmdFifo);

   	      CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 );

            // write to hw
            packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;

            dstSliBottom = YCRUNCH(pParams->dstBottom);
            dstSliTop = YCRUNCH(dstSliCurrent);
            BLTCLIP(pParams->dstLeft, dstSliTop, clip1min);
            BLTCLIP(pParams->dstRight, dstSliBottom, clip1max);
            dstHeight = dstSliBottom - dstSliTop;
            BLTSIZE(dstWidth, dstHeight, bltDstSize);
            BLTXY(pParams->dstLeft, pParams->dstTop, bltDstXY);
            SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  pParams->bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    pParams->bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
            SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
            SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
            SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
            SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                   | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);

            SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
            }
         }   
      }
   else
      {
   	CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 );

      // write to hw
      packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;

      BLTCLIP(pParams->dstLeft, pParams->dstTop, clip1min);
      BLTCLIP(pParams->dstRight, pParams->dstBottom, clip1max);
      dstHeight = pParams->dstBottom - pParams->dstTop;
      BLTSIZE(dstWidth, dstHeight, bltDstSize);
      BLTXY(pParams->dstLeft, pParams->dstTop, bltDstXY);
      SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
      SETPD(cmdFifo, ghw2D->dstBaseAddr,  pParams->bltDstBaseAddr);
      SETPD(cmdFifo, ghw2D->dstFormat,    pParams->bltDstFormat);
      SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
      SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
      SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
      SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
      SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
      SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
      SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                  | ((pParams->bltRop & 0xFF) << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);

      } 
   CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE * 2 );
   SETMOP( cmdFifo, SST_MOP_STALL_2D);
   SETMOP( cmdFifo, SST_MOP_STALL_3D |
          (SST_MOP_STALL_3D_TD << SST_MOP_STALL_3D_SEL_SHIFT) | 
           SST_MOP_FLUSH_TCACHE);

   CMDFIFO_EPILOG( cmdFifo );

   return 1;
}

/*----------------------------------------------------------------------
Function name: DdSliBltNoSP

Description:   This is a low level routine to work on a Distributed
surface.

There are two cases:

1.) Easy Case --  Rectangle to fill is the whole surface.  In this case
we just need to crunch Y to account for Sli.
2.) Hard Case -- Rectangle to fill is part of the whole surface.  This
case breaks down to the following:

   a.) Fill Partial Start <a partial Fill is where the Height of Blit
   is less then BandHeight>.  There can be only one of these on one chip.
   b.) Fill Wholes.  There can be several of these and if conditions are right
   more then one chip can work in parallel.
   c.) Fill Partial End <only one on one chip>.

Return:        None  

----------------------------------------------------------------------*/
#define BLT32_DOBLTNOSP(arg1,arg2,arg3,arg4)        blt32_doBltNoSP(arg1,arg2,arg3,arg4)
DWORD __stdcall BLT32_DOBLTNOSP(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD dstPixelFormat);
int DdSliBltNoSP(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD dstPixelFormat)
{
   long  dstSliBottom;
   long  dstRawSliBottom;
   long dstRawSLITop;
   long dstSliCurrent;
   long dstSliTop;
   DWORD SaveSLIBroadCast;
   DWORD dwChunk;
   DWORD dwMask;
   DWORD dwGroup;
   DWORD i;
   RECTL rDest;
   DWORD dwReturn;
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwSize;
   Sst2CAMEntry CAM;
   CMDFIFO_PROLOG(cmdFifo);

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);
   if (pDstSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
      {
      // Start with the most common and easist case
      // This is the case where the Blt is Full Screen so we just need to munge the
      // Height of the Blt
      rDest = pbd->rDest;
      if ((0x0 == pbd->rDest.top) && (ppdev->bi.biHeight == pbd->rDest.bottom))
         {
         pbd->rDest.bottom = YMAX();
         dwReturn = BLT32_DOBLTNOSP(ppdev, pbd, rop3, dstPixelFormat);
         }
      else
         {
         // There is a partial chunk at the beginning
         dstSliTop = pbd->rDest.top;
         if ((pbd->rDest.top & (_DD(dwBandHeight) - 1)) || (pbd->rDest.bottom - pbd->rDest.top < (long)_DD(dwBandHeight)))
            {
            // Align on Chunk Boundary
            dstRawSLITop = pbd->rDest.top & ~(_DD(dwBandHeight) - 1);
            
            SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(pbd->rDest.top)), cmdFifo);
   
            dstRawSliBottom = MIN(dstRawSLITop + ((long)_DD(dwBandHeight)), pbd->rDest.bottom);
            dstSliCurrent = dstRawSliBottom;
            dstSliTop = YCRUNCH(pbd->rDest.top);
            dstSliBottom = dstSliTop + (dstRawSliBottom - pbd->rDest.top);
   
            pbd->rDest.top = dstSliTop;
            pbd->rDest.bottom = dstSliBottom;
            CMDFIFO_SAVE(cmdFifo); 
            dwReturn = BLT32_DOBLTNOSP(ppdev, pbd, rop3, dstPixelFormat);
            CMDFIFO_RELOAD(cmdFifo);
            SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
            }
         else
            {
            dstSliCurrent = dstSliTop;
            }         
   
         // Here is where we need to group up the wholes Chunks.....
         dwChunk = (rDest.bottom - dstSliCurrent) >> _DD(dwLog2BandHeight);
         for (i=0; i<dwChunk; )
            {
            dwMask = BIT(YUNIT(dstSliCurrent));
            dstSliTop = YCRUNCH(dstSliCurrent);
            // if we start at the Master Unit then we can group dwChunk/(nSlaves+1) chunks together
            dwGroup = (dwChunk - i)/(_FF(dwNumSlaves) + 1);
            if ((BIT(_FF(dwChipID)) == dwMask) && (dwGroup))
               {
               dstSliCurrent = dwGroup * (_FF(dwNumSlaves) + 1) * _DD(dwBandHeight) + dstSliCurrent;
               dwMask = _FF(dwSlaveMask) | dwMask;
               i = i + dwGroup * (_FF(dwNumSlaves) + 1);
               dwSize = dwGroup << _DD(dwLog2BandHeight);
               }
            else
               {
               dstSliCurrent += _DD(dwBandHeight);
               i += 1;
               while (((DWORD)dstSliTop == YCRUNCH(dstSliCurrent)) && (i < dwChunk))
                  {
                  dwMask |= BIT(YUNIT(dstSliCurrent));
                  dstSliCurrent += _DD(dwBandHeight);
                  i += 1;
                  }
               dwSize = _DD(dwBandHeight);
               }
            
            SLI_UNIT_SELECT(SaveSLIBroadCast, dwMask, cmdFifo);
            dstSliBottom = dstSliTop + dwSize;
            pbd->rDest.top = dstSliTop;
            pbd->rDest.bottom = dstSliBottom;
            CMDFIFO_SAVE(cmdFifo); 
            dwReturn = BLT32_DOBLTNOSP(ppdev, pbd, rop3, dstPixelFormat);
            CMDFIFO_RELOAD(cmdFifo); 
            SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
            }
   
         // There is a partial chunk at the end
         if (dstSliCurrent <= rDest.bottom)
            {
            SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(dstSliCurrent)), cmdFifo);
            dstSliBottom = YCRUNCH(pbd->rDest.bottom);
            dstSliTop = YCRUNCH(dstSliCurrent);
            pbd->rDest.top = dstSliTop;
            pbd->rDest.bottom = dstSliBottom;
            CMDFIFO_SAVE(cmdFifo); 
            dwReturn = BLT32_DOBLTNOSP(ppdev, pbd, rop3, dstPixelFormat);
            CMDFIFO_RELOAD(cmdFifo); 
            SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
            }
         CMDFIFO_SAVE(cmdFifo);
         }   
         
      pbd->rDest = rDest;
      }
   else
      dwReturn = BLT32_DOBLTNOSP(ppdev, pbd, rop3, dstPixelFormat);

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: CamLock

Description: This function is used to lock a surface into the CAM
so that we can operate on it. 

Notes: What about wait for idle?

Return:        None  

----------------------------------------------------------------------*/
DWORD CamLock(NT9XDEVICEDATA * ppdev, LPDDRAWI_DIRECTDRAW_GBL lpDD, FXSURFACEDATA * pSurface)
{
   Sst2CAMEntry CAMEntry;
   DWORD ptr;

   if (!pSurface->phantomlfbPtr)
      {
      // Primary will use existing DibEngine CAM entry
      ptr = pSurface->lfbPtr;
      }
   else
      {
      CAMEntry.strideState  =0;

      // tile/linear modes
      if (pSurface->tileFlag & MEM_IN_TILE1)      
         CAMEntry.strideState |= SST_CAM_TILE_MODE1;
      else if (pSurface->tileFlag & MEM_IN_TILE0) 
         CAMEntry.strideState &= ~SST_CAM_TILE_MODE1;
      else                                           
         CAMEntry.strideState |= SST_CAM_LINEAR;

      // stride in physical memory
      CAMEntry.strideState |= pSurface->dwPStride   <<SST_CAM_PSTRIDE_SHIFT;

      CAMEntry.strideState |= pSurface->dwL2MStride << SST_CAM_MSTRIDE_SHIFT;

      // pixel depth
      CAMEntry.strideState |= pSurface->dwBytesPerPixel << SST_CAM_LFB_DEPTH_SHIFT;
      CAMEntry.strideState |= pSurface->dwBytesPerPixel << SST_CAM_PIXEL_DEPTH_SHIFT;

      // Stagger
      if (pSurface->tileFlag & MEM_STAGGER)      
         CAMEntry.strideState |= SST_CAM_EN_STAGGER;

      // Depth buffer?

      // AA ??

      // YUV ??

      // alloc LFB space given stride==power of 2 restriction

#ifdef SLI
      // Enable SLI for the surface
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         if (pSurface->dwFlags & FXSURFACE_IS_DISTRIBUTED)
            CAMEntry.strideState |= SST_CAM_EN_DISTRIBUTED;
         }
#endif
      ptr = pSurface->phantomlfbPtr; // pointer returned to Host caller

      CAMEntry.baseAddress = ptr - _FF(LFBBASE);    // CAM needs offset from top of membase1
      CAMEntry.endAddress  = CAMEntry.baseAddress+pSurface->phantom_blksize;
      CAMEntry.physicalBase= pSurface->hwPtr;

      if (!FxCamProgram(_DD(camMgr),ppdev,&CAMEntry,CAM_ENTRY_ILLEGAL))
         {
         // CAM programming failed - is there a better error condition?
         return FALSE;
         }
      }
   
   return TRUE;
}

/*----------------------------------------------------------------------
Function name: CamUnLock

Description: This function is used to lock a surface into the CAM
so that we can operate on it. 

Notes: What about wait for idle?

Return:        None  

----------------------------------------------------------------------*/
DWORD CamUnLock(NT9XDEVICEDATA * ppdev, LPDDRAWI_DIRECTDRAW_GBL lpDD, FXSURFACEDATA * pSurface)
{

    if (pSurface->phantomlfbPtr)
       {
       if (!FxCamFreeEntry(_DD(camMgr), ppdev, pSurface->phantomlfbPtr-_FF(LFBBASE)))
          {
          return FALSE;
          }
       }
   
   return TRUE;
}

/*----------------------------------------------------------------------
Function name: DdSli2DScn2Scn

Description:   This is a low level routine to perform Screen to Screen
Blits when in SLI distributed mode.

There are 3 main cases with many subcases

1.) Distributed to Distributed
   a.) Same Surface
   b.) Different Surfaces 
2.) Non-Distributed to Distributed
Assumptions:  Each Device has the same image which must be
placed in the correct Device Distributed Surface
3.) Distributed to Non-Distributed
Assumptions: Each Device has part of the image which must be combined
in a off screen surface

Return:        None  

----------------------------------------------------------------------*/
#define BLT32_DOBLTS(arg1,arg2,arg3,arg4,arg5)        blt32_doBltS(arg1,arg2,arg3,arg4,arg5)
DWORD __stdcall BLT32_DOBLTS(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD dstPixelFormat);
DWORD DdSli2DScn2Scn(NT9XDEVICEDATA * ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD dstPixelFormat)
{
   FXSURFACEDATA       *pSrcSurfaceData;
   FXSURFACEDATA       *pDstSurfaceData;
   long dstHeight;
   long srcHeight;
   long dstWidth;
   long srcWidth;
   DWORD dwReturn;
   DWORD dwChunk;
   long dstSliTop;
   long dstRawSLITop;
   long dstSliCurrent;   
   long dstSliBottom;   
   long dstRawSliBottom;
   DWORD dwRawMask;
   DWORD dwMask;
   DWORD SaveSLIBroadCast;
   DWORD i;
   long sMajor;
   long sMinor;
   long cMajor;
   long cMinor;
   long wholeInc;
   long partialHeight;
   long partialInc;
   RECTL rDest;
   RECTL rSrc;
   Sst2CAMEntry CAM;
   CMDFIFO_PROLOG(cmdFifo);

   pSrcSurfaceData = (FXSURFACEDATA*) (pbd->lpDDSrcSurface->lpGbl->dwReserved1);
   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

   // If in Flip Chain assume that it is a Sli Surface
   if ((pSrcSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED) == (pDstSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED))
      {
      if (pSrcSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
         {
         // if this is full screen not a stretch then
         // crunch Y and do it
         dstHeight = pbd->rDest.bottom - pbd->rDest.top;
         srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;
         dstWidth = pbd->rDest.right - pbd->rDest.left;
         srcWidth = pbd->rSrc.right - pbd->rSrc.left;
         if ((dstHeight == srcHeight) && 
             (dstWidth == srcWidth) &&
             (0x0 == pbd->rDest.top) &&
             (0x0 == pbd->rSrc.top) &&
             (ppdev->bi.biHeight == pbd->rDest.bottom))
            {
            // Save crunched bottom
            rDest.bottom = pbd->rDest.bottom;

            // Just Crunch Y
            pbd->rDest.bottom = pbd->rSrc.bottom = YMAX();
            dwReturn = BLT32_DOBLTS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);

            // Restore crunched bottom
            pbd->rDest.bottom = pbd->rSrc.bottom = rDest.bottom;
            }
         else
            {
            if (CamLock(ppdev, pbd->lpDD, pSrcSurfaceData) && CamLock(ppdev, pbd->lpDD, pDstSurfaceData))
               dwReturn = sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
            CamUnLock(ppdev, pbd->lpDD, pSrcSurfaceData);
            CamUnLock(ppdev, pbd->lpDD, pDstSurfaceData);
            }
         }
      else
         { 
         // ND to ND .... nothing to do
         dwReturn = BLT32_DOBLTS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
         }
      }
   else if (pSrcSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
      { // DD to ND
      if (CamLock(ppdev, pbd->lpDD, pSrcSurfaceData) && CamLock(ppdev, pbd->lpDD, pDstSurfaceData))
         dwReturn = sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
      CamUnLock(ppdev, pbd->lpDD, pSrcSurfaceData);
      CamUnLock(ppdev, pbd->lpDD, pDstSurfaceData);
      }
   else
      { 
      // ND to DD
      dstHeight = pbd->rDest.bottom - pbd->rDest.top;
      srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

      // Save
      rDest = pbd->rDest;
      rSrc = pbd->rSrc;

      // Single Line Increment
      sMajor = srcHeight/dstHeight;
      sMinor = srcHeight % dstHeight;      

      // Chunk Line Increment
      cMajor = (sMajor << _DD(dwLog2BandHeight)) + (sMinor << _DD(dwLog2BandHeight))/dstHeight;
      cMinor = (sMinor << _DD(dwLog2BandHeight)) % dstHeight;
      wholeInc = cMajor;
      partialInc = cMinor;

      // this is just a series of YCRUNCH
      // There is a partial chunk at the beginning
      if ((pbd->rDest.top & (_DD(dwBandHeight) - 1)) || ((DWORD)(pbd->rDest.bottom - pbd->rDest.top) < _DD(dwBandHeight)))
         {
         // Align on Chunk Boundary
         dstRawSLITop = pbd->rDest.top & ~(_DD(dwBandHeight) - 1);
         
         SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(pbd->rDest.top)), cmdFifo);

         dstRawSliBottom = MIN(dstRawSLITop + _DD(dwBandHeight), (DWORD)pbd->rDest.bottom);
         dstSliCurrent = dstRawSliBottom;
         dstSliTop = YCRUNCH(pbd->rDest.top);
         dstSliBottom = dstSliTop + (dstRawSliBottom - pbd->rDest.top);

         pbd->rDest.top = dstSliTop;
         pbd->rDest.bottom = dstSliBottom;
         // Modify src bottom
         partialHeight = dstSliBottom - dstSliTop;
         pbd->rSrc.bottom = pbd->rSrc.top + (sMajor * partialHeight) + (sMinor * partialHeight)/dstHeight;
         partialInc += (sMinor * partialHeight);
         partialInc = partialInc % dstHeight;         
         
         CMDFIFO_SAVE(cmdFifo); 
         dwReturn = BLT32_DOBLTS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
         CMDFIFO_RELOAD(cmdFifo); 
         SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
         pbd->rSrc.top = pbd->rSrc.bottom;
         }
      else
         {
         dstSliCurrent = pbd->rDest.top;
         }         

      // Here is where we need to group up the wholes Chunks.....
      dwChunk = (rDest.bottom - dstSliCurrent) >> _DD(dwLog2BandHeight);
      dwRawMask = YUNIT(dstSliCurrent);
      for (i=0; i<dwChunk; i++)
         {
         dwMask = BIT(dwRawMask);
         dwRawMask = (dwRawMask + 1) & _FF(dwNumSlaves);
         dstSliTop = YCRUNCH(dstSliCurrent);
         dstSliBottom = dstSliTop + _DD(dwBandHeight);
         SLI_UNIT_SELECT(SaveSLIBroadCast, dwMask, cmdFifo);


         pbd->rDest.top = dstSliTop;
         pbd->rDest.bottom = dstSliBottom;
         wholeInc = cMajor;
         while (partialInc > dstHeight)
            {
            partialInc -= dstHeight;
            wholeInc++;
            }

         pbd->rSrc.bottom = pbd->rSrc.top + wholeInc;
         partialInc += cMinor;
      
         CMDFIFO_SAVE(cmdFifo); 
         dwReturn = BLT32_DOBLTS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
         CMDFIFO_RELOAD(cmdFifo); 
         SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
         dstSliCurrent += _DD(dwBandHeight);
         pbd->rSrc.top = pbd->rSrc.bottom;
         }

      // There is a partial chunk at the end
      if (dstSliCurrent < rDest.bottom)
         {
         SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(dstSliCurrent)), cmdFifo);

         dstSliBottom = YCRUNCH(rDest.bottom);
         dstSliTop = YCRUNCH(dstSliCurrent);
         pbd->rDest.top = dstSliTop;
         pbd->rDest.bottom = dstSliBottom;
         pbd->rSrc.bottom = rSrc.bottom;

         CMDFIFO_SAVE(cmdFifo); 
         dwReturn = BLT32_DOBLTS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
         CMDFIFO_RELOAD(cmdFifo); 
         SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
         }

      // Restore
      pbd->rDest = rDest;
      pbd->rSrc = rSrc;
      CMDFIFO_SAVE(cmdFifo);
      }   

  return dwReturn;
}

/*----------------------------------------------------------------------
Function name: DdSli2DSystem2Scn

Description:   This is a low level routine to perform Screen to Screen
Blits when in SLI distributed mode.

There are 3 main cases with many subcases

1.) Distributed to Distributed
   a.) Same Surface
   b.) Different Surfaces 
2.) Non-Distributed to Distributed
Assumptions:  Each Device has the same image which must be
placed in the correct Device Distributed Surface
3.) Distributed to Non-Distributed
Assumptions: Each Device has part of the image which must be combined
in a off screen surface

Return:        None  

----------------------------------------------------------------------*/
#define BLT32_SYSTEMTOVIDEO(arg1,arg2,arg3,arg4,arg5)        blt32_systemToVideo(arg1,arg2,arg3,arg4,arg5)
DWORD __stdcall BLT32_SYSTEMTOVIDEO(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD srcPixelFormat, DWORD srcBytePerPixel, DWORD dstPixelFormat);
DWORD DdSli2DSystem2Scn(NT9XDEVICEDATA * ppdev, LPDDHAL_BLTDATA pbd, DWORD srcPixelFormat, DWORD srcBytePerPixel, DWORD dstPixelFormat)
{
   long dstHeight;
   long srcHeight;
   DWORD dwReturn;
   DWORD dwChunk;
   long dstSliTop;
   long dstRawSLITop;
   long dstSliCurrent;   
   long dstSliBottom;   
   long dstRawSliBottom;
   DWORD dwRawMask;
   DWORD dwMask;
   DWORD SaveSLIBroadCast;
   DWORD i;
   long sMajor;
   long sMinor;
   long cMajor;
   long cMinor;
   long wholeInc;
   long partialHeight;
   long partialInc;
   RECTL rDest;
   RECTL rSrc;
   FXSURFACEDATA * pDstSurfaceData;
   Sst2CAMEntry CAM;
   CMDFIFO_PROLOG(cmdFifo);

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

   if (pDstSurfaceData->dwFlags & FXSURFACE_IS_DISTRIBUTED)
      {
      // ND to DD
      dstHeight = pbd->rDest.bottom - pbd->rDest.top;
      srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

      // Save
      rDest = pbd->rDest;
      rSrc = pbd->rSrc;
   
      // Single Line Increment
      sMajor = srcHeight/dstHeight;
      sMinor = srcHeight % dstHeight;      
   
      // Chunk Line Increment
      cMajor = (sMajor << _DD(dwLog2BandHeight)) + (sMinor << _DD(dwLog2BandHeight))/dstHeight;
      cMinor = (sMinor << _DD(dwLog2BandHeight)) % dstHeight;
      wholeInc = cMajor;
      partialInc = cMinor;
   
      // this is just a series of YCRUNCH
      // There is a partial chunk at the beginning
      if ((pbd->rDest.top & (_DD(dwBandHeight) - 1)) || ((DWORD)(pbd->rDest.bottom - pbd->rDest.top) < _DD(dwBandHeight)))
         {
         // Align on Chunk Boundary
         dstRawSLITop = pbd->rDest.top & ~(_DD(dwBandHeight) - 1);
            
         SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(pbd->rDest.top)), cmdFifo);
   
         dstRawSliBottom = MIN(dstRawSLITop + _DD(dwBandHeight), (DWORD)pbd->rDest.bottom);
         dstSliCurrent = dstRawSliBottom;
         dstSliTop = YCRUNCH(pbd->rDest.top);
         dstSliBottom = dstSliTop + (dstRawSliBottom - pbd->rDest.top);
  
         pbd->rDest.top = dstSliTop;
         pbd->rDest.bottom = dstSliBottom;
         // Modify src bottom
         partialHeight = dstSliBottom - dstSliTop;
         pbd->rSrc.bottom = pbd->rSrc.top + (sMajor * partialHeight) + (sMinor * partialHeight)/dstHeight;
         partialInc += (sMinor * partialHeight);
         partialInc = partialInc % dstHeight;         
            
         CMDFIFO_SAVE(cmdFifo); 
         dwReturn = BLT32_SYSTEMTOVIDEO(ppdev, pbd, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
         CMDFIFO_RELOAD(cmdFifo); 
         SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
         pbd->rSrc.top = pbd->rSrc.bottom;
         }
      else
         {
         dstSliCurrent = pbd->rDest.top;
         }         
   
      // Here is where we need to group up the wholes Chunks.....
      dwChunk = (rDest.bottom - dstSliCurrent) >> _DD(dwLog2BandHeight);
      dwRawMask = YUNIT(dstSliCurrent);
      for (i=0; i<dwChunk; i++)
         {
         dwMask = BIT(dwRawMask);
         dwRawMask = (dwRawMask + 1) & _FF(dwNumSlaves);
         dstSliTop = YCRUNCH(dstSliCurrent);
         dstSliBottom = dstSliTop + _DD(dwBandHeight);
         SLI_UNIT_SELECT(SaveSLIBroadCast, dwMask, cmdFifo);
   
   
         pbd->rDest.top = dstSliTop;
         pbd->rDest.bottom = dstSliBottom;
         wholeInc = cMajor;
         while (partialInc > dstHeight)
            {
            partialInc -= dstHeight;
            wholeInc++;
            }
   
         pbd->rSrc.bottom = pbd->rSrc.top + wholeInc;
         partialInc += cMinor;
         
         CMDFIFO_SAVE(cmdFifo); 
         dwReturn = BLT32_SYSTEMTOVIDEO(ppdev, pbd, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
         CMDFIFO_RELOAD(cmdFifo); 
         SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
         dstSliCurrent += _DD(dwBandHeight);
         pbd->rSrc.top = pbd->rSrc.bottom;
         }
   
      // There is a partial chunk at the end
      if (dstSliCurrent < rDest.bottom)
         {
         SLI_UNIT_SELECT(SaveSLIBroadCast, BIT(YUNIT(dstSliCurrent)), cmdFifo);
   
         dstSliBottom = YCRUNCH(rDest.bottom);
         dstSliTop = YCRUNCH(dstSliCurrent);
         pbd->rDest.top = dstSliTop;
         pbd->rDest.bottom = dstSliBottom;
         pbd->rSrc.bottom = rSrc.bottom;
   
         CMDFIFO_SAVE(cmdFifo); 
         dwReturn = BLT32_SYSTEMTOVIDEO(ppdev, pbd, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
         CMDFIFO_RELOAD(cmdFifo); 
         SLI_UNIT_DESELECT(SaveSLIBroadCast, cmdFifo);
         }
   
      // Restore
      pbd->rDest = rDest;
      pbd->rSrc = rSrc;
      CMDFIFO_SAVE(cmdFifo);
      }
   else
      dwReturn = BLT32_SYSTEMTOVIDEO(ppdev, pbd, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
      
   return dwReturn;
}
