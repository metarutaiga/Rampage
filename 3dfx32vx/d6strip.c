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
**  24   3dfx      1.23        11/22/00 Dale  Kenaston  Sage triangle fan 
**       primitive. Added vstart as a parameter to the rendering functions.
**  23   3dfx      1.22        11/13/00 Brent Burton    Changed arg ordering and
**       added new args to render functions.  Added 16 and 32 bit index support 
**       for indexed prims.  Better DX7 to DX8 render function sharing.
**  22   3dfx      1.21        10/31/00 Brent Burton    Added DWORD vstride 
**       argument to rasterizer functions to replace FVFO_SIZE usage for DX8.
**  21   3dfx      1.20        10/4/00  Dale  Kenaston  New Sage macros. Changed
**       IS_RAGE in dp2StripAll to IS_SAGE_ACTIVE.
**  20   3dfx      1.19        9/22/00  Dale  Kenaston  Sage packet3, register 
**       and quickturn initialization changes  Added packet 3 size setup and 
**       restore for specular triangles.
**  19   3dfx      1.18        7/24/00  Brian Danielson Changes to implement 
**       renderstate and shadow register management.
**  18   3dfx      1.17        7/10/00  Michel Conrad   Remove pixel offset 
**       cruft.
**  17   3dfx      1.16        6/15/00  Michel Conrad   Delete really old check 
**       in comments. Delete drawglobal check.
**  16   3dfx      1.15        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  15   3dfx      1.14        4/24/00  Brian Danielson Added Pint and Line fill
**       mode functionality. Also fixed some oversights I found in line and 
**       point code.
**  14   3dfx      1.13        1/27/00  Evan Leland     DX7 changes
**  13   3dfx      1.12        1/25/00  Evan Leland     updates for dx7
**  12   3dfx      1.11        1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  11   3dfx      1.10        12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  10   3dfx      1.9         11/11/99 Brian Danielson Another fix for trivial 
**       rejection clipping. Initialze clip rect, proper vertex processing.
**  9    3dfx      1.8         11/9/99  Brian Danielson Fixed bug in NoSpec 
**       trivial rejection code. Added Compile Time option for rejection code.
**  8    3dfx      1.7         10/25/99 Brian Danielson Updated Trivial 
**       rejection Clip code with new macro and nospec loop.
**  7    3dfx      1.6         10/12/99 Evan Leland     added NPT tex coord 
**       scaling to dp2TriangleAll
**  6    3dfx      1.5         10/1/99  Mark Einkauf    Complete HW_ACCESS 
**       macros work
**  5    3dfx      1.4         10/1/99  Brent Burton    Removed vars i and vi, 
**       added loop counters d3di and tmui to be clearer.  Code cleanup.  
**       Removed all references to prc->dx5texture.  Fixed second-pass specular 
**       Z-buffer test.
**  4    3dfx      1.3         9/29/99  Brian Danielson Added Guard Band 
**       clipping support and trivial rejection clipping.
**  3    3dfx      1.2         9/28/99  Brent Burton    Renamed prc->texture to 
**       prc->dx5texture.
**  2    3dfx      1.1         9/14/99  Mark Einkauf    Init W if perspective, 
**       or TMU0 needs it; update CMDFIFO only optimized fan,tri,strip render 
**       routines to match general "..All" versions.
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
*/

#if defined( PERFTEST )
  #undef SETPH
  #undef SETPD
  #undef SETFPD
  #define FIFO_NOP  0

  #if( PERFTEST == PCI_NOP )
    #define SETPH( hwPtr, data )              SETCF( hwPtr, FIFO_NOP )
    #define SETPD( hwPtr, hwRegister, data )  SETCF( hwPtr, FIFO_NOP )
    #define SETFPD( hwPtr, hwRegister, data ) SETCF( hwPtr, FIFO_NOP )
  #else // PERFTEST == PCI_NULL
    #define SETPH( hwPtr, data )
    #define SETPD( hwPtr, hwRegister, data )
    #define SETFPD( hwPtr, hwRegister, data )
  #endif  
#endif


/*-------------------------------------------------------------------
Function Name:  dp2StripAllFill

Description:    This function handles drawing DrawPrimitive strips in
                wireframe and point fill modes.

Information:    If (PERFTEST == TRI_NULL) this function immediately
                returns without doing any work (null-driver)

Return:         void
-------------------------------------------------------------------*/
void __stdcall  dp2StripAllFill (RC *pRc, DWORD count, DWORD vertexType,
                                 VINDEX idx,       DWORD istride,
                                 LPDWORD vertices, DWORD vstride,
                                 DWORD vstart)
{
  LPDWORD       pA, pB, pC, pF;
  BOOL          flip = 0;
  int           sign;

#if( PERFTEST == TRI_NULL )
  return;
#endif

  switch( pRc->fillMode )
  {
    case D3DFILL_WIREFRAME :

      TRI_INIT();

      for( ; count > 0; count-- )
      {
        TRI_NEXT();

        ((float*)&sign)[0] = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                             ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          dp2Line( pRc, pF, pA, pB, vertexType );
          dp2Line( pRc, pF, pB, pC, vertexType );
          dp2Line( pRc, pF, pC, pA, vertexType );
        }
      }
      break;

    case D3DFILL_POINT :

      TRI_INIT();

      for( ; count > 0; count-- )
      {
        TRI_NEXT();

        ((float*)&sign)[0] = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                             ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));

        sign &= 0x80000000;

        if ((sign ^ pRc->cullMask) != 0x80000000)
        {
          dp2Point( pRc, pF, pA, vertexType );
          dp2Point( pRc, pF, pB, vertexType );
          dp2Point( pRc, pF, pC, vertexType );
        }
      }
      break;

    default :
      D3DPRINT( 0, "Illegal D3D Fill Mode %d", pRc->fillMode );
  }
}



