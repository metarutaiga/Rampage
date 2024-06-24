/* -*-c++-*- */
/* $Header: hardware.c, 1, 9/11/99 10:18:43 PM PDT, StarTeam VTS Administrator$ */
/*
** Copyright (c) 1997-1999, 3Dfx Interactive, Inc.
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
** File name:   hardware.c
**
** Description: Misc. HW specific functions.
**
** $Revision: 1$
** $Date: 9/11/99 10:18:43 PM PDT$
**
** $History: hardware.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:55p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 3/17/99    Time: 4:41p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added in a ifdef so that CmdFifo is 15 & 8 for Avenger and 9 & 2 for
** Banshee
** 
** *****************  Version 47  *****************
** User: Andrew       Date: 3/17/99    Time: 2:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** changed FifoThresh to 15 & 8
** 
** *****************  Version 46  *****************
** User: Stb_skephart Date: 2/22/99    Time: 12:29p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 45  *****************
** User: Stb_skephart Date: 2/19/99    Time: 6:08a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 44  *****************
** User: Cwilcox      Date: 2/18/99    Time: 11:48a
** Updated in $/devel/h3/Win95/dx/minivdd
** Final removal of tiled/linear promotion.
** 
** *****************  Version 43  *****************
** User: Stb_skephart Date: 2/17/99    Time: 7:06p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 42  *****************
** User: Stb_skephart Date: 2/17/99    Time: 3:49p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 41  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Linear versus tiled promotion removal.
** 
** *****************  Version 40  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 8:06a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 39  *****************
** User: Michael      Date: 1/08/99    Time: 1:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 38    11/04/98 2:17a Artg
** mysetcf and my setdf now uses h3write for csim builds.
** Allows DF option to work
** 
** 37    10/29/98 2:14p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 36    8/25/98 12:31a Andrew
** Added a FXWAITFORIDLE to ensure cmdfifo is flushed
** 
** 35    8/07/98 6:44p Ken
** should fix random agp hangs/crashes.   fixes jedi knight menu
** on agp board/driver
** 
** 34    7/24/98 10:38p Ken
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** 33    7/24/98 7:17p Miriam
** AGP command fifo only enabled for D3D.
** 
** 32    7/23/98 4:04p Ken
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** 31    7/18/98 6:41p Ken
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** 30    6/17/98 5:28p Miriam
** Optmize fifo fetch via vidpixelbufthold
** 
** 29    5/18/98 3:54p Ken
** upped command fifo threshold from 8 to 9, AA determined that 8 is
** insufficient.  tested 2d winbench 98 and performance looks the same
** 
** 28    5/15/98 4:35p Michael
** Backout my previous changes.  Add more changes for changing stide when
** tiled vs. linear.
** 
** 27    5/15/98 9:22a Michael
** Modify ResetInvariantState to pass in lpDst.
** 
** 26    5/11/98 5:24p Suninn
** fix aperture stride
** 
** 25    4/28/98 6:17p Artg
** cmdfifo debug save paket header address and data
** 
** 24    4/24/98 1:33p Ken
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** 23    4/21/98 6:52p Ken
** clean up mode set, modes seem to set faster now too
** 
** 22    4/21/98 12:01a Ken
** added agp workaround (set ar=1) for readback unreliability
** 
** 21    4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 20    4/15/98 4:59p Suninn
** move dwordCount into shared.h
** 
** 19    4/03/98 10:51a Artg
** added extra arguments for new dpf
** 
** 18    3/31/98 11:40a Miriam
** Add prints, move data into global to enable global debugging, etc.
*/
//: hardware.c

// STB Begin Changes
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
// STB End Changes


#include "thunk32.h"
#include "h3g.h"

DWORD pfnCSIM;

#if 0
SstCRegs* lpCRegs = (SstCRegs*) (HARDWARE_ADDR + SST_CMDAGP_OFFSET);
SstGRegs* lpGRegs = (SstGRegs*) (HARDWARE_ADDR + SST_2D_OFFSET);
SstIORegs *lpIOregs = (SstIORegs *)(HARDWARE_ADDR + SST_IO_OFFSET);

