/* $Header: ddovl32.c, 57, 12/7/00 9:43:38 AM PST, Ryan Bissell$ */
/*
** Copyright (c) 1996-2000, 3dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   ddovl32.c
**
** Description: Contains DirectDraw overlay support.
**
** $Log: 
**  57   3dfx      1.56        12/7/00  Ryan Bissell    Miscellaneous cleanup, 
**       and CS work
**  56   3dfx      1.55        11/12/00 Ryan Bissell    When allocating N 
**       buffers, and the k-th buffer allocation fails, deallocate buffers 1 
**       through k-1 before returning with error.
**  55   3dfx      1.54        11/6/00  Xing Cong       Set DDBD_X for 
**       overlayAlpha
**  54   3dfx      1.53        11/6/00  Xing Cong       Delete supports for 
**       InterVideo decoder.  Fix caps for overlay alpha
**  53   3dfx      1.52        9/22/00  Xing Cong       Add support for DXVA 
**       tiel1 field display.
**  52   3dfx      1.51        9/6/00   Xing Cong       Add DVD sub-picture 
**       support.  Allocate overlay surface from tile1 if DxVA is active
**  51   3dfx      1.50        8/24/00  Xing Cong       Clean up variables 
**       _DD(dstCKeySurf) and _DD(srcCKeySurf)
**  50   3dfx      1.49        8/15/00  Xing Cong       DxVA support using VTA
**  49   3dfx      1.48        6/8/00   Xing Cong       Tuning vertical scale 
**       factor of VI to prevent last line generated from outside of source.
**  48   3dfx      1.47        5/25/00  Xing Cong       Fix overlay right edge 
**       problem when zooming up.
**  47   3dfx      1.46        4/20/00  Dale  Kenaston  Fixed overlay support by
**       changing get interface api parameter.
**  46   3dfx      1.45        3/15/00  Dale  Kenaston  Added support for hydra 
**       and rage devices to CSIM_Update
**  45   3dfx      1.44        2/4/00   Xing Cong       Overaly uses 512 CLUT
**  44   3dfx      1.43        1/31/00  Xing Cong       Create DrawRect();delete
**       DrawTrangle().
**  43   3dfx      1.42        1/14/00  Xing Cong       Fix compler error when 
**       CF=0
**  42   3dfx      1.41        12/29/99 Xing Cong       Unpdate MoComp with new 
**       Interfaces
**  41   3dfx      1.40        12/21/99 Xing Cong       Optimize command fifo 
**       setting for 2D overlay and MoComp. when 3D HW is used.
**  40   3dfx      1.39        12/17/99 Brent Burton    Removed all WINNT macro 
**       checks.  This removed some unused code.
**  39   3dfx      1.38        12/10/99 Xing Cong       set register 
**       SST_VI_H_INC_START with even number
**  38   3dfx      1.37        11/27/99 Xing Cong       Fix retail version 
**       status register byte reading error.  All the register should read as 
**       DWORD
**  37   3dfx      1.36        11/23/99 Xing Cong       Call SurfaceConversion()
**       to convert tile MCP0 surface into linear surface
**  36   3dfx      1.35        11/22/99 Xing Cong       Support both destination
**       color key and soruce color key eanbled case.
**  35   3dfx      1.34        11/18/99 Xing Cong       MoComp use YUYV422 
**       surface for I and P frame too.  Add P frame support,also clear 
**       macroblock surfcae in ddLock()
**  34   3dfx      1.33        11/16/99 Xing Cong       Move all Mocomp 
**       functions that touch HW into DDRAW callback functions.
**  33   3dfx      1.32        11/16/99 Xing Cong       Add  MoComp support
**  32   3dfx      1.31        11/15/99 Xing Cong       Fix horizontal cropping 
**       problem with VI Shirnk. also setup vertical filters
**  31   3dfx      1.30        11/12/99 Randy Spurlock  Updated for new 
**       Simulator API names
**  30   3dfx      1.29        11/12/99 Xing Cong       Ajust VI horizontal 
**       start address for VI shrink
**  29   3dfx      1.28        11/12/99 Xing Cong       Caution with register 
**       update especially mixer change
**  28   3dfx      1.27        11/5/99  Andrew Sobczyk  Changed 
**       phantom_free/alloc parameters
**  27   3dfx      1.26        11/5/99  Andrew Sobczyk  Added dwFlags surface 
**       initialization
**  26   3dfx      1.25        10/31/99 Michel Conrad   Update to use changed 
**       surface free parameter list.
**  25   3dfx      1.24        10/25/99 Xing Cong       Use ConfigBuffer() 
**       fuinction for YV12 conversion.  Enable UV bilinear filter.
**  24   3dfx      1.23        10/22/99 Xing Cong       Put myLock and myUnlock 
**       inside  #if DEBUG
**  23   3dfx      1.22        10/21/99 Xing Cong       Create a large buffer 
**       that hold 2 shrink surfaces and flter buffers for VI shrink.
**  22   3dfx      1.21        10/21/99 Xing Cong       YV12 support: convert 
**       YV12 into YUYV format
**  21   3dfx      1.20        10/19/99 Xing Cong       YV12 support
**  20   3dfx      1.19        10/8/99  Xing Cong       Add yv12 surface 
**       pointers
**  19   3dfx      1.18        10/8/99  Xing Cong       Algin YVU surface at 
**       DWORD.  limit VI progressive shrink to 1/8
**  18   3dfx      1.17        10/7/99  Xing Cong       Simplified SetVIRegs()
**  17   3dfx      1.16        10/6/99  Xing Cong       QT test modifercations
**  16   3dfx      1.15        10/5/99  Xing Cong       Add function SetVIReg
**  15   3dfx      1.14        10/5/99  Xing Cong       Filter buffer allocation
**  14   3dfx      1.13        10/4/99  Xing Cong       Fix a color key problem.
**        When color key is enabled and disabled, then overlay sometimes doesn't
**       show up.
**  13   3dfx      1.12        10/1/99  Xing Cong       Use VIP_REG
**  12   3dfx      1.11        9/29/99  Philip Zheng    fixed compiler error
**  11   3dfx      1.10        9/28/99  Xing Cong       Set VI start0 and start1
**       to 0
**  10   3dfx      1.9         9/17/99  Xing Cong       Disable VI shrink so 
**       that QA can test video
**  9    3dfx      1.8         9/17/99  Xing Cong       Add debug display and 
**       help functions
**  8    3dfx      1.7         9/16/99  Xing Cong       Allocate one field for 
**       each LptBase
**  7    3dfx      1.6         9/16/99  Xing Cong       Don't set update address
**       bit if VI is used for overlay display
**  6    3dfx      1.5         9/15/99  Goran Devic     Added R3 device support 
**       
**  5    3dfx      1.4         9/14/99  Xing Cong       Update BaseLeft for CSIM
**       too
**  4    3dfx      1.3         9/14/99  Xing Cong       Strigger VI after 
**       overlay registers are updated.  Enable the trigger bit testing
**  3    3dfx      1.2         9/13/99  Xing Cong       
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
**
*/


#include "precomp.h"
#include "ddovl32.h"
#include "fifomgr.h"

#include "ddvpe32.h"

#include "sst2glob.h"
#include "sst2bwe.h"

#define T_OFFSET  0.0f


#define UPDATEREGISTER(reg)  SETDW(ghwVD->vdVoPsStatus0,(reg)| GET(ghwVD->vdVoPsStatus0))

#define WAITFORUPDATE(reg)  { volatile DWORD ddd; \
            do { ddd =GET(ghwVD->vdVoPsStatus0); \
                     } while( ddd & (reg)); } 

#define TRIGGER_VI       SETDW( ghwVD->vdViStatus,  SST_VI_INT_TRIGGER )
#define WAITFORTRIGGER   while(GET(ghwVD->vdViStatus) & SST_VI_INT_TRIGGER)
BOOL DisableAlpha(NT9XDEVICEDATA * ppdev, DWORD);
        

#include "iSST2.h"  // Rampage csim server
#include "iR3.h"    // R3 csim server
#include "iHydra.h" // Hydra csim server
#include "iRage.h"  // Rage csim server

#define USE_CSIM

