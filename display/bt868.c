/* -*-c++-*- */
/* $Header: bt868.c, 1, 9/11/99 8:36:00 PM PDT, StarTeam VTS Administrator$ */
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
** File name:   bt868.c
**
** Description: BrookTree 868 TV Out support functions.
**
** $Revision: 1$
** $Date: 9/11/99 8:36:00 PM PDT$
**
** $History: bt868.c $
** 
** *****************  Version 2  *****************
** User: Mconrad      Date: 9/02/99    Time: 5:27p
** Updated in $/devel/sst2/Win95/dx/hostvdd
** Cleanup warnings on missing control paths.
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:52p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 26  *****************
** User: Stuartb      Date: 3/23/99    Time: 11:41a
** Updated in $/devel/h3/Win95/dx/minivdd
** Noted (fixed) problem in bt868_fixupNonStdModes where reading past end
** of array.
** 
** *****************  Version 25  *****************
** User: Stuartb      Date: 3/19/99    Time: 3:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added BT868_FixupVGA to assure we make the hires to VGA transition
** properly.  Cleaned up BT868_GetStatus so there is less video
** disturbance on call.  Added comments.
** 
** *****************  Version 24  *****************
** User: Stuartb      Date: 3/12/99    Time: 8:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Route bt868 DACs for composite out if so directed by BIOS.
** 
** *****************  Version 8  *****************
** User: Stuartb      Date: 3/08/99    Time: 9:53a
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** Do not restore bt868 autoconfig mode from registry.  Mode is now
** determined from tvstd as saved in NVRAM or established by BIOS.
** 
** *****************  Version 7  *****************
** User: Stuartb      Date: 3/07/99    Time: 12:05p
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** When changing major modes or tvstd, mark all BT868_RegisterShadow
** invalid.
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 3/03/99    Time: 5:04p
** Updated in $/Releases/Voodoo3/MT2/3Dfx/devel/H3/Win95/DX/minivdd
** leave DACs live to make it easier to reenable at shutdown time!
** 
** *****************  Version 20  *****************
** User: Stuartb      Date: 3/01/99    Time: 2:39p
** Updated in $/devel/h3/Win95/dx/minivdd
** Turn off genlock while downloading overscan registers.  Remove
** references to TV_STANDARD_MAJOR....
** 
** *****************  Version 19  *****************
** User: Stuartb      Date: 2/25/99    Time: 10:53a
** Updated in $/devel/h3/Win95/dx/minivdd
** Save tvstd in nvram if STB reference board.
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 2/23/99    Time: 2:36p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to access serial NVRAM if present.  Disable DAC A if H4 as
** it is not used and is shorted to gnd.
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 2/19/99    Time: 11:17a
** Updated in $/devel/h3/Win95/dx/minivdd
** If IS_H4, swap Y/C outputs.
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 2/18/99    Time: 11:42a
** Updated in $/devel/h3/Win95/dx/minivdd
** More work on PAL modes.  Turn off tv out DACs when disabled.
** 
** *****************  Version 15  *****************
** User: Stuartb      Date: 2/13/99    Time: 6:42p
** Updated in $/devel/h3/Win95/dx/minivdd
** First cut at PAL_M, PAL_N, PAL_NC support in tvout.
** 
** *****************  Version 14  *****************
** User: Stuartb      Date: 2/08/99    Time: 8:58a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changes to fix lcd boot and simultaneous VMI & TV/LCD.
** 
** *****************  Version 13  *****************
** User: Agus         Date: 2/01/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move IS_H4 macro define to h3vdd.h
** 
** *****************  Version 12  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:07p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/04/99    Time: 1:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
*/

/*
 * bt868.c - BrookTree TV encoder driver
 */

#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "tv.h"
#include "devtable.h"
#undef  VDDONLY
#include "tvoutdef.h"
#include "bt868.h"
#include "i2c.h"
#include "sagelcd.h"
#include "time.h"

#undef IS_3DFX_REF_BOARD
#define IS_3DFX_REF_BOARD(pDev)  ((pDev->dwSubSystemID & 0xff0000) < 0x00300000)


extern const struct i2cmask I2C_PROTO;
extern const DWORD I2C_INITVAL;

BT868_Regs Bt868_RegShadow;

// Bt868 default (autoconfig) PLL settings, 8.16 numbers
// fclk = (13500000 * pllint.pllfract) / 6  (36.0 = (13.5 * 16) / 6)
FxU32 AutoCfgFclk[] = {0x0c880e, 0x0d1c72, 0x113b14, 0x100000};

// top 16 bits of HposNormal is actually vscale 13:8 for the selected mode
FxU32 HposNormal[][4] = {{0x140175, 0x1001c4, 0x1c024d, 0x160220},
                         {0x140155, 0x1001b0, 0x1c024d, 0x1601f0}};
const FxU8 ClkDly[][4] = {{0x04, 0x04, 0x1c, 0x04},
                          {0x10, 0x10, 0x10, 0x10}};
const FxU8 HExtentNormal[][4] = {{0x5c, 0xe8, 0x18, 0x68},
                                 {0x5c, 0xe8, 0x00, 0x68}};

const FxU8 VposNormal[] = {75, 104, 88, 95};
const FxI8 HPosScale[] = {50, 50, 50, 50};
const FxI8 VPosScale[] = {50, 50, 50, 50};
const FxI8 SIZE_XLATE[] = {0, 1, -1, 2, 3};


#define OSCAN_STEPS   1
#if OSCAN_STEPS

typedef struct
{
	FxI8 hPosOfst;
	FxI8 vPosOfst;
	FxU8 regVals[32];
}  BT868_SUBMODE;

