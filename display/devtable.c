/* -*-c++-*- */
/* $Header: devtable.c, 2, 12/6/00 1:12:28 PM PST, Brent Burton$ */
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
** File name:   devtable.c
**
** Description: Initialize the DevNode table and supporting functions.
**
** $Revision: 2$
** $Date: 12/6/00 1:12:28 PM PST$
**
** $History: devtable.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:53p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 3/18/99    Time: 3:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to record lfbMemoryConfig at boot
** 
** *****************  Version 32  *****************
** User: Andrew       Date: 3/10/99    Time: 4:30p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed so that Banshee VDD will not work on Avenger and visa versa
** 
** *****************  Version 31  *****************
** User: Xingc        Date: 3/06/99    Time: 1:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** GetBiosVersion() only return version number without chip or memory info
** 
** *****************  Version 30  *****************
** User: Xingc        Date: 3/05/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS 4855, "QUERYGETBIOSVERSION not working correctly"
** 
** *****************  Version 29  *****************
** User: Stuartb      Date: 2/25/99    Time: 10:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Capture SubSystemID in InitDevNode and save in Devtable.
** 
** *****************  Version 28  *****************
** User: Stb_pzheng   Date: 2/24/99    Time: 6:04p
** Updated in $/devel/h3/win95/dx/minivdd
** Removed the code that sets tmuGbInit register so that bios can take
** care of it on its own
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 1/29/99    Time: 10:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to save bios information
** 
** *****************  Version 26  *****************
** User: Jw           Date: 1/22/99    Time: 4:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Change pciSetMTRRAmdK6 to pciSetAmdK6MTRR to resolve name conflict in
** some other driver header file.
** 
** *****************  Version 25  *****************
** User: Jw           Date: 1/22/99    Time: 3:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add AMD K6/K7 MTRR support.
** 
** *****************  Version 24  *****************
** User: Michael      Date: 1/07/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 23  *****************
** User: Andrew       Date: 1/05/99    Time: 10:38a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Function GetBIOSVersion to read the version string out of the
** bios.
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 12/09/98   Time: 5:26p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H4_OEM device ID check
** 
** *****************  Version 21  *****************
** User: Andrew       Date: 11/24/98   Time: 8:33a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to save the VendorDeviceID in the device table
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 11/16/98   Time: 8:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to support Avenger
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 10/26/98   Time: 9:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added the Win '98 calls to the MTRR routine to bring inline with GLOP
** 
** *****************  Version 18  *****************
** User: Ken          Date: 10/16/98   Time: 11:14a
** Updated in $/devel/h3/win95/dx/minivdd
** mirroring GLOP changes into TOT for clock change policy (don't touch
** PLLs at boot by default, they still can be tweaked from the registry
** though), and for the new "Hank" memory timing rules (dramInit1 bit 14
** is speed dependent), and memClock and grxClock registry keys are now
** string keys
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 9/23/98    Time: 10:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Inited bPowerState and bMonitorState to D0
** 
** *****************  Version 16  *****************
** User: Andrew       Date: 8/22/98    Time: 12:06p
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed set of VGA Legacy Bit for 2nd monitor as it was breaking DDC
** and reversed order of clear of MTRR
** 
** *****************  Version 15  *****************
** User: Ken          Date: 7/17/98    Time: 3:37p
** Updated in $/devel/h3/win95/dx/minivdd
** separated default clocks into independent default values settable from
** environment variables (GCLK for graphics clock), (MCLK for memory
** clock)
** 
** *****************  Version 2  *****************
** User: Ken          Date: 7/17/98    Time: 3:24p
** Updated in $/Releases/Banshee/A2_Merc/3dfx/devel/h3/Win95/dx/minivdd
** added separate graphics and memory clock defaults, settable in build
** from environment variables GCLK (graphics clock) and MCLK (memory
** clock)
** 
** *****************  Version 1  *****************
** User: Admin        Date: 7/16/98    Time: 2:49p
** Created in $/Releases/Banshee/A2_Merc/3dfx/devel/h3/Win95/dx/minivdd
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 7/14/98    Time: 1:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Modified to check if IRQ has changed before supressing init
** 
** *****************  Version 13  *****************
** User: Andrew       Date: 7/14/98    Time: 9:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed InitDevTable to check if DevNode has already been initialized
** to same physical address before blinding performing the init
** 
** *****************  Version 12  *****************
** User: Ken          Date: 7/10/98    Time: 9:00p
** Updated in $/devel/h3/win95/dx/minivdd
** memory / clock timing default changes.  default grxclock and memclock
** timings are now settable at compile time.   temporarily, set HR=B0 to
** compile for A2/A3 for minivdd only.  dramInit0 is now not touched in
** the win95 init sequence, unless there's an override in the registry
** 
** *****************  Version 11  *****************
** User: Ken          Date: 7/07/98    Time: 2:16p
** Updated in $/devel/h3/win95/dx/minivdd
** now look up to MTRR 20e/20f, and mark 16MB as write combine instead of
** just 4mb
** 
** *****************  Version 10  *****************
** User: Ken          Date: 7/02/98    Time: 6:23p
** Updated in $/devel/h3/win95/dx/minivdd
** added magic parameter values to h3InitSGRAM so that windows boot can
** skip the writing of the SGRAM registers
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 7/01/98    Time: 10:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed InitDevNode to update the bios variable in IoBase+0xD4 to a
** possible new IObase.  This is a problem since the bios gets at boot but
** may change at a latter time.
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 6/23/98    Time: 7:11a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added new field to track power mgmt. Added new field to save scratchpad
** registers
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 6/07/98    Time: 8:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Init new byte field for DPMS
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 5/27/98    Time: 8:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem with count of devices
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 4/28/98    Time: 2:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Took out a check for already initialized devnode since windows was
** moving the physical address which was causing me some problems.
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/09/98    Time: 10:20p
** Updated in $/devel/h3/win95/dx/minivdd
** updated to phil's new init code
**
*/

