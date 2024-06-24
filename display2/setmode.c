/* $Header: setmode.c, 15, 11/5/00 3:11:34 PM PST, Ryan Bissell$ */
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
** File name:   setmode.c
**
** Description: This file contains the device specific code for
**              setting and verifying the physical modes.  It also
**              contains the list of all possible supported modes.
**
** $Log: 
**  15   3dfx      1.14        11/5/00  Ryan Bissell    fixed incorrect 
**       calculation of "memRequired"
**  14   3dfx      1.13        11/1/00  Ryan Bissell    Cleanup of ddmemmgr.c, 
**       and related files.
**  13   3dfx      1.12        10/5/00  Dan O'Connel    Correct code which reads
**       or sets PllCtrl0 and PllCtrl1 registers.  Most of this code was 
**       originally incompletely ported from Napalm.  Also cleanup (mostly by 
**       removing) some other unused or obsolete code concerning Alternate 
**       Timings for DFPs and SGRAMMODE.
**  12   3dfx      1.11        8/31/00  John Zhang      Remove dwBitsPerPixel 
**       form FXSURFACEDATA.
**  11   3dfx      1.10        8/30/00  John Zhang      Fixed for bitmapcache
**  10   3dfx      1.9         8/21/00  Geoff Bullard   Porting necessary fixes 
**       and cleanup from Napalm 9x: better test for mode in range, HWSetMode().
**  9    3dfx      1.8         7/17/00  Dan O'Connel    Delete obsolete code 
**       that was ifdef'ed or commented out in previous checkin.
**  8    3dfx      1.7         7/17/00  Dan O'Connel    Major changes ported 
**       from Napalm driver to support: Registry Controlled Modes, DFP, TvOut, 
**       read OEM config from BIOS, updated 3dfx Tools support, and other 
**       features and bug fixes.
**  7    3dfx      1.6         12/22/99 Ryan Bissell    New clut management code
**  6    3dfx      1.5         11/16/99 Ryan Bissell    Added 1920x1080i for 
**       16bpp and 32bpp.
**  5    3dfx      1.4         11/5/99  Andrew Sobczyk  Fixed a bug where we 
**       were not using CAM entry #1
**  4    3dfx      1.3         11/2/99  Ryan Bissell    Integration of ds/di I2C
**       code, plus power management changes, and support for interlaced desktop
**       modes.
**  3    3dfx      1.2         10/20/99 Andrew Sobczyk  Added code to add a 
**       entry to the CAM to allow SLI/dual chips to work.
**  2    3dfx      1.1         10/6/99  Mark Einkauf    init 
**       pVpc->desktopSurface.scanlinedouble = 0;
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
**
*/


/*==========================================================================;
 *  public functions:
 *      HWSetMode
 *      HWTestMode
 *      HWSetPalette
 *      HWBeginAccess
 *      HWEndAccess
 *
 *  public data:
 *      ModeList
 ***************************************************************************/

#include "sst2.h"
#include "header.h"
#include "sst2glob.h"

#define Not_VxD
#include "minivdd.h"
#include "modelist.h"
#include "cursor.h"
//#include <entrleav.h>
#include "dfpapi.h"
//extern void FXWAITFORIDLE();
//extern int videosetmode(int, int);

extern MODEINFO FAR * ModeList;
extern int nNumModes;
extern UINT GetFlatSel(void);
extern DWORD PLL2MHz(DWORD clock );
//DWORD grxFreq = 0;
//DWORD grxClock = 0;

int bScanlineDouble = 0x0;   // fixup cursor positions if SCANLINE_DBL set

#define DPMS_MASK (SST_DAC_DPMS_ON_HSYNC | SST_DAC_DPMS_ON_VSYNC | SST_DAC_FORCE_VSYNC | SST_DAC_FORCE_HSYNC)

