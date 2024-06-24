/* -*-c++-*- */
/* $Header: i2c.c, 1, 9/11/99 10:22:43 PM PDT, StarTeam VTS Administrator$ */
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
** File name:   i2c.c
**
** Description: i2c functions.
**
** $Revision: 1$ 
** $Date: 9/11/99 10:22:43 PM PDT$
** 
** $History: i2c.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:56p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 12  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/08/99    Time: 4:24p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 10    12/15/98 2:13p Stuartb
** In I2C init set avenger TVOUT_RST_BIT high by default.  This won't
** affect Banshee.
** 
** 9     12/09/98 5:27p Andrew
** changed stop to back the way it was
** 
** 8     12/08/98 5:52p Andrew
** Possible fix to DDC weirdness -- changed stop to how I had it
** originally.
** 
** 7     10/08/98 4:27p Andrew
** Fixed a problem with a counter that was not being incremented.
** Believed to be the DDC monitor and switch box problem
** 
** 6     9/16/98 11:05a Stuartb
** Fixed bug that left SCL low after register write.
** 
** 5     9/10/98 2:45p Stuartb
** Added PLL and sage LCD write routines.
** 
** 4     6/23/98 10:16a Stuartb
** Added I2C multibyte writes.
** 
** 3     5/05/98 1:20p Stuartb
** Adding i2c extEscape functionality.
** 
** 2     4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 1     4/06/98 5:30p Andrew
** Low Level I2C routines
** 
*/

#include "h3vdd.h"
#include "h3.h"
#include "i2c.h"
#include "time.h"

/********************************************************************************
*
* The file i2c.c implements the low level I2C routines.
* void scl(PI2CMASK pI2CMask, DWORD bit);
* void sda(PI2CMASK pI2CMask, DWORD bit);
* int read_scl(PI2CMASK pI2CMask);
* void start(PI2CMASK pI2CMask);
* void stop(PI2CMASK pI2CMask);
* int send_byte(PI2CMASK pI2CMask, BYTE data);
* BYTE read_byte(PI2CMASK pI2CMask, DWORD ack);
*
* int ReadI2CRegister(PI2CMASK pI2CMask);
* int WriteI2CRegister(PI2CMASK pI2cMask);
* void I2CInit(PI2CMASK pI2CMask);
*
*********************************************************************************/
#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

const struct i2cmask I2C_PROTO = {0, 0, 0, 1, 0, I2C_ENABLE, I2C_SCL_OUT_BIT, \
						I2C_SDA_OUT_BIT, I2C_SCL_IN_BIT, I2C_SDA_IN_BIT};
const DWORD I2C_INITVAL = (DWORD) (1 << I2C_ENABLE) | (1 << I2C_SCL_OUT_BIT) | (1 << I2C_SDA_OUT_BIT) | (1 << TVOUT_RST_BIT);

#define GETI2CDW(pReg) (*pReg)
#define SETI2CDW(pReg, dwValue) (*pReg = dwValue)


/*----------------------------------------------------------------------
Function name:  scl

Description:    Sets the Clock to bit
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void scl(PI2CMASK pI2CMask, DWORD bit)
{
	DWORD dwData;
	int nCount = 0;

	DELAY(DTIME);

	dwData = GETI2CDW(pI2CMask->pReg);
	dwData = (dwData & ~((DWORD)1 << pI2CMask->bSCLOutBit)) | (bit << pI2CMask->bSCLOutBit);
	SETI2CDW (pI2CMask->pReg, dwData);

	DELAY(DTIME);

	if (bit)
		while (!read_scl(pI2CMask) && (nCount++ < CLOCK_STRETCH))
			DELAY(DTIME);

	if ((bit) && (!read_scl(pI2CMask)))
		Debug_Printf("Slave did not respond %s %d\n", __FILE__, __LINE__);
}


/*----------------------------------------------------------------------
Function name:  sda

Description:    Sets the Data to bit.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void sda(PI2CMASK pI2CMask, DWORD bit)
{
	DWORD	dwData;
	DELAY(DTIME);
		
   if (bit)
      bit = ((DWORD)1 << pI2CMask->bSDAOutBit);
	else
      bit = 0x0;

	dwData = (GETI2CDW(pI2CMask->pReg)) & ~((DWORD)1 << pI2CMask->bSDAOutBit) | bit;
	SETI2CDW (pI2CMask->pReg, dwData);
	DELAY(DTIME);
}


/*----------------------------------------------------------------------
Function name:  read_scl

Description:    Reads the Clock
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int read_scl(PI2CMASK pI2CMask)
{
	return (GETI2CDW(pI2CMask->pReg) >> pI2CMask->bSCLInBit) & 0x01;
}


/*----------------------------------------------------------------------
Function name:  read_sda

Description:    Reads the Data line
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int read_sda(PI2CMASK pI2CMask)
{
	return (GETI2CDW(pI2CMask->pReg) >> pI2CMask->bSDAInBit) & 0x1;
}


/*----------------------------------------------------------------------
Function name:  start

Description:    Sends a start
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void start(PI2CMASK pI2CMask)
{
	sda(pI2CMask, 1);
	scl(pI2CMask, 1);
	sda(pI2CMask, 0);
	scl(pI2CMask, 0);
}


/*----------------------------------------------------------------------
Function name:  stop

Description:    Sends a stop
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void stop(PI2CMASK pI2CMask)
{
	scl(pI2CMask, 0);
	sda(pI2CMask, 0);
	scl(pI2CMask, 1);
	sda(pI2CMask, 1);
}


/*----------------------------------------------------------------------
Function name:  send_byte

Description:    Sends the byte data
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int send_byte(PI2CMASK pI2CMask, BYTE data)
{
	int	i;
	int nReturn=0;

	for (i = 0; i < 8; i++)
	{
	  sda(pI2CMask, (data << i) & 0x80);
	  scl(pI2CMask, 1);
	  if (read_scl(pI2CMask) == 0)
	  {
	     Debug_Printf("nc\n");
	  }
	  scl(pI2CMask, 0);
	}
	// Check for ACK
	sda(pI2CMask, 1);
	scl(pI2CMask, 1);
	if (read_sda(pI2CMask) == 1)
		nReturn = 1;
	else
		nReturn = 0;

	if (nReturn == 1)
		{
		DELAY(DTIME);
		DELAY(DTIME);
		}
	
	scl(pI2CMask, 0);
	return nReturn;
}


/*----------------------------------------------------------------------
Function name:  read_byte

Description:    Reads a byte and sends a ack if ack=1.
                
Information:

Return:         BYTE
----------------------------------------------------------------------*/
BYTE read_byte(PI2CMASK pI2CMask, DWORD ack)
{
	int	i, data = 0;
	for (i = 0; i < 8; i++)
	{
	  scl(pI2CMask, 1);

	  if (read_scl(pI2CMask) == 0)
			Debug_Printf("Slave not ready for read %s %d\n", __FILE__, __LINE__);

	  data <<= 1;
	  data |= read_sda(pI2CMask);

	  scl(pI2CMask, 0);
	}
	if (ack)
	  sda(pI2CMask, 0);
	else
	  sda(pI2CMask, 1);

	scl(pI2CMask, 1);
	scl(pI2CMask, 0);
	sda(pI2CMask, 1);
	return (BYTE)(data);
}