#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#undef  VDDONLY

#include "p6stuff.h"
#include "h3cinit.h"

typedef unsigned char BOOLEAN;              // Added for Win98DDK usage.

#undef WANTVXDWRAPS
#include <mtrr.h>

#pragma VxD_LOCKED_DATA_SEG
DEVTABLE DevTable[MAX_BANSHEE_DEVICES];
PDEVTABLE pVGADevTable;
DWORD dwNumDevices;

FxU32 isP6;


#pragma VxD_ICODE_SEG

void InitSecondaryDispatchTable(void);
void InitPower(void);
 
#pragma VxD_LOCKED_CODE_SEG


/*----------------------------------------------------------------------
Function name:  PNP_NewDevNode

Description:    Start of PNP_NewDevNode

Information:    

Return:         CONFIGRET   CR_SUCCESS is always returned.
----------------------------------------------------------------------*/
CONFIGRET _cdecl
PNP_NewDevNode(DWORD dwCommand, DEVNODE dwDevNode, DWORD dwLoadtype)
{
   PDEVTABLE pDev;
   Debug_Printf("Command %x DevNode %x loadtype %x\n", dwCommand, dwDevNode, dwLoadtype);
   
   pDev = InitDevNode(dwDevNode);

   Debug_Printf("Init Secondary Dispatch Table\n");

   InitSecondaryDispatchTable();

   return (CR_SUCCESS);   
}

#define MTRR_MASK_LOW_16MB		  0xFF000800

void our_outp(DWORD addr, DWORD dwData);

#define FB_SIZE 0x1000000


