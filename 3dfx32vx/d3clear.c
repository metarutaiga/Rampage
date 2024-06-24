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
*/

#include <ddrawi.h>
#include <d3dhal.h>
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "fifomgr.h"

#ifdef SLI
#include <ddsli2d.h>
#endif

#ifdef CSERVICE
#include <shared.h>
#endif

HRESULT clear2D ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth, DWORD dwFillStencil, DWORD dwNumRects );
HRESULT clear3D ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth, DWORD dwFillStencil, DWORD dwNumRects );
HRESULT clearBlockWrite ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth, DWORD dwFillStencil, DWORD dwNumRects );
void    init3D  ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd );

BOOLEAN bNeedClear3DInitialization = TRUE;

//-------------------------------------------------------------------
//
//  Performance tuning thoughts on ddiClear2 :
//
//      There are two fundamental methods currently used to clear the various buffers. One
//      is the 2D BLT method (assisted with Block Write when appropriate). This requires MOPS
//      to spin down the 3D hardware, do the 2D BLT, spind that down, then return. The other
//      method requires drawing one (or two) triangles in 3D land. Two triangles can clear
//      all three buffers with no MOPs required. The 2D BLT clear, especially with Block Write,
//      is faster by far, but with the MOPs it may not be a gain over staying in 3D land.
//      Andy Sobczyk says :
//          From Napalm we have found the 3D FastFills to be faster then 2D block writes by a
//          significant factor.  We believe that this is related to all of the SDRAM/SGRAM page
//          breaks when we are in tiled mode.
//
//  SLI note :
//
//      While SLI Clear2 (SliClear) is a different animal, it may be possible to use the following
//      triangle based Clear2 for SLI. This is because triangle rendering under SLI happens
//      in the hardware (kind of a freebee).
//

DWORD __stdcall ddiClear2 ( LPD3DHAL_CLEAR2DATA pcd )
{
    SETUP_PPDEV(pcd->dwhContext)

    RC        *pRc;
    BOOL      bAUXOkay    = TRUE;
    BOOL      bTargetOkay = TRUE;
  
    D3D_ENTRY( "ddiClear2" );

#if defined( NULLDRIVER )
    pcd->ddrval = DD_OK;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
#endif
  
    pRc = CONTEXT_PTR(pcd->dwhContext);

#ifdef CUBEMAP
    if (pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_CUBEMAP)
    {
       // Assume we're going to clear a cubemap texture. Mark it dirty
       // so we can do a copy/translate when the texture handle is 
       // referenced for rendering. 

       pRc->lpSurfData->pTxtrDesc->txDirtyBits =
            pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES;
    }
#endif

#ifdef SLI
    if (SLI_MODE_ENABLED == _DD(sliMode))
    {
        pcd->ddrval = SliClear( pRc,
                                pcd->dwFlags,
                                pcd->dwFillColor,           // render target fill color
                                pcd->dvFillDepth,           // z buffer fill value
                                pcd->dwFillStencil,         // stencil buffer fill value
                                (RECT *)pcd->lpRects,
                                (DWORD)pcd->dwNumRects
                              );    // number of rects
    }
    else
#endif
    {
        // Make sure that the requested surfaces for clearing are valid ones. "Okay" here can also mean a
        // non-requested surface that is "invalid".
        if ( ((pcd->dwFlags & D3DCLEAR_ZBUFFER) || (pcd->dwFlags & D3DCLEAR_STENCIL)) && (pRc->DDSZHndl == 0) )
            bAUXOkay = FALSE;
        if ( (pcd->dwFlags & D3DCLEAR_TARGET) && (pRc->DDSHndl == 0) )
            bTargetOkay = FALSE;

        if ( bTargetOkay && bAUXOkay )
        {
            // Test if (ZBUFFER XOR STENCIL = TRUE). Note that many apps only set TARGET and ZBUFFER for clear flags. This
            // would tend to send most clears through this "triangle" code path. However, if stencil is not in use, then
            // a BLT clear can be done. BUT, some apps may enable and disable stencil at will, expecting previous stencil
            // contents to remain intact (which a BLT clear would destroy). Therefore, a stencilInUse flag is tested.
            if (((pcd->dwFlags & D3DCLEAR_ZBUFFER) && !(pcd->dwFlags & D3DCLEAR_STENCIL) && pRc->stencilInUse) ||
               (!(pcd->dwFlags & D3DCLEAR_ZBUFFER) &&  (pcd->dwFlags & D3DCLEAR_STENCIL)) )
            {
#ifdef CSERVICE
                D3D_GRABBING_CS_CONTEXT();
#endif
                pcd->ddrval = clear3D ( pRc,
                                        pcd,
                                        pcd->dwFlags,
                                        pcd->dwFillColor,           // render target fill color
                                        pcd->dvFillDepth,           // z buffer fill value
                                        pcd->dwFillStencil,         // stencil buffer fill value
                                        (DWORD)pcd->dwNumRects       // number of rects
                                      );
            } // if z XOR stencil flags
            else  // Both STENCIL and ZBUFFER flags are set :
            {
                pcd->ddrval = clear2D ( pRc,
                                        pcd,
                                        pcd->dwFlags,
                                        pcd->dwFillColor,           // render target fill color
                                        pcd->dvFillDepth,           // z buffer fill value
                                        pcd->dwFillStencil,         // stencil buffer fill value
                                        (DWORD)pcd->dwNumRects       // number of rects
                                      );
            } // zbuffer XOR stencil test
        } // if (lpDDSZ or lpDDS are valid)
    } // if SLI

    D3D_EXIT( DDHAL_DRIVER_HANDLED );
}