#ifdef USE_CSIM
void CSIM_Update(GLOBALDATA * ppdev)
{
    DWORD dwCount;
    DWORD dwDevNode;

    {
        SST2INTERFACE iSST2;
        SST2DISPLAYMASK mask;

        if(SST2_Get_Interface(&iSST2, (DWORD)SST2_VXD_NAME) != SST2RET_OK)
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
     
        mask.Display_0=1;
        mask.Display_1=0;

        //Update SST2 device simulator video section
        if ( SST2_Device_Video_Update(iSST2, dwDevNode, mask) != SST2RET_OK )
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
        R3DISPLAYMASK mask;
     
        if(R3_Get_Interface(&iR3, (DWORD)R3_VXD_NAME) != R3RET_OK)
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
         
        mask.Display_0=1;
        mask.Display_1=0;

        //Update R3 device simulator video section
        if ( R3_Device_Video_Update(iR3, dwDevNode, mask) != R3RET_OK )
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
        HYDRADISPLAYMASK mask;
     
        if(Hydra_Get_Interface(&iHydra, (DWORD)HYDRA_VXD_NAME) != HYDRARET_OK)
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
         
        mask.Display_0=1;
        mask.Display_1=0;

        //Update Hydra device simulator video section
        if ( Hydra_Device_Video_Update(iHydra, dwDevNode, mask) != HYDRARET_OK )
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
        RAGEDISPLAYMASK mask;
     
        if(Rage_Get_Interface(&iRage, (DWORD)RAGE_VXD_NAME) != RAGERET_OK)
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
         
        mask.Display_0=1;
        mask.Display_1=0;

        //Update Rage device simulator video section
        if ( Rage_Device_Video_Update(iRage, dwDevNode, mask) != RAGERET_OK )
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
#endif
INLINE void CheckFifo(NT9XDEVICEDATA * ppdev)
#ifdef DEBUG
{                                           
 DWORD dwStatus = GET( ghwVD->vdVoPsStatus1);

 Msg(ppdev, 0, "PsStatus1= %lx", dwStatus);
 if( dwStatus & SST_VO_PO_UNDERFLOW)
   Msg(ppdev, 0, "PS FIFO underflow !!!!!!");


}
#else
{}
#endif

#include "ddcam.h"      // cam management routines

FxU32 myLock(LPDDRAWI_DIRECTDRAW_GBL lpDD,FXSURFACEDATA *surfaceData)
{

  // Program CAM for surface
      Sst2CAMEntry CAMEntry;
      FxU32 phantom_blksize,byte_width, ptr;
      DD_ENTRY_SETUP(lpDD)

      CAMEntry.strideState  =0;


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
   // tile/linear modes
      if (surfaceData->tileFlag & MEM_IN_TILE1)      
          CAMEntry.strideState |= SST_CAM_TILE_MODE1;
      else if (surfaceData->tileFlag & MEM_IN_TILE0) 
          CAMEntry.strideState &= ~SST_CAM_TILE_MODE1;
      else                                           
          CAMEntry.strideState |= SST_CAM_LINEAR;

      // stride in physical memory
      CAMEntry.strideState |= surfaceData->dwPStride   <<SST_CAM_PSTRIDE_SHIFT;

      CAMEntry.strideState |= surfaceData->dwL2MStride << SST_CAM_MSTRIDE_SHIFT;

      // pixel depth
      CAMEntry.strideState |= surfaceData->dwBytesPerPixel << SST_CAM_LFB_DEPTH_SHIFT;
      CAMEntry.strideState |= surfaceData->dwBytesPerPixel << SST_CAM_PIXEL_DEPTH_SHIFT;

      // Stagger
      if (surfaceData->tileFlag & MEM_STAGGER)      
          CAMEntry.strideState |= SST_CAM_EN_STAGGER;

      // Depth buffer?

      // AA ??

      // YUV ??

      // alloc LFB space given stride==power of 2 restriction

#ifdef SLI
      // Enable SLI for the surface
      if (SLI_MODE_ENABLED == _DD(sliMode))
         {
         if (surfaceData->inFlipChain)
            CAMEntry.strideState |= SST_CAM_EN_DISTRIBUTED;
         }
#endif

      // if pitch is power of 2, alloc with current size
      // if pitch not a power of 2, alloc additional to allow for mstride rounded up 

      phantom_blksize = surfaceData->endlfbPtr-surfaceData->lfbPtr; // real size is end-start

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
          FxU32 height = phantom_blksize/byte_width;// num rows
          phantom_blksize += height*(surfaceData->dwMStride - byte_width);// add height*extra row length
      }

      // memmgr aligns start on 32B so align size so end also 32B aligned
      phantom_blksize = (phantom_blksize+0x1f) & ~(0x1f);

      phantom_allocSurface(GET_DEVICE_DATA(lpDD),phantom_blksize,(DWORD *)&CAMEntry.baseAddress);

      if (!CAMEntry.baseAddress)
      {
          // can't find suitable phantom address
          return 0;
      }

      ptr = CAMEntry.baseAddress; // pointer returned to Host caller

      surfaceData->phantomlfbPtr = ptr;  // store for eventual unlock

      CAMEntry.baseAddress -= _FF(LFBBASE); // CAM needs offset from top of membase1
      CAMEntry.endAddress   =CAMEntry.baseAddress+phantom_blksize;
      CAMEntry.physicalBase =surfaceData->hwPtr;

      if (!FxCamProgram(_DD(camMgr),ppdev,&CAMEntry,CAM_ENTRY_ILLEGAL))
      {
          // CAM programming failed - is there a better error condition?
         return 0;
      }

      return ptr;

}

BOOL myUnlock(LPDDRAWI_DIRECTDRAW_GBL lpDD,FXSURFACEDATA *surfaceData)
{
   DD_ENTRY_SETUP(lpDD)
    phantom_freeSurface(GET_DEVICE_DATA(lpDD), surfaceData->phantomlfbPtr); // phantomMM uses host (lfb) addr

    if (!FxCamFreeEntry(_DD(camMgr), ppdev, surfaceData->phantomlfbPtr-_FF(LFBBASE)))
    {
       // could not unlock surface 
       return FALSE;
    }
    return TRUE;
}

/*
----------------------------------------------------------------------------
  FindDesktopLayer()
  Find which layer desktop is at in the mixer
----------------------------------------------------------------------------
*/
INLINE DWORD FindDesktopLayer(DWORD dwPmCfg)
{
      if( SST_GET_FIELD( dwPmCfg, SST_VO_PM_LYR1_SEL) == SST_VD_LAYER_PD)
        return 1;
      //not in layer1, see it is in layer2
      else if(SST_GET_FIELD( dwPmCfg, SST_VO_PM_LYR2_SEL) == SST_VD_LAYER_PD)
        return 2;
      else
        return 0;       //impossible
}

/*
----------------------------------------------------------------------------
  FindOverlayLayer()
  Find which layer this overlay is at in the mixer
----------------------------------------------------------------------------
*/
INLINE DWORD FindOverlayLayer(DWORD dwPmCfg)
{
   if( SST_GET_FIELD (dwPmCfg,SST_VO_PM_LYR1_SEL) == SST_VD_LAYER_PV)
      return 1;
   //not in layer1, see it is in layer2
   else if(SST_GET_FIELD( dwPmCfg, SST_VO_PM_LYR2_SEL) == SST_VD_LAYER_PV)
    return 2;
  else
    return 0;       //impossible
}
/*
----------------------------------------------------------------------------
  MoveDesktop()
  Move desktop to layer1 if fLayer1=TRUE or layer2 of the mixer
  dwAlphaMask: select which layer to enable alpha mask,can be 0 , 1, 2, 3
               0 means disable alpha mask for all layers 
----------------------------------------------------------------------------
*/
void MoveDesktop(NT9XDEVICEDATA * ppdev, BOOL fLayer1, DWORD dwAlphaMask)
{

 DWORD dwLayer;
 DWORD dwXmCfg;
 DWORD dwOldLayer;

    //Where is the desktop right now
    dwXmCfg = GET(ghwVD->vdVoPmCfg)& ~(0x300);   //disable alpha mask
    dwLayer = FindDesktopLayer(dwXmCfg);

    switch( dwLayer)
    {
       case 1:
         if( fLayer1)
            break;
          //Move desktop from layer1 to layer2
         dwOldLayer = SST_GET_FIELD (dwXmCfg,SST_VO_PM_LYR2_SEL);
         dwXmCfg &= ~ (SST_VO_PM_LYR1_SEL | SST_VO_PM_LYR2_SEL);
         dwXmCfg |=  (( SST_VD_LAYER_PD) << SST_VO_PM_LYR2_SEL_SHIFT)
                    | (dwOldLayer << SST_VO_PM_LYR1_SEL_SHIFT);
         break;
       case 2:
         if(!fLayer1)
            break;
          //Move desktop from layer2 to layer1
         dwOldLayer = SST_GET_FIELD (dwXmCfg,SST_VO_PM_LYR1_SEL);
         dwXmCfg &= ~ (SST_VO_PM_LYR1_SEL | SST_VO_PM_LYR2_SEL);
         dwXmCfg |= (( SST_VD_LAYER_PD) << SST_VO_PM_LYR1_SEL_SHIFT)
                    | (dwOldLayer << SST_VO_PM_LYR2_SEL_SHIFT);
         break;

      default:
         return;
    }
    dwXmCfg |= (dwAlphaMask << 8);

    SETDW( ghwVD->vdVoPmCfg, dwXmCfg);
}
/*
----------------------------------------------------------------------------
  DisableOverlay()
  Disable Overlay Window dwOVLIndex
  Return FALSE if desktop registers are not changed
----------------------------------------------------------------------------
*/

void DisableOverlay(NT9XDEVICEDATA * ppdev )
{
  DWORD dwXmCfg;
  //only disable it when it is enabled
  if( !_DD(dwOVLFlags)& OVL_ON)
    return;

  _DD(dwOVLFlags) &= ~OVL_ON;
   dwXmCfg = GET( ghwVD->vdVoPmCfg);
   switch( FindOverlayLayer(dwXmCfg) )
   {

     case 1:
        VD_SET_FIELD( dwXmCfg, SST_VO_PM_LYR1_SEL, SST_VD_LAYER_XPARENT);
        break;
     case 2:
        VD_SET_FIELD( dwXmCfg, SST_VO_PM_LYR2_SEL, SST_VD_LAYER_XPARENT);
        break;
   }

   WAITFORUPDATE( SST_VO_PD_UPDATE | SST_VO_PV_UPDATE| SST_VO_PS_UPDATE);

   SETDW( ghwVD->vdVoPmCfg, dwXmCfg);
   //disable color or alpha too
   DisableAlpha(ppdev, DEST_CKEY | SRC_CKEY);
}


/*
----------------------------------------------------------------------------
  EnableOverlay()
  Enable Overlay Window dwOVLIndex
  Add the overlay into the mixer
----------------------------------------------------------------------------
*/

VOID EnableOverlay(NT9XDEVICEDATA * ppdev)
{
 DWORD dwXmCfg;

  //only enable it when it is disabled
  if( _DD(dwOVLFlags & OVL_ON))
    return;

  _DD(dwOVLFlags) |= OVL_ON;

  //The way to enable overlay is to add this overlay
  //to the mixer.   
  //first find where the desktop is

  dwXmCfg = GET( ghwVD->vdVoPmCfg);
  
  if( FindDesktopLayer(dwXmCfg) == 1)
  {
    //layer2 is empty

    VD_SET_FIELD( dwXmCfg,SST_VO_PM_LYR2_SEL, SST_VD_LAYER_PV); 
     
  }
  else
  {
   //layer1 is empty

    VD_SET_FIELD( dwXmCfg,SST_VO_PM_LYR1_SEL, SST_VD_LAYER_PV); 
  }

  WAITFORUPDATE(SST_VO_PS_UPDATE);

  SETDW( ghwVD->vdVoPmCfg, dwXmCfg);
}


/*
----------------------------------------------------------------------------
  SetUpColorKey()
----------------------------------------------------------------------------
*/
void SetUpColorKey(NT9XDEVICEDATA * ppdev,DWORD dwDstKeyLow, DWORD dwDstKeyHigh,
					DWORD dwSrcKeyLow, DWORD dwSrcKeyHigh, DWORD dwFormat)
{

   DWORD dwAlpha0;
   DWORD dwAlpha1;
   BYTE bR, bG,bB;
   BYTE bY, bU,bV;
   
   dwAlpha0 =(5 << SST_VO_PV_ALPHA_BITS_SHIFT)
                | SST_VO_PV_ALPHA_INV;  //always invert alpha
   VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_CREATE,SST_VD_ALPHA_EXTRACT);

   dwAlpha1 = (DWORD)(0x3F << SST_VO_PV_ALPHA_VALUE_SHIFT);

   if(_DD(dwOVLFlags) & SRC_CKEY)
   {

     //For source color key, we set up overlay alpha
     //and put overlay on top of desktop
     //so if the color key matches, desktop will show up

     switch( dwFormat)
     {
        case SST_VD_YUV_UYVY:
        case SST_VD_YUV_YUYV:
        case SST_VD_YUVA:
            bY = (BYTE)(dwSrcKeyLow >> 16);
            bU = (BYTE)(dwSrcKeyLow >> 8);
            bV = (BYTE) dwSrcKeyLow;

            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,bU);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,bY);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,bV);

            bY = (BYTE)(dwSrcKeyHigh >> 16);
            bU = (BYTE)(dwSrcKeyHigh >> 8);
            bV =  (BYTE)dwSrcKeyHigh ;

            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,bU);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,bY);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,bV);

            break;
        case SST_VD_8BPP:
            //Set up color key in overlay windows
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,dwSrcKeyLow & 0xFF);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,dwSrcKeyLow & 0xFF);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,dwSrcKeyLow & 0xFF);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,dwSrcKeyHigh & 0xFF);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,dwSrcKeyHigh & 0xFF);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,dwSrcKeyHigh & 0xFF);
            break;
       case SST_VD_RGB565:        //RGB 5:6:5

            bR = (BYTE)(dwSrcKeyLow >> 8) & 0xF8;
            bR |= (bR >> 5);
            bG = (BYTE)(dwSrcKeyLow >> 3) & 0xFC;
            bG |= (bG >> 6);
            bB = (BYTE)(dwSrcKeyLow << 3) & 0xF8;
            bB |= (bB >> 5);

            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,bB);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,bG);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,bR);

            bR = (BYTE)(dwSrcKeyHigh >> 8) & 0xF8;
            bR |= (bR >> 5);
            bG = (BYTE)(dwSrcKeyHigh >> 3) & 0xFC;
            bG |= (bG >> 6);
            bB = (BYTE)(dwSrcKeyHigh << 3) & 0xF8;
            bB |= (bB >> 5);

            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,bB);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,bG);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,bR);
            break;

       case SST_VD_RGB1555:        //RGB 5:5:5

            bR = (BYTE)(dwSrcKeyLow >> 7) & 0xF8;
            bR |= (bR >> 5);
            bG = (BYTE)(dwSrcKeyLow >> 2) & 0xF8;
            bG |= (bG >> 5);
            bB = (BYTE)(dwSrcKeyLow << 3) & 0xF8;
            bB |= (bB >> 5);

            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,bB);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,bG);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,bR);

            bR = (BYTE)(dwSrcKeyHigh >> 7) & 0xF8;
            bR |= (bR >> 5);
            bG = (BYTE)(dwSrcKeyHigh >> 2) & 0xF8;
            bG |= (bG >> 5);
            bB = (BYTE)(dwSrcKeyHigh << 3) & 0xF8;
            bB |= (bB >> 5);

            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,bB);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,bG);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,bR);
            break;
 
      case  SST_VD_RGB32:
            //RGB 8:8:8:8

            bR = (BYTE)(dwSrcKeyLow >> 16);
            bG = (BYTE)(dwSrcKeyLow >> 8);
            bB = (BYTE)(dwSrcKeyLow );

            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,bB);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,bG);
            VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,bR);

            bR = (BYTE)(dwSrcKeyHigh >> 16);
            bG = (BYTE)(dwSrcKeyHigh >> 8);
            bB = (BYTE)(dwSrcKeyHigh );

            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,bB);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,bG);
            VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,bR);

            break;
     }

     WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PS_UPDATE);

     SETDW( ghwVD->vdVoPvAlpha0, dwAlpha0);
     SETDW( ghwVD->vdVoPvAlpha1, dwAlpha1);

     //move the desktop to layer2 if destination color key is not on
     if(!(_DD(dwOVLFlags) & DEST_CKEY))
    	 MoveDesktop(ppdev, FALSE, 0);

   }
   
   if(_DD(dwOVLFlags) & DEST_CKEY)
   {

     //For destination color key, we set up desktop alpha
     //and put desktop on top of overlay
     //so if the color key mactch, overlay will show up

    DWORD dwByteCount = GETPRIMARYBYTEDEPTH;

     switch( dwByteCount)
     {

       case 1:
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,dwDstKeyLow & 0xFF);
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,dwDstKeyLow & 0xFF);
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,dwDstKeyLow & 0xFF);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,dwDstKeyHigh & 0xFF);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,dwDstKeyHigh & 0xFF);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,dwDstKeyHigh & 0xFF);
        break;
       case 2:  //rgb 565
        bR = (BYTE)(dwDstKeyLow >> 8) & 0xF8;
        bR |= (bR >> 5);
        bG = (BYTE)(dwDstKeyLow >> 3) & 0xFC;
        bG |= (bG >> 6);
        bB = (BYTE)(dwDstKeyLow << 3) & 0xF8;
        bB |= (bB >> 5);

        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,bB);
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,bG);
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,bR);

        bR = (BYTE)(dwDstKeyHigh >> 8) & 0xF8;
        bR |= (bR >> 5);
        bG = (BYTE)(dwDstKeyHigh >> 3) & 0xFC;
        bG |= (bG >> 6);
        bB = (BYTE)(dwDstKeyHigh << 3) & 0xF8;
        bB |= (bB >> 5);

        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,bB);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,bG);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,bR);

        break;
       case 4:

        bR = (BYTE)(dwDstKeyLow >> 16);
        bG = (BYTE)(dwDstKeyLow >> 8);
        bB = (BYTE)(dwDstKeyLow );

        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_BU,bB);
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_GY,bG);
        VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_LO_RV,bR);

        bR = (BYTE)(dwDstKeyHigh >> 16);
        bG = (BYTE)(dwDstKeyHigh >> 8);
        bB = (BYTE)(dwDstKeyHigh );

        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_BU,bB);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_GY,bG);
        VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_HI_RV,bR);

     }

     WAITFORUPDATE(SST_VO_PD_UPDATE | SST_VO_PS_UPDATE);

     SETDW( ghwVD->vdVoPdAlpha0, dwAlpha0);
     SETDW( ghwVD->vdVoPdAlpha1, dwAlpha1);

    //Move the desktop to layer1
     MoveDesktop(ppdev, TRUE,1);
   }
}

/*
----------------------------------------------------------------------------
  DisableAlpha()
  Desable color key, or alpha
  return TREU if desktop registers are changed
----------------------------------------------------------------------------
*/
BOOL DisableAlpha(NT9XDEVICEDATA * ppdev, DWORD dwCKEY )
{
 DWORD dwAlpha0, dwAlpha1;
   dwAlpha0 =(5 << SST_VO_PV_ALPHA_BITS_SHIFT) |
                SST_VO_PV_ALPHA_INV |  //always invert alpha
                (SST_VD_ALPHA_VALUE << SST_VO_PD_ALPHA_CREATE_SHIFT);

    //values = 0
   dwAlpha1 = 0;


   if( ((_DD(dwOVLFlags) & SRC_CKEY)  && ( dwCKEY & SRC_CKEY)) ||
   	   (_DD(dwOVLFlags)	& SRC_ALPHA))
   {
       //disabel source color key
              
       _DD(dwOVLFlags) &= ~(SRC_CKEY | SRC_ALPHA);
       //Disable the alpha in overlay
        SETDW(ghwVD->vdVoPvAlpha0, dwAlpha0);
        SETDW(ghwVD->vdVoPvAlpha1, dwAlpha1);
   }
   
    if( ((_DD(dwOVLFlags) & DEST_CKEY)  && ( dwCKEY & DEST_CKEY)) ||
   	   (_DD(dwOVLFlags)	& DEST_ALPHA))
    {
       //disable destination color key
       _DD(dwOVLFlags) &= ~(DEST_CKEY | DEST_ALPHA) ;
       //Disable the alpha in desktop
       {
        SETDW(ghwVD->vdVoPdAlpha0, dwAlpha0);
        SETDW(ghwVD->vdVoPdAlpha1, dwAlpha1);
       }
       //move desktop to layer2
       MoveDesktop(ppdev, FALSE,0);
       return TRUE;
   }
   
  
   return FALSE;
}