/*
DWORD GammaTable[] = {
	 0x00000000, // 0
	 0x00010101, // 1
	 0x00020202, // 2
	 0x00030303, // 3
	 0x00040404, // 4
	 0x00050505, // 5
	 0x00060606, // 6
	 0x00070707, // 7
	 0x00080808, // 8
	 0x00090909, // 9
	 0x000a0a0a, // 10
	 0x000b0b0b, // 11
	 0x000c0c0c, // 12
	 0x000d0d0d, // 13
	 0x000e0e0e, // 14
	 0x000f0f0f, // 15
	 0x00101010, // 16
	 0x00111111, // 17
	 0x00121212, // 18
	 0x00131313, // 19
	 0x00141414, // 20
	 0x00151515, // 21
	 0x00161616, // 22
	 0x00171717, // 23
	 0x00181818, // 24
	 0x00191919, // 25
	 0x001a1a1a, // 26
	 0x001b1b1b, // 27
	 0x001c1c1c, // 28
	 0x001d1d1d, // 29
	 0x001e1e1e, // 30
	 0x001f1f1f, // 31
	 0x00202020, // 32
	 0x00212121, // 33
	 0x00222222, // 34
	 0x00232323, // 35
	 0x00242424, // 36
	 0x00252525, // 37
	 0x00262626, // 38
	 0x00272727, // 39
	 0x00282828, // 40
	 0x00292929, // 41
	 0x002a2a2a, // 42
	 0x002b2b2b, // 43
	 0x002c2c2c, // 44
	 0x002d2d2d, // 45
	 0x002e2e2e, // 46
	 0x002f2f2f, // 47
	 0x00303030, // 48
	 0x00313131, // 49
	 0x00323232, // 50
	 0x00333333, // 51
	 0x00343434, // 52
	 0x00353535, // 53
	 0x00363636, // 54
	 0x00373737, // 55
	 0x00383838, // 56
	 0x00393939, // 57
	 0x003a3a3a, // 58
	 0x003b3b3b, // 59
	 0x003c3c3c, // 60
	 0x003d3d3d, // 61
	 0x003e3e3e, // 62
	 0x003f3f3f, // 63
	 0x00404040, // 64
	 0x00414141, // 65
	 0x00424242, // 66
	 0x00434343, // 67
	 0x00444444, // 68
	 0x00454545, // 69
	 0x00464646, // 70
	 0x00474747, // 71
	 0x00484848, // 72
	 0x00494949, // 73
	 0x004a4a4a, // 74
	 0x004b4b4b, // 75
	 0x004c4c4c, // 76
	 0x004d4d4d, // 77
	 0x004e4e4e, // 78
	 0x004f4f4f, // 79
	 0x00505050, // 80
	 0x00515151, // 81
	 0x00525252, // 82
	 0x00535353, // 83
	 0x00545454, // 84
	 0x00555555, // 85
	 0x00565656, // 86
	 0x00575757, // 87
	 0x00585858, // 88
	 0x00595959, // 89
	 0x005a5a5a, // 90
	 0x005b5b5b, // 91
	 0x005c5c5c, // 92
	 0x005d5d5d, // 93
	 0x005e5e5e, // 94
	 0x005f5f5f, // 95
	 0x00606060, // 96
	 0x00616161, // 97
	 0x00626262, // 98
	 0x00636363, // 99
	 0x00646464, // 100
	 0x00656565, // 101
	 0x00666666, // 102
	 0x00676767, // 103
	 0x00686868, // 104
	 0x00696969, // 105
	 0x006a6a6a, // 106
	 0x006b6b6b, // 107
	 0x006c6c6c, // 108
	 0x006d6d6d, // 109
	 0x006e6e6e, // 110
	 0x006f6f6f, // 111
	 0x00707070, // 112
	 0x00717171, // 113
	 0x00727272, // 114
	 0x00737373, // 115
	 0x00747474, // 116
	 0x00757575, // 117
	 0x00767676, // 118
	 0x00777777, // 119
	 0x00787878, // 120
	 0x00797979, // 121
	 0x007a7a7a, // 122
	 0x007b7b7b, // 123
	 0x007c7c7c, // 124
	 0x007d7d7d, // 125
	 0x007e7e7e, // 126
	 0x007f7f7f, // 127
	 0x00808080, // 128
	 0x00818181, // 129
	 0x00828282, // 130
	 0x00838383, // 131
	 0x00848484, // 132
	 0x00858585, // 133
	 0x00868686, // 134
	 0x00878787, // 135
	 0x00888888, // 136
	 0x00898989, // 137
	 0x008a8a8a, // 138
	 0x008b8b8b, // 139
	 0x008c8c8c, // 140
	 0x008d8d8d, // 141
	 0x008e8e8e, // 142
	 0x008f8f8f, // 143
	 0x00909090, // 144
	 0x00919191, // 145
	 0x00929292, // 146
	 0x00939393, // 147
	 0x00949494, // 148
	 0x00959595, // 149
	 0x00969696, // 150
	 0x00979797, // 151
	 0x00989898, // 152
	 0x00999999, // 153
	 0x009a9a9a, // 154
	 0x009b9b9b, // 155
	 0x009c9c9c, // 156
	 0x009d9d9d, // 157
	 0x009e9e9e, // 158
	 0x009f9f9f, // 159
	 0x00a0a0a0, // 160
	 0x00a1a1a1, // 161
	 0x00a2a2a2, // 162
	 0x00a3a3a3, // 163
	 0x00a4a4a4, // 164
	 0x00a5a5a5, // 165
	 0x00a6a6a6, // 166
	 0x00a7a7a7, // 167
	 0x00a8a8a8, // 168
	 0x00a9a9a9, // 169
	 0x00aaaaaa, // 170
	 0x00ababab, // 171
	 0x00acacac, // 172
	 0x00adadad, // 173
	 0x00aeaeae, // 174
	 0x00afafaf, // 175
	 0x00b0b0b0, // 176
	 0x00b1b1b1, // 177
	 0x00b2b2b2, // 178
	 0x00b3b3b3, // 179
	 0x00b4b4b4, // 180
	 0x00b5b5b5, // 181
	 0x00b6b6b6, // 182
	 0x00b7b7b7, // 183
	 0x00b8b8b8, // 184
	 0x00b9b9b9, // 185
	 0x00bababa, // 186
	 0x00bbbbbb, // 187
	 0x00bcbcbc, // 188
	 0x00bdbdbd, // 189
	 0x00bebebe, // 190
	 0x00bfbfbf, // 191
	 0x00c0c0c0, // 192
	 0x00c1c1c1, // 193
	 0x00c2c2c2, // 194
	 0x00c3c3c3, // 195
	 0x00c4c4c4, // 196
	 0x00c5c5c5, // 197
	 0x00c6c6c6, // 198
	 0x00c7c7c7, // 199
	 0x00c8c8c8, // 200
	 0x00c9c9c9, // 201
	 0x00cacaca, // 202
	 0x00cbcbcb, // 203
	 0x00cccccc, // 204
	 0x00cdcdcd, // 205
	 0x00cecece, // 206
	 0x00cfcfcf, // 207
	 0x00d0d0d0, // 208
	 0x00d1d1d1, // 209
	 0x00d2d2d2, // 210
	 0x00d3d3d3, // 211
	 0x00d4d4d4, // 212
	 0x00d5d5d5, // 213
	 0x00d6d6d6, // 214
	 0x00d7d7d7, // 215
	 0x00d8d8d8, // 216
	 0x00d9d9d9, // 217
	 0x00dadada, // 218
	 0x00dbdbdb, // 219
	 0x00dcdcdc, // 220
	 0x00dddddd, // 221
	 0x00dedede, // 222
	 0x00dfdfdf, // 223
	 0x00e0e0e0, // 224
	 0x00e1e1e1, // 225
	 0x00e2e2e2, // 226
	 0x00e3e3e3, // 227
	 0x00e4e4e4, // 228
	 0x00e5e5e5, // 229
	 0x00e6e6e6, // 230
	 0x00e7e7e7, // 231
	 0x00e8e8e8, // 232
	 0x00e9e9e9, // 233
	 0x00eaeaea, // 234
	 0x00ebebeb, // 235
	 0x00ececec, // 236
	 0x00ededed, // 237
	 0x00eeeeee, // 238
	 0x00efefef, // 239
	 0x00f0f0f0, // 240
	 0x00f1f1f1, // 241
	 0x00f2f2f2, // 242
	 0x00f3f3f3, // 243
	 0x00f4f4f4, // 244
	 0x00f5f5f5, // 245
	 0x00f6f6f6, // 246
	 0x00f7f7f7, // 247
	 0x00f8f8f8, // 248
	 0x00f9f9f9, // 249
	 0x00fafafa, // 250
	 0x00fbfbfb, // 251
	 0x00fcfcfc, // 252
	 0x00fdfdfd, // 253
	 0x00fefefe, // 254
	 0x00ffffff, // 255
   };
*/