//-------------------------------------------------------------------
//
//  CLEAR 3D :  This clear uses a 3D triangle to clear the areas specified in the Clear2 call.
//              This routine is capable of handling all 8 possible Clear2 flag options.
//
HRESULT clear3D ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth,
                  DWORD dwFillStencil, DWORD dwNumRects )
{
    SETUP_PPDEV(pcd->dwhContext)
    int         cnt;
    FxU32       pixelByteDepth;
    long        dstLeft, dstTop, dstRight, dstBottom;

    DWORD       dwClipMinXMaxX, dwClipMinYMaxY;
    D3DCOLOR    clearColor;
    D3DVALUE    aX, aY, bX, bY, cX, cY;
    DWORD       dwTcuColor, dwTcuAlpha, dwCcuColor, dwCcuAlpha;
    DWORD       dwAllZero = 0;
    float       zFill;
    #ifndef CMDFIFO    
      float       oow = 1.0f;     // w not used, but must be sent for direct writes
    #endif

    FxU32       suMode    = 0, suParamMask = 0;
    FxU32       stencil   = 0, stencilOp   = 0;
    FxU32       fbzMode   = 0;
    FxU32       taControl = 0, peAlphaMode = 0;

    CMDFIFO_PROLOG(cmdFifo);
    HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);

    pixelByteDepth = GETPRIMARYBYTEDEPTH;

    // Get the bpp cause we need to set up fbzMode based upon it
    if ( pixelByteDepth == 2 )            // 16 bpp
    {
        fbzMode |= (SST_PE_FB_16BPP << SST_PE_FB_FORMAT_SHIFT);
    }
    else if (pixelByteDepth == 4)         // if 32 bpp
    {
        fbzMode |= (SST_PE_FB_32BPP << SST_PE_FB_FORMAT_SHIFT);
    }
    else                                  // An error exists - non supported color format
    {
        return (D3DERR_UNSUPPORTEDCOLOROPERATION);
    }

    // MOP to flush all current operations and prepare for changing registers. This was attempted with only
    // Flushing the caches, but that generated CSIM errors. The 3D Stall was therefore required.
    CMDFIFO_CHECKROOM( cmdFifo, (MOP_SIZE));
    SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
                    (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));

    // Basic common setup for registers.
    fbzMode       |= SST_PE_EN_DITHER | SST_PE_DEPTH_ZQ_SEL | (SST_PE_LO_COPY<<SST_PE_LOGIC_OP_SHIFT);

    suMode      |= SST_SU_PACKED_ARGB;
    suParamMask |= SST_SU_RGB0 | SST_SU_A0;

    // ----------------------
    // Modify initial register setup based upon passed clear requirements : TARGET, ZBUFFER, STENCIL
    if (dwFlags & D3DCLEAR_ZBUFFER)
    {
        zFill = dvFillDepth;
        suMode |= SST_SU_Z;                                              // Include Z
        fbzMode |= SST_PE_ZFUNC_LT | SST_PE_ZFUNC_EQ | SST_PE_ZFUNC_GT;   // Use Depth Operation Function : ALWAYS
        fbzMode |= SST_PE_EN_DEPTH_BUFFER | SST_PE_DEPTH_WRMASK;          // Enable depth buffering and write to buffer
    }

    // Here we want to set up the VTA and related texture registers to do a simple color triangle
    dwTcuColor = TCC( ZERO, ZERO, ZERO, ZERO);
    dwTcuAlpha = TCA( ZERO, ZERO, ZERO, ZERO);
    dwCcuColor = CCC( ZERO, ZERO, ZERO, CITER);
    dwCcuAlpha = CCA( ZERO, ZERO, ZERO, AITER);

    if (dwFlags & D3DCLEAR_TARGET)
    {   
        fbzMode   |= SST_PE_A_WRMASK | SST_PE_R_WRMASK | SST_PE_G_WRMASK | SST_PE_B_WRMASK;    // Enable color writes
        clearColor = (D3DCOLOR) dwFillColor;
    }

    if (dwFlags & D3DCLEAR_STENCIL)
    {   // Fill stencil with reference value using REPLACE with Reference Value upon stencil test PASS.
        stencil |= (( dwFillStencil<< SST_PE_ST_REF_SHIFT) & SST_PE_ST_REF);        // Reference value is passed clear value
        stencil |= ((0x0FF << SST_PE_ST_MASK_SHIFT) & SST_PE_ST_MASK);              // 
        stencil |= ((0x0FF << SST_PE_ST_WRITE_MASK_SHIFT) & SST_PE_ST_WRITE_MASK);  // Store all 8 reference bits in stecnil buffer
        stencil |= SST_PE_EN_STENCIL;                                               // Enable Stencil
        stencil |= SST_PE_ST_FUNC_ALWAYS;                                           // Always pass (so color pixels are updated)

        stencilOp = SST_PE_ST_FREPLACE | SST_PE_ST_ZFREPLACE | SST_PE_ST_ZPREPLACE; // xREPLACE to put in "ref" value.
    }

    // Initialize the hardware in the event it has not yet been done.
    if (bNeedClear3DInitialization)
    {
        CMDFIFO_SAVE( cmdFifo );
        init3D  ( pRc, pcd );
        CMDFIFO_RELOAD( cmdFifo );
        bNeedClear3DInitialization = FALSE;
    }

    if(IS_SAGE_ACTIVE)
    {
        int size = 2; // XY always
        int i, lastStage;

        if (suMode & SST_SU_W) size++;
        if (suMode & SST_SU_Z) size++;
        if (suMode & SST_SU_Q) size++;

        lastStage = ((taControl & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT);
        for( i=0; i<lastStage+1; i++)
        {
            if (suMode & SST_SU_PACKED_ARGB)
            {
	            if(( suParamMask & (SST_SU_RGB0 << (i * 4) )  )  |
	               ( suParamMask & (SST_SU_A0   << (i * 4) )  )) size+=1;
            }
            else
            {
	            if(  suParamMask & (SST_SU_RGB0 << (i * 4) )   ) size+=3;
	            if(  suParamMask & (SST_SU_A0   << (i * 4) )   ) size++;
            }
	        if( suParamMask & (SST_SU_ST0  << (i * 4) )        ) size+=2;
            if( suParamMask & (SST_SU_Q0   << (i * 4) )        ) size++;
        }

        // We need to tell SAGE what PKT3 size is implied by suParamMask and suMode.
        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1, kPkt3SizeReg, 0, 0));
        SETCF( cmdFifo, size);
    }

    // Output collected values to hardware
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE+2  +   // Stencil output
                                PH1_SIZE+2  +   // Setup Unit values
                                PH1_SIZE+1  +   // fbzMode
                                0);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, peStencil ) );      // Send stencil stuff to hardware
    SETPD( cmdFifo, ghw0->peStencil, stencil );                             //   (even if stencil clear not used, since
    SETPD( cmdFifo, ghw0->peStencilOp, stencilOp );                         //    this will disable stencil otherwise).

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));            // Setup Unit
    SETPD( cmdFifo, ghw0->suMode, (0 << SST_SU_INDEX_SHIFT) | suMode );
    SETPD( cmdFifo, ghw0->suParamMask, suParamMask);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, peFbzMode));         // fbzMode
    SETPD( cmdFifo, ghw0->peFbzMode, fbzMode );

    
    // Shut down VTA TMU0 texture operations.
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_TMU_0, taMode));
    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->taMode,      dwAllZero );

    //-----
    CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4, SST_UNIT_TMU_0, taTcuColor));
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuColor,   dwTcuColor );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuAlpha,   dwTcuAlpha );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuControl, dwAllZero  );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuColor,   dwCcuColor );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuAlpha,   dwCcuAlpha );

        // Turn off Alpha mode
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peAlphaMode ) );
    SETPD( cmdFifo, ghw0->peAlphaMode, peAlphaMode );

    // Shut down all TMUs (except for the first one)
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, taControl ) );
    SETPD( cmdFifo, ghw0->taControl,  taControl );

    // Now that all hardware is set up, proceed into the for loop for each rectangle
    for (cnt = 0; cnt < (int) pcd->dwNumRects; ++cnt)
    {
        // Get the Clear Rectangle edges
        dstLeft   = (DWORD) pcd->lpRects->x1;
        dstTop    = (DWORD) pcd->lpRects->y1;
        dstRight  = (DWORD) pcd->lpRects->x2;
        dstBottom = (DWORD) pcd->lpRects->y2;

        // If this is AA surface it will be double width and height
        if( pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING )
        {
            dstRight  *= 2;
            dstBottom *= 2; 
        }

        // Set Clip Rectangle to Clear Rectangle
        dwClipMinXMaxX = ( ((dstLeft   << SST_SU_CLIP_MIN_SHIFT) & SST_SU_CLIP_MIN) |
                           ((dstRight  << SST_SU_CLIP_MAX_SHIFT) & SST_SU_CLIP_MAX)  );
        dwClipMinYMaxY = ( ((dstTop    << SST_SU_CLIP_MIN_SHIFT) & SST_SU_CLIP_MIN) |
                           ((dstBottom << SST_SU_CLIP_MAX_SHIFT) & SST_SU_CLIP_MAX)  );

        CMDFIFO_CHECKROOM( cmdFifo, 3*(PH1_SIZE + 1));
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipMinXMaxX[0]));
        SETPD( cmdFifo, ghw0->suClipMinXMaxX[0], dwClipMinXMaxX );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipMinYMaxY[0]));
        SETPD( cmdFifo, ghw0->suClipMinYMaxY[0], dwClipMinYMaxY );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipEnables));
        SETPD( cmdFifo, ghw0->suClipEnables, SST_SU_EN_CLIP0 );

        // The Plan :
        // Create LARGE triangle that covers entire clear rect area. While doubling in X and Y will generate
        // such a triangle, to be sure we don't miss a corner pixel, we will go 4X in size. No performance
        // harm, since it is clipped anyway.

        // ----------------------
        // Prepare vertex data :
        aX = (D3DVALUE) dstLeft;
        aY = (D3DVALUE) dstTop;

        bX = (D3DVALUE) ( dstLeft + ((dstRight - dstLeft) << 2) ); // mult by 4
        bY = (D3DVALUE) dstTop;

        cX = aX;
        cY = (D3DVALUE) ( dstTop  + ((dstBottom - dstTop) << 2) ); // mult by 4

        //----------------------
        // Send out the Vertex Data for our triangle :
        CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE+12 );  // Simple Triangle header + vertex data
        SETPH( cmdFifo, CMDFIFO_SIMPLE_TRI );

        //----> Send 1st vertex    
        #ifndef CMDFIFO    
         // This must be first vertex parameter for direct writes
          SETFPD( cmdFifo, ghw0->vpW, oow );
        #endif

        // Pixel Offset macros allow comparison between sw and viewport
        // handling of d3d pixel centers.

        SETFPD( cmdFifo, ghw0->vpX, aX );
        SETFPD( cmdFifo, ghw0->vpY, aY );

        if (suMode & SST_SU_Z)
          SETFPD( cmdFifo, ghw0->vpZ, zFill );

        if( suParamMask & (SST_SU_RGB0 | SST_SU_A0) )   
            SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->vpARGB, clearColor );

        #ifndef CMDFIFO    
          P6FENCE;
          SETPD( cmdFifo, ghw0->suMode, (1 << SST_SU_INDEX_SHIFT) | suMode );
          P6FENCE;
        #endif    

        //----> Send 2nd vertex    
        #ifndef CMDFIFO
          SETFPD( cmdFifo, ghw0->vpW, oow );
        #endif

        SETFPD( cmdFifo, ghw0->vpX, bX );
        SETFPD( cmdFifo, ghw0->vpY, bY );
        if ( suMode & SST_SU_Z)
          SETFPD( cmdFifo, ghw0->vpZ, zFill );

        if( suParamMask & (SST_SU_RGB0 | SST_SU_A0) )   
            SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->vpARGB, clearColor );

        #ifndef CMDFIFO    
          P6FENCE;
          SETPD( cmdFifo, ghw0->suMode, (2 << SST_SU_INDEX_SHIFT) | suMode );
          P6FENCE;
        #endif

        //----> Send 3rd vertex
        #ifndef CMDFIFO    
          // This must be first vertex parameter for direct writes
          SETFPD( cmdFifo, ghw0->vpW, oow );
        #endif

        SETFPD( cmdFifo, ghw0->vpX, cX );
        SETFPD( cmdFifo, ghw0->vpY, cY );
        if (suMode & SST_SU_Z)
          SETFPD( cmdFifo, ghw0->vpZ, zFill );

        if( suParamMask & (SST_SU_RGB0 | SST_SU_A0) )   
            SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->vpARGB, clearColor );

        #ifndef CMDFIFO    
          P6FENCE;
          SETPD( cmdFifo, ghw0->suDrawCmd, SST_SU_DRAWTRI
               | (0<<SST_SU_VERTEX_A_SHIFT)
               | (1<<SST_SU_VERTEX_B_SHIFT)
               | (2<<SST_SU_VERTEX_C_SHIFT));
          P6FENCE;
        #endif

        ++pcd->lpRects;  // inc to next rect to handle.
    } // for (number of rects)

    // MOP to flush all current operations and prepare for changing registers back to original values
    CMDFIFO_CHECKROOM( cmdFifo, (MOP_SIZE));
    SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
                    (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT));
    
    // Restore all changed registers
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_TMU_0, taMode));
    SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->taMode, pRc->sst.TMU[0].taMode.vFxU32 );

    CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4, SST_UNIT_TMU_0, taTcuColor));
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuColor,   pRc->sst.TMU[0].taTcuColor.vFxU32 );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taTcuAlpha,   pRc->sst.TMU[0].taTcuAlpha.vFxU32 );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuControl, pRc->sst.TMU[0].taCcuControl.vFxU32 );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuColor,   pRc->sst.TMU[0].taCcuColor.vFxU32 );
    SETPD( cmdFifo, SST_TREX(ghw,TREX0)->taCcuAlpha,   pRc->sst.TMU[0].taCcuAlpha.vFxU32 );

    if(IS_SAGE_ACTIVE)
    {
        int size = 2; // XY always
        int i, lastStage;

        if (pRc->sst.suMode.vFxU32 & SST_SU_W) size++;
        if (pRc->sst.suMode.vFxU32 & SST_SU_Z) size++;
        if (pRc->sst.suMode.vFxU32 & SST_SU_Q) size++;

        lastStage = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >> SST_TA_NUM_TMUS_SHIFT);
        for( i=0; i<lastStage+1; i++)
        {
            if (pRc->sst.suMode.vFxU32 & SST_SU_PACKED_ARGB)
            {
	            if(( pRc->sst.suParamMask.vFxU32 & (SST_SU_RGB0 << (i * 4) )  )  |
	               ( pRc->sst.suParamMask.vFxU32 & (SST_SU_A0   << (i * 4) )  )) size+=1;
            }
            else
            {
	            if(  pRc->sst.suParamMask.vFxU32 & (SST_SU_RGB0 << (i * 4) )   ) size+=3;
	            if(  pRc->sst.suParamMask.vFxU32 & (SST_SU_A0   << (i * 4) )   ) size++;
            }
	        if( pRc->sst.suParamMask.vFxU32 & (SST_SU_ST0  << (i * 4) )       )  size+=2;
            if( pRc->sst.suParamMask.vFxU32 & (SST_SU_Q0   << (i * 4) )       )  size++;
        }

        // We need to tell SAGE what PKT3 size is implied by suParamMask and suMode.
        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1, kPkt3SizeReg, 0, 0));
        SETCF( cmdFifo, size);
    }

    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE+2  +   // Stencil output
                                PH1_SIZE+2  +   // Setup Unit values
                                PH1_SIZE+1  +   // fbzMode
                                PH1_SIZE+1  +   // Alpha Mode
                                PH1_SIZE+1  +   // taControl
                                0);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, peStencil ) );
    SETPD( cmdFifo, ghw0->peStencil, pRc->sst.peStencil.vFxU32 );
    SETPD( cmdFifo, ghw0->peStencilOp, pRc->sst.peStencilOp.vFxU32 );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
    SETPD( cmdFifo, ghw0->suMode, pRc->sst.suMode.vFxU32 );
    SETPD( cmdFifo, ghw0->suParamMask, pRc->sst.suParamMask.vFxU32);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, peFbzMode));
    SETPD( cmdFifo, ghw0->peFbzMode, pRc->sst.peFbzMode.vFxU32 );
    
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peAlphaMode ) );
    SETPD( cmdFifo, ghw0->peAlphaMode, pRc->sst.peAlphaMode.vFxU32 );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, taControl ) );
    SETPD( cmdFifo, ghw0->taControl,  pRc->sst.taControl.vFxU32 );

    HW_ACCESS_EXIT(ACCESS_3D);
    CMDFIFO_EPILOG( cmdFifo );

    return (D3D_OK);
}