Fifo fifo;
#endif


// STB Begin Changes
// STB Begin Changes
// STB-SR 1/28/99 Adding support for 2D AGP Cmd Fifo
// Most of the code was taken from dd32/ddflip.c code
// inside the function TOAFIFO().
#ifdef STBPERF_2DAGPCMDFIFO

#ifdef DEBUG
/*----------------------------------------------------------------------
Function name:   outpd

Description:     put DWORD value dwData to port address

Return:          NONE
----------------------------------------------------------------------*/

void outpd(DWORD addr, DWORD dwData)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, eax}
}

/*----------------------------------------------------------------------
Function name:   outpw

Description:     put WORD value dwData to port address

Return:          NONE  
----------------------------------------------------------------------*/

void outpw(DWORD addr, DWORD dwData)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, ax}
}

/*----------------------------------------------------------------------
Function name:   inpd

Description:     get DWORD value dwData from port address and save to
                 eax to return 

Return:          NONE
----------------------------------------------------------------------*/

DWORD inpd(DWORD addr)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {in   eax, dx}
}

/*----------------------------------------------------------------------
Function name:   inpd

Description:     get WORD value dwData from port address and save to
                 eax to return 

Return:          NONE
----------------------------------------------------------------------*/

WORD inpw(DWORD addr)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {in   ax, dx}
}
#endif
/*----------------------------------------------------------------------
Function name:  DoConfig

Description:    copied from the dd32/ddflip function

Return:         int 

                0 - 
----------------------------------------------------------------------*/

int DoConfig(int id)
{
   DWORD dwSize1, dwSize2;
   DWORD physMem, physFB;
   int BusNum;
   int DevNum;
   int nid;
   WORD nCmd;

   DEBUG_FIX;

   for (BusNum=0; BusNum < 256; BusNum++)
      for (DevNum=0; DevNum<32; DevNum++)
         {
         outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11));
         nid = inpd(0xcfc);
         if (nid == id)
            {
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x4);
            nCmd = inpw(0xcfc);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x4);
            outpw(0xcfc, nCmd & 0xFFFC);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x10);
            physMem = inpd(0xcfc) & 0xFF000000;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x10);
            outpd(0xcfc, 0xFFFFFFFF);
            dwSize1 = ~inpd(0xcfc) + 1;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x10);
            outpd(0xcfc, physMem);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x14);
            physFB = inpd(0xcfc) & 0xFF000000;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x14);
            outpd(0xcfc, 0xFFFFFFFF);
            dwSize2 = ~inpd(0xcfc) + 1;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x14);
            outpd(0xcfc, physFB);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x4);
            outpw(0xcfc, nCmd);
            }
         }

   return 0;
}// DoConfig


#define FXGETBUSYSTATUS()      (GET(_FF(lpHWIOregs)->status) & SST_BUSY)
#define FXBUSYWAIT()           do{;}while((GET(_FF(lpHWIOregs)->status) & SST_BUSY))


#endif // #ifdef STBPERF_2DAGPCMDFIFO
// STB End Changes


#define MYP6FENCE	__asm xchg eax, temp;
#define MYFENCVAR	FxU32 temp;
#define MYCMDFIFO	_FF(lpCRegs)->PRIMARY_CMDFIFO
#include "agpcf.c"

