/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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

// "ge write state" :
// All code (if possible) that sets registers for Sage should be in here, especially
// code that uses packet 7 in response to the SetGEState() routine.

// includes

#include <ddrawi.h>
#include <gevlib.h>
#include <gevlu.h>
#include <d3global.h>
#include <d3contxt.h>
#include <fifomgr.h>
#include <d3dvc.uch>
#include <tc2.uch>

// local-only prototypes

static void geWriteVertexSizeRegs (RC *pRc, vRegs *sageVRegs);
static void geWriteVertexBufferRegs( RC *pRc, vbRegs *sageVBRegs );

static void geComputeJumpTablesAndSRam (RC *pRc);
static void geWriteJumpTablesAndSRam (RC *pRc);
static void geComputeJumpTablesAndSRam (RC *pRc);
static void geWriteJumpTablesAndSRam (RC *pRc);
static void vpeSetupLights(RC *pRc,int *numEntries);
static void vpeSRAMAddMaterial(RC *pRc, D3DMATERIAL7 *mtl,int offset);
static void vpeSRAMAddMatrix(RC *pRc, D3DMATRIX *mat,int offset);
static void vpeSRAMAddSceneAmbient(RC *pRc, D3DVALUE ar, D3DVALUE ag, D3DVALUE ab,int offset);
static void vpeSRAMAddUserClipPlanes(RC *pRc,GEVECTOR4 *uClipPlanes,int offset); 
static void vpeSRAMAddLight(RC *pRc,D3DLIGHT7 *light,int offset);

static void geWritePPEViewportRegs(RC *pRc);
static void geWriteHackRegs(RC *pRc);
static void geWriteSuRegs(RC *pRc, gvluState *ss);

//-----------------------------------------------------------------------------------
//
// SetGEState()
// 
//-----------------------------------------------------------------------------------

void SetGEState(RC * pRc, DWORD dwhContext)
{
    gvluState ges;                          // sage intermediate state struct

    SETUP_PPDEV( pRc );

    memset(&ges, '\0', sizeof(gvluState));

    geWriteHackRegs(pRc);

    // 1st: if the fvf changed we need to reprogram the vertex size for Sage
    if ( HW_STATE_CHANGED & SC_FVF ) {
        vRegs       sageVRegs;              // sage vertex size config registers
        vParamCfe   cfeList[40];            // TEMP: enough to hold the largest vertex
        vParamVpe   vpeList[40];            // TEMP: enough to hold the largest vertex
        int         cfeParams, vpeParams;

        // set up cfe and vpe vertex lists from the fvf offset table and system state
        gvluQueryGeState( pRc, &ges );
        gvluSetupCfeVList( pRc->fvfVertexType, fvfOffsetTable, &ges, &cfeParams, cfeList );
        gvluSetupVpeVList( cfeParams, cfeList, &ges, &vpeParams, vpeList );

        // call sage vertex library to compute sage register values
        memset(&sageVRegs, '\0', sizeof(sageVRegs));
        gvlSetupVertexFormat( cfeList, cfeParams, vpeList, vpeParams, &sageVRegs );

        // dump the vertex configuration registers to the hardware
        geWriteVertexSizeRegs( pRc, &sageVRegs );
    }

    // 2nd: if the matrix or light state changed, re-compute all of the
    // jump tables, and VPE cached state.
    if (HW_STATE_CHANGED & (SC_GE_MATRIX | SC_GE_LIGHTING)) {
        if (HW_STATE_CHANGED & SC_GE_MATRIX) {
            DWORD i;
            // Recompute all matrices
            // Adjust the projection matrix to work with the PPE.
            MatrixProduct( &pRc->GERC.matProj,
                           &matProjMod,
                           &pRc->GERC.VPERC.matViewClipMod);

            for (i=0;i <= pRc->GERC.dwNumVertexBlends;i++) {
                // Generate the Model->view matrix.
                MatrixProduct( &pRc->GERC.matWorld[i],
                               &pRc->GERC.matView,
                               &pRc->GERC.VPERC.matModelView[i]);
                // Generate the inverse of the transposed model->view matrix.
                Inverse4x4Transpose( &pRc->GERC.VPERC.matModelView[i],
                                     &pRc->GERC.VPERC.matModelViewInv[i]);
            }
        }
        geComputeJumpTablesAndSRam(pRc);
        geWriteJumpTablesAndSRam(pRc);
    }


    // 3rd: check if we went from tl vertices to non-tl vertices or vice versa?
    if ( pRc->bVtxGeStateToggle != FVF_NO_CHANGE )
    {
        if ( pRc->bVtxGeStateToggle == FVF_CHANGE_TO_TL_VERT )
        {
            // Setup the rampage viewport values  (XY offset is set to 0.5 for d3d pixel centers.) :
            pRc->sst.vpMode.vFxU32    &= ~( SST_VP_EN_W_RECIP | SST_VP_EN_XY_PROJ );
            pRc->sst.vpSizeX.vFloat    = 1.0f;
            pRc->sst.vpCenterX.vFloat  = pRc->pixelOffset;
            pRc->sst.vpSizeY.vFloat    = 1.0f;
            pRc->sst.vpCenterY.vFloat  = pRc->pixelOffset;
            pRc->sst.vpSizeZ.vFloat    = 1.0f;
            pRc->sst.vpCenterZ.vFloat  = 0.0f;
            UPDATE_HW_STATE(SC_VIEWPORT);
        }
        else //pRc->bVtxGeStateToggle == FVF_CHANGE_TO_NON_TL_VERT
        {
            float screenWidth, screenHeight;
            float screenWidthD2, screenHeightD2;

            screenWidth  = pRc->clipRect.MaxX - pRc->clipRect.MinX;
            screenHeight = pRc->clipRect.MaxY - pRc->clipRect.MinY;

            screenWidthD2  = screenWidth / 2.0f;
            screenHeightD2 = screenHeight / 2.0f;

            // Setup the sage ppe viewport values
            //(guard band max x - (screen width / 2))/(screen width / 2)
            pRc->GERC.gePpeRegs.ppeGBXScaleReg = ( GUARDBAND_RIGHT - screenWidthD2 ) / screenWidthD2;
            //(guard band max y - (screen height / 2))/(screen height / 2)
            pRc->GERC.gePpeRegs.ppeGBYScaleReg = ( GUARDBAND_BOTTOM - screenHeightD2 ) / screenHeightD2;
            //PpeVSXReg = screen width / 2
            pRc->GERC.gePpeRegs.ppeVSXReg      = screenWidthD2;
            //PpeVSYReg = screen height / 2
            pRc->GERC.gePpeRegs.ppeVSYReg      = screenHeightD2;

            //Point Y Offset x = 0, y = 1 / screen height, z = 0, w = 0
            pRc->GERC.gePpeRegs.ppeFPState[0x20] = 0.0f;
            pRc->GERC.gePpeRegs.ppeFPState[0x21] = 1.0f / screenHeight;
            pRc->GERC.gePpeRegs.ppeFPState[0x22] = 0.0f;
            pRc->GERC.gePpeRegs.ppeFPState[0x23] = 0.0f;

            //ViewPort Scale x = (screen width / 2), y = (screen height / 2), z = 1, w = 1
            pRc->GERC.gePpeRegs.ppeFPState[0x24] = screenWidthD2;
            pRc->GERC.gePpeRegs.ppeFPState[0x25] = screenHeightD2;
            pRc->GERC.gePpeRegs.ppeFPState[0x26] = 1.0f;
            pRc->GERC.gePpeRegs.ppeFPState[0x27] = 1.0f;

            //PpeVCXReg = (screen width / 2) + d3d pixel center
            screenWidthD2 += (float)pRc->pixelOffset;
            pRc->GERC.gePpeRegs.ppeVCXReg     = ( ( screenWidthD2 > 0.0f ) ? 0x00000000 : 0x00010000 ) |
                                                ( ( float2int(screenWidthD2) & 0x00000fff ) << 4 ) |
                                                ( float2int( screenWidthD2 * 16.0f ) & 0x0000000f );
            //PpeVCYReg = (screen height / 2) + d3d pixel center
            screenHeightD2 += (float)pRc->pixelOffset;
            pRc->GERC.gePpeRegs.ppeVCYReg     = ( ( screenHeightD2 > 0.0f ) ? 0x00000000 : 0x00010000 ) |
                                                ( ( float2int(screenHeightD2) & 0x00000fff ) << 4 ) |
                                                ( float2int( screenHeightD2 * 16.0f ) & 0x0000000f );

            //ViewPort Offset x = (screen width / 2) + d3d pixel center, y = (screen height / 2) + d3d pixel center, z = 0, w = 0
            pRc->GERC.gePpeRegs.ppeFPState[0x28] = screenWidthD2;
            pRc->GERC.gePpeRegs.ppeFPState[0x29] = screenHeightD2;
            pRc->GERC.gePpeRegs.ppeFPState[0x2a] = 0.0f;
            pRc->GERC.gePpeRegs.ppeFPState[0x2b] = 0.0f;

            geWritePPEViewportRegs(pRc);

            // Setup the rampage viewport values  (XY offset is set to 0.5 for d3d pixel centers.) :
            pRc->sst.vpMode.vFxU32    |= SST_VP_EN_W_RECIP | SST_VP_EN_XY_PROJ;

            // vpSizeX is not really the width of the screen, but the size to scale to.
            // Because we have input values from -1 to 1, we scale by half the width, and then
            // add half the width back to it.  That gets us to 0 to width.
            pRc->sst.vpSizeX.vFloat    = screenWidthD2;
            pRc->sst.vpCenterX.vFloat  = screenWidthD2 + .5f;
            // vpSizeY is not really the width of the screen, but the size to scale to.
            // Because we have input values from -1 to 1, we scale by half the width, and then
            // add half the width back to it.  That gets us to 0 to width.
            // The other thing we do here is cause a mirroring about the Y axis.
            // This is due to the fact that in clip space positive Y goes up, and in
            // screen space positive Y goes down.
            pRc->sst.vpSizeY.vFloat    = -screenHeightD2;
            pRc->sst.vpCenterY.vFloat  = screenHeightD2 + .5f;
            // Setup Z to scale from -1 to 1; to 0 to 1;
            pRc->sst.vpSizeZ.vFloat    = 0.5f;  // This results in a -.5 to .5 range.
            pRc->sst.vpCenterZ.vFloat  = 0.5f;  // We add .5 to shift into the 0 to 1 range.
            UPDATE_HW_STATE(SC_VIEWPORT);
        }
    }

    // 4th: did the vertex buffer change?
    if ( pRc->GERC.bUseVertexBuffers && (HW_STATE_CHANGED & SC_GE_VERTEX_BUFFER) ) {
        bParam      vbRec;
        int         count;
        vbRegs      sageVBRegs;

        // set up structures that describe vertex buffers for Sage
        gvluSetupVB( pRc->GERC.surfDataVB, fvfOffsetTable, pRc->fvfVertexType, &vbRec, &count );

        // call library to program sage register for vertex buffers
        gvlSetupVertexStreams( &vbRec, count, &sageVBRegs );

        // dump the vertex buffer configuration register to the hardware
        geWriteVertexBufferRegs( pRc, &sageVBRegs );
    }

    // dump su registers
    geWriteSuRegs( pRc, &ges );
}

