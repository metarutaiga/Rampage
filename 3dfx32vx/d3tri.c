/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** $Revision: 104$
**
** Log at EOF.
*/


#include <ddrawi.h>
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "d3txtr2.h"
#include "fifomgr.h"
#include "d3tri.h"

#ifdef SLI
#include <ddsli2d.h>
#endif

#include "geglobal.h"

#ifdef CSERVICE
#include <shared.h>
#endif

// Macros for updating the instruction pointer to the next instruction
// in the command buffer
#define NEXTINSTRUCTION(ptr, type, num, extrabytes)                        \
        NEXTINSTRUCTION_S(ptr, sizeof(type), num, extrabytes)

#define NEXTINSTRUCTION_S(ptr, typesize, num, extrabytes)                  \
    ptr = (LPD3DHAL_DP2COMMAND)((LPBYTE)ptr + sizeof(D3DHAL_DP2COMMAND) +  \
                                ((num) * (typesize)) + (extrabytes))

DWORD setDX6state (RC *pRc, DWORD dwhContext, DWORD primitiveType,
                   DWORD count, DWORD fvf, DWORD *fvfotVertex );
DWORD setupTextureStage0( RC *pRc );
DWORD setupTextureStage1( RC *pRc );

#if defined( DEBUG )
void checkTextureOp( RC *pRc );
#endif

#define PARSE_ERROR_AND_EXIT(pDP2Data, pIns, pStartIns, ddrvalue)
#define CHECK_CMDBUF_LIMITS(pDP2Data, pBuf, type, num, extrabytes)
#define CHECK_CMDBUF_LIMITS_S(pDP2Data, pBuf, typesize, num, extrabytes)
#define CHECK_DATABUF_LIMITS(pDP2Data, iIndex)

extern DWORD __stdcall ddiClear2 ( LPD3DHAL_CLEAR2DATA pcd );

extern void __stdcall specular(RC *pRc, ULONG state);
extern void setupTexturing( RC* pRc );
extern void setupTexturingDX8( RC* pRc );

extern RENDERFXN   _renderFuncs[] ;        // defined in d3rstate.c

PFND3DPARSEUNKNOWNCOMMAND dp2Callback = NULL;

// This table is used to compute the maximum number of vertex registers
// we can use in the vertex array based on the size of each vertex.
// There are 256 words of data in the vertex array, so we can hold up to
// 256/va_size vertices (with a cap of 32).
//
//   size_to_mvr[va_size] = MIN(32, 256/va_size);

#define SIZE_TO_MVR_SIZE 64
static const FxU8 size_to_mvr[SIZE_TO_MVR_SIZE] = {
  32, 32, 32, 32, 32, 32, 32, 32,
  28, 25, 23, 21, 19, 18, 17, 16,
  15, 14, 13, 12, 12, 11, 11, 10,
  10,  9,  9,  9,  8,  8,  8,  8,
   7,  7,  7,  7,  6,  6,  6,  6,
   6,  6,  5,  5,  5,  5,  5,  5,
   5,  5,  5,  4,  4,  4,  4,  4,
   4,  4,  4,  4,  4,  4,  4,  4
};


//-------------------------------------------------------------------

