/* -*-c++-*- */
/* $Header: h3irq.c, 1, 9/11/99 10:16:06 PM PDT, StarTeam VTS Administrator$ */
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
** File name:   h3irq.c
**
** Description: Interrupt handler and support services.
**
** $Revision: 1$
** $Date: 9/11/99 10:16:06 PM PDT$
**
** $History: h3irq.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:55p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 18  *****************
** User: Stb_lpost    Date: 3/30/99    Time: 7:53a
** Updated in $/devel/h3/win95/dx/minivdd
** V3TV Video Capture Fixes
** 
** *****************  Version 17  *****************
** User: Stb_lpost    Date: 3/19/99    Time: 12:43p
** Updated in $/devel/h3/win95/dx/minivdd
** Kernel Mode V3TV ifdefd code
** 
** *****************  Version 16  *****************
** User: Xingc        Date: 3/04/99    Time: 6:33p
** Updated in $/devel/h3/Win95/dx/minivdd
** For BOB flip overlay based on even and odd field
** 
** *****************  Version 15  *****************
** User: Agus         Date: 2/01/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move IS_H4 macro define to h3vdd.h
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 1/21/99    Time: 5:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Coded EnableInterrupt and DisableInterrupt
** 
** *****************  Version 13  *****************
** User: Andrew       Date: 1/18/99    Time: 9:13p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem with MultiMonitor where we were not sharing IRQ's
** correctly.  This happens due to the fact that the device can be
** disabled in which case we will think that we got a IRQ.  Changed to
** check that lpDriverData is not null before we use it.
** 
** *****************  Version 12  *****************
** User: Agus         Date: 1/15/99    Time: 6:12p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed Avenger's video-in buffer status bits location
** 
** *****************  Version 11  *****************
** User: Agus         Date: 1/15/99    Time: 4:07p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added check for Avenger (H4) when doing manual overlay flipping.
** 
** *****************  Version 10  *****************
** User: Michael      Date: 1/07/99    Time: 1:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 12/09/98   Time: 5:30p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some code to set hIRQ to zero after we force default behavior
** 
** *****************  Version 8  *****************
** User: Agus         Date: 6/30/98    Time: 4:57p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added overlay manual flip to handle overlay cropping properly
** 
** *****************  Version 7  *****************
** User: Stuartb      Date: 6/11/98    Time: 8:34a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added retrace interrupt support, check multiple banshees, share
** interrupt correctly.
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 6/05/98    Time: 4:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** Adding handling for vertical retrace interrupts.
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "devtable.h"
#include "h3irq.h"
#undef VDDONLY

#ifdef V3TV
#include <ddkmmini.h>
#endif

#pragma VxD_LOCKED_DATA_SEG
#pragma VxD_LOCKED_CODE_SEG


extern DWORD GETGBL_dwOvlOffset(DWORD);

extern PDEVTABLE pVGADevTable;

#ifdef V3TV
//h3 intrCtrl bitmasks
#define H3_VMI_INT_ENABLE                   0x00200000
#define H3_VMI_INTERRUPT		    0x00800000
#define H3_VSYNC_INT_ENABLE		    0x00000004
#define H3_VSYNC_INTERRUPT		    0x00000100


#include "kmvt.h"
extern KMTVDATA  kmtvInfo;   
#endif  //V3TV

