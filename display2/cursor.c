/* -*-c++-*- */
/* $Header: cursor.c, 10, 8/4/00 3:15:50 PM PDT, Ryan Bissell$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
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
** File name:   cursor.c
**
** Description: Implements the hardware cursor functions.
**
** $Revision: 10$
** $Date: 8/4/00 3:15:50 PM PDT$
**
** $History: cursor.c $
** 
** *****************  Version 13  *****************
** User: Einkauf      Date: 8/27/99    Time: 2:32p
** Updated in $/devel/sst2/Win95/dx/dd16
** CMDFIFO alive (if CF=1 in setenv.bat)
** 
** *****************  Version 12  *****************
** User: Peterm       Date: 9/09/99    Time: 4:40p
** Updated in $/devel/sst2/Win95/dx/dd16
** Changed all occurances of dwDstFormat and dwSrcFormat to dwFormat
** 
** *****************  Version 11  *****************
** User: Peterm       Date: 9/08/99    Time: 6:32p
** Updated in $/devel/sst2/Win95/dx/dd16
** Added fix to make cursor not go off bottom or right of video display
** 
** *****************  Version 10  *****************
** User: Einkauf      Date: 8/19/99    Time: 5:02p
** Updated in $/devel/sst2/Win95/dx/dd16
** enable cursor invert mode for mono cursor
** 
** *****************  Version 9  *****************
** User: Einkauf      Date: 8/19/99    Time: 9:21a
** Updated in $/devel/sst2/Win95/dx/dd16
** swap black/white interpretation of XORmask for mono cursor
** 
** *****************  Version 8  *****************
** User: Einkauf      Date: 8/18/99    Time: 3:36p
** Updated in $/devel/sst2/Win95/dx/dd16
** fix pattern convert for mono cursor; add hooks for CSim Video update
** (disabled by default)
** 
** *****************  Version 7  *****************
** User: Einkauf      Date: 8/06/99    Time: 3:46p
** Updated in $/devel/sst2/Win95/dx/dd16
** fix mono cursor pattern download for QT testing - correct
** interpretation of AND and XOR masks
** 
** *****************  Version 6  *****************
** User: Peterm       Date: 7/30/99    Time: 2:34a
** Updated in $/devel/sst2/Win95/dx/dd16
** modified for globaldata cleanup
** 
** *****************  Version 5  *****************
** User: Peterm       Date: 7/12/99    Time: 4:55p
** Updated in $/devel/sst2/Win95/dx/dd16
** cleanup
** 
** *****************  Version 4  *****************
** User: Peterm       Date: 7/07/99    Time: 5:06p
** Updated in $/devel/sst2/Win95/dx/dd16
** Enabled dib engine cursor, updated to work with -DR21 and -WX
** 
** *****************  Version 3  *****************
** User: Peterm       Date: 7/06/99    Time: 8:44p
** Updated in $/devel/sst2/Win95/dx/dd16
** updated for bringup on SST2 quickturn
** 
** *****************  Version 2  *****************
** User: Einkauf      Date: 6/11/99    Time: 7:02p
** Updated in $/devel/sst2/Win95/dx/dd16
** use SST2 cursor regs (guarded w/ ifdef R21)
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 2:42p
** Created in $/devel/sst2/Win95/dx/dd16
** copied over from h3\win95\dx\dd16 with merges for csim server and qt
** 
*/

#include "header.h"
#include "fifomgr.h"
#include "cursor.h"
#define Not_VxD
#include "minivdd.h"
#include <string.h>
#include <modelist.h>

POINTERINFO SaveCursor;

typedef void (FNENABLECURSOR)(int);
typedef BOOL (FNMOVECURSOR)(SHORT, SHORT);
typedef BOOL (FNSETCURSOR)(CURSORSHAPE FAR *);

DWORD dwCursorLoc;      // Linear Address of Offscreen Cursor
FNENABLECURSOR * pEnableCursor;
FNMOVECURSOR * pMoveCursor;
FNSETCURSOR * pSetCursor;

int SetUpCursorFunctions(CURSORSHAPE FAR * lpCursor);
void DoCursor(DWORD Offset, DWORD FAR * lpAnd);
void DoSST2Cursor(DWORD Offset, DWORD FAR * lpAnd);
void DoCursorDblX(DWORD Offset, DWORD FAR * lpAnd);
void DoBigCursor(DWORD Offset, DWORD FAR * lpAnd);

// DIB engine Cursor Routines
FNMOVECURSOR DIBMoveCursor;
FNSETCURSOR DIBSetCursor;
FNENABLECURSOR DIBEnableCursor;

// Software Cursor Routines
FNMOVECURSOR SwMoveCursor;
FNSETCURSOR SwSetCursor;
FNENABLECURSOR SwEnableCursor;

// Hardware Cursor Routines
FNMOVECURSOR HwMoveCursor;
FNSETCURSOR HwSetCursor;
FNENABLECURSOR HwEnableCursor;

// External Pointer to Register
extern SstIORegs *lph3IORegs;
extern SstVidRegs   *lph3VidRegs;

int ShowCursor(int nX, int nY);
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue, char FAR * lpStr, WORD nDefault);

DWORD movecursorcount = 0;
DWORD setcursorcount = 0;
int nDIBCursor;

extern void FXWAITFORIDLE();
extern DISPLAYINFO DisplayInfo;
extern int bScanlineDouble;    // in setmode.c

#define HWCURSOR        0x0100 //;driver is using a HWCURSOR right now.
extern WORD FirstEnable; 

DWORD From4BPPto8[] = {
   0x00000000, 0x00000001, 0x00000002, 0x00000003,
   0x00000004, 0x00000005, 0x00000006, 0x00000007,
   0x000000F8, 0x000000F9, 0x000000FA, 0x000000FB,
   0x000000FC, 0x000000FD, 0x000000FE, 0x000000FF,
   };

DWORD From4BPPto16[] = {
   0x00000000, 0x00007800, 0x000003E0, 0x00007BE0,
   0x0000000F, 0x0000780F, 0x000003EF, 0x0000BDF7,
   0x00007BEF, 0x0000F800, 0x000007E0, 0x0000FFE0,
   0x0000001F, 0x0000F81F, 0x000007FF, 0x0000FFFF,
   };

DWORD From4BPPto2432[] = {
   0x00000000, 0x00800000, 0x00008000, 0x00808000,
   0x00000080, 0x00800080, 0x00008080, 0x00C0C0C0,
   0x00808080, 0x00FF0000, 0x0000FF00, 0x00FFFF00,
   0x000000FF, 0x00FF00FF, 0x0000FFFF, 0x00FFFFFF,
   };

///////DELETE ME ///////extern DWORD ColorTable8BPP[256];

#undef CHECK_SW_CURSOR
#ifdef CHECK_SW_CURSOR
int CheckAndExpand(DWORD * lpAndMask, DWORD dwCursor, DWORD dwBpp);
int CheckXorExpand(BYTE * lpXorMask, DWORD * lpAndMask, DWORD dwCursor, DWORD dwBpp);
#endif

int From4BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwSreenFormat);
int From8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwSreenFormat);
int To8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwCursorFormat);

//#define USE_CSIM_UPDATE

#ifdef USE_CSIM_UPDATE
void CsimVideoUpdate();
#endif // USE_CSIM_UPDATE