/***************************************************************************
 *
 * BiosMode
 *
 * this is a table the tells us what BIOS mode to use, given a mode number
 * why do we have two tables? because we give a pointer to ModeList
 * to DirectDraw and it has to be a specific format....
 *
 ***************************************************************************/

UINT BiosMode[] = {
    0x13,            //0
    0x00,            //1
    0x00,            //2
    0x00,            //3
    0x101,            //4 640 x 8
    0x103,            //5 800 x 8
    0x105,           //6 1024 x 8
    0x107,           //7 1280 x 8
    0x00,            //8
    0x00,            //9
    0x00,            //10
    0x00,            //11
    0x00,            //12
    0x00,            //13
    0x00,            //14
    0x00,            //15
    0x00,            //16
    0x00,            //17
    0x00,            //18
    0x00,            //19
    0x00,            //20
    0x00,            //21
    0x00,            //22
    0x111,           //23 640 x 16
    0x114,           //24 800 x 16
    0x117,           //25 1024 x 16
    0x00,            //15
    0x00,            //16
    0x00,            //17
    0x00,            //18
    0x00,            //19
    0x112,           //20 640 x 24
    0x115,           //20 800 X 24
    0x118,           //20 1024 x 24
    0x00,            //20
    0x00,            //20
    0x00,            //20
};