/*----------------------------------------------------------------------
Function name:  ReadI2CRegister

Description:    Reads a I2C register
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int ReadI2CRegister(PI2CMASK pI2CMask)
{
	int nReturn = 0;

	start(pI2CMask);
 	nReturn |= send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE));
	nReturn |= send_byte(pI2CMask, (BYTE)(pI2CMask->nReg));
	start(pI2CMask);
	nReturn |= send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_READ));
	pI2CMask->dwData = read_byte(pI2CMask, 0x0);
	stop(pI2CMask);

    pI2CMask->nSize = 1;
	/*
	Debug_Printf("Register %02x is %02x\n", pI2CMask->nReg, pI2CMask->dwData);
	*/
	return (!nReturn ? FXTRUE : FXFALSE);
}


/*----------------------------------------------------------------------
Function name:  WriteI2CRegister  

Description:    Writes a value dwData to a register nReg
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int WriteI2CRegister(PI2CMASK pI2CMask)
{
	int nCount;
	int nSuccess = FXFALSE;

   /*
   Debug_Printf("Register %02x set %02x\n", pI2CMask->nReg, pI2CMask->dwData);
   */

	for (nCount=0; ((nCount < NUM_RETRY) && (nSuccess == 0)) ; nCount++)
	{
		start(pI2CMask);
		if (!send_byte(pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE)))
		{
			if (!send_byte(pI2CMask, (BYTE)pI2CMask->nReg))
			{
				if (!send_byte(pI2CMask, (BYTE)(pI2CMask->dwData & 0xFF)))
					nSuccess = FXTRUE;
				else
					Debug_Printf("send_byte fail %s %d\n", __FILE__, __LINE__);
			}
			else
					Debug_Printf("send_byte fail %s %d\n", __FILE__, __LINE__);
		}
		else
			Debug_Printf("send_byte failed %s %d\n", __FILE__, __LINE__);

		stop(pI2CMask);
	}

	return (nSuccess);
}


/*----------------------------------------------------------------------
Function name:  WriteI2CRegisterMulti

Description:    Writes multiple values to a register 
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int WriteI2CRegisterMulti(PI2CMASK pI2CMask)
{
	I2CMASK i2c;
	int retries = NUM_RETRY;

m_retry:
	if (!retries)
		return (FXFALSE);
	else if (retries != NUM_RETRY)  // not first time through
		stop (pI2CMask);

	retries--;
	i2c = *pI2CMask;
	start (pI2CMask);
	if (send_byte (pI2CMask, (BYTE)(pI2CMask->bAddr|MASTER_WRITE)))
		goto m_retry;
	if (send_byte(pI2CMask, (BYTE)pI2CMask->nReg))
		goto m_retry;
	while (i2c.nSize--)
	{
		if (send_byte(pI2CMask, (BYTE)(*i2c.multiWriteData++)))
			goto m_retry;
	}
	stop (pI2CMask);
	return (FXTRUE);
}


/*----------------------------------------------------------------------
Function name:  I2CInit

Description:    Used to init a I2C Port
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void I2CInit(PI2CMASK pI2CMask)
{
   Debug_Printf("%08lx %08lx\n", pI2CMask->pReg, *pI2CMask->pReg);
   *pI2CMask->pReg |= (1 << pI2CMask->bEnableBit);
   Debug_Printf("%08lx %08lx\n", pI2CMask->pReg, *pI2CMask->pReg);
   CycleTime();
}


/////////////////////////////////////////////////////////////////////////////////
//
//SECTION FOR 9161PLL SERIAL PROGRAMMING
//
////////////////////////////////////////////////////////////////////

void unlockPLL(PI2CMASK pI2CMask);
int send_dataPLL(PI2CMASK pI2CMask, DWORD data, DWORD reg);


/*----------------------------------------------------------------------
Function name:  unlockPLL

Description:    Unlock sequence
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void unlockPLL(PI2CMASK pI2CMask)
{
	sda(pI2CMask, 0); //Reset data
	scl(pI2CMask, 0); //Reset clk
   
	sda(pI2CMask, 1); //Set data high, toggle clk 5 times
	scl(pI2CMask, 1); //1
	scl(pI2CMask, 0);
	scl(pI2CMask, 1); //2
	scl(pI2CMask, 0);
	scl(pI2CMask, 1); //3
	scl(pI2CMask, 0);
	scl(pI2CMask, 1); //4
	scl(pI2CMask, 0);
	scl(pI2CMask, 1); //5
	scl(pI2CMask, 0);
   
	sda(pI2CMask, 0); //Set data low
	scl(pI2CMask, 1); //Clk high
}


/*----------------------------------------------------------------------
Function name:  send_dataPLL

Description:    Sends the byte data
                
Information:

Return:         INT
----------------------------------------------------------------------*/
int send_dataPLL(PI2CMASK pI2CMask, DWORD data, DWORD reg)
{
	int	i;
	int nReturn=0;

   //Send START
	sda(pI2CMask, 0); //Data low
   //scl(pI2CMask, 1); //Toggle clk
	scl(pI2CMask, 0);
   scl(pI2CMask, 1);
   
   
   //Send 21 data bits:
	for (i = 0; i < 21; i++)
	{
	  sda(pI2CMask, !((data >> i) & 0x01) );
	  scl(pI2CMask, 0);
	  sda(pI2CMask, (data >> i) & 0x01);
	  scl(pI2CMask, 1);
	  if (read_scl(pI2CMask) == 0)
	  {
	     Debug_Printf("nc\n");
	  }
	}
   
   //Send 3 address bits:
	for (i = 0; i < 3; i++)
	{
	  sda(pI2CMask, !((reg >> i) & 0x01) );
	  scl(pI2CMask, 0);
	  sda(pI2CMask, (reg >> i) & 0x01);
	  scl(pI2CMask, 1);
	  if (read_scl(pI2CMask) == 0)
	  {
	     Debug_Printf("nc\n");
	  }
	}
   
   //Send STOP
	sda(pI2CMask, 1); //Data high
	scl(pI2CMask, 0);
   scl(pI2CMask, 1);
   
   DELAY(2*MSEC);    //Delay 2msec
   
	return nReturn;
}