//-------------------------------------------------------------------

void __stdcall dp2StripAll (RC *pRc, DWORD count, DWORD vertexType,
                            VINDEX idx,       DWORD istride,
                            LPDWORD vertices, DWORD vstride,
                            DWORD vstart)
{
    SETUP_PPDEV(pRc)
        LPDWORD       pA, pB, pC, pF;
    float         w1, w2, w3;
    float         z1, z2, z3;
    float         s1[8], s2[8], s3[8], t1[8], t2[8], t3[8];
    D3DCOLOR      aColor, bColor, cColor;
    D3DCOLOR      saColor, sbColor, scColor;
    // float         area;
    int           sign;
    FxU32         su_mode = pRc->sst.suMode.vFxU32;
    FxU32         su_param_mask = pRc->sst.suParamMask.vFxU32;
    BOOL          flip = 0;
    FxU32         tmui, d3di, lastStage;
    txtrDesc     *txtr;
    float         oowA, oowB, oowC;

    CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
    return;
#endif

    HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D);

    TRI_INIT();

    for( ; count > 0; count-- )
    {  
#ifdef  TRIVIAL_REJECTION_CLIPPING
        DWORD   dwCodeA, dwCodeB, dwCodeC;
#endif  // TRIVIAL_REJECTION_CLIPPING

        TRI_NEXT();

#ifdef  TRIVIAL_REJECTION_CLIPPING
        // With Guard Band clipping in place we first test each triangle for trivial rejection here
        // so that if rejected then no triangle data is sent to hardware (performance boost?). We do
        // the test before the cull test as the clip test is less computationally involved.
        // If any bitwise code value is set for all three vertices, then we can trivially reject.
        TRIVIAL_CLIP(pA,dwCodeA);
        TRIVIAL_CLIP(pB,dwCodeB);
        TRIVIAL_CLIP(pC,dwCodeC);

        if ( !( dwCodeA & dwCodeB & dwCodeC ) )
#endif  // TRIVIAL_REJECTION_CLIPPING
        {
            // Culling check
#if 0
            area = ((FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX]) * (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY])) -
                ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX]) * (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));
            sign = (*(unsigned long *)&area) & 0x80000000;
#endif

            ((float*)&sign)[0] = ( (FLTP(pA)[FVFO_SX] - FLTP(pB)[FVFO_SX])
                                   *  (FLTP(pB)[FVFO_SY] - FLTP(pC)[FVFO_SY]))
                - ((FLTP(pB)[FVFO_SX] - FLTP(pC)[FVFO_SX])
                   *  (FLTP(pA)[FVFO_SY] - FLTP(pB)[FVFO_SY]));
            sign &= 0x80000000;

            if ((sign ^ pRc->cullMask) != 0x80000000)
            {
                CMDFIFO_CHECKROOM( cmdFifo, 3 + pRc->pkt3Size[3]);

                if ( !pRc->bFVFDiffusePresent )
                {
                  aColor = FVF_DIFFUSE_DEFAULT;
                  bColor = aColor;
                  cColor = aColor;
                }
                else
                {
                  if ( pRc->shadeMode == D3DSHADE_FLAT )
                  {    
                      aColor = pF[FVFO_COLOR];

                      // Use aColor but preserve per-vertex alpha
                      // bColor = (aColor & 0x00FFFFFF) | (pB[FVFO_COLOR] & 0xFF000000);
                      // cColor = (aColor & 0x00FFFFFF) | (pC[FVFO_COLOR] & 0xFF000000);
                      bColor = aColor;
                      cColor = aColor;
                  }
                  else
                  {    
                      aColor = pA[FVFO_COLOR];
                      bColor = pB[FVFO_COLOR];
                      cColor = pC[FVFO_COLOR];
                  }
                }

                if( pRc->state & STATE_REQUIRES_WBUFFER )
                {
                    // Need to check on w scaling with the viewport
                    // w1 = WSCALE( FLTP(pA)[FVFO_RHW] );
                    // w2 = WSCALE( FLTP(pB)[FVFO_RHW] );
                    // w3 = WSCALE( FLTP(pC)[FVFO_RHW] );
                    w1 = FLTP(pA)[FVFO_RHW];
                    w2 = FLTP(pB)[FVFO_RHW];
                    w3 = FLTP(pC)[FVFO_RHW];

          
                    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
                    {
                        // this wasn't divided before - caused overflow - EJH
                        z1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) / 255.0f;
                        z2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) / 255.0f;
                        z3 = (float)(255 - RGBA_GETALPHA(pC[FVFO_SPECULAR])) / 255.0f;
                    }
                }
                else
                {
                    z1 = FLTP(pA)[FVFO_SZ];
                    z2 = FLTP(pB)[FVFO_SZ];
                    z3 = FLTP(pC)[FVFO_SZ];

                    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
                    {
                        // this wasn't divided before - caused overflow - EJH
                        w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) / 255.0f;
                        w2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) / 255.0f;
                        w3 = (float)(255 - RGBA_GETALPHA(pC[FVFO_SPECULAR])) / 255.0f;
                    }
                    else if (pRc->state & STATE_REQUIRES_HWFOG)
                    {
                        w1 = FLTP(pA)[FVFO_RHW];
                        w2 = FLTP(pB)[FVFO_RHW];
                        w3 = FLTP(pC)[FVFO_RHW];
                    }
                }

                if( pRc->state & (STATE_REQUIRES_PERSPECTIVE|STATE_REQUIRES_W_TMU0) )
                {
                    oowA = FLTP(pA)[FVFO_RHW];
                    oowB = FLTP(pB)[FVFO_RHW];
                    oowC = FLTP(pC)[FVFO_RHW];
                }
