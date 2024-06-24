/* $Header: ddblt32.c, 22, 9/21/00 5:34:49 PM PDT, Miles Smith$ */
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
** File Name:   DDBLT32.C
**
** Description:	Direct Draw blting routines
**	
** $Revision: 22$
** $Date: 9/21/00 5:34:49 PM PDT$
**
** $Log: 
**  22   3dfx      1.21        9/21/00  Miles Smith     code cleanup related to 
**       AA hint flag
**  21   3dfx      1.20        9/13/00  Miles Smith     Changed blit size data 
**       for AA modes
**  20   3dfx      1.19        6/9/00   Miles Smith     minor changes for 
**       fullscreen aa modes
**  19   3dfx      1.18        5/4/00   Miles Smith     many fullscreen AA 
**       related changes - including addition of code to enable   registry 
**       selected AA for apps that didn't know they wanted it.
**  18   3dfx      1.17        4/28/00  Xing Cong       For video to video 
**       stretch blt, support 16BPP surfaces
**  17   3dfx      1.16        4/24/00  Evan Leland     preliminary changes, 
**       getting ready to removing all ddraw structure pointers stored in the 
**       driver
**  16   3dfx      1.15        3/31/00  Evan Leland     Dont call txtrLoad() for
**       color fills to texture surfaces
**  15   3dfx      1.14        2/1/00   Xing Cong       Add StretchBlt_3D() for 
**       filter stretch blt.  Currently only support 32bit surface
**  14   3dfx      1.13        1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  13   3dfx      1.12        12/16/99 Evan Leland     changed function names 
**       and put prototypes in d3txtr.h for all texture routines that are 
**       exported from the texture module
**  12   3dfx      1.11        11/30/99 Evan Leland     only call textureLoad in
**       DdBlt when two textures are blitted, source is in host and dest is in 
**       frame
**  11   3dfx      1.10        11/9/99  Xing Cong       If color fill YUYV or 
**       UYVY data, set blt with 16BPP color.
**  10   3dfx      1.9         11/5/99  Miles Smith     Adding support for 
**       windowed AA
**  9    3dfx      1.8         10/31/99 Michel Conrad   Add support for execute 
**       mode blts.
**  8    3dfx      1.7         10/20/99 Andrew Sobczyk  Fixed Problems with DCT 
**       1.20.0.200 Overlapping Blts and the fact that the CSIM does not support
**       HOST_STRETCH_BLTS but the hardware does.
**  7    3dfx      1.6         10/18/99 Andrew Sobczyk  First Pass cleanup of 
**       ddblt32.c.  Removed unused code and ifdef.  Reformated, fixed some bugs
**       with DCT 1.20.0.200.
**  6    3dfx      1.5         10/14/99 Andrew Sobczyk  Added code to call SLI 
**       routines if SLI is enabled
**  5    3dfx      1.4         10/1/99  Mark Einkauf    Complete HW_ACCESS 
**       macros work
**  4    3dfx      1.3         9/24/99  Mark Einkauf    call CMDFIFO_BUMP for 
**       each scanline of blts - effects AGP fifo only
**  3    3dfx      1.2         9/24/99  Brent Burton    Added the change log 
**       back.
**  2    3dfx      1.1         9/23/99  Brent Burton    Fixed pixelByteDepth 
**       comparisons to enable blt'ing.  This fixes the driver error where 
**       DdBlt() was reporting some blt's as unimplemented.
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
** *****************  Version 85  *****************
** User: Einkauf      Date: 9/03/99    Time: 3:05p
** Updated in $/devel/sst2/Win95/dx/dd32
** add fifo ptr to SETMOP parm list
** 
** *****************  Version 84  *****************
** User: Einkauf      Date: 9/03/99    Time: 12:10a
** Updated in $/devel/sst2/Win95/dx/dd32
** cmd fifo fixes, primarily colorkeys which don't fit in pkt2 on Rampage
** 
** *****************  Version 83  *****************
** User: Pratt        Date: 9/02/99    Time: 8:37a
** Updated in $/devel/R3/Win95/dx/dd32
** removed IFDEF R21 and changed IFNDEF r21 to IF(0)
** 
** *****************  Version 82  *****************
** User: Bdaniels     Date: 8/31/99    Time: 1:21p
** Updated in $/devel/sst2/Win95/dx/dd32
** Adding 32 bpp rendering, 24 bit z / 8 bit stencil support.
** 
** *****************  Version 81  *****************
** User: Einkauf      Date: 8/31/99    Time: 1:59p
** Updated in $/devel/sst2/Win95/dx/dd32
** DirectX CMD FIFO
** 
** *****************  Version 80  *****************
** User: Evan         Date: 8/21/99    Time: 1:43p
** Updated in $/devel/sst2/Win95/dx/dd32
** allow NPT textures to be blitted
** 
** *****************  Version 79  *****************
** User: Mconrad      Date: 8/18/99    Time: 3:54p
** Updated in $/devel/sst2/Win95/dx/dd32
** Enable texture loads.
** 
** *****************  Version 78  *****************
** User: Mconrad      Date: 8/15/99    Time: 7:04p
** Updated in $/devel/sst2/Win95/dx/dd32
** Shotgun Mop of 3D before 2D, be more selective later.
** 
** *****************  Version 77  *****************
** User: Mconrad      Date: 8/10/99    Time: 9:49p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fix warnings.
** 
** *****************  Version 76  *****************
** User: Einkauf      Date: 7/28/99    Time: 2:03p
** Updated in $/devel/sst2/Win95/dx/dd32
** use BLTFMT_STRIDESTATE macro for src/dstFormat in each blt routine
** 
** *****************  Version 75  *****************
** User: Agus         Date: 7/24/99    Time: 5:41p
** Updated in $/devel/sst2/Win95/dx/dd32
** Ddraw clean-up, use dwPStride for surface's pitch 
** 
** *****************  Version 74  *****************
** User: Agus         Date: 7/22/99    Time: 2:38p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added surface tile flag check and set the correct blt in colorfill,
** this fixes foxbear to run cleanly.
** 
** *****************  Version 73  *****************
** User: Agus         Date: 7/22/99    Time: 12:00p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fixes to get flip2d cube and foxbear (somewhat) running.
** 
** 
** *****************  Version 72  *****************
** User: Agus         Date: 7/19/99    Time: 5:02p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fixed destination or source pitch assignment with physical pitch stored
** in FXSURFACEDATA's lPitch; this is correct COLORFILL blt which did not
** clear the background on Flip2D apps
** 
** *****************  Version 71  *****************
** User: Agus         Date: 7/19/99    Time: 2:48p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added include "sst2glob.h"
** 
** *****************  Version 70  *****************
** User: Peterm       Date: 7/19/99    Time: 12:09p
** Updated in $/devel/sst2/Win95/dx/dd32
** fixed blt stride calculation in all blt calls
** 
** *****************  Version 69  *****************
** User: Einkauf      Date: 7/08/99    Time: 7:30p
** Updated in $/devel/sst2/Win95/dx/dd32
** CAM locking for general surfaces, BLT's work better - using proper
** dst/srcFormat settings
** 
** *****************  Version 68  *****************
** User: Peterm       Date: 6/12/99    Time: 3:16p
** Updated in $/devel/sst2/Win95/dx/dd32
** enable sst2 wax blt code, some mm cleanup
** 
** *****************  Version 67  *****************
** User: Peterm       Date: 6/04/99    Time: 2:01a
** Updated in $/devel/sst2/Win95/dx/dd32
** updated for changed inc\sst* and others
** 
** *****************  Version 66  *****************
** User: Peterm       Date: 6/03/99    Time: 11:24p
** Updated in $/devel/sst2/Win95/dx/dd32
** modified to run with H3 tot (adds multimon, various bug fixes, and many
** structural deltas)
** 
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"
#include "sst2glob.h"
#include "fifomgr.h"
#include "d3txtr2.h"                // txtrLoad()
#include "ddovl32.h"

#ifdef SLI
#include <ddsli2d.h>
#endif



/**************************************************************************
* D E F I N E S
***************************************************************************/

#ifdef FXTRACE
#define DUMP_BLTDATA(arg1,arg2,arg3) Dump_BLTDATA(arg1,arg2,arg3)
#define DUMP_DDHAL_GETBLTSTATUSDATA(arg1,arg2,arg3) Dump_DDHAL_GETBLTSTATUSDATA(arg1,arg2,arg3)
#endif

/*
 * ROP utility macros. These take a standard rop byte.
 */
#define ROP_HAS_SRC(rop3) (0 != (0x33 & ( (ULONG)rop3 ^ ((ULONG)rop3 >> 2))))
#define ROP_HAS_PAT(rop3) (0 != (0x0F & ( (ULONG)rop3 ^ ((ULONG)rop3 >> 4))))
#define ROP_HAS_DST(rop3) (0 != (0x55 & ( (ULONG)rop3 ^ ((ULONG)rop3 >> 1))))


/**************************************************************************
* G L O B A L   V A R S
***************************************************************************/

#ifdef FXTRACE
DWORD bltOn = TRUE;
#endif

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

#undef DDHAL_DRIVER_NOTHANDLED
#define DDHAL_DRIVER_NOTHANDLED DDHAL_DRIVER_HANDLED