/*----------------------------------------------------------------------
Function name:  InitCursor

Description:    Called to allocate cursor space on every mode switch
                and to setup the correct cursor routines to use.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void InitCursor(void)
{
    //
    // FIX_ME .. this is where we allocate a 1024 bytes for the cursor
    //

    GetBinary(DisplayInfo.diDevNodeHandle, &nDIBCursor, "DIBCursor", 0x0);
    
    nDIBCursor = 1; // !! SST2 - force dib cursor

    dwCursorLoc = _FF(cursorStart) + _FF(lfbBase);

    if (0x0 != dwCursorLoc)
    {
        SETDW(lph3VidRegs->vdVoPcBase,_FF(cursorStart));
        // set h/w to max (64x64); no valid alpha bits; bypass LUT
        SETDW(lph3VidRegs->vdVoPcCfg0, SST_VO_PC_SOURCE_WIDTH | 
                                       SST_VO_PC_SOURCE_HEIGHT | 
                                       (7 << SST_VO_PC_ALPHA_BITS_SHIFT) |         // alpha bits
                                       (0 << SST_VO_PC_ALPHA_INV_SHIFT)  |         // Alpha Inv
                                       (SST_VD_BYPASS_LUT<<SST_VO_PC_LUT_SEL_SHIFT));

        // enable update of the PS reg group
        SETDW(lph3VidRegs->vdVoPsStatus0,GET(lph3VidRegs->vdVoPsStatus0) | SST_VO_PC_UPDATE); 
    }

    if ((!_FF(fMouseTrailsOn)) &&
        (0x0 != dwCursorLoc))
    {
        pMoveCursor = HwMoveCursor;
        pSetCursor = HwSetCursor;
        pEnableCursor = HwEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(lpPDevice)->deFlags |= HWCURSOR;
    }
    else if (_FF(fMouseTrailsOn) || (nDIBCursor))
    {
        pMoveCursor = DIBMoveCursor;
        pSetCursor = DIBSetCursor;
        pEnableCursor = DIBEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR);
        _FF(gdiFlags) |= (SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(lpPDevice)->deFlags &= ~HWCURSOR;
    }
    else
    {
        pMoveCursor = SwMoveCursor;
        pSetCursor = SwSetCursor;
        pEnableCursor = SwEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(gdiFlags) |= SDATA_GDIFLAGS_SW_CURSOR;
        _FF(lpPDevice)->deFlags &= ~HWCURSOR;
    }

    //
    // Disable Cursor and Move to Upper Left of Screen
    //
    _FF(LastCursorPosY)=0;
    _FF(LastCursorPosX)=0;
    _FF(HotspotX)=0;
    _FF(HotspotY)=0;

    if (FirstEnable)
    {
        SaveCursor.csColor = 0x0;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_ENABLED);
    }

    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED);
  
    if( !(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
    {
        return;
    }

    pEnableCursor(0x0);
    pMoveCursor(0x0, 0x0);
    ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
}

#undef DIBCURSOR

/*----------------------------------------------------------------------
Function name:  DoMouseTrails

Description:    Handles the drawing of the mouse trails.
                Called from the Windows Control function.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void DoMouseTrails(WORD wTrails)
{
#ifdef DIBCURSOR
    DWORD fOldTrails = _FF(fMouseTrailsOn);

    //
    // Are trails being enabled
    //
    if (wTrails)
    {
        _FF(fMouseTrailsOn) = TRUE;
    }
    else
    {
        _FF(fMouseTrailsOn) = FALSE;
    }
    //
    // Has the state of trails changed
    //
    if (fOldTrails != _FF(fMouseTrailsOn))
    {
        if( !(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
        {
            return ;
        }
        SetUpCursorFunctions((CURSORSHAPE FAR *)&SaveCursor);
        ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
    }
#else
   return ;
#endif
}

/*----------------------------------------------------------------------
Function name:  SetCursor

Description:    Sets the shape of the cursor.  Determines whether to
                use the HW or SW cursor functions.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure due to BUSY condition
----------------------------------------------------------------------*/
BOOL FAR PASCAL _loadds SetCursor(CURSORSHAPE FAR *lpCursor)
{
   SHORT x;
   SHORT y;

   if (_FF(lpPDevice)->deFlags & BUSY)
        return FALSE;
     
   if( !(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
       return FALSE;

   pEnableCursor(0x0);
   _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_ENABLED);

   if (lpCursor == NULL)
   {
       ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
       return TRUE;
   }

   DPF(DBGLVL_CURSOR, " Enter SetCursor");

   // cde 030398
   // The routine referenced by pMoveCursor doesn't care about the
   // hotspot, it expects an absolute x,y location to move the cursor
   // to; the MoveCursor routine just below normally takes care of
   // calculating the correct x,y based on the given x,y and the hotspot;
   // However, with this new cursor, the hotspot and cursor could change
   // on the screen before a MoveCursor is called; this causes the new
   // cursor to dance around when it changes; So, we'll first subtract
   // out the old hotspot from the current location, the calculate
   // a new correct offset based on the new hotspot

   _FF(HotspotX) = (UINT)lpCursor->xHotSpot;
   _FF(HotspotY) = (UINT)lpCursor->yHotSpot;

   // Do Save Under after hotspot updated
   if (SetUpCursorFunctions(lpCursor) == -1)
   {
      // If bogus or corrupted data found in the POINTERINFO
      // structure, then just return.  This is to work around
      // a WHQL app bug in WHQL PC98, GDI - Windows, AMovie test.
      // See PRS #s 1784 & 2337.
      ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
      return (TRUE);
   }

   _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_ENABLED;
   if( !( _FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) )
   {
       pMoveCursor(_FF(CursorPosX),  _FF(CursorPosY));
       pSetCursor((CURSORSHAPE FAR *)&SaveCursor);
   }
   else
   {

       pSetCursor((CURSORSHAPE FAR *)&SaveCursor);
//       pMoveCursor(_FF(CursorPosX),  _FF(CursorPosY));  // !! SST2
       
       //
       // Duplicate Cursor Move Code to avoid playing with flags
       //  
       
       RestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
       // Get Cursor Position so SWMove will not cause us to lose one
       // This may cause us to lose a update but should prevent losing a cursor
       x = _FF(CursorPosX) -  _FF(HotspotX);  
       y = _FF(CursorPosY) -  _FF(HotspotY);  
       SaveCursorExclude(x, y);
       DrawCursor(x, y); 
   }
   DPF(DBGLVL_CURSOR, " Exit SetCursor");

   ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 

   return TRUE;
}

/*----------------------------------------------------------------------
Function name:  MoveCursor

Description:    Moves the screen location of the cursor.  Used for both
                HW and SW cursor.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL FAR PASCAL _loadds MoveCursor(SHORT x, SHORT y)
{
    if (!(_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR))
    {
        if (bScanlineDouble & CURSOR_DBL_X)
            x <<= 1;     // fixup cursor positions for special modes

        if (bScanlineDouble & CURSOR_DBL_Y)
            y <<= 1;
    }

   _FF(CursorPosX) = x;
   _FF(CursorPosY) = y;

   return (pMoveCursor)(x, y);
}

/*----------------------------------------------------------------------
Function name:  SetUpCursorFunctions

Description:    Determines whether to use HW or SW cursor functions.

Information:

Return:         int      1 = Success
                        -1 = Failure
----------------------------------------------------------------------*/
int SetUpCursorFunctions(CURSORSHAPE FAR * lpCursor)
{
    DWORD dwFormat;
    DWORD transition=0;
    LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;

    //
    // Save Cursor
    //
    switch (lpPointer->csColor)
    {
    case 0x101:
        *(PPOINTERINFO1)&SaveCursor = *(LPPOINTERINFO1)lpPointer;
        break;

    case 0x401:
        *(PPOINTERINFO4)&SaveCursor = *(LPPOINTERINFO4)lpPointer;
        break;

    case 0x801:
        *(PPOINTERINFO8)&SaveCursor = *(LPPOINTERINFO8)lpPointer;
        break;

    case 0x1001:
        *(PPOINTERINFO16)&SaveCursor = *(LPPOINTERINFO16)lpPointer;
        break;

    case 0x1801:
        *(PPOINTERINFO24)&SaveCursor = *(LPPOINTERINFO24)lpPointer;
        break;

    case 0x2001:
        SaveCursor = *lpPointer;
        break;

    // Save Cursor in case we have to restore it
    default:
        // If bogus or corrupted data found in the POINTERINFO
        // structure, then return error.  It is just a guess that
        // hot spot will never be greater than 0x41.  This is to
        // work around a WHQL app bug in WHQL PC98, GDI - Windows,
        // AMovie test.  See PRS #s 1784 & 2337.
        if ((UINT)lpCursor->xHotSpot > 0x41)
        return -1;

        _fmemcpy(&SaveCursor, lpPointer, 
        sizeof(POINTERINFO) - sizeof(lpPointer->csXORBits) +
           (lpPointer->csColor >> 8) * lpPointer->csWidthBytes * 32);
        break;
    }

    transition = (WORD) ( _FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR ) ;

    transition >>= 1;

    if (_FF(fMouseTrailsOn) || (_FF(HotspotX) < 0) || (_FF(HotspotY) < 0) || (nDIBCursor) ||
        ((0x0101 != lpPointer->csColor) && (0x0401 != lpPointer->csColor) && (0x0801 != lpPointer->csColor) && 
        (0x1001 != lpPointer->csColor) && (0x1801 != lpPointer->csColor) && (0x2001 != lpPointer->csColor)))
    {
        pMoveCursor = DIBMoveCursor;
        pSetCursor = DIBSetCursor;
        pEnableCursor = DIBEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR);
        _FF(gdiFlags) |= (SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(lpPDevice)->deFlags &= ~HWCURSOR;
    }
    else
    {
        pMoveCursor = SwMoveCursor;
        pSetCursor = SwSetCursor;
        pEnableCursor = SwEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(gdiFlags) |= SDATA_GDIFLAGS_SW_CURSOR;
        _FF(lpPDevice)->deFlags &= ~HWCURSOR;
    }

    //  If MouseTrails are not on and
    //  we have space in the FB for a cursor and
    //  the cursor is a mono-cursor and the mode is not 2048x????
    //  then we can handle it

    if ((!_FF(fMouseTrailsOn)) &&
        (0x0 != dwCursorLoc) &&
        (0x0101 == lpPointer->csColor))
    {
        pMoveCursor = HwMoveCursor;
        pSetCursor = HwSetCursor;
        pEnableCursor = HwEnableCursor;
        _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_SW_CURSOR | SDATA_GDIFLAGS_DIB_CURSOR);
        _FF(lpPDevice)->deFlags |= HWCURSOR;
    }
 
    transition  |=(WORD)( ( _FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR ) >> 2);

    //  11 swcursor to swcuror
    //  10 swcursor to dibcursor/hwcursor
    //  01 dibcursor/hwcursor to swcursor
    //  00 dibcursor/hwcursor to dibcursor/hwcursor

    switch (transition)
    {
    case 0x01:
    case 0x03:
        // save cursor exclusion
        DPF(DBGLVL_CURSOR, " sw hw cursorposx %d,  cursorposX %d",
                _FF(CursorPosX), _FF(CursorPosY));
        SaveCursorExclude( _FF(CursorPosX) - _FF(HotspotX), _FF(CursorPosY) - _FF(HotspotY));
        break;

    case 0x02:
        // erase cursor
        DPF(DBGLVL_CURSOR, " sw hw cursorposx %d,  cursorposX %d",
                _FF(CursorPosX), _FF(CursorPosY) );
        RestoreCursorExclude( _FF(LastCursorPosX), _FF(LastCursorPosY));
        break;

    default:
        break;
    }

    dwFormat = GetCursorFormat((CURSORSHAPE FAR *)lpPointer) & SST_WX_SRC_FORMAT;

    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) &&
        (dwFormat != (_FF(ddPrimarySurfaceData.dwFormat) & SST_WX_SRC_FORMAT)) &&
        (SST_WX_PIXFMT_1BPP != dwFormat))
    {
        if (0x0401 == lpPointer->csColor)
            From4BPPCursor(lpCursor, &SaveCursor, _FF(ddPrimarySurfaceData.dwFormat) & SST_WX_SRC_FORMAT);
        else if (SST_WX_PIXFMT_8BPP == dwFormat)
            From8BPPCursor(lpCursor, &SaveCursor, _FF(ddPrimarySurfaceData.dwFormat) & SST_WX_SRC_FORMAT);
        else if (SST_WX_PIXFMT_8BPP == (_FF(ddPrimarySurfaceData.dwFormat) & SST_WX_SRC_FORMAT))
            To8BPPCursor(lpCursor, &SaveCursor, dwFormat);
    }

    return 1;
}

