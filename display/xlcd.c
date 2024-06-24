/* -*-c++-*- */
/* $Header: xlcd.c, 1, 9/12/99 12:52:07 AM PDT, StarTeam VTS Administrator$ */
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
** File name:   xlcd.c
**
** Description: Support functions for xlcd.
**
** $Revision: 1$
** $Date: 9/12/99 12:52:07 AM PDT$
**
** $History: xlcd.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 2:00p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 25  *****************
** User: Stuartb      Date: 3/18/99    Time: 11:48a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added the function panelFixupVGA to set vertical total CRTC register
** when transitioning to a full screen DOS box.
** 
** *****************  Version 24  *****************
** User: Stuartb      Date: 3/18/99    Time: 10:32a
** Updated in $/devel/h3/Win95/dx/minivdd
** Add hack for 1280 in 1280 out.  Tweaked timings.  Add DDC select for
** panel.  Added comments.
** 
** *****************  Version 22  *****************
** User: Stuartb      Date: 2/18/99    Time: 2:57p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added #idef'd hacks for testing Hitachi panel.  Added filter
** coefficient setting code.  Fixed refresh rate bug.
** 
** *****************  Version 21  *****************
** User: Stuartb      Date: 2/16/99    Time: 5:11p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed 8x6 capture clock due to modetabl changes.  Added more support
** for 12x10 panels.
** 
** *****************  Version 20  *****************
** User: Stuartb      Date: 2/08/99    Time: 3:39p
** Updated in $/devel/h3/Win95/dx/minivdd
** Hack to allow (VGA std) Princeton and Caompaq FP500 panels to work.
** Gateway panels may no long work.
** 
** *****************  Version 19  *****************
** User: Stuartb      Date: 2/08/99    Time: 8:58a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changes to fix lcd boot and simultaneous VMI & TV/LCD.
** 
** *****************  Version 18  *****************
** User: Stuartb      Date: 2/02/99    Time: 5:02p
** Updated in $/devel/h3/Win95/dx/minivdd
** Invert DCLK (PANEL_CTRL reg).  Makes noise at 10x7 go away.
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 2/02/99    Time: 10:59a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to support turning LCD on/off on sleep.
** 
** *****************  Version 16  *****************
** User: Agus         Date: 2/01/99    Time: 4:36p
** Updated in $/devel/h3/Win95/dx/minivdd
** Move IS_H4 macro define to h3vdd.h
** 
** *****************  Version 15  *****************
** User: Stuartb      Date: 1/28/99    Time: 4:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Eliminate negative coeffs in filter due to chip problems.
** 
** *****************  Version 14  *****************
** User: Stuartb      Date: 1/26/99    Time: 5:14p
** Updated in $/devel/h3/Win95/dx/minivdd
** Code to enable interpolation filter.  Minor changes for -04 chip.
** Actual DCLK is now in Xcap.
** 
** *****************  Version 13  *****************
** User: Stuartb      Date: 1/22/99    Time: 5:14p
** Updated in $/devel/h3/Win95/dx/minivdd
** Tweaked parameters, fixed bug in pllCalc, added code for 12x10 panels.
** 
** *****************  Version 12  *****************
** User: Stuartb      Date: 1/21/99    Time: 11:18a
** Updated in $/devel/h3/Win95/dx/minivdd
** Allow panels greater that 1024x768.
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/15/99    Time: 9:59a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
*/

#if 1
#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "devtable.h"
#include "h3g.h"
#undef  VDDONLY
#include "i2c.h"

extern const struct i2cmask I2C_PROTO;
extern const DWORD I2C_INITVAL;
extern int read_sda (PI2CMASK);
extern int WaitTime (FxU32);

#else    // debug on PC
#define PI2CMASK void*
#define PDEVTABLE void*
#define XLCD_DEBUG   1
#endif

#include "xlcd.h"

#if 0    // Arghhh, they changed the hardware again
#undef IS_3DFX_REF_BOARD
#define IS_3DFX_REF_BOARD(pDev)  ((pDev->dwSubSystemID & 0xff0000) < 0x00300000)


const DWORD DDC_CRT_INITVAL = (DWORD)((1 << DDC_ENABLE)      |
									  (1 << DDC_SCL_OUT_BIT) |
									  (1 << DDC_SDA_OUT_BIT) |
									  (1 << TVOUT_RST_BIT));
