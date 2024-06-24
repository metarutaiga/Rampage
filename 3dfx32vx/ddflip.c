/* -*-c++-*- */
/* $Header: ddflip.c, 20, 11/2/00 1:39:43 PM PST, Miles Smith$ */
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
** File Name:    DDFLIP.C
**
** Description:  Direct Draw's ddflip and supporting functions.
**
** $Revision: 20$
** $Date: 11/2/00 1:39:43 PM PST$
**
** $History: ddflip.c $
** 
** *****************  Version 33  *****************
** User: Einkauf      Date: 9/08/99    Time: 5:41p
** Updated in $/devel/sst2/Win95/dx/dd32
** CMDFIFO only: MOP and wait for fifo empty before changing video base
** address; remove write of swapcount to make same as non-CMDFIFO compile
** 
** *****************  Version 32  *****************
** User: Pratt        Date: 9/02/99    Time: 8:38a
** Updated in $/devel/R3/Win95/dx/dd32
** removed IFDEF R21 and changed IFNDEF r21 to IF(0)
** 
** *****************  Version 31  *****************
** User: Einkauf      Date: 8/31/99    Time: 1:59p
** Updated in $/devel/sst2/Win95/dx/dd32
** DirectX CMD FIFO
** 
** *****************  Version 30  *****************
** User: Mconrad      Date: 8/10/99    Time: 9:48p
** Updated in $/devel/sst2/Win95/dx/dd32
** Fix warnings.
** 
** *****************  Version 29  *****************
** User: Xingc        Date: 7/22/99    Time: 6:22p
** Updated in $/devel/sst2/Win95/dx/dd32
** Put my last change back
** 
** *****************  Version 28  *****************
** User: Peterm       Date: 7/22/99    Time: 4:48p
** Updated in $/devel/sst2/Win95/dx/dd32
** cleaned up csim server flip backdoor
** 
** *****************  Version 26  *****************
** User: Agus         Date: 7/16/99    Time: 3:17p
** Updated in $/devel/sst2/Win95/dx/dd32
** Implement ddflip code to flip primary surfaces using video register and
** call SST2 device service to update the registers
** 
** *****************  Version 25  *****************
** User: Xingc        Date: 6/14/99    Time: 2:52p
** Updated in $/devel/sst2/Win95/dx/dd32
** Put R2.1 overlay and vpe code back. Add mulit-mon support.
** 
** *****************  Version 24  *****************
** User: Peterm       Date: 6/12/99    Time: 3:17p
** Updated in $/devel/sst2/Win95/dx/dd32
** fixed bug causing dib busy bit to always be set
** 
** *****************  Version 23  *****************
** User: Peterm       Date: 6/03/99    Time: 11:24p
** Updated in $/devel/sst2/Win95/dx/dd32
** modified to run with H3 tot (adds multimon, various bug fixes, and many
** structural deltas)
** 
** *****************  Version 84  *****************
** User: Andrew       Date: 3/30/99    Time: 10:22a
** Updated in $/devel/h3/Win95/dx/dd32
** Added Code to exit early if we are in Low Power Mode
** 
** *****************  Version 83  *****************
** User: Andrew       Date: 3/17/99    Time: 4:40p
** Updated in $/devel/h3/Win95/dx/dd32
** Added in a ifdef so that Fifo is set as 15 8 for avenger and 9 & 2 for
** Banshee
** 
** *****************  Version 82  *****************
** User: Andrew       Date: 3/17/99    Time: 2:49p
** Updated in $/devel/h3/Win95/dx/dd32
** Changed FifoThresh to 15 & 8
*/

// STB Begin Changes
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
// STB End Changes


/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include "precomp.h"
#include "ddovl32.h"
#include "fifomgr.h"

//Needed for running on CSimServer 
#include "iSST2.h"
#include "iR3.h"
#include "iHydra.h"
#include "iRage.h"

/**************************************************************************
* D E F I N E S
***************************************************************************/

#define SHRINKOVERLAYSURFACE(arg1,arg2) shrinkOverlaySurface(arg1,arg2)

#define BUMPAGP(arg1,arg2) bumpAgp(arg1,arg2)
#define FLUSHAGP(arg1) flushAgp(arg1)
#define MYWRAPAGP(arg1,arg2) myWrapAgp(arg1,arg2)
#define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg1,arg2)

extern DWORD SHRINKOVERLAYSURFACE(NT9XDEVICEDATA*, LPDDRAWI_DDRAWSURFACE_LCL);

#define MAGIC_SCRIPT

/*----------------------------------------------------------------------
Function name: FxgetFlipStatus

Description:   checks if the most recent flip has completed

Return:        BOOL

               TRUE  - Flip in progress
			   FALSE - Flip completed
----------------------------------------------------------------------*/