//-------------------------------------------------------------------
//
//  CLEAR 2D :  This clear uses a 2D BLT, with BLOCK WRITE if appropriate, to clear the specified areas.
//              This routine currently is limited to clears that specify BOTH Z and Stencil, or
//              only a target clear.
//
HRESULT clear2D ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth,
                  DWORD dwFillStencil, DWORD dwNumRects )
{
    SETUP_PPDEV(pcd->dwhContext)
    FXSURFACEDATA *pSurfData;
    FXSURFACEDATA *pSurfDataZ;
    FxU32     pixelByteDepth;
    long      dstLeft, dstTop, dstRight, dstBottom;
    DWORD     fillData;
    DWORD     packetHeader = 0, bumpNum=10, clip1min, clip1max;
    DWORD     bltDstBaseAddr, bltDstFormat, bltDstSize, bltRop, bltDstXY;
    DWORD     dstPixelFormat;
    long      dstWidth, dstHeight, dstPitch;
    int       cnt;

    CMDFIFO_PROLOG(cmdFifo);
    HW_ACCESS_ENTRY(cmdFifo, ACCESS_2D);

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY;  // The 2D BLTs will change bltDstBaseAddr

    pixelByteDepth = GETPRIMARYBYTEDEPTH;

    for (cnt = 0; cnt < (int) dwNumRects; ++cnt)
    {
        dstLeft   = pcd->lpRects->x1;
        dstTop    = pcd->lpRects->y1;
        dstRight  = pcd->lpRects->x2;
        dstBottom = pcd->lpRects->y2;

        // If this is AA surface it will be double width and height
        if( pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING )
        {
            dstRight  *= 2;
            dstBottom *= 2;
        }

        dstWidth  = dstRight  - dstLeft;
        dstHeight = dstBottom - dstTop;

        BLTCLIP(dstLeft, dstTop, clip1min);
        BLTCLIP(dstRight, dstBottom, clip1max);
        BLTSIZE(dstWidth, dstHeight, bltDstSize);
        BLTXY(dstLeft, dstTop, bltDstXY);
        bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | (SST_WX_ROP_SRC );

        if ( ((dwFlags & D3DCLEAR_ZBUFFER) | (dwFlags & D3DCLEAR_STENCIL)) && (pRc->DDSZHndl != 0))
        {
            pSurfDataZ = TXTRHNDL_TO_SURFDATA(pRc, pRc->DDSZHndl);
            pRc->lpSurfDataZ = pSurfDataZ;
            fillData =  ((dwFillStencil << 24) & pRc->lpSurfDataZ->dwStencilBitMask) |   // Stencil value
                        (float2int(pcd->dvFillDepth * pRc->lpSurfDataZ->dwZBitMask));         // Z value

            bltDstBaseAddr = pSurfDataZ->hwPtr;
            pixelByteDepth = pSurfDataZ->dwBytesPerPixel;
            GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);

            // For render to texture make sure color buffer
            // and zbuffer size and pitch are the same. 

            if (pRc->lpSurfData->ddsDwCaps & DDSCAPS_TEXTURE )
            {
              BLTFMT_STRIDESTATE(pRc->lpSurfData, dstPitch);
            }
            else
            {
              BLTFMT_STRIDESTATE(pRc->lpSurfDataZ, dstPitch);
            }

            // combine pitch and pixel format into 1 blit format dword
            BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);

            CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9);

            packetHeader |=  dstBaseAddrBit
                         |   dstFormatBit
                         |   ropBit
                         |   clip1minBit
                         |   clip1maxBit
                         |   colorForeBit
                         |   dstSizeBit
                         |   dstXYBit
                         |   commandBit;

            SETPH(cmdFifo, CMDFIFO_BUILD_PK2( packetHeader ) );
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          bltRop );

            SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
            SETPD(cmdFifo, ghw2D->colorFore,    fillData );
            SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
            SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                     | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);
        }    

        if( dwFlags & D3DCLEAR_TARGET && (pRc->DDSHndl != 0))
        {
            DWORD   dwBLTColor;

            // need to access frame buffer address from FXSURFACEDATA for dx7 - mls
            pSurfData = pRc->lpSurfData;
            bltDstBaseAddr = pSurfData->hwPtr;

            pixelByteDepth = GETPRIMARYBYTEDEPTH;  // reload this in case z buffer depth was different
            if (pixelByteDepth == 2)            // if 16 bpp
            {
                dwBLTColor = dwFillColor;  // Assume fill color is rgb888 always
                dwBLTColor = (((dwBLTColor & 0xF80000) >> 8) | ((dwBLTColor & 0xFC00) >> 5) | ((dwBLTColor & 0xF8) >> 3));
            }
            else if (pixelByteDepth == 4)       // if 32 bpp
            {
                dwBLTColor = pcd->dwFillColor;
            }
            else                                // An error exists - non supported color format
            {
                return (D3DERR_UNSUPPORTEDCOLOROPERATION);
            }
            GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);

            BLTFMT_STRIDESTATE(pRc->lpSurfData, dstPitch);

            // combine pitch and pixel format into 1 blit format dword
            BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);

            CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9);

            packetHeader |= dstBaseAddrBit
                         |  dstFormatBit
                         |  ropBit
                         |  clip1minBit
                         |  clip1maxBit
                         |  colorForeBit
                         |  dstSizeBit
                         |  dstXYBit
                         |  commandBit;

            SETPH(cmdFifo, CMDFIFO_BUILD_PK2( packetHeader ) );
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          bltRop );

            SETPD(cmdFifo, ghw2D->clip1min,  clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,  clip1max);
            SETPD(cmdFifo, ghw2D->colorFore, (FxU32) dwBLTColor);
            SETPD(cmdFifo, ghw2D->dstSize,   bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,     bltDstXY);
            SETPD(cmdFifo, ghw2D->command,   SST_WX_RECTFILL | SST_WX_GO
                      | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);
        }  // if target (color) buffer
        ++pcd->lpRects;
    } // for (number of rects)
      
    HW_ACCESS_EXIT(ACCESS_2D);
    CMDFIFO_EPILOG( cmdFifo );
    return (D3D_OK);
}

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
//-------------------------------------------------------------------
//
//  CLEAR BLOCK WRITE:  This clear uses a 2D BLT, with BLOCK WRITE if appropriate, to clear the specified areas.
//     This routine currently is limited to clears that specify BOTH Z and Stencil, or
//     only a target clear.
//

