/* -*-c++-*- */
/* $Header: ddsli.c, 12, 12/7/00 9:43:32 AM PST, Ryan Bissell$ */
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
** File Name: 	ddsli.C
**
** Description: Group the SLI routines in here to be easy to find
**
** $Revision: 12$
** $Date: 12/7/00 9:43:32 AM PST$
**
** $History: ddsli.c $
** 
** *****************  Version 1  *****************
** User: Andrew       Date: 9/07/99    Time: 10:39a
** Created in $/devel/sst2/Win95/dx/dd32
** SLI code
**
*/

#include "precomp.h"
#include "ddovl32.h"
#include "fifomgr.h"
#include "sst2glob.h"
#include "sst2bwe.h"
#include "ddcam.h"

#include "regkeys.h"

#ifdef SLI
#include "ddsli.h"

/*----------------------------------------------------------------------
Function name: RetrieveFromRegistry

Description:   Common code to read a integer value from the registry

Return:        default, Min, max or registry value
----------------------------------------------------------------------*/
DWORD RetrieveFromRegistry(NT9XDEVICEDATA * ppdev, const char * pStr, DWORD dwDefault, DWORD dwMin, DWORD dwMax)
{
  DWORD dwReturn = dwDefault;
  LPSTR lpStr;

  lpStr = GETENV(pStr);
  if (NULL != lpStr)
  {
    dwReturn = ddatoi(lpStr);
    if (dwReturn < dwMin)
      dwReturn = dwMin;
    if (dwReturn > dwMax)
      dwReturn = dwMax;
  }
   
   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: GetLog2

Description:   Function to Log 2

Return:        1
----------------------------------------------------------------------*/
DWORD GetLog2(DWORD dwNum)
{
   DWORD dwReturn;

   for (dwReturn=0, dwNum>>=1; dwNum > 0; dwNum>>=1, dwReturn++)
      ;

   return dwReturn;
}

#define FILENAME "\\\\.\\H6VDD"

/*----------------------------------------------------------------------
Function name:  DoMiniVDDSLI

Description: This is used to keep the mini-VDD upto date on the IMASK

Return:         int

                0 -
----------------------------------------------------------------------*/
int DoMiniVDDSLI(NT9XDEVICEDATA * ppdev, DWORD dwType)
{
   HANDLE hDevice;
   DIOC_DATA DIOC_Data;

   hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
   if (INVALID_HANDLE_VALUE != hDevice)
      {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DIOC_Data.dwSpare = 0x0;
      DeviceIoControl(hDevice, dwType, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      }
   CloseHandle(hDevice);

   return 0;
}

/*----------------------------------------------------------------------
Function name: Enable_SLI

Description:   Function to Enable SLI

Return:        1
----------------------------------------------------------------------*/
int Enable_SLI(NT9XDEVICEDATA * ppdev)
{
   DWORD dwBandHeight;     // Height Per Chip
   DWORD dwsliBaseConfig;  // Non-unique copy of sliConfig
   DWORD dwcmdBaseConfig;  // Ditto for cmdConfig
   DWORD dwsliBaseCfg0;    // Non-unique copy of vdPsSliCfg0
   DWORD dwsliBaseCfg1;    // Non-unique copy of vdPsSliCfg1
   DWORD sliVideoMode;     // SST_VO_SLI_MODE
   DWORD dwClkOut_Phs;     // SST_VO_SO_CLKOUT_PHS
   DWORD dwPulse_Dly;      // SST_VO_SO_PULSE_DLY
   DWORD dwResync_Pixel;   // SST_VO_SLI_RESYNC_PIXEL
   DWORD dwResync_Line;    // SST_VO_SLI_RESYNC_LINE
   DWORD dwSwap;           // SST_VO_SLI_SWAP_LOCK_EN
   DWORD dwChipMask;       // Chip Mask
   DWORD i;
   DWORD dwReturn = 0x0;   // return value
   DWORD strideState;
   Sst2Regs * p3DRegs;
   DWORD dwDisableSLI;
//   Sst2CAMEntry dstCAM;
//   BYTE * pSrc;
   BYTE * pDst;
//   BYTE * pBandDst;
//   BYTE * pSaveDst;
//   DWORD dwBandByte;
//   DWORD dwUnit;
   FxU32 camEntry;
//   DWORD j;
//   DWORD dwWidth;

   dwDisableSLI = RetrieveFromRegistry(ppdev, DISABLE_SLI_NAME, DISABLE_SLI_MIN, DISABLE_SLI_DEFAULT, DISABLE_SLI_MAX);
   // Check for some stuff like 1/2 modes certain filters etc

   //Determine what Video Mode that we are going into
   if ((_FF(dwNumUnits) > 1) && (0x0 == dwDisableSLI))
      {
      switch(_FF(dwNumUnits))
         {
         case 2:
            if (0 == _FF(dwNumSlaves))
               {
               sliVideoMode = SST_VO_SLI_2CHIP_2X1WAY;
               }
            else         
               {
               sliVideoMode = SST_VO_SLI_2CHIP_2WAY;
               }
             break;

         case 4:
            if (1 == _FF(dwNumMasters))
               {
               sliVideoMode = SST_VO_SLI_4CHIP_4WAY;
               }
            else if (2 == _FF(dwNumMasters))
               {
               sliVideoMode = SST_VO_SLI_4CHIP_2X2WAY;
               }
            else
               {
               sliVideoMode = SST_VO_SLI_4CHIP_2WAY2X1WAY;
               }     

         case 8:
            if (1 == _FF(dwNumMasters))
               {
               sliVideoMode = SST_VO_SLI_8CHIP_8WAY;
               }
            else if (2 == _FF(dwNumMasters))
               {
               sliVideoMode = SST_VO_SLI_8CHIP_2X4WAY;
               }
            else
               {
               sliVideoMode = SST_VO_SLI_8CHIP_4WAY2X2WAY;
               }     
         }

      // Call Minivdd to sync Modes, Fifo, copy CAM's, duplicate cursor, etc
      DoMiniVDDSLI(ppdev, ENABLE_SLI);
      // Tell the 2D engine to start to punt
      _FF(gdiFlags) |= SDATA_GDIFLAGS_SLI_MASTER;      
      // Setup the broadcast Register

      if (_FF(dwNumSlaves))
         {
         dwBandHeight = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_NAME, SLI_BAND_HEIGHT_MIN, SLI_BAND_HEIGHT_DEFAULT, SLI_BAND_HEIGHT_MAX);

         // These are all video tweaks
         // When in doubt pass it on to the user
         // This should be resolved when the hardware comes back
         dwSwap = RetrieveFromRegistry(ppdev, SLI_SWAP_LOCK_NAME, SLI_SWAP_LOCK_MIN, SLI_SWAP_LOCK_DEFAULT, SLI_SWAP_LOCK_MAX);
         dwClkOut_Phs = RetrieveFromRegistry(ppdev, SLI_CLKOUT_PHS_NAME, SLI_CLKOUT_PHS_MIN, SLI_CLKOUT_PHS_DEFAULT, SLI_CLKOUT_PHS_MAX);
         dwPulse_Dly = RetrieveFromRegistry(ppdev, SLI_PULSE_DLY_NAME, SLI_PULSE_DLY_MIN, SLI_PULSE_DLY_DEFAULT, SLI_PULSE_DLY_MAX);
         dwResync_Pixel = RetrieveFromRegistry(ppdev, SLI_RESYNC_PIXEL_NAME, SLI_RESYNC_PIXEL_MIN, SLI_RESYNC_PIXEL_DEFAULT, SLI_RESYNC_PIXEL_MAX);
         dwResync_Line = RetrieveFromRegistry(ppdev, SLI_RESYNC_LINE_NAME, SLI_RESYNC_LINE_MIN, SLI_RESYNC_LINE_DEFAULT, SLI_RESYNC_LINE_MAX);

         
         // Config Registers only difference is Chip ID
         dwsliBaseConfig = (_DD(dwLog2NumChips) << SST_SLI_NUM_CHIPS_SHIFT) | (GetLog2(dwBandHeight) << SST_SLI_INTERLEAVE_SHIFT);
         dwcmdBaseConfig = (_DD(dwLog2NumChips) << SST_CMD_SLI_NUM_CHIPS_SHIFT) | (GetLog2(dwBandHeight) << SST_CMD_SLI_INTERLEAVE_SHIFT);

         // Video Registers only difference is Chip ID
         dwsliBaseCfg0 = (dwPulse_Dly << SST_VO_SO_PULSE_DLY_SHIFT) |
            (dwClkOut_Phs << SST_VO_SLI_CLKOUT_PHS_SHIFT) |
            (SST_VO_SLI_MUX << SST_VO_SLI_ADDMUX_MODE_SHIFT) |
            (dwSwap << SST_VO_SLI_SWAP_LOCK_EN_SHIFT) |
            (GetLog2(dwBandHeight) << SST_VO_SLI_BLOCK_SIZE_SHIFT) |
            (sliVideoMode << SST_VO_SLI_MODE_SHIFT) |
            SST_VO_SLI_EN;

         // Video Registers Same for all
         dwsliBaseCfg1 = (dwResync_Line << SST_VO_SLI_RESYNC_LINE_SHIFT) | (dwResync_Pixel << SST_VO_SLI_RESYNC_PIXEL_SHIFT);

         p3DRegs = (Sst2Regs *)(_FF(regBase) + SST_3D_OFFSET);

         // Set the count in the Mask so that the CSIM will broadcast these to the other devices         
         // Configuration Register
         SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(_FF(dwChipID)) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
         SETDW(ghwIO->sliConfig, (_FF(dwChipID) << SST_SLI_CHIP_ID_SHIFT) | dwsliBaseConfig);  
         // Need a 3D Flush Here
         SETDW(ghwAC->cmdSLIConfig, (_FF(dwChipID) << SST_CMD_SLI_RA_CHIP_ID_SHIFT) | (_FF(dwChipID) << SST_CMD_SLI_CHIP_ID_SHIFT) | dwcmdBaseConfig);

         dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
         for (i=0x0; i<8; i++)
            {
            if (dwChipMask & BIT(i))
               {
               // First Set the Broadcast Register for Each Device !!!!
               SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
               
               // Configuration Register
               SETDW(ghwIO->sliConfig, (i << SST_SLI_CHIP_ID_SHIFT) | dwsliBaseConfig);  
               // Need a 3D Flush Here
               SETDW(ghwAC->cmdSLIConfig, (i << SST_CMD_SLI_RA_CHIP_ID_SHIFT) | (i << SST_CMD_SLI_CHIP_ID_SHIFT) | dwcmdBaseConfig);

               // Hardware recommends this but it does not work well
               // with Retail + CSIM
#if 0
               // Setup Video Registers
               // Wait for Update Window
               while (GET(ghwVD->vdVoPsStatus0) & SST_VO_PS_UPDATE)
                  ;
#endif     
 
               SETDW(ghwVD->vdPsSliCfg0, (i << SST_VO_SLI_DEVICE_SHIFT) | dwsliBaseCfg0);
               SETDW(ghwVD->vdPsSliCfg1, dwsliBaseCfg1);
               SETDW(ghwVD->vdVoPsStatus0, GET(ghwVD->vdVoPsStatus0) | SST_VO_PS_UPDATE);

               }
            }
         }

      // Note: Is this the right spot???
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE1_SHIFT) | (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (dwChipMask << SST_SLI_WEN0_MEMBASE0_SHIFT));   
      phantom_allocSurface(ppdev, _FF(mmTransientHeapStart) - _FF(gdiDesktopStart), (DWORD *)&pDst);
      camEntry = FxCamReserveEntry(_DD(camMgr), ppdev, 1);

//      pSaveDst = pDst;
//      if ((NULL != pDst) && (ILLEGAL_CAM_ENTRY != camEntry))
//         {
//         dstCAM.baseAddress = (DWORD)pDst - _FF(LFBBASE);
//         dstCAM.endAddress = dstCAM.baseAddress + (_FF(mmTransientHeapStart) - _FF(gdiDesktopStart));
//         dstCAM.physicalBase = GET(ghwAC->cam[1].physicalBase);
//         dstCAM.strideState = GET(ghwAC->cam[1].strideState);
//
//         dwBandByte = dwBandHeight * _FF(ddPrimarySurfaceData.dwPitch);
//         pSrc = (BYTE *)_FF(ddPrimarySurfaceData.lfbPtr);
//         pDst -= dwBandByte;
//         dwWidth = ppdev->bi.biWidth * _FF(ddPrimarySurfaceData.dwBytesPerPixel);
//         for (i=0; i<(DWORD)ppdev->bi.biHeight; i+= dwBandHeight)
//            {
//            dwUnit = (i >> _DD(dwLog2BandHeight)) & _FF(dwNumSlaves);
//            if (0x0 == dwUnit)
//               pDst += dwBandByte;
//      
//            SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE1_SHIFT) | (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(dwUnit) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
//            FxCamProgram(_DD(camMgr), ppdev, &dstCAM, camEntry);
//            pBandDst = pDst;
//            for (j=0; j<dwBandHeight; j++)
//               {
//               memcpy(pBandDst, pSrc, dwWidth);      
//               pSrc += _FF(ddPrimarySurfaceData.dwPitch);
//               pBandDst += _FF(ddPrimarySurfaceData.dwPitch);
//               }
//
//            FxResetCamEntry(_DD(camMgr), ppdev, camEntry);
//            }
//         }
//
//      if (pSaveDst)
//         phantom_freeSurface(ppdev, (DWORD)pSaveDst);
//
//  NEXT TWO LINES MUST BE DELETED TO RESTORE THIS CODE !!!!!!!!!

      if (pDst)
         phantom_freeSurface(ppdev, (DWORD)pDst);

      if (CAM_ENTRY_ILLEGAL != camEntry)
         FxCamFreeEntry(_DD(camMgr), ppdev, 0x1);
      // Set the Broadcast for Normal Running
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (dwChipMask << SST_SLI_WEN0_MEMBASE0_SHIFT));   

      // Set the CAM to distribute the surface
      strideState = GET(ghwAC->cam[1].strideState);
      strideState |= SST_CAM_EN_DISTRIBUTED;
      SETDW(ghwAC->cam[1].strideState, strideState);

      dwReturn = 0x1;
      }
   else
      {
      dwReturn = 0x0;
      }

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: Disable_SLI