//-----------------------------------------------------------------------------------
//
// geWriteVertexSizeRegs()
// 
//-----------------------------------------------------------------------------------

static void geWriteVertexSizeRegs (RC *pRc, vRegs *sageVRegs)
{
    int i, j, numCfeVfDescDw, numCfeVcDescDw, numCbeVfDescDw;

    SETUP_PPDEV(pRc)
    CMDFIFO_PROLOG(cmdFifo);

    // COPY REGISTER VALUES TO REGISTER SHADOWS

    pRc->GERC.geCfeRegs.cfePkt6Size               = sageVRegs->cfePkt6Size;
    pRc->GERC.geCfeRegs.cfeVcStart                = sageVRegs->cfeVcStart;
    pRc->GERC.geCfeRegs.cfeVfStart                = sageVRegs->cfeVfStart;
    pRc->GERC.geVpeRegs.vpeVertexSizeReg          = sageVRegs->vpeVertexSize;
    pRc->GERC.gePpeRegs.ppeMBControlReg           = sageVRegs->ppeMBControl;
    pRc->GERC.gePpeRegs.ppeMeshLoadTrapOffsetsReg = sageVRegs->ppeMeshLoadTrapOffset;
    pRc->GERC.gePpeRegs.ppeMeshLoadMaskReg        = sageVRegs->ppeMeshLoadMask;
    pRc->GERC.gePpeRegs.ppeVertexSizeReg          = sageVRegs->ppeVertexSize;
    pRc->GERC.geCbeRegs.cbePkt6Size               = sageVRegs->cbePkt6Size;
    pRc->GERC.geCbeRegs.cbeVfdStart               = sageVRegs->cbeVfdStart;

    for (i=0; i<8; i++)
        pRc->GERC.geCfeRegs.cfeVcDescriptor[i] = sageVRegs->cfeVcSram[i];
    for (i=0; i<sageVRegs->numCfeVfDesc; i++)
        pRc->GERC.geCfeRegs.cfeVfDescriptor[i] = sageVRegs->cfeVfSram[i];
    for (i=0; i<8; i++)
        pRc->GERC.gePpeRegs.ppeCPOffsetRemap[i] = sageVRegs->ppeCPOffsetRemap[i];
    for (i=0; i<32; i++)
        pRc->GERC.gePpeRegs.ppeXferDescriptorReg[i] = sageVRegs->ppeXferDesc[i];    
    for (i=0; i<16; i++)
        pRc->GERC.gePpeRegs.ppeIntState[i] = sageVRegs->ppeIntState[i];
    for (i=0; i<sageVRegs->numCbeVfDesc; i++)
        pRc->GERC.geCbeRegs.cbeVfDescriptor[i] = sageVRegs->cbeVfSram[i];

    // TEMP: what precautions do I need to take before resetting the vertex size?

    // write cfe registers

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE*2+3 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCfePkt6Size, 0, 0 ));
    SETCF( cmdFifo, sageVRegs->cfePkt6Size );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 2, kCfeVcStart, 1, 0 ));
    SETCF( cmdFifo, sageVRegs->cfeVcStart );
    SETCF( cmdFifo, sageVRegs->cfeVfStart );

    // the number of vc and vf descriptors should be equal
    numCfeVcDescDw = (sageVRegs->numCfeVfDesc+15)>>4;
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+numCfeVcDescDw );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( numCfeVcDescDw, kCfeVcAddr, 1, 0 ));
    for (i=0; i<numCfeVcDescDw; i++)
        SETCF( cmdFifo, sageVRegs->cfeVcSram[i] );

    numCfeVfDescDw = (sageVRegs->numCfeVfDesc+1)/2;
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+numCfeVfDescDw );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( numCfeVfDescDw, kCfeVfAddr, 1, 0 ));
    for (i=0, j=0; i<numCfeVfDescDw; i++, j+=2)
        SETCF( cmdFifo, (sageVRegs->cfeVfSram[j+1] << 16) | sageVRegs->cfeVfSram[j] );

    // write vpe registers

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE*3+5 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kVpeVertexSizeReg, 0, 0 ));
    SETCF( cmdFifo, sageVRegs->vpeVertexSize );

    // write ppe registers

    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 3, kPpeMeshBufControl, 1, 0 ));
    SETCF( cmdFifo, sageVRegs->ppeMBControl );
    SETCF( cmdFifo, sageVRegs->ppeMeshLoadTrapOffset );
    SETCF( cmdFifo, sageVRegs->ppeMeshLoadMask );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kPpeVertexSizeReg, 0, 0 ));
    SETCF( cmdFifo, sageVRegs->ppeVertexSize );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+8 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 8, kPpeCpOffsetRemap0, 1, 0 ));
    for (i=0; i<8; i++)
        SETCF( cmdFifo, sageVRegs->ppeCPOffsetRemap[i] );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+8 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 8, kPpeXferDescReg0, 1, 0 ));
    for (i=0; i<32; i+=4)
        SETCF( cmdFifo, sageVRegs->ppeXferDesc[i+3] << 24 |
                        sageVRegs->ppeXferDesc[i+2] << 16 |
                        sageVRegs->ppeXferDesc[i+1] <<  8 |
                        sageVRegs->ppeXferDesc[i]);

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+16 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 16, kPpeIntStateAddr, 1, 0));
    for (i=0; i<16; i++)
        SETCF( cmdFifo, sageVRegs->ppeIntState[i]);

    // write cbe registers

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE*2+2 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCbePkt6Size, 0, 0 ));
    SETCF( cmdFifo, sageVRegs->cbePkt6Size );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCbeVfdStartReg, 0, 0 ));
    SETCF( cmdFifo, sageVRegs->cbeVfdStart );

    numCbeVfDescDw = (sageVRegs->numCbeVfDesc+1)/2;
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+numCbeVfDescDw );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( numCbeVfDescDw, kCbeVfDescriptor, 1, 0 ));
    for (i=0, j=0; i<numCbeVfDescDw; i++, j+=2)
        SETCF( cmdFifo, (sageVRegs->cbeVfSram[j+1] << 16) | sageVRegs->cbeVfSram[j] );

    CMDFIFO_EPILOG( cmdFifo );
}