/*----------------------------------------------------------------------
Function name:  From4BPPCursor

Description:    Expands the 4 BPP mask via a colortable and converts it to
                8/16/24/32 bpp from 4 bpp.

Information:

Return:         int      0 = Always returned
----------------------------------------------------------------------*/
int From4BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwScreenFormat)
{
   LPPOINTERINFO4 lpPointer;
   DWORD * pTable;
   BYTE *pByte;
   BYTE bData;
   int i;
   int j;
   int nSize;
   int nIndex;

    // What is screen BPP
    switch (dwScreenFormat)
    {
    case SST_WX_PIXFMT_8BPP:
        lpSaveCursor->csColor = 0x0801;
        nSize = 1;
        pTable = From4BPPto8;
        break;
    case SST_WX_PIXFMT_16BPP:
        lpSaveCursor->csColor = 0x1001;
        nSize = 2;
        pTable = From4BPPto16;
        break;
    case SST_WX_PIXFMT_24BPP:
        lpSaveCursor->csColor = 0x1801;
        nSize = 3;
        pTable = From4BPPto2432;
        break;
    case SST_WX_PIXFMT_32BPP:
        lpSaveCursor->csColor = 0x2001;
        nSize = 4;
        pTable = From4BPPto2432;
        break;
    default:
        lpSaveCursor->csColor = 0x2001;
        nSize = 4;
        break;
    }

    lpPointer = (LPPOINTERINFO4)lpCursor;
    pByte = (BYTE *)&lpSaveCursor->csXORBits;
    for (i=0; i<512; i++)
    {
        bData = lpPointer->csXORBits[i];
        for (j=0; j<2; j++)
        {
            nIndex = bData;      
            bData <<=4;
            nIndex &= 0xF0;
            nIndex >>= 4;
            *((DWORD *)pByte) = pTable[nIndex];
            pByte+=nSize;
        }
    }         

    return 0;
}

/*----------------------------------------------------------------------
Function name:  From8BPPCursor

Description:    Expands the old mask via the ColorTable8BPP and converts it to
                16/24/32 bpp from 8 bpp.

Information:

Return:         int      0 = Always returned
----------------------------------------------------------------------*/
int From8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwScreenFormat)
{
    DWORD dwColor;
    WORD wColor;
    LPPOINTERINFO8 lpPointer;
    BYTE * pByte;
    BYTE * pColor;
    int nSize;
    int i;
    int j;

    // What is screen BPP
    if (SST_WX_PIXFMT_16BPP == dwScreenFormat)
    {
        lpSaveCursor->csColor = 0x1001;
        nSize = 2;
    }
    else if (SST_WX_PIXFMT_24BPP == dwScreenFormat)
    {
        lpSaveCursor->csColor = 0x1801;
        nSize = 3;
    }
    else if (SST_WX_PIXFMT_32BPP == dwScreenFormat)
    {
        lpSaveCursor->csColor = 0x2001;
        nSize = 4;
    }
  
    pByte = (BYTE *)&lpSaveCursor->csXORBits;
    lpPointer = (LPPOINTERINFO8)lpCursor;
   
    // Expand the XOR mask to screen size
    for (i=0; i<1024; i++)
    {
        dwColor = _FF(ColorTable8BPP)[lpPointer->csXORBits[i]];
        if (2 == nSize)
        {
            wColor = (WORD)(((dwColor & 0x00F80000) >> 8) |
                ((dwColor & 0x0000FC00) >> 5) |
                ((dwColor & 0x000000F8) >> 3));
            pColor = (BYTE *)&wColor;
        }
        else
            pColor = (BYTE *)&dwColor;

        for (j=0; j<nSize; j++)
            *pByte++ = *pColor++;
    }
    return 0;
}


/*----------------------------------------------------------------------
Function name:  To8BPPCursor

Description:    Point samples the old mask and converts it from 
                16/24/32 bpp from 8 bpp.

Information:

Return:         int      0 = Always returned
----------------------------------------------------------------------*/
int To8BPPCursor(CURSORSHAPE FAR * lpCursor, LPPOINTERINFO lpSaveCursor, DWORD dwCursorFormat)
{
    LPPOINTERINFO8 lpPointer;
    BYTE * pByte;
    int nSize;
    int i;

    // What is cursor BPP
    if (SST_WX_PIXFMT_16BPP == dwCursorFormat)
        nSize = 2;
    else if (SST_WX_PIXFMT_24BPP == dwCursorFormat)
        nSize = 3;
    else if (SST_WX_PIXFMT_32BPP == dwCursorFormat)
        nSize = 4;
  
    lpSaveCursor->csColor = 0x0801;
    lpPointer = (LPPOINTERINFO8)lpCursor;
    pByte = (BYTE * )&lpPointer->csXORBits;
    lpPointer = (LPPOINTERINFO8)lpSaveCursor;
   
    // Contract the XOR mask to screen size
    for (i=0; i<1024; i++)
    {
        lpPointer->csXORBits[i] = *pByte;
        pByte += nSize;
    }

    return 0;
}

/*----------------------------------------------------------------------
Function name:  CheckCursor

Description:    CheckCursor is called on each timer interrupt. The function
                should determine whether the cursor needs redrawing and
                whether drawing is enabled.  If so, the function should
                redraw the cursor.
Information:

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds CheckCursor(void)
{
    // If we are busy then return
    if (_FF(lpPDevice)->deFlags & BUSY)
        return;

    // If we are in exclusive mode then
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
        return;

    // If we are hardware cursor then return
    if( _FF(gdiFlags) & SDATA_GDIFLAGS_DIB_CURSOR)
        DIB_CheckCursorExt(_FF(lpPDevice));

    // MAE-AGP - something like this needs to happen here to flush AGP fifo
    //  at regular intervals.  If the following is enabled, sporadic errors/hangs
    //  occur because this command can interrupt other driver operations and
    //  do the bump unknown to the interrupted routine.  I thought the above
    //  busy guards were sufficient (they were in this routine originally), but
    //  obviously more is needed.
#if 0 // MAE-AGP def AGP_CMDFIFO
    if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
    {
        bumpAgp( CMDFIFOUNBUMPEDWORDS );
    }
#endif

    return;
}

/*----------------------------------------------------------------------
Function name:  DIBMoveCursor

Description:    Calls the DIB engine MoveCursor function.

Information:

Return:         BOOL    FALSE if BUSY or EXCLUSIVE mode or,
                        result of DIB MoveCursor call.
----------------------------------------------------------------------*/
BOOL DIBMoveCursor(SHORT x, SHORT y)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return (FALSE);

   if (_FF(lpPDevice)->deFlags & BUSY)
      return (FALSE);

   return DIB_MoveCursorExt(x, y, _FF(lpPDevice));
}

/*----------------------------------------------------------------------
Function name:  DIBMoveHelp

Description:    Calls the DIB engine MoveCursorExt function.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void DIBMoveHelp()
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return;

   if (_FF(lpPDevice)->deFlags & BUSY)
      return;

   DIB_MoveCursorExt(_FF(CursorPosX), _FF(CursorPosY), _FF(lpPDevice));
}

/*----------------------------------------------------------------------
Function name:  DIBSetCursor

Description:    Calls the DIB engine SetCursor function.

Information:

Return:         BOOL    FALSE if BUSY or EXCLUSIVE mode or,
                        result of DIB SetCursor call.
----------------------------------------------------------------------*/
BOOL DIBSetCursor(CURSORSHAPE FAR * lpCursor)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return (FALSE);

   if (_FF(lpPDevice)->deFlags & BUSY)
      return (FALSE);

   DIB_SetCursorExt(lpCursor, _FF(lpPDevice));
   return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  DIBEnableCursor

Description:    Performs the DIB engine EnableCursor function via a
                call to the DIB SetCursorExt.

Information:

Return:         BOOL    FALSE if BUSY or EXCLUSIVE mode or,
                        result of DIB SetCursor call.
----------------------------------------------------------------------*/
void DIBEnableCursor(int nFlag)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return;

   if (_FF(lpPDevice)->deFlags & BUSY)
      return ;

   if (nFlag)
      DIB_SetCursorExt((LPVOID)&SaveCursor, _FF(lpPDevice));
   else
      DIB_SetCursorExt(NULL, _FF(lpPDevice));
}