/*
These are overscan registers 0x76 through 0xb4 inclusive.  They are arranged as:
[MAJOR_MODE][OVERSCAN_SETTING].
*/
const BT868_SUBMODE Overscan[4][4] = {
// major mode 0, 640x480 NTSC
{{ -24,  -24, 0x80, 0x80, 0x7c, 0x8a, 0x50, 0x35, 0x18, 0xe9, 0x26, 0x00, 0x20,
              0x8c, 0x03, 0x22, 0x2f, 0xe0, 0x36, 0x48, 0x51, 0xe9, 0xa2, 0x0b,
			  0x0a, 0xe5, 0x76, 0x79, 0x44, 0x85, 0x00, 0x00, 0x00, 0x23},
 {   0,  -16, 0xc0, 0x80, 0x80, 0x90, 0x58, 0x59, 0x1c, 0xe0, 0x26, 0x00, 0x20,
              0x8c, 0x03, 0x37, 0x3a, 0xe0, 0x36, 0x8f, 0x52, 0x7b, 0x15, 0x0c,
			  0x0a, 0xe5, 0x76, 0x79, 0x44, 0x85, 0xed, 0x25, 0xb4, 0x21},
 {  16,   12, 0x40, 0x80, 0x8a, 0x9a, 0x68, 0xa1, 0x24, 0xd1, 0x27, 0x00, 0x20,
              0x8c, 0x03, 0x61, 0x51, 0xe0, 0x36, 0x1f, 0x55, 0xa1, 0xfa, 0x0c,
			  0x0a, 0xe5, 0x75, 0x79, 0x44, 0x85, 0x7c, 0x1a, 0x61, 0x1f},
 {  48,   32, 0xc0, 0x80, 0x92, 0xa6, 0x78, 0xe9, 0x2b, 0xc3, 0x27, 0x00, 0x20,
              0x8c, 0x03, 0x8b, 0x68, 0xe0, 0x36, 0xae, 0x57, 0xc7, 0xdf, 0x0d,
			  0x0a, 0xe5, 0x75, 0x78, 0x44, 0x85, 0xb6, 0xd6, 0x5a, 0x1d}},

// major mode 1, 640x480 PAL
{{ -64,  -32, 0x90, 0x80, 0x7c, 0x94, 0x4e, 0x4b, 0x17, 0x20, 0xa6, 0x00, 0xe8, 
              0xff, 0x03, 0x0d, 0x24, 0xe0, 0x36, 0xe1, 0x4a, 0xab, 0xaa, 0x0b, 
              0x24, 0xf0, 0x59, 0x83, 0x49, 0x8c, 0xca, 0x03, 0x3d, 0x2b},
 { -40,  -48, 0xc0, 0x80, 0x7e, 0x98, 0x54, 0x65, 0x1b, 0x18, 0xa6, 0x00, 0xe8, 
              0x12, 0x0b, 0x1c, 0x2c, 0xe0, 0x36, 0xa6, 0x4b, 0x00, 0x00, 0x0c, 
              0x24, 0xf0, 0x59, 0x82, 0x49, 0x8c, 0xcb, 0x8a, 0x09, 0x2a},
 {0000, 0000, 0x80, 0x80, 0x8e, 0xa8, 0x6c, 0xd3, 0x2e, 0xf2, 0x27, 0x00, 0xc0, 
              0x0a, 0x0b, 0x71, 0x5a, 0xe0, 0x36, 0x00, 0x50, 0x55, 0x55, 0x0d, 
              0x24, 0xf0, 0x58, 0x81, 0x49, 0x8c, 0x50, 0x63, 0xd5, 0x25},
 {  20, 0000, 0xb0, 0x80, 0x90, 0xac, 0x72, 0xef, 0x2e, 0xf2, 0x27, 0x00, 0xd8, 
              0x0a, 0x0b, 0x71, 0x5a, 0xe0, 0x36, 0x00, 0x50, 0xab, 0xaa, 0x0d, 
              0x24, 0xf0, 0x58, 0x81, 0x49, 0x8c, 0xb2, 0x28, 0xe9, 0x24}},

// major mode 2, 800x600 NTSC
{{ -88,  -32, 0x90, 0x20, 0xa2, 0xb6, 0x92, 0xbd, 0x18, 0xe8, 0x38, 0x00, 0x48, 
              0x1e, 0x03, 0xad, 0x3c, 0x58, 0x3a, 0xc1, 0x59, 0x24, 0x54, 0x0f, 
              0x0a, 0xe5, 0x75, 0x78, 0x43, 0x85, 0x78, 0xc0, 0x91, 0x1a},
 { -64,  000, 0xf0, 0x20, 0xaa, 0xbe, 0x9e, 0xf3, 0x1d, 0xde, 0x38, 0x00, 0x48, 
              0x1e, 0x03, 0xcb, 0x4c, 0x58, 0x3a, 0x95, 0x5b, 0x00, 0x00, 0x10, 
              0x0a, 0xe5, 0x74, 0x78, 0x43, 0x85, 0x17, 0x5d, 0x74, 0x19},
 {  32, 0000, 0x00, 0x20, 0xbe, 0xd6, 0xc2, 0x8b, 0x22, 0xd4, 0x3a, 0x00, 0x80, 
              0x52, 0x03, 0xee, 0x5e, 0x58, 0x3a, 0xb7, 0x9d, 0xf0, 0xe6, 0x11, 
              0x0a, 0xe5, 0x74, 0x77, 0x43, 0x85, 0x00, 0x00, 0xc0, 0x16},
 {  64, 0000, 0x80, 0x20, 0xc6, 0xe0, 0xd2, 0xd3, 0x20, 0xd8, 0x3a, 0x00, 0xc0,
              0x40, 0x03, 0xdf, 0x56, 0x58, 0x3a, 0xcd, 0x9c, 0x15, 0xcc, 0x12, 
              0x0a, 0xe5, 0x74, 0x77, 0x43, 0x85, 0xab, 0xaa, 0xaa, 0x15}},

// major mode 3, 800x600 PAL
{{ -48,  -24, 0x50, 0x20, 0x9c, 0xba, 0x86, 0xa9, 0x19, 0x1c, 0xb8, 0x00, 0xe8,
			  0x90, 0x03, 0x99, 0x33, 0x58, 0x3a, 0x0c, 0x52, 0x1c, 0xc7, 0x0e,
			  0x24, 0xf0, 0x57, 0x80, 0x48, 0x8c, 0x26, 0xb2, 0x22, 0x22},
 { -32,  -24, 0x90, 0x20, 0xa0, 0xc0, 0x8c, 0xcd, 0x1d, 0x14, 0xb8, 0x00, 0xe8,
              0x90, 0x03, 0xad, 0x3d, 0x58, 0x3a, 0x12, 0x53, 0xe4, 0x38, 0x0f,
			  0x24, 0xf0, 0x57, 0x80, 0x48, 0x8c, 0x2e, 0x8d, 0x23, 0x21},
 {  24,    0, 0x60, 0x20, 0xb0, 0xd2, 0xa6, 0x45, 0x29, 0xfc, 0x39, 0x00, 0xe8, 
			  0x90, 0x03, 0xee, 0x5f, 0x58, 0x3a, 0x66, 0x96, 0xab, 0xaa, 0x10, 
			  0x24, 0xf0, 0x57, 0x80, 0x48, 0x8c, 0x74, 0x4f, 0x44, 0x1e},
 {  60,   20, 0xd0, 0x20, 0xb8, 0xdc, 0xb4, 0x83, 0x2f, 0xf1, 0x39, 0x00, 0xe8,
              0x90, 0x03, 0x11, 0x73, 0x58, 0x3b, 0x31, 0x98, 0xc7, 0x71, 0x11,
			  0x24, 0xf0, 0x57, 0x7f, 0x48, 0x8c, 0x23, 0xd8, 0xea, 0x1c}}};
#endif

enum
{
	NTSC_OFF,
	NTSC_ON,
	PAL_OFF,
	PAL_ON
};

FxU8 MacroVisionRegsPAL[][18] = {
{0x05, 0x57, 0x20, 0x40, 0x6e, 0x7e, 0xf4, 0x51, 0x0f,
 0xf1, 0x05, 0xd3, 0x78, 0xa2, 0x25, 0x54, 0xa5, 0x00},
{0x05, 0x57, 0x20, 0x40, 0x6e, 0x7e, 0xf4, 0x51, 0x0f,
 0xf1, 0x05, 0xd3, 0x78, 0xa2, 0x25, 0x54, 0xa5, 0x63}};
FxU8 MacroVisionRegsNTSC[][18] = {
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // OFF
 0xf3, 0x09, 0xbd, 0x67, 0xb5, 0x90, 0xb2, 0x7d, 0x00},
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // no color stripe
 0xf3, 0x09, 0xbd, 0x67, 0xb5, 0x90, 0xb2, 0x7d, 0x63},
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // 2 line color stripe
 0xf3, 0x09, 0xbd, 0x6c, 0x31, 0x92, 0x32, 0xdd, 0xe3},
{0x0f, 0xfc, 0x20, 0xd0, 0x6f, 0x0f, 0x00, 0x00, 0x0c,   // 4 line color stripe
 0xf3, 0x09, 0xbd, 0x66, 0xb5, 0x90, 0xb2, 0x7d, 0xe3}};


#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG


/*----------------------------------------------------------------------
Function name:  vpTvStdToBiosStd

Description:    given VP_TV_STANDARD_THING (from win98 ddk) return BIOS_XX value

Information:    PAL GDHIB are all considered 'generic' PAL.

Return:         DWORD
----------------------------------------------------------------------*/
DWORD vpTvStdToBiosStd (int vpTvStd)
{
	DWORD dwPal = VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_G | VP_TV_STANDARD_PAL_D | VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;

	if (vpTvStd & VP_TV_STANDARD_PAL_M)
		return (BIOS_PAL_M);
	else if (vpTvStd & VP_TV_STANDARD_PAL_N)
		return (BIOS_PAL_N);
	else if (vpTvStd & VP_TV_STANDARD_PAL_NC)
		return (BIOS_PAL_Nc);
	else if (vpTvStd & dwPal)
		return (BIOS_PAL);
	else
		return (BIOS_NTSC);
}