//-----------------------------------------------------------------------------------
//
// geWriteVertexBufferRegs()
// 
//-----------------------------------------------------------------------------------

static void geWriteVertexBufferRegs( RC *pRc, vbRegs *sageVBRegs )
{
    int i;

    SETUP_PPDEV(pRc)
    CMDFIFO_PROLOG(cmdFifo);

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1+PH7_SIZE+(sageVBRegs->numCfeVbDesc*4) );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCfeVbStart, 0, 0 ));
    SETCF( cmdFifo, sageVBRegs->cfeVbStart );

    // fill the vb sram with vertex buffer descriptors
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( sageVBRegs->numCfeVbDesc*4, kCfeVbAddr, 1, 0 ));
    for (i=0; i<sageVBRegs->numCfeVbDesc; i++) {
        SETCF( cmdFifo, sageVBRegs->cfeVbDescriptor[i].vbdStride );
        SETCF( cmdFifo, sageVBRegs->cfeVbDescriptor[i].vbdBase );
        SETCF( cmdFifo, sageVBRegs->cfeVbDescriptor[i].vbdMax );
        SETCF( cmdFifo, sageVBRegs->cfeVbDescriptor[i].vbdLength );
    }

    CMDFIFO_EPILOG( cmdFifo );
}

//-----------------------------------------------------------------------------------
//
// geWritePPEViewportRegs()
// 
//-----------------------------------------------------------------------------------

static void geWritePPEViewportRegs(RC *pRc)
{
    SETUP_PPDEV(pRc)
    DWORD i;
    CMDFIFO_PROLOG(cmdFifo);

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+6 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 6, kPpeXGuardBandMult, 1, 0 ));
    SETFCF( cmdFifo, pRc->GERC.gePpeRegs.ppeGBXScaleReg );
    SETFCF( cmdFifo, pRc->GERC.gePpeRegs.ppeGBYScaleReg );
    SETFCF( cmdFifo, pRc->GERC.gePpeRegs.ppeVSXReg );
    SETFCF( cmdFifo, pRc->GERC.gePpeRegs.ppeVSYReg );
    SETCF(  cmdFifo, pRc->GERC.gePpeRegs.ppeVCXReg );
    SETCF(  cmdFifo, pRc->GERC.gePpeRegs.ppeVCYReg );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+12 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 12, kPpeFpeStateAddr+0x20, 1, 0 ));
    for(i=0x20; i<0x2c; i++)
        SETFCF( cmdFifo, pRc->GERC.gePpeRegs.ppeFPState[i]);

    CMDFIFO_EPILOG( cmdFifo );
}