/*----------------------------------------------------------------------
Function name:  VDD_Get_Mini_Dispatch_Table

Description:    Get the Dispatch Table from VDD.

Information:    Uses in-line asm.

Return:         DWORD   value returned from the VxDCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE
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
Function name:  InitDevNode

Description:    Initialize the dev node.

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE 
InitDevNode(DWORD dwDevNode)
{
    PDEVTABLE pDev = NULL;
    SstIORegs *lpIOregs;
    DWORD dwDevID;
    DWORD memBase;
    DWORD	IoBase;			// base of PCI relocatable IO space
    DWORD	PhysMemBase[2];		// physical register address space
    FxU32 grxSpeedInMHz, sgramMode, sgramMask, sgramColor;
    FxU32 memSpeedInMHz;
    FxU32 mtrr1Lo = 0, mtrr1Hi = 0, mtrr2Lo = 0, mtrr2Hi = 0;
    FxU32 foundMTRR;
    PDWORD pDispatch;
    DWORD dwNFunc;
    int i;
    int j;
    int nFound;

    nFound = FALSE;
    for (i=0; i< MAX_BANSHEE_DEVICES; i++)
    {
	if (dwDevNode == DevTable[i].dwDevNode)
       {
       nFound = TRUE;
	    break;
       }  
	if (0x0 == DevTable[i].dwDevNode)
	{
	    dwNumDevices++;
	    break; 
	}
    }

    if (MAX_BANSHEE_DEVICES != i)
    {
	pDev = &DevTable[i];
	DevTable[i].dwDevNode = dwDevNode;
	DevTable[i].lpDriverData = NULL;
	DevTable[i].bDPMSState = 0x0;
	DevTable[i].bPowerState = CM_POWERSTATE_D0;
	DevTable[i].bMonitorState = CM_POWERSTATE_D0;
	// get display data from VDD
	//
	VDD_Get_DISPLAYINFO( &DevTable[i].DispInfo,
			     sizeof(DevTable[i].DispInfo) );

	Debug_Printf("Are they the same DevNode %x DevNode %x %x\n",
		     dwDevNode, DevTable[i].DispInfo.diDevNodeHandle,
		     DevTable[i].DispInfo.diInfoFlags);

	// now we have the devnode, ask for the resources from
	// Config manager
	//
	CM_Get_Alloc_Log_Conf( &DevTable[i].ConfigData, dwDevNode, 0 );

	// and ask the PCI manager for the device ID
	//
	CM_Call_Enumerator_Function( dwDevNode,
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     0, &dwDevID, sizeof(DWORD), 0 );

#ifdef H3
	if (dwDevID != SST_VENDOR_DEVICE_ID_H3)
#else
	if ((dwDevID != SST_VENDOR_DEVICE_ID_H4) &&
	    (dwDevID != SST_VENDOR_DEVICE_ID_H4_OEM))
#endif
	{
	    Debug_Printf( VNAME "Invalid Device ID 0x%x, expecting 0x%x"
			        " or 0x%x or 0x%x\n",
			  dwDevID, SST_VENDOR_DEVICE_ID_H3,
			  SST_VENDOR_DEVICE_ID_H4,
			  SST_VENDOR_DEVICE_ID_H4_OEM);
	    DevTable[i].dwDevNode = 0x0;
	    return(NULL);
	}   

	//
	// find the PCI relocatable memory address space bases
	//
	j = 0;
	memBase = 0;
	while (j < DevTable[i].ConfigData.wNumMemWindows)
	{
	    if (DevTable[i].ConfigData.dMemBase[j] < (1024 * 1024))
            {
		j += 1;
		continue;
            }

      
	    PhysMemBase[memBase++] =
		DevTable[i].ConfigData.dMemBase[j++];

	    if (memBase == H3_NUM_MEM_BASES)
		break;		// found all the memory bases!
	}

	if (memBase != H3_NUM_MEM_BASES)
	{
	    Debug_Printf( VNAME
			  "Too few membases > 1MB, found %d, expecting %d\n",
			  memBase, H3_NUM_MEM_BASES);
	    DevTable[i].dwDevNode = 0x0;
	    return(NULL);
	}
 
	//
	// find the PCI relocatable IO base
	// 
	j = 0;
   IoBase = 0;
	while (j < DevTable[i].ConfigData.wNumIOPorts)
	{
	    if (DevTable[i].ConfigData.wIOPortBase[j] < 0x400)
            {
		j += 1;
		continue;
            }

	    IoBase = DevTable[i].ConfigData.wIOPortBase[j];
	    break;
	}

	if (IoBase == 0)
	{
	    Debug_Printf( VNAME "Found no relocatable IO base\n");
	    DevTable[i].dwDevNode = 0x0;
	    return(NULL);
	}

   // Ok, we already mapped in the resources for this node once
   // has it changed?
   if (TRUE == nFound)
      {
      if ((DevTable[i].PhysMemBase[0] == PhysMemBase[0]) &&
          (DevTable[i].PhysMemBase[1] == PhysMemBase[1]) &&
          (DevTable[i].IoBase == IoBase))
         {
         // If interrupts are disabled then return
         if (0 == DevTable[i].InterruptsEnabled)
            return pDev;

         // Fix me to we need to remove old interrupt?
         // If interrupt is the same then return
         if (DevTable[i].IRQDesc.VID_IRQ_Number == DevTable[i].ConfigData.bIRQRegisters[0])
            return pDev;
         }
      }

   for (memBase=0; memBase<H3_NUM_MEM_BASES; memBase++)
      DevTable[i].PhysMemBase[memBase] =  PhysMemBase[memBase];
   DevTable[i].IoBase = IoBase;      

	// Here is where we need to do a little bit of fixing up
	// The problem is that the Banshee BIOS keeps track of the
	// IOBase in IOBase+D4[1C].  Windows in its infinite wisdom
	// may change the value from what the system BIOS assigned at
	// boot.  To fix this problem we need to update the value here.
	our_outp(DevTable[i].IoBase + 0xD4, 0x1C);
	our_outp(DevTable[i].IoBase + 0xD5, DevTable[i].IoBase >> 8);

	DevTable[i].RegBase = (DWORD)
	    _MapPhysToLinear(DevTable[i].PhysMemBase[0], 32*1024*1024, 0);
	if (DevTable[i].RegBase == NULL)
	{
	    Debug_Printf( VNAME "Unable to map memory base 0\n");
	    DevTable[i].dwDevNode = 0x0;
	    return(NULL);
	}

	DevTable[i].LfbBase = (DWORD)
	    _MapPhysToLinear(DevTable[i].PhysMemBase[1], 32*1024*1024, 0);
	if (DevTable[i].LfbBase == NULL)
	{
	    Debug_Printf( VNAME "Unable to map memory base 1\n");
	    DevTable[i].dwDevNode = 0x0;
	    return(NULL);
	}

#ifdef TRITON_FIX
#pragma message( "Triton ") 
	// fix the triton bug (which one? KMW)
	//
	if (CM_Locate_DevNode( &TritonDN, TRITON_PCI_ID, 0 ) == CR_SUCCESS)
	{
	    BYTE dbOption;

	    CM_Call_Enumerator_Function( dwDevNode,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO,
					 0x43, &dbOption, 
					 sizeof(BYTE), 0 );
	    dbOption |= 0x20;
	    CM_Call_Enumerator_Function( dwDevNode,
					 PCI_ENUM_FUNC_SET_DEVICE_INFO,
					 0x43, &dbOption, 
					 sizeof(BYTE), 0 );
	}
#endif /* #ifdef TRITON_FIX */

	if (DevTable[i].InterruptsEnabled &&
	    (DevTable[i].ConfigData.bIRQRegisters[0] != 0))
	{
	    Debug_Printf(VNAME "Using IRQ %d\n",
			 DevTable[i].ConfigData.bIRQRegisters[0]);
	    DevTable[i].InterruptsEnabled =
		InitializeInterrupts(DevTable[i].ConfigData.bIRQRegisters[0],
				     &DevTable[i]);
	}
	else
	{
	    Debug_Printf(VNAME "No IRQ assigned\n");
	    DevTable[i].InterruptsEnabled = 0;
	}
 
	//
	// initialize h3 hardware
	//

	grxSpeedInMHz = H3_GRXCLOCK_IN_MHZ;

	// quick sanity check on the clock frequency
	//
	if ((grxSpeedInMHz < 50) || (grxSpeedInMHz > 150))
	    grxSpeedInMHz = 100;

	memSpeedInMHz = H3_MEMCLOCK_IN_MHZ;

	// quick sanity check on the clock frequency
	//
	if ((memSpeedInMHz < 50) || (memSpeedInMHz > 150))
	    memSpeedInMHz = 100;

	sgramMode = 0x37;		// get default from regsitry ? XXX
	sgramMask = 0xFFFFFFFF;
	sgramColor = 0;

	//
	// Policy change: now the windows drivers don't touch the mem/grx
	// PLLs at boot, whatever values are there, set by the BIOS at
	// POST, remain there.  The registry overrides for both
	// mem and grx are still enabled for those who wish to over/under
	// clock.    Since this may change again, I'm keeping all the above
	// clock code but commenting out the h3InitPlls call below
	//
	// h3InitPlls(DevTable[i].IoBase, grxSpeedInMHz, memSpeedInMHz);
	//

	// h3InitSgram won't mess with the sgram registers at all when we
	// sending in these magic numbers for sgramMode and sgramMask, this
	// works around a bug in A* silicon where writing the SGRAM registers
	// too far past chip reset causes the chip to go nuts, so we just have
	// to live with BIOS values for these registers in this case.
	//
	// changed again: now we never change SGRAM mode, even for B0,
	// so we always pass in the magic values to prevent them from
	// changing.
	//
	DevTable[i].MemSizeInMB = h3InitSgram(DevTable[i].IoBase,
					      0xDEADBEEF, 0xF00DCAFE,
					      sgramColor,
					      NULL);  // default SGRAM vendor

	// This may need to be fixed
	if (!(DevTable[i].DispInfo.diInfoFlags & DEVICE_IS_NOT_VGA))
	{
	    pVGADevTable = &DevTable[i];
	    h3InitVga(DevTable[i].IoBase, 1);	 // enable VGA decode
	    DevTable[i].bIsVGA = TRUE;
	}
	else
	{
	    DevTable[i].bIsVGA = FALSE;
	    lpIOregs = (SstIORegs * )DevTable[i].RegBase;
#if 0
	    lpIOregs->vgaInit0 |= SST_VGA_LEGACY_DECODE;

	    // Read Command Register
	    CM_Call_Enumerator_Function( dwDevNode,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO,
					 0x4, &wCmd, 
					 sizeof(WORD), 0 );
            
	    // Turn on IO
	    wCmd |= 0x01;
	    CM_Call_Enumerator_Function( dwDevNode,
					 PCI_ENUM_FUNC_SET_DEVICE_INFO,
					 0x4, &wCmd, 
					 sizeof(WORD), 0 );
#endif
	}

	lpIOregs = (SstIORegs * )DevTable[i].RegBase;
	DevTable[i].lfbMemoryConfig  = lpIOregs->lfbMemoryConfig;