/*----------------------------------------------------------------------
Function name:  WritePLLReg

Description:    Writes a value dwData to a register nReg
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void WritePLLReg(PI2CMASK pI2CMask)
{
	//int nSuccess = FXTRUE;
   /*
   Debug_Printf("Register %02x set %02x\n", pI2CMask->nReg, pI2CMask->dwData);
   */

	//for (nCount=0; ((nCount < NUM_RETRY) && (nSuccess == 0)) ; nCount++)
	//{
		unlockPLL(pI2CMask);
      
	   if ( send_dataPLL(pI2CMask, (pI2CMask->dwData & 0x1FFFFF), (DWORD)(pI2CMask->nReg & 0x07)) )
		{
         //nSuccess = FXFALSE;
         Debug_Printf("send_byte fail %s %d\n", __FILE__, __LINE__);
		}

	//}
}


/*----------------------------------------------------------------------
Function name:  SelectPLLReg

Description:    Select register for output
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void SelectPLLReg(PI2CMASK pI2CMask)
{
   switch ( pI2CMask->nReg )
   {
      case 0:
	      scl(pI2CMask, 0);
	      sda(pI2CMask, 0);
         break;
      case 1:
	      scl(pI2CMask, 1);
	      sda(pI2CMask, 0);
         break;
      case 2:
	      scl(pI2CMask, 0);
	      sda(pI2CMask, 1);
         break;
      case 3:
	      scl(pI2CMask, 1);
	      sda(pI2CMask, 1);
         break;
   }
}

#define FAR
#define WINAPI __stdcall
typedef          char  CHAR;
typedef   signed char  SCHAR;
typedef unsigned char  UCHAR;

typedef          short SHORT;
typedef unsigned short USHORT;

typedef          long  LONG;
typedef unsigned long  ULONG;

typedef unsigned char  BYTE,  FAR* LPBYTE;
typedef unsigned short WORD,  FAR* LPWORD;
typedef unsigned long  DWORD, FAR* LPDWORD;
typedef          void  VOID,  FAR* LPVOID;
typedef          int   BOOL;
typedef int (FAR WINAPI *FARPROC)();


#pragma pack( 1 )
    #include "gdidefs.h"
    #include "dibeng.h"
#pragma pack()

#ifdef V3TV
#include "string.h"
#include "stdlib.h"
#endif

#include "3dfx.h"

#include "h3vdd.h"

#include "h3.h"

#include "h3g.h"

#define IS_32 1
#define MM 1

#include "shared.h"
#include "devtable.h"
#undef  THUNK32

#define NT9XDEVICEDATA GLOBALDATA
#define  ghwAC (SstCRegs*)(_FF(regBase) + SST_CMDAGP_OFFSET)

#include "cmddefs.h"
#include "fifomgr.h"

#include <ddkmmini.h>
#include <stddef.h>

#ifdef V3TV
#include "kmvt.h"

#pragma intrinsic (memcmp, memcpy,memset)
#endif

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG

#define BUSY_BIT        0x0004  // bit number to test for BUSY
#define CBUSY_BIT       0x0001  // bit number to test cursor busy

#define DDOVER_BOB	        	0x00200000
#define DDOVER_INTERLEAVED  	0x00800000
#define DDOVER_AUTOFLIP         0x00100000
#define DDFLIP_EVEN             0x00000002
#define DDFLIP_ODD              0x00000004

#ifdef V3TV
//H4 vidInStatus bitmasks
#define H4_VMI_BUFFER_MASK                  0x00030000
#define H4_VMI_FIELD_MASK                   0x00040000
#endif

#ifndef DEBUG
#define  KMVT_CMDFIFO_PROLOG( hwPtr )  CMDFIFO_PROLOG( hwPtr )
#endif

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG
KMVTBUFF bltBuff;

DWORD dwLockCounter = 0;

DWORD ddFlipOverlay(void);
#ifdef V3TV
DWORD ddGetPolarity(void);
DWORD DDGetIRQInfo(void);
DWORD DDIsOurIRQ(void);
DWORD DDEnableIRQ(void);
DWORD DDSkipNextField(void);
DWORD DDBobNextField(void);
DWORD DDSetState(void);
DWORD DDLock(void);
DWORD DDFlipVideoPort(void);
DWORD DDGetPolarity(void);
DWORD DDSetSkipPattern(void);
DWORD DDGetCurrentAutoflip(void);
DWORD DDGetPreviousAutoflip(void);
DWORD DDTransfer(void);
DWORD DDGetTransferStatus(void);

KMTVDATA kmtvInfo;
#endif


WORD SetBusy( WORD * lpBusy, WORD wFlag)
{
      _asm{
        mov   ax, wFlag
        mov   ebx, lpBusy
        bts   WORD PTR [ebx],ax
        jnc   NotBusy
      }
      return 1;
NotBusy:
      return 0;
}// SetBusy


void ClearBusy(WORD *lpBusy, WORD wFlag)
{
   _asm{
       mov  ax, wFlag
       mov  ebx, lpBusy
       btr  WORD PTR [ebx], ax
    }
}


DWORD __stdcall GetKernelInfo(DIOCPARAMETERS * lpParams)
{
   LPDDMINIVDDTABLE lpVddTable;

   if( lpParams->cbOutBuffer < sizeof( DDMINIVDDTABLE ))
       return 1;
   lpParams->lpcbBytesReturned = sizeof(DDMINIVDDTABLE);

   lpVddTable = (LPDDMINIVDDTABLE)lpParams->lpvOutBuffer;
   
   lpVddTable->dwMiniVDDContext = 0x1;     //any number now
   lpVddTable->vddFlipOverlay = ddFlipOverlay;


#ifdef V3TV
   memset (&kmtvInfo, 0, sizeof (kmtvInfo));   
   kmtvInfo.Context =  lpVddTable->dwMiniVDDContext;

   lpVddTable->vddGetIRQInfo = DDGetIRQInfo;
   lpVddTable->vddIsOurIRQ = DDIsOurIRQ;
   lpVddTable->vddEnableIRQ = DDEnableIRQ;   
   lpVddTable->vddSkipNextField = DDSkipNextField;
   lpVddTable->vddBobNextField = DDBobNextField;
   lpVddTable->vddSetState = DDSetState;
   lpVddTable->vddLock = DDLock;
   lpVddTable->vddFlipVideoPort = DDFlipVideoPort;
   lpVddTable->vddGetPolarity = DDGetPolarity;
   lpVddTable->vddReserved1 = DDSetSkipPattern;
   lpVddTable->vddGetCurrentAutoflip = DDGetCurrentAutoflip;
   lpVddTable->vddGetPreviousAutoflip = DDGetPreviousAutoflip;
   lpVddTable->vddTransfer = DDTransfer;
   lpVddTable->vddGetTransferStatus = DDGetTransferStatus;
   kmtvInfo.pDev = pVGADevTable;
#endif
   return 0;   

}

/**********************************************************************
*   DESCRIPTION: Flips the overlay to the target surface.
*                For BOB and autofliping, it is only called when Vport 
*                is not involved, accroding to Scott McDonald.               
*   ENTRY:
*	   ESI	LPDDFLIPOVERLAYINFO
*		    DWORD 		dwSize
*		    LPDDSURFACEDATA	lpCurrentSurface
*		    LPDDSURFACEDATA	lpTargetSurface
*		    DWORD 		dwFlags
*	   EDI  NULL
*
*   EXIT:
*          EAX	0 = success, 1 = error
*
**********************************************************************/
DWORD ddFlipOverlay(void)
{
  DDFLIPOVERLAYINFO *lpFlipInfo;
  LPDDSURFACEDATA   lpTarget;
  DWORD dwSurfAddr;
  GLOBALDATA * ppdev;
  WORD       * pFlags;
  OVLBLTPARAMS * lpBltParams;
  BOOL       fHWBusy = TRUE;
  DWORD        dwDstBaseAddr;
  SstIORegs   *ghwIO;
  SstGRegs   *ghw2D;
  SstRegs   *ghw3D;

   _asm mov lpFlipInfo, esi
   
   lpTarget= (LPDDSURFACEDATA)lpFlipInfo->lpTargetSurface;

   ppdev = (GLOBALDATA *)lpTarget->dwDriverReserved1;

   lpBltParams =
       (OVLBLTPARAMS *)( (DWORD )ppdev + (DWORD)lpTarget->dwDriverReserved2);

   pFlags = lpBltParams->pDebAddr;
   if( !pFlags)
       pFlags = (WORD *)_FF(lpDeFlags);

   //The check deVersion and deType in DIBENGINE
   //if it is not a correct one then we cannot use deFlags too
   if( *(WORD *)((DWORD)pFlags + offsetof(DIBENGINE, deVersion) 
                        - offsetof(DIBENGINE,deFlags)) != 0x400)
           return 0;

   if( *(WORD *)((DWORD)pFlags + offsetof(DIBENGINE, deType) 
                        - offsetof(DIBENGINE,deFlags)) != 0x5250)
           return 0;

   if(lpBltParams->wShrinkFlags & OVL_SHRINK)
   {
       if(lpTarget->dwOverlayDestHeight)
         return 0;      //not sync
        
   }
   else
   {
       if(!lpTarget->dwOverlayDestHeight)
         return 0;      //not sync
   }

   dwSurfAddr = lpTarget->dwSurfaceOffset;


   ghw2D = (SstGRegs*)(_FF(regBase) + SST_2D_OFFSET);
   ghw3D = (SstRegs*)(_FF(regBase) + SST_3D_OFFSET);
   ghwIO = (SstIORegs*)(_FF(regBase) + SST_IO_OFFSET);


   if(!lpTarget->dwOverlayDestHeight)
   {
      //in this case we are shrinking overlay
       dwDstBaseAddr = lpTarget->dwOverlayOffset;

   }
   else
   {
       if(lpTarget->dwOverlayFlags & DDOVER_INTERLEAVED)
       {
          if(lpFlipInfo->dwFlags & DDFLIP_ODD )
          {
            dwSurfAddr += lpTarget->lPitch; //set in ring0 for pitch
            dwSurfAddr &= ~SSTG_IS_TILED;
          }
          else if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
            dwSurfAddr |= 0x80000000;
       }
       else
       {
          if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
            dwSurfAddr |= 0x80000000;
          else
            dwSurfAddr &= ~SSTG_IS_TILED;
       }

       dwDstBaseAddr = dwSurfAddr;
   }

   if( dwLockCounter++ == 0) 
   {
      //check busy bit
      if( !SetBusy(pFlags, BUSY_BIT))
      {

        if(!SetBusy(&_FF(cursorBusy), CBUSY_BIT))
        {
            if(!(GET(ghwIO->status) & SST_BUSY))
            {

               CMDFIFO_PROLOG(hwPtr);
               _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY;
               _FF(gdiFlags) &= ~SDATA_GDIFLAGS_KMTV_FLAG;

               if(!lpTarget->dwOverlayDestHeight)
               {

                 CMDFIFO_CHECKROOM(hwPtr,13);

                 SETPH(hwPtr, CMDFIFO_BUILD_PK2(
                    dstBaseAddrBit
                    | dstFormatBit
                    | ropBit
                    | srcBaseAddrBit
                    | clip1minBit
                    | clip1maxBit
                    | srcFormatBit
                    | srcSizeBit
                    | srcXYBit
                    | dstSizeBit
                    | dstXYBit
                    | commandBit ) );
                 SETPD(hwPtr, ghw2D->dstBaseAddr, dwDstBaseAddr);
                 SETPD(hwPtr, ghw2D->dstFormat, lpBltParams->bltDstFormat); 
                 SETPD(hwPtr, ghw2D->rop,
                 (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC );
                 SETPD(hwPtr, ghw2D->srcBaseAddr, dwSurfAddr);
                 SETPD(hwPtr, ghw2D->clip1min,  lpBltParams->clip1min);     
                 SETPD(hwPtr, ghw2D->clip1max,  lpBltParams->clip1max);     
                 SETPD(hwPtr, ghw2D->srcFormat, lpBltParams->bltSrcFormat); 
                 SETPD(hwPtr, ghw2D->srcSize,   lpBltParams->bltSrcSize);     
                 SETPD(hwPtr, ghw2D->srcXY,     lpBltParams->bltSrcXY);          
                 SETPD(hwPtr, ghw2D->dstSize,   lpBltParams->bltDstSize);     
                 SETPD(hwPtr, ghw2D->dstXY,     lpBltParams->bltDstXY);          
                 SETPD(hwPtr, ghw2D->command,
                    (SSTG_ROP_SRC << SSTG_ROP0_SHIFT )|
                    SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT );

                 BUMP(13);

                 if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
                    dwDstBaseAddr |= 0x80000000;
                 else
                    dwDstBaseAddr &= ~SSTG_IS_TILED;
               }

               CMDFIFO_CHECKROOM(hwPtr, 4);
               SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
               SETPD(hwPtr, ghw3D->leftOverlayBuf, dwDstBaseAddr);
               SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
               SETPD(hwPtr, ghw3D->swapbufferCMD, 1);
               BUMP(4);
   
               CMDFIFO_EPILOG(hwPtr);
               fHWBusy = FALSE;
            }
            ClearBusy(&_FF(cursorBusy), CBUSY_BIT);
        }

          ClearBusy(pFlags, BUSY_BIT);
      }

/*      if (fHWBusy & !(_FF(gdiFlags) & SDATA_GDIFLAGS_KMTV_FLAG))
      {
         DWORD wrPtr;
         KMVT_CMDFIFO_PROLOG(hwPtr);
         hwPtr = bltBuff.data;
            //borrow PhysAddress for blt command fifo buffer
            //so that display and DD driver can access it.
         bltBuff.dwSize = 4;
          _FF(KMVTBuff) = (DWORD)((KMVTBUFF*)&bltBuff);
         wrPtr = 0;
         hwIndex = wrPtr;

         if(!lpTarget->dwOverlayDestHeight)
         {

             srcFormat = lpBltParams->bltSrcFormat & (~0x3FFF);
             dstFormat = lpBltParams->bltDstFormat & (~0x3FFF);

             dstFormat |= (DWORD)lpTarget->dwDriverReserved3;
             srcFormat |=lpTarget->lPitch;


             bltBuff.dwSize = 17;
             SETPH(hwPtr, CMDFIFO_BUILD_PK2(
                  dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | srcBaseAddrBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit ) );
   
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            
            SETPD(hwPtr, ghw2D->dstBaseAddr, dwDstBaseAddr);
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->dstFormat, dstFormat); 
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->rop,
                (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC );
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcBaseAddr, dwSurfAddr);
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->clip1min,  lpBltParams->clip1min);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->clip1max,  lpBltParams->clip1max);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcFormat, srcFormat); 
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcSize,   lpBltParams->bltSrcSize);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->srcXY,     lpBltParams->bltSrcXY);          
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->dstSize,   lpBltParams->bltDstSize);     
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->dstXY,     lpBltParams->bltDstXY);          
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;
            SETPD(hwPtr, ghw2D->command,
                (SSTG_ROP_SRC << SSTG_ROP0_SHIFT )|
                SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT );
            wrPtr = (wrPtr + 1) & 0xFF;
            hwIndex = wrPtr;

            if(lpFlipInfo->dwFlags & DDFLIP_EVEN )
                   dwDstBaseAddr |= 0x80000000;
             else
                   dwDstBaseAddr &= ~SSTG_IS_TILED;

        }
        SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
        wrPtr = (wrPtr + 1) & 0xFF;
        hwIndex = wrPtr;
        SETPD(hwPtr, ghw3D->leftOverlayBuf, dwDstBaseAddr);
        wrPtr = (wrPtr + 1) & 0xFF;
        hwIndex = wrPtr;
        SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
        wrPtr = (wrPtr + 1) & 0xFF;
        hwIndex = wrPtr;
        SETPD(hwPtr, ghw3D->swapbufferCMD, 1);

        _FF(gdiFlags) |= SDATA_GDIFLAGS_KMTV_FLAG;
      }    */
   } 
   dwLockCounter--;
   return 0;
}    

#ifdef V3TV
/**********************************************************************
*   DDGetIRQInfo
*
*   DESCRIPTION: If the Mini VDD is already managing the IRQ, this
*          function returns that information; otherwise, it returns the
*          IRQ number assigned to the device so DDraw can manage the IRQ.
*
*          The returning the IRQ number, it is important that it get the
*          value assigned by the Config Manager rather than simply get
*          the value from the hardware (since it can be remapped by PCI).
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  NULL
*          EDI  LPDDGETIRQINFO
*                   DWORD dwSize;
*                   DWORD dwFlags;
*                   DWORD dwIRQNum;
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
DWORD DDGetIRQInfo(void)
{
   DDGETIRQINFO	*lpGetIrqInfo;
   _asm mov lpGetIrqInfo, edi
// _asm int 1;
   Debug_Printf("KMVT - GetIRQInfo\n");

   //return handled since we have our own irq handler in h3irq and
   //will manage the IRQ ourselves
   lpGetIrqInfo->dwSize = sizeof (DDGETIRQINFO);
   lpGetIrqInfo->dwFlags = IRQINFO_HANDLED;
   return 0;
}

/**********************************************************************
*   DDEnableIRQInfo
*
*   DESCRIPTION: Notifies the Mini VDD which IRQs should be enabled.  If
*          a previously enabled IRQ is not specified in this call,
*          it should be disabled.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDENABLEIRQINFO
*                   DWORD dwSize
*                   DWORD dwIRQSources
*                   DWORD dwLine
*                   DWORD IRQCallback
*                   DWORD dwContext
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
//h3 intrCtrl bitmasks
#define H3_VMI_INT_ENABLE                   0x00200000
#define H3_VMI_INTERRUPT					     0x00800000
#define H3_VSYNC_INT_ENABLE				     0x00000004
#define H3_VSYNC_INTERRUPT					     0x00000100

DWORD DDEnableIRQ(void)
{

   DDENABLEIRQINFO    *lpEnableIRQ;
   DWORD 	       dwReg;

   _asm mov lpEnableIRQ, esi
   Debug_Printf("KMVT - Enable IRQ sources %04.4x (vsync 0x01 vport 0x04)\n", lpEnableIRQ->dwIRQSources);
//_asm int 1;
   if (lpEnableIRQ->dwIRQSources != kmtvInfo.dwIRQSources) {
	dwReg = (((SstRegs *)(kmtvInfo.pDev->RegBase + SST_3D_OFFSET))->intrCtrl & 0x7FFFFFFF);
	if (lpEnableIRQ->dwIRQSources & DDIRQ_VPORT0_VSYNC)
	{
	   // VMI_IRQ_ENABLE;
	   pVGADevTable->dwIMask |= H3_VMI_INT_ENABLE;
	   dwReg |= H3_VMI_INT_ENABLE;
	}
	else
	{
		//  VMI_IRQ_DISABLE;
	   pVGADevTable->dwIMask &= ~H3_VMI_INT_ENABLE;
	   dwReg &= ~H3_VMI_INT_ENABLE;
	}
#if 0
	if (lpEnableIRQ->dwIRQSources & DDIRQ_DISPLAY_VSYNC)
	{
	   //VSYNC_IRQ_ENABLE;
	   pVGADevTable->dwIMask |= H3_VSYNC_INT_ENABLE;
	   dwReg |= H3_VSYNC_INT_ENABLE;
	}
	else
	{
	   //VSYNC_IRQ_DISABLE;
	   pVGADevTable->dwIMask &= ~H3_VSYNC_INT_ENABLE;
	   dwReg &= ~H3_VSYNC_INT_ENABLE;
	}
#endif
//kmtvInfo.Context = lpEnableIRQ->dwContext;
	kmtvInfo.IRQCallback = (lpEnableIRQ->dwIRQSources) ? lpEnableIRQ->IRQCallback : 0;
	kmtvInfo.dwIRQSources = lpEnableIRQ->dwIRQSources;
	((SstRegs *)(pVGADevTable->RegBase + SST_3D_OFFSET))->intrCtrl = dwReg;
     }
	return 0;
}


/**********************************************************************
*   DDIsOurIRQ
*
*  DESCRIPTION: Called when the VDD's IRQ handler is triggered.  This
*          determines if the IRQ was caused by our VGA and if so, it
*          clears the IRQ and returns which event(s) generated the IRQ.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  NULL
*
*   EXIT:
*          EDI  IRQ source flags
*          EAX  0 = success, 1 = error
*
*   NOTE that if ANYTHING weird is going on, like a new version of the
*   VDD resulting in traps in other people's code, it's most likely
*   because edi returned by this function isn't accurately reflecting
*   the interrupt that was generated.  It's crucial that EDI be accurate,
*   so that the system components acknowledge the interrupt correctly.
*
*
**********************************************************************/
DWORD DDIsOurIRQ(void)
{
  
   //This shouldn't occur since we are handling the interupts as stated
   //in ddgetirqinfo when it returned handled 
   int IrqCtrl = ((SstRegs *)(kmtvInfo.pDev->RegBase + SST_3D_OFFSET))->intrCtrl;
   if (IrqCtrl & H3_VSYNC_INTERRUPT) 
   {
      _asm mov edi, DDIRQ_DISPLAY_VSYNC
   }
   if (IrqCtrl & H3_VMI_INTERRUPT) 
   {
      _asm mov edi, DDIRQ_VPORT0_VSYNC
   }

   return 0;
}


/**********************************************************************
*   DDSkipNextField
*
*   DESCRIPTION: Called when they want to skip the next field, usually
*       to undo a 3:2 pulldown but also for decreasing the frame rate.
*       The driver should not lose the VBI lines if dwVBIHeight contains
*       a valid value.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDSKIPINFO
*                   DWORD dwSize
*                   LPDDVIDEOPORTDATA video port
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDSkipNextField(void)
{

   DDSKIPINFO    *lpSkipInfo;
   _asm mov lpSkipInfo, esi
   Debug_Printf("KMVT - DDSkipNextField\n");
//_asm int 1;

   if (lpSkipInfo->dwSkipFlags & DDSKIP_SKIPNEXT)
       kmtvInfo.bSkipNextField = TRUE; 
   else if (lpSkipInfo->dwSkipFlags & DDSKIP_ENABLENEXT)
       kmtvInfo.bSkipNextField = FALSE;
   else
       return 1;

   return 0;
}

/**********************************************************************
*   DDBobNextField
*
*   DDBobNextInterleavedEvenOverlayField
*
*   DESCRIPTION: Called when "bob" is used and a VPORT VSYNC occurs that does
*       not cause a flip to occur (e.g. bobbing while interleaved).  When
*       bobbing, the overlay must adjust itself on every VSYNC, so this
*       function notifies it of the VSYNCs that it doesn't already know
*       about (e.g. VSYNCs that trigger a flip to occur).
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDBOBINFO
*                   DWORD dwSize
*                   LPDDSURFACE lpSurface
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
DWORD DDBobNextField(void)
{

   DDBOBINFO	*lpBobInfo;
   _asm mov lpBobInfo, esi

   Debug_Printf("KMVT - DDBobNextField\n");
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDSetState
*
*   DESCRIPTION: Called when the client wants to switch from bob to weave.
*       The overlay flags indicate which state to use. Only called for interleaved
*   surfaces.
*
*       NOTE: When this is called, the specified surface may not be
*       displaying the overlay (due to a flip).  Instead of failing
*       the call, change the bob/weave state for the overlay that would
*       be used if the overlay was flipped again to the specified surface.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDSTATEININFO
*                   DWORD dwSize
*                   LPDDSURFACEDATA overlay surface
*          EDI  LPDDSTATEOUTINFO
*                   DWORD dwSize
*                   DWORD dwSoftwareAutoflip
*                   DWORD dwSurfaceIndex        ; Return Current hardware autoflip
*
*   DURING:
*           EDI OverlaySuface
*           EBX Overlay Flags
*           ECX Overlay Pitch
*           EDX Overlay Src Height
* 
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDSetState(void)
{

  DDSTATEOUTINFO	*lpStateOut;
  DDSTATEININFO		*lpStateIn;
   _asm mov lpStateIn, esi
   _asm mov lpStateOut, edi

   Debug_Printf("KMVT - DDSetState\n");
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDLock
*
*   DESCRIPTION: Called when the client wants to lock the surface to
*       access the frame buffer. The driver doens't have to do anything,
*       but it can if it needs to.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDLOCKININFO
*                   DWORD dwSize
*                   LPDDSURFACEDATA surface
*          EDI  LPDDLOCKOUTINFO
*                   DWORD dwSize
*                   DWORD Pointer to a pointer to the surface
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDLock(void)
{
//   DDLOCKININFO	*lpLockInfo;
//   _asm mov lpLockInfo, esi

   Debug_Printf("KMVT - DDLock\n");
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDFlipVideoPort
*
*   DESCRIPTION: Flips the video port to the target surface.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDFLIPVIDEOPORTINFO
*                   DWORD dwSize
*                   LPDDVIDEOPORTDATA video port info
*                   LPDDSURFACEDATA current surface
*                   LPDDSURFACEDATA target surface
*                   DWORD dwFlipVPFlags
*          EDI  NULL
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
*
**********************************************************************/
DWORD DDFlipVideoPort(void)
{

   DDFLIPVIDEOPORTINFO	*lpFlipVideoPort;
   _asm mov lpFlipVideoPort, esi
   Debug_Printf("KMVT - DDFlipVideoPort\n");
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDGetPolarity
*
*   DESCRIPTION: Returns the polarity of the current field being written
*       to the specified video port.
*
*   ENTRY:
*          EAX  dwMiniVDDContext;          
*		   ESI  LPDDPOLARITYININFO
*                   DWORD dwSize
*                   LPDDVIDEOPORTDATA
*          EDI  LPDDPOLARITYOUTINFO
*                   DWORD dwSize
*                   DWORD bPolority (even field = TRUE, odd field = FALSE)
*
*   EXIT:
*          EAX  0 = success, 1 = error
*          ECX  0 = odd,     1 = even
*
*
*   MODIFIES:
*          EAX, EBX, ECX
*
**********************************************************************/
DWORD DDGetPolarity(void)
{

   DDPOLARITYININFO   *lpPolarityIn;
   DDPOLARITYOUTINFO  *lpPolarityOut;

   _asm mov lpPolarityIn, esi
   _asm mov lpPolarityOut, edi
   Debug_Printf("KMVT - DDGetPolarity\n");
//_asm int 1;
   
   if ((((SstIORegs *)(kmtvInfo.pDev->RegBase + SST_IO_OFFSET))->vidCurrentLine & H4_VMI_FIELD_MASK ) >> 18)
       lpPolarityOut->bPolarity = FALSE;     //The last field written is even, the current one is odd
   else
       lpPolarityOut->bPolarity = TRUE;      //The last field written is odd, the current one is even

   return 0;
}

/**********************************************************************
*   DDSkipPattern
*
*  DESCRIPTION: Sets the skip pattern that is to start on the next video field.
*                If device returns an error, DDraw perofms the appropriate skipping
*                using IRQ logic and vddSkipNextField function
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDSETSKIPINFO
*          EDI  0
*
*
*   EXIT:
*          EAX  0 = success, 1 = error
*          ECX  0 = odd,     1 = even
*
*
*   MODIFIES:
*          EAX, EBX, ECX
*
**********************************************************************/
DWORD DDSetSkipPattern(void)
{

   DDSKIPINFO	*lpSkipInfo;
   _asm mov lpSkipInfo, esi
   Debug_Printf("KMVT - DDSetSkipPattern\n");
//_asm int 1;
   return 0;
}

/**********************************************************************
*   DDGetCurrentAutoflip
*
*   DESCRIPTION: Returns the current surface receiving data from the
*       video port while autoflipping is taking palce.  Only called when
*   hardware autoflipping.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*          EDI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*                   DWORD               dwSurfaceIndex
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDGetCurrentAutoflip(void)
{

   DDGETAUTOFLIPININFO	*lpAutoFlipIn;
   DDGETAUTOFLIPOUTINFO *lpAutoFlipOut;
   BYTE        bNewBuf;

   _asm mov lpAutoFlipIn, esi
   _asm mov lpAutoFlipOut, edi
//   Debug_Printf("KMVT - DDGetCurrentAutoflip\n");

   bNewBuf = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase + SST_IO_OFFSET))->vidCurrentLine) >> 16) & 0x3;

   lpAutoFlipOut->dwSurfaceIndex = bNewBuf;

   return 0;
}

/**********************************************************************
*   DDGetPreviousAutoflip
*
*   DESCRIPTION: Returns the surface that received the data from the
*       previous field of video port while autoflipping is taking palce. Only
*   called for hardware autoflipping.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*          EDI  LPDDGETAUTOFLIPINFO
*                   DWORD               dwSize
*                   DWORD               dwSurfaceIndex
*
*   EXIT:
*          EAX  0 = success, 1 = error
*
**********************************************************************/
DWORD DDGetPreviousAutoflip(void)
{
   DDGETAUTOFLIPININFO	*lpAutoFlipIn;
   DDGETAUTOFLIPOUTINFO *lpAutoFlipOut;
   BYTE bNewBuf, bBufModeSelect;

   _asm mov lpAutoFlipIn, esi
   _asm mov lpAutoFlipOut, edi
   Debug_Printf("KMVT - DDGetPreviousAutoflip\n");
//_asm int 1;

   //Query buffer mode - single, double or triple
   bBufModeSelect = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase + SST_IO_OFFSET))->vidInFormat) >> 8) & 0x3;

   //Get Current buffer number
   bNewBuf = (BYTE)((((SstIORegs *)(kmtvInfo.pDev->RegBase + SST_IO_OFFSET))->vidCurrentLine) >> 16) & 0x3;
   if (bNewBuf == 0)   {
     //to wrap around just use bufmodeselect since it will have the large
     //available buffer used before wrapping on current
     bNewBuf = bBufModeSelect;
   }
   else {
     //just subtract one
     bNewBuf--;
   }

   lpAutoFlipOut->dwSurfaceIndex = bNewBuf;
   Debug_Printf("KMVT - DDGetPreviousAutoflip - %d\n", bNewBuf);


   return 0;
}

