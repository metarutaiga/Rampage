/** $Header: hwcext.c, 2, 12/22/99 11:59:06 AM PST, Ryan Bissell$ */
/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   hwcext.c
**
** Description: HW extension support functions for applets and glide.
**
** $Log: 
**  2    3dfx      1.1         12/22/99 Ryan Bissell    New clut management code
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
** 3     8/27/99 2:32p Einkauf
** CMDFIFO alive (if CF=1 in setenv.bat)
** 
** 2     5/21/99 4:05p Peterm
** remerged with v3 tot
** 
** 44    5/17/99 1:43p Stb_lpost
** V3TV Video Capture fixes for E3 Demo
** 
** 43    4/06/99 12:39a Andrew
** Changed RestoreDesktop and adding pumbing for UnGlideContext to attempt
** to fix Alt-Tab
** 
** 42    4/04/99 4:13p Sreid
** Added mechanism to coordinate loss of context between Glide and DD
** 
** 41    4/04/99 4:13p Sreid
** 
** 40    4/01/99 11:54a Andrew
** Added code to use new shared names and to return state of exclusive
** flag
** 
** 39    3/22/99 2:43p Xingc
** Add function hwcOvlAddr() to pass current overlay data out.
** 
** 38    3/19/99 6:07p Andrew
** Added code to return TRUE when we go into Low Power Mode
** 
** 37    3/04/99 12:52p Edwin
** fix PRS 4654: desktop cursor disappeared after exiting Unreal.
** hwcRestoreDesktop() needs to restore the saved cursor.
** 
** 36    2/26/99 10:21a Michael
** Proxy for KenW - These changes make alt-tab from winglide apps *much*
** more reliable.  I now see no corruption or hangs from glquake or
** heretic2.   NBA98 also worked.  PRS 3320 is possibly associated.  There
** may be others.
** 
** 35    2/05/99 4:37p Peter
** agp fifo offset
** 
** 34    1/08/99 11:05a Stuartb
** Backed out LCDCTRL changes as advised by KMW.
** 
** 33    1/07/99 1:24p Stuartb
** Another oops; Added HWCEXT_LCDCTRL for control panel flat panel ops.
** 
** 32    1/07/99 1:08p Stuartb
** oops
** 
** 31    1/07/99 12:18p Stuartb
** Added HWCEXT_LCDCTRL for control panel flat panel ops.
** 
** 30    1/04/99 11:58a Peter
** added windowed context support
** 
** 29    12/29/98 1:18p Michael
** Implement the 3Dfx/STB unified header.
** 
** 28    11/30/98 6:52p Peter
** extended execute fifo for video memory fifo's
** 
** 27    11/16/98 8:53p Andrew
** Changed DeviceID & Vendor ID since Vendor is in low part of Dword
** 
** 26    11/16/98 8:35p Andrew
** Added code to return the real Device ID, Vendor ID, and Revision ID.
** 
** 25    10/29/98 2:13p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 24    9/11/98 10:44p Jdt
** Clean up some warnings, fix software cursor collision, minor
** optimization to data copy.
** 
** 23    9/02/98 3:29p Andrew
** Rest of fix for unreal & sw cursor
** 
** 22    8/28/98 11:54p Edwin
** Fix 2395, repaint screen as we exit Glide games.  Unreal should not
** cause desktop background to go black now.
** 
** 21    8/27/98 6:28p Andrew
** Added gamma correction
** 
** 20    8/27/98 2:25p Artg
** added ifdef cmdfifo around body of hwcexecutefifo for direct write
** compile
** 
** 19    8/03/98 6:33a Jdt
** Added code to notify D3D when I slam their hw state.
** 
** 18    7/24/98 2:01p Dow
** Added AGP Stuff
** 
** 17    7/23/98 4:17p Dow
** Return chipRev 3 for B*
** 
** 16    7/20/98 1:30p Jdt
** Increased command buffer size.
** 
** 15    7/18/98 12:20a Jdt
** Added state restoration buffer
** 
** 14    7/16/98 9:11a Andrew
** Last checkin did not restore deFlags.
** 
** 13    7/16/98 7:32a Andrew
** Added some code to restore SaveUnder buffer and reload the cursor when
** exclusive mode exits
** 
** 12    7/13/98 5:20p Jdt
** Added first primitive command buffer allocation and execution code
** for GlideWin
** 
** 
** 10    6/29/98 7:01p Dow
** Mucked around with hwcRlsExclusive
** 
** 9     6/29/98 6:50p Dow
** Added call to HWSetMode to hwcRslExclusive
** 
** 8     6/23/98 10:12a Stuartb
** Added I2C multibyte writes.
** 
** 7     6/16/98 3:54p Michael
** Make sure all device bitmaps are hostified in hwcSetExclusive().  Fixes
** #1650.
** 
** 6     5/20/98 8:13p Dow
** Device rev
** 
** 5     5/05/98 1:19p Stuartb
** Adding i2c extEscapes.
** 
** 4     4/28/98 3:57p Michael
** change Msg() to include a new parameter that will allow for selective
** debug message output.
** 
** 3     4/22/98 5:28p Dow
** Added code for HWCEXT_HWCSETEXLUSIVE and HWCEXT_RLSEXCLUSIVE
** 
** 2     4/16/98 10:15p Dow
** Glide/Windows co-op
** 
** 1     4/14/98 9:58a Dow
**
*/