/*
----------------------------------------------------------------------------
  SetUpAlpha()

----------------------------------------------------------------------------
*/
void SetUpAlpha( NT9XDEVICEDATA * ppdev, DWORD dwAlpha, DWORD dwBitDepth, BOOL fAlphaInPixel, BOOL fNEG)
{

   DWORD  dwAlpha0, dwAlpha1;

   if(fNEG)
    dwAlpha0 =((dwBitDepth>6? 6:dwBitDepth) << SST_VO_PV_ALPHA_BITS_SHIFT);
  else
    dwAlpha0 =((dwBitDepth>6? 6:dwBitDepth) << SST_VO_PV_ALPHA_BITS_SHIFT)|
                 SST_VO_PV_ALPHA_INV;

   dwAlpha1 = 0;

    if((_DD(dwOVLFlags) & DEST_ALPHA) && fAlphaInPixel)
    {

        //set up destination alpha
        if( !fAlphaInPixel)
        {
          //we only has 6 bit alpha
           VD_SET_FIELD( dwAlpha0, SST_VO_PD_ALPHA_CREATE, SST_VD_ALPHA_VALUE); //alpha value
           if(dwBitDepth >= 8)
               VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_VALUE, (dwAlpha >> 2) & 0x3f);
           else
               VD_SET_FIELD( dwAlpha1, SST_VO_PD_ALPHA_VALUE, dwAlpha & 0x3f); 

        }

	    WAITFORUPDATE(SST_VO_PD_UPDATE | SST_VO_PS_UPDATE);

		SETDW( ghwVD->vdVoPdAlpha0, dwAlpha0);
        SETDW( ghwVD->vdVoPdAlpha1, dwAlpha1);
        //Also make sure layer2 alpha is not zero
         SETDW( ghwVD->vdVoPvAlpha0,
            (5 << SST_VO_PV_ALPHA_BITS_SHIFT)|
            (SST_VD_ALPHA_VALUE <<SST_VO_PD_ALPHA_CREATE_SHIFT));
        SETDW( ghwVD->vdVoPvAlpha1, 2 <<SST_VO_PV_ALPHA_VALUE_SHIFT);

        //move the desktop to layer1
        MoveDesktop( ppdev, TRUE,1);

    }
    else
    {
        //set up source alpha
         if(!fAlphaInPixel)
         {
           if(_DD(dwOVLFlags) & DEST_ALPHA)
               dwAlpha0 ^= SST_VO_PV_ALPHA_INV;

           //we only has 6 bit alpha
           VD_SET_FIELD( dwAlpha0, SST_VO_PV_ALPHA_CREATE, SST_VD_ALPHA_VALUE);//alhpa override
           if(dwBitDepth >= 8)
               VD_SET_FIELD( dwAlpha1, SST_VO_PV_ALPHA_VALUE, (dwAlpha >> 2) & 0x3f);
           else
               VD_SET_FIELD( dwAlpha1, SST_VO_PV_ALPHA_VALUE,  dwAlpha & 0x3f);
        }
 	    WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PS_UPDATE);

        SETDW( ghwVD->vdVoPvAlpha0, dwAlpha0);
        SETDW( ghwVD->vdVoPvAlpha1, dwAlpha1);

        //First move the desktop to layer2
        MoveDesktop( ppdev, FALSE,0);

    }
}

/*
----------------------------------------------------------------------------
  SetOverlayRegs()
----------------------------------------------------------------------------
*/
void SetOverlayRegs(NT9XDEVICEDATA * ppdev, OVL_REG * pOVLRegs)
{
    //set primary overlay
    SETDW(ghwVD->vdVoPvCfg0, pOVLRegs->dwCfg0);
    SETDW(ghwVD->vdVoPvCfg1, pOVLRegs->dwCfg1);
    SETDW(ghwVD->vdVoPvCfg2, pOVLRegs->dwCfg2);
    SETDW(ghwVD->vdVoPvCfg3, pOVLRegs->dwCfg3);
    SETDW(ghwVD->vdVoPvCfg4, pOVLRegs->dwCfg4);
    SETDW(ghwVD->vdVoPvCfg5, pOVLRegs->dwCfg5);
    SETDW(ghwVD->vdVoPvCfg6, pOVLRegs->dwCfg6);
    SETDW(ghwVD->vdVoPvCfg7, pOVLRegs->dwCfg7);
    SETDW(ghwVD->vdVoPvAddrCtl , pOVLRegs->dwAddrCtl);
    SETDW(ghwVD->vdVoPvBaseLeft, pOVLRegs->dwBaseLeft);
//    SETDW(ghwVD->vdVoPvBaseRight, pOVLRegs->dwBaseRight);

}
                 

/*
----------------------------------------------------------------------------
  FillDesktopInfo()
  Fill the BW info for desktop
----------------------------------------------------------------------------
*/
void FillDesktopInfo(NT9XDEVICEDATA *ppdev, LPBW_CONFIG lpBWConfig)
{

  lpBWConfig->sDesktop.dwScaleX = 0x1000;  //read from registers
  lpBWConfig->sDesktop.dwByteCount = GETPRIMARYBYTEDEPTH;

  GetMVCLK( lpBWConfig);
}


/*
----------------------------------------------------------------------------
  InitOverlay()
  Fill the Info sturcture
  if Overlay cannot dispaly in this mode return FALSE
----------------------------------------------------------------------------
*/

BOOL InitOverlay( NT9XDEVICEDATA *ppdev ,DDHALINFO    *pHalInfo)
{

  BW_CONFIG bw_Config;
#ifndef USE_CSIM
  DWORD lZoom;
#endif

  bw_Config = _DD(sBWConfig);  
  FillDesktopInfo(ppdev, &bw_Config);

  bw_Config.sOverlay.dwScaleX = 0x4000;     //worst case 4x zoom
  bw_Config.sOverlay.dwScaleY = 0x4000;
  bw_Config.sOverlay.dwByteCount = 2;
  bw_Config.sOverlay.dwFlags = 0;

#ifndef USE_CSIM
  if(!EnoughBandWidth(&bw_Config))
  {
     pHalInfo->ddCaps.ddsCaps.dwCaps &= ~DDSCAPS_OVERLAY;
     pHalInfo->ddCaps.dwCaps &= ~DDSCAPS_OVERLAY;
     return FALSE;                           //We cannot support any overlay
  }

  pHalInfo->ddCaps.dwMaxOverlayStretch   = 10000;

  //find out the miniOveralyStretch
  
  lZoom = 4000 - 400;

  do{

        bw_Config.sOverlay.dwScaleX = (1000 << 16)/ lZoom; 
        bw_Config.sOverlay.dwScaleY = (1000 << 16)/ lZoom ;
        if(!EnoughBandWidth( &bw_Config))
        {

          pHalInfo->ddCaps.dwMinOverlayStretch   =  lZoom + 400;
          break;
        }
        lZoom -= 400;
  }
  while( lZoom > 400);

  pHalInfo->ddCaps.dwMinOverlayStretch   = lZoom;
#else
  pHalInfo->ddCaps.dwMaxOverlayStretch   = 10000;
  pHalInfo->ddCaps.dwMinOverlayStretch   = 125;
#endif

  pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_OVERLAY |
                                     DDSCAPS_ALPHA;

  pHalInfo->ddCaps.dwCaps |= 
                            DDCAPS_OVERLAY
                          | DDCAPS_OVERLAYCANTCLIP
                          | DDCAPS_OVERLAYFOURCC
                         // | DDCAPS_ALIGNSTRIDE  //we should have this
                          | DDCAPS_OVERLAYSTRETCH
#if (DX <= 6)
                          | DDCAPS_STEREOVIEW     //Check this
#endif
                          | DDCAPS_ALPHA
                          ;
  pHalInfo->ddCaps.dwCaps2 |=
                             DDCAPS2_AUTOFLIPOVERLAY
                           | DDCAPS2_CANBOBINTERLEAVED
                           | DDCAPS2_CANBOBNONINTERLEAVED
                           //| DDCAPS2_COLORCONTROLOVERLAY
                           | DDCAPS2_NOPAGELOCKREQUIRED
                           | DDCAPS2_CANBOBHARDWARE
                           | DDCAPS2_CANFLIPODDEVEN
                           ;
 pHalInfo->ddCaps.dwFXCaps |=
                                DDFXCAPS_OVERLAYARITHSTRETCHY
                            //| DDFXCAPS_OVERLAYARITHSTRETCHYN
                            | DDFXCAPS_OVERLAYSHRINKX
                            //| DDFXCAPS_OVERLAYSHRINKXN
                            | DDFXCAPS_OVERLAYSHRINKY
                            //| DDFXCAPS_OVERLAYSHRINKYN
                            | DDFXCAPS_OVERLAYSTRETCHX
                            //| DDFXCAPS_OVERLAYSTRETCHXN
                            | DDFXCAPS_OVERLAYSTRETCHY
                            //| DDFXCAPS_OVERLAYSTRETCHYN
                            | DDFXCAPS_OVERLAYALPHA
                            ;

  pHalInfo->ddCaps.dwCKeyCaps |= 
                                DDCKEYCAPS_DESTOVERLAY
                              | DDCKEYCAPS_DESTOVERLAYYUV
                            //Only one Dest Color key
                              |  DDCKEYCAPS_SRCOVERLAYONEACTIVE
                              | DDCKEYCAPS_NOCOSTOVERLAY    //May be not Xing
                              //| DDCKEYCAPS_DESTOVERLAYCLRSPACE
                              | DDCKEYCAPS_SRCOVERLAY
                              | DDCKEYCAPS_SRCOVERLAYCLRSPACE
                              | DDCKEYCAPS_SRCOVERLAYYUV
                              | DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV
                              ;

  pHalInfo->ddCaps.dwFXAlphaCaps |= 
                                   DDFXALPHACAPS_OVERLAYALPHAPIXELS|
                                   DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG
                                 ;

  pHalInfo->vmiData.dwOverlayAlign       = 32;

  pHalInfo->ddCaps.dwMaxVisibleOverlays  = 1;
  pHalInfo->ddCaps.dwCurrVisibleOverlays = 0;
  pHalInfo->ddCaps.dwAlphaOverlayConstBitDepths = DDBD_2|DDBD_4|DDBD_8;  // actually only 6 for HW
  pHalInfo->ddCaps.dwAlphaOverlaySurfaceBitDepths = DDBD_1|DDBD_8;

  return TRUE;

}


/*
----------------------------------------------------------------------------
  ValidateFormat()
  Check to see overlay can support this surface color format
  Notice dwBotCount is only used when lpFormat == 0
----------------------------------------------------------------------------
*/
HRESULT ValidateOverlayFormat(LPDDPIXELFORMAT lpFormat,
                    DWORD dwBitCount, DWORD *pSSTFormat)
{
  DWORD dwSSTFormat;
  if( lpFormat )
  {
      if (lpFormat->dwFlags & DDPF_FOURCC)
      {
          if((lpFormat->dwFourCC == FOURCC_YUY2 )
           || (lpFormat->dwFourCC == FOURCC_YV12 )
            )
          {
             dwSSTFormat = SST_VD_YUV_YUYV;
             lpFormat->dwYUVBitCount = 16;
          }
          else if(lpFormat->dwFourCC == FOURCC_UYVY )
          {
              dwSSTFormat = SST_VD_YUV_UYVY;
              lpFormat->dwYUVBitCount = 16;
          }
          else
              return DDERR_INVALIDPIXELFORMAT;

      }
      else if (lpFormat->dwFlags & DDPF_RGB)
      {
          if ((lpFormat->dwRGBBitCount == 16)  &&
               (lpFormat->dwRBitMask == 0xf800) &&
               (lpFormat->dwGBitMask == 0x07e0) &&
               (lpFormat->dwBBitMask == 0x001f))   //RGB5:6:5
          {
              dwSSTFormat = SST_VD_RGB565;
          }
          else if((lpFormat->dwRGBBitCount == 16)  &&
                 (lpFormat->dwRBitMask == 0x7c00) &&
                 (lpFormat->dwGBitMask == 0x003e0) &&
                 (lpFormat->dwBBitMask == 0x00001f))    //RGB5:5:5
          {
              dwSSTFormat = SST_VD_RGB1555;
          }
          else if ((lpFormat->dwRGBBitCount == 32)  &&
                 (lpFormat->dwRBitMask == 0xff0000) &&
                 (lpFormat->dwGBitMask == 0x00ff00) &&
                 (lpFormat->dwBBitMask == 0x0000ff))
          {
              dwSSTFormat = SST_VD_RGB32;                 //32 bit RGB
         }
         else
             return DDERR_INVALIDPIXELFORMAT;
 
      }
      else if(lpFormat->dwFlags & DDPF_YUV)
      {

            if(((lpFormat->dwYUVBitCount == 32)  &&
                 (lpFormat->dwYBitMask == 0xff0000) &&
                 (lpFormat->dwUBitMask == 0x00ff00) &&
                 (lpFormat->dwVBitMask == 0x0000ff)))
            {
                dwSSTFormat = SST_VD_YUVA;               //32bit YUV
            }
      }
      else if (DDPF_PALETTEINDEXED8 & lpFormat->dwFlags)
      {
                dwSSTFormat = SST_VD_8BPP;
      }
      else
             return DDERR_INVALIDPIXELFORMAT;
  }
  else      //desktop color format
  {
          //support 16 RGB5:6:5 , 32 bpp, 8bpp
         switch( dwBitCount )
         {
            case 8:
                dwSSTFormat = SST_VD_8BPP;
                break;
            case 16:
               dwSSTFormat = SST_VD_RGB565;
               break;
           case 32:
               dwSSTFormat = SST_VD_RGB32;
               break;
          default:
             return DDERR_INVALIDPIXELFORMAT;
        }
  }

  if( pSSTFormat)
      *pSSTFormat =dwSSTFormat;

  return DD_OK;
}