/*----------------------------------------------------------------------
Function name:  HWTestMode

Description:    This function is called to verify a mode can be done.

Information:
    This function checks to see if there is enough VRAM,
    the hardware is capable and present, and any thing else.

    This function needs to be "safe" it can't disturb the
    hardware or change the current display mode.

Return:         BOOL    TRUE for success, FALSE for failure.
----------------------------------------------------------------------*/
BOOL HWTestMode(int ModeNumber)
{
    MODEINFO *pMode;
    FxU32 memRequired;

    if (ModeNumber < 0 || ModeNumber >= nNumModes)
        return FALSE;

    pMode = &ModeList[ModeNumber];

    if (_FF(TotalVRAM) == 0)
    {
	//
	// we're here because windows is querying a mode w/out having
	// actually loaded the full DRV on the h/w, so let's set our
	// memory size to the max possible, to "allow" all the potentially
	// possible modes
	//
        _FF(TotalVRAM) = 16L*1024L*1024L;
    }

    if (_FF(TotalVRAM) == 0)
    {
        // cant determine the VRAM size.
        return FALSE;
    }

    memRequired =  ALIGN_4KB((_FF(gdiDesktopStart) + (pMode->lPitch * pMode->dwHeight)));
    memRequired += _FF(mmPersistentHeapSize);

    if (memRequired > _FF(TotalVRAM))
    {
        // not enough VRAM.
        return FALSE;
    }

    //
    //  ********************* INSERT CODE HERE ***********************
    //
    // because the is a sample we just fake it, you need better mode
    // vaidation.

    //  Likewise for the sst1 preliminary driver on with s3 card. ag

    //
    // ********************* INSERT CODE HERE ***********************
    //

    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  bppToPixfmt

Description:    Translate a bits per pixel into a video processor
                desktop pixel format setting.
Information:

Return:         FxU32   The pixel format setting or,
                        0 for failure.
----------------------------------------------------------------------*/
FxU32 bppToPixfmt(FxU32 bpp)
{
    switch (bpp)
    {
    case 8:
	    return SST_VD_8BPP;
    case 16:
	    return SST_VD_RGB565;
    case 32:
	    return SST_VD_RGB32;
    default:
	    DPF(DBGLVL_NORMAL, "bppToPixfmt: bad bpp value: %d\n", bpp);
	    return 0;
    }
}

extern DWORD dwDeviceHandle;

/*----------------------------------------------------------------------
Function name:  HWSetMode

Description:    This function is called to set the mode.
                
Information:
    Only a mode that as passed HWTestMode will be get this far.

    This function needs to fill out the following globals.

    If the mode is a linear mode
        DriverData.TotalVRAM        ; total amount of VRAM
        DriverData.ScreenAddress    ; *physical* address of frame buffer
        DriverData.ScreenSel        ; zero

    If the mode is a banked mode
        DriverData.TotalVRAM        ; total amount of VRAM
        DriverData.ScreenAddress    ; zero
        DriverData.ScreenSel        ; selector to VFlatD virtual frame buffer.

Return:         BOOL    TRUE for success, FALSE for failure
----------------------------------------------------------------------*/
#pragma optimize("", off)
void VX900T_Fixup(int ModeNumber);

BOOL HWSetMode(int ModeNumber)
{
    VidProcConfig vpc;
    VidProcConfig *pVpc = &vpc;
    DWORD dwBPP;
    DWORD dwPByteStride;
    DWORD dwMStride;
    DWORD dwL2MStride;
#ifdef napalmCode
	DWORD dacMode;
	DWORD dpmsBits;

    // ensure only modes with SCANLINE_DBL bit set turns this on
    bScanlineDouble = 0x0;    // no cursor positions fixup
#endif  //def napalmCode

    // mode number -1 means restore the video mode present when windows
    // was booting (VGA mode 3 is assumed here)
    if (ModeNumber == -1)
    {

        _asm mov ax,3       ;; go back to text mode.
        _asm int 10h
        return TRUE;
    }

    //
    // verify that the mode number is in range
    //
    if (ModeNumber < 0 || ModeNumber >= nNumModes)
        return FALSE;

    VDDCall(VDD_PRE_MODE_CHANGE, dwDeviceHandle, 0, 0, 0);

    pVpc->changeVideoMode = 1;
    pVpc->width = ModeList[ModeNumber].dwWidth;
    pVpc->height  = ModeList[ModeNumber].dwHeight;
    pVpc->refresh = ModeList[ModeNumber].wVert;

   // @RBISSELL, I've removed the "_FF(TvoActive)" check from the following
   //            if statement, because the Voodoo3 is actually in slave mode,
   //            so that the bt868 is providing the signals.  As such, it really
   //            doesn't matter what we think the refresh rate is for TVOUT.
   //            As it was, this was keeping us from displaying certain modes
   //            on the TV, because we didn't have 60Hz versions of those modes
   //            in the table.
	
	//=======================================================================================
	// DYNAMIC MODE TABLE begins
	//=======================================================================================

    if (DFPisPanelActive() && 
        (ModeList[ModeNumber].dwDFPWidth != 0))
    {
        pVpc->TimingParams.width  = ModeList[ModeNumber].dwDFPWidth;
        pVpc->TimingParams.height = ModeList[ModeNumber].dwDFPHeight;
    }
    else
    {
	    pVpc->TimingParams.width = ModeList[ModeNumber].dwWidth;
	    pVpc->TimingParams.height = ModeList[ModeNumber].dwHeight;
    }
	pVpc->TimingParams.refresh = pVpc->refresh;

	pVpc->TimingParams.HTotal = ModeList[ModeNumber].HTotal;
	pVpc->TimingParams.HSyncStart = ModeList[ModeNumber].HSyncStart;
	pVpc->TimingParams.HSyncEnd = ModeList[ModeNumber].HSyncEnd;
	pVpc->TimingParams.VTotal = ModeList[ModeNumber].VTotal;
	pVpc->TimingParams.VSyncStart = ModeList[ModeNumber].VSyncStart;
	pVpc->TimingParams.VSyncEnd = ModeList[ModeNumber].VSyncEnd;
	pVpc->TimingParams.CRTCflags = ModeList[ModeNumber].CRTCflags;
	pVpc->TimingParams.PixelClock = ModeList[ModeNumber].PixelClock;
	pVpc->TimingParams.CharWidth = ModeList[ModeNumber].CharWidth;
	pVpc->TimingParams.UseGTF = ModeList[ModeNumber].UseGTF;

    if (ModeList[ModeNumber].dwFlags & INTERLACED) 
	{
		// Half the vertical timings for interlaced modes
		// NOTE: This is not fully tested as the Voodoo3 HW does not support interlaced
		pVpc->TimingParams.VTotal /= 2;
		pVpc->TimingParams.VSyncStart /= 2;
		pVpc->TimingParams.VSyncEnd /= 2;
	}

	//=======================================================================================
	// DYNAMIC MODE TABLE ends
	//=======================================================================================

#ifdef napalmCode
    if (ModeList[ModeNumber].dwFlags & SCANLINE_DBL)
    {
       bScanlineDouble = CURSOR_DBL_Y; // fixup cursor positions for this mode
	}
#endif   //def napalmCode

    // for now, always enable this mode as the desktop, and disable the
    // overlay
    //
    pVpc->changeDesktop = 1;
    pVpc->desktopSurface.enable = 1;
	pVpc->desktopSurface.scanlinedouble = !!(ModeList[ModeNumber].dwFlags & SCANLINE_DBL);
	pVpc->desktopSurface.interlacedmode = !!(ModeList[ModeNumber].dwFlags & INTERLACED);

    // Determine whether or not the primary is allocated tiled
    // Note: the overrides are done explicitly in case the default
    // allocation behavior is changed.

    pVpc->desktopSurface.tiled = 1;         // the default behavior


    if (_FF(bPrimaryAlwaysTiled))
        pVpc->desktopSurface.tiled = 1;

    if (_FF(bPrimaryAlwaysLinear))
        pVpc->desktopSurface.tiled = 0;

    if (pVpc->desktopSurface.tiled)
        _FF(ddPrimaryInTile)=1;
    
    dwBPP = ModeList[ModeNumber].dwBPP;

    switch (dwBPP)
    {
    case 8:
        pVpc->desktopSurface.clutBypass = 0;
        _FF(dwRBitMask) = 0x00000000L;
        _FF(dwGBitMask) = 0x00000000L;
        _FF(dwBBitMask) = 0x00000000L;
        break;
    case 16:
        pVpc->desktopSurface.clutBypass = 0;
        _FF(dwRBitMask) = 0x0000F800L;
        _FF(dwGBitMask) = 0x000007E0L;
        _FF(dwBBitMask) = 0x0000001FL;
        break;
    case 32:
        pVpc->desktopSurface.clutBypass = 0;
        _FF(dwRBitMask) = 0x00FF0000L;
        _FF(dwGBitMask) = 0x0000FF00L;
        _FF(dwBBitMask) = 0x000000FFL;
        break;
    default:
        __asm int 3;
        break;
    }
           
    pVpc->desktopSurface.pixFmt = bppToPixfmt(dwBPP);   
    pVpc->desktopSurface.clutSelect = 0;
    pVpc->desktopSurface.startAddress = _FF(gdiDesktopStart);

    if (pVpc->desktopSurface.tiled)
    {
        // round up to next tile boundary
        dwPByteStride = (ModeList[ModeNumber].lPitch + 31) & ~31;
    }
    else
    {
        // round up to next tile boundary
        dwPByteStride = (ModeList[ModeNumber].lPitch + 255) & ~255;
    }

    dwMStride = 1;
    dwL2MStride = 0;

    while (dwMStride < dwPByteStride) 
    {
        dwL2MStride++;
        dwMStride *= 2;
    }

    // convert to tile units
    _FF(ddTileStride) = dwPByteStride >> SST2_TILE_WIDTH_BITS;
    _FF(ddTilePitch) = dwMStride;
    pVpc->desktopSurface.stride = pVpc->desktopSurface.tiled ? _FF(ddTileStride) : dwPByteStride;

    // right now, disable the overlay surface
    //
    pVpc->changeOverlay = 1;
    pVpc->overlaySurface.enable = 0;
    pVpc->overlaySurface.stereo	= 0;
    pVpc->overlaySurface.horizScaling = 0;
    pVpc->overlaySurface.dudx = 0;
    pVpc->overlaySurface.verticalScaling = 0;
    pVpc->overlaySurface.dvdy = 0;
    pVpc->overlaySurface.filterMode = 0;
    pVpc->overlaySurface.tiled = 0;
    pVpc->overlaySurface.pixFmt	= 0;
    pVpc->overlaySurface.clutBypass = 0;
    pVpc->overlaySurface.clutSelect = 0;
    pVpc->overlaySurface.startAddress = 0;
    pVpc->overlaySurface.stride	= 0;

#ifdef NapalmCode
	dacMode = GET(lph3IORegs->dacMode);
#endif  //def NapalmCode
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_SET_VIDEO_MODE, 0, pVpc);
    VX900T_Fixup(ModeNumber);

#ifdef NapalmCode
	// Imhoff - Fix PRS 5469... Changing resolutions overrides the "Disable Monitor"
	// checkbox in the 3dfx TV control panel... To fix this, we simply save and 
	// restore the state of the DPMS bits before and after the modeset.
	if ( _FF(dwTvoActive) )
	{
		dpmsBits = dacMode & DPMS_MASK;
		dacMode = GET(lph3IORegs->dacMode);
		dacMode &= ~DPMS_MASK;
		dacMode |= dpmsBits;
   		SETDW(lph3IORegs->dacMode, dacMode);
	}
#endif  //def NapalmCode

    //Save desktop's info for DDraw and other usage later:
    _FF(dwWidth) = ModeList[ModeNumber].dwWidth;
    _FF(dwHeight) = ModeList[ModeNumber].dwHeight;
    _FF(dwRefreshRate) = ModeList[ModeNumber].wVert;


    _FF(ddPrimarySurfaceData).hwPtr           = pVpc->desktopSurface.startAddress; //Same as CAM0's physical base
    _FF(ddPrimarySurfaceData).lfbPtr          = pVpc->desktopSurface.startAddress+_FF(LFBBASE); 
    _FF(ddPrimarySurfaceData).endlfbPtr       = _FF(ddPrimarySurfaceData).lfbPtr + dwMStride * ModeList[ModeNumber].dwHeight;
    _FF(ddPrimarySurfaceData).dwPixelFormat   = pVpc->desktopSurface.pixFmt;
    _FF(ddPrimarySurfaceData).dwBytesPerPixel = ModeList[ModeNumber].dwBPP >> 3;
    _FF(ddPrimarySurfaceData).dwMStride       = dwMStride;
    _FF(ddPrimarySurfaceData).dwL2MStride     = dwL2MStride;
    _FF(ddPrimarySurfaceData).dwPStride       = pVpc->desktopSurface.tiled ? dwPByteStride/32 : dwPByteStride;
    _FF(ddPrimarySurfaceData).dwStride        = dwPByteStride;
    _FF(ddPrimarySurfaceData).tileFlag        = pVpc->desktopSurface.tiled ? (MEM_AT_16K | MEM_IN_TILE1) : MEM_IN_LINEAR;

   _FF(EndCamEntry) = 0;

#ifdef SLI
   // Put a entry in the CAM for the low memory stuff which is needed in SLI modes
   if (_FF(dwNumUnits) > 1)
      {
       // program CAM for DIB engine access of desktop
       SETDW(lph3agp->cam[_FF(EndCamEntry)].baseAddress,  0x0);
       SETDW(lph3agp->cam[_FF(EndCamEntry)].endAddress,   pVpc->desktopSurface.startAddress);
       SETDW(lph3agp->cam[_FF(EndCamEntry)].physicalBase, SST_CAM_EN_PURE_LINEAR); 
       SETDW(lph3agp->cam[_FF(EndCamEntry)].strideState,
                              ((0x01L) << SST_CAM_LFB_DEPTH_SHIFT) |
                              ((0x01L) << SST_CAM_PIXEL_DEPTH_SHIFT) |
                              (SST_CAM_LINEAR) |
                              (0x05L << SST_CAM_MSTRIDE_SHIFT) |   // stride for host mem
                              (0x20L << SST_CAM_PSTRIDE_SHIFT));     // stride for physical buffer 
      _FF(EndCamEntry)++;
      }
#endif

    // program CAM for DIB engine access of desktop
    SETDW(lph3agp->cam[_FF(EndCamEntry)].baseAddress,  pVpc->desktopSurface.startAddress);
    SETDW(lph3agp->cam[_FF(EndCamEntry)].endAddress,   pVpc->desktopSurface.startAddress + dwMStride*ModeList[ModeNumber].dwHeight);
    SETDW(lph3agp->cam[_FF(EndCamEntry)].physicalBase, pVpc->desktopSurface.startAddress); 
    SETDW(lph3agp->cam[_FF(EndCamEntry)].strideState,
                           ((dwBPP>>3) << SST_CAM_LFB_DEPTH_SHIFT) |
                           ((dwBPP>>3) << SST_CAM_PIXEL_DEPTH_SHIFT) |
                           (pVpc->desktopSurface.tiled ? SST_CAM_TILE_MODE1 : SST_CAM_LINEAR) |
                           (dwL2MStride << SST_CAM_MSTRIDE_SHIFT) |   // stride for host mem
                           (_FF(ddPrimarySurfaceData.dwPStride) << SST_CAM_PSTRIDE_SHIFT));     // stride for physical buffer 

    return TRUE;              
}