#if 0
/*----------------------------------------------------------------------
Function name:  vpTvStdToNvStd

Description:    given VP_TV_STANDARD_THING (from win98 ddk) return nvram value

Information:    PAL GDHIB are all considered 'generic' PAL.

Return:         DWORD
----------------------------------------------------------------------*/
DWORD vpTvStdToNvStd (int vpTvStd)
{
	DWORD dwPal = VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_G | VP_TV_STANDARD_PAL_D | VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;

	if (vpTvStd & VP_TV_STANDARD_PAL_M)
		return (3);
	else if (vpTvStd & VP_TV_STANDARD_PAL_N)
		return (4);
	else if (vpTvStd & VP_TV_STANDARD_PAL_NC)
		return (5);
	else if (vpTvStd & dwPal)
		return (2);
	else
		return (1);
}
#endif

/*----------------------------------------------------------------------
Function name:  getTvStdInfo

Description:    given BIOS_XX tvStd return bt868 auto-config mode and 
                VP_TV_STANDARD_THING

Information:    PAL GDHIB are all considered 'generic' PAL.

Return:         void
----------------------------------------------------------------------*/
void getTvStdInfo (PDEVTABLE pDev, FxU32 biosStd, FxU32 *autoCfg, FxU32 *vpTvStd)
{
	SstIORegs *sstIOregs = (SstIORegs *)pDev->RegBase;
    DWORD xres = sstIOregs->vidScreenSize & 0xfff;
	DWORD dwPal = VP_TV_STANDARD_PAL_B | VP_TV_STANDARD_PAL_G | VP_TV_STANDARD_PAL_D | VP_TV_STANDARD_PAL_H | VP_TV_STANDARD_PAL_I;


	if (autoCfg)
	{
		switch (biosStd)
		{
			case BIOS_NTSC:
			case BIOS_PAL_M:
				*autoCfg = xres == 800 ? 2 : 0;
				break;
			case BIOS_PAL:
			case BIOS_PAL_N:
			case BIOS_PAL_Nc:
				*autoCfg = xres == 800 ? 3 : 1;
		}
	}
	if (vpTvStd)
	{
		if (biosStd == BIOS_PAL_M)
			*vpTvStd = VP_TV_STANDARD_PAL_M;
		else if (biosStd == BIOS_PAL_N)
			*vpTvStd = VP_TV_STANDARD_PAL_N;
		else if (biosStd == BIOS_PAL_Nc)
			*vpTvStd = VP_TV_STANDARD_PAL_NC;
		else if (biosStd == BIOS_PAL)
			*vpTvStd = dwPal;
		else
			*vpTvStd = VP_TV_STANDARD_NTSC_M;
	}
}


/*----------------------------------------------------------------------
Function name:  i2c_write

Description:    Perform an i2c register write.

Information:

Return:         INT     The result of the write.
----------------------------------------------------------------------*/
static int i2c_write (I2CMASK *i2c_mask, int reg, int data, DWORD RegBase)
{
	if (RegBase)
	{
		*i2c_mask = I2C_PROTO;           //Setup the I2C bitmasks
		i2c_mask->pReg = (PDWORD)(RegBase + I2COUT_PORT);
		*i2c_mask->pReg = I2C_INITVAL;
		i2c_mask->bAddr = BT868_ADDR;
		i2c_mask->nSize = 1;
	}
	i2c_mask->nReg = reg;
	i2c_mask->dwData = data;
	if (reg >= 0x76 && reg < 0xd8)
		((char *)&Bt868_RegShadow)[(reg - 0x76) / 2] = data;
	return (WriteI2CRegister (i2c_mask));
}

/*----------------------------------------------------------------------
Function name:  BT868_GetStatus

Description:    Return the status of the BT868.

Information:

Return:         INT     The status word or,
                        -1 for failure.
----------------------------------------------------------------------*/
int BT868_GetStatus (PDEVTABLE pDev, int estatus)
{
   I2CMASK I2CMask;
   TVOUT_CURSETUP *btSetup = (void *)&pDev->tvOutData;
   FxU8 readData;
   FxU32 vidInFmt = ((SstIORegs * )pDev->RegBase)->vidInFormat;

   ((SstIORegs * )pDev->RegBase)->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE;
 
   I2CMask = I2C_PROTO;             // Setup the I2C bitmasks
   I2CMask.pReg = (PDWORD)(pDev->RegBase + I2COUT_PORT);
   I2CInit(&I2CMask);               // Enable I2C
   I2CMask.bAddr = BT868_ADDR;
   I2CMask.nSize = 1;
   stop(&I2CMask);                  // Reset devices

   if (estatus == 1)
   {
	   I2CMask.nReg = 0xba;             //Bt868 reg 0xba
	   I2CMask.dwData = 0x40 | (!btSetup->is_master * 0x20);  // Set check_stat
	   if (IS_H4(pDev))
		   I2CMask.dwData |= 1;     // disable unused DAC A

	   if (FXFALSE == WriteI2CRegister (&I2CMask))
	   {
		   ((SstIORegs * )pDev->RegBase)->vidInFormat = vidInFmt;
		   return (-1);
	   }
   }

   I2CMask.nReg = 0xc4;             //Bt868 reg 0xc4
   I2CMask.dwData = ((estatus << 6) | btSetup->is_master);   // Set estatus
   if (FXFALSE == WriteI2CRegister (&I2CMask))
   {
	   ((SstIORegs * )pDev->RegBase)->vidInFormat = vidInFmt;
	   return (-1);
   }
   if (FXFALSE == ReadI2CRegister(&I2CMask))                 // readback
   {
	   ((SstIORegs * )pDev->RegBase)->vidInFormat = vidInFmt;
	   return (-1);
   }
   readData = (FxU8)I2CMask.dwData;

   if (estatus == 1)
   {
	   I2CMask.nReg = 0xba;             //Bt868 reg 0xba
	   I2CMask.dwData = !btSetup->is_master * 0x20;         // unset check_stat
	   if (IS_H4(pDev))
		   I2CMask.dwData |= 1;     // disable unused DAC A
	   if (FXFALSE == WriteI2CRegister (&I2CMask))
	   {
		   ((SstIORegs * )pDev->RegBase)->vidInFormat = vidInFmt;
		   return (-1);
	   }
   }

   ((SstIORegs * )pDev->RegBase)->vidInFormat = vidInFmt;
   return (readData);
}