/*----------------------------------------------------------------------
Function name:  SwMoveCursor

Description:    Move the screen location of the cursor via SW.

Information:

Return:         BOOL    FALSE = Failure (multiple conditions)
                        TRUE  = Success 
----------------------------------------------------------------------*/
BOOL SwMoveCursor(SHORT x, SHORT y)
{
    DPF(DBGLVL_CURSOR, " Enter SW MoveCursor");

    if (_FF(lpPDevice)->deFlags & BUSY)
    {
        (_FF(cursorMissed))++;
        return(FALSE);
    }

    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_EXCLUDE) ||
        (!(_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_ENABLED)))
    {
        (_FF(cursorMissed))++;
        return(FALSE);
    }

    if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
   	    return TRUE;

    if(!(SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
    {
        return FALSE;
    }

    RestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
    x = x - _FF(HotspotX);
    y = y - _FF(HotspotY);
    SaveCursorExclude(x, y);
    DrawCursor(x, y); 

#ifdef DEBUG_CURSOR
    GottaCursor(); 
#endif

    DPF(DBGLVL_CURSOR, " Exit MoveCursor");
    ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 

    return (TRUE);
}
 
/*----------------------------------------------------------------------
Function name:  GetCursorFormat

Description:    Returns dword with stride and pix depth info for XOR
                mask ONLY. The and mask stride and pix depth is same
                across pix depth.  Returned dword is used for
                srcFormat/dstFormat register.  The cursor is 32 by 32
                bytes.
Information:

Return:         DWORD   Used to set srcFormat/DstFormat HW registers
----------------------------------------------------------------------*/
DWORD GetCursorFormat (CURSORSHAPE FAR *lpCursor)
{
    DWORD retval;
    LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;
   
    switch (lpPointer->csColor)
    {
    case 0x101:
        // stride for mono xor mask is  4 bytes
        retval = CURSOR_BMP_STRIDE | SST_WX_PIXFMT_1BPP;
        break;
    case 0x801:
        // stride for 8bpp xor mask is  32 bytes
        retval = CURSOR_BMP_STRIDE | SST_WX_PIXFMT_8BPP;
        break;
    case 0x1001:
        // stride for 16bpp xor mask is  64 bytes
        retval = CURSOR_BMP_STRIDE | SST_WX_PIXFMT_16BPP;
        break;
    case 0x1801:
        DPF(DBGLVL_CURSOR, " 24bpp cursor fmt not allowed on SST2");
        break;
    case 0x2001:
        // stride for 8bpp xor mask is  128 bytes
        retval = CURSOR_BMP_STRIDE | SST_WX_PIXFMT_32BPP;
        break;
    default:
        // this is for 4bpp
        // Put in a Bogus Format which we will fix up latter
        retval = CURSOR_BMP_STRIDE | (0x0FUL << SST_WX_SRC_FORMAT_SHIFT);
        break;
    }
 
    return retval;
}
   
/*----------------------------------------------------------------------
Function name:  SwSetCursor

Description:    The software set cursor function.  Also, determines if
                it's a HW or SW cursor.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL SwSetCursor(CURSORSHAPE FAR * lpCursor)
{
    DWORD srcFormat, dstFormat;
    DWORD * lpXorMask;
    DWORD * lpAndMask;
    WORD  bytesPerPix;
    DWORD  dwordsNeeded;
    unsigned int i;
    LPPOINTERINFO lpPointer = (LPPOINTERINFO)lpCursor;
    CMDFIFO_PROLOG(cmdFifo);

    if (_FF(lpPDevice)->deFlags & BUSY)
    {
        return(FALSE);
    }
 
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
    {
        _FF(SWcursorFormat)=GetCursorFormat(lpCursor);
        return TRUE;
    }

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

#ifndef CHECK_SW_CURSOR
    FXWAITFORIDLE();
#endif

    CMDFIFO_SETUP(cmdFifo);

    lpAndMask = (DWORD *) &(lpPointer->csANDBits); 
    lpXorMask = (DWORD *) &(lpPointer->csXORBits); 
    
    // cursor storage area is 32X32X4bytes.
 
    if (0x0101 == ((LPPOINTERINFO)lpCursor)->csColor)
        dstFormat=(_FF(ddPrimarySurfaceData.dwFormat) & SST_WX_SRC_FORMAT) | CURSOR_BMP_STRIDE;
    else
        dstFormat=GetCursorFormat(lpCursor);

    _FF(SWcursorFormat) = dstFormat;
 
    // host blt mono "and" mask to color cursor 
    // bitmap location
    // using 0xFFFFFF as chroma color 
    CMDFIFO_CHECKROOM(cmdFifo,13 );
    SETPH(cmdFifo, SSTCP_PKT2|
          clip0minBit        |
          clip0maxBit        |
          dstBaseAddrBit     |
          dstFormatBit       |
          commandExBit       |
          srcFormatBit       |
          srcXYBit           |
          colorBackBit       |
          colorForeBit       |
          dstXYBit           |
          dstSizeBit         |
          commandBit );
    SET(cmdFifo, lph3g->clip0min, 0);
    SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
    SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorAndStart));
    SET(cmdFifo, lph3g->dstFormat, dstFormat);
    SET(cmdFifo, lph3g->commandEx, 0x0);
    SET(cmdFifo, lph3g->srcFormat, AND_MASK_STRIDE | SST_WX_SRC_PACK_SRC | SST_WX_PIXFMT_1BPP);
    SET(cmdFifo, lph3g->srcXY , 0);
    SET(cmdFifo, lph3g->colorBack, 0x00000000L)
    SET(cmdFifo, lph3g->colorFore, 0x00FFFFFF)
    SET(cmdFifo, lph3g->dstSize , (32UL << 16)| 32UL );
    SET(cmdFifo, lph3g->dstXY , 0UL );
    SETC(cmdFifo, lph3g->command,
        (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT)|
        SST_WX_HOST_BLT );
    BUMP(13);
 
    // and mask is mono 
    // 32pix X 32pix /8PixelperByte / 4bytesPerDwords = 32
    CMDFIFO_CHECKROOM(cmdFifo, 33);
    SETPH(cmdFifo, SSTCP_PKT1| 
          SSTCP_PKT1_2D|
          LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
          32UL  <<SSTCP_PKT1_2D_NWORDS_SHIFT); 
    for( i=0; i< 32; i++)
    {
        SET(cmdFifo, lph3g->launch[0], lpAndMask[i]);
    }
    BUMP(33);

#ifdef CHECK_SW_CURSOR
    CheckAndExpand(lpAndMask, _FF(SWcursorBitmapStart) + _FF(lfbBase), lpPointer->csColor >> 8);
#endif  
 
   // stride and bpp for xormask
   srcFormat=GetCursorFormat(lpCursor);
    
   // If mask is monochrome as is the case with 2048x1536
   if (SST_WX_PIXFMT_1BPP == (srcFormat & SST_WX_SRC_FORMAT))
{
      // host blt mono "xor" mask to color cursor 
      // bitmap location
      // using 0xFFFFFF as chroma color 
      CMDFIFO_CHECKROOM(cmdFifo, 7);
      SETPH(cmdFifo, SSTCP_PKT2|
            dstBaseAddrBit     |
            srcFormatBit       |
            srcXYBit           |
            //colorBackBit     |
            //colorForeBit     |
            dstXYBit           |
            dstSizeBit         |
            commandBit );
      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorXorStart));
      SET(cmdFifo, lph3g->srcFormat, AND_MASK_STRIDE | SST_WX_SRC_PACK_SRC | SST_WX_PIXFMT_1BPP);
      SET(cmdFifo, lph3g->srcXY , 0);
      //SET(cmdFifo, lph3g->colorBack, 0x00000000L)
      //SET(cmdFifo, lph3g->colorFore, 0x00FFFFFF)
      SET(cmdFifo, lph3g->dstSize , (32UL << 16)| 32UL );
      SET(cmdFifo, lph3g->dstXY , 0UL );
      SETC(cmdFifo, lph3g->command,
         (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT)|
         SST_WX_HOST_BLT );
      BUMP(7);
 
      // xor mask is mono 
      // 32pix X 32pix /8PixelperByte / 4bytesPerDwords = 32
      CMDFIFO_CHECKROOM(cmdFifo, 33);
      SETPH(cmdFifo, SSTCP_PKT1| 
            SSTCP_PKT1_2D|
            LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
            32UL  <<SSTCP_PKT1_2D_NWORDS_SHIFT); 
      for( i=0; i< 32; i++)
         {
         SET(cmdFifo, lph3g->launch[0], lpXorMask[i]);
         }
      BUMP(33);
}
   else
      {
      // dst chroma blt color xormask to SWcursorbitmap 
      // ie rop1(src fail, dst pass)
      // fail write pix, pass ignore pix
      CMDFIFO_CHECKROOM(cmdFifo, 7);
      SETPH(cmdFifo, SSTCP_PKT2|
            dstBaseAddrBit     |
            srcFormatBit       |
            srcXYBit           |
            dstXYBit           |
            dstSizeBit         |
            commandBit
            );

      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorXorStart));
      SET(cmdFifo,  lph3g->srcFormat,srcFormat);
      SET(cmdFifo,  lph3g->srcXY, 0L);
      SET(cmdFifo,  lph3g->dstSize , (32UL << 16)| 32UL );
      SET(cmdFifo,  lph3g->dstXY , 0UL);
      SETC(cmdFifo,  lph3g->command,
          SST_WX_HOST_BLT|
          0xCC000000);
      BUMP (7);
        
 
      bytesPerPix = ((lpPointer->csColor) >> 8)  >> 3 ;
      dwordsNeeded = ( 32 * 32 * bytesPerPix) >> 2 ;
      CMDFIFO_CHECKROOM(cmdFifo, dwordsNeeded + 1);
      SETPH(cmdFifo, SSTCP_PKT1| 
            SSTCP_PKT1_2D|
            LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
            dwordsNeeded  <<SSTCP_PKT1_2D_NWORDS_SHIFT); 

      for( i=0; i< dwordsNeeded; i++)
         {
         SET(cmdFifo, lph3g->launch[0], lpXorMask[i]);
         }
      BUMP(dwordsNeeded +1);
      }
#ifdef CHECK_SW_CURSOR
    CheckXorExpand((BYTE *)lpXorMask, lpAndMask, _FF(SWcursorBitmapStart) + _FF(lfbBase), lpPointer->csColor >> 8);
#endif

    CMDFIFO_EPILOG(cmdFifo);
    return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  SwEnableCursor

Description:    The software EnableCursor function

Information:

Return:         VOID
----------------------------------------------------------------------*/
void SwEnableCursor(int nFlag)
{
    if (_FF(lpPDevice)->deFlags & BUSY)
    {
        return ;
    }

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return;

    // If cursor is already enabled and sw then erase it...
    if ((SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))) &&
        (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR))
        RestoreCursorExclude((int)_FF(LastCursorPosX), (int)_FF(LastCursorPosY));
}