ULONG __stdcall calculateLMSFromLODBias(RC *pRc, ULONG uLODBias);
DWORD __stdcall ddiDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA lpdp2d )
{
  SETUP_PPDEV(lpdp2d->dwhContext)
  RC                  *pRc;
  LPDWORD             lpVertices, lpVOffset, lpV0, lpV1;
  LPD3DHAL_DP2COMMAND lpCmd, lpResumeCmd;
  LPBYTE              lpPrim, lpIdx;
  DWORD               cmdEnd, i, j, vertexType, vStart;
  HRESULT             hr = D3D_OK;
  VINDEX              vindex;

  D3D_ENTRY( "ddiDrawPrimitives2" );

#if defined( NULLDRIVER )
  lpdp2d->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
#endif
      
#ifdef FXTRACE
  if (CONTEXT_VALIDATE(lpdp2d->dwhContext)) 
  {
    D3DPRINT( 255, "DrawPrimitives2 bad context =0x08lx", lpdp2d->dwhContext);
    lpdp2d->ddrval = D3DHAL_CONTEXT_BAD;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
 
  pRc = CONTEXT_PTR(lpdp2d->dwhContext);

  if( lpdp2d->dwFlags & D3DHALDP2_USERMEMVERTICES )
    lpVertices = (LPDWORD)((LPBYTE)lpdp2d->lpVertices + lpdp2d->dwVertexOffset);
  else {

    lpVertices = (LPDWORD)((LPBYTE)lpdp2d->lpDDVertex->lpGbl->fpVidMem +
                           lpdp2d->dwVertexOffset);
    pRc->GERC.surfDataVB = (FXSURFACEDATA*)lpdp2d->lpDDVertex->lpGbl->dwReserved1;

    if ( (pRc->GERC.surfDataVB != NULL) &&
         (pRc->GERC.surfDataVB->dwFlags & FXSURFACE_IS_VB) ) {
      // TEMP: for now always set the flag that we need to update the VB
      UPDATE_HW_STATE( SC_GE_VERTEX_BUFFER );
      pRc->GERC.bUseVertexBuffers = TRUE;
      D3DPRINT( 255, "surface is a driver-managed VB" );
    }
  }

  D3DPRINT( 255, "wpVtData = 0x%08lx", lpVertices );
  
  lpCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpdp2d->lpDDCommands->lpGbl->fpVidMem +
                                lpdp2d->dwCommandOffset);
  cmdEnd = (DWORD)lpCmd + lpdp2d->dwCommandLength;

#ifdef CUBEMAP
  // Clear dimension fields before known vertex types bypass FVF generation code.
  memset( &pRc->tCoordDimension[0], 0, sizeof(FxU32)*D3DHAL_TSS_MAXSTAGES);
#endif

  for(;;)
  {
    D3DPRINT( DL_DX6_DP2_OPS, "  DP2 Command =%2d, Count =%d",
              lpCmd->bCommand, lpCmd->wStateCount );

    switch( lpCmd->bCommand )
    {
      case D3DDP2OP_RENDERSTATE :
      {
        D3DPRINT(DL_DX6_DP2_OPS, "  Command RENDERSTATE (%d, %d)",
                  lpCmd->bCommand, lpCmd->wStateCount );
                        
        lpPrim = (LPBYTE)(lpCmd + 1); 
        
        for (i = 0; i < lpCmd->wStateCount; i++, ((LPD3DHAL_DP2RENDERSTATE)lpPrim)++) 
        {        
          D3DRENDERSTATETYPE renderState = ((LPD3DHAL_DP2RENDERSTATE)lpPrim)->RenderState;
          DWORD data                     = ((LPD3DHAL_DP2RENDERSTATE)lpPrim)->dwState;
          
          printRenderState( renderState, data );
                  
          if( IS_OVERRIDE( renderState ) ) 
          {
            DWORD override = GET_OVERRIDE( renderState );

            if ( data ) 
            {
                STATESET_SET( pRc->overrides, override);
            } 
            else 
            {
                STATESET_CLEAR( pRc->overrides, override);
            }
            continue;
          }

          if( STATESET_ISSET( pRc->overrides, renderState ) ) 
          {
            continue;
          }

          {
            // We are going to use renderState to access our array of
            // rendering functions.   Here we account for the possiblity
            // that NT is running some future version of DirectX that we
            // do not have a rendering functions for.  We will test
            // renderState before we use it so that we don't read
            // past the end of the array of rendering functions.
            // If we do not have a renderinf function, we will use
            // the dummy function at index 0.

            D3DRENDERSTATETYPE renderState_Tmp =
                  renderState <= MAX_RENDERSTATES ? renderState : 0;

            // if we're recording a state block just go save the render state
            if (FALSE == pRc->bSBRecMode)
            {
                BYTE    *pRcBytePtr;

                // Here we need to determine if we are going to actually
                // call the render function.  But, if
                // RENDERSTATE_FUNCTION_PATHS is set, we make the call so
                // D3PRINTs are processed.
#ifdef RENDERSTATE_FUNCTION_PATHS
                _renderFuncs[ renderState_Tmp ].renderFXN( pRc, data );
#else
                switch (_renderFuncs[ renderState_Tmp ].callType)
                {
                case RF_ALWAYS_CALL :   // Always make the renderfunction call
                    _renderFuncs[ renderState_Tmp ].renderFXN( pRc, data );
                    break;
                case RF_CHECK_FIRST :
                  // Make the call only if the new state differs from the
                  // current state compute address of pRc field based off
                  // of RC structure address and field offset
                    pRcBytePtr = (BYTE*)(((LONG) pRc) + _renderFuncs[ renderState_Tmp ].fieldIndex);
                    if ( *((ULONG*)pRcBytePtr) != data)
                    {
                        _renderFuncs[ renderState_Tmp ].renderFXN( pRc, data );
                    }
                    break;
                case RF_UPDATE_ONLY :
                  // Only need to update the pRc value (no shadow regs
                  // affected) compute address of pRc field based off of RC
                  // structure address and field offset
                    pRcBytePtr = (BYTE*)(((LONG) pRc) + _renderFuncs[ renderState_Tmp ].fieldIndex);
                    *((ULONG*)pRcBytePtr) = data;
                    break;
                case RF_DUMMY :
                    // if it is a "dummy", then nothing to do.
                    break;
                }
#endif
            }
            else
            {
              // make sure it's a render state we care about
              if ((NULL != pRc->pCurrSB) &&
                    (renderState <= MAX_RENDERSTATES) &&
                    (_renderFuncs[renderState].callType != RF_DUMMY))
              {
                D3DPRINT(RSTATE_DBG_LVL, "    storing STATEBLOCK Renderstate %ld, state = %ld", renderState, data);
                pRc->pCurrSB->u.uc.RenderStates[renderState] = data;
                // set flag to mark this render state is stored in state block
                SET_SB_RS_FLAG(pRc->pCurrSB,renderState);
              }
            }
          }
        }

        lpCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(lpCmd + 1) + (lpCmd->wStateCount * sizeof(D3DHAL_DP2RENDERSTATE)));
        hr = D3D_OK;
        break;
      }

      // Texture Stage States
      case D3DDP2OP_TEXTURESTAGESTATE :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command TEXTURESTAGE (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        lpPrim = (LPBYTE)(lpCmd + 1);

        for(i = 0; i < lpCmd->wStateCount; i++, ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)++ )
        {
          DWORD                     stage = ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)->wStage;
          D3DTEXTURESTAGESTATETYPE  state = ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)->TSState;
          DWORD                     data  = ((LPD3DHAL_DP2TEXTURESTAGESTATE)lpPrim)->dwValue;

          printTextureStageState( stage, state, data );

          // Check for unsupported options for this hardware
          if ( state == D3DTSS_TEXTURETRANSFORMFLAGS )
          {
            if ( (data != (D3DTTFF_PROJECTED | D3DTTFF_COUNT3))  &&     // For projected textures U,V,W
                 (data != (D3DTTFF_COUNT2))                      &&     // For normal U,V coordinates
#ifdef CUBEMAP
                 (data != (D3DTTFF_COUNT3))                      &&     // For cube textures U,V,W
#endif
                 (data != 0) )                                          // For clearing the state
            {
// This is commented out because we are not sure what the error reporting
// mechanism should be when we cannot support the texture dimensionality
// that we are being asked to render with. The attempted error reporting in
// this commented code is not believed to be correct. The
// D3DERR_TEXTURE_NO_SUPPORT simply tells the runtime that we do not do
// textures, so rendering continues without them for EVERYTHING, even when
// we so support it.  Brian Danielson 03/01/2000. BHD.  SPECIAL UPDATE :
// 07/31/2000 - see above in this file for : MC:XXX
//
//                lpdp2d->ddrval = D3DERR_INVALIDVERTEXFORMAT; // D3DERR_TEXTURE_NO_SUPPORT;
//                D3D_EXIT( DDHAL_DRIVER_HANDLED );
            }
          }
          // if we're recording a state block just go save the texture stage state
          if (FALSE == pRc->bSBRecMode)
          {
              ((LPDWORD)&pRc->textureStage[stage])[state] = data;

#if (DIRECT3D_VERSION < 0x0800) || HACK_DX8DDK_BUGS
              if (state == D3DTSS_ADDRESS)
              {
                  ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
                  ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
              }
#endif // DX < 8

              switch( state )
              {
                  case D3DTSS_COLOROP:      // =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
                  case D3DTSS_COLORARG1:    // =  2, /* D3DTA_* (texture arg) */
                  case D3DTSS_COLORARG2:    // =  3, /* D3DTA_* (texture arg) */
                  case D3DTSS_ALPHAOP:      // =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
                  case D3DTSS_ALPHAARG1:    // =  5, /* D3DTA_* (texture arg) */
                  case D3DTSS_ALPHAARG2:    // =  6, /* D3DTA_* (texture arg) */
                      if(pRc->texMapBlend == 0x7ffffffe)
                      {
                         pRc->texMapBlend = D3DTBLEND_MODULATE;
                      }
                      break;

                  case D3DTSS_MIPMAPLODBIAS:
                      ((LPDWORD)&pRc->textureStage[stage])[state] = 
                          calculateLMSFromLODBias( pRc, data ); 
                      break;
              }
		  }
		  else
          {
            if ((NULL != pRc->pCurrSB) &&
                (stage <= NUMTEXTUREUNITS) &&
                (state <= MAX_TEXTURESTAGESTATES))
            {
              D3DPRINT(D3DDBGLVL, "    storing STATEBLOCK stage=%ld  state=%ld  data=%ld", stage, state, data);
              pRc->pCurrSB->u.uc.TssStates[stage][state] = data;
              // set flag to mark this texture stage state is stored in state block
              SET_SB_TSS_FLAG(pRc->pCurrSB,stage,state);

#if (DIRECT3D_VERSION < 0x0800) || HACK_DX8DDK_BUGS
              if (state == D3DTSS_ADDRESS)
              {
                  pRc->pCurrSB->u.uc.TssStates[stage][D3DTSS_ADDRESSU] = data;
                  pRc->pCurrSB->u.uc.TssStates[stage][D3DTSS_ADDRESSV] = data;
                  SET_SB_TSS_FLAG(pRc->pCurrSB,stage,D3DTSS_ADDRESSU);
                  SET_SB_TSS_FLAG(pRc->pCurrSB,stage,D3DTSS_ADDRESSV);
              }
#endif // DX < 8
            }
          }
          ((LPDWORD)&pRc->textureStage[stage])[TSS_CHANGED] = 1;
        }

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(lpCmd + 1) + (lpCmd->wStateCount * sizeof(D3DHAL_DP2TEXTURESTAGESTATE)));
        hr = D3D_OK;
        break;
      }

      case D3DDP2OP_VIEWPORTINFO :
      {
        DWORD   dwMinX, dwMaxX, dwMinY, dwMaxY;

        D3DPRINT( DL_DX6_DP2_OPS, "  Command VIEWPORTINFO (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        lpPrim = (LPBYTE)(lpCmd + 1);
        lpCmd  = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2VIEWPORTINFO*)(lpPrim) + 1);
        
        dwMinX = ((D3DHAL_DP2VIEWPORTINFO*)lpPrim)->dwX;
        dwMaxX = ((D3DHAL_DP2VIEWPORTINFO*)lpPrim)->dwWidth  + dwMinX;
        dwMinY = ((D3DHAL_DP2VIEWPORTINFO*)lpPrim)->dwY;
        dwMaxY = ((D3DHAL_DP2VIEWPORTINFO*)lpPrim)->dwHeight + dwMinY;

        pRc->clipRect.MinX = (D3DVALUE)dwMinX;
        pRc->clipRect.MaxX = (D3DVALUE)dwMaxX;
        pRc->clipRect.MinY = (D3DVALUE)dwMinY;
        pRc->clipRect.MaxY = (D3DVALUE)dwMaxY;

        pRc->sst.suClipMinXMaxX[0].vFxU32 = ( ((dwMinX << SST_SU_CLIP_MIN_SHIFT) & SST_SU_CLIP_MIN) |
                                             ((dwMaxX << SST_SU_CLIP_MAX_SHIFT) & SST_SU_CLIP_MAX)  );
        pRc->sst.suClipMinYMaxY[0].vFxU32 = ( ((dwMinY << SST_SU_CLIP_MIN_SHIFT) & SST_SU_CLIP_MIN) |
                                             ((dwMaxY << SST_SU_CLIP_MAX_SHIFT) & SST_SU_CLIP_MAX)  );
        pRc->sst.suClipEnables.vFxU32 = SST_SU_EN_CLIP0;

        // Setup up the viewport values  (XY offset is set to 0.5 for d3d pixel centers.) :
        pRc->sst.vpMode.vFxU32    = SST_VP_EN_STQ_PROJ;
        pRc->sst.vpSizeX.vFloat   = 1.0f;
        pRc->sst.vpCenterX.vFloat = pRc->pixelOffset;
        pRc->sst.vpSizeY.vFloat   = 1.0f;
        pRc->sst.vpCenterY.vFloat = pRc->pixelOffset;
        pRc->sst.vpSizeZ.vFloat   = 1.0f;
        pRc->sst.vpCenterZ.vFloat = 0.0f;

        UPDATE_HW_STATE(SC_SETUP_UNIT_CLIP | SC_VIEWPORT);
        hr = D3D_OK;
        break;
      }

      case D3DDP2OP_WINFO :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command WINFO (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        lpPrim = (LPBYTE)(lpCmd + 1);
        
        pRc->minW = ((LPD3DHAL_DP2WINFO)lpPrim)->dvWNear;
        pRc->maxW = ((LPD3DHAL_DP2WINFO)lpPrim)->dvWFar;
        pRc->aW = ((pRc->minW * pRc->maxW) / (pRc->minW - pRc->maxW)) * ((1.f / WMAX) - 1.f);
        pRc->bW = (1.f / WMIN) - (pRc->aW / pRc->minW);

        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2WINFO*)(lpPrim) + 1);
        break;
      }
      
      // Rendering of triangles
      
      // Triangle list
      case D3DDP2OP_TRIANGLELIST :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command TRIANGLELIST (%d, %d)",
                  lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Update the starting vertex index
        vStart = ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart;

        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = lpVertices + (vStart * FVFO_SIZE);

        vindex.dw = 0;

        pRc->RndrTriangleList( pRc, lpCmd->wPrimitiveCount, vertexType,
                               vindex, sizeof(WORD), lpVOffset, FVFO_SIZE, vStart);

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLELIST*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed triangle list
      case D3DDP2OP_INDEXEDTRIANGLELIST :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command INDEXEDTRIANGLELIST (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;
        
        // Update pointer to primitive data
        vindex.pb = lpPrim = (LPBYTE)(lpCmd + 1);

        pRc->RndrIndexedTriangleList( pRc, lpCmd->wPrimitiveCount, vertexType,
                                      vindex, sizeof(WORD), lpVOffset, FVFO_SIZE, 0 );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)));
        hr = D3D_OK;
        break;
      }
      
      // Indexed triangle list2
      case D3DDP2OP_INDEXEDTRIANGLELIST2 :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command INDEXEDTRIANGLELIST2 (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );

        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;
                          
        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Update the starting vertex index
        vStart = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = lpVertices + (vStart * FVFO_SIZE);
        vindex.pb = lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

        // Draw the triangle list
        pRc->RndrIndexedTriangleList2( pRc, lpCmd->wPrimitiveCount, vertexType,
                                       vindex, sizeof(WORD), lpVOffset, FVFO_SIZE, vStart );

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2)));

        hr = D3D_OK;
        break;
      }
      // Triangle strip
      case D3DDP2OP_TRIANGLESTRIP :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command TRIANGLESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Update the starting vertex index
        vStart = ((LPD3DHAL_DP2TRIANGLESTRIP)lpPrim)->wVStart;

        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = lpVertices + (vStart * FVFO_SIZE);
        vindex.pb = 0;
        pRc->RndrTriangleStrip( pRc, lpCmd->wPrimitiveCount, vertexType,
                                vindex, sizeof(WORD), lpVOffset, FVFO_SIZE, vStart );
        
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLESTRIP*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // triangle indexed strip
      case D3DDP2OP_INDEXEDTRIANGLESTRIP :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command INDEXEDTRIANGLESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Update the starting vertex index
        vStart = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = lpVertices + (vStart * FVFO_SIZE);
        vindex.pb = lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);

        pRc->RndrIndexedTriangleStrip( pRc, lpCmd->wPrimitiveCount, vertexType,
                                       vindex, sizeof(WORD), lpVOffset,
                                       FVFO_SIZE, vStart);

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPWORD)lpPrim + (lpCmd->wPrimitiveCount + 2));
        hr = D3D_OK;
        break;
      }      

      // Rendering fans
      
      // Immediate data fans with data embedded in the command stream
      case D3DDP2OP_TRIANGLEFAN_IMM :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command TRIANGLEAN_IMM (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Check primtive data pinter in command stream is DWORD aligned
        lpPrim = (LPBYTE)(((DWORD)(lpCmd + 1) + 3 ) & ~3);
        
        // Move primitive data pointer past edge flags
        lpPrim += sizeof(D3DHAL_DP2TRIANGLEFAN_IMM);

        vindex.dw = 0;
        pRc->RndrTriangleFanImm( pRc, lpCmd->wPrimitiveCount, vertexType,
                                 vindex, sizeof(WORD), (LPDWORD)lpPrim, FVFO_SIZE, 0);

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrim + ((lpCmd->wPrimitiveCount + 2) * FVFO_SIZE) );
        hr = D3D_OK;
        break;
      }

      // Fans
      case D3DDP2OP_TRIANGLEFAN :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command TRIANGLEFAN (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Update the starting vertex index
        vStart = ((LPD3DHAL_DP2TRIANGLEFAN)lpPrim)->wVStart;

        // Read the first vertex offset value
        lpVOffset = lpVertices + (vStart * FVFO_SIZE);
        vindex.dw = 0;
        pRc->RndrTriangleFan( pRc, lpCmd->wPrimitiveCount, vertexType,
                              vindex, sizeof(WORD), lpVOffset, FVFO_SIZE, vStart);

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2TRIANGLEFAN*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed Fans
      case D3DDP2OP_INDEXEDTRIANGLEFAN :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command INDEXEDTRIANGLEFAN (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Update the starting vertex index
        vStart = ((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart;

        // Read the vertex offset value and adjust the primtive data pointer
        lpVOffset = lpVertices + (vStart * FVFO_SIZE);
        vindex.pb = lpPrim += sizeof(D3DHAL_DP2STARTVERTEX);

        pRc->RndrIndexedTriangleFan( pRc, lpCmd->wPrimitiveCount, vertexType,
                                     vindex, sizeof(WORD), lpVOffset, FVFO_SIZE, vStart);

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPWORD)lpPrim + (lpCmd->wPrimitiveCount + 2));
        hr = D3D_OK;
        break;
      }            

      // Rendering of points
      
      case D3DDP2OP_POINTS :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command POINTS (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Start our wPrimiveCount loop
        for(i = 0; i < lpCmd->wPrimitiveCount; i++)
        {
          // Read the first vertex offset value
          lpVOffset = lpVertices + (((LPD3DHAL_DP2POINTS)lpPrim)->wVStart * FVFO_SIZE);

          // Start our point loop
          for( j = 0; j < ((LPD3DHAL_DP2POINTS)lpPrim)->wCount; j++ )
          {
            // Draw a point and update vertex pointer to the next vertex point
            pRc->RndrPoints( pRc, lpVOffset, lpVOffset, vertexType );
            lpVOffset += FVFO_SIZE;          
          }

          // Update the primitive data pointer to the next point primitive          
          lpPrim += sizeof(D3DHAL_DP2POINTS);
        }
        
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2POINTS*)(lpCmd + 1) + lpCmd->wPrimitiveCount);
        hr = D3D_OK;
        break;
      }

      // Rendering of lines
      
      // Line list
      case D3DDP2OP_LINELIST :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command LINELIST (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );

        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;
        
        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Get the start vertex offset and update the primitive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2LINELIST)lpPrim)->wVStart * FVFO_SIZE);
        lpPrim = lpPrim + sizeof(D3DHAL_DP2LINELIST);

        // Get pointer to first vertex
        lpV0 = lpVOffset;

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
            lpV1 = lpV0 + FVFO_SIZE;
            // draw a line from lpV0 to lpV1
            pRc->RndrLineList( pRc, lpV0, lpV0, lpV1, vertexType );
            lpV0 += 2*FVFO_SIZE;
         }
                
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2LINELIST*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed line list
      case D3DDP2OP_INDEXEDLINELIST :
      {
        D3DPRINT( 0, "  <WARNING> Unsupported DP2 command - INDEXEDLINELIST (%d, %d)", lpCmd->bCommand, lpCmd->wPrimitiveCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;
        
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2INDEXEDLINELIST*)(lpCmd + 1) + lpCmd->wPrimitiveCount);
        hr = D3D_OK;
        break;
      }

      // Indexed line list2
      case D3DDP2OP_INDEXEDLINELIST2 :
        D3DPRINT( 0, "  <WARNING> Unsupported DP2 command - INDEXEDLINELIST2 (%d, %d)", lpCmd->bCommand, lpCmd->wPrimitiveCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;
        
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(lpCmd + 1) + sizeof(D3DHAL_DP2STARTVERTEX) +
          (lpCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST)));
        hr = D3D_OK;
        break;
        
      // Line strip
      case D3DDP2OP_LINESTRIP :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command LINESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Get the start vertex offset and update the primitive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2LINELIST)lpPrim)->wVStart * FVFO_SIZE);
        lpPrim = lpPrim + sizeof(D3DHAL_DP2LINELIST);

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Get pointer to first vertex
          lpV0 = lpVOffset;
          lpVOffset += FVFO_SIZE;
          
          // Get pointer to second vertex          
          lpV1 = lpVOffset;
          
          // Draw a line          
          pRc->RndrLineStrip( pRc, lpV0, lpV0, lpV1, vertexType );
        }

        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((D3DHAL_DP2LINESTRIP*)(lpCmd + 1) + 1);
        hr = D3D_OK;
        break;
      }

      // Indexed line strip
      case D3DDP2OP_INDEXEDLINESTRIP :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command INDEXEDLINESTRIP (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;

        // Update pointer to primitive data
        lpPrim = (LPBYTE)(lpCmd + 1);

        // Get the vertex start index offset and update the primitive data pointer
        lpVOffset = lpVertices + (((LPD3DHAL_DP2STARTVERTEX)lpPrim)->wVStart * FVFO_SIZE);
        lpPrim = lpPrim + sizeof(D3DHAL_DP2STARTVERTEX);

        // Keep a copy of the index offset pointer and read the first index
        lpIdx = lpPrim;        
        lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]) * FVFO_SIZE];

        // Now loop through each line
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Move the last vertex pointer into the first
          lpV0 = lpV1;

          // Now get the next vertex pointer, adjusting the index offset pointer
          // if we have read the last vertex pointer in the group of two
          if( i & 1 )
          {
            lpIdx += sizeof(D3DHAL_DP2INDEXEDLINESTRIP);
            lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[0]) * FVFO_SIZE];
          }
          else
            lpV1 = &lpVOffset[(((LPD3DHAL_DP2INDEXEDLINESTRIP)lpIdx)->wV[1]) * FVFO_SIZE];

          // Draw a line          
          pRc->RndrIndexedLineStrip( pRc, lpV0, lpV0, lpV1, vertexType );
        }
        
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)(lpPrim + (sizeof(WORD) * (lpCmd->wPrimitiveCount + 1)));
        hr = D3D_OK;
        break;
      }
      
      // Line list using data embedded in command stream
      case D3DDP2OP_LINELIST_IMM :
      {
        D3DPRINT( DL_DX6_DP2_OPS, "  Command LINELIST_IMM (%d, %d)", lpCmd->bCommand, lpCmd->wStateCount );
        
        // Setup the hardware state
        hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, lpdp2d->dwVertexType, &vertexType );
        if (D3D_OK != hr)
          break;
        
        // Check primtive data pinter in command stream is DWORD aligned
        lpPrim = (LPBYTE)(((DWORD)(lpCmd + 1) + 3) & ~3);

        // Get the first vertex pointer
        lpV1 = (LPDWORD)lpPrim;
        
        for( i = 0; i < lpCmd->wPrimitiveCount; i++ )
        {
          // Copy the last vertex pointer into the first and then get
          // the next vertex pointer
          
          // 2/25/00 miles - I suspect this is incorrect but I don't have way to test it.  
          // I believe it should look similar to the LINELIST case above where the lpVO index
          // gets bumped up by 2*FVFO_SIZE each pass through the loop. This looks like it will
          // draw a strip instead. ???
          lpV0 = lpV1;
          lpV1 = lpV0 + FVFO_SIZE;
          
          pRc->RndrLineListImm( pRc, lpV0, lpV0, lpV1, vertexType );
        }
        
        // Update the command stream pointer
        lpCmd = (LPD3DHAL_DP2COMMAND)((LPDWORD)lpPrim + ((lpCmd->wPrimitiveCount + 1) * FVFO_SIZE));
        hr = D3D_OK;
        break;
      }
      
      case D3DDP2OP_TEXBLT:
      {
        DWORD                   hSrcSurf, hDstSurf;
        D3DHAL_DP2TEXBLT       *pTexBlt;
        txtrDesc               *pTxtrSrc, *pTxtrDst;
        RECTL                   srcRect;
        POINT                   dstPoint;
        int k;

        // Inform the driver to perform a BitBlt operation from a source
        // texture to a destination texture. A texture can also be cubic
        // environment map. The driver should copy a rectangle specified
        // by rSrc in the source texture to the location specified by pDest
        // in the destination texture. The destination and source textures
        // are identified by handles that the driver was notified with
        // during texture creation time. If the driver is capable of
        // managing textures, then it is possible that the destination
        // handle is 0. This indicates to the driver that it should preload
        // the texture into video memory (or wherever the hardware
        // efficiently textures from). In this case, it can ignore rSrc and
        // pDest. Note that for mipmapped textures, only one D3DDP2OP_TEXBLT
        // instruction is inserted into the D3dDrawPrimitives2 command stream.
        // In this case, the driver is expected to BitBlt all the mipmap
        // levels present in the texture.

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_TEXBLT (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        lpPrim = (LPBYTE)(lpCmd + 1);

        for (k=0; k<lpCmd->wPrimitiveCount; k++) {
            pTexBlt = (D3DHAL_DP2TEXBLT *)lpPrim;

            CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2TEXBLT, lpCmd->wStateCount, 0);

            hSrcSurf = pTexBlt->dwDDSrcSurface;
            hDstSurf = pTexBlt->dwDDDestSurface;
            pTxtrSrc = TXTRHNDL_TO_TXTRDESC(pRc, hSrcSurf);
            pTxtrDst = TXTRHNDL_TO_TXTRDESC(pRc, hDstSurf);

            // copy data from rect and point so we dont modify the command stream
            srcRect.left   = pTexBlt->rSrc.left;
            srcRect.top    = pTexBlt->rSrc.top;
            srcRect.right  = pTexBlt->rSrc.right;
            srcRect.bottom = pTexBlt->rSrc.bottom;
            dstPoint.x     = pTexBlt->pDest.x;
            dstPoint.y     = pTexBlt->pDest.y;

            if ((pTxtrSrc != 0) && (pTxtrDst != 0))
            {
                // call TEXTURELOAD to download map or mipmap levels
                hr = txtrLoadDp2(ppdev, pTxtrSrc, &srcRect, pTxtrDst, &dstPoint);
                if (D3D_OK != hr)
	                break;
            }
            lpPrim += sizeof(D3DHAL_DP2TEXBLT);
        }
        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2TEXBLT, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_SETPALETTE:
      {
        // Attach a palette to a texture, that is, map an association
        // between a palette handle and a surface handle, and specify
        // the characteristics of the palette. The number of
        // D3DNTHAL_DP2SETPALETTE structures to follow is specified by
        // the wStateCount member of the D3DNTHAL_DP2COMMAND structure

        D3DHAL_DP2SETPALETTE  *pSetPal;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETPALETTE (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETPALETTE, lpCmd->wStateCount, 0);

        pSetPal = (D3DHAL_DP2SETPALETTE *)lpPrim;

        for (i = 0; i < lpCmd->wStateCount; i++, pSetPal++)
        {
          hr = PaletteSet(pRc,
                          pSetPal->dwSurfaceHandle,
                          pSetPal->dwPaletteHandle,
                          pSetPal->dwPaletteFlags);
          if (D3D_OK != hr)
            break;
        }

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETPALETTE, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_UPDATEPALETTE:
      {
        // Perform modifications to the palette that is used for palettized
        // textures. The palette handle attached to a surface is updated
        // with wNumEntries PALETTEENTRYs starting at a specific wStartIndex
        // member of the palette. (A PALETTENTRY (defined in wingdi.h and
        // wtypes.h) is actually a DWORD with an ARGB color for each byte.)
        // After the D3DNTHAL_DP2UPDATEPALETTE structure in the command
        // stream the actual palette data will follow (without any padding),
        // comprising one DWORD per palette entry. There will only be one
        // D3DNTHAL_DP2UPDATEPALETTE structure (plus palette data) following
        // the D3DNTHAL_DP2COMMAND structure regardless of the value of
        // wStateCount.

        D3DHAL_DP2UPDATEPALETTE *pUpdatePal;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_UPDATEPALETTE");

        lpPrim = (LPBYTE)(lpCmd + 1);
        pUpdatePal = (D3DHAL_DP2UPDATEPALETTE *)lpPrim;

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2UPDATEPALETTE, 1, pUpdatePal->wNumEntries * sizeof(PALETTEENTRY));

        // We will ALWAYS have only 1 palette update structure + palette
        // following the D3DDP2OP_UPDATEPALETTE token

        hr = PaletteUpdate(pRc,
                           pUpdatePal->dwPaletteHandle,
                           pUpdatePal->wStartIndex,
                           pUpdatePal->wNumEntries,
                           (BYTE*)(pUpdatePal+1));

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2UPDATEPALETTE, 1,
                        ((D3DHAL_DP2UPDATEPALETTE *)lpPrim)->wNumEntries*sizeof(PALETTEENTRY));
        break;
      }

      case D3DDP2OP_SETRENDERTARGET:
      {
        // Map a new rendering target surface and depth buffer in
        // the current context.  This replaces the old mySetRenderTarget32
        // callback.

        D3DHAL_DP2SETRENDERTARGET *pSRTData;
        DWORD hRenderTarg, hZBuff;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETRENDERTARGET");

        lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, D3DHAL_DP2SETRENDERTARGET, lpCmd->wStateCount, 0);

        // get new data by ignoring all but the last structure
        pSRTData = (D3DHAL_DP2SETRENDERTARGET *)lpPrim + (lpCmd->wStateCount - 1);

        hRenderTarg = pSRTData->hRenderTarget;

        switch( _DD(ddFSAAMode)  )
        {
            case  FSAA_MODE_4XBLT:
            case  FSAA_MODE_DEMO: 
                hRenderTarg = 2; // force the use of "backbuffer 1"
                break;
            default:
                // this is the default case for all non-aa rendering
                hRenderTarg = pSRTData->hRenderTarget;
                break;
        }
    
        hZBuff = pSRTData->hZBuffer;
        if (hZBuff)
        {
            hr = SetRenderTarget(pRc, hRenderTarg, hZBuff);
        }
        else
        {
            hr = SetRenderTarget(pRc, hRenderTarg, (DWORD) NULL);
        }

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETRENDERTARGET, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_CLEAR:
      {
        // Perform hardware-assisted clearing on the rendering target,
        // depth buffer or stencil buffer. This replaces the old ddiClear
        // and ddiClear2 callbacks.

        D3DHAL_DP2CLEAR    *pClear;
        D3DHAL_CLEAR2DATA   cd;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_CLEAR");

        lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, RECT, lpCmd->wStateCount, (sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT)));

        pClear = (D3DHAL_DP2CLEAR *)lpPrim;