static void geWriteHackRegs(RC *pRc)
{
    FxU32 i, d;
    FxU32 mbsize = 8;
    SETUP_PPDEV(pRc)
    CMDFIFO_PROLOG(cmdFifo);

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kGeMiscReg, 1, 0 ));
    //GeMiscReg
    SETCF( cmdFifo, (1 << kCbeVertexLenShift) );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCfeControl, 1, 0 ));
    //CfeControl
    SETCF( cmdFifo, (1 << kCfeVertexLenShift) );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCfeSanitizeMask, 1, 0 ));
    //CfeSanitizeMask
    SETCF( cmdFifo, 0xffffffff );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1, kCfeMeshSize, 1, 0 ));
    //CfeMeshSize
    SETCF( cmdFifo, mbsize );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+2 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 2, kPpeControlReg, 1, 0 ));
    //PpeControlReg
    SETCF( cmdFifo, (1 << kPpeControlVSEnableShift) |
                    (1 << kPpeControlPICEnableShift) );
    //PpeRECacheControlReg
    SETCF( cmdFifo, 0x00000000 );


    // Set the TA TR registers in the PPE.
    // For now we do NO! clipping.
    CMDFIFO_CHECKROOM( cmdFifo, (PH7_SIZE+1)*5);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1,kPpePointTRMaskReg,0,0));
    SETCF( cmdFifo, 0);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1,kPpeLineTRMaskReg,0,0));
    SETCF( cmdFifo, 0);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1,kPpeLineTAMaskReg,0,0));
    SETCF( cmdFifo, 0);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1,kPpeTriTRMaskReg,0,0));
    SETCF( cmdFifo, 0);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 1,kPpeTriTAMaskReg,0,0));
    SETCF( cmdFifo, 0);

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+3);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( 3,kPpeJumpTableAddr,1,0));
    // Points
    d = ((xferpoint>>2) << kPpeJumpTableDrawShift) 
        | (0xff << kPpeJumpTableDiscardShift) 
        | (0xff << kPpeJumpTableClipShift);
    SETCF( cmdFifo, d);
    // Lines
    d = (0xfe << kPpeJumpTableDrawShift) 
        | (0xff << kPpeJumpTableDiscardShift) 
        | ((ep_clipline>>2) << kPpeJumpTableClipShift);
    SETCF( cmdFifo, d);
    // Triangles
    d = (0xfe << kPpeJumpTableDrawShift) 
        | (0xff << kPpeJumpTableDiscardShift) 
        | ((ep_cliptri>>2) << kPpeJumpTableClipShift);
    SETCF( cmdFifo, d);



    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+kPpeVsTrans1Cnt );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( kPpeVsTrans1Cnt, kPpeVsTrans1Addr, 1, 0 ));
    //Ppe Vertex Shuffle Translate Table 1
    d = 0x03020100;
    for(i=0; i<kPpeVsTrans1Cnt; i++,d+=0x04040404)
        SETCF( cmdFifo, d );

    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+kPpeVsTrans2Cnt );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w( kPpeVsTrans2Cnt, kPpeVsTrans2Addr, 1, 0 ));
    //Ppe Vertex Shuffle Translate Table 2
    SETCF( cmdFifo, ((mbsize) << 16) | ((mbsize + 1) << 8) | (mbsize + 2) );
    SETCF( cmdFifo, 0x00000000 );
    SETCF( cmdFifo, 0x00000000 );
    SETCF( cmdFifo, 0x00000000 );

    CMDFIFO_EPILOG( cmdFifo );
}

//-----------------------------------------------------------------------------------
//
// geComputeJumpTablesAndSRam (RC *pRc)
//    This routine recomputes all of the Cached State and Jump Tables for the VPE
//    based on the current D3D state.
// 
//-----------------------------------------------------------------------------------

