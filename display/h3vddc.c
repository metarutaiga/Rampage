/* -*-c++-*- */
/* $Header: h3vdd.c, 3, 6/13/00 7:48:29 PM PDT, John Weidman$ */
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
** File name:   h3vdd.c
**
** Description: VMM/VDD functions.
**
** $Revision: 3$
** $Date: 6/13/00 7:48:29 PM PDT$
**
** $History: h3vdd.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:55p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 96  *****************
** User: Xingc        Date: 3/22/99    Time: 5:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add GetExtraAddr() IOControl function
** 
** *****************  Version 95  *****************
** User: Andrew       Date: 3/18/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to reprogram lfbMemConfig when we go to Full-Screen DOS
** 
** *****************  Version 94  *****************
** User: Stuartb      Date: 3/18/99    Time: 11:56a
** Updated in $/devel/h3/Win95/dx/minivdd
** On DisableGDIDesktop (ie. transitioning to full screen DOS box), fixup
** CRTC registers.  Do the same for tvout if active.
** 
** *****************  Version 93  *****************
** User: Michael      Date: 3/17/99    Time: 4:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS 5179.  In SetVideoMode, comment out clearing of screen when
** resolution is 2046.  Code moved to dd16\h3.c.
** 
** *****************  Version 92  *****************
** User: Stuartb      Date: 3/12/99    Time: 3:40p
** Updated in $/devel/h3/Win95/dx/minivdd
** In H3VDD_GET_TVINIT_STATUS revert back to reading
** BIOS_SCRATCH_REGISTER2.
** 
** *****************  Version 91  *****************
** User: Stuartb      Date: 3/12/99    Time: 8:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Be sure to pass driver new status, cvbsOut and tvBoot.
** 
** *****************  Version 90  *****************
** User: Stuartb      Date: 3/11/99    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move TvOutFn, encoder specific function pointer to DEVTABLE so will
** work with secondary adapter.
** 
** *****************  Version 89  *****************
** User: Stuartb      Date: 3/03/99    Time: 5:03p
** Updated in $/devel/h3/Win95/dx/minivdd
** On MiniVDD_System_Exit reenable TVOUT if appropriate.  Fixes problem
** that only occurs on 'shutdown to MS-DOS' from windows.
** 
** *****************  Version 88  *****************
** User: Stb_srogers  Date: 2/25/99    Time: 6:59a
** Updated in $/devel/h3/win95/dx/minivdd
** Clearing the screen to black during mode switch to the 2046 mode, Fixes
** PRS#4574
** 
** *****************  Version 87  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added call to tvstdToNVRAM().
** 
** *****************  Version 86  *****************
** User: Stuartb      Date: 2/16/99    Time: 3:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** At boot time set tv standard to whatever BIOS thinks it should be.
** 
** *****************  Version 85  *****************
** User: Stuartb      Date: 2/16/99    Time: 11:56a
** Updated in $/devel/h3/Win95/dx/minivdd
** Expose more bits of BIOS SCRATCH_REG2  in H3VDD_GET_TVINIT_STATUS call.
** 
** *****************  Version 84  *****************
** User: Stuartb      Date: 2/10/99    Time: 2:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** One byte change, pass out extra bits in BIOS scratch register that will
** be used for new PAL tvout modes.
** 
** *****************  Version 83  *****************
** User: Ken          Date: 2/08/99    Time: 2:10p
** Updated in $/devel/h3/win95/dx/minivdd
** added cpu/OS detection of pentium III (katmai) processors, and added
** katmai-optimized d3d texture download
** 
** *****************  Version 82  *****************
** User: Stuartb      Date: 2/08/99    Time: 8:59a
** Updated in $/devel/h3/Win95/dx/minivdd
** Change to get pDev from lookup, don't assume isVGA.
** 
** *****************  Version 81  *****************
** User: Stuartb      Date: 2/05/99    Time: 12:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** Do not turn panel off on H3VDD_DISABLE_GDI_DESKTOP  because this
** clobbers BIOS which just turned it on!
** 
** *****************  Version 80  *****************
** User: Stuartb      Date: 2/02/99    Time: 11:02a
** Updated in $/devel/h3/Win95/dx/minivdd
** On DISABLE_GDI_DESKTOP, power down flat panel.
** 
** *****************  Version 79  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 12:48p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 78  *****************
** User: Andrew       Date: 1/29/99    Time: 10:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Small modification to suppress multi-reads of the BIOS Version string
** 
** *****************  Version 77  *****************
** User: Stuartb      Date: 1/26/99    Time: 2:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** Check for lcd enabled before turning it on at mode change.  Fixed oops
** in tvo code.
** 
** *****************  Version 76  *****************
** User: Stuartb      Date: 1/26/99    Time: 2:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** I think someone clobbered me when I had this file out 1/25-26.  Am
** checking in to get back in sync.  No changes made by me.
** 
** *****************  Version 75  *****************
** User: Andrew       Date: 1/21/99    Time: 5:47p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added New Function to Handle Device IO Ctl Messages
** 
** *****************  Version 74  *****************
** User: Stuartb      Date: 1/14/99    Time: 3:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H3VDD_FLATPNL_CTRL, changed H3VDD_RW_REGISTER
** to be able to write byte wide i/o regs.
** 
** *****************  Version 73  *****************
** User: Stuartb      Date: 1/12/99    Time: 2:30p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H3VDD_RW_REGISTER, a helper to simple r/w sstio regs.
** 
** *****************  Version 72  *****************
** User: Michael      Date: 1/08/99    Time: 11:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 71  *****************
** User: Stuartb      Date: 1/06/99    Time: 5:09p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed misleading define H3_VDD_GET_BIOS_SCRATCH_REG to something
** meaningful.
** 
** *****************  Version 70  *****************
** User: Stuartb      Date: 1/05/99    Time: 4:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Reversed order of reenabling flat panel & tvout on mode change.
** 
** *****************  Version 69  *****************
** User: Andrew       Date: 1/05/99    Time: 10:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added function to get bios string at mdoe switch time and a subfunction
** to retrieve it.
** 
** *****************  Version 68  *****************
** User: Stuartb      Date: 12/18/98   Time: 5:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added function hook: H3VDD_FLATPNL_PHYSICAL.
** 
** *****************  Version 67  *****************
** User: Stuartb      Date: 12/15/98   Time: 3:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added call to xlcd enable.  Cleaned up bt868 initialization.
** 
** *****************  Version 66  *****************
** User: Andrew       Date: 12/09/98   Time: 5:29p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed multi-monitor to work with IRQ's
** 
** *****************  Version 65  *****************
** User: Andrew       Date: 12/08/98   Time: 5:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed LockAPI_Init back once more so that we cna work in a
** multi-monitor situation
** 
** *****************  Version 64  *****************
** User: Peter        Date: 12/08/98   Time: 9:16a
** Updated in $/devel/h3/Win95/dx/minivdd
** c-ified video memory fifo support stuff
** 
** *****************  Version 63  *****************
** User: Andrew       Date: 12/05/98   Time: 1:55a
** Updated in $/devel/h3/Win95/dx/minivdd
** Turned off AGP when Banshee is secondary as this fixes a AGP problem
** with a Gateway machine
** 
** *****************  Version 62  *****************
** User: Martin       Date: 12/04/98   Time: 3:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix for multi-mon.
** 
** *****************  Version 61  *****************
** User: Stuartb      Date: 12/02/98   Time: 11:51a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS3382, DVD copy protect requests leaves vidInFormat == TVOUT.
** Do not assum booted to tvout if !bIsVGA or encoder not preset.
** 
** *****************  Version 60  *****************
** User: Peter        Date: 11/30/98   Time: 6:48p
** Updated in $/devel/h3/Win95/dx/minivdd
** query for possibly re-mapped base address
** 
** *****************  Version 59  *****************
** User: Andrew       Date: 11/24/98   Time: 8:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to fill in the AGPCaps when hwinfo is requested
** 
** *****************  Version 58  *****************
** User: Stuartb      Date: 11/19/98   Time: 1:17p
** Updated in $/devel/h3/Win95/dx/minivdd
** More work to support copy protection, impl. color stripe, TriggerBits.
** 
** *****************  Version 57  *****************
** User: Andrew       Date: 11/16/98   Time: 8:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to return the Device ID, Vendor ID, revision Id, and SSIDs.
** 
** *****************  Version 56  *****************
** User: Stuartb      Date: 11/12/98   Time: 12:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed typo.
** 
** *****************  Version 55  *****************
** User: Stuartb      Date: 11/11/98   Time: 9:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** First cut at tvout changes to support Macrovision copy protection.
** Only works under Win98.
** 
** *****************  Version 54  *****************
** User: Stuartb      Date: 11/06/98   Time: 5:41p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor change to make sure that QUERY_TVSTATUS fails properly if no part
** on board.
** 
** *****************  Version 53  *****************
** User: Stuartb      Date: 11/05/98   Time: 8:14a
** Updated in $/devel/h3/Win95/dx/minivdd
** Init tvout size & position upon first envocation.
** 
** *****************  Version 52  *****************
** User: Stuartb      Date: 11/04/98   Time: 3:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** Hook to respawn TV out on VGA to hires if booted to tvout.
** 
** *****************  Version 51  *****************
** User: Stuartb      Date: 10/26/98   Time: 1:11p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added calls for H3VDD_GET_TVSIZE_CTRL.
** 
** *****************  Version 50  *****************
** User: Stuartb      Date: 10/20/98   Time: 3:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Mods to fix random I2c hang with BT868.
** 
** *****************  Version 49  *****************
** User: Stuartb      Date: 10/12/98   Time: 2:33p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed silly oops on I2C_READ/WRITE calls.
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 10/05/98   Time: 4:00p
** Updated in $/devel/h3/Win95/dx/minivdd
** Turned on Driver DDC support
** 
** *****************  Version 47  *****************
** User: Stuartb      Date: 9/17/98    Time: 5:04p
** Updated in $/devel/h3/Win95/dx/minivdd
** Reinstated minimal support for Chronte encoder, added auto detect.
** 
** *****************  Version 46  *****************
** User: Stuartb      Date: 9/16/98    Time: 11:05a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added additional tvout functionality.
** 
** *****************  Version 45  *****************
** User: Stuartb      Date: 9/10/98    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** TVOUT work in progress.
** 
** *****************  Version 44  *****************
** User: Andrew       Date: 8/24/98    Time: 6:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** Need to set return value in ax in DPMS
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 8/23/98    Time: 7:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Commented out the default case in VESASupport.  Put it back in.
** 
** *****************  Version 42  *****************
** User: Andrew       Date: 8/22/98    Time: 12:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed VESA Pre/Post Processing and Stop Hooking Int 10 since the bios
** is now doing DDC
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 7/27/98    Time: 1:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a parameter to SetVideoMode call.
** 
** *****************  Version 40  *****************
** User: Ken          Date: 7/23/98    Time: 4:04p
** Updated in $/devel/h3/win95/dx/minivdd
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** *****************  Version 39  *****************
** User: Ken          Date: 7/18/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** *****************  Version 38  *****************
** User: Stuartb      Date: 6/23/98    Time: 10:16a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added I2C multibyte writes.
** 
** *****************  Version 37  *****************
** User: Stuartb      Date: 6/11/98    Time: 8:34a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added retrace interrupt support, check multiple banshees, share
** interrupt correctly.
** 
** *****************  Version 36  *****************
** User: Andrew       Date: 6/07/98    Time: 8:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added DPMS processing
** 
** *****************  Version 35  *****************
** User: Stuartb      Date: 6/05/98    Time: 4:43p
** Updated in $/devel/h3/Win95/dx/minivdd
** Adding IRQ handling for Vsync.
** 
** *****************  Version 34  *****************
** User: Ken          Date: 6/01/98    Time: 11:49a
** Updated in $/devel/h3/win95/dx/minivdd
** cleaned up indentation and added comments for resetting of DRAM refresh
** in a mode set
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 6/01/98    Time: 8:55a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to set DRAM1[0] to 1.
** 
** *****************  Version 32  *****************
** User: Stuartb      Date: 5/05/98    Time: 1:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Adding i2c extEscape calls
** 
** *****************  Version 31  *****************
** User: Andrew       Date: 4/28/98    Time: 3:25p
** Updated in $/devel/h3/Win95/dx/minivdd
** Set Field Mtrr in DevTable to zero
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 4/26/98    Time: 8:19a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a call to HelpVDD to get the VDD to make a bios call which causes
** it to set a flag such that when DDC is run a int 10 ax=0x3 is not
** generated when causes the FB to get corrupted.
** 
** *****************  Version 29  *****************
** User: Ken          Date: 4/21/98    Time: 6:52p
** Updated in $/devel/h3/win95/dx/minivdd
** clean up mode set, modes seem to set faster now too
** 
** *****************  Version 28  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