/*----------------------------------------------------------------------
Function name:  bt868_position

Description:    Handle x,y positioning.

Information:
    This is awful!  There's no reasonable bt868 handle for
    vertical positioning.  After all, it doesn't have a field
    store and it can't show us the data before it arrives!  So
    we will use the CRT timing totals to move vertical.  The 
    horizontal is controlled via bt868 regs (9a:80).

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
static int bt868_position (PDEVTABLE pDev, int xpos, int ypos)
{
    I2CMASK I2CMask;
    TVOUT_CURSETUP *tvOutData = (void *)&pDev->tvOutData;
	SstIORegs *sstIOregs = (SstIORegs *)pDev->RegBase;
    DWORD yres = (sstIOregs->vidScreenSize >> 12) & 0xfff;
    DWORD xres = sstIOregs->vidScreenSize & 0xfff;
    FxU8 mode = tvOutData->mode;
	BT868_SUBMODE const *oscan = 0;
    FxU8 v_scale = (FxU8)(HposNormal[IS_H4(pDev)][mode] >> 16);

	if (OSCAN_STEPS && SIZE_XLATE[tvOutData->size] >= 0)
		oscan = &Overscan[mode][SIZE_XLATE[tvOutData->size]];
    if (xpos >= 0 && xpos <= 100)
    {
        tvOutData->hpos = xpos;
		xpos -= 50;     // make bipolar
        xpos = HposNormal[IS_H4(pDev)][mode] + ((xpos * HPosScale[mode]) / 50);
        if (oscan)
        {
            xpos += (oscan->hPosOfst * 2);
            v_scale = oscan->regVals[18] & 0x3f;
        }
        i2c_write (&I2CMask, 0x80, xpos & 0xff, pDev->RegBase);
        i2c_write (&I2CMask, 0x9a, v_scale | ((xpos & 0x300) >> 2), 0);
    }
    if (ypos >= 0 && ypos <= 100)
    {
        tvOutData->vpos = ypos;
        ypos -= 50;     // make bipolar
        ypos = VposNormal[mode] + ((ypos * VPosScale[mode]) / 50);
        if (oscan)
            ypos += oscan->vPosOfst;
		if (IS_H4(pDev))
			*(DWORD *)(pDev->RegBase + 0x3c) = ((ypos + yres) << 16) | ypos;
		else
			sstIOregs->vidInYDecimDeltas = ((ypos + yres) << 16) | ypos;
    }

    xpos = HExtentNormal[IS_H4(pDev)][tvOutData->mode];
	if (IS_H4(pDev))
		*(DWORD *)(pDev->RegBase + 0x74) = ((xpos + xres) << 16) | xpos;
	else
		sstIOregs->vidInXDecimDeltas = ((xpos + xres) << 16) | xpos;
    return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPosition

Description:    Get current horizontal/vertical position.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPosition (PTVCURPOS pTvCurPos, void *tvOutData)
{
	pTvCurPos->dwCurLeft = ((TVOUT_CURSETUP *)tvOutData)->hpos;
	pTvCurPos->dwCurTop = ((TVOUT_CURSETUP *)tvOutData)->vpos;
	pTvCurPos->dwCurRight = 0;
	pTvCurPos->dwCurBottom = 0;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetPosition

Description:    Set position.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_SetPosition(PTVSETPOS pTvSetPosition, PDEVTABLE pDev)
{
	bt868_position (pDev, pTvSetPosition->dwLeft, pTvSetPosition->dwTop);
	return(0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetStandard

Description:    Set TV type.

Information:

Return:         INT     retult of the BT868_Enable call.
----------------------------------------------------------------------*/
int BT868_SetStandard (PTVSETSTANDARD pTvSetStandard, PDEVTABLE pDev)
{
	FxU32 mode;

	mode = vpTvStdToBiosStd (pTvSetStandard->dwStandard);
    ((TVOUT_CURSETUP *)&pDev->tvOutData)->tvStd = mode;
	getTvStdInfo (pDev, mode, &mode, 0);
    ((TVOUT_CURSETUP *)&pDev->tvOutData)->mode = mode;
	return (BT868_Enable (pDev, -1));
}


/*----------------------------------------------------------------------
Function name:  mul64

Description:    multiply a u32 by a u32 and return an __int64

Information:

Return:         __int64
----------------------------------------------------------------------*/
__int64 mul64 (FxU32 prodA, FxU32 prodB)
{
    _asm
	{
		mov eax,[prodA]
		mul [prodB]
	}
}

/*----------------------------------------------------------------------
Function name:  div64

Description:    divide a __int64 by a u32 and return an u32

Information:

Return:         __int64
----------------------------------------------------------------------*/
FxU32 div64 (FxU32 *dividend, FxU32 divisor)
{
	FxU32 retval;

	_asm
	{
		mov edx,[dividend]
		mov eax,[edx]
		mov edx,[edx+4]
		div [divisor]
		mov [retval],eax
	}
	return (retval);
}

/*----------------------------------------------------------------------
Function name:  bt868_fixupNonStdModes

Description:    for little used PAL M, Nc and N fixup subcarrier, etc

Information:

Return:         nadda
----------------------------------------------------------------------*/
void bt868_fixupNonStdModes (PDEVTABLE pDev, int extMode)
{
	TVOUT_CURSETUP *btSetup = (void *)&pDev->tvOutData;
	FxU32 subc = (btSetup->mode & 1) ? 4433619 : 3579545;
	I2CMASK I2CMask;
	FxU32 clk, i, reg;
	__int64 uli;
	FxU8 palSetup[] = {0xf0, 0x57, 0x80, 0x48, 0x8c};

	switch (btSetup->tvStd)
	{
		case BIOS_PAL_M:
			subc = 3575611;
			i2c_write (&I2CMask, 0xa2, 0x2a, pDev->RegBase);
			break;
		case BIOS_PAL_N:
			subc = 4433619;
			i2c_write (&I2CMask, 0xa2, 0x2e, pDev->RegBase);
			break;
		case BIOS_PAL_Nc:
			subc = 3582056;
			i2c_write (&I2CMask, 0xa2, 0x24, pDev->RegBase);
			break;
		default:        // no need to do anything
			return;
//			i2c_write (&I2CMask, 0xa2, 0x24, pDev->RegBase);
	}

	// setup sync_amp, burst_amp, mcr-y, mcb-y and mcy for PAL

	for (reg = 0xa4, i = 0; reg <= 0xac; reg += 2)
		i2c_write (&I2CMask, reg, palSetup[i++], 0);

	// subcarrier processing

	// master clk = (13500000 * PLL_INT.PLL_FRACT) / (65536 * 6)
	clk = (Bt868_RegShadow.regA0.ucPLL_INT << 16)  | 
       	  (Bt868_RegShadow.reg9E.ucPLL_FRACT << 8) |
		  (Bt868_RegShadow.reg9C.ucPLL_FRACT);
	uli = mul64 (clk, 13500000);
	clk = div64 ((void *)&uli, 65536 * 6);
	// subc incr = (subcarrier_freq * 0x100000000) / master_clk
	uli = mul64 (0xffffffff, subc);
	clk = div64 ((void *)&uli, clk);

	i2c_write (&I2CMask, 0xae, clk, 0);        // subc mpx lsb
	i2c_write (&I2CMask, 0xb0, clk >> 8, 0);
	i2c_write (&I2CMask, 0xb2, clk >> 16, 0);
	i2c_write (&I2CMask, 0xb4, clk >> 24, 0);  // subc mpx msb
}


