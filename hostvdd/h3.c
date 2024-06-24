/* $Header: h3.c, 33, 11/5/00 3:10:15 PM PST, Ryan Bissell$ */
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
** File name:   h3.c
**
** Description: Enable/Disable and supporting functions, structures,
**              macros, etc. for the Banshee/Avenger HW.
**
** $Log: 
**  33   3dfx      1.32        11/5/00  Ryan Bissell    NML phase II
**  32   3dfx      1.31        11/2/00  Michel Conrad   Adding extEscape support
**       for Rampage HW Perfromance counters.
**  31   3dfx      1.30        11/1/00  Ryan Bissell    Forgot to remove an 
**       "#ifdef CSERVICE" in this file.  Also added code to calculate 
**       mmPhantomHeapSize and mmPhantomHeapStart.
**  30   3dfx      1.29        11/1/00  Ryan Bissell    Cleanup of ddmemmgr.c, 
**       and related files.
**  29   3dfx      1.28        10/31/00 Ryan Bissell    Phase I of development 
**       of the "No Man's Land" reclamation heap.
**  28   3dfx      1.27        10/5/00  Dan O'Connel    Correct code which reads
**       or sets PllCtrl0 and PllCtrl1 registers.  Most of this code was 
**       originally incompletely ported from Napalm.  Also cleanup (mostly by 
**       removing) some other unused or obsolete code concerning Alternate 
**       Timings for DFPs and SGRAMMODE.
**  27   3dfx      1.26        10/4/00  Dale  Kenaston  New Sage macros. Changed
**       _FF(doSageCF) in FXWAITFORIDLE to IS_SAGE_ACTIVE.
**  26   3dfx      1.25        9/22/00  Evan Leland     fixed build break 
**  25   3dfx      1.24        9/22/00  Dale  Kenaston  Sage packet3, register 
**       and quickturn initialization changes  Modified fxwaitforidle to flush 
**       Sage using a tagged mop and busy poll. Removed HW_TNL from code that 
**       copies gebase and gephysbase from hwinfo to globaldata.
**  24   3dfx      1.23        9/19/00  Brian Danielson Placed BITMAP_CACHING 
**       ifdef around bitmap caching changes that were recently added (August 
**       28-30). Added compiler option to enable / disable BMC.
**  23   3dfx      1.22        8/30/00  John Zhang      Fixed for bitmapcache
**  22   3dfx      1.21        8/10/00  Dale  Kenaston  Sage membase, fifo 
**       initialization and packet 3 size setup
**  21   3dfx      1.20        8/4/00   Ryan Bissell    Added support for true 
**       AGP command FIFOs, and long JSR support in Central Services (needed 
**       because of AGP command FIFOs.)
**  20   3dfx      1.19        8/3/00   Miles Smith     Added code to check 
**       registry for a flag to skip the reserving of extra desktop memory for a
**       super sized AA surface.
**  19   3dfx      1.18        7/17/00  Dan O'Connel    Delete obsolete code 
**       that was ifdef'ed or commented out in previous checkin.
**  18   3dfx      1.17        7/17/00  Dan O'Connel    Major changes ported 
**       from Napalm driver to support: Registry Controlled Modes, DFP, TvOut, 
**       read OEM config from BIOS, updated 3dfx Tools support, and other 
**       features and bug fixes.
**  17   3dfx      1.16        7/14/00  Ryan Bissell    Partial fix for "report 
**       free memory" size bug in fxmm
**  16   3dfx      1.15        6/7/00   Evan Leland     adds preliminary 
**       instrumentation to Rampage driver
**  15   3dfx      1.14        4/26/00  Tim Little      Added initialization of 
**       GE membase
**  14   3dfx      1.13        4/14/00  Ryan Bissell    More Central Services 
**       deployment
**  13   3dfx      1.12        3/6/00   Ryan Bissell    Deployment of Central 
**       Services
**  12   3dfx      1.11        2/4/00   Xing Cong       Call set_overlaygamma in
**       Enable1()
**  11   3dfx      1.10        2/1/00   Michel Conrad   Comments on reporting 
**       phantom vs "true" heap size.
**  10   3dfx      1.9         1/14/00  Miles Smith     Moves start of linear 
**       heap down if Fullscreen aa is enabled
**  9    3dfx      1.8         12/22/99 Ryan Bissell    New clut management code
**  8    3dfx      1.7         11/18/99 Michel Conrad   Support for AGP execute 
**       mode tweak variables. Removed 1000 lines of old checks in comments. 
**       Rearranged code to avoid internal complier error in 
**       setupOffscreenMemory routine.
**  7    3dfx      1.6         10/27/99 Andrew Sobczyk  Changed FXWAITFORIDLE to
**       read status from all chips if in SLI mode
**  6    3dfx      1.5         10/20/99 Kyle Pratt      clean up of if(0) and 
**       ifndef r21
**  5    3dfx      1.4         9/29/99  Mark Einkauf    init 
**       lastAccessType=ACCESS_NONE
**  4    3dfx      1.3         9/24/99  Mark Einkauf    temporarily reduce AGP 
**       fifo size to 128KB, from 4MB to reduce latency for QuickTurn testing  
**  3    3dfx      1.2         9/22/99  Mark Einkauf    timeout and int1 in wait
**       for idle, #if 0 around CAM init for 8.0 netlist decode bug workaround
**  2    3dfx      1.1         9/16/99  Mark Einkauf    Enable AGP CmdFifo for 
**       whole driver - dd16,minivdd,dd32,d3d
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
**
*/


#include <string.h>
#include "header.h"
#include "sst2glob.h"

#include <memory.h>


#include "valmode.inc"
#include "3dfx.h"
#include "memmgr16.h"

#include "modelist.h"
#include "qmodes.h"
extern int tvoutGetBootStatus();

#define Not_VxD
#include "minivdd.h"

#define DDFXS32_DLLNAME "3dfx32vx.dll"

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

#include "hwcext.h"
#include "cursor.h"

#include <vmm.h>
#define MIDL_PASS     // suppress 32-bit only #pragma pack(push)

#include "regkeys.h"


#pragma warning (disable: 4047 4704)
#include <configmg.h>
#pragma warning (default: 4047 4704)

#include "gramp.h"
#include "fxtvout.h"

// edgetools stuff Paul Magee 21 Jan 99
#define STB_EDGETOOLS  //for testing
#ifdef  STB_EDGETOOLS
#include "edgeesc.h"
#endif  // STB_EDGETOOLS

#ifdef CSERVICE
#include "cservice.h"
#endif

static DWORD RNDUP_16KB( DWORD size )        { size+=0x3fffL; return(size & ~0x3fffL); }
static DWORD RNDUP_T0_PGWIDTH( DWORD width ) { width +=0xfL;  return(width & ~0xfL);   }
static DWORD RNDUP_T1_PGWIDTH( DWORD width ) { width += 0x3L; return(width & ~0x3L);   }

/* 
  16 bit data seg. starts on word boundary, pad up for DriverData so it
  aligns on dword boundary.
*/

WORD hwAll=1;

#ifdef GBLDATA_IN_PDEV
PDEV FAR * PASCAL lpDriverPDevice;
#else
GLOBALDATA      DriverData = { 0 };
#endif

/* ------------------ !!! ADD NEW VARIABLES BELOW. !!! ------------------*/

///////DELETE ME ///////DWORD ColorTable8BPP[256];

HANDLE hCsimLib;
SstRegs * sst;
SstGRegs h3g;

// assume that fifocache0 will be shared with d3d/dd
FIFOCACHE     fifocache0;
FIFOCACHE     *lpfifocache0;
SstGRegs      *lph3g;
SstCRegs      *lph3agp;
CmdFifo       *lph3cmdfifo0;
SstIORegs     *lph3IORegs;
SstRegs       *lph3_3d;


SstVidRegs    *lph3VidRegs;


#ifndef  BITMAP_CACHE
DWORD         cmdStartAddress =  PHYS_CMDFIFO_BASE_ADDR;
#endif

int IS_CSIM = 0;
int AGP        = 0;
int MANUAL_BUMP = 0;
int FIRST = 1;
int FIRSTPRIME = 0;
int INVARREG = 0;

#ifdef CRASHTEST
FxU32 doCT = 0;
#endif // #ifdef CRASHTEST

extern DWORD dwDeviceHandle;
extern DISPLAYINFO DisplayInfo;
///////DELETE ME ///////extern DWORD GammaTable[256];
extern int nNumModes;
extern int useDefaultMode;

/***************************************************************************
 *
 * globals
 *
 ***************************************************************************/

GLOBALDATA      *lpDriverData;
//GLOBALDATA      SST1Data;
HMODULE         hModule;

HANDLE hLib16;

DWORD fb_addr;
DWORD sel;
DWORD base_ptr;
DWORD tex_ptr;
DWORD fb_ptr;

DWORD Palettized;               // Global palettized mode flag 1=Y, 0=N.

// for enabling hw features on the fly.
/*
    hwall hwxxx    !(hwall&hwxxx)
    0        0        1
    0        1        1
    1        0        1
    1        1        0     (don't do dibeng)
*/

WORD hwBlt=1; hwBitBltSS=1, hwBitBltPS=1, hwBitBltHS=1 ;
WORD hwStretchBlt=1; hwStretchBltSS=1; hwStretchBltHS=1; hwStretchBltPS=1;

WORD hwText=1, hwTextLPDX=1; hwTextNoLPDX=1;
WORD hwOutput =1;
WORD hwAltPolygon=0, hwWindPolygon=0;
WORD hwPolyline=1;
WORD hwRect=1;
WORD hwPolyScanline=1, hwScanline =1 ;
WORD hwDibBlt =0 ; hwDibToDevice=0;
int  PacketCount=0;
DWORD  LastPtr=0;

DWORD * buffer;

#define LFB_BASE _FF(lfbBase)
/***************************************************************************
 *
 * internal functions.
 *
 ***************************************************************************/

DWORD GetRegInt(LPSTR valname, DWORD def);
int   di_FindMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwFlags);
UINT  GetFlatSel(void);
void FreeFlatSel(void);
int ddatoi( const char *s ); // mls - 8/3/00

DWORD PhysToLinear(DWORD PhysAddress, DWORD Size);
BOOL  DDCreateDriverObject(BOOL bReset);
static void CmdFifo0Init(FIFOCACHE * fifo0,
                                    DWORD fifoStart,
                                    DWORD size );

extern BOOL InitThunks( DWORD dwBase, DWORD dwSize);
extern BOOL initFifo( DWORD dwSize, DWORD dwBase);
extern void FAR DbgOut(char *);

extern void FAR PASCAL hook_int2f(void);
extern void FAR PASCAL unhook_int2f(void);
extern void InitCursor(void);
extern void DiscardAllSSB(void);

extern void InitDeviceBitmapFilter(void);
extern void InitDeviceBitmap(void);
extern void EnableDeviceBitmaps(void);
extern void DoAllHost(void);
extern int tvoutSetStdInternal(void);
extern int tvoutDisableInternal(void);
extern int TVOutVideoParameters(LPVIDEOPARAMETERS);
extern void * __cdecl memset(void *, int, size_t);
extern void ClearMem(DWORD, DWORD);
extern void FAR * di_AllocModeTable( int nummodes );
extern void di_FillModeTable( MODEINFO FAR * info, int nummodes );
extern void di_SortModeTable( MODEINFO FAR * info, int nummodes );

extern char *ddgetenv( const char *varname); //mls 8/3/00

extern BOOL bSSB;        // in ssb.c

void DoMouseTrails(WORD wTrails);

LONG FAR PASCAL
hwcExt(DWORD *lpInput, DWORD *lpOutput) ;

void FAR PASCAL _loadds myBeginAccess(DIBENGINE FAR *pde, int left, int top, int right, int bottom, UINT flags);
void FAR PASCAL _loadds myEndAccess(DIBENGINE FAR *pde, UINT flags);
int DoIntersect (SHORT left, SHORT top, SHORT right, SHORT bottom);
#ifdef HAL_CSIM

UINT FAR PASCAL _loadds HostEnable1(LPVOID lpDevice, UINT style,
                                    LPSTR lpDeviceType, LPSTR lpOutput,
                                    LPVOID lpStuff);
void HostInitRegs();
#endif

/***************************************************************************
 *
 * Enable   called by GDI to enable the device and set the video mode
 *
 ***************************************************************************/

WORD FirstEnable = 1;           // =1 means driver is loading for the first
                                // time -- Enable() will do its minivdd
                                // start-up protocol, needed only only per
                                // boot, FirstEnable will be =0 thereafter

FARPROC         fpRepaintScreen;


/*----------------------------------------------------------------------
Function name:  ResetHiresMode

Description:    Presently does nothing.
                
Information:    Not presently used!

Return:         VOID
----------------------------------------------------------------------*/
void _loadds ResetHiresMode()
{
    int himom = 1;
}


/*----------------------------------------------------------------------
Function name:  _InquireInfo

Description:    Register driver capabilities in pdevice.
                
Information:    

Return:         UINT    size returned from DIB_Enable or,
                        0 if DIB_Enable failed.
----------------------------------------------------------------------*/
UINT
_InquireInfo(LPVOID lpDevice,
         UINT style,
         DWORD dwResolutionX,
         DWORD dwResolutionY,
         DWORD dwBPP)
{
    GDIINFO FAR *pdp;
    UINT size;

    DPF(DBGLVL_NORMAL,"InquireInfo.");

    size = DIB_Enable(lpDevice, style, NULL, NULL, NULL);

    if (size == 0)
    return 0;

    pdp = (GDIINFO FAR *)lpDevice;

    pdp->dpCaps1 |= C1_DIBENGINE;           // we are a mini-driver
    pdp->dpCaps1 |= C1_COLORCURSOR;         // we do color cursors
    pdp->dpCaps1 |= C1_REINIT_ABLE;         // we can re-enable
    pdp->dpCaps1 |= C1_BYTE_PACKED;         // we handle BYTE packed fonts
    pdp->dpCaps1 |= C1_GLYPH_INDEX;
    pdp->dpCaps1 |= C1_GAMMA_RAMP;

    DPF(DBGLVL_NORMAL,"dpCaps1 = 0X%x",pdp->dpCaps1);

    //pdp->dpRaster|=RC_BITBLT;
    //pdp->dpRaster|=RC_PALETTE;
    //pdp->dpRaster|=RC_DIBTODEV;
    //pdp->dpRaster|=RC_BIGFONT;
    //pdp->dpRaster|=RC_STRETCHBLT;
    //pdp->dpRaster|=RC_STRETCHDIB;

    // S3's setting
    pdp->dpRaster=0xeeb9;
    pdp->dpRaster |= RC_SAVEBITMAP;

    pdp->dpHorzRes   = (UINT)dwResolutionX; // screen width
    pdp->dpVertRes   = (UINT)dwResolutionY; // screen height
    pdp->dpBitsPixel = (UINT)dwBPP;         // screen bit depth
    pdp->dpDCManage = DC_IgnoreDFNP;        // Load a second Driver!!!!!

    //  pdp->dpLines=0x0023;
    //  pdp->dpLines|= LC_POLYSCANLINE;
    pdp->dpLines|= LC_POLYLINE;
    pdp->dpLines|= LC_STYLED;

    pdp->dpCurves=0x0089;

    pdp->dpPolygonals |= PC_ALTPOLYGON;
    pdp->dpPolygonals |= PC_RECTANGLE;
    pdp->dpPolygonals |= PC_SCANLINE;
    pdp->dpPolygonals |= PC_STYLED;
    pdp->dpPolygonals |= PC_INTERIORS;

    pdp->dpText=0x2004;
    pdp->dpClip=0x1;

    Palettized = 0;             // Initialize to Non-Palette Managed

    if (dwBPP < 8)
    {
    pdp->dpNumPens      = (1 << dwBPP); //# of pens driver realizes
    pdp->dpNumColors    = (1 << dwBPP); //# colors in color table
    }
    else if (dwBPP == 8)
    {
    pdp->dpNumPens      = 16;   //# of pens this driver realizes
    pdp->dpNumColors    = 20;   //# colors in color table
    pdp->dpNumPalReg    = 256;  //# palette registers
    pdp->dpPalReserved  = 20;   //# reserved palette entries
    pdp->dpColorRes     = 18;   //# palette res

    pdp->dpRaster |= RC_PALETTE;// mark as a palette device
    Palettized = 1;             // Indicate palette managed mode for DIBEng

    }
    else
    {
    pdp->dpNumPens   = -1; //# of pens this driver realizes
    pdp->dpNumColors = -1; //# colors in color table
    }

#ifdef GBLDATA_IN_PDEV
    // for future expansion of pdev
    // bump size for everything in PDEV after the DIBENGINE struct
    pdp->dpDEVICEsize += sizeof(PDEV) - sizeof(DIBENGINE);
#endif

    return size;
}


