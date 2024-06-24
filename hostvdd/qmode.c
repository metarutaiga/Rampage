/* $Header: qmodes.c, 5, 8/3/00 4:07:21 PM PDT, Miles Smith$ */
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
** File name:   qmodes.c
**
** Description: QueryMode and supporting functions.
**
** $Revision: 5$ 
** $Date: 8/3/00 4:07:21 PM PDT$
**
** $History: qmodes.c $
** 
** *****************  Version 4  *****************
** User: Peterm       Date: 7/30/99    Time: 2:38a
** Updated in $/devel/sst2/Win95/dx/dd16
** modified for compiler options cleanup
** 
** *****************  Version 3  *****************
** User: Peterm       Date: 7/07/99    Time: 5:03p
** Updated in $/devel/sst2/Win95/dx/dd16
** updated to work with -DR21 and -WX on command line
** 
** *****************  Version 2  *****************
** User: Peterm       Date: 5/21/99    Time: 4:05p
** Updated in $/devel/sst2/Win95/dx/dd16
** remerged with v3 tot
** 
** *****************  Version 34  *****************
** User: Stb_rbissell Date: 5/14/99    Time: 9:47p
** Updated in $/devel/h3/win95/dx/dd16
** tvout and dfp changes
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 5/10/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 32  *****************
** User: Andrew       Date: 5/06/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/dd16
** Modified to work with Trapping "C" Simulator
** 
** *****************  Version 31  *****************
** User: Stb_mimhoff  Date: 4/19/99    Time: 12:18p
** Updated in $/devel/h3/win95/dx/dd16
** When running TV Out and shutting down the monitor, it is generally
** preferred to shut down the VSYNC, not the HSYNC...Halting the VSYNC is
** the same action that we do in DPMS - Suspend.
** 
** *****************  Version 30  *****************
** User: Andrew       Date: 3/13/99    Time: 7:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Added code to check DACMODE before checking VSYNC
** 
** *****************  Version 29  *****************
** User: Stuartb      Date: 2/25/99    Time: 12:48p
** Updated in $/devel/h3/Win95/dx/dd16
** Added table to reference individual board caps bt subsysID.  Added
** FPFLAG_CAPABLE.
** 
** *****************  Version 28  *****************
** User: Stuartb      Date: 2/23/99    Time: 3:06p
** Updated in $/devel/h3/Win95/dx/dd16
** A change to pass FPFLAG_FILTER_XX got lost.
** 
** *****************  Version 27  *****************
** User: Stuartb      Date: 2/18/99    Time: 2:54p
** Updated in $/devel/h3/Win95/dx/dd16
** Added xlcd FPFLAGS for filter mode setting.
** 
** *****************  Version 26  *****************
** User: Andrew       Date: 2/11/99    Time: 11:17a
** Updated in $/devel/h3/Win95/dx/dd16
** Added AGP cap Query
** 
** *****************  Version 25  *****************
** User: Stuartb      Date: 2/02/99    Time: 5:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Minor change to LCD_CTRL.  Don't tweak hardware unless we enable or
** disable panel (eg. if only request for status).
** 
** *****************  Version 24  *****************
** User: Stuartb      Date: 2/01/99    Time: 2:18p
** Updated in $/devel/h3/Win95/dx/dd16
** Once again, when disabling analog crt do not set SST_DAC_FORCE_VSYNC
** lest we pend on VRETRACE which will never happen.
**  
** 
** *****************  Version 23  *****************
** User: Stuartb      Date: 1/29/99    Time: 9:50a
** Updated in $/devel/h3/Win95/dx/dd16
** When disabling analog CRT don't force syncs as this stops VRETRACE
** toggling.
** 
** *****************  Version 22  *****************
** User: Stuartb      Date: 1/28/99    Time: 4:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Reinstate broadcastMonitor change on tv/lcd enable/disable.
** 
** *****************  Version 21  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:38a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 20  *****************
** User: Stuartb      Date: 1/21/99    Time: 11:18a
** Updated in $/devel/h3/Win95/dx/dd16
** Added FPFLAG_SETRES and FPFLAG_SETREFR for app code.  Removed
** broadcastMonitorChange calls.
** 
** *****************  Version 19  *****************
** User: Stuartb      Date: 1/14/99    Time: 3:46p
** Updated in $/devel/h3/Win95/dx/dd16
** Changed flat panel & tvout enable/disable philosophy.  No longer
** requires booting to that device to enable.
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 1/12/99    Time: 2:27p
** Updated in $/devel/h3/Win95/dx/dd16
** Added QUERY_ANALOG_MONITOR & modified QUERY_LCDCTRL.
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 1/08/99    Time: 8:26p
** Updated in $/devel/h3/Win95/dx/dd16
** Fixed load of palette to be in Vertical Retrace
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 1/08/99    Time: 3:34p
** Updated in $/devel/h3/Win95/dx/dd16
** Added QUERY_LCDCTRL for control panel flat panel ops.
** 
** *****************  Version 15  *****************
** User: Bob          Date: 1/05/99    Time: 5:14p
** Updated in $/devel/h3/Win95/dx/dd16
** Altered nesting of header files so that tv.h is not inside qmodes.h.
** 
** This is a sharing issue with NT.
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 1/05/99    Time: 10:52a
** Updated in $/devel/h3/Win95/dx/dd16
** Added new function to retrieve bios version string
** 
** *****************  Version 13  *****************
** User: Michael      Date: 12/29/98   Time: 2:36p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
** 
** 12    10/22/98 10:33a Stuartb
** Added TvOut extern function defs.  These were previously in tvout.h but
** DX6 now has a real tvout.h that we need to include.
** 
** 11    10/16/98 3:37p Michael
** In QUERYSETGAMMA, check default_gamma_flag before assign_gammaramp().
** 
** 10    9/16/98 5:35p Michael
** Fred's (igx) assign_gammaramp() implementation and a minor gamma fix.
** 
** 9     9/16/98 11:06a Stuartb
** Added additional tvout functionality.
** 
** 7     9/10/98 2:53p Stuartb
** Added TVOUT ExtEscapes.
** 
** 6     8/27/98 6:21p Andrew
** Added Gamma Correction
** 
** 5     8/21/98 7:34p Michael
** The GammaRamp implementation broke the 3Dfx properties page.  Fix made
** in QUERYSETGAMMA to initialize gamma_ramp structure.
** 
** 4     7/13/98 5:25p Andrew
** Changed to support a GammaTable
** 
** 3     7/11/98 8:15a Andrew
** Added Gamma Code
** 
** 2     5/12/98 9:34a Andrew
** Updates to add TV out and fix minor bugs
** 
** 1     4/22/98 2:48p Andrew
** Query Mode Function
** 
*/