/*----------------------------------------------------------------------
Function name:  BT868_Enable

Description:    Enable the TV device.

Information:

Return:         INT     FXTRUE if success,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
int BT868_Enable (PDEVTABLE pDev, int vgaMode)
{
	I2CMASK I2CMask;
	TVOUT_CURSETUP *btSetup = (void *)&pDev->tvOutData;
	DWORD RegBase = pDev->RegBase;
	PDWORD pdwVidInFormat       = (PDWORD)(RegBase + VID_IN_FORMAT);
	DWORD autoCfg, i;
	volatile FxU32 *pdwTvClkDly = IS_H4(pDev) ? &((SstIORegs *)RegBase)->tmuGbeInit
                                     : &((SstIORegs *)RegBase)->miscInit1;
    TVSETCAP TvSetPicControl;
	DWORD xres = ((SstIORegs *)RegBase)->vidScreenSize & 0xfff;
	TVSETSIZE tvSize;
	//Setup Banshee as slave for TV-Out operation mode
	DWORD dwVidInFormat = SST_VIDEOIN_VSYNC_POLARITY_LOW |
						  SST_VIDEOIN_HSYNC_POLARITY_LOW |
						  //SST_VIDEOIN_G4_FOR_POSEDGE   |
						  SST_VIDEOIN_GENLOCK_ENABLE     |
						  SST_VIDEOIN_NOT_USE_VGA_TIMING |
						  H3_VMI_MODE_TV                 |
						  (1 << 18)                      |   // for H4
						  0;

	*(DWORD *)(pDev->RegBase + I2COUT_PORT) |= (1 << TVOUT_RST_BIT);

	if (xres > 800)
	{
		BT868_Disable (RegBase);
		return (FXFALSE);
	}

	// the following will be overidden later.  This temporarily gives a clock
	// to the bt868 so we can program it.
	*pdwVidInFormat = SST_VIDEOIN_TVOUT_ENABLE;
	DELAY(20);     // delay 20uS for clock to settle

	btSetup->is_master = vgaMode < 0;
	getTvStdInfo (pDev, btSetup->tvStd, &autoCfg, 0);
	i2c_write (&I2CMask, 0xba, 0x80, RegBase);     // reset the bt868
	btSetup->mode = autoCfg;
	// invalidate Register Shadow
	for (i = 0; i < sizeof(Bt868_RegShadow); i++)
		((FxU8 *)&Bt868_RegShadow)[i] = 0;
	i2c_write (&I2CMask, 0xb8, btSetup->mode, 0);

	if (IS_H4(pDev))
	{
		i2c_write (&I2CMask, 0xba, 0x01, 0);       // disable DAC A
		*pdwTvClkDly = (*pdwTvClkDly & ~0xf8000) | ((ClkDly[1][btSetup->mode] & 15) << 16L);
	}
	else
		*pdwTvClkDly = (*pdwTvClkDly & ~0xe0000000) | (ClkDly[0][btSetup->mode] << 28L);
	if (ClkDly[IS_H4(pDev)][btSetup->mode] & 0x10)
		dwVidInFormat |= SST_VIDEOIN_G4_FOR_POSEDGE;

	// setup DAC routing
	i2c_write (&I2CMask, 0xce,
		IS_H4(pDev) ? (btSetup->cvbsOut ? 0 : 0x24) : 0x18, 0); 
	i2c_write (&I2CMask, 0xc6, 0, 0); 
	if (btSetup->is_master)
	{
		i2c_write (&I2CMask, 0xc4, 0x01, 0);   // enable pins
		i2c_write (&I2CMask, 0x70, 0x1f, 0);   // hsync width
	}

	tvSize.dwOverScan = btSetup->size;
	BT868_SetSize (&tvSize, pDev);

	TvSetPicControl.dwCap = TV_BRIGHTNESS;
	TvSetPicControl.dwStep = btSetup->bright;
	BT868_SetPicControl (&TvSetPicControl, pDev);

	TvSetPicControl.dwCap = TV_SATURATION;
	TvSetPicControl.dwStep = btSetup->saturation;
	BT868_SetPicControl (&TvSetPicControl, pDev);

	TvSetPicControl.dwCap = TV_FLICKER;
	TvSetPicControl.dwStep = btSetup->filter;
	BT868_SetPicControl (&TvSetPicControl, pDev);

	*pdwVidInFormat = (*pdwVidInFormat & ~0x3c160) | dwVidInFormat;
	return (FXTRUE);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPicControl

Description:    Get the picture contorl (Brightness, Stautation, or
                Flicker).
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPicControl (PTVCURCAP tvPicValue, void *tvOutData)
{
	switch (tvPicValue->dwCap)
	{
		case TV_BRIGHTNESS:
			tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->bright;
			break;
		case TV_SATURATION:
			tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->saturation;
			break;
		case TV_FLICKER:
			tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->filter;
			break;
	}
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetFilterControl

Description:    Get the filter contorl.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetFilterControl (PTVCURCAP tvPicValue, void *tvOutData)
{
	tvPicValue->dwStep = ((TVOUT_CURSETUP *)tvOutData)->filter;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetSizeControl

Description:    Get the horizontal/vertical contorl.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetSizeControl (PTVCURSIZE tvPicValue, void *tvData)
{
	TVOUT_CURSETUP *tvOutData = tvData;

	tvPicValue->dwCurHorInput = tvPicValue->dwCurHorOutput = tvOutData->size;
	tvPicValue->dwCurVerInput = tvPicValue->dwCurVerOutput = tvOutData->size;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetPicControl

Description:    Set the picture contorl (Brightness, Stautation, or
                Flicker).
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_SetPicControl (PTVSETCAP pTvSetPicControl, PDEVTABLE pDev)
{
   static const DWORD STEP_XLATE[] = { 0, 3, 2, 1, 4 };
   I2CMASK I2CMask;
   DWORD dwStep;
   TVOUT_CURSETUP *tvOutData = (void *)&pDev->tvOutData;
 
   I2CMask = I2C_PROTO;             //Setup the I2C bitmasks
   I2CMask.pReg = (PDWORD)(pDev->RegBase + I2COUT_PORT);
   I2CInit(&I2CMask);               //Enable I2C
   I2CMask.bAddr = BT868_ADDR;
   I2CMask.nSize = 1;
   stop(&I2CMask);                  //Reset devices
   
   switch (pTvSetPicControl->dwCap)
   {
      case TV_BRIGHTNESS:
	     tvOutData->bright = pTvSetPicControl->dwStep;
         I2CMask.nReg = 0xca;     //Bt868 reg YAttenuate
         I2CMask.dwData = min( pTvSetPicControl->dwStep, 7) | 0xc0;
         WriteI2CRegister(&I2CMask);
         break;
         
      case TV_SATURATION:
	     tvOutData->saturation = pTvSetPicControl->dwStep;
         I2CMask.nReg = 0xcc;     //Bt868 reg CAttenuate
         I2CMask.dwData = min( pTvSetPicControl->dwStep, 7) | 0xc0;
         WriteI2CRegister(&I2CMask);
         break;               
         
      case TV_FLICKER:
	     tvOutData->filter = pTvSetPicControl->dwStep;
         I2CMask.nReg = 0xc8;       //Bt868 reg FFilter
         dwStep = STEP_XLATE[ min(pTvSetPicControl->dwStep, 4) ];
         
         if ( dwStep == 4 )         //Disable FFilter
            I2CMask.dwData = BT868_DIS_FFILT;
         else            
            I2CMask.dwData = (dwStep << 3) | dwStep;

		 I2CMask.dwData |= 0x80;
         WriteI2CRegister(&I2CMask);
         break;
               
      case TV_HUE:
      case TV_CONTRAST:
      case TV_GAMMA:
      case TV_SHARPNESS:
      case TV_CHROMA:
      case TV_LUMA:
      default:
         return TV_CONTROL_UNSUPPORTED;
   }
   
   return(0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetSize

Description:    Set the picture size.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_SetSize(PTVSETSIZE pTvSetSize, PDEVTABLE pDev)
{
	TVOUT_CURSETUP *btSetup = (void *)&pDev->tvOutData;
	FxU32 vidInFmt = ((SstIORegs * )pDev->RegBase)->vidInFormat;
	I2CMASK I2CMask;
	int reg, i;
	const FxU8 *regdat;

	I2CMask = I2C_PROTO;           //Setup the I2C bitmasks
	I2CMask.pReg = (PDWORD)(pDev->RegBase + I2COUT_PORT);
	*I2CMask.pReg = I2C_INITVAL;
	I2CMask.bAddr = BT868_ADDR;
	I2CMask.nSize = 1;

	// We have a problem.  When we blast the BT868 registers and we are in
	// master mode we drive Avenger crazy.  Syncs and clocks go every which
	// way.  So, even though it looks atrocious, disable genlock until register
	// load is done.

	if ((reg = SIZE_XLATE[pTvSetSize->dwOverScan]) >= 0)
	{
		regdat = Overscan[btSetup->mode][reg].regVals;
		// check to see if we can skip the load
		for (reg = 0x76, i = 0; reg <= 0xb4; reg += 2, i++)
		{
			if (reg == 0x80)   // bt868_position will set this for us
				continue;
			if (((FxU8 *)&Bt868_RegShadow)[(reg - 0x76) / 2] != regdat[i])
				break;
		}
		if (reg < 0xb4)
		{
			((SstIORegs * )pDev->RegBase)->vidInFormat = 0;  // genlock off
			i2c_write (&I2CMask, 0xb8, btSetup->mode, 0);
			for (reg = 0x76; reg <= 0xb4; reg += 2)
				i2c_write (&I2CMask, reg, *regdat++, 0); 
			((SstIORegs * )pDev->RegBase)->vidInFormat = vidInFmt;   // back
		}
	}
	else   // normal, ie. middle size
	{
		// do the auto config and done
		i2c_write (&I2CMask, 0xb8, btSetup->mode, 0);

		// since we counted on the bt868 to setup the PLL regs we need to update
		// the shadow to what it really is FBO bt868_fixupNonStdModes
		Bt868_RegShadow.reg9C.ucPLL_FRACT = (BYTE)AutoCfgFclk[btSetup->mode];
		Bt868_RegShadow.reg9E.ucPLL_FRACT = (BYTE)(AutoCfgFclk[btSetup->mode] >> 8);
		Bt868_RegShadow.regA0.ucPLL_INT = (BYTE)(AutoCfgFclk[btSetup->mode] >> 16);
		Bt868_RegShadow.regA0.ucBY_PLL = Bt868_RegShadow.regA0.ucEN_XCLK = 0;
	}
	btSetup->size = pTvSetSize->dwOverScan;

	// since we may have overwritten registers, rerun the following, it's cheap
	bt868_position (pDev, btSetup->hpos, btSetup->vpos);
	BT868_CopyProtect (pDev, btSetup->copyProtectOn);
	bt868_fixupNonStdModes (pDev, btSetup->tvStd);
    return(0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetStandard

Description:    Get the TV type.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetStandard (PDEVTABLE pDev, LPTVGETSTANDARD lpOutput)
{
	DWORD vpTvStd;

	getTvStdInfo (pDev, ((TVOUT_CURSETUP *)&pDev->tvOutData)->tvStd, 0,
																&vpTvStd);
	lpOutput->dwStandard = vpTvStd;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetSizeCap

Description:    Get the Min/Max size capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetSizeCap (PDEVTABLE pDev, LPTVSIZECAP lpOutput)
{
	lpOutput->dwMaxHorInput = 800;
	lpOutput->dwMaxVerInput = 600;
	lpOutput->dwMaxHorOutput = 800;
	lpOutput->dwMaxVerOutput = 600;
	lpOutput->dwMinHorInput = 640;
	lpOutput->dwMinVerInput = 480;
	lpOutput->dwMinHorOutput = 640;
	lpOutput->dwMinVerOutput = 480;
	lpOutput->dwHorStepSize = 1;
	lpOutput->dwVerStepSize = 1;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPosCap

Description:    Get the position capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPosCap (PDEVTABLE pDev, LPTVPOSCAP lpOutput)
{
	lpOutput->dwMaxLeft = 0;
	lpOutput->dwMaxRight = 800;
	lpOutput->dwHorGranularity = 1;
	lpOutput->dwMaxTop = 0;
	lpOutput->dwMaxBottom = 600;
	lpOutput->dwVGAGranularity = 8;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetFilterCap

Description:    Get the filter capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetFilterCap (PDEVTABLE pDev, LPTVCAPDATA lpOutput)
{
	switch (lpOutput->dwCap)
	{
		case TV_FLICKER:
			lpOutput->dwNumSteps = 5;
			break;
		default:
			lpOutput->dwNumSteps = 0;
	}
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_GetPicCap

Description:    Get the picture capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int BT868_GetPicCap (PDEVTABLE pDev, LPTVCAPDATA lpOutput)
{
	switch (lpOutput->dwCap)
	{
		case TV_BRIGHTNESS:
		case TV_SATURATION:
			lpOutput->dwNumSteps = 8;
			break;
		default:
			lpOutput->dwNumSteps = 0;
	}
	return (0);
}

#if SAGE_LCD

/*----------------------------------------------------------------------
Function name:  WriteHPort

Description:    Write the host port (for the flat-panel).

Information:

Return:         VOID
----------------------------------------------------------------------*/
void WriteHPort( DWORD RegBase, WORD reg, WORD data )
{
   PDWORD pdwHPort = (PDWORD)(RegBase + VID_IN_SERIAL_PARALLEL);
   DWORD  dwHPort  = (*pdwHPort & ~(H3_HPORT_MASK|H3_VMI_DATAOUT_DISABLE));
   
   *pdwHPort = dwHPort | ((DWORD)reg<<6);
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)reg<<6) | H3_HPORT_ALE;
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)reg<<6);
   DELAY(USEC);  //Delay 1 microsec
   
   *pdwHPort = dwHPort | ((DWORD)data<<6);
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)data<<6) | H3_HPORT_WR;
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)data<<6);
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | H3_VMI_DATAOUT_DISABLE;
}