/**********************************************************************
*   DDTransfer
*
*   DESCRIPTION: tells the driver to bus master data from a surface to the buffer
*                specified in the memory descriptor list (MDL).  The MDL is defined
*                in the WDM documentation.   Called at HW interrupt time
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDTRANSFERININFO
*
*          EDI  LPDDTRANSFEROUTINFO
*
*   EXIT:
*          EAX  0 = success, 1 = error
*          ECX  0 = odd,     1 = even
*
*
*   MODIFIES:
*          EAX, EBX, ECX
*
**********************************************************************/
DWORD DDTransfer(void)
{
   DDTRANSFERININFO      *lpTransferIn;
   DDTRANSFEROUTINFO     *lpTransferOut;
   DDSURFACEDATA         *lpSurfaceData;
   PMDL                   pMDL;
   DWORD                  dwBufferSize;
   BYTE                  *lpBufferStart, *lpBufferDest;
   DWORD                  j, dwRemainingSrc;
// BYTE 		 bNewBuf;

   _asm mov lpTransferIn, esi
   _asm mov lpTransferOut, edi
 //_asm int 1;
//    return 0;
  
   lpSurfaceData = (DDSURFACEDATA *)lpTransferIn->lpSurfaceData;
	lpTransferOut->dwSize = sizeof (DDTRANSFEROUTINFO ); 
	if ((((SstIORegs *)(kmtvInfo.pDev->RegBase + SST_IO_OFFSET))->vidCurrentLine & H4_VMI_FIELD_MASK ) >> 18)
       lpTransferOut->dwBufferPolarity = FALSE;     //The last field written is even, the current one is odd
   else
       lpTransferOut->dwBufferPolarity = TRUE;      //The last field written is odd, the current one is even
   pMDL = lpTransferIn->lpDestMDL;
   if (pMDL==0){
     Debug_Printf("KMVT - MDL is NULL - return\n");    
     return 0;
   }
   kmtvInfo.dwTransferID = lpTransferIn->dwTransferID;
//   Debug_Printf("KMVT - DDTransfer - ID=%x Num=%d flags %08.8x\n",
//	   kmtvInfo.dwTransferID,kmtvInfo.Num++,lpTransferIn->dwTransferFlags);



//#define QUEUEING_TRANSFERS
#ifdef QUEUING_TRANSFERS
   dwCurrentTransfer = kmtvInfo.CurrentTransferIndex;
   lpTransferData = (PTRANSFER_DATA)kmtvInfo.TransferArray[dwCurrentTransfer];

   //check if we are filled up
   if (lpTransferData->fFilled)
      return 1;

   lpTransferData->dwTransferID = lpTransferIn->dwTransferID;
   lpTransferData->ddSurfaceData = *lpSurfaceData;
   lpTransferData->DestMDL = *pMDL;
   lpTransferData->dwStartLine = lpSurfaceData->dwStartLine;
   lpTransferData->dwEndLine = lpSurfaceData->dwEndLine;
   lpTransferData->dwSurfaceOffset = lpSurfaceData->dwSurfaceOffset;
   lpTransferData->lPitch = lpSurfaceData->lPitch;
   lpTransferData->dwFormatBitCount = lpSurfaceData->dwFormatBitCount;
   lpTransferData->fFilled = 1;

   kmtvInfo.CurrentTransferIndex++;
   if (kmtvInfo.CurrentTransferIndex >= MAX_TRANSFERS)
      kmtvInfo.CurrentTransferIndex = 0;

   //Now need to setup up DPC events to do I/O transfers at
   //less busy time

#else

   //Get size of buffer which will the number of lines
   // times the byte depth * the width
   dwBufferSize = 	(lpTransferIn->dwEndLine - lpTransferIn->dwStartLine + 1) * 
			lpSurfaceData->lPitch;

   //Now calculate the buffer starting address
   lpBufferStart = (BYTE *)(lpSurfaceData ->fpLockPtr);

   //To copy from the destination
   dwRemainingSrc = dwBufferSize;
   
   //in the case of vddtransfer we need to copy the data as specified 
   //in the linked list of MDL
   for (j=0; dwRemainingSrc;)
   {
		if (pMDL->ByteCount < dwBufferSize)
			j = pMDL->ByteCount;
		else
			j = dwBufferSize;
		 //Now get the pointer to the memory destination
		 lpBufferDest = (BYTE *)(pMDL->lpMappedSystemVa);
		if (lpTransferIn->dwTransferFlags & DDTRANSFER_INVERT)
		{
			BYTE *lpLine = lpBufferStart + (lpTransferIn->dwStartLine * lpSurfaceData->lPitch);
			BYTE *lpDest = lpBufferDest;
			int curline = 0;
			int bytes = 0;
			int linecnt = (lpTransferIn->dwEndLine - lpTransferIn->dwStartLine + 1);
			for (curline = 0, bytes = 0;
					curline < linecnt && bytes < j;
					curline++, lpLine -= lpSurfaceData->lPitch, 
						lpDest += lpSurfaceData->lPitch, bytes += lpSurfaceData->lPitch)
				memcpy (lpBufferDest, lpLine, lpSurfaceData->lPitch);

		}
		else
			memcpy (lpBufferDest, lpBufferStart, j);
 
     //update buffer pointer to next copy point
     lpBufferStart += j;

     dwRemainingSrc -= j;

     pMDL = pMDL->MdlNext;

     if ((pMDL == 0) && (dwRemainingSrc > 0))
//      return 1;
        return 0;
     if (pMDL==0)   
		return 0;
  

   }    
#endif


   return 0;
}

