/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
**  30   3dfx      1.29        10/23/00 Brent Burton    More DX8-specific 
**       changes.
**  29   3dfx      1.28        10/10/00 Evan Leland     now set flag in 
**       FXSURFACDATA when the surface is a driver-managed VB
**  28   3dfx      1.27        10/5/00  Michel Conrad   Remove #ifdef cubemap. 
**  27   3dfx      1.26        10/4/00  Dale  Kenaston  New Sage macros. Changed
**       IS_RAGE in CreateExecuteBuffer32 to IS_SAGE_ACTIVE.
**  26   3dfx      1.25        9/21/00  Evan Leland     added support for 
**       swapping vertex buffers
**  25   3dfx      1.24        9/6/00   Michel Conrad   Added labels needed and 
**       variables needed for cubemapping.
**  24   3dfx      1.23        8/10/00  Evan Leland     Adds support for 
**       explicit vertex buffers; removes the VERT_BUFF compile option.
**  23   3dfx      1.22        7/24/00  Brian Danielson Changes to implement 
**       renderstate and shadow register management.
**  22   3dfx      1.21        7/10/00  Michel Conrad   Add cubemap support.
**  21   3dfx      1.20        6/16/00  Evan Leland     Made VERT_BUFF a 
**       separate compile option that is not tied to the HW_R3 compile option; 
**       fixed the vertex buffer code in d7d3d.c so that it would compile.
**  20   3dfx      1.19        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  19   3dfx      1.18        5/11/00  Evan Leland     dx7 structure cleanup 
**       effort complete
**  18   3dfx      1.17        4/24/00  Evan Leland     changes to track removal
**       of dd surf structure pointer in the TXHNDLSTRUCT which lives in 
**       d3txtr2.h
**  17   3dfx      1.16        4/24/00  Evan Leland     preliminary changes, 
**       getting ready to removing all ddraw structure pointers stored in the 
**       driver
**  16   3dfx      1.15        4/12/00  Evan Leland     Moved code in 
**       ddiCreateSurfaceEx that initializes the txtrDesc structure into a 
**       routine that now lives over in the texture module, called 
**       txtrSurfaceCreateEx.
**  15   3dfx      1.14        2/7/00   Evan Leland     minor change to 
**       ddiCreateSurfaceEx to create a local instance of a TXHNDLSTRUCT for 
**       purposes of assigning a resource context value to its contextID field.
**  14   3dfx      1.13        2/4/00   Evan Leland     changed a txtrDesc 
**       structure name
**  13   3dfx      1.12        1/28/00  Evan Leland     modified 
**       ddiCreateSurfaceEx to support system memory textures correctly
**  12   3dfx      1.11        1/28/00  Evan Leland     more dx7 upgrade work
**  11   3dfx      1.10        1/27/00  Evan Leland     DX7 changes
**  10   3dfx      1.9         1/25/00  Evan Leland     updates for dx7
**  9    3dfx      1.8         12/17/99 Brian Danielson Compiler error fixups 
**       for Brent.
**  8    3dfx      1.7         12/17/99 Brent Burton    Removed all WINNT macro 
**       checks.  This removed some unused code.
**  7    3dfx      1.6         11/5/99  Andrew Sobczyk  Changed phantom_alloc 
**       and phantom_free parameters
**  6    3dfx      1.5         10/12/99 Philip Zheng    Added vertex buffer 
**       alloc/free 
**  5    3dfx      1.4         10/8/99  Philip Zheng    Make ddiCreateSurfaceEx 
**       accept DDSCAPS_EXECUTEBUFFER
**  4    3dfx      1.3         10/7/99  Philip Zheng    Added vertex buffer 
**       support.  Only affects R3.   The compiler flag is VERT_BUFF
**  3    3dfx      1.2         9/30/99  Philip Zheng    Implemented the work 
**       around for DX 7 multiple buffer problem
**  2    3dfx      1.1         9/29/99  Philip Zheng    Fixed compiler error
**  1    3dfx      1.0         9/13/99  Philip Zheng    
** $Revision: 11$
** $Date: 10/23/00 2:11:00 PM PDT$
**
*/

/**************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* ddiGetDriverState    --- DX7 GetDriverState entry point
* ddiCreateSurfaceEx   --- DX7 CreateSurfaceEx entry point
* ddiDestroyDDLocal    --- DX7 DestroyDDLocal entry point
*
* GetHndlListPtr
*
* INTERNAL FUNCTIONS:
*
* AllocHndlList
* ReleaseHndlList
* GetTxtrHndl
* AllocTxtrHndl
* ReleaseTxtrHndl
*
***************************************************************************/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"
#include "ddcam.h"      // cam management routines

#include "d3txtr.h"
#include "d3contxt.h"
#include "ddglobal.h"

#if HL_LINKED_LIST
#define g_pHndlList   _D3(pHndlList)
#endif

/**************************************************************************
* D E F I N E S
***************************************************************************/

#define ENTRY_EXIT_DBG_LEVEL    2
#define NORMAL_DBG_LEVEL        2

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

static HNDLLIST *AllocHndlList(NT9XDEVICEDATA*, LPVOID);
static VOID ReleaseHndlList(NT9XDEVICEDATA*, HNDLLIST*);
static DWORD GetTxtrHndl(NT9XDEVICEDATA*,LPVOID,HNDLLIST*,DWORD);
static DWORD AllocTxtrHndl(NT9XDEVICEDATA*, LPVOID, HNDLLIST*, DWORD);
static VOID ReleaseTxtrHndl(NT9XDEVICEDATA *ppdev, HNDLLIST *pHndlList, DWORD dwTxtrHndl);

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

