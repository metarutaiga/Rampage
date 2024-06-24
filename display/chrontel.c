/* -*-c++-*- */
/* $Header: chrontel.c, 1, 9/11/99 8:42:35 PM PDT, StarTeam VTS Administrator$ */
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
** File name:   chrontel.c
**
** Description: Chrontel TV Out support functions.
**
** $Revision: 1$ 
** $Date: 9/11/99 8:42:35 PM PDT$
**
** $History: chrontel.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:52p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 11  *****************
** User: Stuartb      Date: 3/02/99    Time: 4:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed obsolete TV_STANDARD_XXX defines.
** 
** *****************  Version 10  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 9  *****************
** User: Michael      Date: 1/04/99    Time: 4:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 8     11/11/98 9:28a Stuartb
** First cut at tvout changes to support Macrovision copy protection.
** Only works under Win98.
** 
** 7     9/24/98 1:37p Stuartb
** Punt to bt868 for legacy flat panel support.
** 
** 6     9/17/98 5:04p Stuartb
** Reinstated chrontel encoder interface.
** 
** 5     9/10/98 2:44p Stuartb
** TVOUT work in progress.
** 
** 4     6/22/98 9:42a Stuartb
** Whoops on checkout.  Set nSize to 1.
** 
** 2     4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 1     4/06/98 5:26p Andrew
** Chrontel 7003 TV Out file
** 
*/

#include "h3vdd.h"
#include "h3.h"
#include "i2c.h"
#define VDDONLY
#include "tv.h"
#include "devtable.h"
#undef  VDDONLY
#include "chrontel.h"
#include "bt868.h"
#include "tvoutdef.h"

extern const struct i2cmask I2C_PROTO;
extern const DWORD I2C_INITVAL;

/********************************************************************************
*
* The file Chrontel.h implements the Chrontel TV out Functions and has the 
* Chrontel specific TVOut stuff.
*
*
*********************************************************************************/

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

char * ppReg[] = {
	"Display Mode",   //0x00
	"Flicker Filter", //0x01
	"",               //0x02
	"VBW Filter",     //0x03
   "Input Data Format", //0x04
   "",               //0x05
   "Clock Mode",               //0x06
   "SAV",               //0x07
   "Position Overflow",               //0x08
   "Black Level",               //0x09
   "HP",               //0x0A
   "VP",               //0x0B
   "",               //0x0C
   "Sync Polarity",               //0x0D
   "PMR",               //0x0E
   "",               //0x0F
   "CDR",               //0x10
   "CE",               //0x11
   "",               //0x12
   "MNE",               //0x13
   "PLLM",               //0x14
   "PLLN",               //0x15
   "",               //0x16
   "BCO",               //0x17
   "FSCI",               //0x18
   "FSCI",               //0x19
   "FSCI",               //0x1A
   "FSCI",               //0x1B
   "FSCI",               //0x1C
   "FSCI",               //0x1D
   "FSCI",               //0x1E
   "FSCI",               //0x1F
   "PLL Mem & Control",               //0x20
   "CIVC",               //0x21
   "CIV",               //0x22
   "CIV",               //0x23
   "CIV",               //0x24
   "VID",               //0x25
   "TR",               //0x26
   "TR",               //0x27
   "TR",               //0x28
   "TR",               //0x29
   "AR",               //0x2A
	};

BYTE Mask[] = {
	0xFF,	//0
	0x03, //1
	0x00,	//2
	0x7F,	//3
	0x3F,	//4
	0x00,	//5
	0x7F,	//6
	0xFF,	//7
	0x07,	//8
	0xFF, //9
	0xFF, //A
	0xFF, //B
	0x00, //C
	0x0F, //D
	0x1F, //E
	0x00, //F
	0x0F, //10
	0x07, //11
	0x00, //12
	0x1F, //13
	0xFF, //14
	0xFF, //15
	0x00, //16
	0x3F, //17
	0x0F, //18
	0x0F, //19
	0x0F, //1A
	0x0F, //1B
	0x0F, //1C
	0x0F, //1D
	0x0F, //1E
	0x0F, //1F
	0x3F, //20
	0x07, //21
	0xFF, //22
	0xFF, //23
	0xFF, //24
	0x1F, //25
	0xFF, //26
	0xFF, //27
	0xFF, //28
	0xFF, //29
	0x3F, //2A
	};