#if 0
//PingZ @STB 2/24/99 Removed to let bios worry about this register
#ifdef H3_A0
	// turn off texture cache - hw workaround for A0
	ISET32(tmuGbeInit, 0x4555);
#else  // #ifdef H3_A0
	// the cam works in A1
	ISET32(tmuGbeInit, 0x555);
#endif // #ifdef H3_A0
#endif

	CM_Call_Enumerator_Function (dwDevNode,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_DEVICE_ID,
					 &DevTable[i].dwVendorDeviceID, sizeof(DWORD), 0); 
	CM_Call_Enumerator_Function (dwDevNode,
					 PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_SSID,
					 &DevTable[i].dwSubSystemID, sizeof(DWORD), 0); 

	//
	// are we on a P6 or a PII, or another processor that supports MTRR's?
	//
	CheckForP6();


	//
	// are we on a P6 or a PII, or another processor that supports MTRR's?
	//
	CheckForP6();

	if (!isP6)
	    return(pDev);		// no, just return now

	if ( isP6 == P6_AMDK6_MTRRS )
	{
		//
		// AMD MTRR code for K6
		//
		//   If the os supports the MTRR calls for this CPU use them.
		//

		if (MTRRGetVersion()) 
		{
			if (0x0 != pDev->Mtrr)
			{
				// Uncache the old physical address
				MTRRSetPhysicalCacheTypeRange((PVOID)pDev->Mtrr, FB_SIZE, MmNonCached);
			}

			MTRRSetPhysicalCacheTypeRange((PVOID)(DevTable[i].PhysMemBase[1] & 0xFF000000), FB_SIZE, MmFrameBufferCached);
			pDev->Mtrr = DevTable[i].PhysMemBase[1] & 0xFF000000;
		}
		else 
		{
			// No OS MTRR support

			pciSetAmdK6MTRR( 0, (PVOID)(DevTable[i].PhysMemBase[1] & 0xFF000000), FB_SIZE, PciMemTypeWriteCombining );
		}
	}
	else if ( !(isP6 & (P6_NONINTEL_WITH_INTEL_MTRRS | P6_INTELCPU_WITH_MTRRS)) )	// Do we support Intel style MTRR's
	{
	    return(pDev);		    // no, just return now
	}
	else
	{

	   // Windows 95 or doesn't support setting MTRR's for this processor?
	   dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispatch);
	   if ( (dwNFunc < NBR_MINI_VDD_FUNCTIONS_41) | (MTRRGetVersion()==FALSE) )
	      {
		   //
		   // Yes we are, so mark the raw lfb space USWC (write combine).
		   // First, find a free mtrr set.   By convention, we should only use
		   // one of the the first 5 pairs
		   //

		   foundMTRR = 0;
		   if (0x0 != pDev->Mtrr)
		      {
		      // We have been here before so we can reuse the same Mtrr
		      // First set it to zero so that we can find it again
		      SetMTRR(pDev->Mtrr + 1, 0, 0);
		      SetMTRR(pDev->Mtrr, 0, 0);
		      }

		   for (pDev->Mtrr = 0x200; pDev->Mtrr <= 0x20e; pDev->Mtrr += 2)
		      {
		      GetMTRR(pDev->Mtrr, &mtrr1Lo, &mtrr1Hi);
		      GetMTRR(pDev->Mtrr + 1, &mtrr2Lo, &mtrr2Hi);

		      if ((mtrr1Lo == 0) && (mtrr1Hi == 0) &&
			      (mtrr2Lo == 0) && (mtrr2Hi == 0))
	            {
			      foundMTRR = 1;
			      break;
	            }

		    if ((mtrr1Lo == ((DevTable[i].PhysMemBase[1] & 0xFF000000) | 1)) &&
			     (mtrr1Hi == 0) &&
			     (mtrr2Lo == MTRR_MASK_LOW_16MB) &&
			     (mtrr2Hi == 0xf))
	            {
			      foundMTRR = 2;
			      break;
	            }
		   }

		   if (foundMTRR == 0)
		      {
	          pDev->Mtrr = 0;
		      Debug_Printf(VNAME "ACK!  No free MTRRs!\n");
		      return(pDev);
		      }

		   if (foundMTRR == 2)
	         {
		      return(pDev);		// our MTRR is already there
	         }

		   // first, clear both MTRRs
		   //
		   SetMTRR(pDev->Mtrr, 0, 0);
		   SetMTRR(pDev->Mtrr + 1, 0, 0);

		   mtrr1Lo = (DevTable[i].PhysMemBase[1] & 0xFF000000) | 1;
		   mtrr1Hi = 0;
		   mtrr2Lo = MTRR_MASK_LOW_16MB;
		   mtrr2Hi = 0xf;

		   // then, set them to the required values
		   //
		   SetMTRR(pDev->Mtrr, mtrr1Lo, mtrr1Hi);
		   SetMTRR(pDev->Mtrr + 1, mtrr2Lo, mtrr2Hi);
	      }
	   // Windows '98
		else
	      { 
	      if (0x0 != pDev->Mtrr)
	      	{
		      // Uncache the old physical address
	         MTRRSetPhysicalCacheTypeRange((PVOID)pDev->Mtrr, FB_SIZE, MmNonCached);
	      	}
      
	      MTRRSetPhysicalCacheTypeRange((PVOID)(DevTable[i].PhysMemBase[1] & 0xFF000000), FB_SIZE, MmFrameBufferCached);
	      pDev->Mtrr = DevTable[i].PhysMemBase[1] & 0xFF000000;
	      }
	  }
   }
   else
	   Debug_Printf("Max Device exceeded\n");

    return pDev;
}