/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include <string.h>
#include "header.h"

#define Not_VxD
#include "minivdd.h"

#define Not_VxD
#include <vmm.h>

#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)

#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "h3g.h"
#include "modelist.h"
#include "tv.h"
#include "qmodes.h"
#include "gramp.h"
#include "dfpapi.h"

extern DISPLAYINFO DisplayInfo;
extern DWORD dwDeviceHandle;

extern void TVOutStatus( LPQIN lpQIN, LPQTVSTATUS lpOutput );
extern void TVOutConStatus( LPQIN lpQIN, LPTVCONSTATUS lpOutput );
extern void TVOutGetPicCap( LPQIND lpQIN, LPTVCAPDATA lpOutput );
extern void TVOutGetFilterCap( LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetPosCap( LPQIN lpQIN, LPTVPOSCAP lpOutput );
extern void TVOutGetSizeCap( LPQIN lpQIN, LPTVSIZECAP lpOutput );
extern void TVOutGetSpecialCap( LPQIND lpQIN, LPVOID lpOutput );
extern void TVOutGetStandard( LPQIN lpQIN, LPTVGETSTANDARD lpOutput );
extern void TVOutGetPicControl( LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetFilterControl( LPQIND lpQIN, LPTVCURCAP lpOutput );
extern void TVOutGetPosControl( LPQIN lpQIN, LPTVCURPOS lpOutput );
extern void TVOutGetSizeControl( LPQIN lpQIN, LPTVCURSIZE lpOutput );
extern void TVOutGetSpecialCtl( LPQIND lpQIN, LPVOID lpOutput );
extern void TVOutGetConStatus( LPQIN lpQIN, LPTVCONSTATUS lpOutput );
extern void TVOutSetStandard( LPTVSETSTANDARD lpQIN );
extern void TVOutSetPicControl( LPTVSETCAP lpQIN );
extern void TVOutSetFilterControl( LPTVSETCAP lpQIN );
extern void TVOutSetPosControl( LPTVSETPOS lpQIN );
extern void TVOutSetSizeControl( LPTVSETSIZE lpQIN );
extern void TVOutSetSpecial( LPTVSETSPECIAL lpQIN );
extern void TVOutSetConStatus( LPTVSETCONNECTOR lpQIN );
extern void TVOutCommitReg( LPQIN lpQIN );
extern void TVOutRefreshMem( LPQIN lpQIN );
extern void TVOutDisable( );
extern void TVOutEnable( LPQIN lpQIN );


int DoList(void);

// When disabling a monitor, it is generally preferred to shut down the VSYNC,
// not the HSYNC... Halting the VSYNC is the same action that we do in DPMS - Suspend.
#define MONITOR_DPMS_MASK ( SST_DAC_DPMS_ON_VSYNC )

/*----------------------------------------------------------------------
Function name:  QueryMode

Description:    This Control Subfunction is used to handle all of
                the queries defined in qmodes.h.
Information:

Return:         INT     TDFXACK for Success or TDFXERR for Failure.
----------------------------------------------------------------------*/
int QueryMode(LPQIN lpQIN, LPVOID lpOutput)
{
   LPQGETAGPCAPS lpGetAGPCaps;
   LPQVERSION lpQVersion;
   LPQNUMMODE lpQNumMode;
   LPQMODE lpQMode;
   LPQDEVNODE lpQDevNode;
   LPQGETGAMMA lpQGetGamma;
   LPQSETGAMMA lpQSetGamma;
//   SstIoRegRW sstIO;
   DWORD dwStatus;
   int i;
   int nReturn = TDFXERR;
   GLOBALDATA *glbData = (void *)&_FF(lpPDevice);

   switch (lpQIN->dwSubFunc)
      {
      case QUERYVERSION:
         lpQVersion = (LPQVERSION)lpOutput;
         lpQVersion->dwMajor = QUERYMODE_MAJOR;
         lpQVersion->dwMinor = QUERYMODE_MINOR;
         nReturn = TDFXACK;
         break;

      case QUERYNUMMODES:
         lpQNumMode = (LPQNUMMODE)lpOutput;
         for (i=0; 0 != ModeList[i].dwWidth; i++)
            ;
         lpQNumMode->dwNum = i;
         nReturn = TDFXACK;
         break;

      case QUERYMODES:
         lpQMode = (LPQMODE)lpOutput;
		 di_ValidateModeList();  	// Re-validate the Mode List array - DYNAMIC MODE TABLE 
         for (i=0; 0 != ModeList[i].dwWidth; i++)
            {
            lpQMode->dwX = ModeList[i].dwWidth;
            lpQMode->dwY = ModeList[i].dwHeight;
            lpQMode->dwBpp = ModeList[i].dwBPP;
            if (IS_VALID_MODE == (ModeList[i].dwFlags & IS_VALID_MODE))
               lpQMode->dwValid = QUERY_MODE_VALID;
            else
               lpQMode->dwValid = 0x0L;
            lpQMode->dwRef = (DWORD)ModeList[i].wVert;
            lpQMode++;
            }
         nReturn = TDFXACK;
         break;

      case QUERYDEVNODE:
         lpQDevNode = (LPQDEVNODE)lpOutput;
         lpQDevNode->dwDevNode = DisplayInfo.diDevNodeHandle;
         nReturn = TDFXACK;
         break;

      case QUERYGETGAMMA:
         lpQGetGamma = (LPQGETGAMMA)lpOutput;
         lpQGetGamma->dwRed = _FF(dwRed);
         lpQGetGamma->dwGreen = _FF(dwGreen);
         lpQGetGamma->dwBlue = _FF(dwBlue);
         for (i=0; i<256; i++)
            lpQGetGamma->GammaTable[i] = _FF(GammaTable)[i];
         nReturn = TDFXACK;
         break;

      case QUERYSETGAMMA:
         lpQSetGamma = (LPQSETGAMMA)lpQIN;
         _FF(dwRed) = lpQSetGamma->dwRed;
         _FF(dwGreen) = lpQSetGamma->dwGreen;
         _FF(dwBlue) = lpQSetGamma->dwBlue;
         for (i=0; i<256; i++)
            _FF(GammaTable)[i] = lpQSetGamma->GammaTable[i];

         //now install either the gamma table, or a gamma-tized 256 entry palette
         setupPalette(0, 256, NULL);
         nReturn = TDFXACK;
         break;

      case QUERYSETOVERLAYGAMMA:
         lpQSetGamma = (LPQSETGAMMA)lpQIN;
         set_overlaygamma(0, 256, lpQSetGamma->GammaTable);
         nReturn = TDFXACK;
         break;

      case QUERYGETAGPCAPS:
         lpGetAGPCaps = (LPQGETAGPCAPS)lpOutput;
         lpGetAGPCaps->dwAGPCaps = _FF(AGPCaps);            
         nReturn = TDFXACK;
         break;

      case QUERYGETBIOSVERSION:
         dwStatus = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_GET_BIOS_VERSION, 0, lpOutput);
         if (0 == dwStatus)
            nReturn = TDFXACK;
         else
            nReturn = TDFXERR;
         break;

      //TV-Out functions----------------------------------------------------------//
      
      case QUERYTVAVAIL:
         TVOutStatus( lpQIN, (LPQTVSTATUS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYTVSENSE:
         TVOutConStatus( lpQIN, (LPTVCONSTATUS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPICCAP:
         TVOutGetPicCap( (LPQIND)lpQIN, (LPTVCAPDATA)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETFILTERCAP:
         TVOutGetFilterCap( (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPOSCAP:
         TVOutGetPosCap( lpQIN, (LPTVPOSCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSIZECAP:
         TVOutGetSizeCap( lpQIN, (LPTVSIZECAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSPECIALCAP:
         TVOutGetSpecialCap( (LPQIND)lpQIN, (LPVOID)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSTANDARD:
      case QUERYGETOVERRIDE:
         TVOutGetStandard( lpQIN, (LPTVGETSTANDARD)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPICCONTROL:
         TVOutGetPicControl( (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETFILTERCONTROL:
         TVOutGetFilterControl( (LPQIND)lpQIN, (LPTVCURCAP)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETPOSCONTROL:
         TVOutGetPosControl( lpQIN, (LPTVCURPOS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSIZECONTROL:
         TVOutGetSizeControl( lpQIN, (LPTVCURSIZE)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETSPECIAL:
         TVOutGetSpecialCtl( (LPQIND)lpQIN, (LPVOID)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYGETCONSTATUS:
         TVOutGetConStatus( lpQIN, (LPTVCONSTATUS)lpOutput );
         nReturn = TDFXACK;
         break;
      case QUERYSETOVERRIDE:
      case QUERYSETSTANDARD:
         TVOutSetStandard( (LPTVSETSTANDARD)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETPICCONTROL:
         TVOutSetPicControl( (LPTVSETCAP)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETFILTERCONTROL:
         TVOutSetFilterControl( (LPTVSETCAP)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETPOSCONTROL:
         TVOutSetPosControl( (LPTVSETPOS)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETSIZECONTROL:
         TVOutSetSizeControl( (LPTVSETSIZE)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSSETSPECIAL:
         TVOutSetSpecial( (LPTVSETSPECIAL)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYSETCONSTATUS:
         TVOutSetConStatus( (LPTVSETCONNECTOR)lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYCOMMITREG:
         TVOutCommitReg( lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYREFRESH:
         TVOutRefreshMem( lpQIN );
         nReturn = TDFXACK;
         break;
      case QUERYDISABLETV:
         TVOutDisable( );
         nReturn = TDFXACK;
         break;
      case QUERYENABLETV:
         TVOutEnable( lpQIN );
         nReturn = TDFXACK;
         break;

      //End TV-Out section----------------------------------------------------------//

	  case QUERY_LCDCTRL:
#ifdef DEBUG
         _asm int 3;  // this interface is obsolete and should not be used.
#endif
         nReturn = TDFXACK;
         break;

	 case QUERY_ANALOG_MONITOR:
#if 0 // !! SST2
         sstIO.regRWflags = 0;
		 sstIO.regAddr = (DWORD)&((SstIORegs FAR *)0L)->dacMode;
		 VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
										H3VDD_RW_REGISTER, 0, &sstIO);
		 if (!(sstIO.regValue & MONITOR_DPMS_MASK))
			 ((QGETSET_MONITOR_CTL *)lpQIN)->monitorStatus = MONITOR_IS_ENABLED;
		 else
			 ((QGETSET_MONITOR_CTL *)lpQIN)->monitorStatus = 0;


         if (_FF(dwTvoActive)) //???napalmCode && (_FF(dwTvoStd) != VP_TV_STANDARD_NTSC_M) && !(_FF(allowPALCRT)))
         {
             ((QGETSET_MONITOR_CTL *)lpQIN)->monitorStatus = 0;                 //say the monitor is off
             ((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl = DISABLE_MONITOR;  //turn the monitor off
         }


		 sstIO.regRWflags = RWFLAG_WRITE_REG;
		 if (((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl & ENABLE_MONITOR)
			 sstIO.regValue &= ~MONITOR_DPMS_MASK;
		 else if (((QGETSET_MONITOR_CTL *)lpQIN)->monitorControl & DISABLE_MONITOR)
			 sstIO.regValue |= MONITOR_DPMS_MASK;
		 else
			 break;

		 VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
										H3VDD_RW_REGISTER, 0, &sstIO);
#endif // !! SST2
		 break;

   default:
         nReturn = TDFXERR;
         break;
      }

   return nReturn;
} 

#define RED_NAME  "SSTH3_RGAMMA"
#define BLUE_NAME  "SSTH3_BGAMMA"
#define GREEN_NAME  "SSTH3_GGAMMA"

#define FRACMULT (10000L)
#define RANGE(val, low, hi) ((low <= val) && (val <= hi))
#define EPILSON (10)
#define SUB(a,b) (((a) > (b)) ? (a)-(b) : (b)-(a))

DWORD gamma(WORD wValue, DWORD gamma1, DWORD g255);
DWORD ipow(DWORD a, DWORD x);
DWORD logex(DWORD x);
char *ddgetenv( const char *varname );
DWORD atof100x( const char *s );
DWORD idiv(DWORD x, DWORD y, DWORD fracmult);
DWORD imult(DWORD x, DWORD y, DWORD fracmult);


/*----------------------------------------------------------------------
Function name:  LoadGamma

Description:    Program the DAC.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void LoadGamma(void)
{
#ifdef WIN_CSIM
   SstIORegs FAR *lph3IORegs = (SstIORegs *)_FF(regRealBase);
#endif
   DWORD rgamma;
   DWORD ggamma;
   DWORD bgamma;
   DWORD rgamma1;
   DWORD rg255;
   DWORD ggamma1;
   DWORD gg255;
   DWORD bgamma1;
   DWORD bg255;
   DWORD color;
//   DWORD garbage;
   int i;
   LPSTR lpStr;

   lpStr = ddgetenv(RED_NAME);
   if (NULL != lpStr)
      rgamma = atof100x(lpStr);
   else
      rgamma = 100;

   lpStr = ddgetenv(GREEN_NAME);
   if (NULL != lpStr)
      ggamma = atof100x(lpStr);
   else
      ggamma = 100;

   lpStr = ddgetenv(BLUE_NAME);
   if (NULL != lpStr)
      bgamma = atof100x(lpStr);
   else
      bgamma = 100;

   // Clamp these babies
   if (rgamma < 43)
      rgamma = 43;
   else if (rgamma > 400)
      rgamma = 400;
    
   if (ggamma < 43)
      ggamma = 43;
   else if (ggamma > 400)
      ggamma = 400;
    
   if (bgamma < 43)
      bgamma = 43;
   else if (bgamma > 400)
      bgamma = 400;

   rgamma1 = idiv(FRACMULT, rgamma * FRACMULT/100, FRACMULT);
   rg255 = ipow((DWORD)255 * FRACMULT, rgamma1);
   ggamma1 = idiv(FRACMULT, ggamma * FRACMULT/100, FRACMULT);
   gg255 = ipow((DWORD)255 * FRACMULT, ggamma1);
   bgamma1 = idiv(FRACMULT, bgamma * FRACMULT/100, FRACMULT);
   bg255 = ipow((DWORD)255 * FRACMULT, bgamma1);

   // wait for vblank b4 update dac entries to prevent sparkle
   //
   // Make sure Hsync and Vsync are toggling before we check them
#if 0 // !! SST2
   if  (!(GET(lph3IORegs->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
      {
       while (!((GET(lph3IORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK)))
	      ;
      }
#endif

    for (i=0; i<256; i++)
    {
        // try to reduce math since gamma is time consuming
        color = gamma(i, bgamma1, bg255);
        if (bgamma1 == ggamma1)
            color =  color | (color & 0xFF) << 8;
        else
            color =  color | gamma(i, ggamma1, gg255) << 8;

        if (bgamma1 == rgamma1)
            color =  color | (color & 0xFF) << 16;
        else
            color = color | gamma(i, rgamma1, rg255) << 16;
#if 0 // !! SST2
        SETDW(lph3IORegs->dacAddr, i);
        garbage = GET(lph3IORegs->dacAddr);
        SETDW(lph3IORegs->dacData, color);
        garbage = GET(lph3IORegs->dacData);
#endif
    }
}


/*----------------------------------------------------------------------
Function name:  gamma

Description:    This function computes (wValue/255)^(1/dwGamma)
                * 255 + .5 which is what we use to calculate
                gamma correction.

Information:

Return:         DWORD   The calculation noted above.
----------------------------------------------------------------------*/
DWORD gamma(WORD wValue, DWORD gamma1, DWORD g255)
{
   DWORD dwReturn;

   dwReturn = ((imult(idiv(ipow((DWORD)wValue * FRACMULT, gamma1), g255, FRACMULT), 255 * FRACMULT, FRACMULT) + 5 * FRACMULT/10)/FRACMULT);

   return dwReturn;
}


/*----------------------------------------------------------------------
Function name:  ipow

Description:    This function computes a^x by using the series
                exponential a^x = 1 + x * ln a + (x * ln a)^2/2!
                + (x * ln a) ^ 3/3! + ...

Information:    This is calculated to 4 decimal places

Return:         DWORD   The result of the calculation.
----------------------------------------------------------------------*/
DWORD ipow(DWORD a, DWORD x)
{
   DWORD dRet;
   DWORD dOldRet;
   DWORD xlogea;
   DWORD xnew;
   DWORD xfac;
   int n;
   int nCount=1000;

   if (a < EPILSON)
      return 0;
   xlogea = imult(x, logex(a), FRACMULT);
   n = 1;
   xfac = 1*FRACMULT;
   dRet = 1 * FRACMULT;
   dOldRet = 2;
   while ((SUB(dRet,dOldRet) > EPILSON) && (nCount-- > 0))
      {
      dOldRet = dRet;
      xnew = xlogea/n;
      xfac = imult(xfac, xnew, FRACMULT);
      n += 1;
      dRet = dRet + xfac;
      }

   return dRet;
}


/*----------------------------------------------------------------------
Function name:  logex

Description:    This functions computes ln x.  We use the formula:
                ln (1+x)/(1-x) = 2(x + x^3/3 + x^5/5 + x^7/7 + ...)
                where we set y=(1+x)/(1-x) and solve for x giving
                x=(y-1)/y+1)

Information:    This is calculated to 6 decimal places

Return:         DWORD   The result of the calculation.
----------------------------------------------------------------------*/
DWORD logex(DWORD x)
{
   DWORD xfac;
   DWORD xPart;
   DWORD xsqrd;
   DWORD xnum;
   DWORD dRet;
   DWORD dOldRet;
   int nCount = 1000;

   x = x * 100;
   xfac = idiv((x - 1 * FRACMULT*100L), (x + 1 * FRACMULT*100L), FRACMULT*100L);
   xsqrd = imult(xfac, xfac, FRACMULT*100L);

   dRet = xfac;
   dOldRet = 2*EPILSON;
   xPart = xfac;
   xnum = 3 * FRACMULT*100L;
   while ((SUB(dRet, dOldRet) > EPILSON) && (nCount-- > 0))
      {
      dOldRet = dRet;
      xPart = imult(xPart, xsqrd, FRACMULT*100L);
      dRet = dRet + idiv(xPart,xnum, FRACMULT*100L);
      xnum += (2 * FRACMULT*100L); 
      }

   dRet = dRet/100;
   dRet = dRet * 2;

   return dRet;
}


/*----------------------------------------------------------------------
Function name:  ddgetenv

Description:    Extract environment information from WIN.INI or
                the registry.
Information:

Return:         char *  to valid data or,
                        NULL if failure.
----------------------------------------------------------------------*/
#define  PRSIZE  50      // GetProfileString return buffer
char    rstr[ PRSIZE ];
char    DevNodeKey[MAX_VMM_REG_KEY_LEN];
char *ddgetenv( const char *varname )
{
  char    nstr[]="\0";
  char    *lpnull;
  char    *lprstr;
  DWORD   DevNode;


  lprstr = rstr;
  lpnull = nstr;

  // First check in [3dfx] section of WIN.INI
  if( GetProfileString( "3Dfx", varname, lpnull, lprstr, PRSIZE ) )
  {
    return( lprstr );
  }

  DevNode = _FF(DevNode);

  // convert devnode to a registry key
  // when successful, this thing returns a string something like
  // "System\CurrentControlSet\Services\Class\DISPLAY\XXXX"
  //
  if (CR_SUCCESS == CM_Get_DevNode_Key(DevNode,
                                        NULL,
                                        (PFARVOID)DevNodeKey,
                                        sizeof(DevNodeKey),
                                        CM_REGISTRY_SOFTWARE))
  {
    typedef struct _H3_SEARCH_TYPE
    {
      HKEY  hkey;
      char  *pszSubkey;
    } H3_SEARCH_TYPE;

    static const H3_SEARCH_TYPE H3SearchOrder[] =
    {
      { HKEY_CURRENT_USER,  "\\Glide" },
      { HKEY_CURRENT_USER,  NULL    },
      { HKEY_LOCAL_MACHINE, "\\Glide" },
      { HKEY_LOCAL_MACHINE, "\\D3D" },
      { HKEY_LOCAL_MACHINE, NULL    },
    };
    const H3_SEARCH_TYPE *pSearchLoc;
    HKEY  hkey;
    DWORD type;
    DWORD length;
    ULONG DevNodeKeyLength;


    // save original length of DevNodeKey
    DevNodeKeyLength = strlen(DevNodeKey);

    // loop over possible registry locations
    for (pSearchLoc = &H3SearchOrder[0];
         pSearchLoc < &H3SearchOrder[sizeof(H3SearchOrder)/sizeof(H3SearchOrder[0])];
         pSearchLoc++)
    {
      // if we have a non NULL subkey
      // tack the subkey to the end of the DevNodeKey
      if (NULL != pSearchLoc->pszSubkey)
        strcat(DevNodeKey, pSearchLoc->pszSubkey);

      // attempt to open the key
      if (ERROR_SUCCESS == RegOpenKey(pSearchLoc->hkey,
                                        DevNodeKey,
                                        &hkey))
      {
        // the key exists so attempt to read the value of varname
        length = sizeof(rstr);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             varname,
                                             0,
                                             &type,
                                             lprstr,
                                             &length))
        {
          // the value exists so check it's type
          // if it's a string then return it
          // otherwise loop to the next search location
          //
          // we could put a switch statement here
          // and convert other types to strings
          if (REG_SZ == type)
          {
            RegCloseKey(hkey);
            return lprstr;
          }
        }

        RegCloseKey(hkey);
      }

      // restore the original DevNodeKey, in case we tacked on a subkey above
      DevNodeKey[DevNodeKeyLength] = '\0';
    }
  }

  // Return NULL if not found
  return( NULL );
}


/*----------------------------------------------------------------------
Function name:  atof100x

Description:    Perform an ASCII to Float x 100.

Information:

Return:         DWORD   the converted value.
----------------------------------------------------------------------*/
DWORD atof100x( const char *s )
{
   DWORD left, sign, mult;
   int i;

   for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
     ;

   if (s[i] == '+' || s[i] == '-')
     sign = (s[i++] == '+') ? 1 : -1;
   else
     sign = 1; 
 
   for (left = 0; s[i] >= '0' && s[i] <= '9'; i++)
     left = (left * 10) + (s[i] - '0');

   left *= 100;
   if (s[i++] != '.')
     return( sign * left);
     
   // Here's where we get the bottom 2
   for (mult = 10; s[i] >= '0' && s[i] <= '9' && mult > 0; i++, mult /= 10)
     left = left + (mult * (s[i] - '0'));

   return(sign * left);
}