#ifdef SLI
        if (SLI_MODE_ENABLED == _DD(sliMode))
        {
            hr = SliClear(pRc,
                      pClear->dwFlags,
                      pClear->dwFillColor,           // render target fill color
                      pClear->dvFillDepth,           // z buffer fill value
                      pClear->dwFillStencil,         // stencil buffer fill value
                      (LPRECT)((LPBYTE)pClear + sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT)),  // ptr to rects
                      (DWORD)lpCmd->wStateCount);    // number of rects
        }
        else
#endif
        {
            cd.dwhContext     = lpdp2d->dwhContext;
            cd.dwFlags        = pClear->dwFlags;
            cd.dwFillColor    = pClear->dwFillColor;
            cd.dvFillDepth    = pClear->dvFillDepth;
            cd.dwFillStencil  = pClear->dwFillStencil;
            cd.lpRects        = (LPD3DRECT)((LPBYTE)pClear + sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT));
            cd.dwNumRects     = (DWORD)lpCmd->wStateCount;

            hr = ddiClear2 (&cd);   // Call through the DX6 Clear2 routine (this return value ignored).
            hr = cd.ddrval;
        }
        NEXTINSTRUCTION(lpCmd, RECT, lpCmd->wStateCount, (sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT)));
        break;
      }

      case D3DDP2OP_STATESET:
      {
        D3DHAL_DP2STATESET *pStateSetOp;

        D3DPRINT(D3DDBGLVL, "D3DDP2OP_STATESET (wPrimitiveCount=%ld)",
                 lpCmd->wPrimitiveCount);

        lpPrim = (LPBYTE)(lpCmd + 1);

        CHECK_CMDBUF_LIMITS(lpdp2d, lpPrim, tempD3DHAL_DP2STATESET, lpCmd->wStateCount, 0);

        pStateSetOp = (D3DHAL_DP2STATESET *)lpPrim;

        for (i = 0; i < lpCmd->wStateCount; i++, pStateSetOp++)
        {
          switch (pStateSetOp->dwOperation)
          {
            case D3DHAL_STATESETBEGIN  :
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETBEGIN  dwHandle=%ld", pStateSetOp->dwParam);
              hr = BeginStateBlock(pRc,pStateSetOp->dwParam);
              break;
            case D3DHAL_STATESETEND    :
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETEND");
              hr = EndStateBlock(pRc);
              break;
            case D3DHAL_STATESETDELETE :
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETDELETE  dwHandle=%ld", pStateSetOp->dwParam);
              hr = DeleteStateBlock(pRc,pStateSetOp->dwParam);
              break;
            case D3DHAL_STATESETEXECUTE:
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETEXECUTE  dwHandle=%ld", pStateSetOp->dwParam);
              hr = ExecuteStateBlock(pRc,pStateSetOp->dwParam);
              break;
            case D3DHAL_STATESETCAPTURE:
              D3DPRINT(D3DDBGLVL, "  D3DHAL_STATESETCAPTURE  dwHandle=%ld", pStateSetOp->dwParam);
              hr = CaptureStateBlock(pRc,pStateSetOp->dwParam);
              break;
            default :
              D3DPRINT(0, "D3DDP2OP_STATESET has invalid dwOperation %8lXh",
                       pStateSetOp->dwOperation);
              break;
          }
          if (D3D_OK != hr)
            break;
        }

        NEXTINSTRUCTION(lpCmd, D3DHAL_DP2STATESET, lpCmd->wStateCount, 0);
        break;
      }

      case D3DDP2OP_ZRANGE:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_ZRANGE");

		hr = GE_SetZRange(pRc, &lpCmd);
        break;

      case D3DDP2OP_SETMATERIAL:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETMATERIAL");

        hr = GE_SetMaterial(pRc, &lpCmd);
        break;

      case D3DDP2OP_SETLIGHT:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETLIGHT");

        hr = GE_SetLight(pRc, &lpCmd);
        break;

      case D3DDP2OP_CREATELIGHT:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_CREATELIGHT");
		
		hr = GE_CreateLight(pRc, &lpCmd);
        break;

      case D3DDP2OP_SETTRANSFORM:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETTRANSFORM");

        hr = GE_SetTransform(pRc, &lpCmd);
        break;

      case D3DDP2OP_EXT:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_EXT");

        lpCmd = (LPD3DHAL_DP2COMMAND)
               ((D3DHAL_DP2EXT *)(lpCmd + 1) + lpCmd->wStateCount);
        hr = D3D_OK;
        break;

      case D3DDP2OP_SETCLIPPLANE:
        D3DPRINT(D3DDBGLVL, "D3DDP2OP_SETCLIPPLANE");

        hr = GE_SetClipPlane(pRc, &lpCmd);
	    break;