/*----------------------------------------------------------------------
Function name:  InitFifo

Description:    Initialize the cmd fifo.
                
Information:
    fifobase in physical address of framebuffer
    fifosize in bytes

Return:         VOID
----------------------------------------------------------------------*/
void
InitFifo(FxU32 fifoBase, FxU32 fifoSize)
{
#ifdef CMDFIFO
  FxU32   startVirtualAddr; 
  DWORD   physAddr;
#endif // #ifdef CMDFIFO
  DWORD   baseSize;

  DEBUG_FIX;

  baseSize = (_FF(fifoSize) + 4095) & ~4095;

  FXWAITFORIDLE();

// STB Begin Changes
// STB-SR 1/28/99 Adding support for 2D AGP Cmd Fifo
// Most of the code was taken from dd32/ddflip.c code
// inside the function TOAFIFO().
#ifdef STBPERF_2DAGPCMDFIFO
  if (_FF(enableAGPCF))
  {
      FxU32  startVirtualAddr;
      DWORD fifoLength;

      // Base size should be 4 MB, need to change
      baseSize = (_FF(agpMain.sizeInB) + 4095) & ~4095;

      _FF(doAgpCF) = 1;

      _FF(mainFifo.base)  = (_FF(agpMain.linAddr ) + 4095) & ~4095;
      _FF(mainFifo.start) = _FF(mainFifo.base);

      // disable command fifo 0
      //
      SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

// MAGIC_SCRIPT was defined for the 9X driver in dd32/ddflip.c
// I asked Andrew Sobczyk about this and he said, "On Banshee, we could not 
// get AGP to work without doing this.  I have no explanation for why it makes 
// AGP work but it does."  The Magic Script code has since moved to the vxd, and
// since I don't think I can call a vxd function from within the 32-bit display
// driver attatched to the VXD, I'm using the old MAGIC SCRIPT function.
//#ifdef MAGIC_SCRIPT
      // Do a config causes this is a good thing
      DoConfig(0x0003121a);
//#endif

      //
      // h/w bug workaround -- the high water mark (the number << 5)
      // must be 8 or greater
      //
#ifdef H4
     SETDW(_FF(lpCRegs)->cmdFifoThresh,             (15 << 5) | 8 );
#else
     SETDW(_FF(lpCRegs)->cmdFifoThresh,             (9 << 5) | 2 );
#endif
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrL,  _FF(agpMain.physAddr) );
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrH,  0);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.aMin,      _FF(agpMain.physAddr) - 4);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.aMax,      _FF(agpMain.physAddr) - 4);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth,     0);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.holeCount, 0);

	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL, (_FF(agpMain.physAddr) >> 12) );
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  SST_EN_CMDFIFO  |
                                            SST_CMDFIFO_AGP |
                                            SST_CMDFIFO_DISABLE_HOLES);

      startVirtualAddr     = _FF(agpMain.linAddr );
	  fifoLength           = _FF(agpMain.sizeInB );

      if (fifoLength > (4 * 1024L * 1024L))
	      fifoLength = 4 * 1024L * 1024L;
	
      _FF(cmdFifoBasePtr)  = (FxU32)&_FF(mainFifo.base);

	  CMDFIFOPTR           = startVirtualAddr;
	  CMDFIFOSTART         = startVirtualAddr;
	  CMDFIFOSPACE         = (fifoLength / 4) - 3;
	  CMDFIFOEND           = startVirtualAddr + fifoLength - 12;

	  CMDFIFOOFFSET        = startVirtualAddr - _FF(agpMain.physAddr);
	  CMDFIFOJMP           = SSTCP_PKT0_JMP_AGP | (((_FF(agpMain.physAddr) & 0x00FFFFFF) >> 2) << 6);
	  CMDFIFOJMP2          = (_FF(agpMain.physAddr) >> 25) & ((1L << 12) - 1);

	  CMDFIFOEPILOGPTR     = CMDFIFOSTART;
	  CMDFIFOUNBUMPEDWORDS = 0;

	  RESET_HW_PTR(startVirtualAddr);    // cmdfifo debugging aid

	  _FF(InPacket)          = 0;
	  _FF(WordsLeftInPacket) = 0;
	  _FF(Wrapping)          = 0;
  }
  else
  {
#endif // #ifdef STBPERF_2DAGPCMDFIFO
// STB End Changes


  _FF(doAgpCF) = 0;

  _FF(mainFifo.base)  = (fifoBase + 4095) & ~4095;
  _FF(mainFifo.start) = lpDriverData->lfbBase + _FF(mainFifo.base);

  // disable command fifo 0
  //
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

#ifdef CMDFIFO
  //
  // h/w bug workaround -- the high water mark (the number << 5)
  // must be 8 or greater
  //
#ifdef H4
  SETDW(_FF(lpCRegs)->cmdFifoThresh,             (15 << 5) | 8 );
#else
  SETDW(_FF(lpCRegs)->cmdFifoThresh,             (9 << 5) | 2 );
#endif
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrL,  _FF(mainFifo.base) );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrH,  0 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.aMin,      _FF(mainFifo.base) - 4 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.aMax,      _FF(mainFifo.base) - 4 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.depth,     0 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.holeCount, 0 );

  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL, _FF(mainFifo.base) >> 12 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  ((baseSize >> 12) - 1) | SST_EN_CMDFIFO );

  startVirtualAddr    = _FF(mainFifo.start);
  physAddr            =  ((GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL) & 0x3FF) << 12);

  _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

  CMDFIFOPTR          = startVirtualAddr;
  CMDFIFOSTART        = startVirtualAddr;
  CMDFIFOSPACE        = (fifoSize / 4) - 2;
  CMDFIFOEND          = startVirtualAddr + fifoSize - 8;

  CMDFIFOOFFSET       = startVirtualAddr - physAddr;
  CMDFIFOJMP          = SSTCP_PKT0_JMP_LOCAL | ((physAddr >> 2) << 6);

  RESET_HW_PTR(startVirtualAddr);    // cmdfifo debugging aid

  _FF(InPacket)          = 0;         
  _FF(WordsLeftInPacket) = 0;     
  _FF(Wrapping)          = 0;
  