Description:   Function to Disable SLI

Return:        1
----------------------------------------------------------------------*/
int Disable_SLI(NT9XDEVICEDATA * ppdev)
{
   DWORD dwsliBaseConfig;  // Non-unique copy of sliConfig
   DWORD dwcmdBaseConfig;  // Ditto for cmdConfig
   DWORD dwsliBaseCfg0;    // Non-unique copy of vdPsSliCfg0
   DWORD dwsliBaseCfg1;    // Non-unique copy of vdPsSliCfg1
   DWORD dwReturn = 0x0;   // return value
   DWORD dwChipMask;       // Chip Mask
   DWORD dwBandHeight;     // Height Per Chip
   DWORD i;                // index
   DWORD strideState;
   Sst2CRegs * p2CRegs;
   Sst2Regs * p3DRegs;
   Sst2CAMEntry CAM[16];
   FxU32 oldcmdBaseSize;
   FxU32 newcmdBaseSize;
   FxU32 dwStatus;
//   Sst2CAMEntry dstCAM;
//   BYTE * pSrc;
   BYTE * pDst;
//   BYTE * pSaveDst;
   FxU32 camEntry;
//   DWORD dwWidth;

   CMDFIFO_PROLOG(hwPtr);
   CMDFIFO_CHECKROOM(hwPtr, 2 * MOP_SIZE);
   SETMOP(hwPtr, SST_MOP_STALL_2D); 
   SETMOP(hwPtr, SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE | SST_MOP_STALL_3D | ((SST_MOP_STALL_3D_TA | SST_MOP_STALL_3D_TD | SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT)); 
   CMDFIFO_EPILOG(hwPtr);

   //Disable SLI
   if (_DD(sliMode) && _FF(dwNumSlaves))
      {
      dwBandHeight = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_NAME, SLI_BAND_HEIGHT_MIN, SLI_BAND_HEIGHT_DEFAULT, SLI_BAND_HEIGHT_MAX);
      dwsliBaseConfig = (0x0 << SST_SLI_NUM_CHIPS_SHIFT) | (_DD(dwLog2NumChips) << SST_SLI_NUM_CHIPS_SHIFT) | (0x0 << SST_SLI_INTERLEAVE_SHIFT);
      dwcmdBaseConfig = (0x0 << SST_CMD_SLI_NUM_CHIPS_SHIFT) | (_DD(dwLog2NumChips) << SST_SLI_NUM_CHIPS_SHIFT) | (0x0 << SST_CMD_SLI_INTERLEAVE_SHIFT);

      // Video Registers only difference is Chip ID
      dwsliBaseCfg0 = 0x0;

      // Video Registers Same for all
      dwsliBaseCfg1 = 0x0;

      dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
      p3DRegs = (Sst2Regs *)(_FF(regBase) + SST_3D_OFFSET);
      p2CRegs = (Sst2CRegs *)(_FF(regBase) + SST_CMD_OFFSET);

      // Note: Is this the right spot???
      phantom_allocSurface(ppdev, _FF(mmTransientHeapStart) - _FF(gdiDesktopStart), (DWORD *)&pDst);
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(_FF(dwChipID)) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
      camEntry = FxCamReserveEntry(_DD(camMgr), ppdev, 1);
//      pSaveDst = pDst;
//      if ((NULL != pDst) && (ILLEGAL_CAM_ENTRY != camEntry))
//         {
//         //
//         // Do a Rebuild by having 1 cam distributed and 1 not distributed
//         // the not is only on the Master Chip
//         //
//         dstCAM.baseAddress = (DWORD)pDst - _FF(LFBBASE);
//         dstCAM.endAddress = dstCAM.baseAddress + _FF(mmTransientHeapStart) - _FF(gdiDesktopStart);
//         dstCAM.physicalBase = GET(ghwAC->cam[1].physicalBase);
//         dstCAM.strideState = GET(ghwAC->cam[1].strideState) & ~SST_CAM_EN_DISTRIBUTED;
//
//         FxCamProgram(_DD(camMgr), ppdev, &dstCAM, camEntry);
//         pSrc = (BYTE *)_FF(ddPrimarySurfaceData.lfbPtr);
//         dwWidth = ppdev->bi.biWidth * _FF(ddPrimarySurfaceData.dwBytesPerPixel);
//         pSrc = pSrc + (_FF(mmTransientHeapStart) - _FF(gdiDesktopStart)) - _FF(ddPrimarySurfaceData.dwPitch);
//         pDst = pDst + (_FF(mmTransientHeapStart) - _FF(gdiDesktopStart)) - _FF(ddPrimarySurfaceData.dwPitch);
//         for (i=0; i<(DWORD)ppdev->bi.biHeight; i+= dwBandHeight)
//            {
//            memcpy(pDst, pSrc, dwWidth);      
//            pDst -= _FF(ddPrimarySurfaceData.dwPitch);
//            pSrc -= _FF(ddPrimarySurfaceData.dwPitch);
//            }
//         }
//         
//      if (pSaveDst)
//         phantom_freeSurface(ppdev, (DWORD)pSaveDst);
      if (pDst)
         phantom_freeSurface(ppdev, (DWORD)pDst);

      if (CAM_ENTRY_ILLEGAL != camEntry)
         FxCamFreeEntry(_DD(camMgr), ppdev, 0x1);

      // Set the Broadcast for Normal Running
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (dwChipMask << SST_SLI_WEN0_MEMBASE0_SHIFT));   

      // Wait for everyone to go idle......
      // Everybody out
      P6FENCE;

      do 
         {
         dwStatus = 0x0;
         for (i=0x0; i<8; i++)
            {
            if (dwChipMask & BIT(i))
               {
               // First Set the Broadcast Register for Each Device !!!!
               SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
               dwStatus |= GET(ghwIO->status); 
               }
            }
         }
      while (dwStatus & SST2_BUSY);      

      // Disable CMDFIFO on all
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | dwChipMask);   
      oldcmdBaseSize = p2CRegs->cmdFifo.baseSize;
      newcmdBaseSize = oldcmdBaseSize & ~SST_EN_CMDFIFO;
      p2CRegs->cmdFifo.baseSize = newcmdBaseSize;

      // Save CAM
      for (i=0; i<16; i++)
         {
         CAM[i].baseAddress = p2CRegs->cam[i].baseAddress;
         CAM[i].endAddress = p2CRegs->cam[i].endAddress;
         CAM[i].physicalBase = p2CRegs->cam[i].physicalBase;
         CAM[i].strideState = p2CRegs->cam[i].strideState;
         }

      // Kill CAM on all
      for (i=0; i<16; i++)
         {
         p2CRegs->cam[i].baseAddress = 0x0;
         p2CRegs->cam[i].endAddress = 0x0;
         p2CRegs->cam[i].physicalBase = 0x0;
         p2CRegs->cam[i].strideState = 0x0;
         }

      for (i=0x0; i<8; i++)
         {
         if (dwChipMask & BIT(i))
            {
            // First Set the Broadcast Register for Each Device !!!!
            SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
               
            // Configuration Register
            if (i != _FF(dwChipID))
               {
               SETDW(ghwIO->sliConfig, (i << SST_SLI_CHIP_ID_SHIFT) | dwsliBaseConfig);  
               // Need a 3D Flush Here
               SETDW(ghwAC->cmdSLIConfig, (i << SST_CMD_SLI_RA_CHIP_ID_SHIFT) | (i << SST_CMD_SLI_CHIP_ID_SHIFT) | dwcmdBaseConfig);
               }

            // Hardware recommends this but it does not work well
            // with Retail + CSIM
#if 0
            // Setup Video Registers
            // Wait for Update Window
            while (GET(ghwVD->vdVoPsStatus0) & SST_VO_PS_UPDATE)
               ;
#endif
      
            SETDW(ghwVD->vdPsSliCfg0, (i << SST_VO_SLI_DEVICE_SHIFT) | dwsliBaseCfg0);
            SETDW(ghwVD->vdPsSliCfg1, dwsliBaseCfg1);
            SETDW(ghwVD->vdVoPsStatus0, GET(ghwVD->vdVoPsStatus0) | SST_VO_PS_UPDATE);
            }
         }

      // Set the Broadcast for Normal Running
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(_FF(dwChipID)) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
      SETDW(ghwIO->sliConfig, (_FF(dwChipID) << SST_SLI_CHIP_ID_SHIFT) | dwsliBaseConfig);  
      SETDW(ghwAC->cmdSLIConfig, (_FF(dwChipID) << SST_CMD_SLI_RA_CHIP_ID_SHIFT) | (_FF(dwChipID) << SST_CMD_SLI_CHIP_ID_SHIFT) | dwcmdBaseConfig);

      // Restore CAM on Master
      for (i=0; i<16; i++)
         {
         p2CRegs->cam[i].baseAddress = CAM[i].baseAddress;
         p2CRegs->cam[i].endAddress = CAM[i].endAddress;
         p2CRegs->cam[i].physicalBase = CAM[i].physicalBase;
         p2CRegs->cam[i].strideState = CAM[i].strideState;
         }

      strideState = GET(ghwAC->cam[1].strideState);
      strideState &= ~SST_CAM_EN_DISTRIBUTED;
      SETDW(ghwAC->cam[1].strideState, strideState);

      // Restore CMDFIFO
      p2CRegs->cmdFifo.baseSize = oldcmdBaseSize;

      dwReturn = 0x1;
      }
  
   _DD(sliMode) = SLI_MODE_DISABLED;
   _DD(sliMemoryMode) = MEMORY_MODE_DISABLED;
   _DD(dwComputeMode) = FALSE;
   // Tell the 2D engine to stop punting
   _FF(gdiFlags) &= ~SDATA_GDIFLAGS_SLI_MASTER;      

   return dwReturn;
}