/*----------------------------------------------------------------------
Function name: ddiGetDriverState

Description:   DX7 Callback GetDriverState()

   This callback is used by both the DirectDraw and Direct3D runtimes to obtain
   information from the driver about its current state.

   Parameters

       pgdsd
             pointer to GetDriverState data structure

             dwFlags
                     Flags to indicate the data required
             dwhContext
                     The ID of the context for which information
                     is being requested
             lpdwStates
                     Pointer to the state data to be filled in by the driver
             dwLength
                     Length of the state data buffer to be filled
                     in by the driver
             ddRVal
                     Return value

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
ddiGetDriverState( LPDDHAL_GETDRIVERSTATEDATA pgdsd )
{
  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,">> ddiGetDriverState");

  pgdsd->ddRVal = DD_OK;

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,"<< ddiGetDriverState");

  return DDHAL_DRIVER_HANDLED;
} // ddiGetDriverState

/*----------------------------------------------------------------------
Function name: ddiCreateSurfaceEx

Description:   DX7 Callback CreateSurfaceEx()

   ddiCreateSurfaceEx creates a Direct3D surface from a DirectDraw surface and
   associates a requested handle value to it.

   All Direct3D drivers must support ddiCreateSurfaceEx.

   ddiCreateSurfaceEx creates an association between a DirectDraw surface and
   a small integer surface handle. By creating these associations between a
   handle and a DirectDraw surface, ddiCreateSurfaceEx allows a surface handle
   to be imbedded in the Direct3D command stream. For example when the
   D3DDP2OP_TEXBLT command token is sent to ddiDrawPrimitives2 to load a texture
   map, it uses a source handle and destination handle which were associated
    with a DirectDraw surface through ddiCreateSurfaceEx.

   For every DirectDraw surface created under the local DirectDraw object, the
   runtime generates a valid handle that uniquely identifies the surface and
   places it in pcsxd->lpDDSLcl->lpSurfMore->dwSurfaceHandle. This handle value
   is also used with the D3DRENDERSTATE_TEXTUREHANDLE render state to enable
   texturing, and with the D3DDP2OP_SETRENDERTARGET and D3DDP2OP_CLEAR commands
   to set and/or clear new rendering and depth buffers. The driver should fail
   the call and return DDHAL_DRIVER_HANDLE if it cannot create the Direct3D
   surface. If the DDHAL_CREATESURFACEEX_SWAPHANDLES flag is set, the handles
   should be swapped over two sequential calls to ddiCreateSurfaceEx.
   As appropriate, the driver should also store any surface-related information
   that it will subsequently need when using the surface. The driver must create
   a new surface table for each new lpDDLcl and implicitly grow the table when
   necessary to accommodate more surfaces. Typically this is done with an
   exponential growth algorithm so that you don't have to grow the table too
   often. Direct3D calls ddiCreateSurfaceEx after the surface is created by
   DirectDraw by request of the Direct3D runtime or the application.

   Parameters

        pcsxd
             pointer to CreateSurfaceEx structure that contains the information
             required for the driver to create the surface (described below).

             dwFlags
                     May have the value(s):
                     DDHAL_CREATESURFACEEX_SWAPHANDLES
                                If this flag is set, ddiCreateSurfaceEx will be
                                called twice, with different values in lpDDSLcl
                                in order to swap the associated texture handles
             lpDDLcl
                     Handle to the DirectDraw object created by the application.
                     This is the scope within which the lpDDSLcl handles exist.
                     A DD_DIRECTDRAW_LOCAL structure describes the driver.
             lpDDSLcl
                     Handle to the DirectDraw surface we are being asked to
                     create for Direct3D. These handles are unique within each
                     different DD_DIRECTDRAW_LOCAL. A DD_SURFACE_LOCAL structure
                     represents the created surface object.
             ddRVal
                     Specifies the location in which the driver writes the return
                     value of the ddiCreateSurfaceEx callback. A return code of
                     DD_OK indicates success.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
ddiCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA pcsxd )
{
  NT9XDEVICEDATA              *ppdev;
  LPVOID                      pDDLcl  = (LPVOID)pcsxd->lpDDLcl;
  LPDDRAWI_DDRAWSURFACE_LCL   pDDSLcl = pcsxd->lpDDSLcl;
  LPATTACHLIST                curr;
  HNDLLIST                    *pHndlList;
  DWORD                       dwTxtrHndl;
  TXHNDLSTRUCT                *thstruct;
  txtrDesc                    *pTxtrDesc;
  FxU32                       rc;

  LPDDRAWI_DDRAWSURFACE_LCL   m_pDDSLcl[6];
  LPDDRAWI_DDRAWSURFACE_LCL   pDDSNextLcl;
  HRESULT                     hr;
  DWORD                       facesDetected=1;
  DWORD                       i;


  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,">> ddiCreateSurfaceEx");

  pcsxd->ddRVal = DD_OK;

  if (NULL == pDDSLcl || NULL == pDDLcl)
  {
    D3DPRINT(0,"ddiCreateSurfaceEx received NULL pDDLcl or pDDSLcl pointer");
    return DDHAL_DRIVER_HANDLED;
  }

  // We check that what we are handling is a texture, zbuffer or a rendering
  // target buffer. We don't check if it is however stored in local video
  // memory since it might also be a system memory texture that we will later
  // blt with __TextureBlt.
  // also if your driver supports DDSCAPS_EXECUTEBUFFER create itself, it must
  // process DDSCAPS_EXECUTEBUFFER here as well.

  if (!(pDDSLcl->ddsCaps.dwCaps & (DDSCAPS_TEXTURE  |
                                   DDSCAPS_3DDEVICE |
                                   DDSCAPS_ZBUFFER  |
                                   DDSCAPS_EXECUTEBUFFER)))
  {
    D3DPRINT(NORMAL_DBG_LEVEL,"ddiCreateSurfaceEx w/o "
                              "DDSCAPS_TEXTURE/3DDEVICE/ZBUFFER Ignored, "
                              "dwCaps=%8lXh dwSurfaceHandle=%ld",
             pDDSLcl->ddsCaps.dwCaps,
             pDDSLcl->lpSurfMore->dwSurfaceHandle);

    return DDHAL_DRIVER_HANDLED;
  }

  D3DPRINT(255, "CreateSurfaceEx: surface type: Tex %x 3d %x Zb %x Ex %x",
           pDDSLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE,
           pDDSLcl->ddsCaps.dwCaps & DDSCAPS_3DDEVICE,
           pDDSLcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER,
           pDDSLcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER);

  if (pDDSLcl->lpGbl->fpVidMem == 0)
    return DDHAL_DRIVER_HANDLED;

  ppdev = (NT9XDEVICEDATA *)pcsxd->lpDDLcl->lpGbl->dwReserved3;

  memset( &m_pDDSLcl[0], 0, sizeof(LPDDRAWI_DDRAWSURFACE_LCL)*6);

   // Obtain pionters to all top level surfaces into array

  if (pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP)
  { 
      DDSCAPS2 ddscaps;
      memset( &ddscaps, 0, sizeof(ddscaps));
      ddscaps.dwCaps = DDSCAPS_TEXTURE;

      if (!(pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEX))
      {
         ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX;
         hr = myDDGetAttachedSurface( pDDSLcl, &ddscaps, &pDDSNextLcl );
         if ((hr != D3D_OK) && (hr != DDERR_NOTFOUND))
         {
            pcsxd->ddRVal = hr;
            goto exit_ddicsx;
         }
         if (hr == DDERR_NOTFOUND)
         {
            m_pDDSLcl[0] = NULL;
         }
         else
         {
            // use POSITIVEX surface to query others
            pDDSLcl = pDDSNextLcl;
            m_pDDSLcl[0] = pDDSLcl;
         }
      }
      else
      {
         m_pDDSLcl[0] = pDDSLcl;
      }

      // get the rest of the top level surfaces, in order
      for (i = 1; i < 6; i++)
      {
         switch(i) {
            case 1: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEX; break;
            case 2: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_POSITIVEY; break;
            case 3: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEY; break;
            case 4: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_POSITIVEZ; break;
            case 5: ddscaps.dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEZ; break;
         }
         ddscaps.dwCaps2 |= DDSCAPS2_CUBEMAP;
         hr = myDDGetAttachedSurface( pDDSLcl, &ddscaps, &pDDSNextLcl );
         if ((hr != D3D_OK) && (hr != DDERR_NOTFOUND))
         {
            pcsxd->ddRVal = hr;
            goto exit_ddicsx;
         }
         if (hr == DDERR_NOTFOUND)
         {
            m_pDDSLcl[i] = NULL;
         }
         else
         {
            m_pDDSLcl[i] = pDDSNextLcl;
            facesDetected++;
         }
      }
  }
  else 
  {
      m_pDDSLcl[0] = pDDSLcl;
      facesDetected = 1;
  }

  // Non cube map case treated as a single face surface

  for( i=0; i<facesDetected; i++)
  {
    pDDSLcl = m_pDDSLcl[i];

    // Now allocate the texture data space
    do
    {
      D3DPRINT(NORMAL_DBG_LEVEL, "pDDLcl = %8lXh, pDDSLcl = %8lXh, dwHandle = %ld, dwCaps = %08lXh",
               pDDLcl, pDDSLcl, pDDSLcl->lpSurfMore->dwSurfaceHandle, pDDSLcl->ddsCaps.dwCaps);
    
      if (0 == pDDSLcl->lpSurfMore->dwSurfaceHandle)
      {
        D3DPRINT(0,"ddiCreateSurfaceEx got 0 dwSurfaceHandle, dwCaps=%08lXh",
                 pDDSLcl->ddsCaps.dwCaps);
        break;
      }
    
      // find or allocate the HNDLLIST associated with this pDDLcl
      pHndlList = GetHndlListPtr(ppdev, pDDLcl);
      if (NULL == pHndlList)
      {
        // a HNDLLIST isn't already associated with the pDDLcl so create one now
        pHndlList = AllocHndlList(ppdev, pDDLcl);
        if (NULL == pHndlList)
        {
          D3DPRINT(0, "  unable to allocate a HndlList for pDDLcl=%8lXh",pDDLcl);
          pcsxd->ddRVal = DDERR_OUTOFMEMORY;
          break;
        }
      }
      D3DPRINT(NORMAL_DBG_LEVEL, "  pHndlList = %8lXh", pHndlList);
    
      // for DX7 do what ddiHandleCreate does here, except that we don't know
      // the context yet
    
      // find or allocate the TXTRHNDL for this pDDLcl-Handle pair
      dwTxtrHndl = GetTxtrHndl(ppdev, pDDLcl, pHndlList, pDDSLcl->lpSurfMore->dwSurfaceHandle);
      if (0 == dwTxtrHndl)
      {
        // a TXTRHNDL doesn't exist for this pDDLcl-Handle pair
        // so allocate one now
        dwTxtrHndl = AllocTxtrHndl(ppdev, pDDLcl, pHndlList, pDDSLcl->lpSurfMore->dwSurfaceHandle);
        if (0 == dwTxtrHndl)
        {
          D3DPRINT(0, "  unable to allocate a TXTRHNDL for handle %ld, pDDLcl = %8lXh",
                   pDDSLcl->lpSurfMore->dwSurfaceHandle, pDDLcl);
          pcsxd->ddRVal = DDERR_OUTOFMEMORY;
          break;
        }
      }

      thstruct = pHndlList->ppTxtrHndlList[dwTxtrHndl];
      thstruct->surfData = (FXSURFACEDATA*) pDDSLcl->lpGbl->dwReserved1;
    
      if (DDSCAPS_TEXTURE & pDDSLcl->ddsCaps.dwCaps)
      {
          rc = txtrSurfaceCreateEx(ppdev, pDDSLcl, thstruct, &pTxtrDesc);
          if (rc = FALSE) {
              pcsxd->ddRVal = DDERR_OUTOFMEMORY;
              return DDHAL_DRIVER_HANDLED;
          }
      }
    
#if (DIRECT3D_VERSION >= 0x0800)
      if (DDSCAPS_EXECUTEBUFFER & pDDSLcl->ddsCaps.dwCaps)
      {
#if 0
          // Per Refrast, there's no need to identify particular types of
          // buffers yet, but this is how:
          if (DDSCAPS2_VERTEXBUFFER & pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2) /*...*/;
          if (DDSCAPS2_INDEXBUFFER  & pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2) /*...*/;