BOOL
FXGETFLIPSTATUS(NT9XDEVICEDATA *ppdev)
{

    if (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY))
    {
        // not using overlay - if status bits zero, no update is pending so flip is complete
        if ( GET(ghwVD->vdVoPsStatus0) & SST_VO_PD_UPDATE )
        {
          return (TRUE);
        }
        else
        {
          return (FALSE);
        }
    }
    else
    {
        // using overlay - hardware is counting swaps - SLI is always in overlay
        if (_DD(ddLastSwapCount) > READSWAPCOUNT(ppdev))
        {
          return (TRUE);
        }
        else
        {
          return (FALSE);
        }
    }

}// FxgetFlipStatus

/*----------------------------------------------------------------------
Function name: FxInVerticalBlank

Description:   checks if the chip is in vertical blank

Return:        BOOL

               TRUE  - Vertical Retrace Active
			   FALSE - Display Active
----------------------------------------------------------------------*/

BOOL
FXINVERTICALBLANK(NT9XDEVICEDATA *ppdev)
{

    return ( GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_VBLNK );

}// FxInVerticalBlank

/*----------------------------------------------------------------------
Function name: FxGetScanLine

Description:   Returns vertical scanline position

Return:        DWORD position - vertical scanline position
----------------------------------------------------------------------*/

DWORD
FXGETSCANLINE(NT9XDEVICEDATA *ppdev)
{

    return ( GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_YLINE );

}// FxGetScanLine


/*----------------------------------------------------------------------
Function name: DdWaitForVerticalBlank

Description:
               Three cases:

               DDWAIT_I_TESTVB     - vblank status request

               DDWAITVB_BLOCKBEGIN - wait for vertical retrace to end and
			                         for the display period to end

               DDWAUTVB_BLOCKEND   - wait for vblank interval to end

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED    - waited for vblank period end
               DDHAL_DRIVER_NOTHANDLED - invalid dwvb->dwFlags
----------------------------------------------------------------------*/

DWORD __stdcall
DdWaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pwvb)
{
  DD_ENTRY_SETUP(pwvb->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "WaitForVerticalBlank32" ));
  DUMP_WAITFORVERTICALBLANKDATA(ppdev, DEBUG_DDGORY, pwvb );
  #endif

  switch( pwvb->dwFlags )
  {
    case DDWAITVB_I_TESTVB:
      // return current vertical blank status
      pwvb->bIsInVB = GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_VBLNK;
      pwvb->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;

    case DDWAITVB_BLOCKBEGIN:
      // wait until the vertical retrace is over, then wait for the display period to end.
      while(  GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_VBLNK );
      while(!(GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_VBLNK));
      pwvb->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;

    case DDWAITVB_BLOCKEND:
      // wait for the vblank interval to end.
      while(!(GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_VBLNK));
      while(  GET(ghwVD->vdVoPsStatus1) & SST_VO_PS_VBLNK );
      pwvb->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
  }

  return DDHAL_DRIVER_NOTHANDLED;

}// DdWaitForVerticalBlank

/*----------------------------------------------------------------------
Function name:  DdGetScanLine

Description:   If a vertical blank is in progress the scan line is in
			   indeterminant. If the scan line is indeterminant we return
			   the error code DDERR_VERTICALBLANKINPROGRESS.
			   Otherwise we return the scan line and a success code
			
Return:        DWORD DDRAW result

               DDERR_VERTICALBLANKINPROGRESS - scanline is in progress
			   DDHAL_DRIVER_HANDLED          - scanline returned in
			                                   pgsl->dwScanLine
----------------------------------------------------------------------*/

DWORD __stdcall
DdGetScanLine( LPDDHAL_GETSCANLINEDATA pgsl )
{
  DD_ENTRY_SETUP(pgsl->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "GetScanLine32" ));
  DUMP_GETSCANLINE(ppdev, DEBUG_DDGORY, pgsl );
  #endif

  if (FXINVERTICALBLANK(ppdev))
  {
    pgsl->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
  }
  else
  {
    pgsl->dwScanLine = FXGETSCANLINE(ppdev);

    // if vblank low, scanline might still be outside displayable range
    if ( (pgsl->dwScanLine == 0) ||
         (pgsl->dwScanLine >= _DD(fbiHeight)) )
    {
        pgsl->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
    }
    else
    {
        pgsl->ddRVal = DD_OK;
    }
    
  }
  return DDHAL_DRIVER_HANDLED;

}// DdGetScanLine