/*----------------------------------------------------------------------
Function name:  DoChrontel

Description:    Initialize i2c for the Chrontel 7003 device.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void DoChrontel(PBYTE pMap)
{
   I2CMASK I2CMask;
 
   I2CMask.pReg = (PDWORD)(pMap + I2COUT_PORT);
   I2CMask.bAddr = CH7003_ADDR;
   I2CMask.nSize = 1;
   I2CMask.bEnableBit = I2C_ENABLE;
   I2CMask.bSCLOutBit = I2C_SCL_OUT_BIT;
   I2CMask.bSDAOutBit = I2C_SDA_OUT_BIT;
   I2CMask.bSCLInBit = I2C_SCL_IN_BIT;
   I2CMask.bSDAInBit = I2C_SDA_IN_BIT;
   I2CInit(&I2CMask);

   Read_AllRegister(&I2CMask);
   Adapter(&I2CMask);
}


/*----------------------------------------------------------------------
Function name:  Read_AllRegister

Description:    Read all the i2c registers and dump the contents to
                the debug output device.
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Read_AllRegister(PI2CMASK pI2CMask)
{
	int nReg;

	start(pI2CMask);
 	send_byte(pI2CMask, (unsigned char)(pI2CMask->bAddr|MASTER_WRITE));
	send_byte(pI2CMask, (unsigned char)AUTO_INC);
	start(pI2CMask);
	send_byte(pI2CMask, (unsigned char)(pI2CMask->bAddr|MASTER_READ));
	for (nReg=0; nReg<0x2A; nReg++)
      if (Mask[nReg])
   		Debug_Printf("[%s] Register %x=%x\n", ppReg[nReg], nReg, read_byte(pI2CMask, 0x1) & Mask[nReg]);

	Debug_Printf("[%s] Register %x=%x\n", ppReg[nReg], nReg, read_byte(pI2CMask, 0x0) & Mask[nReg]);
	stop(pI2CMask);
	sda(pI2CMask, 0);
	scl(pI2CMask, 0);

	return 0;
}


/*----------------------------------------------------------------------
Function name:  Adapter

Description:    

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Adapter(PI2CMASK pI2CMask)
{
   pI2CMask->nReg = 0x0E;
   pI2CMask->dwData = 0x03;
   WriteI2CRegister(pI2CMask);
   pI2CMask->nReg = 0x10;
   pI2CMask->dwData = 0x01;
   WriteI2CRegister(pI2CMask);
   pI2CMask->nReg = 0x10;
   pI2CMask->dwData = 0x00;
   WriteI2CRegister(pI2CMask);
   pI2CMask->nReg = 0x10;
   ReadI2CRegister(pI2CMask);   
   return 0x0;
}

typedef struct
{
	FxI8 reg;
	FxU8 data;
}  CHRONTEL_REGSET;

CHRONTEL_REGSET ChrontelRegsets[][28] = { 
  // NTSC 640x480  5/6th overscan   {0}
   {{0x0e, 0x03}, // remove reset
    {0x0e, 0x0b}, // remove reset
    {0x04, 0x05}, // 12-bit multipliexed "I" scheme
    {0x00, 0x6b},
    {0x01, 0x03}, //00000011, high flicker filter
    {0x03, 0x34}, //00000000, low bandwidth
    {0x04, 0x05}, // 12-bit multipliexed "I" scheme
    {0x06, 0x40}, // cfrb=1 for slave mode; slave, neg edge, 1x clock
    {0x07, 0x8c}, // ~200 blank + 28 pipe cycles
    {0x08, 0x00}, // no overflow for SAV, HP, and VP
    {0x09, 0x7f}, // black level = 127
    {0x0a, 0x38}, // 0 horizontal shift
    {0x0b, 0xf0}, // 0 vertical shift MSB in 0:0
    {0x0d, 0x04}, // active low syncs
    {0x14, 0x59}, // M divisor
    {0x15, 0xbe}, // N divisor
    {0x21, 0x01}, // set ACIV bit to 0 to bypass the oscillator, and use ROM
    {0x18, 0x02},
    {0x19, 0x05},
    {0x1a, 0x02},
    {0x1b, 0x04},
    {0x1c, 0x09},
    {0x1d, 0x02},
    {0x1e, 0x04},
    {0x1f, 0x09},
    {-1, 0xff}},

  // NTSC 640x480  7/8th overscan   shud be NTSC 640x480 default   {1}
   {{0x0e, 0x03}, // remove reset
    {0x0e, 0x0b}, // remove reset
    {0x04, 0x05}, // 12-bit multipliexed "I" scheme
    {0x00, 0x6a},
    {0x01, 0x03}, //00000011, high flicker filter
    {0x03, 0x34}, //00000000, low bandwidth
    {0x04, 0x05}, // 12-bit multipliexed "I" scheme
    {0x06, 0x40}, // cfrb=1 for slave mode; slave, neg edge, 1x clock
    {0x07, 0x86}, // ~200 blank + 28 pipe cycles
    {0x08, 0x01}, // no overflow for SAV, HP, and VP
    {0x09, 0x7f}, // black level = 127
    {0x0a, 0x2e}, // 0 horizontal shift
    {0x0b, 0x04}, // 0 vertical shift MSB in 0:0
    {0x0d, 0x04}, // active low syncs
    {0x14, 0x3f}, // M divisor
    {0x15, 0x7e}, // N divisor
    {0x21, 0x01}, // set ACIV bit to 0 to bypass the oscillator, and use ROM
    {0x18, 0x02},
    {0x19, 0x00},
    {0x1a, 0x08},
    {0x1b, 0x00},
    {0x1c, 0x00},
    {0x1d, 0x00},
    {0x1e, 0x00},
    {0x1f, 0x00},
    {-1, 0xff}},

  // NTSC 640x480  1/1th overscan   {2}
   {{0x0e, 0x03}, // remove reset
    {0x0e, 0x0b}, // remove reset
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x00, 0x69},
	{0x01, 0x03}, //00000011, high flicker filter
	{0x03, 0x34}, //00000000, low bandwidth
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x06, 0xc0}, // cfrb=1 for slave mode; slave, neg edge, 1x clock
	{0x07, 0xe4}, // ~200 blank + 28 pipe cycles
	{0x08, 0x00}, // no overflow for SAV, HP, and VP
	{0x09, 0x7f}, // black level = 127
	{0x0a, 0x20}, // 0 horizontal shift
	{0x0b, 0x00}, // 0 vertical shift
	{0x0d, 0x04}, // active low syncs
	{0x14, 0x3f},
	{0x15, 0x6e},
	{0x21, 0x00}, // set ACIV bit to 0 to bypass the oscillator, and use ROM
	{0x18, 0x02},
	{0x19, 0x05},
	{0x1a, 0x02},
	{0x1b, 0x04},
	{0x1c, 0x09},
	{0x1d, 0x02},
	{0x1e, 0x04},
	{0x1f, 0x09},
	{-1, 0xff}},

// NTSC 800x600  5/6th overscan   THIS IS JUNK   {3}
   {{0x0e, 0x03}, // remove reset
    {0x0e, 0x0b}, // remove reset
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x00, 0x8b},
	{0x01, 0x03}, //00000011, high flicker filter
	{0x03, 0x34}, //00000000, low bandwidth
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x06, 0xc0}, // cfrb=1 for slave mode; slave, neg edge, 1x clock
	{0x07, 0xb2}, // ~200 blank + 28 pipe cycles
	{0x08, 0x00}, // no overflow for SAV, HP, and VP
	{0x09, 0x7f}, // black level = 127
	{0x0a, 0x3c}, // 0 horizontal shift
	{0x0b, 0x00}, // 0 vertical shift
	{0x0d, 0x04}, // active low syncs
	{0x13, 0x00},
	{0x14, 0x21}, // synth m
	{0x15, 0x5e}, // synth n
	{0x21, 0x01}, // set ACIV bit to 0 to bypass the oscillator, and use ROM
	{0x18, 0x01},
	{0x19, 0x0f},
	{0x1a, 0x01},
	{0x1b, 0x0c},
	{0x1c, 0x07},
	{0x1d, 0x01},
	{0x1e, 0x0c},
	{0x1f, 0x07},
	{-1, 0xff}},

// NTSC 800x600  7/10 overscan    shud be 800x600 default for NTSC   {4}
   {{0x0e, 0x03}, // remove reset
    {0x0e, 0x0b}, // remove reset
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x00, 0x8d},
	{0x01, 0x03}, //00000011, high flicker filter
	{0x03, 0x34}, //00000000, low bandwidth
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x06, 0x40}, // cfrb=1 for slave mode; slave, neg edge, 1x clock
	{0x07, 0xb2}, // ~200 blank + 28 pipe cycles
	{0x08, 0x00}, // no overflow for SAV, HP, and VP
	{0x09, 0x7f}, // black level = 127
	{0x0a, 0x3c}, // 0 horizontal shift
	{0x0b, 0xf0}, // 0 vertical shift
	{0x0d, 0x04}, // active low syncs
	{0x13, 0x02},
	{0x14, 89  }, // synth m
	{0x15, 0x2e}, // synth n
	{0x21, 0x00}, // set ACIV bit to 0 to bypass the oscillator, and use ROM
	{0x18, 0x01},
	{0x19, 0x09},
	{0x1a, 0x08},
	{0x1b, 0x0b},
	{0x1c, 0x03},
	{0x1d, 0x0a},
	{0x1e, 0x06},
	{0x1f, 0x03},
	{-1, 0xff}},

// PAL 800x600  5/6 overscan   {5}
   {{0x0e, 0x03}, // remove reset
    {0x0e, 0x0b}, // remove reset
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x00, 0x83}, // mode 20
	{0x01, 0x03}, //00000011, high flicker filter
	{0x03, 0x34}, //00000000, low bandwidth
	{0x04, 0x05}, // 12-bit multipliexed "I" scheme
	{0x06, 0xc0}, // cfrb=0 for slave mode; slave, neg edge, 1x clock
	{0x07, 0xb2}, // ~200 blank + 28 pipe cycles
	{0x08, 0x01}, // VP overflow
	{0x09, 0x7f}, // black level = 127
	{0x0a, 0x40}, // 0 horizontal shift
	{0x0b, 0x22}, // 0 vertical shift
	{0x0d, 0x04}, // active low syncs
	{0x13, 0x00},
	{0x14, 33  }, // synth m
	{0x15, 86  }, // synth n
	{0x21, 0x00}, // set ACIV bit to 0 to bypass the oscillator, and use ROM
	{0x18, 0x01},
	{0x19, 0x0f},
	{0x1a, 0x08},
	{0x1b, 0x07},
	{0x1c, 0x02},
	{0x1d, 0x08},
	{0x1e, 0x01},
	{0x1f, 0x08},
	{-1, 0xff}},

};


/*----------------------------------------------------------------------
Function name:  initChrontel

Description:    Initialize the Chrontel device.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void initChrontel (I2CMASK *i2cMask, int regset)
{
	CHRONTEL_REGSET *regmap = ChrontelRegsets[regset];

	while (regmap->reg >= 0)
	{
		i2cMask->nReg = regmap->reg;
		i2cMask->dwData = regmap->data;
		WriteI2CRegister (i2cMask);
		regmap++;
	}
}


/*----------------------------------------------------------------------
Function name:  Chrontel_SetStandard

Description:    Set as a standard TV device.

Information:

Return:         INT     0 for success,
                        TV_STANDARD_UNSUPPORTED for failure.
----------------------------------------------------------------------*/
int Chrontel_SetStandard(PTVSETSTANDARD pTvSetStandard, PDEVTABLE pDev)
{
	DWORD RegBase = pDev->RegBase;
	DWORD    dwVidInFormat;
	DWORD    dwHFrontPorchW, dwHBackPorchStart;
	DWORD    dwVFrontPorchW, dwVBackPorchStart;
	I2CMASK  I2CMask;
	PDWORD   pdwVidInFormat       = (PDWORD)(RegBase + VID_IN_FORMAT);
	PDWORD   pdwMiscInit1         = (PDWORD)(RegBase + MISC_INIT1);
	PDWORD   pdwVidInXDecimDeltas = (PDWORD)(RegBase + VID_IN_DECIMX_DELTA);
	PDWORD   pdwVidInYDecimDeltas = (PDWORD)(RegBase + VID_IN_DECIMY_DELTA);
	FxU8 isNtsc = !!(pTvSetStandard->dwStandard & VP_TV_STANDARD_NTSC_M);
	FxU16 xres = (FxU16) ((SstIORegs *)RegBase)->vidScreenSize & 0xfff;

	I2CMask = I2C_PROTO;             //Setup the I2C bitmasks
	I2CMask.pReg = (PDWORD)(RegBase + I2COUT_PORT);
	I2CInit(&I2CMask);               //Enable I2C
	I2CMask.bAddr = CH7003_ADDR;
	I2CMask.nSize = 1;
	stop(&I2CMask);                  //Reset devices

	if (isNtsc)
	   initChrontel (&I2CMask, xres == 640 ? 1 : 4);
	else
	   initChrontel (&I2CMask, xres == 640 ? 1 : 5);

	dwVidInFormat = *pdwVidInFormat & ~H3_VMI_MODE_MASK;
	//Setup Banshee as slave for TV-Out operation mode
	dwVidInFormat |= SST_VIDEOIN_VSYNC_POLARITY_LOW |
					SST_VIDEOIN_HSYNC_POLARITY_LOW |
					SST_VIDEOIN_G4_FOR_POSEDGE     |
					SST_VIDEOIN_GENLOCK_ENABLE     |
					SST_VIDEOIN_NOT_USE_VGA_TIMING |
					H3_VMI_MODE_TV                 |
					0;

	*pdwVidInFormat = dwVidInFormat;
	*pdwMiscInit1 = (*pdwMiscInit1 & ~0xe0000000) | 0x70000000; //TV-Out Clk dly
   
	if (xres == 640)
	{
		dwHFrontPorchW = 100;
		dwHBackPorchStart = 640 + dwHFrontPorchW;
		dwVFrontPorchW = 74;
		dwVBackPorchStart = 480 + dwVFrontPorchW;
	}
	else
	{
		dwHFrontPorchW = 144;
		dwHBackPorchStart = 800 + dwHFrontPorchW;
		dwVFrontPorchW = 44;
		dwVBackPorchStart = 600 + dwVFrontPorchW;
	}

	*pdwVidInXDecimDeltas = (dwHBackPorchStart << 16) | dwHFrontPorchW;
	*pdwVidInYDecimDeltas = (dwVBackPorchStart << 16) | dwVFrontPorchW;
 
	return(0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_Disable

Description:    Disable the Chrontel device.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_Disable(DWORD RegBase)
{
	PDWORD   pdwVidInFormat = (PDWORD)(RegBase + VID_IN_FORMAT);
	I2CMASK  I2CMask;

	*pdwVidInFormat = 0;
	I2CMask = I2C_PROTO;             //Setup the I2C bitmasks
	I2CMask.pReg = (PDWORD)(RegBase + I2COUT_PORT);
	*I2CMask.pReg = I2C_INITVAL;
	I2CInit(&I2CMask);               //Enable I2C
	I2CMask.bAddr = CH7003_ADDR;
	I2CMask.nSize = 1;
	stop(&I2CMask);                  //Reset devices

	I2CMask.nReg = 0x0e;
	I2CMask.dwData = 0x03;
	WriteI2CRegister (&I2CMask);
	I2CMask.nReg = 0x0e;
	I2CMask.dwData = 0x0b;
	WriteI2CRegister (&I2CMask);

	return(0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetStatus

Description:    Get the status of the Chrontel device.

Information:

Return:         INT     FXTRUE if success,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
int Chrontel_GetStatus (PDEVTABLE pDev, int estatus)
{
	DWORD RegBase = pDev->RegBase;
	I2CMASK  I2CMask;

	I2CMask = I2C_PROTO;             //Setup the I2C bitmasks
	I2CMask.pReg = (PDWORD)(RegBase + I2COUT_PORT);
	*I2CMask.pReg = I2C_INITVAL;
	I2CInit(&I2CMask);               //Enable I2C
	I2CMask.bAddr = CH7003_ADDR;
	I2CMask.nSize = 1;

	I2CMask.nReg = 0x0e;
	I2CMask.dwData = 0x03;
	if (FXFALSE == WriteI2CRegister (&I2CMask))
		return (-1);

	I2CMask.nReg = 0x10;
	I2CMask.dwData = 0x01;
	if (FXFALSE == WriteI2CRegister (&I2CMask))
		return (-1);

	I2CMask.nReg = 0x10;
	I2CMask.dwData = 0x00;
	if (FXFALSE == WriteI2CRegister (&I2CMask))
		return (-1);
	if (FXFALSE == ReadI2CRegister (&I2CMask))
		return (-1);

	return (FXTRUE);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetPosition

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetPosition (PTVCURPOS pTvCurPos, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_SetPicControl

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_SetPicControl(PTVSETCAP pTvSetPicControl, PDEVTABLE pDev)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_SetPosition

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_SetPosition(PTVSETPOS pTvSetPosition, PDEVTABLE pDev)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_SetSize

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_SetSize(PTVSETSIZE pTvSetSize, PDEVTABLE pDev)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_SetSpecial

Description:    Pass down to BT868_SetSpecial.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_SetSpecial(PTVSETSPECIAL pTvSetSpecial, DWORD RegBase)
{
	return (BT868_SetSpecial (pTvSetSpecial, RegBase));
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetPicControl

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetPicControl (PTVCURCAP pTvCurCap, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetFilterControl

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetFilterControl (PTVCURCAP tvPicValue, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetSizeControl

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetSizeControl (PTVCURSIZE tvPicValue, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_Enable

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_Enable (PDEVTABLE pDev, int vgaMode)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetStandard

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetStandard (PDEVTABLE pDev, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetSizeCap

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetSizeCap (PDEVTABLE pDev, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetPosCap

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetPosCap (PDEVTABLE pDev, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetFilterCap

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetFilterCap (PDEVTABLE pDev, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_GetPicCap

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_GetPicCap (PDEVTABLE pDev, void *tvOutData)
{
	return (0);
}


/*----------------------------------------------------------------------
Function name:  Chrontel_CopyProtect

Description:    Function is currently empty.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int Chrontel_CopyProtect (PDEVTABLE pDev, int flags)
{
	return (0);
}


#if 1

// We'll need this when we have more than one encoder to support

const TVOUT_DEV_CALLS ChrontelDevCalls = {
	Chrontel_GetStatus,
	Chrontel_GetPosition,
	Chrontel_SetStandard,
	Chrontel_SetPicControl,
	Chrontel_SetPosition,
	Chrontel_SetSize,
	Chrontel_SetSpecial,
	Chrontel_Disable,
	Chrontel_GetPicControl,
	Chrontel_GetFilterControl,
	Chrontel_GetSizeControl,
	Chrontel_Enable,
    Chrontel_GetStandard,
    Chrontel_GetSizeCap,
    Chrontel_GetPosCap,
    Chrontel_GetFilterCap,
    Chrontel_GetPicCap,
	Chrontel_CopyProtect
};



#endif