/*----------------------------------------------------------------------
Function name:  DibEnable

Description:    Call dib engine to enable, fill in BITMAPINFO. create
                and initialize dib pdevice.
                
Information:    

Return:         INT     FALSE if DIB_Enable fails,
                        TRUE  otherwise.
----------------------------------------------------------------------*/
int
DibEnable(LPVOID lpDevice,
     UINT style,
     int ModeNumber)
{
    DIBENGINE FAR *pde;

    //
    // call the DIBENG and let it enable
    //
    if (!DIB_Enable(lpDevice, style, NULL, NULL, NULL))
    {
    DPF(DBGLVL_NORMAL,"DIBENG failed to enable");
    return FALSE;
    }

    //
    // now fill in a BITMAPINFO that describes our mode
    // and call the DIBENG function CreateDIBPDevice to
    // fill in our PDevice
    //
    _FF(bi).biSize        = sizeof(BITMAPINFOHEADER);
    _FF(bi).biPlanes      = 1;
    _FF(bi).biWidth       = ModeList[ModeNumber].dwWidth;
    _FF(bi).biHeight      = ModeList[ModeNumber].dwHeight;
    _FF(bi).biBitCount    = (BYTE)ModeList[ModeNumber].dwBPP;
    _FF(bi).biCompression =  0;

    if (8 == ModeList[ModeNumber].dwBPP)
         {
         // If 8-bpp, the color table must be left alone.
         // This fixes defect Track 1766 (PRS 4843).
		 // Hmm...  I think this is incorrect.  Actually,
		 // I believe these should get set back to all zeros.
		 // This fixes the GUIMan flashing (PRS 4843).
		 // I think this was also responsible for PRS 1922.
         _FF(color_table[0]) = 0x00000000L;
         _FF(color_table[1]) = 0x00000000L;
         _FF(color_table[2]) = 0x00000000L;
         }
      else if (16 == ModeList[ModeNumber].dwBPP)
         {
         _FF(color_table[0]) = 0x0000F800L;
         _FF(color_table[1]) = 0x000007E0L;
         _FF(color_table[2]) = 0x0000001FL;
         }
      else if (24 == ModeList[ModeNumber].dwBPP)
         {
         _FF(color_table[0]) = 0x00FF0000L;
         _FF(color_table[1]) = 0x0000FF00L;
         _FF(color_table[2]) = 0x000000FFL;
         }
      else
         {
         _FF(color_table[0]) = 0x00FF0000L;
         _FF(color_table[1]) = 0x0000FF00L;
         _FF(color_table[2]) = 0x000000FFL;
         }

    CreateDIBPDevice(&_FF(bi), lpDevice, NULL,
           MINIDRIVER|VRAM);
/*
  {
  __asm int 3
  DPF(DBGLVL_NORMAL,"CreateDIBPDevice failed");
  return FALSE;
  }
  */
    //
    //  set a few things in the DIBENGINE structure that CreateDIBPDevice
    //  did not do.
    //
    pde = (DIBENGINE FAR *)lpDevice;

    pde->deBitsSelector = GetFlatSel();
    pde->deBitsOffset = _FF(ScreenAddress);
    pde->deDeltaScan = _FF(ddPrimarySurfaceData.dwMStride);

    (DWORD)pde->deBeginAccess = (DWORD)BeginAccess;
    (DWORD)pde->deEndAccess = (DWORD)EndAccess;

    if (ModeList[ModeNumber].dwBPP == 8)
    pde->deFlags |= PALETTIZED;
    else
    pde->deFlags &= ~PALETTIZED;

    if (ModeList[ModeNumber].dwBPP == 16)
    pde->deFlags |= FIVE6FIVE;
    else
    pde->deFlags &= ~FIVE6FIVE;

    if (_FF(ScreenAddress) == 0)
    pde->deFlags |= BANKEDVRAM;
    else
    pde->deFlags &= ~BANKEDVRAM;

    if ((0x10000 % pde->deDeltaScan) != 0 && (pde->deFlags & BANKEDVRAM))
    pde->deFlags |= BANKEDSCAN;
    else
    pde->deFlags &= ~BANKEDSCAN;

    return TRUE;
}

#ifndef NOLOWRESFIX
FxU32 lowreshack = 0;
FxU32 lowresheight;
#endif // #ifndef NOLOWRESFIX


/*----------------------------------------------------------------------
Function name:  GetRegStrToInt

Description:    Reads a string key from the registry and returns
                the value converted to an integer.  Ignores any
                characters that aren't decimal numbers ('0' - '9').
Information:    

Return:         FxBOOL  FXTRUE  if successfully read from registry,
                        FXFALSE otherwise.
----------------------------------------------------------------------*/
char str[256];

FxBool
GetRegStrToInt(DWORD dwDevNodeHandle,
               char *lpStr,
               DWORD *lpValue)
{
    DWORD length = sizeof(str);
    DWORD i;

    *lpValue = 0;

    if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
                               "DEFAULT",
                               lpStr,
                               REG_SZ,
                               (LPBYTE)&str[0],
                               &length,
                               CM_REGISTRY_SOFTWARE) != CR_SUCCESS)
    {
        return FXFALSE;
    }

    for (i = 0; i < length; i++)
    {
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            *lpValue *= 10;
            *lpValue += str[i] - '0';
        }
    }
        
    return FXTRUE;
}


/*----------------------------------------------------------------------
Function name:  GetBinaryDword

Description:    Reads a binary or DWORD value from the registry.

Information:    

Return:         FxBOOL  FXTRUE  if successfully read from registry,
                        FXFALSE otherwise.
----------------------------------------------------------------------*/
FxBool
GetBinaryDword(DWORD dwDevNodeHandle,
               char *lpStr,
               DWORD *lpValue)
{
    FxU32 bufSize = sizeof(DWORD);

    // Try first for Binary Value
    if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
                               "DEFAULT",
                               lpStr,
                               REG_BINARY,
                               (LPBYTE)lpValue,
                               &bufSize,
                               CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
    {
        return FXTRUE;
    }

    // Try next for Dword Value
    if (CM_Read_Registry_Value(DisplayInfo.diDevNodeHandle,
                               "DEFAULT",
                               lpStr,
                               REG_DWORD,
                               (LPBYTE)lpValue,
                               &bufSize,
                               CM_REGISTRY_SOFTWARE) == CR_SUCCESS)
    {
        return FXTRUE;
    }

    return FXFALSE;
}
#include "../minivdd/plltable.h"
#include "../minivdd/h4pll.h"
#include "../minivdd/h4oempll.h"

#ifdef AGP_CMDFIFO
FxU32 doAgp = 1;
#endif // #ifdef AGP_CMDFIFO

#define SST_VENDOR_DEVICE_ID_H3        (0x0003121aL)
#define SST_VENDOR_DEVICE_ID_H4_OEM    (0x0004121aL)


/*----------------------------------------------------------------------
Function name:  tweakFromRegistry

Description:    Modifies certain HW registers based on overridding
                values found (if any) in the registry.
Information:
  Currently may "tweak" any of the following:
  sgramMode, dramInit0, dramInit1, memSize, memClock, grxClock;

Return:         VOID
----------------------------------------------------------------------*/
void
tweakFromRegistry()
{
  FxU32 dramInit0, dramInit1, memSize, memClock, grxClock;
  FxU32 cbValue = sizeof(DWORD);
  FxU32 physicalMemSizeInMB;
  FxU32 old_dramInit0;
  FxU32 local_pllCtrl0;

  // SETDW and GET need the flat selector
  //
  GetFlatSel();

  if (GetBinaryDword(_FF(DevNode), "dramInit0", &dramInit0))
  {
    old_dramInit0 = GET(lph3IORegs->dramInit0);

    DPF(DBGLVL_ALL,"tweaking dramInit0: old=0x%08lx, ", old_dramInit0);

    DPF(DBGLVL_ALL, "new=0x%08lx\n", dramInit0);

    SETDW(lph3IORegs->dramInit0, dramInit0);
  }

  if (GetBinaryDword(_FF(DevNode), "dramInit1", &dramInit1))
  {

    FxU32 old_dramInit1 = GET(lph3IORegs->dramInit1);

    DPF(DBGLVL_ALL,"tweaking dramInit1: old=0x%08lx, ", old_dramInit1);

    // preserve strap bits from the current value of dramInit0 into
    // the new dramInit0
    //
    dramInit1 &= ~(SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPS);
    old_dramInit1 &= SST_SGRAM_TYPE | SST_SGRAM_NUM_CHIPS;

    dramInit1 = dramInit1 | old_dramInit1;

    DPF(DBGLVL_ALL, "new=0x%08lx\n", dramInit1);

    SETDW(lph3IORegs->dramInit0, dramInit1);


  }

  if (GetBinaryDword(_FF(DevNode), "memSize", &memSize))
  {
    physicalMemSizeInMB = _FF(TotalVRAM) / (1024L * 1024L);
    if (memSize <= physicalMemSizeInMB)
    {
      DPF(DBGLVL_ALL,"tweaking memSize: old=%ldMB, new=%ldMB\n",
            physicalMemSizeInMB, memSize);

            _FF(TotalVRAM) = memSize * 1024L * 1024L;
    }
    else
    {
              DPF(DBGLVL_ALL,
            "requested memSize (%dMB) greater than physical mem (%dMB): "
            " no change\n", memSize, physicalMemSizeInMB);
    }
  }

    /* Save off the default graphics and memory clock rate */
    _FF(dwDefaultClock) = GET(lph3IORegs->pllCtrl0);
  
    if (GetRegStrToInt(_FF(DevNode), "memClock", &memClock))
    {
        if ((memClock >= MIN_PLL_FREQ) && (memClock <= MAX_PLL_FREQ))
        {
            local_pllCtrl0 = GET(lph3IORegs->pllCtrl0);
            DPF(DBGLVL_ALL,
                    "tweaking memClock: old (PLL)=0x%08lx, new (MHz)=0x%08lx\n",
                    (local_pllCtrl0 >> 16), memClock);

            local_pllCtrl0 = (local_pllCtrl0 & 0xffff) | (pllTable[memClock] <<16);

            SETDW(lph3IORegs->pllCtrl0, local_pllCtrl0);
        }
        else
        {
            DPF(DBGLVL_ALL,"memClock Mhz value out of range: %d, ignoring\n",
                    memClock);
            DPF(DBGLVL_ALL,"(valid values are from %d to %d, inclusive)\n",
                    MIN_PLL_FREQ, MAX_PLL_FREQ);
        }
    }

    if (GetRegStrToInt(_FF(DevNode), "grxClock", &grxClock))
    {
        if ((grxClock >= MIN_PLL_FREQ) && (grxClock <= MAX_PLL_FREQ))
        {
            local_pllCtrl0 = GET(lph3IORegs->pllCtrl0);
            DPF(DBGLVL_ALL,
                    "tweaking grxClock: old (PLL)=0x%08lx, new (MHz)=0x%08lx\n",
                    (local_pllCtrl0 & 0xffff), grxClock);

            local_pllCtrl0 = (local_pllCtrl0 & 0xffff0000) | pllTable[grxClock];

            SETDW(lph3IORegs->pllCtrl0, local_pllCtrl0);
        }
        else
        {
            DPF(DBGLVL_ALL,"grxClock Mhz value out of range: %d, ignoring\n",
                    grxClock);
            DPF(DBGLVL_ALL,"(valid values are from %d to %d, inclusive)\n",
                    MIN_PLL_FREQ, MAX_PLL_FREQ);
        }
    }
#ifdef AGP_CMDFIFO
  if ((DWORD)IS_AGP_READY == (_FF(AGPCaps) & IS_AGP_READY))
      {
      if (GetBinaryDword(_FF(DevNode), "aFifo", &_FF(enableAGPCF)) == FXFALSE)
         _FF(enableAGPCF) = 1;
      }
  else
     _FF(enableAGPCF) = 0;
     
  // if the agp command fifo is turned off, don't try to allocate
  // AGP memory from the system at all (agp support may be busted or not
  // properly installed on this system)
  // 
  if (_FF(enableAGPCF) == 0)
    doAgp = 0;
#endif

#ifdef AGP_EXECUTE
  if ((DWORD)IS_AGP_READY == (_FF(AGPCaps) & IS_AGP_READY))
  {
      if (GetBinaryDword(_FF(DevNode), "agpExecute", &_FF(enableAGPEM)) == FXFALSE)
         _FF(enableAGPEM) = 1;

      if (GetBinaryDword(_FF(DevNode), "agpHeapSize", &_FF(agpHeapSize)) == FXFALSE)
         _FF(agpHeapSize) = 16;  //16mb AGP heap
  }
  else
  {
     _FF(enableAGPEM) = 0;
     _FF(agpHeapSize) = 0;
  }
#endif // AGP_EXECUTE

}


/*----------------------------------------------------------------------
Function name:  setupVgaMode12h

Description:    Sets up for VGA mode 12h.  Decides whether to support
                mode 12h in a window.  (If we don't do it, we get
                more memory for 3d apps.)
Information:

Return:         INT     value read out of registry to determine if
                        we want to support mode 12h in a window.
----------------------------------------------------------------------*/
void GetBinary(DWORD dwDevNodeHandle, WORD FAR * lpValue,
               char FAR * lpStr, WORD nDefault);

int
setupVgaMode12h(DWORD dwDeviceHandle)
{
    WORD doVgaMode12h;
    long mode12Argument;

    // decide whether or not to do VGA mode 12h in a window
    // (if we don't do it, we get more memory for 3d apps...)
    // if the registry key isn't present, use the default value
    //
    GetBinary(DisplayInfo.diDevNodeHandle, &doVgaMode12h,
              "vgamode12", (WORD) _FF(doVgaMode12h));

    if (doVgaMode12h)
        mode12Argument = 0;             // allow mode 12h in a window
    else
        mode12Argument = -1;            // don't allow mode 12h in a window
        
    VDDCall(VDD_DRIVER_REGISTER,
            dwDeviceHandle,
            0x200000L,    /* XXX FIXME XXX nbytes used by the screen */
            mode12Argument,             // do mode12 in a window?
            (LPVOID)ResetHiresMode);

    return doVgaMode12h;
}