const DWORD DDC_LCD_INITVAL = (DWORD)((1 << DDC_ENABLE)      |
									  (1 << DDC_SCL_OUT_BIT) |
									  (1 << DDC_SDA_OUT_BIT) |
									  (SST_SERPAR_GPIO_1)    |
									  (1 << TVOUT_RST_BIT));
const struct i2cmask DDC_PROTO = {0, 0, 0, 1, 0, DDC_ENABLE, DDC_SCL_OUT_BIT, \
						DDC_SDA_OUT_BIT, DDC_SCL_IN_BIT, DDC_SDA_IN_BIT};
#endif

FxU8 LCD_DEBUG[64];

typedef struct
{
	FxU16 width;
	FxU16 height;
	FxU8  refresh;
}  ESTABLISHED_TIMING;

// established timings in EDID per VESA

static ESTABLISHED_TIMING EstTimings[] =
   {{800, 600, 60},
	{800, 600, 56},
	{640, 480, 75},
	{640, 480, 72},
	{640, 480, 67},
	{640, 480, 60},
	{720, 400, 88},
	{720, 400, 70},
	{1280, 1024, 75},
	{1024, 768, 75},
	{1024, 768, 70},
	{1024, 768, 60},
	{1024, 768, 87},
	{832, 624, 75},
	{800, 600, 75},
	{800, 600, 72},
	{1152, 700, 75},
	{0, 0, 0}};


#define XLCD_ADDR     0xe0
#define INTERPOLATE   1       // off til we get things going
#define RETRIES       90

#if XLCD_DEBUG
extern char *XlcdRegNames[];
#endif

#define RGB888_BPP    3
#define SYNC_INV      (CAP_FLAG_INV_HSYNC | CAP_FLAG_INV_VSYNC)

static const DWORD Xlcd_DclkMatrix[][5] = {
/*                             6x4     8x6     10x7  12x10   */
/* for 1024x768 panels  */  { 68011,  64982,  65455,      0},
/* for 1280x1024 panels */  {111682, 109772, 109176, 108000},
};

const XLCD_DISPLAY Xdisp_1024 = {1344, 1024, 160, 24, 806, 768, 6, 3};
const XLCD_DISPLAY Xdisp_1280 = {1680, 1280, 112, 48, 1066, 1024, 6, 3};
const XLCD_CAPTURE Xcap_1280 = {1280, 238, 1024, 37, 0, 108000};
const XLCD_CAPTURE Xcap_1024 = {1024, 144, 768, 29, SYNC_INV, 65455};
const XLCD_CAPTURE Xcap_800 =  {800, 72, 600, 23, 0, 65455};
const XLCD_CAPTURE Xcap_640 =  {640, 41, 480, 31, SYNC_INV, 68011};

static DWORD FpFlags;  // not real kosher, remember fpflags across on/off cycles


const FxU8 InterpCoeffs[][64] = {
{	0,   1, 127,  0,    // bilinear
	0,   8, 120,  0,
	0,  16, 112,  0,
	0,  24, 104,  0,
	0,  32,  96,  0,
	0,  40,  88,  0,
	0,  48,  80,  0,
	0,  56,  72,  0,
	0,  64,  64,  0,
	0,  72,  56,  0,
	0,  80,  48,  0,
	0,  88,  40,  0,
	0,  96,  32,  0,
	0, 104,  24,  0,
	0, 112,  16,  0,
	0, 120,   8,  0},

{   0,   1, 127, 0,     // gausian
	0,   8, 120, 0,
	2,  14, 110, 2,
	3,  21, 101, 3,
	4,  28,  92, 4,
	4,  36,  84, 4,
	6,  42,  74, 6,
	7,  49,  65, 7,
	8,  56,  56, 8,
	7,  65,  49, 7,
	6,  74,  42, 6,
	4,  84,  36, 4,
	4,  92,  28, 4,
	3, 101,  21, 3,
	2, 110,  14, 2,
	0, 120,   8, 0},

{	0x80,   1, 127,  0x80,   // sharp
	0x81,   8, 120,  0x81,
	0x82,  18, 114,  0x82,
	0x83,  27, 107,  0x83,
	0x84,  36, 100,  0x84,
	0x84,  44,  92,  0x84,
	0x86,  54,  86,  0x86,
	0x87,  63,  79,  0x87,
	0x88,  72,  72,  0x88,
	0x87,  79,  63,  0x87,
	0x86,  86,  54,  0x86,
	0x84,  92,  44,  0x84,
	0x84, 100,  36,  0x84,
	0x83, 107,  27,  0x83,
	0x82, 114,  18,  0x82,
	0x80, 120,   8,  0x80},

{   0x00,  1, 127, 0x00,      // crinkly
	0x82, 10, 122, 0x82,
	0x84, 20, 116, 0x84,
	0x86, 30, 110, 0x86,
	0x88, 40, 104, 0x88,
	0x8a, 50,  98, 0x8a,
	0x8c, 60,  92, 0x8c,
	0x8e, 70,  86, 0x8e,
	0x90, 80,  80, 0x90,
	0x8e, 86,  70, 0x8e,
	0x8c, 92,  60, 0x8c,
	0x8a, 98,  50, 0x8a,
	0x88, 104, 40, 0x88,
	0x86, 110, 30, 0x86,
	0x84, 116, 20, 0x84,
	0x82, 122, 10, 0x82}};



