/* -*-c++-*- */
/* $Header: ddfxs32.c, 26, 11/5/00 3:09:42 PM PST, Ryan Bissell$ */
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
** File Name: 	DDFXS32.C
**
** Description: 
**
** $Revision: 26$
** $Date: 11/5/00 3:09:42 PM PST$
**
** $History: ddfxs32.c $
** 
** *****************  Version 83  *****************
** User: Andrew       Date: 9/07/99    Time: 10:37a
** Updated in $/devel/sst2/Win95/dx/dd32
** Changes for SLI
** 
** *****************  Version 82  *****************
** User: Einkauf      Date: 8/31/99    Time: 2:00p
** Updated in $/devel/sst2/Win95/dx/dd32
** DirectX CMD FIFO
** 
** *****************  Version 81  *****************
** User: Xingc        Date: 8/27/99    Time: 7:41p
** Updated in $/devel/sst2/Win95/dx/dd32
** FXSURFACEDATA structure change
** 
** *****************  Version 80  *****************
** User: Agus         Date: 8/16/99    Time: 10:44a
** Updated in $/devel/sst2/Win95/dx/dd32
** Clean up compiler warnings in promote_primarytooverlay and hwsetpalette
** 
** *****************  Version 79  *****************
** User: Agus         Date: 8/13/99    Time: 2:01p
** Updated in $/devel/sst2/Win95/dx/dd32
** Added primary overlay promotion/demotion for SST2
** 
** *****************  Version 78  *****************
** User: Peterm       Date: 7/30/99    Time: 2:41a
** Updated in $/devel/sst2/Win95/dx/dd32
** modified for updated fxsurfacedata struct
** 
** *****************  Version 77  *****************
** User: Peterm       Date: 6/04/99    Time: 2:01a
** Updated in $/devel/sst2/Win95/dx/dd32
** updated for changed inc\sst* and others
** 
** *****************  Version 76  *****************
** User: Peterm       Date: 6/03/99    Time: 11:25p
** Updated in $/devel/sst2/Win95/dx/dd32
** modified to run with H3 tot (adds multimon, various bug fixes, and many
** structural deltas)
** 
**
*/

#include "precomp.h"
#include "ddovl32.h"
#include "sst2glob.h"
#include "sst2bwe.h"
#include "fifomgr.h"
#include "regkeys.h"


#include "..\dd16\hwcext.h"


#define PROMOTE_PRIMARYTOOVERLAY(arg1,arg2,arg3,arg4) promote_PrimaryToOverlay(arg1,arg2,arg3,arg4)
#define DEMOTE_3DTONONEOVERLAY(arg1) demote_3DToNoneOverlay(arg1)
#define LOADGAMMA(arg1) LoadGamma(arg1)
#define RESTOREGAMMA(arg1) RestoreGamma(arg1)
#define HWSETPALETTE(arg1,arg2,arg3,arg4) HwSetPalette(arg1,arg2,arg3,arg4)
#define TXTRNEWPALETTE(arg1,arg2) txtrNewPalette(arg1,arg2)
#define TXTRCHANGEDPALETTE(arg1,arg2) txtrChangedPalette(arg1,arg2)

#ifdef FXTRACE
#ifdef MM
#define DUMP_CREATEPALETTEDATA(arg1,arg2,arg3) Dump_CREATEPALETTEDATA(arg1,arg2,arg3)
#else
#define DUMP_CREATEPALETTEDATA(arg1,arg2,arg3) Dump_CREATEPALETTEDATA(arg2,arg3)
#endif
#endif

#ifdef MM
#define FLUSHAGP(arg1) flushAgp(arg1)
#else
#define FLUSHAGP(arg1) flushAgp()
#endif

int LOADGAMMA(NT9XDEVICEDATA * ppdev);
int RESTOREGAMMA(NT9XDEVICEDATA * ppdev);
HWSETPALETTE(NT9XDEVICEDATA * ppdev, DWORD FAR *pGamma, int nStart, int nSize);

#ifdef SLI
#include <ddsli.h>
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

/*
 *******************************************************************
 *  DirectDraw Object Callbacks
 *******************************************************************
 */


/*----------------------------------------------------------------------
Function name: DdDestroyDriver

Description:   destroy the driver

Return:        DWORD 

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdDestroyDriver( LPDDHAL_DESTROYDRIVERDATA pcsd )
{
  DD_ENTRY_SETUP(pcsd->lpDD);
  return DDHAL_DRIVER_HANDLED;
}// DdDestroyDriver



/*----------------------------------------------------------------------
Function name: SetBusy

Description:   

Return:        
----------------------------------------------------------------------*/
int Set_Busy(WORD * pFlags);



/*----------------------------------------------------------------------
Function name:  AABltToPrimary

Description:    Copy the secondary buffer AA contents to a non-AA primary buffer
                AA modes which use this can only be turned on from the registry.                

Return:         void
----------------------------------------------------------------------*/