/*
----------------------------------------------------------------------------
  SetUpViForShrink()
  Shrink overlay using VI.
  Note:  Overlay surface must in YUYV or UYVY format
----------------------------------------------------------------------------
*/

void SetUpViForShrink(NT9XDEVICEDATA *ppdev,
            FXSURFACEDATA *srcSurface, DWORD dwFlags)
{
   DWORD dwXScale, dwYScale;
   DWORD dwDstWidth, dwDstHeight,dwHSeed;
   VIP_REG vipRegs;
   
   memset(&vipRegs, 0, sizeof(VIP_REG));

   
    if(dwFlags& DDOVER_INTERLEAVED)
    {
	  if(dwFlags & DDOVER_BOB)
		dwFlags = VPE_BOB | VPE_INTERLACED;
	  else 
		dwFlags = VPE_WEAVE | VPE_INTERLACED;

    }    
    else 
       dwFlags = 0;

    vipRegs.dwViCfg0 = SST_VI_EN;
    vipRegs.dwViCfg4 = SST_VI_V_FIELD_EN_0 | SST_VI_V_FIELD_EN_1;

    if(_DD(dwXScale) > 0x10000)
    {
      //x shrink
      dwDstWidth = _DD(ovlDest).right - _DD(ovlDest).left;

    }
    else
      dwDstWidth = _DD(ovlSrc).right - _DD(ovlSrc).left;


    if(_DD(dwYScale) > 0x10000)
    {
      //y shrink
      dwDstHeight = _DD(ovlDest).bottom - _DD(ovlDest).top;
      // find extra adjustment to prevent last line read from bad source data
      if(_DD(dwYScale) <0x14000)    
        dwHSeed = 0x600;
      else if(_DD(dwYScale) <0x18000)
        dwHSeed = 0x400;
      else if(_DD(dwYScale) <0x1C000)
        dwHSeed = 0x200;
      else
        dwHSeed = 0;

    }
    else
    {
      dwDstHeight = _DD(ovlSrc).bottom - _DD(ovlSrc).top;
      dwHSeed = 0;
    }    
    dwYScale = (((_DD(ovlSrc).bottom - _DD(ovlSrc).top) << 12) - dwHSeed) /
					dwDstHeight;


	dwXScale = ((_DD(ovlSrc).right - _DD(ovlSrc).left) << 12) /
					dwDstWidth;

   if( dwXScale > 0x1000)
   {
      dwHSeed =  dwXScale >> 13;
   }
   else 
    dwHSeed = 0;


    SetVIRegs(  dwDstWidth, dwDstHeight, dwXScale, dwYScale,
                (_DD(ovlSrc).left + dwHSeed + srcSurface->dwOvlHOffset) & ~1,
                _DD(ovlSrc).top, _DD(ovlSrc).top,
                dwFlags, &vipRegs);

    vipRegs.dwVIntDlyAddrCtl |= _DD(filterMemStride) * 2;
    vipRegs.dwLpfDlyAddrCtl  |= _DD(filterMemStride);


    vipRegs.dwViLpfDlyBase0 = _DD(lpFilterMem0);
    vipRegs.dwViLpfDlyBase1= _DD(lpFilterMem1);
    vipRegs.dwViVIntDlyBase0 = _DD(lpFilterMem0) - _DD(filterMemStride )* 6;
    if(srcSurface->dwPixelFormat == SST_VD_YUV_UYVY)
    {
       vipRegs.dwVipVideoFifoCntl =
               SST_VI_VV_H_ANCILLARY_DISABLE|
               SST_VI_VV_V_ANCILLARY_DISABLE| 
               (SST_VD_INTERNAL_INPUT << SST_VI_VV_IP_MUX_SEL_SHIFT) |
               ((_DD(ovlSrc).right + srcSurface->dwOvlHOffset)<< SST_VI_HSRC_WIDTH_SHIFT);
    }
    else
    {
       vipRegs.dwVipVideoFifoCntl =
               SST_VI_VV_H_ANCILLARY_DISABLE|
               SST_VI_VV_V_ANCILLARY_DISABLE| 
               (SST_VD_INTERNAL_INPUT << SST_VI_VV_IP_MUX_SEL_SHIFT) |
               ((_DD(ovlSrc).right + srcSurface->dwOvlHOffset)<< SST_VI_HSRC_WIDTH_SHIFT)|
               SST_VI_VV_YC_ORDER;
    }

    SETDW( ghwVD->vdViCfg1, vipRegs.dwViCfg1);
    SETDW( ghwVD->vdViCfg2, vipRegs.dwViCfg2);
    SETDW( ghwVD->vdViCfg3, vipRegs.dwViCfg3);
    SETDW( ghwVD->vdViCfg4, vipRegs.dwViCfg4);
    SETDW( ghwVD->vdViCfg5, vipRegs.dwViCfg5);
    SETDW( ghwVD->vdVipVideoFifoCntl, vipRegs.dwVipVideoFifoCntl);
    SETDW( ghwVD->vdViVIntDlyBase0, vipRegs.dwViVIntDlyBase0);
    SETDW( ghwVD->vdViLpfDlyBase0, vipRegs.dwViLpfDlyBase0);
    SETDW( ghwVD->vdViLpfDlyBase1, vipRegs.dwViLpfDlyBase1);
    SETDW( ghwVD->vdViVIntDlyAddrCtl, vipRegs.dwVIntDlyAddrCtl);
    SETDW( ghwVD->vdViLpfDlyAddrCtl, vipRegs.dwLpfDlyAddrCtl);
    SETDW( ghwVD->vdViCfg0, vipRegs.dwViCfg0);

} 

/*
----------------------------------------------------------------------------
  StartViShrink()
  Shrink overlay using VI.
  Note:  Overlay surface must in YUYV or UYVY format
----------------------------------------------------------------------------
*/
void StartViShrink(NT9XDEVICEDATA *ppdev,
            FXSURFACEDATA *srcSurface, DWORD dwFlags)
{
  FXSURFACEDATA * dstSurface;
  DWORD dwSrcAddress, dwDstAddress, dwStride;

   dwSrcAddress = srcSurface->hwPtr;

   _FF(dwOVLSurfStatus) ^=VI_USE_SURF2;       //change current surface
   
   dstSurface = _DD(shrinkSurface1);
   if(_FF(dwOVLSurfStatus) & VI_USE_SURF2 )
   {
      dwDstAddress = _DD(shrinkPtr2);

   }
   else
   {
      dwDstAddress = dstSurface->hwPtr;
   }

   dwStride = dstSurface->dwStride;

  /* if(dwFlags & DDOVER_INTERLEAVED)
   {
      //double the stride for one field
      dwStride <<= 1;
        
      if(dwFlags & DDFLIP_ODD)
      {
        //adjust one line
        dwSrcAddress += dwStride;
      }
   }
   */

   if((dstSurface->tileFlag & MEM_IN_TILE0))
   {

        SETDW( ghwVD->vdViFrameAddrCtl, (dwStride >> 5) |
            (SST_VD_TILE_MODE_0 << SST_VI_FRAME_TILE_SHIFT) ); //use single buff
    }
    else if((dstSurface->tileFlag & MEM_IN_TILE1))
    {
        SETDW( ghwVD->vdViFrameAddrCtl, (dwStride >> 5) |
            (SST_VD_TILE_MODE_1 << SST_VI_FRAME_TILE_SHIFT));
    }
    else
    {
        SETDW( ghwVD->vdViFrameAddrCtl, dwStride );
    }

#ifdef FXTRACE
  DISPDBG((ppdev, 0, "VIShrink srcAdd=%lx, dstAdd=%lx",
        dwSrcAddress,dwDstAddress));
#endif

    SETDW( ghwVD->vdViFrameBase0, dwDstAddress);

    dwStride = srcSurface->dwPStride;

    if((srcSurface->tileFlag & MEM_IN_TILE0))
    {

        SETDW( ghwVD->vdViReferenceAddrCtl, dwStride |
            (SST_VD_TILE_MODE_0 << SST_VI_REF_TILE_SHIFT));
    }
    else if((srcSurface->tileFlag & MEM_IN_TILE1))
    {
        SETDW( ghwVD->vdViReferenceAddrCtl, dwStride |
            (SST_VD_TILE_MODE_1 << SST_VI_REF_TILE_SHIFT));
    }
    else
    {
        SETDW( ghwVD->vdViReferenceAddrCtl, dwStride);
    }


    //starts vi
    if(dwFlags & DDFLIP_ODD)
    {
       //use buffer 1
    Msg(ppdev, 0, "VI ODD");
       SETDW( ghwVD->vdViReferenceBase1, dwSrcAddress);
       SETDW( ghwVD->vdViStatus, SST_VI_UPDATE | SST_VI_BASE_UPDATE |
        SST_VI_INT_FID);

    }
    else
    {
    Msg(ppdev, 0, "VI EVEN");
        SETDW( ghwVD->vdViReferenceBase0, dwSrcAddress);
        SETDW( ghwVD->vdViStatus, SST_VI_UPDATE | SST_VI_BASE_UPDATE);
    }
//    while( GET(ghwVD->vdViStatus) & SST_VI_UPDATE)
//        ;                           //wait for unpdating

}

/*
----------------------------------------------------------------------------
  CanCreateOverlaySurface()
  Check to see overlay can support this surface color format
----------------------------------------------------------------------------
*/
HRESULT CanCreateOverlaySurface(NT9XDEVICEDATA *ppdev,
            LPDDPIXELFORMAT lpFormat, DWORD dwBitCount,DWORD dwCaps)
{

  //Check overlay windows are available
  if( _DD(overlaySurfaceCnt) && !(dwCaps & DDSCAPS_BACKBUFFER))
       return DDERR_NOOVERLAYHW;

   //Check Color Format
   return ValidateOverlayFormat( lpFormat, dwBitCount, 0);
}

/*
----------------------------------------------------------------------------
  CreateOverlaySurface()
----------------------------------------------------------------------------
*/