/*----------------------------------------------------------------------
Function name:  _VWIN32_PulseWin32Event

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
_VWIN32_PulseWin32Event( DWORD dwEvent )
{
    __asm mov eax,dwEvent;
    VxDCall(_VWIN32_PulseWin32Event);
}


/*----------------------------------------------------------------------
Function name:  Schedule_VM_Event

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
Schedule_VM_Event( HVM hVM, DWORD dwCallback, DWORD dwRefval )
{
    __asm mov ebx,hVM;
    __asm mov esi,dwCallback;
    __asm mov edx,dwRefval;
    VMMCall(Schedule_VM_Event);
}

#if 0
HIRQ hIRQ;
VID IRQDesc;
DWORD dwVCount;
BOOL bVMInts;
#endif

BYTE bOldBuf = 0xff;


/*----------------------------------------------------------------------
Function name:  InitializeInterrupts

Description:    Establish handler for interrupts.
                
Information:    

Return:         BOOL    TRUE for success or,
                        FALSE for failure.
----------------------------------------------------------------------*/
BOOL InitializeInterrupts( int iIRQ , PDEVTABLE pDev)
{
    // disable before grab

    pDev->bVMInts = TRUE;
    DisableInterrupts(pDev);

    // connect to interrupt

    pDev->IRQDesc.VID_IRQ_Number = iIRQ;
    pDev->IRQDesc.VID_Options = VPICD_OPT_CAN_SHARE;
    pDev->IRQDesc.VID_Hw_Int_Proc = (ULONG)InterruptHandler;
    if( pDev->hIRQ = VPICD_Virtualize_IRQ( &pDev->IRQDesc ) )
    {
        VPICD_Physically_Unmask( pDev->hIRQ );

        // now enable

        EnableInterrupts(pDev);
        return TRUE;
    }
    return FALSE;
}