/*----------------------------------------------------------------------
Function name:  ReadHPort

Description:    Read the host port (for the flat-panel).

Information:

Return:         INT     Result of the read.
----------------------------------------------------------------------*/
int ReadHPort( DWORD RegBase, WORD reg )
{
   PDWORD pdwHPort = (PDWORD)(RegBase + VID_IN_SERIAL_PARALLEL);
   DWORD  dwHPort  = (*pdwHPort & ~(H3_HPORT_MASK|H3_VMI_DATAOUT_DISABLE));
   DWORD  dwRead;

   *pdwHPort = dwHPort | ((DWORD)reg<<6);
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)reg<<6) | H3_HPORT_ALE;
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)reg<<6);
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)reg<<6) | H3_VMI_DATAOUT_DISABLE;
   
   DELAY(USEC);  //Delay 1 microsec
   *pdwHPort = dwHPort | ((DWORD)reg<<6) | H3_HPORT_RD | H3_VMI_DATAOUT_DISABLE;
   DELAY(USEC);  //Delay 1 microsec
   dwRead = *pdwHPort;
   *pdwHPort = dwHPort;
   
   return( (int)(dwRead >> 6) & 0x00ff );
}


/*----------------------------------------------------------------------
Function name:  EnableHPort

Description:    Enable the host port (for the flat-panel).

Information:

Return:         VOID
----------------------------------------------------------------------*/
void EnableHPort( DWORD RegBase )
{
   PDWORD   pdwVidInFormat = (PDWORD)(RegBase + VID_IN_FORMAT);
   PDWORD   pdwVidInSerialParallel = (PDWORD)(RegBase + VID_IN_SERIAL_PARALLEL);

   *pdwVidInFormat = (*pdwVidInFormat & ~H3_VMI_MODE_MASK) | H3_VMI_MODE_VMI;
   *pdwVidInSerialParallel = (*pdwVidInSerialParallel & ~H3_HPORT_MASK) | 
                                                          H3_VMI_DATAOUT_DISABLE |
                                                          H3_SERIALPARALLEL_HOSTPORT | 
                                                          H3_SERIALPARALLEL_VMICSN |
                                                          H3_SERIALPARALLEL_VMIDSN |
                                                          H3_SERIALPARALLEL_VMIRWN |
                                                          H3_SERIALPARALLEL_VMI    |
                                                          H3_SERIALPARALLEL_GPIO1;
}


/*----------------------------------------------------------------------
Function name:  DisableHPort

Description:    Disable the host port (for the flat-panel).

Information:

Return:         VOID
----------------------------------------------------------------------*/
void DisableHPort( DWORD RegBase )
{
   PDWORD   pdwVidInFormat = (PDWORD)(RegBase + VID_IN_FORMAT);
   PDWORD   pdwVidInSerialParallel = (PDWORD)(RegBase + VID_IN_SERIAL_PARALLEL);

   *pdwVidInFormat = (*pdwVidInFormat & ~H3_VMI_MODE_MASK) | H3_VMI_MODE_TV;
   *pdwVidInSerialParallel = (*pdwVidInSerialParallel & ~(H3_SERIALPARALLEL_HOSTPORT|H3_SERIALPARALLEL_GPIO1));
}