HRESULT clear2D_BlockWrite ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth,
                  DWORD dwFillStencil, DWORD dwNumRects )
{
    SETUP_PPDEV(pcd->dwhContext)
    FXSURFACEDATA *pSurfData;
    FXSURFACEDATA *pSurfDataZ;
    FxU32     pixelByteDepth;
    long      dstLeft, dstTop, dstRight, dstBottom;
    DWORD     fillData;
    DWORD     packetHeader = 0, bumpNum=10, clip1min, clip1max;
    DWORD     bltDstBaseAddr, bltDstFormat, bltDstSize, bltRop, bltDstXY;
    DWORD     dstPixelFormat;
    long      dstWidth, dstHeight, dstPitch;
    int       cnt;

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
    BOOL      surfaceOkForBlockWrite = FALSE; // added for block writes, will be set TRUE if surface is OK for block write
    DWORD     bltCommandExtra = 0;            // added for block writes
#endif        
    DWORD     blockWriteBump  = 0;            // will bump fifo checkroom count by 1 if needed for block write

    CMDFIFO_PROLOG(cmdFifo);
    HW_ACCESS_ENTRY(cmdFifo, ACCESS_2D);

    _FF(gdiFlags) |= SDATA_GDIFLAGS_2D_DIRTY;  // The 2D BLTs will change bltDstBaseAddr

    pixelByteDepth = GETPRIMARYBYTEDEPTH;

    for (cnt = 0; cnt < (int) dwNumRects; ++cnt)
    {
        dstLeft   = pcd->lpRects->x1;
        dstTop    = pcd->lpRects->y1;
        dstRight  = pcd->lpRects->x2;
        dstBottom = pcd->lpRects->y2;

        // If this is AA surface it will be double width and height
        if( pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_HINTANTIALIASING )
        {
            dstRight  *= 2;
            dstBottom *= 2;
        }

        dstWidth  = dstRight  - dstLeft;
        dstHeight = dstBottom - dstTop;

        BLTCLIP(dstLeft, dstTop, clip1min);
        BLTCLIP(dstRight, dstBottom, clip1max);
        BLTSIZE(dstWidth, dstHeight, bltDstSize);
        BLTXY(dstLeft, dstTop, bltDstXY);
        bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | (SST_WX_ROP_SRC );

        if ( ((dwFlags & D3DCLEAR_ZBUFFER) | (dwFlags & D3DCLEAR_STENCIL)) && (pRc->DDSZHndl != 0))
        {
            pSurfDataZ = TXTRHNDL_TO_SURFDATA(pRc, pRc->DDSZHndl);
            pRc->lpSurfDataZ = pSurfDataZ;
            fillData =  ((dwFillStencil << 24) & pRc->lpSurfDataZ->dwStencilBitMask) |   // Stencil value
                        (float2int(pcd->dvFillDepth * pRc->lpSurfDataZ->dwZBitMask));         // Z value

            // access z buffer address from FXSURFACEDATA  - mls
            pSurfDataZ = pRc->lpSurfDataZ;
            bltDstBaseAddr = pSurfDataZ->hwPtr;
            pixelByteDepth = pSurfDataZ->dwBytesPerPixel;
            GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
            // I am assuming these are always going to be in local memory - not in AGP or this could fail
            // I know these work in 16 and 32 bit depths
            // 8 bit modes may be a problem on some - need to test each one.
            switch( bltDstSize )
            {
                case 0x1E00280:   // 640x480
                case 0x1E002D0:   // 720x480
                case 0x24002D0:   // 720x576
                case 0x3000400:   // 1024x768
                case 0x3600480:   // 1152x480
                case 0x4000500:   // 1280x1024
                    surfaceOkForBlockWrite = TRUE;
                    break;
                default:
                    surfaceOkForBlockWrite = FALSE;    
            }

            if( surfaceOkForBlockWrite )
            {
                // Check if surface starts on 256 BYTE BOUNDRY
                // Check if width is even multiple of 256 
                if ( (bltDstBaseAddr & 0xff) || ( (dstWidth*pixelByteDepth) & 0xff) ) 
                {
                    surfaceOkForBlockWrite = FALSE;
                }
                if( surfaceOkForBlockWrite )
                {
                    // Set BLOCK WRITE ENABLE BIT in COMMAND EXTRA REGISTER
                    bltCommandExtra |= SST_WX_EN_BLOCK_WRITE;
                    // add 1 to fifo count if we use block write
                    blockWriteBump = 1;
                }
            }

            if( surfaceOkForBlockWrite)
            {
                // force the memory format flag to linear instead of tiled
                dstPitch = 0;
                dstPitch |= SST_WX_DST_LINEAR;        // force to linear
                dstPitch |= dstWidth*pixelByteDepth;  // calculate correct pitch
            }
            else
            {
                // do blt without block write
                BLTFMT_STRIDESTATE(pRc->lpSurfDataZ, dstPitch);
            }
#else
            BLTFMT_STRIDESTATE(pRc->lpSurfDataZ, dstPitch);
#endif  
            // combine pitch and pixel format into 1 blit format dword
            BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);

            CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 + blockWriteBump );

            packetHeader |=  dstBaseAddrBit
                         |   dstFormatBit
                         |   ropBit
                         |   clip1minBit
                         |   clip1maxBit
                         |   colorForeBit
                         |   dstSizeBit
                         |   dstXYBit
                         |   commandBit;

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
            if( surfaceOkForBlockWrite )
            {
                // write to hw with block write on
                packetHeader |=  commandExBit;
            }
#endif
            SETPH(cmdFifo, CMDFIFO_BUILD_PK2( packetHeader ) );
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          bltRop );

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
            // send value for command extra register
            if( packetHeader & commandExBit ) 
            {
                SETPD(cmdFifo, ghw2D->commandEx,      bltCommandExtra);
            }
