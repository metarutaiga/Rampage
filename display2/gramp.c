/* $Header: gramp.c, 3, 7/17/00 4:44:42 PM PDT, Dan O'Connel$ */
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
** File name:   gramp.c
**
** Description: Support functions for gamma ramp.
**
** $Log: 
**  3    3dfx      1.2         7/17/00  Dan O'Connel    Major changes ported 
**       from Napalm driver to support: Registry Controlled Modes, DFP, TvOut, 
**       read OEM config from BIOS, updated 3dfx Tools support, and other 
**       features and bug fixes.
**  2    3dfx      1.1         12/22/99 Ryan Bissell    New clut management code
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
**
*/

#include "header.h"
#include "gramp.h"
#include "modelist.h"

#define Not_VxD
#include "minivdd.h"

extern DWORD dwDeviceHandle;


void init_desktopgamma()
{
  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                                            H3VDD_CLUT_INITIALIZE,
                                            0,
                                            NULL);

  get_desktopgamma(0, 256, _FF(GammaTable));
}



int set_desktopgamma(int start, int count, FxU32 FAR* lpRRGGBB)
{
  FxU32 param;

  param = (((FxU32)start & 0x0000FFFF) << 16) | ((FxU32)count & 0x0000FFFF);
  return !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                                                    H3VDD_CLUT_SETDESKTOP,
                                                    param,
                                                    lpRRGGBB);
}


int set_overlaygamma(int start, int count, FxU32 FAR* lpRRGGBB)
{
  FxU32 param;

  param = (((FxU32)start & 0x0000FFFF) << 16) | ((FxU32)count & 0x0000FFFF);
  return !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                                                    H3VDD_CLUT_SETOVERLAY,
                                                    param,
                                                    lpRRGGBB);
}


int get_desktopgamma(int start, int count, FxU32 FAR* lpRRGGBB)
{
  FxU32 param;

  param = (((FxU32)start & 0x0000FFFF) << 16) | ((FxU32)count & 0x0000FFFF);
  return !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                                                    H3VDD_CLUT_GETDESKTOP,
                                                    param,
                                                    lpRRGGBB);
}


int get_overlaygamma(int start, int count, FxU32 FAR* lpRRGGBB)
{
  FxU32 param;

  param = (((FxU32)start & 0x0000FFFF) << 16) | ((FxU32)count & 0x0000FFFF);
  return !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                                                    H3VDD_CLUT_GETOVERLAY,
                                                    param,
                                                    lpRRGGBB);
}



/*----------------------------------------------------------------------
Function name:  DDIGammaRamp

Description:    Read or write hardware gamma ramp.
                
Information:
    Read or write hardware gamma ramp. This entry is ordinal 32
    in display.def.

Return:         BOOL    1 = success
                        0 = failure
----------------------------------------------------------------------*/
BOOL FAR PASCAL _loadds OEMGammaRamp(LPDIBENGINE lpPDevice,
                                     WORD wSetNotGet,
                                     LPDDGAMMARAMP lpGamma)
{
  int i;
  FxU32 gamma[256];

  //if not 16bpp or more, fail (gamma handled differently at 8bpp)
  if ( _FF(ddPrimarySurfaceData.dwBytesPerPixel) < 2 )
    return 0;

  if (wSetNotGet)
  {
    //windows seems to get red confused with blue
    //when it uses this interface... so swap 'em.
    for (i=0; i<256; i++)
    {
      gamma[i]  = ((FxU32)(lpGamma->red[i]   & 0xFF00) >> 8) << SST_VO_PO_LUT256_BLU_SHIFT;
      gamma[i] |= ((FxU32)(lpGamma->blue[i]  & 0xFF00) >> 8) << SST_VO_PO_LUT256_RED_SHIFT;
      gamma[i] |= ((FxU32)(lpGamma->green[i] & 0xFF00) >> 8) << SST_VO_PO_LUT256_GRN_SHIFT;
    }

    return set_desktopgamma(0, 256, gamma);
  }
  else
  {
    if (!get_desktopgamma(0, 256, gamma))
      return 0;

    //windows seems to get red confused with blue
    //when it uses this interface... so swap 'em.
    for (i=0; i<256; i++)
    {
      lpGamma->red[i]   = ((gamma[i] >> SST_VO_PO_LUT256_BLU_SHIFT) & 0xFF) << 8;
      lpGamma->blue[i]  = ((gamma[i] >> SST_VO_PO_LUT256_RED_SHIFT) & 0xFF) << 8;
      lpGamma->green[i] = ((gamma[i] >> SST_VO_PO_LUT256_GRN_SHIFT) & 0xFF) << 8;
    }
  }
}