/*
 * h3cvxd.c - Main Mini-vdd for 3Dfx Interactive Banshee (code named H3)
 */
#define WIN40SERVICES
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "tv.h"
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#include "shared.h"
#undef  VDDONLY

#include "h3cinit.h"
#include "i2c.h"
#include "tvoutdef.h"
#include "bt868.h"
#include "chrontel.h"
#include "xlcd.h"
#include "p6stuff.h"

extern const struct i2cmask I2C_PROTO;
extern const DWORD I2C_INITVAL;

#pragma VxD_LOCKED_DATA_SEG

// Note Bien!
// So as to not clutter up the include files with #ifdefs removing all
// references to floats, I'm defining _fltused here so that reference to floats
// can be made in macros/inline functions, etc., that aren't actually called.
// Also, you can't reference any floating point library calls -- the driver
// wouldn't link with 'em in there.  -KMW

DWORD _fltused;

// card configuration
#if 0
DWORD   IoBase;                 // base of PCI relocatable IO space
DWORD   PhysMemBase[2];         // physical register address space
DWORD   RegBase;                // CPU linear address mapping to PhysMemBase[0]
DWORD   LfbBase;                // CPU linear address mapping to PhysMemBase[1]
DWORD   MemSizeInMB;            // # of megabytes of board memory

BOOL    InterruptsEnabled = 1;

BOOL    bIsVGA;

#endif
DWORD   OldHandler;
DWORD   Old_3c2_Handler;

#if 0
FxU32 GdiDesktopEnabled;
#endif

FxU32 isP6;

HVM   FullScreenVM;

// Windows data

HVM hSysVM;

// Miscellaneous

WORD     wVESATotalMemory;

// pageable data

#pragma VxD_PAGEABLE_DATA_SEG

// comms with VDD and Config manager
#if 0
CMCONFIG        ConfigData;              // resources of board
DISPLAYINFO     DispInfo;                // data from main VDD
#endif
// config space



// selector for interface

WORD    wMMIfSelector;

/*
TVOUT function pointers for device specific fns.  If we want to support
multiple Banshee cards with differing encoders this will need to be moved
into DevTable.  Right now, it's just Brooktree so it's initialized
statically.
*/

extern TVOUT_DEV_CALLS Bt868DevCalls;
extern TVOUT_DEV_CALLS ChrontelDevCalls;

#define TvOutFn  (((TVOUT_DEV_CALLS *)pDev->tvOutFn))

/* lock.c */
extern FxU32 lockedVM;
extern void LockAPI_ClearLock(void);
extern FxU32 hwcPageMappingFind(const FxU32 mapAddr,
                                const FxU32 remapAddr);
// Begin STB Changes
/* kmtv.c */
extern DWORD  __stdcall GetKernelInfo(DIOCPARAMETERS * pDIOCParms);
// End STB Changes
extern DWORD GetExtraAddr( DIOCPARAMETERS * pDIOCParms);

/****************************************************************************
 Helpers
 ***************************************************************************/

#pragma VxD_ICODE_SEG
#pragma VxD_IDATA_SEG

extern void CommAPI_Init(void);


/*----------------------------------------------------------------------
Function name:  VDD_Get_Mini_Dispatch_Table

Description:    
                
Information:    uses in-line asm.

Return:         DWORD   return from VxDCall 
----------------------------------------------------------------------*/
DWORD 
VDD_Get_Mini_Dispatch_Table(DWORD **pDT)
{
    DWORD dwR;

    __asm pushad;
    VxDCall(VDD_Get_Mini_Dispatch_Table);
    __asm mov dwR,ecx;
    __asm mov eax,pDT;
    __asm mov [eax],edi;
    __asm popad;
    return(dwR);
}


/*----------------------------------------------------------------------
Function name:  VDD_Takeover_VGA_Port

Description:    
                
Information:    uses in-line asm.

Return:         BOOL    return from VxDCall 
----------------------------------------------------------------------*/
BOOL VXDINLINE
VDD_Takeover_VGA_Port( DWORD dwPort, DWORD dwFunc )
{
    BOOL bR;

    __asm mov edx,dwPort;
    __asm mov ecx,dwFunc;
    VxDCall(VDD_Takeover_VGA_Port);
    __asm mov OldHandler, ecx;
    __asm mov eax,0;
    __asm adc al,0;
    __asm mov bR,eax;
    return(bR);
}


/*----------------------------------------------------------------------
Function name:  _BuildDescriptorDWORDs

Description:    
                
Information:    uses in-line asm.

Return:         QWORD   return from VxDCall 
----------------------------------------------------------------------*/
QWORD VXDINLINE
_BuildDescriptorDWORDs(DWORD dwBase, 
                      DWORD dwLimit, 
                      DWORD dwType, 
                      DWORD dwSize,
                      DWORD dwFlags)
{
    QWORD qwR;

    __asm push dwFlags;
    __asm push dwSize;
    __asm push dwType;
    __asm push dwLimit;
    __asm push dwBase;
    VMMCall(_BuildDescriptorDWORDs);
    __asm add esp,5*4;
    __asm mov dword ptr qwR,eax;
    __asm mov dword ptr qwR+4,edx;
    return(qwR);
}


/*----------------------------------------------------------------------
Function name:  _Allocate_LDT_Selector

Description:    
                
Information:    uses in-line asm.

Return:         WORD    return from VxDCall 
----------------------------------------------------------------------*/
WORD VXDINLINE
_Allocate_LDT_Selector( HVM hVM, 
                        DWORD dwDescH, 
                        DWORD dwDescL, 
                        DWORD dwCount,
                        DWORD dwFlags )
{
    WORD wR;

    __asm push dwFlags;
    __asm push dwCount;
    __asm push dwDescL;
    __asm push dwDescH;
    __asm push hVM;
    VMMCall(_Allocate_LDT_Selector);
    __asm add esp,5*4;
    __asm mov wR,ax;
    return(wR);
}


/*----------------------------------------------------------------------
Function name:  _Free_LDT_Selector

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
_Free_LDT_Selector( HVM hVM, 
                    DWORD dwSel, 
                    DWORD dwFlags )
{

    __asm push dwFlags;
    __asm push dwSel;
    __asm push hVM;
    VMMCall(_Free_LDT_Selector);
    __asm add esp,3*4;
}


/*----------------------------------------------------------------------
Function name:  MakeLDTEntry

Description:    
                
Information:

Return:         WORD    return from the _Allocate_LDT_Selector.
----------------------------------------------------------------------*/
WORD
MakeLDTEntry( DWORD dwBase, DWORD dwSize )
{
    union {
        QWORD qwDesc;
        DWORD dwDesc[2];
    } Desc;

    Desc.qwDesc = _BuildDescriptorDWORDs( dwBase,
                                          ((dwSize+4095)>>12)-1,
                                          RW_DATA_TYPE,
                                          D_PAGE32,
                                          0 );
    return( _Allocate_LDT_Selector( hSysVM,
                                    Desc.dwDesc[1],
                                    Desc.dwDesc[0], 1, 0 ) );
}

/****************************************************************************
 MiniVDD_Dynamic_Init
 ***************************************************************************/
#if 0
extern void CheckForP6(void);
extern void GetMTRR(FxU32 msrNum,
                    FxU32 *mtrrLo,
                    FxU32 *mtrrHi);
extern void SetMTRR(FxU32 msrNum,
                    FxU32 mtrrLo,
                    FxU32 mtrrHi);
#endif

void HookInt10(void);