#endif
            SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
            SETPD(cmdFifo, ghw2D->colorFore,    fillData );
            SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
            SETPD(cmdFifo, ghw2D->command,      SST_WX_RECTFILL | SST_WX_GO
                     | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);
        }    

        if( dwFlags & D3DCLEAR_TARGET && (pRc->DDSHndl != 0))
        {
            DWORD   dwBLTColor;

            // need to access frame buffer address from FXSURFACEDATA for dx7 - mls
            pSurfData = pRc->lpSurfData;
            bltDstBaseAddr = pSurfData->hwPtr;

            pixelByteDepth = GETPRIMARYBYTEDEPTH;  // reload this in case z buffer depth was different
            if (pixelByteDepth == 2)            // if 16 bpp
            {
                dwBLTColor = dwFillColor;  // Assume fill color is rgb888 always
            dwBLTColor = (((dwBLTColor & 0xF80000) >> 8) | ((dwBLTColor & 0xFC00) >> 5) | ((dwBLTColor & 0xF8) >> 3));
            }
            else if (pixelByteDepth == 4)       // if 32 bpp
            {
                dwBLTColor = pcd->dwFillColor;
            }
            else                                // An error exists - non supported color format
            {
                return (D3DERR_UNSUPPORTEDCOLOROPERATION);
            }
            GETPIXELFORMAT(pixelByteDepth, dstPixelFormat);

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
            // this will only work for local video memory
            switch( bltDstSize )
            {
                case 0x1E00280:   // 640x480
                case 0x1E002D0:   // 720x480
                case 0x24002D0:   // 720x576
                case 0x3000400:   // 1024x768
                case 0x3600480:   // 1152x480
                case 0x4000500:   // 1280x1024
                    surfaceOkForBlockWrite = TRUE;
                    break;
                default:
                    surfaceOkForBlockWrite = FALSE;    
            }

            if( surfaceOkForBlockWrite )
            {
                // Check if surface starts on 256 BYTE boundry
                // Check if width is even multiple of 256 
                if ( (bltDstBaseAddr & 0xff) || ( (dstWidth*pixelByteDepth) & 0xff) ) 
                {
                    surfaceOkForBlockWrite = FALSE;
                }
                if( surfaceOkForBlockWrite )
                {
                    // Set BLOCK WRITE ENABLE BIT in COMMAND EXTRA REGISTER
                    bltCommandExtra |= SST_WX_EN_BLOCK_WRITE;
                    // add 1 to fifo count if we use block write
                    blockWriteBump = 1;
                }
            }

            if( surfaceOkForBlockWrite)
            {
                // force the memory format flag to linear instead of tiled
                dstPitch  = 0;
                dstPitch |= SST_WX_DST_LINEAR;        // force to linear
                dstPitch |= dstWidth*pixelByteDepth;  // calculate correct pitch
            }
            else
            {
                // do blt without block write
                BLTFMT_STRIDESTATE(pRc->lpSurfData, dstPitch);
            }
#else
            BLTFMT_STRIDESTATE(pRc->lpSurfData, dstPitch);
#endif  
            // combine pitch and pixel format into 1 blit format dword
            BLTFMT_DST(dstPitch, dstPixelFormat, bltDstFormat);

            CMDFIFO_CHECKROOM( cmdFifo, PH2_SIZE + 9 + blockWriteBump );  // (blockWriteBump initialized to zero)

            packetHeader |= dstBaseAddrBit
                         |  dstFormatBit
                         |  ropBit
                         |  clip1minBit
                         |  clip1maxBit
                         |  colorForeBit
                         |  dstSizeBit
                         |  dstXYBit
                         |  commandBit;

#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
            if( surfaceOkForBlockWrite )
            {
                // write to hw with block write on
                packetHeader |=  commandExBit;
            }
#endif
            SETPH(cmdFifo, CMDFIFO_BUILD_PK2( packetHeader ) );
            SETPD(cmdFifo, ghw2D->dstBaseAddr,  bltDstBaseAddr);
            SETPD(cmdFifo, ghw2D->dstFormat,    bltDstFormat);
            SETPD(cmdFifo, ghw2D->rop,          bltRop );
#ifdef USE_BLOCK_WRITE_BUFFER_CLEAR
            if( packetHeader & commandExBit ) 
            {
                SETPD(cmdFifo, ghw2D->commandEx,      bltCommandExtra);
            }
#endif
            SETPD(cmdFifo, ghw2D->clip1min,  clip1min);
            SETPD(cmdFifo, ghw2D->clip1max,  clip1max);
            SETPD(cmdFifo, ghw2D->colorFore, (FxU32) dwBLTColor);
            SETPD(cmdFifo, ghw2D->dstSize,   bltDstSize);
            SETPD(cmdFifo, ghw2D->dstXY,     bltDstXY);
            SETPD(cmdFifo, ghw2D->command,   SST_WX_RECTFILL | SST_WX_GO
                      | (SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) | SST_WX_CLIPSELECT);
        }  // if target (color) buffer
        ++pcd->lpRects;
    } // for (number of rects)
      
    HW_ACCESS_EXIT(ACCESS_2D);
    CMDFIFO_EPILOG( cmdFifo );
    return (D3D_OK);
}
#endif  // ~BLOCK_WRITE_BUFFER_CLEAR