#ifndef CMDFIFO // non-CMDFIFO requires w always, so always initialize it
                // if the texture is not prespective correct then the w is effectively equal to 1
                else
                {

                    // Packet 3 treats missing W as implict 1.0f
                    oowA = oowB = oowC = 1.0f;
                }
#endif 

                if (pRc->specular)
                {
                    if( pRc->shadeMode == D3DSHADE_FLAT )
                    {    
                        saColor = pA[FVFO_SPECULAR];
                        sbColor = saColor;
                        scColor = saColor;
                    }
                    else
                    {    
                        saColor = pA[FVFO_SPECULAR];
                        sbColor = pB[FVFO_SPECULAR];
                        scColor = pC[FVFO_SPECULAR];
                    }
                }

                lastStage = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT);

                for( d3di=0; d3di <= lastStage; d3di++)
                {
                    if( pRc->state & pRc->state_requires_tmu_flags[d3di] )
                    {
                        FxU32 t1CoordIndex = pRc->tCoordIndex[d3di];
                        s1[d3di] = FLTP(pA)[FVFO_TU(0) + t1CoordIndex];
                        t1[d3di] = FLTP(pA)[FVFO_TV(0) + t1CoordIndex];
          
                        s2[d3di] = FLTP(pB)[FVFO_TU(0) + t1CoordIndex];
                        t2[d3di] = FLTP(pB)[FVFO_TV(0) + t1CoordIndex];

                        s3[d3di] = FLTP(pC)[FVFO_TU(0) + t1CoordIndex];
                        t3[d3di] = FLTP(pC)[FVFO_TV(0) + t1CoordIndex];

                        // make sure there is a valid texture at this point
                        if (TXTRHNDL_SURFDATA_VALID(pRc, TS[d3di].textureHandle) && 
                            TXTRHNDL_TXTRDESC_VALID(pRc, TS[d3di].textureHandle))
                            txtr = TXTRHNDL_TO_TXTRDESC(pRc, TS[d3di].textureHandle);
                        else
                            txtr = 0;

                        if (txtr && (txtr->txFormatFlags & TEXFMTFLG_NPT))
                        {
                            // for NPT, scale s & t by the texture size
                            s1[d3di] *= txtr->txMipInfo[0].mipWidth;
                            s2[d3di] *= txtr->txMipInfo[0].mipWidth;
                            s3[d3di] *= txtr->txMipInfo[0].mipWidth;
                            t1[d3di] *= txtr->txMipInfo[0].mipHeight;
                            t2[d3di] *= txtr->txMipInfo[0].mipHeight;
                            t3[d3di] *= txtr->txMipInfo[0].mipHeight;
                        }
                        WRAPI( s, (pRc->wrap[d3di] & D3DWRAP_U), d3di);
                        WRAPI( t, (pRc->wrap[d3di] & D3DWRAP_V), d3di);
                    }
                }

                // TODO: optimize - can move this outside the loop, unless we're doing specular
                SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
                SETPD( cmdFifo, ghw0->suMode, (0 << SST_SU_INDEX_SHIFT) | su_mode );
                SETPD( cmdFifo, ghw0->suParamMask, su_param_mask );

                //SETPH( cmdFifo, CMDFIFO_SIMPLE_TRI );
                SETPH( cmdFifo,  pRc->header_table[GR_PKT3_TRISTRIP_LOAD_ABC_DRAW_ABC][pRc->curVertexId] );
                pRc->curVertexId = pRc->nvi_table[3][pRc->curVertexId];  // 3 verts/packet

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
                    SETFPD( cmdFifo, ghw0->vpQ, w1 );	// was oowA - pretty sure it's supposed to be w1 - EJH

                // We have to write out registers in 0..lastStage order.
                for( tmui=0,d3di=lastStage; tmui <= lastStage; tmui++,d3di--)
                {
                    if( su_param_mask & pRc->su_parammask_rgba_flags[tmui] )
                    {
                        if (pRc->specular & (1 << d3di)) // if this stage is dedicated specular
                        {
                            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->vpARGB, saColor);
                        }
                        else
                        {
                            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->vpARGB, aColor );
                        }
                    }

                    if( su_param_mask & pRc->su_parammask_tmu_flags[tmui] )
                    {
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, s1[d3di] );
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, t1[d3di] );
                    }
                    if ( su_param_mask & pRc->su_parammask_q_flags[tmui] )  // Check if Q set for projected textures
                    {
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpQ, (FLTP(pA)[FVFO_TW(0) + pRc->tCoordIndex[d3di]]) );
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

                // We have to write out registers in 0..lastStage order.
                // Match texture coords with lastStage..0 order with 'd3di'.
                for( tmui=0,d3di=lastStage; tmui<=lastStage; tmui++,d3di--)
                {
                    if( su_param_mask & pRc->su_parammask_rgba_flags[tmui] )
                    {
                        if (pRc->specular & (1 << d3di)) // if this stage is dedicated specular
                        {
                            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->vpARGB, sbColor);
                        }
                        else
                        {
                            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->vpARGB, bColor );
                        }
                    }
            
                    if( su_param_mask & pRc->su_parammask_tmu_flags[tmui] )
                    {
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, s2[d3di] );
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, t2[d3di] );
                    }
                    if ( su_param_mask & pRc->su_parammask_q_flags[tmui] )  // Check if Q set for projected textures
                    {
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpQ, (FLTP(pB)[FVFO_TW(0) + pRc->tCoordIndex[d3di]]) );
                    }
                }