/*----------------------------------------------------------------------
Function name:  HwSetCursor

Description:    The SST2 HW dependent SetCursor function

Information:    Cursor is 64x64 pattern of 32bpp ARGB values
                Pattern stride is cursor width (32)
                If supplied pattern less than 64x64, remainder is padded w/ "Unchanged" values                    

Return:         VOID
----------------------------------------------------------------------*/
BOOL HwSetCursor(CURSORSHAPE * lpCursor)
{

#define CURSOR_SRC_WIDTH    32
#define CURSOR_SRC_HEIGHT   32

#define SST2_CURSOR_WIDTH   64
#define SST2_CURSOR_HEIGHT  64

#define SST2_CURSOR_WHITE   0xffffffff  // white, with alpha
#define SST2_CURSOR_BLACK   0xff000000  // black
#define SST2_CURSOR_NOCHNG  0x00000000  // alpha = 0 - leaves pixel unchanged
#define SST2_CURSOR_INVERT  0x80FFFFFF  // alpha = 0x80 means "XOR" with RGB of cursor pixel

    LPPOINTERINFO1 lpPointerInfo1 = (LPPOINTERINFO1)lpCursor;
    DWORD pCursorPattern;
    DWORD FAR * lpANDBits = (DWORD FAR *)&lpPointerInfo1->csANDBits;
    DWORD FAR * lpXORBits = (DWORD FAR *)&lpPointerInfo1->csXORBits;
    DWORD Row[SST2_CURSOR_WIDTH];

    int c,r;
    DWORD AndMask,XorMask;
    BYTE AndByte,XorByte;

   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return FALSE;

   // Get the pointer into FB where to put the cursor
   pCursorPattern = dwCursorLoc;

   // build cursor, row by row, and copy to video memory
   // Note masks are 32bits, 1 bpp, of following form
   // MSB                           LSB (number is corresponding pixel)
   // |24 - 31 |16 - 23|8  - 15|0  - 7|
   for (r=0; r<CURSOR_SRC_HEIGHT; r++)
   {
        AndMask = lpANDBits[r];
        XorMask = lpXORBits[r];
        AndByte = (BYTE)(AndMask & 0xFF);
        XorByte = (BYTE)(XorMask & 0xFF);
        for (c=0; c<CURSOR_SRC_WIDTH; c++)
        {
            // Mono cursor interpretation, from NT5DDK graphics doc
            //AND value XOR value Resultant Pixel 
            // 0           0       White. 
            // 0           1       Black. 
            // 1           0       Pixel is unchanged. 
            // 1           1       Pixel color is inverted. 

            if (AndByte & 0x80)//dig pixels out starting w/ MSB of byte
            {
                // AND bit on - invert if XOR=1, unchanged if XOR=0
                Row[c] = (XorByte & 0x80) ? SST2_CURSOR_INVERT : SST2_CURSOR_NOCHNG;
            }
            else
            {                   
                // AND bit off - use White is XOR=1, else Black
                Row[c] = (XorByte & 0x80) ? SST2_CURSOR_WHITE : SST2_CURSOR_BLACK;
            }
            AndByte <<= 1;// shift next pixel's mask to MSB of byte
            XorByte <<= 1;

#ifdef PREFER_NAIVE
            // every 8 bits, switch to next byte of mask
            switch (c)
            {
            case 7:// next pixel needs bit 8, so switch to next byte...
                AndByte = (BYTE)((AndMask>>8) & 0xFF);
                XorByte = (BYTE)((XorMask>>8) & 0xFF);
                break;

            case 15:// next pixel needs bit 16, so switch to next byte...
                AndByte = (BYTE)((AndMask>>16) & 0xFF);
                XorByte = (BYTE)((XorMask>>16) & 0xFF);
                break;

            case 23:// next pixel needs bit 24, so switch to next byte...
                AndByte = (BYTE)((AndMask>>24) & 0xFF);
                XorByte = (BYTE)((XorMask>>24) & 0xFF);
                break;
            }

#else // def PREFER_NAIVE
            // here's the clever version, but a bit cryptic
            if ((c & 7) == 7) 
            {
                AndByte = (BYTE)((AndMask>>(c+1)) & 0xFF);
                XorByte = (BYTE)((XorMask>>(c+1)) & 0xFF);
            }
#endif // def PREFER_NAIVE

        }

        // pad remainder of row w/ UNCHANGED when SST2 cursor width > src width
        for (; c<SST2_CURSOR_WIDTH; c++) Row[c]=SST2_CURSOR_NOCHNG;

        // copy row to video memory 
        DoSST2Cursor(pCursorPattern, Row);

        pCursorPattern += SST2_CURSOR_WIDTH*sizeof(DWORD);
   } // endfor

   // pad remaining rows w/ UNCHANGED when SST2 cursor height > src height
   for (; r<SST2_CURSOR_HEIGHT; r++)
   {
       for (c=0; c<SST2_CURSOR_WIDTH; c++)
       {
           Row[c]=SST2_CURSOR_NOCHNG;
       }

       // copy row to video memory 
       DoSST2Cursor(pCursorPattern, Row);

       pCursorPattern += SST2_CURSOR_WIDTH*sizeof(DWORD);
   } // endfor


 #ifdef CRASHTEST
    if (doCT)
    {
	FXWAITFORIDLE();
    }
 #endif // #ifdef CRASHTEST

    //
    //Now that the cursor is ready enable it
    //
    HwEnableCursor(0x01);
 
    return TRUE;
}

/*----------------------------------------------------------------------
Function name:  HwMoveCursor

Description:    The V2/V3 HW dependent MoveCursor function.  Only called
                if a HW cursor is active.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure due to EXCLUSIVE mode
----------------------------------------------------------------------*/
BOOL HwMoveCursor(SHORT x, SHORT y)
{
   // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return FALSE;

    //
    // Don't allow X and Y to go < 0
    //
    // Adjust the cursor by 63x63 to account for lower right start
    // and subtract off the Hotspot

    x = x - _FF(HotspotX);
    y = y - _FF(HotspotY);

    if (x <= (-SST2_CURSOR_WIDTH))
        x = -SST2_CURSOR_WIDTH + 1;

    if (x >= _FF(dwWidth))
        x = _FF(dwWidth) - 1;

    if (y <= (-SST2_CURSOR_HEIGHT))
        y = -SST2_CURSOR_HEIGHT + 1;

    if (y >= _FF(dwHeight))
        y = _FF(dwHeight) - 1;

    SETDW(lph3VidRegs->vdVoPcCfg1, ((DWORD)y << SST_VO_PC_START_LINE_SHIFT) |
                                   ((DWORD)x << SST_VO_PC_START_PIXEL_SHIFT));
                                    
    // enable update of the PC reg group
    SETDW(lph3VidRegs->vdVoPsStatus0,GET(lph3VidRegs->vdVoPsStatus0) | SST_VO_PC_UPDATE);

#ifdef USE_CSIM_UPDATE
    CsimVideoUpdate();
#endif

    _FF(LastCursorPosX) =  _FF(CursorPosX) ;
    _FF(LastCursorPosY) =  _FF(CursorPosY) ;

    return TRUE;
}

/*----------------------------------------------------------------------
Function name:  HwEnableCursor

Description:    The V2/V3 HW dependent EnableCursor function

Information:

Return:         VOID
----------------------------------------------------------------------*/
void HwEnableCursor(int nFlag)
{
    DWORD dwData,dwOrg;

      // If we are in exclusive mode then
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
      return;

   dwOrg = GET(lph3VidRegs->vdVoPmCfg);
   dwData = nFlag ? (SST_VO_PM_PC_EN|SST_VO_PM_CURSOR_INV_MODE) : 0;

    //make sure mixer need to be changed
   if( (dwOrg & ( SST_VO_PM_PC_EN | SST_VO_PM_CURSOR_INV_MODE)) != dwData)
   {
        //check any thing pending
        while(GET(lph3VidRegs->vdVoPsStatus0) & SST_VO_PS_UPDATE)
         ; 
        dwData |= dwOrg & ~( SST_VO_PM_PC_EN | SST_VO_PM_CURSOR_INV_MODE);

        SETDW(lph3VidRegs->vdVoPmCfg, dwData);

        // enable update of the PS reg group
        //if mixer is changed, better to update all.
        SETDW(lph3VidRegs->vdVoPsStatus0,
            GET(lph3VidRegs->vdVoPsStatus0) | SST_VO_PS_UPDATE | SST_VO_PD_UPDATE |
                             SST_VO_PV_UPDATE | SST_VO_PC_UPDATE );

#if 0 // bpb, commented out at Xing's suggestion to speed QT testing.
        //we might not wait for the update, but for now 
        //just do it for safty
        while(GET(lph3VidRegs->vdVoPsStatus0) & SST_VO_PS_UPDATE)
           ;
#endif
    }


}
/* ************************************************************  */ 
 