/*----------------------------------------------------------------------
Function name: DdGetBltStatus

Description:   DDRAW Callback GetBltStatus()

               Determines if the blitter queue has room for more blits
               and if the blitter is busy.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdGetBltStatus( LPDDHAL_GETBLTSTATUSDATA lpGetBltStatus )
{
   DD_ENTRY_SETUP(lpGetBltStatus->lpDD);

#ifdef FXTRACE
   DISPDBG((ppdev, DEBUG_APIENTRY, "GetBltStatus32" ));
#endif

   // Ignore device status and return DD_OK for 3D applications,
   // otherwise return DDERR_WASSTILLDRAWING if device busy.

   lpGetBltStatus->ddRVal = DD_OK;

   if (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY))
      {
      if (lpGetBltStatus->dwFlags == DDGBS_ISBLTDONE)
         {
         if (FXGETBUSYSTATUS(ppdev))
	         {
            lpGetBltStatus->ddRVal = DDERR_WASSTILLDRAWING;
	         }
         }
      }

   return DDHAL_DRIVER_HANDLED;
} /* DdGetBltStatus */

/*----------------------------------------------------------------------
Function name:  blt32_colorFill

Description:    Fill a rectangular region of a surface

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall blt32_colorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat)
{
   DWORD     bltDstBaseAddr, bltDstFormat, bltDstSize, bltRop, bltDstXY;
   RECTL     dstRect = pbd->rDest;
   long      dstLeft, dstTop, dstRight, dstBottom;
   long      dstWidth, dstHeight, dstPitch;
   DWORD     packetHeader = 0, bumpNum=10, clip1min, clip1max;
   FXSURFACEDATA *surfData;

   CMDFIFO_PROLOG(hwPtr);
   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   dstTop = dstRect.top;
   dstRight = dstRect.right;
   dstBottom = dstRect.bottom;
   dstLeft = dstRect.left;

   dstWidth = dstRight - dstLeft;
   dstHeight = dstBottom - dstTop;

#ifdef AGP_EXECUTE
    // bltDstBaseAddr = pbd->lpDDDestSurface->lpGbl->fpVidMem
    //                 - _FF(agpHeapLinBaseAddr)
    //                 + _FF(agpHeapPhybasesAddr) ;

   if( pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
      bltDstBaseAddr = GET_AGP_ADDR(pbd->lpDDDestSurface);
   else 
#endif
      bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
  
   surfData = (FXSURFACEDATA*)(pbd->lpDDDestSurface->lpGbl->dwReserved1);
   BLTFMT_STRIDESTATE(surfData,dstPitch);

   BLTCLIP(dstLeft, dstTop, clip1min);
   BLTCLIP(dstRight, dstBottom, clip1max);

   BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);
   BLTSIZE(dstWidth, dstHeight, bltDstSize);
   BLTXY(dstLeft, dstTop, bltDstXY);
   bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | (SST_WX_ROP_SRC );

   // write to hw
   packetHeader |=  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;
   CMDFIFO_CHECKROOM(hwPtr, bumpNum);

   SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
   SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
   SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
   SETPD(hwPtr, ghw2D->rop, bltRop);
   SETPD(hwPtr, ghw2D->clip1min, clip1min);
   SETPD(hwPtr, ghw2D->clip1max, clip1max);
   // DDraw saves the depth & color info in the same place (grab it)
   SETPD(hwPtr, ghw2D->colorFore, pbd->bltFX.dwFillColor);
   SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
   SETPD(hwPtr, ghw2D->dstXY, bltDstXY);
   SETPD(hwPtr, ghw2D->command, SST_WX_RECTFILL | SST_WX_GO | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);

   HW_ACCESS_EXIT(ACCESS_2D);

   CMDFIFO_EPILOG(hwPtr);
   pbd->ddRVal = DD_OK;
   return DDHAL_DRIVER_HANDLED;
}// blt32_colorFill

/*----------------------------------------------------------------------
Function name:  blt32_doBltNoSP

Description:    Handle blts that don't use a source or pattern.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall blt32_doBltNoSP(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD dstPixelFormat)
{
   DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
   DWORD dstColorkeyMax, dstColorkeyMin, dstPitch;
   DWORD bltRop, dwBltFlags, bltCommand = 0, bltCommandExtra = 0;
   RECTL dstRect = pbd->rDest;
   long  dstLeft, dstTop, dstRight, dstBottom, dstWidth, dstHeight;
   DWORD packetHeader = 0, bumpNum = 0;
   DWORD clip1min, clip1max;
   FXSURFACEDATA *surfData;

   CMDFIFO_PROLOG(hwPtr);
   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   // chroma key
   dwBltFlags = pbd->dwFlags;

   // No Src, so no source color key.

   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
      dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
      bltCommandExtra |= SST_WX_EN_DST_COLORKEY_EX;
      bltCommand = SST_WX_RECTFILL | SST_WX_GO | (SST_WX_ROP_DST << SST_WX_ROP0_SHIFT)  | SST_WX_CLIPSELECT;
      packetHeader |=  commandExBit;
      bumpNum += PH1_SIZE+2 + 1; // separate PKT1 for color keys, plus 1 DWORD for commandEx
      }
   else
      {
      bltCommand = SST_WX_RECTFILL | SST_WX_GO | (rop3 << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT;
      }

   // get rectangle
   dstTop = dstRect.top;
   dstRight = dstRect.right;
   dstBottom = dstRect.bottom;
   dstLeft = dstRect.left;

   dstWidth = dstRight - dstLeft;
   dstHeight = dstBottom - dstTop;

   // get base address
#ifdef AGP_EXECUTE
   if( pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
     bltDstBaseAddr = GET_AGP_ADDR(pbd->lpDDDestSurface);
   else 
#endif
     bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
      
   surfData = (FXSURFACEDATA*)(pbd->lpDDDestSurface->lpGbl->dwReserved1);
   BLTFMT_STRIDESTATE(surfData,dstPitch);

   // now stuff them in the hardware format
   BLTCLIP(dstLeft, dstTop, clip1min);
   BLTCLIP(dstRight, dstBottom, clip1max);
   BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);
   BLTSIZE(dstWidth, dstHeight, bltDstSize);
   BLTXY(dstLeft, dstTop, bltDstXY);

   bltRop = (SST_WX_ROP_DST << 16 )| (SST_WX_ROP_DST << 8 ) | (rop3);

   // write to hw

   packetHeader |= dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | dstSizeBit | dstXYBit | commandBit;

   bumpNum += PH2_SIZE + 8;

   CMDFIFO_CHECKROOM(hwPtr, bumpNum);

   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, dstColorkeyMin));
      SETPD(hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin);
      SETPD(hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax);
      }

   SETPH( hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
   SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
   SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
   SETPD(hwPtr, ghw2D->rop, bltRop);
   if (packetHeader & commandExBit)
      {
      SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
      }

   SETPD(hwPtr, ghw2D->clip1min, clip1min);
   SETPD(hwPtr, ghw2D->clip1max, clip1max);
   SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
   SETPD(hwPtr, ghw2D->dstXY, bltDstXY);
   SETPD(hwPtr, ghw2D->command, bltCommand);

   HW_ACCESS_EXIT(ACCESS_2D);

   CMDFIFO_EPILOG(hwPtr);

   pbd->ddRVal = DD_OK;
   return DDHAL_DRIVER_HANDLED;
}// blt32_doBltNoSP



/*----------------------------------------------------------------------
Function name:  StretchBlt_3D

Description:   StretchBlt using 3D engine

Return:         NONE

----------------------------------------------------------------------*/
void StrechBlt_3D( NT9XDEVICEDATA  *ppdev, 
  FXSURFACEDATA * srcSurface, FXSURFACEDATA * dstSurface,
  DWORD dstSurfHeight,
  DWORD srcFormat, DWORD dstFormat,
        RECTL * rSrc, RECTL * rDst)
{

 GrTexNPTInfoExt     srcTexture;
 TMURegs  sstReg;
 Vertex vtxA, vtxB, vtxC, vtxD;

  #ifdef CMDFIFO
  CMDFIFO_PROLOG(cmdFifo);
#else
  FxU32 *cmdFifo;
  FxU32  hwIndex;
#endif
   HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);
   memset( &sstReg, 0, sizeof( TMURegs));

   switch( srcFormat)
   {
   	case SST_WX_PIXFMT_32BPP:
		srcTexture.format          = SST_TA_ARGB8888;
		break;
	case SST_WX_PIXFMT_422YUV:
		srcTexture.format          = SST_TA_YUYV422;
		break;
	case SST_WX_PIXFMT_422UYV:
		srcTexture.format          = SST_TA_UYVY422;
		break;
   case SST_WX_PIXFMT_16BPP:
   default:
		srcTexture.format          = SST_TA_RGB565;
   }
   srcTexture.maxS            = rSrc->right;
   srcTexture.maxT            = rSrc->bottom;
   srcTexture.baseAddr        = srcSurface->hwPtr;
   srcTexture.nptStride       = srcSurface->dwStride;
   srcTexture.bFilter         = 1;
   SetNPTSourceExt(0, &srcTexture,srcSurface->tileFlag, &sstReg); 

