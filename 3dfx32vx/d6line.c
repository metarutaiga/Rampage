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
**  17   3dfx      1.16        10/11/00 Brent Burton    DX8 code integration.  
**       Changes to headers, new code.
**  16   3dfx      1.15        9/14/00  Don Fowler      Specular was being added
**       to the color of the line and then alpha blended with the color of the 
**       line. Removed the initial add to the color of the line so that specular
**       only affected the resultant color by the alpha blending mode.
**  15   3dfx      1.14        7/24/00  Brian Danielson Changes to implement 
**       renderstate and shadow register management.
**  14   3dfx      1.13        7/10/00  Michel Conrad   Remove pixel offset 
**       cruft.
**  13   3dfx      1.12        6/15/00  Michel Conrad   Delete really old check 
**       in comments and drawglobal check.
**  12   3dfx      1.11        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  11   3dfx      1.10        4/24/00  Brian Danielson Added Pint and Line fill
**       mode functionality. Also fixed some oversights I found in line and 
**       point code.
**  10   3dfx      1.9         2/9/00   Evan Leland     Minor change to some DX7
**       code to fix a build break when building with command fifo off, CF=0. 
**       The code that was breaking had never been built with CF=0 so we didn't 
**       know there was a syntax error.
**  9    3dfx      1.8         1/27/00  Evan Leland     DX7 changes
**  8    3dfx      1.7         1/25/00  Evan Leland     updates for dx7
**  7    3dfx      1.6         1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  6    3dfx      1.5         12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  5    3dfx      1.4         10/12/99 Evan Leland     added NPT texture 
**       coordinate scaling to lines and points
**  4    3dfx      1.3         9/28/99  Brent Burton    Renamed prc->texture to 
**       prc->dx5texture.
**  3    3dfx      1.2         9/14/99  Mark Einkauf    Init W if perspective, 
**       or TMU0 needs it; update CMDFIFO only optimized fan,tri,strip render 
**       routines to match general "..All" versions.
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
*/

#include <ddrawi.h>
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d6fvf.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "d3txtr2.h"                    // txtrDesc