/**********************************************************************
*   DDGetTransferStatus
*
*   DESCRIPTION: Returns the transfer id in the DDGETTRANSFERSTATUSOUTINFO 
*                structure.  This function is used to determined which 
*                hardware bus master has completed.
*
*   ENTRY:
*          EAX  dwMiniVDDContext
*          ESI  LPDDGETTRANSFERSTATUSOUTINFO
*
*          EDI  0
*
*
*   EXIT:
*          EAX  0 = success, 1 = error
*          ECX  0 = odd,     1 = even
*
*
*   MODIFIES:
*          EAX, EBX, ECX
*
**********************************************************************/
DWORD DDGetTransferStatus(void)
{
   DDGETTRANSFERSTATUSOUTINFO	*lpTransferStatus;
   TRANSFER_DATA   CurrentTransfer;

   _asm mov lpTransferStatus, edi
//   Debug_Printf("KMVT - DDGetTransferStatus - ID=%x Num=%d\n",kmtvInfo.dwTransferID,kmtvInfo.Num);
   lpTransferStatus->dwTransferID = kmtvInfo.dwTransferID;
   lpTransferStatus->dwSize = sizeof (DDGETTRANSFERSTATUSOUTINFO);
   kmtvInfo.dwTransferID = 0;
   return 0;
}
#endif

#undef IS_32
#undef MM
#define THUNK32
