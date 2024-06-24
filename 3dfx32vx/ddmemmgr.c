/* $Header: ddmemmgr.c, 37, 12/8/00 12:13:49 PM PST, Brent Burton$ */
/*
** Copyright (c) 1996-2000, 3Dfx Interactive, Inc.
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
** File name:   ddmemmgr.c
**
** Description: Direct Draw/3D memory manager.
**
** $Log: 
**  37   3dfx      1.36        12/8/00  Brent Burton    Moved the assignments of
**       pVidMemAlloc and pVidMemFree out of a conditional to always set these.
**  36   3dfx      1.35        12/7/00  Ryan Bissell    Miscellaneous cleanup, 
**       and CS work
**  35   3dfx      1.34        11/23/00 Ryan Bissell    Lost-surface 
**       notification in Central Services, and new CS features requested by OGL 
**       Core.
**  34   3dfx      1.33        11/13/00 Miles Smith     cleanup of mode names 
**       for FSAA
**  33   3dfx      1.32        11/12/00 Ryan Bissell    Fixed some problems with
**       my NML phase II checkin from a week ago.
**  32   3dfx      1.31        11/5/00  Ryan Bissell    NML phase II, and misc 
**       cleanup
**  31   3dfx      1.30        11/1/00  Ryan Bissell    Cleanup of ddmemmgr.c, 
**       and related files.
**  30   3dfx      1.29        10/31/00 Ryan Bissell    Phase I of development 
**       of the "No Man's Land" reclamation heap.
**  29   3dfx      1.28        8/21/00  Ryan Bissell    Wasn't resizing the 
**       transient (ddraw) heap at modeset
**  28   3dfx      1.27        8/9/00   Miles Smith     Fixed some stride issues
**       which were breaking SDK sample apps
**  27   3dfx      1.26        8/4/00   Ryan Bissell    Added support for true 
**       AGP command FIFOs, and long JSR support in Central Services (needed 
**       because of AGP command FIFOs.)
**  26   3dfx      1.25        7/25/00  Ryan Bissell    Was freeing CS render 
**       buffers from the wrong heap.
**  25   3dfx      1.24        7/21/00  Ryan Bissell    Different configuration 
**       for heaps when using AGP command FIFO.
**  24   3dfx      1.23        7/14/00  Ryan Bissell    Partial fix for "report 
**       free memory" size bug in fxmm
**  23   3dfx      1.22        7/13/00  Ryan Bissell    Partial removal of ddmem
**       / fxmm shim layer, and bug fix in cs_SurfaceAllocator
**  22   3dfx      1.21        7/10/00  Michel Conrad   Externs for addattached 
**       surface to help cubemap code.
**  21   3dfx      1.20        6/29/00  Miles Smith     Placed an if statement 
**       around a macro which should only be used if tiled memory is in stagger 
**       mode
**  20   3dfx      1.19        6/29/00  Ryan Bissell    Fixed one-off in the 
**       memory manager shim, that was causing WinSIM to complain about 
**       alignment errors.
**  19   3dfx      1.18        6/26/00  Ryan Bissell    Incremental CS 
**       development.  Also removed some message pragmas, in anticipation of CS 
**       activation.
**  18   3dfx      1.17        6/15/00  Michel Conrad   Fix compilier warnings.
**  17   3dfx      1.16        6/9/00   Miles Smith     Updated code in 
**       createsurface and ddlock to handle new fullscreen aa modes
**  16   3dfx      1.15        5/4/00   Miles Smith     fixed a problem with 
**       tile sizes that was breaking windowed AA
**  15   3dfx      1.14        5/3/00   Ryan Bissell    Continued deployment of 
**       Central Services and related changes.
**  14   3dfx      1.13        4/14/00  Ryan Bissell    Central Services 
**       deployment
**  13   3dfx      1.12        3/17/00  Miles Smith     I changed an AA global 
**       variable name to match the Napalm code.  Was AppRequestedAA - changed 
**       to ddAAModeRequested
**  12   3dfx      1.11        3/6/00   Ryan Bissell    Added control interfaces
**       for Central Services
**  11   3dfx      1.10        3/3/00   Miles Smith     Made a minor change 
**       regarding AA surface sizes
**  10   3dfx      1.9         2/3/00   Ryan Bissell    Fixed memory leaks in 
**       memMgr_VidMemAlloc(), unnecessary alignment rounding in 
**       memMgr_VidMemAlloc(), and an early-exit bug in mmWatchDog().  (See PRS 
**       12655, and 12658.)
**  9    3dfx      1.8         2/1/00   Michel Conrad   Make phantom space 2x of
**       physical heap space.
**  8    3dfx      1.7         1/10/00  Miles Smith     changes for fullscreen 
**       aa support
**  7    3dfx      1.6         11/30/99 Andrew Sobczyk  Fixed a bug with 
**       RemoveSLIMemory where we were GPF when more then one item was in the 
**       list
**  6    3dfx      1.5         11/23/99 Xing Cong       Allocate overlay surface
**       from linear surface instead of tile1.   So overlay can fetch data from 
**       every other line. 
**  5    3dfx      1.4         11/5/99  Andrew Sobczyk  Changed 
**       phantom_free/alloc parameters
**  4    3dfx      1.3         11/5/99  Andrew Sobczyk  Modified to support SLI 
**       heap and to remove bug in mmWatchDog
**  3    3dfx      1.2         10/31/99 Michel Conrad   Add support for 
**       non-local surface allocation using ddraw's memory manger.
**  2    3dfx      1.1         9/24/99  Xing Cong       Delete tmeporary fix 
**       memory list and CAM entry init.
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
**
*/

#include "precomp.h"

#include <sst2glob.h>                //Must be included into sst2.h

#include "ddcam.h"                  // FxCamInit()

#include "fxmmcfg.h"    //configure FXMM for this OS
#include "fxmm.h"
#include "fxmmsst2.h"   //configure FXMM for this device

#ifdef CSERVICE //[
#include "cstypes.h"
#endif //] CSERVICE

#define MINI_LEFT_MEM  0x2000        //8k smallest meory size to be saved 
                                     //in a node after truncated
#define MINI_MEM       0x1000        //4k smallest memory size for a node 


static DWORD (*pVidMemAlloc)();      //Function ptr to memMgr_VidMemAlloc or ddHal_VidMemAlloc
static int   (*pVidMemFree)();       //Function ptr to memMgr_VidMemFree

extern FLATPTR WINAPI  HeapVidMemAllocAligned( LPVIDMEM lpVidMem, DWORD dwWidth, DWORD dwHeight,
                                               LPSURFACEALIGNMENT lpAlignment, LPLONG lpNewPitch );

extern void    WINAPI  DDHAL32_VidMemFree( LPDDRAWI_DIRECTDRAW_GBL, int, FLATPTR );
extern void    WINAPI  VidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr );

// GetAttachedSurfaceLcl is a DDraw dll export function (like the heap
// alloc and free functions above). Used by cube map code to get attached
// faces. Since ddraw.lib is included in this module and not in d3d the
// myDDGetAttachedSurface is wrapper to it.

extern HRESULT WINAPI DDGetAttachedSurfaceLcl(
   LPDDRAWI_DDRAWSURFACE_LCL  this_lcl,
   LPDDSCAPS2  lpDDSCaps,
   LPDDRAWI_DDRAWSURFACE_LCL  *lplpDDAttachedSurfaceLcl);

HRESULT myDDGetAttachedSurface( 
   LPDDRAWI_DDRAWSURFACE_LCL  this_lcl,
   LPDDSCAPS2  lpDDSCaps,
   LPDDRAWI_DDRAWSURFACE_LCL  *lplpDDAttachedSurfaceLcl)
{
   HRESULT  rc;
   rc = DDGetAttachedSurfaceLcl( this_lcl, lpDDSCaps, lplpDDAttachedSurfaceLcl );
   return rc;
}


extern DWORD mmEvict32 (NT9XDEVICEDATA *ppdev, DWORD, DWORD, DWORD);
extern DWORD mmReclaim32 (NT9XDEVICEDATA *ppdev, DWORD);


typedef enum {TRANSIENT=0, PHANTOM, RECLAIMED, NONE} MEM_REGION;


#define FXMM_NAME_TRANSIENTHEAP heapnames[0]
#define FXMM_NAME_PHANTOMHEAP heapnames[1]
#define FXMM_NAME_RECLAMATIONHEAP heapnames[2]
#define FXMM_NAME_PERSISTENTHEAP heapnames[3]
static char* heapnames[] = { "TRANSIENT", "PHANTOM", "RECLAMATION", "PERSISTENT" };


typedef struct {
    GLOBALDATA *ppdev;
    DWORD dwSize;                 // Block size
    DWORD dwMemFlag;              // Memory type flags
    DWORD dwAddrMask;             // Linear address mask
    DWORD *lpPitch;               // Pitch info
    MEM_REGION memPool;
} VIDMEMALLOC_PARMS;

static DWORD memMgr_VidMemAlloc(VIDMEMALLOC_PARMS *pVMA);

static DWORD ddHal_VidMemAlloc(VIDMEMALLOC_PARMS *pVMA);

typedef struct {
    GLOBALDATA *ppdev;
    DWORD fpVidMem;
    DWORD hwVidMem;
    MEM_REGION memPool;
} VIDMEMFREE_PARMS;
                
static int memMgr_VidMemFree(VIDMEMFREE_PARMS *pVMF);


#ifdef DEBUG
BOOL mmWatchDog(GLOBALDATA *ppdev);
#endif

static DWORD RNDUP_16KB( DWORD size )        { size+=0x3fffL; return(size & ~0x3fffL); }
static DWORD RNDUP_T0_PGWIDTH( DWORD width ) { width+=0x0fL;  return(width & ~0x0fL);  }
static DWORD RNDUP_T1_PGWIDTH( DWORD width)  { width+=0x3L;   return(width & ~0x3L);   }

/************************************************************************
 * PUBLIC EXPORTED FUNCTIONS:
 *    surfMgr_allocSurface
 *    surfMgr_freeSurface
 ************************************************************************/

/*----------------------------------------------------------------------
Function name: surfMgr_allocSurface

Description:   Manage standard and user specified surfaces creations

               Allocation schemes:

               1. Color buffer (backbuffer & 3rd buffer) and Z buffer
                  Optimized/staggered mode TILE_1 at 16KB boundary

               2. Overlay 
                  Linear mode at dwAddrMask alignment or 32 bytes
                  
               3. Overlay/videoport
                  Linear mode at dwAddrMask alignment or 32 bytes

               4. Others 
                  Surface is created by setting surf type to zero
                  and specifying surface's attributes
                  Width/Height, or TileWidth/TileHeight
                  Tile flag for tile modes and alignment info
                  If tile mode is not specified, linear surface is
                  assumed

               NOTE:
               For user specified surface, set ddstype to 0, and 
               if tileflag is not specified, the default is linear,
               and dwAddrMask is only used for linear alignment
                     
Return:        DD_OK
               DDERR_UNSUPPORTED
               DDERR_INVALIDPARAMS
               DDERR_OUTOFVIDEOMEMORY
----------------------------------------------------------------------*/