#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  HWBeginAccess

Description:    This function is called to make sure the framebuffer
                can be written to if your hardware has blitter mode
                and a framebuffer mode, this function needs to wait
                for the blitter to finish and enable framebufffer
                mode.
                
Information:    This routine is empty!  BeginAccess handled
                elsewhere.

Return:         VOID
----------------------------------------------------------------------*/
void HWBeginAccess()
{
    //
    // ********************* INSERT CODE HERE ***********************
    //
    // because the is a sample we just fake it, or if your hardware
    // does not need to do anything you can fake it too.

    //
    // ********************* INSERT CODE HERE ***********************
    //
}


/*----------------------------------------------------------------------
Function name:  HWEndAccess

Description:    Compliment of HWBeginAccess.
                
Information:    This routine is empty!  EndAccess handled
                elsewhere.

Return:         VOID
----------------------------------------------------------------------*/
void HWEndAccess()
{
    //
    // ********************* INSERT CODE HERE ***********************
    //
    // because the is a sample we just fake it, or if your hardware
    // does not need to do anything you can fake it too.

    //
    // ********************* INSERT CODE HERE ***********************
    //
}

/**** DELETE ME? ******
#define FRACMULT (10000L)
#define RANGE(val, low, hi) ((low <= val) && (val <= hi))
#define EPILSON (10)
#define SUB(a,b) (((a) > (b)) ? (a)-(b) : (b)-(a))
***********************/