/* 3D Transformations */
    /*---- 
      A-B
      |\|
      C-D
      -----*/
    vtxA.w = 1.0f;
    vtxB = vtxC = vtxD = vtxA;

    vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(rSrc->left);
    vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(rSrc->right);
    vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(rSrc->top);
    vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(rSrc->bottom);
    
    vtxA.x = vtxC.x = (float)(rDst->left);
    vtxB.x = vtxD.x = (float)(rDst->right);
    vtxA.y = vtxB.y = (float)(rDst->top);
    vtxC.y = vtxD.y = (float)(rDst->bottom);
   TMULoad(0, &sstReg);
   preSetRegs( ppdev, dstSurface, dstSurfHeight, &cmdFifo, &hwIndex);
   SetupTmus( ppdev, &sstReg,0,&cmdFifo, &hwIndex);
   switch( dstFormat)
   {
	 case SST_WX_PIXFMT_32BPP:
	   ConfigBuffer( ppdev,SST_PE_ARGB_WRMASK,4,0,&cmdFifo, &hwIndex);
		break;
	 case SST_WX_PIXFMT_16BPP:
   	 default:
	   ConfigBuffer( ppdev,SST_PE_ARGB_WRMASK,3,0,&cmdFifo, &hwIndex);
   }
 
   DrawRect(ppdev, &vtxA, &vtxB, &vtxC,&vtxD,0,&cmdFifo, &hwIndex);

   HW_ACCESS_EXIT(ACCESS_3D);

   CMDFIFO_EPILOG( cmdFifo );  

}



