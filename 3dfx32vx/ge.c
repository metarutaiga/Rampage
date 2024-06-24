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
** $Log: 
*/


#include <ddrawi.h>
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "geglobal.h"
#include "gevlib.h"                     // Sage vertex size register setup routines
#include "gevlu.h"                      // routines to translate from fvf to gevlib inputs

#ifdef SLI
#include <ddsli2d.h>
#endif

//-----------------------------------------------------------------------------------
//
// GE_SetZRange()
// 
//-----------------------------------------------------------------------------------

DWORD GE_SetZRange(RC *pRc, LPD3DHAL_DP2COMMAND * ppCmd)
{
  SETUP_PPDEV(pRc)
  LPD3DHAL_DP2ZRANGE pZRange;

  // Keep only the last viewport notification
  pZRange = (D3DHAL_DP2ZRANGE *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount - 1;

  // Update T&L viewport state
  pRc->GERC.Viewport.dvMinZ = pZRange->dvMinZ;
  pRc->GERC.Viewport.dvMaxZ = pZRange->dvMaxZ;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_ZRANGE;

  *ppCmd = (LPD3DHAL_DP2COMMAND)
           ((D3DHAL_DP2ZRANGE *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount);

  UPDATE_HW_STATE( SC_GE_MATRIX );
  return D3D_OK;
}

//-----------------------------------------------------------------------------------
//
// GE_SetMaterial()
// 
//-----------------------------------------------------------------------------------

DWORD GE_SetMaterial(RC *pRc, LPD3DHAL_DP2COMMAND * ppCmd)
{
  SETUP_PPDEV(pRc)
  LPD3DHAL_DP2SETMATERIAL pSetMat;

  // Keep only the last material notification
  pSetMat = (D3DHAL_DP2SETMATERIAL *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount - 1;

  pRc->GERC.Material = *(D3DMATERIAL7 *)pSetMat;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_MATERIAL;

  *ppCmd = (LPD3DHAL_DP2COMMAND)
          ((D3DHAL_DP2SETMATERIAL *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount);
  UPDATE_HW_STATE( SC_GE_LIGHTING );
  return D3D_OK;
}

//-----------------------------------------------------------------------------------
//
// GE_SetLight()
// 
//-----------------------------------------------------------------------------------

DWORD GE_SetLight(RC *pRc, LPD3DHAL_DP2COMMAND * ppCmd)
{
  SETUP_PPDEV(pRc)
  WORD wNumSetLight = ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount;
  LPD3DHAL_DP2SETLIGHT pSetLight = (LPD3DHAL_DP2SETLIGHT)(*ppCmd + 1);
  int  k;
  DWORD dwIndex;
  DWORD dwStride = sizeof(D3DHAL_DP2SETLIGHT);
  D3DLIGHT7 *pLightData = NULL;

  for (k = 0; k < wNumSetLight; k++)
  {
    dwIndex = pSetLight->dwIndex;
    dwStride = sizeof(D3DHAL_DP2SETLIGHT);
  
    if(dwIndex >= GE_MAX_ACTIVE_LIGHTS)
	  break;

    switch (pSetLight->dwDataType)
    {
      case D3DHAL_SETLIGHT_ENABLE:
	    pRc->GERC.geLightArray[dwIndex].bLightEnable = TRUE;
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_SETLIGHT;
        break;
      case D3DHAL_SETLIGHT_DISABLE:
	    pRc->GERC.geLightArray[dwIndex].bLightEnable = FALSE;
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_SETLIGHT;
        break;
      case D3DHAL_SETLIGHT_DATA:
        pLightData = (D3DLIGHT7 *)((LPBYTE)pSetLight + dwStride);

        switch (pLightData->dltType)
        {
          case D3DLIGHT_POINT:
          case D3DLIGHT_SPOT:
          case D3DLIGHT_DIRECTIONAL:
            // No other light types are allowed
      	    memcpy(&pRc->GERC.geLightArray[dwIndex].LightData, pLightData, sizeof(D3DLIGHT7));
            break;
		  default:
   	        pRc->GERC.geLightArray[dwIndex].bLightEnable = FALSE;
			break;
        }

        pRc->GERC.dwDirtyFlags |= GE_DIRTY_SETLIGHT;
        dwStride += sizeof(D3DLIGHT7);
        break;
    }

    // Update the command buffer pointer
    pSetLight = (D3DHAL_DP2SETLIGHT *)((LPBYTE)pSetLight +
                                       dwStride);
  }

  *ppCmd = (LPD3DHAL_DP2COMMAND)pSetLight;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  return D3D_OK;
}

//-----------------------------------------------------------------------------------
//
// GE_CreateLight()
// 
//-----------------------------------------------------------------------------------

DWORD GE_CreateLight(RC *pRc, LPD3DHAL_DP2COMMAND * ppCmd)
{
  SETUP_PPDEV(pRc)
  DWORD dwIndex;
  WORD wNumCreateLight = ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount;
  LPD3DHAL_DP2CREATELIGHT pCreateLight = (LPD3DHAL_DP2CREATELIGHT)(*ppCmd + 1);
  HRESULT hr = D3D_OK;
  int i;

  for (i = 0; i < wNumCreateLight; i++, pCreateLight++)
  {
    dwIndex = pCreateLight->dwIndex;

    // If the index is not already allocated, grow the light array
    // by REF_LIGHTARRAY_GROWTH_SIZE
    pRc->GERC.geLightArray[dwIndex].bLightEnable = FALSE;
    pCreateLight++;
  }

  *ppCmd = (LPD3DHAL_DP2COMMAND)
           ((D3DHAL_DP2CREATELIGHT *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount);
 
  UPDATE_HW_STATE( SC_GE_LIGHTING );
  return D3D_OK;
}

//-----------------------------------------------------------------------------------
//
// GE_SetTransform()
// 
//-----------------------------------------------------------------------------------

DWORD GE_SetTransform(RC *pRc, LPD3DHAL_DP2COMMAND * ppCmd)
{
  SETUP_PPDEV(pRc)
  WORD wNumXfrms = ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount;
  LPD3DHAL_DP2SETTRANSFORM pSetXfrm = (LPD3DHAL_DP2SETTRANSFORM)(*ppCmd + 1);
  D3DMATRIX *pMat = &pSetXfrm->matrix;
  int i;
  // BUGBUG is there a define for 0x80000000?
  BOOL bSetIdentity = (pSetXfrm->xfrmType & 0x80000000) != 0;

  for(i = 0; i < (int) wNumXfrms; i++, pSetXfrm++)
  {
    DWORD dwxfrmType = (DWORD)pSetXfrm->xfrmType & (~0x80000000);
    switch (dwxfrmType)
    {
      case D3DTRANSFORMSTATE_WORLD:
        memcpy(&(pRc->GERC.matWorld[0]), pMat, sizeof(D3DMATRIX));
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_WORLDXFM;
        break;

      case D3DTRANSFORMSTATE_VIEW:
        memcpy(&pRc->GERC.matView, pMat, sizeof(D3DMATRIX));
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_VIEWXFM;
        break;

      case D3DTRANSFORMSTATE_PROJECTION:
        memcpy(&pRc->GERC.matProj, pMat, sizeof(D3DMATRIX));
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_PROJXFM;
        break;

      case D3DTRANSFORMSTATE_WORLD1:
        memcpy(&(pRc->GERC.matWorld[1]), pMat, sizeof(D3DMATRIX));
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_WORLD1XFM;
        break;

      case D3DTRANSFORMSTATE_WORLD2:
        memcpy(&(pRc->GERC.matWorld[2]), pMat, sizeof(D3DMATRIX));
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_WORLD2XFM;
        break;

      case D3DTRANSFORMSTATE_WORLD3:
        memcpy(&(pRc->GERC.matWorld[3]), pMat, sizeof(D3DMATRIX));
        pRc->GERC.dwDirtyFlags |= GE_DIRTY_WORLD3XFM;
        break;

      case D3DTRANSFORMSTATE_TEXTURE0:
      case D3DTRANSFORMSTATE_TEXTURE1:
      case D3DTRANSFORMSTATE_TEXTURE2:
      case D3DTRANSFORMSTATE_TEXTURE3:
      case D3DTRANSFORMSTATE_TEXTURE4:
      case D3DTRANSFORMSTATE_TEXTURE5:
      case D3DTRANSFORMSTATE_TEXTURE6:
      case D3DTRANSFORMSTATE_TEXTURE7:
      {
        DWORD dwStage = pSetXfrm->xfrmType - D3DTRANSFORMSTATE_TEXTURE0;
        if (bSetIdentity)
        {
          memcpy(&pRc->GERC.matTexture[dwStage], &matIdent, sizeof(D3DMATRIX));
        }
        else
        {
          memcpy(&pRc->GERC.matTexture[dwStage], pMat, sizeof(D3DMATRIX));
        }
      }
        break;
      default:
        D3DPRINT(D3DDBGLVL, "Ignoring unknown transform type");
    }
  }

  *ppCmd = (LPD3DHAL_DP2COMMAND)
           ((D3DHAL_DP2SETTRANSFORM *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount);

  UPDATE_HW_STATE( SC_GE_MATRIX );
  return D3D_OK;
}

//-----------------------------------------------------------------------------------
//
// GE_SetClipPlane()
// 
//-----------------------------------------------------------------------------------

DWORD GE_SetClipPlane(RC *pRc, LPD3DHAL_DP2COMMAND * ppCmd)
{
  SETUP_PPDEV(pRc)
  WORD wNumClipPlanes = ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount;
  DWORD dwIndex;
  int  i;
  LPD3DHAL_DP2SETCLIPPLANE pSetClipPlane =
        (LPD3DHAL_DP2SETCLIPPLANE)(*ppCmd + 1);

  for (i = 0; i < (int) wNumClipPlanes; i++, pSetClipPlane++)
  {
    dwIndex = pSetClipPlane->dwIndex;
    if(dwIndex < GE_MAX_USER_CLIPPLANES)
      memcpy( &pRc->GERC.userClipPlanes[dwIndex], pSetClipPlane->plane, sizeof(GEVECTOR4) );
  }

  pRc->GERC.dwDirtyFlags |= GE_DIRTY_CLIPPLANES;

  *ppCmd = (LPD3DHAL_DP2COMMAND)
             ((D3DHAL_DP2SETCLIPPLANE *)(*ppCmd + 1) + ((LPD3DHAL_DP2COMMAND)*ppCmd)->wStateCount);

  UPDATE_HW_STATE( SC_GE_CLIPPING );
  return D3D_OK;
}

//-----------------------------------------------------------------------------------
//
// setGeRenderFuncs()
// 
//-----------------------------------------------------------------------------------

void __stdcall setGeRenderFuncs(RC *pRc)
{
    pRc->RndrPoints = gePoint;

    pRc->RndrLineList = geLine;
    pRc->RndrIndexedLineList = geLine;
    pRc->RndrIndexedLineList2 = geLine;
    pRc->RndrLineStrip = geLine;
    pRc->RndrIndexedLineStrip = geLine;
    pRc->RndrLineListImm = geLine;

    if(pRc->fillMode != D3DFILL_SOLID)
    {
        pRc->RndrTriangleList = geTriangleAllFill;
        pRc->RndrIndexedTriangleList = geIdxTriangleAllFill;
        pRc->RndrIndexedTriangleList2 = geIdxTriangle2AllFill;

        pRc->RndrTriangleStrip = geStripAllFill;
        pRc->RndrIndexedTriangleStrip = geIdxStripAllFill;

        pRc->RndrTriangleFanImm = geFanAllFill;
        pRc->RndrTriangleFan = geFanAllFill;
        pRc->RndrIndexedTriangleFan = geIdxFanAllFill;
    }
    else
    {
        pRc->RndrTriangleList = geTriangleAll;
        pRc->RndrIndexedTriangleList = geIdxTriangleAll;
        pRc->RndrIndexedTriangleList2 = geIdxTriangle2All;

        pRc->RndrTriangleStrip = geStripAll;
        pRc->RndrIndexedTriangleStrip = geIdxStripAll;

        pRc->RndrTriangleFanImm = geFanAll;
        pRc->RndrTriangleFan = geFanAll;
        pRc->RndrIndexedTriangleFan = geIdxFanAll;
    }
}
