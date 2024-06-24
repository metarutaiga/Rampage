/* -*-c++-*- */
/* $Header: funcapi.c, 5, 10/5/00 9:59:03 AM PDT, Dan O'Connel$ */
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
** File name:   funcapi.c
**
** Description: Provide Functionality to check status & capabilities of card.
**
** $Revision: 5$
** $Date: 10/5/00 9:59:03 AM PDT$
**
** $Log: 
**  5    3dfx      1.4         10/5/00  Dan O'Connel    Correct code which reads
**       or sets PllCtrl0 and PllCtrl1 registers.  Most of this code was 
**       originally incompletely ported from Napalm.  Also cleanup (mostly by 
**       removing) some other unused or obsolete code concerning Alternate 
**       Timings for DFPs and SGRAMMODE.
**  4    3dfx      1.3         8/21/00  Geoff Bullard   Porting necessary fixes 
**       and cleanup from Napalm 9x: call to DFPisPanelPresent() becomes 
**       DFPisAdapterDfpCapable().
**  3    3dfx      1.2         7/17/00  Dan O'Connel    Major changes ported 
**       from Napalm driver to support: Registry Controlled Modes, DFP, TvOut, 
**       read OEM config from BIOS, updated 3dfx Tools support, and other 
**       features and bug fixes.
**  2    3dfx      1.1         12/22/99 Ryan Bissell    New clut management code
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $ 
*/


#define _TEXT(x) x
#include "funcapi.h"
#include "edgedefs.h"
#include "edgecaps.h"

// driver specific includes below 
#include "header.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)
#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "h3g.h"
#include "modelist.h"
#include "qmodes.h"	/* for LPQIN Structure */
#include "tv.h"
#include "dfpapi.h"

#define Not_VxD
#include "minivdd.h"

#include <string.h>
#include <pci.h>

#define	FREF	143184   //Fref*10000 

#define PCI_SSVID	0x2C	//Subsystem Vendor ID
#define PCI_VID		0x00	//Vendor ID
#define STB_SSVENDOR_ID					(0x000010b4L)
#define TDFX_SSVENDOR_ID				(0x0000121aL)
#define BUS_MASK	0x0001
#define	TVO_MASK	0x0002
#define LCD_MASK	0x0004
#define MEM_MASK	0x0008

extern DISPLAYINFO DisplayInfo;
extern DWORD dwDevNode;
extern DWORD dwDeviceHandle;
extern long atol( const char *string );

extern void TVOutGetStandard( LPQIN lpQIN, LPTVGETSTANDARD lpOutput );
extern void TVOutStatus( LPQIN lpQIN, LPQTVSTATUS lpOutput );