#ifndef CMDFIFO    
                P6FENCE;
                SETPD( cmdFifo, ghw0->suMode, (2 << SST_SU_INDEX_SHIFT) | su_mode );
                P6FENCE;
#endif

                // Send 3rd vertex
                if(su_mode & SST_SU_W)  {
                    SETFPD( cmdFifo, ghw0->vpW, oowC );
                }
#ifndef CMDFIFO    
                else {
                    // This must be first vertex parameter for direct writes
                    SETFPD( cmdFifo, ghw0->vpW, oowC );
                }
#endif

                SETFPD( cmdFifo, ghw0->vpX, FLTP(pC)[FVFO_SX] );
                SETFPD( cmdFifo, ghw0->vpY, FLTP(pC)[FVFO_SY] );
                if(su_mode & SST_SU_Z)
                    SETFPD( cmdFifo, ghw0->vpZ, z3 );
                if(su_mode & SST_SU_Q)
                    SETFPD( cmdFifo, ghw0->vpQ, w3 );

                // We have to write out registers in 0..lastStage order.
                // Match texture coords with lastStage..0 order with 'd3di'.
                for( tmui=0,d3di=lastStage; tmui <= lastStage; tmui++,d3di--)
                {
                    if( su_param_mask & pRc->su_parammask_rgba_flags[tmui] )
                    {
                        if (pRc->specular & (1 << d3di)) // if this stage is dedicated specular
                        {
                            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->vpARGB, scColor );
                        }
                        else
                        {
                            SETPD( cmdFifo, SST_TREX(ghw0,tmui)->vpARGB, cColor );
                        }
                    }
                    if( su_param_mask & pRc->su_parammask_tmu_flags[tmui] )
                    {
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, s3[d3di] );
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, t3[d3di] );
                    }
                    if ( su_param_mask & pRc->su_parammask_q_flags[tmui] )  // Check if Q set for projected textures
                    {
                        SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpQ, (FLTP(pC)[FVFO_TW(0) + pRc->tCoordIndex[d3di]]) );
                    }
                }

#ifndef CMDFIFO    
                P6FENCE;
                SETPD( cmdFifo, ghw0->suDrawCmd, SST_SU_DRAWTRI
                       | (0<<SST_SU_VERTEX_A_SHIFT)
                       | (1<<SST_SU_VERTEX_B_SHIFT)
                       | (2<<SST_SU_VERTEX_C_SHIFT));
                
                P6FENCE;