#define BUSY_BIT        0x0004  // bit number to test for BUSY
/*----------------------------------------------------------------------
Function name:  SetBusy

Description:   	Set the Busy Bit so that SW cursor does not try to use
				the command fifo at the same time as DDFLIP
				Must be called before CMDFIFO_PROLOG

Return:        WORD *pFlags
----------------------------------------------------------------------*/

WORD  Set_Busy(WORD * pFlags)
{
      __asm mov   ebx, pFlags
      __asm bts   WORD PTR [ebx], BUSY_BIT
      return *pFlags;
}// SetBusy

/*----------------------------------------------------------------------
Function name:  readSwapCount

Description:    Read swap count register in CE

Return:         DWORD
----------------------------------------------------------------------*/

DWORD
READSWAPCOUNT(NT9XDEVICEDATA *ppdev)
{

    return (int)(GET (_DD(sst2CRegs)->swapCount) & SST_SWAP_BUFFER_COUNT);

}// readSwapCount


void __stdcall
CSIMVIDEOUPDATE()
{
    DWORD dwCount;
    DWORD dwDevNode;

    {
        SST2INTERFACE iSST2;
        SST2DISPLAYMASK mask;
        SST2HOSTINFO hostInfo;

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

        if(SST2_Get_Device_Host(iSST2, dwDevNode, &hostInfo) != SST2RET_OK)
        {
            SST2_Release_Interface(iSST2);
            goto SST2ConnectFail;
        }

        hostInfo.Flags.Render_Enabled = 0;

        if(SST2_Set_Device_Host(iSST2, dwDevNode, &hostInfo) != SST2RET_OK)
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
        R3HOSTINFO hostInfo;
     
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

        if(R3_Get_Device_Host(iR3, dwDevNode, &hostInfo) != R3RET_OK)
        {
            R3_Release_Interface(iR3);
            goto R3ConnectFail;
        }

        hostInfo.Flags.Render_Enabled = 0;

        if(R3_Set_Device_Host(iR3, dwDevNode, &hostInfo) != R3RET_OK)
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
        HYDRAHOSTINFO hostInfo;
     
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

        if(Hydra_Get_Device_Host(iHydra, dwDevNode, &hostInfo) != HYDRARET_OK)
        {
            Hydra_Release_Interface(iHydra);
            goto HydraConnectFail;
        }

        hostInfo.Flags.Render_Enabled = 0;

        if(Hydra_Set_Device_Host(iHydra, dwDevNode, &hostInfo) != HYDRARET_OK)
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
        RAGEHOSTINFO hostInfo;
     
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

        if(Rage_Get_Device_Host(iRage, dwDevNode, &hostInfo) != RAGERET_OK)
        {
            Rage_Release_Interface(iRage);
            goto RageConnectFail;
        }

        hostInfo.Flags.Render_Enabled = 0;

        if(Rage_Set_Device_Host(iRage, dwDevNode, &hostInfo) != RAGERET_OK)
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


/*----------------------------------------------------------------------
Function name:  DdFlip

Description:    DDRAW callback Flip

                This callback is invoked whenever we are about to flip to
                from one surface to another.   pfd->lpSurfCurr is the
                surface we were at, pfd->lpSurfTarg is the one we are
                flipping to.

                You should point the hardware registers at the new surface,
                and also keep track of the surface that was flipped away
                from, so that if the user tries to lock it, you can be sure
                that it is done being displayed

Return:         DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdFlip( LPDDHAL_FLIPDATA pfd )
{
    NT9XDEVICEDATA  *ppdev = (NT9XDEVICEDATA *)pfd->lpDD->dwReserved3;
    WORD * pFlags = (WORD *)_FF(lpDeFlags);
    WORD SaveBusy = *pFlags;
    WORD TrashBusy = Set_Busy(pFlags);
    DWORD targetHWAddress; // used for FS AA
    
    CMDFIFO_PROLOG(hwPtr);

    if (LOW_POWER_MODE(SaveBusy))
    {
        pfd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

#ifdef FXTRACE
    DISPDBG((ppdev, DEBUG_APIENTRY, "Flip32" ));
    DUMP_DDHAL_FLIPDATA(ppdev, DEBUG_DDGORY, pfd );
#endif

    /* Store addresses of current (flipped from) and target (flipped to) surfaces. */

    switch( _DD(ddFSAAMode)  )
    {
        case  FSAA_MODE_4XFLIP:
            /* Store addresses of current (flipped from) and target (flipped to) surfaces. */
            _DD(ddSurfaceFlippedFrom) = pfd->lpSurfCurr->lpGbl->fpVidMem;
            _DD(ddSurfaceFlippedTo)   = pfd->lpSurfTarg->lpGbl->fpVidMem;
            targetHWAddress = GET_HW_ADDR(pfd->lpSurfTarg);

            if(  _DD(ddAAFlipCount) == 3) 
            {
                // this is a fix to clean up screen garbage that you get when
                // starting up fullscreen AA apps.
                // copy data from backbuffer back onto primary surface to fix up AA transition mess
                CMDFIFO_SAVE(hwPtr);
                FixPrimaryBufferForAA( ppdev,  pfd  );
                CMDFIFO_RELOAD(hwPtr);
                _DD(ddAAFlipCount) = 4;  // don't need to inc beyond this
            }
            if( _DD(ddAAFlipCount) < 3 )
                _DD(ddAAFlipCount)++;
            break;
        case  FSAA_MODE_4XBLT:
        case  FSAA_MODE_DEMO:
            // Force the app to always draw to the secondary 
            // then we blt to primary from here
            // instead of flipping
            _DD(ddSurfaceFlippedFrom) = _DD(ddAASecondaryBuffer1Start);
            _DD(ddSurfaceFlippedTo)   = _DS(gdiDesktopStart);
            targetHWAddress = _DS(gdiDesktopStart);
            CMDFIFO_SAVE(hwPtr);
            AABltToPrimary( ppdev, pfd, targetHWAddress );
            CMDFIFO_RELOAD(hwPtr);
            break;
        case FSAA_MODE_FOUR:
            
            //This mode will become part of 4XBLT when I get to it - mls
            
            // for this mode, do an AA blt to the primary or sec buffer that is currently not visible
            _DD(ddSurfaceFlippedFrom) = _DD(ddAASecondaryBuffer1Start);
            _DD(ddAAFlipCount)++;

            //this mode is still under construction
            _DD(ddSurfaceFlippedTo)   = _DS(gdiDesktopStart);
             targetHWAddress = _DS(gdiDesktopStart);
//            if( _DD(ddAAFlipCount) & 0x1 )
//            {
//               _DD(ddSurfaceFlippedTo)  = _DD(ddAASecondaryBuffer2Start);
//               targetHWAddress = _DD(ddAASecondaryBuffer2HW); 
//            }
//            else
//            {
//               _DD(ddSurfaceFlippedTo) = _DS(gdiDesktopStart);
//               targetHWAddress = _DS(gdiDesktopStart);
//            }
            CMDFIFO_SAVE(hwPtr);
            AABltToPrimary( ppdev, pfd, targetHWAddress );
            CMDFIFO_RELOAD(hwPtr);
            break;
        default:
            targetHWAddress = GET_HW_ADDR(pfd->lpSurfTarg);
            break;
    }




    /* Wait on previous flip, if not 3D rendering surface. */

    if (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY))
    {
        if (pfd->dwFlags & DDFLIP_WAIT)
        {
            while (FXGETFLIPSTATUS(ppdev));
        }
        else if (FXGETFLIPSTATUS(ppdev))
        {
            RESTORE_BUSY(pFlags,SaveBusy);
            pfd->ddRVal = DDERR_WASSTILLDRAWING;
            return DDHAL_DRIVER_HANDLED;
        }
    }

    // Flip the desktop surface
    if(!(pfd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY) && (!_FF(dd3DInOverlay)))
    {
        // NOTE SLI will always be for d3dinOverlay surface only, so will never come in here
          
        /* Rampage cannot pipeline writes to the desktop  */
	    /* start address, so pipeline must be flushed.    */
	    /* If a flip is already in progress, the flush    */
	    /* will end up waiting on vertical retrace. -CGW- */

        // Flush engines w/ MOPs and wait for fifo to drain (if active)
        CMDFIFO_CHECKROOM( hwPtr, 2*MOP_SIZE );
        SETMOP( hwPtr,SST_MOP_STALL_2D );
        SETMOP( hwPtr,SST_MOP_FLUSH_PCACHE |
                      SST_MOP_STALL_3D |
                     (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));
        CMDFIFO_EPILOG(hwPtr);

    #ifdef AGP_CMDFIFO
        {
          void FLUSHAGP(NT9XDEVICEDATA * ppdev);
          if (_FF(doAgpCF))
            FLUSHAGP(ppdev);
        }
    #endif // #ifdef AGP_CMDFIFO

        // wait for non-busy, meaning fifo drained as well
        // Q: Should we Fence here to force all fifo data across the bus?
        FXBUSYWAIT(ppdev); 

        //Do direct write to the primary base left video register to change primary screen base address
        //when flipping primary surface chains
        SETDW(ghwVD->vdVoPdBaseLeft, GET_HW_ADDR(pfd->lpSurfTarg)); 
        SETDW(ghwVD->vdVoPsStatus0,  GET(ghwVD->vdVoPsStatus0)|SST_VO_PD_UPDATE );
   
        pfd->ddRVal = DD_OK;

        CSIMVIDEOUPDATE();
    }
    else if(pfd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
    {   
        DWORD dwRet = FlipOverlaySurface( pfd);

        // Flip overlay will call CSim itself to update video output, if required
    }
    else
    {
        // 3D OVERLAY (PROMOTED) FLIP

        // make sure we don't overflow the internal video address queue
        while ( ((int)_DD(ddLastSwapCount)-SST_MAX_SWAPS_PENDING) >= (int)READSWAPCOUNT(ppdev) )
        { RELENT_WHILE_POLLING }

        // Note that we are letting CE control update - not hitting PS_STATUS
        // Queue the swap - this will cause CE to issue swap req to video, which will accept new address
        // Queue stall until swap - don't want to start render until new render buffer not visible
        HW_ACCESS_ENTRY(hwPtr, ACCESS_3D);
        CMDFIFO_CHECKROOM( hwPtr, PH1_SIZE+1 + 2*MOP_SIZE );
        SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, leftOverlayBuf));
        // mls - I changed this to work with new AA modes
        // normal non-aa behavior should be the same as it was
        SETPD( hwPtr, ghw0->leftOverlayBuf, targetHWAddress>>5 ); 
        SETMOP( hwPtr,SST_MOP_QUEUE_SWAP );
        SETMOP( hwPtr,SST_MOP_STALL_SWAP1 );
        HW_ACCESS_EXIT(ACCESS_3D);
        CMDFIFO_EPILOG(hwPtr);

        INCREMENT_SWAP_COUNT;
  
      #ifdef AGP_CMDFIFO
        {
          void  FLUSHAGP(NT9XDEVICEDATA * ppdev);
          FLUSHAGP(ppdev);
        }
      #endif // #ifdef AGP_CMDFIFO

        pfd->ddRVal = DD_OK;

      // CSIM does random execute size which makes the chip mismatch which
      // breaks SLI pretty pictures....
      // Note: THIS IS ONLY needed for CSIM Server
#ifdef SLI
         if (_DD(sliMode))
            {
            DWORD dwStatus;
            DWORD i;
            DWORD dwChipMask;
            dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
            do 
               {
               dwStatus = 0x0;
               for (i=0x0; i<_FF(dwNumUnits); i++)
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
            SETDW(ghwIO->sliBroadcast, (_FF(dwChipID) << SST_SLI_RENID_MEMBASE0_SHIFT) | dwChipMask);   
            }
#endif
        CSIMVIDEOUPDATE();
    }

    RESTORE_BUSY(pFlags,SaveBusy);
    return DDHAL_DRIVER_HANDLED;

}// DdFlip