/*----------------------------------------------------------------------
Function name:  blt32_doBltS

Description:    Handle blts that use a source only, no pattern.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall blt32_doBltS(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD dstPixelFormat)
{
   DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
   DWORD bltBufBaseAddr;
   DWORD srcColorkeyMin, srcColorkeyMax, dstColorkeyMax, dstColorkeyMin;
   DWORD bltRop, dwBltFlags, bltCommand, bltCommandExtra;
   DWORD bltSrcBaseAddr, bltSrcFormat, bltSrcSize, bltSrcXY;

   DWORD dstLeft, dstTop, dstRight, dstBottom, dstWidth, dstHeight, dstPitch;
   DWORD srcLeft, srcTop, srcRight, srcBottom, srcWidth, srcHeight, srcPitch;
   DWORD srcX, srcY, dstX, dstY, clip1min, clip1max;
   DWORD packetHeader = 0;
   DWORD packetHeaderBtoS;
   DWORD bumpNum = 14;
   DWORD bumpNumBtoS;
   DWORD i;
   DWORD sourceIsAA; 
   BOOL  Stretch;
   FXSURFACEDATA *srcSurfData, *dstSurfData;

   CMDFIFO_PROLOG(hwPtr);
   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   dwBltFlags = pbd->dwFlags;

   bltCommand = (DWORD)rop3 << SST_WX_ROP0_SHIFT;
   bltCommandExtra = 0;


   // chroma key
   if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
      {
      srcColorkeyMin   = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
      srcColorkeyMax   = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
      bltCommandExtra |= SST_WX_EN_SRC_COLORKEY_EX ;
      bumpNum += PH1_SIZE +2;
      }

   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      dstColorkeyMin   = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
      dstColorkeyMax   = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
      bltCommandExtra |= SST_WX_EN_DST_COLORKEY_EX;
      bumpNum += PH1_SIZE +2;
      bltCommand = (DWORD) SST_WX_ROP_DST << SST_WX_ROP0_SHIFT;
      }

   // get rectangle
   dstTop = pbd->rDest.top;
   dstRight = pbd->rDest.right;
   dstBottom = pbd->rDest.bottom;
   dstLeft = pbd->rDest.left;

   srcTop = pbd->rSrc.top;
   srcRight = pbd->rSrc.right;
   srcBottom = pbd->rSrc.bottom;
   srcLeft = pbd->rSrc.left;

   dstWidth = dstRight - dstLeft;
   dstHeight = dstBottom - dstTop;

   srcWidth = srcRight - srcLeft;
   srcHeight = srcBottom - srcTop;

   // get base address
#ifdef AGP_EXECUTE
   if( pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
     bltDstBaseAddr = GET_AGP_ADDR(pbd->lpDDDestSurface);
   else 
#endif
     bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);

#ifdef AGP_EXECUTE
   if( pbd->lpDDSrcSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
     bltSrcBaseAddr = GET_AGP_ADDR(pbd->lpDDSrcSurface);
   else 
#endif
     bltSrcBaseAddr = GET_HW_ADDR(pbd->lpDDSrcSurface);

   srcSurfData = (FXSURFACEDATA*)(pbd->lpDDSrcSurface->lpGbl->dwReserved1);
   dstSurfData = (FXSURFACEDATA*)(pbd->lpDDDestSurface->lpGbl->dwReserved1);
   BLTFMT_STRIDESTATE(srcSurfData,srcPitch);
   BLTFMT_STRIDESTATE(dstSurfData,dstPitch);

   // check for AA flag 
   sourceIsAA = srcSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING;  

   // Determine if this is a stretchblt AND source is NOT aa  
   if ( ((dstWidth != srcWidth) || (dstHeight != srcHeight)) && !(sourceIsAA) ) 
      {  // STRETCH/SHRINK
      bltCommand |= SST_WX_STRETCH_BLT | SST_WX_GO | SST_WX_CLIPSELECT;
      Stretch     = TRUE;
      } 
   else 
      {
      bltCommand |= SST_WX_BLT | SST_WX_GO | SST_WX_CLIPSELECT;
      Stretch     = FALSE;
      }

   //if destination is above and left of src, we starts srccopy at the upper left corner
   // otherwise:
   srcX = srcLeft;
   srcY = srcTop;
   dstX = dstLeft;
   dstY = dstTop;

   // Remember, we have to special-case chromakey also -- the following code
   // will NOT work for chromakey.

   if (bltDstBaseAddr == bltSrcBaseAddr)
      {
      if (!Stretch)
         {
         if ((dstTop > srcTop) && (dstTop < srcBottom))
            {
            // start from the bottom side
            srcY = srcBottom - 1;
            dstY = dstBottom - 1;
            bltCommand |= SST_WX_YDIR;
            }
         //This fix is taken from the Napalm Driver and was reported in Raid #318750 -- 
         //Discussion with Russ Lind tell me this is the same problem that I am seeing
         // DCT 1.20.0.200 BLT_OnscreenOverlap when the feline face is middle top and being moved to the right
         else if ((dstTop == srcTop) && (srcLeft<dstLeft) && (srcRight>dstLeft))
            {
            // Starts from the right side -- Reverse X Direction Blt

            // Check if chromakey is used.  If so, use special case code.
            if (!bltCommandExtra)           // Chromakey in use?
               {
               dstX = dstRight - 1;
               srcX = srcRight - 1;
               bltCommand |= SST_WX_XDIR;
               }
            else
               {
               // This is not a 'Pure' bitblt.  HW doesn't directly support
               // right-to-left chromakey, stretch, or conversion blits.  For
               // chromakey, we need to so something special.  We'll blit
               // each scanline of the source to the 2D stretchbuffer, and then
               // blit it back to its actual destination using chromakey if
               // necessary.

               bltBufBaseAddr = _DS(stretchBltStart);

               // Here, set up stuff that doesn't change in Blits...
               BLTFMT_DST(dstPitch, srcPixelFormat, bltDstFormat);
               BLTFMT_SRC(srcPitch, srcPixelFormat, bltSrcFormat);
               if( sourceIsAA ) //if aa set the format bit
                   bltSrcFormat |= SST_WX_SRC_EN_AA; 
               BLTSIZE(srcWidth, 0x00001L, bltDstSize);
               BLTCLIP(dstRight, dstBottom, clip1max);

               CMDFIFO_CHECKROOM(hwPtr, 6);
               SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstFormatBit | clip1minBit | clip1maxBit | srcFormatBit | dstSizeBit));
               SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
               SETPD(hwPtr, ghw2D->clip1min, 0);
               SETPD(hwPtr, ghw2D->clip1max, clip1max);
               SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
               SETPD(hwPtr, ghw2D->dstSize, bltDstSize);

               bltCommand = (rop3 << SST_WX_ROP0_SHIFT) | SST_WX_BLT | SST_WX_CLIPSELECT | SST_WX_GO;
 
               // Set up packetheader for screentobuffer blit
               bumpNum = 7;
               packetHeader = dstBaseAddrBit | srcBaseAddrBit | commandExBit | srcXYBit | dstXYBit | commandBit;


               // Set up packetheader for buffertoscreen blit
               bumpNumBtoS = bumpNum;
               packetHeaderBtoS = packetHeader;

               bltCommandExtra = 0;

               if (dwBltFlags & DDBLT_KEYSRCOVERRIDE) 
                  {
                  bumpNumBtoS += 1;
                  packetHeaderBtoS |= ropBit;
                  bltCommandExtra  |= SST_WX_EN_SRC_COLORKEY_EX;

                  CMDFIFO_CHECKROOM(hwPtr, 3);
                  SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, srcColorkeyMin));
                  SETPD(hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin);
                  SETPD(hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax);
                  }
          
               if (dwBltFlags & DDBLT_KEYDESTOVERRIDE) 
                  {
                  bumpNumBtoS += 1;
                  packetHeaderBtoS |= ropBit;
                  bltCommandExtra |= SST_WX_EN_DST_COLORKEY_EX;

                  CMDFIFO_CHECKROOM(hwPtr, 3);
                  SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, dstColorkeyMin));
                  SETPD(hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin);
                  SETPD(hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax);
                  }

               if (bltCommandExtra)  
                  {
                  bltRop = (SST_WX_ROP_DST << 16 )| (SST_WX_ROP_DST << 8 ) | rop3;
                  }


               // Start of scanline blit loop
               for (i=srcTop; i <= srcBottom; i++) 
                  {
                  // Move scan from src to Stretchbuffer
                  BLTXY(0x0L, 0x0L, bltDstXY);
                  BLTXY(srcLeft, i, bltSrcXY);
                  CMDFIFO_CHECKROOM(hwPtr, bumpNum);
                  SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
                  SETPD(hwPtr, ghw2D->dstBaseAddr, bltBufBaseAddr);
                  SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
                  SETPD(hwPtr, ghw2D->commandEx, 0);
                  SETPD(hwPtr, ghw2D->srcXY, bltSrcXY);
                  SETPD(hwPtr, ghw2D->dstXY, bltDstXY);
                  SETPD(hwPtr, ghw2D->command, bltCommand);

                  // Move scan from stretchbuffer to dest, using ColorKeys if needed
                  BLTXY(dstLeft, i, bltDstXY);
                  BLTXY(0x0L, 0x0L, bltSrcXY);
                  CMDFIFO_CHECKROOM(hwPtr, bumpNumBtoS);
                  SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeaderBtoS));
                  SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
                  if (packetHeaderBtoS & ropBit)  
                     {
                     SETPD(hwPtr, ghw2D->rop, bltRop);
                     }

                  SETPD(hwPtr, ghw2D->srcBaseAddr, bltBufBaseAddr);
                  SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
                  SETPD(hwPtr, ghw2D->srcXY, bltSrcXY);
                  SETPD(hwPtr, ghw2D->dstXY, bltDstXY);
                  SETPD(hwPtr, ghw2D->command, bltCommand);
                  } 

               goto B32DBS_Exit;     //  We're all done now
               }
            }
         }
         // STRETCH BLT:   (What to do with Shrinks?  we should fail them.
      else if (!(srcRight < dstLeft) || (srcLeft > dstRight))  // Overlap in X?
         {
         // Now check for COMPLETE overlap in Y
         if (((srcTop > dstTop) && (srcTop < dstBottom)) && ((srcBottom > dstTop) && (srcBottom < dstBottom)))
            {
            // Do special Move Blit -- As Russ Lind suggested, in this case we
            // blit the original bitmap to the lower-right corner of the dest
            // rectangle, and then do the stretch from there.  The first blit
            // is straight screen-to-screen srccopy

            BLTCLIP(dstRight - srcWidth, dstBottom - srcHeight, clip1min);
            BLTCLIP(dstRight, dstBottom, clip1max);

            // Format for HW
            BLTFMT_DST(dstPitch, srcPixelFormat, bltDstFormat);
            BLTFMT_SRC(srcPitch, srcPixelFormat, bltSrcFormat);

            BLTSIZE(srcWidth, srcHeight, bltDstSize);

            // start from the bottom side
            srcY = srcBottom - 1;
            dstY = dstBottom - 1;
            dstX = dstRight  - 1;
            srcX = srcRight  - 1;

            BLTXY(dstX, dstY, bltDstXY);
            BLTXY(srcX, srcY, bltSrcXY);

            // Write to hw
            packetHeader |= dstBaseAddrBit | dstFormatBit | srcBaseAddrBit | clip1minBit | clip1maxBit | srcFormatBit | srcXYBit | dstSizeBit | dstXYBit | commandBit;

            CMDFIFO_CHECKROOM(hwPtr, 11);

            SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));

            SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
            SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
            SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
            SETPD(hwPtr, ghw2D->clip1min, clip1min);
            SETPD(hwPtr, ghw2D->clip1max, clip1max);
            SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
            SETPD(hwPtr, ghw2D->srcXY, bltSrcXY);
            SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
            SETPD(hwPtr, ghw2D->dstXY, bltDstXY);
            SETPD(hwPtr, ghw2D->command, (DWORD)(SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_BLT | SST_WX_GO | SST_WX_CLIPSELECT | SST_WX_YDIR | SST_WX_XDIR);
            // Just fake out locals to use the rest of the HW code below...
            dstX = dstLeft;
            dstY = dstTop;
            srcX = dstRight  - srcWidth;
            srcY = dstBottom - srcHeight;
            }
         // If we overlap bottom only, do reverse Y blt.  If overlap in Top
         // only then we do nothing special -- normal top-down blit.
         else if (srcBottom > dstTop && srcBottom < dstBottom) // Bottom Only?
            {
            // Do Reverse Y Blt
            srcY = srcBottom - 1;
            dstY = dstBottom -1 ;
            bltCommand |= SST_WX_YDIR;
            }
         }
      }
	  else if( Stretch && (rop3 == 0xCC) &&
   		 !(dwBltFlags & (DDBLT_KEYDESTOVERRIDE|DDBLT_KEYSRCOVERRIDE)) &&
  		 (srcPixelFormat != SST_WX_PIXFMT_24BPP) &&
  		 (srcPixelFormat != SST_WX_PIXFMT_8BPP) &&
  		 (dstPixelFormat != SST_WX_PIXFMT_8BPP))
	  {
	   	//use 3D to blt
	     FXSURFACEDATA *srcSurfaceData,*dstSurfaceData;
	     dstSurfaceData = (FXSURFACEDATA*)(pbd->lpDDDestSurface->lpGbl->dwReserved1);
   	  	 srcSurfaceData = (FXSURFACEDATA*)(pbd->lpDDSrcSurface->lpGbl->dwReserved1);
		 if(dstSurfaceData->tileFlag & MEM_IN_LINEAR)
		   goto NormalBlt;
  		 HW_ACCESS_EXIT(ACCESS_2D);
	 	 CMDFIFO_SAVE(hwPtr);
		 StrechBlt_3D( ppdev, srcSurfaceData, dstSurfaceData,
	   	 (DWORD)pbd->lpDDDestSurface->lpGbl->wHeight,srcPixelFormat, dstPixelFormat,&(pbd->rSrc), &(pbd->rDest));
		 CMDFIFO_RELOAD(hwPtr);
	     goto B32DBS_Exit1;

     
	  }    
NormalBlt:
   BLTCLIP(dstLeft, dstTop, clip1min);
   BLTCLIP(dstRight, dstBottom, clip1max);

   // now stuff them in the hardware format
   BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);
   BLTSIZE(dstWidth, dstHeight, bltDstSize);
   BLTXY(dstX, dstY, bltDstXY);

   BLTFMT_SRC(srcPitch, srcPixelFormat, bltSrcFormat);  // 16 bpp for now
   if( sourceIsAA ) //if source is aa set the format bit for aa
       bltSrcFormat |= SST_WX_SRC_EN_AA;
   BLTSIZE(srcWidth, srcHeight, bltSrcSize);
   BLTXY(srcX, srcY, bltSrcXY);

   bltRop = (SST_WX_ROP_DST << 16 )| (SST_WX_ROP_DST << 8 ) | rop3;

   // write to hw

   CMDFIFO_CHECKROOM(hwPtr, bumpNum);

   if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
      {
      SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, srcColorkeyMin));
	  SETPD( hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin);
      SETPD( hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax);
      }
   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, dstColorkeyMin));
      SETPD(hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin);
      SETPD(hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax);
      }

   packetHeader |= dstBaseAddrBit | dstFormatBit | ropBit | srcBaseAddrBit | commandExBit | clip1minBit | clip1maxBit | srcFormatBit | srcSizeBit | srcXYBit | dstSizeBit | dstXYBit | commandBit;


	SETPH( hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
	SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
	SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
	SETPD(hwPtr, ghw2D->rop, bltRop);
	SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
	SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
	SETPD(hwPtr, ghw2D->clip1min, clip1min);
	SETPD(hwPtr, ghw2D->clip1max, clip1max);
	SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
	SETPD(hwPtr, ghw2D->srcSize, bltSrcSize);
	SETPD(hwPtr, ghw2D->srcXY,bltSrcXY);
	SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
	SETPD(hwPtr, ghw2D->dstXY,bltDstXY);
	SETPD(hwPtr, ghw2D->command, bltCommand);

B32DBS_Exit:
   HW_ACCESS_EXIT(ACCESS_2D);
B32DBS_Exit1:
	CMDFIFO_EPILOG(hwPtr);

  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}// blt32_doBltS

/*----------------------------------------------------------------------
Function name:  blt32_videoToVideo

Description:

Return:         DWORD
----------------------------------------------------------------------*/
DWORD __stdcall blt32_videoToVideo(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD srcPixelFormat, DWORD dstPixelFormat)
{
   DWORD dwBltFlags = pbd->dwFlags, rop3;

   if (DDBLT_ROP & dwBltFlags)
      rop3 = HIWORD(pbd->bltFX.dwROP);
   else
      rop3 = HIWORD(SRCCOPY);

#ifdef SLI
   if (SLI_MODE_ENABLED == _DD(sliMode))
      {
      return DdSli2DScn2Scn(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
      }
   else
#endif
      return (blt32_doBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat));

}// blt32_DoBltSP