#define LINEWRAP(uv, wrap, i)           \
  {                                     \
  if (wrap)                             \
  {                                     \
    float  el;                          \
    el = d3absval(uv##2[i] - uv##1[i]); \
    if (el > DTOVALP(0.5, 24))          \
    {                                   \
      if (uv##2 < uv##1)                \
        uv##2[i] += ITOVALP(1, 24);     \
      else                              \
        uv##1[i] += ITOVALP(1, 24);     \
    }                                   \
  }                                     \
  }

#define WRAPST(s, t, wrap_s, wrap_t, i) \
  do                                    \
  {                                     \
      LINEWRAP(s, wrap_s, i);           \
      LINEWRAP(t, wrap_t, i);           \
  } while (0)


//---------------------------------------------------------------
//
//  dp2Line :  Line drawing primitive
//             (used also by D3DFILL_WIREFRAME fill mode tri, fan, strip primitives)
//
void __stdcall dp2Line( RC *pRc, LPDWORD pF, LPDWORD pA, LPDWORD pB, DWORD vertexType )
{
  SETUP_PPDEV(pRc)
  D3DCOLOR    aColor, bColor, aBaseColor, bBaseColor;
  D3DCOLOR    saColor, sbColor;

  FxU32       su_mode = pRc->sst.suMode.vFxU32;
  FxU32       su_param_mask = pRc->sst.suParamMask.vFxU32;

  float       w1, w2;
  float       z1, z2;
  float       s1[8], s2[8], t1[8], t2[8];

  FxU32       i, lastStage, d3di;
  float       oowA, oowB;

  CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
  return;
#endif

  CMDFIFO_CHECKROOM( cmdFifo, 3 + pRc->pkt3Size[2]);

  if ( !pRc->bFVFDiffusePresent )
  {
    aBaseColor = FVF_DIFFUSE_DEFAULT;
    bBaseColor = aBaseColor;
  }
  else
  {
    if ( pRc->shadeMode == D3DSHADE_FLAT )
    {
      aBaseColor = pF[FVFO_COLOR];  // use flat value - strips and fans
      bBaseColor = aBaseColor;
    }
    else
    {
      aBaseColor = pA[FVFO_COLOR];
      bBaseColor = pB[FVFO_COLOR];
    }
  }

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {    

    aColor = aBaseColor;
    bColor = aColor;
  }
  else
  {    
    aColor = aBaseColor;
    bColor = bBaseColor;
  }

  if( pRc->state & STATE_REQUIRES_WBUFFER )
  {
    // Need to check on w scaling with the viewport
    // w1 = WSCALE( FLTP(pA)[FVFO_RHW] );
    // w2 = WSCALE( FLTP(pB)[FVFO_RHW] );
    // w3 = WSCALE( FLTP(pC)[FVFO_RHW] );
    w1 = FLTP(pA)[FVFO_RHW];
    w2 = FLTP(pB)[FVFO_RHW];

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
      z1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) / 255.0f;
      z2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) / 255.0f;
    }
  }
  else
  {
    z1 = FLTP(pA)[FVFO_SZ];
    z2 = FLTP(pB)[FVFO_SZ];

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
      w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) / 255.0f;
      w2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) / 255.0f;
    }
    else if (pRc->state & STATE_REQUIRES_HWFOG)
    {
      w1 = FLTP(pA)[FVFO_RHW];
      w2 = FLTP(pB)[FVFO_RHW];
    }
  }
   
  if( pRc->state & (STATE_REQUIRES_PERSPECTIVE|STATE_REQUIRES_W_TMU0) )
  {
     oowA = FLTP(pA)[FVFO_RHW];
     oowB = FLTP(pB)[FVFO_RHW];
  }
  // if the texture is not prespective correct then the w is effectively equal to 1
  else
  {
    #ifndef CMDFIFO
     // Packet 3 treats missing W as implict 1.0f
     oowA = oowB = 1.0f;
    #endif 
  }

   if (pRc->specular)
  {
      if( pRc->shadeMode == D3DSHADE_FLAT )
      {    
          saColor = pA[FVFO_SPECULAR];
          sbColor = saColor;
      }
      else
      {    
          saColor = pA[FVFO_SPECULAR];
          sbColor = pB[FVFO_SPECULAR];
      }
  }

  lastStage = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT);

  for( i=0; i<=lastStage; i++)
  {
     txtrDesc *txtr;
     FxU32     t1CoordIndex = pRc->tCoordIndex[i];

     if( pRc->state & pRc->state_requires_tmu_flags[i] )
     {
       s1[i] = FLTP(pA)[FVFO_TU(0) + t1CoordIndex];
       t1[i] = FLTP(pA)[FVFO_TV(0) + t1CoordIndex];
    
       s2[i] = FLTP(pB)[FVFO_TU(0) + t1CoordIndex];
       t2[i] = FLTP(pB)[FVFO_TV(0) + t1CoordIndex];

       // make sure there is a valid texture at this point
       if (TXTRHNDL_SURFDATA_VALID(pRc, TS[i].textureHandle) && 
           TXTRHNDL_TXTRDESC_VALID(pRc, TS[i].textureHandle))
           txtr = TXTRHNDL_TO_TXTRDESC(pRc, TS[i].textureHandle);
       else
           txtr = 0;

       if (txtr && (txtr->txFormatFlags & TEXFMTFLG_NPT))
       {
           // for NPT, scale s & t by the texture size
           s1[i] *= txtr->txMipInfo[0].mipWidth;
           s2[i] *= txtr->txMipInfo[0].mipWidth;
           t1[i] *= txtr->txMipInfo[0].mipHeight;
           t2[i] *= txtr->txMipInfo[0].mipHeight;
       }

       WRAPST(s, t, pRc->wrapU, pRc->wrapV, i);
     }
  }

  // TODO: optimize - can move this outside this routine
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
  SETPD( cmdFifo, ghw0->suMode, (0 << SST_SU_INDEX_SHIFT) | su_mode );
  SETPD( cmdFifo, ghw0->suParamMask, su_param_mask );

  SETPH( cmdFifo,  pRc->header_table[GR_PKT3_LINESTRIP_LOAD_AB_DRAW_AB][pRc->curVertexId] );
  pRc->curVertexId = pRc->nvi_table[2][pRc->curVertexId];  // 2 verts/packet

  //Send 1st vertex    
  if(su_mode & SST_SU_W)  {
    SETFPD( cmdFifo, ghw0->vpW, oowA );
  }
  #ifndef CMDFIFO    
  else {
   // This must be first vertex parameter for direct writes
   SETFPD( cmdFifo, ghw0->vpW, oowA );
  }
  #endif

  SETFPD( cmdFifo, ghw0->vpX, FLTP(pA)[FVFO_SX] );
  SETFPD( cmdFifo, ghw0->vpY, FLTP(pA)[FVFO_SY] );
  if(su_mode & SST_SU_Z)
    SETFPD( cmdFifo, ghw0->vpZ, z1 );
  if(su_mode & SST_SU_Q)
    SETFPD( cmdFifo, ghw0->vpQ, oowA );

  for( i=0,d3di=lastStage; i<=lastStage; i++,d3di--)
  {
     if( su_param_mask & pRc->su_parammask_rgba_flags[i] )
     {
        if (pRc->specular & (1 << d3di)) // if this stage is dedicated specular
        {
            SETPD( cmdFifo, SST_TREX(ghw0,i)->vpARGB, saColor);
        }
        else
        {
            SETPD( cmdFifo, SST_TREX(ghw0,i)->vpARGB, aColor );
        }
     }

     if( su_param_mask & pRc->su_parammask_tmu_flags[i] )
     {
       SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpS, s1[d3di] );
       SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpT, t1[d3di] );
     }
     if ( su_param_mask & pRc->su_parammask_q_flags[i] )  // Check if Q set for projected textures
     {
        SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpQ, (FLTP(pA)[FVFO_TW(0) + pRc->tCoordIndex[d3di]]) );
     }
  }

  #ifndef CMDFIFO    
    P6FENCE;
    SETPD( cmdFifo, ghw0->suMode, (1 << SST_SU_INDEX_SHIFT) | su_mode );
    P6FENCE;
  #endif    

  // Send 2nd vertex
  if( su_mode & SST_SU_W)  {
    SETFPD( cmdFifo, ghw0->vpW, oowB );
  }
  #ifndef CMDFIFO
  else
    SETFPD( cmdFifo, ghw0->vpW, oowB );
  #endif

  SETFPD( cmdFifo, ghw0->vpX, FLTP(pB)[FVFO_SX] );
  SETFPD( cmdFifo, ghw0->vpY, FLTP(pB)[FVFO_SY] );
  if( su_mode & SST_SU_Z)
    SETFPD( cmdFifo, ghw0->vpZ, z2 );
  if( su_mode & SST_SU_Q)
    SETFPD( cmdFifo, ghw0->vpQ, w2 );

  for( i=0,d3di=lastStage; i<=lastStage; i++,d3di--)
  {
     if( su_param_mask & pRc->su_parammask_rgba_flags[i] )
     {
        if (pRc->specular & (1 << d3di)) // if this stage is dedicated specular
        {
            SETPD( cmdFifo, SST_TREX(ghw0,i)->vpARGB, sbColor);
        }
        else
        {
            SETPD( cmdFifo, SST_TREX(ghw0,i)->vpARGB, bColor );
        }
     }

     if( pRc->sst.suParamMask.vFxU32 & pRc->su_parammask_tmu_flags[i] )
     {
        SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpS, s2[d3di] );
        SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpT, t2[d3di] );
     }
     if ( su_param_mask & pRc->su_parammask_q_flags[i] )  // Check if Q set for projected textures
     {
        SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpQ, (FLTP(pB)[FVFO_TW(0) + pRc->tCoordIndex[d3di]]) );
     }
  }

  #ifndef CMDFIFO    
    P6FENCE;
    SETPD( cmdFifo, ghw0->suDrawCmd, 
           (0<<SST_SU_VERTEX_A_SHIFT) | (1<<SST_SU_VERTEX_B_SHIFT));
    P6FENCE;
  #endif

  CMDFIFO_EPILOG( cmdFifo );

} 