/*----------------------------------------------------------------------
Function name:  DdGetFlipStatus

Description:    DDRAW callback GetFlipStatus

                If the display has went through one refresh cycle since the
                flp occurred we return DD_OK.  If it has not went through
                one refresh cycle we return DDERR_WASSTILLDRAWING to
                indicate that this surface is still busy "drawing" the
                flipped page.   We also return DDERR_WASSTILLDRAWING if the
                bltter is busy and the caller wanted to know if they could
                flip yet

Return:        	DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pgd )
{
  DD_ENTRY_SETUP(pgd->lpDD);
  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "GetFlipStatus32" ));
  DUMP_GETFLIPSTATUSDATA(ppdev, DEBUG_DDGORY, pgd );
  #endif

  pgd->ddRVal = DD_OK;

  if (DDSCAPS_OVERLAY & pgd->lpDDSurface->ddsCaps.dwCaps)
  {
      pgd->ddRVal= GetOverlayFlipStatus( ppdev, pgd->dwFlags,
                            GET_HW_ADDR( pgd->lpDDSurface));

  }
  else if ( pgd->dwFlags == DDGFS_ISFLIPDONE )
  {
    /* Handle DDGFS_ISFLIPDONE by checking flip status. */

    if (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY)) // Return DD_OK for 3D rendering.
	{
      /* Return flip status only for surface flipped from or to. */

	  if (( pgd->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedFrom)) ||
          ( pgd->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedTo)))
      {
        /* Check primary surface or video overlay flipping status. */

        if ( FXGETFLIPSTATUS(ppdev) )
		{
          pgd->ddRVal = DDERR_WASSTILLDRAWING;
		}
      }
    }
  }
  else if ( pgd->dwFlags == DDGFS_CANFLIP )
  {
    /* Handle DDGFS_CANFLIP by checking busy status. */

    if (!(_FF(dd3DInOverlay) & D3D_USING_OVERLAY)) // Return DD_OK for 3D rendering.
	{
      if ( FXGETBUSYSTATUS(ppdev) )
      {
        pgd->ddRVal = DDERR_WASSTILLDRAWING;
      }
    }
  }
  else
  {
    pgd->ddRVal = DDERR_INVALIDPARAMS;
  }
  return DDHAL_DRIVER_HANDLED;
}// DdGetFlipStatus