DWORD surfMgr_allocSurface(

  LPDDRAWI_DIRECTDRAW_GBL lpDD,  // [IN] Ddraw VidMemAlloc may need this
  DWORD ddstype,                 // [IN] Type of DDRAW surface
  DWORD ddstype2,                // [IN] Type extension of DDRAW surface
  DWORD width,                   // [IN] width in bytes (linear space)
  DWORD height,                  // [IN] height in pixel (linear space)
  DWORD dwLnrAddrMask,           // [IN] address mask for linear mem
  DWORD *fpVidMem,               // [OUT] host lfb start address of allocation
  DWORD *endfpVidMem,            // [OUT] host lfb start address of allocation
  DWORD *hwVidMem,               // [OUT] hw vidmem address
  DWORD *lpPitch,                // [OUT] pitch in bytes
  DWORD *tileFlag)               // [IN]  Mode:      MEM_IN_TILE0, MEM_IN_TILE1, MEM_IN_LINEAR, MEM_STAGGER
                                 //       Alignment: MEM_AT_EVEN16K, MEM_AT_ODD16K, MEM_AT_EVEN8K, MEM_AT_ODD8K,
                                 //                  MEM_AT_16K, MEM_AT_8K, MEM_AT_4K, MEM_AT_256, MEM_AT_32
                                 // [OUT] Mode:      MEM_IN_TILE0, MEM_IN_TILE1, MEM_IN_LINEAR, MEM_STAGGER                                 
                                 //       Alignment: MEM_AT_EVEN16K, MEM_AT_ODD16K, MEM_AT_EVEN8K, MEM_AT_ODD8K,
                                 //                  MEM_AT_16K, MEM_AT_8K, MEM_AT_4K, MEM_AT_256, MEM_AT_32
{
   DWORD tileInX, tileInY, dwBlockSize;
   DWORD lfbPtr, dwMemAlign, dwMemTile;
   DWORD dwPitch;
   BOOL fixTileSizeForAA = FALSE;  // tile size will get doubled if in windowed or fullscreen mode AA mode
   VIDMEMALLOC_PARMS VMAparms;

   DD_ENTRY_SETUP(lpDD)

   VMAparms.memPool=TRANSIENT;

   if ( !(width && height) )
      return( DDERR_INVALIDPARAMS );
   
   if ( ddstype ) // Standard surface type?
   {
#ifdef AGP_EXECUTE
      if ( ddstype & DDSCAPS_NONLOCALVIDMEM ) 
      {
        FLATPTR lfptr;
        SURFACEALIGNMENT Alignment;
        LPVIDMEM   pvmHeap;
        DWORD dwNewPitch;

        if ( !*tileFlag || (*tileFlag & MEM_IN_LINEAR) )
        {
          //Allocate linear memory aligned at dwLnrAddrMask or otherwise 32byte boundary
          dwMemTile   = MEM_IN_LINEAR;
          dwMemAlign  = dwLnrAddrMask ? MEM_AT_LNR_MASK : MEM_AT_32;
         
          dwBlockSize = width * height;
          dwPitch     = width;
        }
        else
        {
          //Calculate width and height in tiles based on surface tile type and the given dimensions
          tileInX = (width + (SST2_TILE_WIDTH-1)) >> SST2_TILE_WIDTH_BITS;

          if ( *tileFlag & MEM_IN_TILE1 )                 //Tile 1 memory type
            tileInY = (height + (SST2_TILE1_HEIGHT-1)) >> SST2_TILE1_HEIGHT_BITS;
          else                                            //Tile 0 memory type
            tileInY = (height + (SST2_TILE0_HEIGHT-1)) >> SST2_TILE0_HEIGHT_BITS;

            
          if ( *tileFlag & MEM_IN_TILE1 )                 //Tile 1 memory type
          {
            dwMemTile = MEM_IN_TILE1;
            
            if ( *tileFlag & MEM_STAGGER )
            {
               tileInX = RNDUP_T1_PGWIDTH(tileInX);      //Stagger mode TILE1 requirements
               dwMemTile |= MEM_STAGGER;
               dwMemAlign = MEM_AT_16K;                  //Stagger mode alignment
            }
            else
            {
               dwMemAlign = *tileFlag & MEM_ALIGN_MASK;  //Use the specified alignment
            }
            width   = tileInX << SST2_TILE_WIDTH_BITS;
            height  = tileInY << SST2_TILE1_HEIGHT_BITS;
            dwBlockSize = width * height;
            if ( *tileFlag & MEM_STAGGER )
               dwBlockSize = RNDUP_16KB(dwBlockSize);    //Stagger mode requirements
            dwPitch = width;
          }
          else                                            //Tile 0 memory type
          {
            dwMemTile = MEM_IN_TILE0;
            
            if ( *tileFlag & MEM_STAGGER )
            {
               tileInX = RNDUP_T0_PGWIDTH(tileInX);      //Stagger mode TILE0 requirements
               dwMemTile |= MEM_STAGGER;
               dwMemAlign = MEM_AT_16K;                  //Stagger mode alignment
            }
            else
            {
               dwMemAlign = *tileFlag & MEM_ALIGN_MASK;  //Use the specified alignment
            }
            width   = tileInX << SST2_TILE_WIDTH_BITS;
            height  = tileInY << SST2_TILE0_HEIGHT_BITS;
            dwBlockSize = width * height;
            if ( *tileFlag & MEM_STAGGER )
               dwBlockSize = RNDUP_16KB(dwBlockSize);    //Stagger mode requirements
            dwPitch = width;
          }
        }

        memset(&Alignment, 0, sizeof(Alignment));

        Alignment.Linear.dwStartAlignment = 256;
        Alignment.Linear.dwPitchAlignment = 256;

        // Point to base of vidMem[AGP_HEAP_ID]
        pvmHeap = lpDD->vmiData.pvmList+1;

        // Since we only get the linear gart address back, later
        // we calculate an offset from the linear base to
        // get the gart physical addresses of the surface. 

        if ( lfptr = HeapVidMemAllocAligned(pvmHeap, 
                                         dwBlockSize,
                                         1, 
                                         &Alignment, 
                                         &dwNewPitch) )
        {
          *fpVidMem = lfptr;
          *endfpVidMem = lfptr+dwBlockSize;
          *hwVidMem = lfptr - _FF(agpHeapLinBaseAddr)
                            + _FF(agpHeapPhysBaseAddr);

          *tileFlag = dwMemTile|dwMemAlign;
          *lpPitch   = dwNewPitch;
          return( DD_OK );
        }
        else
          return( DDERR_OUTOFVIDEOMEMORY );
      }
#endif 

      if ( (ddstype & DDSCAPS_3DDEVICE)      //3D Color buffer
           || (ddstype & DDSCAPS_ZBUFFER) )  //Or Z-buffer
      {
         tileInX = (width + (SST2_TILE_WIDTH-1)) >> SST2_TILE_WIDTH_BITS;
         if ( *tileFlag & MEM_STAGGER )
         {
             // mls 6/29/00 previously this was not inside an if check 
             // and it broke 800x600x16 mode if stagger was not on
             tileInX = RNDUP_T1_PGWIDTH(tileInX);      //Stagger mode requirements
         }
         tileInY = (height + (SST2_TILE1_HEIGHT-1)) >> SST2_TILE1_HEIGHT_BITS;

         // handle tile sizes for AA modes
         switch( _DD(ddFSAAMode)  )
         {
             case  FSAA_MODE_4XFLIP:
                // we only need to enlarge Y for secondary buffers here
                // because tileInX is already correct size
                tileInY <<= 1;
                break;
             case  FSAA_MODE_4XBLT:
                // in mode 3 the primary surface is normal sized,
                // but secondary buffer is doubled
                tileInX <<= 1;
                tileInY <<= 1; 
                break;
             case FSAA_MODE_FOUR: // this mode is currently not used, but will be combined with 4xblt mode
                if( _DD(ddAASecondaryBuffer1Start) == (DWORD) NULL )
                {
                    // this is back buffer 1 which is always super sized
                    tileInX <<= 1;
                    tileInY <<= 1;
                }
                break;
             default:   // take care  of windowed mode apps
                if( ddstype2 & DDSCAPS2_HINTANTIALIASING )
                {
                    tileInX <<= 1;
                    tileInY <<= 1;
                    break;
                }
         }

         
         width   = tileInX << SST2_TILE_WIDTH_BITS;
         height  = tileInY << SST2_TILE1_HEIGHT_BITS;
         dwBlockSize = RNDUP_16KB(width * height); //Stagger mode requirements
         dwPitch = width;

         VMAparms.ppdev=ppdev;
         VMAparms.dwSize=dwBlockSize;
         VMAparms.dwMemFlag=MEM_AT_16K;
         VMAparms.dwAddrMask=0;
         VMAparms.lpPitch=&dwPitch;

#ifdef SLI
         //if _THIS_ ddraw app is exclusive, and the desired surface isn't a
         //texture, try the RECLAMATION heap first.
         if ((_DS(ddExclusiveMode) == (DWORD)lpDD) && !(ddstype & DDSCAPS_TEXTURE))
         {
            VMAparms.memPool=RECLAIMED;
            if (!(lfbPtr = (*pVidMemAlloc)(&VMAparms)))
            {
              VMAparms.memPool=TRANSIENT;
              lfbPtr = (*pVidMemAlloc)(&VMAparms);
            }
         }
         else
           lfbPtr = (*pVidMemAlloc)(&VMAparms);

         if (0x0 != lfbPtr)
#else
         if ( lfbPtr = (*pVidMemAlloc)(&VMAparms) )
#endif
         {
            *fpVidMem = lfbPtr;
            *endfpVidMem = lfbPtr+dwBlockSize;
            *hwVidMem = lfbPtr - _FF(LFBBASE);
            //XXX FixME *tileFlag = MEM_IN_TILE1|MEM_STAGGER|MEM_AT_16K;         
            *tileFlag = MEM_IN_TILE1|MEM_AT_16K;         
            *lpPitch  = dwPitch;
            return( DD_OK );
         }
         return( DDERR_OUTOFVIDEOMEMORY );
      }
      else if ( ddstype & (DDSCAPS_OVERLAY|DDSCAPS_VIDEOPORT))
      {
         //Allocate linear memory aligned at dwLnrAddrMask or otherwise 32byte boundary
         dwMemAlign  = dwLnrAddrMask ? MEM_AT_LNR_MASK : MEM_AT_32;
         
         dwBlockSize = width * height;
         dwPitch     = width;
            
         VMAparms.ppdev=ppdev;
         VMAparms.dwSize=dwBlockSize;
         VMAparms.dwMemFlag=dwMemAlign;
         VMAparms.dwAddrMask=dwLnrAddrMask;
         VMAparms.lpPitch=&dwPitch;

         if ( lfbPtr = (*pVidMemAlloc)(&VMAparms) )
         {
            *fpVidMem = lfbPtr;
            *endfpVidMem = lfbPtr+dwBlockSize;
            *hwVidMem = lfbPtr - _FF(LFBBASE);
            *tileFlag = MEM_IN_LINEAR|dwMemAlign;
            *lpPitch  = dwPitch;
            return( DD_OK );
         }
         return( DDERR_OUTOFVIDEOMEMORY );
      }

      //else if ( ddstype & DDSCAPS_NEW ) //Additional standard surface support can be added here
         
      return( DDERR_UNSUPPORTED );
   }
   else //User specified surface type
   {
      if ( !*tileFlag || (*tileFlag & MEM_IN_LINEAR) )
      {
         //Allocate linear memory aligned at dwLnrAddrMask or otherwise 32byte boundary
         dwMemTile   = MEM_IN_LINEAR;
         dwMemAlign  = dwLnrAddrMask ? MEM_AT_LNR_MASK : MEM_AT_32;
         
         dwBlockSize = width * height;
         dwPitch     = width;
      }
      else
      {
         //Calculate width and height in tiles based on surface tile type and the given dimensions
         tileInX = (width + (SST2_TILE_WIDTH-1)) >> SST2_TILE_WIDTH_BITS;

         if ( *tileFlag & MEM_IN_TILE1 )                 //Tile 1 memory type
            tileInY = (height + (SST2_TILE1_HEIGHT-1)) >> SST2_TILE1_HEIGHT_BITS;
         else                                            //Tile 0 memory type
            tileInY = (height + (SST2_TILE0_HEIGHT-1)) >> SST2_TILE0_HEIGHT_BITS;

            
         if ( *tileFlag & MEM_IN_TILE1 )                 //Tile 1 memory type
         {
            dwMemTile = MEM_IN_TILE1;
            
            if ( *tileFlag & MEM_STAGGER )
            {
               tileInX = RNDUP_T1_PGWIDTH(tileInX);      //Stagger mode TILE1 requirements
               dwMemTile |= MEM_STAGGER;
               dwMemAlign = MEM_AT_16K;                  //Stagger mode alignment
            }
            else
            {
               dwMemAlign = *tileFlag & MEM_ALIGN_MASK;  //Use the specified alignment
            }
            width   = tileInX << SST2_TILE_WIDTH_BITS;
            height  = tileInY << SST2_TILE1_HEIGHT_BITS;
            dwBlockSize = width * height;
            if ( *tileFlag & MEM_STAGGER )
               dwBlockSize = RNDUP_16KB(dwBlockSize);    //Stagger mode requirements
            dwPitch = width;
         }
         else                                            //Tile 0 memory type
         {
            dwMemTile = MEM_IN_TILE0;
            
            if ( *tileFlag & MEM_STAGGER )
            {
               tileInX = RNDUP_T0_PGWIDTH(tileInX);      //Stagger mode TILE0 requirements
               dwMemTile |= MEM_STAGGER;
               dwMemAlign = MEM_AT_16K;                  //Stagger mode alignment
            }
            else
            {
               dwMemAlign = *tileFlag & MEM_ALIGN_MASK;  //Use the specified alignment
            }
            width   = tileInX << SST2_TILE_WIDTH_BITS;
            height  = tileInY << SST2_TILE0_HEIGHT_BITS;
            dwBlockSize = width * height;
            if ( *tileFlag & MEM_STAGGER )
               dwBlockSize = RNDUP_16KB(dwBlockSize);    //Stagger mode requirements
            dwPitch = width;
         }
      }
   }
   
   //This allocation attempt could be iterated based on memory aligment type which is obtained by indexing
   //alignment iteration table from best to least best alignment - Linear should be the last one in the entries.
   //

   VMAparms.ppdev=ppdev;
   VMAparms.dwSize=dwBlockSize;
   VMAparms.dwMemFlag=dwMemAlign;
   VMAparms.dwAddrMask=dwLnrAddrMask;
   VMAparms.lpPitch=&dwPitch;

   if ( lfbPtr = (*pVidMemAlloc)(&VMAparms) ) 
   {
#ifdef DEBUG
      mmWatchDog(ppdev);
#endif    
      
      *fpVidMem = lfbPtr;
      *endfpVidMem = lfbPtr+dwBlockSize;
      *hwVidMem = lfbPtr - _FF(LFBBASE);
      *tileFlag = dwMemTile|dwMemAlign;
      *lpPitch   = dwPitch;
      return( DD_OK );
   }

  return( DDERR_OUTOFVIDEOMEMORY );
  
}