static void geComputeJumpTablesAndSRam (RC *pRc)
{
    int numMatrices = pRc->GERC.dwNumVertexBlends + 1;
    int numEntries;
    int curArgEntry;
    WORD materialOffset;
    GE_CONTEXT *GERC = &pRc->GERC;
    VPE_CONTEXT *VPERC = &pRc->GERC.VPERC;

    // reset all of the state data.
    numEntries = 0;

    // Allocate things in state that we might need for multiple routines.
    {
        materialOffset = MTL_OFFSET;
                               
        vpeSRAMAddMaterial(pRc,&GERC->Material,MTL_OFFSET);
    }
    

    // Setup the jump table first.
    // Setup the init routine's arguments first.
    {
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Initialize;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
        VPERC->jmpTable.jmpEntry[numEntries].arg2 = 0;
        VPERC->jmpTable.jmpEntry[numEntries].arg3 = 0;
        VPERC->jmpTable.jmpEntry[numEntries].arg4 = 0;
        VPERC->jmpTable.jmpEntry[numEntries].arg5 = 0;
        VPERC->jmpTable.jmpEntry[numEntries].arg6 = 0;
        VPERC->jmpTable.jmpEntry[numEntries].arg7 = 0;
        numEntries++;
    }
    // Start with group B
    switch (numMatrices) {
        case 1:
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Transform;
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            curArgEntry = numEntries + 1;
            // This is the state ram location of the matrices to be used. 
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = MTX_BLND1_WV;
            // Add the matrices to the cached ram.
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[0],MTX_BLND1_WV);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[0],MTX_BLND1_IWV);
            vpeSRAMAddMatrix(pRc,&VPERC->matViewClipMod,MTX_BLND1_VC);
            // This is the vertex source for position.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.wpos; 

            // This is where in the vertex to source for model space normal from.
            if (GERC->geVertexInfo.normal != PARAM_NOT_PRESENT)
                VPERC->jmpTable.jmpEntry[curArgEntry].arg4 = GERC->geVertexInfo.normal; 
            else
                VPERC->jmpTable.jmpEntry[curArgEntry].arg4 = 0; 

            // This is the state ram location of the Material block.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = MTL_OFFSET;

            numEntries++;
            break;
        case 2:
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Geometry_Blend_2;
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            curArgEntry = numEntries + 1;
            // This is the state ram location of the matrices to be used. 
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = MTX_BLND2_WV0;
            // Add the matrices to the cached ram.
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[0],MTX_BLND2_WV0);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[1],MTX_BLND2_WV1);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[0],MTX_BLND2_IWV0);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[1],MTX_BLND2_IWV1);
            vpeSRAMAddMatrix(pRc,&VPERC->matViewClipMod,MTX_BLND2_VC);

            // This is the vertex source for position.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.wpos; 
            // This is an obsolete parameter for flat shading that still needs to be filled in.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg4 = GERC->geVertexInfo.wpos; 

            // This is where in the vertex to source for model space normal from.
            if (GERC->geVertexInfo.normal != PARAM_NOT_PRESENT)
                VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = GERC->geVertexInfo.normal; 
            else
                VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = 0; 

            // This is the vertex source for the blending weights
            VPERC->jmpTable.jmpEntry[curArgEntry].arg6 = GERC->geVertexInfo.blend;

            // This is the state ram location of the Material block.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg7 = materialOffset;

            numEntries++;
            break;
        case 3:
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Geometry_Blend_3;
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            curArgEntry = numEntries + 1;
            // This is the state ram location of the matrices to be used. 
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = MTX_BLND3_WV0;
            // Add the matrices to the cached ram.
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[0],MTX_BLND3_WV0);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[1],MTX_BLND3_WV1);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[2],MTX_BLND3_WV2);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[0],MTX_BLND3_IWV0);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[1],MTX_BLND3_IWV1);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[2],MTX_BLND3_IWV2);
            vpeSRAMAddMatrix(pRc,&VPERC->matViewClipMod,MTX_BLND3_VC);

            // This is the vertex source for position.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.wpos; 
            // This is an obsolete parameter for flat shading that still needs to be filled in.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg4 = GERC->geVertexInfo.wpos; 

            if (GERC->geVertexInfo.normal != PARAM_NOT_PRESENT)
                VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = GERC->geVertexInfo.normal; 
            else
                VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = 0; 

            // This is the vertex source for the blending weights
            VPERC->jmpTable.jmpEntry[curArgEntry].arg6 = GERC->geVertexInfo.blend;

            // This is the state ram location of the Material block.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg7 = materialOffset;

            numEntries++;
            break;
        case 4:
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Geometry_Blend_4;
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            curArgEntry = numEntries + 1;
            // This is the state ram location of the matrices to be used. 
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = MTX_BLND4_WV0;
            // Add the matrices to the cached ram.
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[0],MTX_BLND4_WV0);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[1],MTX_BLND4_WV1);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[2],MTX_BLND4_WV2);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelView[3],MTX_BLND4_WV3);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[0],MTX_BLND4_IWV0);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[1],MTX_BLND4_IWV1);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[2],MTX_BLND4_IWV2);
            vpeSRAMAddMatrix(pRc,&VPERC->matModelViewInv[3],MTX_BLND4_IWV3);
            vpeSRAMAddMatrix(pRc,&VPERC->matViewClipMod,MTX_BLND3_VC);

            // This is the vertex source for position.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.wpos; 
            // This is an obsolete parameter for flat shading that still needs to be filled in.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg4 = GERC->geVertexInfo.wpos; 

            // This is where in the vertex to source for model space normal from.
            if (GERC->geVertexInfo.normal != PARAM_NOT_PRESENT)
                VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = GERC->geVertexInfo.normal; 
            else
                VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = 0; 

            // This is the vertex source for the blending weights
            VPERC->jmpTable.jmpEntry[curArgEntry].arg6 = GERC->geVertexInfo.blend;

            // This is the state ram location of the Material block.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg7 = materialOffset;

            numEntries++;
            break;
        default:    // This is an error we need to deal with.
            break;

    }
    // Group C: optional (Normal treatment)

    if (GERC->geVertexInfo.normal == PARAM_NOT_PRESENT)
    {
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_No_Normals;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries+1;
        numEntries++;
    }
    if (GERC->bNormalizeNormals) {
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Normalize_Normals;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
        curArgEntry = numEntries + 1;
        VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = materialOffset + MTL_SPC_OFFSET;
        numEntries++;
    }

    // Group D: optional (User clip planes)
    if (GERC->dwClipPlanEnable != 0)
    {
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries+1;
        curArgEntry = numEntries + 1;
        VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = USR_CLIP_OFFSET;
        vpeSRAMAddUserClipPlanes(pRc,&GERC->userClipPlanes[0],USR_CLIP_OFFSET);
        VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.vPos;
        VPERC->jmpTable.jmpEntry[curArgEntry].arg4 = GERC->geVertexInfo.uClip03;

        if (GERC->dwClipPlanEnable & (D3DCLIPPLANE4 | D3DCLIPPLANE5 |
                                      (D3DCLIPPLANE5 << 1) | (D3DCLIPPLANE5 << 2) ))
        {
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_8_User_Clip_Planes;
            VPERC->jmpTable.jmpEntry[curArgEntry].arg5 = GERC->geVertexInfo.uClip47;
        }
        else
        {
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_4_User_Clip_Planes;
        }
        numEntries++;
    }

    // Group E: optional (Ambient Color source form Vertex)
    if (GERC->ambMaterialSrc != D3DMCS_MATERIAL) {
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Ambient_From_Vertex;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
        curArgEntry = numEntries + 1;
        if (GERC->ambMaterialSrc == D3DMCS_COLOR1) { 
            // Where in the ambient term from the vertex.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.diffuse_in;
        }
        else { 
            // Where in the ambient term from the vertex.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.specular_in; 
        }
        numEntries++;
    }

    // Group F: optional (Diffuse Color source form Vertex)
    if (GERC->dfusMaterialSrc != D3DMCS_MATERIAL) {
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Diffuse_From_Vertex;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
        curArgEntry = numEntries + 1;
        if (GERC->dfusMaterialSrc == D3DMCS_COLOR1) { 
            // Where in the ambient term from the vertex.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.diffuse_in;
        }
        else { 
            // Where in the ambient term from the vertex.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.specular_in;
        }
        numEntries++;
    }

    // Group G: optional (Specular Color source Vertex/Material)
    if (GERC->specMaterialSrc != D3DMCS_MATERIAL) { 
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_From_Vertex;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
        curArgEntry = numEntries + 1;
        if (GERC->specMaterialSrc == D3DMCS_COLOR1) { 
            // Where in the ambient term from the vertex.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.diffuse_in; 
        }
        else { 
            // Where in the ambient term from the vertex.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.specular_in; 
        }
    }
    else {
        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_From_Material;
        VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
        curArgEntry = numEntries + 1;
        VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = materialOffset + MTL_SPC_OFFSET;
        numEntries++;
    }

    // Group H: required. (Global ambient emissive routine)
    // Group I: optional (Specular local viewer precompute) This is combined for convienience.
    {
        if (GERC->emisMaterialSrc == D3DMCS_MATERIAL) {
            if (GERC->bLocalViewer) {
                VPERC->jmpTable.jmpEntry[numEntries].fncAddr = 
                                        D3D_Global_Ambient_Emissive_From_Material_Specular_Precompute;
            }
            else {
                VPERC->jmpTable.jmpEntry[numEntries].fncAddr = 
                                        D3D_Global_Ambient_Emissive_From_Material;
            }
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            curArgEntry = numEntries + 1;
            // Where in the material to look for the emissive color.
            VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = materialOffset + MTL_EMS_OFFSET; 
        }
        else {
            if (GERC->bLocalViewer) {
                VPERC->jmpTable.jmpEntry[numEntries].fncAddr = 
                                            D3D_Global_Ambient_Emissive_From_Vertex_Specular_Precompute;
            }
            else {
                VPERC->jmpTable.jmpEntry[numEntries].fncAddr = 
                                            D3D_Global_Ambient_Emissive_From_Vertex;
            }
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            curArgEntry = numEntries + 1;
            if (GERC->emisMaterialSrc == D3DMCS_COLOR1) {
                // Where in the material to look for the emissive color.
                VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.diffuse_in; 
            }
            else { 
                // Where in the material to look for the emissive color.
                VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.specular_in; 
            }
        }

        // This is the global ambient color scene light.
        VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = SCN_AMB_OFFSET;
        vpeSRAMAddSceneAmbient(pRc, 
                               GERC->ambient_red, 
                               GERC->ambient_green, 
                               GERC->ambient_blue,
                               SCN_AMB_OFFSET);

        numEntries++;
    }

    // Group J: optional (Lights)
    {
        vpeSetupLights(pRc,&numEntries);
    }

    // Group K: optional (Texgen)
    {
        // Not currently used.
    }

    // Group L: optional (Texture transform)
    {
        // Not currently used.
    }

    // Group M: optional (Range Fog precompute)
    {
        // Not currently used.
    }

    // Group N: optional (Fog)
    {
        // Not currently used.
    }

    // Group O: optional (Alpha source)
    {
        // Not currently used.
    }

    // Group P: optional (Fog factor source)
    {
        // Not currently used.
    }

    // Group Q: required.  (Color output routine)
    {
        // As this will be the last entry in the jump table, we should use the 
        // arguments in the very first table entry.
        curArgEntry = 1;
        if (GERC->geVertexInfo.specular_out == PARAM_NOT_PRESENT) {
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Output_Primary_Color;
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.diffuse_out;
        }
        else {
            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Output_Primary_Color;
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = GERC->geVertexInfo.diffuse_out;
            VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = GERC->geVertexInfo.specular_out;
        }
        // Now loop, but don't execute the initialization routine again.
        VPERC->jmpTable.jmpEntry[numEntries].next = 1;
        numEntries++;
    }

    // Group R: optional (Negate normals)
    {
        // Not currently used.  Primarily used for 2 sided lighting
        // which is not supported in D3D.
    }
    VPERC->jmpTable.numEntries = numEntries;
    // Mark the first jump entry to advance to the next vertex.
    // To do this we set the jump address advance bit (11) of the address.
    VPERC->jmpTable.jmpEntry[1].fncAddr |= 0x800;
}