/*----------------------------------------------------------------------
Function name:  FindPDEVFromDevNode

Description:    Find the pDev from the DevNode table.

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE
FindPDEVFromDevNode(DWORD dwDevNode)
{
   PDEVTABLE pReturn = NULL;
   int i;
   
   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if (dwDevNode == DevTable[i].dwDevNode)
         {
         pReturn = &DevTable[i];
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }

   return pReturn;      
}


/*----------------------------------------------------------------------
Function name:  InitSecondaryDispatchTable

Description:    Initialize a secondary displatch table.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
InitSecondaryDispatchTable(void)
{
   PDWORD pDispatch;
   DWORD dwNFunc;

   dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispatch);
   pDispatch[REGISTER_DISPLAY_DRIVER]= (DWORD)RegisterDisplayDriver;
   InitPower();
}


/*----------------------------------------------------------------------
Function name:  GetBIOSVersion

Description:    This function is used to get the BIOS Version and
                should only be called at Enable Time.

Information:    At Enable time in Win '98 the current bios is mapped
                in at C000:0000.

Return:         VOID
----------------------------------------------------------------------*/
#define ROM_CONFIG (0x50)
typedef struct oemtable {
   DWORD regPCIInit0; // IOBase[04h]
   DWORD regMiscInit0; // IOBase[10h]
   DWORD regMiscInit1; // IOBase[14h]
   DWORD regDRAMInit0; // IOBase[18h]
   DWORD regDRAMInit1; // IOBase[1Ch]
   DWORD regAGPInit0; // IOBase[20h]
   DWORD regPLLCtrl1; // IOBase[44h]
   DWORD regPLLCtrl2; // IOBase[48h]
   DWORD regSGRAMMode; // IOBase[30h][10Dh]
   } OEMTABLE, * POEMTABLE;