/*----------------------------------------------------------------------
Function name:  setupPalette

Description:    Program the HW palette.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void setupPalette(int start, int count, FxU32 FAR * lpcolors)
{
  int i;
  FxU32 palette[256];
  WORD red, blue, green;

  //this fixes 5866 Problem with Turok
  if (_FF(gdiFlags) & SDATA_GDIFLAGS_HWC_EXCLUSIVE)
    return;

  for (i=0; i<256; i++)
  {
    if (8 != ModeList[_FF(ModeNumber)].dwBPP)
      red = blue = green = i;
    else
    {
      // update our palette shadow buffer with any new entries
      if (lpcolors && (i < count))
        _FF(ColorTable8BPP)[start+i] = lpcolors[i];

      //gamma-tize the 8bpp indexed palette
      red   = (_FF(ColorTable8BPP)[i] & 0x000000FF) >>  0;
      blue  = (_FF(ColorTable8BPP)[i] & 0x00FF0000) >> 16;
      green = (_FF(ColorTable8BPP)[i] & 0x0000FF00) >>  8;
      
    }

    // must convert from BBGGRR --> RRGGBB
    palette[i]  = (_FF(GammaTable)[red]   & 0x00FF0000);
    palette[i] |= (_FF(GammaTable)[blue]  & 0x000000FF);
    palette[i] |= (_FF(GammaTable)[green] & 0x0000FF00);
  }

  //when in 8bpp mode, the desktop gamma table holds the indexed
  //palette, which we have gamma-tized in software, above.
  //in all other bit depths, the desktop gamma table is
  //purely a gamma function, nothing else.
  set_desktopgamma(0, 256, palette);
}



/*----------------------------------------------------------------------
Function name:  DisplayMemoryLayout

Description:    Dump memory layout to the debug terminal.

Information:    Only used for debugging.

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef DEBUG
#undef DPF
#define DPF Msg
void __cdecl _loadds Msg(DWORD DbgLvl, LPSTR szFormat, ...);
int DisplayMemoryLayout(void)
{
   DPF(DBGLVL_ALL,"VGA Start %lx Size %lx", _FF(vgaStart), _FF(vgaSize));
   DPF(DBGLVL_ALL,"FIFO Start %lx Size %lx", _FF(fifoStart), _FF(fifoSize));
   //DPF(DBGLVL_ALL,"Font Start %lx Size %lx", _FF(FontCacheAddr), _FF(FontCacheSize));
   DPF(DBGLVL_ALL,"Cursor Start %lx Size %lx", _FF(cursorStart), _FF(cursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Start %lx Size %lx", _FF(SWcursorAndStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Start %lx Size %lx", _FF(SWcursorXorStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Start %lx Size %lx", _FF(SWcursorSrcStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"SW Cursor Exclusion %lx Size %lx", _FF(SWcursorExclusionStart), _FF(SWcursorSize));
   DPF(DBGLVL_ALL,"stretchBlt Start %lx Size %lx", _FF(stretchBltStart), _FF(stretchBltSize));
   DPF(DBGLVL_ALL,"");
   DPF(DBGLVL_ALL,"gdiDesktopStart        %08lX End %08lX (Size %9lu)", _FF(gdiDesktopStart),        _FF(gdiDesktopStart)+_FF(gdiDesktopSize)-1,                _FF(gdiDesktopSize));
   DPF(DBGLVL_ALL,"mmReclamationHeapStart %08lX End %08lX (Size %9lu)", _FF(mmReclamationHeapStart), _FF(mmReclamationHeapStart)+_FF(mmReclamationHeapSize)-1,  _FF(mmReclamationHeapSize));
   DPF(DBGLVL_ALL,"mmPhantomHeapStart     %08lX End %08lX (Size %9lu)", _FF(mmPhantomHeapStart),     _FF(mmPhantomHeapStart)+_FF(mmPhantomHeapSize)-1,          _FF(mmPhantomHeapSize));
   DPF(DBGLVL_ALL,"mmTransientHeapStart   %08lX End %08lX (Size %9lu)", _FF(mmTransientHeapStart),   _FF(mmTransientHeapStart)+_FF(mmTransientHeapSize)-1,      _FF(mmTransientHeapSize));
   DPF(DBGLVL_ALL,"mmPersistentHeapStart  %08lX End %08lX (Size %9lu)", _FF(mmPersistentHeapStart),  _FF(mmPersistentHeapStart)+_FF(mmPersistentHeapSize)-1,    _FF(mmPersistentHeapSize));
   return 0;
}
#endif

//mls 8/3/00 I added this to help read registry keys
/*----------------------------------------------------------------------
Function name: ddatoi

Description:   

Return:        int
----------------------------------------------------------------------*/
int ddatoi( const char *s )
{
   int i, n, sign;

   for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
     ;
     
   sign = 1;
   if (s[i] == '+' || s[i] == '-')
     sign = (s[i++] == '+') ? 1 : -1;
     
   for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
     n = 10 * n + s[i] - '0';
     
   return(sign * n);
}// ddatoi


/*----------------------------------------------------------------------
Function name:  setupOffscreenMemory

Description:    Allocates offscreen memory as shown:

  --------------------------  
          VGA memory
  --------------------------  
          Command FIFO
  --------------------------  
          Hardware cursor
  --------------------------  
          Software cursor
  --------------------------  
          Stretch BLT
  --------------------------  
          Desktop/Primary
  ---------------------------
          Various heaps
  --------------------------  

----------------------------------------------------------------------*/

void
setupOffscreenMemory(int modeNumber, int doVgaMode12h)
{

  DWORD dwResolutionX, dwResolutionY, dwBPP;
  DWORD screenSize, tileInX, tileInY;
  LPSTR   lpStr = NULL;
  DWORD   dwValue = 0;

    // Get resolution from ModeList table.
  dwResolutionX = (DWORD)ModeList[modeNumber].dwWidth;
  dwResolutionY = (DWORD)ModeList[modeNumber].dwHeight;
  dwBPP = (DWORD) ModeList[modeNumber].dwBPP;

  // Allow the outward appearance of 2046 while internally we are 2048.

//??? is this line voodoo3 specific??? DanO
  if (dwResolutionX == 2046)
         dwResolutionX = 2048;

  // Compute tiled mode 1 width, height, and screen size.
  tileInY = ( (DWORD)dwResolutionY + (SST2_TILE1_HEIGHT - 1) ) >>
            SST2_TILE1_HEIGHT_BITS;

  // ME 7/8 - use pitch, rounded up to next pow2
  tileInX = 1;
  while ( tileInX < (DWORD)ModeList[modeNumber].lPitch) tileInX<<=1;
  tileInX += (SST2_TILE_WIDTH - 1);
  //tileInX >>= SST2_TILE_WIDTH_BITS;

  // For staggered mode we need multiple of page-width tiles in X
  // and multiple of 16KB in size.
  tileInX = RNDUP_T1_PGWIDTH(tileInX >> SST2_TILE_WIDTH_BITS);

  screenSize = (tileInX << SST2_TILE_WIDTH_BITS) *
               (tileInY << SST2_TILE1_HEIGHT_BITS);

  screenSize = RNDUP_16KB(screenSize);                 


  // Allocate VGA memory  

  // Even though we're only using 32K of mode 12h virtualization
  // memory, Banshee's VGA actually has a footprint of about 96K
  // If the value returned from GetVDDBank is 64K instead of 32K,
  // this size must be adjusted too!

  _FF(vgaStart) = 0;

  // Even though we're only using 32K of mode 12h virtualization
  // memory, Banshee's VGA actually has a footprint of about 96K
  // If the value returned from GetVDDBank is 64K instead of 32K,
  // this size must be adjusted too!

  if (doVgaMode12h)
      _FF(vgaSize) = 96L * 1024L;
  else
      _FF(vgaSize) = 0;

  // Allocate command fifo and compute size (4K aligned)

  _FF(fifoStart) = ALIGN_4KB( (_FF(vgaStart)+_FF(vgaSize)) );
  switch(_FF(TotalVRAM) >> 20)
  {
    case 64: _FF(fifoSize) = 0x100000L; break;	// 1Mb
    case 32: _FF(fifoSize) = 0x100000L; break;	// 1Mb
    case 16: _FF(fifoSize) = 0x100000L; break;	// 1Mb
    case  8: _FF(fifoSize) = 0x040000L; break;	// 256Kb
    case  4: _FF(fifoSize) = 0x010000L; break;	// 64Kb
    default: _FF(fifoSize) = 0x010000L; break;	// 64Kb
  }

  // Allocate hardware cursor (1K aligned)

  _FF(cursorStart) = ALIGN_1KB( (_FF(fifoStart) + _FF(fifoSize)) );

  // SST2 cursor can be 64x64x32bpp pixels

  _FF(cursorSize)  = 64*64*4;

  // Allocate software cursor (1K aligned)

  _FF(SWcursorAndStart) = ALIGN_1KB( (_FF(cursorStart) + _FF(cursorSize)) );
  _FF(SWcursorSize)  = SWCURSOR_SIZE;
  _FF(SWcursorXorStart) = (_FF(SWcursorAndStart) + _FF(SWcursorSize));
  _FF(SWcursorSrcStart) = (_FF(SWcursorXorStart) + _FF(SWcursorSize));
  _FF(SWcursorExclusionStart) = (_FF(SWcursorSrcStart) + _FF(SWcursorSize));

  // Allocate 16KB StretchBLT scratch (16K aligned)

  _FF(stretchBltStart) = ALIGN_16KB( (_FF(SWcursorExclusionStart) + _FF(SWcursorSize)) );
  _FF(stretchBltSize)  = 0x4000L;

  // Allocate desktop, align on ODD 16KB boundary (guaranteed EVEN 8KB alignment)
  // Note: we can't use the slop space on the desktop for anything because they could
  // used during staggered mode bytes swizzeling
  _FF(gdiDesktopStart) = ALIGN_16KB( (_FF(stretchBltStart) + _FF(stretchBltSize)) );

  if ( !(_FF(gdiDesktopStart) & 0x4000L) )
    _FF(gdiDesktopStart) += 0x4000L;

  _FF(gdiDesktopSize) = screenSize;

  // Allocate linear heap, with remaining memory.

  // If we report to ddraw a linear heap that matches our phantom space
  // directdraw test will report it as total memory which is not correct.
  // Though available vidmem is reported correctly it is possible WHQL
  // may use heap size as a basis for memory allocation verification.
  // Need to confirm this true/false. If we don't report the entire
  // phantom space as a heap then a possible concern is that we may 
  // return phantom addresses that are outside the reported range of
  // the heap template we returned. Does anyone validate returned
  // addresses against the heap the surface alledgely came from? So for
  // now return a heap that reflects the amount of phyiscal vidmem and
  // possibly return phantom addresses outside the heap template range.
  // Note that when we mapped the VRAM we asked for 2x the physical size
  // so we do have a linear space sufficent for worst case phantom memory
  // allocations.

    lpStr = ddgetenv(FORCE_FS_AA_NAME); 
    if (NULL != lpStr)
    {
      dwValue = ddatoi(lpStr);
      if (dwValue < 0)
          dwValue = 0;
      if (dwValue > 3 )              // 3 is currently highest mode
          dwValue = 0;               // mode not supported 
    }
  

    _FF(mmTransientHeapStart) = _FF(gdiDesktopSize);
    if (dwValue && (dwValue != 3))
      _FF(mmTransientHeapStart) *= 4;

    _FF(mmTransientHeapStart) = ALIGN_4KB( _FF(gdiDesktopStart)+_FF(mmTransientHeapStart) );

    _FF(mmTransientHeapSize) =  _FF(TotalVRAM) - _FF(mmTransientHeapStart);
    _FF(mmTransientHeapSize) -= _FF(mmPersistentHeapSize);

    _FF(mmReclamationHeapStart) = ALIGN_4KB((_FF(gdiDesktopStart) + screenSize));
    _FF(mmReclamationHeapSize) = _FF(mmTransientHeapStart) - _FF(mmReclamationHeapStart);


    #ifdef SLI
        #define SLI_NUMUNITS _FF(dwNumUnits)
    #else
        #define SLI_NUMUNITS 1
    #endif

    //the maximum size of the phantom heap is 256 megabytes, less the phantom space needed for
    //the desktop, and everything prior to it.
    _FF(mmPhantomHeapStart) = (_FF(gdiDesktopStart)+_FF(gdiDesktopSize)) * SLI_NUMUNITS * 2;
    _FF(mmPhantomHeapSize) = min((256<<20)-_FF(mmPhantomHeapStart), _FF(TotalVRAM) * SLI_NUMUNITS * 2);

    #undef SLI_NUMUNITS


    DPF(DBGLVL_NORMAL,"%dx%dx%d", (int)dwResolutionX, (int)dwResolutionY, (int)dwBPP);

#ifdef DEBUG
    DisplayMemoryLayout();
#endif

}

/*----------------------------------------------------------------------
Function name:  reserve_commit

Description:    Initializes global variables and structures for
                AGP memory.
Information:

Return:         DWORD
----------------------------------------------------------------------*/
static DWORD
reserve_commit(AgpSrvc *srvc, DWORD nPages)
{
  srvc->service                = RESERVE;
  srvc->params.reserve.devNode = DisplayInfo.diDevNodeHandle;
  srvc->params.reserve.nPages  = nPages;

  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_AGP_SERVICE, 0, srvc);

  if (srvc->params.reserve.gartLinAddr == 0)
  {
    DPF(DBGLVL_NORMAL, "reserve failed\n");
    return FALSE;
  }

  srvc->service                   = COMMIT;
  srvc->params.commit.gartLinAddr = srvc->params.reserve.gartLinAddr;
  srvc->params.commit.pageOffset  = 0;
  srvc->params.commit.nPages      = nPages;
  
  VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_AGP_SERVICE, 0, srvc);
  
  if (srvc->params.commit.retval == 0)
  {
    DPF(DBGLVL_NORMAL, "commit failed\n");
    return FALSE;	
  }

  DPF(DBGLVL_NORMAL, "committed: NP:%d LA:0x%08lx, GA:0x%08lx",
                     srvc->params.commit.nPages,
                     srvc->params.commit.gartLinAddr,
                     srvc->params.commit.gartPhysAddr);

  return TRUE;
}


/*----------------------------------------------------------------------
Function name:  setupAgpMemory

Description:    Initializes global variables and structures for
                AGP memory.
Information:

Return:         VOID
----------------------------------------------------------------------*/

// TEMP!!! V3/Napalm have 4M - use much smaller for QT/CsimSrvr to prevent huge latency
//      at 4M, tunnel takes about 5 minutes to draw the whole buffer, so can't quit quickly
//#define nAgpPages     1024L	  //   1K    pages == 4MB
#define nAgpPages       32L	  //   pages are 4K, so 32 pages == 128KB
#define nTexPages       32L	  //   256*256*2 / 4096
#define nSyncPages       1L   //   really only need 1 word.

void Force1XRate(void);

void csimUpdateAgpConfig( DWORD physBase, DWORD linBase, DWORD sizeInBytes );

void setupAgpMemory()
{
  AgpSrvc agpSrvc;
  DWORD dwForce;
  
  DPF(DBGLVL_NORMAL, "setupAgpMemory:");
  if (FALSE == reserve_commit( &agpSrvc, nAgpPages ))
  {
    _FF(agpMain.linAddr ) = 0;
    _FF(agpMain.physAddr) = 0;
    _FF(agpMain.sizeInB ) = 0;

    _FF(enableAGPCF) = 0;
  }
  else
  {
    _FF(agpMain.linAddr ) = agpSrvc.params.commit.gartLinAddr;
    _FF(agpMain.physAddr) = agpSrvc.params.commit.gartPhysAddr;
    _FF(agpMain.sizeInB ) = (nAgpPages * 4 * 1024L);

    //RYAN: tell WinSIM about our AGP command FIFO.
    //      this range will be updated by DdUpdateNonLocalHeap().
    if (!_FF(isHardware))
    {
      csimUpdateAgpConfig( _FF(agpMain.physAddr),
                           _FF(agpMain.linAddr ),
                           _FF(agpMain.sizeInB ) );
    }
  }

  ////////////////    Allocate some storage for texture downloads /////////////

#if defined(AGPTEXTUREDOWNLOAD) && (defined(H4) || defined(H5))
  {
    int     i;
    AgpSrvc agpSrvcTex;

    for (i=0; i<NUM_AGP_TEX_BUFS; i++)
    {
      DPF(DBGLVL_NORMAL, "setupAgpTexMemory:");

      if (FALSE == reserve_commit( &agpSrvcTex, nTexPages ))
      {
        _FF(agpTexLinAddr [i]) = 0;
        _FF(agpTexPhysAddr[i]) = 0;

        return;
      }

      _FF(agpTexLinAddr [i]) = agpSrvcTex.params.commit.gartLinAddr;
      _FF(agpTexPhysAddr[i]) = agpSrvcTex.params.commit.gartPhysAddr;
    }

    for (i=0; i<NUM_AGP_TEX_BUFS; i++)
    {
      DPF(DBGLVL_NORMAL, "setupAgpTexSyncMemory:");

      if (FALSE == reserve_commit( &agpSrvcTex, nSyncPages ))
      {
        _FF(agpSyncVirtAddr[i]) = 0;
        _FF(agpSyncPhysAddr[i]) = 0;

        return;
      }

      _FF(agpSyncVirtAddr[i]) = agpSrvcTex.params.commit.gartLinAddr;
      _FF(agpSyncPhysAddr[i]) = agpSrvcTex.params.commit.gartPhysAddr;
    }
  }
#endif
 
  if (FXTRUE == GetBinaryDword(_FF(DevNode),"force1xrate", &dwForce))
    if (1 == dwForce)
    {
      DPF(DBGLVL_NORMAL,"Forceing 1X AGP Data Rate");
      Force1XRate();
    }
}