/*----------------------------------------------------------------------
Function name:  MiniVDD_Dynamic_Init

Description:    
                
Information:

Return:         DWORD    VXD_SUCCESS or VXD_FAILURE
----------------------------------------------------------------------*/
DWORD _stdcall
MiniVDD_Dynamic_Init( HVM hWindowsVM )
{
#if 0
    DWORD dwDevID;          // device PCI ID
#endif
    PDWORD pDispTab;        // VDD dispatch table
    volatile int i;
    DWORD dwRevision = 0;
    DWORD dwNFunc;
    PDEVTABLE pDev;
#if 0
    DWORD memBase;
#endif
#if 0
    FxU32 speedInMHz, sgramMode, sgramMask, sgramColor;
    FxU32 mtrr1Lo = 0, mtrr1Hi = 0, mtrr2Lo = 0, mtrr2Hi = 0;
    FxU32 foundMTRR;
#endif

    hSysVM = hWindowsVM;
    FullScreenVM = hSysVM;

    // Initialize the Table    
    for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      DevTable[i].dwDevNode = 0x0;
      DevTable[i].Mtrr = 0x0;
      DevTable[i].dwIMask = 0x0;
      DevTable[i].dwPowerReg[SGRAMMODE_INDEX] = DEFAULT_SGRAMMODE;
      DevTable[i].bBIOSVersion[0] = '\0';
#ifdef NOIRQ
      DevTable[i].InterruptsEnabled = 0;
#else
      DevTable[i].InterruptsEnabled = 0x1;
#endif // #ifdef NOIRQ    
      }

    pVGADevTable = NULL;
    dwNumDevices = 0;
    VDD_Get_DISPLAYINFO( &DevTable[0].DispInfo, sizeof(DevTable[0].DispInfo) );

    Debug_Printf("Are they the same DevNode %x %x\n",
                 DevTable[0].DispInfo.diDevNodeHandle,
                 DevTable[0].DispInfo.diInfoFlags);

    pDev = InitDevNode(DevTable[0].DispInfo.diDevNodeHandle);   
    if (NULL == pDev)
    {
        Debug_Printf( VNAME "Failed InitDevNode\n" );
        return(VXD_FAILURE);
    }
      

    if (!(pDev->DispInfo.diInfoFlags & DEVICE_IS_NOT_VGA))
    {
        if (pVGADevTable != NULL)
        {
            pVGADevTable->GdiDesktopEnabled = 0;
        }
        else
        {
            Debug_Printf(VNAME "huh? VGA, but pVGADevTable is null!\n");
        }
        
        dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispTab);
        if (dwNFunc < NBR_MINI_VDD_FUNCTIONS)
        {
            Debug_Printf( VNAME "Too Few minivdd functions. Version confict?\n" );
            return(VXD_FAILURE);
        }

#if 0
        pDispTab[REGISTER_DISPLAY_DRIVER]= (DWORD)RegisterDisplayDriver;
        pDispTab[DISPLAY_DRIVER_DISABLING]= (DWORD)DisplayDriverDisabling;
        pDispTab[VESA_CALL_POST_PROCESSING]= (DWORD)VESAPostSupport;
        pDispTab[VESA_SUPPORT] = (DWORD)VESASupport;
        pDispTab[PRE_HIRES_TO_VGA] = (DWORD)PreHiResToVGA;
//      pDispTab[POST_HIRES_TO_VGA] = (DWORD)PostHiResToVGA;
        pDispTab[PRE_VGA_TO_HIRES] = (DWORD)PreVGAToHires;
        pDispTab[POST_VGA_TO_HIRES] = (DWORD)PostVGAToHiRes;
        pDispTab[GET_TOTAL_VRAM_SIZE] = (DWORD)GetTotalVRAMSize;
        pDispTab[VIRTUALIZE_SEQUENCER_OUT] = (DWORD)VirtualizeSequencerOut;
        pDispTab[VIRTUALIZE_DAC_OUT] = (DWORD)VirtualizeDacOut;
        pDispTab[VIRTUALIZE_CRTC_OUT] = (DWORD)VirtualizeCRTCout;

        pDispTab[GET_VDD_BANK] = (DWORD)GetVDDbank;

        if (VDD_Takeover_VGA_Port(0x3c2, (DWORD)Virtual_3c2h))
        {
            Debug_Printf( VNAME "Unable to hook port 0x3c2\n" );
            return(VXD_FAILURE);
        }
        Old_3c2_Handler = OldHandler;

        /* dpc - 3 nov 98 - install stuff for locks from
         * fullscreen apps. This currently hooks the
         * CheckScreenSwitch vdd entry-point.
         */
#endif

        // CommAPI_Init();

#if 0
        if (dwNFunc >= NBR_MINI_VDD_FUNCTIONS_41)
        {
            pDispTab[TURN_VGA_OFF] = (DWORD)TurnVGAOff;
            pDispTab[TURN_VGA_ON] = (DWORD)TurnVGAOn;
        }
#endif
    }

    // update info key - ignore errors

//    CM_Write_Registry_Value( 0, "INFO", "ChipType",
//                             REG_SZ, "test", _lstrlen("test"),
//                             CM_REGISTRY_SOFTWARE );
    HookInt10();
#if 0
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "ChipType",
                             REG_SZ, pszChip, _lstrlen(pszChip),
                                                         CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "DACType",
                             REG_SZ, pszDac, _lstrlen(pszDac),
                             CM_REGISTRY_SOFTWARE );
    bVal = 1;
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "HWCursor",
                             REG_BINARY, &bVal, 1,
                             CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "Linear",
                             REG_BINARY, &bVal, 1,
                             CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "MMIO",
                             REG_BINARY, &bVal, 1,
                             CM_REGISTRY_SOFTWARE );
    dwRevision += '0';
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "Revision",
                             REG_SZ, &dwRevision, 1,
                             CM_REGISTRY_SOFTWARE );
    CM_Write_Registry_Value( DispInfo.diDevNodeHandle, "INFO", "VideoMemory",
                             REG_DWORD, &pMMIf->dwFBSize, sizeof(DWORD),
                             CM_REGISTRY_SOFTWARE );
#endif /* #if 0 KMW */


    return(VXD_SUCCESS);
}

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG


/*----------------------------------------------------------------------
Function name:  VMM_Install_IO_Handler

Description:    
                
Information:    uses in-line asm.

Return:         DWORD    return from the VxDCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VMM_Install_IO_Handler(DWORD pCallback, DWORD port)
{
    DWORD retval;

    __asm pushad;
    __asm mov   esi, pCallback;
    __asm mov   edx, port;
    VxDCall(Install_IO_Handler);
    __asm popad;
    __asm mov   eax, 0;
    __asm adc   al, 0;
    __asm mov   retval, eax;

    return retval;
}


/*----------------------------------------------------------------------
Function name:  VMM_Remove_IO_Handler

Description:    
                
Information:    uses in-line asm.

Return:         DWORD    return from the VxDCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VMM_Remove_IO_Handler(DWORD port)
{
    DWORD retval;

    __asm pushad;
    __asm mov   edx, port;
    VxDCall(Remove_IO_Handler);
    __asm popad;
    __asm mov   eax, 0;
    __asm adc   al, 0;
    __asm mov   retval, eax;

    return retval;
}


/*----------------------------------------------------------------------
Function name:  VMM_Enable_Global_Trapping

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
VOID VXDINLINE
VMM_Enable_Global_Trapping(DWORD port)
{
    __asm pushad;
    __asm mov   edx, port;
    VxDCall(Install_IO_Handler);
    __asm popad;
}


/*----------------------------------------------------------------------
Function name:  EnableIoHandler

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
EnableIoHandler()
{
    DWORD port;
    
    if (NULL != pVGADevTable)
    {
        for (port = 0; port < 0x100; port += 4)
        {
            if (VMM_Install_IO_Handler((DWORD)myIOhandler,
                                       pVGADevTable->IoBase + port))
            {
                Debug_Printf( VNAME "port trap failed for port %d\n", port);
                return;
            }

            VMM_Enable_Global_Trapping(pVGADevTable->IoBase + port);
        }
    }
}


/*----------------------------------------------------------------------
Function name:  DisableIoHandler

Description:    
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
DisableIoHandler()
{
    DWORD port;
    
    if (NULL != pVGADevTable)
    {
        for (port = 0; port < 0x100; port += 4)
        {
            if (VMM_Remove_IO_Handler(pVGADevTable->IoBase + port))
            {
                Debug_Printf( VNAME "port untrap failed for port %d\n", port);
                return;
            }
        }
    }
}


/*----------------------------------------------------------------------
Function name:  VMM_SelectorMapFlat

Description:    
                
Information:    

Return:         FxU32   return from the VMMCall
----------------------------------------------------------------------*/
FxU32 VXDINLINE
VMM_SelectorMapFlat(FxU32 vm, FxU32 selector, FxU32 flags)
{
    FxU32 retval;
    
    __asm pushad;
    __asm push flags;
    __asm push selector;
    __asm push vm;
    VMMCall(_SelectorMapFlat);
    __asm add esp, 3*4;
    __asm mov retval, eax;
    __asm popad;

    return retval;
}


#define _NP() \
  __asm pushfd \
  __asm pushad \
  __asm mov ebp, esp \
  __asm sub esp, __LOCAL_SIZE

#define _NE() \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd \
  __asm ret

#define _NE2(x) \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd \
  __asm x \
  __asm ret

#define _NE_NORET() \
  __asm mov esp, ebp \
  __asm popad \
  __asm popfd


DWORD _4planeVirtArea;


/*----------------------------------------------------------------------
Function name:  GetVDDbank

Description:    
                
Information:    uses in-line asm.

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
GetVDDbank( VOID )
{
    _NP();

    __asm mov _4planeVirtArea, ecx;

    _NE_NORET();
    __asm 
    {
        mov     ah, 0;          /* bank offset into the virtualization area */
        mov     edx, 0x8000;    /* 32 K virtualization area */
        ret;
    }
}


/*----------------------------------------------------------------------
Function name:  myIOhandler

Description:    virtualizes the pci relocatable i/o registers
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
myIOhandler( VOID )
{
    DWORD hVM, ioType;
    
    _NP();
    __asm mov   hVM, ebx;
    __asm mov   ioType, ecx;

    /* only allow output to i/o ports if we're not on the windows
     * desktop (i.e., in full screen VGA/VESA).  Always allow
     * input from h/w ports.
     */
    if ((FullScreenVM != hSysVM) || !(ioType & OUTPUT))
    {
        _NE_NORET();
        VxDCall(VDD_Do_Physical_IO);
        __asm ret;
    }
    
    _NE();
}