int FunctionAPIGetProperty(void * pInData,void * pOutData)
{
   TVSETSTANDARD Output;
   
   DWORD   grxFreq, grxClock;

	switch (((STB_PROPERTY *)pInData)->ulPropertyId)
		{	
		case (STB_FUNCTION_CAPS):
		{		
			/* initialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			/* ensure VGA capabilities are always enabled */
			
			((STB_PROPERTY *)pOutData)->ulPropertyValue=
				((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_VGA;

			/* check tv out status */
            if (_FF(dwTvoCapable))
            {
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = 
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_TV;
            }
            else 
            {
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = 
                        ((STB_PROPERTY *)pOutData)->ulPropertyValue & ~STB_FUNCTIONCAPS0_TV;
            }


			/* check for DFP support */
			
			if (DFPisAdapterDfpCapable())
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue=
				((STB_PROPERTY *)pOutData)->ulPropertyValue | STB_FUNCTIONCAPS0_DFP;
			}
			else
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue=
				(((STB_PROPERTY *)pOutData)->ulPropertyValue) & ~STB_FUNCTIONCAPS0_DFP;
			}


			return (STB_ESCAPE_HANDLED);
		}


	case (STB_FUNCTION_ACTIVE):
		{
			QTVSTATUS TVStatus;
			
			QGETSET_MONITOR_CTL Monitor;

			/* initialise output structure */

			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			/* check if vga monitor is enabled */

			/* QUERY_ANALOG_MONITOR in qmodes.c casts lpQIN to (QGETSET_MONITOR_CTL *) */
			/* which is a struct of DWORD dwSubfunc, DWORD monitorStatus and */
			/* DWORD monitorControl */

			Monitor.dwSubFunc = QUERY_ANALOG_MONITOR;
			Monitor.monitorStatus = 0;
            Monitor.monitorControl = 0;

			QueryMode((LPQIN)&Monitor, (LPVOID)&TVStatus); 
			/* second parameter is ignored for this call */

			if (Monitor.monitorStatus == MONITOR_IS_ENABLED)
			{
				((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_VGA;
			}

			/* check tv out status */

			if (_FF(dwTvoActive))
			{
	  			((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_TV;
			}
		
			/* Check for DFP enabled */

			if (DFPisPanelActive())
      		{
				((STB_PROPERTY *)pOutData)->ulPropertyValue |= STB_FUNCTIONACTIVE0_DFP;
			}

			return(STB_ESCAPE_HANDLED);
		}
	case (STB_SIM_VGA):
        {

            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
            ((STB_PROPERTY *)pOutData)->ulPropertyValue=0x00;

            // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for either PAL or NTSC.
            // Note: "allowPALCRT" allows simultaneous TV/VGA for both PAL and NTSC even though the name
            // is misleading.
#if 0
            if (!(_FF(allowPALCRT)))
            {
#endif
                // allow simultanious vga/dfp
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = STB_SIM0_DFP | STB_SIM0_VGA;
#if 0
            }
            else
            {
                // allow simultanious vga/dfp and vga/tv
                ((STB_PROPERTY *)pOutData)->ulPropertyValue=STB_SIM0_TV | STB_SIM0_DFP | STB_SIM0_VGA;
            }
#endif
            ((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
            return (STB_ESCAPE_HANDLED);
        }

    case (STB_SIM_TV):
        {
            STB_PROPERTY	property, returnproperty;

            ((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
            ((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;

            property.Guid=((STB_PROPERTY *)pInData)->Guid;
            property.ulPropertyId=STB_FUNCTION_CAPS;

            /* do escape call to see if there is any point in proceding */

            FunctionAPIGetProperty((void *)&property,(void *) &returnproperty);	

            if  ((returnproperty.ulResult == STB_ESCAPE_HANDLED) &&
                    (returnproperty.ulPropertyValue & STB_FUNCTIONCAPS0_TV))
            {
#if 0
                // Unless "allowPALCRT" is specified don't allow simultaneous TV/VGA for either PAL or NTSC.
                // Note: "allowPALCRT" allows simultaneous TV/VGA for both PAL and NTSC even though the name
                // is misleading.
                if (!(_FF(allowPALCRT)))
                {
#endif
                    ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_TV);
#if 0
                }
                else
                {
                    ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA | STB_SIM0_TV | !STB_SIM0_DFP) ;
                }
#endif

                ((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
            }
            else
            {	/* if tv is not supported return error code */
                ((STB_PROPERTY *)pOutData)->ulPropertyValue = 0x00;
                return (STB_FEATURE_NOT_SUPPORTED);
            }

            return (STB_ESCAPE_HANDLED);
        }

    case (STB_SIM_DFP):
		{
			STB_PROPERTY	property, returnproperty;
			
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			
			property.Guid=((STB_PROPERTY *)pInData)->Guid;
			property.ulPropertyId=STB_FUNCTION_CAPS;

			/* do escape call to see if there is any point in proceding */

			FunctionAPIGetProperty((void *)&property,(void *) &returnproperty);	
			
			if  ((returnproperty.ulResult == STB_ESCAPE_HANDLED) &&
				(returnproperty.ulPropertyValue & STB_FUNCTIONCAPS0_DFP))
			{
               ((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)(STB_SIM0_VGA | !STB_SIM0_TV | STB_SIM0_DFP);
				((STB_PROPERTY *)pOutData)->ulResult=(DWORD) STB_ESCAPE_HANDLED;
			}
			else
			{	/* if tv is not supported return error code */
				((STB_PROPERTY *)pOutData)->ulPropertyValue = 0x00;
				return (STB_FEATURE_NOT_SUPPORTED);
			}
			return (STB_ESCAPE_HANDLED);
           
	case (STB_FUNCTION_CLOCKRATE): // Return the current clock rate	in MHz
		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulPropertyValue=0x0000;
			((STB_PROPERTY *)pOutData)->ulResult=1;

            // get graphics clock from low half of pllCtrl0
            grxClock = GET(lph3IORegs->pllCtrl0) & 0xffff;
	         
	        grxFreq = PLL2MHz(grxClock);	
          
          			
			((STB_PROPERTY *)pOutData)->ulPropertyValue = grxFreq ;

		    return (STB_ESCAPE_HANDLED);
		}
       
	case (STB_FUNCTION_CLOCKDEFAULT): // Return the default clock rate in MHz

		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult=1;

            grxClock = _FF(dwDefaultClock) & 0xffff;
			((STB_PROPERTY *)pOutData)->ulPropertyValue = PLL2MHz(grxClock);
           
			return (STB_ESCAPE_HANDLED);
		}
       
       
	case (STB_FUNCTION_CLOCKMAX): // Return the Max clock rate in MHz

		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			((STB_PROPERTY *)pOutData)->ulPropertyValue = 220; // This should have been got from the MiniVDD .h files, but
           												   // these files define an array for the clock speeds, so the 
                                                              // .h files can only be called once in a module 


			return (STB_ESCAPE_HANDLED);
		}

	case (STB_FUNCTION_CLOCKMIN): // Return the Min clock rate in MHz

		{		
			/* intitialise output structure */
			((STB_PROPERTY *)pOutData)->Guid=((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulPropertyId=((STB_PROPERTY *)pInData)->ulPropertyId;
			((STB_PROPERTY *)pOutData)->ulResult=1;

			((STB_PROPERTY *)pOutData)->ulPropertyValue = 100;  // This should have been got from the MiniVDD .h files, but
           												   // these files define an array for the clock speeds, so the 
                                                              // .h files can only be called once in a module 


			return (STB_ESCAPE_HANDLED);
		}
       
       
	case (STB_DISPLAY_ENUM):
// *********************************************************
// Property name : STB_DISPLAY_ENUM
// Gets the registry location of the driver from the DevNode
// The location is of the form :
// HKLM\System\CurrentControlSet\Class\Display\xxxx
// We then get the value represented by XXXX, convert it to a DWORD
// and return this in ulPropertyValue
// *********************************************************
		{
			char szBuffer[255];
			char szDisplayEnum[255];
			int iSize;
			int i,j;
			int iResult;
			long dwEnum; 
			DWORD	dwVendorID, dwDeviceID;
			/* DWORD dwSubVendorID;  Not needed.  See SNadella's comment below*/

			/* 
				First of all do the checks to see if it is an STB or 3DFX card
			*/

			CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_VID, &dwVendorID, sizeof(DWORD), 0 );

			dwDeviceID = dwVendorID>>16;

			if(dwDeviceID < 3)
			{
				((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_NON_3DFX_CARD;
				return (STB_NON_3DFX_CARD);
			}

			dwVendorID &= 0x0000ffff; 

			if (! ((dwVendorID == STB_SSVENDOR_ID) || (dwVendorID == TDFX_SSVENDOR_ID)) )
			{
				((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_NON_3DFX_CARD;
				return (STB_NON_3DFX_CARD);
			}

			/*
				Get SubVendor ID to ensure that it is a 3Dfx or STB board
			*/
            /*
                    Get SubVendor ID to ensure that it is a 3Dfx or STB board
            */
            /*  SNadella: PRS 8829 This function call has a bug
                          on Packard Bell's new motherboard upon
                          returning from Power standby mode. It doesnt
                          look like we need to verify the Sub Vendor Id for
                          V3 since we are already verifying the Vendor ID and
                          only 3dfx makes V3 boards now
            */
            /*
			CM_Call_Enumerator_Function( _FF(DevNode),
				     PCI_ENUM_FUNC_GET_DEVICE_INFO,
				     PCI_SSVID, &dwSubVendorID, sizeof(DWORD), 0 );


			dwSubVendorID &= 0x0000ffff; 
			if (!(	( dwSubVendorID == STB_SSVENDOR_ID ) || 
					( dwSubVendorID == TDFX_SSVENDOR_ID ) ||
					( dwSubVendorID == 0xFFFF )	// stuck in due to there being too many feckin' reference boards!!
					))

			{
				// this is not one of our cards
				((STB_PROPERTY *)pOutData)->ulResult = (DWORD)STB_NON_3DFX_CARD; 
				return (STB_NON_3DFX_CARD);
			}
			*/


			iResult = CM_Get_DevNode_Key(	DisplayInfo.diDevNodeHandle, 
											NULL, 
											&szBuffer, 
											sizeof(szBuffer), 
											CM_REGISTRY_SOFTWARE);

		// Test the value of iResult and the length of szBuffer for
		// the possible chance of an error occuring
	
		// Get the last 4 characters of the string (plus the null terminator) 
		// - these represent the number that we want 

			iSize=lstrlen(szBuffer);
			j=0; 
			for(i=(iSize-4);i<(iSize+1);i++)
			{
				szDisplayEnum[j]=szBuffer[i];
				j++;
			}
			dwEnum = atol(szDisplayEnum);
			DPF(DBGLVL_ALL,"Enum = %d ", dwEnum);

			((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
			((STB_PROPERTY *)pOutData)->ulResult = STB_ESCAPE_HANDLED; 
			((STB_PROPERTY *)pOutData)->ulPropertyId = STB_DISPLAY_ENUM;
			((STB_PROPERTY *)pOutData)->ulPropertyValue = (DWORD)dwEnum;
			return (STB_ESCAPE_HANDLED);
			}
		default:
			{
				((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
				((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
				((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
				return(STB_ERROR_SETTING_VALUE);
			}
		}
    }
}

void TVOutCancelSettings(void * pInData,void * pOutData);

int FunctionAPISetProperty(void * pInData,void * pOutData)
{
   ((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
   ((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
   ((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_ESCAPE_HANDLED;

  	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
      case (STB_FUNCTION_CANCEL):
         DFPCancelSettings(pInData, pOutData);
         TVOutCancelSettings(pInData, pOutData);
         _FF(ulDevicesThatHaveTheirStatesSaved) = 0UL;
         return (STB_ESCAPE_HANDLED);

      case (STB_FUNCTION_COMMIT):
         _FF(ulDevicesThatHaveTheirStatesSaved) = 0UL;
         return (STB_ESCAPE_HANDLED);

   	default:
	/*
		*************************
		Function API is Read Only
		************************* 
	*/
	((STB_PROPERTY *)pOutData)->Guid =  ((STB_PROPERTY *)pInData)->Guid;
	((STB_PROPERTY *)pOutData)->ulPropertyId =  ((STB_PROPERTY *)pInData)->ulPropertyId;
	((STB_PROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
	return (STB_ESCAPE_NOT_SUPPORTED);
}
}

int FunctionAPIGetGroupProperty(void * pInData,void * pOutData)
{
	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
	default:
		{
			((STB_GROUPPROPERTY *)pOutData)->Guid = ((STB_GROUPPROPERTY *)pInData)->Guid;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)pInData)->ulPropertyId;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertySize = ((STB_GROUPPROPERTY *)pInData)->ulPropertySize;
			((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
	}


}

int FunctionAPISetGroupProperty(void * pInData,void * pOutData)
{
	switch (((STB_GROUPPROPERTY *)pInData)->ulPropertyId)
	{
	default:
		{
			((STB_GROUPPROPERTY *)pOutData)->Guid = ((STB_GROUPPROPERTY *)pInData)->Guid;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)pInData)->ulPropertyId;
			((STB_GROUPPROPERTY *)pOutData)->ulPropertySize = ((STB_GROUPPROPERTY *)pInData)->ulPropertySize;
			((STB_GROUPPROPERTY *)pOutData)->ulResult=(DWORD)STB_FEATURE_NOT_SUPPORTED;
			return (STB_ESCAPE_NOT_SUPPORTED);
		}
	}
}


int FunctionAPIInit()
{

	QTVSTATUS TVStatus;

#if 0
//??? not ported from WinNT
    TDFX_QUERY_VALUE_INFO queryValueInfo;
    DWORD                 numBytes;
#endif

   #define REG_D3D_SINGLE   "D3D\\SingleChipAASLI"
   #define REG_D3D_DUAL     "D3D\\DualChipAASLI"
   #define REG_D3D_QUAD     "D3D\\QuadChipAASLI"    
   
   #define REG_GLIDE_SINGLE   "Glide\\SingleChipAASLI"
   #define REG_GLIDE_DUAL     "Glide\\DualChipAASLI"
   #define REG_GLIDE_QUAD     "Glide\\QuadChipAASLI"
   
   
   #define REG_DISABLED "Disabled"
   #define REG_CONTROL  "Control"
   #define REG_ENABLE	 "0"
   #define REG_DISABLE	 "2"
   
   char tempStr[256];  
   DWORD length = sizeof(tempStr); 
   
   PFARVOID singleStatus; 
   PFARVOID dualStatus;
   PFARVOID quadStatus;
   DWORD    chips;
   DWORD    dwStatus;

   DWORD	DevNode = _FF(DevNode);
   
   

   /* Write out the produce name */

   /* Get the name from the BIOS through the MiniVdd */
   dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_OEM_BOARD_NAME, 0, tempStr);
   if (0 != dwStatus)
       tempStr[0] = '\0';  //if error set to empty string.
   
   
   if (_FF(AGPCaps & IS_AGP_CARD)) 
	   	lstrcat((LPSTR)&tempStr,(LPCSTR)"AGP");
	else 
		lstrcat((LPSTR)&tempStr,(LPCSTR)"PCI");

   
   CM_Write_Registry_Value(DevNode, "", (PFARCHAR) "DriverDesc", 
						   	REG_SZ, (PFARCHAR) &tempStr, sizeof(tempStr), 
						   	CM_REGISTRY_SOFTWARE);
   

   /* Turn on different tweaks based on the number of graphic chips */
   
   
   /* If we are running on a Voodoo3, then switch all the AA/SLI tweaks
      off by making the chip number selection fall through to default 
      condition */
   
#ifdef SLI
   chips = _FF(dwNumUnits);
#else
   chips = 1;
#endif
   
   switch (chips)
	{
   	case 1:
   	{
       	singleStatus = REG_ENABLE; 
   		dualStatus   = REG_DISABLE;
   		quadStatus   = REG_DISABLE;
      
   		break;
   	}
       
       case 2:
       {
       	singleStatus = REG_DISABLE; 
   		dualStatus   = REG_ENABLE;
   		quadStatus   = REG_DISABLE;
      
   		break;
       }
       	
       case 4:
       {
       	singleStatus = REG_DISABLE; 
   		dualStatus   = REG_DISABLE;
   		quadStatus   = REG_ENABLE;
      
   		break;

       }
       
       default:
       {
       	singleStatus = REG_DISABLE; 
   		dualStatus   = REG_DISABLE;
   		quadStatus   = REG_DISABLE;
      
   		break;
       }
   }
   
   
   /* Write out if the registry entries exist. All tweaks will have a Control key - so 
      check for it */
      
   /* D3D First */   
   
   if (CM_Read_Registry_Value(DevNode, REG_D3D_SINGLE, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_SINGLE, (PFARCHAR) REG_DISABLED, 
									REG_SZ, singleStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
         
   if (CM_Read_Registry_Value(DevNode, REG_D3D_DUAL, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_DUAL, (PFARCHAR) REG_DISABLED, 
									REG_SZ, dualStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   if (CM_Read_Registry_Value(DevNode, REG_D3D_QUAD, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_D3D_QUAD, (PFARCHAR) REG_DISABLED, 
									REG_SZ, quadStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   /* Then the Gilde */
   
   if (CM_Read_Registry_Value(DevNode, REG_GLIDE_SINGLE, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_GLIDE_SINGLE, (PFARCHAR) REG_DISABLED, 
									REG_SZ, singleStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
         
   if (CM_Read_Registry_Value(DevNode, REG_GLIDE_DUAL, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_GLIDE_DUAL, (PFARCHAR) REG_DISABLED, 
									REG_SZ, dualStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}
       
   if (CM_Read_Registry_Value(DevNode, REG_GLIDE_QUAD, REG_CONTROL, REG_SZ,
   	(LPBYTE)&tempStr[0], &length, CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
   	{
   		CM_Write_Registry_Value(DevNode, REG_GLIDE_QUAD, (PFARCHAR) REG_DISABLED, 
									REG_SZ, quadStatus, sizeof(REG_DISABLE), 
									CM_REGISTRY_SOFTWARE);
   	}

			
    // check tv out status to insure ppdev->dwTvoActive is properly initialized
    // ppdev->dwTvoActive is updated as sideeffect of above TVOutGetStatus call.
    TVOutStatus( NULL /*not used*/, &TVStatus );

    // save TVOut Capable flag to reduce the number of times the this module needs to call TVOutStatus.
    //  Each time TVOutStatus is called the TV screen is garbled.
    if (TV_ENCODER_PRESENT == (TVStatus.dwEncoder & TV_ENCODER_PRESENT))
        _FF(dwTvoCapable) = TRUE;
    else 
        _FF(dwTvoCapable) = FALSE;

#if 0
// not ported from WinNT DanO 1/25/99
    // Check if allowPALCRT registry key for a string exists in registry.  If so allow simultanious PAL and CRT display.
    ppdev->bAllowPALCRT = FALSE;
    // call ioctl to have miniport read data from registry
    queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
    if (! EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                           "allowPALCRT",
                           strlen("allowPALCRT") + 1,
                           &queryValueInfo,
                           sizeof(queryValueInfo),
                           &numBytes))
    {
      ppdev->bAllowPALCRT = TRUE;
    }
#endif

   return(0);
   
}


// Function to convert half a Rampage PLL control reg to Frequency.
FxU32	PLL2MHz(FxU32 pll )
{
	FxU32	k, exp, m, n, fout;

	k		= (pll >> 14) & 0x3;	// Post divider value:	bits[15:14] 
	exp		= 1 << k;				// 2^k used in conversion

	m		= (pll >> 8) & 0x3f;	// PLL input divider:	bits[13:8]

	n		= pll & 0xff;			// PLL multiplier:		bits[7:0]
	
	// formula for MHz given by the PllCtrl specification
	fout	= (FREF * (n + 2)) / ((m + 2) * (exp));

	// FREF was scaled up, so downscale result and return
	return (fout / 10000);
}

 
 

 
 