/*----------------------------------------------------------------------
Function name:  DdFlipToGDISurface

Description:   	DDRAW callback FlipToGDISurface

Return:         DWORD DDRAW result

                DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdFlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA pftgs )
{

  NT9XDEVICEDATA *ppdev = (NT9XDEVICEDATA *)(pftgs->lpDD->dwReserved3);
  WORD * pFlags = (WORD *)_FF(lpDeFlags);
  WORD SaveBusy = *pFlags;
  WORD TrashBusy = Set_Busy(pFlags);

  if (LOW_POWER_MODE(SaveBusy))
  {
      pftgs->ddRVal = DD_OK;
      return DDHAL_DRIVER_NOTHANDLED;
  }

  if( pftgs->dwToGDI )
  {
    DWORD addr;
    CMDFIFO_PROLOG(hwPtr);

    addr = _DS(gdiDesktopStart);

    if(!_FF(dd3DInOverlay))
    {
        // Flush engines w/ MOPs and wait for fifo to drain (if active)
        CMDFIFO_CHECKROOM( hwPtr, MOP_SIZE );
        SETMOP( hwPtr,SST_MOP_STALL_2D );
        CMDFIFO_EPILOG(hwPtr);

        // wait for non-busy, meaning fifo drained as well
        // Q: Should we Fence here to force all fifo data across the bus?
        FXBUSYWAIT(ppdev); 

        //Do direct write to the primary base left video register to change primary screen base address
        //when flipping primary surface chains
        SETDW(ghwVD->vdVoPdBaseLeft, addr); 
        SETDW(ghwVD->vdVoPsStatus0,  GET(ghwVD->vdVoPsStatus0)|SST_VO_PD_UPDATE );

        //CSIMVIDEOUPDATE();
    }
    else
    {

        // Update overlay base address
        // make sure we don't overflow the internal video address queue
        while ( ((int)_DD(ddLastSwapCount)-SST_MAX_SWAPS_PENDING) >= (int)READSWAPCOUNT(ppdev) )
        { RELENT_WHILE_POLLING }

        // Note that we are letting CE control update - not hitting PS_STATUS
        // Queue the swap - this will cause CE to issue swap req to video, which will accept new address
        // Queue stall until swap - don't want to start render until new render buffer not visible
        HW_ACCESS_ENTRY(hwPtr, ACCESS_3D);
        CMDFIFO_CHECKROOM( hwPtr, PH1_SIZE+1 + 2*MOP_SIZE );
        SETPH( hwPtr, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, leftOverlayBuf));
        SETPD( hwPtr, ghw0->leftOverlayBuf, _DS(gdiDesktopStart)>>5 );
        SETMOP( hwPtr,SST_MOP_QUEUE_SWAP );
        SETMOP( hwPtr,SST_MOP_STALL_SWAP1 );
        HW_ACCESS_EXIT(ACCESS_3D);
        CMDFIFO_EPILOG(hwPtr);

        INCREMENT_SWAP_COUNT;

      #ifdef AGP_CMDFIFO
        {
          void  FLUSHAGP(NT9XDEVICEDATA * ppdev);
          FLUSHAGP(ppdev);
        }
      #endif // #ifdef AGP_CMDFIFO

      //CSIMVIDEOUPDATE();

    }
    Msg(ppdev, DEBUG_APIENTRY, "FlipToGDISurface32 (to GDI)" );
    CMDFIFO_EPILOG(hwPtr);
  }
  else
  {
    Msg(ppdev, DEBUG_APIENTRY, "FlipToGDISurface32 (away from GDI)" );
  }

  RESTORE_BUSY(pFlags,SaveBusy);

  pftgs->ddRVal = DD_OK;  
  return DDHAL_DRIVER_HANDLED;

}// DdFlipToGDISurface

#ifdef MAGIC_SCRIPT

/*----------------------------------------------------------------------
Function name:  DoConfig

Description: This is the magic script to enable AGP on Banshee

Return:         int

                0 -
----------------------------------------------------------------------*/

