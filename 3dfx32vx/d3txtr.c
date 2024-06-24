/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Log: 
**  48   3dfx      1.47        9/6/00   Don Fowler      Altered the palletized 
**       alpha code to get the alpha from the flags field of the palette entry.
**  47   3dfx      1.46        7/10/00  Michel Conrad   Palette download cleanup
**       and tweak to mop.
**  46   3dfx      1.45        6/16/00  Evan Leland     Add texture palette 
**       support routines
**  45   3dfx      1.44        6/15/00  Michel Conrad   Delete checkin comments 
**       from sourcesafe era. Get ready to delete txtrinit.
**  44   3dfx      1.43        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  43   3dfx      1.42        3/30/00  Miles Smith     Fixed some init code for
**       the fog table variables.
**  42   3dfx      1.41        2/4/00   Evan Leland     moved AGP download code 
**       to d3txtr2.c
**  41   3dfx      1.40        1/28/00  Brent Burton    Fixed some struct 
**       declarations in shared.h and removed all references to the obsolete 
**       TXTRDESC.  All references are to txtrDesc.
**  40   3dfx      1.39        1/28/00  Evan Leland     mods for DX7
**  39   3dfx      1.38        1/28/00  Evan Leland     more dx7 upgrade work
**  38   3dfx      1.37        1/27/00  Evan Leland     DX7 changes
**  37   3dfx      1.36        1/25/00  Evan Leland     updates for dx7
**  36   3dfx      1.35        1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  35   3dfx      1.34        1/13/00  Brent Burton    Clean up UV and UVL 
**       texture format checking and associated macros.
**  34   3dfx      1.33        1/5/00   Evan Leland     Fix for PRS 11217 
**       texture gradient tests fail: not returning pitch correctly at surface 
**       create, no setting return address at texture lock for small mipmaps.
**  33   3dfx      1.32        1/4/00   Evan Leland     moved TXTRDESC and 
**       MIPMAPINFO structs to shared.h
**  32   3dfx      1.31        12/16/99 Evan Leland     fixed compile warning in
**       d3init.c by moving two prototypes
**  31   3dfx      1.30        12/16/99 Evan Leland     changed function names 
**       and put prototypes in d3txtr.h for all texture routines that are 
**       exported from the texture module
**  30   3dfx      1.29        12/14/99 Evan Leland     Preliminary merge of 
**       texture code with FXSURFACEDATA structure, also added new 
**       TEXTURELOADDP2 entry point
**  29   3dfx      1.28        12/9/99  Evan Leland     fixed a compile bug
**  28   3dfx      1.27        12/9/99  Evan Leland     fixes for texture 
**       mapping->formats tests in verdict
**  27   3dfx      1.26        12/1/99  Evan Leland     modifications for dx7
**  26   3dfx      1.25        11/30/99 Evan Leland     correctly fail RGB 888 
**       format on surface create; allocate texture virtual address at surface 
**       create time; clean up surfaceData values; free cam entry and vm in 
**       textureSurfaceDelete; use previously allocated cam entry if any at 
**       textureLock time; add textureUnlock
**  25   3dfx      1.24        11/19/99 Andrew Sobczyk  Changed code to support 
**       SetRenderTarget
**  24   3dfx      1.23        11/18/99 Evan Leland     fail surface create when
**       unsupported texture format requested; take format of primary surface 
**       when no texture format specified; add support for lock of NPT textures
**  23   3dfx      1.22        11/18/99 Michel Conrad   Fix caps check probelm 
**       for non-lcoal surfaces.
**  22   3dfx      1.21        11/15/99 Evan Leland     fixed a few things with 
**       the camMgr... interface
**  21   3dfx      1.20        11/15/99 Evan Leland     now using new camMgr 
**       interface for obtaining a CAM entry
**  20   3dfx      1.19        11/9/99  Evan Leland     more code 
**       cleanup/rearranging
**  19   3dfx      1.18        11/9/99  Evan Leland     code cleanup, removed 
**       unnecessary header files
**  18   3dfx      1.17        11/8/99  Evan Leland     General code cleanup: 
**       move texture-specific structures to d3txtr.h, move texture handle 
**       allocation macros to d3txtr.c, remove static texture handle allocation 
**       code that is no longer used; remove WINNT define
**  17   3dfx      1.16        11/8/99  Evan Leland     fix for textureLock: 
**       must idle hardware before granting the lock
**  16   3dfx      1.15        11/5/99  Andrew Sobczyk  Changed phantom_alloc 
**       and phantom_free parameters
**  15   3dfx      1.14        11/5/99  Andrew Sobczyk  Added code to 
**       allocate/delete FXSURFACEDATA
**  14   3dfx      1.13        11/1/99  Michel Conrad   Correct problem with 
**       local textures when AGP_EXECUTE is not defined.
**  13   3dfx      1.12        10/31/99 Michel Conrad   Add execute mode texture
**       support.
**  12   3dfx      1.11        10/26/99 Evan Leland     Implements FXT1 texture 
**       mode support
**  11   3dfx      1.10        10/21/99 Evan Leland     adds npt texture 
**       download thru host blt
**  10   3dfx      1.9         10/19/99 Evan Leland     minor cleanup prior to 
**       enabling npt blt download
**  9    3dfx      1.8         10/6/99  Evan Leland     incorporated new texorg 
**       changes
**  8    3dfx      1.7         10/1/99  Mark Einkauf    Complete HW_ACCESS 
**       macros work
**  7    3dfx      1.6         9/28/99  Brent Burton    Renamed prc->texture to 
**       prc->dx5texture.
**  6    3dfx      1.5         9/24/99  Mark Einkauf    call CMDFIFO_BUMP for 
**       each scanline of blts - effects AGP fifo only
**  5    3dfx      1.4         9/24/99  Evan Leland     fixes bug where lfb NPT 
**       stride must be a power of two
**  4    3dfx      1.3         9/16/99  Evan Leland     forgot something on 
**       previous checkin
**  3    3dfx      1.2         9/16/99  Evan Leland     adds 
**       txtrDownloadLevelDbg : debug download method (arranges textures in 
**       tiled format but without using a blt or cam)
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
*/

