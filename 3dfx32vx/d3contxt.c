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
**  11   3dfx      1.10        11/30/00 Brent Burton    DX8 Pixel Shader context
**       creation added, as well as sst2malloc() and sst2free() funcs that are 
**       passed to the shader compiler.
**  10   3dfx      1.9         11/7/00  Brent Burton    Added memset() call to 
**       clear *pRc prior to InitNewRC() call.
**  9    3dfx      1.8         5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  8    3dfx      1.7         5/11/00  Evan Leland     dx7 structure cleanup 
**       effort complete
**  7    3dfx      1.6         4/24/00  Evan Leland     preliminary changes, 
**       getting ready to removing all ddraw structure pointers stored in the 
**       driver
**  6    3dfx      1.5         12/17/99 Brian Danielson Compiler error fixups 
**       for Brent.
**  5    3dfx      1.4         12/17/99 Brent Burton    Removed all WINNT macro 
**       checks.  This removed some unused code.
**  4    3dfx      1.3         11/8/99  Evan Leland     General code cleanup: 
**       move texture-specific structures to d3txtr.h, move texture handle 
**       allocation macros to d3txtr.c, remove static texture handle allocation 
**       code that is no longer used; remove WINNT define
**  3    3dfx      1.2         9/30/99  Philip Zheng    Implemented the work 
**       around for DX 7 multiple buffer problem
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
** 16    8/31/99 2:02p Einkauf
** DirectX CMD FIFO
** 
** 15    8/13/99 10:06a Mconrad
** Sync to CSIM dll tree.
** 
** 14    8/10/99 10:28p Mconrad
** Make multimon happy.
** 
** 13    6/03/99 11:03p Peterm
** Made changes to get component to build with merged dd32 from h3 tot
** 
*/
#include "precomp.h"

#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"