#define FILENAME "\\\\.\\H6VDD"

int DoConfig(NT9XDEVICEDATA * ppdev)
{
   DIOC_DATA DIOC_Data;
   HANDLE hDevice;

   hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
   if (INVALID_HANDLE_VALUE != hDevice)
      {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DeviceIoControl(hDevice, AGP_WARMUP, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      }
   CloseHandle(hDevice);

   return 0;
}// DoConfig

/*----------------------------------------------------------------------
Function name:  UpdateIMask

Description: This is used to keep the mini-VDD upto date on the IMASK (Interrupt mask)

Return:         int

                0 -
----------------------------------------------------------------------*/
int UpdateIMask(NT9XDEVICEDATA * ppdev, DWORD dwIMask)
{
   HANDLE hDevice;
   DIOC_DATA DIOC_Data;

   hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
   if (INVALID_HANDLE_VALUE != hDevice)
      {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DIOC_Data.dwSpare = dwIMask;
      DeviceIoControl(hDevice, UPDATE_IMASK, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      }
   CloseHandle(hDevice);

   return 0;
}
#endif



#ifdef AGP_CMDFIFO

#define TOVFIFO(arg1) toVFifo(arg1)
#define TOAFIFO(arg1) toAFifo(arg1)

VOID TOVFIFO(NT9XDEVICEDATA * ppdev);
VOID TOAFIFO(NT9XDEVICEDATA * ppdev);

static FxBool agpFifoNow=FXFALSE;

// call this from DdBlt, or other commonly used routine to test fifo switch
VOID switch_fifo(NT9XDEVICEDATA * ppdev)
{
    if(agpFifoNow)
    {
        // from AGP to Video
        TOAFIFO(ppdev);
        agpFifoNow = FXFALSE;
    }
    else
    {
        // from Video to AGP
        TOVFIFO(ppdev);
        agpFifoNow = FXTRUE;
    }
}

// Here's code that was in DdCreateSurface - just after conditional call to promote_primarytooverlay
#if 0 // MAE-AGP fifo always or never - no switching
#if defined(AGP_CMDFIFO)
    if (psurf->ddsCaps.dwCaps & DDSCAPS_3DDEVICE)
    {   
        extern VOID TOAFIFO(NT9XDEVICEDATA * ppdev);

        WORD * pFlags = (WORD *)_FF(lpDeFlags);
        WORD SaveBusy = *pFlags;
        Set_Busy(pFlags);

        if (!LOW_POWER_MODE(SaveBusy))
        {
        TOAFIFO(ppdev);
        }
        RESTORE_BUSY(pFlags,SaveBusy);
    }
#endif // defined(AGP_CMDFIFO)
#endif // 0 MAE-AGP


/*----------------------------------------------------------------------
Function name:  toVFifo

Description:    swap to the video command fifo

Return:         NONE
----------------------------------------------------------------------*/

VOID TOVFIFO(NT9XDEVICEDATA * ppdev)
{

  FxU32  startVirtualAddr;

  if (_FF(doAgpCF))
  {
    DWORD   baseSize;

    if (CMDFIFOUNBUMPEDWORDS)
    {
        BUMPAGP(ppdev, CMDFIFOUNBUMPEDWORDS );
    }

    _FF(doAgpCF) = 0;

    // drain fifo
    while ( _DD(sst2CRegs)->PRIMARY_CMDFIFO.depth > 0)
    {}

    // wait for all ops to complete - no commands can be left in video fifo
    FXBUSYWAIT(ppdev);

    // disable fifo
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

    baseSize = (_FF(fifoSize) + 4095) & ~4095;

    _FF(mainFifo.base)  = (_FF(fifoStart) + 4095) & ~4095;
    _FF(mainFifo.start) = _FF(LFBBASE) + _FF(mainFifo.base);

    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.baseAddr,  _FF(mainFifo.base)>>SST2_CMD_LOG_PAGE_SIZE );
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.readPtr,   _FF(mainFifo.base) );
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.aMin,      _FF(mainFifo.base) - 4 );
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.aMax,      _FF(mainFifo.base) - 4 );
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.depth,     0 );
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.holeCount, 0 );
    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.holeInt, SST_MASK(23));// timeout - set to all 1's (MAX)

    SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.baseSize, 
          ((_FF(fifoSize) >> SST2_CMD_LOG_PAGE_SIZE)-1) |  // size in pages
          SST_EN_CMDFIFO |                            // enable the fifo
          0);

    startVirtualAddr    = _FF(mainFifo.start);
    _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

    CMDFIFOPTR          = startVirtualAddr;
    CMDFIFOSTART        = startVirtualAddr;
    CMDFIFOOFFSET       = CMDFIFOSTART - CMDFIFOBASE;
    CMDFIFOEND          = startVirtualAddr + _FF(fifoSize) - FIFO_END_ADJUST;
    CMDFIFOSPACE        = (CMDFIFOEND - CMDFIFOSTART)>>2; // space in dword units
    CMDFIFOJMP          = ((_FF(mainFifo.base) << SSTCP_PKT0_ADDR_SHIFT) >> 2) | 
                          (SSTCP_PKT0_JMP_LOCAL << SSTCP_PKT0_FUNC_SHIFT);
    CMDFIFOUNBUMPEDWORDS= 0; // used in AGP only - but set to 0 in case try to switch to AGP

    // RESET_HW_PTR(startVirtualAddr);    // cmdfifo debugging aid

    _FF(InPacket)         = 0;
    _FF(WordsLeftInPacket)= 0;
    _FF(Wrapping)         = 0;

  }

}// toVFifo

