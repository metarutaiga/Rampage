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
** $Revision: 33$
** $Date: 11/14/00 2:58:50 PM PST$
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

// Everything NT builds need are in precomp.h!
#include "d3txtr.h"
#include "d3contxt.h"
#include "ddglobal.h"
#include "d3global.h"
#include "fifomgr.h"
#include "d3txtr2.h"                    // txtrDesc

/**************************************************************************
* D E F I N E S
***************************************************************************/

#define NORMAL_DBG_LEVEL    2

#define MAXTEXTURECOUNT                                                                 \
    (_D3(numTextureDescs) )

/**************************************************************************
* E X T E R N  P R O T O T Y P E S
***************************************************************************/
extern RENDERFXN   _renderFuncs[] ;        // defined in d3rstate.c

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

static STATEBLOCK *FindStateBlock(RC *, DWORD);
static HRESULT AddStateBlockToTable(RC *, DWORD, STATEBLOCK *);
static STATEBLOCK *CompressStateBlock(RC *, STATEBLOCK *);

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

void ConfigSLI(NT9XDEVICEDATA * ppdev, int nSLI);

/*-------------------------------------------------------------------
Function Name:  SetRenderTarget

Description:

Return:
-------------------------------------------------------------------*/

HRESULT SetRenderTarget(
    RC         *pRc,
    DWORD       hRenderTarg,
    DWORD       hZBuffer)
{
    FXSURFACEDATA *surfData;
    FXSURFACEDATA *surfDataZ;

    SETUP_PPDEV(pRc)

    surfData = TXTRHNDL_TO_SURFDATA(pRc, hRenderTarg);

    // cannot render into system memory
    if (surfData->ddsDwCaps & DDSCAPS_SYSTEMMEMORY)
        return DDERR_CURRENTLYNOTAVAIL;

    pRc->DDSHndl = hRenderTarg;
    pRc->lpSurfData = (FXSURFACEDATA *)TXTRHNDL_TO_SURFDATA(pRc, hRenderTarg);

    if (hZBuffer) {
        surfDataZ = TXTRHNDL_TO_SURFDATA(pRc, hZBuffer);

        // cannot render if z buffer is in system memory
        if (surfDataZ->ddsDwCaps & DDSCAPS_SYSTEMMEMORY)
            return DDERR_CURRENTLYNOTAVAIL;

        pRc->lpSurfDataZ = (FXSURFACEDATA *)TXTRHNDL_TO_SURFDATA(pRc, hZBuffer);
        pRc->DDSZHndl = hZBuffer;
    }
    else
    {
        pRc->lpSurfDataZ = NULL;
        pRc->DDSZHndl = 0;
    }

#ifdef SLI
    if ((SLI_MODE_ENABLED == _DD(sliMode)) && 
        (pRc->lpSurfData->dwFlags & FXSURFACE_IS_DISTRIBUTED)) {
        pRc->sst.raControl.vFxU32 |= SST_RA_ENABLE_SLI;
        pRc->sst.peFbzMode.vFxU32 |= SST_PE_SLI_EN;
        ConfigSLI(ppdev, TRUE);
    }
    else {
        pRc->sst.raControl.vFxU32 &= ~SST_RA_ENABLE_SLI;
        pRc->sst.peFbzMode.vFxU32 &= ~SST_PE_SLI_EN;
        if (SLI_MODE_ENABLED == _DD(sliMode)) {
            ConfigSLI(ppdev, FALSE);
        }
    } 
#endif // sli

    D3DPRINT(NORMAL_DBG_LEVEL, "SetRenderTarget, dwhContext=%08lXh surfH:%d, zH:%d",
             pRc,hRenderTarg, hZBuffer);
    _D3(lastContext) = 0;
    return D3D_OK;
}


/*-------------------------------------------------------------------
Function Name:  PaletteSet

Description:    Attaches a palette handle to a texture in the given
                context.  The texture is the one associated to the
                given surface handle.

Return:
-------------------------------------------------------------------*/