#if (DIRECT3D_VERSION >= 0x0800)

      //---------------- Vertex Shader ----------------
      case D3DDP2OP_CREATEVERTEXSHADER:
      {
          D3DHAL_DP2CREATEVERTEXSHADER *cvs;
          lpPrim = (LPBYTE)(lpCmd + 1);
          cvs = (D3DHAL_DP2CREATEVERTEXSHADER *)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_CREATEVERTEXSHADER - unrecognized!!  (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  CreateVertexShader: Handle = %3d DeclSize = 0x%8x CodeSize = 0x%8x",
                   cvs->dwHandle, cvs->dwDeclSize, cvs->dwCodeSize);
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2CREATEVERTEXSHADER, 1,
                          cvs->dwDeclSize + cvs->dwCodeSize);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_DELETEVERTEXSHADER:
      {
          D3DHAL_DP2VERTEXSHADER *vs;
          lpPrim = (LPBYTE)(lpCmd + 1);
          vs = (D3DHAL_DP2VERTEXSHADER*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DELETEVERTEXSHADER - unrecognized!!  (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  DeleteVertexShader: dwHandle = %3d", vs->dwHandle);
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2VERTEXSHADER, 1, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_SETVERTEXSHADER:
      {
          D3DHAL_DP2VERTEXSHADER *vs;
          lpPrim = (LPBYTE)(lpCmd + 1);
          vs = (D3DHAL_DP2VERTEXSHADER*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETVERTEXSHADER - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  SetVertexShader: Handle aka FVF flags = 0x%08x", vs->dwHandle);
          pRc->dwDX8fvf = vs->dwHandle;     // FVF flags since driver doesn't support
                                            // actual vertex shaders yet.
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2VERTEXSHADER, 1, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_SETVERTEXSHADERCONST:
      {
          D3DHAL_DP2SETVERTEXSHADERCONST *svsc;
          DWORD total_size = 0, count = lpCmd->wStateCount;
          lpPrim = (LPBYTE)(lpCmd + 1);
          svsc = (D3DHAL_DP2SETVERTEXSHADERCONST*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETVERTEXSHADERCONST - unrecognized!!  (%d prims)",
                   count);
          while (count--)
          {
              D3DPRINT(DL_DX8_DP2, "  SetVertexShaderConst: Register=%d Count=%d",
                       svsc->dwRegister, svsc->dwCount);
              total_size += svsc->dwCount * 4 * sizeof(float);
              svsc = (D3DHAL_DP2SETVERTEXSHADERCONST*)
                  ((LPBYTE)(svsc+1) + svsc->dwCount * 4 * sizeof(float));
          }
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETVERTEXSHADERCONST, 0, total_size);
          hr = D3D_OK;
          break;
      }

      //---------------- Pixel Shader ----------------
      case D3DDP2OP_CREATEPIXELSHADER:
      {
          D3DHAL_DP2CREATEPIXELSHADER *cps;
          PSC_PIXEL_SHADER *pstemp;
          lpPrim = (LPBYTE)(lpCmd + 1);
          cps = (D3DHAL_DP2CREATEPIXELSHADER*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_CREATEPIXELSHADER - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  CreatePixelShader: Handle=%d CodeSize=%d",
                   cps->dwHandle, cps->dwCodeSize);

          pstemp = (PSC_PIXEL_SHADER*)DXMALLOCZ(sizeof(PSC_PIXEL_SHADER));
          //if (pRc->pPixelShaders[cps->dwHandle] != NULL)
          //    DXFREE(pRc->pPixelShaders[cps->dwHandle]);
          pRc->pPixelShaders[cps->dwHandle] = pstemp;
          PSCCreatePixelShader (pRc->pPscContext, cps->dwCodeSize,
                                (LPDWORD)(cps+1), pstemp);

          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2CREATEPIXELSHADER, lpCmd->wStateCount,
                          cps->dwCodeSize);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_DELETEPIXELSHADER:
      {
          D3DHAL_DP2PIXELSHADER *dps;
          lpPrim = (LPBYTE)(lpCmd + 1);
          dps = (D3DHAL_DP2PIXELSHADER*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DELETEPIXELSHADER - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  DeletePixelShader: Handle=%d",
                   dps->dwHandle);

          if (pRc->pPixelShaders[dps->dwHandle] != NULL)
              DXFREE(pRc->pPixelShaders[dps->dwHandle]);
          if (dps->dwHandle == pRc->dwCurrentPShader)
              pRc->dwCurrentPShader = 0;    // i.e., use DX7 pipeline setup

          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2PIXELSHADER, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_SETPIXELSHADER:
      {
          D3DHAL_DP2PIXELSHADER *sps;
          lpPrim = (LPBYTE)(lpCmd + 1);
          sps = (D3DHAL_DP2PIXELSHADER*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETPIXELSHADER - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  SetPixelShader: Handle=%d",
                   sps->dwHandle);

          if (sps->dwHandle != pRc->dwCurrentPShader)
          {
              pRc->dwCurrentPShader = sps->dwHandle;
              pRc->bPscDirty = TRUE;
              // If a zero handle, force a DX7 reconfiguration.
              TS[0].changed = TS[0].changed || !sps->dwHandle;
          }
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2PIXELSHADER, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_SETPIXELSHADERCONST:
      {
          D3DHAL_DP2SETPIXELSHADERCONST *spsc;
          DWORD total_size = 0, count = lpCmd->wStateCount;
          lpPrim = (LPBYTE)(lpCmd + 1);
          spsc = (D3DHAL_DP2SETPIXELSHADERCONST*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETPIXELSHADERCONST - (%d prims)",
                   count);
          while (count--)
          {
              D3DPRINT(DL_DX8_DP2, "  SetPixelShaderConst: Register=%d Count=%d",
                       spsc->dwRegister, spsc->dwCount);
              PSCSetPixelShaderConst (pRc->pPscContext, spsc->dwRegister,
                                      spsc->dwCount, (float*)(spsc+1));
              pRc->bPscDirty = TRUE;
              total_size += (spsc->dwCount * 4 * sizeof(float)
                             + sizeof(D3DHAL_DP2SETPIXELSHADERCONST));
              spsc = (D3DHAL_DP2SETPIXELSHADERCONST*)
                  ((LPBYTE)(spsc+1) + spsc->dwCount * 4 * sizeof(float));
          }
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETPIXELSHADERCONST, 0, total_size);
          hr = D3D_OK;
          break;
      }

      // ---------------- Vertex and Index Streams ----------------
      case D3DDP2OP_SETSTREAMSOURCE:
      {
          D3DHAL_DP2SETSTREAMSOURCE *sss;
          FXSURFACEDATA *fxsd;
          lpPrim = (LPBYTE)(lpCmd + 1);
          sss = (D3DHAL_DP2SETSTREAMSOURCE*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETSTREAMSOURCE - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  SetStreamSource: Stream=%d VBHandle=%d Stride=0x%04x",
                   sss->dwStream, sss->dwVBHandle, sss->dwStride);

          // sss->dwStream is always zero b/c only 1 stream is supported.
          if (sss->dwVBHandle) {
              fxsd = TXTRHNDL_TO_SURFDATA(pRc, sss->dwVBHandle);
              pRc->pStream[sss->dwStream] = (void*)(fxsd ? fxsd->lfbPtr : 0);
          } else {
              pRc->pStream[sss->dwStream] = 0;
          }
          pRc->Stream[sss->dwStream].dwHandle = sss->dwVBHandle;
          pRc->Stream[sss->dwStream].dwStride = sss->dwStride;
          if (pRc->bSBRecMode) {
              pRc->pCurrSB->streams[sss->dwStream].dwHandle = sss->dwVBHandle;
              pRc->pCurrSB->streams[sss->dwStream].dwStride = sss->dwStride;
              pRc->pCurrSB->streams[sss->dwStream].bIsSet   = TRUE;
          }
          D3DPRINT(DL_DX8_DP2, "  Stream Zero = 0x%08x", pRc->pStream[sss->dwStream]);

          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETSTREAMSOURCE, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_SETSTREAMSOURCEUM:
      {
          D3DHAL_DP2SETSTREAMSOURCEUM *sss;
          lpPrim = (LPBYTE)(lpCmd + 1);
          sss = (D3DHAL_DP2SETSTREAMSOURCEUM*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETSTREAMSOURCEUM - unrecognized!!  (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  SetStreamSourceUm: Stream=%d Stride=0x%04x",
                   sss->dwStream, sss->dwStride);
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETSTREAMSOURCEUM, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_SETINDICES:
      {
          D3DHAL_DP2SETINDICES *si;
          FXSURFACEDATA *fxsd;
          lpPrim = (LPBYTE)(lpCmd + 1);
          si = (D3DHAL_DP2SETINDICES*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_SETINDICES - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  SetIndices: VBHandle=%d Stride=0x%04x",
                   si->dwVBHandle, si->dwStride);
          // currently only use 16-bit indices...change in ddinit32.c/d3init.c
          if (si->dwVBHandle) {
              fxsd = TXTRHNDL_TO_SURFDATA(pRc, si->dwVBHandle);
              pRc->pCurrentIndexBuffer = (LPBYTE)(fxsd ? fxsd->lfbPtr : 0);
          } else
              pRc->pCurrentIndexBuffer = 0;
          pRc->CurrentIndexBuffer.dwHandle = si->dwVBHandle;
          pRc->CurrentIndexBuffer.dwStride = si->dwStride;
          if (pRc->bSBRecMode) {
              pRc->pCurrSB->indices.dwHandle = si->dwVBHandle;
              pRc->pCurrSB->indices.dwStride = si->dwStride;
              pRc->pCurrSB->indices.bIsSet   = TRUE;
          }

          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2SETINDICES, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }

      //---------------- Untransformed Primitives (T&L) ----------------
      case D3DDP2OP_DRAWPRIMITIVE:
      {
          D3DHAL_DP2DRAWPRIMITIVE *dp;
          lpPrim = (LPBYTE)(lpCmd + 1);
          dp = (D3DHAL_DP2DRAWPRIMITIVE*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DRAWPRIMITIVE - unrecognized!!  (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  DrawPrimitive: Type=%d VStart=%d PrimCount=%d",
                   dp->primType, dp->VStart, dp->PrimitiveCount);
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWPRIMITIVE, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }
      case D3DDP2OP_DRAWINDEXEDPRIMITIVE:
      {
          D3DHAL_DP2DRAWINDEXEDPRIMITIVE *dip;
          lpPrim = (LPBYTE)(lpCmd + 1);
          dip = (D3DHAL_DP2DRAWINDEXEDPRIMITIVE*)lpPrim;
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DRAWINDEXEDPRIMITIVE - unrecognized!!  (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  DrawIndexedPrimitive: Type=%d BaseVertIdx=%d Min=%d NumV=%d StartIdx=%d Count=%d",
                   dip->primType,
                   dip->BaseVertexIndex,
                   dip->MinIndex,
                   dip->NumVertices,
                   dip->StartIndex,
                   dip->PrimitiveCount);
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWINDEXEDPRIMITIVE, lpCmd->wStateCount, 0);
          hr = D3D_OK;
          break;
      }

      //---------------- Transformed Primitives ----------------
      case D3DDP2OP_DRAWPRIMITIVE2:
      {
          D3DHAL_DP2DRAWPRIMITIVE2 *dp2 = (D3DHAL_DP2DRAWPRIMITIVE2*)(lpCmd+1);

          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DRAWPRIMITIVE2 - (%d prims)", lpCmd->wStateCount);
          D3DPRINT(0, "  DrawPrimitive2: Type=%d FirstVOffset=0x%x Count=%d",
                   dp2->primType, dp2->FirstVertexOffset, dp2->PrimitiveCount);

          hr = setDX6state (pRc, lpdp2d->dwhContext, 0, 0, pRc->dwDX8fvf, &vertexType);
          if (hr == D3D_OK)
          {
              for (i=0; i < lpCmd->wStateCount; i++, dp2++)
              {
                  DWORD vstride = pRc->Stream[0].dwStride / sizeof(DWORD);
                  vindex.dw = 0;

                  lpVOffset = (LPDWORD)(pRc->pStream[0] + dp2->FirstVertexOffset);
                  switch (dp2->primType) {
                    case D3DPT_POINTLIST:
                      D3DPRINT(DL_DX8_DP2, "  DP2:PointList: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrSprites( pRc, dp2->PrimitiveCount, vertexType,
                                        vindex, 0, lpVOffset, vstride,
                                        dp2->FirstVertexOffset);
                      break;
                    case D3DPT_LINELIST:
                      D3DPRINT(DL_DX8_DP2, "  DP2:LineList: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      while (dp2->PrimitiveCount-- > 0)
                      {
                          pRc->RndrLineList( pRc, lpVOffset, lpVOffset,
                                             lpVOffset + vstride, vertexType);
                          lpVOffset += vstride * 2;
                      }
                      break;
                    case D3DPT_LINESTRIP:
                      D3DPRINT(DL_DX8_DP2, "  DP2:LineStrip: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      while (dp2->PrimitiveCount-- > 0)
                      {
                          pRc->RndrLineList (pRc, lpVOffset, lpVOffset,
                                             lpVOffset + vstride, vertexType);
                          lpVOffset += vstride;
                      }
                      break;
                    case D3DPT_TRIANGLELIST:
                      D3DPRINT(DL_DX8_DP2, "  DP2:TriangleList: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrTriangleList (pRc, dp2->PrimitiveCount, vertexType,
                                             vindex, 0, lpVOffset, vstride,
                                             dp2->FirstVertexOffset);
                      break;
                    case D3DPT_TRIANGLESTRIP:
                      D3DPRINT(DL_DX8_DP2, "  DP2:TriangleStrip: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrTriangleStrip (pRc, dp2->PrimitiveCount, vertexType,
                                              vindex, 0, lpVOffset, vstride,
                                              dp2->FirstVertexOffset);
                      break;
                    case D3DPT_TRIANGLEFAN:
                      D3DPRINT(DL_DX8_DP2, "  DP2:TriangleFan: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrTriangleFan (pRc, dp2->PrimitiveCount, vertexType,
                                            vindex, 0, lpVOffset, vstride,
                                            dp2->FirstVertexOffset);
                      break;
                  } // switch primtype
              } // for each primtype
          } // if setdx6state
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWPRIMITIVE2, lpCmd->wStateCount, 0);
          break;
      }
      case D3DDP2OP_DRAWINDEXEDPRIMITIVE2:
      {
          D3DHAL_DP2DRAWINDEXEDPRIMITIVE2 *dip = (D3DHAL_DP2DRAWINDEXEDPRIMITIVE2*)(lpCmd+1);
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DRAWINDEXEDPRIMITIVE2 - (%d prims)",
                   lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  DIP2: Type=%d BaseVertIdx=%d Min=%d Num=%d StartIdx=%d Count=%d",
                   dip->primType,
                   dip->BaseVertexOffset,
                   dip->MinIndex,
                   dip->NumVertices,
                   dip->StartIndexOffset,
                   dip->PrimitiveCount);
          hr = setDX6state( pRc, lpdp2d->dwhContext, 0, 0, pRc->dwDX8fvf, &vertexType);
          if (hr == D3D_OK)
          {
              for (i=0; i < lpCmd->wStateCount; i++, dip++)
              {
                  LPDWORD pA, pB;
                  DWORD istride = pRc->CurrentIndexBuffer.dwStride;
                  DWORD vstride = pRc->Stream[0].dwStride / sizeof(DWORD);

                  vindex.pb = pRc->pCurrentIndexBuffer + dip->StartIndexOffset;
                  lpVOffset = (LPDWORD)(pRc->pStream[0] + dip->BaseVertexOffset);
                  switch (dip->primType) {

                    case D3DPT_POINTLIST:
                      D3DPRINT(DL_DX8_DP2, "  DIP2:PointList: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrIndexedSprites (pRc, dip->PrimitiveCount, vertexType,
                                               vindex, istride, lpVOffset, vstride,
                                               dip->BaseVertexOffset);
                      break;

                    case D3DPT_LINELIST:
                      D3DPRINT(DL_DX8_DP2, "  DIP2:LineList: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      while (dip->PrimitiveCount-- > 0)
                      {
                          pA = lpVOffset + (istride == 2 ? *vindex.pw++ : *vindex.pd++) * vstride;
                          pB = lpVOffset + (istride == 2 ? *vindex.pw++ : *vindex.pd++) * vstride;
                          pRc->RndrLineList (pRc, pA, pA, pB, vertexType);
                      }
                      break;

                    case D3DPT_LINESTRIP:
                      D3DPRINT(DL_DX8_DP2, "  DIP2:LineStrip: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pB = lpVOffset + (istride == 2 ? *vindex.pw++ : *vindex.pd++) * vstride;
                      while (dip->PrimitiveCount-- > 0)
                      {
                          pA = pB;
                          pB = lpVOffset + (istride == 2 ? *vindex.pw++ : *vindex.pd++) * vstride;
                          pRc->RndrLineList( pRc, pA, pA, pB, vertexType);
                      }
                      break;

                    case D3DPT_TRIANGLELIST:
                      D3DPRINT(DL_DX8_DP2, "  DIP:TriangleList: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrIndexedTriangleList2 (
                          pRc, dip->PrimitiveCount, vertexType,
                          vindex,    pRc->CurrentIndexBuffer.dwStride,
                          lpVOffset, vstride,
                          dip->BaseVertexOffset);
                      break;

                    case D3DPT_TRIANGLESTRIP:
                      D3DPRINT(DL_DX8_DP2, "  DIP:TriangleStrip: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrIndexedTriangleStrip (
                          pRc, dip->PrimitiveCount, vertexType,
                          vindex,    pRc->CurrentIndexBuffer.dwStride,
                          lpVOffset, vstride,
                          dip->BaseVertexOffset);
                      break;

                    case D3DPT_TRIANGLEFAN:
                      D3DPRINT(DL_DX8_DP2, "  DIP:TriangleFan: dwVertexType = 0x%08x, DX8fvf = 0x%08x",
                               lpdp2d->dwVertexType, pRc->dwDX8fvf);
                      pRc->RndrIndexedTriangleFan (
                          pRc, dip->PrimitiveCount, vertexType,
                          vindex,    pRc->CurrentIndexBuffer.dwStride,
                          lpVOffset, vstride,
                          dip->BaseVertexOffset);
                      break;
                  } // switch primtype
              } // for each primtype
          } // if setdx6state
          NEXTINSTRUCTION(lpCmd, D3DHAL_DP2DRAWINDEXEDPRIMITIVE2, lpCmd->wStateCount, 0);
          break;
      }

      //---------------- Clipped Triangle Fan ----------------
      case D3DDP2OP_CLIPPEDTRIANGLEFAN:
      {
          D3DHAL_CLIPPEDTRIANGLEFAN *ctf = (D3DHAL_CLIPPEDTRIANGLEFAN*)(lpCmd+1);
          D3DPRINT(DL_DX8_DP2, "D3DDP2OP_CLIPPEDTRIANGLEFAN - %d prims", lpCmd->wStateCount);
          D3DPRINT(DL_DX8_DP2, "  ClippedTriangleFan: FirstVertOffset=%d EdgeFlags=0x%08x Count=%d",
                   ctf->FirstVertexOffset, ctf->dwEdgeFlags, ctf->PrimitiveCount);
          hr = setDX6state (pRc, lpdp2d->dwhContext, 0, 0, pRc->dwDX8fvf, &vertexType); 
          if (hr == D3D_OK)
          {
              DWORD vstride = pRc->Stream[0].dwStride / sizeof(DWORD);
              vindex.dw = 0;
              for (i=0; i < lpCmd->wStateCount; i++, ctf++)
              {
                  lpVOffset = (DWORD*)(pRc->pStream[0] + ctf->FirstVertexOffset);
                  // TODO: Need to check ctf->dwEdgeFlags if drawing wireframe.
                  pRc->RndrTriangleFan (pRc, ctf->PrimitiveCount, vertexType, vindex, 0,
                                        lpVOffset, vstride, ctf->FirstVertexOffset);
              }
          }
          NEXTINSTRUCTION(lpCmd, D3DHAL_CLIPPEDTRIANGLEFAN, lpCmd->wStateCount, 0);
          break;
      }

      //---------------- HOS Tokens ----------------
      case D3DDP2OP_DRAWRECTPATCH:
        D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DRAWRECTPATCH - unrecognized!!");
        hr = D3DERR_INVALIDCALL;
        break;
      case D3DDP2OP_DRAWTRIPATCH:
        D3DPRINT(DL_DX8_DP2, "D3DDP2OP_DRAWTRIPATCH - unrecognized!!");
        hr = D3DERR_INVALIDCALL;
        break;

      //---------------- Etc ----------------
      case D3DDP2OP_VOLUMEBLT:
        D3DPRINT(DL_DX8_DP2, "D3DDP2OP_VOLUMEBLT - unrecognized!!");
        hr = D3DERR_INVALIDCALL;
        break;
      case D3DDP2OP_BUFFERBLT:
        D3DPRINT(DL_DX8_DP2, "D3DDP2OP_BUFFERBLT - unrecognized!!");
        hr = D3DERR_INVALIDCALL;
        break;
      case D3DDP2OP_MULTIPLYTRANSFORM:
        D3DPRINT(DL_DX8_DP2, "D3DDP2OP_MULTIPLYTRANSFORM - unrecognized!!");
        hr = D3DERR_INVALIDCALL;
        break;

#endif // DIRECT3D_VERSION >= 0x0800

      default:
        // If we don't recognise the command we need to refer it back to the
        // DrawPrimitive2 callback.
        if( D3D_OK == ( hr = dp2Callback( (LPVOID)lpCmd, (LPVOID*)&lpResumeCmd ) ) )
        {
          lpCmd = lpResumeCmd;
          break;
        }

        // We got an error back from the callback, we will now fall out of
        // the command stream parsing because hr != D3D_OK
        D3DPRINT( DL_DX6_DP2_OPS, "  Unhandled DrawPrimitive2 Command - %d", lpCmd->bCommand );        
        break;
    }

    // Check our current return code, if it is not D3D_OK then we need
    // to calculate the address of the command that caused the error and return
    // this back when we exit.
    if( D3D_OK != hr )
    {
      if( D3DERR_COMMAND_UNPARSED == hr )
        lpdp2d->dwErrorOffset = (LPBYTE)lpCmd - (LPBYTE)(lpdp2d->lpDDCommands->lpGbl->fpVidMem);
      
      break;
    }

    // Check and see if we have reached the end of the command stream.        
    if( (DWORD)lpCmd >= cmdEnd )
      break;
  }

  lpdp2d->ddrval = hr;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}


//-------------------------------------------------------------------
//
//  DO_CHROMA_KEYING  - called from setDX6State
//
//      Chroma Keying
//      ---------------
//      The chroma key value is stored in the texture surface. This is the only attribute
//      that is stored in the surface and not the rendering context. If the source chroma
//      key value is set for the texture surface then use the color. SST-1 chroma key
//      tests after the bi-linear filter and this is not correct. D3D performs the chroma
//      test before the filter. Scott/Gary said they would fix it one day...
//
//-------------------------------------------------------------------

void doChromaKeying (RC *pRc, int txtrh, txtrDesc *txtr)                                                                     
{  
    SETUP_PPDEV(pRc)

    TXHNDLSTRUCT    *texhndl;
    FXSURFACEDATA   *surfData; 
    int              flags;

    CMDFIFO_PROLOG(cmdFifo);

    /* map from the texture handle to the texture handle structure */                                  
    texhndl  = TXTRHNDL_TO_TXHNDLSTRUCT(pRc, txtrh);                                        
    surfData = texhndl->surfData;                                                           
    flags    = surfData->dwSurfLclFlags;                                                    
               
    if (pRc->colorKeyEnable && (flags & DDRAWISURF_HASCKEYSRCBLT))                          
    {                                                                                       
        int lr, lg, lb, lxrgb;	/* color space low  */                                      
        int hr, hg, hb, hxrgb;	/* color space high */                                      
        int range = 0;                                                                      
        lr=lg=lb= surfData->dwColorSpaceLowValue;                                           
        hr=hg=hb= surfData->dwColorSpaceHighValue;                                          
        if (hr > lr)                                                                        
            range = 1;	/* chromarange instead of one color */                              
                                                                                            
        /* format of the chroma key value is the format of the texture */                   
        /* need to convert from texture format to xRGB-x888 */                              
        switch (txtr->txFormat >> SST_TA_FORMAT_SHIFT)                                                             
        {                                                                                   
          case (TEXFMT_ARGB_4444):                                   
            lr = ( lr >> 8  ) & 0x0F;                                                       
            lr = _imgMSBReplicate( lr, 4, 0 );                                              
            lg = ( lg >> 4  ) & 0x0F;                                                       
            lg = _imgMSBReplicate( lg, 4, 0 );                                              
            lb = ( lb >> 0  ) & 0x0F;                                                       
            lb = _imgMSBReplicate( lb, 4, 0 );                                              
            if (range)                                                                      
            {                                                                               
                hr = ( hr >> 8  ) & 0x0F;                                                   
                hr = _imgMSBReplicate( hr, 4, 0 );                                          
                hg = ( hg >> 4  ) & 0x0F;                                                   
                hg = _imgMSBReplicate( hg, 4, 0 );                                          
                hb = ( hb >> 0  ) & 0x0F;                                                   
                hb = _imgMSBReplicate( hb, 4, 0 );                                          
            }                                                                               
            break;                                                                          
          case (TEXFMT_ARGB_1555):                                   
            lr = ( lr >> 10 ) & 0x1F;                                                       
            lr = _imgMSBReplicate( lr, 3, 2);                                               
            lg = ( lg >> 5  ) & 0x1F;                                                       
            lg = _imgMSBReplicate( lg, 3, 2);                                               
            lb =   lb         & 0x1F;                                                       
            lb = _imgMSBReplicate( lb, 3, 2);                                               
            if (range)                                                                      
            {                                                                               
                hr = ( hr >> 10 ) & 0x1F;                                                   
                hr = _imgMSBReplicate( hr, 3, 2);                                           
                hg = ( hg >> 5  ) & 0x1F;                                                   
                hg = _imgMSBReplicate( hg, 3, 2);                                           
                hb =   hb         & 0x1F;                                                   
                hb = _imgMSBReplicate( hb, 3, 2);                                           
            }                                                                               
            break;                                                                          
          default:  
          case (TEXFMT_RGB_565):                                     
            lr = ( lr >> 11 ) & 0x1F;                                                       
            lr = _imgMSBReplicate( lr, 3, 2);                                               
            lg = ( lg >> 5 ) &  0x3F;                                                       
            lg = _imgMSBReplicate( lg, 2, 4);                                               
            lb = ( lb >> 0 ) &  0x1F;                                                       
            lb = _imgMSBReplicate( lb, 3, 2 );                                              
            if (range)                                                                      
            {                                                                               
                hr = ( hr >> 11 ) & 0x1F;                                                   
                hr = _imgMSBReplicate( hr, 3, 2);                                           
                hg = ( hg >> 5 ) &  0x3F;                                                   
                hg = _imgMSBReplicate( hg, 2, 4);                                           
                hb = ( hb >> 0 ) &  0x1F;                                                   
                hb = _imgMSBReplicate( hb, 3, 2 );                                          
            }                                                                               
            break;    

          case (TEXFMT_ARGB_8888):
            lr = ( ( lr >> 16 ) & 0xff );      
            lg = ( ( lg >> 8 )& 0xff );
            lb = ( lb & 0xff );
            if( range )
            {
                hr = ( ( hr >> 16 ) & 0xff );
                hg = ( ( hg >> 8 ) & 0xff );
                hb = ( hb & 0xff );
            }
            break;

          case (TEXFMT_ARGB_8332):              
          case (TEXFMT_RGB_332):                                     
            lr = ( lr >> 5 ) & 0x07;                                                        
            lr = ( lr << 5 ) | (lr << 2) | (lr >> 1) ;                                       
            lg = ( lg >> 2 ) & 0x07;                                                        
            lg = ( lg << 5 ) | (lg << 2) | (lg >> 1) ;                                       
            lb =   lb        & 0x03;                                                        
            lb = ( lb << 6 ) | (lb << 4) | (lb << 2) | lb ;                                 
            if (range)                                                                      
            {                                                                               
                hr = ( hr >> 5 ) & 0x07;                                                    
                hr = ( hr << 5 ) | (hr << 2) | (hr >> 1) ;                                   
                hg = ( hg >> 2 ) & 0x07;                                                    
                hg = ( hg << 5 ) | (hg << 2) | (hg >> 1) ;                                   
                hb =   hb        & 0x03;                                                    
                hb = ( hb << 6 ) | (hb << 4) | (hb << 2) | hb ;                             
            }                                                                               
            break;    

          case (TEXFMT_INTENSITY_8):
          case (TEXFMT_ALPHA_INTENSITY_88):                                     
            lr = ( lr & 0xff );
            lg = ( lg & 0xff );
            lb = ( lb & 0xff );
            if (range)                                                                      
            {                                                                               
                hr = ( hr & 0xff );
                hg = ( hg & 0xff );
                hb = ( hb & 0xff );
            }                                                                               
            break;            
            
          case (TEXFMT_ALPHA_INTENSITY_44):                                     
            lr = ( ( lr & 0x0f ) << 4 ) | ( lr & 0x0f );
            lg = ( ( lg & 0x0f ) << 4 ) | ( lg & 0x0f );
            lb = ( ( lb & 0x0f ) << 4 ) | ( lb & 0x0f );
            if (range)                                                                      
            {                                                                               
                hr = ( ( hr & 0x0f ) << 4 ) | ( hr & 0x0f );
                hg = ( ( hg & 0x0f ) << 4 ) | ( hg & 0x0f );
                hb = ( ( hb & 0x0f ) << 4 ) | ( hb & 0x0f );
            }                                                                               
            break;            

          case (TEXFMT_P8_RGB):  
          case (TEXFMT_P8_RGBA): 
          case (TEXFMT_ALPHA_P8_RGB):
          {                                                                                 
              PALHNDL *pPalHndl = TXTRHNDL_TO_PALHNDLSTRUCT(pRc, txtrh);         
              
              // a range of chroma keying is required to pass the DCT tests when using 
              // palettized formats. 
              #define CHROMAKEY_PALETTIZED_RANGE_BIAS 2
              if( !range )
              {
                  range = TRUE;
                  lr -= CHROMAKEY_PALETTIZED_RANGE_BIAS;
                  hr += CHROMAKEY_PALETTIZED_RANGE_BIAS;
              }

              if (pPalHndl != NULL)                                                         
              {                                                                             
                  lg = pPalHndl->ColorTable[lr].peGreen;                                    
                  lg |= lg >> 6;
                  lb = pPalHndl->ColorTable[lr].peBlue;                                     
                  lb |= lb >> 6;
                  lr = pPalHndl->ColorTable[lr].peRed;                                      
                  lr |= lr >> 6;

              }                                                                             
              if (range)                                                                    
              {                                                                             
                  hg = pPalHndl->ColorTable[hr].peGreen;                                    
                  hg |= hg >> 6;
                  hb = pPalHndl->ColorTable[hr].peBlue;                                     
                  hb |= hb >> 6;
                  hr = pPalHndl->ColorTable[hr].peRed;                                      
                  hr |= hr >> 6;                                      
              }                                                                             
          }                                                                                 
          break;
        }  

        lxrgb = (lr << SST_TA_CHROMA_KEY_RED_SHIFT) |                                       
            (lg << SST_TA_CHROMA_KEY_GREEN_SHIFT) |                                         
            (lb << SST_TA_CHROMA_KEY_BLUE_SHIFT);                                           
        if (range)                                                                          
        {                                                                                   
            lxrgb |= (SST_TA_CHROMA_RANGE << SST_TA_CHROMA_MODE_SHIFT);                     
            hxrgb = (hr << SST_TA_CHROMA_KEY_RED_SHIFT) |                                   
                (hg << SST_TA_CHROMA_KEY_GREEN_SHIFT) |                                     
                (hb << SST_TA_CHROMA_KEY_BLUE_SHIFT);                                       
            /* no other bit adjustments need to be made to hxrgb since D3D's */             
            /* chromarange mode is inclusive intersection */                                
            CMDFIFO_CHECKROOM(cmdFifo, PH1_SIZE + 2);                                       
            SETPH(cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_TMU_0, taChromaKey));              
            SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->taChromaKey, lxrgb);                       
            SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->taChromaRange, hxrgb);                     
            pRc->sst.TMU[0].taChromaKey.vFxU32 = lxrgb;                                      
            pRc->sst.TMU[0].taChromaRange.vFxU32 = hxrgb;                                    
        }                                                                                   
        else                                                                                
        {                                                                                   
            lxrgb |= (SST_TA_CHROMA_KEY << SST_TA_CHROMA_MODE_SHIFT);                       
            CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );                                     
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_TMU_0, taChromaKey) );            
            SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->taChromaKey, lxrgb );                     
            pRc->sst.TMU[0].taChromaKey.vFxU32 = lxrgb;                                      
        }                                                                                   
    }                                                                                       
    else                                                                                    
    {                                                                                       
        pRc->sst.TMU[0].taChromaKey.vFxU32 &= ~SST_TA_CHROMA_MODE;                           
        CMDFIFO_CHECKROOM(cmdFifo, PH1_SIZE + 1);                                           
        SETPH(cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_TMU_0, taChromaKey));                  
        SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->taChromaKey, pRc->sst.TMU[0].taChromaKey.vFxU32);
    } 
    CMDFIFO_EPILOG( cmdFifo );
}