#undef PNL_1280_TEST   // to test 12x10 panel with no EDID, best we can do

/*----------------------------------------------------------------------
Function name:  xlcdSendByte

Description:    Send a byte to the XLCD device.

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
int xlcdSendByte (PI2CMASK pI2CMask, BYTE data)
{
	int	i;

	for (i = 0; i < 8; i++)
	{
	  sda (pI2CMask, (data << i) & 0x80);
	  scl (pI2CMask, 1);
	  scl (pI2CMask, 0);
	}
	scl (pI2CMask, 1);   // these two lines are backwards, but then again so
	sda (pI2CMask, 1);   // is MRTI chip
	i = (read_sda(pI2CMask) == 1) ? -1 : 0;
	scl (pI2CMask, 0);
	return (i);
}


/*----------------------------------------------------------------------
Function name:  writePnlReg

Description:    

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
int writePnlReg (PI2CMASK pI2CMask, int reg, int val)
{
	int retries;

	LCD_DEBUG[reg & 63] = val;
	for (retries = RETRIES + 1; retries--;   )
	{
		start (pI2CMask);
		if (!xlcdSendByte (pI2CMask, XLCD_ADDR))
			if (!xlcdSendByte (pI2CMask, 0x00))
				if (!xlcdSendByte (pI2CMask, (char)reg))
					if (!xlcdSendByte (pI2CMask, (char)val))
						break;
		stop (pI2CMask);
	}
	stop (pI2CMask);
	if (retries != RETRIES)
		Debug_Printf ("%d write retries\n", RETRIES - retries);

#if 0
	printf ("(%02x) %s written %02x, read %02x\n", reg, XlcdRegNames[reg],
						val, dbgReadXLCDReg (reg, XLCD_ADDR));
#endif
	return (retries <= 0 ? -1 : 0);
}


/*----------------------------------------------------------------------
Function name:  xlcdFilter

Description:    

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
int xlcdFilter (PI2CMASK pI2CMask)
{
	int retries;
	int addr = 0;
	FxU8 const *coeffs;

	if (FpFlags & FPFLAG_FILTER_HARD)
		coeffs = InterpCoeffs[!(FpFlags & FPFLAG_FILTER_SOFT) ? 2 : 3];
	else
		coeffs = InterpCoeffs[!(FpFlags & FPFLAG_FILTER_SOFT) ? 0 : 1];

	for (retries = RETRIES + 1; retries--;   )
	{
		start (pI2CMask);
		if (!xlcdSendByte (pI2CMask, XLCD_ADDR))
		{
			if (!xlcdSendByte (pI2CMask, 0x60))
			{
				if (!xlcdSendByte (pI2CMask, (FxU8)addr))  // send starting addr
				{
					// setup H & V filters identically
					for (     ; addr < 128; addr++)
					{
						if (xlcdSendByte (pI2CMask, coeffs[addr & 63]))
							break;
					}
				}
			}
		}
		stop(pI2CMask);
		if (addr == 128)
			break;
		if (addr & 1)   // need to always send pairs of bytes
			addr--;
	}
	if (retries != RETRIES)
		Debug_Printf ("%d h filter retries\n", RETRIES - retries);

	return (retries <= 0 ? -1 : 0);
}


/*----------------------------------------------------------------------
Function name:  PLLcalc

Description:    Determine PLL m/n that yields the Dclk closest to that
                requested.

Information:    

Return:         INT
----------------------------------------------------------------------*/
int PLLcalc (int refFreq, int desiredFreq)
{
	long freq, diff, closest;
	int retval, m, n;

	closest = 0x7fffffff;
	for (n = 3; n < 18; n++)
	{
		for (m = 3; m < 66; m++)
		{
			freq = (long)(refFreq * m) / n;
			if ((diff = freq - desiredFreq) < 0)
				diff = 0 - diff;
			if (diff < closest)
			{
				closest = diff;
				retval = (m << 8) | n;
			}
		}
	}
	retval -= 0x0202;    // as we program m-2 and n-2
	return (retval);
}