/*----------------------------------------------------------------------
Function name: surfMgr_freeSurface

Description:   Free this surface memory

Return:        void 
----------------------------------------------------------------------*/
void surfMgr_freeSurface(
  LPDDRAWI_DIRECTDRAW_GBL lpDD,  // [IN] Ddraw VidMemFree may need this
  DWORD fpVidMem,                // [IN] Surface lfb start address
  DWORD hwVidMem,                // [IN] Surface hw video mem address
  DWORD dwCaps )                 // [IN] Surface flags (local/nonlocal)
{
  int result;
  VIDMEMFREE_PARMS VMFparms;

  DD_ENTRY_SETUP(lpDD)

#ifdef AGP_EXECUTE
  if ( dwCaps & DDSCAPS_NONLOCALVIDMEM  )
  {
    DDHAL32_VidMemFree(lpDD, AGP_HEAP_ID, fpVidMem );
    return;
  }
#endif

  VMFparms.memPool=TRANSIENT;

  fpVidMem = hwVidMem + _FF(LFBBASE); //In many cases only valid hwVidMem passed

  if ( pVidMemFree )
  {
    VMFparms.ppdev=ppdev;
    VMFparms.fpVidMem = fpVidMem;
    VMFparms.hwVidMem = hwVidMem;
    result = (*pVidMemFree)( &VMFparms );    //this function calls "mmReclaim32" for us.

    if (!result && (dwCaps & (DDSCAPS_ZBUFFER | DDSCAPS_3DDEVICE)))
    {
      //zbuffers and color buffers could be in either TRANSIENT or RECLAMATION space.
      VMFparms.memPool = RECLAIMED;
      (*pVidMemFree)( &VMFparms );    //this function calls "mmReclaim32" for us.
    }
  }
  else
  {
    //Points to LINEAR_HEAP_ID
    LPVIDMEM pvmHeap = _FF(HALInfo).vmiData.pvmList;
    VidMemFree( pvmHeap->lpHeap, fpVidMem );

    //Tell 16 bit memmgr that the area can be re-used
    mmReclaim32( ppdev, hwVidMem );
  }  
}