//-------------------------------------------------------------------
//
//      Set Render Funcs
//
//  This routine is called from setDX6state to setup the rendering
//  function pointers. It is only called when the bRecalcRndrFn
//  dirty flag is set.
//
//-------------------------------------------------------------------

void __stdcall setRenderFuncs(RC *pRc)
{
    SETUP_PPDEV(pRc)
    
    if(IS_SAGE_ACTIVE && GE_FVF_NOTL(pRc->current_FVF))
    {
        setGeRenderFuncs(pRc);
        return;
    }

    pRc->RndrPoints = dp2Point;
    pRc->RndrLineList = dp2Line;
    pRc->RndrLineListImm = dp2Line;
    pRc->RndrIndexedLineList = NULL;
    pRc->RndrIndexedLineList2 = NULL;
    pRc->RndrLineStrip = dp2Line;
    pRc->RndrIndexedLineStrip = dp2Line;

#if (DIRECT3D_VERSION >= 0x0800)
    pRc->RndrSprites = dp2SpriteAll;
    pRc->RndrIndexedSprites = dp2IdxSpriteAll;
#endif // DX == 8

    if(pRc->fillMode != D3DFILL_SOLID)
    {
        pRc->RndrTriangleList = dp2TriangleAllFill;
        pRc->RndrIndexedTriangleList = dp2IdxTriangleAllFill;
        pRc->RndrIndexedTriangleList2 = dp2IdxTriangle2AllFill;
        pRc->RndrTriangleStrip = dp2StripAllFill;
        pRc->RndrIndexedTriangleStrip = dp2IdxStripAllFill;
        pRc->RndrTriangleFan = dp2FanAllFill;
        pRc->RndrTriangleFanImm = dp2FanAllFill;
        pRc->RndrIndexedTriangleFan = dp2IdxFanAllFill;
    }
    else
    {
        pRc->RndrTriangleList = dp2TriangleAll;
        pRc->RndrIndexedTriangleList = dp2IdxTriangleAll;
        pRc->RndrIndexedTriangleList2 = dp2IdxTriangle2All;

#ifdef CMDFIFO
        if((pRc->state & STATE_REQUIRES_SPECULAR) ||
           (pRc->shadeMode == D3DSHADE_FLAT) ||
           (pRc->state & STATE_REQUIRES_WRAP))
#else
        if(1)
#endif
        {
            pRc->RndrTriangleStrip = dp2StripAll;
            pRc->RndrIndexedTriangleStrip = dp2IdxStripAll;
            pRc->RndrTriangleFan = dp2FanAll;
            pRc->RndrTriangleFanImm = dp2FanAll;
            pRc->RndrIndexedTriangleFan = dp2IdxFanAll;
        }
        else
        {
#ifdef CMDFIFO
            if(pRc->numVertexRegs < GR_PKT3_TRIFAN_MIN_VREGS)
#else
            if(1)
#endif
            {
                pRc->RndrTriangleStrip = dp2StripAll;
                pRc->RndrIndexedTriangleStrip = dp2IdxStripAll;
                pRc->RndrTriangleFanImm = dp2FanAll;
                pRc->RndrTriangleFan = dp2FanAll;
                pRc->RndrIndexedTriangleFan = dp2IdxFanAll;
            }
            else
            {
                pRc->RndrTriangleStrip = dp2StripNoSpec;
                pRc->RndrIndexedTriangleStrip = dp2IdxStripNoSpec;
                pRc->RndrTriangleFanImm = dp2FanNoSpec;
                pRc->RndrTriangleFan = dp2FanNoSpec;
                pRc->RndrIndexedTriangleFan = dp2IdxFanNoSpec;
            }
        }
    }
}


//-------------------------------------------------------------------
//
//      S E T    D X 6   S T A T E
//
//  This routine transfers all the shadow register content to the actual
//  hardware registers IF there has been a change in the state for a
//  group of registers, or all registers if a context change has occured.
//
//-------------------------------------------------------------------

DWORD setDX6state (RC *pRc, DWORD dwhContext, DWORD primitiveType, DWORD count, DWORD fvf, DWORD *fvfotVertex )
{
    SETUP_PPDEV(pRc)

    static DWORD    dwhPreviousContext = 0;
    int             size, offset, rc;
    FxU32           va_size, fbmode, multiSample; 
    FxU32           i, lastStage;
    FXSURFACEDATA  *surfaceData, *surfaceDataZ;

    CMDFIFO_PROLOG(cmdFifo);

    HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);
    
#ifdef CSERVICE
    D3D_GRABBING_CS_CONTEXT();