/*----------------------------------------------------------------------
Function name:  panelOff

Description:    Turn off the panel device.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void panelOff (PDEVTABLE pDev)
{
	struct i2cmask i2cMask = I2C_PROTO;
	SstIORegs *sstIOregs = (void *)pDev->RegBase;

	i2cMask.pReg = (DWORD *)&sstIOregs->vidSerialParallelPort;
	*i2cMask.pReg |= (I2C_INITVAL | 0x80000000);
	writePnlReg (&i2cMask, PANEL_POWER, 0);
}


/*----------------------------------------------------------------------
Function name:  panelOn

Description:    Turn on the panel device.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void panelOn (PDEVTABLE pDev)
{
	struct i2cmask i2cMask = I2C_PROTO;
	SstIORegs *sstIOregs = (void *)pDev->RegBase;

	i2cMask.pReg = (DWORD *)&sstIOregs->vidSerialParallelPort;
	*i2cMask.pReg |= (I2C_INITVAL | 0x80000000);
	writePnlReg (&i2cMask, PANEL_POWER, 0x3d);
}


/*----------------------------------------------------------------------
Function name:  panelConfig

Description:    Configure the panel device.

Information:    

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
panelConfig (PDEVTABLE pDev, XLCD_CAPTURE *cap, XLCD_DISPLAY *disp, int pd)
{
	SstIORegs *sstIO = (void *)pDev->RegBase;
	int v, start, m, n;
	int hzoom = (long)(cap->capWidth * 2048L) / disp->dispWidth;
	int vzoom = (long)(cap->capHeight * 2048L) / disp->dispHeight;
	struct i2cmask ddcMask = I2C_PROTO;

	sstIO->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE;
	sstIO->vidInFormat &= ~(SST_VIDEOIN_G4_FOR_POSEDGE);
	// setup pixclk to pixdata delay
	sstIO->tmuGbeInit = (sstIO->tmuGbeInit & ~0xf8000) | 0;
	ddcMask.pReg = (DWORD *)&sstIO->vidSerialParallelPort;
	*ddcMask.pReg |= (I2C_INITVAL | 0x80000000);

#if 0
	printf ("capW = %d, capH = %d, capHofs = %d, capVofs = %d\n",
				cap->capWidth, cap->capHeight, cap->capHofst, cap->capVofst);
	printf ("dispWidth = %d, dispHeight = %d, dispHtotal = %d, dispVtotal = %d, dispHblk = %d, dispVBlk = %d\n", disp->dispWidth, disp->dispHeight,
	                             disp->htotal, disp->vtotal, disp->hblank, 
								 disp->vblank);
#endif

	if (pd)        // try not to destroy panel, people will get mad at you
		panelOff (pDev);
	
	// Calulate PLL m/n

	n = PLLcalc (14318, cap->clkFreq);
	m = n >> 8;
	/*
	PLL_CONTROL = 0x0e = (M - 2) = 0x39
	MISC_GP_REG = 0x0f = (N - 2) = 0x0b   , need docs
	64.98mc = (14.31818 * 59) / 13
	*/
	writePnlReg (&ddcMask, PLL_CONTROL, m);
	writePnlReg (&ddcMask, MISC_GP_REG, 0x30 | (n & 15));

	// Display parameters
	writePnlReg (&ddcMask, DISPLAY_HTOTAL, disp->htotal / 8);
	writePnlReg (&ddcMask, DISPLAY_HSTOP, (disp->dispWidth / 8) - 1);
	writePnlReg (&ddcMask, DISPLAY_HBLK_START,
						   (disp->dispWidth + disp->hsyncOfst) / 8);
	writePnlReg (&ddcMask, DISPLAY_VTOTAL_LO, disp->vtotal & 0xff);
	writePnlReg (&ddcMask, DISPLAY_VTOTAL_HI, disp->vtotal >> 8);
	disp->dispHeight--;
	writePnlReg (&ddcMask, DISPLAY_VSTOP_LO, disp->dispHeight & 0xff);
	writePnlReg (&ddcMask, DISPLAY_VSTOP_HI, disp->dispHeight >> 8);
	disp->dispHeight++;
	start = disp->dispHeight + disp->vsyncOfst;
	writePnlReg (&ddcMask, DISPLAY_VBLK_START_LO, start & 255);
	writePnlReg (&ddcMask, DISPLAY_VBLK_START_HI, start >> 8);
	writePnlReg (&ddcMask, DISPLAY_VSYNC_WIDTH, disp->vsyncWidth);
	writePnlReg (&ddcMask, DISPLAY_HSYNC_WIDTH, disp->hsyncWidth / 8);

	// Capture parameters
	writePnlReg (&ddcMask, CAPTURE_HSTART, cap->capHofst >> 3);
	writePnlReg (&ddcMask, CAPTURE_PEL_ALIGNMENT, cap->capHofst & 7);
	writePnlReg (&ddcMask, CAPTURE_HSTOP, (cap->capWidth / 8) + (cap->capHofst >> 3));
	writePnlReg (&ddcMask, CAPTURE_VSTART_LO, cap->capVofst);
	writePnlReg (&ddcMask, CAPTURE_VSTART_HI, 0);
	cap->capHeight += (cap->capVofst + 1);
	writePnlReg (&ddcMask, CAPTURE_VSTOP_LO, cap->capHeight & 0xff);
	writePnlReg (&ddcMask, CAPTURE_VSTOP_HI, cap->capHeight >> 8);
	cap->capHeight -= (cap->capVofst + 1);

	// Zoom parameters
	writePnlReg (&ddcMask, DISPLAY_HZOOM_LO, hzoom & 0xff);
	writePnlReg (&ddcMask, DISPLAY_VZOOM_LO, vzoom & 0xff);
	writePnlReg (&ddcMask, DISPLAY_HV_ZOOM_HI, ((hzoom >> 8) << 4) | (vzoom >> 8));
	v = ((cap->capWidth * RGB888_BPP) / 8) - 1;
	writePnlReg (&ddcMask, CAPTURE_LINE_WIDTH_LO, v & 255);
	writePnlReg (&ddcMask, CAPTURE_LINE_WIDTH_HI, v >> 8);

	// Miscellaneous parameters
	writePnlReg (&ddcMask, INTERRUPT_MASK, 0);

	writePnlReg (&ddcMask, CAPTURE_IP_CTRL, cap->flags & 6);

	/*
	HACK ALERT!!!!!!!!!!!!!!

	The only way to get a stable 1:1 picture on the Hitachi 1280x1024 panel 
	is to frig the chip in test mode and flip Dclk phase.  This is all wrong
	but it works.  It won't work when scaling.  The Htotal was changed slightly
	from 1688 (VESA) to 1680 to make up for the fact the clock is wrong.  This
	hack seems to stem from the phase relationship of DENA vs Hsync which 
	changes in this mode.  Assume v = 0 for non-hack case.    3/17/99 SMB
	*/

	v = (disp->dispWidth == 1280 && cap->capWidth == 1280);
	writePnlReg (&ddcMask, PANEL_CTRL, v ? 0x18 : 0x30);
	writePnlReg (&ddcMask, TEST_MODE0, v);
	writePnlReg (&ddcMask, PANEL_DITHER, 0);
	writePnlReg (&ddcMask, TEST_MODE1, 0);
	writePnlReg (&ddcMask, ENGINE_ENABLES, 0);