void GetBIOSVersion(PDEVTABLE pDevTable, FxU32 vm)
{
   DWORD * pStrLoc;
   BYTE * pByteLoc;
   DWORD j;
   DWORD k;
   DWORD dwBIOSSize;
   PVMMCB hVM = (PVMMCB)vm;
   PWORD pFixSpot;
   PWORD pROMConfig;
   POEMTABLE pOEMConfig;
   

   pDevTable->bBIOSVersion[0]='\0';
   pDevTable->bBIOSVersion[MAX_BIOS_VERSION_STRING-1]='\0';
   // Get Address of Currently mapped in ROM @ C000:0000
	pStrLoc = (DWORD *)(0xC0000 + hVM->CB_High_Linear);
   pByteLoc = (BYTE *)pStrLoc;

   if (NULL == pByteLoc)
      return;

   // Get the size byte in the ROM header.  This is right after
   // 0x55 0xAA
   dwBIOSSize = pByteLoc[2]<<7;   

   for (j=0; j<dwBIOSSize; j++)
   {
      if ((BIOS_STRING_1 == pStrLoc[0]) && (BIOS_STRING_2 == pStrLoc[1]))
      {
            j = 0;

            pByteLoc = (BYTE *)&pStrLoc[2];
            for (k=0; k<MAX_BIOS_VERSION_STRING - 1; k++)
            {
                if(j)
                {
                    //stop if it will be next word
                    //or it is not a digit nor a char.
                    if((pByteLoc[k]==' ') || !pByteLoc[k] ||
                        ( ((pByteLoc[k] <'0') || (pByteLoc[k] >'9'))
                          &&((pByteLoc[k] <'A') || (pByteLoc[k] >'Z'))
                          &&((pByteLoc[k] <'a') || (pByteLoc[k] >'z'))
                          &&(pByteLoc[k]!='.')) )
                       
                    {
                       pDevTable->bBIOSVersion[j] ='\0';
                       break;
                    }
                    else
                     pDevTable->bBIOSVersion[j++] = pByteLoc[k];

                }
                else
                {
                   //only start with digit
                   if((pByteLoc[k] <= '9') &&
                    (pByteLoc[k] >= '0' ))
                  {
                      //must start at a word boundary
                      if( pByteLoc[k-1] == ' ') 
                       pDevTable->bBIOSVersion[j++] = pByteLoc[k];
                  }
                }
            }
            break;
     }
 
     pStrLoc = (DWORD *) ( pByteLoc + j);   //check it byte by byte
   }
   // While we are at it let's get the SGRAMMODE which is in the BIOS OEMTable
   // We may need it if we go into low power mode.
   pFixSpot = (PWORD)(0xC0000 + hVM->CB_High_Linear + ROM_CONFIG);
   pROMConfig = (PWORD)(0xC0000 + hVM->CB_High_Linear + *pFixSpot);
   pOEMConfig = (POEMTABLE)(0xC0000 + hVM->CB_High_Linear + *pROMConfig);      
   pDevTable->dwPowerReg[SGRAMMODE_INDEX] = pOEMConfig->regSGRAMMode;
}

