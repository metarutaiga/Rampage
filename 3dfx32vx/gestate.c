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

RENDERFXN_RETVAL __stdcall GE_State_Clipping(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE clipping renderstate: %d", state );
  if((ULONG)pRc->GERC.bDoClipping == state)
    RENDERFXN_OK;

  pRc->GERC.bDoClipping = (BOOL)state;

  UPDATE_HW_STATE( SC_GE_CLIPPING );
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall GE_State_Lighting(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE lighting renderstate: %d", state );
  if((ULONG)pRc->GERC.bDoLighting == state)
    RENDERFXN_OK;

  pRc->GERC.bDoLighting = (BOOL)state;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_Extents(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "GE extents renderstate: %d", state );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_Ambient(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE ambient renderstate: %d", state );
  if(pRc->GERC.ambient_save  == state)
    RENDERFXN_OK;

  pRc->GERC.ambient_save  = state;
  pRc->GERC.ambient_red   = D3DVAL(RGBA_GETRED(state))/(255.f);
  pRc->GERC.ambient_green = D3DVAL(RGBA_GETGREEN(state))/(255.f);
  pRc->GERC.ambient_blue  = D3DVAL(RGBA_GETBLUE(state))/(255.f);
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_MATERIAL;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_FogVertexMode(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE fog vertex mode renderstate: %d", state );
  if(pRc->GERC.dwFogVertexMode == state)
    RENDERFXN_OK;

  pRc->GERC.dwFogVertexMode = state;

  pRc->GERC.dwDirtyFlags |= GE_DIRTY_FOG;

  UPDATE_HW_STATE( SC_GE_MISC );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_ColorVertex(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE color vertex renderstate: %d", state );
  if((ULONG)pRc->GERC.bColorVertex == state)
    RENDERFXN_OK;

  pRc->GERC.bColorVertex = (BOOL)state;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_COLORVTX;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_LocalViewer(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE local viewer renderstate: %d", state );
  if((ULONG)pRc->GERC.bLocalViewer == state)
    RENDERFXN_OK;

  pRc->GERC.bLocalViewer = (BOOL)state;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_NormNormals(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE normalize normals renderstate: %d", state );
  if((ULONG)pRc->GERC.bNormalizeNormals == state)
    RENDERFXN_OK;

  pRc->GERC.bNormalizeNormals = (BOOL)state;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_NORMAL;

  UPDATE_HW_STATE( SC_GE_MISC );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_DfusMaterialSrc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE diffuse material src renderstate: %d", state );
  if((ULONG)pRc->GERC.dfusMaterialSrc == state)
    RENDERFXN_OK;

  pRc->GERC.dfusMaterialSrc = state;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_COLORVTX;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_SpecMaterialSrc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE specular material src renderstate: %d", state );
  if((ULONG)pRc->GERC.specMaterialSrc == state)
    RENDERFXN_OK;

  pRc->GERC.specMaterialSrc = state;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_COLORVTX;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_AmbMaterialSrc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE ambient material src renderstate: %d", state );
  if((ULONG)pRc->GERC.ambMaterialSrc == state)
    RENDERFXN_OK;

  pRc->GERC.ambMaterialSrc = state;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_COLORVTX;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall GE_State_EmisMaterialSrc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE emissive material src renderstate: %d", state );
  if((ULONG)pRc->GERC.emisMaterialSrc == state)
    RENDERFXN_OK;

  pRc->GERC.emisMaterialSrc = state;
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_COLORVTX;

  UPDATE_HW_STATE( SC_GE_LIGHTING );
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall GE_State_ClipPlanEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE clip plane enable renderstate: %d", state );
  if((ULONG)pRc->GERC.dwClipPlanEnable == state)
      RENDERFXN_OK;

  pRc->GERC.dwClipPlanEnable = state; 
  pRc->GERC.dwDirtyFlags |= GE_DIRTY_CLIPPLANES;

  UPDATE_HW_STATE( SC_GE_CLIPPING );
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall GE_State_CKeyBlend(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE color key blend enable renderstate: %d", state );
  if((ULONG)pRc->GERC.bColorKeyBlendEnable == state)
      RENDERFXN_OK;

  pRc->GERC.bColorKeyBlendEnable = (BOOL)state;

  UPDATE_HW_STATE( SC_GE_MISC);
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall GE_State_VertexBlend(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "GE vertex blend renderstate: %d", state );
  if(pRc->GERC.dwNumVertexBlends == state)
      RENDERFXN_OK;

  pRc->GERC.dwNumVertexBlends = state;

  UPDATE_HW_STATE( SC_GE_MATRIX );
  RENDERFXN_OK;
}