//-----------------------------------------------------------------------------------
//
// vpeSetupLights(RC *pRc,int *numEntries,int materialOffset)
//    Setup all of the active lights in the jump tables.
// 
//-----------------------------------------------------------------------------------

static void vpeSetupLights (RC *pRc, int *_numEntries)
{
// This macro actually does a sqrt() call which we don't have access to in the driver.
#ifdef D3DLIGHT_RANGE_MAX
#undef D3DLIGHT_RANGE_MAX 
#define D3DLIGHT_RANGE_MAX 1.84467e+019
#endif
    int i;
    int curArgEntry;
    int numEntries = *_numEntries;   
    GE_CONTEXT *GERC = &pRc->GERC;
    VPE_CONTEXT *VPERC = &GERC->VPERC;

    for (i=0; i < GE_MAX_ACTIVE_LIGHTS;i++)
    {
        if (GERC->geLightArray[i].bLightEnable)
        {
            int lightOffset = LIGHT_OFFSET + (LIGHT_SIZE * i);
            curArgEntry = numEntries + 1;
            VPERC->jmpTable.jmpEntry[numEntries].next = numEntries + 1;
            VPERC->jmpTable.jmpEntry[curArgEntry].arg2 = lightOffset;
            if (pRc->state & STATE_REQUIRES_SPECULAR) 
            {   // Specular is enabled.
                // The specular power in the material.
                VPERC->jmpTable.jmpEntry[curArgEntry].arg3 = MTL_POW_OFFSET;
                switch(GERC->geLightArray[i].LightData.dltType) 
                {
                    case D3DLIGHT_POINT: 
                        if (GERC->geLightArray[i].LightData.dvAttenuation0 == 1.f &&
                            GERC->geLightArray[i].LightData.dvRange == D3DLIGHT_RANGE_MAX)
                        {   // use the optimized routine. 
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_Point_Light;
                        }
                        else
                        {
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_Point_Light_Quadratic_Attenuation;
                        }
                        break;

                    case D3DLIGHT_SPOT:
                        if (GERC->geLightArray[i].LightData.dvAttenuation0 == 1.f &&
                            GERC->geLightArray[i].LightData.dvRange == D3DLIGHT_RANGE_MAX)
                        {   // use the optimized routine.  
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_Spotlight;
                        }
                        else
                        {
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_Spotlight_Quadratic_Attenuation;
                        }
                        break;

                    case D3DLIGHT_DIRECTIONAL:
                        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Specular_Directional_Light;
                        break;

                    default:
                        break;
                }
            }
            else
            {   // No specular.
                switch(GERC->geLightArray[i].LightData.dltType) {
                    case D3DLIGHT_POINT: 
                        if (GERC->geLightArray[i].LightData.dvAttenuation0 == 1.0f &&
                            GERC->geLightArray[i].LightData.dvRange == D3DLIGHT_RANGE_MAX)
                        {   // use the optimized routine.  
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Diffuse_Point_Light;
                        }
                        else
                        {
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Diffuse_Point_Light_Quadratic_Attenuation;
                        }
                        break;

                    case D3DLIGHT_SPOT:
                        if (GERC->geLightArray[i].LightData.dvAttenuation0 == 1.0f &&
                            GERC->geLightArray[i].LightData.dvRange == D3DLIGHT_RANGE_MAX)
                        {   // use the optimized routine.  
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Diffuse_Spotlight;
                        }
                        else
                        {
                            VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Diffuse_Spotlight_Quadratic_Attenuation;
                        }
                        break;

                    case D3DLIGHT_DIRECTIONAL:
                        VPERC->jmpTable.jmpEntry[numEntries].fncAddr = D3D_Diffuse_Directional_Light;
                        break;

                    default:
                        break;
                }
            }
            vpeSRAMAddLight(pRc,&GERC->geLightArray[i].LightData,lightOffset);
            numEntries++;
        }
    }
    // Move the count back into the original
    *_numEntries = numEntries;
}

static void vpeSRAMAddUserClipPlanes(RC *pRc,GEVECTOR4 *uClipPlanes,int offset)
{
    int i;
    int bankNum = offset >> 7;
    GEVECTOR4 *vptr;
    offset = offset & 0x7f;
    vptr = (GEVECTOR4 *)&pRc->GERC.VPERC.vpeSRAM[bankNum].bank[offset].data[0];

    for (i=0;i < GE_MAX_USER_CLIPPLANES;i++)
    {
        vptr[i] = uClipPlanes[i];
    }
}