/*----------------------------------------------------------------------
Function name:  InterruptsToDOS

Description:    Give up handling interrupt for dos VM.
                
Information:    Uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID 
InterruptsToDOS( VOID )
{
    __asm pushfd;
    __asm cli;
    if( pVGADevTable && pVGADevTable->hIRQ && pVGADevTable->bVMInts )
    {
        pVGADevTable->bVMInts = FALSE;
        DisableInterrupts(pVGADevTable);
        VPICD_Force_Default_Behavior( pVGADevTable->hIRQ );
        pVGADevTable->hIRQ = 0x0;
    }
    __asm popfd;
}


/*----------------------------------------------------------------------
Function name:  InterruptsToSVM

Description:    Re-initialize handler.
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID InterruptsToSVM( VOID )
{
    if( pVGADevTable->IRQDesc.VID_IRQ_Number && !pVGADevTable->bVMInts )
        InitializeInterrupts( pVGADevTable->IRQDesc.VID_IRQ_Number, pVGADevTable );
}


static FxU16 LastServicedBanshee;
DWORD hInIRQ;


/*----------------------------------------------------------------------
Function name:  InterruptHandler

Description:    Handle the interrupt.
                
Information:    Uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID 
InterruptHandler( VOID )
{
	FxI16 count;
	DWORD dwReg, sst3dRegs;
   DWORD       dwOvlBufAddr;  // Overlay buffer hw address
   PDEVTABLE   pDevTable;     // Ptr to device table array
   DWORD       dwOvlOffset;
   DWORD       PllReg;
   DWORD       dwFieldStatus;
   BYTE        bNewBuf;
    

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        mov     hInIRQ,eax;
//      mov     hVM,ebx;
    }

	for (count = (FxI16)dwNumDevices; --count >= 0;  )
	{
		if (++LastServicedBanshee >= dwNumDevices)
			LastServicedBanshee = 0;
		if (sst3dRegs = DevTable[LastServicedBanshee].RegBase)
		{
         PllReg = ((SstIORegs *)sst3dRegs)->pllCtrl0;
		   sst3dRegs += SST_3D_OFFSET;
			dwReg = ((SstRegs *)sst3dRegs)->intrCtrl;

         // If IO is disabled on this device
         // then the HOST/PCI Bridge will do a Master Abort
         // For a read cycle the HOST/PCI Bridge is required to return all 1's
         // This will check for this occurence 
#ifdef V3TV
			// VMI_IRQ_USAGE
			if ((dwReg & (0x300 | H3_VMI_INTERRUPT)) && (0xFFFFFFFF != PllReg))
				break;
#else
      			if ((dwReg & 0x300) && (0xFFFFFFFF != PllReg))
      				break;
#endif
		}
	}
   if (count < 0)
   {
      // naked epilog code, pass on this interrupt (not for us)
      __asm
      {
         stc;
         mov     esp,ebp;
         pop     ebp;
         ret;
      }
   }

   //Get the pointer to Banshee device generating the interrupt
   pDevTable = &DevTable[LastServicedBanshee];

#if 0 // KMW
    if( dwIPend & softrapen_INT_OCCURRED )   // DMA interrupt
    {
        DWORD dwSoftrap, dwQPos;

        dwSoftrap = mgaReadDWord( DWG1_SOFTRAP );
        if( !(dwSoftrap & 0x80000000) )
        {
            pMMIf->wFlags &= ~MMMIF_DMAINPROGRESS;
            // count given by register
            pMMIf->dwDMACount = dwSoftrap >> 2;
        }
        else
        {
            switch( dwSoftrap )
            {
            case MMMST_STOP:
                pMMIf->wFlags &= ~MMMIF_DMAINPROGRESS;
                // find count of completed DMA secondaries, and update
                dwQPos = mgaReadDWord( HST_PRIMADDRESS ) & ~3;
                dwQPos -= pMMIf->dwDMACQPhys; // now bytes from start of Q
                dwQPos = dwQPos/sizeof(MMDMAENTRY) + 1;
                pMMIf->dwDMACount = (pMMIf->dwDMACount + dwQPos) & 0xFFFF;
                break;

            case MMMST_LOOP:    // restart primary at start of CQ
                // Q finished, so update dma count by number of entries
                pMMIf->dwDMACount = (pMMIf->dwDMACount + MMDMAENTRIES) & 0xFFFF;
                pMMIf->DMAStop.dwVal[0] = MMMST_STOP;
                mgaWriteDWord( HST_PRIMADDRESS, pMMIf->dwDMACQPhys );
                mgaWriteDWord( HST_PRIMEND, pMMIf->dwDMACQPhys + 
                    ((MMDMAENTRIES+1)*sizeof(MMDMAENTRY))/sizeof(DWORD) );
                break;

            default:            // update free
                pMMIf->wFlags &= ~MMMIF_DMAINPROGRESS;
                break;
            }
        }
    }

    if( dwIPend & vsyncpen_INT_OCCURRED )   // vsync interrupt
    {
        BYTE bIndexSave;

        dwVCount++;

        // save CRTC index
        bIndexSave = mgaReadByte( VgaReg|VGA_CRTC_INDEX );

        // update start address if required
        if( pMMIf->wFlags & MMMIF_PLEASEFLIP )
        {
            BYTE bExtIndexSave;

            pMMIf->wFlags &= ~MMMIF_PLEASEFLIP;
            mgaWriteByte( VgaReg|VGA_CRTC_INDEX, 13); 
            mgaWriteByte( VgaReg|VGA_CRTC_DATA, pMMIf->dwFlipToOffset );
            mgaWriteByte( VgaReg|VGA_CRTC_INDEX, 12); 
            mgaWriteByte( VgaReg|VGA_CRTC_DATA, pMMIf->dwFlipToOffset >> 8 );
            bExtIndexSave = mgaReadByte( VgaReg|VGA_CRTCEXT_INDEX ); 
            mgaWriteByte( VgaReg|VGA_CRTCEXT_INDEX, 0); 
            mgaWriteByte( VgaReg|VGA_CRTCEXT_DATA, 
                (mgaReadByte( VgaReg|VGA_CRTCEXT_DATA ) & 0xf0) |
                (pMMIf->dwFlipToOffset >> 16)) ;
            mgaWriteByte( VgaReg|VGA_CRTCEXT_INDEX, bExtIndexSave);
        }
        // post event if needed
        if( pMMIf->wFlags & MMMIF_VSYNCEVENT )
        {
            Schedule_VM_Event( hSysVM, (DWORD)SignalVMEvent, 0 );
        }
        mgaWriteByte( VgaReg|VGA_CRTC_INDEX, 17); 
        mgaWriteByte( VgaReg|VGA_CRTC_DATA, 
            mgaReadByte( VgaReg|VGA_CRTC_DATA ) & ~0x30 );
        mgaWriteByte( VgaReg|VGA_CRTC_DATA, 
            mgaReadByte( VgaReg|VGA_CRTC_DATA ) | 0x10 );
        mgaWriteByte( VgaReg|VGA_CRTC_INDEX, bIndexSave );
    }

    if( dwIPend & pickpen_INT_OCCURRED )   // pick interrupt
    {
        pMMIf->wFlags |= MMMIF_PICKED;
    }

    // reset MGA interrupt sources

    mgaWriteDWord( HST_ICLEAR, dwIPend & ~vsyncpen_MASK );

#endif /* #if 0 KMW */