HRESULT CreateOverlaySurface(NT9XDEVICEDATA *ppdev,
            LPDDRAWI_DDRAWSURFACE_LCL lpSurface,
            DWORD dwCaps,
            DWORD pWidth, DWORD height, DWORD * pixelByteDepth, int iSurface)
{

   FXSURFACEDATA * surfaceData,*surfaceDataOrg;
   LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl;
   DWORD dwAlpha0,dwAlpha1;
   DWORD dwFormat,dwRet;
   DWORD dwHeight;

   if( !iSurface && _DD(overlaySurfaceCnt) && !(dwCaps & DDSCAPS_BACKBUFFER))
       return DDERR_NOOVERLAYHW;

   surfaceDataOrg = GET_SURF_DATA(lpSurface);

   psurf_gbl = lpSurface->lpGbl;

   if((lpSurface->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
   {
         *pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
         dwRet =
         ValidateOverlayFormat( &(psurf_gbl->ddpfSurface), *pixelByteDepth << 3,&dwFormat);
    }
    else
    {
         *pixelByteDepth = GETPRIMARYBYTEDEPTH;
         dwRet =
         ValidateOverlayFormat( 0, *pixelByteDepth << 3,&dwFormat);
    }

    if( dwRet != DD_OK )
        return dwRet;

    surfaceDataOrg->dwPixelFormat = dwFormat;

   if(_DD(overlaySurfaceCnt) <= 1)
   {
     if(!(dwCaps & DDSCAPS_VIDEOPORT) && !_DD(overlaySurfaceCnt) &&
        ((dwFormat == SST_VD_YUV_UYVY) || (dwFormat == SST_VD_YUV_YUYV)))
     {

        surfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));

        if(!surfaceData)
        {
          return DDERR_OUTOFVIDEOMEMORY;
        }

        //need extra memory for vi filter
        dwHeight =  2* (height + 4) + 6;   //two extra field for LpfDly and
                                    //3  double lines for IntDly

        //allocates shrink surface
        surfaceData->tileFlag = MEM_IN_LINEAR;  //must be linear
        surfaceData->dwFlags |= FXSURFACE_IS_EXCLUSIVE;

        dwRet = surfMgr_allocSurface(
                psurf_gbl->lpDD,          // direct draw vidmemalloc need this
                0,                        // type of surface (can use the standard DD surface flags) [IN]
                0,                       // type of surface extension [IN]
                (pWidth * 2+ 31) & ~31,              // width in bytes (linear space) [IN]
                dwHeight,                  // height in pixel (linear space) [IN]
                0,                        // linear address mask
                &(surfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
                &(surfaceData->endlfbPtr),// host lfb end address of allocation [OUT]
                &(surfaceData->hwPtr),    // hw vidmem address
                &(surfaceData->dwStride), // pitch [OUT]
                &(surfaceData->tileFlag)  // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                );

        if ( dwRet != DD_OK )
        {
            DXFREE((void*)surfaceData);
            return DDERR_OUTOFVIDEOMEMORY;
        }


		_DD(lpFilterMem0) = surfaceData->hwPtr;
		_DD(lpFilterMem1) = _DD(lpFilterMem0) +
				(height + 4) * surfaceData->dwStride;
		_DD(filterMemStride) = surfaceData->dwStride;
		_FF(dwOVLSurfStatus) = 0;

        //need allocate 2 shrink buffers
        dwHeight = 2* height;

        //allocates shrink surface
        surfaceData->tileFlag = MEM_IN_LINEAR;  //must be linear
        surfaceData->dwFlags |= FXSURFACE_IS_EXCLUSIVE;

        dwRet = surfMgr_allocSurface(
                psurf_gbl->lpDD,          // direct draw vidmemalloc need this
                0,                        // type of surface (can use the standard DD surface flags) [IN]
                0,                       // type of surface extension [IN]
                (pWidth * 2+ 31) & ~31,              // width in bytes (linear space) [IN]
                dwHeight,                  // height in pixel (linear space) [IN]
                0,                        // linear address mask
                &(surfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
                &(surfaceData->endlfbPtr),// host lfb end address of allocation [OUT]
                &(surfaceData->hwPtr),    // hw vidmem address
                &(surfaceData->dwStride), // pitch [OUT]
                &(surfaceData->tileFlag)  // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                );

        if ( dwRet != DD_OK )
        {
            DXFREE((void*)surfaceData);
            goto ddcos_cleanupalloc;
        }


		_DD(shrinkSurface1)= surfaceData;
		_FF(dwShrinkSurfAddr1) = surfaceData->hwPtr;
		_DD(shrinkPtr2)= _FF(dwShrinkSurfAddr2) =
			surfaceData->hwPtr + height * surfaceData->dwStride;;
	 } 
     if(psurf_gbl->ddpfSurface.dwFourCC == FOURCC_YV12)
     {
		surfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));

		if(!surfaceData)
		{
            goto ddcos_cleanupalloc;
		}

		//allocates shrink surface
		surfaceData->tileFlag = MEM_IN_TILE1 | MEM_AT_16K;  //must be tile
      surfaceData->dwFlags |= FXSURFACE_IS_EXCLUSIVE;

		dwRet = surfMgr_allocSurface(
				psurf_gbl->lpDD,          // direct draw vidmemalloc need this
				0,                        // type of surface (can use the standard DD surface flags) [IN]
				0,                       // type of surface extension [IN]
				(pWidth * 2+ 31)& ~31,              // width in bytes (linear space) [IN]
				height,                  // height in pixel (linear space) [IN]
				0,                        // linear address mask
				&(surfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
				&(surfaceData->endlfbPtr),// host lfb end address of allocation [OUT]
				&(surfaceData->hwPtr),    // hw vidmem address
				&(surfaceData->dwStride), // pitch [OUT]
				&(surfaceData->tileFlag)  // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
				);

		if ( dwRet != DD_OK )
		{
            DXFREE((void*)surfaceData);
            goto ddcos_cleanupalloc;
		}

		surfaceData->dwPixelFormat = SST_VD_YUV_YUYV;
		surfaceData->dwBytesPerPixel =1;

		if( _DD(overlaySurfaceCnt) == 0)
		{

		   _DD(yv12Surface1)= surfaceData;
		   _FF(dwYV12SurfAddr1) = surfaceData->hwPtr;
		   _FF(dwOVLSurfStatus) &= ~YV12_USE_TWO_BUFF;
		}
		else
		{
		   _DD(yv12Surface2)= surfaceData;
		   _FF(dwYV12SurfAddr2) = surfaceData->hwPtr;
		   _FF(dwOVLSurfStatus) |= YV12_USE_TWO_BUFF;
		}
     }
    
   }

   if(_DD(overlaySurfaceCnt)++)
     return DD_OK;

   dwAlpha0 =(5 << SST_VO_PV_ALPHA_BITS_SHIFT) |
                SST_VO_PV_ALPHA_INV |  //always invert alpha
                (SST_VD_ALPHA_VALUE << SST_VO_PD_ALPHA_CREATE_SHIFT);

   dwAlpha1 = 0;
  // MoveDesktop(ppdev, FALSE, 0);    //move desktop to layer2
 
/*
    //this is used for recover from fifo underflow problem
    //it should be gone after the problem is fixed in display driver.
   {       
     DWORD dwPsCfg0, dwPmCfg;
     dwPsCfg0 = GET(ghwVD->vdVoPsCfg0);
     dwPmCfg  = GET(ghwVD->vdVoPmCfg);
     SETDW( ghwVD->vdVoPsCfg0, dwPsCfg0 & ~SST_VO_PS_EN);
     SETDW( ghwVD->vdVoPmCfg,0);
     SETDW( ghwVD->vdVoPsStatus0, 0x1F);
     while( GET(ghwVD->vdVoPsStatus0) & SST_VO_PS_UPDATE)
        ;
     SETDW( ghwVD->vdVoPsCfg0, dwPsCfg0 );
     SETDW( ghwVD->vdVoPmCfg,dwPmCfg);

     SETDW( ghwVD->vdVoPsStatus0, 0x1F);
     while( GET(ghwVD->vdVoPsStatus0) & SST_VO_PS_UPDATE)
        ;
      
    }
*/ 

//   WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE | SST_VO_PS_UPDATE);
   SETDW(ghwVD->vdVoPvAlpha0 , dwAlpha0);
   SETDW(ghwVD->vdVoPvAlpha1, dwAlpha1);
   SETDW(ghwVD->vdVoPdAlpha0 , dwAlpha0);
   SETDW(ghwVD->vdVoPdAlpha1, dwAlpha1);

   _DD(dwOVLFlags) &= (DXVA_ON | PRE_DXVA);  //clear flag

                          //so that overlay is on the top of it.
   _DD(visibleOverlaySurf) = 0;

   MoveDesktop(ppdev, FALSE, 0);    //move desktop to layer2

   UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE |
	   SST_VO_PC_UPDATE |SST_VO_PS_UPDATE);
   //make sure it is updated
//   WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PS_UPDATE);

#ifdef USE_CSIM
//   CSIM_Setup( ppdev,lpSurface->lpGbl, TRUE);
#endif
   return DD_OK;

//RYAN@20001111, cleanup on alloc failure
ddcos_cleanupalloc:
   if ( !(dwCaps & DDSCAPS_VIDEOPORT) && !_DD(overlaySurfaceCnt) )
   {
      switch (dwFormat)
      {
         case SST_VD_YUV_UYVY:
         case SST_VD_YUV_YUYV:
            if (_DD(lpFilterMem0))
            {
               surfMgr_freeSurface(psurf_gbl->lpDD, 0, _DD(lpFilterMem0), 0);
              _DD(lpFilterMem0) = 0;
            }
            if (_DD(shrinkSurface1))
            {
               DXFREE((void*)_DD(shrinkSurface1));
               if (_FF(dwShrinkSurfAddr1))
                  surfMgr_freeSurface(psurf_gbl->lpDD, 0, _FF(dwShrinkSurfAddr1), 0);

               _DD(shrinkSurface1) = 0;
               _FF(dwShrinkSurfAddr1) = 0;
            }
            break;
      }
   }

   return DDERR_OUTOFVIDEOMEMORY;
}



/*
----------------------------------------------------------------------------
  DestoryOverlaySurface()
----------------------------------------------------------------------------
*/

VOID DestroyOverlaySurface(  LPDDHAL_DESTROYSURFACEDATA lpInput)
{

  FXSURFACEDATA * pSurfaceData;
  DD_ENTRY_SETUP(lpInput->lpDD);

   if((pSurfaceData = GET_SURF_DATA(lpInput->lpDDSurface)) == 0)
       return;

   if( _DD(overlaySurfaceCnt) <= 2)
   {

        if((pSurfaceData->dwPixelFormat == SST_VD_YUV_UYVY) ||
            (pSurfaceData->dwPixelFormat == SST_VD_YUV_YUYV))
        {

             if( _DD(overlaySurfaceCnt)== 1)
             {

		       if( _DD(lpFilterMem0))
                   surfMgr_freeSurface(
                     lpInput->lpDDSurface->lpGbl->lpDD,          // direct draw vidmemalloc need this
                      0,                               // host lfb start address of allocation [OUT]
                     _DD(lpFilterMem0),                // hw start address of allocation [OUT]
                     0                                 // ddscaps (need for nonlocal surfaces)
                     );
               if(_DD(shrinkSurface1))
               {
                   surfMgr_freeSurface(
                     lpInput->lpDDSurface->lpGbl->lpDD,          // direct draw vidmemalloc need this
                     _DD(shrinkSurface1)->lfbPtr,      // host lfb start address of allocation [OUT]
                     _DD(shrinkSurface1)->hwPtr,       // hw start address of allocation [OUT]
                     0                                 // ddscaps (need for nonlocal surfaces)
                     );
                    DXFREE((void*)_DD(shrinkSurface1));
                
                     _DD(shrinkSurface1) = 0;
                }

               if(_DD(yv12Surface1))
               {
                   surfMgr_freeSurface(
                     lpInput->lpDDSurface->lpGbl->lpDD,          // direct draw vidmemalloc need this
                     _DD(yv12Surface1)->lfbPtr,      // host lfb start address of allocation [OUT]
                     _DD(yv12Surface1)->hwPtr,       // hw start address of allocation [OUT]
                     0                               // ddscaps (need for nonlocal surfaces)
                     );
                    DXFREE((void*)_DD(yv12Surface1));

                    _DD(yv12Surface1) = 0;
                }
             }
             else
             {
                
                if(_DD(yv12Surface2))
                {
                   surfMgr_freeSurface(
                     lpInput->lpDDSurface->lpGbl->lpDD,          // direct draw vidmemalloc need this
                     _DD(yv12Surface2)->lfbPtr,      // host lfb start address of allocation [OUT]
                     _DD(yv12Surface2)->hwPtr,       // hw start address of allocation [OUT]
                     0                               // ddscaps (need for nonlocal surfaces)
                     );
                    DXFREE((void*)_DD(yv12Surface2));

                    _DD(yv12Surface2) = 0;
                }
             }
        }

        if( _DD(overlaySurfaceCnt)== 1)
        {
        #ifdef USE_CSIM
        //         CSIM_Setup( ppdev,lpInput->lpDDSurface->lpGbl, FALSE);
        #endif
			
			if(_DD(dwOVLFlags) & OVL_ON) 
            {
                //disable overlay
                DisableOverlay(ppdev);
                UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE
                     | SST_VO_PS_UPDATE | SST_VO_PC_UPDATE);
  
        #ifdef USE_CSIM
                CSIM_Update(ppdev);
        #endif

            }
			
//			WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE | SST_VO_PS_UPDATE);
            MoveDesktop(ppdev, TRUE, 0);
			UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE | SST_VO_PS_UPDATE);

        }
   }

   //clear destination color key owner

   _DD(dwOVLFlags) &=  DXVA_ON;  //clear flag
                       
   if(_DD(overlaySurfaceCnt) > 0 )
       _DD(overlaySurfaceCnt)--;

}


/*
----------------------------------------------------------------------------
  GetOverlayFlipStatus()
  Check last flip is done.
----------------------------------------------------------------------------
*/

HRESULT GetOverlayFlipStatus(NT9XDEVICEDATA *ppdev, DWORD dwFlags,
                DWORD fpVidMem)
{

  if ((dwFlags == DDGFS_CANFLIP) ||
       ((_DD(dwOVLFlags) & OVL_FLIP) &&
        ((fpVidMem == 0) || (fpVidMem == _DD(ddSurfaceFlippedFrom)))))
  {
      volatile DWORD dwStatus =   GET(ghwVD->vdVoPsStatus0); //make sure read as DWORD
      if( _FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)
      {
         if((GET(ghwVD->vdViStatus) & SST_VI_INT_TRIGGER) ||
            (dwStatus & SST_VO_PV_UPDATE))
            return (DWORD)DDERR_WASSTILLDRAWING;

      }
      else if( dwStatus & (SST_VO_PV_BASE_UPDATE | SST_VO_PV_UPDATE))
          return (DWORD)DDERR_WASSTILLDRAWING;

     _DD(dwOVLFlags) &= ~OVL_FLIP;
     return DD_OK;
  }

  return DD_OK;
}

/*
----------------------------------------------------------------------------
  FlipOverlaySurface()
----------------------------------------------------------------------------
*/
DWORD FlipOverlaySurface( LPDDHAL_FLIPDATA lpFlipData )
{
  DWORD           dwSurfBase;

  FXSURFACEDATA * pSurfaceData;
  DD_ENTRY_SETUP(lpFlipData->lpDD);
#ifdef FXTRACE
  Msg(ppdev, 0, "FlipOverlay" );
#endif

	if((pSurfaceData = GET_SURF_DATA(lpFlipData->lpSurfTarg)) == 0)
	{

		#ifdef FXTRACE
		DISPDBG((ppdev, 0, "FlipOverlay No SufaceData" ));
		#endif

		lpFlipData->ddRVal = DDERR_NOTAOVERLAYSURFACE;
		return DDHAL_DRIVER_HANDLED;
	}

	CheckFifo(ppdev);

	_DD(visibleOverlaySurf) = GET_HW_ADDR(lpFlipData->lpSurfTarg);

	if (!(lpFlipData->dwFlags & DDFLIP_NOVSYNC) &&
		(GetOverlayFlipStatus(ppdev,DDGFS_CANFLIP,0) != DD_OK))
	{
		DISPDBG((ppdev, 0, "FlipOverlay Flip Waitting" ));

		lpFlipData->ddRVal = DDERR_WASSTILLDRAWING;
		return DDHAL_DRIVER_HANDLED;
	}

	if(lpFlipData->lpSurfTarg->lpGbl->ddpfSurface.dwFourCC
		== FOURCC_YV12)
	{
		if(pSurfaceData->dwFlags & SURFACE_UPDATE)
		{
		   ConvertYV12(ppdev, pSurfaceData,
			 (DWORD) lpFlipData->lpSurfTarg->lpGbl->wWidth,
			 (DWORD) lpFlipData->lpSurfTarg->lpGbl->wHeight);
		   pSurfaceData->dwFlags &= ~ SURFACE_UPDATE;
		}

		if(_FF(dwOVLSurfStatus) & VI_USE_SURF2 )
		{
			pSurfaceData = _DD(yv12Surface2);
		}
		else
		{
			pSurfaceData = _DD(yv12Surface1);
		}
	}


	if( (_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)||
		((_DD(dwOVLFlags) & DXVA_ON) && 
		 (lpFlipData->dwFlags & (DDFLIP_EVEN | DDFLIP_ODD))) )
	{
	//using vi to shrink
		//Msg(ppdev, 0, "FlipOverlay preFlipAddr= %lx",GET(ghwVD->vdViCurrentBase));
		if(_DD(dwOVLFlags) & DXVA_ON)
		{
			DWORD dwCfg0 = GET(ghwVD->vdVoPvCfg0);
			if(!(dwCfg0 & ( SST_VD_VI_SWAP << SST_VO_PV_SWAP_CTL_SHIFT)))
			{	
				VD_SET_FIELD( dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_VI_SWAP); //use address in VI
				UPDATEREGISTER(SST_VO_PV_UPDATE );

			}
			_FF(dwOVLSurfStatus) &= ~VI_USED_FOR_SHRINK;

		}

		if(lpFlipData->lpSurfTarg->lpSurfMore->dwOverlayFlags & DDOVER_INTERLEAVED)
			 StartViShrink( ppdev,
				pSurfaceData, lpFlipData->dwFlags|DDOVER_INTERLEAVED);
		else
			 StartViShrink( ppdev,
				pSurfaceData, lpFlipData->dwFlags);

		TRIGGER_VI;

		if(lpFlipData->dwFlags & DDFLIP_WAIT)
		{
			WAITFORTRIGGER;
		}
	}
	else
	{
		// Determine the offset to the new area.
		 dwSurfBase = pSurfaceData->hwPtr;

		//see the data is interleaved or not)
	    WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PV_BASE_UPDATE);

		if(_DD(dwOVLFlags) & DXVA_ON)
		{
			DWORD dwCfg0 = GET(ghwVD->vdVoPvCfg0);
			if( dwCfg0 & ( SST_VD_CPU_SWAP << SST_VO_PV_SWAP_CTL_SHIFT))
			{	
				VD_SET_FIELD( dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_CPU_SWAP); 
			}

		}
		if(lpFlipData->dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
		{
			DWORD dwCfg3;

				dwCfg3 = _DD(ovlSrc).bottom - _DD(ovlSrc).top;

				if(lpFlipData->dwFlags * DDFLIP_EVEN)
				{
				//adjust half line down
					dwCfg3 += 0x80;

				}
				else
				{
				//adjust half line up
				dwCfg3 -= 0x80;
				if(lpFlipData->lpSurfTarg->lpSurfMore->dwOverlayFlags &
						DDOVER_INTERLEAVED)
					dwSurfBase += pSurfaceData->dwStride;
				}

				SETDW( ghwVD->vdVoPvCfg3, dwCfg3);
		}

		SETDW(ghwVD->vdVoPvBaseLeft, dwSurfBase);

		UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PV_BASE_UPDATE);

	//    UpdateRegisters(ppdev, FALSE,FALSE, TRUE);
	}

	if(lpFlipData->dwFlags & DDFLIP_WAIT)
	{
	    WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PV_BASE_UPDATE);

		_DD(dwOVLFlags) &= ~OVL_FLIP;
	}
	else
		_DD(dwOVLFlags) |= OVL_FLIP;
	#ifdef USE_CSIM
	CSIM_Update(ppdev);
	#endif
	lpFlipData->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}