#endif


                // Specular on Textures
                // specular highlights on texture where the specular color is not black
                if (pRc->specular == SST_TWOPASS_SPECULAR
                    && CI_MASKALPHA(saColor | sbColor | scColor))
                {
                    // if specular is turned on then we need to add in the specular color
                    // and this means re-render the triangle gouraud shaded using the specular
                    // color information and then add it to the textured triangle we just rendered.
                    // We can't just add it to the triangle color because this color would be
                    // blended or replaced with the texture and highlights are added into textures.
                    //
                    FxU32 fbzMode, suflag;

                    if(IS_SAGE_ACTIVE)
                    {
                        FxU32 size;

                        //     X   Y   ARGB
                        size = 1 + 1 + 1;
                        if(pRc->state & STATE_REQUIRES_ZBUFFER)
                            size++;

                        // We need to tell SAGE what PKT3 size is implied by suParamMask and suMode.
                        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1);
                        SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1, kPkt3SizeReg, 0, 0));
                        SETCF( cmdFifo, size);
                    }

                    // second pass - worst case size
                    //  2*=set/reset,2=fog,2=alpha,2=z,7=tcu/ccu,3=sumode/mask(temp),3=nVtx,4=x,y,z,col

                    //  note we don't use the pkt3 size table, since offsets into it depend on vtx
                    //  size, and vtx size of specular pass may differ from first pass.
                    //  Specular is supposedly rare, so we'll do it naively unless we need to optimize

                    CMDFIFO_CHECKROOM( cmdFifo, (2*(2+2+1+2+7))+3+(3*6)+14);
          
                    // Where FOG() = fog function
                    // FOG(T1 + T2) = AlphaFog * FogColor + (1 - AlphaFog)[T1 +T2]   
                    //    Pass 1    = AlphaFog * FogColor + (1 - AlphaFog)T1 
                    //    Pass 2    =                       (1 - AlphaFog)T2

                    if( pRc->shadeMode == D3DSHADE_FLAT )
                    {    
                        saColor = pA[FVFO_SPECULAR];
                        sbColor = saColor;
                        scColor = saColor;
                    }
                    else
                    {    
                        saColor = pA[FVFO_SPECULAR];
                        sbColor = pB[FVFO_SPECULAR];
                        scColor = pC[FVFO_SPECULAR];
                    }

                    if (pRc->fogEnable)
                    {
                        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFogMode ) );
                        SETPD( cmdFifo, ghw0->peFogMode, (pRc->sst.peFogMode.vFxU32 | SST_PE_FOG_ADD) & ~SST_PE_EN_FOGGING );         
                    }
          
                    // let's just add in the color ignoring the effect on alpha blending for
                    // the time being
                    // (Src * 1 + Dst * 1)
                    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peAlphaMode ) );
                    SETPD( cmdFifo, ghw0->peAlphaMode, (
                                                        SST_PE_EN_ALPHA_BLEND 
                                                        | (SST_PE_ABLEND_ONE << SST_PE_RGB_SRC_FACT_SHIFT) 
                                                        | (SST_PE_ABLEND_ONE << SST_PE_RGB_DST_FACT_SHIFT)) );

                    // W,Q don't cares for specular pass
                    suflag = su_mode & ~(SST_SU_W | SST_SU_Q );
            
                    if ( pRc->zEnable )
                    {
                        // if z-buffering then this triangle z values equals the values written on pass 1

                        fbzMode  =  pRc->sst.peFbzMode.vFxU32 & ~(SST_PE_ZFUNC_LT | SST_PE_ZFUNC_EQ | SST_PE_ZFUNC_GT) ;
                        fbzMode |= SST_PE_ZFUNC_EQ ;

                        // supposed to have FLUSH_PCACHE before fbzmode change XXX allocate space
                        SETMOP( cmdFifo, SST_MOP_FLUSH_PCACHE );

                        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFbzMode ) );
                        SETPD( cmdFifo, ghw0->peFbzMode, fbzMode );
            
                        if( !(pRc->state & STATE_REQUIRES_ZBUFFER) )
                            suflag &= ~(SST_SU_Z );
                    }

                    // texture mapping off and texture blending off
                    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, taControl ) );
                    SETPD( cmdFifo, ghw0->taControl,  pRc->sst.taControl.vFxU32 & ~SST_TA_NUM_TMUS);

                    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3|R4, SST_UNIT_TMU_0, taTcuColor));        
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuColor, TCC( ZERO, ZERO, ZERO, ZERO ));
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuAlpha, TCA( ZERO, ZERO, ZERO, ZERO ));
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuColor, CCC( ZERO, ZERO, ZERO, CITER ));
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuAlpha, CCA( ZERO, ZERO, ZERO, ZERO ));

                    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
                    SETPD( cmdFifo, ghw0->suMode, (0 << SST_SU_INDEX_SHIFT) | suflag );
                    SETPD( cmdFifo, ghw0->suParamMask, SST_SU_RGB0 | SST_SU_A0 );

                    //  note we don't use the pkt3 header tables, since offsets into them depend on vtx
                    //  size, and vtx size of specular pass may differ from first pass.
                    //  Specular is supposedly rare, so we'll do it naively unless we need to optimize
                    SETPH( cmdFifo, CMDFIFO_SIMPLE_TRI );

                    // Send 1st vertex      
#ifndef CMDFIFO    
                    // This must be first vertex parameter for direct writes
                    SETFPD( cmdFifo, ghw0->vpW, 1.0f );