//-------------------------------------------------------------------
//
//  INIT  3D :  This routine sets up the hardware and rendering state to draw the 3D triangle
//              specified in clear3D. This is provided because there are cases where Clear2
//              is called before setDX6State is ever called (like via Verdict). Note that the
//              folowing is a subset of setDX6State and actually was taken from there.
//
void    init3D  ( RC* pRc, LPD3DHAL_CLEAR2DATA pcd )
{
    SETUP_PPDEV(pcd->dwhContext)

    FXSURFACEDATA*  surfaceData;
    FXSURFACEDATA*  surfaceDataZ;

    CMDFIFO_PROLOG(cmdFifo);
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 7);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(7, SST_UNIT_FBI, vpMode));
    SETPD( cmdFifo, ghw0->vpMode, SST_VP_EN_STQ_PROJ
                                // | SST_VP_EN_XY_PROJ
                                // | SST_VP_EN_Z_PROJ
                                // | SST_VP_WINDOW_SCALE
                                // | SST_VP_EN_Q_PROJ
                                // | SST_VP_EN_W_RECIP 
                                );

    // With XY Projection disabled set viewport size
    // to pass screen coordinates thru without scaling.

    // XY offset is set to 0.5 for d3d pixel centers.
    SETFPD( cmdFifo, ghw0->vpSizeX,   1.0f );
    SETFPD( cmdFifo, ghw0->vpCenterX, pRc->pixelOffset );
    SETFPD( cmdFifo, ghw0->vpSizeY,   1.0f );
    SETFPD( cmdFifo, ghw0->vpCenterY, pRc->pixelOffset );
    SETFPD( cmdFifo, ghw0->vpSizeZ,   1.0f );
    SETFPD( cmdFifo, ghw0->vpCenterZ, 0.0f );

    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 2 + PH4_SIZE + 2);      

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, raControl ) );      
    SETPD( cmdFifo, ghw0->raControl, pRc->sst.raControl.vFxU32 );
    SETPD( cmdFifo, ghw0->raStipple, pRc->sst.raStipple.vFxU32 );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2, SST_UNIT_FBI, peColBufferAddr));
    surfaceData = (FXSURFACEDATA*)pRc->lpSurfData;

    SETPD( cmdFifo, ghw0->peColBufferAddr, surfaceData->hwPtr );
    SETPD( cmdFifo, ghw0->peBufferSize, SET_PE_BUFFER_SIZE_TILED( 
           surfaceData->dwStride,
           surfaceData->dwHeight ));
   
    if (pRc->DDSZHndl && pRc->zEnable)
    {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 2);

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, peAuxBufferAddr));
        surfaceDataZ = (FXSURFACEDATA*)pRc->lpSurfDataZ;

        SETPD( cmdFifo, ghw0->peAuxBufferAddr, surfaceDataZ->hwPtr );
        SETPD( cmdFifo, ghw0->peBufferSize, SET_PE_BUFFER_SIZE_TILED( 
               surfaceDataZ->dwStride, surfaceDataZ->dwHeight ));
    }

    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 2 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, vpSTscale0));
    SETPD( cmdFifo, ghw0->vpSTscale0, pRc->sst.vpSTscale0.vFxU32);
    SETPD( cmdFifo, ghw0->vpSTscale1, pRc->sst.vpSTscale1.vFxU32);

    CMDFIFO_CHECKROOM( cmdFifo, 
                           PH1_SIZE+3 +    // fog
                           PH1_SIZE+1 +    // SDConst
                           PH1_SIZE+1 +    // line width
                           0);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 3, SST_UNIT_FBI, peFogMode ) );

    SETPD( cmdFifo, ghw0->peFogMode, pRc->sst.peFogMode.vFxU32 );
    SETPD( cmdFifo, ghw0->peFbzMode, pRc->sst.peFbzMode.vFxU32 );
    SETPD( cmdFifo, ghw0->peAlphaTest, pRc->sst.peAlphaTest.vFxU32 );

    // If Z buffer is enabled then we need to send the current
    // ZBIAS value. - AS
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peSDConst ) );
    SETPD( cmdFifo, ghw0->peSDConst, pRc->sst.peSDConst.vFxU32 );

    // send line state - width is constant, but we may have swapped w/ a non D3D 3D context (OpenGL??)
    //  so reload it - line pattern is a D3D state member
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, suLineWidth ) );
    SETPD( cmdFifo, ghw0->suLineWidth, DEFAULT_LINEWIDTH );
   
    CMDFIFO_EPILOG( cmdFifo );
}