/*
----------------------------------------------------------------------------
  SetOverlayPosition32()
----------------------------------------------------------------------------
*/
DWORD __stdcall SetOverlayPosition32( LPDDHAL_SETOVERLAYPOSITIONDATA psopd )
{

  WORD  wDesktopW, wDesktopH;
  FXSURFACEDATA * pSurfaceData;
  long lDstLeft, lDstTop;

  DD_ENTRY_SETUP(psopd->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, 0, "SetOverlayPosition32" ));
  #endif


	if((pSurfaceData = GET_SURF_DATA(psopd->lpDDSrcSurface)) == 0)
	{

	psopd->ddRVal = DDERR_NOTAOVERLAYSURFACE;
	return DDHAL_DRIVER_HANDLED;
	}
	CheckFifo(ppdev);

	if(psopd->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC
		== FOURCC_YV12)
	{

		if(_FF(dwOVLSurfStatus) & VI_USE_SURF2 )
		{
			pSurfaceData = _DD(yv12Surface2);
		}
		else
		{
			pSurfaceData = _DD(yv12Surface1);
		}

	}
	//Primary surface/desktop width and height

	wDesktopW = (WORD)psopd->lpDDDestSurface->lpGbl->wWidth;
	wDesktopH = (WORD)psopd->lpDDDestSurface->lpGbl->wHeight;

	lDstLeft = psopd->lXPos;
	lDstTop  = psopd->lYPos;

	//Save the new overlay coordinates in video destination rectangle struct
	_DD(ovlDest).right = lDstLeft+
					_DD(ovlDest).right -
					_DD(ovlDest).left;
	_DD(ovlDest).bottom= lDstTop +
					_DD(ovlDest).bottom -
					_DD(ovlDest).top;

	_DD(ovlDest).left  = psopd->lXPos;
	_DD(ovlDest).top   = psopd->lYPos;

	//if left >= right or top >= bottom , don't show overlay

	if( (_DD(ovlDest).left >= _DD(ovlDest).right ) ||
	  (_DD(ovlDest).top >= _DD(ovlDest).bottom ) ||
	  (_DD(ovlDest).left >= wDesktopW) ||
	  (_DD(ovlDest).top >= wDesktopH) ||
	  (_DD(ovlDest).right <= 0 ) ||
	  (_DD(ovlDest).bottom <= 0 ) )

	{
	    //disable overlay
    	DisableOverlay(ppdev);
        UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE
            | SST_VO_PS_UPDATE | SST_VO_PC_UPDATE);
  
	}
	else
	{
	  DWORD dwCfg5, dwCfg7;
	  long lDstRight;
	  DWORD dwSrcWidth;

	  dwCfg5 = GET( ghwVD->vdVoPvCfg5);
	  dwCfg7 = GET( ghwVD->vdVoPvCfg7);

	  lDstRight = min( wDesktopW, _DD(ovlDest).right);

	  if( (_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK) && (_DD(dwXScale) > 0x10000))
	  {
		 dwSrcWidth = lDstRight - lDstLeft;     //vi shrink
	  }
	  else
		  dwSrcWidth = (lDstRight - lDstLeft) *
				  (_DD(ovlSrc).right - _DD(ovlSrc).left)/
				  (_DD(ovlDest).right - _DD(ovlDest).left);

	  if(dwSrcWidth > 0)
		  VD_SET_FIELD( dwCfg5, SST_VO_PV_ON_DUR,
	   (((dwSrcWidth  + _DD(ovlSrc).left)* pSurfaceData->dwBytesPerPixel + 30 ) >> 5)
			- ((_DD(ovlSrc).left * pSurfaceData->dwBytesPerPixel) >> 5));
	  else
		  VD_SET_FIELD( dwCfg5, SST_VO_PV_ON_DUR, 1);

	  VD_SET_FIELD( dwCfg7, SST_VO_PV_H_START, lDstLeft );
	  VD_SET_FIELD( dwCfg7, SST_VO_PV_V_START, lDstTop );

	  WAITFORUPDATE(SST_VO_PV_UPDATE);

	  //set the overlay address
	  SETDW( ghwVD->vdVoPvCfg5, dwCfg5);    
	  SETDW( ghwVD->vdVoPvCfg7, dwCfg7);

	  UPDATEREGISTER(SST_VO_PV_UPDATE );
	//      UpdateRegisters(ppdev, FALSE,FALSE, FALSE);
	}

	#ifdef USE_CSIM
	CSIM_Update(ppdev);
	#endif
	psopd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

/*
----------------------------------------------------------------------------
  Updateverlay32()
----------------------------------------------------------------------------
*/
DWORD __stdcall UpdateOverlay32( LPDDHAL_UPDATEOVERLAYDATA puod )
{

  // can't use PDEV_DECL here since CMDFIFO_PROLOG needs ppdev initialized
#if defined(GBLDATA_IN_PDEV)
    PDEV  *ppdev = puod->lpDD->dwReserved3;
#endif
    RECTL rVidSrc, rVidDst;
    DWORD overlayAddr;
    DWORD dwByteCount;
    FXSURFACEDATA * pSurfaceData;
    BOOL fDesktopUpdate = FALSE;
    BOOL fVPortSurface = FALSE;

#if !defined(GBLDATA_IN_PDEV)
    DD_ENTRY_SETUP(puod->lpDD);
#endif

    #ifdef FXTRACE
    DISPDBG((ppdev, 0, "UpdateOverlay32" ));
    #endif

    if((pSurfaceData = GET_SURF_DATA(puod->lpDDSrcSurface)) == 0)
    {
    
      puod->ddRVal = DDERR_NOTAOVERLAYSURFACE;
      return DDHAL_DRIVER_HANDLED;
    }

    overlayAddr = pSurfaceData->hwPtr;

    //Hide overlay
    if(puod->dwFlags & DDOVER_HIDE)
    {
       if(_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)
       {
            //if VI is used last time turn it off
            SETDW( ghwVD->vdViCfg0, GET(ghwVD->vdViCfg0) & ~SST_VI_EN);
			SETDW( ghwVD->vdVipVideoFifoCntl, 
					GET(ghwVD->vdVipVideoFifoCntl) & 
					~(SST_VD_INTERNAL_INPUT << SST_VI_VV_IP_MUX_SEL_SHIFT));
            SETDW( ghwVD->vdViStatus, SST_VI_UPDATE);
       }
       _FF(dwOVLSurfStatus) &= ~VI_USED_FOR_SHRINK;
       _DD(visibleOverlaySurf) = 0;

       DisableOverlay(ppdev );
       UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE
            | SST_VO_PS_UPDATE | SST_VO_PC_UPDATE);
  
#ifdef USE_CSIM
        CSIM_Update(ppdev);
#endif

       puod->ddRVal = DD_OK;
       return DDHAL_DRIVER_HANDLED;
    }
    else if ((DDOVER_SHOW & puod->dwFlags) ||
             (_DD(visibleOverlaySurf) == overlayAddr))
    {

      OVL_REG sOVLRegs;
      WORD  wDesktopW, wDesktopH;
      int   iSrcWidth, iSrcHeight, iDstWidth ,iDstHeight;
      DWORD dwXScale, dwYScale;
      DWORD dwStride, dwOn_Dur, dwSeed, dwSurfaceHeight;
      DWORD dwDstKeyHigh, dwDstKeyLow, dwSrcKeyHigh, dwSrcKeyLow;
      DWORD dwZoomAdj;
      //Show overlay
      //
    
      if( _DD(visibleOverlaySurf) == 0 )
      {
        _DD(visibleOverlaySurf) = overlayAddr;
      }
      else if( _DD(visibleOverlaySurf) != overlayAddr)
      {
        puod->ddRVal = DD_OK;
        return DDHAL_DRIVER_NOTHANDLED;
      }

      if(((puod->dwFlags & DDOVER_CKEY) && (puod->dwFlags & DDOVER_ALPHA))
         ||((puod->dwFlags & (DDOVER_ALPHADEST |DDOVER_ALPHADESTCONSTOVERRIDE ))
            &&(puod->dwFlags & (DDOVER_ALPHASRC | DDOVER_ALPHASRCCONSTOVERRIDE)) ))
      {
        //cannot support color key and alpha  or 
        //source alpha and dest alpha at the same time
        puod->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
      }

      
      CheckFifo(ppdev);
      memset(&sOVLRegs, 0, sizeof(OVL_REG));

      // use 512 CLUT
      sOVLRegs.dwCfg0= SST_VD_512X10_LUT<< SST_VO_PV_LUT_SEL_SHIFT;

      //Overlay pixel format
     
      if(puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC
            == FOURCC_YV12)
          dwByteCount = 2;
      else
          dwByteCount = pSurfaceData->dwBytesPerPixel;

    
      if((pSurfaceData->dwPixelFormat ==SST_VD_YUV_UYVY) ||
         (pSurfaceData->dwPixelFormat ==SST_VD_YUV_YUYV) ||
         (pSurfaceData->dwPixelFormat ==SST_VD_YUVA))

      {
               sOVLRegs.dwCfg0 |= SST_VO_PV_CSC_CTL;
      }

      if((dwByteCount == 4) ||(pSurfaceData->dwPixelFormat ==SST_VD_RGB1555))
      {
           //Disable alpha channel at first
           SETDW(ghwVD->vdVoPvAlpha0, (5 << SST_VO_PV_ALPHA_BITS_SHIFT) |
                         SST_VO_PV_ALPHA_INV |  //always invert alpha
                         (SST_VD_ALPHA_VALUE << SST_VO_PD_ALPHA_CREATE_SHIFT));

           SETDW(ghwVD->vdVoPvAlpha1,0);
      }
      //This rectangle represents a region on the source surface to be overlaid on destination
      rVidSrc = puod->rSrc;

      //This rectangle represents a region on the destination surface where overlay is mapped on
      rVidDst   = puod->rDest;
      if((pSurfaceData->dwPixelFormat == SST_VD_YUV_UYVY) ||
          (pSurfaceData->dwPixelFormat == SST_VD_YUV_YUYV))
      {
        //left and right must aligned at DWORD
        rVidSrc.right = rVidSrc.right & ~1;
        rVidSrc.left  = (rVidSrc.left + 1) & ~1; 
      }

      _DD(ovlSrc) = rVidSrc;
      _DD(ovlDest) = rVidDst;
      iSrcWidth  = rVidSrc.right - rVidSrc.left;
      iSrcHeight = rVidSrc.bottom - rVidSrc.top;
      iDstWidth  = rVidDst.right - rVidDst.left;
      iDstHeight = rVidDst.bottom - rVidDst.top;

      DISPDBG((ppdev, 0, "SrcHeight  %d, DstHeight=%d",
		  iSrcHeight, iDstHeight ));
      //See the overlay can be displayed
      if(( iDstWidth <= 0 ) ||
           (iDstHeight <= 0) ||
           (iSrcWidth <= 0) || (iSrcHeight <= 0))
      {
        //disable overlay
        DisableOverlay(ppdev);
        UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE
            | SST_VO_PS_UPDATE | SST_VO_PC_UPDATE);
  
        puod->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;

      }

      if(puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC
            == FOURCC_YV12)
      {
            if(pSurfaceData->dwFlags & SURFACE_UPDATE)
            {
               ConvertYV12(ppdev, pSurfaceData,
                 (DWORD) puod->lpDDSrcSurface->lpGbl->wWidth,
                 (DWORD) puod->lpDDSrcSurface->lpGbl->wHeight);
			   pSurfaceData->dwFlags &= ~ SURFACE_UPDATE;
            }

            if(_FF(dwOVLSurfStatus) & VI_USE_SURF2 )
            {
                pSurfaceData = _DD(yv12Surface2);
            }
            else
            {
                pSurfaceData = _DD(yv12Surface1);
           }
           overlayAddr = pSurfaceData->hwPtr;
           
      }


      //Primary surface/desktop width and height
      wDesktopW = (WORD)puod->lpDDDestSurface->lpGbl->wWidth;
      wDesktopH = (WORD)puod->lpDDDestSurface->lpGbl->wHeight;

      dwZoomAdj = 0;

      if(( iSrcWidth > 1) &&
         (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC != FOURCC_YV12) &&
        ( ((pSurfaceData->dwOvlHOffset + rVidSrc.right) * dwByteCount + overlayAddr) & 0x1F))
      {
        //if zoom up and right edge is not at 32 byte boundary
        //change zoom factor -- fix a HW bug
        
        if((pSurfaceData->dwPixelFormat == SST_VD_YUV_UYVY) ||
          (pSurfaceData->dwPixelFormat == SST_VD_YUV_YUYV))
            dwZoomAdj = 1;
        else if( iSrcWidth < iDstWidth)
            dwZoomAdj = 1;

      }

      dwXScale = (((DWORD)(iSrcWidth) - dwZoomAdj) << 16 ) /
                  (DWORD)(iDstWidth);

      dwYScale = ((DWORD)(iSrcHeight) << 16 ) /
                  (DWORD)(iDstHeight);
                             
                                    
    #if ENABLE_VIDEOPORT
      if( (_DD(dwVPEFlags)& VPE_ON ) &&
          ((puod->lpDDDestSurface == _DD(vpeSurf)) ||
          ( puod->dwFlags && DDOVER_AUTOFLIP)))

      {
         fVPortSurface = TRUE;
          sOVLRegs.dwCfg0 |=  SST_VD_YUV_UYVY <<SST_VO_PV_FORMAT_SHIFT;
      }
      else
    #endif
      if((!(_DD(dwVPEFlags)& VPE_ON ) && _DD(shrinkSurface1) &&
           ((pSurfaceData->dwPixelFormat == SST_VD_YUV_UYVY) ||
            (pSurfaceData->dwPixelFormat == SST_VD_YUV_YUYV)) && 
            ((dwXScale > 0x10000) || (dwYScale > 0x10000))) ||
			((_DD(dwOVLFlags) & DXVA_ON) &&
			 (puod->dwFlags & DDOVER_BOB) &&
             (puod->dwFlags & DDOVER_INTERLEAVED)))
      {
        _FF(dwOVLSurfStatus) |= VI_USED_FOR_SHRINK;

        sOVLRegs.dwCfg0 |=  SST_VD_YUV_UYVY <<SST_VO_PV_FORMAT_SHIFT;
          //vi always output UYVY
        
      }
      else 
      {
         
          if(_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)
          {
            //if VI is used last time turn it off
            SETDW( ghwVD->vdViCfg0, GET(ghwVD->vdViCfg0) & ~SST_VI_EN);
     		SETDW( ghwVD->vdVipVideoFifoCntl, 
					GET(ghwVD->vdVipVideoFifoCntl) & 
					~(SST_VD_INTERNAL_INPUT << SST_VI_VV_IP_MUX_SEL_SHIFT));
            SETDW( ghwVD->vdViStatus, SST_VI_UPDATE);
          }
          VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_FORMAT,
                    pSurfaceData->dwPixelFormat);
          _FF(dwOVLSurfStatus) &= ~VI_USED_FOR_SHRINK;
       }

  

      Msg(ppdev, 0, "OVL Addr=%x",overlayAddr );

      _DD(dwXScale) = dwXScale;
      _DD(dwYScale) = dwYScale;


      //Check BandWidth
      _DD(sBWConfig).sOverlay.dwFlags |=
                    _DD(dwOVLFlags) | OVL_ON;

      //check to see we can use vi for shrink
      if(_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)
      {

         if(dwXScale > 0x10000)
            dwXScale = 0x10000;

         if(dwYScale > 0x10000)
            dwYScale = 0x10000;

      }

      _DD(sBWConfig).sOverlay.dwScaleX = dwXScale;
      _DD(sBWConfig).sOverlay.dwScaleY = dwYScale;
      _DD(sBWConfig).sOverlay.dwByteCount = dwByteCount;

      FillDesktopInfo(ppdev, &(_DD(sBWConfig)) );

      //Check vertical filter setting, which can increas BW
      //set filters
      _DD(sBWConfig).sOverlay.dwFlags &=
                            ~(BW_VIDEOPORT|BW_UNDITHER|BW_THREETAP|BW_TWOTAP);
      dwSeed = 0;

      if( dwByteCount != 1)   //we don't set virtical filer in 8bit mode
      {
           //Vertical filer first
           if(dwYScale < 0x10000 )
           {
              //zoom up case

                if((puod->dwFlags & DDOVER_INTERLEAVED ) &&
                    !(puod->dwFlags & DDOVER_BOB))
                {
                    //display a frame with two fields 

                    //single 3 tap - De-flicker
                    //we can use single 4 tap half-pxiel interp,
                    //but it may cost too much BW.

                    _DD(sBWConfig).sOverlay.dwFlags |=
                                                        BW_THREETAP;
                    VD_SET_FIELD(sOVLRegs.dwCfg0,
                        SST_VO_PV_VFIL_MODE, SST_VD_3TAP);
                }
                else
                {
                    //use vertical zooming 
                    //4 tap filter
                    _DD(sBWConfig).sOverlay.dwFlags |= BW_TWOTAP;

                    if(fVPortSurface)
                    {
                        _DD(sBWConfig).sOverlay.dwFlags
                            |= BW_VIDEOPORT;

                        VD_SET_FIELD(sOVLRegs.dwCfg0,
                            SST_VO_PV_VFIL_MODE, SST_VD_DUAL_2TAP);

                    }
                    else
                    {
                        VD_SET_FIELD(sOVLRegs.dwCfg0,
                            SST_VO_PV_VFIL_MODE, SST_VD_4TAP);

                    }

                } 
            }
            else if(dwYScale > 0x10000)
            {
                 //zoom down case
                 
                 dwSeed = dwYScale >> 17;
                 //dual 3 tap  filter for vertical scale
                 VD_SET_FIELD(sOVLRegs.dwCfg0,
                        SST_VO_PV_VFIL_MODE, SST_VD_DUAL_3TAP);

            }
            else    //1:1 case
            {
//                //if the color is RGB5:6:5 ,using undither filter
//                if((pSurfaceData->dwPixelFormat == SST_VD_RGB565) ||
//                   (pSurfaceData->dwPixelFormat == SST_VD_RGB1555))
//                {
//                     sOVLRegs.dwCfg0 |= SST_VO_PV_UD_EN | SST_VO_PV_UD_FIL_EN;
//                }

                if( fVPortSurface)
                {
                    _DD(sBWConfig).sOverlay.dwFlags
                                    |= BW_VIDEOPORT;
                            
                   VD_SET_FIELD(sOVLRegs.dwCfg0,
                          SST_VO_PV_VFIL_MODE, SST_VD_2TAP_TEMPORAL);  //temporay filter
                }
                else
                   VD_SET_FIELD(sOVLRegs.dwCfg0,
                        SST_VO_PV_VFIL_MODE, SST_VD_BYPASS);


            }
      }
  