//
//
// Big Note: The CSIM does not match the hardware.  The hardware can actually
// do a X stretch during a Host to Screen but it cannot do a Y stretch.
// Neither the Napalm or SST2 CSIM implemented this due to the fact that
// they consider this hardware solution insufficient.
// The Y stretch is done by replication.  It should be noted that this
// command is not defined in sst2flds.h <4> but it does exist.
//
//
#define CSIM_IS_BROKEN
#ifdef CSIM_IS_BROKEN
/*----------------------------------------------------------------------
Function name:  LoadLine

Description: This routine loads a single line of the Host Bit map
into the offscreen linear buffer. The routine setups the hardware for
the transfer and then does the transfer.  The hardware is reset so
that we can do just a bunch of small commands to do the transfers.

Return: Don't Care         

----------------------------------------------------------------------*/
typedef struct loadlineparams {
   DWORD bltSrcFormat;
   DWORD bltSrcSize;
   long int dwSize;
   DWORD * pData;
   DWORD bltDstBaseAddr;
   DWORD bltDstFormat;
   DWORD bltCommandExtra;
   DWORD bltDstSize;
   DWORD clip1min;
   DWORD clip1max;
} LOADLINEPARAMS, * PLOADLINEPARAMS;

int LoadLine(NT9XDEVICEDATA * ppdev, PLOADLINEPARAMS pLL)
{
   long int i;
   DWORD * pData;
   CMDFIFO_PROLOG(hwPtr);

   CMDFIFO_CHECKROOM(hwPtr, 9);
   SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit | dstFormatBit | commandExBit | clip1minBit | clip1maxBit | dstSizeBit | dstXYBit | commandBit));
   SETPD(hwPtr, ghw2D->dstBaseAddr, _FF(stretchBltStart));
   SETPD(hwPtr, ghw2D->dstFormat, pLL->bltSrcFormat);
   SETPD(hwPtr, ghw2D->commandEx, 0x0);
   SETPD(hwPtr, ghw2D->clip1min, 0x0);
   SETPD(hwPtr, ghw2D->clip1max, (1 << SST_WX_DST_Y_SHIFT) | 2048);
   SETPD(hwPtr, ghw2D->dstSize, pLL->bltSrcSize);
   SETPD(hwPtr, ghw2D->dstXY, 0x0);
   SETPD(hwPtr, ghw2D->command, (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT | (SST_WX_HOST_BLT << SST_WX_COMMAND_SHIFT));

   CMDFIFO_CHECKROOM(hwPtr, (DWORD)(pLL->dwSize + 1));
   // we should probably write these into the launch area 32 dwords
   // at a time, rather than one dword at a time
   SETPH(hwPtr, (SSTCP_PKT1 | SSTCP_PKT1_2D | (((FxU32)regoffsetof(Sst2WxRegs, launch[0])) << SSTCP_REGBASE_SHIFT) | ((pLL->dwSize) << SSTCP_PKT1_2D_NWORDS_SHIFT)));
   pData = pLL->pData;
   for (i=0; i<pLL->dwSize; i++)
      {
      SETPD(hwPtr, ghw2D->launch[0], *pData);
      pData++;
      }
   CMDFIFO_BUMP(hwPtr);

   // Reset Hardware
   CMDFIFO_CHECKROOM(hwPtr, 7);

   SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit | dstFormatBit | commandExBit | clip1minBit | clip1maxBit | dstSizeBit));
   SETPD(hwPtr, ghw2D->dstBaseAddr, pLL->bltDstBaseAddr);
   SETPD(hwPtr, ghw2D->dstFormat, pLL->bltDstFormat);
   SETPD(hwPtr, ghw2D->commandEx, pLL->bltCommandExtra);
   SETPD(hwPtr, ghw2D->clip1min, pLL->clip1min);
   SETPD(hwPtr, ghw2D->clip1max, pLL->clip1max);
   SETPD(hwPtr, ghw2D->dstSize, pLL->bltDstSize);

   CMDFIFO_EPILOG(hwPtr);

   return 0x0;
}