#endif

          // create FXSURFACEDATA struct which defines this surface for the driver
          if (txtrSurfDataAlloc(ppdev, &thstruct->surfData, 0, 0, 0))
          {
              pcsxd->ddRVal = DDERR_OUTOFMEMORY;
              return DDHAL_DRIVER_HANDLED;
          }

          // store FXSURFACEDATA ptr in the globally reachable tx handle struct
          thstruct->surfData->dwFlags =
              (DDSCAPS2_INDEXBUFFER & pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2) ?
              FXSURFACE_IS_RTIB : FXSURFACE_IS_RTVB;
          thstruct->surfData->dwSurfLclFlags = pDDSLcl->dwFlags;
          thstruct->surfData->ddsDwCaps      = pDDSLcl->ddsCaps.dwCaps;
          thstruct->surfData->ddsDwCaps2     = pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2;
          // Since this is either a vertex or index buffer created by the runtime,
          // we can use these fields
          thstruct->surfData->lfbPtr    = pDDSLcl->lpGbl->fpVidMem;
          thstruct->surfData->endlfbPtr = pDDSLcl->lpGbl->fpVidMem + pDDSLcl->lpGbl->dwLinearSize;
          thstruct->surfData->dwWidth   = pDDSLcl->lpGbl->wWidth;
          thstruct->surfData->dwHeight  = pDDSLcl->lpGbl->wHeight;
          thstruct->surfData->dwPitch   =
          thstruct->surfData->dwStride  = pDDSLcl->lpGbl->lPitch;
      }
#endif // DX8

      // TEMP: I dont think we need this code!
      // update palette handle on texture swaps
      // if (pDDSLcl->dwReserved1 && (pTxtrDesc != NULL) && TXTR_IS_PALETTIZED(pTxtrDesc))
      // {
      //     if (pTxtrDesc->txPaletteHandle)
      //     {
      //       D3DPRINT(NORMAL_DBG_LEVEL, "  updating palette handle for TxtrHndl[%ld] from %ld to %ld",
      //                dwTxtrHndl, thstruct->dwPaletteHandle, pTxtrDesc->txPaletteHandle);
      //       thstruct->dwPaletteHandle = pTxtrDesc->txPaletteHandle;
      //     }
      // }