/*----------------------------------------------------------------------
Function name:  VirtualizeCRTCout

Description:    Called whenever a i/o trapped port write to
                the VGA crtc is made.
Information:    
from msdn:
 Call With
 EAX: Contains the value to output to port (on an OUT call). 
 EBX: Contains the VM handle for which the I/O is being done. 
 ECX: Contains the I/O flags (documented in VMM.INC). 
 EDX: Contains the port that the I/O is to be done to or from. 
 EBP: Points to VM's client registers. 
 Return Values
 CY set indicates that the mini-VDD has done the I/O and that the Main
 VDD should take no further action. In this case, AL should contain the
 return value from the IN call and all registers except EBX and EBP may
 be destroyed.
 NC set indicates that the mini-VDD may have taken some action but wants
 the Main VDD to do the default action. In this case the mini-VDD must
 not have destroyed ANY registers.

 3Dfx Virtualization strategy: we use the VGA crtc to control both the
 vga and hires video timings.  When displaying on the desktop, writes
 to the vga crtc thus need to be dumped on the floor.

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
VirtualizeCRTCout( VOID )
{
    DWORD dumpAccess;
    DWORD hVM;
    
    _NP();
    __asm
    {
        mov     hVM, ebx;
    }

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }

    if (dumpAccess)
    {
        _NE2(stc);
    }
    else
    {
        _NE2(clc);
    }
}


/*----------------------------------------------------------------------
Function name:  VirtualizeSequencerOut

Description:    Parameters and return values are the same as
                VirtualizeCRTCout.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
VirtualizeSequencerOut( VOID )
{
    DWORD dumpAccess;
    DWORD hVM;
    
    _NP();
    __asm mov   hVM, ebx;

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }
    
    if (dumpAccess)
    {
        _NE2(stc);
    }
    else
    {
        _NE2(clc);
    }
}


/*----------------------------------------------------------------------
Function name:  VirtualizeDacOut

Description:    Parameters and return values are the same as
                VirtualizeCRTCout
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
VirtualizeDacOut( VOID )
{
    DWORD dumpAccess;
    DWORD hVM;
    
    _NP();
    __asm mov   hVM, ebx;

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }
    
    if (dumpAccess)
    {
        _NE2(stc);
    }
    else
    {
        _NE2(clc);
    }
}


/*----------------------------------------------------------------------
Function name:  LockVgaTimingRegisters

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
LockVgaTimingRegisters()
{
    PDEVTABLE pDev = pVGADevTable;
    FxU32 vgainit1;
    
    // lock VGA video timing registers
    //
    if (NULL != pDev)
    {
        vgainit1 = IGET32(vgaInit1);
        vgainit1 |= (BIT(21) | BIT(22) | BIT(23) | BIT(24) |
                     BIT(25) | BIT(26) | BIT(27) | BIT(28));
        ISET32(vgaInit1, vgainit1);
    }
}


/*----------------------------------------------------------------------
Function name:  UnLockVgaTimingRegisters

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
UnlockVgaTimingRegisters()
{
    PDEVTABLE pDev = pVGADevTable;
    FxU32 vgainit1;
    
    // unlock VGA video timing registers
    //
    if (NULL != pDev)
    {
        vgainit1 = IGET32(vgaInit1);
        vgainit1 &= ~(BIT(21) | BIT(22) | BIT(23) | BIT(24) |
                      BIT(25) | BIT(26) | BIT(27) | BIT(28));
        ISET32(vgaInit1, vgainit1);
    }
}


/*----------------------------------------------------------------------
Function name:  EnableGdiDesktop

Description:    Called from SetVideoMode to setup the protections &
                virtualizations that will prevent DOS apps & the
                BIOS  from clobbering VGA video timing registers
                and relocatable IO registers that the desktop
                requires.   Called either on a full-screen DOS to
                desktop transition, or during a desktop mode change.
                Don't re-trap i/o on a dekstop mode change.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
EnableGdiDesktop()
{
    LockVgaTimingRegisters();

    // don't re-enable the io handler if it's already enabled
    // (on a desktop to desktop mode change)
    //
    if (NULL != pVGADevTable)
    {
        if (pVGADevTable->GdiDesktopEnabled)
            return;

        EnableIoHandler();

        pVGADevTable->GdiDesktopEnabled = 1;
    }
}


#define SETDW(hwRegister, data)   hwRegister = data
/*----------------------------------------------------------------------
Function name:  DisableGdiDesktop

Description:    Called from a display driver VDDcall when the GDI
                desktop is being disabled and disables VGA/IO
                virtualizations, thus allowing VGA/VESA mode
                changes to occur.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
DisableGdiDesktop(PDEVTABLE pDev)
{
    SstIORegs *lpIOregs;
    WORD wCmd;

    if (pDev->bIsVGA)
    {
        if (!pDev->GdiDesktopEnabled)
            return;

        UnlockVgaTimingRegisters();
        DisableIoHandler();
        lpIOregs = (SstIORegs * )pDev->RegBase;
        SETDW(lpIOregs->lfbMemoryConfig, pDev->lfbMemoryConfig) ;
    }
    else
    {
        // Read Command Register
        CM_Call_Enumerator_Function( pDev->dwDevNode,
                                     PCI_ENUM_FUNC_GET_DEVICE_INFO,
                                     0x4, &wCmd, 
                                     sizeof(WORD), 0 );
            
        // Turn off Everything
        wCmd &= ~(0x03);
        CM_Call_Enumerator_Function( pDev->dwDevNode,
                                     PCI_ENUM_FUNC_SET_DEVICE_INFO,
                                     0x4, &wCmd, 
                                     sizeof(WORD), 0 );
    }  

    pDev->GdiDesktopEnabled = 0;
}


/*----------------------------------------------------------------------
Function name:  SetVideoMode

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
SetVideoMode(VidProcConfig *pVpc, PDEVTABLE pDev)
{
    FxU32 dramInit1;
#if 0
	int i;
	DWORD *pDesktopSurface;
#endif
    UnlockVgaTimingRegisters();

    // don't enable the video processor now, or you'll get an ugly
    // mode set (e.g., you'll see the contents of uninitialized memory)
    //
    h3InitVideoProc(pDev->IoBase, SST_CURSOR_MICROSOFT);
    
    if (pVpc->changeDesktop)
    {
        h3InitVideoDesktopSurface(pDev->IoBase,
                                  pVpc->desktopSurface.enable,
                                  pVpc->desktopSurface.tiled,
                                  pVpc->desktopSurface.pixFmt,
                                  pVpc->desktopSurface.clutBypass,
                                  pVpc->desktopSurface.clutSelect,
                                  pVpc->desktopSurface.startAddress,
                                  pVpc->desktopSurface.stride);

    }
    
    if (pVpc->changeOverlay)
    {
        h3InitVideoOverlaySurface(pDev->IoBase,
                                  pVpc->overlaySurface.enable,
                                  pVpc->overlaySurface.stereo,
                                  pVpc->overlaySurface.horizScaling,
                                  pVpc->overlaySurface.dudx,
                                  pVpc->overlaySurface.verticalScaling,
                                  pVpc->overlaySurface.dvdy,
                                  pVpc->overlaySurface.filterMode,
                                  pVpc->overlaySurface.tiled,
                                  pVpc->overlaySurface.pixFmt,
                                  pVpc->overlaySurface.clutBypass,
                                  pVpc->overlaySurface.clutSelect,
                                  pVpc->overlaySurface.startAddress,
                                  pVpc->overlaySurface.stride);
    }
    
    if (pVpc->changeVideoMode)
    {
        h3InitSetVideoMode(pDev->IoBase,
                           pVpc->width,
                           pVpc->height,
                           pVpc->refresh,
#ifdef H3_B0
                                1,
            pVpc->desktopSurface.scanlinedouble); 
#else
                           1);          // don't initialize CLUT
#endif

        // turn back on extended (non-vga) DRAM refresh -- the BIOS always
        // turns this off when setting a VGA mode.
        // 
        dramInit1 = IGET32(dramInit1);
        dramInit1 |= SST_DRAM_REFRESH_EN;
        ISET32(dramInit1, dramInit1);
    }

    EnableGdiDesktop();
	// check to see if flat panel is enabled
	outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
	if (inp ((FxU16)(pDev->IoBase + 0xd5)) & 4)
		panelSet (pDev, 1);    // flat panel setup
	if (pDev->tvOutActive && TvOutFn)
		TvOutFn->tvout_Enable (pDev, -1);
}


/*----------------------------------------------------------------------
Function name:  VMM_Get_DDB

Description:    Used to get the DDB for the VMM

Information:    

Return:         PVMMDDB
----------------------------------------------------------------------*/
#define MY_GET_DDB GetVxDServiceOrdinal(Get_DDB)
VXDINLINE PVMMDDB VMM_Get_DDB(DWORD DeviceID, DWORD Name)
{
    DWORD p;
    __asm mov eax, DeviceID;
    __asm mov edi, Name;
    _asm _emit 0xcd \
    _asm _emit 0x20 \
    _asm _emit (MY_GET_DDB) & 0xff \
    _asm _emit (MY_GET_DDB >> 8) & 0xff \
    _asm _emit (MY_GET_DDB >> 16) & 0xff \
    _asm _emit (MY_GET_DDB >> 24) & 0xff; 
    __asm mov p, ecx;
    return((PVMMDDB)p);
}

DWORD dwFirstTime = 0;
void HelpVDD(CLIENT_STRUCT * pCR, FxU32 vm);
void AgpService(AgpSrvc *pAs);
void GetBIOSVersion(PDEVTABLE pDevTable, FxU32 vm);


