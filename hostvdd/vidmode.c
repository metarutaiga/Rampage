/* $Header: vidmode.c, 2, 11/2/00 3:37:00 PM PST, Michel Conrad$ */
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
** File name:   vidmode.c
**
** Description: CSIM support for setting the modes.
**
** $Revision: 2$
** $Date: 11/2/00 3:37:00 PM PST$
**
** $History: vidmode.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 2:50p
** Created in $/devel/sst2/Win95/dx/dd16
** copied over from h3\win95\dx\dd16 with merges for csim server and qt
** 
** *****************  Version 9  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/dd16
** Linear versus tiled promotion removal.
** 
** *****************  Version 8  *****************
** User: Cwilcox      Date: 1/25/99    Time: 11:40a
** Updated in $/devel/h3/Win95/dx/dd16
** Minor modifications to remove compiler warnings.
** 
** *****************  Version 7  *****************
** User: Michael      Date: 12/30/98   Time: 2:29p
** Updated in $/devel/h3/Win95/dx/dd16
** Implement the 3Dfx/STB unified header.
**
*/

#include "header.h"

extern DDHALMODEINFO ModeList[50];
extern UINT FAR BiosMode[];
extern int IS_CSIM;
extern int FIRST;
extern int FIRSTPRIME;


/*----------------------------------------------------------------------
Function name:  setinvariantreg

Description:    sets the registers to a known state.

Information:    This fuction is currently commented out by
                "#if 0" and "#ifdef HAL_CSIM".
  
Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int setinvariantreg(void)

{
    FIRST=0;
    FIRSTPRIME=1;
//    int setinvariantreg(void)
#if 0
    IS_CSIM=1;
    SETDW(HWPTR, lph3g->clip0min, 0L);
    SETDW(HWPTR, lph3g->clip0max, _FF(bi).biHeight << 16 |
            _FF(bi).biWidth);
    // because the of the 15 bpp shift, tricks don't work
    // need to decode h3 bpp dstformat with switch
    switch(_FF(bpp))
    {
        case 8:
            dwtemp = SSTG_PIXFMT_8BPP;
        break;

        case 15:
            dwtemp = SSTG_PIXFMT_15BPP;
        break;

        case 16:
            dwtemp = SSTG_PIXFMT_16BPP;
        break;

        case 24:
            dwtemp = SSTG_PIXFMT_24BPP;
        break;

        case 32:
            dwtemp = SSTG_PIXFMT_32BPP;
        break;
    }

    SETDW(HWPTR, lph3g->dstFormat, dwtemp | _FF(pitch));
    _FF(screenFormat) = dwtemp | _FF(pitch);

    SETDW(HWPTR, lph3g->dstBaseAddr, PHYS_SCREEN_BASE_ADDR);
    SETDW(HWPTR, lph3g->srcBaseAddr, PHYS_SCREEN_BASE_ADDR);

#endif
return 1;
}


/*----------------------------------------------------------------------
Function name:  videosetmode

Description:    Sets the video mode on the CSIM.

Information:    This fuction is currently commented out by
                "#ifdef HAL_CSIM".
  
Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
#pragma optimize("", off)
int videosetmode(int ModeNumber, int refresh)
{

        __asm  push ax
        __asm  push bx
        __asm  push dx

        //  set vesa mode
        __asm mov bx, ModeNumber
        __asm shl bx, 1
        __asm  mov bx, word ptr [BiosMode][bx]
        __asm  mov ax, 04F02H
        __asm int 10h

#ifdef HW
    setinvariantreg();
#endif


    /* ************************************************************
        S3 Init Code for the Simulator.
    ************************************************************ */
#ifdef HAL_CSIM
        //   unlock registers

        //   crtc register 38 unlocks crtc registers from 39-3f
        //   crtc register 39 unlocks  system control and system
        //      extension regs
        //   crtc register 40 bit 1 unlocks enhanced command regs.

        __asm  mov al,38h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  mov al, 48h
        __asm  out dx, al

        __asm  mov al,39h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  mov al, 0A0h
        __asm  out dx, al

        __asm  mov al,40h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  in al, dx
        __asm  or al,1
        __asm  out dx, al



        //  set enhance mode
        __asm mov dx, 04AE8h
        __asm in al, dx
        __asm or al, 1
        __asm out dx, al
        //   bit 3 of cr31
        __asm  mov al, 31h
        __asm  mov dx, 03d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  in al, dx
        __asm  or al, 08h
        __asm  out dx, al

        //  set video ram to 8 bytes.

        __asm  mov al,58h
        __asm  mov dx, 03d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  in al, dx
        __asm  or al, 03h
        __asm  out dx,al


        //   put s3 968 in linear fb mode

        __asm  mov al, 58h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  in al, dx
        __asm  or al, 10h
        __asm  out dx, al

        //   Set physical address from chip in CR58, CR59

        __asm  mov al,59h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  mov al, 0F0h
        __asm  out dx, al

        __asm  mov al,5Ah
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  mov al, 00h
        __asm  out dx, al


        //   Lock the registers
        __asm  mov al,38h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  mov al, 00h
        __asm  out dx, al

        __asm  mov al,39h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  mov al, 00h
        __asm  out dx, al

        __asm  mov al,40h
        __asm  mov dx, 3d4h
        __asm  out dx, al
        __asm  inc dx
        __asm  in al, dx
        __asm  and al,0feh
        __asm  out dx, al

        __asm  pop dx
        __asm  pop bx
        __asm  pop ax

        // Hardwire in the amount of vram for the S3 Since I don't care.
        _FF(TotalVRAM)     = 4l*1024l*1024l;
        _FF(PhysAddress) = _FF(ScreenAddress) = 0xF0000000;  // *physical* address
        _FF(ScreenSel)     = 0;    // flat address we dont need this.
#endif

return 1;
}
#pragma optimize("", on)