/*----------------------------------------------------------------------
Function name:  blt32_systemToVideo

Description:

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD 
__stdcall blt32_systemToVideo(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD srcPixelFormat, DWORD srcBytePerPixel, DWORD dstPixelFormat)
{
   DWORD bltDstXY;
   DWORD srcColorkeyMin, srcColorkeyMax, dstColorkeyMax, dstColorkeyMin;
   DWORD bltRop, dwBltFlags, bltCommand = 0;
   DWORD packetHeader = 0, bumpNum = 0x0;
   DWORD rop3;
   long dstLeft, dstTop, dstWidth, dstHeight, dstPitch;
   long srcWidth, srcHeight, srcPitch;
   long i, j, *scanline;
   int d;
   int incSrc;
   int incDst;
   LOADLINEPARAMS LL;
   char *srcOffset;
   FXSURFACEDATA *surfData;

   CMDFIFO_PROLOG(hwPtr);
   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   LL.bltCommandExtra = 0x0;

#ifdef FXTRACE
   DISPDBG((ppdev, DEBUG_APIENTRY, "Handle CPU To Screen" ));
#endif
   dwBltFlags = pbd->dwFlags;

   if (DDBLT_ROP & dwBltFlags)
      rop3 = HIWORD(pbd->bltFX.dwROP);
   else
      rop3 = HIWORD(SRCCOPY);

   if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
      {
      srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
      srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
      LL.bltCommandExtra |= SST_WX_EN_SRC_COLORKEY_EX;
      bumpNum += PH1_SIZE + 2;
      }

   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
      dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
      LL.bltCommandExtra |= SST_WX_EN_DST_COLORKEY_EX;
      bumpNum += PH1_SIZE + 2;
      bltCommand |= SST_WX_ROP_DST << SST_WX_ROP0_SHIFT;  // ROP0 = dst copy
      bltRop = rop3;                                  // ROP1 = rop3
      }
   else
      {
      bltCommand |= rop3 << SST_WX_ROP0_SHIFT;          // ROP0 = rop3
      bltRop = SST_WX_ROP_DST << 8;                     // ROP2 = dst copy
      }

   // The case with both src colorkey & dst colorkey used simultaneously
   // is not being handled.  What would be correct for that case

#ifdef AGP_EXECUTE
   if( pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
     LL.bltDstBaseAddr = GET_AGP_ADDR(pbd->lpDDDestSurface);
   else 
#endif
     LL.bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);

   surfData = (FXSURFACEDATA*)pbd->lpDDDestSurface->lpGbl->dwReserved1;
   BLTFMT_STRIDESTATE(surfData,dstPitch);

   // get rectangle
   dstTop = pbd->rDest.top;
   dstLeft = pbd->rDest.left;

   dstWidth = pbd->rDest.right - dstLeft;  // for now no stretch blit - SS
   dstHeight = pbd->rDest.bottom - dstTop;

   srcOffset = (char*) pbd->lpDDSrcSurface->lpGbl->fpVidMem;   // in byte
  
   //Need to be changed for SST2?? - AS 7/24/99
   srcPitch  = pbd->lpDDSrcSurface->lpGbl->lPitch;             // in byte

   srcWidth = pbd->rSrc.right - pbd->rSrc.left;
   srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

   LL.pData = (DWORD *) (srcOffset +  (pbd->rSrc.top * srcPitch) + (pbd->rSrc.left * srcBytePerPixel));

   // prepare register values
   BLTCLIP(dstLeft, dstTop, LL.clip1min);
   BLTCLIP(pbd->rDest.right, pbd->rDest.bottom, LL.clip1max);
   BLTFMT_DST(dstPitch, dstPixelFormat, LL.bltDstFormat);  // 16 bpp for now
   BLTXY(dstLeft, dstTop, bltDstXY);

   BLTFMT_SRC(srcPitch, srcPixelFormat, LL.bltSrcFormat);

   if ((dstWidth != srcWidth) || (dstHeight != srcHeight))
      bltCommand |= SST_WX_STRETCH_BLT | SST_WX_CLIPSELECT | SST_WX_GO;
   else
      bltCommand |= SST_WX_HOST_BLT | SST_WX_CLIPSELECT;

   // Compute the number of dwords to write per scanline
   // If there is a partial dword at the tail of the scanline
   // round the number of dwords up
   LL.dwSize = (srcWidth * srcBytePerPixel + 3) / 4;
   if (SST_WX_HOST_BLT == (SST_WX_HOST_BLT & bltCommand))  // Non-Stretch Blit
      {
      BLTSIZE(srcWidth, srcHeight, LL.bltSrcSize);
      BLTSIZE(dstWidth, dstHeight, LL.bltDstSize);
      // write to hw
      bumpNum += 13;
      packetHeader |=  dstBaseAddrBit | dstFormatBit | ropBit | commandExBit | clip1minBit | clip1maxBit | srcFormatBit | srcSizeBit | srcXYBit | dstSizeBit | dstXYBit | commandBit;

      CMDFIFO_CHECKROOM(hwPtr, bumpNum);

      if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
         {
         SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, srcColorkeyMin));
	      SETPD( hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin);
         SETPD( hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax);
         }
      if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
         {
         SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, dstColorkeyMin));
         SETPD( hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin);
         SETPD( hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax);
         }

      SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
      SETPD(hwPtr, ghw2D->dstBaseAddr, LL.bltDstBaseAddr);
      SETPD(hwPtr, ghw2D->dstFormat, LL.bltDstFormat);
      SETPD( hwPtr, ghw2D->rop, bltRop);
      SETPD( hwPtr, ghw2D->commandEx, LL.bltCommandExtra);
      SETPD( hwPtr, ghw2D->clip1min, LL.clip1min);
      SETPD( hwPtr, ghw2D->clip1max, LL.clip1max);
      SETPD( hwPtr, ghw2D->srcFormat, LL.bltSrcFormat);
      SETPD( hwPtr, ghw2D->srcSize, LL.bltSrcSize);
      SETPD( hwPtr, ghw2D->srcXY, 0);
      SETPD( hwPtr, ghw2D->dstSize, LL.bltDstSize);
      SETPD( hwPtr, ghw2D->dstXY, bltDstXY);
      SETPD( hwPtr, ghw2D->command, bltCommand);

      for( i = 0; i < srcHeight; i++)
         {
         scanline = (DWORD*)LL.pData;
         CMDFIFO_CHECKROOM(hwPtr, (unsigned long)(LL.dwSize + 1));
         // we should probably write these into the launch area 32 dwords
         // at a time, rather than one dword at a time
         SETPH(hwPtr, (SSTCP_PKT1 | SSTCP_PKT1_2D | (((FxU32)regoffsetof(Sst2WxRegs, launch[0])) << SSTCP_REGBASE_SHIFT) | ((LL.dwSize) << SSTCP_PKT1_2D_NWORDS_SHIFT)));

         for (j=0; j < LL.dwSize; j++)
            {
            SETPD( hwPtr, ghw2D->launch[0], *scanline );
            scanline++;
            }
         LL.pData = (DWORD *)((DWORD)LL.pData + srcPitch);
         CMDFIFO_BUMP( hwPtr );
         }
      }
   else  // Stretch Blt
      {
      BLTSIZE(srcWidth, 0x1, LL.bltSrcSize);
      BLTSIZE(dstWidth, 0x1, LL.bltDstSize);
      // These parameters will remain the same throughout this operation
      bumpNum += 6;
      packetHeader |=  ropBit | srcBaseAddrBit | srcFormatBit | srcSizeBit | srcXYBit ;

      CMDFIFO_CHECKROOM(hwPtr, bumpNum);

      if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
         {
         SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, srcColorkeyMin));
	      SETPD(hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin);
         SETPD(hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax);
         }
      if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
         {
         SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, dstColorkeyMin));
         SETPD(hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin);
         SETPD(hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax);
         }

      SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
      SETPD(hwPtr, ghw2D->rop, bltRop);
      SETPD(hwPtr, ghw2D->srcBaseAddr, _FF(stretchBltStart));
      SETPD(hwPtr, ghw2D->srcFormat, LL.bltSrcFormat);
      SETPD(hwPtr, ghw2D->srcSize, LL.bltSrcSize);
      SETPD(hwPtr, ghw2D->srcXY, 0);

      // We stretch in the Y by replicating scanline.  We do this
      // by using the MidPoint Algorithm.  See Foley, van Dam, Feiner, Hughes 1990
      // Pages 74-78.
      // This algorithm assumes that dstHeight is the major axis and will implement
      // by one per cycle.

      incDst = (srcHeight << 1);
      d = incDst - dstHeight;
      incSrc = (srcHeight - dstHeight) << 1;
      CMDFIFO_SAVE(hwPtr);   
      LoadLine(ppdev, &LL);
      CMDFIFO_RELOAD(hwPtr);   
      for (i=dstTop; i<pbd->rDest.bottom; i++)
         {
         // Do the Stretch....
         CMDFIFO_CHECKROOM(hwPtr, 3);
         SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstXYBit | commandBit));
         SETPD(hwPtr, ghw2D->dstXY, bltDstXY);
         SETPD(hwPtr, ghw2D->command, bltCommand);
         if (d <= 0) 
            d += incDst;
         else
            {
            d += incSrc;
            LL.pData = (DWORD *)((DWORD)LL.pData + srcPitch);
            CMDFIFO_SAVE(hwPtr);   
            LoadLine(ppdev, &LL);
            CMDFIFO_RELOAD(hwPtr);   
            }
         bltDstXY += (1 << SST_WX_SRC_Y_SHIFT);
         CMDFIFO_BUMP(hwPtr);
         }
      }

#if !defined(CMDFIFO) && !defined(H3_FIFO)
   P6FENCE;
#endif

  CMDFIFO_EPILOG( hwPtr );
  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}// blt32_systemToVideo

#else

/*----------------------------------------------------------------------
Function name:  blt32_systemToVideo

Description:

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall blt32_systemToVideo(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD srcPixelFormat, DWORD srcBytePerPixel, DWORD dstPixelFormat)
{
   DWORD           bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY, bltSrcFormat ;
   DWORD           srcColorkeyMin, srcColorkeyMax, dstColorkeyMax, dstColorkeyMin;
   DWORD           bltRop, dwBltFlags, bltCommand = 0, bltCommandExtra = 0;
   DWORD           packetHeader = 0, bumpNum = 13, clip1min, clip1max;
   DWORD           bltSrcSize;
   DWORD           rop3;
   long            dstLeft, dstTop, dstWidth, dstHeight, dstPitch;
   long            srcWidth, srcHeight, srcPitch;
   long            i, j, *scanline, num_dwWrite;
   int             errorterm;
   int             twodeltaX;
   int             twodeltaY;
   char            *srcOffset, *src;
   CMDFIFO_PROLOG(hwPtr);

   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

#ifdef FXTRACE
   DISPDBG((ppdev, DEBUG_APIENTRY, "Handle CPU To Screen" ));
#endif
   dwBltFlags = pbd->dwFlags;

   if (DDBLT_ROP & dwBltFlags)
      rop3 = HIWORD(pbd->bltFX.dwROP);
   else
      rop3 = HIWORD(SRCCOPY);

   if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
      {
      srcColorkeyMin = pbd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
      srcColorkeyMax = pbd->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
      bltCommandExtra |= SST_WX_EN_SRC_COLORKEY_EX;
      bumpNum += PH1_SIZE + 2;
      }

   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      dstColorkeyMin = pbd->bltFX.ddckDestColorkey.dwColorSpaceLowValue;
      dstColorkeyMax = pbd->bltFX.ddckDestColorkey.dwColorSpaceHighValue;
      bltCommandExtra |= SST_WX_EN_DST_COLORKEY_EX;
      bumpNum += PH1_SIZE + 2;
      bltCommand |= SST_WX_ROP_DST << SST_WX_ROP0_SHIFT;  // ROP0 = dst copy
      bltRop = rop3;                                  // ROP1 = rop3
      }
   else
      {
      bltCommand |= rop3 << SST_WX_ROP0_SHIFT;          // ROP0 = rop3
      bltRop = SST_WX_ROP_DST << 8;                     // ROP2 = dst copy
      }

   // The case with both src colorkey & dst colorkey used simultaneously
   // is not being handled.  What would be correct for that case

   bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);

   BLTFMT_STRIDESTATE(pbd->lpDDDestSurface->lpGbl,dstPitch);

   // get rectangle
   dstTop = pbd->rDest.top;
   dstLeft = pbd->rDest.left;

   dstWidth = pbd->rDest.right - dstLeft;  // for now no stretch blit - SS
   dstHeight = pbd->rDest.bottom - dstTop;

   srcOffset = (char*) pbd->lpDDSrcSurface->lpGbl->fpVidMem;   // in byte
  
   //Need to be changed for SST2?? - AS 7/24/99
   srcPitch  = pbd->lpDDSrcSurface->lpGbl->lPitch;             // in byte

   srcWidth = pbd->rSrc.right - pbd->rSrc.left;
   srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;

   src = (char*) ( srcOffset + ( pbd->rSrc.top * srcPitch) + (pbd->rSrc.left * srcBytePerPixel ) ) ;

   // prepare register values
   BLTCLIP(dstLeft, dstTop, clip1min);
   BLTCLIP(pbd->rDest.right, pbd->rDest.bottom, clip1max);
   BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);  // 16 bpp for now
   BLTSIZE(srcWidth, srcHeight, bltSrcSize);
   BLTSIZE(dstWidth, dstHeight, bltDstSize);
   BLTXY(dstLeft, dstTop, bltDstXY);

   BLTFMT_SRC(srcPitch, srcPixelFormat, bltSrcFormat);

   if ((dstWidth != srcWidth) || (dstHeight != srcHeight))
//      bltCommand |= SST_WX_STRETCH_BLT | SST_WX_CLIPSELECT;
      bltCommand |= (SST_WX_HOST_BLT + 1) | SST_WX_CLIPSELECT;
   else
      bltCommand |= SST_WX_HOST_BLT | SST_WX_CLIPSELECT;

   // write to hw
   packetHeader |=  dstBaseAddrBit | dstFormatBit | ropBit | commandExBit | clip1minBit | clip1maxBit | srcFormatBit | srcSizeBit | srcXYBit | dstSizeBit | dstXYBit | commandBit;

   CMDFIFO_CHECKROOM(hwPtr, bumpNum);

   if (dwBltFlags & DDBLT_KEYSRCOVERRIDE)
      {
      SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, srcColorkeyMin));
	   SETPD( hwPtr, ghw2D->srcColorkeyMin, srcColorkeyMin);
      SETPD( hwPtr, ghw2D->srcColorkeyMax, srcColorkeyMax);
      }
   if (dwBltFlags & DDBLT_KEYDESTOVERRIDE)
      {
      SETPH(hwPtr, CMDFIFO_BUILD_2DPK1(2, dstColorkeyMin));
      SETPD( hwPtr, ghw2D->dstColorkeyMin, dstColorkeyMin);
      SETPD( hwPtr, ghw2D->dstColorkeyMax, dstColorkeyMax);
      }

   SETPH(hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
   SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
   SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
   SETPD( hwPtr, ghw2D->rop, bltRop);
   SETPD( hwPtr, ghw2D->commandEx, bltCommandExtra);
   SETPD( hwPtr, ghw2D->clip1min, clip1min);
   SETPD( hwPtr, ghw2D->clip1max, clip1max);
   SETPD( hwPtr, ghw2D->srcFormat, bltSrcFormat);
   SETPD( hwPtr, ghw2D->srcSize, bltSrcSize);
   SETPD( hwPtr, ghw2D->srcXY, 0);
   SETPD( hwPtr, ghw2D->dstSize, bltDstSize);
   SETPD( hwPtr, ghw2D->dstXY, bltDstXY);
   SETPD( hwPtr, ghw2D->command, bltCommand);

   // Compute the number of dwords to write per scanline
   // If there is a partial dword at the tail of the scanline
   // round the number of dwords up
   num_dwWrite = (srcWidth * srcBytePerPixel + 3) / 4;

   if (SST_WX_HOST_BLT & bltCommand)  // Non-Stretch Blit
      {
      for( i = 0; i < srcHeight; i++)
         {
         scanline = (DWORD*)src;
         CMDFIFO_CHECKROOM(hwPtr, (unsigned long)(num_dwWrite + 1));
         // we should probably write these into the launch area 32 dwords
         // at a time, rather than one dword at a time
         SETPH(hwPtr, (SSTCP_PKT1 | SSTCP_PKT1_2D | (((FxU32)regoffsetof(Sst2WxRegs, launch[0])) << SSTCP_REGBASE_SHIFT) | ((num_dwWrite) << SSTCP_PKT1_2D_NWORDS_SHIFT)));

         for( j = 0; j < num_dwWrite; j++)
            {
            SETPD( hwPtr, ghw2D->launch[0], *scanline );
            scanline++;
            }
         src += srcPitch;
         CMDFIFO_BUMP( hwPtr );
         }
      }
   else  // Stretch Blt
      {
      //  We have to stretch in Y by replicating scanlines.  We do this using
      //  standard bresenham to tell us when it's time to move to the next
      //  source scanline.

      twodeltaX = (int)(srcHeight + srcHeight);
      twodeltaY = (int)(dstHeight + dstHeight);
      errorterm = twodeltaX + (int)srcHeight - twodeltaY;

      for (i = 0; i < dstHeight; i++)
         {
         scanline = (DWORD*)src;

         CMDFIFO_CHECKROOM( hwPtr, (unsigned long)(num_dwWrite + 1));
         // we should probably write these into the launch area 32 dwords
         // at a time, rather than one dword at a time
         SETPH(hwPtr, (SSTCP_PKT1 | SSTCP_PKT1_2D | (((FxU32)regoffsetof(Sst2WxRegs, launch[0])) << SSTCP_REGBASE_SHIFT) | ((num_dwWrite) << SSTCP_PKT1_2D_NWORDS_SHIFT)));

         for (j = 0; j < num_dwWrite; j++)
            {
            SETPD( hwPtr, ghw2D->launch[0], *scanline);
            scanline++;
            }

         CMDFIFO_BUMP( hwPtr );
 
         while (errorterm >= 0)
            {
            src += srcPitch;
            errorterm -= twodeltaY;
            }

         errorterm += twodeltaX;
         }
      }

#if !defined(CMDFIFO) && !defined(H3_FIFO)
   P6FENCE;
#endif

  CMDFIFO_EPILOG( hwPtr );
  pbd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}// blt32_systemToVideo
#endif

/*----------------------------------------------------------------------
Function name: DdBlt

Description:   DDRAW API Blt() - 32 blt routine.

Information:   Everything you need is in pdb->bltFX.
               Look at pdb->dwFlags to determine what kind of blt you are doing,
			   DDBLT_xxxx are the flags.

               ALPHA NOTES:
               Alpha ALWAYS comes in BLTFX.   You don't need to go
               looking for the attached surface. If DDBLT_ALPHA is
               specified, then either a constant alpha or alpha surface
               has been specified. Just look for DDBLT_ALPHASURFACEOVERRIDE
               or DDBLT_ALPHACONSTANTOVERRIDE Look for DDBLT_ALPHASURFACEDESTRECT
               to use the destination rectangle for choosing the rectangle
               in the alpha surface.   Otherwise use the source rectangle.

               Z BUFFER NOTES:
               ZBuffer ALWAYS comes in BLTFX.   You don't need to go
               looking for the attached surface. If DDBLT_ZBUFFER is
               specified, then either a constant z or z buffer surface
               has been specified for the source and destination. Just
               look for the DDBLT_ZBUFFERDESTOVERRIDE, DDBLT_ZBUFFERSRCOVERRIDE,
               DDBLT_ZBUFFERCONSTANTDESTOVERRIDE, or DDBLT_ZBUFFERCONSTANTSRCOVERRIDE.

               COLORKEY NOTES:
               ColorKey ALWAY comes in BLTFX.   You don't have to look it
               up in the surface.


Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
			   DDHAL_DRIVER_NOTHANDLED
			   DD_OK
----------------------------------------------------------------------*/