int DoConfig(NT9XDEVICEDATA * ppdev);

/*----------------------------------------------------------------------
Function name:  toAFifo

Description:   	swap to the AGP command fifo

Return:         NONE
----------------------------------------------------------------------*/

VOID TOAFIFO(NT9XDEVICEDATA * ppdev)
{
  FxU32 startVirtualAddr;

  if (_FF(doAgpCF)) 
  {
      // already AGP fifo 
      return;
  }

  _FF(doAgpCF) = 1;

  // drain fifo
  while ( _DD(sst2CRegs)->PRIMARY_CMDFIFO.depth > 0)
  {}

  // wait for all ops to complete - no commands can be left in video fifo
  FXBUSYWAIT(ppdev);

  // disable fifo
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

  _FF(mainFifo.base)  = (_FF(agpMain.physAddr) + 4095) & ~4095;
  _FF(mainFifo.start) = _FF(mainFifo.base);

#ifdef MAGIC_SCRIPT
  // MAGIC_SCRIPT was defined for the V3 9X driver in dd32/ddflip.c
  // Andrew Sobczyk says about this: "On Banshee, we could not 
  // get AGP to work without doing this.  I have no explanation for why it makes 
  // AGP work but it does."  The Magic Script code has since moved to the vxd, and
  // since it is believed we can't call a vxd function from within the 32-bit display
  // driver attatched to the VXD, the old MAGIC SCRIPT function is done right here.
  DoConfig(ppdev);
#endif // MAGIC_SCRIPT


  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.baseAddr,  _FF(mainFifo.base)>>SST2_CMD_LOG_PAGE_SIZE );
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.readPtr,   _FF(mainFifo.base) );
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.aMin,      0 );  // N/A if holecount disabled
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.aMax,      0 );  // N/A if holecount disabled
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.depth,     0 );
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.holeCount, 0 );
  SETDW( _DD(sst2CRegs)->PRIMARY_CMDFIFO.holeInt, SST_MASK(23));// timeout - set to all 1's (MAX)

  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize, 
        ((_FF(agpMain.sizeInB) >> SST2_CMD_LOG_PAGE_SIZE)-1) |  // size in pages
        SST_EN_CMDFIFO |                            // enable the fifo
        SST_CMDFIFO_DISABLE_HOLES |                 // hw will not count holes
        SST_CMDFIFO_AGP);                           // fifo is in AGP space

  startVirtualAddr    = _FF(agpMain.linAddr );
  _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

  CMDFIFOPTR          = startVirtualAddr;
  CMDFIFOSTART        = startVirtualAddr;
  CMDFIFOEND      = startVirtualAddr + _FF(agpMain.sizeInB) - FIFO_END_ADJUST;
  CMDFIFOSPACE        = (CMDFIFOEND - CMDFIFOSTART)>>2; // space in dword units
  CMDFIFOOFFSET       = CMDFIFOSTART - CMDFIFOBASE;