/*----------------------------------------------------------------------
Function name:  FXWAITFORIDLE

Description:    Spin until the HW is available.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void FXWAITFORIDLE()
{
  FxU32 timeout=0xffff; // 16bit max
#ifdef SLI
   DWORD dwStatus;
   DWORD i;
   DWORD dwChipMask;
   DWORD dwsliBroadcast;

  if(IS_SAGE_ACTIVE)
  {
    FxU32 lastTag;
    CMDFIFO_PROLOG(cmdFifo);

    CMDFIFO_SETUP(cmdFifo);

    lastTag = (GET(_FF(lpIOregs)->intrCtrl) & SST_USER_INTR_TAG) >> SST_USER_INTR_TAG_SHIFT;
    lastTag = (lastTag + 1) & 0xf;

    CMDFIFO_CHECKROOM(cmdFifo, 1);
    SETMOP(cmdFifo, SST_MOP_INTR_CMD |
                    (lastTag << SST_MOP_INTR_TAG_SHIFT));
    BUMP(1);
    CMDFIFO_EPILOG(cmdFifo);   

    if (CMDFIFOUNBUMPEDWORDS)
        bumpAgp( CMDFIFOUNBUMPEDWORDS );

    while((((GET(_FF(lpIOregs)->intrCtrl) & SST_USER_INTR_TAG) >> SST_USER_INTR_TAG_SHIFT) != lastTag) && timeout--);

    if(!timeout)
    {
        _asm int 1
    }

    timeout=0xffff;
  }

   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_MASTER)
      {
      dwsliBroadcast = GET(lph3IORegs->sliBroadcast);
      dwChipMask = _FF(dwSlaveMask) | BIT(_FF(dwChipID));
      if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
         {
         bumpAgp( CMDFIFOUNBUMPEDWORDS );
         do
            {
            dwStatus = 0;
            for (i=0; i<_FF(dwNumUnits); i++)
               {
               if (dwChipMask & BIT(i))
                  {
                  // First Set the Broadcast Register for Each Device !!!!
                  SETDW(lph3IORegs->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
                  dwStatus |= GET(lph3IORegs->status); 
                  }
               }
            }
         while ((dwStatus & SST_CE_BUSY) && timeout--);

         if (!timeout) 
            { 
            _asm int 1 
            }
         }

      timeout=0xffff;
      do
         {
         dwStatus = 0;
         for (i=0; i<_FF(dwNumUnits); i++)
            {
            if (dwChipMask & BIT(i))
               {
               // First Set the Broadcast Register for Each Device !!!!
               SETDW(lph3IORegs->sliBroadcast, (i << SST_SLI_RENID_MEMBASE0_SHIFT) | (BIT(i) << SST_SLI_WEN0_MEMBASE0_SHIFT));   
               dwStatus |= GET(lph3IORegs->status); 
               }
            }
         }
      while ((dwStatus & SST2_BUSY) && timeout--);

      if (!timeout) 
         { 
         _asm int 1 
         }

      SETDW (lph3IORegs->sliBroadcast, dwsliBroadcast);
      }
   else
      {
      if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
         {

         bumpAgp( CMDFIFOUNBUMPEDWORDS );

         while ((GET(lph3IORegs->status) & SST_CE_BUSY) && timeout--)
            ;
         if (!timeout) 
            { 
            _asm int 1 
            }
         }

      timeout=0xffff;
      while ((GET(lph3IORegs->status) & SST2_BUSY) && timeout--)
         ;

      if (!timeout) 
         { 
         _asm int 1 
         }
      }
#else
  if(IS_SAGE_ACTIVE)
  {
    FxU32 lastTag;
    CMDFIFO_PROLOG(cmdFifo);

    CMDFIFO_SETUP(cmdFifo);

    lastTag = (GET(_FF(lpIOregs)->intrCtrl) & SST_USER_INTR_TAG) >> SST_USER_INTR_TAG_SHIFT;
    lastTag = (lastTag + 1) & 0xf;

    CMDFIFO_CHECKROOM(cmdFifo, 1);
    SETMOP(cmdFifo, SST_MOP_INTR_CMD |
                    (lastTag << SST_MOP_INTR_TAG_SHIFT));
    BUMP(1);
    CMDFIFO_EPILOG(cmdFifo);   

    if (CMDFIFOUNBUMPEDWORDS)
        bumpAgp( CMDFIFOUNBUMPEDWORDS );
    
    while((((GET(_FF(lpIOregs)->intrCtrl) & SST_USER_INTR_TAG) >> SST_USER_INTR_TAG_SHIFT) != lastTag) && timeout--);

    if(!timeout)
    {
        _asm int 1
    }

    timeout=0xffff;
  }

  if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
  {

    bumpAgp( CMDFIFOUNBUMPEDWORDS );

    while ((GET(lph3IORegs->status) & SST_CE_BUSY) && timeout--)
      ;
    if (!timeout) { _asm int 1 }
  }

  timeout=0xffff;
  while ((GET(lph3IORegs->status) & SST2_BUSY) && timeout--)
    ;

  if (!timeout) { _asm int 1 }
#endif
}

/*----------------------------------------------------------------------
Function name:  LoadTVPCIDLL

Description:    Load TV PCI 16bit DLL (STBTV16.DLL) into video driver 
                name space so that no other versions of the STBTV16.DLL 
                will be loaded
				
Information:    DELL specific routine to support DELL WDM driver

Return:         void
----------------------------------------------------------------------*/

void LoadTVPCIDLL (char * DLLName)
{
/*  Original STBWTV16.DLL was hardcoded specifically for nVidia TNT in 
	accordance to DELL specifictions. DELL now wants this same DLL to 
	work with Voodoo3 but does not want to update the DLL in an effort to
	not test a new PCI TV driver with all of their products. So, we load 
	the updated TV PCI DLL into the system's windows directory and load
	it during driver load so that the new DLL will be used instead of 
	the old DLL while keeping the old DLL in the Windows\System directory.
*/
    DWORD retVal;
	char FileName[MAX_PATH];

    // get string with system's Windows directory
    retVal = GetWindowsDirectory (FileName, MAX_PATH);
	if (retVal == 0) return;

    // append the DLL that we need to load
    strcat (FileName, "\\");
	strcat (FileName, DLLName);

    // load the TV PCI DLL with Voodoo3 support
    LoadLibrary (FileName);
}

/*----------------------------------------------------------------------
Function name:  Enable1

Description:    Enables the HW.
                Initializes lots of variables, hooks INT 2Fh,
                initialized memory manager, device bitmaps, cursors,
                palette, etc., and set the required mode.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    Size of the structure filled in or,
                        TRUE if successful or,
                        FALSE if failure.
----------------------------------------------------------------------*/
DWORD  apfnTable32;     // table of 32-bit entrypoints

#ifdef DEBUG
int __cdecl inp(unsigned);
#pragma intrinsic(inp)
#endif

int __cdecl outp(unsigned, unsigned);
#pragma intrinsic(outp)

#pragma optimize("", off)
UINT FAR PASCAL _loadds
Enable1(LPVOID      lpDevice,
  UINT        style,
  LPSTR       lpDeviceType,
  LPSTR       lpOutput,
  LPVOID      lpStuff)
{
    DWORD dwResolutionX;
    DWORD dwResolutionY;
    DWORD dwBPP;
    DWORD dwMemClk;
    DWORD dwFlags;
    DWORD dwData;
    int   modeNumber;
    UINT size;
    HwInfo myHwInfo;
    HwInfo *pHwInfo = &myHwInfo;
    DWORD fIsDisplay;
    int doVgaMode12h;
    BYTE bIndex;
    BYTE bAddr;
    int i;

// STB-SR 12/30/98 Adding variable for temporary storage of the customer number
    FxU32 customerNumber;

// STB-EC 4/26/99  Need OS version info
    OSVERSIONINFO   ovi;
    DWORD vsize, value;
    DWORD OverrideTV =0;
    char valueName[24];

    // STB-SJ - 3/3/99
    // if this is on a DELL system, load the fixed TV PCI DLL
	// to get Voodoo3 support - see comment for LoadTVPCIDLL() above
    if (_FF(customerNumber) == 9) LoadTVPCIDLL ("stbwtv16.dll");

    fIsDisplay = TRUE;

    if (!FirstEnable)
      FXWAITFORIDLE();

    // Initialize gamma ramp and Flat panel status
    if (FirstEnable)
    {
        init_desktopgamma();
        _FF(ddMiscFlags) = 0;

#ifndef WIN_CSIM
        // Note: dfp initialization is done as a side effect of tvout Initialization because it
        // was easier to call GetBIOSInfo in that code path and we need the values from the bios
        // filled into "biosBoardConfigInfo" before we initialize the DFP code.
        _FF(dwTvoActive) = !!tvoutGetBootStatus();
#else
        _FF(dwTvoActive) = 0x0;
#endif

        // Dynamically create the Mode Table here based upon registry entries from the INF.
        // See "INFNOTES.TXT" in the DOCS directory for information on how to edit modes.
        // DYNAMIC MODE TABLE
        if ( ModeList == NULL )		// If ModeList has not been created, alloc space for it
        {
            nNumModes = di_QueryNumRegistryModes( );
            if (nNumModes == 0)
            {
                nNumModes = 1;
                useDefaultMode = TRUE;
            }

            ModeList = ( MODEINFO FAR *)di_AllocModeTable( nNumModes );
            if ( ModeList == NULL )
                return FALSE;
            di_FillModeTable( ModeList, nNumModes );		// Parse registry & fill ModeList
            di_SortModeTable( ModeList, nNumModes );		// Sort the ModeList
        }


        // if we didn't boot to the TV, but the user shut down while
        // on the TV last time, we want to go there now, UNLESS one of
        // the following is true:
        //    --There is no TV connected (@TODO: Implement this!)
        //    --The only reason we were on the TV last time was because
        //      no monitor was present, but had there been, we would have been
        //      on the monitor
        if (!_FF(dwTvoActive))
        {
            vsize = sizeof(value);
            value = 0; // initialize to "off" in case key isn't in registry
            strcpy(valueName, "enabled");
            CM_Read_Registry_Value (DisplayInfo.diDevNodeHandle, "TV",
                    valueName, REG_DWORD, (LPBYTE)&value, 
                    &vsize, CM_REGISTRY_SOFTWARE);

            if (value)
            {
                // hmm... registry says use the TV.  If the reason for this was
                // because no monitor was present last time, then we should ignore
                // this, because there IS a monitor present THIS time.
                vsize = sizeof(value);
                value = 0; // initialize to "no" in case key isn't in registry
                strcpy(valueName, "priorbootTV");
                CM_Read_Registry_Value (DisplayInfo.diDevNodeHandle, "TV",
                        valueName, REG_DWORD, (LPBYTE)&value, 
                        &vsize, CM_REGISTRY_SOFTWARE);

                _FF(dwTvoActive) = value ? 0 : 1;
            }
            else
                _FF(dwTvoActive) = 0;

            // if the last time we booted up, there was a tv, but this time:
            //    --there is no tv
            //    --the "connector" setting is set to "auto"
            // then we should ignore the registry and boot up to the CRT.
            if (_FF(dwTvoActive))
            {
                _FF(dwTvoActive) = !!tvoutCallMinivdd (H3VDD_GET_TVNVRAM_STATUS, 0);
                if (!(_FF(dwTvoActive)))
                {
                    OverrideTV = 1; // don't commit to registry
                }
            }
        }

        // commit our decisions back to the registry
        value = OverrideTV ? 1 : _FF(dwTvoActive);  // are we presently active?
        strcpy(valueName, "enabled");
        CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, "TV",
                valueName, REG_DWORD, &value,
                sizeof(DWORD), CM_REGISTRY_SOFTWARE);

        value = OverrideTV ? 0 : !!tvoutGetBootStatus();  // did we boot to TV this time?
        strcpy(valueName, "priorbootTV");
        CM_Write_Registry_Value(DisplayInfo.diDevNodeHandle, "TV",
                valueName, REG_DWORD, &value,
                sizeof(DWORD), CM_REGISTRY_SOFTWARE);

#if 0
    if (GetRegStrToInt(_FF(DevNode), "allowPALCRT", &value))
        {
            DPF(DBGLVL_ALL, "The 'allowPALCRT' value was found in the registry.\n");
            TvoutAllowPALandCRT(1UL);
            _FF(allowPALCRT) = 1;
        }
        else
        {
            DPF(DBGLVL_ALL, "The 'allowPALCRT' value was NOT found in the registry.\n");
            TvoutAllowPALandCRT(0UL);
            _FF(allowPALCRT) = 0;
        }
#endif //zero      
    }
    
    //
    //  if we are the display driver read the mode
    //  info from the registry.
    //
    //  if we are not the display driver go into
    //  a default mode.
    //
    if (fIsDisplay)
    {
            DPF(DBGLVL_NORMAL,"Enable as the DISPLAY driver.");

            VDDCall(VDD_GET_DISPLAY_CONFIG, dwDeviceHandle, sizeof(DisplayInfo), 0,
                    &DisplayInfo);
            dwBPP         = DisplayInfo.diBpp;
            dwMemClk      = 120;         // FIX_ME this is totally bogus 120MHz
            dwFlags       = DisplayInfo.diInfoFlags;
            dwResolutionX = DisplayInfo.diXRes;
            dwResolutionY = DisplayInfo.diYRes;
    }
    else
    {
            DPF(DBGLVL_NORMAL,"Enable as external driver.");
            dwBPP         = ModeList[0].dwBPP;
            dwResolutionX = ModeList[0].dwWidth;
            dwResolutionY = ModeList[0].dwHeight;
            dwFlags       = 0;
    }

    //
    // see if the mode is valid.
    // if it is not a mode we can support we fail
    //
    
    // if this is an enable that matches a driver disable, then restore
    // the mode that was last set before the disable, not the current settings
    // from the registry (monster truck madness 1 depends on this behavior)
    // careful!  don't clear the disabled flag until we get the second
    // enable call
    //
    if (_FF(ddMiscFlags) & DDMF_DRIVER_DISABLED)
    {
            modeNumber = (int) _FF(ModeNumber);

            // we need to adjust dwResolutionX and Y so that the _InquireInfo
            // has the right values
            dwResolutionX = ModeList[modeNumber].dwWidth;
            dwResolutionY = ModeList[modeNumber].dwHeight;
    }
    else
    {

// STB-SR 12/30/98 Adding support for specific customer number read from registry
// We need to do this before we go through the available list of modes.
		  if (GetRegStrToInt(DisplayInfo.diDevNodeHandle, "custNum", &customerNumber))
		  {
			DPF(DBGLVL_ALL, "Reading Customer Number: %d\n", customerNumber);
			_FF(customerNumber) = (WORD)customerNumber;
		  }
		  else
		  {
			DPF(DBGLVL_ALL, "Customer Number Not Found! Make sure INF is installing key.\n");
			_FF(customerNumber) = 0;
		  }

            modeNumber = di_FindMode(dwResolutionX, dwResolutionY, dwBPP, dwFlags);

//???#ifdef DEBUG
//???   #if 0
   //
   // DISPLAY drivers need to fail if the mode is not in the registry
   // but while debugging it is nice not to do this.
   //
   if (modeNumber == -1)
   {
      DPF(DBGLVL_NORMAL,"cant find mode, using default.");
      modeNumber = 0;
      dwBPP         = ModeList[0].dwBPP;
      dwResolutionX = ModeList[0].dwWidth;
      dwResolutionY = ModeList[0].dwHeight;
      dwFlags       = 0;
   }
//???   #endif
//???#endif

            // If we fail to find the mode, then step down and attempt to
            // force 640x480xdwBpp.  This will avert booting into the MS
            // vga.drv driver.  Fixes PRS 3866.
            // Note that this functionality is also needed for TVOUT to work correctly.
            if ((modeNumber == -1) && (dwResolutionX > 640))
            {
                dwResolutionX = 640;
                dwResolutionY = 480;
                modeNumber = di_FindMode(dwResolutionX, dwResolutionY, dwBPP, dwFlags);
            }
    }

    //
    // InquireInfo means fill in a GDIINFO structure
    // that describes the mode and the capabilities of the device
    //
    // we call DIB_Enable() and modify the fields specific to our
    // driver.
    //
    // NOTE you should never set (ie assign to) the dpRasterCaps
    // or dpCaps1 fields.  you should set specific bits (|=val), or in
    // rare cases clear a bit (&=~val).
    //
    // return the size of the structure we filled in.
    //
    if (style == InquireInfo)
    {
            size = _InquireInfo(lpDevice, style,
                                dwResolutionX, dwResolutionY, dwBPP);
            return size;
    }

    // clear the diasbled flag
    // 
    if (_FF(ddMiscFlags ) & DDMF_DRIVER_DISABLED)
      _FF(ddMiscFlags) &= ~DDMF_DRIVER_DISABLED;