/*----------------------------------------------------------------------
Function name: phantom_allocSurface

Description:   Alloc space in "phantom" memory region.
               Since host address regions accessed via the CAM must have a
               power of 2 pitch (mStride), a different view of memory is
               required than the actual "physical" address range.
               The phantom memory is managed using the same DDraw memory manager
               routines that are used for physical memory.  However, when
               called for phantom management, the size may have been increased
               to accomodate the pitch being rounded up to the next power of 2.

Return:        DD_OK
               DDERR_INVALIDPARAMS
               DDERR_OUTOFVIDEOMEMORY

----------------------------------------------------------------------*/

DWORD phantom_allocSurface(
    GLOBALDATA * ppdev,              // [IN]  Device Data
    FxU32 phantom_blksize,          // [IN]  size of region requested - in bytes
    DWORD *fpVidMem)                // [OUT] host lfb start address of allocation
{
  if (!phantom_blksize)
    return DDERR_INVALIDPARAMS;

  if (!fxmm_allocateblock(&_FF(mmPhantomHeap), fpVidMem, phantom_blksize, 32, 0, 0, 0))
    return DDERR_OUTOFVIDEOMEMORY;

  return DD_OK;
}

/*----------------------------------------------------------------------
Function name: phantom_freeSurface

Description:   Free space in "phantom" memory region
               [see Description in phantom_allocSurface for background on "phantom" region]

Return:        void 
----------------------------------------------------------------------*/
void phantom_freeSurface(
  GLOBALDATA * ppdev,            // [IN] Global Device Data
  DWORD fpVidMem)                // [IN] Surface lfb start address
{
  fxmm_freeblock(&_FF(mmPhantomHeap), fpVidMem);
}

/*----------------------------------------------------------------------
Function name: memMgr_initLists

Description:   Select Rampage or DDraw memory manager to use
               Initialize memory manager variables the free node list
               _DD(memFree), create the initial free node, and 
               set the used node list _DD(memConsumed) to 0

Return:        BOOL

               TRUE  - 
               FALSE - 
----------------------------------------------------------------------*/
BOOL memMgr_initLists(GLOBALDATA *ppdev)
{
  BOOL result = TRUE;
  FXMM_PHEAPINFO pLendingHeap;
  FXMM_LENDFUNCTION pLendingFunction;

  #if defined(AGP_CMDFIFO)
    pLendingHeap = NULL;      //don't need to lend persistent to transient when we have an AGP fifo.
    pLendingFunction = NULL;  //don't need to lend persistent to transient when we have an AGP fifo.
  #else
    pLendingHeap = &_FF(mmPersistentHeap);
    pLendingFunction = fxmm_sst2_persistentlender;
  #endif

  // These two assignments were inside the 'else' clause below.  Moving them
  // here allows the 32-bit DD/D3D driver to be reloaded on the fly, averting
  // a full reboot.  The proper fix is to re-initialize variables at PROCESS_ATTACH
  // time to trigger these being set.
  pVidMemAlloc = memMgr_VidMemAlloc;  //Use rampage memory manager
  pVidMemFree  = memMgr_VidMemFree;   //Use rampage memory manager

  if (_FF(mmHeapsInitialized))
  {
    //we've already init'd the heaps, so this must be the result
    //of a change in the primary's size, or something.
    //reset all heaps except AGP and persistent.
    fxmm_resetheap(&_FF(mmPhantomHeap),     _FF(mmPhantomHeapStart)     + _FF(LFBBASE), _FF(mmPhantomHeapSize));
    fxmm_resetheap(&_FF(mmTransientHeap),   _FF(mmTransientHeapStart)   + _FF(LFBBASE), _FF(mmTransientHeapSize));
    fxmm_resetheap(&_FF(mmReclamationHeap), _FF(mmReclamationHeapStart) + _FF(LFBBASE), _FF(mmReclamationHeapSize));
  }
  else
  {
    result &= fxmm_createheap(FXMM_NAME_PERSISTENTHEAP,                   //visual identification string (debug)
                              &_FF(mmPersistentHeap),                     //the 'this' pointer for this heap
                              (FXMM_MALLOCHANDLE)ppdev,                   //allocation context handle
                              _FF(mmPersistentHeapStart) + _FF(LFBBASE),  //offset of the start of heap, in bytes
                              _FF(mmPersistentHeapSize),                  //size of the heap in bytes
                              4,                                          //granularity of 4 bytes
                              NULL,                                       //doesn't borrow from other heaps
                              pLendingFunction,                           //lends to other heaps from its low-address end
                              NULL,                                       //uses the standard freespace function
                              fxmm_rightjustifiedsplit,                   //use the right-hand piece of a block first
                              fxmm_lastfitinsertion,                      //keep things sorted in last-fit order
                              NULL );                                     //no notification for this heap

    result &= fxmm_createheap(FXMM_NAME_PHANTOMHEAP,                  //visual identification string (debug)
                              &_FF(mmPhantomHeap),                    //the 'this' pointer for this heap
                              (FXMM_MALLOCHANDLE)ppdev,               //allocation context handle
                              _FF(mmPhantomHeapStart) + _FF(LFBBASE), //offset of the start of heap, in bytes
                              _FF(mmPhantomHeapSize),                 //size of the heap in bytes
                              32,                                     //granularity of 32 bytes
                              NULL,                                   //doesn't borrow from other heaps
                              NULL,                                   //uses the standard freespace function
                              NULL,                                   //doesn't loan itself to other heaps
                              fxmm_leftjustifiedsplit,                //use the left-hand piece of a block first
                              fxmm_bestfitinsertion,                  //keep things sorted in best-fit order
                              NULL );                                 //no notification for this heap

    result &= fxmm_createheap(FXMM_NAME_TRANSIENTHEAP,                  //previously known as PHYSICAL, LINEAR, or DIRECTDRAW
                              &_FF(mmTransientHeap),                    //the 'this' pointer for this heap
                              (FXMM_MALLOCHANDLE)ppdev,                 //allocation context handle
                              _FF(mmTransientHeapStart) + _FF(LFBBASE), //offset of the start of heap, in bytes
                              _FF(mmTransientHeapSize),                 //size of the heap in bytes
                              1024,                                     //granularity of 1024 bytes
                              pLendingHeap,                             //can borrow from the PERSISTENT heap
                              NULL,                                     //doesn't loan itself to other heaps
                              fxmm_sst2_reportfreememory,               //needs to account for borrowable space
                              fxmm_leftjustifiedsplit,                  //use the left-hand piece of a block first
                              fxmm_bestfitinsertion,                    //keep things sorted in last-fit order
                              fxmm_sst2_heapnotify );                   //notify when blocks are lost in this heap.

    result &= fxmm_createheap(FXMM_NAME_RECLAMATIONHEAP,                  //visual identification string (debug)
                              &_FF(mmReclamationHeap),                    //the 'this' pointer for this heap
                              (FXMM_MALLOCHANDLE)ppdev,                   //allocation context handle
                              _FF(mmReclamationHeapStart) + _FF(LFBBASE), //offset of the start of heap, in bytes
                              _FF(mmReclamationHeapSize),                 //size of the heap in bytes
                              1024,                                       //granularity of 1024 bytes
                              NULL,                                       //doesn't borrow from other heaps
                              NULL,                                       //doesn't loan itself to other heaps
                              NULL,                                       //uses the standard freespace function
                              fxmm_leftjustifiedsplit,                    //use the left-hand piece of a block first
                              fxmm_bestfitinsertion,                      //keep things sorted in best-fit order
                              fxmm_sst2_heapnotify );                     //notify when blocks are lost in this heap.
  }

  return _FF(mmHeapsInitialized) = !!result;
}