#endif

    // setup the vertex type and the fvf offset table
    rc = setFVFOffsetTable(fvf, pRc, fvfotVertex);
    if (rc != D3D_OK)
        return rc;

    // setup the primitive rendering functions if their state is dirty
    if( pRc->bRecalcRndrFn || !(pRc->bVtxGeStateToggle == FVF_NO_CHANGE) )
    {
        setRenderFuncs(pRc);
        pRc->bRecalcRndrFn = FALSE;
    }

    // Adjust the alpha blending modes assuming that the destination has no alpha.. this is probably 
    // a bad assumption but it's passing all of the tests at the moment.
    if( pRc->alphaBlendEnable )
    {
      switch ((pRc->sst.peAlphaMode.vFxU32 & SST_PE_RGB_SRC_FACT) >> SST_PE_RGB_SRC_FACT_SHIFT)
      {
        case SST_PE_ABLEND_DST_ALPHA:
          pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_SRC_FACT); // Clear RGB source
          pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE << SST_PE_RGB_SRC_FACT_SHIFT);
          break;
        case SST_PE_ABLEND_ONE_MINUS_DST_ALPHA:
        case SST_PE_ABLEND_SATURATE:
          pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_SRC_FACT); // Clear RGB source
          pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ZERO << SST_PE_RGB_SRC_FACT_SHIFT);
          break;
      }

      switch ((pRc->sst.peAlphaMode.vFxU32 & SST_PE_RGB_DST_FACT) >> SST_PE_RGB_DST_FACT_SHIFT)
      {
        case SST_PE_ABLEND_DST_ALPHA:
          pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_DST_FACT); // Clear RGB destination
          pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE << SST_PE_RGB_DST_FACT_SHIFT);
          break;
        case SST_PE_ABLEND_ONE_MINUS_DST_ALPHA:
        case SST_PE_ABLEND_SATURATE :
          pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_DST_FACT); // Clear RGB destination
          pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ZERO << SST_PE_RGB_DST_FACT_SHIFT);
          break;
      }
    }

#ifdef CUBEMAP
    // Check if any texture handles reference a cube map and if so 
    // do copy / translate for the cube map now and not halfway
    // into the state setting code. Call setupTexturing up here
    // so state flag for texturing is defined.

#if 0
    // Having problems whenever this code is executed on the first
    // pass into a WHQL cubemap test. If I skip the first pass the
    // when state is not set up the remaining tests are happy.
    setupTexturing( pRc );
#endif

    if ( (pRc->state & STATE_REQUIRES_ST_TMU0) && (_D3( cubeMapSignFix ) != 1))
    {  
      FxU32 i, numStages;

      txtrDesc *txtr;

      numStages = (pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT;
      for (i=0; i <= numStages; i++ ) 
      {
        if (TS[i].textureHandle > 0)
        {
          txtr = TXTRHNDL_TO_TXTRDESC(pRc, TS[i].textureHandle);
          if ((txtr->txFormatFlags & TEXFMTFLG_CUBEMAP) &&
              (txtr->txDirtyBits & DDSCAPS2_CUBEMAP_ALLFACES))
          {
             CMDFIFO_SAVE( cmdFifo );

             #define CUBEMAP_DEBUG 1
             #ifdef CUBEMAP_DEBUG
                 txtrCopyCubeFace2D( ppdev, txtr );
             #endif 

             txtrCopyCubeFace3D( ppdev, txtr );

             CMDFIFO_RELOAD( cmdFifo );

             // Treat the 3d texture copy op as a context change
             UPDATE_HW_STATE (SC_ALL_CHANGED);
          }
        }
      }
    }
#endif

    if ( dwhContext != dwhPreviousContext ) // has context changed from last time?
    {
      dwhPreviousContext = dwhContext;
      UPDATE_HW_STATE (SC_ALL_CHANGED);     // since context has changed, need to set up everything
    }  // if Context Has Changed

    { // ALWAYS do this section, whether context has changed or not :

      if (IS_SAGE_ACTIVE && GE_FVF_NOTL(pRc->current_FVF))
      {  // Allow SAGE state management first crack at everything.
         CMDFIFO_SAVE( cmdFifo );
         SetGEState(pRc, dwhContext);
         CMDFIFO_RELOAD( cmdFifo );
      }

      multiSample = 0; 
	  // The following changes to/from aa mode 
      switch( _DD(ddFSAAMode)  )
      {
         // This flag will be on for most FSAA modes 
        case  FSAA_MODE_4XFLIP:
        case  FSAA_MODE_4XBLT:
            multiSample |= DDSCAPS2_HINTANTIALIASING;
            break;
        case  FSAA_MODE_DEMO:
            break; //toggle aa off from hotkey
        default:
    	    //handle windowed aa
            multiSample = pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING; 
            break;
      }

      fbmode = (pRc->sst.peFbzMode.vFxU32 & SST_PE_FB_FORMAT) >> SST_PE_FB_FORMAT_SHIFT;
      
      // If we are doing 32 bpp rendering, then we need to send this fact to fbzMode. Otherwise 16bpp
      // is assumed, as fbzMode is not initialized elsewhere based on the rendering bit depth.
      if (GETPRIMARYBYTEDEPTH == 4)
      {
          fbmode = SST_PE_FB_32BPP;       // AA will be properly set via following switch
      }

	  switch (fbmode)
	  {
	    case SST_PE_FB_16BPP:
	    case SST_PE_FB_AA16:
	        fbmode = (multiSample) ? SST_PE_FB_AA16 : SST_PE_FB_16BPP;
	        break;
	    case SST_PE_FB_32BPP:
	    case SST_PE_FB_AA32:
	        fbmode = (multiSample) ? SST_PE_FB_AA32 : SST_PE_FB_32BPP;
	        break;
	    case SST_PE_FB_1555:
	    case SST_PE_FB_AA1555:
	        fbmode = (multiSample) ? SST_PE_FB_AA1555 : SST_PE_FB_1555;
	        break;
	  }
	  pRc->sst.peFbzMode.vFxU32 &= ~SST_PE_FB_FORMAT;
	  pRc->sst.peFbzMode.vFxU32 |= fbmode << SST_PE_FB_FORMAT_SHIFT;

      // the following is to set suMode for AA (or not) as necessary...
      if (!multiSample && (pRc->sst.suMode.vFxU32 & SST_SU_ANTI_ALIAS))
      {
          // we're not in AA so make sure this is reset
          pRc->sst.suMode.vFxU32 &= ~SST_SU_ANTI_ALIAS;
      }
      else if (multiSample )
      {
          // we are in an AA mode
          if( !(pRc->sst.suMode.vFxU32 & SST_SU_ANTI_ALIAS) )
          {
              // we are in aa mode but mode bit is not yet set
#if (DIRECT3D_VERSION >= 0x0800 )          
              // if we are in an AA mode which was forced on by the registry key,
              // then we must set the suMode bit regardless of the multisample renderstate.
              if( pRc->multiSampleAA || (_DD(ddAARegistryMode) > 1) ) 
              {
                  // for dx8 we must check renderstate
                  pRc->sst.suMode.vFxU32 |= SST_SU_ANTI_ALIAS;
              }
#else
              // for dx7 we can just turn it on
              pRc->sst.suMode.vFxU32 |= SST_SU_ANTI_ALIAS;
#endif
          }
#if (DIRECT3D_VERSION >= 0x0800 )          
          else if( !pRc->multiSampleAA )
          {
              // we are in aa mode, suMode is set but renderstate is off
              // so we need to reset the SST_SU_ANTI_ALIAS bit
              pRc->sst.suMode.vFxU32 &= ~SST_SU_ANTI_ALIAS;
          }
#endif
      }  // end if multiSample

      // Set up the surfaces
      surfaceData = (FXSURFACEDATA*)pRc->lpSurfData;
      pRc->sst.peColBufferAddr.vFxU32 = (FxU32) surfaceData->hwPtr;
      pRc->sst.peBufferSize.vFxU32    = (FxU32) SET_PE_BUFFER_SIZE_TILED( surfaceData->dwStride, surfaceData->dwHeight );

      if (pRc->zEnable)
      {
          if (pRc->DDSZHndl) 
          {
              DWORD dwStride, dwHeight;

              dwStride = surfaceData->dwStride;
              dwHeight = surfaceData->dwHeight;

              // if we're using Fullscreen AA the stride and height values may need to be tweeked
              // mls added 7/28/00 if this code is not here you will see an effect in FSAA where
              // every other frame does not render correctly. The incorrect frame will have a double
              // image in the top half of the screen and garbage or nothing in the bottom half.
              switch( _DD(ddFSAAMode)  )
              {  
                case  FSAA_MODE_4XFLIP:
                    dwHeight = dwHeight >> 1;
                    dwStride = surfaceData->dwWidth * surfaceData->dwBytesPerPixel;
                    break;
                default:
                    break;
              }

              // Use the color buffer's size since there's only one size register
              // for z and color and they need to be the same. Mismatch shows up
              // doing render to texture where the zbuffer still seems to have
              // the dimensions of the primary zbuffer.
              surfaceDataZ = (FXSURFACEDATA*)pRc->lpSurfDataZ;
              pRc->sst.peAuxBufferAddr.vFxU32 = (FxU32) surfaceDataZ->hwPtr;
              pRc->sst.peBufferSize.vFxU32    = (FxU32) SET_PE_BUFFER_SIZE_TILED( dwStride, dwHeight );
          }
          else // (0 == pRc->DDSZHndl)
          {  // delayed clear of z buffer settings from zEnable and zWriteEnable render state functions

              D3DPRINT(0, "setDX6State: zEnable=%ld, zWriteEnable=%ld but lpDDSZ is NULL,"
                          " clearing zEnable & zWriteEnable render states", pRc->zEnable, pRc->zWriteEnable);

              // Clear the z buffer render states now so the rendering and fog code won't use the wrong state settings
              // the alternative is to check both pRc->lpDDSZ && pRc->zEnable where ever we currently
              // just check pRc->zEnable and to check both pRc->lpDDSZ && pRc->zWriteEnable where ever
              // we currently just check pRc->zWriteEnable
              pRc->zEnable = D3DZB_FALSE;

              // no z buffer, undo the zEnable & zWriteEnable render settings
              pRc->sst.peFbzMode.vFxU32 &= ~( SST_PE_DEPTH_FLOAT  | SST_PE_EN_DEPTH_BUFFER |
                                             SST_PE_DEPTH_WRMASK | SST_PE_DEPTH_ZQ_SEL    );

              if(!(pRc->state & (STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG)))
              {
                  pRc->state &= ~STATE_REQUIRES_W_FBI;
                  pRc->sst.suMode.vFxU32 &= ~SST_SU_Q;
              }
              pRc->state &= ~(STATE_REQUIRES_OOZ | STATE_REQUIRES_ZBUFFER | STATE_REQUIRES_WBUFFER);
              pRc->sst.suMode.vFxU32 &= ~SST_SU_Z;
          }
      }  // if zEnable

      // If anything changed, need to do a MOP
      CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE + PH1_SIZE + 1);
      SETMOP( cmdFifo, SST_MOP_STALL_FBZMODE_LOAD );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFbzMode ) );  // Would rather do this in the PE setup later,
      SETPD( cmdFifo, ghw0->peFbzMode,    pRc->sst.peFbzMode.vFxU32    );  // but if not here, it generates csim errors.

      // If Flexible Vertex Format does not support specular, turn it off here.
      if ( !pRc->bFVFSpecularPresent )
      {
          specular (pRc, 0);
      }
      triangleFlavour( pRc->state, primitiveType, count );  // This is for debug dump only

#if (DIRECT3D_VERSION >= 0x0800)
      // See if pixel shader is being used, and if it's dirty, configure.
      if (pRc->dwCurrentPShader != 0)
          setupTexturingDX8 (pRc);
      else