#endif

                    SETFPD( cmdFifo, ghw0->vpX, FLTP(pA)[FVFO_SX] );
                    SETFPD( cmdFifo, ghw0->vpY, FLTP(pA)[FVFO_SY] );
                    if(suflag & SST_SU_Z)
                        SETFPD( cmdFifo, ghw0->vpZ, z1 );

                    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->vpARGB, saColor );

#ifndef CMDFIFO    
                    P6FENCE;
                    SETPD( cmdFifo, ghw0->suMode, (1 << SST_SU_INDEX_SHIFT) | suflag );
                    P6FENCE;
#endif    

                    //Send 2nd vertex    
#ifndef CMDFIFO    
                    SETFPD( cmdFifo, ghw0->vpW, 1.0f );
#endif

                    SETFPD( cmdFifo, ghw0->vpX, FLTP(pB)[FVFO_SX] );
                    SETFPD( cmdFifo, ghw0->vpY, FLTP(pB)[FVFO_SY] );

                    if(suflag & SST_SU_Z)
                        SETFPD( cmdFifo, ghw0->vpZ, z2 );

                    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->vpARGB, sbColor );


#ifndef CMDFIFO    
                    P6FENCE;
                    SETPD( cmdFifo, ghw0->suMode, (2 << SST_SU_INDEX_SHIFT) | suflag );
                    P6FENCE;
#endif    


                    // Send 3rd vertex
#ifndef CMDFIFO    
                    SETFPD( cmdFifo, ghw0->vpW, 1.0f );
#endif

                    SETFPD( cmdFifo, ghw0->vpX, FLTP(pC)[FVFO_SX] );
                    SETFPD( cmdFifo, ghw0->vpY, FLTP(pC)[FVFO_SY] );

                    if(suflag & SST_SU_Z)
                        SETFPD( cmdFifo, ghw0->vpZ, z3 );

                    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->vpARGB, scColor );

#ifndef CMDFIFO    
                    P6FENCE;
                    SETPD( cmdFifo, ghw0->suDrawCmd, SST_SU_DRAWTRI
                           | (0<<SST_SU_VERTEX_A_SHIFT)
                           | (1<<SST_SU_VERTEX_B_SHIFT)
                           | (2<<SST_SU_VERTEX_C_SHIFT));
                    P6FENCE;
#endif
      
                    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, taControl ) );
                    SETPD( cmdFifo, ghw0->taControl,  pRc->sst.taControl.vFxU32);
                    // change to restore from the last active stage

                    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3|R4, SST_UNIT_TMU_0, taTcuColor));        
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuColor, pRc->sst.TMU[lastStage].taTcuColor.vFxU32 );
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuAlpha, pRc->sst.TMU[lastStage].taTcuAlpha.vFxU32 );
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuColor, pRc->sst.TMU[lastStage].taCcuColor.vFxU32 );
                    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuAlpha, pRc->sst.TMU[lastStage].taCcuAlpha.vFxU32 );

                    if (pRc->fogEnable)
                    {
                        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFogMode ) );
                        SETPD( cmdFifo, ghw0->peFogMode, pRc->sst.peFogMode.vFxU32 );
                    }
          
                    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peAlphaMode ) );
                    SETPD( cmdFifo, ghw0->peAlphaMode, pRc->sst.peAlphaMode.vFxU32 );

                    // supposed to have FLUSH_PCACHE before fbzmode change XXXX allocate space
                    SETMOP( cmdFifo, SST_MOP_FLUSH_PCACHE );

                    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFbzMode ) );
                    SETPD( cmdFifo, ghw0->peFbzMode, pRc->sst.peFbzMode.vFxU32 );

                    if(IS_SAGE_ACTIVE)
                    {
                        // We need to tell SAGE what PKT3 size is implied by suParamMask and suMode.
                        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1);
                        SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1, kPkt3SizeReg, 0, 0));
                        SETCF( cmdFifo, pRc->pkt3Size[1] - pRc->pkt3Size[0]);
                    }

                } // two-pass specular
            } // Culling check
        } // GB Clip trivial reject
    } // every triangle
  
    HW_ACCESS_EXIT(ACCESS_3D);

    CMDFIFO_EPILOG( cmdFifo );

} // Independent Triangle rendering

// This is needed because you can't put defines in the processVertices macro.
#define PT_FOR_DX7()                                                        \
    if ( su_param_mask & pRc->su_parammask_q_flags[i] )                     \
    {                                                                       \
        SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpQ,                             \
               (FLTP(pA)[FVFO_TW(0) + pRc->tCoordIndex[d3di]]) );           \
    }                                                                       \

#define COLOR_FOR_DX7()                                                     \
    if ( !pRc->bFVFDiffusePresent )                                         \
    {                                                                       \
        aColor = FVF_DIFFUSE_DEFAULT;                                       \
    }                                                                       \
    else