HRESULT
PaletteSet(RC *pRc, DWORD dwTxtrHndl, DWORD dwPaletteHandle, DWORD dwPaletteFlags)
{
  SETUP_PPDEV(pRc)
  HNDLLIST                    *pHndlList = pRc->pHndlList;
  PALHNDL                     *pPalHndl;
  FXSURFACEDATA               *surfData;

  D3DPRINT(10,">> PaletteSet");
  D3DPRINT(NORMAL_DBG_LEVEL,"SETPALETTE PalHndl=%ld to SurfHndl=%ld", dwPaletteHandle, dwTxtrHndl);

  if (0 == dwTxtrHndl)
  {
    // invalid dwSurfaceHandle, skip it
    D3DPRINT(0,"__PaletteSet:NULL==pTexture Palette=%ld Surface=%ld", dwPaletteHandle, dwTxtrHndl);
    return DDERR_INVALIDPARAMS;
  }

  // if the TXTRHNDL_PTR is NULL, then it's probably a handle for a mipmap level
  // smaller than the top level mipmap, we'll just ignore it for now
  if (NULL == pHndlList->ppTxtrHndlList[dwTxtrHndl])
  {
    return D3D_OK;
  }

  // save palette handle in surface data struct
  surfData = TXTRHNDL_TO_SURFDATA(pRc, dwTxtrHndl);
  surfData->dwPaletteHandle = dwPaletteHandle;

  if (TS[0].textureHandle == dwTxtrHndl)
    _D3(flags) |= PALETTECHANGED;

  if (0 == dwPaletteHandle)
  {
    // palette association is OFF
    return D3D_OK;
  }

  // if we don't have a ppPalHndlList or it's not big enough then
  // resize the current one
  if (NULL == pHndlList->ppPalHndlList || dwPaletteHandle > (DWORD)pHndlList->ppPalHndlList[0])
  {
    // we need to account for using index zero as a count of how many elements are in
    // the ppPalHndlList array, so add one to dwPaletteHandle
    DWORD   newsize = (((dwPaletteHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;
    PALHNDL **newlist= (PALHNDL **)DXMALLOCZ(sizeof(PALHNDL *)*newsize);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pDDLcl=%X's PaletteList[%X] size to %08lXh",
             pRc->pDDLcl, newlist, newsize);

    if (NULL == newlist)
    {
      D3DPRINT(0,"D3DDP2OP_SETPALETTE Out of memory failed to grow PALHNDL list");
      return DDERR_OUTOFMEMORY;
    }

    // if we had a valid PALHNDL list,
    // copy it to the newlist and free the memory allocated before
    if (NULL != pHndlList->ppPalHndlList)
    {
      // copy counter in element zero plus all PALHNDL *'s to new array
      memcpy(newlist, pHndlList->ppPalHndlList,
             ((DWORD)pHndlList->ppPalHndlList[0] + 1) * sizeof(PPALHNDL));
      DXFREE(pHndlList->ppPalHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old PaletteList[%X]",
               pRc->pDDLcl, pHndlList->ppPalHndlList);
    }

    pHndlList->ppPalHndlList = newlist;
    // store size in ppPalHndlList[0]
    // since we're using element zero to hold the array size
    // we only have newsize-1 entries to store PALHNDL * elements in
    (DWORD)pHndlList->ppPalHndlList[0] = newsize - 1;
  }

  // If we don't have a palette hanging from this palette list
  // element we have to create one. The actual palette data will
  // come down in the D3DDP2OP_UPDATEPALETTE command token.
  if (NULL == pHndlList->ppPalHndlList[dwPaletteHandle])
  {
    // now allocate a PALHNDL
    pPalHndl = (PALHNDL *)DXMALLOCZ(sizeof(PALHNDL));

    if (NULL == pPalHndl)
    {
      D3DPRINT(0, "D3DDP2OP_SETPALETTE out of memory, failed to alloc PALHNDL");
      return DDERR_OUTOFMEMORY;
    }

    // store the PALHNDL in the pHndlList array in the dwPaletteHandle element
    pHndlList->ppPalHndlList[dwPaletteHandle] = pPalHndl;
  }
  else
    pPalHndl = pHndlList->ppPalHndlList[dwPaletteHandle];

  // driver may store this dwFlags to decide whether ALPHA exists in Palette
  pPalHndl->dwFlags = dwPaletteFlags;

  D3DPRINT(NORMAL_DBG_LEVEL,"Set pDDLcl=%8lXh PaletteHandle=%ld pPalHndl = %8lXh",
           pRc->pDDLcl, dwPaletteHandle, pPalHndl);
  D3DPRINT(10,"<< PaletteSet");

  return D3D_OK;
} // PaletteSet

/*-------------------------------------------------------------------
Function Name:  PaletteUpdate

Description:    Updates the entries of a palette attached to a texture
                in the given context

Return:
-------------------------------------------------------------------*/

HRESULT
PaletteUpdate(RC    *pRc,
              DWORD dwPaletteHandle,
              WORD  wStartIndex,
              WORD  wNumEntries,
              BYTE  *pPaletteData)
{
  SETUP_PPDEV(pRc)
  PALHNDL *pPalHndl;


  D3DPRINT(10,">> PaletteUpdate");

  D3DPRINT(NORMAL_DBG_LEVEL,"UPDATEPALETTE dwPaletteHandle %ld, StartIndex %ld, NumEntries %ld",
           dwPaletteHandle,
           wStartIndex,
           wNumEntries);

  ASSERTDD(NULL != pRc->pHndlList, "NULL pHndlList");
  ASSERTDD(NULL != pRc->pHndlList->ppPalHndlList, "NULL ppPalList");

  pPalHndl = pRc->pHndlList->ppPalHndlList[dwPaletteHandle];
  if (NULL != pPalHndl)
  {
    ASSERTDD(256 >= wStartIndex + wNumEntries,
             "wStartIndex+wNumEntries>256 in D3DDP2OP_UPDATEPALETTE");

    // Copy the palette & associated data
    pPalHndl->wStartIndex = wStartIndex;
    pPalHndl->wNumEntries = wNumEntries;

    memcpy((LPVOID)&pPalHndl->ColorTable[wStartIndex],
           (LPVOID)pPaletteData,
           (DWORD)wNumEntries*sizeof(PALETTEENTRY));

    // If we are currently texturing and the texture is using the
    // palette we just updated, dirty the texture flag so that
    // it set up with the right (updated) palette
    if (0 != TS[0].textureHandle)
    {
      // if the TXTRHNDL_PTR is NULL, then it's probably a handle for a mipmap level
      // smaller than the top level mipmap, we'll just ignore it for now
      if (NULL != pRc->pHndlList->ppTxtrHndlList[TS[0].textureHandle])
      {
        FXSURFACEDATA *surfData = TXTRHNDL_TO_SURFDATA(pRc, TS[0].textureHandle);
        if (surfData->dwPaletteHandle == dwPaletteHandle)
          _D3(flags) |= PALETTECHANGED;
      }
    }
  }
  else
  {
    D3DPRINT(10,"<< PaletteUpdate");

    return DDERR_INVALIDPARAMS;
  }

  D3DPRINT(10,"<< PaletteUpdate");

  return D3D_OK;
} // PaletteUpdate

/*-------------------------------------------------------------------
Function Name:  BeginStateBlock

Description:    create a new state block identified by dwHandle and
                enable recording states

Return:
-------------------------------------------------------------------*/

HRESULT
BeginStateBlock(RC *pRc, DWORD dwHandle)
{
  SETUP_PPDEV(pRc)
  STATEBLOCK  *pSB;


  D3DPRINT(10,">> BeginStateBlock dwHandle=%ld",dwHandle);

  // make sure we're not already in recording mode
  if (TRUE == pRc->bSBRecMode)
  {
    D3DPRINT(0, "Currently recording state block");
    D3DPRINT(10,"<< BeginStateBlock");
    return D3DERR_INBEGINSTATEBLOCK;
  }

  // create a new state block
  pSB = DXMALLOCZ(sizeof(STATEBLOCK));
  if (NULL == pSB)
  {
    D3DPRINT(0, "Out of memory for additional state block");
    D3DPRINT(10,"<< BeginStateBlock");
    return DDERR_OUTOFMEMORY;
  }

  // save handle to current state block
  pSB->dwHandle = dwHandle;
  pSB->bCompressed = FALSE;

  // set the current state block in the RC
  pRc->pCurrSB = pSB;

  // start recording mode
  pRc->bSBRecMode = TRUE;

  D3DPRINT(NORMAL_DBG_LEVEL, "  Starting record mode, SBHandle=%ld, pCurrSB=%lXh for pRc=%lXh",
           dwHandle, pSB, pRc);

  D3DPRINT(10,"<< BeginStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  EndStateBlock

Description:    disable recording states, revert to executing them
                compress recorded state block and store in context

Return:
-------------------------------------------------------------------*/

HRESULT
EndStateBlock(RC *pRc)
{
  DWORD       dwHandle;
  STATEBLOCK  *pSB;
  HRESULT     hr;


  D3DPRINT(10,">> EndStateBlock");

  // make sure we're in recording mode
  if (FALSE == pRc->bSBRecMode)
  {
    D3DPRINT(0, "Currently NOT recording state block");
    D3DPRINT(10,"<< EndStateBlock");
    return D3DERR_NOTINBEGINSTATEBLOCK;
  }

  hr = D3D_OK;

  if (pRc->pCurrSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  Ending record mode, SBHandle=%ld, pCurrSB=%lXh for pRc=%lXh",
             pRc->pCurrSB->dwHandle, pRc->pCurrSB, pRc);

    dwHandle = pRc->pCurrSB->dwHandle;

    // compress state block
    // Note: after being compressed, the uncompressed version is freed
    // so don't access pRc->pCurrSB after this, use pSB instead
    pSB = CompressStateBlock(pRc, pRc->pCurrSB);
    D3DPRINT(NORMAL_DBG_LEVEL, "  CompressStateBlock returned pSB=%lXh", pSB);

    // add state block to ppSBTable
    hr = AddStateBlockToTable(pRc, dwHandle, pSB);
  }

  // clear the current state block in the RC
  pRc->pCurrSB = NULL;

  // end recording mode
  pRc->bSBRecMode = FALSE;

  D3DPRINT(10,"<< EndStateBlock");
  return hr;
}

/*-------------------------------------------------------------------
Function Name:  DeleteStateBlock

Description:    delete the recorded state block identified by dwHandle

Return:
-------------------------------------------------------------------*/

HRESULT
DeleteStateBlock(RC *pRc, DWORD dwHandle)
{
  SETUP_PPDEV(pRc)
  STATEBLOCK  *pSB;


  D3DPRINT(10,">> DeleteStateBlock dwHandle=%ld",dwHandle);

  pSB = FindStateBlock(pRc, dwHandle);
  if (NULL != pSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             pSB->dwHandle, pSB, pRc);

    pRc->ppSBTable[dwHandle] = NULL;
    DXFREE(pSB);
  }

  D3DPRINT(10,"<< DeleteStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  ExecuteStateBlock

Description:

Return:
-------------------------------------------------------------------*/

HRESULT
ExecuteStateBlock(RC *pRc, DWORD dwHandle)
{
  STATEBLOCK       *pSB;
  DWORD             i, j;
  HRESULT           hr;

  D3DPRINT(10,">> ExecuteStateBlock dwHandle=%ld",dwHandle);

  pSB = FindStateBlock(pRc, dwHandle);
  if (NULL != pSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  executing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             pSB->dwHandle, pSB, pRc);

    if (! pSB->bCompressed)
    {
      // uncompressed state block
      D3DPRINT(NORMAL_DBG_LEVEL, "    uncompressed state block");

      // Execute any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored render states");
      for (i = 1; i <= MAX_RENDERSTATES; i++)   // renderstate zero is unused
      {
        if (IS_SB_RS_FLAG_SET(pSB, i))
        {
          D3DRENDERSTATETYPE  renderState;
          DWORD               data;
          BYTE               *pRcBytePtr;

          renderState = i;
          data = pSB->u.uc.RenderStates[renderState];

          D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", renderState, data);

          // Here we need to determine if we are going to actually call the render function.
          // But, if RENDERSTATE_FUNCTION_PATHS is set, we make the call so D3PRINTs are processed.
#ifdef RENDERSTATE_FUNCTION_PATHS
          _renderFuncs[ renderState ].renderFXN( pRc, data );
#else
          switch (_renderFuncs[ renderState ].callType)
          {
          case RF_ALWAYS_CALL :   // Always make the renderfunction call
              _renderFuncs[ renderState ].renderFXN( pRc, data );
              break;
          case RF_CHECK_FIRST :   // Make the call only if the new state differs from the current state
              // compute address of pRc field based off of RC structure address and field offset
              pRcBytePtr = (BYTE*)(((LONG) pRc) + _renderFuncs[ renderState ].fieldIndex);
              if ( *((ULONG*)pRcBytePtr) != data)
              {
                  _renderFuncs[ renderState ].renderFXN( pRc, data );
              }
              break;
          case RF_UPDATE_ONLY :   // Only need to update the pRc value (no shadow regs affected)
              // compute address of pRc field based off of RC structure address and field offset
              pRcBytePtr = (BYTE*)(((LONG) pRc) + _renderFuncs[ renderState ].fieldIndex);
              *((ULONG*)pRcBytePtr) = data;
              break;
          case RF_DUMMY :
              // if it is a "dummy", then nothing to do.
              break;
          }
#endif
          hr = DD_OK;

          if (DD_OK != hr)
            return hr;
        }
      }

      // Execute any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored texture stage states");
      for (j = 0; j < NUMTEXTUREUNITS; j++)
      {
        for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)   // TSS zero is the texture handle
        {
          if (IS_SB_TSS_FLAG_SET(pSB, j, i))
          {
            DWORD stage, state, data;

            stage = j;
            state = i;
            data = pSB->u.uc.TssStates[stage][state];

            D3DPRINT(D3DDBGLVL, "        stage=%ld, state=%ld, data=%ld", stage, state, data);

            pRc->textureStage[stage].changed = TRUE;

            switch (state)
            {
              case 0 :  // Texture Handle  fix a bug with DCT 50's, they send down -1 for a texture handle.
                if (data == 0xFFFFFFFF)
                  data = 0x0;

                ((LPDWORD)&pRc->textureStage[stage])[state] = data;
                break;

#if (DIRECT3D_VERSION < 0x0800) || HACK_DX8DDK_BUGS
              case D3DTSS_ADDRESS :
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
                break;
#endif // DX < 8

              default :
                ((LPDWORD)&pRc->textureStage[stage])[state] = data;
                break;
            }
          }
        }
      }
      // Execute any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
    }
    else
    {
      DWORD dwFinalState;

      // compressed state set
      D3DPRINT(NORMAL_DBG_LEVEL, "    compressed state block");

      // Execute any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored render states");
      for (i = 0; i < pSB->u.cc.dwNumRS; i++)
      {
        D3DRENDERSTATETYPE  renderState;
        DWORD               data;
        BYTE               *pRcBytePtr;

        renderState = pSB->u.cc.pair[i].dwType;
        data = pSB->u.cc.pair[i].dwValue;

        D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", renderState, data);

        // Here we need to determine if we are going to actually call the render function.
        // But, if RENDERSTATE_FUNCTION_PATHS is set, we make the call so D3PRINTs are processed.
#ifdef RENDERSTATE_FUNCTION_PATHS
        _renderFuncs[ renderState ].renderFXN( pRc, data );
#else
        switch (_renderFuncs[ renderState ].callType)
        {
        case RF_ALWAYS_CALL :   // Always make the renderfunction call
            _renderFuncs[ renderState ].renderFXN( pRc, data );
            break;
        case RF_CHECK_FIRST :   // Make the call only if the new state differs from the current state
            // compute address of pRc field based off of RC structure address and field offset
            pRcBytePtr = (BYTE*)(((LONG) pRc) + _renderFuncs[ renderState ].fieldIndex);
            if ( *((ULONG*)pRcBytePtr) != data)
            {
                _renderFuncs[ renderState ].renderFXN( pRc, data );
            }
            break;
        case RF_UPDATE_ONLY :   // Only need to update the pRc value (no shadow regs affected)
            // compute address of pRc field based off of RC structure address and field offset
            pRcBytePtr = (BYTE*)(((LONG) pRc) + _renderFuncs[ renderState ].fieldIndex);
            *((ULONG*)pRcBytePtr) = data;
            break;
        case RF_DUMMY :
            // if it is a "dummy", then nothing to do.
            break;
        }
#endif
        hr = DD_OK;

        if (DD_OK != hr)
          return hr;
      }

      // Execute any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored texture stage states");
      dwFinalState = pSB->u.cc.dwNumRS;
      for (j = 0; j < NUMTEXTUREUNITS; j++)
      {
        dwFinalState += pSB->u.cc.dwNumTSS[j];
        for (; i < dwFinalState; i++)       // i falls through from above
        {
          DWORD stage, state, data;

          stage = j;
          state = pSB->u.cc.pair[i].dwType;
          data  = pSB->u.cc.pair[i].dwValue;

          D3DPRINT(D3DDBGLVL, "        stage=%ld, state=%ld, data=%ld", stage, state, data);

          pRc->textureStage[stage].changed = TRUE;

          switch (state)
          {
            case 0 :  // Texture Handle  fix a bug with DCT 50's, they send down -1 for a texture handle.
              if (data == 0xFFFFFFFF)
                data = 0x0;

              ((LPDWORD)&pRc->textureStage[stage])[state] = data;
              break;

#if (DIRECT3D_VERSION < 0x0800) || HACK_DX8DDK_BUGS
            case D3DTSS_ADDRESS :
              ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
              ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
              break;
#endif // DX < 8

            default :
              ((LPDWORD)&pRc->textureStage[stage])[state] = data;
              break;
          }
        }
      }
      // Execute any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
    }

#if (DIRECT3D_VERSION >= 0x0800)
    {
        FXSURFACEDATA *fxsd;

        // Capture stream information if set.
        for (i=0; i < MAX_STREAMS; i++) {
            if (pSB->streams[i].bIsSet) {
                pRc->Stream[i] = pSB->streams[i];
                if (pRc->Stream[i].dwHandle) {
                    fxsd = TXTRHNDL_TO_SURFDATA(pRc, pRc->Stream[i].dwHandle);
                    pRc->pStream[i] = (LPBYTE)(fxsd ? fxsd->lfbPtr : 0);
                } else
                    pRc->pStream[i] = 0;
            }
        }
        // And fetch index buffer
        if (pSB->indices.bIsSet) {
            pRc->CurrentIndexBuffer = pSB->indices;
            if (pRc->CurrentIndexBuffer.dwHandle) {
                fxsd = TXTRHNDL_TO_SURFDATA(pRc, pRc->CurrentIndexBuffer.dwHandle);
                pRc->pCurrentIndexBuffer = (LPBYTE)(fxsd ? fxsd->lfbPtr : 0);
            } else
                pRc->pCurrentIndexBuffer = 0;
        }
    }
#endif
  }

  D3DPRINT(10,"<< ExecuteStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  CaptureStateBlock

Description:

Return:
-------------------------------------------------------------------*/

HRESULT
CaptureStateBlock(RC *pRc, DWORD dwHandle)
{
  STATEBLOCK  *pSB;
  DWORD       i, j;
  DWORD       dwFinalState;
  DWORD       state;
  DWORD       data;

  D3DPRINT(10,">> CaptureStateBlock dwHandle=%ld",dwHandle);

  pSB = FindStateBlock(pRc, dwHandle);
  if (NULL != pSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  capturing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             pSB->dwHandle, pSB, pRc);

    if (! pSB->bCompressed)
    {
      // uncompressed state set
      D3DPRINT(NORMAL_DBG_LEVEL, "    uncompressed state block");

      // Capture any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing render states");
      for (i = 1; i <= MAX_RENDERSTATES; i++)     // renderstate zero is unused
      {
        if (IS_SB_RS_FLAG_SET(pSB, i))
        {
          if (0 == _renderFuncs[i].fieldIndex)  // if 0, we do not support this renderstate
          {
            // special case for scene capture
            if (D3DRENDERSTATE_SCENECAPTURE == i)
            {
              SETUP_PPDEV(pRc)
              data = (_D3(flags) & IN_RENDER_SCENE ? TRUE : FALSE);
            }
            // otherwise they are render states we don't care about
            // so just return zero
            else
              data = 0;
          }
          else
            data = *(DWORD *)((BYTE *)pRc + _renderFuncs[i].fieldIndex);

          pSB->u.uc.RenderStates[i] = data;

          D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", i, pSB->u.uc.RenderStates[i]);
        }
      }

      // Capture any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing texture stage states");
      for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      {
        for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)   // TSS zero is the texture handle
        {
          if (IS_SB_TSS_FLAG_SET(pSB, j, i))
          {
            data = ((LPDWORD)&pRc->textureStage[j])[i];

            pSB->u.uc.TssStates[j][i] = data;

            switch( i )
            {
              case D3DTSS_COLOROP:      // =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
              case D3DTSS_COLORARG1:    // =  2, /* D3DTA_* (texture arg) */
              case D3DTSS_COLORARG2:    // =  3, /* D3DTA_* (texture arg) */
              case D3DTSS_ALPHAOP:      // =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
              case D3DTSS_ALPHAARG1:    // =  5, /* D3DTA_* (texture arg) */
              case D3DTSS_ALPHAARG2:    // =  6, /* D3DTA_* (texture arg) */
                if(pRc->texMapBlend == 0x7ffffffe)
                  pRc->texMapBlend = D3DTBLEND_MODULATE;
                 break;
            }

            D3DPRINT(NORMAL_DBG_LEVEL, "        stage=%ld, state=%ld, data=%ld", j, i, pSB->u.uc.TssStates[j][i]);
          }
        }
      }

      // Capture any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
    }
    else
    {
      // compressed state set
      D3DPRINT(NORMAL_DBG_LEVEL, "    compressed state block");

      // Capture any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing render states");
      for (i = 0; i < pSB->u.cc.dwNumRS; i++)
      {
        state = pSB->u.cc.pair[i].dwType;
        if (0 == _renderFuncs[state].fieldIndex)    // if 0 we do not support this renderstate
          data = 0;
        else
          data = *(DWORD *)((BYTE *)pRc + _renderFuncs[state].fieldIndex);

        pSB->u.cc.pair[i].dwValue = data;

        D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", state, pSB->u.cc.pair[i].dwValue);
      }

      // Capture any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing texture stage states");
      dwFinalState = pSB->u.cc.dwNumRS;
      for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      {
        dwFinalState += pSB->u.cc.dwNumTSS[j];
        for (; i < dwFinalState; i++)
        {
          DWORD state;
          DWORD data;

          state = pSB->u.cc.pair[i].dwType;
          data = ((LPDWORD)&pRc->textureStage[j])[i];

          pSB->u.cc.pair[i].dwValue = data;

          switch( state )
          {
            case D3DTSS_COLOROP:      // =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
            case D3DTSS_COLORARG1:    // =  2, /* D3DTA_* (texture arg) */
            case D3DTSS_COLORARG2:    // =  3, /* D3DTA_* (texture arg) */
            case D3DTSS_ALPHAOP:      // =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
            case D3DTSS_ALPHAARG1:    // =  5, /* D3DTA_* (texture arg) */
            case D3DTSS_ALPHAARG2:    // =  6, /* D3DTA_* (texture arg) */
              if(pRc->texMapBlend == 0x7ffffffe)
                pRc->texMapBlend = D3DTBLEND_MODULATE;
              break;
          }

          D3DPRINT(NORMAL_DBG_LEVEL, "        stage=%ld, state=%ld, data=%ld", j, state, pSB->u.cc.pair[i].dwValue);
        }
      }

      // Capture any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
    } // else compressed

#if (DIRECT3D_VERSION >= 0x0800)
    // Capture stream information if set.
    for (i=0; i < MAX_STREAMS; i++) {
        if (pSB->streams[i].bIsSet) {
            pSB->streams[i] = pRc->Stream[i];
        }
    }
    // And fetch index buffer
    if (pSB->indices.bIsSet) {
        pSB->indices = pRc->CurrentIndexBuffer;
    }
#endif

  } // pSB != NULL

  D3DPRINT(10,"<< CaptureStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  ReleaseContextStateBlocks

Description:    Delete any remaining state blocks for cleanup purposes

Return:
-------------------------------------------------------------------*/

VOID
ReleaseContextStateBlocks(RC *pRc)
{
  SETUP_PPDEV(pRc)
  STATEBLOCK  *pSB;
  DWORD       i;


  D3DPRINT(10,">> ReleaseContextStateBlocks");

  if (pRc->ppSBTable)
  {
    // walk over table and free any state blocks still in use
    for (i = 1; i < (DWORD)pRc->ppSBTable[0]; i++)    // entry zero is the array size
    {
      pSB = pRc->ppSBTable[i];
      if (NULL != pSB)
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "  freeing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
                 pSB->dwHandle, pSB, pRc);
        DXFREE(pSB);
      }
    }

    // free state block table
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing ppSBTable=%lXh for pRc=%lXh", pRc->ppSBTable, pRc);
    DXFREE(pRc->ppSBTable);
    pRc->ppSBTable = NULL;
  }

  D3DPRINT(10,"<< ReleaseContextStateBlocks");
}

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

/*-------------------------------------------------------------------
Function Name:  FindStateBlock

Description:    Find a state block identified by dwHandle
                If not found, returns NULL.

Return:
-------------------------------------------------------------------*/

static STATEBLOCK *
FindStateBlock(RC *pRc, DWORD dwHandle)
{
  if (NULL == pRc->ppSBTable)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "No state blocks yet - %ld not found", dwHandle);
    return NULL;
  }

  if (dwHandle < (DWORD)pRc->ppSBTable[0])
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "found SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             dwHandle, pRc->ppSBTable[dwHandle], pRc);
    return pRc->ppSBTable[dwHandle];
  }
  else
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "State block %ld not found (max = %ld)",
             dwHandle, (DWORD)pRc->ppSBTable[0]);
    return NULL;
  }
}