void __stdcall AABltToPrimary( NT9XDEVICEDATA *ppdev, LPDDHAL_FLIPDATA pfd, DWORD dwHWDstAddress )
{
   DWORD    dwPacketHeader;
   DWORD    dwClip1min, dwClip1max;
   DWORD    dwBltRop;
   DWORD    dwBltSrcSize;
   DWORD    dwBltSrcXY;
   DWORD    dwBltDstSize;
   DWORD    dwBltDstXY;
   DWORD    dwBltCommand;
   DWORD    dwSrcPixelFormat;
   DWORD    dwDstPixelFormat;
   DWORD    dwBltSrcFormat;
   DWORD    dwBltDstFormat;
   DWORD    dwBltSrcBaseAddr;
   DWORD    dwBltDstBaseAddr;
   DWORD    dwWidth;
   DWORD    dwHeight;
   DWORD    dwByteDepth;
   DWORD    dwSrcPitch;
   DWORD    dwDstPitch;

   CMDFIFO_PROLOG(hwPtr);
   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY;  // The 2D BLTs will change bltDstBaseAddr

   CMDFIFO_CHECKROOM( hwPtr, MOP_SIZE );
   SETMOP( hwPtr,SST_MOP_STALL_3D | 
                    SST_MOP_FLUSH_TCACHE | 
                    SST_MOP_FLUSH_PCACHE |
                    ((SST_MOP_STALL_3D_TA|
                      SST_MOP_STALL_3D_TD|
                      SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));



    // mls note - I'm assuming tiled memory for this since we are in fullscreen 3d mode
    dwDstPitch = 0x8000000 | _FF(ddPrimarySurfaceData).dwPStride;
    dwSrcPitch = 0x8000000 | _FF(ddPrimarySurfaceData).dwPStride*2;
    dwByteDepth = (DWORD) GETPRIMARYBYTEDEPTH;

    GETPIXELFORMAT(dwByteDepth, dwSrcPixelFormat);
    BLTFMT_SRC(dwSrcPitch, dwSrcPixelFormat, dwBltSrcFormat);
 
    GETPIXELFORMAT(dwByteDepth, dwDstPixelFormat);
    BLTFMT_SRC(dwDstPitch, dwDstPixelFormat, dwBltDstFormat);

    // the dst address is either the third buffer or the primary buffer
    dwBltDstBaseAddr = dwHWDstAddress;  

    // the src address is always first back buffer. (which is always super sized for aa)
    dwBltSrcBaseAddr = _DD(ddAASecondaryBuffer1HW); 

    dwWidth          = _FF(ddPrimarySurfaceData).dwWidth; 
    dwHeight         = _FF(ddPrimarySurfaceData).dwHeight; 

    switch( _DD(ddFSAAMode)  )   
    {
        case  FSAA_MODE_4XBLT:
            dwBltSrcFormat |= SST_WX_SRC_EN_AA;  // our src is aa - dest is not
            break;
        default:
            break;
    }
    
    dwBltRop   = 0;
    dwClip1min = 0;
    dwBltSrcXY = 0;
    dwBltDstXY = 0;
    switch( _DD(ddFSAAMode)  )
    {
        case  FSAA_MODE_4XBLT:
            BLTCLIP(dwWidth*2, dwHeight*2, dwClip1max);    
            BLTSIZE(dwWidth*2, dwHeight*2, dwBltSrcSize);
            break;
        default:
            // use this if we switch to demo mode ( hotkey )
            BLTCLIP(dwWidth, dwHeight, dwClip1max);  
            BLTSIZE(dwWidth, dwHeight, dwBltSrcSize);
            break;
    }
    BLTSIZE(dwWidth,   dwHeight,   dwBltDstSize);
    dwBltCommand = 0;
    dwBltCommand |= SST_WX_BLT | SST_WX_GO | SST_WX_CLIPSELECT | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT);


    CMDFIFO_CHECKROOM(hwPtr, 12);

    dwPacketHeader = 0;
    dwPacketHeader |= dstBaseAddrBit
                   | dstFormatBit
                   | ropBit
                   | srcBaseAddrBit
                   | clip1minBit
                   | clip1maxBit
                   | srcFormatBit
                   | srcXYBit
                   | srcSizeBit
                   | dstSizeBit
                   | dstXYBit
                   | commandBit;
 
    SETPH(hwPtr, CMDFIFO_BUILD_PK2(dwPacketHeader));
	SETPD(hwPtr, ghw2D->dstBaseAddr, dwBltDstBaseAddr);
	SETPD(hwPtr, ghw2D->dstFormat,   dwBltDstFormat);
	SETPD(hwPtr, ghw2D->rop,         dwBltRop);
	SETPD(hwPtr, ghw2D->srcBaseAddr, dwBltSrcBaseAddr);
	SETPD(hwPtr, ghw2D->clip1min,    dwClip1min);
	SETPD(hwPtr, ghw2D->clip1max,    dwClip1max);
	SETPD(hwPtr, ghw2D->srcFormat,   dwBltSrcFormat);
	SETPD(hwPtr, ghw2D->srcSize,     dwBltSrcSize);
	SETPD(hwPtr, ghw2D->srcXY,       dwBltSrcXY);
	SETPD(hwPtr, ghw2D->dstSize,     dwBltDstSize);
	SETPD(hwPtr, ghw2D->dstXY,       dwBltDstXY);
	SETPD(hwPtr, ghw2D->command,     dwBltCommand);


   HW_ACCESS_EXIT(ACCESS_2D);
   CMDFIFO_EPILOG(hwPtr);
}




/*----------------------------------------------------------------------
Function name:  FixPrimaryBufferForAA

Description:    Copy the secondary buffer contents to the primary buffer while
                starting an AA fullscreen app.
                This is primarily for weird apps like winbench that blt to the 
                primary before we start treating it as tiled memory. Once 2 
                flips have occured we can be reasonably certain the backbuffer 
                has valid contents, while the primary may still have desktop 
                garbage on it.

Return:         void
----------------------------------------------------------------------*/