/*----------------------------------------------------------------------
Function name:  RegisterDisplayDriver

Description:    Called as a general communication method between
                the display driver and the mini VDD.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM 

Return:         PVMMDDB
----------------------------------------------------------------------*/
_declspec ( naked ) VOID RegisterDisplayDriver( VOID )
{
    BYTE PCIConfSpace[256];
    PVMMDDB pDDB;
    CLIENT_STRUCT *pCR;           // client registers
    FxU32 vm;
    extern DWORD func_table[];
    VidProcConfig *pVpc;
    AgpSrvc *pAgpSrvc;
    HwInfo *pHwInfo;
    HWGETBIOSVERSION * pHWGetBIOSVersion;
    PDEVTABLE pDev;
    struct i2cmask I2CMask;
    DWORD *lpdw;
    void *tvout;
    int nCapIndex;
    int i;

    __asm mov   eax, ebp;               // save ebp, _NP() will wack it
    _NP();
    __asm mov   pCR, eax;
    __asm mov   vm, ebx;
    
    switch( pCR->CRS.Client_ECX )
    {
    case H3VDD_GET_HW_INFO:
	// translate dd16's es:di into a flat pointer to return the
	// memory bases & memory size to the display driver
	//
#if 0
	pHwInfo = (HwInfo *) VMM_SelectorMapFlat(vm,
						 pCR->CRS.Client_ES,
						 0);
	pHwInfo = (HwInfo *)((FxU32)pHwInfo + pCR->CWRS.Client_DI);
#else
	pHwInfo = (HwInfo *)(pCR->CRS.Client_EDI);
#endif

	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	if (NULL != pDev)
	{
	    pHwInfo->regBase = pDev->RegBase;
#ifndef NOIRQ
	    // enable PCI interrupts, no selective interrupts enabled at this
	    // point so nothing should happen.
	    ((SstIORegs *)pDev->RegBase)->pciInit0 |= 0x40000;
#endif // #ifdef NOIRQ    
	    pHwInfo->lfbBase = pDev->LfbBase;
	    pHwInfo->ioBase = pDev->IoBase;
	    pHwInfo->memSizeInMB = pDev->MemSizeInMB;
         
	    CM_Call_Enumerator_Function( pDev->dwDevNode,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO,
					 SST_PCI_DEVICE_ID, &PCIConfSpace,
					 sizeof(PCIConfSpace), 0);
 
	    pHwInfo->VendorDeviceID = *(FxU32 *)(&PCIConfSpace[SST_PCI_DEVICE_ID]);
	    pHwInfo->RevisionID = 0x0;
	    pHwInfo->RevisionID |= PCIConfSpace[SST_PCI_REVISION_ID];
	    pHwInfo->SSID = *(FxU32 *)(&PCIConfSpace[SST_PCI_SSID]);

	    // Determine AGP capabilities
	    pHwInfo->AGPCaps = 0x0;

	    // Put this in to solve a problem with a Gateway box
	    // where AGP was not happening?????
	    if (pDev->bIsVGA)
	    {
		for (nCapIndex = PCIConfSpace[SST_1ST_CAP];
		     0x0 != nCapIndex;
		     nCapIndex=PCIConfSpace[nCapIndex+1])
		{
		    if (SST_AGP_CAP == PCIConfSpace[nCapIndex])
		    {
			pHwInfo->AGPCaps |= IS_AGP_CARD;
			break;
		    } 
		}
	    }

	    pDDB = VMM_Get_DDB(VMM_DEVICE_ID, NULL);
	    if ((NULL != pDDB) &&
		((GetVxDServiceOrdinal(_GARTMemAttributes) & 0xFFFF) <=
		 pDDB->DDB_Service_Table_Size))
	    {
		pHwInfo->AGPCaps |= IS_GART_AVAILABLE; 
	    }

	    pHwInfo->cpuType = 0;
	    if (isP6 == (P6_INTELCPU_WITH_MTRRS | P6_INTELCPU_WITH_KNI))
		pHwInfo->cpuType |= P6_INTELCPU_WITH_KNI;
	}
	break;

    case H3VDD_SET_VIDEO_MODE:
	// The VDD needs a int 10 to warm up.  The first time the VDD
	// hooks a int 10 it issues a int 10 <ax=0x0003>.  This will corrupt
	// our frame buffer. This is late enough in the init process 
	// and is a good time to give it one.
	if (0 == dwFirstTime) 
	{
	    HelpVDD(pCR, vm);
	    dwFirstTime = 1;
	}

	// translate dd16's es:di into a flat pointer to get the vpc
	// settings for this mode
#if 0
	pVpc = (VidProcConfig *) VMM_SelectorMapFlat(vm,
						     pCR->CRS.Client_ES,
						     0);
	pVpc = (VidProcConfig *)((FxU32)pVpc + pCR->CWRS.Client_DI);
#else
	pVpc = (VidProcConfig *)(pCR->CRS.Client_EDI);
#endif

	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	if (NULL != pDev) {
//            if (lockedVM != 0x00UL) LockAPI_ClearLock();
            SetVideoMode(pVpc, pDev);
            if ('\0' == pDev->bBIOSVersion[0])
		GetBIOSVersion(pDev, vm);
	}
	break;

    case H3VDD_GET_FN_TABLE32:
	pCR->CRS.Client_EAX = (FxU32) func_table;
	break;

    case H3VDD_DISABLE_GDI_DESKTOP:
	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	if (NULL != pDev)
	{
	    DisableGdiDesktop(pDev);
		outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
		if (inp ((FxU16)(pDev->IoBase + 0xd5)) & 4)   // if panel is active
			panelFixupVGA (pDev, 0);
		if (pDev->tvOutActive && TvOutFn)             // if tvout is active
			TvOutFn->tvout_FixupVGA (pDev, 0);
	}
	break;

    case H3VDD_I2C_WRITE:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	I2CMask = I2C_PROTO;
	I2CMask.pReg = (DWORD *)(pDev->RegBase + I2COUT_PORT);
	*I2CMask.pReg |= I2C_INITVAL;
	I2CMask.bAddr = (FxU8)(pCR->CRS.Client_EDX >> 16);
	I2CMask.nReg = (FxU8)(pCR->CRS.Client_EDX >> 8);
	I2CMask.dwData = (FxU8)(pCR->CRS.Client_EDX >> 0);
	/* Debug_Printf( VNAME "i2c parm = %x\n", pCR->CRS.Client_EDX); */
	pCR->CRS.Client_EAX = WriteI2CRegister (&I2CMask);
	break;

    case H3VDD_I2C_MULTI_WRITE:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	// convert 16bit _far ptr to flat
	lpdw = (DWORD*)VMM_SelectorMapFlat(vm, pCR->CRS.Client_EDX >> 16, 0);
	lpdw = (DWORD *)((FxU32)lpdw + (pCR->CRS.Client_EDX & 0xffff));
	I2CMask = I2C_PROTO;
	I2CMask.pReg = (DWORD *)(pDev->RegBase + I2COUT_PORT);
	*I2CMask.pReg |= I2C_INITVAL;
	I2CMask.bAddr = (FxU8)(lpdw[0] >> 16);
	I2CMask.nReg = (FxU8)(lpdw[0] >> 8);
	I2CMask.nSize = (FxU8)(lpdw[0] >> 0);
	I2CMask.multiWriteData = (FxU8 *)lpdw[1];
	/* Debug_Printf( VNAME "i2c parm = %x\n", pCR->CRS.Client_EDX); */
	pCR->CRS.Client_EAX = WriteI2CRegisterMulti (&I2CMask);
	break;

    case H3VDD_I2C_READ:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	I2CMask = I2C_PROTO;
	I2CMask.pReg = (DWORD *)(pDev->RegBase + I2COUT_PORT);
	*I2CMask.pReg |= I2C_INITVAL;
	I2CMask.bAddr = (FxU8)(pCR->CRS.Client_EDX >> 8);
	I2CMask.nReg = (FxU8)(pCR->CRS.Client_EDX >> 0);
	/* Debug_Printf( VNAME "i2c parm = %x\n", pCR->CRS.Client_EDX); */
	if (FXTRUE == ReadI2CRegister (&I2CMask))
	    pCR->CRS.Client_EAX = I2CMask.dwData | 0x100;
	else
	    pCR->CRS.Client_EAX = 0;    /* failed */

	break;

    case H3VDD_AGP_SERVICE:
	pAgpSrvc = (AgpSrvc *) VMM_SelectorMapFlat(vm,
						   pCR->CRS.Client_ES,
						   0);
	pAgpSrvc = (AgpSrvc *)((FxU32)pAgpSrvc + pCR->CWRS.Client_DI);
	AgpService(pAgpSrvc);
	//
	// AgpService places the return value into the AgpSrvc structure
	//
	break;

    case H3VDD_GET_BIOS_VERSION:
	pCR->CRS.Client_EAX = 0xFFFFFFFF;
	pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI);
	if (NULL != pDev)
	{
	    pHWGetBIOSVersion = (HWGETBIOSVERSION *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	    if (0xFFFFFFFF == (DWORD)pHWGetBIOSVersion)
		break; 
	    pHWGetBIOSVersion = (HWGETBIOSVERSION *)((DWORD)pHWGetBIOSVersion + pCR->CWRS.Client_DI);
	    for (i=0; i<MAX_BIOS_VERSION_STRING; i++)
		pHWGetBIOSVersion->bBIOSVersion[i] = pDev->bBIOSVersion[i];                   
	    pCR->CRS.Client_EAX = 0;
	}
	break;

    case H3VDD_GET_TVINIT_STATUS:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
		pCR->CRS.Client_EAX = 0xDEADBEEF;
		break;
	}
	// if unitialized return -1
	pCR->CRS.Client_EAX = 0;    // no tv hardware case
	if (((TVOUT_CURSETUP *)pDev->tvOutData)->encoderInitAttempt)
	{
		if (TvOutFn)
		{
			outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);       // BIOS' tvstd
			pCR->CRS.Client_EAX = inp ((FxU16)(pDev->IoBase + 0xd5));
		}
	}
	else
		pCR->CRS.Client_EAX = 0xffffffff;

	break;

	//TV-Out support functions     
    case H3VDD_GET_TVSTATUS:
    case H3VDD_SET_TVSTANDARD:        
    case H3VDD_SET_TVCONTROL:
    case H3VDD_SET_TVPOSITION:
    case H3VDD_SET_TVSIZE:
    case H3VDD_SET_SPECIAL:
    case H3VDD_GET_TVPOSITION_CTRL:
    case H3VDD_GET_TVSIZE_CTRL:
    case H3VDD_GET_TVPIC_CTRL:
    case H3VDD_GET_FILTER_CTRL:
    case H3VDD_DISABLE_TV:
    case H3VDD_SET_REGISTRY_INFO:
    case H3VDD_GET_REGISTRY_INFO:
    case H3VDD_GET_PIC_CAPS:
    case H3VDD_GET_FILTER_CAPS:
    case H3VDD_GET_POS_CAPS:
    case H3VDD_GET_SIZE_CAPS:
    case H3VDD_GET_STANDARD:
    case H3VDD_MACROVISION_ON:
    case H3VDD_MACROVISION_OFF:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	if (TvOutFn == NULL)
	{
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->encoderInitAttempt = 1;
	    if (BT868_GetStatus (pDev, 0) >= 0)
		TvOutFn = &Bt868DevCalls;
	    else if (Chrontel_GetStatus (pDev, 0) >= 0)
		TvOutFn = &ChrontelDevCalls;
	    else   // try enabling clock to BT868 
	    {
		if (BT868_GetStatus (pDev, 0) >= 0)
		    TvOutFn = &Bt868DevCalls;
		else
		    pCR->CRS.Client_EAX = 0xDEADBEEF; 

		break;
	    }
				// set some defaults that will be overwritten by app later
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->size = 2;   // middle size
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->hpos = 50;  // h centre
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->vpos = 50;  // v centre
		outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);       // BIOS' tvstd
		i = inp ((FxU16)(pDev->IoBase + 0xd5));
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->tvStd = i & BIOS_TVSTD_MASK;
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->cvbsOut = !!(i & 0x40);
	    ((TVOUT_CURSETUP *)pDev->tvOutData)->tvBoot = !!(i & 0x20);
	}
	tvout = (PTVSETSTANDARD)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	tvout = (PTVSETSTANDARD)((FxU32)tvout + pCR->CWRS.Client_DI);
	switch (pCR->CRS.Client_ECX)
	{
	case H3VDD_GET_TVSTATUS:
	    pCR->CRS.Client_EAX = (FxU32)TvOutFn->tvout_GetStatus (pDev, 0);
	    if (pCR->CRS.Client_EAX == 0xffffffff)    // error
		break;
	    pCR->CRS.Client_EAX = (pCR->CRS.Client_EAX << 8) | 
		(FxU32)TvOutFn->tvout_GetStatus (pDev, 1);
	    pCR->CRS.Client_EAX = (pCR->CRS.Client_EAX << 8) | 
		(FxU32)TvOutFn->tvout_GetStatus (pDev, 2);

	    pCR->CRS.Client_EAX &= ~0x80;
	    if (pDev->tvOutActive)
		pCR->CRS.Client_EAX |= 0x80;
	    break;
	case H3VDD_SET_TVSTANDARD:
		if (tvstdToNVRAM (pDev, (PTVSETSTANDARD)tvout) < 0)
			Debug_Printf (VNAME "tvstdToNVRAM failed\n");
	    pCR->CRS.Client_EAX = \
			(FxU32)TvOutFn->tvout_SetStandard ((PTVSETSTANDARD)tvout, pDev);
	    pDev->tvOutActive = (pCR->CRS.Client_EAX == FXTRUE);
	    break;
	case H3VDD_SET_TVCONTROL:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_SetPicControl ((PTVSETCAP)tvout, pDev);
	    break;
	case H3VDD_SET_TVPOSITION:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_SetPosition ((PTVSETPOS)tvout, pDev);
	    break;
	case H3VDD_SET_TVSIZE:
	    pCR->CRS.Client_EAX = \
		(FxU32)TvOutFn->tvout_SetSize ((PTVSETSIZE)tvout, pDev);
	    break;
	case H3VDD_SET_SPECIAL:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_SetSpecial((PTVSETSPECIAL)tvout,
						 pDev->RegBase);
	    break;
	case H3VDD_DISABLE_TV:
	    pCR->CRS.Client_EAX =  \
		(FxU32)TvOutFn->tvout_Disable (pDev->RegBase);
	    pDev->tvOutActive = 0;
	    break;
	case H3VDD_GET_TVPOSITION_CTRL:
	    pCR->CRS.Client_EAX =  \
		TvOutFn->tvout_GetPosition ((PTVCURPOS)tvout, &pDev->tvOutData);
	    break;
	case H3VDD_GET_TVPIC_CTRL:
	    pCR->CRS.Client_EAX =  \
		TvOutFn->tvout_GetPicControl ((PTVCURCAP)tvout,
					      &pDev->tvOutData);
	    break;
	case H3VDD_GET_FILTER_CTRL:
	    break;
	case H3VDD_GET_REGISTRY_INFO:
	    pCR->CRS.Client_EAX = tvOutGetNextValue ((FxU8 *)tvout,
						     (TVOUT_CURSETUP *)&pDev->tvOutData);
	    break;
	case H3VDD_SET_REGISTRY_INFO:
	    pCR->CRS.Client_EAX = tvOutSetNextValue ((FxU8 *)tvout,
						     pCR->CRS.Client_EDX, (TVOUT_CURSETUP *)&pDev->tvOutData);
	    break;
	case H3VDD_GET_STANDARD:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetStandard (pDev, tvout);
	    break;
	case H3VDD_GET_SIZE_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetSizeCap (pDev, tvout);
	    break;
	case H3VDD_GET_POS_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetPosCap (pDev, tvout);
	    break;
	case H3VDD_GET_FILTER_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetFilterCap (pDev, tvout);
	    break;
	case H3VDD_GET_PIC_CAPS:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_GetPicCap (pDev, tvout);
	    break;
	case H3VDD_GET_TVSIZE_CTRL:
	    pCR->CRS.Client_EAX = \
		TvOutFn->tvout_GetSizeControl (tvout, &pDev->tvOutData);
	    break;
	case H3VDD_MACROVISION_ON:
	    pCR->CRS.Client_EAX = \
		TvOutFn->tvout_CopyProtect (pDev, *(int *)tvout);
	    break;
	case H3VDD_MACROVISION_OFF:
	    pCR->CRS.Client_EAX = TvOutFn->tvout_CopyProtect (pDev, 0);
	    break;
	}
	break;

    case H3VDD_LINEAR_MAP_OFFSET:
    {
	MapInfo* clientData;
	FxU32* linAddr;
	FxU32 selBase = VMM_SelectorMapFlat(vm,
					    pCR->CRS.Client_ES,
					    0);
	clientData = (MapInfo*)(selBase + pCR->CWRS.Client_DX);
	linAddr = (FxU32*)(selBase + pCR->CWRS.Client_DI);

	*linAddr = hwcPageMappingFind(clientData->mapAddr,
				      clientData->remapAddr);
    }
    break;

    case H3VDD_FLATPNL_PHYSICAL:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	pCR->CRS.Client_EAX = flatPanelPhysical (pDev, pCR->CWRS.Client_DI + VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0));
	break;

    case H3VDD_RW_REGISTER:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	tvout = (void *)VMM_SelectorMapFlat(vm, pCR->CRS.Client_ES, 0);
	tvout = (void *)((FxU32)tvout + pCR->CWRS.Client_DI);
	lpdw = (DWORD *)(pDev->RegBase + ((SstIoRegRW *)tvout)->regAddr);

	if (((SstIoRegRW *)tvout)->regRWflags & RWFLAG_WRITE_BYTE)
	{
	    outp ((FxU16)(pDev->IoBase + \
			  ((SstIoRegRW *)tvout)->regAddr),
		  (FxU8)((SstIoRegRW *)tvout)->regValue);
	}
	else if (((SstIoRegRW *)tvout)->regRWflags & RWFLAG_WRITE_REG)
	    *lpdw = ((SstIoRegRW *)tvout)->regValue;
	else
	    ((SstIoRegRW *)tvout)->regValue = *lpdw;
	break;

    case H3VDD_FLATPNL_CTRL:
	if (!(pDev = FindPDEVFromDevNode(pCR->CRS.Client_ESI)))
	{
	    pCR->CRS.Client_EAX = 0xDEADBEEF;
	    break;
	}
	panelSet (pDev, pCR->CWRS.Client_DX);
	break;

    default:           
	pCR->CRS.Client_EAX = 0xDEADBEEF;
	break;
    }

    _NE();
}