DWORD __stdcall DdBlt( LPDDHAL_BLTDATA pbd )
{
   DWORD                rop3, dst_dwCaps,  src_dwCaps;
   DWORD                dwDestSurfFlags, dwSrcSurfFlags;
   DWORD                dwBltFlags;
   DWORD                srcPixelFormat, dstPixelFormat, pixelByteDepth;
   FXSURFACEDATA        *srcSurfData = NULL;
   FXSURFACEDATA        *dstSurfData;

   DD_ENTRY_SETUP(pbd->lpDD);

#ifdef FXTRACE
   DISPDBG((ppdev, DEBUG_APIENTRY, "Blt32" ));
#endif

   dstSurfData = (FXSURFACEDATA*)(pbd->lpDDDestSurface->lpGbl->dwReserved1); 
   dst_dwCaps = pbd->lpDDDestSurface->ddsCaps.dwCaps;

   if (pbd->lpDDSrcSurface != NULL)
   {
       src_dwCaps = pbd->lpDDSrcSurface->ddsCaps.dwCaps;
       srcSurfData = (FXSURFACEDATA*)(pbd->lpDDSrcSurface->lpGbl->dwReserved1); 
   }

   // check source local surface data for aa hint flag
   if( srcSurfData ) // make sure there is a source before we reference it
   {
       if( srcSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING )  
       {
           // if surface is AA double size of source
          pbd->rSrc.left   *=2;
          pbd->rSrc.top    *=2;
          pbd->rSrc.right  *=2;
          pbd->rSrc.bottom *=2;
       }
   }

   // check destination surface for AA hint flag
   if( dstSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING )  
   {
           // if surface is AA double size of destination
        pbd->rDest.left   *=2;
        pbd->rDest.top    *=2;
        pbd->rDest.right  *=2;
        pbd->rDest.bottom *=2;
   }

   _DD(ddAcceleratorUsed) = 1;  // Notify flipping code.
   {
   CMDFIFO_PROLOG(cmdFifo);
   HW_ACCESS_ENTRY(cmdFifo,ACCESS_2D);
   // D3D-Bringup, be more selective later
   CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );
   SETMOP( cmdFifo,SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE | ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));
   CMDFIFO_EPILOG(cmdFifo);// subroutines may output to fifo, but this routine itself is done
   }
  
   // Texture download
   if (dst_dwCaps & DDSCAPS_TEXTURE)
   {
      if ((dst_dwCaps & DDSCAPS_VIDEOMEMORY)  &&    // destination in video
          (src_dwCaps & DDSCAPS_SYSTEMMEMORY) &&    // source in system
          (!(pbd->dwFlags & DDBLT_COLORFILL)))      // for now dont do color fills
      {
            DISPDBG((ppdev, DEBUG_DDDETAILS,"Blt32 - Texture download"));
            txtrLoad(ppdev, pbd->lpDDSrcSurface, &pbd->rSrc, 
                     pbd->lpDDDestSurface, &pbd->rDest);
            pbd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
      }
   }

   // texture check is redundant from above
   // what if there is more caps bits set than just SYSTEMMEMORY?
   //if( dst_dwCaps == DDSCAPS_TEXTURE  || dst_dwCaps == DDSCAPS_SYSTEMMEMORY)
   if (dst_dwCaps & DDSCAPS_SYSTEMMEMORY)
      {
      pbd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_NOTHANDLED;
      }

   // Get common used flags
   dwBltFlags = pbd->dwFlags;

   // Note that unlike Windows 95, Windows NT always guarantees that
   // there will be a valid 'ddpfSurface' structure
   dwDestSurfFlags = pbd->lpDDDestSurface->dwFlags;
   if (dwDestSurfFlags & DDRAWISURF_HASPIXELFORMAT)
      {
      // clear PALETTEINDEXED8 bit, so 8bpp falls into DDPF_RGB case
      switch (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFlags & ~(DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS))
         {
         case DDPF_RGB :
            pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwRGBBitCount ) >> 3;
            GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
#ifdef AGP_EXECUTE
            if( dst_dwCaps & DDSCAPS_NONLOCALVIDMEM)
              dstPixelFormat |= SST_WX_DST_AGP;
#endif
            break;

         case DDPF_ZBUFFER :
            pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwZBufferBitDepth ) >> 3;
            if ((pixelByteDepth != 2) && (pixelByteDepth != 4) )  // 16 or 32 (24z/8s) bit depth
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return DDHAL_DRIVER_HANDLED;
               }
            GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
            break;

         case DDPF_FOURCC:
            pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
            if (pixelByteDepth != 2)
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return (DDHAL_DRIVER_HANDLED);
               }

            if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_YUY2)
               {
               dstPixelFormat = SST_WX_PIXFMT_422YUV;
               }
            else if (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_UYVY)
               {
               dstPixelFormat = SST_WX_PIXFMT_422UYV;
               }
            else
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return DDHAL_DRIVER_HANDLED;
               }
            break;

         default:
            pbd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_NOTHANDLED;
            break;
         }
      }
   else
      {
      if (dst_dwCaps == DDSCAPS_ZBUFFER)
         {
         pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwZBufferBitDepth ) >> 3;
         }
      else
         {
         pixelByteDepth = GETPRIMARYBYTEDEPTH;
         }
      GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);