void __stdcall FixPrimaryBufferForAA( NT9XDEVICEDATA *ppdev, LPDDHAL_FLIPDATA pfd )
{
   DWORD    dwPacketHeader;
   DWORD    dwClip1min, dwClip1max;
   DWORD    dwBltRop;
   DWORD    dwBltSrcSize;
   DWORD    dwBltSrcXY;
   DWORD    dwBltDstSize;
   DWORD    dwBltDstXY;
   DWORD    dwBltCommand;
   DWORD    dwSrcPixelFormat;
   DWORD    dwBltSrcFormat;
   DWORD    dwBltDstFormat;
   DWORD    dwBltSrcBaseAddr;
   DWORD    dwBltDstBaseAddr;
   DWORD    dwWidth;
   DWORD    dwHeight;
   DWORD    dwByteDepth;
   DWORD    dwSrcPitch;
   FXSURFACEDATA *srcSurfData;

   CMDFIFO_PROLOG(hwPtr);
   HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY;  // The 2D BLTs will change bltDstBaseAddr

    CMDFIFO_CHECKROOM( hwPtr, MOP_SIZE );
    SETMOP( hwPtr,SST_MOP_STALL_3D | 
                    SST_MOP_FLUSH_TCACHE | 
                    SST_MOP_FLUSH_PCACHE |
                    ((SST_MOP_STALL_3D_TA|
                      SST_MOP_STALL_3D_TD|
                      SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));


    srcSurfData = (FXSURFACEDATA*)(pfd->lpSurfCurr->lpGbl->dwReserved1);
    BLTFMT_STRIDESTATE(srcSurfData,dwSrcPitch);
    dwByteDepth = (DWORD) GETPRIMARYBYTEDEPTH;
    GETPIXELFORMAT(dwByteDepth, dwSrcPixelFormat);
    BLTFMT_SRC(dwSrcPitch, dwSrcPixelFormat, dwBltSrcFormat);

    dwBltDstBaseAddr = _DS(gdiDesktopStart);                 // get the primary buffer address
    dwBltSrcBaseAddr = GET_HW_ADDR(pfd->lpSurfTarg);         // try to get a secondary buffer address
    if( dwBltSrcBaseAddr == dwBltDstBaseAddr )               // if this was not a secondary buffer
    {
        dwBltSrcBaseAddr = GET_HW_ADDR(pfd->lpSurfCurr);         // then this HAS to be a secondary buffer address
    }

    dwWidth          = _FF(ddPrimarySurfaceData).dwWidth;  
    dwHeight         = _FF(ddPrimarySurfaceData).dwHeight;

    dwBltDstFormat = dwBltSrcFormat;
    dwBltRop   = 0;
    dwClip1min = 0;
    dwBltSrcXY = 0;
    dwBltDstXY = 0;
    BLTCLIP(dwWidth, dwHeight, dwClip1max);
    BLTSIZE(dwWidth, dwHeight, dwBltSrcSize);
    BLTSIZE(dwWidth, dwHeight, dwBltDstSize);
    dwBltCommand = 0;
    dwBltCommand |= SST_WX_BLT
                 | SST_WX_GO
                 | SST_WX_CLIPSELECT
                 | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT);


    CMDFIFO_CHECKROOM(hwPtr, 12);

    dwPacketHeader = 0;
    dwPacketHeader |= dstBaseAddrBit
                   | dstFormatBit
                   | ropBit
                   | srcBaseAddrBit
                   | clip1minBit
                   | clip1maxBit
                   | srcFormatBit
                   | srcXYBit
                   | srcSizeBit
                   | dstSizeBit
                   | dstXYBit
                   | commandBit;
 
    SETPH(hwPtr, CMDFIFO_BUILD_PK2(dwPacketHeader));
	SETPD(hwPtr, ghw2D->dstBaseAddr, dwBltDstBaseAddr);
	SETPD(hwPtr, ghw2D->dstFormat,   dwBltDstFormat);
	SETPD(hwPtr, ghw2D->rop,         dwBltRop);
	SETPD(hwPtr, ghw2D->srcBaseAddr, dwBltSrcBaseAddr);
	SETPD(hwPtr, ghw2D->clip1min,    dwClip1min);
	SETPD(hwPtr, ghw2D->clip1max,    dwClip1max);
	SETPD(hwPtr, ghw2D->srcFormat,   dwBltSrcFormat);
	SETPD(hwPtr, ghw2D->srcSize,     dwBltSrcSize);
	SETPD(hwPtr, ghw2D->srcXY,       dwBltSrcXY);
	SETPD(hwPtr, ghw2D->dstSize,     dwBltDstSize);
	SETPD(hwPtr, ghw2D->dstXY,       dwBltDstXY);
	SETPD(hwPtr, ghw2D->command,     dwBltCommand);


   HW_ACCESS_EXIT(ACCESS_2D);
   CMDFIFO_EPILOG(hwPtr);
}