static void vpeSRAMAddLight(RC *pRc,D3DLIGHT7 *light,int offset)
{
    int bankNum = offset >> 7;
    float *fptr;
    float p1,p2,chTheta,chPhi;
    D3DMATRIX   itV;
    GEVECTOR4  tpos,tdir;
    GEVECTOR4  vpos,vdir;
    offset = offset & 0x7f;
    fptr = (float*)&pRc->GERC.VPERC.vpeSRAM[bankNum].bank[offset].data[0]; 

    // Because lights are defined in world space, and used in view space,
    // we need to xform them into view space before we give them to SAGE.
    // We also need to change the "handedness" of the position and direction.
    tpos.x = light->dvPosition.x;
    tpos.y = light->dvPosition.y;
    tpos.z = light->dvPosition.z;
    tpos.w = 1.f;

    tdir.x = -light->dvDirection.x;
    tdir.y = -light->dvDirection.y;
    tdir.z = -light->dvDirection.z;
    tdir.w = 0.f;

    Inverse4x4Transpose(&pRc->GERC.matView,&itV);
    VectorXForm(&pRc->GERC.matView,&tpos,&vpos);
    VectorXForm(&itV,&tdir,&vdir);

    // Position
    if (light->dltType == D3DLIGHT_DIRECTIONAL) {
        *fptr++ = vdir.x;
        *fptr++ = vdir.y;
        *fptr++ = vdir.z;
        *fptr++ = vdir.w;
    }
    else {
        *fptr++ = vpos.x;
        *fptr++ = vpos.y;
        *fptr++ = vpos.z;
        *fptr++ = vpos.w;
    }

    // Ambient color
    *fptr++ = light->dcvAmbient.b;
    *fptr++ = light->dcvAmbient.g;
    *fptr++ = light->dcvAmbient.r;
    *fptr++ = light->dcvAmbient.a;

    // Diffuse color
    *fptr++ = light->dcvDiffuse.b;
    *fptr++ = light->dcvDiffuse.g;
    *fptr++ = light->dcvDiffuse.r;
    *fptr++ = light->dcvDiffuse.a;

    // Specular color
    *fptr++ = light->dcvSpecular.b;
    *fptr++ = light->dcvSpecular.g;
    *fptr++ = light->dcvSpecular.r;
    *fptr++ = light->dcvSpecular.a;

    if (0) { // If we are using a directional non-local viewer light.
        // Halfway vector (directional lights, non-local viewer only)
        // This is the normalized vector between the eye and light position.
        // We need to calculate this if the light is directional and
        // we have a non-local viewer.
        *fptr++ = 0.f;  
        *fptr++ = 0.f;
        *fptr++ = 0.f;
        *fptr++ = 0.f;
    }
    else { // We are potentially using a light with attenuation.
        // Constant, Linear, Quadratic atenuation coefficient, Range
        *fptr++ = light->dvAttenuation0;
        *fptr++ = light->dvAttenuation1;
        *fptr++ = light->dvAttenuation2;
        *fptr++ = light->dvRange;
    }

    // Spotlight Direction
    *fptr++ = vdir.x;
    *fptr++ = vdir.y;
    *fptr++ = vdir.z;
    *fptr++ = 0;

    // Spotlight parameter 1, parameter 2, -unused-, falloff
    if (light->dltType == D3DLIGHT_SPOT)
    {
        chTheta = fxCos(light->dvTheta / 2.f);
        chPhi   = fxCos(light->dvPhi / 2.f);
        p2 = chTheta - chPhi;
        p1 = 1.f / p2;
        p2 = -(chPhi) / p2;
        *fptr++ = p1; 
        *fptr++ = p2;
        *fptr++ = 0.f;
        *fptr++ = light->dvFalloff;
    }
    else
    {
        *fptr++ = 0.f;
        *fptr++ = 0.f;
        *fptr++ = 0.f;
        *fptr++ = 0.f;
    }
}

static void vpeSRAMAddSceneAmbient(RC *pRc, D3DVALUE ar, D3DVALUE ag, D3DVALUE ab,int offset)
{
    int bankNum = offset >> 7;
    QDWORD *sram;
    offset &= 0x7f;
    sram = &pRc->GERC.VPERC.vpeSRAM[bankNum].bank[offset]; 
    (float)sram->data[0] = ar;
    (float)sram->data[1] = ag;
    (float)sram->data[2] = ab;
}

static void vpeSRAMAddMaterial(RC *pRc, D3DMATERIAL7 *mtl,int offset)
{
    int bankNum = offset >> 7;
    QDWORD *sram;
    offset &= 0x7f;
    sram = &pRc->GERC.VPERC.vpeSRAM[bankNum].bank[offset];  
    
    (float)sram->data[0] = mtl->ambient.b;
    (float)sram->data[1] = mtl->ambient.g;
    (float)sram->data[2] = mtl->ambient.r;
    (float)sram->data[3] = mtl->ambient.a;

    sram++;
    (float)sram->data[0] = mtl->diffuse.b;
    (float)sram->data[1] = mtl->diffuse.g;
    (float)sram->data[2] = mtl->diffuse.r;
    (float)sram->data[3] = mtl->diffuse.a;

    sram++;
    (float)sram->data[0] = mtl->specular.b;
    (float)sram->data[1] = mtl->specular.g;
    (float)sram->data[2] = mtl->specular.r;
    (float)sram->data[3] = mtl->specular.a;

    sram++;
    (float)sram->data[0] = mtl->emissive.b;
    (float)sram->data[1] = mtl->emissive.g;
    (float)sram->data[2] = mtl->emissive.r;
    (float)sram->data[3] = mtl->emissive.a;

    sram++;
    (float)sram->data[0] = mtl->power;
}


static void vpeSRAMAddMatrix(RC *pRc, D3DMATRIX *mat,int offset)
{
    int bankNum = offset >> 7;
    QDWORD *sram;
    offset &= 0x7f;
    sram = &pRc->GERC.VPERC.vpeSRAM[bankNum].bank[offset];  
    // Copy the full 4x4 matrix and rotate it.
    (float)sram[0].data[0] = mat->_11;
    (float)sram[0].data[1] = mat->_21;
    (float)sram[0].data[2] = mat->_31;
    (float)sram[0].data[3] = mat->_41;

    (float)sram[1].data[0] = mat->_12;
    (float)sram[1].data[1] = mat->_22;
    (float)sram[1].data[2] = mat->_32;
    (float)sram[1].data[3] = mat->_42;

    (float)sram[2].data[0] = mat->_13;
    (float)sram[2].data[1] = mat->_23;
    (float)sram[2].data[2] = mat->_33;
    (float)sram[2].data[3] = mat->_43;

    (float)sram[3].data[0] = mat->_14;
    (float)sram[3].data[1] = mat->_24;
    (float)sram[3].data[2] = mat->_34;
    (float)sram[3].data[3] = mat->_44;
   
}