/*----------------------------------------------------------------------
Function name:  printDebugToMono 

Description:    

Information:

Return:         INT     0 is always returned
----------------------------------------------------------------------*/
printDebugToMono (void *pTvSetSpecial)
{
	static FxU8 *disp;
	int i = 0;
	FxU8 *c = pTvSetSpecial;

	if (!disp)
		disp = _MapPhysToLinear (0xb0000, 25*80*2, 0);

	while (*c)
	{
		disp[(i + (24 * 80)) * 2] = *c++;
		disp[((i + (24 * 80)) * 2) + 1] = 7;
		i++;
	}
	for (i = 0; i < (24 * 80 * 2); i++)
		disp[i] = disp[i + 160];
	for (i = 0; i < 80; i++)
	{
		disp[(i + (24 * 80)) * 2] = ' ';
		disp[((i + (24 * 80)) * 2) + 1] = 7;
	}
	return (0);
}


/*----------------------------------------------------------------------
Function name:  BT868_SetSpecial

Description:    Set the TV as a special device (ie. LCD panel).

Information:

Return:         INT     0 is always returned
----------------------------------------------------------------------*/
int BT868_SetSpecial(PTVSETSPECIAL pTvSetSpecial, DWORD RegBase)
{
   PDWORD   pdwMiscInit1         = (PDWORD)(RegBase + MISC_INIT1);
   I2CMASK I2CMask;
   int      i;
   DWORD    dwPLLFreq;
   
   if ( pTvSetSpecial->dwCap == TV_SETSPECIAL_LCDPANEL )
   {
      I2CMask = I2C_PROTO;             //Setup the I2C bitmasks
      I2CMask.pReg = (PDWORD)(RegBase + I2COUT_PORT);
      I2CInit(&I2CMask);               //Enable I2C
      I2CMask.bAddr = 0x00;
      I2CMask.nSize = 1;
      
      //Program 9161 PLL device:
      I2CMask.nReg = (WORD)((pTvSetSpecial->dwIPclock & 0xe00000) >> 21);  //Reg
      dwPLLFreq = pTvSetSpecial->dwIPclock & 0x1fffff; 
      I2CMask.dwData = dwPLLFreq;      //Data
      WritePLLReg(&I2CMask);

      I2CMask.nReg = 0x00;             //Select output reg
      SelectPLLReg(&I2CMask);
      
      //Enable hostport interface and setup FlatPanel interface:
      EnableHPort( RegBase );

      
      if (pTvSetSpecial->dwIndex)
      {
         for (i = 0; (i < (int)pTvSetSpecial->dwIndex) && (i < MAX_SPECIAL_DATA); i += 2)
         {
            WriteHPort( RegBase, pTvSetSpecial->wData[i], pTvSetSpecial->wData[i+1] );
            wSageRegData[i] = pTvSetSpecial->wData[i];      //Save the file data
            wSageRegData[i+1] = pTvSetSpecial->wData[i+1];
         }
      }
      else //Use default table values
      {
         for ( i = 0; i < (sizeof(wSageRegData)/sizeof(wSageRegData[0])); i += 2)
            WriteHPort( RegBase, wSageRegData[i], wSageRegData[i+1] );
      }
      
      //Test register write/read
      //WriteHPort( RegBase, 0x19, 0xaa );
      //dwSageData = ReadHPort(RegBase, 0x19);
    
      //Disable hostport and enable TV-Out for LCD panel      
      DisableHPort( RegBase );
	  *pdwMiscInit1 = (*pdwMiscInit1 & 0x1fffffff) | 0x40000000;
   }
   else if (pTvSetSpecial->dwCap == TV_SETSPECIAL_WR_LCDREG)
   {
#if 1
		i = pTvSetSpecial->dwIndex;
		EnableHPort (RegBase);
		WriteHPort (RegBase, (WORD)(i >> 8), (WORD)(i & 255));
		DisableHPort (RegBase);
#else
		printDebugToMono ((void *)pTvSetSpecial->wData);
#endif
   }
   else if (pTvSetSpecial->dwCap == TV_SETSPECIAL_RD_LCDREG)
   {
		EnableHPort (RegBase);
		pTvSetSpecial->dwIndex = ReadHPort (RegBase, (WORD)pTvSetSpecial->dwIndex);
		DisableHPort (RegBase);
   }
   return(0);
}

#else

int BT868_SetSpecial(PTVSETSPECIAL pTvSetSpecial, DWORD RegBase)
{
	return (0);
}

#endif    // SAGE_LCD


/*----------------------------------------------------------------------
Function name:  BT868_Disable

Description:    Disable the TV device.

Information:

Return:         INT     0 is always returned
----------------------------------------------------------------------*/
int BT868_Disable(DWORD RegBase)
{
   PDWORD   pdwVidInFormat = (PDWORD)(RegBase + VID_IN_FORMAT);
//   I2CMASK I2CMask;
//
//   leave DACs live to make it easier to reenable at shutdown time!
//   i2c_write (&I2CMask, 0xc4, 0x00, RegBase);   // disable output pins
//   i2c_write (&I2CMask, 0xba, 0x10, 0);         // disable DACs
   *pdwVidInFormat = 0;
   return(0);
}


/* 
Formatting for save/restore of parameters.  This will be used by all TV
encoders, not just the bt.  This'll get moved eventually.
*/

char *TvOutParms[] = {
	"mode",
	"brightness",
	"saturation",
	"filter",
	"hposition",
	"vposition",
	"size",
	};

#define NUM_TVPARMS   (sizeof(TvOutParms) / sizeof(TvOutParms[0]))


/*----------------------------------------------------------------------
Function name:  tvOutGetNextValue

Description:    Return TV capability values.

Information:    can not fail, given name, return current value.

Return:         INT     one of upto seven values.
----------------------------------------------------------------------*/
int tvOutGetNextValue (char *valueName, TVOUT_CURSETUP *tvOutData)
{
	FxU8 i, idx;

	tvOutData->lastValueIndex = tvOutData->lastValueIndex >= (NUM_TVPARMS - 1)
								? 0 : tvOutData->lastValueIndex + 1;
	idx = tvOutData->lastValueIndex;
    for (i = 0; valueName[i] = TvOutParms[idx][i]; i++) ;   // strcpy
	switch (idx)
	{
		case 0: return (tvOutData->mode);
		case 1: return (tvOutData->bright);
		case 2: return (tvOutData->saturation);
		case 3: return (tvOutData->filter);
		case 4: return (tvOutData->hpos);
		case 5: return (tvOutData->vpos);
		case 6: return (tvOutData->size);
//		case 7: return (tvOutData->tvStd);
      default: return (0);
	}
}