int DPMSSupport(PCRS pCRS, PDEVTABLE pDevTable);
int DDCSupport(PDEVTABLE pDevTable, PCRS pCRS);
int nInDDC = FALSE;


/*----------------------------------------------------------------------
Function name:  VESASupport

Description:    Called before passing VESA command to BIOS.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM which we use to ignore
  Exit..
  CY set -  Handled.. no need to call BIOS or TSR
  CY clear - Normal VESA processing

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID VESASupport( VOID )
{
    PCRS pCR;           // client registers
    PDEVTABLE pDevTable;
    int passToBIOS;
    int failCall;
    int nSkip;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
        mov     eax,[ebp];      // get client ptr
        mov     pCR,eax;
    }

    passToBIOS = 0;
    failCall = 0;
    nSkip = 1;    

    // route VESA call to correct handler

    switch( pCR->Client_EAX & 0xffff )
    {
      case 0x4f00:
          passToBIOS = 1;
          break;

      case 0x4F10:
         pDevTable = FindActiveBanshee();
         if (NULL == pDevTable)
            passToBIOS = 1;
         else
            {
            nSkip = 0;
            passToBIOS = 0;
            DPMSSupport(pCR, pDevTable);
            }
         break;

      case 0x4F15:
#if 1
         pDevTable = FindActiveBanshee();
         if (NULL == pDevTable)
            passToBIOS = 1;
         else
            {
            passToBIOS = !DDCSupport(pDevTable, pCR);
            nSkip = 0;
            }
#else
         nInDDC = TRUE;
         if (1 == pVGADevTable->GdiDesktopEnabled)
            { 
              if (VMM_Remove_IO_Handler(pVGADevTable->IoBase + 0x78))
                    {
                         Debug_Printf( VNAME "port untrap failed for port %d\n", 0x78);
                    }
            } 
              passToBIOS = 1;
#endif
         break;

      default:
          if (pVGADevTable->GdiDesktopEnabled)
              failCall = 1;
          else
              passToBIOS = 1;
          break;
    }

    if (passToBIOS)
    {
        __asm
        {
            pop     edi;
            pop     esi;
            clc;
            mov     esp,ebp;
            pop     ebp;
            ret;
        }
    }
    else
    {
      if (nSkip)
         {
           // handle the call here, returning either sucess or failure
              pCR->Client_EAX &= ~0xffff;
        
         if (failCall)
                  pCR->Client_EAX |= 0x034f;    // func. call invalid in curr. mode
              else
                 pCR->Client_EAX |= 0x004f;     // call succeeded
         }
      }

        __asm
        {
            pop     edi;
            pop     esi;
            stc;
            mov     esp,ebp;
            pop     ebp;
            ret;
        }
}


/*----------------------------------------------------------------------
Function name:  VESAPostSupport

Description:    Called after passing VESA command to BIOS.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM which we use to ignore

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID VESAPostSupport( VOID )
{
    PCRS pCR;           // client registers

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
        mov     eax,[ebp];      // get client ptr
        mov     pCR,eax;
    }

   if (TRUE == nInDDC)
      {
      nInDDC = FALSE;
      if (1 == pVGADevTable->GdiDesktopEnabled)
         { 
              if (VMM_Install_IO_Handler((DWORD)myIOhandler, pVGADevTable->IoBase + 0x78))
            {
                      Debug_Printf( VNAME "port untrap failed for port %d\n", 0x78);
                 }
         else
                 VMM_Enable_Global_Trapping(pVGADevTable->IoBase + 0x78);
         }
      }
        __asm
        {
            pop     edi;
            pop     esi;
            mov     esp,ebp;
            pop     ebp;
            ret;
        }
}


/*----------------------------------------------------------------------
Function name:  PostVGAToHiRes

Description:    Called after changing to hires mode

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM which we ignore

Return:         VOID
----------------------------------------------------------------------*/
VOID 
PostVGAToHiRes( VOID )
{
    PDEVTABLE pDev = pVGADevTable;
#ifdef H3_A0
    //
    // put the clocks back to 100Mhz
    //
    if (NULL != pDev)
    {
        ISET32(pllCtrl1, 0x2805);
        ISET32(pllCtrl2, 0x2805);
        if (pDev->tvOutActive && TvOutFn)
            TvOutFn->tvout_Enable (pDev, -1);
    }
#endif
}


