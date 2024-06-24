/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/

#include "precomp.h"
#include "ddovl32.h"
#include "ddvpe32.h"
#include "sst2bwe.h"

#ifndef DPF
#define DPF	printf
#endif

DWORD GetClock(DWORD n, DWORD k, DWORD m)
 
{
        DWORD t1, t2;
        DWORD p2;
 
        t1 = (14318 * (n+2));
 
		if(k == 0)
			p2 = 1;
		else if(k == 1)
			p2 = 2;
		else if(k == 2)
			p2 = 4;
		else if (k == 3)
			p2 = 8;


		t2 = ((m+2) * p2);
 

//        DPF ("%ld \tm:%ld n:%ld k:%ld \n", t1 / t2,m,n,k);
 
		return (t1 /t2);
}

/********************************************************************************
* get VCLK and MCLK from the HW settings
*
*
**********************************************************************************/
void GetMVCLK( LPBW_CONFIG lpBWConfig)
{
    //use 90% of available memory bandwidth
	lpBWConfig->dwMCLK = GetClock(1090,2,24) * 9 /10;	//for test now
	lpBWConfig->sDesktop.dwVCLK = GetClock( 1140,2,24); //for test now
}

/*****************************************************************************************
*
*	Given pixel clock and scale factor
*	return bandwidth
*
*****************************************************************************************/

DWORD BandWidthForScale( DWORD dwVCLK, DWORD dwScaleX )
{
	

	  return (dwVCLK  * (dwScaleX  >> 16) + ((dwVCLK  * (dwScaleX & 0xFFFF)) >> 16));

}


DWORD DesktopBandWidth( LPBW_DESKTOP lpDesktop)
{
   DWORD dwBW;

    dwBW = BandWidthForScale(lpDesktop->dwVCLK, lpDesktop->dwScaleX) * lpDesktop->dwByteCount ;

    if( (lpDesktop->dwFlags & BW_THREETAP) & (lpDesktop->dwFlags & BW_UNDITHER))
       dwBW <<= 2;
    else if(lpDesktop->dwFlags & BW_THREETAP)
      dwBW *= 3;
    if( lpDesktop->dwFlags & BW_UNDITHER)
       dwBW <<= 1;

    return dwBW;
}


DWORD OverlayBandWidth(DWORD dwBW,LPBW_OVERLAY lpOverlay)
{
  DWORD dwScaleY = lpOverlay->dwScaleY;
  DWORD dwBurstFactor = 1;

  dwBW = dwBW * lpOverlay->dwByteCount ;

  if( dwScaleY == 0x10000)
  {
      if((lpOverlay->dwFlags & BW_UNDITHER) && (lpOverlay->dwFlags & BW_THREETAP) )
		   dwBurstFactor = 4;
      else  if(lpOverlay->dwFlags & BW_THREETAP)
           dwBurstFactor = 3;
      else if(lpOverlay->dwFlags & (BW_UNDITHER | BW_TWOTAP))
            dwBurstFactor = 2;

      if(lpOverlay->dwFlags & BW_VIDEOPORT)
		  dwBurstFactor <<= 1;

  }
  else if( dwScaleY > 0x10000 )
  {
         //zoom up Y case
       dwBurstFactor = 2;

       if(lpOverlay->dwFlags & (BW_VIDEOPORT |BW_TWOTAP | BW_THREETAP))
	        dwBurstFactor <<= 1;
  }
  else
 	  dwBurstFactor = 4 ;          //scale down 4 tab filter

  if( dwBurstFactor > 4)
		dwBurstFactor = 4;

/*	if( lpOverlay->dwFlags & BW_3DRGB )
	{
		if(dwBurstFactor >= 3 )
			dwBurstFactor = (dwBurstFactor -1 ) << 1;
		else 
			dwBurstFactor <<= 1;
	} */

   return dwBW * dwBurstFactor;
}

/***********************************************************************
* 1. The max BW for each overlay is Grx_CLK * 2 pixel /sec
* 2. Undither filter is only set for RGB5:6:5 and no Zoom nor scale case
*
*
************************************************************************/

BOOL EnoughBandWidth( LPBW_CONFIG lpBWConfig)
{

  DWORD dwThisBandWidth ;
  DWORD dwCurrentBandWidth = 0;

  //First find the total band width for desktop
  dwThisBandWidth = DesktopBandWidth( &(lpBWConfig->sDesktop));

  DPF("BW for PD = %ld ", dwThisBandWidth);
  if( dwThisBandWidth > 
      lpBWConfig->dwMCLK * lpBWConfig->sDesktop.dwByteCount * 2 )
  {	
	  DPF("FALSE in PD /n");     
	  return FALSE;       //too much desktop BW
  }	
  dwCurrentBandWidth = dwThisBandWidth;


  if( lpBWConfig->sOverlay.dwFlags & OVL_ON )
  {
        
     dwThisBandWidth = BandWidthForScale(lpBWConfig->sDesktop.dwVCLK ,
                       	  lpBWConfig->sOverlay.dwScaleX);
     if( dwThisBandWidth > lpBWConfig->dwMCLK * 2 )
     {	
           
   		  DPF("FALSE in PV BW= %ld \n",dwThisBandWidth);	
   		  return FALSE;       //too much overlay BW
     }

     dwThisBandWidth = OverlayBandWidth( dwThisBandWidth,
                                    &(lpBWConfig->sOverlay));
     DPF("BW for PV = %ld ", dwThisBandWidth);

     dwCurrentBandWidth += dwThisBandWidth;
  }

  if( lpBWConfig->sVideoPort.dwFlags & VPE_ON )
  {
     if(!lpBWConfig->sVideoPort.dwTransferRate)
        // PAL-SQP worst case pixel rate in KHz
          lpBWConfig->sVideoPort.dwTransferRate = 14750l;

      if( lpBWConfig->sVideoPort.dwScale == 0x1000)		//0x1000 is 1 for video port
         dwCurrentBandWidth +=
             lpBWConfig->sVideoPort.dwTransferRate * 2 * 4 ;
       else
         dwCurrentBandWidth +=
             lpBWConfig->sVideoPort.dwTransferRate * 2 * 9;
   }

  DPF("Total BW = %ld ", dwCurrentBandWidth);
  
  if(dwCurrentBandWidth > ( lpBWConfig->dwMCLK << 5 ) )
     return FALSE;

  return TRUE;

}