#endif
          setupTexturing (pRc);             // normal dx6/7 texturing.

    }  // END : ALWAYS section

    // ===== VIEWPORT REGISTERS
    if ( HW_STATE_CHANGED & SC_VIEWPORT )
    {
      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 7);
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1(7, SST_UNIT_FBI, vpMode));
      SETPD( cmdFifo, ghw0->vpMode, pRc->sst.vpMode.vFxU32);

      SETFPD( cmdFifo, ghw0->vpSizeX,   pRc->sst.vpSizeX.vFloat   );
      SETFPD( cmdFifo, ghw0->vpCenterX, pRc->sst.vpCenterX.vFloat );
      SETFPD( cmdFifo, ghw0->vpSizeY,   pRc->sst.vpSizeY.vFloat   );
      SETFPD( cmdFifo, ghw0->vpCenterY, pRc->sst.vpCenterY.vFloat );
      SETFPD( cmdFifo, ghw0->vpSizeZ,   pRc->sst.vpSizeZ.vFloat   );
      SETFPD( cmdFifo, ghw0->vpCenterZ, pRc->sst.vpCenterZ.vFloat );

      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 2 );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, vpSTscale0));
      SETPD( cmdFifo, ghw0->vpSTscale0, pRc->sst.vpSTscale0.vFxU32);
      SETPD( cmdFifo, ghw0->vpSTscale1, pRc->sst.vpSTscale1.vFxU32);
    }  // END : Viewport

    // ===== SETUP UNIT REGISTERS
    if ( HW_STATE_CHANGED & SC_SETUP_UNIT )
    {
      CMDFIFO_CHECKROOM( cmdFifo, 1*PH1_SIZE + 0 + 1);
     // SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suMode));                // Output in rasterizers
     // SETPD( cmdFifo, ghw0->suMode,      pRc->sst.suMode.vFxU32 );                 // so do not need to output here
     // SETPD( cmdFifo, ghw0->suParamMask, pRc->sst.suParamMask.vFxU32 );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, suLineWidth ) );
      SETPD( cmdFifo, ghw0->suLineWidth, pRc->sst.suLineWidth.vFxU32 );
    }  // END : Setup Unit

    // ===== SETUP UNIT CLIPPING REGISTERS
    if ( HW_STATE_CHANGED & SC_SETUP_UNIT_CLIP )
    {
      CMDFIFO_CHECKROOM( cmdFifo, 3*(PH1_SIZE + 1));
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipMinXMaxX[0]));       // Note : only zero-th element used now.
      SETPD( cmdFifo, ghw0->suClipMinXMaxX[0], pRc->sst.suClipMinXMaxX[0].vFxU32 );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipMinYMaxY[0]));
      SETPD( cmdFifo, ghw0->suClipMinYMaxY[0], pRc->sst.suClipMinYMaxY[0].vFxU32 );
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipEnables));
      SETPD( cmdFifo, ghw0->suClipEnables, pRc->sst.suClipEnables.vFxU32 );
    }  // END : Setup Unit Clip

    // ===== RASTER UNIT REGISTERS
    if ( HW_STATE_CHANGED & SC_RA_SETUP )
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 2));
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, raControl ) );
      SETPD( cmdFifo, ghw0->raControl, pRc->sst.raControl.vFxU32 );
      SETPD( cmdFifo, ghw0->raStipple, pRc->sst.raStipple.vFxU32 );
    }  // END : Raster Unit

    // ===== VTA CONTROL REGISTERS
    if ( HW_STATE_CHANGED & SC_VTA_CONTROL )
    {
      CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE + 1));
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, taControl ) );
      SETPD( cmdFifo, ghw0->taControl,  pRc->sst.taControl.vFxU32 );
    }  // END : VTA Control

    // ===== TMU GROUP A & B REGISTERS      - we may split these groups up later.
    if ( HW_STATE_CHANGED & (SC_TMUGROUP_A | SC_TMUGROUP_B) )
    {
      if ( pRc->state & STATE_REQUIRES_ST_TMU0 )
      {  
        FxU32 i, j, numStages;
        txtrDesc *txtr;
        // setup the texture mode, location of texture and tlod register for this texture

        numStages = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT);
        for( i=0, j=numStages; i <= numStages; i++, j-- ) 
        {
            FxU32 format;
        
            // If taMode and taNpt/Block linear are fully setup this check
            // should not be needed, but removing it fails

            if (TS[i].textureHandle > 0)
                txtr = TXTRHNDL_TO_TXTRDESC(pRc, TS[i].textureHandle);
            else
                txtr = 0;

            if (txtr)
            {
                format = txtr->txFormat >> SST_TA_FORMAT_SHIFT;
                if ((format <= TEXFMT_DXT5 && format >= TEXFMT_DXT1) ||
                    (format == TEXFMT_FXT1))
                {
                    pRc->sst.TMU[i].taNPT.vFxU32 |= SST_BLOCK_LINEAR;
                }
                else
                {
                    pRc->sst.TMU[i].taNPT.vFxU32 &= ~SST_BLOCK_LINEAR;
                }
            }
            else
                pRc->sst.TMU[i].taNPT.vFxU32 &= ~SST_BLOCK_LINEAR;

            CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE+5 + PH1_SIZE+4 + PH1_SIZE+5 );

            // Here we do the switcheroo, i is in d3d ordering, j reverses the actual register
            // it's written to based on the number of active stages.
            SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R4|R5, SST_UNIT_TMU_0+j, taMode));
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taMode, pRc->sst.TMU[i].taMode.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taLMS, pRc->sst.TMU[i].taLMS.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taShiftBias, pRc->sst.TMU[i].taShiftBias.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taNPT, pRc->sst.TMU[i].taNPT.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taBaseAddr0, pRc->sst.TMU[i].taBaseAddr0.vFxU32 );

            SETPH( cmdFifo, CMDFIFO_BUILD_PK1(4, SST_UNIT_TMU_0+j, taColorAR0));
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorAR0,  pRc->sst.TMU[i].taColorAR0.vFxU32)
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorGB0,  pRc->sst.TMU[i].taColorGB0.vFxU32)
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorAR1,  pRc->sst.TMU[i].taColorAR1.vFxU32)
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorGB1,  pRc->sst.TMU[i].taColorGB1.vFxU32)

            SETPH( cmdFifo, CMDFIFO_BUILD_PK1(5, SST_UNIT_TMU_0+j, taTcuColor));
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taTcuColor,   pRc->sst.TMU[i].taTcuColor.vFxU32   );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taTcuAlpha,   pRc->sst.TMU[i].taTcuAlpha.vFxU32   );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taCcuControl, pRc->sst.TMU[i].taCcuControl.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taCcuColor,   pRc->sst.TMU[i].taCcuColor.vFxU32   );
            SETPD( cmdFifo, SST_TREX(ghw0,j)->taCcuAlpha,   pRc->sst.TMU[i].taCcuAlpha.vFxU32   );

            D3DPRINT( 255, "taLMS %d =0x%08lx", i, pRc->sst.TMU[i].taLMS.vFxU32 );
        }  // for

        for (i=0; i <= numStages; i++)
        {
          int txtrh = TS[i].textureHandle;
          txtrDesc *txtr;

          if (txtrh > 0)
          {
              txtr = TXTRHNDL_TO_TXTRDESC(pRc, txtrh);

              // if palettized texture and it changed then redownload it.
              // why download now instead of when texture id changes - this is the conservative
              // approach and the palette can change for numerous reasons so lets make sure
              // that it is download before each mesh (can optimize later)

              if (TXTR_IS_PALETTIZED(txtr) && (PALETTECHANGED & _D3(flags)))
              {
                  LPPALETTEENTRY pal;
                  CMDFIFO_SAVE( cmdFifo );

                  pal = TXTRHNDL_TO_PALETTE(pRc, txtrh);
                  txtrDownloadPalette( ppdev, txtr, pal );

                  // _D3(currentPalette) = (void *) TXTRHNDL_TO_PALHNDL(pRc, txtrh);
                  _D3(currentPalette) = (void *) TXTR_GET_PAL_DESCRIPTOR(txtr, pRc, txtrh);

                  CMDFIFO_RELOAD( cmdFifo );
              }
          }
          if (txtrh != 0)
          {
              CMDFIFO_SAVE( cmdFifo );
              doChromaKeying ( pRc, txtrh, txtr );      // CHROMA KEYING
              CMDFIFO_RELOAD( cmdFifo );
          }
        }// for
      }  // if ( pRc->state & STATE_REQUIRES_ST_TMU0 )
      else
      {
        // Not textured.
        // As of 11/11/99, this else clause is run when no texturing is performed.
        // The first TMU in the pipeline is for diffuse color (flat or gouraud) and
        // the second TMU, if set, is used for the specular color.  If specular,
        // TMU 1 will be diffuse and TMU 0 will be specular.
        UINT    tmui, d3di; 
        UINT    laststage = (pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT;

        CMDFIFO_CHECKROOM( cmdFifo, (laststage+1)*(PH1_SIZE + 5) );
        for (tmui=laststage, d3di=0; d3di <= laststage; d3di++, tmui--)
        {
            OURTMUREGS *ptmui = &(pRc->sst.TMU[d3di]);

            SETPH( cmdFifo, CMDFIFO_BUILD_PK1(5, SST_UNIT_TMU_0+tmui, taTcuColor));
            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->taTcuColor,   ptmui->taTcuColor.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->taTcuAlpha,   ptmui->taTcuAlpha.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->taCcuControl, 0);
            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->taCcuColor,   ptmui->taCcuColor.vFxU32 );
            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->taCcuAlpha,   ptmui->taCcuAlpha.vFxU32 );
        }  // for

        CMDFIFO_CHECKROOM( cmdFifo, 2*(PH4_SIZE + 2));
        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R4, SST_UNIT_TMU_0, taMode ) );
        SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->taMode, pRc->sst.TMU[0].taMode.vFxU32 );
        SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->taNPT, pRc->sst.TMU[0].taNPT.vFxU32 );

        SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R4, SST_UNIT_TMU_1, taMode ) );
        SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->taMode, pRc->sst.TMU[1].taMode.vFxU32 );
        SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->taNPT, pRc->sst.TMU[1].taNPT.vFxU32 );

      } // else
    }  // END : TMU Group A

    // ===== PIXEL ENGINE REGISTERS
    if ( HW_STATE_CHANGED & SC_PE_SETUP )
    {
      if ( pRc->zEnable)
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peSDConst ) );
        SETPD( cmdFifo, ghw0->peSDConst,     pRc->sst.peSDConst.vFxU32  );

        // If the z-buffer is enabled, then we clear the aux buffer bit
        pRc->sst.peCache.vFxU32 &= ~SST_PE_READ_AUX_EN_N;
      }
      else
      {
        // If the z-buffer is disabled, then we set the aux buffer bit
        pRc->sst.peCache.vFxU32 |= SST_PE_READ_AUX_EN_N;

        // If the z-buffer is disabled really make sure depth writemask
        // is cleared or risk hanging.....
        if (pRc->sst.peFbzMode.vFxU32 & SST_PE_DEPTH_WRMASK)
        {
            pRc->sst.peFbzMode.vFxU32 &= ~SST_PE_DEPTH_WRMASK;
        }
      }

      CMDFIFO_CHECKROOM( cmdFifo, 3*PH1_SIZE + 3 + 3 + 3);
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 3, SST_UNIT_FBI, peFbzMode ) );
      SETPD( cmdFifo, ghw0->peFbzMode,       pRc->sst.peFbzMode.vFxU32    );
      SETPD( cmdFifo, ghw0->peAlphaTest,     pRc->sst.peAlphaTest.vFxU32  );
      SETPD( cmdFifo, ghw0->peAlphaMode,     pRc->sst.peAlphaMode.vFxU32  );

	  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 3, SST_UNIT_FBI, peStencil ) );
      SETPD( cmdFifo, ghw0->peStencil,       pRc->sst.peStencil.vFxU32    );
      SETPD( cmdFifo, ghw0->peStencilOp,     pRc->sst.peStencilOp.vFxU32  );
      SETPD( cmdFifo, ghw0->peCache,         pRc->sst.peCache.vFxU32      );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 3, SST_UNIT_FBI, peColBufferAddr));
      SETPD( cmdFifo, ghw0->peColBufferAddr, pRc->sst.peColBufferAddr.vFxU32 );
      SETPD( cmdFifo, ghw0->peAuxBufferAddr, pRc->sst.peAuxBufferAddr.vFxU32 );
      SETPD( cmdFifo, ghw0->peBufferSize,    pRc->sst.peBufferSize.vFxU32    );
      // SETPD( cmdFifo, ghw0->peClipMinXMaxX,  pRc->sst.peClipMinXMaxX.vFxU32  ); 
      // SETPD( cmdFifo, ghw0->peClipMinYMaxY,  pRc->sst.peClipMinYMaxY.vFxU32  ); 
      // SETPD( cmdFifo, ghw0->peExMask,        pRc->sst.peExMask.vFxU32        ); 

#if (DIRECT3D_VERSION >= 0x0800 ) 
      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1);
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peExMask )); 
      SETPD( cmdFifo, ghw0->peExMask, pRc->sst.peExMask.vFxU32 );  
#endif // dx8 accumulation buffer
    
    }  // END : Pixel Engine

    // ===== FOG REGISTERS
    if ( HW_STATE_CHANGED & SC_FOG )
    {
      if (pRc->fogEnable)
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFogColor ) );
        SETPD( cmdFifo, ghw0->peFogColor, pRc->fogColor );
        if (pRc->fogTableMode != D3DFOG_NONE)                   // hardware table fog
        {
            if (_D3(currentFogTableMode) == pRc->fogTableMode)  // Download the fog table if it has changed.
            {
                if( _D3(currentFogTableMode) == D3DFOG_LINEAR ) // did the parameters change?
                {// if using linear modes and parameters have changed - reload table
                    if( (_D3(currentFogTableStart) != pRc->fogTableStart) ||
        	            (_D3(currentFogTableEnd)   != pRc->fogTableEnd)      )
        	        {
                        CMDFIFO_SAVE( cmdFifo );
                        createTableAndLoad(pRc);
                        CMDFIFO_RELOAD( cmdFifo );
        	        }
                }
                else
                {
                    if( _D3(currentFogTableDensity) != pRc->fogDensity )    // if using exp modes and density is wrong - reload table
                    {
                        CMDFIFO_SAVE( cmdFifo );
                        createTableAndLoad(pRc);
                        CMDFIFO_RELOAD( cmdFifo );
        	        }
                }
            }
            else
            {
               CMDFIFO_SAVE( cmdFifo );                         // If current mode is wrong, reload table
               createTableAndLoad(pRc);
               CMDFIFO_RELOAD( cmdFifo );
            }
        }
      }

      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
	  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFogMode ) );
      SETPD( cmdFifo, ghw0->peFogMode, pRc->sst.peFogMode.vFxU32 );
      // SETPD( cmdFifo, ghw0->peFogBias,      pRc->sst.peFogBias.vFxU32);       // Currently not changed
    }  // END : Fog

    // ===== ANY OTHER REGISTERS    [Currently these are not changed after initialization]
    // if ( HW_STATE_CHANGED & SC_MISC )
    // {
    //     SETPD( cmdFifo, ghw0->taLfbMode,    pRc->sst.taLfbMode.vFxU32);
    //     SETPD( cmdFifo, ghw0->taLfbADConst, pRc->sst.taLfbADConst.vFxU32);
    // } // END : Other

    // Clear the hardware changed status :
    RESET_HW_STATE;

    // ===== SETUP PACKET 3 DATA - Vertex Array
    // at this point, size of vtx dependent on suMode and suParamMask only

    size = 2;	// XY always
    va_size = 0; // packed argb stored as 4 dwords in vtx array, so we may need to add 3 extra to size

    if (pRc->sst.suMode.vFxU32 & SST_SU_W) size++;
    if (pRc->sst.suMode.vFxU32 & SST_SU_Z) size++;
    if (pRc->sst.suMode.vFxU32 & SST_SU_Q) size++;

    lastStage = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT);
    for( i=0; i<lastStage+1; i++)
    {
	    if( pRc->sst.suParamMask.vFxU32 & pRc->su_parammask_rgba_flags[i] ) { size++; va_size+=3; }
	    if( pRc->sst.suParamMask.vFxU32 & pRc->su_parammask_tmu_flags[i]  )  size+=2;
        if( pRc->sst.suParamMask.vFxU32 & pRc->su_parammask_q_flags[i]    )  size++;
    }
    va_size += size; // add non-color components

    // possible for va_size to change while size stays same, and vice versa - so check both        
    if ((va_size<<16 | size) != pRc->vertexSize)
    {
      // set size (in DWords) required in fifo for pkt's of 0, 1, 2, 3, and 4 vertices
      pRc->pkt3Size[0] = 1;  // packet header only
      pRc->pkt3Size[1] = pRc->pkt3Size[0] + size;
      pRc->pkt3Size[2] = pRc->pkt3Size[1] + size;
      pRc->pkt3Size[3] = pRc->pkt3Size[2] + size;
      pRc->pkt3Size[4] = pRc->pkt3Size[3] + size;

      // size of vertex has changed update PKT3 header array pointers
      pRc->vertexSize    = (va_size<<16 | size);
      pRc->numVertexRegs = size_to_mvr[va_size];

      pRc->curVertexId   = 0;

      offset = g_header_offset[pRc->numVertexRegs];
      for ( i=0 ; i<5 ; i++ )
	    pRc->nvi_table[i]    = g_nvi_table[i]    + offset;
      for ( i=0 ; i<GR_NUM_PACKET3_TYPES ; i++ )
	    pRc->header_table[i] = g_header_table[i] + offset;

      if(IS_SAGE_ACTIVE && GE_FVF_TNL(pRc->current_FVF))
      {
        // We need to tell SAGE what PKT3 size is implied by suParamMask and suMode.
        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1, kPkt3SizeReg, 0, 0));
        if(pRc->sst.suMode.vFxU32 & SST_SU_PACKED_ARGB)
          SETCF( cmdFifo, size);
        else
          SETCF( cmdFifo, size + va_size);
      }
    }

    HW_ACCESS_EXIT(ACCESS_3D);

    CMDFIFO_EPILOG( cmdFifo );

    return D3D_OK;
}