#ifdef AGP_EXECUTE
      if( dst_dwCaps & DDSCAPS_NONLOCALVIDMEM)
          dstPixelFormat |= SST_WX_DST_AGP;
#endif
      }

   if (dstPixelFormat ==  INVALID_PIXELFORMAT)
      {
      pbd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_NOTHANDLED;
      }

   // ColorFill
   if ( dwBltFlags & (DDBLT_COLORFILL | DDBLT_DEPTHFILL))
      {

        // checking for blts the hw can't do
       if ((SST_WX_PIXFMT_422YUV == dstPixelFormat) || (SST_WX_PIXFMT_422UYV == dstPixelFormat))
       {
       // if it's a YUY2-YUY2 or UYVY-UYVY blt, tell the hw its 565 rgb data
       // and let the blt go thru
         dstPixelFormat = SST_WX_PIXFMT_16BPP;
       }
#ifdef SLI
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         return Sli_ColorFill(ppdev, pbd, dstPixelFormat);
         }
      else
#endif
#ifdef AGP_EXECUTE
         { 
           if( dst_dwCaps & DDSCAPS_NONLOCALVIDMEM )
             dstPixelFormat |= SST_WX_DST_AGP;
         }
#endif
         return (blt32_colorFill(ppdev, pbd, dstPixelFormat));
      }

      // whiteness and blackness rops
   if (!ROP_HAS_SRC(HIWORD(pbd->bltFX.dwROP)))
      {
      rop3 = HIWORD(pbd->bltFX.dwROP);
#ifdef SLI
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         return DdSliBltNoSP(ppdev, pbd, rop3, dstPixelFormat);
         }
      else
#endif
#ifdef AGP_EXECUTE
         {
           if( dst_dwCaps & DDSCAPS_NONLOCALVIDMEM)
             dstPixelFormat |= SST_WX_DST_AGP;
         }
#endif
         return (blt32_doBltNoSP(ppdev, pbd, rop3, dstPixelFormat));
      }

   if (src_dwCaps == DDSCAPS_TEXTURE)
      {
      pbd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_NOTHANDLED;
      }

   //check src pixel format
   // Note that unlike Windows 95, Windows NT always guarantees that
   // there will be a valid 'ddpfSurface' structure
   dwSrcSurfFlags = pbd->lpDDSrcSurface->dwFlags;
   if (dwSrcSurfFlags & DDRAWISURF_HASPIXELFORMAT)
      {
      // clear PALETTEINDEXED8 bit, so 8bpp falls into DDPF_RGB case
      switch (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & ~(DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS))
         {
         case DDPF_RGB:
            pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwRGBBitCount) >> 3;
            GETPIXELFORMAT(pixelByteDepth, srcPixelFormat);
#ifdef AGP_EXECUTE
            if( src_dwCaps & DDSCAPS_NONLOCALVIDMEM)
              srcPixelFormat |= SST_WX_SRC_AGP;
#endif
            break;

         case DDPF_FOURCC:         // Handle fourcc surfaces -- YUV,UYV.
            pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
            if (pixelByteDepth != 2)
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return (DDHAL_DRIVER_HANDLED);
               }

            if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_YUY2)
               {
               srcPixelFormat = SST_WX_PIXFMT_422YUV;
               }
            else if (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC==FOURCC_UYVY)
               {
               srcPixelFormat = SST_WX_PIXFMT_422UYV;
               }
            else
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return(DDHAL_DRIVER_HANDLED);
               }
            break;

         case DDPF_YUV :
            pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwYUVBitCount ) >> 3;
            if (pixelByteDepth != 2)
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return DDHAL_DRIVER_HANDLED;
               }
            srcPixelFormat = SST_WX_PIXFMT_422YUV;
            break;

         case DDPF_ZBUFFER :
            pixelByteDepth = (pbd->lpDDSrcSurface->lpGbl->ddpfSurface.dwZBufferBitDepth ) >> 3;
            if ((pixelByteDepth != 2) && (pixelByteDepth != 4))  // 16 or 32 (24z/8s) bit depth
               {
               pbd->ddRVal = DDERR_UNSUPPORTED;
               return DDHAL_DRIVER_HANDLED;
               }
            GETPIXELFORMAT(pixelByteDepth, srcPixelFormat);
            break;

         default:
            pbd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_NOTHANDLED;
         }
      }
   else
      {
      if (src_dwCaps & DDSCAPS_ZBUFFER)
         {
         pixelByteDepth = (pbd->lpDDDestSurface->lpGbl->ddpfSurface.dwZBufferBitDepth ) >> 3;
         }
      else
         {
         pixelByteDepth = GETPRIMARYBYTEDEPTH;
         }
      GETPIXELFORMAT(pixelByteDepth, srcPixelFormat);
#ifdef AGP_EXECUTE
      if( src_dwCaps & DDSCAPS_NONLOCALVIDMEM)
        srcPixelFormat |= SST_WX_SRC_AGP;
#endif
      }

   if (srcPixelFormat ==  INVALID_PIXELFORMAT)
      {
      pbd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_NOTHANDLED;
      }

   // error checking for blts the hw can't do
   if ((SST_WX_PIXFMT_422YUV == dstPixelFormat) || (SST_WX_PIXFMT_422UYV == dstPixelFormat))
      {
      // if it's a YUY2-YUY2 or UYVY-UYVY blt, tell the hw its 565 rgb data
      // and let the blt go thru
      if (srcPixelFormat == dstPixelFormat)
         {
         srcPixelFormat = dstPixelFormat = SST_WX_PIXFMT_16BPP;
         }
      // fail YUY2-UYVY and UYVY-YUY2 color conversion blts
      else
         {
         pbd->ddRVal = DDERR_UNSUPPORTED;
         return DDHAL_DRIVER_HANDLED;
         }
      }

   // the hw can't handle bltting from an 8bpp src to a non-8bpp dst
   // or from a non-8bpp src to an 8bpp dst
   if ((srcPixelFormat != dstPixelFormat) && ((SST_WX_PIXFMT_8BPP == dstPixelFormat) || (SST_WX_PIXFMT_8BPP == srcPixelFormat)))
      {
      pbd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
      }

   //  SRCCOPY
   // Video to Video
   if (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY && pbd->lpDDSrcSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
      {
      return (blt32_videoToVideo(ppdev, pbd, srcPixelFormat, dstPixelFormat));
      }
   // Video to System
   else if ((pbd->lpDDSrcSurface->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) && (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
      {
#ifdef SLI
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         return DdSli2DSystem2Scn(ppdev, pbd,  srcPixelFormat, pixelByteDepth, dstPixelFormat);
         }
      else
#endif
#ifdef AGP_EXECUTE
         {
           if( dst_dwCaps & DDSCAPS_NONLOCALVIDMEM)
             dstPixelFormat |= SST_WX_DST_AGP;
         }
#endif
         return (blt32_systemToVideo(ppdev, pbd, srcPixelFormat, pixelByteDepth, dstPixelFormat));
      }

  return DD_OK;
} /* DdBlt */