#ifdef GBLDATA_IN_PDEV
    lpDriverPDevice = lpDevice;
    lpDriverData = &(((PDEV FAR *)lpDevice)->DriverData);
    if (FirstEnable)
      memset(lpDriverData,0,sizeof(GLOBALDATA));
#else
    lpDriverData = &DriverData;
#endif
    // TVOUT and DFP multimon fix
    if (FirstEnable)
    {
      lpDriverData->ulSavedStatesOfSavedDevices = 0;
      lpDriverData->ulDevicesThatHaveTheirStatesSaved = 0;
    }


#ifndef NOLOWRESFIX
    lowreshack = 0;
#endif // #ifndef NOLOWRESFIX

    if (modeNumber == -1)
    {
            DPF(DBGLVL_NORMAL,"Enable failed, cant find mode.");
            return FALSE;
    }

    _FF(fIsDisplay) = fIsDisplay;

    //
    //  EnableDevice means actualy set the mode.
    //  return zero for fail, non zero for success.
    //

    if (style != EnableDevice)
        return 0;

    //make sure the bit that identifies the code segment is 32 bit is set.
    //Changeto32();

    DPF(DBGLVL_NORMAL,"PhysicalEnable");
    // hook int 2f
    if (_FF(fIsDisplay) && (1 == dwDeviceHandle))
    {
        hook_int2f();
    }

    if (FirstEnable)
    {
      int i;

      VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
              H3VDD_GET_HW_INFO, 0, pHwInfo);

      _FF(isHardware) = pHwInfo->isHardware;

#ifdef SLI
      _FF(dwNumUnits) = pHwInfo->dwNumUnits;
      _FF(dwNumMasters) = pHwInfo->dwNumMasters;
      _FF(dwChipID) = pHwInfo->dwChipID;
      _FF(dwNumSlaves) = pHwInfo->dwNumSlaves;
      _FF(dwSlaveMask) = pHwInfo->dwSlaveMask;
#endif

        if (pHwInfo->ioBase == 0)
            return FALSE;
        
        // Let's ramp these tests up a bit
        // The problem is that the Win '98 ConfigMgr will fail
        // to program the bridge chip correct if
        // it has to do a double rebalance.  This condition will occur
        // if you insert a ISA device that the system bios does not
        // know about.  This will causes two rebalances to have to be done
        // 1) For AGP Video ROM BAR -- This is typically not allocated space by
        // system bioes
        // 2) For the new ISA device IRQ.
        // After a reboot the Configmgr will write the IRQ to the ESCD
        // and the next boot will work right.  In the meantime we should
        // not hang the system.
        //

        bIndex = (BYTE)inp((unsigned)pHwInfo->ioBase + 0xd4);
        outp((unsigned)pHwInfo->ioBase + 0xd4, 0x1c);
        bAddr = (BYTE)inp((unsigned)pHwInfo->ioBase + 0xd5);
        outp((unsigned)pHwInfo->ioBase + 0xd4, (unsigned)bIndex);
        if (bAddr != pHwInfo->ioBase >> 8)
            return FALSE;
   
        if (pHwInfo->regBase == 0)
            return FALSE;

        GetFlatSel();
#if 0 // NETLIST 8.0 workaround
        // init all CAM entries to 0 - unused entries should be set to 0
        for (i=0; i<SST2_NUM_CAM_ENTRIES; i++)
        {
            SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[i].baseAddress,0);
            SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[i].endAddress,0);
            SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[i].physicalBase,0);
            SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[i].strideState,0);
        }
#endif
        // init cam[0] for SLI
        SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[0].baseAddress,
            0);
        SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[0].endAddress,
            pHwInfo->memSizeInMB*1024*1024);
        SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[0].physicalBase,
            0);
        SETDW(((Sst2CRegs *)(pHwInfo->regBase + SST_CMD_OFFSET))->cam[0].strideState,
            0x25014020);

#if 0 // !! SST2

// todo: write a mop to the part
//       find out why the fxwaitforidle doesn't seem to work

        // More Tests
        // Yo... Registers are you mapped and ready?
        lph3g = (SstGRegs *) (pHwInfo->regBase + SST_2D_OFFSET);
        dwData = GET(lph3g->colorBack);
        SETDW(lph3g->colorBack, 0xA55A5AA5);
        FXWAITFORIDLE();
        if (0xA55A5AA5 != GET(lph3g->colorBack))
        {
          SETDW(lph3g->colorBack, dwData);
          return FALSE; 
        }
        SETDW(lph3g->colorBack, dwData);
#endif // !! SST2

        if (pHwInfo->lfbBase == 0)
            return FALSE;

        // Yet More Tests
        // Yo... FrameBuffer are you mapped and ready?
        dwData = h3READ(NULL, (unsigned long far *)pHwInfo->lfbBase);
        h3WRITE(NULL, (unsigned long far *)pHwInfo->lfbBase, 0x5AA5A55A);
        if (0x5AA5A55A != h3READ(NULL, (unsigned long far *)pHwInfo->lfbBase))
        {
            h3WRITE(NULL, (unsigned long far *)pHwInfo->lfbBase, dwData);
            return FALSE;
        }
        h3WRITE(NULL, (unsigned long far *)pHwInfo->lfbBase, dwData);
          
        switch(pHwInfo->memSizeInMB)
        {
        case 16:
        case 32:
        case 64:            // !! SST2 - note: 64 meg sst2 device fails while allocating on 128M system -- find out why
            break;
        case 128:           // !! SST2 - test this
        case 256:           // !! SST2 - test this
            __asm int 3;
            break;
        default:
            return FALSE;
        }

        _FF(ioBase) = pHwInfo->ioBase;
        _FF(regBase) = pHwInfo->regBase;
        _FF(lfbBase) = pHwInfo->lfbBase;
        _FF(geBase) = pHwInfo->geBase;
        _FF(gePhysBase) = pHwInfo->gePhysBase;
        _FF(TotalVRAM) = (FxU32) pHwInfo->memSizeInMB * 1024L * 1024L;
	    _FF(VendorDeviceID) = pHwInfo->VendorDeviceID;
	    _FF(RevisionID) = pHwInfo->RevisionID;
	    _FF(SSID) = pHwInfo->SSID;
	    _FF(AGPCaps) = pHwInfo->AGPCaps;
	    _FF(cpuType) = pHwInfo->cpuType;

        // XXX TBD XXX
        // need to check pHwInfo to see whether this is an SDRAM board
        // or not.  For now, just assume that its SGRAM, and we can turn
        // this on/off for performance evaluation via the control call.
        //
        _FF(isSdram) = FALSE;

        // choose the default value for doing vga mode 12h in a window
        // based on the memory size.   8MB and 16MB boards will have it
        // on by default, 4MB boards will have it off by default
        //
        if (pHwInfo->memSizeInMB == 4)
            _FF(doVgaMode12h) = 0;
        else
            _FF(doVgaMode12h) = 1;

        _FF(DevNode) = DisplayInfo.diDevNodeHandle;

        if (fpRepaintScreen == NULL)
        {
            HINSTANCE h;

            if (h = GetModuleHandle("USER"))
                fpRepaintScreen = GetProcAddress(h, MAKEINTATOM(275));
            _FF(fpRepaintScreen) = (DWORD) fpRepaintScreen;
        }


        lph3VidRegs = (SstVidRegs *) (_FF(regBase) + SST_VID_OFFSET);
        lph3agp = (SstCRegs *)(_FF(regBase) + SST_CMD_OFFSET );// should rename since not agp only

        lph3IORegs = (SstIORegs *)(_FF(regBase) + SST_IO_OFFSET);
        lph3_3d = (SstRegs *)(_FF(regBase) + SST_3D_OFFSET);
        lph3cmdfifo0 = (CmdFifo *)&(lph3agp->PRIMARY_CMDFIFO);
        lpfifocache0=&fifocache0;

        // read the registry to get tweak values, if any
        //
        tweakFromRegistry();

        _FF(mmHeapsInitialized) = 0;   //tell the ddraw driver, "no, in fact you have NOT initialized the heaps."
        _FF(mmPersistentHeapSize) = 0; //this heap is inactive unless CSERVICE is defined

#ifdef CSERVICE //[
        _FF(csInitialized) = 0;        //0 if not initialized, or handle of dd32.dll otherwise
        _FF(csCamAllocator) = 0;       //thunk handle of cs_CamAllocator()
        _FF(csCamLiberator) = 0;       //thunk handle of cs_CamLiberator()
        _FF(csSurfaceAllocator) = 0;   //thunk handle of cs_SurfaceAllocator()
        _FF(csSurfaceLiberator) = 0;   //thunk handle of cs_SurfaceLiberator()
        _FF(csDirectDrawHeaps) = 0;    //we haven't obtained the DDRAW heaps yet.

        _FF(mmPersistentHeapSize) = CS_PERSISTENTHEAPSIZE;
#endif //] CSERVICE

        _FF(mmPersistentHeapStart) = _FF(TotalVRAM) - _FF(mmPersistentHeapSize);

        // Move by APS to before we set the DDMF_ENABLE_DEVICEBITMAPS bit
        _FF(ddMiscFlags) = 0;
        InitDeviceBitmapFilter();

        EnableDeviceBitmaps();    // set a flag to enable device bitmaps

        // initialized by D3D when it loads so default to unknown state
        _FF(pD3context) = 0xffffffff; 
        _FF(pD3changed) = 0xffffffff;
        _FF(pD3colbuff) = 0xffffffff;
        _FF(pD3auxbuff) = 0xffffffff;


        _FF(ddCurrentSurfaceLevel) = 0;

        // PRS 4457 - web browser hang - hardware WAX bug 
        _FF(lastY) = 0;

        _FF(lastAccessType) = ACCESS_NONE;

    } // FirstEnable

    doVgaMode12h = setupVgaMode12h(dwDeviceHandle);
    //
    // Do all linear memory manager house cleaning before we change the
    // offscreen memory configuration in setupOffscreenMemory().  Fix
    // defect 1854.
    //
    DiscardAllSSB();    // invalidate save screen bitmaps
    DoAllHost();        // Hostify
    setupOffscreenMemory(modeNumber, doVgaMode12h);

    if((DWORD) ModeList[modeNumber].dwBPP == 16)
        _FF(ddMiscFlags) |= DDMF_16BPP_PRIMARY;
    else
        _FF(ddMiscFlags) &= ~DDMF_16BPP_PRIMARY;
      
#ifdef AGP_CMDFIFO
    if (doAgp)
    {
            setupAgpMemory();
            doAgp = 0;
    }
#endif // #ifdef AGP_CMDFIFO

    //
    // set the video mode
    // enables the desktop surface to start at _FF(gdiDesktopOffset)
    // disables the overlay surface
    //

    // note: dd3DInOverlay and ddVisibleOverlaySurf need to be cleared here,
    // before HWSetMode because they may be set in HWSetMode when a pixel
    // doubled low res mode surface is simulated via a stretched overlay
    // surface
    //

    // override flags for primary allocation.
    _FF(bPrimaryAlwaysTiled) = TRUE;
    _FF(bPrimaryAlwaysLinear) = FALSE;

    _FF(dd3DInOverlay) = FALSE;
    _FF(ddVisibleOverlaySurf) = 0;

    if (!HWSetMode(modeNumber))
    {
        DPF(DBGLVL_NORMAL,"HWSetMode failed");
        return FALSE;
    }

    // read Misc Output reg and save bit 7 (vsync polarity) in ddMiscFlags
    _FF(ddMiscFlags) &= ~DDMF_VSYNC_POLARITY_MASK;
	if ((FxU16)inp((WORD)_FF(ioBase) + 0xCC) & 0x80)
    {
      _FF(ddMiscFlags) |= DDMF_VSYNC_POLARITY_MASK;
    }

    //
    // every thing worked remember the mode number and the driver
    // PDevice and return success
    //
    _FF(dwVersion) = DDMINI_VERSION;
    _FF(ModeNumber) = modeNumber;
    _FF(lpPDevice) = lpDevice;
    _FF(fHardwareCursor) = FALSE;

#if defined(H5)
    _FF(ModeUses2PixPerClkRender) = (ModeList[modeNumber].dwFlags & REND2PIX_PER_CLK) ? 
                                    SST_CM_ENTWOPIXELSPERCLOCK:0x0;
#endif
    _FF(dwRefreshRate) = ModeList[modeNumber].wVert;
    dwMemClk = (GET(lph3IORegs->pllCtrl0) & 0xffff0000) >> 16;
    _FF(memclk) = (WORD)(PLL2MHz((DWORD)dwMemClk));	
    _FF(fReset) = TRUE;
    _FF(lastOverlayAddress) = INVALID_ADDRESS;

    _FF(dd3DSurfaceCount) = 0;

    // These are not needed in Rampage.
    _FF(ddTileMark) = _FF(TotalVRAM);
    _FF(ddTiledHeapActive) = FALSE;

    _FF(ScreenAddress) = _FF(gdiDesktopStart) + _FF(LFBBASE);
    
    _FF(MsgFcn) = (DWORD) DbgOut;
    (char *)(_FF(bufferptr16)) = (char*) &(_FF(charbuffer[0]));
    (char *)(buffer) = (char *)(_FF(charbuffer));

    if (!DibEnable(lpDevice, style, modeNumber))
        return FALSE;

    //
    // now re-register with DirectDraw so it knows all about the
    // new display mode.
    //
    if (_FF(HALCallbacks).lpSetInfo)
    {
        DDCreateDriverObject(TRUE);
    }

    apfnTable32 = VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
                          H3VDD_GET_FN_TABLE32, 0, 0);

    _FF(lpPDevice)->deFlags |= BUSY;
    // arguements are only inportant for csim builds
    InitThunks(_FF(LFBBASE), 0x1000000L);
    _FF(lpPDevice)->deFlags &= ~BUSY;

    // forcibly remove HWC_exclusive.
    // XXX TODO XXX
    // To be complete, we also need to
    // set state noting that we've done this, which isn't done here
    //
    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);
    _FF(UnGlideContext) = TRUE;
    

    // enable SaveScreenBitmap support if mmInit() succeeds
    bSSB = mmInit();
    mmInit();

    InitCursor();

#ifdef STB_EDGETOOLS
	EdgeInit();
#endif

#ifndef HAL_CSIM
    setupPalette(0, 256, NULL); // reset palette or gamma ramp
#endif
    //reset overlay too
    set_overlaygamma(0, 256, NULL);

    // STB-EC 4/26/99 check for Win95 OS and set the win95OS variable in GLOBALDATA
    memset(&ovi, 0, sizeof(ovi));
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);
    if (ovi.dwMinorVersion < 10) 
       (_FF(win95OS) = 1);
    else 
       (_FF(win95OS) = 0);    

    // Initialize Device Bit Maps
    InitDeviceBitmap();