/*----------------------------------------------------------------------
Function name: memMgr_GetFreeMemSize

Description:   report total free video memory size

Return:        DWORD
----------------------------------------------------------------------*/
DWORD memMgr_GetFreeMemSize(LPDDRAWI_DIRECTDRAW_GBL lpDD)
{
  FxU32 size;
  DD_ENTRY_SETUP(lpDD);

  //RYAN@COMMENT, Any loanable space in the PERSISTENT heap
  //is included by the call to fxmm_reportfreememory(TRANSIENT)
  size  = fxmm_reportfreememory(&_FF(mmTransientHeap))
        + fxmm_reportfreememory(&_FF(mmReclamationHeap));

  return size;
}



/*----------------------------------------------------------------------
Function name: memMgr_VidMemAlloc

Description:   Rampage linear memory allocator
               Find a memory block for this request.
               return hardware video memory address.

Return:        DWORD 
----------------------------------------------------------------------*/


static DWORD memMgr_VidMemAlloc(VIDMEMALLOC_PARMS *pVMA)
{
  FxU32 log2;
  FxU32 align;    //LCM of alignments we favor
  FxU32 malign;   //RYAN@PUN, an alignment we avoid with extreme prejudice.
  FxU32 offset;
  FxU32 linAddress;
  GLOBALDATA *ppdev = pVMA->ppdev;
  FXMM_PHEAPINFO heaplist[] = { &_FF(mmTransientHeap), &_FF(mmPhantomHeap), &_FF(mmReclamationHeap) };

  ///////////////////////// = { 32  256      4K      8K   EVEN8K    ODD8K      16K  EVEN16K   ODD16K                      };
  FxU32 listAlignment[16]   = { 32, 256, 4*1024, 8*1024, 16*1024,  8*1024, 16*1024, 16*1024, 16*1024, 0, 0, 0, 0, 0, 0, 0 };
  FxU32 listMalignment[16]  = {  0,   0,      0,      0,       0, 16*1024,       0,       0, 32*1024, 0, 0, 0, 0, 0, 0, 0 };

  if (pVMA->memPool >= NONE)  //insurance
    return 0;

  //convert the MEM_AT_XXXXXXX value into both an 'align' and 'malign' values.
  switch (pVMA->dwMemFlag & MEM_ALIGN_MASK)
  {
    case MEM_AT_LNR_MASK:
      align = pVMA->dwAddrMask+1;  //convert mask to a multiple
      malign=0;
      break;

    default:
      //find log2(dwMemFlag), and use it as index into the tables
      for (log2=0; !(1 & ((pVMA->dwMemFlag >> 16) >> log2)); log2++);

      align = listAlignment[log2];
      malign = listMalignment[log2];
      break;
  }

  if (!align)
  {
#ifdef DEBUG
    __asm int 3   //oops!  perhaps the above tables need to be updated.
#endif
    align = 1;
    malign = 0;
  }

  if (!fxmm_allocateblock(heaplist[pVMA->memPool], &linAddress, pVMA->dwSize, align, malign, 0, 0))
    return 0;

  //Tell 16 bit memmgr to free the area starting at linAddress, ending at linAddress+dwSize-1,
  //and the size dwSize
  offset = linAddress - _FF(LFBBASE);  //mmEvict requires hw address of video memory

  switch (pVMA->memPool)
  {
    case TRANSIENT:
    case RECLAIMED:
      mmEvict32(ppdev, offset, (offset + pVMA->dwSize-1), pVMA->dwSize);
      break;

    default:
      break;
  }

#ifdef DEBUG
  fxmm_heapwatchdog(heaplist[pVMA->memPool]);
#endif

  return linAddress;
}