#if INTERPOLATE
	writePnlReg (&ddcMask, DISPLAY_CTRL, (hzoom == 2048 ? 0 : 0x05) |
	                                     (vzoom == 2048 ? 0 : 0x0a));
#else
	writePnlReg (&ddcMask, DISPLAY_CTRL, (hzoom == 2048 ? 0 : 0x01) |
	                                     (vzoom == 2048 ? 0 : 0x02));
#endif

	// Filter coeffs
	if (INTERPOLATE && (hzoom != 2048 || vzoom != 2048))
		xlcdFilter (&ddcMask);

	writePnlReg (&ddcMask, CAPTURE_ENABLE, 1);
	panelOn (pDev);
	return (0);
}


/*----------------------------------------------------------------------
Function name:  flatPanelPresent

Description:    Query for a panel device.

Information:    

Return:         INT     0 for no panel device, else panel device
----------------------------------------------------------------------*/
int flatPanelPresent (PDEVTABLE pDev)
{
	struct i2cmask ddcMask;
	SstIORegs *sstIO = (void *)pDev->RegBase;
	int status = 0;
	DWORD vidInFmt = sstIO->vidInFormat;   // save current value
	DWORD vidSerialParallel = sstIO->vidSerialParallelPort;

	if (!IS_H4(pDev))
		return (0);

	ddcMask = I2C_PROTO;
	ddcMask.pReg = (DWORD *)&sstIO->vidSerialParallelPort;
	*ddcMask.pReg |= (I2C_INITVAL | SST_SERPAR_GPIO_1);
	I2CInit (&ddcMask);
	// Probe to see if panel is phyically connected
#ifndef PNL_1280_TEST
	start (&ddcMask);
	status = send_byte (&ddcMask, (BYTE)(0xa0 | MASTER_WRITE));
	stop (&ddcMask);
#endif
	sstIO->vidSerialParallelPort = vidSerialParallel;  // restore orig value

	if (!status)  // If panel connected make sure controller chip is there
	{
		ddcMask = I2C_PROTO;
		ddcMask.pReg = (DWORD *)&sstIO->vidSerialParallelPort;
		*ddcMask.pReg |= I2C_INITVAL;
		sstIO->vidInFormat |= SST_VIDEOIN_TVOUT_ENABLE; 
		status = writePnlReg (&ddcMask, TEST_MODE1, 0);
		sstIO->vidInFormat = vidInFmt;     // restore that value
	}
	return (!status);
}