#if !defined(WIN_CSIM) && defined(R21_TVOUT_READY) // SST2 - Temp! avoid until tv/panel code ready
    tvoutSetStdInternal();

    // If the TV is supposed to be on at boot, make sure it is on.
    if (FirstEnable && _FF(dwTvoActive))
    {
     // Imhoff - Make sure size and centering adjustments are loaded from
     // saved registry settings. No action is taken if reg entries are missing.
       TVOutRefreshMem( NULL );

      // this is how you turn on the TV, oddly enough...
         TVSetStd.dwSubFunc = QUERYSETSTANDARD;
         TVSetStd.dwStandard = 0; 
         TVOutSetStandard(&TVSetStd);
#endif

    // Move Down here so Cursor Enable Bit would not be lost
    FirstEnable = 0;

    VDDCall(VDD_POST_MODE_CHANGE, dwDeviceHandle, 0, 0, 0);

#ifdef WIN_CSIM
    SETDW(((SstIORegs *)_FF(regRealBase))->vidDesktopStartAddr, (_FF(gdiDesktopStart) & SST_VIDEO_START_ADDR) << SST_VIDEO_START_ADDR_SHIFT);
    {
    SstRegs FAR * l3dRegs = (SstRegs *)(_FF(regFakeBase) + SST_3D_OFFSET);
    SETDW(l3dRegs->nopCMD, 0xF)
    SETDW(l3dRegs->chipMask, 0xFFFFFFFFL);
    SETDW(l3dRegs->fbzMode, SST_RGBWRMASK);
    SETDW(l3dRegs->aaCtrl, 0);
    SETDW(l3dRegs->combineMode, 0);
    }
#endif

    DPF(DBGLVL_NORMAL,"PhysicalEnable success");
    return TRUE;
}


/*----------------------------------------------------------------------
Function name:  Disable1

Description:    Disables the HW.
                Hostifies device bitmaps and  objects under the
                control of the memory manager, unhooks INT 2Fh,
                disables TV-out (if applicable).

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    1 is always returned.
----------------------------------------------------------------------*/
#pragma optimize("", on)
UINT FAR PASCAL _loadds Disable1(DIBENGINE FAR *pde)
{
    DPF(DBGLVL_NORMAL,"Disable");

    // Hostify Bitmaps
    DoAllHost();
    DIB_Disable(pde);           // let the DIBENG clean up.
    FreeAllNodeTablePages();    // free system memory used by mem mgr
    if (_FF(fIsDisplay) && (1 == dwDeviceHandle))
    {
        unhook_int2f();         // remove us from the int 2f chain
    }

    pde->deFlags |= BUSY;       // device is BUSY

    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle,
            H3VDD_DISABLE_GDI_DESKTOP, 0, 0);

    HWSetMode(-1);              // call code in setmode.c to "unset" the mode

    //FreeFlatSel();            // dont leak a selector
    _FF(ddMiscFlags) |= DDMF_DRIVER_DISABLED;

#ifndef WIN_CSIM
// Imhoff - calls to tvoutDisableInternal() were causing the "Restart in MS-DOS mode"
// to display no TV signal... It is also likely that the "It's now safe to turn off your computer"
// screen would not be visible during a shutdown. Since having the TV on when unneeded is likely less 
// of a crime than TV off when TV on is absolutely needed, we will leave it on here.
//        tvoutDisableInternal();
#endif
    return 1;
}


/*----------------------------------------------------------------------
Function name:  ToBackground

Description:    Disables GDI desktop.
                Hostifies device bitmaps and  objects under the
                control of the memory manager.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL ToBackground()
{
    DPF(DBGLVL_NORMAL,"ToBackground");

    // Clear Flag to Tell Glide that context has been lost
    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);
    _FF(UnGlideContext) = TRUE;

    // Invalidate/remove all GDI objects in the Memory Manager
    DoAllHost();                                   // Device Bitmaps
    DiscardAllSSB();                               // Save Screen Bitmaps
    _FF(mmFlags2D) |= MM_FONT_CACHE_INVALID;   // Fonts

    _FF(lpPDevice)->deFlags |= BUSY;
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_DISABLE_GDI_DESKTOP, 0, 0);
}


/*----------------------------------------------------------------------
Function name:  ToForeground

Description:    Enables GDI desktop.
                Resets the desktop mode, reinitializes thunks,
                resets palette, repaints screen, restores the
                cursor.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
int RestoreCursor(void);
void FAR PASCAL ToForeground()
{
    DPF(DBGLVL_NORMAL,"ToForeground");

    // Clear Flag to Tell Glide that context has been lost
    _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_HWC_EXCLUSIVE);
    _FF(UnGlideContext) = TRUE;

    if ((int)_FF(ModeNumber) < 0)
        return;

    HWSetMode((int)_FF(ModeNumber));

    _FF(ddResetOverlay) = (DWORD) TRUE;
    
    _FF(lpPDevice)->deFlags |= BUSY;
    InitThunks(_FF(LFBBASE), 0x400000L);
#ifdef NapalmCode
// is this fix needed in Rampage??? DanO
    // We use to call InitThunks here but this turn out to be a bad deal
    // what happens is occasionally, we would be in the 32 bit thunk code
    // and we would get a interrupt.  When this happen, sometimes
    // the upper half of eip would get hacked... <some sorta of Inside
    // windows joke> and we would get a gpf.
    // To fix this I just call and ask the mini-vdd to politely ask to do this 
    // for me....
    VDDCall(VDD_REGISTER_DISPLAY_DRIVER_INFO, dwDeviceHandle, H3VDD_INITFIFO, 0, 0);
#endif

    setupPalette(0, 256, NULL);  // reset palette or gamma ramp

    _FF(lpPDevice)->deFlags &= ~BUSY;

    if (fpRepaintScreen != NULL)
    {
        fpRepaintScreen();
    }

    // Sometimes when exiting a DOS Application like
    // vmtest the order of SetCursor and ToForeground is not
    // ToForeground --> SetCursor.  ToForeground has the sideeffect of
    // clearing the vidProcCfg register.  Unfortunately, we cannot be
    // smarter in SetVideoMode since sometimes the calls are such that
    // the SetCursor calls come before ToBackground which is quite strange.
    // I sure don't understand why the order of the calls vary???
    // But I do know that if I reenable the cursor here if it is enabled
    // then all is well in BansheeLand.
    RestoreCursor();

    VDDCall(VDD_POST_MODE_CHANGE, dwDeviceHandle, 0, 0, 0);
}



/*----------------------------------------------------------------------
Function name:  ValidateMode1

Description:    Called by GDI to verify a mode is valid 
                prior to setting it.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    VALMODE_YES        if successful or,
                        VALMODE_NO_UNKNOWN if unsuccessful
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds
ValidateMode1(DISPVALMODE FAR *pdvm)
{
    DPF(DBGLVL_NORMAL,"ValidateMode1");

#if 0
    //
    // if the system is just testing possible modes by calling this
    // function, return YES to everything
    //
    if (FirstEnable)
    return VALMODE_YES;
#endif
    //
    // otherwise, go ahead and look up the mode in the mode table,
    // and run HwTestMode to make sure it can fit
    //
    if (di_FindMode(pdvm->dvmXRes, pdvm->dvmYRes, pdvm->dvmBpp, 0) != -1)
        return VALMODE_YES;
    else
        return VALMODE_NO_UNKNOWN;
}


/*----------------------------------------------------------------------
Function name:  ReEnable1

Description:    Re-enables the HW.
                Needs to Hostify all device bitmaps prior to
                re-enabling.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         UINT    TRUE    if successful or,
                        FALSE   if unsuccessful
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds ReEnable1(
    DIBENGINE FAR *     lpPDevice,
    GDIINFO FAR *       lpGDIInfo)
{
    DPF(DBGLVL_NORMAL,"ReEnable");

    // Low Power Mode?
    // SST2 - power mgmt change - won't use pllctrl1 for PM (??)
    if (0 /* TEMPORARY!!! we can't be in low power mode until power mgmt support added */)

       return FALSE; 

    // Here is one spot that we need to do this
    // Before we set the mode make sure DeviceBitMaps are on the host
    // Hostify
    DoAllHost();

    if (Enable1(lpPDevice, EnableDevice, NULL, NULL, NULL))
    {
        Enable1(lpGDIInfo, InquireInfo, NULL, NULL, NULL);
        _FF(ddCurrentSurfaceLevel)++;
        _FF(ddPrimarySurfaceData).surfaceLevel = _FF(ddCurrentSurfaceLevel);
        return TRUE;
    }

    return FALSE;
}


/*----------------------------------------------------------------------
Function name:  BeginAccess

Description:    Support exclusion/restoration of the SW cursor.  Also,
                supports controlled access to the frame buffer.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds
BeginAccess(DIBENGINE FAR *pde,
       int left,
       int top,
       int right,
       int bottom,
       UINT flags)
{
   // Hey... If we are BUSY we may be in Low Power Mode
   // if so we will wait in here for the device to be ready forever
   if (_FF(lpPDevice)->deFlags & BUSY)
      return;

   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
      {  //
         // software cursor is in use, exclude it
         // EndAccess will wait for blter to idle
         //
         myBeginAccess(pde, left, top, right, bottom, flags);
      }
    //
    // idle the chip before allowing windows/the dib engine lfb access to the
    // framebuffer
    //
   FXWAITFORIDLE();

    // if we are using a DIB cursor turn it off.
    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_DIB_CURSOR) &&
        (_FF(lpPDevice) == pde) &&
        (flags & CURSOREXCLUDE))
    {
        DIB_BeginAccess(pde, left, top, right, bottom, flags);
    }
}


/*----------------------------------------------------------------------
Function name:  EndAccess

Description:    Support exclusion/restoration of the SW cursor.  Also,
                supports controlled access to the frame buffer.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds EndAccess(DIBENGINE FAR *pde, UINT flags)
{
   if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
      {  //
         // software cursor is in use, exclude it
         // EndAccess will wait for blter to idle
         //
         myEndAccess(pde, flags);
      }

    // if we are using a software cursor turn it back on.
    if ((_FF(gdiFlags) & SDATA_GDIFLAGS_DIB_CURSOR) &&
        (_FF(lpPDevice) == pde) &&
        (flags & CURSOREXCLUDE))
    {
        DIB_EndAccess(pde, flags);
#if 0
        // Hey I want to see the cursor here
        // but this really makes Winbench suck!!!!
        DIBMoveCursor(_FF(CursorPosX), _FF(CursorPosY));
#endif
    }

    //
    // XXX FIXME? XXX
    //
    // we may need an idle here even w/out CRASHTEST, check to see whether any
    // write occuring behind an LFB write could possibly cause anything to get
    // to memory ahead of the LFB write
    // -KMW
    //
#ifdef CRASHTEST
    FXWAITFORIDLE();
#endif // #ifdef CRASHTEST
}


/*----------------------------------------------------------------------
Function name:  myBeginAccess

Description:    HW specific functionality of generic BeginAccess.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds
myBeginAccess(DIBENGINE FAR *pde,
       int left,
       int top,
       int right,
       int bottom,
       UINT flags)
{
   if (_FF(lpPDevice)->deFlags & BUSY)
      return;

   if ((flags & CURSOREXCLUDE) &&
       (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) )
   {
       // check for cursor exclusion
       if (DoIntersect(left, top, right, bottom))
       {

          if ((SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
             {
             if (_FF(gdiFlags) & SDATA_GDIFLAGS_CURSOR_ENABLED)
                RestoreCursorExclude(_FF(LastCursorPosX), _FF(LastCursorPosY));
             _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_EXCLUDE;
             ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
             }
         _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED;

       }
   }

   _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_EXCLUDE;
#ifdef DEBUG_CURSOR
   GottaCursor();
#endif
}


/*----------------------------------------------------------------------
Function name:  myEndAccess

Description:    HW specific functionality of generic EndAccess.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void FAR PASCAL _loadds myEndAccess(DIBENGINE FAR *pde, UINT flags)
{
   int x;
   int y;

#if 0
   if (_FF(lpPDevice)->deFlags & BUSY)
      return;
#endif

   // Wait for FB writes to Flush <Doesnot really work but should pause us a little>
   FXWAITFORIDLE();
    // if we are using a software cursor turn it back on.
   if ( (flags & CURSOREXCLUDE) &&
       (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR) )
   {
// 1   2   4   5   6   7   8   9   a   b   c   d   e   f

           if (SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED & _FF(gdiFlags))
              {
              x = _FF(CursorPosX) - _FF(HotspotX);
              y = _FF(CursorPosY) - _FF(HotspotY);
              if ((SetCursorBusy( (WORD FAR *) & (_FF(cursorBusy)) ) ) )
                 {
                 _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE);
                 SaveCursorExclude(x,y);
                 DrawCursor(x,y);
#ifdef DEBUG_CURSOR
                 GottaCursor();
#endif
                 ClearCursorBusy((WORD FAR *) & (_FF(cursorBusy))); 
                 }
              else
                 _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE);
              }
    }
    //
    // XXX FIXME? XXX
    //
    // we may need an idle here even w/out CRASHTEST, check to see whether any
    // write occuring behind an LFB write could possibly cause anything to get
    // to memory ahead of the LFB write
    // -KMW
    //
#ifdef CRASHTEST
   FXWAITFORIDLE();
#endif // #ifdef CRASHTEST

   _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED);
}


/*----------------------------------------------------------------------
Function name:  SetPalette1

Description:    Set the HW palette.

Information:

Return:         UINT    value returned from call to DIB_SetPaletteExt.
----------------------------------------------------------------------*/
UINT FAR PASCAL _loadds SetPalette1(
    UINT        start,
    UINT        count,
    DWORD FAR  *lpPalette)
{
    UINT rc;

    DPF(DBGLVL_NORMAL,"SetPalette1");

    rc = DIB_SetPaletteExt(start, count, lpPalette, _FF(lpPDevice));

    if (!(_FF(lpPDevice)->deFlags & BUSY))
    {
        setupPalette(start, count, lpPalette);
        //HWSetPalette(start, count, lpPalette);
    }

    return rc;
}


/*----------------------------------------------------------------------
Function name:  Control1

Description:    Called by GDI when an application calls Escape
                or ExtEscape.  If the escape is not handled, it
                needs to be passed on to the dib engine.

Information:    Refer to the driver DDK documentation for generic
                description.

Return:         LONG    value returned from call to DIB_Control or
                        other numerous possibilities.
----------------------------------------------------------------------*/
#ifdef HAL_CSIM
WORD doddhal = 1;    // if 0, don't pass writes from DX to CSIM
            // (helps when trying to kill an app)
#endif /* #ifdef HAL_CSIM */

#define OPENGL_GETINFO  4353        /* for OpenGL ExtEscape */