/*----------------------------------------------------------------------
Function name: ComputeConfig

Description: Compute SLI/AA Configuration

Return:        1
----------------------------------------------------------------------*/
int ComputeConfig(NT9XDEVICEDATA * ppdev)
{
   DWORD dwDisableSLI;
   DWORD dwNumChips;       // Num of Chips -- Slaves + Master

   dwDisableSLI = RetrieveFromRegistry(ppdev, DISABLE_SLI_NAME, DISABLE_SLI_MIN, DISABLE_SLI_DEFAULT, DISABLE_SLI_MAX);

   //Determine what Video Mode that we are going into
   if ((_FF(dwNumUnits) > 1) && (0x0 == dwDisableSLI))
      {
      _DD(sliMode) = SLI_MODE_ENABLED;
      _DD(sliMemoryMode) = MEMORY_MODE_DISABLED;
      switch(_FF(dwNumUnits))
         {
         case 2:
            if (0 == _FF(dwNumSlaves))
               {
               _DD(sliMemoryMode) = MEMORY_MODE_DISABLED;      
               }
            else         
               {
               _DD(sliMemoryMode) = MEMORY_MODE_DOUBLE;      
               }
             break;

         case 4:
            if (1 == _FF(dwNumMasters))
               {
               _DD(sliMemoryMode) = MEMORY_MODE_QUAD;      
               }
            else if (2 == _FF(dwNumMasters))
               {
               _DD(sliMemoryMode) = MEMORY_MODE_DOUBLE;      
               }
            else
               {
               if (_FF(dwNumSlaves) > 0)
                  _DD(sliMemoryMode) = MEMORY_MODE_DOUBLE;      
               else     
                  _DD(sliMemoryMode) = MEMORY_MODE_DISABLED;      
               }     

         case 8:
            if (1 == _FF(dwNumMasters))
               {
               _DD(sliMemoryMode) = MEMORY_MODE_BY_8;      
               }
            else if (2 == _FF(dwNumMasters))
               {
               _DD(sliMemoryMode) = MEMORY_MODE_QUAD;      
               }
            else
               {
               if (_FF(dwNumSlaves) > 2)
                  _DD(sliMemoryMode) = MEMORY_MODE_QUAD;
               else      
                  _DD(sliMemoryMode) = MEMORY_MODE_DOUBLE;      
               }     
         }

      if (_FF(dwNumSlaves))
         {
         _DD(dwBandHeight) = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_NAME, SLI_BAND_HEIGHT_MIN, SLI_BAND_HEIGHT_DEFAULT, SLI_BAND_HEIGHT_MAX);
         _DD(dwLog2BandHeight) = GetLog2(_DD(dwBandHeight));
         _DD(dwLog2GroupHeight) = _DD(dwLog2BandHeight) + GetLog2(_FF(dwNumSlaves) + 1);
         _DD(dwMaxHeight) = ((ppdev->bi.biHeight + (1 << _DD(dwLog2GroupHeight)) - 1) >> _DD(dwLog2GroupHeight)) << _DD(dwLog2BandHeight);
         dwNumChips = _FF(dwNumSlaves) + 1;
         _DD(dwLog2NumChips) = GetLog2(dwNumChips);
         }
      }
   else
      {
      _DD(sliMode) = SLI_MODE_DISABLED;
      _DD(sliMemoryMode) = MEMORY_MODE_DISABLED;
      }

   return 0x0;
}