#define processVertices(  numVertsToLoad ){                                 \
    for ( ; numVertsToLoad>0 ; numVertsToLoad-- )                           \
    {                                                                       \
      COLOR_FOR_DX7()                                                       \
      {                                                                     \
        if ( (pRc->specular) && !(pRc->sst.suParamMask.vFxU32 & SST_SU_ST_ALL) ) \
        {                                                                   \
          CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);              \
        }                                                                   \
        else                                                                                \
        {                                                                                   \
          aColor = pA[FVFO_COLOR];                                                          \
        }                                                                                   \
      }                                                                                     \
      if( pRc->state & STATE_REQUIRES_WBUFFER )                                             \
      {                                                                                     \
        w1 = FLTP(pA)[FVFO_RHW];                                                            \
                                                                                            \
        if (pRc->state & STATE_REQUIRES_VERTEXFOG)                                          \
        {                                                                                   \
          z1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]))/255.0f;                      \
        }                                                                                   \
      }                                                                                     \
      else                                                                                  \
      {                                                                                     \
        z1 = FLTP(pA)[FVFO_SZ];                                                             \
                                                                                            \
        if (pRc->state & STATE_REQUIRES_VERTEXFOG)                                          \
        {                                                                                   \
          w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]))/255.0f;                      \
        }                                                                                   \
        else if (pRc->state & STATE_REQUIRES_HWFOG)                                         \
        {                                                                                   \
          w1 = FLTP(pA)[FVFO_RHW];                                                          \
        }                                                                                   \
      }                                                                                     \
                                                                                            \
      if( pRc->state & (STATE_REQUIRES_PERSPECTIVE|STATE_REQUIRES_W_TMU0) )                 \
      {                                                                                     \
         oowA = FLTP(pA)[FVFO_RHW];                                                         \
      }                                                                                     \
                                                                                            \
      lastStage = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT); \
                                                                                            \
      for( i=0; i<lastStage+1; i++)                                                         \
      {                                                                                     \
         if( pRc->state & pRc->state_requires_tmu_flags[i] )                                \
         {                                                                                  \
           FxU32 t1CoordIndex = pRc->tCoordIndex[i];                                        \
           s1[i] = FLTP(pA)[FVFO_TU(0) + t1CoordIndex];                                     \
           t1[i] = FLTP(pA)[FVFO_TV(0) + t1CoordIndex];                                     \
                                                                                            \
           /* make sure there is a valid texture at this point */                           \
           if (TXTRHNDL_SURFDATA_VALID(pRc, i) &&                                           \
               TXTRHNDL_TXTRDESC_VALID(pRc, i))                                             \
               txtr = TXTRHNDL_TO_TXTRDESC(pRc, i);                                         \
           else                                                                             \
               txtr = 0;                                                                    \
                                                                                            \
           if (txtr && (txtr->txFormatFlags & TEXFMTFLG_NPT))                               \
           {                                                                                \
               /* for NPT we scale all s,t coordinates by the texture size */               \
               s1[i] *= txtr->txMipInfo[0].mipWidth;                                        \
               t1[i] *= txtr->txMipInfo[0].mipHeight;                                       \
           }                                                                                \
           /* No WRAP support in tstrip - requires independent tri's for fixup macro*/      \
        }                                                                                   \
      }                                                                                     \
                                                                                            \
      if(su_mode & SST_SU_W)  {                                                             \
        SETFPD( cmdFifo, ghw0->vpW, oowA );                                                 \
      }                                                                                     \
                                                                                            \
      SETFPD( cmdFifo, ghw0->vpX, FLTP(pA)[FVFO_SX] );                                      \
      SETFPD( cmdFifo, ghw0->vpY, FLTP(pA)[FVFO_SY] );                                      \
      if(su_mode & SST_SU_Z)                                                                \
        SETFPD( cmdFifo, ghw0->vpZ, z1 );                                                   \
      if(su_mode & SST_SU_Q)                                                                \
        SETFPD( cmdFifo, ghw0->vpQ, w1 );                                                   \
                                                                                            \
      for( i=0,d3di=lastStage; i<lastStage+1; i++,d3di--)                                   \
      {                                                                                     \
         if( su_param_mask & pRc->su_parammask_rgba_flags[i] )                              \
           SETPD( cmdFifo, SST_TREX(ghw0,i)->vpARGB, aColor );                              \
                                                                                            \
         if( su_param_mask & pRc->su_parammask_tmu_flags[i] )                               \
         {                                                                                  \
           SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpS, s1[d3di] );                              \
           SETFPD( cmdFifo, SST_TREX(ghw0,i)->vpT, t1[d3di] );                              \
         }                                                                                  \
         PT_FOR_DX7();                                                                      \
      }                                                                                     \
                                                                                            \
      TRI_NEXT_VA();                                                                        \
                                                                                            \
    }                                                                                       \
                                                                                            \
}