#include "d3global.h"
#include "ddglobal.h"
#include "fifomgr.h"
#include "ddcam.h"                          // FxCam..., cam manager defines
#include "d3txtr.h"                         // self
#include "d3npt.h"

// prototypes for functions that are not exported from the texture module

void __stdcall textureHandle(RC *, ULONG );

signed int  __stdcall ALLOCTEXTUREDESC(NT9XDEVICEDATA * ppdev, txtrDesc *txtrList);
void __stdcall FREETEXTUREDESC(NT9XDEVICEDATA * ppdev, int handle);
signed int  __stdcall ALLOCTEXTUREHANDLE(NT9XDEVICEDATA * ppdev, TXHNDLSTRUCT *txtrList);
void __stdcall FREETEXTUREHANDLE(NT9XDEVICEDATA * ppdev, int handle);
void  __stdcall TXTRFREEHANDLESFORPROCESS(NT9XDEVICEDATA *ppdev, DWORD pid);
void  __stdcall TXTRFREEHANDLESFORCONTEXT(NT9XDEVICEDATA * ppdev, DWORD context);

// macros that are not exported from the texture module

#define TXTRHNDL_INVALID                -1
#define TXTRHNDL_ALLOC_ENTRIES_SIZE     1280

#define TEXTURE_IS_NPT_TEXTURE(w, h, slog, tlog) \
    (((slog == MAX_LMS_LOG2) && (w != 2048)) ||  \
     ((tlog == MAX_LMS_LOG2) && (h != 2048)))

#define MAX_LMS_LOG2  11
#define LOG2LMS(log2) (11-log2)
#define LOG2LOD(log2) (11-log2)

#define MAXTEXTURECOUNT \
    (_D3(numTextureDescs) )

#define MAXTEXTUREHANDLE \
    (_D3(numTextureHndls) )

// typedefs and structs that are not exported from the texture module

static float textureCenter[12] = {
    1.f/2.f,    1.f/4.f,    1.f/8.f,    1.f/16.f,
    1.f/32.f,   1.f/64.f,   1.f/128.f,  1.f/256.f, 
    1.f/512.f,  1.f/1024.f, 1.f/2048.f, 1.f/4096.f
};

// TEMP: these will become macros when they are ready

TXHNDLSTRUCT * TXTRHNDL_TO_TXHNDLSTRUCT(RC *pRc, DWORD txtrHndl)
{
    HNDLLIST *handle_list;
    TXHNDLSTRUCT *thstruct;

    if (txtrHndl == 0)
        return 0;

    handle_list = pRc->pHndlList;
    if (handle_list == 0)
        return 0;

    if (txtrHndl > (DWORD)pRc->pHndlList->ppTxtrHndlList[0]) {
        _asm int 3;
        return 0;
    }

    thstruct = pRc->pHndlList->ppTxtrHndlList[txtrHndl];
    if (thstruct == 0)
        return 0;

    return thstruct;
}