LONG FAR PASCAL _loadds Control1(
    DIBENGINE FAR * lpDevice,
    UINT            function,
    LPVOID          lpInput,
    LPVOID          lpOutput)
{
    DCICMD FAR *pdci;
#ifdef HAL_CSIM
    DWORD addr, data;
#endif // #ifdef HAL_CSIM
    int retval;

    LPVIDEOPARAMETERS lpVidParams;
    DPF(DBGLVL_NORMAL,"Control1");

#ifdef STB_EDGETOOLS //Paul Magee 21 Jan 99
	if	(EdgeEscape((int) function, (void FAR *)lpInput, (void FAR *) lpOutput))
	{
		return TRUE;
	}
#endif /* STB_EDGETOOLS */

    switch (function) {
#ifdef CSERVICE
    case CS_EXTESCAPE: 
        return CentralServices(lpInput, lpOutput);  //RYAN@PUN, Terry Gilliam would be proud!
#endif
    //
    // QUERYESCSUPPORT is sent to ask
    // "do you support this" lpInput points
    // the function code to query
    //
    case QUERYESCSUPPORT:
        switch (*(UINT FAR *)lpInput) {
        case DCICOMMAND:
	 	    // This is the "MS magic" that allows for
		    // the DirectX certification bit to be
		    // enabled.  Normally, this returns
		    // DD_HAL_VERSION (0x100).  By changing
		    // to return 0xFF, the certification bit
		    // is "magically" set!
            // return 0xFF;			//Set the magic certification bit
		    return DD_HAL_VERSION;
         
        case VIDEO_PARAMETERS:
            return TRUE;
         
        /* OpenGL Driver Info Escape */
        case OPENGL_GETINFO:
            return TRUE;

        case QUERYESCINS:
            return TRUE;

	    // PRS 5262: For performance and other reasons,
	    // MouseTrails are not supported, we return false
	    // so that the Mouse Properties menu will be grayed out.
	    // PRS 5400: Disabling MOUSETRAILS in Win95 causes problems.
	    // Check OS first, disable only for Win98.
	    // srogers 5/14/99 - Temporary Fix for PRS 5878.  If we disable mouse trails,
	    // then Nascar Revolution does not load.  We will work with EA Sports to fix this
        // case MOUSETRAILS:
        //  if (_FF(win95OS) == 0)
        // 	    return FALSE;
        }
        break;

    case OPENGL_GETINFO: {
#       define MAX_PATH        260
        typedef struct {
            unsigned long ulVersion;
            unsigned long ulDriverVersion;
            char  awch[MAX_PATH+1];
        } OglOutInfo;
        OglOutInfo FAR *info = lpOutput;
        info->ulVersion       = 2;
        info->ulDriverVersion = 1;
        info->awch[0]         = '3';
        info->awch[1]         = 'D';
        info->awch[2]         = 'f';
        info->awch[3]         = 'x';
        info->awch[4]         = 0;
        return TRUE;
        break;
    }

    case MOUSETRAILS:
        //
        // keep track of state of mouse trails
        // we need to know if mouse trails are on
        // so we can turn off our hardware cursor.
        //
        if (lpInput)
            DoMouseTrails(*(UINT FAR *)lpInput);

        break;

    case QUERYESCMODE:
        if (TDFXACK == QueryMode((LPQIN)lpInput, lpOutput))
            return TDFXACK;
        break;

#if defined( INSTRUMENTATION )
    // case SST_PERF_ISINSTUMENTED: is aliased to:
    case QUERYESCINS:
    {
        DWORD FAR *pinst;
        pinst = (DWORD FAR *) lpOutput;
        *pinst = (DWORD)lpDriverData->insHeader;
        return TRUE;
    }

    case SST_PERF_HASPERFCOUNTER:
    {
        return TRUE;
    }

    case SST_PERF_CNTRCONTROL:
    {
        DWORD  cmd, i; 
        SSTPERFORMANCEDATA FAR *info = lpOutput;

	     if (info->m_size != sizeof(SSTPERFORMANCEDATA))
            return FALSE;

        cmd = info->m_cmd;
        switch (cmd)
        {
            case SST_PERFCMD_VALIDDATA:
                 info->m_cmd = 0; 
                 break;

            case SST_PERFCMD_SET_CONTROL:
            {
                 CMDFIFO_PROLOG(cf);
                 CMDFIFO_SETUP(cf);

                 SETDW( _FF(lpCRegs)->stats.misc, info->m_misc);
                 SETDW( _FF(lpCRegs)->stats.misc, info->m_misc);
                 SETDW( _FF(lpCRegs)->stats.counterSel, info->m_cntrSel);
                 SETDW( _FF(lpCRegs)->stats.opSel, 
                         info->m_opSel | 0x03333333); // PERF_OP_COPY 0-6
                 SETDW( _FF(lpCRegs)->stats.opSelHigh, 
                         info->m_opSelHigh | 0x0333); // PERF_OP_COPY 7-9
                 SETDW( _FF(lpMCregs)->perfmonitorCtl, 0 );

                 CMDFIFO_CHECKROOM(cf, 2);
                 SETPH( cf, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, mopCMD));
                 SETPD( cf, FBI->mopCMD, SST_MOP_STATS_CTRL |
                                         SST_MOP_RESET_STATS |
                                         SST_MOP_ENABLE_STATS);
                 BUMP(2);
                 CMDFIFO_EPILOG(cf);
                 break;
            }
            case SST_PERFCMD_GET_CONTROL:
                 info->m_misc      = GET(_FF(lpCRegs)->stats.misc);
                 info->m_cntrSel   = GET(_FF(lpCRegs)->stats.counterSel);
                 info->m_opSel     = GET( _FF(lpCRegs)->stats.opSel);
                 info->m_opSelHigh = GET( _FF(lpCRegs)->stats.opSelHigh);
                 break;

            case SST_PERFCMD_GET_SAVEDATA:
            case SST_PERFCMD_GET_SWAPHISTORY:
                 info->m_swapHistory = GET( _FF(lpCRegs)->stats.swapHistory);
                 break;

            case SST_PERFCMD_GET_CNTRDATA:
            case SST_PERFCMD_GET_ALLCNTRDATA:
                 info->m_misc = GET(_FF(lpCRegs)->stats.misc);
                 info->m_cntrSel   = GET(_FF(lpCRegs)->stats.counterSel);
                 info->m_nrOfCntrs = SST_PERF_PROGRAMMABLE_CNTRS +
                                     SST_PERF_HOSTINTERFACE_CNTRS + 
                                     SST_PERF_MEMORYCTRL_CNTRS;

                 for(i=0; i < SST_PERF_PROGRAMMABLE_CNTRS; i++)
                     info->m_cntr[i] = GET( _FF(lpCRegs)->stats.statCounter[i]);

                 info->m_save1 = GET( _FF(lpCRegs)->stats.statSave1);
                 info->m_save2 = GET( _FF(lpCRegs)->stats.statSave2);
                 info->m_swapHistory = GET( _FF(lpCRegs)->stats.swapHistory);

                 // for( i=0; i < SST_PERF_HOSTINTERFACE_CNTRS; i++)
                 i=0;
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciClkCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciSwArivCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciSwDphaseCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciFwArivCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciFwDphaseCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciRdArivCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciRdDphaseCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.pciRetry);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.agpWrArivAccum);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.agpWrCompltnCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.agpWrDphaseCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.agpRdArivAccum);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.agpRdCompltnCnt);
                 info->m_hiCntr[i++] = GET( _FF(lpCRegs)->pciStats.agpRdDphaseCnt);


                 // for( i=0; i < SST_PERF_MEMORYCTRL_CNTRS; i++)
                 i=0;
                 info->m_memCntr[i] = GET( _FF(lpMCregs)->perfCompletions);
                 info->m_memCntr[i] = GET( _FF(lpMCregs)->perfClocksBusy);
                 info->m_memCntr[i] = GET( _FF(lpMCregs)->perfClocksIdle);
                 info->m_memCntr[i] = GET( _FF(lpMCregs)->perfElapsedClocks);

                 break;
        }
        return TRUE;
    }
#endif

    case DCICOMMAND:
        pdci = (DCICMD FAR *)lpInput;

        if (pdci == NULL || pdci->dwVersion != DD_VERSION)
        {
            break;
        }

        /*
        * this request gives us our direct draw routines to call
        */
        if (pdci->dwCommand == DDNEWCALLBACKFNS)
        {
            DPF(DBGLVL_NORMAL,"DDNEWCALLBACKFNS");
            _FF(HALCallbacks) = *((LPDDHALDDRAWFNS)pdci->dwParam1);
            return TRUE;
        }
        /*
        * return information about our 32-bit DLL
        *
        * we pass a point to our shared global Data
        * to the 32-bit driver so we can talk to each
        * other by reading each others mind.
        */
        else if (pdci->dwCommand == DDGET32BITDRIVERNAME)
        {
            LPDD32BITDRIVERDATA p32dd = (LPDD32BITDRIVERDATA)lpOutput;

            DPF(DBGLVL_NORMAL,"DDGET32BITDRIVERNAME");

            lstrcpy(p32dd->szName, DDFXS32_DLLNAME);
            lstrcpy(p32dd->szEntryPoint, "DriverInit");
            p32dd->dwContext = GetSelectorBase(SELECTOROF(lpDriverData)) +
                            OFFSETOF(lpDriverData);

            return TRUE;
        }
        /*
        * handle the request to create a driver
        * NOTE we must return our HINSTANCE in *lpOutput
        */
        else if (pdci->dwCommand == DDCREATEDRIVEROBJECT)
        {
            DPF(DBGLVL_NORMAL,"DDCREATEDRIVEROBJECT");
            DDCreateDriverObject(FALSE);
            *(DWORD FAR *)lpOutput = _FF(HALInfo).hInstance;
            return TRUE;
        }
        else if (pdci->dwCommand == DDVERSIONINFO)
        {
            LPDDVERSIONDATA pddvd;

            pddvd = (LPDDVERSIONDATA) lpOutput;
            pddvd->dwHALVersion = DD_RUNTIME_VERSION; //Declare our HAL version
            DPF(DBGLVL_NORMAL, "Setting HAL Version=%08lx", pddvd->dwHALVersion );
            return TRUE;
        }
        break;

    case 0x8050:
        // Super, duper, ugly hack, for turning on/off SDRAM support
        // on the fly (e.g., turning on/off use of block writes)
        //
        switch (*(FxU32 *)lpInput) {
        FxU32 miscInit1;

        case 0:
            // block write off (== SDRAM on)
            miscInit1 = GET(lph3IORegs->miscInit1);
            miscInit1 |= 1L << 15;
            SETDW(lph3IORegs->miscInit1, miscInit1);
            _FF(isSdram) = TRUE;
            break;

        case 1:
            // block write on (== SDRAM off)
            miscInit1 = GET(lph3IORegs->miscInit1);
            miscInit1 &= ~(1L << 15);
            SETDW(lph3IORegs->miscInit1, miscInit1);
            _FF(isSdram) = FALSE;
            break;
        }
        break;

#ifdef HAL_CSIM
    // communication path from direct draw/d3d hal to/from the csim
    case EXT_DDHAL_CSIM_WRITE:
        if (doddhal)
        {
            addr = *(DWORD *)lpInput;
            data = *((DWORD *)lpInput + 1);
            h3WRITE(0, (DWORD *)addr, data);
        }
        return TRUE;

    case EXT_DDHAL_CSIM_READ:
        if (doddhal)
        {
             addr = *(DWORD *)lpInput;
             data = h3READ(0, (DWORD *)addr);
             *(DWORD *)lpOutput = data;
             return TRUE;
        }
#endif // #ifdef HAL_CSIM
    case EXT_HWC:
        return hwcExt((DWORD *) lpInput, (DWORD *) lpOutput);
        break;

    case VIDEO_PARAMETERS:
        retval = TVOutVideoParameters ((LPVIDEOPARAMETERS)lpInput);
        if (retval >= 0)
            return (retval);
        lpVidParams = (LPVIDEOPARAMETERS)lpInput;
        if ( lpVidParams->dwCommand == VP_COMMAND_GET )
        {
            lpVidParams->dwFlags = 0;
            lpVidParams->dwMode = 0;
            lpVidParams->dwTVStandard = 0;
            lpVidParams->dwAvailableModes = 0;
            lpVidParams->dwAvailableTVStandard = 0;
            lpVidParams->dwFlickerFilter = 0;
            lpVidParams->dwOverScanX = 0;
            lpVidParams->dwOverScanY = 0;
            lpVidParams->dwMaxUnscaledX = 0;
            lpVidParams->dwMaxUnscaledY = 0;
            lpVidParams->dwPositionX = 0;
            lpVidParams->dwPositionY = 0;
            lpVidParams->dwBrightness = 0;
            lpVidParams->dwContrast = 0;
            lpVidParams->dwCPType = 0;
            lpVidParams->dwCPCommand = 0;
            lpVidParams->dwCPStandard = 0;
            lpVidParams->dwCPKey = 0;
            lpVidParams->bCP_APSTriggerBits = 0;
            memset (lpVidParams->bOEMCopyProtection, 0,
            sizeof(lpVidParams->bOEMCopyProtection));
        }
        return TRUE;
    }

    return DIB_Control(lpDevice,function,lpInput,lpOutput);
}


/*----------------------------------------------------------------------
Function name:  GetRegInt

Description:    Read a integer from the HKEY_CURRENT_CONFIG\
                Display\Settings key in the registry.  Will read
                a string value and return a integer, if the string
                is of the form X,Y will return X<<16+Y
Information:

Return:         DWORD   int from registry, or default if not there
----------------------------------------------------------------------*/
DWORD GetRegInt(LPSTR valname, DWORD def)
{
    HKEY    hkey;
    char    ach[20];
    LONG    cb;
    int     i;

    if (RegOpenKey(HKEY_CURRENT_CONFIG, "Display\\Settings", &hkey) == 0)
    {
        ach[0] = 0;
        cb = sizeof(ach);

        if (RegQueryValueEx(hkey, valname, NULL, NULL, ach, &cb) == 0)
        {
            for (def=i=0; ach[i]; i++)
            {
                if (ach[i] >= '0' && ach[i] <= '9')
                    *((WORD*)&def) = LOWORD(def) * 10 + ach[i]-'0';

                if (ach[i] == ',')
                    def = def << 16;
            }
        }

        RegCloseKey(hkey);
    }

    return def;
}


/*----------------------------------------------------------------------
Function name:  GetFlatSel

Description:    Allocates a selector and calls INT 31h to
                set the limit.
Information:    

Return:         UINT    The selector if successful or,
                        0 if failure.
----------------------------------------------------------------------*/

// STB Begin Changes
// STB-SR 1/13/98 Added code for bj
#ifdef STB_FIFO16_ENABLE
UINT FlatSel;				// Compiling for FIFO16 code, remove contraints on FlatSel
#else
// STB End Changes
static UINT FlatSel;		// Else, keep it static.
// STB Begin Changes
#endif
// STB End Changes


#pragma optimize("", off)
UINT GetFlatSel()
{
    if (FlatSel != 0)
        return FlatSel;

    FlatSel = AllocSelector(SELECTOROF((LPVOID)&FlatSel));

    if (FlatSel == 0)
        return 0;

    SetSelectorBase(FlatSel, 0);

    // SetSelectorLimit(FlatSel, -1);
    _asm    mov     ax,0008h            ; DPMI set limit
    _asm    mov     bx,FlatSel
    _asm    mov     dx,-1
    _asm    mov     cx,-1
    _asm    int     31h

    return FlatSel;
}
#pragma optimize("", on)


/*----------------------------------------------------------------------
Function name:  FreeFlatSel

Description:    Releases a previously allocated flat selector.

Information:    

Return:         UINT    The selector if successful or,
                        0 if failure.
----------------------------------------------------------------------*/
void FreeFlatSel()
{
    if (FlatSel)
    {
        SetSelectorLimit(FlatSel, 0);
        FreeSelector(FlatSel);
        FlatSel = 0;
    }
}


/*----------------------------------------------------------------------
Function name:  PhysToLinear

Description:    Calls INT 31h to do a physical to linear tranlation.

Information:    

Return:         DWORD   The linear address
----------------------------------------------------------------------*/
#pragma optimize("", off)
DWORD PhysToLinear(DWORD PhysAddress, DWORD PhysSize)
{
    DWORD LinearAddress;

    PhysSize = PhysSize-1;      // we want limit, not size for DPMI

    _asm
    {
        mov     cx, word ptr PhysAddress[0]
        mov     bx, word ptr PhysAddress[2]
        mov     di, word ptr PhysSize[0]
        mov     si, word ptr PhysSize[2]
        mov     ax, 0800h               ; DPMI phys to linear
        int     31h
        mov     word ptr LinearAddress[0], cx
        mov     word ptr LinearAddress[2], bx
    }

    return LinearAddress;
}


#ifdef DEBUG

/*----------------------------------------------------------------------
Function name:  Msg

Description:    Format a debug string and send it to the debug
                monitor.

Information:    Only used when "#ifdef DEBUG"

Return:         VOID
----------------------------------------------------------------------*/
/* h3g.h defines DPF(DBGLVL_NORMAL,) into Msg() */
DWORD DbgMsgLvl = 0; //DBGLVL_NORMAL;
//DWORD DbgMsgLvl = DBGLVL_LCD;

#define START_STR "H3 2D: "

void __cdecl _loadds Msg(DWORD DbgLvl, LPSTR szFormat, ...)
{
    char        str[1024];
    static int (WINAPI *fpwvsprintf)(LPSTR lpszOut, LPCSTR lpszFmt,
                  const void FAR* lpParams);

    if (!(DbgLvl & DbgMsgLvl)) return;

    if (fpwvsprintf == NULL)
    {
        fpwvsprintf = (LPVOID) GetProcAddress(GetModuleHandle("USER"),"wvsprintf");
        if (fpwvsprintf == NULL)
            return;
    }

    lstrcpy(str, START_STR);
    fpwvsprintf(str+lstrlen(str), szFormat, (LPVOID)(&szFormat+1));
    lstrcat(str, "\r\n");
    OutputDebugString(str);
}