//-----------------------------------------------------------------------------------
//
// geWriteVertexSizeRegs()
// 
//-----------------------------------------------------------------------------------

static void geWriteJumpTablesAndSRam (RC *pRc)
{
    int dwordCount,i;
    FxU32 *outData;
    VPE_CONTEXT *VPERC = &pRc->GERC.VPERC;
    SETUP_PPDEV(pRc)
    CMDFIFO_PROLOG(cmdFifo);


    // Write Light state.
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+(LIGHT_SIZE_TOTAL*4));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w((LIGHT_SIZE_TOTAL*4),kVpeStateSramAddr+(LIGHT_OFFSET*4),1,0));
    outData = (FxU32*)&VPERC->vpeSRAM[0].bank[LIGHT_OFFSET];
    for (i=0;i < (LIGHT_SIZE_TOTAL*4);i++)
    {
        SETCF( cmdFifo,*outData++);
    }
    // Write Matrices
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+(MTX_SIZE_TOTAL*4));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w((MTX_SIZE_TOTAL*4),kVpeStateSramAddr+(MTX_OFFSET*4),1,0));
    outData = (FxU32*)&VPERC->vpeSRAM[0].bank[MTX_OFFSET];
    for (i=0;i < (MTX_SIZE_TOTAL*4);i++)
    {
        SETCF( cmdFifo,*outData++);
    }
    // Write Clip planes
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+(USR_CLIP_SIZE_TOTAL*4));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w((USR_CLIP_SIZE_TOTAL*4),kVpeStateSramAddr+(USR_CLIP_OFFSET*4),1,0));
    outData = (FxU32*)&VPERC->vpeSRAM[0].bank[USR_CLIP_OFFSET];
    for (i=0;i < (USR_CLIP_SIZE_TOTAL*4);i++)
    {
        SETCF( cmdFifo,*outData++);
    }
    // Write Material
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+(MTL_SIZE_TOTAL*4));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w((MTL_SIZE_TOTAL*4),kVpeStateSramAddr+(MTL_OFFSET*4),1,0));
    outData = (FxU32*)&VPERC->vpeSRAM[0].bank[MTL_OFFSET];
    for (i=0;i < (MTL_SIZE_TOTAL*4);i++)
    {
        SETCF( cmdFifo,*outData++);
    }
    // Write Fog data
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+(FOG_SIZE_TOTAL*4));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w((FOG_SIZE_TOTAL*4),kVpeStateSramAddr+(FOG_OFFSET*4),1,0));
    outData = (FxU32*)&VPERC->vpeSRAM[0].bank[FOG_OFFSET];
    for (i=0;i < (FOG_SIZE_TOTAL*4);i++)
    {
        SETCF( cmdFifo,*outData++);
    }
    // Write Scene Ambient
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+(SCN_AMB_SIZE_TOTAL*4));
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w((SCN_AMB_SIZE_TOTAL*4),kVpeStateSramAddr+(SCN_AMB_OFFSET*4),1,0));
    outData = (FxU32*)&VPERC->vpeSRAM[0].bank[SCN_AMB_OFFSET];
    for (i=0;i < (SCN_AMB_SIZE_TOTAL*4);i++)
    {
        SETCF( cmdFifo,*outData++);
    }



    // Download the Global State SRAM
    outData = (FxU32 *)&VPERC->jmpTable.jmpEntry[0];
    dwordCount = VPERC->jmpTable.numEntries * sizeof(VPE_JMPTABLE_ENTRY) / 4;
    CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+dwordCount);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(dwordCount,kVpeStateGlobalSramAddr,1,0));
    for (i=0;i < dwordCount;i++) {
        SETCF( cmdFifo, *outData++);
    }


    // Download the start addr for the uCode.
    CMDFIFO_CHECKROOM( cmdFifo,PH7_SIZE+1);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1,kVpeMainLoopAddrReg,0,0));
    // Note the HACK for the FSIM.  By adding 0x8000 we tell the FSIM to use
    // the generic DX7 D3D uCode.  The CSIM and the Hardware will both ignore
    // the MSBs of the word.
    SETCF( cmdFifo, D3D_Initialize + 0x8000);

    CMDFIFO_EPILOG(cmdFifo);
}

//-----------------------------------------------------------------------------------
//
// geWriteSuRegs()
// 
//-----------------------------------------------------------------------------------

static void geWriteSuRegs (RC *pRc, gvluState *ss)
{
    FxU32 su_mode = pRc->sst.suMode.vFxU32;
    FxU32 su_param_mask = pRc->sst.suParamMask.vFxU32;

    SETUP_PPDEV(pRc)
    CMDFIFO_PROLOG(cmdFifo);
    HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);  // TEMP: is this necessary?

    // sage always wants to send w and z
    su_mode |= SST_SU_W | SST_SU_Z;

    switch ( ss->num_tex_coords ) {
        case 8: su_param_mask |= SST_SU_ST7;
        case 7: su_param_mask |= SST_SU_ST6;
        case 6: su_param_mask |= SST_SU_ST5;
        case 5: su_param_mask |= SST_SU_ST4;
        case 4: su_param_mask |= SST_SU_ST3;
        case 3: su_param_mask |= SST_SU_ST2;
        case 2: su_param_mask |= SST_SU_ST1;
        case 1: su_param_mask |= SST_SU_ST0;
    }

    switch ( ss->num_light_colors ) {
        case 8: su_param_mask |= SST_SU_RGB7 | SST_SU_A7;
        case 7: su_param_mask |= SST_SU_RGB6 | SST_SU_A6;
        case 6: su_param_mask |= SST_SU_RGB5 | SST_SU_A5;
        case 5: su_param_mask |= SST_SU_RGB4 | SST_SU_A4;
        case 4: su_param_mask |= SST_SU_RGB3 | SST_SU_A3;
        case 3: su_param_mask |= SST_SU_RGB2 | SST_SU_A2;
        case 2: su_param_mask |= SST_SU_RGB1 | SST_SU_A1;
        case 1: su_param_mask |= SST_SU_RGB0 | SST_SU_A0;
    }

    pRc->sst.suMode.vFxU32 = su_mode;
    pRc->sst.suParamMask.vFxU32 = su_param_mask;

    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE+2);
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
    SETPD( cmdFifo, ghw0->suMode, su_mode );
    SETPD( cmdFifo, ghw0->suParamMask, su_param_mask);

    HW_ACCESS_EXIT(ACCESS_3D);
    CMDFIFO_EPILOG(cmdFifo);
}