#if RC_LINKED_LIST
       // walk context linked list
       {
           RC  *pRc = (RC *)&g_pContexts;
       
           while (pRc->pNext)
           {
             pRc = pRc->pNext;
       
             // if the context pDDLcl is this calls pDDLcl
             // update the TXTRHNDL contextId
             if (pRc->pDDLcl == pDDLcl)
             {
               TXHNDLSTRUCT *ths = TXTRHNDL_TO_TXHNDLSTRUCT(pRc, dwTxtrHndl);
               ths->contextId = (DWORD)pRc;
               D3DPRINT(NORMAL_DBG_LEVEL, "  setting TxtrHndl[%ld] contextId to %8lXh",
                        dwTxtrHndl, pRc);
               break;
             }
           }
       }
#else
       {
           DWORD handle;
       
           // loop over context array
           for (handle = 0; handle < NUMCONTEXTS; handle++)
           {
               RC  *pRc = CONTEXT_INDEX(handle);
       
               // if the context is in use and the context pDDLcl is this calls pDDLcl
               // update the TXTRHNDL contextId
               if (CONTEXT_MAP(handle) && pRc->pDDLcl == pDDLcl)
               {
                   TXTRHNDL_PTR(dwTxtrHndl)->contextId = (DWORD)pRc;
                   D3DPRINT(NORMAL_DBG_LEVEL, "  setting TxtrHndl[%ld] contextId to %8lXh",
                              dwTxtrHndl, pRc);
                   break;
               }
            }
       }
#endif
       
       // for some surfaces other than MIPMAP or CUBEMAP, such as flipping chains,
       // we make a slot for every surface, as they are not as interleaved
       if ((DDSCAPS_MIPMAP & pDDSLcl->ddsCaps.dwCaps) ||
           (DDSCAPS2_CUBEMAP & pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2))
         break;
       curr = pDDSLcl->lpAttachList;
       if (NULL == curr)
         break;
       pDDSLcl = curr->lpAttached;
     } while ((NULL != pDDSLcl) && (pDDSLcl != pcsxd->lpDDSLcl));

  } // *** bottom of for loop on number of facesDetected ***

exit_ddicsx:

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,"<< ddiCreateSurfaceEx");
  return DDHAL_DRIVER_HANDLED;
} // ddiCreateSurfaceEx

/*----------------------------------------------------------------------
Function name: ddiDestroyDDLocal

Description:   DX7 Callback ddiDestroyDDLocal()

   ddiDestroyDDLocal destroys all the Direct3D surfaces previously created by
   ddiCreateSurfaceEx that belong to the same given local DirectDraw object.

   All Direct3D drivers must support ddiDestroyDDLocal.
   Direct3D calls ddiDestroyDDLocal when the application indicates that the
   Direct3D context is no longer required and it will be destroyed along with
   all surfaces associated to it. The association comes through the pointer to
   the local DirectDraw object. The driver must free any memory that the
   driver's ddiCreateSurfaceExDDK_ddiCreateSurfaceEx_GG callback allocated for
   each surface if necessary. The driver should not destroy the DirectDraw
   surfaces associated with these Direct3D surfaces; this is the application's
   responsibility.

   Parameters

        lpdddd
              Pointer to the DestoryLocalDD structure that contains the
              information required for the driver to destroy the surfaces.

              dwFlags
                    Currently unused
              pDDLcl
                    Pointer to the local Direct Draw object which serves as a
                    reference for all the D3D surfaces that have to be destroyed.
              ddRVal
                    Specifies the location in which the driver writes the return
                    value of ddiDestroyDDLocal. A return code of DD_OK indicates
                     success.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
               DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
ddiDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA pdddd )
{
  NT9XDEVICEDATA  *ppdev;
  LPVOID          pDDLcl  = (LPVOID)pdddd->pDDLcl;
  HNDLLIST        *pHndlList;
  DWORD           i;


  ppdev = (NT9XDEVICEDATA *)pdddd->pDDLcl->lpGbl->dwReserved3;

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,">> ddiDestroyDDLocal - ppdev %8lXh", ppdev);

  D3DPRINT(NORMAL_DBG_LEVEL, "pDDLcl = %8lXh", pDDLcl);

  pHndlList = GetHndlListPtr(ppdev, pDDLcl);

  if (NULL != pHndlList)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "pHndlList = %8lXh", pHndlList);

    // release any TXTRHNDL's
    if (NULL != pHndlList->ppTxtrHndlList)
    {
      // loop over the array and free any TXTRHNDL's that are still in use
      for (i = 1; i < (DWORD)pHndlList->ppTxtrHndlList[0]; i++)
        if (NULL != pHndlList->ppTxtrHndlList[i])
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "Releasing TXTRHNDL %ld  TXTRHNDL ptr=%8lXh",
                   i, pHndlList->ppTxtrHndlList[i]);
          ReleaseTxtrHndl(ppdev, pHndlList, i);
        }

      // now release the memory for the array
      D3DPRINT(NORMAL_DBG_LEVEL,
               "Releasing pHndlList[%X]->ppTxtrHndlList[%X] for pDDLcl %X",
               pHndlList, pHndlList->ppTxtrHndlList, pDDLcl);
      DXFREE(pHndlList->ppTxtrHndlList);
      pHndlList->ppTxtrHndlList = NULL;
    }

    // release any PALHNDL's
    if (NULL != pHndlList->ppPalHndlList)
    {
      // loop over the array and free any PALHNDL's that were allocated
      for (i = 1; i < (DWORD)pHndlList->ppPalHndlList[0]; i++)
        if (NULL != pHndlList->ppPalHndlList[i])
        {
          D3DPRINT(NORMAL_DBG_LEVEL, "Releasing PPALHNDL %8lXh (paletteHandle = %ld)",
                   pHndlList->ppPalHndlList[i], i);
          DXFREE(pHndlList->ppPalHndlList[i]);
        }

      // now release the memory for the array
      D3DPRINT(NORMAL_DBG_LEVEL,
               "Releasing pHndlList[%X]->ppPalHndlList[%X] for pDDLcl %x",
               pHndlList, pHndlList->ppPalHndlList, pDDLcl);
      DXFREE(pHndlList->ppPalHndlList);
      pHndlList->ppPalHndlList = NULL;
    }

    // now mark the HNDLLIST as unused
    if (NULL != pHndlList)
      ReleaseHndlList(ppdev, pHndlList);
  }

  pdddd->ddRVal = DD_OK;

  D3DPRINT(ENTRY_EXIT_DBG_LEVEL,"<< ddiDestroyDDLocal");

  return DDHAL_DRIVER_HANDLED;
} // ddiDestroyDDLocal

/*-------------------------------------------------------------------
Function Name:  GetHndlListPtr

Description:    finds the HndlList owned by pDDLcl.  If there
                isn't a HndlList owned by pDDLcl, then NULL is
                returned

Return:         HNDLLIST * or NULL
-------------------------------------------------------------------*/