#include "header.h"
#include "3dfx.h"
#include "hwcext.h"
#include "minivdd.h"
#include "cursor.h"

extern void DoAllHost(void);
extern void DisableDeviceBitmaps(void);
extern void DiscardAllSSB(void);

/* xhwcext.asm */
extern BOOL hwcSetGDIBusy( WORD FAR *lpGDISemaphore );
extern void hwcClearGDIBusy( WORD FAR *lpGDISemaphore );

extern DWORD dwDeviceHandle;
extern FARPROC fpRepaintScreen;

#define VALIDATECONTEXT

static char *extNames[] = {
  "HWCEXT_GETDRIVERVERSION",      /* 0x00 */
  "HWCEXT_ALLOCCONTEXT",          /* 0x01 */
  "HWCEXT_GETDEVICECONFIG",       /* 0x02 */
  "HWCEXT_GETLINEARADDR",         /* 0x03 */ 
  "HWCEXT_ALLOCFIFO",             /* 0x04 */
  "HWCEXT_EXECUTEFIFO",           /* 0x05 */
  "HWCEXT_QUERYCONTEXT",          /* 0x06 */
  "HWCEXT_RELEASECONTEXT"         /* 0x07 */
  "HWCEXT_HWCSETEXCLUSIVE",       /* 0x08 */
  "HWCEXT_HWCRLSEXCLUSIVE",       /* 0x09 */
  "HWCEXT_I2C_WRITE_REQ",         /* 0x0A */
  "HWCEXT_I2C_READ_REQ",          /* 0x0B */
  "HWCEXT_I2C_READ_RES",          /* 0x0C */
  "HWCEXT_I2C_MULTI_WRITE_REQ",   /* 0x0D */
  "HWCEXT_GETAGPINFO",            /* 0x0E */
  "HWCEXT_VIDTIMING",             /* 0x0F */
  "HWCEXT_FIFOINFO",              /* 0x10 */
  "HWCEXT_LINEAR_MAP_OFFSET",     /* 0x11 */
};


// PRS 4654 fix: the restore cursor code is moved here from hwcRlsExclusive(),
// it is called by hwcRlsExclusive() and hwcRestoreDesktop().
//
int SaveCursorExclude(int x, int y);
void RestoreCursor(void);