/*----------------------------------------------------------------------
Function name: ddHal_VidMemAlloc

Description:   DDHAL linear memory allocator
               Find a memory block for this request.
               return hardware video memory address.

Return:        DWORD 
----------------------------------------------------------------------*/
static DWORD ddHal_VidMemAlloc(VIDMEMALLOC_PARMS *pVMA)
{
   FLATPTR lfptr;
   SURFACEALIGNMENT Alignment;
   LPVIDMEM pvmHeap;
   DWORD width, height, dwNewPitch;
   DWORD hwPtr;
   GLOBALDATA *ppdev=pVMA->ppdev;
   
   memset(&Alignment, 0, sizeof(Alignment));
   Alignment.Linear.dwPitchAlignment = 1;
   
   //Find the alignment requirement
   switch ( pVMA->dwMemFlag )
   {
      case MEM_AT_16K:
         Alignment.Linear.dwStartAlignment = 0x4000L;
         break;
         
      case MEM_AT_8K:
         Alignment.Linear.dwStartAlignment = 0x2000L;
         break;
         
      case MEM_AT_4K:
         Alignment.Linear.dwStartAlignment = 0x1000L;
         break;
         
      case MEM_AT_256:
         Alignment.Linear.dwStartAlignment = 256;
         break;
         
      case MEM_AT_32:
         Alignment.Linear.dwStartAlignment = 32;
         break;
      
      case MEM_AT_LNR_MASK:      
         Alignment.Linear.dwStartAlignment = pVMA->dwAddrMask+1;
         break;
         
      //DDHAL mem allocator does not support ODD/EVEN 
      //alignment, so just failed it 
      case MEM_AT_EVEN16K:
      case MEM_AT_ODD16K:
      case MEM_AT_EVEN8K:
      case MEM_AT_ODD8K:
      
      default:
         return 0;
   }   
   
   pvmHeap = _FF(HALInfo).vmiData.pvmList;   //Points to LINEAR_HEAP_ID
   width   = *pVMA->lpPitch;
   height  = pVMA->dwSize/width;
   
   if ( lfptr = HeapVidMemAllocAligned(pvmHeap, width, height, &Alignment, &dwNewPitch) )
   {
      //Tell 16 bit memmgr to free the area starting at lfptr, ending at lfptr+dwSize-1,
      //and the size dwSize
      
      hwPtr = lfptr - _FF(LFBBASE);  //mmEvict requires hw address of video memory
      
      mmEvict32(pVMA->ppdev, hwPtr, (hwPtr + pVMA->dwSize-1) , pVMA->dwSize);
      
      *pVMA->lpPitch = dwNewPitch;
            
      return lfptr;
   }
   
   return 0;
}

/*----------------------------------------------------------------------
Function name: memMgr_VidMemFree

Description:   Free this video memory

Return:        void 
----------------------------------------------------------------------*/
static int memMgr_VidMemFree( VIDMEMFREE_PARMS *pVMF )
{
  int result;
  GLOBALDATA *ppdev = pVMF->ppdev;
  FXMM_PHEAPINFO heaplist[] = { &_FF(mmTransientHeap), &_FF(mmPhantomHeap), &_FF(mmReclamationHeap) };

  if (pVMF->memPool < NONE)  //insurance
    if (result = fxmm_freeblock(heaplist[pVMF->memPool], pVMF->fpVidMem))
    {
      switch (pVMF->memPool)
      {
        case TRANSIENT:
        case RECLAIMED:
          mmReclaim32(ppdev, pVMF->hwVidMem); //tell 16 bit memmgr that the area can be re-used
          break;

        default:
          break;
      }
    }

#ifdef DEBUG
  fxmm_heapwatchdog(heaplist[pVMF->memPool]);
#endif

  return result;
}



#ifdef SLI
/*----------------------------------------------------------------------
Function name: AddSLISpace

Description:   Function for adding SLI space when
 entering SLI mode.  The SLI space is the space freed
 when we enter SLI mode.  It is the amount of space between
 the new end of the Primary GDI surface and _FF(mmTransientHeapStart).
 It is keep in a seperate list to ensure that not just anything is allocated
 but only surfaces that we will destroy when exiting SLI mode.  I consider
 these surfaces to be BackBuffers and Z-Buffers since we will be distributed 
 them across the Devices anyway.

Return:        
----------------------------------------------------------------------*/
void AddSLISpace(NT9XDEVICEDATA * ppdev, DWORD dwSize)
{
  _FF(mmReclamationHeapSize)  += dwSize;
  _FF(mmReclamationHeapStart) -= dwSize;

  //grow the reclamation heap to include this new space.  (assume it's consecutive)
  fxmm_resizeheap(&_FF(mmReclamationHeap), _FF(mmReclamationHeapStart)+_FF(LFBBASE), _FF(mmReclamationHeapSize));

/************
  //EXPERIMENTAL: resize the phantom heap to compensate
  _FF(mmPhantomHeapSize) += _FF(dwNumUnits)*dwSize*2;
  _FF(mmPhantomHeapStart) = _FF(dwNumUnits)*_FF(mmReclamationHeapStart)*2;
  fxmm_resizeheap(&_FF(mmPhantomHeap), _FF(mmPhantomHeapStart)+_FF(LFBBASE), _FF(mmPhantomHeapSize));
************/

  _DD(dwSLIStartAddress) = _FF(mmReclamationHeapStart);
  _DD(dwSLISize) = dwSize;
}



/*----------------------------------------------------------------------
Function name: RemoveSLISpace

Description:   Function for removing SLI space added when
 entering SLI mode.  The GDI Primary Surface is now going to reclaim the
 area between the old SLI end of the GDI Primary Surface and _FF(mmTransientHeapStart).
 Since we have only allocated surfaces here that are trashable we trash them
 when we exit SLI space.  This allows us not to worry about having to reconstruct these
 surfaces in a non-distributed manner.

Return:        
----------------------------------------------------------------------*/
void RemoveSLISpace(NT9XDEVICEDATA * ppdev, DWORD dwSize)
{
  _FF(mmReclamationHeapSize) -= dwSize;
  _FF(mmReclamationHeapStart) += dwSize;

  //shrink the reclamation heap to remove this space.
  fxmm_resizeheap(&_FF(mmReclamationHeap), _FF(mmReclamationHeapStart)+_FF(LFBBASE), _FF(mmReclamationHeapSize));

/************
  //EXPERIMENTAL: resize the phantom heap to compensate
  _FF(mmPhantomHeapSize) -= _FF(dwNumUnits)*dwSize*2;
  _FF(mmPhantomHeapStart) = _FF(dwNumUnits)*_FF(mmReclamationHeapStart)*2;
  fxmm_resizeheap(&_FF(mmPhantomHeap), _FF(mmPhantomHeapStart)+_FF(LFBBASE), _FF(mmPhantomHeapSize));
************/

  _DD(dwSLIStartAddress) = 0x0;
  _DD(dwSLISize) = 0x0;
}
#endif

#ifdef DEBUG

/*----------------------------------------------------------------------
Function name: mmWatchDog

Description:   Function for monitoring and debugging 

Return:        BOOL
               TRUE - fails integrity check
----------------------------------------------------------------------*/