/*
 * $Log: 
 *  104  3dfx      1.103       12/11/00 Miles Smith     fixed a spot where I 
 *       forgot to check for room in Cmd Fifo before sending a packet
 *  103  3dfx      1.102       12/8/00  Miles Smith     Added state changes for 
 *       COLORWRITEENABLE, BLENDOP, and MULTISAMPLEMASK
 *  102  3dfx      1.101       11/30/00 Brent Burton    Fix typo.
 *  101  3dfx      1.100       11/30/00 Brent Burton    New DX8 DP2 tokens for 
 *       pixel shaders fully done.  Added call to setupTexturingDX8() for pixel 
 *       shader configuration.  Fixed up debug level printing.
 *  100  3dfx      1.99        11/30/00 Miles Smith     changes for AA 
 *       renderstate in DX8  AA can now be turned on and off on the fly under 
 *       app control via  a renderstate call
 *  99   3dfx      1.98        11/27/00 Dale  Kenaston  DX8 compile fixes. Added
 *       the first vertex offset as a parameter to triangle rendering functions 
 *       called from dx8 tokens.
 *  98   3dfx      1.97        11/24/00 Miles Smith     adding dx8 code for 
 *       multisample buffers
 *  97   3dfx      1.96        11/22/00 Dale  Kenaston  Sage triangle fan 
 *       primitive. Added the vStart local variable and modified the triangle 
 *       tokens to extract the starting vertex into vStart before computing the 
 *       starting vertex pointer. vStart is then passed into the primitive 
 *       functions.
 *  96   3dfx      1.95        11/16/00 Brent Burton    Fix DX8 build error.  No
 *       logic change.
 *  95   3dfx      1.94        11/14/00 Brent Burton    DX8: Added PointSprite 
 *       support through new render functions.  RC now contains new fields 
 *       RndrSprites and RndrIndexedSprites.  Renderstates dealing with 
 *       PointSprites are now processed.
 *  94   3dfx      1.93        11/14/00 Dale  Kenaston  Sage triangle list 
 *       fixes. Modified the packet 3 size setup code to only write the register
 *       when the current fvf is a tl vertex.
 *  93   3dfx      1.92        11/14/00 Brent Burton    DX8: better primitive 
 *       processing; fixes to not call dp2StripNoSpec().  More primitives 
 *       handled.
 *  92   3dfx      1.91        11/13/00 Brent Burton    Changed arg ordering and
 *       added new args to render functions.  Added 16 and 32 bit index support 
 *       for indexed prims.  Better DX7 to DX8 render function sharing.
 *  91   3dfx      1.90        11/7/00  Brent Burton    Removed reference of 
 *       MAX_NUM_RSTATES and used MAX_RENDERSTATES instead.
 *  90   3dfx      1.89        11/2/00  Miles Smith     cleanup of FSAA mode 
 *       names
 *  89   3dfx      1.88        11/1/00  Don Fowler      Added support for more 
 *       palettized chroma keys types
 *  88   3dfx      1.87        10/31/00 Brent Burton    Added DWORD vstride 
 *       argument to rasterizer functions to replace FVFO_SIZE usage for DX8.
 *  87   3dfx      1.86        10/30/00 Brent Burton    StateSet support for 
 *       DX8.  Added recording and execution of vertex and index stream 
 *       statesets.
 *  86   3dfx      1.85        10/30/00 Tim Little      Protected the CMDFIFO 
 *       from itself.  CMDFIFO_SAVE/RESTORE added.
 *  85   3dfx      1.84        10/28/00 Evan Leland     Sage changes: fix vertex
 *       toggle flag that tells Sage code when to become active based on the 
 *       vertex type.
 *  84   3dfx      1.83        10/24/00 Brent Burton    Added basic 
 *       TriangleStrip and PointList primitives for DX8 DrawPrimitive2 token.
 *  83   3dfx      1.82        10/20/00 Dale  Kenaston  Viewport setup for Sage,
 *       etc. Moved rampage viewport register setup from setdx6state to 
 *       D3DDP2OP_VIEWPORTINFO. Modified setRenderFuncs to use the new 
 *       GE_FVF_NOTL macro.
 *  82   3dfx      1.81        10/12/00 Evan Leland     modified Sage VB code
 *  81   3dfx      1.80        10/11/00 Brent Burton    DX8 code integration.  
 *       Changes to headers, new code.
 *  80   3dfx      1.79        10/10/00 Evan Leland     VB updates for Sage, 
 *       code to support resetting of rasterizer pointers when the vertex type 
 *       changes between vertices that Sage renders and those that Rampage 
 *       renders by itself.
 *  79   3dfx      1.78        10/9/00  Dale  Kenaston  Sage rendering function 
 *       pointer fix. Inverted the result of GE_FVF_TRANSFORMED in 
 *       setRenderFuncs.
 *  78   3dfx      1.77        10/6/00  Dale  Kenaston  Sage rendering function 
 *       stubs. Removed all the bSkipTnL sections from ddiDrawPrimitives2. 
 *       Modified setRenderFuncs to call setGeRenderFuncs when sage is active 
 *       and the fvf type is non-tl. Modified setDX6state to set the rendering 
 *       function dirty flag when the fvf type changes from tl to non-tl or vice
 *       versa.
 *  77   3dfx      1.76        10/4/00  Dale  Kenaston  New Sage macros. Changed
 *       IS_RAGE in setDX6state to IS_SAGE_ACTIVE.
 *  76   3dfx      1.75        9/28/00  Dale  Kenaston  Rampage rendering 
 *       function pointers. Moved the rendering function prototypes to 
 *       d3dglobal.h. Modified the triangle list, indexed triangle list, indexed
 *       triangle list 2, triangle strip, indexed triangle strip, triangle fan 
 *       immediate, triangle fan, indexed triangle fan, points, line list, line 
 *       strip, indexed line strip and line list immediate tokens to use the new
 *       function pointers in the rc. Added the setRenderFuncs function which 
 *       calculates the new function pointers from the rc. Modified setDX6state 
 *       to call setRenderFuncs when the render function dirty flag is set.
 *  75   3dfx      1.74        9/27/00  Evan Leland     Moves code that 
 *       calculates fvf offset table into a routine called during setDX6state. 
 *       Also adds code that checks if the fvf is changing before recalculating 
 *       the fvf offset table.
 *  74   3dfx      1.73        9/26/00  Miles Smith     changes for AA hotkey 
 *       demo mode
 *  73   3dfx      1.72        9/22/00  Dale  Kenaston  Sage packet3, register 
 *       and quickturn initialization changes  Moved the packet 3 size setup 
 *       inside existing vertex size code.
 *  72   3dfx      1.71        9/14/00  Don Fowler      Added code to 
 *       setDX6State to map the src and dst blend types from 
 *       destalpha/invdestalpha to one/zero respectively. This was done with the
 *       assumption that the destination will never have alpha.. this may be a 
 *       bad assumption but we're passing all of the tests and it can be easily 
 *       modified to test for destination alpha
 *  71   3dfx      1.70        9/14/00  Michel Conrad   Generally include NPT 
 *       register writes output of VTA state, to make csim happy. Csim complains
 *       if taMode is DXT and npt != block linear even when texturing is 
 *       disabled.
 *  70   3dfx      1.69        9/8/00   Evan Leland     mod to fvfOffsetTable 
 *       setup in ddiDrawPrimitives2 to fix a mistake in the way vertex blending
 *       weights are set up.
 *  69   3dfx      1.68        9/6/00   Don Fowler      Altered code to store 
 *       the LOD Bias passed in for the TextureStageState
 *  68   3dfx      1.67        9/6/00   Michel Conrad   Fixes for cubemapping, 
 *       tweak to cache the palette descriptor (not the handle) so the palette 
 *       is not downloaded each setdx6state. Fix for peCache bits with reversed 
 *       sense.
 *  67   3dfx      1.66        8/29/00  Tim Little      Removed some of the 
 *       #ifdef HW_TNL compile checks, more to come latter
 *  66   3dfx      1.65        8/28/00  Evan Leland     added support for non 
 *       tnl fvf types VERTEX and LVERTEX
 *  65   3dfx      1.64        8/22/00  Brian Danielson Changed the renderstate 
 *       management from being tied to the current render context to being a 
 *       global concept, now accessable from DD.
 *  64   3dfx      1.63        8/14/00  Tim Little      Now SetGEState gets 
 *       called in setDX6state, this is runtime checked.
 *  63   3dfx      1.62        8/11/00  Brian Danielson Fixed setDX6State bug, 
 *       enabled peCache, added Central Services code.
 *  62   3dfx      1.61        8/10/00  Dale  Kenaston  Sage membase, fifo 
 *       initialization and packet 3 size setup
 *  61   3dfx      1.60        8/3/00   Don Fowler      Added code to handle 
 *       legacy ALPHA MODULATE state. Alpha modulate will now get alpha 
 *       information from the iterator if there is no alpha information in the 
 *       pixel format.    Added support for texture SRC colorkeying    Fixed Z 
 *       bias to support Rampage 24 bit bias and added an arbitrary multiplier 
 *       to "fixup" the value of 0-15 passed by ddraw. The number I chose was 
 *       entirely arbitrary and was just enough to get the tests to pass.    
 *       Coupled the AlphaDepthTest with the AlphaColorTest because of a bug in 
 *       CSIM and possibly in the hardware.
 *  60   3dfx      1.59        8/1/00   Brian Danielson Added define in global.h
 *       to control renderstate function code paths so that D3PRINT statements 
 *       would be executed based on this define.
 *  59   3dfx      1.58        7/28/00  Miles Smith     Changed a block in 
 *       setdx6state that was causing problems with FSAA
 *  58   3dfx      1.57        7/24/00  Brian Danielson Changes to implement 
 *       renderstate and shadow register management.
 *  57   3dfx      1.56        7/10/00  Michel Conrad   Adding cubemap support. 
 *       Some clean up of mops. Fix for rendertarget  mismatch of Z and Color 
 *       buffer.
 *  56   3dfx      1.55        6/27/00  Evan Leland     removed an invalid 
 *       non-printable character that was messing up softice's ability to match 
 *       source code with assembly
 *  55   3dfx      1.54        6/15/00  Michel Conrad   Delete really old check 
 *       in comments. Replace tempD3DHA_DP2STATE with real one.
 *  54   3dfx      1.53        6/9/00   Miles Smith     Updating fullscreen aa 
 *       code to handle multiple modes.
 *  53   3dfx      1.52        6/7/00   Evan Leland     adds preliminary 
 *       instrumentation to Rampage driver
 *  52   3dfx      1.51        5/22/00  Evan Leland     removed dx7-specific 
 *       ifdefs and code targeted to the pre-dx7 driver
 *  51   3dfx      1.50        5/11/00  Evan Leland     dx7 structure cleanup 
 *       effort complete
 *  50   3dfx      1.49        4/24/00  Evan Leland     support for removing dd 
 *       surf local pointer from TXHNDLSTRUCT
 *  49   3dfx      1.48        4/24/00  Brian Danielson Added Pint and Line fill
 *       mode functionality. Also fixed some oversights I found in line and 
 *       point code.
 *  48   3dfx      1.47        4/24/00  Evan Leland     preliminary changes, 
 *       getting ready to removing all ddraw structure pointers stored in the 
 *       driver
 *  47   3dfx      1.46        4/20/00  Evan Leland     surflcl was not 
 *       initialized before use at line 2082
 *  46   3dfx      1.45        4/4/00   Evan Leland     Corrects parsing of 
 *       command stream to correctly process multiple loads to the same texture
 *  45   3dfx      1.44        3/31/00  Evan Leland     Changes to the TEXBLT 
 *       portion of ddiDrawPrim2, to fix support for partial texture download.
 *  44   3dfx      1.43        3/16/00  Michel Conrad   Validate Zbuffer against
 *       zEnable/zWriteEnable and clear raster flags as needed. Clear block 
 *       linear bit in taNPT when a compressed texture is no longer used.
 *  43   3dfx      1.42        3/1/00   Brent Burton    Fix DX7 build's U,V 
 *       wrapping from the D3DTSS_ADDRESS texture state.
 *  42   3dfx      1.41        2/29/00  Brian Danielson Fixed texture coordinate
 *       indexing for FVFs.
 *  41   3dfx      1.40        2/28/00  Brian Danielson Fix for DX7 where added 
 *       var to incorrect place in d3global.h.
 *  40   3dfx      1.39        2/25/00  Miles Smith     Fixed 2 line drawing 
 *       loops which had bad index calculations  Brain made a fix dealing with 
 *       texture coordinate dimensions.
 *  39   3dfx      1.38        2/11/00  Brian Danielson Fixed SLI_Clear call 
 *       where else was missing brackets for else section.
 *  38   3dfx      1.37        1/28/00  Evan Leland     updated TEXBLT operation
 *       to work correctly for DX7
 *  37   3dfx      1.36        1/28/00  Evan Leland     more dx7 upgrade work
 *  36   3dfx      1.35        1/27/00  Evan Leland     DX7 changes
 *  35   3dfx      1.34        1/25/00  Evan Leland     updates for dx7
 *  34   3dfx      1.33        1/25/00  Evan Leland     part of 
 *       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
 *  33   3dfx      1.32        1/18/00  Brent Burton    Fix texture address mode
 *       when passed through TSS.  When the "Address" state is specified it now 
 *       sets the "AddressU" and "AddressV" states as dx6/7 refrast does.
 *  32   3dfx      1.31        1/12/00  Miles Smith     adding support for 
 *       fullscreen AA.
 *  31   3dfx      1.30        1/11/00  Miles Smith     Adding support for 
 *       fullscreen AA
 *  30   3dfx      1.29        1/10/00  Michel Conrad   Use modifed d3dglobal 
 *       structure. Fix problem with out of range renderstates.
 *  29   3dfx      1.28        12/17/99 Brian Danielson Added FVF, Projected 
 *       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
 *       Clear and remapped to ddiClear2, cleanups.
 *  28   3dfx      1.27        12/17/99 Evan Leland     fixed compile break
 *  27   3dfx      1.26        12/17/99 Brent Burton    Removed all WINNT macro 
 *       checks.  This removed some unused code.
 *  26   3dfx      1.25        12/16/99 Evan Leland     fixed build break: had 
 *       the wrong function name TEXTURELOADP2 instead of TEXTURELOADDP2
 *  25   3dfx      1.24        12/14/99 Evan Leland     Preliminary merge of 
 *       texture code with FXSURFACEDATA structure, also added new 
 *       TEXTURELOADDP2 entry point
 *  24   3dfx      1.23        11/17/99 Brent Burton    Added a 
 *       CMDFIFO_CHECKROOM() to non textured specular code.
 *  23   3dfx      1.22        11/11/99 Brent Burton    Fixed specular color 
 *       when texturing is disabled.
 *  22   3dfx      1.21        11/9/99  Ping Zheng      Fixed a few bugs in T&L 
 *       related code
 *  21   3dfx      1.20        11/4/99  Ping Zheng      Added code to hook T&L 
 *       related dp2 commands
 *  20   3dfx      1.19        11/1/99  Brent Burton    Fixed GPF in D3D due to 
 *       DXT* texturing.  Minor change.
 *  19   3dfx      1.18        10/26/99 Evan Leland     Implements FXT1 texture 
 *       mode support
 *  18   3dfx      1.17        10/19/99 Miles Smith     Added some tracking 
 *       variables so that if 2 or more different apps are running with 
 *       different fog table parameters the driver will switch back and forth 
 *       between the tables as needed.
 *  17   3dfx      1.16        10/15/99 Brian Danielson Change by Miles Smith to
 *       fix fog problem.
 *  16   3dfx      1.15        10/15/99 Andrew Sobczyk  Mods to call SLI clear
 *  15   3dfx      1.14        10/13/99 Philip Zheng    Added T&L hooks for R3
 *  14   3dfx      1.13        10/11/99 Evan Leland     fixed a build break with
 *       dx7 texture download
 *  13   3dfx      1.12        10/7/99  Brent Burton    Fixed a bug when CF=1 in
 *       ddiDrawPrimitives2().  The rendering functions were mis-selected due to
 *       a test of prc->dx5texture, which should not be performed anymore.
 *  12   3dfx      1.11        10/6/99  Evan Leland     modified dx7 texture 
 *       download section
 *  11   3dfx      1.10        10/1/99  Mark Einkauf    Complete HW_ACCESS 
 *       macros work
 *  10   3dfx      1.9         9/30/99  Philip Zheng    Implemented the work 
 *       around for DX 7 multiple buffer problem
 *  9    3dfx      1.8         9/29/99  Brian Danielson Added Guard Band 
 *       clipping support and trivial rejection clipping.
 *  8    3dfx      1.7         9/29/99  Brent Burton    Fixed an off-by-one 
 *       error when checking palletized textures in setDX6State().
 *  7    3dfx      1.6         9/28/99  Brent Burton    Fix build problem when 
 *       CF=1
 *  6    3dfx      1.5         9/28/99  Brent Burton    Renamed prc->texture to 
 *       prc->dx5texture.  Cleaned up setDX6State()'s usage of prc->texture to 
 *       use instead each stage's texture.
 *  5    3dfx      1.4         9/21/99  Mark Einkauf    in setDX6State, set 
 *       count in CMDFIFO_CHECKROOM properly, rearrange a few others for better 
 *       readibility
 *  4    3dfx      1.3         9/20/99  Mark Einkauf    check size and va_size 
 *       to determine if packet3 headers need to be recomputed.
 *  3    3dfx      1.2         9/16/99  Brian Danielson Fixes to stencil code : 
 *       typo, control output, initial value setup.
 *  2    3dfx      1.1         9/13/99  Philip Zheng    
 *  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
 * $
 */