#ifdef V3TV
    // VMI_IRQ_USAGE
	if(dwReg & H3_VMI_INTERRUPT)   // vmi interrupt
	{
//Debug_Printf(" VMI Interrupt Num=%d \n",kmtvInfo.Num);
		// clear vmi interrupt - always do this
		dwReg &= ~H3_VMI_INTERRUPT;
		((SstRegs *)sst3dRegs)->intrCtrl = (0x80000000 | dwReg);
		// do actions of interest on video port VREF rising edge

		if (kmtvInfo.IRQCallback && (kmtvInfo.dwIRQSources & DDIRQ_VPORT0_VSYNC))
		{
		   _asm {
			  push ebp
			  push esi
			  push eax
			  push ebx
			  mov eax, DDIRQ_VPORT0_VSYNC
			  mov ebx, kmtvInfo.Context
			  call [kmtvInfo.IRQCallback]
			  pop ebx
			  pop eax
			  pop esi
			  pop ebp          
		   } 
		}
		if (kmtvInfo.IRQCallback && kmtvInfo.dwTransferID)
		{
		   _asm {
			  push ebp
			  push esi
			  push eax
			  push ebx
			  mov eax, DDIRQ_BUSMASTER
			  mov ebx, kmtvInfo.Context
			  call [kmtvInfo.IRQCallback]
			  pop ebx
			  pop eax
			  pop esi
			  pop ebp          
		   } 
		}
	}