/*----------------------------------------------------------------------
Function name:  SaveCursorExclude

Description:    Saves the area of the display under the cursor prior to
                drawing the cursor.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int SaveCursorExclude(int x, int y)
{ 
    DWORD dstFormat;
    SHORT Xsize;
    SHORT Ysize;
    SHORT x1, y1;

    CMDFIFO_PROLOG(cmdFifo);
 
    if (_FF(gdiFlags) & (SDATA_GDIFLAGS_HWC_EXCLUSIVE | SDATA_GDIFLAGS_CURSOR_EXCLUDE))
   	return TRUE;
   
    x1 = x;
    y1 = y;
   
    Xsize = 32;
    if (x < 0)
       {
       Xsize += x;
       x = 0;
       }
    Ysize = 32;
    if (y < 0)
       {
       Ysize += y;
       y = 0;
       }

    // Have we been clipped ?
    if ((Xsize > 0) && (Ysize > 0))
      {
      _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

      CMDFIFO_SETUP(cmdFifo);
 
      dstFormat = _FF(ddPrimarySurfaceData.dwFormat);
      dstFormat &= ~(SST_WX_SRC_PACK);
      dstFormat &= 0xFFFFC000;
      dstFormat |= EXCLUSION_BMP_STRIDE;
        
      // save under cursor 32x32
      CMDFIFO_CHECKROOM(cmdFifo, 15);
      SETPH(cmdFifo, SSTCP_PKT2|
             clip0minBit|
             clip0maxBit|
             dstBaseAddrBit|
             dstFormatBit|
             srcBaseAddrBit|
             commandExBit|
             srcFormatBit|
             srcXYBit|
             dstSizeBit|
             dstXYBit|
             commandBit);
        
   	SET(cmdFifo, lph3g->clip0min, 0);
   	SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorExclusionStart));
      SET(cmdFifo, lph3g->dstFormat, dstFormat);
      SET(cmdFifo, lph3g->srcBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo, lph3g->commandEx, 0x0);
      SET(cmdFifo, lph3g->srcFormat, _FF(ddPrimarySurfaceData.dwFormat));


      SET(cmdFifo, lph3g->srcXY, ((DWORD) y << 16) | (DWORD) x );
      SET(cmdFifo, lph3g->dstSize, ((DWORD) Ysize << 16) | (DWORD)Xsize );
      SET(cmdFifo, lph3g->dstXY,  0UL);
      SETC(cmdFifo, lph3g->command,
           SST_WX_GO |
           0xCC000000 |
           SST_WX_BLT);
        
      SETPH(cmdFifo,   SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit);
      SET(cmdFifo,lph3g->dstBaseAddr, _FF(gdiDesktopStart)); 
      SET(cmdFifo,lph3g->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
      BUMP(15);

      CMDFIFO_EPILOG(cmdFifo);
      }  

    _FF(LastCursorPosX)= x1;
    _FF(LastCursorPosY)= y1;

    return(TRUE);
 
}

 
/*----------------------------------------------------------------------
Function name:  RestoreCursorExclude

Description:    Restore the area of the display where the cursor was
                drawn.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int RestoreCursorExclude(int x, int y)
{
    DWORD srcFormat;
    SHORT Xsize;
    SHORT Ysize;

    CMDFIFO_PROLOG(cmdFifo);
    
    if (_FF(gdiFlags) & (SDATA_GDIFLAGS_HWC_EXCLUSIVE | SDATA_GDIFLAGS_CURSOR_EXCLUDE))
   	return TRUE;

    Xsize = 32;
    if (x < 0)
      {
      Xsize += x;
      x = 0;
      }
    Ysize = 32;
    if (y < 0)
      {
      Ysize += y;
      y = 0;
      }

    //Have we been clipped?
    if ((Xsize > 0) && (Ysize > 0))
      { 
      CMDFIFO_SETUP(cmdFifo);

      CMDFIFO_CHECKROOM(cmdFifo,15 );

      SETPH(cmdFifo, SSTCP_PKT2|
             clip0minBit|
             clip0maxBit|
             dstBaseAddrBit|
             dstFormatBit|
             srcBaseAddrBit|
             commandExBit|
             srcFormatBit|
             srcXYBit|
             dstSizeBit|
             dstXYBit|
             commandBit);
 
      SET(cmdFifo, lph3g->clip0min, 0);
      SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
      SET(cmdFifo, lph3g->dstBaseAddr,_FF(gdiDesktopStart)); 
      SET(cmdFifo, lph3g->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
      SET(cmdFifo, lph3g->srcBaseAddr, _FF(SWcursorExclusionStart));
      SET(cmdFifo, lph3g->commandEx, 0x0);
      srcFormat = _FF(ddPrimarySurfaceData.dwFormat);
      srcFormat &= ~(SST_WX_SRC_PACK);
      srcFormat &= 0xFFFFC000;
      srcFormat |= EXCLUSION_BMP_STRIDE;
      SET(cmdFifo, lph3g->srcFormat, srcFormat);
      SET(cmdFifo, lph3g->srcXY, 0UL);

      SET(cmdFifo, lph3g->dstSize, ((DWORD)Ysize << 16) | (DWORD)Xsize);
      SET(cmdFifo, lph3g->dstXY, (((DWORD) y <<16) | (DWORD) x ) );
      SETC(cmdFifo, lph3g->command,
            SST_WX_GO |
            0xCC000000 |
            SST_WX_BLT);
 
      // restore src Base and src Format
       
      SETPH(cmdFifo,   SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit);
      SET(cmdFifo,lph3g->srcBaseAddr, _FF(gdiDesktopStart)); 
      SET(cmdFifo,lph3g->srcFormat, _FF(ddPrimarySurfaceData.dwFormat));
      BUMP(15);

      CMDFIFO_EPILOG(cmdFifo);
      }
 
    return(TRUE);
}

/*----------------------------------------------------------------------
Function name:  DrawCursor

Description:    Draws the cursor by blitting the bitmap in the
                SwCursorbitmapStart.
Information:

Return:         int     TRUE = early return due to EXCLUSIVE mode
                        0    = Successfully drawn
----------------------------------------------------------------------*/
int DrawCursor(int X, int Y)
{
   DWORD dstFormat;
   SHORT xSize;
   SHORT ySize;
   SHORT xCur;
   SHORT yCur;

   CMDFIFO_PROLOG(cmdFifo);

   if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
   	return TRUE;
   
   if ((SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))) &&
       (SDATA_GDIFLAGS_CURSOR_EXCLUDE != (SDATA_GDIFLAGS_CURSOR_EXCLUDE & _FF(gdiFlags))))
      {

      // if X is less then zero then fix it up
      xSize = 32;
      xCur = 0;
      if (X < 0)
         {
         xSize = xSize + X;
         xCur = 32 - xSize;
         X = 0;  
         }

      // Ditto with Y
      ySize = 32;
      yCur = 0;
      if (Y < 0)
         {
         ySize = ySize + Y;
         yCur = 32 - ySize;
         Y = 0;  
         }

      // if the cursor is clipped no need to draw it
      if ((xSize < 1) || (ySize < 1))
         return 0;

      CMDFIFO_SETUP(cmdFifo);
 
      dstFormat = _FF(ddPrimarySurfaceData.dwFormat);
      dstFormat &= ~(SST_WX_SRC_PACK);
      dstFormat &= 0xFFFFC000;
      dstFormat |= EXCLUSION_BMP_STRIDE;
        
      // Get Screen
      CMDFIFO_CHECKROOM(cmdFifo, 12);
      SETPH(cmdFifo, SSTCP_PKT2|
             clip0minBit|
             clip0maxBit|
             dstBaseAddrBit|
             dstFormatBit|
             srcBaseAddrBit|
             commandExBit|
             srcFormatBit|
             srcXYBit|
             dstSizeBit|
             dstXYBit|
             commandBit);
        
   	SET(cmdFifo, lph3g->clip0min, 0);
   	SET(cmdFifo, lph3g->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
      SET(cmdFifo, lph3g->dstBaseAddr, _FF(SWcursorSrcStart));
      SET(cmdFifo, lph3g->dstFormat, dstFormat);
      SET(cmdFifo, lph3g->srcBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo, lph3g->commandEx, 0x0);
      SET(cmdFifo, lph3g->srcFormat, _FF(ddPrimarySurfaceData.dwFormat));

      SET(cmdFifo, lph3g->srcXY, ((DWORD) Y << 16) | (DWORD) X );
      SET(cmdFifo, lph3g->dstSize, ((DWORD) ySize << 16) | (DWORD)xSize );
      SET(cmdFifo, lph3g->dstXY,((DWORD) yCur << 16) | (DWORD)xCur);
      SETC(cmdFifo, lph3g->command,
           SST_WX_GO |
           0xCC000000 |
           SST_WX_BLT);

      BUMP(12);        
      CMDFIFO_CHECKROOM(cmdFifo, 7);
      SETPH(cmdFifo, SSTCP_PKT2|
            srcBaseAddrBit     |
            srcFormatBit       |
            srcXYBit           |
            dstSizeBit         |
            dstXYBit           |
            commandBit
            );  
        
      SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorAndStart));
      SET(cmdFifo,  _FF(lpGRegs)->srcFormat, _FF(SWcursorFormat));
      SET(cmdFifo,  _FF(lpGRegs)->srcXY, ((DWORD) yCur << 16) | (DWORD)xCur);
      SET(cmdFifo,  _FF(lpGRegs)->dstSize , ((DWORD) ySize << 16) | (DWORD)xSize );
      SET(cmdFifo,  _FF(lpGRegs)->dstXY, ((DWORD) yCur << 16) | (DWORD)xCur);
      SETC(cmdFifo,  _FF(lpGRegs)->command,
           SST_WX_GO|
           SST_WX_BLT|
           SST_WX_ROP_AND << SST_WX_ROP0_SHIFT);
        
      BUMP(7);

      CMDFIFO_CHECKROOM(cmdFifo, 3);
      SETPH(cmdFifo, SSTCP_PKT2|
            srcBaseAddrBit     |
            commandBit
            );  
        
      SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorXorStart));
      SETC(cmdFifo,  _FF(lpGRegs)->command,
           SST_WX_GO|
           SST_WX_BLT|
           SST_WX_ROP_XOR << SST_WX_ROP0_SHIFT);
        
      BUMP(3);

      CMDFIFO_CHECKROOM(cmdFifo, 7);
      SETPH(cmdFifo, SSTCP_PKT2|
            dstBaseAddrBit     |
            dstFormatBit       |
            srcBaseAddrBit     |
            srcFormatBit       |
            dstXYBit           |
            commandBit
            );  
        
      SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo, _FF(lpGRegs)->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
      SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorSrcStart));
      SET(cmdFifo, lph3g->srcFormat, dstFormat);
      SET(cmdFifo,  _FF(lpGRegs)->dstXY,( ((DWORD)(Y & 0x1FFF) <<16) ) | (DWORD)(X & 0x1FFF));
      SETC(cmdFifo,  _FF(lpGRegs)->command,
           SST_WX_GO|
           SST_WX_BLT|
           0xCC000000);
        
      BUMP(7);

      CMDFIFO_CHECKROOM(cmdFifo, 3);
      SETPH(cmdFifo,   SSTCP_PKT2|
              srcBaseAddrBit|
              srcFormatBit);
      SET(cmdFifo,_FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart)); 
      SET(cmdFifo,_FF(lpGRegs)->srcFormat, _FF(ddPrimarySurfaceData.dwFormat));
      BUMP(3);
      CMDFIFO_EPILOG(cmdFifo);
   }

   return 0;
}

/* ************************************************************ 
 *
 * DebugBltCursor is debug routine to see the whats in the
 * color SWcursorBitmapStart.
 * Draws the cursor to (x, y)
 *
 * ************************************************************ */

// ============================================================ 
//
//  Below are debug routines used to debug sw cursor.
//
// ============================================================ 