/*----------------------------------------------------------------------
Function name: ConfigSLI

Description: This function is called to write to cmdSliConfig to
 enable/disable SLI.

Return:        1
----------------------------------------------------------------------*/
int ConfigSLI(NT9XDEVICEDATA * ppdev, int nSLI)
{
   DWORD dwcmdBaseConfig;  // Ditto for cmdConfig
   DWORD dwStatus;
   DWORD dwChipMask;
   int i;

   CMDFIFO_PROLOG(hwPtr);
   CMDFIFO_CHECKROOM(hwPtr, 2 * MOP_SIZE);
   SETMOP(hwPtr, SST_MOP_STALL_2D); 
   SETMOP(hwPtr, SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE | SST_MOP_STALL_3D | ((SST_MOP_STALL_3D_TA | SST_MOP_STALL_3D_TD | SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT)); 
   CMDFIFO_EPILOG(hwPtr);
   
   dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));

   // Wait for everyone to go idle......
   // Everybody out
   P6FENCE;

   do 
      {
      dwStatus = 0x0;
      for (i=0x0; i<8; i++)
         {
         if (dwChipMask & BIT(i))
            {
            // First Set the Broadcast Register for Each Device !!!!
            SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
            dwStatus |= GET(ghwIO->status); 
            }
         }
      }
   while (dwStatus & SST2_BUSY);      

   if (nSLI)
      {
      dwcmdBaseConfig = (_DD(dwLog2NumChips) << SST_SLI_NUM_CHIPS_SHIFT) | (_DD(dwLog2BandHeight) << SST_SLI_INTERLEAVE_SHIFT);
      for (i=0x0; i<8; i++)
         {
         if (dwChipMask & BIT(i))
            {
            // First Set the Broadcast Register for Each Device !!!!
            SETDW(ghwIO->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
            SETDW(ghwAC->cmdSLIConfig, (i << SST_CMD_SLI_RA_CHIP_ID_SHIFT) | (i << SST_CMD_SLI_CHIP_ID_SHIFT) | dwcmdBaseConfig);
            }
         }
      }
   else
      {
      dwcmdBaseConfig = (_DD(dwLog2NumChips) << SST_SLI_NUM_CHIPS_SHIFT) | (0x0 << SST_SLI_INTERLEAVE_SHIFT);
      SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (dwChipMask << SST_SLI_WEN0_MEMBASE0_SHIFT));   
      SETDW(ghwAC->cmdSLIConfig, (_FF(dwChipID) << SST_CMD_SLI_RA_CHIP_ID_SHIFT) | (_FF(dwChipID) << SST_CMD_SLI_CHIP_ID_SHIFT) | dwcmdBaseConfig);
      }

   // Set the Broadcast for Normal Running
   SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | (dwChipMask << SST_SLI_WEN0_MEMBASE0_SHIFT));   

   return TRUE;   
}


#endif