/*----------------------------------------------------------------------
Function name: promote_PrimaryToOverlay

Description:   

Return:        DWORD DDRAW result
               DD_OK
----------------------------------------------------------------------*/
DWORD PROMOTE_PRIMARYTOOVERLAY(
  NT9XDEVICEDATA * ppdev,
  DWORD width,
  DWORD height,
  FXSURFACEDATA *pSurfaceData )
{

  CMDFIFO_PROLOG(hwPtr);

  CMDFIFO_CHECKROOM(hwPtr, 2 * MOP_SIZE);
  SETMOP(hwPtr, SST_MOP_STALL_2D); 
  SETMOP(hwPtr, SST_MOP_STALL_3D |
               (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));

  CMDFIFO_EPILOG(hwPtr);

  if (LOW_POWER_MODE(SaveBusy))
  {
    return DD_OK;
  }
  LOADGAMMA(ppdev);

#ifdef SLI
  CMDFIFO_SAVE(hwPtr);
  Enable_SLI(ppdev); 
  CMDFIFO_RELOAD(hwPtr);
#endif

  if (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY))
  {
      OVL_REG sOVLRegs;
      DWORD   dwOn_Dur;
      DWORD   dwPmCfg;
      DWORD   dwByteCount;
      DWORD   dwAA_mode = 0; // needed for fullscreen AA support

      memset(&sOVLRegs, 0, sizeof(OVL_REG));

      // VD_SET_FIELD(sOVLRegs.dwCfg0, SST_VO_PV_LUT_SEL, SST_VD_BYPASS_LUT);
      sOVLRegs.dwCfg1 = GET(ghwVD->vdVoPvCfg1);

      //Overlay pixel format setup
      dwByteCount = pSurfaceData->dwBytesPerPixel;
      
      switch (pSurfaceData->dwPixelFormat)
      {  
         case DDPF_RGB :
           switch (dwByteCount)
           {
               case 1:
                  VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_8BPP); 
                  break;
               case 2:
                   // mls - if 16 bit aa mode is on we need to set RGB32 here because video HW will treat it as 32 bits anyway
                  switch ( _DD(ddFSAAMode) )
                  {
                     // primary will be AA sized only for AA flip modes
                     case  FSAA_MODE_4XFLIP:
                        VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_RGB32); 
                        break;
                     default:
                        VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_RGB565); 
                        break;
                  }
                  break;
               case 4:
                  VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_RGB32);
                  break;
               default:
                  return DDERR_INVALIDPIXELFORMAT; //Unsupported RGB pixel depth
           }
           break;
           
         case FOURCC_YUY2 :
           VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_YUV_YUYV);
           //sOVLRegs.dwCfg0 |= (SST_VO_PV_CSC_CTL | SST_VO_PV_GMA_CTL);//2.2 gamma ?
           sOVLRegs.dwCfg0 |= SST_VO_PV_CSC_CTL;
           break;
           
         case FOURCC_UYVY:
           VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_YUV_UYVY); 
           //sOVLRegs.dwCfg0 |= (SST_VO_PV_CSC_CTL | SST_VO_PV_GMA_CTL);//2.2 gamma ?
           sOVLRegs.dwCfg0 |= SST_VO_PV_CSC_CTL;
           break;

         case DDPF_YUV:
           VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT, SST_VD_YUVA); 
           //sOVLRegs.dwCfg0 |= (SST_VO_PV_CSC_CTL | SST_VO_PV_GMA_CTL);//2.2 gamma ?
           sOVLRegs.dwCfg0 |= SST_VO_PV_CSC_CTL;
           break;

         default:
           return DDERR_INVALIDPIXELFORMAT; //Unsupported pixel format
      }
              
      //Disable overlay alpha 
      SETDW(ghwVD->vdVoPvAlpha0, (6 << SST_VO_PV_ALPHA_BITS_SHIFT) |
                                  SST_VO_PV_ALPHA_INV |  //always invert alpha
                                  (SST_VD_ALPHA_EXTRACT << SST_VO_PD_ALPHA_CREATE_SHIFT) );
      SETDW(ghwVD->vdVoPvAlpha1, 0);

      //Set vertical filtering to BYPASS mode for now  
      VD_SET_FIELD(sOVLRegs.dwCfg0, SST_VO_PV_VFIL_MODE, SST_VD_BYPASS);
 
      sOVLRegs.dwAddrCtl = (pSurfaceData->dwPStride) << SST_VO_PV_STRIDE_SHIFT;
      
      if((pSurfaceData->tileFlag & MEM_IN_TILE0))
      {
          sOVLRegs.dwAddrCtl |= SST_VD_TILE_MODE_0 << SST_VO_PV_TILE_SHIFT;
      }
      else if((pSurfaceData->tileFlag & MEM_IN_TILE1))
      {
          sOVLRegs.dwAddrCtl |= SST_VD_TILE_MODE_1 << SST_VO_PV_TILE_SHIFT;
      }

      VD_SET_FIELD( sOVLRegs.dwCfg2, SST_VO_PV_TARGET_WIDTH, width );
      VD_SET_FIELD( sOVLRegs.dwCfg2, SST_VO_PV_TARGET_HEIGHT, height );
      VD_SET_FIELD( sOVLRegs.dwCfg3, SST_VO_PV_V_OFS, 0 );
      VD_SET_FIELD( sOVLRegs.dwCfg4, SST_VO_PV_V_INC, 0x10000 ); //1:1
      VD_SET_FIELD( sOVLRegs.dwCfg4, SST_VO_PV_SOURCE_HEIGHT, height-1 );

      dwOn_Dur = ( width * dwByteCount + 31) >> 5;
      
      VD_SET_FIELD( sOVLRegs.dwCfg5, SST_VO_PV_ON_DUR, dwOn_Dur); //Number of 32byte mem chunk to fetch
      VD_SET_FIELD( sOVLRegs.dwCfg6, SST_VO_PV_H_INC, 0x10000 );  //1:1
      VD_SET_FIELD( sOVLRegs.dwCfg7, SST_VO_PV_H_START, 0 );
      VD_SET_FIELD( sOVLRegs.dwCfg7, SST_VO_PV_V_START, 0 );


      VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_SWAP_CNT,1);              //Set frame count between swap
      VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_CE_SWAP); //Use address in 3dspace->leftOverlayBuf
      sOVLRegs.dwBaseLeft = pSurfaceData->hwPtr;                         //Initial base address
        
      _FF(ddPmCfgSave) = GET( ghwVD->vdVoPmCfg); //Save current mixer config and set mixer config to 100% overlay
      dwPmCfg = (SST_VD_LAYER_XPARENT << SST_VO_PM_LYR1_SEL_SHIFT) | (SST_VD_LAYER_PV << SST_VO_PM_LYR2_SEL_SHIFT);
      SETDW( ghwVD->vdVoPmCfg, dwPmCfg);
     
      // dwAA_mode will only be set for aa flip modes - blt mode we leave the primary as normal surface.
      switch( _DD(ddFSAAMode)  )
      {
        // handle fullscreen AA modes
        case  FSAA_MODE_4XFLIP:
           switch (dwByteCount)
           {
               case 1:
                  dwAA_mode = 0; // not supported
                  break;
               case 2:
                  dwAA_mode = 6; // aa mode set for 565
                  break;
               case 4:
                  dwAA_mode = 3; // aa mode set for 32bit ARGB
                  break;
               default:
                  dwAA_mode = 0; //Unsupported RGB pixel depth
           }
           break;
        default:
           break;
      }

      
      //Set video overlay registers:
      SETDW(ghwVD->vdVoPvCfg0, sOVLRegs.dwCfg0);
      SETDW(ghwVD->vdVoPvCfg1, sOVLRegs.dwCfg1);
      SETDW(ghwVD->vdVoPvCfg2, sOVLRegs.dwCfg2);
      SETDW(ghwVD->vdVoPvCfg3, sOVLRegs.dwCfg3);
      SETDW(ghwVD->vdVoPvCfg4, sOVLRegs.dwCfg4);
      SETDW(ghwVD->vdVoPvCfg5, sOVLRegs.dwCfg5);
      SETDW(ghwVD->vdVoPvCfg6, sOVLRegs.dwCfg6);
      SETDW(ghwVD->vdVoPvCfg7, sOVLRegs.dwCfg7);
      // if aa is not being used then dwAA_mode will be 0
      SETDW(ghwVD->vdVoPvAddrCtl , (sOVLRegs.dwAddrCtl | dwAA_mode << SST_VO_PD_AA_MODE_SHIFT) ); 

      //Update video overlay registers, and primary surface registers
      SETDW(ghwVD->vdVoPsStatus0, GET(ghwVD->vdVoPsStatus0)|SST_VO_PV_UPDATE|SST_VO_PS_UPDATE);

      //  Set driver's swap count to CE's current so compare of ddLastSwapCount and
      //  CE's current count will be accurate
      _DD(ddLastSwapCount) = READSWAPCOUNT(ppdev);

      // Note that we are letting CE control update - not hitting PS_STATUS
      // Queue the swap - this will cause CE to issue swap req to video, which will accept new address
      // Queue stall until swap - don't want to start render until new render buffer not visible
      HW_ACCESS_ENTRY(hwPtr, ACCESS_3D);
      CMDFIFO_CHECKROOM( hwPtr, PH1_SIZE+1 + 2*MOP_SIZE );
      SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, leftOverlayBuf));
      SETPD( hwPtr, ghw0->leftOverlayBuf, (pSurfaceData->hwPtr)>>5 );
      SETMOP( hwPtr,SST_MOP_QUEUE_SWAP );
      SETMOP( hwPtr,SST_MOP_STALL_SWAP1 );
      HW_ACCESS_EXIT(ACCESS_3D);
      CMDFIFO_EPILOG(hwPtr);

      INCREMENT_SWAP_COUNT; // increments driver swap count to account for swap just requested