#if 0
	////////////////////////////////////////////////////////////////////
	if(dwReg & 0x0300)   // VSYNC interrupt
	{
		if (kmtvInfo.IRQCallback && (kmtvInfo.dwIRQSources & DDIRQ_DISPLAY_VSYNC))
		{
		   _asm {
			  push ebp
			  push esi
			  push eax
			  push ebx
			  mov eax, DDIRQ_DISPLAY_VSYNC
			  mov ebx, kmtvInfo.Context
			  call [kmtvInfo.IRQCallback]
			  pop ebx
			  pop eax
			  pop esi
			  pop ebp          
		   } 
		}
#endif

    //Overlay flipping
    //------------------------------------------------------------------------------
    // Check to make sure that lpDriverData is not bogus also
    else
#endif //V3TV
    if (( pDevTable ) && (pDevTable->lpDriverData))
    {
       //If overlay offset is not zero do manual flipping
       if ( (dwOvlOffset = GETGBL_dwOvlOffset(pDevTable->lpDriverData)) )
       {
          if ( IS_H4(pDevTable) )
          {
            dwFieldStatus = *(DWORD*)(pDevTable->RegBase + H4_VID_IN_STATUS_CURLINE);
            bNewBuf = (BYTE) ((dwFieldStatus >> 16 ) & 0x3);
            dwFieldStatus &= 0x40000;

          }
          else
          {
            dwFieldStatus = *(DWORD*)(pDevTable->RegBase + H3_VID_IN_STATUS);
            bNewBuf = (BYTE)(dwFieldStatus & 0x3);
            dwFieldStatus &= 0x4;
          }

          dwOvlOffset &= 0x7FFFFFFF;

          switch ( bNewBuf )
          {
            case 0:
               dwOvlBufAddr = *(DWORD*)(pDevTable->RegBase + H3_VID_IN_ADDR0);
               break;
            case 1:
               dwOvlBufAddr = *(DWORD*)(pDevTable->RegBase + H3_VID_IN_ADDR1);
               break;
            case 2:
               dwOvlBufAddr = *(DWORD*)(pDevTable->RegBase + H3_VID_IN_ADDR2);
               break;
            default:    
               dwOvlBufAddr = 0;
               break;
          }
              
          if ( dwOvlBufAddr && bNewBuf != bOldBuf )
          {
            dwOvlBufAddr += dwOvlOffset;
            
          if( !(*(DWORD*)(pDevTable->RegBase + H3_VID_PROC_CFG) &
             SST_OVERLAY_DEINTERLACE_EN))
            dwFieldStatus = 0;      //don't set even/odd field if not BOB

            //Direct write to swap overlay buffer
          if(dwFieldStatus)
            ((SstRegs *)(pVGADevTable->RegBase + SST_3D_OFFSET))->leftOverlayBuf
                = dwOvlBufAddr |0x80000000;     //set evne/odd field
           else
            ((SstRegs *)(pVGADevTable->RegBase + SST_3D_OFFSET))->leftOverlayBuf = dwOvlBufAddr;
            ((SstRegs *)(pVGADevTable->RegBase + SST_3D_OFFSET))->swapbufferCMD = 0;
            
            bOldBuf = bNewBuf;
          }   
       }
           
       //------------------------------------------------------------------------------

	// clear vertical interrupts only
	((SstRegs *)sst3dRegs)->intrCtrl = 0x80000000 | (dwReg & ~0x300);

    }

    // ack it and exit
    VPICD_Phys_EOI (hInIRQ);
    __asm
    {
        clc;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}


/*----------------------------------------------------------------------
Function name:  SignalVMEvent

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID
SignalVMEvent( VOID )
{
#ifdef KMW
    _VWIN32_PulseWin32Event( pMMIf->dwVSyncEvent );
#endif // #ifdef KMW
}

/*----------------------------------------------------------------------
Function name:  EnableInterrupts

Description: Used to Physical Enable Interrupts on the 3DFX Device    
                
Information:    

Return:         
----------------------------------------------------------------------*/
VOID EnableInterrupts(PDEVTABLE pDevTable)
{
   DWORD dwReg;
   DWORD sst3dRegs = pDevTable->RegBase;
   DWORD PllReg;         

   PllReg = ((SstIORegs *)sst3dRegs)->pllCtrl0;

   // If IO is disabled on this device
   // then the HOST/PCI Bridge will do a Master Abort
   // For a read cycle the HOST/PCI Bridge is required to return all 1's
   // This will check for this 
   if (0xFFFFFFFF == PllReg)    
      {
      // This is not good ---
      // Why would we try to do something on a card that is disabled?
#ifdef DEBUG
      _asm {int 03}
#endif
      return;
      }

   sst3dRegs += SST_3D_OFFSET;
	dwReg = ((SstRegs *)sst3dRegs)->intrCtrl;

   // This is critical code
   // We need to make this a atomic operation since the flags
   // can get modified on a asynchronous basis
   __asm pushfd;
   DISABLE_INTERRUPTS();
   dwReg |= pDevTable->dwIMask;
   
   ((SstRegs *)sst3dRegs)->intrCtrl = dwReg;
   __asm popfd;
}

/*----------------------------------------------------------------------
Function name:  DisableInterrupts

Description: Used to Physical Disable Interrupts on the 3DFX Device    
                
Information:    

Return:         
----------------------------------------------------------------------*/
VOID DisableInterrupts(PDEVTABLE pDevTable)
{
   DWORD dwReg;
   DWORD sst3dRegs = pDevTable->RegBase;
   DWORD PllReg;         

   PllReg = ((SstIORegs *)sst3dRegs)->pllCtrl0;

   // If IO is disabled on this device
   // then the HOST/PCI Bridge will do a Master Abort
   // For a read cycle the HOST/PCI Bridge is required to return all 1's
   // This will check for this 
   if (0xFFFFFFFF == PllReg)    
      {
      // This is not good ---
      // Why would we try to do something on a card that is disabled?
#ifdef DEBUG
      _asm {int 03}
#endif
      return;
      }

   sst3dRegs += SST_3D_OFFSET;
	dwReg = ((SstRegs *)sst3dRegs)->intrCtrl;

   dwReg &= ~(H3_IMASK);
   ((SstRegs *)sst3dRegs)->intrCtrl = dwReg;
}