#endif

// STB Begin Changes
#ifdef STBPERF_2DAGPCMDFIFO
   }
#endif

}


/*----------------------------------------------------------------------
Function name:  InitRegs

Description:    Initialize the chip's registers.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
InitRegs()
{
    DWORD lfbMemoryConfig;
    DWORD screenSize;
    DWORD vidProcCfg;
    DWORD aperture;
    
    DEBUG_FIX;

    _FF(lpCRegs) = (SstCRegs *)(lpDriverData->regBase + SST_CMDAGP_OFFSET);
    _FF(lpGRegs) = (SstGRegs *)(lpDriverData->regBase + SST_2D_OFFSET);
    _FF(lpIOregs) = (SstIORegs *)(lpDriverData->regBase);

    _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

    FXWAITFORIDLE();
    
    screenSize = (lpDriverData->bi.biHeight << 16) | lpDriverData->bi.biWidth;

    if (_FF(ddPrimaryInTile))
    {
      lpDriverData->screenFormat = _FF(ddTileStride);
    }
    else
    {
      lpDriverData->screenFormat = _FF(pitch);
    }

    switch( lpDriverData->bpp)
    {
      case 8:  lpDriverData->screenFormat |= SSTG_PIXFMT_8BPP;  break;
      case 15: lpDriverData->screenFormat |= SSTG_PIXFMT_15BPP; break;
      case 16: lpDriverData->screenFormat |= SSTG_PIXFMT_16BPP; break;
      case 24: lpDriverData->screenFormat |= SSTG_PIXFMT_24BPP; break;
      case 32: lpDriverData->screenFormat |= SSTG_PIXFMT_32BPP; break;
    }

    // clear the desktop to black
    // XXX FIXME is palette entry 0 guaranteed to be black, for paletted modes?
    // 
    SETDW(_FF(lpGRegs)->clip0min, 0);
    SETDW(_FF(lpGRegs)->clip0max, screenSize);
    SETDW(_FF(lpGRegs)->dstFormat, lpDriverData->screenFormat);
    SETDW(_FF(lpGRegs)->dstBaseAddr, lpDriverData->gdiDesktopStart);
    SETDW(_FF(lpGRegs)->srcBaseAddr, lpDriverData->gdiDesktopStart);
    SETDW(_FF(lpGRegs)->colorFore, 0);
    SETDW(_FF(lpGRegs)->dstSize, screenSize);
    SETDW(_FF(lpGRegs)->dstXY, 0);
    SETDW(_FF(lpGRegs)->commandEx, 0);
    SETDW(_FF(lpGRegs)->command, 0xCC000000 | SSTG_GO | SSTG_RECTFILL);
    SETDW(_FF(lpIOregs)->vidPixelBufThold, (DWORD)0x00010410);

#if 0   // ScottK 02/22/99 -- this code removed until BIOS is finished. At that time, it will be deleted.
//PingZ 02/17/99 FIFO tweaks for 3D
// ScottK 02/17/99
// These changes greatly improve 3D Winbench!
// Of these changes, the change to pciInit0, and tmuGbeInit are known
// to be safe. The change to dramInit0 is believed to be safe, based on
// feedback from Andrew Tao. These will be removed as soon as a BIOS
// has been created that incorporates them, as these changes more properly
// belong in the BIOS.
// 
    SETDW(_FF(lpIOregs)->pciInit0, (DWORD)0x0584fb04);
    SETDW(_FF(lpIOregs)->tmuGbeInit, (DWORD)0xff0);
    SETDW(_FF(lpIOregs)->dramInit0, (DWORD)0xc17a9e9);
#endif    


    switch(lpDriverData->ddTilePitch)
    {
      case 1024: aperture=0;break;
      case 2048: aperture=1;break;
      case 4096: aperture=2;break;
      case 8192: aperture=3;break;
      default: aperture=4;
    }       
     // initialize memory config for tile surface
    lfbMemoryConfig = (lpDriverData->ddTileMark >> 12L)  // in page
      | ( aperture << 13L)   // aperture at 13th bit
      | (lpDriverData->ddTileStride << 16L); // at 16th bit
    SETDW(_FF(lpIOregs)->lfbMemoryConfig, lfbMemoryConfig);

    // now enable the video processor
    //
    vidProcCfg = GET(_FF(lpIOregs)->vidProcCfg);
    SETDW(_FF(lpIOregs)->vidProcCfg, vidProcCfg | SST_VIDEO_PROCESSOR_EN);
}


/*----------------------------------------------------------------------
Function name:  ResetInvariantState

Description:    Reset the registers that must not change.
                
Information:
    SIDE EFFECTS: based on the bits in resetMask: the screen's
    pDevice's stride will be updated; the 2d invariant registers
    will be set in the h/w.

Return:         VOID
----------------------------------------------------------------------*/
void
ResetInvariantState(FxU32 resetMask)
{
    FxU32 cmdMask, cmdWords, dstFormat;
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;

    cmdWords = 0;
    cmdMask = 0;

    if (resetMask & RESET_PDEV)
    {
	if (_FF(ddPrimaryInTile))
	{
	    _FF(gdiDesktopStart) |= SSTG_IS_TILED;
	    dstFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					      SSTG_DST_TILE_STRIDE);
	    dstFormat |= _FF(ddTileStride) << SSTG_DST_STRIDE_SHIFT;
	    _FF(screenFormat) = dstFormat;
	}
	else
	{
	    // linear
	    _FF(gdiDesktopStart) &= ~(SSTG_IS_TILED);
	    dstFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					      SSTG_DST_TILE_STRIDE);
	    dstFormat |= _FF(pitch) << SSTG_DST_STRIDE_SHIFT;
	    _FF(screenFormat) = dstFormat;
	}

	// now change the dib engine's stride so that it can render to
	// the desktop
    } 

    if (resetMask & RESET_DST)
    {
	cmdWords += 5;
	cmdMask |= (clip0minBit | clip0maxBit | dstBaseAddrBit | dstFormatBit |
		    commandExBit);
    }

    if (resetMask & RESET_SRC)
    {
	cmdWords += 1;
	cmdMask |= srcBaseAddrBit;
    }
    
    if (cmdWords > 0)
    {
	cmdWords += 1;
	CMDFIFO_SETUP(cmdFifo);
	CMDFIFO_CHECKROOM(cmdFifo, cmdWords);
	SETPH(cmdFifo, SSTCP_PKT2 | cmdMask);
	if (resetMask & RESET_DST)
	{
	    SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);
	    SET(cmdFifo, _FF(lpGRegs)->clip0max,
		(lpDriverData->bi.biHeight << 16) |
		lpDriverData->bi.biWidth);   
	    SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr,
		lpDriverData->gdiDesktopStart);
	    SET(cmdFifo, _FF(lpGRegs)->dstFormat, lpDriverData->screenFormat);
	}
	if (resetMask & RESET_SRC)
	{
	    SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr,
		lpDriverData->gdiDesktopStart);
	}
	if (resetMask & RESET_DST)
	{
	    SET(cmdFifo, _FF(lpGRegs)->commandEx, 0);
	}
	
	BUMP(cmdWords);

	CMDFIFO_EPILOG(cmdFifo);   
    }

}