#define SSTCP_PKT0_ADDR_BITS_IN_WORD0  26
  CMDFIFOJMP           = (((_FF(agpMain.physAddr) & SST_MASK(SSTCP_PKT0_ADDR_BITS_IN_WORD0)) >> 2) << SSTCP_PKT0_ADDR_SHIFT) |
                         (SSTCP_PKT0_JMP_AGP << SSTCP_PKT0_FUNC_SHIFT) |
                         SSTCP_PKT0_LONGADDR;
  CMDFIFOJMP2          = _FF(agpMain.physAddr) >> SSTCP_PKT0_ADDR_BITS_IN_WORD0; // might be 0 if no upper address bits
  CMDFIFOEPILOGPTR     = CMDFIFOSTART;
  CMDFIFOUNBUMPEDWORDS = 0;

  _FF(InPacket)          = 0;
	_FF(WordsLeftInPacket) = 0;
	_FF(Wrapping)          = 0;

}// toAFifo

#endif // AGP_CMDFIFO


#define MYP6FENCE	__asm xchg eax, temp;
#define MYFENCVAR	FxU32 temp;
#define DEBUG_FIX	// as nothing
#define MYCMDFIFO	_FF(lpCRegs)->PRIMARY_CMDFIFO
#include "..\minivdd\agpcf.c"