/*----------------------------------------------------------------------
Function name:  TurnVGAOff

Description:    Disable the VGA registers and memory aperture.

Information:    

Return:         BOOL    TRUE is always returned.
----------------------------------------------------------------------*/
BOOL TurnVGAOff( VOID )
{
   WORD wCmd;

   if (NULL != pVGADevTable)
      {
      // Disable Interrupts
      if ((pVGADevTable->InterruptsEnabled) && (pVGADevTable->hIRQ))
         {
         DisableInterrupts(pVGADevTable);
         VPICD_Force_Default_Behavior(pVGADevTable->hIRQ);
         pVGADevTable->hIRQ=0x0;
         }

      wCmd = 0x0;
   	CM_Call_Enumerator_Function( pVGADevTable->dwDevNode,
	   			     PCI_ENUM_FUNC_SET_DEVICE_INFO,
		   		     SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0 );
      }
    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  TurnVGAOn

Description:    Enable the VGA registers and memory aperture.

Information:    

Return:         BOOL    TRUE is always returned.
----------------------------------------------------------------------*/
BOOL TurnVGAOn( VOID )
{
   WORD wCmd;

   if (NULL != pVGADevTable)
      {
      wCmd = 0x03;
   	CM_Call_Enumerator_Function( pVGADevTable->dwDevNode,
	   			     PCI_ENUM_FUNC_SET_DEVICE_INFO,
		   		     SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0x0);

      //Reenable Interrupts
	   if (pVGADevTable->InterruptsEnabled &&
	      (pVGADevTable->ConfigData.bIRQRegisters[0] != 0))
         pVGADevTable->InterruptsEnabled = InitializeInterrupts(pVGADevTable->ConfigData.bIRQRegisters[0], pVGADevTable);

      }
    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  GetTotalVRAMSize

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
GetTotalVRAMSize( VOID )
{
    DWORD dwSize;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        push    esi;
        push    edi;
    }

    if (NULL != pVGADevTable)
      dwSize = pVGADevTable->MemSizeInMB * 1024L * 1024L;
    else
      dwSize = 0x0;

    __asm
    {
        mov     ecx,dwSize;
        pop     edi;
        pop     esi;
        stc;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

/****************************************************************************
 Message code etc.
 ***************************************************************************/


/*----------------------------------------------------------------------
Function name:  PreHiResToVGA

Description:    Called before changing to VGA mode.

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM going to full screen VGA mode

Return:         VOID
----------------------------------------------------------------------*/

VOID 
PreHiResToVGA( VOID )
{
    PDEVTABLE pDev = pVGADevTable;
    SstCRegs *lpCRegs;

    if (NULL == pDev)
        return;
    
    __asm mov FullScreenVM, ebx;

#ifdef H3_A0
    // XXX FIXME XXX
    // set the clocks to 50MHz
    // only for demoing full-screen dos boxes in windows -- temporary!
    //
    ISET32(pllCtrl1, 0x2806);
    ISET32(pllCtrl2, 0x2806);
#endif // #ifdef H3_A0
   
    DisableGdiDesktop(pDev);

    // wait for chip idle and turn off the command fifo
    while (IGET32(status) & SST_BUSY)
        ;

    lpCRegs = (SstCRegs *) ((FxU32)pDev->RegBase + SST_CMDAGP_OFFSET);
    
    SETDW(lpCRegs->PRIMARY_CMDFIFO.baseSize, 0);
}


/*----------------------------------------------------------------------
Function name:  PostHiResToVGA

Description:    

Information:    Function is empty.

Return:         VOID
----------------------------------------------------------------------*/
VOID 
PostHiResToVGA( VOID )
{
//      if (pVGADevTable->tvOutActive)
//          TvOutFn->tvout_Enable (pVGADevTable, 3);
}


/*----------------------------------------------------------------------
Function name:  PreVGAToHires

Description:    

Information:    
  Entry..
  ebp   -   Client Register pointer
  ebx   -   VM leaving full screen VGA mode

Return:         VOID
----------------------------------------------------------------------*/
VOID 
PreVGAToHires( VOID )
{
    FullScreenVM = hSysVM;
}


/*----------------------------------------------------------------------
Function name:  MiniVDD_Sys_VM_Terminate

Description:    

Information:    

Return:         DWORD   VXD_SUCCESS is always returned.
----------------------------------------------------------------------*/
DWORD __stdcall 
MiniVDD_Sys_VM_Terminate( VOID )
{
    if (NULL != pVGADevTable)
    {
        if( pVGADevTable->RegBase != NULL )      // just in case
        {
            PreHiResToVGA();
            // int 10 should do the rest
        }
    }
    return(VXD_SUCCESS);
}


/*----------------------------------------------------------------------
Function name:  MiniVDD_System_Exit

Description:    

Information:    Function is empty.

Return:         DWORD   VXD_SUCCESS is always returned.
----------------------------------------------------------------------*/
DWORD __stdcall
MiniVDD_System_Exit( VOID )
{
	PDEVTABLE pDev = pVGADevTable;

	/*
	Unfortunately, when we get here the driver has already done an INT10 BUT
	later disabled the desktop (turning off TVOUT).  This is the reverse of
	the order when opening a full screen DOS session.  We need to reenable
	TVOUT if it was active.
	*/

	if (pDev && TvOutFn && pDev->RegBase && pDev->IoBase)
	{
		outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
		if (inp ((FxU16)(pDev->IoBase + 0xd5)) & BIOS_TVOUT_ACTIVE)
		{
			((SstIORegs * )pDev->RegBase)->vidInFormat = \
											SST_VIDEOIN_TVOUT_ENABLE       |
											SST_VIDEOIN_G4_FOR_POSEDGE     |
											SST_VIDEOIN_VSYNC_POLARITY_LOW |
											SST_VIDEOIN_HSYNC_POLARITY_LOW;
		}
	}
    return(VXD_SUCCESS);
}


/*----------------------------------------------------------------------
Function name:  DisplayDriverDisabling

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID
DisplayDriverDisabling( VOID )
{
    MiniVDD_Sys_VM_Terminate();
}

/****************************************************************************
 Port Trapping.
 ***************************************************************************/


/*----------------------------------------------------------------------
Function name:  VDD_Get_VM_Info

Description:    

Information:    

Return:         HVM
----------------------------------------------------------------------*/
HVM VXDINLINE
VDD_Get_VM_Info()
{
    HVM hVM;

    VxDCall(VDD_Get_VM_Info);
    __asm mov hVM,edi;
    return(hVM);
}


/*----------------------------------------------------------------------
Function name:  MiniVDD_Virtual3C2H

Description:    Called for port access to 3C2.

Information:    
  Entry..
  eax   -   Value to write
  ebx   -   VM
  ecx   -   Type of I/O
  edx   -   Port

Return:         HVM
----------------------------------------------------------------------*/
_declspec ( naked ) VOID
Virtual_3c2h()
{
    WORD wPort;
    WORD wValue;
    HVM  hVM;
    DWORD dwFlags;
    static BOOL bTrig;
    static BYTE dbExt7;
    DWORD dumpAccess;

    _NP();
    __asm 
    {
        mov     wPort, dx;
        mov     wValue, ax;
        mov     dwFlags, ecx;
        mov     hVM, ebx;
    }

    dumpAccess = 0;

    if (pVGADevTable->GdiDesktopEnabled && (hVM == hSysVM))
    {
        dumpAccess = 1;
    }

    if (dumpAccess)
    {
        _NE();
    }
    else
    {
        _NE_NORET();
        __asm jmp       Old_3c2_Handler;
    }
}

#ifdef IDONTTHINKWENEEDTHIS_KMW


/*----------------------------------------------------------------------
Function name:  MiniVDD_Virtual3DEH

Description:    Called for port access to 3DE/3DF.

Information:    Presently #ifdef'd out.
  Entry..
  eax   -   Value to write
  ebx   -   VM
  ecx   -   Type of I/O
  edx   -   Port

Return:         DWORD
----------------------------------------------------------------------*/
_declspec ( naked ) DWORD MiniVDD_Virtual3DEH( VOID )
{
    WORD wPort;
    WORD wValue;
    HVM  hVM;
    DWORD dwFlags;
    DWORD dwReply;
    static BOOL bTrig;
    static BYTE dbExt7;

    // naked prolog code
    __asm
    {
        push    ebp;
        mov     ebp,esp;
        sub     esp,__LOCAL_SIZE;
        mov     wPort,dx;
        mov     wValue,ax;
        mov     dwFlags,ecx;
        mov     hVM,ebx;
    }

    // physical if controlling VM??
    if( hVM == hSysVM ||
        hVM == VDD_Get_VM_Info() )
    {
        switch( dwFlags )
        {
        case BYTE_INPUT:
            dwReply = inp(wPort);            
            break;

        case BYTE_OUTPUT:
            outp( wPort, (BYTE)wValue );
            break;

        case WORD_INPUT:
            dwReply = inpw( wPort );
            break;

        case WORD_OUTPUT:
            outpw( wPort, wValue );
            break;

        default:
            break;
        }
    }
    else switch( dwFlags )
    {
    case BYTE_INPUT:
        if( wPort == 0x3DF && bTrig )
            dwReply = dbExt7;
        break;

    case BYTE_OUTPUT:
        if( wPort == 0x3DE)
        {
            if( (wValue & 0xFF) == 7 )
                bTrig = TRUE;
            else
                bTrig = FALSE;
        }
        else
        if( wPort == 0x3DF && bTrig )
            dbExt7 = wValue;
        break;

    case WORD_INPUT:
        if( wPort == 0x3DE && bTrig )
            dwReply = (dbExt7 << 8) | 7;
        break;

    case WORD_OUTPUT:
        if( wPort == 0x3DE && (wValue & 0xFF) == 7 )
        {
            bTrig = TRUE;
            dbExt7 = wValue >> 8;
        }
        else
            bTrig = FALSE;
        break;

    default:
        break;
    }
    // naked epilog code
    __asm
    {
        mov     eax,dwReply;
        mov     esp,ebp;
        pop     ebp;
        ret;
    }
}

#endif // #ifdef IDONTTHINKWENEEDTHIS_KMW

DWORD _cdecl SetMonitorPowerState(DEVNODE devnode, DWORD PowerState);
/****************************************************************************
 DPMS support.
 ***************************************************************************/


/*----------------------------------------------------------------------
Function name:  DPMSSupport

Description:    

Information:    

Return:         INT     1 if success, 0 if failure.
----------------------------------------------------------------------*/
int DPMSSupport(PCRS pCRS, PDEVTABLE pDevTable)
{
   int nReturn = 0;
   CLIENT_STRUCT *pCR = (CLIENT_STRUCT *)pCRS;
 
   switch( pCR->CBRS.Client_BL & 0xFF )
      {
      case 0:     // report
         pCR->CWRS.Client_AX = 0x4F;
         pCR->CWRS.Client_BX = 0x0710;   // full support
         nReturn = 1;
         break;

      case 1:     // set state
         pCR->CWRS.Client_AX = 0x4F;
         switch(pCR->CBRS.Client_BH)
            {
            case 0:         // normal
               SetMonitorPowerState(pDevTable->dwDevNode, 0x01);
               pDevTable->bDPMSState = 0;
               nReturn = 1;
               break;

            case 1:         // standby
               SetMonitorPowerState(pDevTable->dwDevNode, 0x02);
               pDevTable->bDPMSState = 1;
               nReturn = 1;
               break;

            case 2:         // suspend
               SetMonitorPowerState(pDevTable->dwDevNode, 0x04);
               pDevTable->bDPMSState = 2;
               nReturn = 1;
               break;


            case 4:         // off
               SetMonitorPowerState(pDevTable->dwDevNode, 0x08);
               pDevTable->bDPMSState = 4;
               nReturn = 1;
               break;

            default:
               pCR->CWRS.Client_AX = 0x14F;    // failure
               break;
            }
        break;

      case 2:     // get state
         pCR->CWRS.Client_AX = 0x4F;
         pCR->CBRS.Client_BH = pDevTable->bDPMSState;
         nReturn = 1;
         break;

      default:   // bogus state ... give them a error
         pCR->CWRS.Client_AX = 0x14F;
         break;
   }

   return nReturn;
}

/*----------------------------------------------------------------------
Function name:  MiniVDD_W32_DevIoCtl

Description:    
This function handles the DEVIOCTL calls.

Information:    

Return:         INT     1 if success, 0 if failure.
----------------------------------------------------------------------*/
DWORD __stdcall MiniVDD_W32_DevIoCtl(DIOCPARAMETERS * pDIOCParms)
{
   PDEVTABLE pDev;
   DWORD dwReturn = 0;
   DWORD dwMMIO;
   DWORD dwOrigMMIO;
   DWORD dwFB;
   DWORD dwOrigFB;
   PDIOC_DATA pDIOC;
   WORD wCmd;
   WORD wOrigCmd;

   switch (pDIOCParms->dwIoControlCode)
      {
      case DIOC_OPEN:
      case DIOC_CLOSEHANDLE:
         dwReturn = 0;
         break;

      case VDD_IOCTL_GET_DDHAL:         
         dwReturn = GetKernelInfo(pDIOCParms);
         break;

      // Update the Interrupt Mask <Used in EnableInterrupt>
      case UPDATE_IMASK:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               pDev->dwIMask = pDIOC->dwSpare & H3_IMASK;
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;         


      // This is the magic script
      case AGP_WARMUP:
         if (NULL != (pDIOC = (PDIOC_DATA)pDIOCParms->lpvInBuffer))
            {
            pDev = FindPDEVFromDevNode(pDIOC->dwDevNode);
            if (NULL != pDev)
               {
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0 );
               wOrigCmd = wCmd;
               wCmd &= ~(0x03);
               // Disable the Device
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_COMMAND_ID, &wCmd, sizeof(WORD), 0 );

               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_FB_ID, &dwFB, sizeof(DWORD), 0 );
        
               // Save Original
               dwOrigMMIO = dwMMIO & 0xFF000000;
               dwOrigFB = dwFB & 0xFF000000;

               // Max Size
               dwMMIO = 0xFFFFFFFF;
               dwFB = 0xFFFFFFFF;
               // Find Size
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_FB_ID, &dwFB, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_FB_ID, &dwFB, sizeof(DWORD), 0 );

               // Back to Original        
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_MMIO_ID, &dwOrigMMIO, sizeof(DWORD), 0 );
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_FB_ID, &dwOrigFB, sizeof(DWORD), 0 );
      
               // Reenable IO             
               CM_Call_Enumerator_Function( pDev->dwDevNode, PCI_ENUM_FUNC_SET_DEVICE_INFO, SST_PCI_COMMAND_ID, &wOrigCmd, sizeof(WORD), 0 );
               }
            else
               {
               dwReturn = ERROR_INVALID_PARAMETER;
               }
            }
         else
            {
            dwReturn = ERROR_INVALID_PARAMETER;
            }
         break;
      case GET_DIB_ADDR:
         dwReturn = GetExtraAddr( pDIOCParms);
         break;

      default:
         // use WIN32 GetLastError to find out about this
         dwReturn = ERROR_NOT_SUPPORTED;
         break;
      }

   return dwReturn;
}