#ifndef USE_CSIM
      if(!EnoughBandWidth( &(_DD(sBWConfig))) )
      {

        _DD(sBWConfig).sOverlay.dwFlags &= ~OVL_ON;
         puod->ddRVal = DDERR_OUTOFCAPS;
         return DDHAL_DRIVER_HANDLED;
      }
#endif

      if (puod->dwFlags & (DDOVER_KEYDEST | DDOVER_KEYDESTOVERRIDE))
      {

        if (puod->dwFlags & DDOVER_KEYDEST)
        {
            dwDstKeyLow =
                puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;

            dwDstKeyHigh =
                puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceHighValue;

        }
        else
        {
            dwDstKeyLow =
                puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue;

            dwDstKeyHigh =
                puod->overlayFX.dckDestColorkey.dwColorSpaceHighValue;

        }
        _DD(dwOVLFlags) |= DEST_CKEY;  //destination color key

        fDesktopUpdate = TRUE;

      }
      else if ( _DD(dwOVLFlags) & DEST_CKEY)
      {
             DisableAlpha(ppdev, DEST_CKEY);
             fDesktopUpdate = TRUE;
            
      }

      if (puod->dwFlags & (DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE))
      {
        
         if (puod->dwFlags & DDOVER_KEYSRC)
         {

            dwSrcKeyLow =
                puod->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
            dwSrcKeyHigh =
                puod->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceHighValue;

         }
         else
         {

            dwSrcKeyLow =
                puod->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
            dwSrcKeyHigh =
                puod->overlayFX.dckSrcColorkey.dwColorSpaceHighValue;
 
         }

         _DD(dwOVLFlags) |= SRC_CKEY;  //source color key

      }
      else if ( _DD(dwOVLFlags) & SRC_CKEY)
      {
            DisableAlpha(ppdev, SRC_CKEY);
      }

	  if(_DD(dwOVLFlags) & (DEST_CKEY | SRC_CKEY))
          SetUpColorKey(ppdev, dwDstKeyLow, dwDstKeyHigh, 
          					   dwSrcKeyLow, dwSrcKeyHigh,
                               pSurfaceData->dwPixelFormat);

      if (puod->dwFlags & (DDOVER_ALPHADEST |DDOVER_ALPHADESTCONSTOVERRIDE))
      {

         _DD(dwOVLFlags) &= ~SRC_ALPHA;  //clear src alpha
         _DD(dwOVLFlags) |= DEST_ALPHA;  //destination alpha
         if (puod->dwFlags & DDOVER_ALPHADESTCONSTOVERRIDE)
         {
    #ifdef FXTRACE
    DISPDBG((ppdev, 0, "alpha1= %lx, alpha2=%lx",
        puod->overlayFX.dwAlphaDestConst,
        puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue));
    #endif
             if(puod->dwFlags & DDOVER_ALPHADESTNEG)
                SetUpAlpha(ppdev,puod->overlayFX.dwAlphaDestConst,
                    puod->overlayFX.dwAlphaDestConstBitDepth,FALSE, TRUE);
            else
               SetUpAlpha(ppdev,puod->overlayFX.dwAlphaDestConst,
                    puod->overlayFX.dwAlphaDestConstBitDepth,FALSE, FALSE);
         }
         else
         {
            //alpha value is in pixel
            if(GETPRIMARYBYTEDEPTH == 4)
            {
                if(puod->dwFlags & DDOVER_ALPHADESTNEG)
                  SetUpAlpha(ppdev,0,6,TRUE, TRUE);
                else
                  SetUpAlpha(ppdev,0,6,TRUE, FALSE);
            }
            else
            {
                puod->ddRVal = DDERR_UNSUPPORTED;
                return DDHAL_DRIVER_HANDLED;
            }
          }
         fDesktopUpdate = TRUE;
      }
      else if( _DD(dwOVLFlags) & DEST_ALPHA)
      {
        //disable destination alpha
         DisableAlpha(ppdev,0);
         fDesktopUpdate = TRUE;

      }

      if (puod->dwFlags & (DDOVER_ALPHASRC |DDOVER_ALPHASRCCONSTOVERRIDE))
      {

         _DD(dwOVLFlags) &= ~DEST_ALPHA;//clear destination alpha
         _DD(dwOVLFlags) |= SRC_ALPHA;  //source alpha
         if(puod->dwFlags & DDOVER_ALPHASRCCONSTOVERRIDE)
         {

             if(puod->dwFlags & DDOVER_ALPHASRCNEG)
                 SetUpAlpha(ppdev,puod->overlayFX.dwAlphaSrcConst,
                    puod->overlayFX.dwAlphaSrcConstBitDepth,FALSE, TRUE);
              else
                 SetUpAlpha(ppdev,puod->overlayFX.dwAlphaSrcConst,
                puod->overlayFX.dwAlphaSrcConstBitDepth,FALSE, FALSE);
         }
         else
         {
            if((pSurfaceData->dwPixelFormat == SST_VD_RGB32) ||
                (pSurfaceData->dwPixelFormat == SST_VD_YUVA) ||
                (pSurfaceData->dwPixelFormat == SST_VD_RGB1555) )
            {
                 if(puod->dwFlags & DDOVER_ALPHASRCNEG)
                    SetUpAlpha(ppdev,0,6,TRUE, TRUE);
                else
                    SetUpAlpha(ppdev,0,6,TRUE, FALSE);

            }
            else
            {
                puod->ddRVal = DDERR_UNSUPPORTED;
                return DDHAL_DRIVER_HANDLED;
            }
          }

      }
      else if( _DD(dwOVLFlags) & SRC_ALPHA)
      {
        //disable source alpha
         DisableAlpha(ppdev,0);

      }


      if(_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)
      {
        //vi is used
        SetUpViForShrink(ppdev,pSurfaceData, 0);  //check it again when vi shrink is fixed
			//,puod->dwFlags);

        StartViShrink(ppdev, pSurfaceData,
         (puod->dwFlags & DDOVER_INTERLEAVED )? DDOVER_INTERLEAVED : 0);

        pSurfaceData = _DD(shrinkSurface1);
       
        rVidSrc.right -= rVidSrc.left;
        rVidSrc.bottom -= rVidSrc.top;
        rVidSrc.top = rVidSrc.left = 0;
        if(dwXScale == 0x10000)
            rVidSrc.right = iDstWidth;
        if(dwYScale == 0x10000)
            rVidSrc.bottom = iDstHeight;
        iSrcWidth  = rVidSrc.right - rVidSrc.left;
        iSrcHeight = rVidSrc.bottom - rVidSrc.top;

        dwStride = pSurfaceData->dwStride;
		if(_DD(dwOVLFlags) & DXVA_ON)
			dwSurfaceHeight = iSrcHeight;
		else
			dwSurfaceHeight = iDstHeight;


      }
      else
        dwSurfaceHeight = puod->lpDDSrcSurface->lpGbl->wHeight;

      if(!fVPortSurface && (puod->dwFlags & DDOVER_BOB) &&
                    (puod->dwFlags & DDOVER_INTERLEAVED)) 
      {
          dwStride = pSurfaceData->dwStride << 1;
          dwYScale >>= 1 ;          //double y scale
		  dwSurfaceHeight >>=1;
      }
      else
        dwStride = pSurfaceData->dwStride;

      if( iSrcWidth > 1)
      {
          dwOn_Dur = ((((((iSrcWidth  + rVidSrc.left + pSurfaceData->dwOvlHOffset) << 8)* dwByteCount  -0x100) >> 8) + 31 ) >> 5)
            - ((rVidSrc.left * dwByteCount) >> 5);
                
      }  
      else
          dwOn_Dur = 1;

      Msg(ppdev, 0, "SrcWidth= %ld, Srcleft=%ld, dur=%ld. extraH=%ld",
         iSrcWidth,rVidSrc.left, dwOn_Dur,pSurfaceData->dwOvlHOffset);

      if((pSurfaceData->tileFlag & MEM_IN_TILE0))
      {
          sOVLRegs.dwAddrCtl =
            (dwStride >>5 ) <<SST_VO_PV_STRIDE_SHIFT;

          sOVLRegs.dwAddrCtl |= SST_VD_TILE_MODE_0 << SST_VO_PV_TILE_SHIFT;
      }
      else if((pSurfaceData->tileFlag & MEM_IN_TILE1))
      {
           sOVLRegs.dwAddrCtl =
            (dwStride >>5) <<SST_VO_PV_STRIDE_SHIFT;

           sOVLRegs.dwAddrCtl |= SST_VD_TILE_MODE_1 << SST_VO_PV_TILE_SHIFT;

      }
      else
         sOVLRegs.dwAddrCtl = dwStride  <<SST_VO_PV_STRIDE_SHIFT;

      VD_SET_FIELD( sOVLRegs.dwCfg2, SST_VO_PV_TARGET_WIDTH,
                                    rVidDst.right - rVidDst.left );
      VD_SET_FIELD( sOVLRegs.dwCfg2, SST_VO_PV_TARGET_HEIGHT,
                                    rVidDst.bottom - rVidDst.top );
      VD_SET_FIELD( sOVLRegs.dwCfg3, SST_VO_PV_V_OFS,
                                    (rVidSrc.top + dwSeed) << 8);
      VD_SET_FIELD( sOVLRegs.dwCfg5, SST_VO_PV_ON_DUR,dwOn_Dur);

      VD_SET_FIELD( sOVLRegs.dwCfg4, SST_VO_PV_V_INC, dwYScale );
      if( dwSurfaceHeight)
       VD_SET_FIELD( sOVLRegs.dwCfg4, SST_VO_PV_SOURCE_HEIGHT,
        dwSurfaceHeight -1 );
      else
       VD_SET_FIELD( sOVLRegs.dwCfg4, SST_VO_PV_SOURCE_HEIGHT, 0);

      VD_SET_FIELD( sOVLRegs.dwCfg6, SST_VO_PV_H_INC, dwXScale );

      //Overlay screen top, left
      rVidDst.left = min(rVidDst.left, wDesktopW);
      rVidDst.top  = min(rVidDst.top, wDesktopH);

      VD_SET_FIELD( sOVLRegs.dwCfg7, SST_VO_PV_H_START, rVidDst.left );
      VD_SET_FIELD( sOVLRegs.dwCfg7, SST_VO_PV_V_START, rVidDst.top );

      if(puod->dwFlags & DDOVER_BOB )
      {
         _DD(dwOVLFlags) |= OVL_BOB;
      }
      else
        _DD(dwOVLFlags) &= ~OVL_BOB;