#ifdef AGP_CMDFIFO
      {
        void  FLUSHAGP(NT9XDEVICEDATA * ppdev);
        FLUSHAGP(ppdev);
      }
#endif // #ifdef AGP_CMDFIFO

      _FF(dd3DInOverlay)        = D3D_USING_OVERLAY; //Set 3D overlay in use flag
      _FF(ddVisibleOverlaySurf) = GETPRIMARY - _FF(LFBBASE);
      _FF(lastOverlayAddress)   = INVALID_ADDRESS;
  }

  return DD_OK;

} //promote_PrimaryToOverlay


/*----------------------------------------------------------------------
Function name: DdSetDrvColorKey

Description:   DDRAW callback SetColorKey
               Sets the color key value for the specified surface

Return:        DWORD DDRAW result
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetDrvColorKey( LPDDHAL_DRVSETCOLORKEYDATA psdck )
{
#ifndef MM
  DD_ENTRY_SETUP(psdck->lpDD);
#endif

  #ifdef FXTRACE
#ifndef MM
  Msg(ppdev, DEBUG_APIENTRY, "SetDrvColorKey32" );
#endif
  #endif

  psdck->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
} //DdSetDrvColorKey


/*----------------------------------------------------------------------
Function name: DdCreatePalette

Description:   DDRAW callback CreatPalette
               Creates a DirectDrawPalette object for this DirectDraw object

Return:        DWORD
               DHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdCreatePalette( LPDDHAL_CREATEPALETTEDATA pcp )
{
    DD_ENTRY_SETUP(pcp->lpDD);
    #ifdef FXTRACE
    Msg(ppdev, DEBUG_APIENTRY, "CreatePalette32" );
    DUMP_CREATEPALETTEDATA(ppdev, DEBUG_DDGORY, pcp );
    #endif

    pcp->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

} // DdCreatePalette



/*----------------------------------------------------------------------
Function name: demote_3DToNoneOverlay

Description:   

Return:        DWORD DDRAW result
               DD_OK
----------------------------------------------------------------------*/
DWORD DEMOTE_3DTONONEOVERLAY(NT9XDEVICEDATA * ppdev)
{

  DWORD dwCfg0;

  // if GDI promoted the surface to overlay (e.g., low res 16bpp mode),
  // then don't do anything
  //
  CMDFIFO_PROLOG(hwPtr);
  CMDFIFO_CHECKROOM(hwPtr, 2 * MOP_SIZE);
  SETMOP(hwPtr, SST_MOP_STALL_2D); 
  SETMOP(hwPtr, SST_MOP_STALL_3D |
               (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));
  CMDFIFO_EPILOG(hwPtr);

  // wait for non-busy, meaning fifo drained as well
  P6FENCE;
  FXBUSYWAIT(ppdev); 
 
  if (_FF(dd3DInOverlay) & GDI_USING_OVERLAY)
      return DD_OK;

  _FF(dd3DInOverlay) = FALSE;

  if( _DD(ddFSAAMode )   )
  {
      DWORD newStrideState = 0;
      // If surface variables for the primary surface were doubled for AA we must return them to normal size.
      // In cases where the desktop was the same size/bit depth as FSAA app we would not display
      // the desktop properly without adjusting the stride back to its original size.
      // This would also show up in apps like Monster Truck madness - if you paused the game and then
      // resumed it again with FSAA on the stride values would double every time.
      switch( _DD(ddFSAAMode) )
      {
        case  FSAA_MODE_4XFLIP:
            _FF(ddPrimarySurfaceData.dwPStride)   >>= 1;
            _FF(ddPrimarySurfaceData.dwStride)    >>= 1;
            _FF(ddPrimarySurfaceData.dwL2MStride)  -= 1;
            _FF(ddPrimarySurfaceData.dwMStride)   >>= 1;
            _FF(ddPrimarySurfaceData.dwWidth)     >>= 1;
            _FF(ddPrimarySurfaceData.dwHeight)    >>= 1;


            //we also need to reset the dibEngine cam entry here in case a mode set doesn't do it
            if( _DD(sst2CRegs)->cam[_FF(EndCamEntry)].strideState & SST_CAM_EN_AA )
            {
                
                // I'm not sure if this could ever be anything except linear
                // but check all modes just in case
                if (_FF(ddPrimarySurfaceData.tileFlag) & MEM_IN_TILE1)      
                    newStrideState |= SST_CAM_TILE_MODE1;
                else if (_FF(ddPrimarySurfaceData.tileFlag) & MEM_IN_TILE0) 
                    newStrideState &= ~SST_CAM_TILE_MODE1;
                else                                           
                    newStrideState |= SST_CAM_LINEAR;

                newStrideState |= _FF(ddPrimarySurfaceData.dwPStride)   << SST_CAM_PSTRIDE_SHIFT;     // PStride is adjusted for FSAA
                newStrideState |= _FF(ddPrimarySurfaceData.dwL2MStride) << SST_CAM_MSTRIDE_SHIFT;
                newStrideState |= _FF(ddPrimarySurfaceData.dwBytesPerPixel)  << SST_CAM_LFB_DEPTH_SHIFT;   // pixel depth
                newStrideState |= _FF(ddPrimarySurfaceData.dwBytesPerPixel)  << SST_CAM_PIXEL_DEPTH_SHIFT;
                if (_FF(ddPrimarySurfaceData.tileFlag) & MEM_STAGGER)                                     // check for stagger
                    newStrideState |= SST_CAM_EN_STAGGER;
#ifdef SLI
                // Enable SLI for the surface
                if (SLI_MODE_ENABLED == _DD(sliMode))
                {
                    if (_FF(ddPrimarySurfaceData.dwFlags) & FXSURFACE_IS_DISTRIBUTED)
                        newStrideState |= SST_CAM_EN_DISTRIBUTED;
                }
#endif
                _DD(sst2CRegs)->cam[_FF(EndCamEntry)].strideState = newStrideState;
                _DD(sst2CRegs)->cam[_FF(EndCamEntry)].endAddress  = _DD(sst2CRegs)->cam[_FF(EndCamEntry)].baseAddress + 
                                                                   (_FF(ddPrimarySurfaceData.dwMStride)*_FF(ddPrimarySurfaceData.dwHeight));

            }
            break;
        default:
            break;
      }
      
      _DD(ddFSAAMode) = FSAA_MODE_NONE;  // clear this global flag for fullscreen aa
  }

  dwCfg0 = GET(ghwVD->vdVoPvCfg0);
  VD_SET_FIELD( dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_CPU_SWAP); //Use address in base Left
  SETDW(ghwVD->vdVoPvCfg0, dwCfg0);

  //Restore mixer config to its state before primary promotion
  SETDW( ghwVD->vdVoPmCfg, _FF(ddPmCfgSave) ); 

  //Update primary surface registers for mixer update
  SETDW(ghwVD->vdVoPsStatus0, GET(ghwVD->vdVoPsStatus0)|SST_VO_PV_UPDATE|SST_VO_PS_UPDATE);
    
  if(_FF(ddVisibleOverlaySurf) == (GETPRIMARY - _DS(LFBBASE)))
  {
    _FF(ddVisibleOverlaySurf) = 0;
  }

#ifdef SLI
  Disable_SLI(ppdev); 
#endif

  return DD_OK;

} //demote_3DToNoneOverlay


 /*----------------------------------------------------------------------
Function name: DdSetExclusiveMode

Description:   DDRAW callback SetExclusiveMode
               Sets a DirectDraw object to exclusive mode

Return:        DWORD DDRAW retsult
               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetExclusiveMode( LPDDHAL_SETEXCLUSIVEMODEDATA psemd )
{
    DD_ENTRY_SETUP(psemd->lpDD);

    if (psemd->dwEnterExcl )
    {
      Msg(ppdev, DEBUG_APIENTRY, "SetExclusiveMode32 (entering)" );
      _DS(ddExclusiveMode) = (DWORD)psemd->lpDD;
    }
    else
    {
      Msg(ppdev, DEBUG_APIENTRY, "SetExclusiveMode32 (leaving)" );
      _DS(ddExclusiveMode) = 0UL;

      RESTOREGAMMA(ppdev);

#if 1
      // Bug: Single Buffer Fullscreen promotes to overlay but runtime
      // doesn't call Destroy surface so dd3DInOverlay=1 and subsequant
      // promtion never happens.
      // Possible Fix: In SetExclusive Mode call demote

      if (_FF(dd3DInOverlay) & D3D_USING_OVERLAY )
      {
        DEMOTE_3DTONONEOVERLAY(ppdev);
        _FF(ddVisibleOverlaySurf) = 0;
      }
#endif

      if (ppdev->gdiFlags & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      {
	    HDC       hdc;
	    hwcExtRequest_t req;

	    req.which = HWCEXT_RESTORE_DESKTOP;

	    hdc = GetDC(0);
    
	    ExtEscape(hdc,           //  handle to device context
		      EXT_HWC,	     //  escape function
		      sizeof(req),   //  bytes in input structure
		      (const char *)&req,	  //  ptr to input structure
		      0,             //  bytes in output structure
		      NULL);         //  pointer to output structrue
    
	    ReleaseDC(NULL,           // handle of window
		      hdc);           // handle of device context

	    ppdev->gdiFlags &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);

	    // if the mode during exclusive was the same as the
	    // current mode (hopefully the desktop!), then go ahead
	    // and kill the glide process, to avoid garbage on the
	    // desktop
	    //
            if (_FF(ModeNumber) == _FF(ioBase))
                TerminateProcess((HANDLE)_FF(regBase), (UINT)0);
      }

    }

    psemd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}// DdSetExclusiveMode


/*----------------------------------------------------------------------
Function name: DdSetPalette

Description:   DDRAW callback SetPalette
               Sets a palette to a specific surface

Return:        DWORD DDRAW retsult
               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetPalette( LPDDHAL_SETPALETTEDATA psp )
{
  // This SetPalette may be the only indication we get
  // that a different palette has been attached to a surface.
  // So, force a palette download when/if a palettized
  // texture handle is encountered in the triangle code.

  DD_ENTRY_SETUP(psp->lpDD);
  txtrNewPalette(ppdev, (PALHNDL *) psp->lpDDPalette);

  psp->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}// DdSetPalette


/*----------------------------------------------------------------------
Function name: DdDestroyPalette

Description:   DDRAW callback DestroyPalette
               Destroys a specified palette

Return:        DWORD DDRAW result
               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdDestroyPalette( LPDDHAL_DESTROYPALETTEDATA pdp )
{
  // if this palette is loaded then lets just make sure that the
  // new palette gets downloaded properly.
  DD_ENTRY_SETUP(pdp->lpDD);

  txtrNewPalette(ppdev, (PALHNDL *) pdp->lpDDPalette);

  pdp->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}// DdDestroyPalette


/*----------------------------------------------------------------------
Function name: DdSetEntries

Description:   DDRAW callback SetEntries
               Changes the palette entries in a specified palette immediately

Return:        DWORD DDRAW result
               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetEntries( LPDDHAL_SETENTRIESDATA pse )
{
  // palette will need to be downloaded again
  DD_ENTRY_SETUP(pse->lpDD);

  txtrNewPalette(ppdev, (PALHNDL *) pse->lpDDPalette);

  pse->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}// DdSetEntries


/*----------------------------------------------------------------------
Function name: LoadGamma

Description:   Set the hardware palette to using Gamma palette values
               Works only for 16bpp and above

Return:        int
               0 - 
----------------------------------------------------------------------*/
double ourpow(double a, double x);
double logex(double x);
int LOADGAMMA(NT9XDEVICEDATA *ppdev)
{

   DWORD Gamma[256];
   double rgamma,ggamma,bgamma;
   double rg1, gg1, bg1;
   double fval;
   LPSTR lpStr;
   int ir, ig, ib;
   int i;

   // if 8 bpp don't get involved
   if (SST_VD_8BPP == _FF(ddPrimarySurfaceData.dwPixelFormat))
      return 0;

   lpStr = GETENV(RED_NAME);
   if (NULL != lpStr)
      rgamma = ddatof(lpStr);
   else
      rgamma = 1.0;

   if (rgamma < 0.43)
      rgamma = 0.43;

   lpStr = GETENV(GREEN_NAME);
   if (NULL != lpStr)
      ggamma = ddatof(lpStr);
   else
      ggamma = 1.0;

   if (ggamma < 0.43)
      ggamma = 0.43;

   lpStr = GETENV(BLUE_NAME);
   if (NULL != lpStr)
      bgamma = ddatof(lpStr);
   else
      bgamma = 1.0;

   if (bgamma < 0.43)
      bgamma = 0.43;

   rg1 = 1.0/rgamma;
   gg1 = 1.0/ggamma;
   bg1 = 1.0/bgamma;
   for (i=0; i<256; i++)
      {
      fval = (double)i/(double)255.0;
      ir = (int)ddftol((float)(ourpow(fval, rg1) * 255.0 + .5));
      ig = (int)ddftol((float)(ourpow(fval, gg1) * 255.0 + .5));
      ib = (int)ddftol((float)(ourpow(fval, bg1) * 255.0 + .5));
      Gamma[i] = ((ir << 8) | ig) << 8 | ib;
      }               

   HWSETPALETTE(ppdev, Gamma, 0, 256);

   return 0;

}// LoadGamma