/*----------------------------------------------------------------------
Function name:  panelDDC

Description:    

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
int panelDDC (PDEVTABLE pDev, XLCD_DISPLAY *xdisp)
{
	SstIORegs *sstIO = (void *)pDev->RegBase;
	struct i2cmask ddcMask;
	int i, nb, sum, blk, clk, retstat;
	FxU8 ddcData[256], *dt;
	ESTABLISHED_TIMING *eTiming = 0;
	FxU8 retries = 8;
	DWORD vidSerialParallel = sstIO->vidSerialParallelPort;

retry:
	ddcMask = I2C_PROTO;
	ddcMask.pReg = (DWORD *)&sstIO->vidSerialParallelPort;
	*ddcMask.pReg |= (I2C_INITVAL | SST_SERPAR_GPIO_1);
	retstat = 0;
	start (&ddcMask);
	retstat |= xlcdSendByte (&ddcMask, (BYTE)(0xa0 | MASTER_WRITE));
	retstat |= xlcdSendByte (&ddcMask, 0);
	stop (&ddcMask);
	start (&ddcMask);
	retstat |= xlcdSendByte (&ddcMask, (BYTE)(0xa0 | MASTER_READ));
	for (sum = i = 0, nb = 128; i < nb; i++)
	{
		ddcData[i] = read_byte (&ddcMask, i != (nb - 1));
		nb = ddcData[0] >= 0x20 ? 256 : 128;
		sum += ddcData[i];
		DELAY(10000);
	}
	stop (&ddcMask);
	sstIO->vidSerialParallelPort = vidSerialParallel;  // restore orig value

	// make sure everything looks reasonable before proceeding
	if (!retstat && !(sum & 255) && ddcData[0] < 0x80 && sum)
	{
		/* EDID parsing is taken from VESA EDID Standard v3 r0, 1997 */

		if (ddcData[0] >= 0x20)   // decode V2.X EDID
		{
			dt = &ddcData[128];   
			// skip over luma table if present
			if (ddcData[0x7e] & 0x20)
			{
				if (ddcData[0x80] & 0x80)
					dt += ((ddcData[0x80] & 0x1f) * 3);
				else
					dt += (ddcData[0x80] & 0x1f);
			}
			// skip frequency ranges
			dt += (8 * ((ddcData[0x7e] >> 2) & 7));
			// if timing ranges are present
			if (ddcData[0x7e] & 3)
			{
				// extract H parameters
				blk = ((dt[4] << 4) & 0xf00) | dt[2];
				blk += (((dt[13] << 4) & 0xf00) | dt[11]);
				xdisp->dispWidth = ((dt[23] << 4) & 0xf00) | dt[21];
				xdisp->htotal = xdisp->dispWidth + (blk / 2);
				xdisp->hsyncOfst = ((dt[8] << 2) & 0x300) | dt[5];
				xdisp->hsyncWidth = ((dt[8] << 4) & 0x300) | dt[6];
				// extract V parameters
				blk = ((dt[4] << 8) & 0xf00) | dt[3];
				blk += ((dt[13] << 8) & 0xf00) | dt[12];
				xdisp->dispHeight = ((dt[23] << 8) & 0xf00) | dt[22];
				xdisp->vtotal = xdisp->dispHeight + (blk / 2);
				xdisp->vsyncOfst = ((dt[8] << 2) & 0x030) | (dt[7] >> 4);
				xdisp->vsyncWidth = ((dt[8] << 4) & 0x030) | (dt[7] & 15);
				// get the average clk rate
				clk = *(FxU16 *)&dt[0];
				clk += (dt[9] + (dt[10] << 8));
				xdisp->dispClock = (long)(clk / 2) * (long)10;
			}
		}
		else    // EDID V1.X
		{
			// parse out the established timings to get the minimum we need to
			// know - display height, width and refresh.  Give preference to 
			// highest resolution and allow only 60hz refresh.

			// scan from highest resolution to lowest
			for (i = 0; i < 8; i++)
			{
				if ((ddcData[0x24] & (1 << i)) &&
					EstTimings[i + 8].refresh == 60)
						eTiming = &EstTimings[i + 8];
			}
			for (i = 0; i < 8; i++)
			{
				if ((ddcData[0x23] & (1 << i)) &&
					EstTimings[i + 0].refresh == 60)
						eTiming = &EstTimings[i + 0];
			}
			if (eTiming)
			{
				xdisp->dispHeight = eTiming->height;
				xdisp->dispWidth = eTiming->width;
			}

			dt = &ddcData[54];
			if ((clk = *(FxU16 *)&dt[0]) <= 0x0101)
			{
				xdisp->vsyncWidth = 6;    // HACK HACK
				return (0);    // no detailed timing, just dimensions
			}
			xdisp->dispClock = clk * 10;
			// extract H parameters
			xdisp->dispWidth = ((dt[4] << 4) & 0xf00) | dt[2];
			xdisp->htotal = ((dt[4] << 8) & 0xf00) + dt[3] + xdisp->dispWidth;
			xdisp->hsyncOfst = ((dt[11] << 2) & 0x300) | dt[8];
			xdisp->hsyncWidth = ((dt[11] << 4) & 0x300) | dt[9];
			// extract V parameters
			xdisp->dispHeight = ((dt[7] << 4) & 0xf00) | dt[5];
			xdisp->vtotal = ((dt[7] << 8) & 0xf00) + dt[6] + xdisp->dispHeight;
			xdisp->vsyncOfst = ((dt[11] << 2) & 0x30) | (dt[10] >> 4);
			xdisp->vsyncWidth = ((dt[11] << 4) & 0x30) | (dt[10] & 15);
		}
	}
	else if (--retries)
		goto retry;
	else
		retstat = 1;

	return (retstat ? -1 : 0);
}