/*-------------------------------------------------------------------
Function Name:  AddStateBlockToTable

Description:    Add an entry to the state block table.
                If necessary, grow the table.

Return:
-------------------------------------------------------------------*/

static HRESULT
AddStateBlockToTable(RC *pRc, DWORD dwHandle, STATEBLOCK *pSB)
{
  SETUP_PPDEV(pRc)
  DWORD       dwNewSize;
  STATEBLOCK  **pNewSBTable;


  // If the current list is not large enough, we'll have to grow a new one
  if ((NULL == pRc->ppSBTable) || (dwHandle > (DWORD)pRc->ppSBTable[0]))
  {
    // New size of our state block table
    // (round up dwHandle in steps of LISTGROW)
    // we need to account for using index zero as a count of how many elements are in
    // the ppSBTable array, so add one to dwHandle
    dwNewSize = (((dwHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;

    // we have to grow our list
    pNewSBTable = (STATEBLOCK **)DXMALLOCZ(dwNewSize*sizeof(STATEBLOCK *));
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pRc=%lX's ppSBTable[%X] size to %lXh",
             pRc, pNewSBTable, dwNewSize);

    if (NULL == pNewSBTable)
    {
      D3DPRINT(0, "AddStateBlockToTable failed to increase SBTable");
      return DDERR_OUTOFMEMORY;
    }

    if (pRc->ppSBTable)
    {
      // if we already had a previous list, we must transfer its data
      // copy counter in element zero plus all STATEBLOCK *'s to new array
      memcpy(pNewSBTable,
             pRc->ppSBTable,
             ((DWORD)pRc->ppSBTable[0] + 1) * sizeof(STATEBLOCK *));

      // and get rid of it
      DXFREE(pRc->ppSBTable);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pRc=%lXh old ppSBTable[%X]",
               pRc, pRc->ppSBTable);
    }

    // New index table data
    pRc->ppSBTable = pNewSBTable;
    // store size in ppSBTable[0]
    // since we're using element zero to hold the array size
    // we only have dwNewSize-1 entries to store STATEBLOCK * elements in
    (DWORD)pRc->ppSBTable[0] = dwNewSize - 1;
  }

  // Store our state set pointer into our access list
  pRc->ppSBTable[dwHandle] = pSB;

  D3DPRINT(NORMAL_DBG_LEVEL,"Store state block, pRc=%lXh SBHandle=%ld pSB = %lXh",
           pRc, dwHandle, pSB);

  return DD_OK;
}

/*-------------------------------------------------------------------
Function Name:  CompressStateBlock

Description:    Compress a state set so it uses the minimum necessary
                space. Since we expect some apps to make extensive use
                of state sets we want to keep things tidy.
                Returns address of new structure (ir old, if it wasn't compressed)

Return:
-------------------------------------------------------------------*/

// Leave compressed stateblock support off until this code gets
// better tested.  This is primarily for DX8 bringup, but stateblocks
// were not tested under DX7 either.
#define ENABLE_COMPRESSED_STATEBLOCKS   0

static STATEBLOCK *
CompressStateBlock(RC *pRc, STATEBLOCK *pUncompressedSB)
{
#if ENABLE_COMPRESSED_STATEBLOCKS
  SETUP_PPDEV(pRc)
  STATEBLOCK  *pCompressedSB;
  DWORD       i, j, dwSize, dwIndex, dwCount;


  D3DPRINT(NORMAL_DBG_LEVEL, "attempting to compress pSB=%lXh", pUncompressedSB);

  // Create a new state set of just the right size we need

  // Calculate how large
  dwCount = 0;
  for (i = 1; i <= MAX_RENDERSTATES; i++)
  {
    if (IS_SB_RS_FLAG_SET(pUncompressedSB, i))
      dwCount++;
  }
  D3DPRINT(NORMAL_DBG_LEVEL, "  stored render state count = %ld", dwCount);

  for (j = 0; j < NUMTEXTUREUNITS; j++)
  {
    for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)
    {
      if (IS_SB_TSS_FLAG_SET(pUncompressedSB, j, i))
        dwCount++;
    }
    D3DPRINT(NORMAL_DBG_LEVEL, "  stored tss[%ld]+renderstate count = %ld", dwCount);
  }
  D3DPRINT(NORMAL_DBG_LEVEL, "  total stored tss + renderstate count = %ld", dwCount);

  // Create a new state set of just the right size we need
  // ANY CHANGE MADE TO THE STATEBLOCK structure MUST BE REFLECTED HERE!
  dwSize = 2*sizeof(DWORD) +                          // handle, compressed flag
           1+(NUMTEXTUREUNITS+1)*sizeof(DWORD) +      // # of RS & TSS[NUMTEXTUREUNITS+1]
           2*dwCount*sizeof(DWORD);                   // compressed structure

  if (dwSize >= sizeof(STATEBLOCK))
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "compressed SB size would be larger than uncompressed SB size");
    D3DPRINT(NORMAL_DBG_LEVEL, "leaving SB uncompressed");

    // it is not efficient to compress, leave uncompressed !
    pUncompressedSB->bCompressed = FALSE;
    return pUncompressedSB;
  }

  pCompressedSB = (STATEBLOCK *)DXMALLOCZ(dwSize);
  if (NULL != pCompressedSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "pCompressedSB=%lXh", pCompressedSB);

    // adjust data in new compressed state set
    pCompressedSB->bCompressed = TRUE;
    pCompressedSB->dwHandle = pUncompressedSB->dwHandle;

    // Transfer our info to this new state set
    pCompressedSB->u.cc.dwNumRS = 0;
    for (i = 0; i < NUMTEXTUREUNITS+1; i++)
      pCompressedSB->u.cc.dwNumTSS[i] = 0;
    dwIndex = 0;

    D3DPRINT(NORMAL_DBG_LEVEL, "  compressing render states");
    for (i = 1; i <= MAX_RENDERSTATES; i++)
    {
      if (IS_SB_RS_FLAG_SET(pUncompressedSB, i))
      {
        pCompressedSB->u.cc.pair[dwIndex].dwType = i;
        pCompressedSB->u.cc.pair[dwIndex].dwValue = pUncompressedSB->u.uc.RenderStates[i];

        D3DPRINT(NORMAL_DBG_LEVEL, "    dwIndex=%ld, state=%ld, data=%ld",
                 dwIndex, pCompressedSB->u.cc.pair[dwIndex].dwType,
                 pCompressedSB->u.cc.pair[dwIndex].dwValue);

        pCompressedSB->u.cc.dwNumRS++;
        dwIndex++;
      }
    }
    D3DPRINT(NORMAL_DBG_LEVEL, "    num compressed render states = %ld", pCompressedSB->u.cc.dwNumRS);

    D3DPRINT(NORMAL_DBG_LEVEL, "  compressing texture stage states");
    for (j = 0; j < NUMTEXTUREUNITS+1; j++)
    {
      for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)
      {
        if (IS_SB_TSS_FLAG_SET(pUncompressedSB, j, i))
        {
          pCompressedSB->u.cc.pair[dwIndex].dwType = i;
          pCompressedSB->u.cc.pair[dwIndex].dwValue = pUncompressedSB->u.uc.TssStates[j][i];

          D3DPRINT(NORMAL_DBG_LEVEL, "    dwIndex=%ld, stage=%ld, state=%ld, data=%ld",
                   dwIndex, pCompressedSB->u.cc.pair[dwIndex].dwType,
                   pCompressedSB->u.cc.pair[dwIndex].dwValue);

          pCompressedSB->u.cc.dwNumTSS[j]++;
          dwIndex++;
        }
      }
      D3DPRINT(NORMAL_DBG_LEVEL, "    num compressed TSS[%ld] states = %ld", j, pCompressedSB->u.cc.dwNumTSS[j]);
    }

    // Get rid of the old(uncompressed) one
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing pUncompressedSB=%lXh", pUncompressedSB);
    DXFREE(pUncompressedSB);
    return pCompressedSB;
  }
  else
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "Not enough memory left to compress state block");
    D3DPRINT(NORMAL_DBG_LEVEL, "leaving SB uncompressed");

    pUncompressedSB->bCompressed = FALSE;
    return pUncompressedSB;
  }
#else
  return pUncompressedSB;
#endif
}