txtrDesc * TXTRHNDL_TO_TXTRDESC(RC *pRc, DWORD txtrHndl)
{
    HNDLLIST *handle_list;
    TXHNDLSTRUCT *thstruct;
    txtrDesc *txtr;

    if (txtrHndl == 0)
        return 0;

    handle_list = pRc->pHndlList;
    if (handle_list == 0)
        return 0;

    if (txtrHndl > (DWORD)pRc->pHndlList->ppTxtrHndlList[0]) {
        _asm int 3;
        return 0;
    }

    thstruct = pRc->pHndlList->ppTxtrHndlList[txtrHndl];
    if (thstruct == 0)
        return 0;

    if (thstruct->surfData == 0)
        return 0;

    txtr = thstruct->surfData->pTxtrDesc;
    if (txtr == 0)
        return 0;

    return txtr;
}

int TXTRHNDL_SURFDATA_VALID(RC *pRc, DWORD txtrHndl)
{
    TXHNDLSTRUCT *thstruct;

    if (txtrHndl == 0)
        return 0;
    if (pRc->pHndlList == 0)
        return 0;
    if (pRc->pHndlList->ppTxtrHndlList == 0)
        return 0;
    if (txtrHndl > (DWORD)pRc->pHndlList->ppTxtrHndlList[0]) {
        _asm int 3;
        return 0;
    }

    thstruct = pRc->pHndlList->ppTxtrHndlList[txtrHndl];

    if (thstruct == 0)
        return 0;
    if (thstruct->surfData == 0)
        return 0;
    else
        return 1;
}

int TXTRHNDL_TXTRDESC_VALID(RC *pRc, DWORD txtrHndl)
{
    TXHNDLSTRUCT *thstruct;

    if (txtrHndl == 0)
        return 0;
    if (pRc->pHndlList == 0)
        return 0;
    if (pRc->pHndlList->ppTxtrHndlList == 0)
        return 0;
    if (txtrHndl > (DWORD)pRc->pHndlList->ppTxtrHndlList[0]) {
        _asm int 3;
        return 0;
    }

    thstruct = pRc->pHndlList->ppTxtrHndlList[txtrHndl];

    if (thstruct == 0)
        return 0;
    if (thstruct->surfData == 0)
        return 0;
    if (thstruct->surfData->pTxtrDesc == 0)
        return 0;
    else
        return 1;
}

FXSURFACEDATA* TXTRHNDL_TO_SURFDATA(RC *pRc, DWORD txtrHndl)
{
    if (txtrHndl == 0)
        return 0;
    if (pRc->pHndlList == 0)
        return 0;
    if (pRc->pHndlList->ppTxtrHndlList == 0)
        return 0;
    if (txtrHndl > (DWORD)pRc->pHndlList->ppTxtrHndlList[0]) {
        _asm int 3;
        return 0;
    }
    if (pRc->pHndlList->ppTxtrHndlList[txtrHndl]->surfData == 0)
        return 0;
    else
        return (FXSURFACEDATA*)pRc->pHndlList->ppTxtrHndlList[txtrHndl]->surfData;
}

//---------------------------------------------------------------------------------------
//
// txtrInit()
//
// Init texture management
// this is called from d3init()
//
//---------------------------------------------------------------------------------------

void  __stdcall txtrInit(NT9XDEVICEDATA * ppdev)
{
    // This was used for a private texture buffer, not yet allocated.
    //_D3(buffer) = NULL ;
}

//---------------------------------------------------------------------------------------
//
// fxNopCmd()
//
//---------------------------------------------------------------------------------------

void fxNopCmd(
   NT9XDEVICEDATA *ppdev)
{
  CMDFIFO_PROLOG(cmdFifo);

  HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D); // 3d mop uses 3d reg if direct write

  CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );

  SETMOP(cmdFifo, (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT) |
                                 SST_MOP_FLUSH_PCACHE | SST_MOP_STALL_3D );

  HW_ACCESS_EXIT(ACCESS_3D);

  CMDFIFO_EPILOG( cmdFifo );
}

