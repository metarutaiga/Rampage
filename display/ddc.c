/* -*-c++-*- */
/* $Header: ddc.c, 1, 9/11/99 9:11:43 PM PDT, StarTeam VTS Administrator$ */
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
** File name:   ddc.c
**
** Description: DDC functions and support.
**
** $Revision: 1$ 
** $Date: 9/11/99 9:11:43 PM PDT$
** $History: ddc.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:53p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 12  *****************
** User: Michael      Date: 1/04/99    Time: 4:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 11    12/24/98 11:52a Andrew
** Added some code to increase retrys
** 
** 10    12/09/98 5:25p Andrew
** Removed some redundant error checking as these seems to improve
** reliability of DDC.
** 
** 9     10/30/98 7:27a Andrew
** Changed a "||" to a "&&" to match GLOP.  The early exit is believed to
** cause problems on certain monitors.
** 
** 8     10/08/98 5:29p Andrew
** Fixed a early exit to exit on either SCL or SDA not being asserted
** 
** 7     10/05/98 3:58p Andrew
** Updated to support Multimonitor
** 
** 6     8/23/98 6:53p Andrew
** Fixed the problem with the Vivitron and Driver DDC code.  It seems the
** Vivitron does not like a stop before the read command.
** 
** 5     6/23/98 10:15p Andrew
** Changed the error checking for dmon97.exe so that they could pass in a
** bogus di for function 0.
** 
** 4     5/27/98 8:46a Andrew
** Removed 256 byte DDC stuff
** 
** 3     4/23/98 4:41p Andrew
** Changed return code when DDC fails on a monitor from 024f to 004f.
** 
** 2     4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 1     4/06/98 5:29p Andrew
** DDC 1.0 & 2.0 support
** 
*/

#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "devtable.h"
#include "i2c.h"
#include "ddc2b.h"
#include "time.h"

int DDCCommand(PI2CMASK pI2CMask);
int DoWakeUp(PI2CMASK pI2CMask);

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

#undef DDC_DEBUG