#if HL_LINKED_LIST
HNDLLIST *
GetHndlListPtr(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl)
{
  HNDLLIST  *pHndlList = (HNDLLIST *)&g_pHndlList;
  HNDLLIST  *pHL;


  D3DPRINT(10,">> GetHndlListPtr");
  ASSERTDD(NULL != pDDLcl, "GetHndlListPtr invalid pDDLcl");

  while (pHndlList->pNext)
  {
    pHL = pHndlList->pNext;
    if (pDDLcl == pHL->pDDLcl)
    {
      D3DPRINT(NORMAL_DBG_LEVEL,"found HndlList=%8lXh", pHL);
      return pHL;
    }
    pHndlList = pHndlList->pNext;
  }

  D3DPRINT(NORMAL_DBG_LEVEL, "no HndlList found");
  D3DPRINT(10,"<< GetHndlListPtr");

  return NULL;
}
#else
HNDLLIST *
GetHndlListPtr(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl)
{
  HNDLLIST  *pHndlList;
  DWORD     i;


  D3DPRINT(10,">> GetHndlListPtr");
  ASSERTDD(NULL != pDDLcl, "GetHndlListPtr invalid pDDLcl");

  // if we don't have a HNDLLIST yet, just return NULL
  if (NULL == _D3(pHndlList))
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "no HndlList found");
    return NULL;
  }

  // Search the list for the hndlList owned by pDDLcl
  for (i = 0, pHndlList = _D3(pHndlList); i < _D3(numHndlLists); i++, pHndlList++)
    if (pDDLcl == pHndlList->pDDLcl)
      break;

  // if no match was found, return NULL
  if (i == _D3(numHndlLists))
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "no HndlList found");
    pHndlList = NULL;
  }
#if DBG
  else
    D3DPRINT(NORMAL_DBG_LEVEL,"found HndlList=%8lXh", pHndlList);
#endif

  D3DPRINT(10,"<< GetHndlListPtr");

  return pHndlList;
}
#endif

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

#if HL_LINKED_LIST
/*-------------------------------------------------------------------
Function Name:  AllocHndlList

Description:    Allocates a HndlList and insert it at the head of
                a linked list

Return:         HNDLLIST * or NULL
-------------------------------------------------------------------*/

static HNDLLIST *
AllocHndlList(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl)
{
  HNDLLIST  *pHL;


  D3DPRINT(10,">> AllocHndlList");

  pHL = (HNDLLIST *)DXMALLOCZ(sizeof(HNDLLIST));
  if (NULL != pHL)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  allocated HNDLLIST = %8lXh", pHL);

    // insert new HNDLLIST at head of linked list
    if (NULL != g_pHndlList)
    {
      pHL->pNext = g_pHndlList;
      g_pHndlList = pHL;
      D3DPRINT(NORMAL_DBG_LEVEL, "  inserting at head of list, g_pHndlList=%8lXh, pHL->pNext=%8lXh",
               g_pHndlList, pHL->pNext);
    }
    else
    {
      pHL->pNext = NULL;
      g_pHndlList = pHL;
      D3DPRINT(NORMAL_DBG_LEVEL, "  adding to empty list, g_pHndlList=%8lXh, pHL->pNext=%8lXh",
               g_pHndlList, pHL->pNext);
    }

    // if we allocated a HNDLLIST, then mark it used by this pDDLcl
    pHL->pDDLcl = pDDLcl;
  }
  else
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  unable to allocate HNDLIST");
  }

  D3DPRINT(10,"<< AllocTxtrHndl");

  return pHL;
}

/*-------------------------------------------------------------------
Function Name:  ReleaseHndlList

Description:    Remove a HNDLLIST from the linked list and free it

Return:         void
-------------------------------------------------------------------*/
static VOID
ReleaseHndlList(NT9XDEVICEDATA *ppdev, HNDLLIST *pHL)
{
  HNDLLIST  *pHndlList = (HNDLLIST *)&g_pHndlList;


  ASSERTDD(pHL != NULL, "can't free a NULL HNDLLIST");

  while (pHndlList->pNext)
  {
    if (pHL == pHndlList->pNext)
    {
      pHndlList->pNext = pHL->pNext;

      D3DPRINT(NORMAL_DBG_LEVEL, "  after unlinking HNDLLIST=%8lXh, pHndlList=%8lXh, pHndlList->pNext=%8lXh",
               pHL, pHndlList, pHndlList->pNext);

      pHL->pNext = NULL;
      DXFREE(pHL);

      return;
    }
    pHndlList = pHndlList->pNext;
  }

} // ReleaseHndlList
#else
/*-------------------------------------------------------------------
Function Name:  AllocHndlList

Description:    Allocates a HndlList in the HndlList array.  If the
                array isn't large enough, a new, larger array is
                allocated and the old array is copied into it.

Return:         HNDLLIST * or NULL
-------------------------------------------------------------------*/

#define HNDLLIST_ALLOC_ENTRIES_SIZE   10

static HNDLLIST *
AllocHndlList(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl)
{
  HNDLLIST  *pHndlList;
  DWORD     i;


  D3DPRINT(10,">> AllocHndlList");

  // if we already have a HNDLLIST, then search it to see if one is available
  if (NULL != _D3(pHndlList))
  {
    for (i = 0, pHndlList = _D3(pHndlList); i < _D3(numHndlLists); i++, pHndlList++)
      if (NULL == pHndlList->pDDLcl)
        break;
  }
  else
  {
    // we haven't allocated a HndlList yet,
    // so init i such that we'll allocate a HNDLLIST now
    i = _D3(numHndlLists);
  }

  if (i == _D3(numHndlLists))
  {
    // out of handlelists
    HNDLLIST  *pNewHndlList;
    int       size;


    // allocate a larger array. The theory is that there will be an upper limit on
    // the maximum and eventually the array will reach the max and allocation will
    // never occur again
    size = (_D3(numHndlLists) + HNDLLIST_ALLOC_ENTRIES_SIZE) * sizeof(HNDLLIST);
    pNewHndlList = (HNDLLIST *)DXMALLOCZ(size);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pDDLcl=%X's pHndlList[%X] size to %08lXh",
             pDDLcl, pNewHndlList, size);

    if (NULL == pNewHndlList)
    {
      D3DPRINT(0, "AllocHndlList failed to increase _D3(pHndlList)");
      pHndlList = NULL;
    }
    else
    {
      // copy the old data into the new array
      if (_D3(numHndlLists))
        memcpy(pNewHndlList, _D3(pHndlList), _D3(numHndlLists) * sizeof(HNDLLIST));
      // new entries should already be zero initialized by the memory allocation!

      // the first free HndlList better be the first one in the
      // new chunk of allocated HndlLists
      pHndlList = pNewHndlList + _D3(numHndlLists);

      // now update our count and HndlListPtr in _D3()
      _D3(numHndlLists) += HNDLLIST_ALLOC_ENTRIES_SIZE;

      // free old array
      if (NULL != _D3(pHndlList))
      {
        DXFREE(_D3(pHndlList));
        D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old HndlList[%X]",
                 pDDLcl, pHndlList);
      }

      // save pointer to new array
      _D3(pHndlList) = pNewHndlList;
    }
  }

  // if we allocated a HNDLLIST, then mark it used by this pDDLcl
  if (NULL != pHndlList)
    pHndlList->pDDLcl = pDDLcl;

  D3DPRINT(NORMAL_DBG_LEVEL,"  pDDLcl=%8lXh pHndlList=%8lX",
           pDDLcl, pHndlList);

  D3DPRINT(10,"<< AllocHndlList");

  return pHndlList;
}