BOOL mmWatchDog(GLOBALDATA *ppdev)
{
  BOOL result=TRUE;
  
  result &= fxmm_heapwatchdog(&_FF(mmReclamationHeap));
  result &= fxmm_heapwatchdog(&_FF(mmPhantomHeap));
  result &= fxmm_heapwatchdog(&_FF(mmTransientHeap));
  result &= fxmm_heapwatchdog(&_FF(mmPersistentHeap));

  return result;
}

#endif



#if defined(CSERVICE) //[

__declspec(dllexport) FxU32 cs9x_SurfaceAllocator(GLOBALDATA *ppdev, FxU32 align, FxU32 malign, FxU32 size, FxU32* phstride, FxU32 locale, FxU32 memtype, FxU32 bufftype, FxU32* poffset, FxU32 owner, FxBool biggest)
{
  FxU32 linAddress=0;

/*
#ifdef SLI
  FXMM_PHEAPINFO pheap;
#endif
*/

  FxU32 avail1, avail2;
  FXMM_PHEAPINFO pHeap1, pHeap2;

#ifdef AGP_EXECUTE
  LPVIDMEM pvmHeap;
  SURFACEALIGNMENT agpAlignment;
#endif


  switch (locale)
  {
    case CS_LOCALE_AGP:
#ifdef AGP_EXECUTE
      //RYAN: since DDraw has no concept of EVEN/ODD alignment, we have to ignore the "malign" parameter.
      //RYAN: ditto for our new "biggest" parameter.
      memset(&agpAlignment, 0, sizeof(agpAlignment));
      agpAlignment.Linear.dwStartAlignment = align;
      agpAlignment.Linear.dwPitchAlignment = *phstride;
      pvmHeap = ((LPVIDMEM)_FF(csDirectDrawHeaps))+1;
      if (linAddress = HeapVidMemAllocAligned(pvmHeap, size, 1, &agpAlignment, phstride))
      {
        //convert linear address to gart physical address (client will lock to obtain linear address)
        *poffset = linAddress - _FF(agpHeapLinBaseAddr) + _FF(agpHeapPhysBaseAddr);
        return CS_SUCCESS;
      }
      return CS_APIERROR_DIRECTDRAWFAILED;
#else
      return CS_APIERROR_OUTOFMEMORY;
#endif

    case CS_LOCALE_FRAMEBUFFER:
      switch (bufftype)
      {
        case CS_BUFFER_FIFO:
        case CS_BUFFER_PERSISTENT:
          if (biggest)
            size = __min(size, fxmm_largestfreeblock(&_FF(mmPersistentHeap), size, align, malign));

          if (!size)
            return CS_APIERROR_OUTOFMEMORY; //usually means we can't meet the (m)alignment requirements.

          if (!fxmm_allocateblock(&_FF(mmPersistentHeap), &linAddress, size, align, malign, 0, owner)) 
            return CS_APIERROR_OUTOFMEMORY;
          break;
       
       
        case CS_BUFFER_RENDER:
        case CS_BUFFER_ZBUFFER:
        case CS_BUFFER_TEXTURE:    
        case CS_BUFFER_TEXTUREHEAP:
          pHeap1 = &_FF(mmReclamationHeap);
          pHeap2 = &_FF(mmTransientHeap);

          if (biggest)
          {
            avail1 = pHeap1 ? fxmm_largestfreeblock(pHeap1, size, align, malign) : 0;
            avail2 = pHeap2 ? fxmm_largestfreeblock(pHeap2, size, align, malign) : 0;

            if (!avail1 && !avail2)
              return CS_APIERROR_OUTOFMEMORY; //usually means we can't meet the (m)alignment requirements.

            if (avail1 >= size)       //no need to search further
              pHeap2 = NULL;
            else if (avail2 >= size)  //no need to search further
              pHeap1 = NULL;
            else if (avail1 > avail2) //pHeap1 is better
            {
              size = avail1;
              pHeap2 = NULL;
            }
            else if (avail2 > avail1) //pHeap2 is better
            {
              size = avail2;
              pHeap1 = NULL;
            }
          }

          if (!pHeap1 || !fxmm_allocateblock(pHeap1, &linAddress, size, align, malign, 0, owner))
            if (!pHeap2 || !fxmm_allocateblock(pHeap2, &linAddress, size, align, malign, 0, owner))
              return CS_APIERROR_OUTOFMEMORY;

          break;

        default: return CS_APIERROR_INVALIDPARAM;
      }

      if (!linAddress) 
        DEBUGINT3;

      *poffset = linAddress - _FF(LFBBASE);  //mmEvict requires hw address of video memory
      mmEvict32(ppdev, *poffset, (*poffset+size-1), size); //tell GDI memmgr to vacate this area
      break;
  }

  return CS_SUCCESS;
}



__declspec(dllexport) FxU32 cs9x_SurfaceLiberator(GLOBALDATA *ppdev, FxU32 locale, FxU32 bufftype, FxU32 offset)
{
  int result=1;
  FxU32 linAddress;
  LPVIDMEM pvmHeap;

  switch (locale)
  {
    case CS_LOCALE_AGP:
#ifdef AGP_EXECUTE
      linAddress = offset + _FF(agpHeapLinBaseAddr);   //convert AGP phys address to linear address
      pvmHeap = ((LPVIDMEM)_FF(csDirectDrawHeaps))+1;
      VidMemFree(pvmHeap->lpHeap, linAddress);
      break;
#else
      return CS_APIERROR_IMPOSSIBILITY;
#endif

    case CS_LOCALE_FRAMEBUFFER:
      switch (bufftype)
      {
        case CS_BUFFER_FIFO:
        case CS_BUFFER_PERSISTENT:
          linAddress = offset + _FF(LFBBASE);   //convert FB offset to linear address
          result &= fxmm_freeblock(&_FF(mmPersistentHeap), linAddress);
          break;

        case CS_BUFFER_RENDER:
        case CS_BUFFER_ZBUFFER:
        case CS_BUFFER_TEXTURE:
          mmReclaim32(ppdev, offset);
          linAddress = offset + _FF(LFBBASE);   //convert FB offset to linear address

#ifdef SLI  //[
          if ((SLI_MODE_ENABLED == _DD(sliMode)) && fxmm_heapcontains(&_FF(mmReclamationHeap), linAddress))
            result &= fxmm_freeblock(&_FF(mmReclamationHeap), linAddress);
          else
#endif //] SLI
            result &= fxmm_freeblock(&_FF(mmTransientHeap), linAddress);
          break;
          
        default: return CS_APIERROR_INVALIDPARAM;
      }
      break;

    default:
      return CS_APIERROR_INVALIDPARAM;
  }

  return (result ? CS_SUCCESS : CSFREE_APIERROR_ALREADYFREE);
}



#endif  //] defined(CSERVICE) 