//---------------
//
// Context Create
//
//---------------
DWORD __stdcall ddiContextCreate(LPD3DHAL_CONTEXTCREATEDATA pccd)
{
  RC  *newContext;
#if (DIRECT3D_VERSION < 0x0700) || (DX < 7)
  LPDDRAWI_DDRAWSURFACE_INT interfaceDDSZ;
#endif
  NT9XDEVICEDATA  *ppdev;

  D3D_ENTRY( "ddiContextCreate" );

  ppdev = (GLOBALDATA *)pccd->lpDDLcl->lpGbl->dwReserved3;
  
  //--------------------------------------------------------------------------
  // Allocate a new context handle:
  // This callback is invoked when a new surface is to be used as a
  // rendering target. The context handle returned will be used whenever rendering
  // to this surface is to be performed.
  //--------------------------------------------------------------------------
  pccd->dwhContext = CONTEXT_ALLOC( ppdev );
  if (pccd->dwhContext == CONTEXT_INVALID)
  {
    pccd->ddrval = D3DHAL_OUTOFCONTEXTS;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
  newContext = CONTEXT_PTR(pccd->dwhContext);

  D3DPRINT( 255,"ddiContextCreate, pccd->lpDDGbl =%08lx, pccd->lpDDS =%08lx", pccd->lpDDGbl, pccd->lpDDS );
  D3DPRINT( 255,"                  pccd->dwhContext =%08lx, pccd->lpDDSZ= %08lx", pccd->dwhContext, pccd->lpDDSZ );

  //---------------------------------------------------------------------------
  // Init the new Rendering context.
  //
  // lpDDS is the surface data which contains pointer to surface global data.
  //---------------------------------------------------------------------------
  memset(newContext, 0, sizeof(*newContext));
  newContext->ppdev = ppdev;
  initNewRC(newContext);
  STATESET_INIT(newContext->overrides);

  newContext->lpSurfDataZ = pccd->lpDDSZLcl != NULL ?
                            (FXSURFACEDATA *)pccd->lpDDSZLcl->lpGbl->dwReserved1 : NULL;
  newContext->lpSurfData = (FXSURFACEDATA *)pccd->lpDDSLcl->lpGbl->dwReserved1;
  newContext->pid = pccd->dwPID;

  {
    HNDLLIST  *pHndlList;
    DWORD     i;

    pHndlList = GetHndlListPtr(ppdev, pccd->lpDDLcl);

    newContext->pHndlList = pHndlList;
    newContext->DDSHndl = pccd->lpDDSLcl->lpSurfMore->dwSurfaceHandle;

    if ((NULL == newContext->pHndlList) ||
        (NULL == newContext->pHndlList->ppTxtrHndlList) ||
        (NULL == newContext->pHndlList->ppTxtrHndlList[newContext->DDSHndl]))
    {
      D3DPRINT(0, "ddiContextCreate - txtrHndl is NULL for the render target");
      D3DPRINT(0, "                   failing context creation");
      // we don't have a TXTRHNDL for the render target
      // bad things are going to happen if we allow this
      // so fail the context creation here!
      CONTEXT_FREE(ppdev, pccd->dwhContext);
      pccd->dwhContext = 0;
      pccd->ddrval = D3DHAL_CONTEXT_BAD;
      D3D_EXIT(DDHAL_DRIVER_HANDLED);
    }
    if (pccd->lpDDSZLcl)
    {
      newContext->DDSZHndl = pccd->lpDDSZLcl->lpSurfMore->dwSurfaceHandle;
      if ((NULL == newContext->pHndlList->ppTxtrHndlList) ||
          (NULL == newContext->pHndlList->ppTxtrHndlList[newContext->DDSZHndl]))
      {
        D3DPRINT(0, "ddiContextCreate - txtrHndl is NULL for the zbuffer");
        D3DPRINT(0, "                   failing context creation");
        // we don't have a TXTRHNDL for the z buffer
        // bad things are going to happen if we allow this
        // so fail the context creation here!
        CONTEXT_FREE(ppdev, pccd->dwhContext);
        pccd->dwhContext = 0;
        pccd->ddrval = D3DHAL_CONTEXT_BAD;
        D3D_EXIT(DDHAL_DRIVER_HANDLED);
      }
    }
    else
      newContext->DDSZHndl = 0;

    // go fill in contextId's of any TXTRHNDL's already created
    // for this context
    if (NULL != pHndlList->ppTxtrHndlList)
    {
      for (i = 1; i < (DWORD)pHndlList->ppTxtrHndlList[0]; i++)
      {
        if (0 != pHndlList->ppTxtrHndlList[i])
        {
          pHndlList->ppTxtrHndlList[i]->contextId = (DWORD)newContext;
        }
      }
    }

    // init state block info
    newContext->bSBRecMode = FALSE;
    newContext->pCurrSB = NULL;
    newContext->ppSBTable = NULL;

#if (DIRECT3D_VERSION >= 0x0800)
    // Pixel Shader initialization
    newContext->bPscDirty = TRUE;
    newContext->dwCurrentPShader = 0;       // 0 for DX7.  Nonzero for PS handle.
    newContext->pPscContext = (PSC_CONTEXT*)DXMALLOCZ(sizeof(PSC_CONTEXT));
    newContext->pPscContext->malloc = sst2malloc;
    newContext->pPscContext->free   = sst2free;
    newContext->pPscContext->userData = newContext;
#endif

  }

  pccd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
} 

//--------------- 
//
// ContextDestroy
//
//---------------
DWORD __stdcall ddiContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pcdd)
{

  SETUP_PPDEV(pcdd->dwhContext)

  D3D_ENTRY( "ddiContextDestroy" );

  //-------------------------------------------------------------------------
  // Delete the context. This callback is invoked when a context is to be destroyed.
  //-------------------------------------------------------------------------
  D3DPRINT( 255,"ddiContextDestroy, pcdd->dwhContext =%08lx", pcdd->dwhContext );
  
  // release state blocks for this context
  ReleaseContextStateBlocks((RC *)pcdd->dwhContext);

#if DIRECT3D_VERSION >= 0x0800
  DXFREE( ((RC*)pcdd->dwhContext)->pPscContext);
#endif

  CONTEXT_FREE(ppdev, pcdd->dwhContext);
  if (pcdd->dwhContext == CONTEXT_INVALID)
  {
    D3DPRINT( 255,"ddiContextDestroy, bad context =0x08lx", pcdd->dwhContext );
    pcdd->ddrval = D3DHAL_CONTEXT_BAD;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

  pcdd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
} 

//--------------------
//
// Context Destroy All
//
//--------------------
DWORD __stdcall ddiContextDestroyAll(LPD3DHAL_CONTEXTDESTROYALLDATA pcdd)
{
  NT9XDEVICEDATA  *ppdev;
  int  i;

  D3D_ENTRY( "ddiContextDestroyAll" );

  //----------------------------------------------------------------
  // This callback is invoked when a process dies.  All the contexts
  // which were created by this context need to be destroyed.
  //----------------------------------------------------------------
  D3DPRINT( 255, "ddiContextDestroyAll, pcdd->dwPID =%08lx", pcdd->dwPID );

  for( i=0; i<NUM_DEVICES; i++)
  {
      ppdev = pDevices[i];

      if ( NULL == ppdev )
         continue;
      
      CONTEXT_FREE_PID( ppdev, pcdd->dwPID );
  }

  triangleFlavour( 0xFFFFFFFF, 0, 0 );
  pcdd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
} 


//----------------
// malloc() and free() functions for pixel shader usage:
//----------------
#if DIRECT3D_VERSION >= 0x0800

void * sst2malloc (void *userData, size_t size)
{
    RC *pRc = (RC*)userData;
    SETUP_PPDEV(pRc)

    return DXMALLOCZ(size);
}


void   sst2free   (void *userData, void * p)
{
    RC *pRc = (RC*)userData;
    SETUP_PPDEV(pRc)

    DXFREE(p);
}

#endif // DX8