/*-------------------------------------------------------------------
Function Name:  ReleaseHndlList

Description:    Release a previously allocated HndlList

Return:         void
-------------------------------------------------------------------*/
static VOID
ReleaseHndlList(NT9XDEVICEDATA *ppdev, HNDLLIST *pHndlList)
{
  // only need to clear the pDDLcl to release the HndlList
  pHndlList->pDDLcl = NULL;
} // ReleaseHndlList
#endif

/*-------------------------------------------------------------------
Function Name:  GetTxtrHndl

Description:    Find the index of the TXTRHNDL associated to this
                pDDLcl-Handle pair

Return:         0 or index into pHndlList->ppTxtrHndlList
-------------------------------------------------------------------*/

static DWORD
GetTxtrHndl(NT9XDEVICEDATA *ppdev,
            LPVOID pDDLcl,
            HNDLLIST *pHndlList,
            DWORD dwSurfaceHandle)
{
  D3DPRINT(10,">> GetTxtrHndl");
  ASSERTDD(pHndlList->pDDLcl == pDDLcl, "GetTxtrHndl invalid pDDLcl");

  // see if ppTxtrHndlList has an array allocated for it
  // and if so how long it is
  if ((NULL != pHndlList->ppTxtrHndlList) &&
      ((DWORD)pHndlList->ppTxtrHndlList[0] > dwSurfaceHandle))
  {
    if (NULL != pHndlList->ppTxtrHndlList[dwSurfaceHandle])
    {
      D3DPRINT(NORMAL_DBG_LEVEL,"found TxtrHndl=%lXh for surfHandle=%ld",
               pHndlList->ppTxtrHndlList[dwSurfaceHandle], dwSurfaceHandle);
      D3DPRINT(10,"<< GetTxtrHndl");

      return dwSurfaceHandle;
    }
  }

  D3DPRINT(NORMAL_DBG_LEVEL, "no TxtrHndl found");
  D3DPRINT(10,"<< GetTxtrHndl");

  return 0;
} // GetTxtrHndl

/*-------------------------------------------------------------------
Function Name:  AllocTxtrHndl

Description:    Allocates a TXTRHNDL in the ppTxtrHndlList array.
                If the array isn't large enough, a new, larger array is
                allocated and the old array is copied into it.

Return:         0 or index into ppHndlList->ppTxtrHndlList
-------------------------------------------------------------------*/