//---------------------------------------------------------------------------------------
//
// txtrDownLoadDXT(): download an S3 (DX6) compressed texture from a given real address
//                    to a CSIM address for use for later rendering
//
// TEMP: I removed all command fifo macros because they didn't change function
//       and I don't yet understand if they are necessary
//
// TEMP: removed register writes to taLMS and taMode which are done in the myTextureDownload
//       loop, but didn't seem to have any effect on the outcome
//
//---------------------------------------------------------------------------------------

#define WRITEDW(hwMemAddr, data) {*((FxU32*)(&hwMemAddr))  = (FxU32)(data); }
#define WRITEPD(hwPtr, hwMemAddr, data) WRITEDW((hwMemAddr), (data));

// texture palette manipulations ////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
//
// TXTR_GET_PAL_DESCRIPTOR
//
//---------------------------------------------------------------------------------------

PALHNDL* TXTR_GET_PAL_DESCRIPTOR(
    txtrDesc    *txtr,
    RC          *pRC,
    DWORD        txhndl)
{
    PALHNDL **palHandleListBase = pRC->pHndlList->ppPalHndlList;
    FXSURFACEDATA *surfData = TXTRHNDL_TO_SURFDATA(pRC, txhndl);
    DWORD palHandle = surfData->dwPaletteHandle;

    if ((palHandle != 0) && (palHandleListBase != NULL))
    {
        PALHNDL *palHandleCurrent = palHandleListBase[palHandle];
   
        if (palHandleCurrent != NULL)
            return palHandleCurrent;
    }

    return NULL;
}

//---------------------------------------------------------------------------------------
//
// txtrNewPalette()
//
//---------------------------------------------------------------------------------------

void __stdcall txtrNewPalette(
    NT9XDEVICEDATA *ppdev,
    PALHNDL        *paletteGBL)
{
    // if the new palette is not downloaded then mark it for download
    if ( (void *)paletteGBL != _D3(currentPalette) )
        _D3(flags) |= PALETTECHANGED; 
}

//---------------------------------------------------------------------------------------
//
// txtrDownloadPalette
//
//---------------------------------------------------------------------------------------

// try and burst 8 entry tables at a time (256/8=32)
#define BURSTSIZE	8
#define NUMBURSTS (256/BURSTSIZE)

void __stdcall txtrDownloadPalette(
  NT9XDEVICEDATA * ppdev, 
  txtrDesc *txtr,
  LPPALETTEENTRY pal)
{
  ULONG burst;
  ULONG line;
  ULONG entry = 0;

  CMDFIFO_PROLOG( cmdFifo );
  HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D);

  CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );
  // SETMOP( cmdFifo, SST_MOP_STALL_TEXTURE_PALETTE_LOAD );
  SETMOP( cmdFifo, SST_MOP_FLUSH_ALL_3D );


  if( txtr->txFormatFlags & TEXFMTFLG_PALETTIZED_ALPHA )
  {
    for( burst = 0; burst < NUMBURSTS; burst++ )
    {
      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + BURSTSIZE );
      SETPH( cmdFifo, CMDFIFO_BUILD_PAL_PK1(BURSTSIZE, SST_UNIT_PALETTE, entry));
    
      for( line = 0; line < BURSTSIZE; line++ )
      {
        ULONG data =
          // R G B 888
          ((pal[entry].peFlags & 0xFC) << 16) | ((pal[entry].peRed & 0xFC) << 10) | ((pal[entry].peGreen & 0xFC) << 4) | 
          ((pal[entry].peBlue & 0xFC) >> 2);

          SETPD( cmdFifo, *((volatile FxU32 *)(ghwTP) + entry), data );
          entry++;
      }
    }
  }
  else
  {
      for( burst = 0; burst < NUMBURSTS; burst++ )
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + BURSTSIZE );
        SETPH( cmdFifo, CMDFIFO_BUILD_PAL_PK1(BURSTSIZE, SST_UNIT_PALETTE, entry));
    
        for( line = 0; line < BURSTSIZE; line++ )
        {
          ULONG data =
            // R G B 888
            (pal[entry].peRed << 16) | (pal[entry].peGreen << 8) | (pal[entry].peBlue);

          SETPD( cmdFifo, *((volatile FxU32 *)(ghwTP) + entry), data );
          entry++;
        }
      }
  }

  // palette is downloaded
  _D3(flags) &= ~PALETTECHANGED; 

  HW_ACCESS_EXIT(ACCESS_3D);
  CMDFIFO_EPILOG( cmdFifo );

} /* txtrDownloadPalette */ 