/*----------------------------------------------------------------------
Function name:  DebugBltCursor

Description:    Debug routine to see the what's in the color
                SWcursorBitmapStart.  Draws the cursor to (x, y).

Information:    Used for cursor debugging only.

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int DebugBltCursor(int x, int y)
{
    CMDFIFO_PROLOG(cmdFifo);

    if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
	return TRUE;

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

    CMDFIFO_SETUP(cmdFifo);

    CMDFIFO_CHECKROOM(cmdFifo, 9);
    SETPH(cmdFifo, SSTCP_PKT2|
          dstBaseAddrBit|
          dstFormatBit|
          srcBaseAddrBit|
          srcFormatBit|
          srcXYBit|
          dstSizeBit|
          dstXYBit|
          commandBit);
 
    SET(cmdFifo, lph3g->dstBaseAddr,_FF(gdiDesktopStart)); 
    SET(cmdFifo, lph3g->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
    SET(cmdFifo, lph3g->srcBaseAddr, _FF(SWcursorAndStart));
    SET(cmdFifo, lph3g->srcFormat, _FF(SWcursorFormat) );
    SET(cmdFifo, lph3g->srcXY, 0UL);
    SET(cmdFifo, lph3g->dstSize, ((32UL << 16) | 32UL ));
    SET(cmdFifo, lph3g->dstXY,( ((DWORD) y) << 16) | (DWORD) x );
    SETC(cmdFifo, lph3g->command,
        SST_WX_GO |
        0xCC000000 |
        SST_WX_BLT);
 
    BUMP(9);
 
    CMDFIFO_CHECKROOM(cmdFifo, 3);
    SETPH(cmdFifo,   SSTCP_PKT2|
          srcBaseAddrBit|
          srcFormatBit);
  
    SET(cmdFifo,lph3g->srcBaseAddr, _FF(gdiDesktopStart)); 
    SET(cmdFifo,lph3g->srcFormat, _FF(ddPrimarySurfaceData.dwFormat));
    BUMP(3);
 
    CMDFIFO_EPILOG(cmdFifo);
 
    return(TRUE);
 }
 

/*----------------------------------------------------------------------
Function name:  DebugBltExclusion

Description:    Debug routine to see the what's in the color
                SWcursorExclusion.

Information:    Used for cursor debugging only.

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int DebugBltExclusion(int x, int y)
{
    CMDFIFO_PROLOG(cmdFifo);

    if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
	return TRUE;

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

    CMDFIFO_SETUP(cmdFifo);
 
    CMDFIFO_CHECKROOM(cmdFifo, 9);
    SETPH(cmdFifo, SSTCP_PKT2|
          dstBaseAddrBit|
          dstFormatBit|
          srcBaseAddrBit|
          srcFormatBit|
          srcXYBit|
          dstSizeBit|
          dstXYBit|
          commandBit);
 
    SET(cmdFifo, lph3g->dstBaseAddr,_FF(gdiDesktopStart)); 
    SET(cmdFifo, lph3g->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
    SET(cmdFifo, lph3g->srcBaseAddr, _FF(SWcursorExclusionStart));
    SET(cmdFifo, lph3g->srcFormat, _FF(SWcursorFormat));
    SET(cmdFifo, lph3g->srcXY, 0UL);
    SET(cmdFifo, lph3g->dstSize, ((32UL << 16) | 32UL ));
    SET(cmdFifo,  lph3g->dstXY , (((DWORD) y << 16)| (DWORD) x) );
    SETC(cmdFifo, lph3g->command,
        SST_WX_GO |
        0xCC000000 |
        SST_WX_BLT);
 
    BUMP(9);
 
    CMDFIFO_CHECKROOM(cmdFifo, 3);
    SETPH(cmdFifo,   SSTCP_PKT2|
          srcBaseAddrBit|
          srcFormatBit);
  
    SET(cmdFifo,lph3g->srcBaseAddr, _FF(gdiDesktopStart)); 
    SET(cmdFifo,lph3g->srcFormat, _FF(ddPrimarySurfaceData.dwFormat));
    BUMP(3);
 
    CMDFIFO_EPILOG(cmdFifo);
    return(TRUE);
}
 

/*----------------------------------------------------------------------
Function name:  ClearCursorRect

Description:    Debug routine to draw white 32x32 box.

Information:    Used for cursor debugging only.  Appears to be unused!

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int ClearCursorRect(int x, int y)
{
    CMDFIFO_PROLOG(cmdFifo);

    if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
	return TRUE;

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY; 

    CMDFIFO_SETUP(cmdFifo);
 
    CMDFIFO_CHECKROOM(cmdFifo,7 );
    SETPH(cmdFifo, SSTCP_PKT2|
          dstBaseAddrBit     |
          dstFormatBit       |
          colorForeBit       |
          dstXYBit           |
          dstSizeBit         |
          commandBit );
    
    SET(cmdFifo,  lph3g->dstBaseAddr, _FF(gdiDesktopStart));
    SET(cmdFifo,  lph3g->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
    SET(cmdFifo,  lph3g->colorFore, 0xFFFFFFFFL)
    SET(cmdFifo,  lph3g->dstSize , (32UL << 16)| 32UL );
    SET(cmdFifo,  lph3g->dstXY , ((DWORD) y << 16)| (DWORD)x );
    SETC(cmdFifo,  lph3g->command,
         SST_WX_RECTFILL |
         SST_WX_GO|
         SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT);
    
    BUMP(7);
 
    CMDFIFO_EPILOG(cmdFifo);
    return(TRUE);
}

  
/*----------------------------------------------------------------------
Function name:  ShowCursor

Description:    Debug routine to draw cursor.

Information:    Used for cursor debugging only.  Appears to be unused!

Return:         int     0 = Always returned
----------------------------------------------------------------------*/
int ShowCursor(int nX, int nY)
{
   CMDFIFO_PROLOG(cmdFifo);
   CMDFIFO_SETUP(cmdFifo);
 
    CMDFIFO_CHECKROOM(cmdFifo, 14 );

    SETPH(cmdFifo, CMDFIFO_BUILD_2DPK1( 2, srcColorkeyMin ));
    SET(cmdFifo,  _FF(lpGRegs)->srcColorkeyMin, 0x13141314L );
    SET(cmdFifo,  _FF(lpGRegs)->srcColorkeyMax, 0x13141314L );

    SETPH(cmdFifo, SSTCP_PKT2|
                   dstBaseAddrBit     |
                   dstFormatBit       |
                   ropBit             |
                   srcBaseAddrBit     |
                   commandExBit       |
                   srcFormatBit       |
                   srcXYBit           |
                   dstSizeBit         |
                   dstXYBit           |
                   commandBit
                   );  
        
   SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, _FF(gdiDesktopStart));
   SET(cmdFifo, _FF(lpGRegs)->dstFormat, _FF(ddPrimarySurfaceData.dwFormat));
   // set rop1 
   SET(cmdFifo,  _FF(lpGRegs)->rop, (SST_WX_ROP_DST<< 8) );
   SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorAndStart));
   SET(cmdFifo,  _FF(lpGRegs)->commandEx, SST_WX_EN_SRC_COLORKEY_EX);
   SET(cmdFifo,  _FF(lpGRegs)->srcFormat, _FF(SWcursorFormat));
   SET(cmdFifo,  _FF(lpGRegs)->srcXY, 0L);
   SET(cmdFifo,  _FF(lpGRegs)->dstSize , (32UL << 16)| 32UL );
   SET(cmdFifo,  _FF(lpGRegs)->dstXY,( ((DWORD) nY <<16) ) | 
                               (DWORD) nX);
   SETC(cmdFifo,  _FF(lpGRegs)->command,
                  SST_WX_GO|
                  SST_WX_BLT|
                  0xCC000000);
        
   BUMP(14);
   CMDFIFO_EPILOG(cmdFifo);

   return 0;
}


/*----------------------------------------------------------------------
Function name:  GottaCursor

Description:    Debug routine to get a cursor.

Information:    Used for cursor debugging only.
                Has an "_asm int 3" statement.
                Surrounded by "#ifdef DEBUG_CURSOR"

Return:         int     nReturn
----------------------------------------------------------------------*/
#ifdef DEBUG_CURSOR
int nGottaCursor = 0;
int GottaCursor(void)
{
   DWORD dwSave = _FF(lfbBase) + _FF(SWcursorExclusionStart);
   DWORD dwCursor = _FF(lfbBase) + _FF(SWcursorBitmapStart);
   DWORD dw1;
   DWORD dw2;
   DWORD dwMask;
   int i;
   int j;
   int nReturn = 0;
   int nOffset;
   int nCount = 0;
   int nTotal = 0;

   FXWAITFORIDLE();
 
   nOffset = 0;
   for (j=0; j<32; j++)
      {
      for (i=0; i<16; i++)
         {
         dw1 = h3READ(0x0, (DWORD *)(dwCursor + nOffset + (i<<2)));
         if (dw1 == 0x13141314L)
            continue;
         if ((dw1 & 0xFFFF0000L) == 0x13140000L)
            dwMask = 0xFFFFL;
         else if ((dw1 & 0xFFFFL) == 0x1314L)        
            dwMask = 0xFFFF0000L;
         else
            dwMask = 0xFFFFFFFFL;
      
         dw2 = h3READ(0x0, (DWORD *)(dwSave + nOffset + (i<<2)));
         
         if ((dw1 & dwMask) == (dw2 & dwMask))
            nCount++;
         nTotal++;
         }

      nOffset += 0x80;    
      }

   if ((nTotal) && (nTotal == nCount))
      {
      nReturn = 0x01;
      if (nGottaCursor)
         __asm  int 03
      }

   return nReturn;
}
#endif


/*----------------------------------------------------------------------
Function name:  RestoreCursor

Description:    Restores the cursor shape if the cursor is Enabled.

Information:    

Return:         int     00 = Always returned
----------------------------------------------------------------------*/
int RestoreCursor(void)
{
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_ENABLED)
    {
        SetCursor((CURSORSHAPE FAR *)&SaveCursor);
    }

   return 0;
}


/*----------------------------------------------------------------------
Function name:  CheckAndExpand

Description:    Checks expansion of cursor AND mask.

Information:    Used for cursor debugging only.
                Has an "_asm int 3" statement.
                Surrounded by "#ifdef CHECK_SW_CURSOR"

Return:         int     00 = Always returned
----------------------------------------------------------------------*/
#ifdef CHECK_SW_CURSOR
int CheckAndExpand(DWORD * lpAndMask, DWORD dwCursor, DWORD dwBpp)
{
   DWORD AndMask;
   DWORD dw1;
   DWORD dw2;
   DWORD dwFB;
   DWORD dwMask;
   int i;
   int j;
   int k;
   int nSize;
   BYTE AndByte;

   if (8 == dwBpp)
      {
      nSize = 1;
      dwMask = 0xFFL;
      }
   else if (16 == dwBpp)
      {
      nSize = 2;
      dwMask = 0xFFFFL;
      }
   else if (24 == dwBpp)
      {
      nSize = 3;
      dwMask = 0xFFFFFFL;
      }
   else if (32 == dwBpp)
      {
      nSize = 4;
      dwMask = 0xFFFFFFL;
      }

   FXWAITFORIDLE();

   for (i=0; i<32; i++)
      {
      AndMask = lpAndMask[i];
      dwFB = dwCursor;
      dwCursor += 128;
      for (j=0; j<4; j++) 
         {
            AndByte = (BYTE)(AndMask & 0xFF);
            for (k=0; k<8; k++)
               {
               dw2 = h3READ(0x0, (DWORD *)dwFB) & dwMask;
               dwFB += nSize;

               if (AndByte & 0x80)
                  dw1 = 0x13141314L & dwMask;
               else
                  dw1 = 0xFFFFFFFFL & dwMask;
               if (dw1 != dw2)
                  _asm int 03;   
               AndByte <<=1;         
               }
         AndMask >>= 8;
         }
      }
   
   return 0;
}