#if ENABLE_VIDEOPORT
      if(  fVPortSurface )
      {
        
         VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_VI_SWAP); //use address in VI
		 // since we always de-interlace, so don't care for BOB or WEAVE mode		
      }
      else
#endif
      {
        
        if(_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK)
            VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_VI_SWAP); //use address in VI
        else
            VD_SET_FIELD( sOVLRegs.dwCfg0, SST_VO_PV_SWAP_CTL,SST_VD_CPU_SWAP); //use address in VI
        sOVLRegs.dwBaseLeft = overlayAddr;
      }

      //Chose horizontal filter
      if( dwXScale > 0x10000)
      {
            DWORD dwLog2;
            //zoom down case

            dwSeed = dwXScale >> 17;

            //Find zoom down factor
            if( dwXScale >= 0x40000)
                dwLog2 = 0x1F;
            else
            {
              //find 3log2 dwXScale

              dwLog2 = 1;
              while( (dwXScale >>= 1) > 0x10000)
                dwLog2 += 1;

              dwLog2 <<= 3;
            }

            VD_SET_FIELD(sOVLRegs.dwCfg0, SST_VO_PV_HFIL_CTL,dwLog2 );

      }
      else
      {
            VD_SET_FIELD(sOVLRegs.dwCfg0, SST_VO_PV_HFIL_CTL, 0 );
            dwSeed = 0;
      }
      VD_SET_FIELD( sOVLRegs.dwCfg5, SST_VO_PV_H_OFS,
         (rVidSrc.left + pSurfaceData->dwOvlHOffset+ dwSeed) << 8);
      SetOverlayRegs(ppdev, &sOVLRegs);

      EnableOverlay(ppdev);

      if((_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK) || fVPortSurface)
		   WAITFORUPDATE(SST_VO_PV_UPDATE | SST_VO_PS_UPDATE)
      else
		   WAITFORUPDATE(SST_VO_PV_UPDATE | 
					SST_VO_PV_BASE_UPDATE |SST_VO_PS_UPDATE);

	  
      if((_FF(dwOVLSurfStatus) & VI_USED_FOR_SHRINK) || fVPortSurface)
      {
         if (fDesktopUpdate)
         {
            UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE |
               SST_VO_PS_UPDATE | SST_VO_PC_UPDATE );
         }
         else
         {  
            UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PS_UPDATE);
         }

         if( !fVPortSurface)
         {  
            TRIGGER_VI;
            WAITFORTRIGGER;
         }
      }
      else  if(fDesktopUpdate)
      {  
        UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PD_UPDATE |
            SST_VO_PS_UPDATE | SST_VO_PV_BASE_UPDATE | SST_VO_PC_UPDATE);
//          UpdateRegisters(ppdev, TRUE,TRUE,TRUE);
      }
      else
//          UpdateRegisters(ppdev, FALSE,TRUE, TRUE);
         UPDATEREGISTER(SST_VO_PV_UPDATE | SST_VO_PS_UPDATE |
                SST_VO_PV_BASE_UPDATE);


    }

#ifdef USE_CSIM
    CSIM_Update(ppdev);
#endif

    puod->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

/*
----------------------------------------------------------------------------
  TMULoadY()
  loads Y component into TMU
----------------------------------------------------------------------------
*/

void TMULoadY( FxU8 tmuNum, TMURegs *sst )
{
    //load current texture

	//filter out R G part only leave B 
	//set constant color to filter out  R and G
	sst->TMU[tmuNum].taColorAR0 = 0;
	sst->TMU[tmuNum].taColorGB0 = 0x100 << SST_TA_CONSTANT_COLOR0_BLUE_SHIFT;

	// for YUYV
    // a = Y0  Y0  Y0
    // b = 0  0  0
    // c=  0  0  1
    // d=  0  0  0
  //output 0  0  Y0

   TexCombineColorExt( tmuNum,
                         SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//load alpha   with Y1
	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);


	//filter out G B part only leave R 
	//set constant color to filter out  G and B
	sst->TMU[tmuNum].taColorAR1 = 0x100 << SST_TA_CONSTANT_COLOR0_RED_SHIFT;
	sst->TMU[tmuNum].taColorGB1 = 0;

    // a = Y1  Y1  Y1
    // b = 0  0   0
    // c=  1  0   0
    // d=  0  0   Y0
  //output Y1  0  Y0

	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ATEX,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_C1,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //zero alpha
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

}

/*
----------------------------------------------------------------------------
  TMULoadV()
  loads V component into TMU
----------------------------------------------------------------------------
*/
void TMULoadV( FxU8 tmuNum, TMURegs *sst )
{
   //load current texture

	// for YUYV case                            
	// a =   V  0   0  0
	// b =   0  0   0  0
	// c=    0  0   0  0
	// d =  0   Y1  0  Y0 
	//output V Y1  0  Y0    

	TexCombineColorExt( tmuNum,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_CPREV,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);


  	//pass TCU
  
	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

}

/*
----------------------------------------------------------------------------
  TMULoadU()
  loads U component into TMU
----------------------------------------------------------------------------
*/

void TMULoadU( FxU8 tmuNum ,TMURegs *sst)
{
   //load current texture

    //set constant color to filter out  R B
	sst->TMU[tmuNum].taColorAR0 = 0;
    sst->TMU[tmuNum].taColorGB0 = 0x100 << SST_TA_CONSTANT_COLOR0_GREEN_SHIFT;

	// for YUYV case                            
	// a =   U  U   U  U
	// b =   0  0   0  0
	// c=    0  0   1  0
	// d =   V  Y1  U   Y0
	//output V  Y1  U   Y0   

	TexCombineColorExt( tmuNum,
	                  SST_TA_TCC_CTEX,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_C0,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_CPREV,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

  	//pass TCU
  
	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

}


/*
----------------------------------------------------------------------------
  ConvertYV12()
  convert YV12 into YUYV format
----------------------------------------------------------------------------
*/

VOID ConvertYV12(NT9XDEVICEDATA *ppdev,FXSURFACEDATA *srcSurface,
    DWORD width, DWORD  height)
{
  FXSURFACEDATA * dstSurface;
  DWORD dwSrcAddress, dwDstAddress, dwStride;
  DWORD yAddr, uAddr,vAddr;
  volatile DWORD dwStatus;
  GrTexNPTInfoExt     yTexture, uTexture, vTexture;
  Vertex vtxA, vtxB, vtxC, vtxD;
  TMURegs  sstReg;
#ifdef CMDFIFO
  CMDFIFO_PROLOG(cmdFifo);
#else
  FxU32 *cmdFifo;
  FxU32  hwIndex;
#endif
  HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);

   memset( &sstReg, 0, sizeof( TMURegs));

   if(_FF(dwOVLSurfStatus) & YV12_USE_TWO_BUFF)
   {
       _FF(dwOVLSurfStatus) ^=YV12_USE_SURF2;       //change current surface
   }

   if(_FF(dwOVLSurfStatus) & YV12_USE_SURF2 )
   {
      dstSurface = _DD(yv12Surface2);
   }
   else
   {
      dstSurface = _DD(yv12Surface1);
   }

   dwSrcAddress = srcSurface->hwPtr;
   dwDstAddress = dstSurface->hwPtr;
   
   dwStride = srcSurface->dwStride;

   yAddr = dwSrcAddress;
   vAddr = yAddr + height * dwStride;
   uAddr = vAddr + height * dwStride / 4;

   yTexture.format          = SST_TA_AI88;
   yTexture.maxS            = width /2 ; //two Ys in one pixel 
   yTexture.maxT            = height;
   yTexture.baseAddr        = yAddr;
   yTexture.nptStride       = dwStride;
   yTexture.bFilter         = 0;

   SetNPTSourceExt(2, &yTexture,srcSurface->tileFlag, &sstReg); //Y in TMU2

   vTexture.format          = SST_TA_A8;
   vTexture.maxS            = width /2;
   vTexture.maxT            = height /2;
   vTexture.baseAddr        = vAddr;
   vTexture.nptStride       = dwStride/2;
   vTexture.bFilter         = 1;

   SetNPTSourceExt(1, &vTexture,srcSurface->tileFlag, &sstReg); //V in TMU1
 
   uTexture.format          = SST_TA_A8;
   uTexture.maxS            = width /2 ;
   uTexture.maxT            = height /2;
   uTexture.baseAddr        = uAddr;
   uTexture.nptStride       = dwStride/2;
   uTexture.bFilter         = 1;

   SetNPTSourceExt(0, &uTexture,srcSurface->tileFlag, &sstReg); //U in TMU0



    /* 3D Transformations */
    /*---- 
      A-B
      |\|
      C-D
      -----*/
    vtxA.w = 1.0f;
    vtxB = vtxC = vtxD = vtxA;

    vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = 0.0f;
    vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(width /2);
    vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = T_OFFSET;
    vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(height/2) + T_OFFSET;

    vtxA.tmuvtx[1].s = vtxC.tmuvtx[1].s = 0.0f;
    vtxB.tmuvtx[1].s = vtxD.tmuvtx[1].s = (float)(width /2 );
    vtxA.tmuvtx[1].t = vtxB.tmuvtx[1].t = T_OFFSET;
    vtxC.tmuvtx[1].t = vtxD.tmuvtx[1].t = (float)(height /2) + T_OFFSET;

    vtxA.tmuvtx[2].s = vtxC.tmuvtx[2].s = 0.0f;
    vtxB.tmuvtx[2].s = vtxD.tmuvtx[2].s = (float)(width /2);
    vtxA.tmuvtx[2].t = vtxB.tmuvtx[2].t = 0.0f;
    vtxC.tmuvtx[2].t = vtxD.tmuvtx[2].t = (float)(height);

    vtxA.x = vtxC.x = 0.0f;
    vtxB.x = vtxD.x = (float)(width/2);
    vtxA.y = vtxB.y = 0.0f;
    vtxC.y = vtxD.y = (float)(height);

    TMULoadY(2, &sstReg);
    TMULoadV(1, &sstReg);
    TMULoadU(0, &sstReg);


//    dstSurface = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));

    //set registers
     preSetRegs( ppdev, dstSurface, height, &cmdFifo, &hwIndex);

     SetupTmus( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
     ConfigBuffer( ppdev,SST_PE_ARGB_WRMASK,4, 0,&cmdFifo, &hwIndex);
     DrawRect(ppdev, &vtxA, &vtxB, &vtxC,&vtxD ,2,&cmdFifo, &hwIndex);

     //wait for done
     do{
        dwStatus = GET(ghwIO->status);
    }
     while( dwStatus & (SST2_BUSY) );
 
   	HW_ACCESS_EXIT(ACCESS_3D);

	CMDFIFO_EPILOG( cmdFifo );  
}