static DWORD
AllocTxtrHndl(NT9XDEVICEDATA *ppdev, LPVOID pDDLcl, HNDLLIST *pHndlList, DWORD dwSurfaceHandle)
{
  DWORD         dwTxtrHndl;
  TXHNDLSTRUCT  *pTxtrHndl;


  D3DPRINT(10,">> AllocTxtrHndl");

  ASSERTDD(NULL != pDDLcl && NULL != pHndlList, "AllocTxtrHndl invalid input");
  ASSERTDD(pHndlList->pDDLcl == pDDLcl, "AllocTxtrHndl invalid pDDLcl");

  // if we don't have a ppTxtHndlList or it's not big enough then
  // resize the current one
  if (NULL == pHndlList->ppTxtrHndlList ||
      dwSurfaceHandle > (DWORD)pHndlList->ppTxtrHndlList[0])
  {
    // dwSurfaceHandle numbers are going to be ordinal numbers starting
    // at one, so we use this number to figure out a "good" size for
    // our new list.
    // we need to account for using index zero as a count of how many elements are in
    // the ppTxtrHndlList array, so add one to dwSurfaceHandle
    DWORD newsize = (((dwSurfaceHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;
    TXHNDLSTRUCT **newlist= (TXHNDLSTRUCT **)DXMALLOCZ(sizeof(TXHNDLSTRUCT *) * newsize);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pDDLcl=%X's ppTxtrHndlList[%X] size to %08lXh",
             pDDLcl, newlist, newsize);

    if (NULL == newlist)
    {
      D3DPRINT(0, "AllocTxtrHndl failed to increase pHndlList->ppTxtrHndlList[%8lXh]",
               pHndlList);
      return 0;
    }

    // if we had a valid TXTRHNDL list,
    // copy it to the newlist and free the memory allocated before
    if (NULL != pHndlList->ppTxtrHndlList)
    {
      // copy counter in element zero plus all TXTRHNDL *'s to new array
      memcpy(newlist, pHndlList->ppTxtrHndlList,
             ((DWORD)pHndlList->ppTxtrHndlList[0] + 1) * sizeof(TXHNDLSTRUCT *));
      DXFREE(pHndlList->ppTxtrHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old ppTxtrHndlList[%X]",
               pDDLcl, pHndlList->ppTxtrHndlList);
    }

    pHndlList->ppTxtrHndlList = newlist;
    // store size in ppTxtrHndlList[0]
    // since we're using element zero to hold the array size
    // we only have newsize-1 entries to store TXTRHNDL elements in
    (DWORD)pHndlList->ppTxtrHndlList[0] = newsize - 1;
  }

  // If we don't have a TXTRHNDL hanging from this TXTRHNDL list
  // element we have to create one.
  if (NULL == pHndlList->ppTxtrHndlList[dwSurfaceHandle])
  {
    // now allocate a TXTRHNDL
    pTxtrHndl = (TXHNDLSTRUCT *)DXMALLOCZ(sizeof(TXHNDLSTRUCT));

    if (NULL == pTxtrHndl)
    {
      D3DPRINT(0, "AllocTxtrHndl out of memory, failed to alloc TXHNDLSTRUCT");
      return 0;
    }

    // mark it in use
    pTxtrHndl->flags = HandleInUse;

    // store the TXHNDLSTRUCT in the pHndlList array in the dwSurfaceHandle element
    pHndlList->ppTxtrHndlList[dwSurfaceHandle] = pTxtrHndl;
  }
  else
  {
    pTxtrHndl = pHndlList->ppTxtrHndlList[dwSurfaceHandle];
    // mark it in use
    pTxtrHndl->flags = HandleInUse;
  }

  dwTxtrHndl = dwSurfaceHandle;

  D3DPRINT(NORMAL_DBG_LEVEL,"Set pDDLcl=%8Xlh Handle=%ld dwTxtrHndl = %ld, pTxtrHndl=%8lXh",
           pDDLcl, dwSurfaceHandle, dwTxtrHndl, pTxtrHndl);

  D3DPRINT(10,"<< AllocTxtrHndl");

  return dwTxtrHndl;
} // AllocTxtrHndl

/*-------------------------------------------------------------------
Function Name:  ReleaseTxtrHndl

Description:    Release a previously allocated TXHNDLSTRUCT

Return:         void
-------------------------------------------------------------------*/

static VOID
ReleaseTxtrHndl(NT9XDEVICEDATA *ppdev, HNDLLIST *pHndlList, DWORD dwTxtrHndl)
{
  TXHNDLSTRUCT  *pTxtrHndl = pHndlList->ppTxtrHndlList[dwTxtrHndl];

  if (HandleInUse & pTxtrHndl->flags)
  {
    memset(pTxtrHndl, 0, sizeof(TXHNDLSTRUCT));
    DXFREE(pTxtrHndl);
    pHndlList->ppTxtrHndlList[dwTxtrHndl] = NULL;
  }
}

//---------------------------------------------------------------------------------------
//
// CanCreateExecuteBuffer32
//
// typedef struct _D3DVERTEXBUFFERDESC {
//     DWORD dwSize;
//     DWORD dwCaps;
//     DWORD dwFVF;
//     DWORD dwNumVertices;
// } D3DVERTEXBUFFERDESC, *LPD3DVERTEXBUFFERDESC;
//
// #define D3DVBCAPS_SYSTEMMEMORY      0x00000800l
// #define D3DVBCAPS_WRITEONLY         0x00010000l
// #define D3DVBCAPS_OPTIMIZED         0x80000000l
// #define D3DVBCAPS_DONOTCLIP         0x00000001l
//
//---------------------------------------------------------------------------------------

DWORD __stdcall CanCreateExecuteBuffer32( LPDDHAL_CANCREATESURFACEDATA lpd)
{
    LPDDSURFACEDESC  lpddsd;
    DWORD dwCaps;

    DPF("in CanCreateExecuteBuffer32");

    lpddsd = lpd->lpDDSurfaceDesc;
    dwCaps = lpddsd->ddsCaps.dwCaps;

    // assume we can't support it
    lpd->ddRVal = DDERR_UNSUPPORTED;

    // if the buffer is marked as Write Only we'll give it a shot!
    if (dwCaps & DDSCAPS_WRITEONLY)
        lpd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

//---------------------------------------------------------------------------------------
//
// CreateExecuteBuffer32
//
//---------------------------------------------------------------------------------------

int CreateVB( FXSURFACEDATA **surfaceData, DWORD dwCaps, LPDDHAL_CREATESURFACEDATA pcsd, LPDDRAWI_DDRAWSURFACE_LCL psurf_lcl );
void DeleteVB( LPDDHAL_CREATESURFACEDATA pcsd, FXSURFACEDATA *surfaceData, LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl );
#define DOUBLE_BUFFER_VBS 0 // TEMP: until I figure out how to these get used!

DWORD __stdcall CreateExecuteBuffer32( LPDDHAL_CREATESURFACEDATA pcsd )
{
    LPDDRAWI_DDRAWSURFACE_LCL FAR *lplpSList;
    LPDDRAWI_DDRAWSURFACE_LCL psurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl;
    int i, rc;
    DWORD dwCaps, dwCaps2;
    FXSURFACEDATA *surfaceData1, *surfaceData2;

    DD_ENTRY_SETUP(pcsd->lpDD);

    DPF("in CreateExecuteBuffer32");

    lplpSList = pcsd->lplpSList;
    pcsd->ddRVal = DD_OK;

    for (i=0; i<(int)pcsd->dwSCnt; i++)
    {
        psurf_lcl = lplpSList[i];
        psurf_gbl = psurf_lcl->lpGbl;
        dwCaps = psurf_lcl->ddsCaps.dwCaps;
        dwCaps2 = psurf_lcl->lpSurfMore->ddsCapsEx.dwCaps2;

        // if the VERTEXBUFFER flag is not present in dwCaps2 or we are not a Sage
        if (!(dwCaps2 & DDSCAPS2_VERTEXBUFFER) || (!IS_SAGE_ACTIVE)) {
            // it's either an implicit VB or a command buffer: punt to DirectDraw
            psurf_gbl->dwReserved1 = 0;
            pcsd->ddRVal = DDHAL_PLEASEALLOC_BLOCKSIZE;
            return DDHAL_DRIVER_HANDLED;
        }
        // else conditions are right: we will allocate and manage this VB
	    else if (IS_SAGE_ACTIVE) {

            // create first vertex buffer
            rc = CreateVB( &surfaceData1, dwCaps, pcsd, psurf_lcl);
            if (rc != TRUE)
                goto unwind1;

            if ( DOUBLE_BUFFER_VBS ) {

                // TEMP: by default set up two vbs for double buffering
                rc = CreateVB( &surfaceData2, dwCaps, pcsd, psurf_lcl );
                if (rc != TRUE)
                    goto unwind2;

                // all is well, let's return successfully
                surfaceData1->dwVertexBuffer1 = (DWORD)surfaceData1;
                surfaceData1->dwVertexBuffer2 = (DWORD)surfaceData2;
                surfaceData2->dwVertexBuffer1 = (DWORD)surfaceData1;
                surfaceData2->dwVertexBuffer2 = (DWORD)surfaceData2;
            }
            else {
                surfaceData1->dwVertexBuffer1 = (DWORD)surfaceData1;
                surfaceData1->dwVertexBuffer2 = 0;
            }

            psurf_gbl->dwReserved1 = (DWORD)surfaceData1;
            psurf_gbl->fpVidMem = surfaceData1->lfbPtr;
            psurf_gbl->lPitch = 0;          // Report 0 as stride.
	    }
    }
    return DDHAL_DRIVER_HANDLED;

unwind2:
    DeleteVB( pcsd, surfaceData1, psurf_gbl );

unwind1:
    return DDHAL_DRIVER_HANDLED;
}

//---------------------------------------------------------------------------------------
//
// DestroyExecuteBuffer32
//
//---------------------------------------------------------------------------------------

DWORD __stdcall DestroyExecuteBuffer32( LPDDHAL_DESTROYSURFACEDATA pdsd )
{
    LPDDRAWI_DDRAWSURFACE_LCL psurf;
    LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl;
    FXSURFACEDATA        *surfaceData1, *surfaceData2;

    DD_ENTRY_SETUP(pdsd->lpDD);

    psurf = pdsd->lpDDSurface;
    psurf_gbl = psurf->lpGbl;

    DPF("in DestroyExecuteBuffer32");
    pdsd->ddRVal = DD_OK;

    if ((DDHAL_PLEASEALLOC_BLOCKSIZE == psurf_gbl->fpVidMem ) || 
        (NULL == (LPVOID)psurf_gbl->fpVidMem))
        return DDHAL_DRIVER_HANDLED;

    surfaceData1 = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
    surfaceData2 = (FXSURFACEDATA*) surfaceData1->dwVertexBuffer2;

    DeleteVB( (LPDDHAL_CREATESURFACEDATA) pdsd, surfaceData1, psurf_gbl );
    DeleteVB( (LPDDHAL_CREATESURFACEDATA) pdsd, surfaceData2, psurf_gbl );

    psurf_gbl->fpVidMem=(FLATPTR)NULL;
    return DDHAL_DRIVER_HANDLED;
}

//---------------------------------------------------------------------------------------
//
// LockExecuteBuffer32
//
//---------------------------------------------------------------------------------------

DWORD __stdcall LockExecuteBuffer32( LPDDHAL_LOCKDATA lpd )
{
    FXSURFACEDATA       *surfaceData;

    DD_ENTRY_SETUP(lpd->lpDD);
    DPF("in LockExecuteBuffer32");

    // We only support execute buffers that are explicit vertex buffers;
    // also, these are agp-only vertex buffers. This means we can just
    // return the previously allocated address and return.

    surfaceData = (FXSURFACEDATA*) (lpd->lpDDSurface->lpGbl->dwReserved1);
    if ((surfaceData != NULL) && (surfaceData->surfaceLevel != _FF(ddCurrentSurfaceLevel)))
    {
        lpd->ddRVal = DDERR_SURFACELOST;
        return DDHAL_DRIVER_HANDLED;
    }

    if (surfaceData == NULL) {
        lpd->lpSurfData = (LPVOID)lpd->lpDDSurface->lpGbl->fpVidMem;
        lpd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    // no cam entry is necessary, just return the agp address
    lpd->lpSurfData = (LPVOID)lpd->lpDDSurface->lpGbl->fpVidMem;
    lpd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

//---------------------------------------------------------------------------------------
//
// UnlockExecuteBuffer32
//
//---------------------------------------------------------------------------------------

DWORD __stdcall UnlockExecuteBuffer32( LPDDHAL_UNLOCKDATA puld )
{
    FXSURFACEDATA  *surfaceData;
    DD_ENTRY_SETUP(puld->lpDD);

    DPF("in UnlockExecuteBuffer32");

    surfaceData = (FXSURFACEDATA*) (puld->lpDDSurface->lpGbl->dwReserved1);

    // kinda dull but maybe we will think of something to do in the future...

    if (surfaceData == NULL)
    {
        puld->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    puld->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

//---------------------------------------------------------------------------------------
//
// CreateVB()
//
//---------------------------------------------------------------------------------------

int CreateVB(
    FXSURFACEDATA             **surfaceData,
    DWORD                       dwCaps,
    LPDDHAL_CREATESURFACEDATA   pcsd,
    LPDDRAWI_DDRAWSURFACE_LCL   psurf_lcl)
{
    DWORD dwWidth;
    DD_ENTRY_SETUP(pcsd->lpDD);

    // allocate local surface descriptor
    *surfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));
    if (!*surfaceData) {
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        goto unwind1;
    }

    dwWidth = psurf_lcl->lpGbl->dwLinearSize;
    dwWidth = (dwWidth+31) & ~31; // Make it a multiple of 32bytes

    (*surfaceData)->surfaceLevel  = _FF(ddCurrentSurfaceLevel);
    (*surfaceData)->inFlipChain   = FALSE;
    (*surfaceData)->dwPixelFormat = DDPF_RGB;
    (*surfaceData)->ddsDwCaps     = dwCaps;
    (*surfaceData)->tileFlag      = MEM_IN_LINEAR;
    (*surfaceData)->dwPStride     = dwWidth;
    (*surfaceData)->dwStride      = 0;
    (*surfaceData)->dwFlags       = FXSURFACE_IS_VB;

    pcsd->ddRVal = surfMgr_allocSurface(
        psurf_lcl->lpGbl->lpDD,     // direct draw vidmemalloc needs this
        DDSCAPS_NONLOCALVIDMEM,     // put vertex buffers in agp memory
        0,                          // type of surface extension [IN]
        dwWidth,                    // width in bytes (linear space) [IN]
        1,                          // height in pixels (linear space) [IN]
        0,                          // linear address mask
        &((*surfaceData)->lfbPtr),  // host lfb start address of allocation [OUT]
        &((*surfaceData)->endlfbPtr),// host lfb end address of allocation [OUT]
        &((*surfaceData)->hwPtr),   // hw vidmem address
        &((*surfaceData)->dwStride),// pitch [OUT]
        &((*surfaceData)->tileFlag));// MEM_IN_TILED or MEM_IN_LINEAR [OUT]

    if ( pcsd->ddRVal != DD_OK ) {
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        goto unwind2;
    }

    return TRUE;

unwind2:
    // free FXSURFACEDATA structure 2
    DXFREE((void*)surfaceData);

unwind1:
    return FALSE;
}

//---------------------------------------------------------------------------------------
//
// DeleteVB()
//
//---------------------------------------------------------------------------------------

void DeleteVB(
    LPDDHAL_CREATESURFACEDATA   pcsd,
    FXSURFACEDATA              *surfaceData,
    LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl )
{
    DD_ENTRY_SETUP(pcsd->lpDD);

    if ((surfaceData != (FXSURFACEDATA*) NULL) && (surfaceData->hwPtr != 0)) {
        surfMgr_freeSurface(
            psurf_gbl->lpDD,            // direct draw vidmemalloc needs this
            surfaceData->lfbPtr,        // host lfb start address of allocation [OUT]
            surfaceData->hwPtr,         // hw start address of allocation [OUT]
            surfaceData->ddsDwCaps);
    }
    DXFREE((void*)surfaceData);
}