void MouseCtrl(DWORD dwSetting, DWORD dwData)
{
    PDEVTABLE pDev;
    SstIORegs *lpIOregs;
    short sTmp;
    DWORD dwTmp;

    pDev = FindPDEVFromDevNode(DevTable[0].DispInfo.diDevNodeHandle);

    if(pDev != NULL)
    {
        lpIOregs = (SstIORegs * )pDev->RegBase;

        switch(dwSetting)
        {
        // Set enabled
        //      dwData = 1 to enable
        //               0 to disable
        case 1:
            dwTmp = lpIOregs->vidProcCfg;

            dwTmp &= ~ ( SST_CURSOR_EN | SST_CURSOR_MODE);

            if(dwData)
                dwTmp |= SST_CURSOR_EN | SST_CURSOR_MICROSOFT;

            SETDW(lpIOregs->vidProcCfg, dwTmp) ;
            break;

        // Set pattern
        //      dwData = * to 64x64 argb
        case 2:
            {
                DWORD *pCursor;
                BYTE  *pHwCursor;
                DWORD  i, j, k, ci;
                BYTE   pat0, pat1;
                DWORD  c0, c1;

                pat0 = 0;
                pat1 = 0;
                SETDW(lpIOregs->hwCurPatAddr, 0x0);

                pCursor = (DWORD *)dwData;
                pHwCursor = (BYTE *)(pDev->LfbBase + 0x0);

                for(i=0,j=7,k=0,ci=0,c0=0x00010101,c1=0x00010101;
                    i<(64*64);
                    i++)
                {
                    if(pCursor[i] >> 24)
                    {
                        if(ci < 2)
                        {
                            if((pCursor[i] & 0x00ffffff) != c0)
                            {
                                if(ci == 0)
                                {
                                    SETDW(lpIOregs->hwCurC0, pCursor[i] & 0x00ffffff);
                                    c0 = pCursor[i] & 0x00ffffff;
                                }
                                else
                                {
                                    SETDW(lpIOregs->hwCurC1, pCursor[i] & 0x00ffffff);
                                    c1 = pCursor[i] & 0x00ffffff;
                                }
                                ci++;
                            }
                        }

                        if((pCursor[i] & 0x00ffffff) == c0)
                        {
                            pat0 |= 0 << j;
                            pat1 |= 0 << j;
                        }
                        else if((pCursor[i] & 0x00ffffff) == c1)
                        {
                            pat0 |= 0 << j;
                            pat1 |= 1 << j;
                        }
                        else
                        {
                            pat0 |= 1 << j;
                            pat1 |= 1 << j;
                        }
                    }
                    else
                    {
                        pat0 |= 1 << j;
                        pat1 |= 0 << j;
                    }

                    j--;

                    if(j > 7)
                    {
                        *(pHwCursor+8) = pat1;
                        *(pHwCursor++) = pat0;
                        j = 7;
                        pat0 = pat1 = 0;
                        k++;
                    }

                    if(k > 7)
                    {
                        pHwCursor += 8;
                        k = 0;
                    }
                }
            }
            break;

        // Set color0
        //      dwData = 0rgb
        case 3:
            SETDW(lpIOregs->hwCurC0, dwData);
            break;

        // Set color1
        //      dwData = 0rgb
        case 4:
            SETDW(lpIOregs->hwCurC1, dwData);
            break;

        // Set pos
        //      dwData = 15:0  x
        //               31:16 y
        case 5:
            sTmp = (WORD)(dwData >> 16);
            sTmp += 63;
            if(sTmp < 0)
              sTmp = 0;
            else if(sTmp > 2047)
              sTmp = 2047;
            dwTmp = (DWORD)sTmp << 16;

            sTmp = (WORD)(dwData & 0x0000ffff);
            sTmp += 63;
            if(sTmp < 0)
              sTmp = 0;
            else if(sTmp > 2047)
              sTmp = 2047;
            dwTmp |= (DWORD)sTmp;

            SETDW(lpIOregs->hwCurLoc, dwTmp);
            break;

        default:
            break;
        }
    }
}