// Strip - semi-optimized, doesn't do specular, and doesn't resend shared vertices
//  it does still do tests in the main loop to determine what parameters to send
//  WORKS WITH CMDFIFO ONLY - DON'T USE W/ DIRECT WRITES
void __stdcall dp2StripNoSpec (RC *pRc, DWORD count, DWORD vertexType,
                               VINDEX idx,       DWORD istride,
                               LPDWORD vertices, DWORD vstride,
                               DWORD vstart)
{
  SETUP_PPDEV(pRc)
  LPDWORD       pA;
#ifdef  TRIVIAL_REJECTION_CLIPPING
  LPDWORD       pVerts;
  VINDEX        pIdx;
#endif
  float         w1;
  float         z1;
  float         s1[8], t1[8];
  D3DCOLOR      aColor;
  FxU32         su_mode = pRc->sst.suMode.vFxU32;
  FxU32         su_param_mask = pRc->sst.suParamMask.vFxU32;
  FxU32         i, d3di, lastStage;
  txtrDesc     *txtr;

  float         oowA;

  FxI32         numVertsToLoad;

  CMDFIFO_PROLOG(cmdFifo);

#ifdef  TRIVIAL_REJECTION_CLIPPING
    // Do not process strips if all vertices are trivially clip rejected
    if ( count >= 2)
    {
        DWORD   dwCodeSum = 0x0F;
        FxU32   i;

        pVerts = vertices;  // Save this so it can be restored for actual loop below
        pIdx   = idx;       // Save this so it can be restored for actual loop below
        TRI_INIT_VA();
        numVertsToLoad = 4;
        for ( i = count; i >= 2; i-=2 )
        {
            for ( ; numVertsToLoad>0 ; numVertsToLoad-- )
            {
                DWORD   dwCode;

                TRIVIAL_CLIP(pA,dwCode);
                dwCodeSum &= dwCode;
                TRI_NEXT_VA();              // Get next vertex
            } // for each vertex of a triangle
            numVertsToLoad=2;               // load 2 new vertices on all but 1st pkt (where we load 3)
            if ( dwCodeSum == 0 )
                break;
        } // for each triangle
        if ( dwCodeSum != 0 )
        {
            CMDFIFO_EPILOG( cmdFifo );
            return;                         // All triangles have been rejected
        }
        vertices = pVerts;                  // Restore original vertex pointer
        idx      = pIdx;                    // Restore original index pointer
    }
#endif  // TRIVIAL_REJECTION_CLIPPING

#if( PERFTEST == TRI_NULL )
  return;
#endif

  HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D);

  TRI_INIT_VA();

  numVertsToLoad = 4;

  CMDFIFO_CHECKROOM( cmdFifo, 3 );

  SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
  SETPD( cmdFifo, ghw0->suMode, (0 << SST_SU_INDEX_SHIFT) | su_mode );
  SETPD( cmdFifo, ghw0->suParamMask, su_param_mask );

  for( ; count>=2 ; count-=2 )
  {  

    CMDFIFO_CHECKROOM( cmdFifo, pRc->pkt3Size[numVertsToLoad]);

    SETPH( cmdFifo,  pRc->header_table[GR_PKT3_TRISTRIP_LOAD_ABCD_DRAW_ABCD+4-numVertsToLoad][pRc->curVertexId] );
                                    
    pRc->curVertexId = pRc->nvi_table[numVertsToLoad][pRc->curVertexId];

    processVertices( numVertsToLoad );
    numVertsToLoad=2; // load 2 new vertices on all but 1st pkt

  } // every triangle

  if (numVertsToLoad == 4)
  {
      // never entered loop above, count must be 1 or 0
      // if not 1, do nothing since it's not a triangle
      if (count==1)
      {
        CMDFIFO_CHECKROOM( cmdFifo, pRc->pkt3Size[3]);
        SETPH( cmdFifo,  pRc->header_table[GR_PKT3_TRISTRIP_LOAD_ABC_DRAW_ABC][pRc->curVertexId] );
        pRc->curVertexId = pRc->nvi_table[3][pRc->curVertexId];
        numVertsToLoad = 3;
      }
      else
      {
          // bogus strip - do nothing
          numVertsToLoad = 0;
      }
  }
  else
  {
      // We've made it through the main loop at least once.  This means that
      // count can only be 1 or 0 (since count was at least 2, and we only
      // subtract 2 from count at each iteration).  We only need to draw a triangle
      // if count is 1.  If count is 0, and we've gone through the main loop above,
      // that means that count was previously 2, which means we downloaded and drew 
      // the whole strip and nothing is left over.
      if (count==1)
      {
          CMDFIFO_CHECKROOM( cmdFifo, pRc->pkt3Size[1]);
          SETPH( cmdFifo,  pRc->header_table[GR_PKT3_TRISTRIP_LOAD_C_DRAW_ABC][pRc->curVertexId] );
          pRc->curVertexId = pRc->nvi_table[1][pRc->curVertexId];
          numVertsToLoad = 1;
      }
      else
      {
          // all done
          numVertsToLoad = 0;
      }

  }

  // following is identical to for loop in main loop - need a macro or inline routine
  for ( ; numVertsToLoad>0 ; numVertsToLoad-- )
  {
      processVertices( numVertsToLoad );
  }

  HW_ACCESS_EXIT(ACCESS_3D);

  CMDFIFO_EPILOG( cmdFifo );

} // Strip - semi-optimized