//
//  Note the code below here is called from thunk32.c so
//  it is not VDD code so watch the stack
//
#define DEBUG_FIX                       \
{                                       \
    __asm   movzx   ebp, bp             \
    __asm   mov     eax, [ebp]          \
    __asm   movzx   eax, ax             \
    __asm   mov     [ebp], eax          \
}


/*----------------------------------------------------------------------
Function name:  FindLPFromDevNode

Description:    Find the lpDriverData based on the DevNode.

Information:    

Return:         DWORD   lpDriverData if success or,
                        0 for failure.
----------------------------------------------------------------------*/
DWORD
FindLPFromDevNode(DWORD dwDevNode)
{
   DWORD dwReturn;
   int i;
   
   DEBUG_FIX;
   
   dwReturn = 0x0;
   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if (dwDevNode == DevTable[i].dwDevNode)
         {
         dwReturn = DevTable[i].lpDriverData;
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }

   return dwReturn;      
}


/*----------------------------------------------------------------------
Function name:  SetLPFromDevNode

Description:    Set lpDriverData into the Dev table.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void 
SetLPFromDevNode(DWORD dwDevNode, DWORD lpDriverData)
{
   int i;
   
   DEBUG_FIX;

   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if (dwDevNode == DevTable[i].dwDevNode)
         {
         DevTable[i].lpDriverData = lpDriverData;
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }

}


/*----------------------------------------------------------------------
Function name:  FindActiveBanshee

Description:    Lookup the active DevNode from the PCI command
                register bits.
Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE FindActiveBanshee(void)
{
   PDEVTABLE pReturn = NULL;
   WORD wCmd;
   int i;

   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // No more Banshee's ?
      if (0x0 == DevTable[i].dwDevNode)
         break;

      CM_Call_Enumerator_Function( DevTable[i].dwDevNode,
                                   PCI_ENUM_FUNC_GET_DEVICE_INFO,
                                   0x4, &wCmd, 
                                   sizeof(WORD), 0 );

      // This is the one!!!
      if (0x3 == (wCmd & 0x03))
         { 
         pReturn = &DevTable[i];
         break;
         }
      }

   return pReturn;
}


// AMD K6 has only two MTRRs

#define MTRR_LAST_AMDK6    1  

/*
** The MTRRs of the AMD K6 are not compatible to the MTRRs on Intel
** processors. There are two MTRRs which are in a single MSR.
*/