/*----------------------------------------------------------------------
Function name:  FXWAITFORIDLE

Description:    Wait while HW is busy.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
FXWAITFORIDLE()
{
  if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
  {
    bumpAgp( CMDFIFOUNBUMPEDWORDS );
    
    while (GET(_FF(lpIOregs)->status) & SST_PCIFIFO_BUSY)
      ;
    
    while (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) > 0)
      ;
  }

  // !! SST-2 Hack because csim doesn't seem to clear busy flag
  // while (GET(_FF(lpIOregs)->status) & SST_BUSY)
    ;
}






//
// externals used in FXENTER / FXLEAVE
// 
char *fxEnterProcName;
FxU32 _SrcFormat;


#ifdef H3_AGP_WORKAROUND


/*----------------------------------------------------------------------
Function name:  myLoadCmdfifoRdptr

Description:    Early chips (a0, a1, a2) have command fifo
                readpointer readback reliability problems.   This
                sequence reads back the correct value but may
                yield worse performance when the readopinter is
                changing faster than we can read it back (e.g.,
                small primitives) -KMW
                
Information:    #ifdef H3_AGP_WORKAROUND

Return:         FxU32   The read ptr
----------------------------------------------------------------------*/
FxU32 
myLoadCmdfifoRdptr(FxU32 cRegs)
{
    FxU32 rdPtr1, rdPtr2, dummy;

    DEBUG_FIX;
    
    do
    {
	rdPtr1 = GET( ((SstCRegs *)cRegs)->PRIMARY_CMDFIFO.readPtrL );
	dummy = GET( ((SstIORegs *)_FF(regBase))->status );
	rdPtr2 = GET( ((SstCRegs *)cRegs)->PRIMARY_CMDFIFO.readPtrL );

    } while (rdPtr1 != rdPtr2);

    return rdPtr1;
}