static void 
hwcRestoreCursor(void)
{
  int nSaveBusy;

  nSaveBusy = _FF(lpPDevice)->deFlags;
  _FF(lpPDevice)->deFlags = _FF(lpPDevice)->deFlags & ~(BUSY);
   if((SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
   {
     SaveCursorExclude(_FF(LastCursorPosX), _FF(LastCursorPosY));
   }
   ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
   RestoreCursor();
  _FF(lpPDevice)->deFlags = nSaveBusy;
}

/*----------------------------------------------------------------------
Function name:  hwcGetDriverVersion

Description:    Obtain the version number of the driver.

Information:

Return:         LONG    1L always returned
----------------------------------------------------------------------*/
static LONG
hwcGetDriverVersion(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->optData.driverVersionRes.major = 0xdead;
  res->optData.driverVersionRes.minor = 0xcafe;  

  res->resStatus = 1;

  return res->resStatus;

} /* hwcGetDriverVersion */


/*----------------------------------------------------------------------
Function name:  hwcAllocContext

Description:    Obtain the Allocation Context.

Information:

Return:         LONG     1L for success or,
                        -1L for failure.
----------------------------------------------------------------------*/
static LONG
hwcAllocContext(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  static int contextID;

  if (req->optData.allocContextReq.protocolRev != HWCEXT_PROTOCOLREV) {
    res->resStatus = -1;
    return res->resStatus;
  }
  
  res->optData.allocContextRes.contextID = ++contextID;
  res->resStatus = 1;

  return 1;
} /* hwcAllocContext */


/*----------------------------------------------------------------------
Function name:  hwcGetDeviceConfig

Description:    Obtain information about the display device.

Information:

Return:         LONG     1L is always returned.
----------------------------------------------------------------------*/
static LONG
hwcGetDeviceConfig(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  VALIDATECONTEXT;

  res->optData.deviceConfigRes.devNum     = 0;
  res->optData.deviceConfigRes.vendorID   = _FF(VendorDeviceID) & 0xFFFF;
  res->optData.deviceConfigRes.deviceID   = _FF(VendorDeviceID)>>16;
  res->optData.deviceConfigRes.fbRam      = _FF(TotalVRAM);
  res->optData.deviceConfigRes.pciStride  = _FF(ddTilePitch);
  res->optData.deviceConfigRes.hwStride   = _FF(ddTileStride);
  res->optData.deviceConfigRes.tileMark   = _FF(ddTileMark);
    
  res->optData.deviceConfigRes.chipRev = _FF(RevisionID);

  res->resStatus = 1;

  return res->resStatus;

} /* hwcGetDeviceConfig */


/*----------------------------------------------------------------------
Function name:  hwcGetLinearAddr

Description:    Obtain the Linear Address.

Information:

Return:         LONG     1L is always returned.
----------------------------------------------------------------------*/
static LONG
hwcGetLinearAddr(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  VALIDATECONTEXT;

  _FF(GlideAppProcessNumber) = req->optData.linearAddrReq.pHandle;

  res->optData.linearAddressRes.numBaseAddrs = 3;
  res->optData.linearAddressRes.baseAddresses[0] = _FF(regBase);
  res->optData.linearAddressRes.baseAddresses[1] = _FF(lfbBase);
  res->optData.linearAddressRes.baseAddresses[2] = _FF(ioBase);

  res->resStatus = 1;

  return res->resStatus;

} /* hwcGetLinearAddr */


/*----------------------------------------------------------------------
Function name:  allocSelector

Description:    Allocate a Selector using INT 31h.

Information:

Return:         WORD     Value of the Selector.
----------------------------------------------------------------------*/
#define DPMI_INT  0x31
#define ALLOC_SEL 0x0
#define FREE_SEL  0x1
#define GET_BASE  0x6
#define SET_BASE  0x7
#define SET_LIMIT 0x8

#pragma warning( disable : 4704 ) 
static WORD allocSelector( void ) {
    WORD rv;
    _asm {
        mov ax, ALLOC_SEL
        mov cx, 1
        int DPMI_INT
        jb  fubar
        mov word ptr rv, ax
        jmp notfubar
fubar:
        mov word ptr rv, 0
notfubar:
    }
    return rv;
}


/*----------------------------------------------------------------------
Function name:  freeSelector

Description:    Free a previously allocated Selector using INT 31h.

Information:

Return:         WORD    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
static WORD freeSelector( WORD selector ) {
    WORD rv;
    _asm {
        mov ax, FREE_SEL
        mov bx, word ptr selector
        int DPMI_INT
        jb  fubar
        mov word ptr rv, 1
        jmp notfubar
fubar:
        mov word ptr rv, 0
notfubar:
    }
    return rv;
}


/*----------------------------------------------------------------------
Function name:  setSelectorAddr

Description:    Set a previously allocated Selector's base
                address and limit using INT 31h.
Information:

Return:         WORD    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
static WORD setSelectorAddr( WORD selector, DWORD base, WORD limit ) {
    WORD rv;
    _asm {
        mov ax, SET_BASE
        mov bx, word ptr selector
        mov cx, word ptr [base+2]
        mov dx, word ptr base
        int DPMI_INT
        jb  fubar
        mov ax, SET_LIMIT
        mov bx, word ptr selector
        mov cx, 0
        mov dx, word ptr limit
        int DPMI_INT
        jb  fubar
        mov word ptr rv, 1
        jmp notfubar
fubar:
        mov word ptr rv, 0
notfubar:        
    }
    return rv;
}


/*----------------------------------------------------------------------
Function name:  getSelectorBase

Description:    Return the base address of a previously allocated
                Selector using INT 31h.
Information:

Return:         DWORD    Base address of the selector.
----------------------------------------------------------------------*/
static DWORD getSelectorBase( WORD selector ) {
    DWORD rv;
    _asm {
        mov ax, GET_BASE
        mov bx, word ptr selector
        int DPMI_INT
        jb fubar
        mov word ptr rv, dx;
        mov word ptr [rv+2], cx;
        jmp notfubar
fubar:
        mov word ptr rv, 0;
        mov word ptr [rv+2], 0;
notfubar:
    }
    return rv;
}


/*----------------------------------------------------------------------
Function name:  hwcAllocFifo

Description:    Allocate the command FIFO
                
Information:

Return:         LONG    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
#define CMD_BUF_SIZE   32768
#define STATE_BUF_SIZE 4096

static LONG
hwcAllocFifo(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  LONG
    retVal = 0;

  VALIDATECONTEXT;

  res->optData.allocFifoRes.commandBuffer = 0x00UL;
  res->optData.allocFifoRes.psBuffer      = 0x00UL;

#ifdef CMDFIFO
  {
    HGLOBAL   
      cmdHndl = 0x00UL, 
      psHndl = 0x00UL;
    DWORD
      linBase;
    char far*
      buffer;

    cmdHndl = GlobalAlloc( GMEM_FIXED | GMEM_SHARE | GMEM_ZEROINIT, 
                           CMD_BUF_SIZE );
    if (cmdHndl == 0x00UL) goto __errExit;

    buffer = GlobalLock( cmdHndl );
    linBase = getSelectorBase( SELECTOROF( buffer ) );
    res->optData.allocFifoRes.commandBuffer   = linBase + OFFSETOF( buffer );
    res->optData.allocFifoRes.commandBufferSz = CMD_BUF_SIZE;

    psHndl = GlobalAlloc( GMEM_FIXED | GMEM_SHARE | GMEM_ZEROINIT, 
                          STATE_BUF_SIZE );
    if (psHndl == 0x00UL) goto __errExit;

    buffer = GlobalLock( psHndl );
    linBase = getSelectorBase( SELECTOROF( buffer ) );
    res->optData.allocFifoRes.psBuffer        = linBase + OFFSETOF( buffer );
    res->optData.allocFifoRes.psBufferSz      = STATE_BUF_SIZE;

    res->resStatus = HWCEXT_FIFO_HOST;
    retVal = 1;

    if (0) {
  __errExit:
      if (cmdHndl != 0x00UL) GlobalFree(cmdHndl); 
      if (psHndl != 0x00UL) GlobalFree(psHndl);  
    }
  }
#endif /* CMDFIFO */

    return retVal;
} /* hwcAllocFifo */


/*----------------------------------------------------------------------
Function name:  hwcExecuteFifo

Description:    Service the Execute FIFO requests.
                
Information:

Return:         LONG    1 for success or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcExecuteFifo(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  LONG 
    retVal = 0;

  res->resStatus = 0;

#ifdef CMDFIFO
  {
    CMDFIFO_PROLOG(cmdFifo);
#if 0 // !! SST2
    /* check if s/w cursor or FSEM is active */
    if ( !hwcSetGDIBusy( &_FF(lpPDevice)->deFlags ) ) 
      /* if so we must punt */
      {
      if  ((GET(lph3IORegs->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
         {
         res->resStatus = 1;
         return 1;
         }
      else
         {
         return 0;
         }
      }
#endif // !! SST2
    /* Default to success */
    retVal = 1;
  
    CMDFIFO_SETUP(cmdFifo);
  
    {
      /* Storage for jmp packets */
      FxU32 
        stateJmp[2],
        stateRet[2],
        cmdJmp[2],
        cmdRet[2];
      FxU32
        jmpWords;
      static FxU32
        lastContextID = 0x00UL;
      FxBool 
        doStateP = ((_FF(pD3context) != 0xFFFFFFFFUL) || /* d3d loaded? */
                    (_FF(pD3context) != 0x00UL) ||       /* d3d context? */
                    (req->contextID != lastContextID));  /* last execution? */
      
      switch(req->optData.executeFifoReq.fifoType) {
      case HWCEXT_FIFO_FB:
        {
          jmpWords = 1;

          if (doStateP) {
            stateJmp[0] = ((req->optData.executeFifoReq.stateOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                           SSTCP_PKT0_JSR |
                           SSTCP_PKT0);
            stateRet[0] = (SSTCP_PKT0_RET |
                           SSTCP_PKT0);
          }
            
          cmdJmp[0] = ((req->optData.executeFifoReq.fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                       SSTCP_PKT0_JSR |
                       SSTCP_PKT0);
          cmdRet[0] = (SSTCP_PKT0_RET |
                       SSTCP_PKT0);
                                      
          goto __jmpWrite;
        }
        break;

#ifdef AGP_CMDFIFO
      case HWCEXT_FIFO_AGP:
        {
          const FxU32
            agpCmdFifoOffset = (cmdFifo - _FF(agpMain.linAddr));
          jmpWords = 2;
          
          if (doStateP) {
            stateJmp[0] = ((req->optData.executeFifoReq.stateOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                           SSTCP_PKT0_JMP_AGP |
                           SSTCP_PKT0);
            stateJmp[1] = 0x00UL;

            stateRet[0] = (((agpCmdFifoOffset + (sizeof(FxU32) << 0x01UL)) << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                           SSTCP_PKT0_JMP_AGP |
                           SSTCP_PKT0);
            stateRet[1] = 0x00UL;
          }
          
          cmdJmp[0] = ((req->optData.executeFifoReq.fifoOffset << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                       SSTCP_PKT0_JMP_AGP |
                       SSTCP_PKT0);
          cmdJmp[1] = 0x00UL;

          cmdRet[0] = (((agpCmdFifoOffset + (sizeof(FxU32) << (0x01UL + doStateP))) << (SSTCP_PKT0_ADDR_SHIFT - 2UL)) |
                       SSTCP_PKT0_JMP_AGP |
                       SSTCP_PKT0);
          cmdRet[1] = 0x00UL;

          goto __jmpWrite;
        }
        break;
#endif /* AGP_CMDFIFO */

    __jmpWrite:
        {
          FxU32
            i,
            *clientPtr;

          P6FENCE;

          if (doStateP) {
            /* State return to fifo */
            clientPtr = (FxU32*)(req->optData.executeFifoReq.statePtr + 
                                 (req->optData.executeFifoReq.stateSize << 0x02UL));
            
            for(i = 0; i < jmpWords; i++) 
              h3WRITE(NULL, clientPtr + i, stateRet[i]);
            
            P6FENCE;
            
            /* Jump to state routine */
            CMDFIFO_CHECKROOM(cmdFifo, jmpWords);
            for(i = 0; i < jmpWords; i++) 
              SET(cmdFifo, 0, stateJmp[i]);
            BUMP(req->optData.executeFifoReq.psSizeInDWORDS + (jmpWords << 0x01UL));
          }

          /* command return to fifo */
          clientPtr = (FxU32*)(req->optData.executeFifoReq.fifoPtr +
                               (req->optData.executeFifoReq.fifoSize << 2UL));
          
          for(i = 0; i < jmpWords; i++) 
            h3WRITE(NULL, clientPtr + i, cmdRet[i]);

          P6FENCE;

          /* Jump to state routine */
          CMDFIFO_CHECKROOM(cmdFifo, jmpWords);
          for(i = 0; i < jmpWords; i++) 
            SET(cmdFifo, 0, cmdJmp[i]);
          BUMP(req->optData.executeFifoReq.fifoSizeInDWORDS + (jmpWords << 0x01UL));
#if 0 // !! SST2 PKT5 format changed from H3
          /* Write the serial # for this fifo execution. */
          CMDFIFO_CHECKROOM(cmdFifo, 3);
          SET(cmdFifo, 0, (SSTCP_PKT5_LFB |
                           (0x00UL << SSTCP_PKT5_BYTEN_W2_SHIFT) |
                           (0x00UL << SSTCP_PKT5_BYTEN_WN_SHIFT) |
                           (0x01UL << SSTCP_PKT5_NWORDS_SHIFT) |
                           SSTCP_PKT5));
          SET(cmdFifo, 0, req->optData.executeFifoReq.sentinalOffset);
          SET(cmdFifo, 0, req->optData.executeFifoReq.serialNumber);
#endif // !! SST2
        }
        break;

      case HWCEXT_FIFO_HOST:
        {
          DWORD 
            far* cmdBuf,
            far* cmdBufEnd;
          DWORD 
            numCmds;
          WORD 
            sel = allocSelector();

          if (doStateP) {
            CMDFIFO_CHECKROOM( cmdFifo, req->optData.executeFifoReq.stateSize);
            
            numCmds = req->optData.executeFifoReq.stateSize;
            setSelectorAddr( sel, req->optData.executeFifoReq.statePtr, (WORD)numCmds * 4 );
            
            cmdBuf    = MAKELP( sel, 0 );
            cmdBufEnd = cmdBuf + numCmds;
            
            while( cmdBuf < cmdBufEnd ) {
              /* This has NO direct write equivalent */
              SET( cmdFifo, 0, *cmdBuf );
              cmdBuf++;
            }
            BUMP( numCmds );
          }
        
          /* Current command stream */
          CMDFIFO_CHECKROOM( cmdFifo, req->optData.executeFifoReq.fifoSize );
          numCmds = req->optData.executeFifoReq.fifoSize;
          setSelectorAddr( sel, req->optData.executeFifoReq.fifoPtr, (WORD)numCmds * 4 );
        
          cmdBuf    = MAKELP( sel, 0 );
          cmdBufEnd = cmdBuf + numCmds;
        
          while( cmdBuf < cmdBufEnd ) {
            /* This has NO direct write equivalent */
            SET( cmdFifo, 0, *cmdBuf );
            cmdBuf++;
          }
          BUMP( numCmds );

          freeSelector( sel );
        }
        break;

      default:
        retVal = 0;
        break;
      }

      lastContextID = req->contextID;
    }

    CMDFIFO_EPILOG(cmdFifo);
    D3notify();
    hwcClearGDIBusy( &_FF(lpPDevice)->deFlags );
  }
#endif /* CMDFIFO */

  res->resStatus = retVal;
    
  return retVal;
} /* hwcExecuteFifo */


/*----------------------------------------------------------------------
Function name:  hwcQueryContext

Description:    Query the HW Context
                
Information:    Presently unused!

Return:         LONG    0 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcQueryContext(hwcExtRequest_t *req, hwcExtResult_t *res)
{
  LONG dwReturn;
  VALIDATECONTEXT;

  dwReturn = _FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE ? HWC_CONTEXT_ACTIVE : HWC_CONTEXT_LOST;

  res->resStatus = dwReturn; 
  return dwReturn;
} /* hwcQueryContext */


/*----------------------------------------------------------------------
Function name:  hwcReleaseContext

Description:    Release the HW Context
                
Information:    Presently unused!

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcReleaseContext(hwcExtRequest_t *req, hwcExtResult_t *res)
{

  VALIDATECONTEXT;

  return 1;
} /* hwcReleaseContext */


/*----------------------------------------------------------------------
Function name:  hwcSetExclusive

Description:    Sets environment for exclusive mode.
                
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
void LoadGamma(void);
static LONG
hwcSetExclusive(hwcExtRequest_t *req, hwcExtResult_t *res) 
{

  LoadGamma(); 
  _FF(gdiFlags) |= SDATA_GDIFLAGS_HWC_EXCLUSIVE;
  _FF(UnGlideContext) = FALSE;

//  DiscardAllSSB();          // invalidate all save screen bitmaps
  DoAllHost();              // move all device bitmaps to host
  DisableDeviceBitmaps();   // disallow future device bitmaps

  _FF(PreGlideModeNumber) = _FF(ModeNumber);

  res->resStatus = 1;

  return 1;
  
} /* hwcSetExclusive */


/*----------------------------------------------------------------------
Function name:  hwcRlsExclusive

Description:    Releases exclusive mode.
                
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
int setupPalette(int start, int count, FxU32 FAR * lpcolors);

static LONG
hwcRlsExclusive(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  extern BOOL InitThunks( DWORD dwBase, DWORD dwSize);

  VALIDATECONTEXT;


  DPF(DBGLVL_NORMAL,"hwcRlsExclusive: setting Mode Number 0x%d",
    _FF(ModeNumber ));

  if (!HWSetMode( (int)_FF(ModeNumber) )) {
    DPF(DBGLVL_NORMAL,"HWSetMode failed from HWC release exclusive");
  }


  InitThunks(_FF(ScreenAddress), 0x400000L);
  
  setupPalette(0, 256, NULL); //reset palette or gamma ramp
  
  _FF(gdiFlags) &= ~SDATA_GDIFLAGS_HWC_EXCLUSIVE;
  _FF(UnGlideContext) = TRUE;

  hwcRestoreCursor();

  res->resStatus = 1;

  if (fpRepaintScreen != NULL)
  {
    fpRepaintScreen();  // fix PRS 2395, exiting Unreal casues black screen
  }

  return 1;

} /* hwcRlsExclusive */


/*----------------------------------------------------------------------
Function name:  hwcI2C_write

Description:    Perform an I2C write via the display driver's VxD.
                
Information:

Return:         LONG    Result of the VDDCall or,
                        FXFALSE if i2C was busy.
----------------------------------------------------------------------*/
#ifndef ERROR_BUSY
//
// MessageId: ERROR_BUSY
//
// MessageText:
//
//  The requested resource is in use.
//
#define ERROR_BUSY                       170L
#endif

static LONG
hwcI2C_write (hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  static FxU16 busy = 1;

  VALIDATECONTEXT;

  _asm    /*had to put in assbly to get atomic reference to busy*/
    {
      dec word ptr [busy];
      jnz i2c_busy;
    }
  {
    res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, 
                             dwDeviceHandle, H3VDD_I2C_WRITE,
                             (req->optData.i2c_WriteReq.deviceAddress << 16) |
                             (req->optData.i2c_WriteReq.i2c_regNum << 8) |
                             req->optData.i2c_WriteReq.i2c_regValue, 0);
    busy = 1;
    return (res->resStatus);
  }
i2c_busy:
    {
      res->resStatus = ERROR_BUSY;
      return (FXFALSE);
    }
} /* hwcI2C_write */


/*----------------------------------------------------------------------
Function name:  hwcI2C_MultiWrite

Description:    Perform multiple I2C write via the display
                driver's VxD.
                
Information:

Return:         LONG    Result of the VDDCall or,
                        FXFALSE if i2C was busy.
----------------------------------------------------------------------*/
static LONG
hwcI2C_MultiWrite (hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  static FxU16 busy = 1;
  long mwParms[2];   // too much stuff to pass, pass by reference

  VALIDATECONTEXT;

  _asm    /*had to put in assbly to get atomic reference to busy*/
    {
      dec word ptr [busy];
      jnz i2c_busy;
    }
  {
    mwParms[0] = (req->optData.i2c_MultiWriteReq.deviceAddress << 16) |
      (req->optData.i2c_MultiWriteReq.i2c_regNum << 8) |
      (req->optData.i2c_MultiWriteReq.i2c_numBytes);
    // note: app's flat pointer will make more sense on vxd side
    mwParms[1] = (long)req->optData.i2c_MultiWriteReq.i2c_regValues;

    res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, 
                             dwDeviceHandle, H3VDD_I2C_MULTI_WRITE,
                             (DWORD)((long _far*)mwParms), 0);

    busy = 1;
    return (res->resStatus);
  }
i2c_busy:
    {
      res->resStatus = ERROR_BUSY;
      return (FXFALSE);
    }
} /* hwcI2C_MultiWrite */


/*----------------------------------------------------------------------
Function name:  hwcI2C_Read

Description:    Perform an I2C read via the display driver's VxD.
                
Information:

Return:         LONG    Result of the VDDCall or,
                        FXFALSE if i2C was busy.
----------------------------------------------------------------------*/
static LONG
hwcI2C_read (hwcExtRequest_t *req, hwcExtResult_t *res) 
{
  static FxU16 busy = 1;

  VALIDATECONTEXT;

  _asm    /*had to put in assbly to get atomic reference to busy*/
    {
      dec word ptr [busy];
      jnz i2c_busy;
    }
  {
    res->resStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, 
                             dwDeviceHandle, H3VDD_I2C_READ,
                             (req->optData.i2c_ReadReq.deviceAddress << 8) |
                             req->optData.i2c_ReadReq.i2c_regNum, 0);
    busy = 1;
    res->optData.i2c_ReadRes.i2c_regValue = res->resStatus & 255;
    return (res->resStatus >> 8);
  }
i2c_busy:
    {
      res->resStatus = ERROR_BUSY;
      return (FXFALSE);
    }
} /* hwcI2C_read */


/*----------------------------------------------------------------------
Function name:  hwcAGPInfo

Description:    Obtain information about AGP.
                
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcAGPInfo (hwcExtRequest_t *req, hwcExtResult_t *res)
{
  res->optData.agpInfoRes.lAddr = _FF(agpMain.linAddr );
  res->optData.agpInfoRes.pAddr = _FF(agpMain.physAddr);
  res->optData.agpInfoRes.size  = _FF(agpMain.sizeInB );

  res->resStatus = 1;

  return 1;
} /* hwcAGPInfo */

static LONG
hwcFifoInfo(hwcExtRequest_t* req, 
            hwcExtResult_t*  res)
{
  res->optData.fifoInfoRes.fifoType = ((_FF(enableAGPCF) && _FF(doAgpCF))
                                       ? HWCEXT_FIFO_AGP
                                       : HWCEXT_FIFO_FB);

  res->resStatus = 1;

  return 1;
}


/*----------------------------------------------------------------------
Function name:  hwcLinearMapOffset

Description:    Obtain the Linear Map Offset.
                
Information:

Return:         LONG    1 for a valid linear address or,
                        0 for failure.
----------------------------------------------------------------------*/
static LONG
hwcLinearMapOffset(hwcExtRequest_t* req,
                   hwcExtResult_t*  res)
{
  res->resStatus = 0;

#ifdef AGP_CMDFIFO
  {
    hwcExtResult_t agpInfo;

    if (hwcFifoInfo(NULL, &agpInfo) && 
        (agpInfo.optData.fifoInfoRes.fifoType == HWCEXT_FIFO_AGP) &&
        hwcAGPInfo(NULL, &agpInfo)) {
      
      res->resStatus = ((req->optData.mapInfoReq.remapAddr >= agpInfo.optData.agpInfoRes.lAddr) &&
                        (req->optData.mapInfoReq.remapAddr < (agpInfo.optData.agpInfoRes.lAddr +
                                                              agpInfo.optData.agpInfoRes.size)));
      if (res->resStatus) {
        res->optData.mapInfoRes.linAddrOffset = ((req->optData.mapInfoReq.remapAddr - agpInfo.optData.agpInfoRes.lAddr) +
                                                 0x2000000UL); /* Size of memBase1 */
      }
    }
  }
#endif /* AGP_CMDFIFO */

  if (!res->resStatus) {
    hwcExtLinearMapInfoReq_t 
      tempData;
    FxU32
      linAddrOffset;
    
    /* Copy the data because we aren't supposed to whack the
     * request data from the client.
     */
    tempData.mapAddr   = req->optData.mapInfoReq.mapAddr;
    tempData.remapAddr = req->optData.mapInfoReq.remapAddr;
    
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, 
            H3VDD_LINEAR_MAP_OFFSET, 
            (DWORD)&tempData, &linAddrOffset);
    
    res->optData.mapInfoRes.linAddrOffset = linAddrOffset;
    res->resStatus = (linAddrOffset != 0x00UL);
  }
  
  return res->resStatus;
}


/*----------------------------------------------------------------------
Function name:  hwcRestoreDesktop

Description:    Restore desktop to pre-exclusive mode state
                
Information:

Return:         LONG    1 is always returned
----------------------------------------------------------------------*/
static LONG
hwcRestoreDesktop()
{
    extern UINT FAR PASCAL Enable1(LPVOID lpDevice,
				   UINT style,
				   LPSTR lpDeviceType,
				   LPSTR lpOutput,
				   LPVOID lpStuff);

#if 0
    if (_FF(ModeNumber) == _FF(PreGlideModeNumber))
    {
	Enable1(_FF(lpPDevice), EnableDevice, NULL, NULL, NULL);

    hwcRestoreCursor(); // PRS 4654 fix

	if (fpRepaintScreen != NULL)
	{
	    fpRepaintScreen();
	}
    }
#endif
	    
    return 1;
}

/*----------------------------------------------------------------------
Function name:  hwcOvlAddr

Description:    Obtain information about Overlay.
                
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcOvlAddr (hwcExtResult_t *res)
{
  if(_FF(ddVisibleOverlaySurf))
      res->optData.ovlInfoRes.ovlAddr   = _FF(ddVisibleOverlaySurf);
  else
      res->optData.ovlInfoRes.ovlAddr   = _FF(ovlCurAddr);

  res->optData.ovlInfoRes.ovlXScale = _FF(ovlXScale);
  res->optData.ovlInfoRes.ovlYScale = _FF(ovlYScale);

#ifdef V3TV  // gwa overlay info
	res->optData.ovlInfoRes.dwOvlWidth = _FF(dwOvlWidth);
	res->optData.ovlInfoRes.dwOvlHeight = _FF(dwOvlHeight);
	res->optData.ovlInfoRes.dwOvlStride = _FF(dwOvlStride);
	res->optData.ovlInfoRes.dwOvlPxlFormat = _FF(dwOvlPxlFormat);
	res->optData.ovlInfoRes.dwOvlTiled = _FF(dwOvlTiled);
#endif

  res->resStatus = 1;

  return 1;
} /* hwcOvlAddr */

/*----------------------------------------------------------------------
Function name:  hwcShareContextDword

Description:    Coordinates loss of context safely with Glide.
                
Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
static LONG
hwcShareContextDword(hwcExtRequest_t *req, hwcExtResult_t *res) 
{
   HWGETFLATADDRESS HwGetFlat;
   HwGetFlat.pData = &_FF(UnGlideContext);

   VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
	    H3VDD_GET_FLAT_ADDRESS, 0, &HwGetFlat);

   res->optData.shareContextDWORDRes.contextDWORD = HwGetFlat.FlatAddress;
   res->resStatus=1;
   return 1;
} /* hwcShareContextDword */

/*----------------------------------------------------------------------
Function name:  hwcExt

Description:    Massive switch statement that parses/processes
                the Extended request.
Information:

Return:         LONG    Value of the subsequently called function.
----------------------------------------------------------------------*/
LONG FAR PASCAL
hwcExt(DWORD *lpInput, DWORD *lpOutput) {
  hwcExtRequest_t *req = (hwcExtRequest_t *) lpInput;
  hwcExtResult_t  *res = (hwcExtResult_t *) lpOutput;

  DPF(DBGLVL_NORMAL, "hwcExt(%s)\n", extNames[req->which]);

  switch (req->which) {
  case HWCEXT_GETDRIVERVERSION:
    return hwcGetDriverVersion(req, res);
    break;

  case HWCEXT_ALLOCCONTEXT:
    return hwcAllocContext(req, res);
    break;

  case HWCEXT_GETDEVICECONFIG:
    return hwcGetDeviceConfig(req, res);
    break;

  case HWCEXT_GETLINEARADDR:
    return hwcGetLinearAddr(req, res);
    break;
    
  case HWCEXT_ALLOCFIFO:
    return hwcAllocFifo(req, res);
    break;
    
  case HWCEXT_EXECUTEFIFO:
    return hwcExecuteFifo(req, res);
    break;

  case HWCEXT_QUERYCONTEXT:
    return hwcQueryContext(req, res);
    break;

  case HWCEXT_RELEASECONTEXT:
    return hwcReleaseContext(req, res);
    break;

  case HWCEXT_HWCSETEXCLUSIVE:
    return hwcSetExclusive(req, res);
    break;

  case HWCEXT_HWCRLSEXCLUSIVE:
    return hwcRlsExclusive(req, res);
    break;

  case HWCEXT_I2C_WRITE_REQ:
    return hwcI2C_write (req, res);
          break;

  case HWCEXT_I2C_MULTI_WRITE_REQ:
    return hwcI2C_MultiWrite (req, res);
        break;

  case HWCEXT_I2C_READ_REQ:
    return hwcI2C_read (req, res);
    break;

  case HWCEXT_GETAGPINFO:
    return hwcAGPInfo(req, res);
    break;

  case HWCEXT_FIFOINFO:
    return hwcFifoInfo(req, res);
    break;

  case HWCEXT_LINEAR_MAP_OFFSET:
    return hwcLinearMapOffset(req, res);
    break;

  case HWCEXT_RESTORE_DESKTOP:
    return hwcRestoreDesktop();
    break;

  case HWCEXT_OVERLAY_DATA:
    return hwcOvlAddr ( res);
    break;
    
  case HWCEXT_SHARE_CONTEXT_DWORD:
    return hwcShareContextDword( req, res );
    break;
      

//  case HWCEXT_LCDCTRL:
//    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
//                      H3VDD_FLATPNL_PHYSICAL, 0, &req->optData.lcdControl);
//  break;

  default:
    return -1;
    break;
  }

  return 1;  
} /* hwcExt */