/*----------------------------------------------------------------------
Function name:  CheckXorExpand

Description:    Checks expansion of cursor XOR mask.

Information:    Used for cursor debugging only.
                Has an "_asm int 3" statement.
                Surrounded by "#ifdef CHECK_SW_CURSOR"

Return:         int     00 = Always returned
----------------------------------------------------------------------*/
int CheckXorExpand(BYTE * lpXorMask, DWORD * lpAndMask, DWORD dwCursor, DWORD dwBpp)
{
   DWORD AndMask;
   DWORD dw1;
   DWORD dw2;
   DWORD dwFB;
   DWORD dwMask;
   int i;
   int j;
   int nSize;
   int k;
   int l;
   BYTE AndByte;

   if (8 == dwBpp)
      {
      nSize = 1;
      dwMask = 0xFFL;
      }
   else if (16 == dwBpp)
      {
      nSize = 2;
      dwMask = 0xFFFFL;
      }
   else if (24 == dwBpp)
      {
      nSize = 3;
      dwMask = 0xFFFFFFL;
      }
   else if (32 == dwBpp)
      {
      nSize = 4;
      dwMask = 0xFFFFFFL;
      }

   FXWAITFORIDLE();

   for (i=0; i<32; i++)
      {
      AndMask = lpAndMask[i];
      dwFB = dwCursor;
      dwCursor += 128;
      for (j=0; j<4; j++) 
         {
         AndByte = (BYTE)(AndMask & 0xFF);
         AndMask >>= 8;
         for (k=0; k<8; k++)
            {
            if (AndByte & 0x80)
               dw1 = 0x13141314L & dwMask;
            else
               {
               dw1 = 0;
               for (l=0; l<nSize; l++)
                  {
                  dw1 |= (((DWORD)lpXorMask[l]) << (8*l));
                  }
               dw1 &= dwMask;
               }
            dw2 = h3READ(0x0, (DWORD *)dwFB) & dwMask;
            dwFB += nSize;
            lpXorMask += nSize;
            if (dw1 != dw2)
               _asm int 03
            AndByte <<= 1;
            }
         }
      }
   
   return 0;
}

#endif

#undef LONG
#include "iSST2.h"
#include "iR3.h"
#include "iHydra.h"
#include "iRage.h"

void CsimVideoUpdate()
{
    DWORD dwCount;
    DWORD dwDevNode;

    {
        SST2INTERFACE iSST2;
        SST2HOSTINFO hostInfo;
        SST2DISPLAYMASK mask;

        if(SST2_Get_Interface(&iSST2, SST2_VXD_ID) != SST2RET_OK)
            goto SST2ConnectFail;

        dwCount = 1;

        if(SST2_Get_Device_Devnodes(iSST2, &dwCount, &dwDevNode) != SST2RET_OK)
        {
            SST2_Release_Interface(iSST2);
            goto SST2ConnectFail;
        }

        if(dwCount != 1)
        {
            SST2_Release_Interface(iSST2);
            goto SST2ConnectFail;
        }
     
        mask.Display_0=1;
        mask.Display_1=0;

        //Update SST2 device simulator video section
        if ( SST2_Device_Video_Update(iSST2, dwDevNode, mask) != SST2RET_OK )
        {
            SST2_Release_Interface(iSST2);
            goto SST2ConnectFail;
        }

        SST2_Release_Interface(iSST2);

        goto ConnectDone;
    }
SST2ConnectFail: ;

    //-------------------------------------------------------------
    // Now try R3 interface
    //-------------------------------------------------------------
    {
        R3INTERFACE iR3;
        R3HOSTINFO hostInfo;
        R3DISPLAYMASK mask;
     
        if(R3_Get_Interface(&iR3, R3_VXD_ID) != R3RET_OK)
            goto R3ConnectFail;

        dwCount = 1;

        if(R3_Get_Device_Devnodes(iR3, &dwCount, &dwDevNode) != R3RET_OK)
        {
            R3_Release_Interface(iR3);
            goto R3ConnectFail;
        }

        if(dwCount != 1)
        {
            R3_Release_Interface(iR3);
            goto R3ConnectFail;
        }
         
        mask.Display_0=1;
        mask.Display_1=0;

        //Update R3 device simulator video section
        if ( R3_Device_Video_Update(iR3, dwDevNode, mask) != R3RET_OK )
        {
            R3_Release_Interface(iR3);
            goto R3ConnectFail;
        }

        R3_Release_Interface(iR3);

        goto ConnectDone;
    }
R3ConnectFail: ;

    //-------------------------------------------------------------
    // Now try Hydra interface
    //-------------------------------------------------------------
    {
        HYDRAINTERFACE iHydra;
        HYDRAHOSTINFO hostInfo;
        HYDRADISPLAYMASK mask;
     
        if(Hydra_Get_Interface(&iHydra, HYDRA_VXD_ID) != HYDRARET_OK)
            goto HydraConnectFail;

        dwCount = 1;

        if(Hydra_Get_Device_Devnodes(iHydra, &dwCount, &dwDevNode) != HYDRARET_OK)
        {
            Hydra_Release_Interface(iHydra);
            goto HydraConnectFail;
        }

        if(dwCount != 1)
        {
            Hydra_Release_Interface(iHydra);
            goto HydraConnectFail;
        }
         
        mask.Display_0=1;
        mask.Display_1=0;

        //Update Hydra device simulator video section
        if ( Hydra_Device_Video_Update(iHydra, dwDevNode, mask) != HYDRARET_OK )
        {
            Hydra_Release_Interface(iHydra);
            goto HydraConnectFail;
        }

        Hydra_Release_Interface(iHydra);

        goto ConnectDone;
    }
HydraConnectFail: ;

    //-------------------------------------------------------------
    // Now try Rage interface
    //-------------------------------------------------------------
    {
        RAGEINTERFACE iRage;
        RAGEHOSTINFO hostInfo;
        RAGEDISPLAYMASK mask;
     
        if(Rage_Get_Interface(&iRage, RAGE_VXD_ID) != RAGERET_OK)
            goto RageConnectFail;

        dwCount = 1;

        if(Rage_Get_Device_Devnodes(iRage, &dwCount, &dwDevNode) != RAGERET_OK)
        {
            Rage_Release_Interface(iRage);
            goto RageConnectFail;
        }

        if(dwCount != 1)
        {
            Rage_Release_Interface(iRage);
            goto RageConnectFail;
        }
         
        mask.Display_0=1;
        mask.Display_1=0;

        //Update Rage device simulator video section
        if ( Rage_Device_Video_Update(iRage, dwDevNode, mask) != RAGERET_OK )
        {
            Rage_Release_Interface(iRage);
            goto RageConnectFail;
        }

        Rage_Release_Interface(iRage);

        goto ConnectDone;
    }
RageConnectFail: ;

ConnectDone: ;
}


void csimUpdateAgpConfig( DWORD physBase, DWORD linBase, DWORD sizeInBytes )
{
    DWORD dwCount;
    DWORD dwDevNode;

    {
        SST2INTERFACE iSST2;
   
        if(SST2_Get_Interface(&iSST2, (DWORD) SST2_VXD_ID) != SST2RET_OK)
            goto SST2ConnectFail;

        dwCount = 1;

        if(SST2_Get_Device_Devnodes(iSST2, &dwCount, &dwDevNode) != SST2RET_OK)
        {
           SST2_Release_Interface(iSST2);
           goto SST2ConnectFail;
        }

        if(dwCount != 1)
        {
           SST2_Release_Interface(iSST2);
           goto SST2ConnectFail;
        }

        SST2_Enable_Device_AGP(iSST2, 
                               dwDevNode,
                               physBase,
                               linBase,
                               sizeInBytes );
 
        SST2_Release_Interface(iSST2);

        goto ConnectDone;
    }
SST2ConnectFail: ;

    {
        R3INTERFACE iR3;
        DWORD dwCount;
        DWORD dwDevNode;
   
        if(R3_Get_Interface(&iR3, (DWORD) R3_VXD_ID) != R3RET_OK)
            goto R3ConnectFail;

        dwCount = 1;

        if(R3_Get_Device_Devnodes(iR3, &dwCount, &dwDevNode) != R3RET_OK)
        {
           R3_Release_Interface(iR3);
           goto R3ConnectFail;
        }

        if(dwCount != 1)
        {
           R3_Release_Interface(iR3);
           goto R3ConnectFail;
        }

        R3_Enable_Device_AGP(iR3, 
                             dwDevNode,
                             physBase,
                             linBase,
                             sizeInBytes );
 
        R3_Release_Interface(iR3);

        goto ConnectDone;
    }
R3ConnectFail: ;

    {
        HYDRAINTERFACE iHydra;
        DWORD dwCount;
        DWORD dwDevNode;
   
        if(Hydra_Get_Interface(&iHydra, (DWORD) HYDRA_VXD_ID) != HYDRARET_OK)
            goto HydraConnectFail;

        dwCount = 1;

        if(Hydra_Get_Device_Devnodes(iHydra, &dwCount, &dwDevNode) != HYDRARET_OK)
        {
           Hydra_Release_Interface(iHydra);
           goto HydraConnectFail;
        }

        if(dwCount != 1)
        {
           Hydra_Release_Interface(iHydra);
           goto HydraConnectFail;
        }

        Hydra_Enable_Device_AGP(iHydra, 
                                dwDevNode,
                                physBase,
                                linBase,
                                sizeInBytes );
 
        Hydra_Release_Interface(iHydra);

        goto ConnectDone;
    }
HydraConnectFail: ;

    {
        RAGEINTERFACE iRage;
        DWORD dwCount;
        DWORD dwDevNode;
   
        if(Rage_Get_Interface(&iRage, (DWORD) RAGE_VXD_ID) != RAGERET_OK)
            goto RageConnectFail;

        dwCount = 1;

        if(Rage_Get_Device_Devnodes(iRage, &dwCount, &dwDevNode) != RAGERET_OK)
        {
           Rage_Release_Interface(iRage);
           goto RageConnectFail;
        }

        if(dwCount != 1)
        {
           Rage_Release_Interface(iRage);
           goto RageConnectFail;
        }

        Rage_Enable_Device_AGP(iRage, 
                               dwDevNode,
                               physBase,
                               linBase,
                               sizeInBytes );
 
        Rage_Release_Interface(iRage);

        goto ConnectDone;
    }
RageConnectFail: ;

ConnectDone: ;
}