#endif // #ifdef H3_AGP_WORKAROUND

#ifdef DEBUGFIFO


/*----------------------------------------------------------------------
Function name:  countBits

Description:    Count the bits in a FxU32.
                
Information:    #ifdef DEBUGFIFO

Return:         FxU32   number of bits.
----------------------------------------------------------------------*/
FxU32
countBits(FxU32 data)
{
    FxU32 nbits;

    DEBUG_FIX;

    nbits = 0;
    
    while (data != 0)
    {
    if ((data & 1) != 0)
        nbits += 1;

    data >>= 1;
    }

    return nbits;
}

//shadow registers for debugging
DWORD sLastHwPtr;
DWORD sInPacket;             // =0 expecting header, =1 not expecting hdr
DWORD sWordsLeftInPacket;    // # of words left in packet, counting header
DWORD sWrapping;
DWORD sLastPH;
DWORD sLastWR;
DWORD sCurrPH;
DWORD sCurrWR;


/*----------------------------------------------------------------------
Function name:  dfgpf

Description:    Assign shared fifo vars to global vars
                
Information:    #ifdef DEBUGFIFO

Return:         VOID
----------------------------------------------------------------------*/
void
dfgpf()
{

    sLastHwPtr        = _FF(LastHwPtr) ;
    sInPacket         = _FF(InPacket) ;        
    sWordsLeftInPacket= _FF(WordsLeftInPacket) ;
    sWrapping         = _FF(Wrapping) ;
    sLastPH           = _FF(LastPH) ;
    sLastWR           = _FF(LastWR) ;
    sCurrPH           = _FF(CurrPH) ;
    sCurrWR           = _FF(CurrWR) ;
    
}