/*----------------------------------------------------------------------
Function name: RestoreGamma

Description:   
               Works only for 16bpp and above

Return:        int
               0 - 
----------------------------------------------------------------------*/
int RESTOREGAMMA(NT9XDEVICEDATA * ppdev)
{
   //DWORD Gamma[256];
   //int i;

   // if 8 bpp don't get involved
   if (SST_VD_8BPP == _FF(ddPrimarySurfaceData.dwPixelFormat))
      return 0;

   HWSETPALETTE(ppdev, _FF(GammaTable), 0, 256);

#if 0
   for (i=0; i<256; i++)
   {
      /* Note that gamma_ramp table has data in most significant byte,
       * not least!
       */
      Gamma[i] = (_FF(gamma_ramp).blue[i] & 0xff00) << 8;
      Gamma[i] |= (_FF(gamma_ramp).green[i] & 0xff00);
      Gamma[i] |= (_FF(gamma_ramp).red[i] & 0xff00) >> 8;
   }

   HWSETPALETTE(ppdev, Gamma,0, 256);
#endif

   return 0;
}// RestoreGamma


/*----------------------------------------------------------------------
Function name: HWSetPalette

Description:   

Return:        BOOL
               TRUE
----------------------------------------------------------------------*/
#define FILENAME "\\\\.\\H6VDD"
HWSETPALETTE(NT9XDEVICEDATA * ppdev, DWORD FAR *pGamma, int nStart, int nSize)
{
   HANDLE hDevice;
   DIOC_DATA DIOC_Data;
   
   hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
   if (INVALID_HANDLE_VALUE != hDevice)
   {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DIOC_Data.dwSpare = (((FxU32)nStart & 0x0000FFFF) << 16) | ((FxU32)nSize & 0x0000FFFF);
      DIOC_Data.dwAnotherSpare = (DWORD)pGamma;
      DeviceIoControl(hDevice, CLUT_SETDESKTOP, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
   }
   CloseHandle(hDevice);
}// HWSetPalette


#define RANGE(val, low, hi) ((low <= val) && (val <= hi))
#define EPILSON (0.00001)


/*----------------------------------------------------------------------
Function name: ourpow

Description:   This functions computes a^x by using the series
               exponential a^x = 1 + x * ln a + (x * ln a)^2/2!
               + (x * ln a) ^ 3/3! + ...

Return:        double dRet - value of a to the x power
----------------------------------------------------------------------*/
double ourpow(double a, double x)
{
   double dRet;
   double dOldRet;
   double xlogea;
   double xFac;
   double xfac;
   int n;

   if (RANGE(a, 0.0 - EPILSON, EPILSON))
      return (double)0.0;
   xlogea = x * logex(a);   
   n = 1;
   xFac = 1.0;
   xfac = 1.0;
   dRet = 1.0;
   dOldRet = 2.0;
   while (fabs(dRet - dOldRet) > EPILSON)
      {
      dOldRet = dRet;
      xFac = xFac * n; 
      xfac = xfac * xlogea;                  
      n += 1;
      dRet = dRet + xfac/xFac;      
      }
   
   return dRet;
}// ourpow

/*----------------------------------------------------------------------
Function name: logex

Description:   This functions computes ln x.  We use the formula
			   ln (1+x)/(1-x) = 2(x + x^3/3 + x^5/5 + x^7/7 + ...)
			    
			   where we set y=(1+x)/(1-x) and solve for x giving
               x=(y-1)/y+1)

Return:        double dRet - the natural log of x
----------------------------------------------------------------------*/
double logex(double x)
{
   double xfac;
   double xPart;
   double xsqrd;
   double xnum;
   double dRet;
   double dOldRet;

   xfac = (x - 1.0)/(x + 1.0);
   xsqrd = xfac * xfac;   

   dRet = xfac;
   dOldRet = 2.0;
   xPart = xfac;
   xnum = 3.0;
   while (fabs(dRet - dOldRet) > EPILSON)
      {
      dOldRet = dRet;
      xPart = xPart * xsqrd;
      dRet = dRet + xPart/xnum;
      xnum += 2.0; 
      }

   dRet = dRet * 2.0;

   return dRet;
}// logex