/*----------------------------------------------------------------------
Function name:  tvOutSetNextValue

Description:    Set TV capability values.

Information:    This shouldn't ever fail.  It should be called
                right after GetNext so LastValueIndex should point
                to the parm.  However, we check to make sure and
                return FXFALSE if the parameter name doesn't match
                what we expect.

Return:         INT     FXTRUE if success,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
int tvOutSetNextValue (FxI8 *valueName, int value, TVOUT_CURSETUP *tvOutData)
{
	FxI8 idx = tvOutData->lastValueIndex;
	FxI8 *c = TvOutParms[idx];
	int i = 0;

	while (valueName[i] == c[i] && (c[i] && valueName[i]))    // strcmp
		i++;
	if (!c[i] && !valueName[i])
	{
		switch (idx)
		{
			case 0:
//				tvOutData->mode = value;
				return (FXTRUE);
			case 1:
				tvOutData->bright = value;
				return (FXTRUE);
			case 2:
				tvOutData->saturation = value;
				return (FXTRUE);
			case 3:
				tvOutData->filter = value;
				return (FXTRUE);
			case 4:
				tvOutData->hpos = value;
				return (FXTRUE);
			case 5:
				tvOutData->vpos = value;
				return (FXTRUE);
			case 6:
				tvOutData->size = value;
				return (FXTRUE);
		}
	}
	return (FXFALSE);
}


/*----------------------------------------------------------------------
Function name:  BT868_CopyProtect

Description:    Enable or Disable Macrovision encoding on bt869

Information:    setting = 1 means on

Return:         INT     1 or 0
----------------------------------------------------------------------*/
int BT868_CopyProtect (PDEVTABLE pDev, int setting)
{
	FxU32 regBase = pDev->RegBase;
	I2CMASK I2CMask;
	FxU16 reg;
	FxU8 *regdat;

	setting &= 3;
	((TVOUT_CURSETUP *)pDev->tvOutData)->copyProtectOn = setting;
	if (((TVOUT_CURSETUP *)pDev->tvOutData)->mode & 1)   // PAL
		regdat = MacroVisionRegsPAL[!!setting];
	else
		regdat = MacroVisionRegsNTSC[setting];

	for (reg = 0xda; reg <= 0xfc; reg += 2, regBase = 0)
	{
		if (!i2c_write (&I2CMask, reg, *regdat++, regBase))
			break;
	}
	return (reg == 0xfe);
}

/*----------------------------------------------------------------------
Function name:  BT868_FixupVGA

Description:    Set VGA CRTC registers for full screen DOS mode 3

Information:    Returns 0

Return:         int
----------------------------------------------------------------------*/

int BT868_FixupVGA (PDEVTABLE pDev, int unused)
{
	FxU8 dummy;
	FxU16 m3_625_regs[] = \
		{0x7100, 0x8003, 0x5804, 0x8005, 0x6f06, 0x3e07, 0xf010, 0};
	FxU16 m3_525_regs[] = \
		{0x5d00, 0x8003, 0x5504, 0x8005, 0x5606, 0x3e07, 0xf010, 0};
	FxU16 *crtcRegs = (((TVOUT_CURSETUP *)pDev->tvOutData)->mode & 1) ?
						m3_625_regs : m3_525_regs;

	outpw ((FxU16)(pDev->IoBase + 0xc4), 0x0101);   // select 8 bit characters
	dummy = inp ((FxU16)(pDev->IoBase + 0xba));     // select address register
	outp ((FxU16)(pDev->IoBase + 0xc0), 0x33);      // select pixel panning reg
	outp ((FxU16)(pDev->IoBase + 0xc0), 0x00);      // and set it to 0

	outpw ((FxU16)(pDev->IoBase + 0xd4), 0x0c11);   // unlock VGA registers
	while (*crtcRegs)
		outpw ((FxU16)(pDev->IoBase + 0xd4), *crtcRegs++);
	outpw ((FxU16)(pDev->IoBase + 0xd4), 0x8c11);   // relock VGA registers
	return (0);
}

// We'll need this when we have more than one encoder to support

const TVOUT_DEV_CALLS Bt868DevCalls = {
	BT868_GetStatus,
	BT868_GetPosition,
	BT868_SetStandard,
	BT868_SetPicControl,
	BT868_SetPosition,
	BT868_SetSize,
	BT868_SetSpecial,
	BT868_Disable,
	BT868_GetPicControl,
	BT868_GetFilterControl,
	BT868_GetSizeControl,
	BT868_Enable,
    BT868_GetStandard,
    BT868_GetSizeCap,
    BT868_GetPosCap,
    BT868_GetFilterCap,
    BT868_GetPicCap,
	BT868_CopyProtect,
	BT868_FixupVGA
};


// let's just put the serial rom stuff here for now....

#define NVRAM_VERSION   0x05


/*----------------------------------------------------------------------
Function name:  nvramRead128

Description:    Read the 128 byte nvram into buffer

Information:    Returns -1 on failure or checksum which should be 0

Return:         int
----------------------------------------------------------------------*/

FxI32 nvramRead128 (PDEVTABLE pDev, FxU8 *buffer)
{
	I2CMASK i2c_mask;
	DWORD sum = 0;
	DWORD i, status;

	i2c_mask = I2C_PROTO;           //Setup the I2C bitmasks
	i2c_mask.pReg = (PDWORD)(pDev->RegBase + I2COUT_PORT);
	*i2c_mask.pReg = I2C_INITVAL;
	i2c_mask.bAddr = 0xa0;
	i2c_mask.nSize = 1;

	start (&i2c_mask);
	status = send_byte (&i2c_mask, 0xa0);
	status |= send_byte (&i2c_mask, 0x00);
	start (&i2c_mask);
	status |= send_byte (&i2c_mask, 0xa1);
	if (status)
		return (-1);
	for (i = 0; i < 128; i++)
	{
		buffer[i] = read_byte (&i2c_mask, i != 127);
		sum += buffer[i];
	}
	stop (&i2c_mask);
	return (sum & 255);
}

/*----------------------------------------------------------------------
Function name:  nvramWriteByte

Description:    Write value at addr which should be in the range 0 -> 127

Information:    Returns -1 on failure, 0 on success

Return:         int
----------------------------------------------------------------------*/
FxI32 nvramWriteByte (I2CMASK *i2c_mask, DWORD addr, DWORD value)
{
	int retries = 90;
	
	i2c_mask->dwData = value;
	i2c_mask->nReg = (WORD)addr;
	while (!WriteI2CRegister (i2c_mask) && --retries)  ;
	return (retries <= 0 ? -1 : 0);
}

/*----------------------------------------------------------------------
Function name:  nvramWrite

Description:    Write numBytes of data to addr.  Update or calculate new 
                checksum and VERSION.

Information:    Returns -1 on failure.

Return:         int
----------------------------------------------------------------------*/
FxI32 nvramWrite (PDEVTABLE pDev, DWORD addr, DWORD numBytes, FxU8 *values)
{
	I2CMASK i2c_mask;
	FxU8 rdBuffer[128];
	FxI32 i, sum;

	i2c_mask = I2C_PROTO;           //Setup the I2C bitmasks
	i2c_mask.pReg = (PDWORD)(pDev->RegBase + I2COUT_PORT);
	*i2c_mask.pReg = I2C_INITVAL;
	i2c_mask.bAddr = 0xa0;
	i2c_mask.nSize = 1;

	if ((sum = nvramRead128 (pDev, rdBuffer)) < 0)   // read failed
		return (-1);
	if (sum)          // sum should be 0
		sum ^= nvramRead128 (pDev, rdBuffer);
	if (sum)    // we couldn't get the same sum twice!!  Give up
		return (-1);

	for (i = 0; i < (int)numBytes; i++)
	{
		if (rdBuffer[i + addr] != values[i])
		{
			nvramWriteByte (&i2c_mask, i + addr, values[i]);
		    rdBuffer[i + addr] = values[i];
		}
	}
	// recalculate sum
	for (sum = i = 0; i < 127; i++)
		sum -= rdBuffer[i];

	if (rdBuffer[126] != (FxU8)NVRAM_VERSION)
		nvramWriteByte (&i2c_mask, 126, NVRAM_VERSION);
	if (rdBuffer[127] != (FxU8)sum)
		nvramWriteByte (&i2c_mask, 127, sum);
   return 1;
}

/*----------------------------------------------------------------------
Function name:  tvstdToNVRAM

Description:    Update tv standard in nvram.  It is written to location 0.

Information:    Returns -1 on failure, 0 if OK or not an STB card.

Return:         int
----------------------------------------------------------------------*/
FxI32 tvstdToNVRAM (PDEVTABLE pDev, PTVSETSTANDARD tvstd)
{
	FxU8 nvstd;

	if (!IS_3DFX_REF_BOARD(pDev))
	{
		nvstd = (FxU8)(vpTvStdToBiosStd (tvstd->dwStandard));
		return (nvramWrite (pDev, 0, 1, &nvstd));
	}
	else
		return (0);
}