#endif // #ifdef DEBUG

/*
addr        : address to the base memmaped registers.
            if sim the addr must == 0x100000000.
register    : offset from register base address
data        :  dword data

Typically GET is a macro but I am implementing it as a function until
debugged.
Routine decides hardware or csim.
The level above decide hw or dibengine.

*/

#define VXDLDR_INIT_DEVICE 1
//#define VXDLDR_DEVICE_ID 0x27
#define VXDLDR_LoadDevice 1
#define VXDLDR_UnloadDevice 2


/*----------------------------------------------------------------------
Function name:  get_vxdldr_apiproc

Description:    Get the dynamic vxd loader's protected mode api
                entry point.
Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
DWORD vxdldr_apiproc;
int
get_vxdldr_apiproc()
{
    if (vxdldr_apiproc != 0L)
    return 1;

    _asm xor di,di;
    _asm mov es,di;
    _asm mov ax, 1684h;
    _asm mov bx,VXDLDR_DEVICE_ID;
    _asm int 2fh;
    _asm mov ax,es;
    _asm or ax,di;
    _asm jnz GoAhead;
    DPF(DBGLVL_NORMAL,"Could not find vxdldr entry point");
    return 0;

  GoAhead:
    _asm mov word ptr [vxdldr_apiproc],di;
    _asm mov word ptr [vxdldr_apiproc+2],es;

    return 1;
}


char VxDName[] = "H3HAL.VXD";
char VxDModName[]="H3HAL";


/*----------------------------------------------------------------------
Function name:  loadCsimVxd

Description:    Calls the VXDLDR to dynamically load the csim vxd

Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
int
loadCsimVxd()
{
    _asm mov ax,VXDLDR_LoadDevice;
    _asm mov dx, offset VxDName;
    _asm call dword ptr [vxdldr_apiproc];
    _asm jnc GoAhead2;
    DPF(DBGLVL_NORMAL,"Error loading csim VxD");
    return 0;

  GoAhead2:
    return 1;
}


/*----------------------------------------------------------------------
Function name:  get_csim_apiproc

Description:    gets the csim vxd's PM API entrypoint

Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
DWORD csim_api;
int
get_csim_apiproc()
{
    _asm xor di,di;
    _asm mov es,di;
    _asm mov ax, 1684h;
    _asm mov bx,0beefh;    csim vxdid (hack!)
    _asm int 2fh;
    _asm mov ax,es;
    _asm or ax,di;
    _asm jnz GoAhead;
    DPF(DBGLVL_NORMAL,"Could not find csim vxd entry point");
    return 0;

  GoAhead:
    _asm mov word ptr [csim_api],di;
    _asm mov word ptr [csim_api+2],es;
    return 1;

}


/*----------------------------------------------------------------------
Function name:  unloadCsimVxD

Description:    Calls the VXDLDR to dynamically unload the csim vxd.

Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
int
unloadCsimVxD()
{
    csim_api = 0L;    /* clear the PM api entrypoint */

    _asm mov ax,VXDLDR_UnloadDevice;
    _asm mov dx, offset VxDModName;
    _asm mov bx,-1;
    _asm call dword ptr [vxdldr_apiproc];
    _asm jnc GoAhead3;
    DPF(DBGLVL_NORMAL,"Error unloading csim VxD");
    return 0;

  GoAhead3:
    return 1;
}


/*----------------------------------------------------------------------
Function name:  start_csim

Description:    Loads the csim vxd and initializes the
                invariant registers.
Information:    

Return:         INT     1 for Success, or
                        0 for Failure.
----------------------------------------------------------------------*/
int
start_csim()
{
    DWORD screenAddress = _FF(ScreenAddress);
    DWORD nbytes = 0x400000L;

    if (!get_vxdldr_apiproc())
    return 0;

    if (!loadCsimVxd())
    return 0;

    if (!get_csim_apiproc())
    return 0;

    __asm _emit 66h __asm push di;
    __asm _emit 66h __asm mov di, word ptr screenAddress;
    __asm _emit 66h __asm mov ax, word ptr nbytes;
    __asm _emit 66h __asm xor cx, cx;
    __asm mov cx, 1;             //ecx is fn #1 (init simulator)
    __asm call dword ptr [csim_api];
    __asm _emit 66h __asm pop di;

    return 1;
}


WORD is_csim_started = 0;
WORD reload_csim = 0;
WORD csimLoadError = 0;

DWORD funresult;

#ifdef HAL_CSIM
/*----------------------------------------------------------------------
Function name:  h3WRITE

Description:    Write a Banshee/Avenger HW register.

Information:    Only used for HAL_CSIM

Return:         VOID
----------------------------------------------------------------------*/
void FAR _loadds h3WRITE(unsigned long far * hwptr, DWORD * reg, DWORD data)
{
    DWORD dwReg = (DWORD)reg;

  // left over from 16 bit only csim/driver
  // with 32 bit driver, csim is loaded by default.

    /* finally, actually send call to csim */
        __asm _emit 66h __asm push di;
        __asm _emit 66h __asm mov di, word ptr dwReg;//edi is reg #
        __asm _emit 66h __asm mov ax, word ptr data;    //eax is data
        __asm _emit 66h __asm xor cx, cx;
        __asm mov cx, 2;             //ecx is fn #2 (write register)
        __asm call dword ptr [csim_api];
        __asm _emit 66h __asm pop di;

}


/*----------------------------------------------------------------------
Function name:  h3READ

Description:    Read a Banshee/Avenger HW register.

Information:    Only used for HAL_CSIM.

Return:         ULONG   register value
----------------------------------------------------------------------*/
unsigned long FAR _loadds h3READ(unsigned long far * hwptr,
                    unsigned long far * reg)
{

    DWORD dwReg = (DWORD)reg;
    unsigned long retval;


        __asm _emit 66h __asm push di;
        __asm _emit 66h __asm mov di, word ptr dwReg;//edi is reg #
        __asm _emit 66h __asm xor cx, cx;
        __asm mov cx, 3;             //ecx is fn #3 reg read
        __asm call dword ptr [csim_api];
        __asm _emit 66h __asm pop di;

        __asm mov word ptr retval, ax
        __asm mov word ptr retval+2, dx

    return retval;

}


/*----------------------------------------------------------------------
Function name:  h3WRITE_HW

Description:    Write data to Banshee/Avenger HW.  h3WRITE is
                necessary because the 16-bit compiler can't do
                16:32 pointers, only direct writes.

Information:    Only used for HAL_CSIM.

Return:         VOID
----------------------------------------------------------------------*/
void FAR _loadds
h3WRITE_HW (DWORD * addr,  
             DWORD data)
{
    __asm
    {
       push    es;
       _emit 0x66 _asm push    di;
       _emit 0x66 _asm push    si;
       mov    ax, word ptr FlatSel;
       mov    es, ax;
       _emit 0x66 _asm mov    di, word ptr addr
       _emit 0x66 _asm mov    cx, word ptr data

      //  the assembler can't figure out
      //     mov   es:[edi], ecx
      //  so I will do it for it......
      _emit 0x67
      _emit 0x66
      _emit 0x26
      _emit 0x89
      _emit 0x0F

      _emit 0x66 _asm pop    si;
      _emit 0x66 _asm pop    di;
       pop    es;
    }

// this makes the cursor damn slow
#if 0
    DPF(DBGLVL_NORMAL,"h3Write: address = 0x%08lx, data = 0x%08lx\n",
    (DWORD)addr,
    (DWORD) data);
#endif

}


/*----------------------------------------------------------------------
Function name:  h3READ_HW

Description:    Read data from Banshee/Avenger HW.  h3READ is
                necessary because the 16-bit compiler can't do
                16:32 pointers.

Information:    Only used for HAL_CSIM.

Return:         ULONG   register value
----------------------------------------------------------------------*/
unsigned long FAR _loadds
h3READ_HW( unsigned long far * addr)
{
    FxU32 retval;

    __asm
    {
       push    es;
       _emit 0x66 _asm push    di;
       _emit 0x66 _asm push    si;
       mov    ax, word ptr FlatSel;
       mov    es, ax;
       _emit 0x66 _asm mov    di, word ptr addr
       // this is what I want
       //    mov   eax, es:[edi]
       //  but the assembler can figure that out
       //  so I will do it for it......
       _emit 0x67
       _emit 0x66
       _emit 0x26
       _emit 0x8B
       _emit 0x07
       _emit 0x66 _asm mov    word ptr retval, ax
       _emit 0x66 _asm pop    si;
       _emit 0x66 _asm pop    di;
       pop    es;
    }


#if 0
    DPF(DBGLVL_NORMAL,"h3Read: address = 0x%08lx\n", (DWORD)addr);
#endif

    return retval;
}






#else // #ifdef HAL_CSIM


/*----------------------------------------------------------------------
Function name:  h3WRITE

Description:    Write a Banshee/Avenger HW register.

Information:    Used for actual HW.

Return:         VOID
----------------------------------------------------------------------*/
void FAR _loadds
h3WRITE(unsigned long far * hwptr, DWORD * addr, DWORD data)
{
    __asm
    {
       push    es;
 _emit 0x66 _asm push    di;
 _emit 0x66 _asm push    si;
       mov    ax, word ptr FlatSel;
       mov    es, ax;
 _emit 0x66 _asm mov    di, word ptr addr
 _emit 0x66 _asm mov    cx, word ptr data
// this is what I want
//    mov   es:[edi], ecx
//  but the assembler can't figure that out
//  so I will do it for it......
      _emit 0x67
      _emit 0x66
      _emit 0x26
      _emit 0x89
      _emit 0x0F
 _emit 0x66 _asm pop    si;
 _emit 0x66 _asm pop    di;
       pop    es;
    }

// this makes the cursor damn slow
#if 0
    DPF(DBGLVL_NORMAL,"h3Write: address = 0x%08lx, data = 0x%08lx\n",
    (DWORD)addr,
    (DWORD) data);
#endif

}


/*----------------------------------------------------------------------
Function name:  h3READ

Description:    Read a Banshee/Avenger HW register.

Information:    Used for actual HW.

Return:         ULONG   register value
----------------------------------------------------------------------*/
unsigned long FAR _loadds
h3READ(unsigned long far * hwptr,
       unsigned long far * addr)
{
    FxU32 retval;

    __asm
    {
       push    es;
 _emit 0x66 _asm push    di;
 _emit 0x66 _asm push    si;
       mov    ax, word ptr FlatSel;
       mov    es, ax;
 _emit 0x66 _asm mov    di, word ptr addr
// this is what I want
//    mov   eax, es:[edi]
//  but the assembler can figure that out
//  so I will do it for it......
      _emit 0x67
      _emit 0x66
      _emit 0x26
      _emit 0x8B
      _emit 0x07
 _emit 0x66 _asm mov    word ptr retval, ax
 _emit 0x66 _asm pop    si;
 _emit 0x66 _asm pop    di;
       pop    es;

    }

#if 0
    DPF(DBGLVL_NORMAL,"h3Read: address = 0x%08lx\n", (DWORD)addr);
#endif

    return retval;
}

#endif // #ifdef HAL_CSIM


/*----------------------------------------------------------------------
Function name:  VDDCall

Description:    Calls down into the display driver's VDD.

Information:    

Return:         DWORD   Result returned by the call into the VDD or,
                        -1L if failure.
----------------------------------------------------------------------*/
#pragma optimize("", off)
DWORD VDDCall(DWORD myEAX, DWORD VDDmagicNumber, DWORD myECX, DWORD myEDX, LPVOID esdi)
{
    static DWORD   VDDEntryPoint = 0;
#if 0
    static DWORD   VDDmagicNumber = 0;
#endif
    DWORD   result=0xFFFFFFFF;
    DWORD dwDevNode;

    if (VDDEntryPoint == 0)
    {
    _asm
    {
       xor       di,di           ;//set these to zero before calling
       mov     es,di           ;
       mov     ax,1684h        ;//INT 2FH: Get VxD API Entry Point
       mov     bx,0ah          ;//this is device code forVDD
       int     2fh             ;//call the multiplex interrupt
       mov     word ptr VDDEntryPoint[0],di    ;
       mov     word ptr VDDEntryPoint[2],es    ;//save the returned data
#if 0
       mov     ax, 1683h;
       int       2fh;
       mov     word ptr VDDmagicNumber[0], bx
#endif
    }


    if (VDDEntryPoint == 0)
       return result;
    }

    dwDevNode = DisplayInfo.diDevNodeHandle;

    _asm
    {
    _emit 66h _asm push si                       ;// push esi
    _emit 66h _asm push di                       ;// push edi
    _emit 66h _asm mov ax,word ptr myEAX      ;//eax = function
    _emit 66h _asm mov bx,word ptr VDDmagicNumber   ;//ebx = device
    _emit 66h _asm mov cx,word ptr myECX      ;//ecx = buffer_size
    _emit 66h _asm mov dx,word ptr myEDX         ;//edx = flags
    _emit 66h _asm mov si,word ptr dwDevNode    ;//esi=dwDevNode
    _emit 66h _asm xor di,di                     ;// HIWORD(edi)=0
    les     di, esdi                ;
    call    dword ptr VDDEntryPoint             ;//call the VDDs PM API
    _emit 66h _asm mov word ptr result, ax

      _emit 66h _asm pop di                        ;// pop edi
      _emit 66h _asm pop si                        ;// pop esi
    }

    return result;
}
#pragma optimize("", on)



/*----------------------------------------------------------------------
Function name:  DoIntersect

Description:    Evaluate whether or not two rectangles intersect.
                One rectange is always the cursor rect.  Routine
                used bitmask to evalulate all possibilities at once.
Information:    

Return:         DWORD   TRUE  if is intersect or
                        FALSE if not intersect.
----------------------------------------------------------------------*/
int IsIntersect[] = {  // y2 x2 y1 x1
   FALSE,            // 0  0  0  0
   FALSE,            // 0  0  0  1
   FALSE,            // 0  0  1  0
   TRUE,             // 0  0  1  1
   FALSE,            // 0  1  0  0
   FALSE,            // 0  1  0  1
   TRUE,             // 0  1  1  0
   TRUE,             // 0  1  1  1
   FALSE,            // 1  0  0  0
   TRUE,             // 1  0  0  1
   FALSE,            // 1  0  1  0
   TRUE,             // 1  0  1  1
   TRUE,             // 1  1  0  0
   TRUE,             // 1  1  0  1
   TRUE,             // 1  1  1  0
   TRUE,             // 1  1  1  1
   };
#define RANGE(val,lo,hi) ((lo) <= (val) && (val) <= (hi))
int DoIntersect (SHORT left, SHORT top, SHORT right, SHORT bottom)
{
   int bIntersect;
   int cl, cr, ct, cb;

   DEBUG_FIX;


#if 0
   cl = _FF(LastCursorPosX) - _FF(HotspotX);
   cr = cl + (int) SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY) - _FF(HotspotY);
   cb = ct + (int) SWCURSOR_HEIGHT;
#else
   cl = _FF(LastCursorPosX);
   cr = cl + (int) SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY);
   cb = ct + (int) SWCURSOR_HEIGHT;
#endif

   if (right - left <= SWCURSOR_WIDTH)
      {
      bIntersect = RANGE(left, cl, cr);
      bIntersect |= (RANGE(right, cl, cr) << 2);
      }
   else
      {
      bIntersect = RANGE(cl, left, right);
      bIntersect |= (RANGE(cr, left, right) << 2);
      }

   if (bottom - top <= SWCURSOR_WIDTH)
      {
      bIntersect |= (RANGE(top, ct, cb) << 1);
      bIntersect |= (RANGE(bottom, ct, cb) << 3);
      }
   else
      {
      bIntersect |= (RANGE(ct, top, bottom) << 1);
      bIntersect |= (RANGE(cb, top, bottom) << 3);
      }
   return IsIntersect[bIntersect];
}