/*----------------------------------------------------------------------
Function name:  panelSet

Description:    

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
int panelSet (PDEVTABLE pDev, DWORD fpFlags)
{
	SstIORegs *sstIOregs = (void *)pDev->RegBase;
    DWORD yres = (sstIOregs->vidScreenSize >> 12) & 0xfff;
    DWORD xres = sstIOregs->vidScreenSize & 0xfff;
	XLCD_CAPTURE xcap = Xcap_1024;
#ifndef PNL_1280_TEST
	XLCD_DISPLAY xdisp = Xdisp_1024;
#else
	XLCD_DISPLAY xdisp = Xdisp_1280;
#endif

	if (!fpFlags)    // Turn off the display
	{
		// disable BIOS support through SCRATCH_REG2
		outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
		outp ((FxU16)(pDev->IoBase + 0xd5),
			(FxU8)(inp ((FxU16)(pDev->IoBase + 0xd5)) & ~4));
		panelOff (pDev);
		sstIOregs->vidInFormat &= ~(SST_VIDEOIN_TVOUT_ENABLE);  // off
		return (0);
	}
	else if (fpFlags != 1)
		FpFlags = fpFlags;

	// tick reset for controller chip

	sstIOregs->vidSerialParallelPort &= ~(1 << TVOUT_RST_BIT);
	DELAY(100);
	sstIOregs->vidSerialParallelPort |= (1 << TVOUT_RST_BIT);

	if (!flatPanelPresent (pDev))
		return (-1);

	panelDDC (pDev, &xdisp);
	switch (xdisp.dispWidth)
	{
		case 1280:    // do it again with proper defaults
			xdisp = Xdisp_1280;
			panelDDC (pDev, &xdisp);
			break;
		default:
			break;
	}

	// setup appropriate default capture parms
	switch (xres)
	{
		case 640:
			xcap = Xcap_640;
			if (xdisp.dispWidth == 1280)
			{
				xdisp.htotal -= 32;     // ???????????
				xcap.clkFreq = Xlcd_DclkMatrix[1][0];
			}
			break;
		case 800:
			xcap = Xcap_800;
			if (xdisp.dispWidth == 1280)
				xcap.clkFreq = Xlcd_DclkMatrix[1][1];
			break;
		case 1024:
			if (xdisp.dispWidth == 1280)
				xcap.clkFreq = Xlcd_DclkMatrix[1][2];
			break;
		case 1280:
			xcap = Xcap_1280;
			break;
	}

	panelConfig (pDev, &xcap, &xdisp, 1);
	// enable BIOS support through SCRATCH_REG2
	outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
	outp ((FxU16)(pDev->IoBase + 0xd5),
		(FxU8)(inp ((FxU16)(pDev->IoBase + 0xd5)) | 4));
	return (0);
}


/*----------------------------------------------------------------------
Function name:  flatPanelPhysical

Description:    

Information:    

Return:         INT     0 or -1.
----------------------------------------------------------------------*/
int flatPanelPhysical (PDEVTABLE pDev, DWORD fp_out)
{
	FlatPnlPhysical *fpp = (void *)fp_out;
	static FlatPnlPhysical fpDims;    // remember dims if we get it once
#ifndef PNL_1280_TEST
	XLCD_DISPLAY xdisp = Xdisp_1024;
#else
	XLCD_DISPLAY xdisp = Xdisp_1280;
#endif
	DWORD ddcFlags = FPFLAG_GET_DIMS | FPFLAG_PRESENT;

	xdisp.dispWidth = 0;
	fpp->fpFlags &= ~(FPFLAG_PRESENT_AT_BOOT | FPFLAG_PRESENT);

	if (flatPanelPresent (pDev))
	{
		fpp->fpFlags |= FPFLAG_PRESENT;
		outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
		if (inp ((FxU16)(pDev->IoBase + 0xd5)) & 4)
			fpp->fpFlags |= FPFLAG_PRESENT_AT_BOOT;
	}

	if ((fpp->fpFlags & ddcFlags) == ddcFlags)
	{
		if (fpDims.maxWidth)    // we already got the edid, so just memcpy
		{
			fpp->maxWidth = fpDims.maxWidth;
			fpp->maxHeight = fpDims.maxHeight;
			fpp->refreshRate = fpDims.refreshRate;
			return (0);
		}
		else if (!panelDDC (pDev, &xdisp))
		{
			if (!xdisp.dispWidth)
				return (-1);
			fpp->maxWidth = xdisp.dispWidth;
			fpp->maxHeight = xdisp.dispHeight;
			fpp->refreshRate = 60;
			fpDims = *fpp;     // remember this for next time
			return (0);
		}
		else   // oh foo, just assume 10x7 panel if we can't get EDID for now
		{
#ifndef PNL_1280_TEST
			fpp->maxWidth = 1024;
			fpp->maxHeight = 768;
#else
			fpp->maxWidth = 1280;
			fpp->maxHeight = 1024;
#endif
			fpp->refreshRate = 60;
			fpDims = *fpp;     // remember this for next time
			return (0);
		}
	}
	return (-1);
}

/*----------------------------------------------------------------------
Function name:  isPanelActive

Description:    Answers the burning question in minivdd

Information:    

Return:         INT     0 or 1.
----------------------------------------------------------------------*/

int isPanelActive (PDEVTABLE pDev)
{
	outp ((FxU16)(pDev->IoBase + 0xd4), 0x1e);
	return (!!(inp ((FxU16)(pDev->IoBase + 0xd4)) & 4));
}

/*----------------------------------------------------------------------
Function name:  panelFixupVGA

Description:    Called when disabling desktop to fix VGA CRTC registers FBO
                a proper full screen DOS box on the panel.

Information:    Only called if panel is active.

Return:         Returns 0
----------------------------------------------------------------------*/

int panelFixupVGA (PDEVTABLE pDev, DWORD unused)
{
	outpw ((FxU16)(pDev->IoBase + 0xd4), 0x0c11);   // unlock VGA registers
	outpw ((FxU16)(pDev->IoBase + 0xd4), 0xff06);   // adjust refresh rate
	outpw ((FxU16)(pDev->IoBase + 0xd4), 0x8c11);   // relock VGA registers
	return (0);
}