/*----------------------------------------------------------------------
Function name:  ResetHwPtr

Description:    Resets the HW pointer
                
Information:    #ifdef DEBUGFIFO

Return:         VOID
----------------------------------------------------------------------*/
void
ResetHwPtr(FxU32 hwPtr)
{
    DEBUG_FIX;
    
    _FF(LastHwPtr) = hwPtr - 4;
}


void mySetCF(FxU32 hwPtr, FxU32 hwIndex, FxU32 data);


/*----------------------------------------------------------------------
Function name:  mySetPH

Description:    Set the packet header
                
Information:    #ifdef DEBUGFIFO

Return:         VOID
----------------------------------------------------------------------*/
void 
mySetPH(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 nWords;
    FxU32 jumpTo;
    
    DEBUG_FIX;

    // can print the packet header 
    // DPF(DBG_DEBUG, 128, "DF: PH%d - Addr=0x%08lx[%d] Val=0x%08lx \n", data & 0x7, hwPtr, hwIndex, data );

    if (_FF(InPacket))
    {
	DPF(DBG_DEBUG, 128, "DF! Writing packet header while in packet \n"); 
	GPF();
    }
    
    _FF(InPacket) = 1;
    _FF(CurrPH) = data ;
    _FF(currPHHwPtr) = hwPtr + hwIndex ;
    _FF(currPHData) = data ;

    switch (data & SSTCP_PKT)
    {
      case SSTCP_PKT0:
	  switch (data & ~SSTCP_PKT0_ADDR)
	  {
	    case SSTCP_PKT0_JMP_LOCAL:
		// make sure we're jumping back to top!
		//
		jumpTo = (data & SSTCP_PKT0_ADDR) >> SSTCP_PKT0_ADDR_SHIFT;
		jumpTo <<= 2;
		if (jumpTo != (CMDFIFOSTART - lpDriverData->lfbBase))
		{
		    DPF(DBG_DEBUG, 128, "DF! Not jumping back to top \n");
		    GPF();
		}
		break;

	    default:
	    {
		DPF(DBG_DEBUG, 128, "DF! Invalid packet 0 \n");
		GPF();
	    }
	  }
	  _FF(WordsLeftInPacket) = 1;
	  _FF(Wrapping) = 1;
	  mySetCF(hwPtr, hwIndex, data);
	  _FF(Wrapping) = 0;
	  break;

      case SSTCP_PKT1:
	  nWords = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;
	  if (nWords == 0)
	  {
	      DPF(DBG_DEBUG, 128, "DF! Packet 1 invalid number of words \n");
	      GPF();
	  }
	  _FF(WordsLeftInPacket) = nWords + 1;
	  mySetCF(hwPtr, hwIndex, data);
	  break;

      case SSTCP_PKT2:
	  nWords = countBits(data & SSTCP_PKT2_MASK);
	  if (nWords == 0)
	  {
	      DPF(DBG_DEBUG, 128, "DF! Packet 2 invalid number of words \n");
	      GPF();
	  }
	  _FF(WordsLeftInPacket) = nWords + 1;
	  mySetCF(hwPtr, hwIndex, data);
	  break;
      
      default:
      {
          DPF(DBG_DEBUG, 128, "DF! Invalid packet \n");
          GPF();
      }

    }
}