/*----------------------------------------------------------------------
Function name:  DoMapFlat

Description:    Perform a flat mapping.

Information:    

Return:         DWORD       Value returned from the VMMCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE DoMapFlat(BYTE bAH, BYTE bAL)
{
    DWORD retval;
    
    __asm pushad;
    __asm mov ah, bAH;
    __asm mov al, bAL;
    VMMCall(Map_Flat);
    __asm mov retval, eax;
    __asm popad;

    return retval;
}


/*----------------------------------------------------------------------
Function name:  DDCSupport

Description:    This routine is used to handle the VBE/DDC
                function calls.

Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int DDCSupport(PDEVTABLE pDevTable, PCRS pCR)
{
   I2CMASK I2CMask;
   CLIENT_STRUCT * pCSRS = (CLIENT_STRUCT *)pCR;
   PBYTE pByte;
   int nReturn = 0x0;
   int i;
   WORD nStatus;
   
   if (NULL != pDevTable)
      {
      I2CMask.pReg = (DWORD *)(pDevTable->RegBase + I2COUT_PORT);
      I2CMask.bAddr = DDC_ADDR_1;

      I2CMask.bEnableBit = DDC_ENABLE;
      I2CMask.bSCLOutBit = DDC_SCL_OUT_BIT;
      I2CMask.bSDAOutBit = DDC_SDA_OUT_BIT;
      I2CMask.bSCLInBit = DDC_SCL_IN_BIT;
      I2CMask.bSDAInBit = DDC_SDA_IN_BIT;
      I2CInit(&I2CMask);

      switch (pCSRS->CBRS.Client_BL)
         {
         // Report DDC Capabilities
         case 0:
            // ES:DI should be zero on this call
            // dmon97.exe which is a WHQL test has this wrong
            //
#ifdef NULL_PTR_CHK
            if (0x0 == (pCSRS->CWRS.Client_DI | pCSRS->CRS.Client_ES))
#else
            if (0x0 == pCSRS->CRS.Client_ES)
#endif
               {
               nStatus = 0x0100;
               for (i=0; i < NUM_RETRY; i++)
                  if (DoWakeUp(&I2CMask))
                     {
                     nStatus |= 0x0002;
                     break;
                     }
               
               // EDID 2.0 Capable??
#ifdef DDC_20
               if (nStatus & 0x0002)
                  {
                  I2CMask.bAddr = DDC_ADDR_2;
                  for (i=0; i < NUM_RETRY; i++)
                     if (DoWakeUp(&I2CMask))
                        {
                        nStatus |= 0x0008;
                        break;
                        }
                  pCSRS->CWRS.Client_AX = 0x004F;
                  pCSRS->CWRS.Client_BX = nStatus;
                  }
               else
#endif
                  {
                  pCSRS->CWRS.Client_AX = 0x004F;
                  pCSRS->CWRS.Client_BX = nStatus;
                  }
               }
            else
               pCSRS->CWRS.Client_AX = 0x014F;
            
            nReturn = 1;
            break;

         // Read EDID
         case 1:
            pByte = (PBYTE)CLIENT_PTR_FLAT(ES, DI);
            if (0xFFFFFFFF != (DWORD)pByte)
               {
               if (ReadDDC((PBYTE)pDevTable->RegBase, DDC_ADDR_1, pByte, A0_EDID_SIZE))
                  pCSRS->CWRS.Client_AX = 0x004F;
               else
                  pCSRS->CWRS.Client_AX = 0x014F;
               }
            else
               pCSRS->CWRS.Client_AX = 0x014F;

            nReturn = 1;
            break;

         // Read VDIF <unsupported>
         case 2:
            pCSRS->CBRS.Client_AL = 0x0;
            nReturn = 1;
            break;

         // Read EDID 2
         case 3:
            pByte = (PBYTE)CLIENT_PTR_FLAT(ES, DI);
            if (0xFFFFFFFF != (DWORD)pByte)
               {
               if (ReadDDC((PBYTE)pDevTable->RegBase, DDC_ADDR_2, pByte, A2_EDID_SIZE))
                  pCSRS->CWRS.Client_AX = 0x004F;
               else
                  pCSRS->CWRS.Client_AX = 0x014F;
               }
            else
               pCSRS->CWRS.Client_AX = 0x014F;

            nReturn = 1;
            break;

         // Undefined and Unsupported
         default:
            pCSRS->CBRS.Client_AL = 0x0;
            nReturn = 1;
            break;
         }
      }

   return nReturn;
}


/*----------------------------------------------------------------------
Function name:  ReadDDC

Description:    High-level routine to read DDC for either
                128 or 256 EDID.
Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int ReadDDC(PBYTE pMap, int nAddr, PBYTE pBuffer, int nSize)
{
   I2CMASK I2CMask;
   int i;
   int j;
   int nReturn = 0;
   BYTE bChkSum;

   I2CMask.pReg = (DWORD *)(pMap + I2COUT_PORT);
   I2CMask.bAddr = nAddr;

   I2CMask.bEnableBit = DDC_ENABLE;
   I2CMask.bSCLOutBit = DDC_SCL_OUT_BIT;
   I2CMask.bSDAOutBit = DDC_SDA_OUT_BIT;
   I2CMask.bSCLInBit = DDC_SCL_IN_BIT;
   I2CMask.bSDAInBit = DDC_SDA_IN_BIT;

   I2CInit(&I2CMask);

   for (j=0; j<NUM_RETRY<<1; j++)
      {
      if (DoWakeUp(&I2CMask))
         {   
         if (DDCCommand(&I2CMask))
            {
            for (i=0; i<nSize-1; i++)
               pBuffer[i] = read_byte(&I2CMask, 0x01);
            pBuffer[i] = read_byte(&I2CMask, 0x00);

            stop(&I2CMask);
            bChkSum=0;

            for (i=0; i<nSize; i++)
               {
#ifdef DDC_DEBUG
               Debug_Printf("%d %02x\n", i, pBuffer[i]);
#endif
               bChkSum += pBuffer[i];
               }

            if (0 == bChkSum)
               {
               nReturn = 1;
               break;
               }
            else
               {
               Debug_Printf("Chksum failure %02x \n", bChkSum);
               }
            }
         else
            {
            Debug_Printf("DDCCommand failed\n");
            }
         }
      else
         {
         Debug_Printf("Wake Up Failed\n");
         }
      }

   return nReturn;
}


/*----------------------------------------------------------------------
Function name:  DDCCommand

Description:    Routine to check if this is a DDC Monitor.

Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int DDCCommand(PI2CMASK pI2CMask)
{
   int nReturn = 0;

   start(pI2CMask);
   if (!send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE)))
      {
      if (!send_byte(pI2CMask, 0x0))
         {
         // Does not work on Vivitron ?
#if 0
         stop(pI2CMask);
#endif
         start(pI2CMask);
         send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_READ));
         sda(pI2CMask, 1);
         nReturn = 1;
         }
      else
         {
         stop(pI2CMask);
         Debug_Printf("failed to send %x\n", 0x0);
         }
      }
   else
      {
      stop(pI2CMask);
      Debug_Printf("failed to send %x\n", pI2CMask->bAddr|MASTER_WRITE);
      }

   return nReturn;
}


/*----------------------------------------------------------------------
Function name:  DoWakeUp

Description:    Routine to query the monitor to see if it is alive.

Information:    

Return:         INT     1 if success,
                        0 if failure
----------------------------------------------------------------------*/
int read_sda(PI2CMASK pI2CMask);
int DoWakeUp(PI2CMASK pI2CMask)
{
   int i;
   int nReturn = 0;
   BYTE bCH;

   // Possible Switch Box Fix?
   sda(pI2CMask, 1);
   scl(pI2CMask, 1);
   // 166 MSEC is approx 60 Hz
   for (i=0; i<166; i++)
      {
      if ((1 == read_scl(pI2CMask)) && (1 == read_sda(pI2CMask)))
         break;
      DELAY(MSEC);
      }

   if ((1 != read_scl(pI2CMask)) && (1 != read_sda(pI2CMask)))
      return nReturn; 

   if (DDCCommand(pI2CMask))
      {
      bCH = read_byte(pI2CMask, 0x01);
      if (0x00 == bCH)
         {
         bCH = read_byte(pI2CMask, 0x00);
         if (0xFF == bCH)
            nReturn = 1;
         else
            {
            stop(pI2CMask);
            Debug_Printf("Read Failed expected 0xFF got %02x\n", bCH);
            }
          }
       else
         {
         stop(pI2CMask);
         Debug_Printf("Read Failed expected 0x00 got %02x\n", bCH);
         }
       }
    else
       Debug_Printf("DDC Command Failed\n");

   return nReturn;
}