/*

int HWSetPalette(int start, int count, DWORD FAR* colors)
{
  int i;
  WORD wRed;
  WORD wBlue;
  WORD wGreen;
  FxU32 param;
  FxU32 color[256];

  if (ModeList[_FF(ModeNumber)].dwBPP > 8)  //this should only be used in palettized modes.
    return 0;

  // This fixes 5866 Problem with Turok
  if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
    return TRUE;

  for (i = start; i < (start + count); i++)
  {
    color[i] = colors[i-start];

    if (8 == ModeList[_FF(ModeNumber)].dwBPP)
    {
      wRed   = (WORD)((color[i] & 0x00FF0000) >> 16);
      wGreen = (WORD)((color[i] & 0x0000FF00) >> 8);
      wBlue  = (WORD) (color[i] & 0x000000FF);
    }
    else
      wRed = wGreen = wBlue = i;

    //The STB/3dfx Tools convention for the gamma table in the registry (and 
    //GammaTable in drv) is 0x00BBGGRR, but the minivdd and hardware expect 0x00RRGGBB
    color[i] = ((GammaTable[wRed]   & 0x000000FF) << 16) |
               ((GammaTable[wGreen] & 0x0000FF00) <<  0) |
               ((GammaTable[wBlue]  & 0x00FF0000) >> 16);
  }

  param = (((FxU32)start & 0x0000FFFF) << 16) | ((FxU32)count & 0x0000FFFF);
  return !VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                                                    H3VDD_CLUT_SETDESKTOP,
                                                    param,
                                                    color);
}


*/