#define MTRR_MSR_AMDK6     0xC0000085

// Stuff for programming K6 MSRs
typedef struct MSRInfo_s {
  unsigned long
    msrNum,                     /* Which MSR? */
    msrLo, msrHi;               /* MSR Values  */
} MSRInfo;


/*----------------------------------------------------------------------
Function name:  genMTRRvalAmdK6

Description:    Computes bit pattern to program into a K6-style MTRR

Information:    Computes bit pattern to program into a K6-style MTRR
                based on the desired memory range base address, size,
                and type.
                
Return:         Returns FXFALSE if memory type unsupported, block size
                out of range, or block size not power of 2, or base
                address not multiple of block size.  Otherwise,
                returns FXTRUE.
----------------------------------------------------------------------*/

static FxBool
genMTRRvalAmdK6(void *physBaseAddress, FxU32 physSize, PciMemType type, FxU32 *MTRRval)
{
   FxBool retVal = FXFALSE;
   FxU32 memTypeBits = 0;
   FxU32 physAddrMask;

   /* Validate memory type */

   if (type == PciMemTypeUncacheable) {
      memTypeBits = 0x1;
   }
   else if (type == PciMemTypeWriteCombining) {
      memTypeBits = 0x2;
   }
   else {
      return retVal;
   }

   /* Validate memory range size */

   if (physSize < 128*1024) {         /* make sure size >= 128 K */ 
      return retVal;
   }

   if (physSize & (physSize - 1)) {   /* make sure size is power of two */
      return retVal;
   }

   /* Validate base address */

   if ((FxU32)physBaseAddress % physSize) {  /* make sure base is multiple of size */
      return retVal;
   }

   /* Convert the range size into a mask */

   physAddrMask = 0x7FFF;
   physSize >>=18;            /* 128K --> 0, 256K --> 1, etc */
   while (physSize) {
      physAddrMask <<= 1;
      physSize     >>= 1;
   }

   /* Now mask: 128K => 7FFF, 256K => 7FFE, 512K => 7FFC, ... , 4G => 0000 */

   *MTRRval = ((((FxU32) physBaseAddress >> 17) & 0x7FFF) << 17) |
              (physAddrMask & 0x7FFF) << 2        |
              memTypeBits;

   retVal = FXTRUE;

   return retVal;
}


/*----------------------------------------------------------------------
Function name:  pciSetAmdK6MTRR

Description:    Writes a K6-style MTRR

Information:    Set up the specified K6-style MTRR based on physical
                address, physical size, and type.

Return:         Returns FXTRUE if the MTRR is set, otherwise returns
                FXFALSE.
----------------------------------------------------------------------*/


/*
**  pciSetAmdK6MTRR - 
**
**  NOTE:  A zero for the physical size results in the MTRR being
**  cleared. 
**
*/
FxBool
pciSetAmdK6MTRR(FxU32 mtrrNum, void *physBaseAddr, FxU32 physSize,
                PciMemType type)
{
  FxBool
    res, 
    rVal = FXFALSE;

  FxU32
    MTRRval;

  MSRInfo
    inS;

  if (mtrrNum > MTRR_LAST_AMDK6)
    return rVal;

  inS.msrNum = MTRR_MSR_AMDK6;   /* One MSR for both MTRRs */

  if (physSize == 0) {  /* size of 0 implies clear MTRR */

     GetMTRR( MTRR_MSR_AMDK6, &inS.msrLo, &inS.msrHi);

     if (mtrrNum) {
        inS.msrHi = 0x0;   /* clear MTRR 1 */
     }
     else {
        inS.msrLo = 0x0;   /* clear MTRR 0 */
     }

     SetMTRR(MTRR_MSR_AMDK6, inS.msrLo, inS.msrHi);
  }
  else {

     // Generate masks and set the MTRR

     res = genMTRRvalAmdK6(physBaseAddr, physSize, type, &MTRRval);

     if (res == FXFALSE)
       return rVal;

     GetMTRR( MTRR_MSR_AMDK6, &inS.msrLo, &inS.msrHi);

     if (mtrrNum) {
        inS.msrHi = MTRRval;   /* program MTRR 1 */
     }
     else {
        inS.msrLo = MTRRval;   /* program MTRR 0 */
     }

     SetMTRR(MTRR_MSR_AMDK6, inS.msrLo, inS.msrHi);
  }

  rVal = FXTRUE;

  return rVal;
} // pciSetAmdK6MTRR