/*----------------------------------------------------------------------
Function name:  mySetCF

Description:    Set the cmd fifo
                
Information:    #ifdef DEBUGFIFO

Return:         VOID
----------------------------------------------------------------------*/
void
mySetCF(FxU32 hwPtr, FxU32 hwIndex, FxU32 data)
{
    FxU32 rdPtr;
    
    DEBUG_FIX;
    
    // really dump data
    //DPF(DBG_DEBUG, 128,
    //   "DF: PD   - Addr=0x%08lx[%d] Val=0x%08lx \n", hwPtr, hwIndex, data );

    hwPtr += hwIndex * 4;

    rdPtr = LOAD_CMDFIFO_RDPTR(_FF(lpCRegs)) + CMDFIFOOFFSET;
    if (hwPtr == rdPtr)
    {
	if (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) != 0)
	{
	    DPF(DBG_DEBUG, 128, "DF! Command fifo depth not zero \n");
	    GPF();
	}
    }

    if (CMDFIFOSPACE & 0xFF000000)
    {
	DPF(DBG_DEBUG, 128, "DF! Command fifo space gone neg./too large \n");
	GPF();
    }

    // rdptr is incremented past the last word written before it is executed
    if (rdPtr > (CMDFIFOEND+4))
    {
	DPF(DBG_DEBUG, 128, "DF! rdPtr past end of command fifo \n");
	GPF();
    }

    if (rdPtr < CMDFIFOSTART)
    {
	DPF(DBG_DEBUG, 128, "DF! rdPtr before start of command fifo \n");
	GPF();
    }

    if (CMDFIFOSPACE > (((CMDFIFOEND - CMDFIFOSTART + 8) / 4) - 2))
    {
	DPF(DBG_DEBUG,128,"DF! cmdfifospace more free space than available\n");
	GPF();
    }

    if (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) >
	(CMDFIFOEND - CMDFIFOSTART))
    {
	DPF(DBG_DEBUG, 128, "DF! command fifo depth larger than max depth \n");
	GPF();
    }
    
    if (!_FF(InPacket))
    {
	DPF(DBG_DEBUG, 128, "DF! writing command outside of packet \n");
	GPF();
    }

    if (_FF(fifoDwordCount) == 0)
    {
	DPF(DBG_DEBUG, 128, "DF! writing more words than expected \n");
	GPF();
    }

    if ((CMDFIFOSPACE <= 0) && !_FF(Wrapping))
    {
	DPF(DBG_DEBUG, 128, "DF! no command fifo space left & not wrapping\n");
	GPF();
    }

    if (hwPtr != (_FF(LastHwPtr) + 4))
    {
	DPF(DBG_DEBUG, 128, "DF! hwptr farther ahead than last write \n");
	GPF();
    }

    if (hwPtr > CMDFIFOEND)
    {
	DPF(DBG_DEBUG, 128, "DF! hwptr past fifo end \n");
	GPF();
    }

    if (hwPtr < CMDFIFOSTART)
    {
	DPF(DBG_DEBUG, 128, "DF! hwptr less than the fifo start \n");
	GPF();
    }
    
    #ifdef HAL_CSIM
        h3Write( hwPtr,  data);
    #else
        *(FxU32 *)hwPtr = data;
    #endif

    _FF(LastHwPtr) += 4;
    _FF(CurrWR) = 2;

    _FF(WordsLeftInPacket) -= 1;
    if (_FF(WordsLeftInPacket) == 0)
    {
	_FF(InPacket) = 0;

	// last driver to finish a packet & it's packet
	_FF(LastWR) = TH32_SIGNATURE;
	_FF(LastPH) = _FF(CurrPH);

    _FF(lastPHHwPtr)=_FF(currPHHwPtr) ;
    _FF(lastPHData) = _FF(currPHData) ;
    }
}

FxU32 trapOnDW = 0;


/*----------------------------------------------------------------------
Function name:  mySetDW

Description:    Trap on DW writes.
                
Information:    #ifdef DEBUGFIFO

Return:         VOID
----------------------------------------------------------------------*/
void
mySetDW(FxU32 hwPtr, FxU32 data)
{
    DEBUG_FIX;
    
    if (trapOnDW)
    {
    GPF();
    }

#ifdef HAL_CSIM
    h3Write( hwPtr,  data);
#else
    *(FxU32 *)hwPtr = data;
#endif

}

#endif // #ifdef DEBUGFIFO
