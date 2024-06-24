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
**  24   Rampage   1.23        12/9/00  Jeffrey Newquist OpenGL source 
**       compatibility fix, use SAGECP() macro to use packet macros.
**  23   Rampage   1.22        12/8/00  Evan Leland     merge vertex blend fix; 
**       attempt to over write Dale's changes to merge them later
**  22   Rampage   1.21        12/5/00  Dale  Kenaston  Sage points, lines and 
**       microcode fixes. Modified gvlSetupPpeVertexFormats to set the clip base
**       field of ppeMBControl. Added one the perspective correct count in 
**       intstate to account for position.
**  21   Rampage   1.20        11/14/00 Evan Leland     changed 
**       VPARAM_CBE_DESC_COUNT_DW macro (it had an unused parameter)
**  20   Rampage   1.19        11/10/00 Dale  Kenaston  Sage AGP Vertex Buffer 
**       Fixes. Fixed cfe vertex buffer register setup code.
**  19   Rampage   1.18        11/9/00  Evan Leland     Resync with Jeffrey's 
**       code for computing vf format descriptors; adds code to specify output 
**       dwords and ordering at the cbe.
**  18   Rampage   1.17        11/6/00  Evan Leland     Temporary fix for cbe 
**       swazzling colors (for opengl?) - this puts things the way d3d wants 
**       them.
**  17   Rampage   1.16        11/2/00  Evan Leland     Fixes to get closer to 
**       that 1st triangle on d3d sage.
**  16   Rampage   1.15        11/1/00  Evan Leland     removed some unused 
**       variables
**  15   Rampage   1.14        11/1/00  Evan Leland     More fixes for vertex 
**       size calculations for Sage.
**  14   Rampage   1.13        10/27/00 Tim Little      Added a pragma to 
**       disable optimizations when building for debug.
**  13   Rampage   1.12        10/12/00 Evan Leland     integrate more changes 
**       from Jeffrey Newquist
**  12   Rampage   1.11        10/10/00 Evan Leland     integration of fixes 
**       from Jeff for vertex size calculations
**  11   Rampage   1.10        9/29/00  Jeffrey Newquist Changed how packet 
**       macros are named to be more compatible with the OGL environment.  
**       Merely wrapping a macro around names to build kNAME in D3D and 
**       SAGECP_NAME in OGL.
**  10   Rampage   1.9         9/29/00  Evan Leland     Added code to track the 
**       number of cfe and cbe vertex format descriptors that are needed.
**  9    Rampage   1.8         9/27/00  Evan Leland     Interface changes.
**  8    Rampage   1.7         9/21/00  Evan Leland     added VPARAM_IGNORED 
**       vKind
**  7    Rampage   1.6         9/18/00  Evan Leland     updated gevlib interface
**       to specify separate vParam structures for the cfe and vpe
**  6    Rampage   1.5         9/7/00   Evan Leland     added calculations to 
**       set up ppeCPOffsetRemap register
**  5    Rampage   1.4         9/6/00   Evan Leland     minor name changes and 
**       updates
**  4    Rampage   1.3         9/5/00   Evan Leland     removed some function 
**       prototypes that were no longer needed
**  3    Rampage   1.2         9/5/00   Evan Leland     initial revision; 
**       changed name from sage vertex library to ge vertex library
**  2    Rampage   1.1         9/5/00   Evan Leland     
**  1    Rampage   1.0         9/5/00   Evan Leland     
** $Revision$
** $Date: 12/9/00 7:28:05 AM PST$
*/

// ge vertex library c file

// includes


#if defined(OPENGL)
#include "sstcontext.h"
#include "sagepkt.h"                    // kCfeVbNumShift, etc
#else
#ifdef DEBUG
#pragma optimize("",off)
#endif
#include <packet.h>                     // kCfeVbNumShift, etc
#endif
#include <gevlib.h>			// self
#include <assert.h>                     // assert()

// static procedure prototypes

static void gvlSetupCfeVertexFormats(vParamCfe*, int, int*, vRegs*);
static void gvlSetupVpeVertexFormats(int, int, vRegs*);
static void gvlSetupPpeVertexFormats(vParamVpe*, int, int*, int, vRegs*);
static void gvlSetupCbeVertexFormats(vParamVpe*, int, int, vRegs*);

// local-only defines

#define VC_DESC_INDEX_0     0

#define VFT_UINT            0x0000
#define VFT_INT             0x0100
#define VFT_FLOAT           0x0400
#define VFT_DOUBLE          0x0600
#define VFT_USHORT          0x1000
#define VFT_SHORT           0x1300
#define VFT_UBYTE           0x3000
#define VFT_BYTE            0x3f00
#define VFT_BYPASS_CYCLE    0x4000

#define VFT_TYPECAST_LANEA  0x0100
#define VFT_TYPECAST_LANEB  0x0200
#define VFT_TYPECAST_LANEC  0x0400
#define VFT_TYPECAST_LANED  0x0800

#define LANEA(v)            ((v)<<0)
#define LANEB(v)            ((v)<<2)
#define LANEC(v)            ((v)<<4)
#define LANED(v)            ((v)<<6)

#define CBE_LANEA(v)        ((v)<<0)
#define CBE_LANEB(v)        ((v)<<4)
#define CBE_LANEC(v)        ((v)<<8)
#define CBE_LANED(v)        ((v)<<12)

#define CBE_VFD_LANE_IDLE   0x4
#define CBE_VFD_LOAD_DW0    0x0
#define CBE_VFD_LOAD_DW1    0x1
#define CBE_VFD_LOAD_DW2    0x2
#define CBE_VFD_LOAD_DW3    0x3
#define CBE_VFD_LOAD_DW0_Q  0x8
#define CBE_VFD_LOAD_DW1_Q  0x9
#define CBE_VFD_LOAD_DW2_Q  0xa
#define CBE_VFD_LOAD_DW3_Q  0xb

// procedure definitions

//-----------------------------------------------------------------------------------
//
// gvlSetupVertexStreams()
// 
//-----------------------------------------------------------------------------------

#define VB_DESCRIPTOR   (out_vbRegs->cfeVbDescriptor[i]) // handy shorthand
#define STREAM          (cfeInStreams[i])                 // handy shorthand

void gvlSetupVertexStreams(             // set up the cfe to read vertex buffers
    bParam      cfeInStreams[],         // input streams descriptor array
    int         cfeStreamCount,         // count of input streams
    vbRegs     *out_vbRegs)             // IN/OUT: structure of register values
{
    int i;

    // set number of streams/vbuffers for cfe to read; always use descriptor 0
    out_vbRegs->cfeVbStart = ( cfeStreamCount - 1 ) << SAGECP(CfeVbNumShift) | 0;

    for (i=0; i<cfeStreamCount; i++) {
        
        VB_DESCRIPTOR.vbdStride = STREAM.vbStride;
        VB_DESCRIPTOR.vbdBase   = STREAM.vbStartAddr + STREAM.vbOffset;
        VB_DESCRIPTOR.vbdMax    = STREAM.vbNumVerts - 1;
        VB_DESCRIPTOR.vbdLength = STREAM.vbLength;
    }

    out_vbRegs->numCfeVbDesc = cfeStreamCount;
}

//-----------------------------------------------------------------------------------
//
// gvlSetupVertexFormat()
// 
//-----------------------------------------------------------------------------------

void gvlSetupVertexFormat(              // set up the vertex format for sage registers
    vParamCfe   cfeInVertex[],          // the cfe input vertex stream description
    int         cfeInVpCount,           // the number of vertex params in cfe stream
    vParamVpe   vpeOutVertex[],         // the vpe output vertex stream description
    int         vpeOutVpCount,          // the number of vertex params in vpe stream
    vRegs      *out_vRegs)              // IN/OUT: structure of register values
{
    unsigned int cfeOutVpCount;
    unsigned int ppeOutVpCount;

	// set up the various cfe vertex format registers
	gvlSetupCfeVertexFormats(
        cfeInVertex,                    // cfe input vertex stream
		cfeInVpCount,                   // cfe input parameter count
        &cfeOutVpCount,                 // OUT: cfe output vertex size in qdwords
		out_vRegs);                     // OUT: struct of sage vertex size regs

	// set up the various vpe vertex format registers
	gvlSetupVpeVertexFormats(
		vpeOutVpCount,                  // vpe output vertex size in qdwords
        cfeOutVpCount,                  // cfe output vertex size in qdwords
		out_vRegs);                     // OUT: struct of sage vertex size regs

	// set up the various ppe vertex format registers
	gvlSetupPpeVertexFormats(
	    vpeOutVertex,                   // vpe output vertex stream
	    vpeOutVpCount,                  // vpe output vertex size in qdwords
        &ppeOutVpCount,                 // ppe output vertex size in qdwords
        out_vRegs->flags & VPARAM_FLAG_TWO_SIDED, // two-sided color flag
		out_vRegs);                     // OUT: struct of sage vertex size regs

	// set up the various cbe vertex format registers
	gvlSetupCbeVertexFormats(
        vpeOutVertex,                   // vpe output vertex stream
        vpeOutVpCount,                  // vpe output vertex size in qdwords
	    ppeOutVpCount,                  // ppe output vertex size in qdwords
	    out_vRegs);                     // OUT: struct of sage vertex size regs
}

//-----------------------------------------------------------------------------------
//
// gvlSetupCfeVertexFormats()
//                           
//-----------------------------------------------------------------------------------

#define VF_DESCRIPTOR(n) (out_vregs->cfeVfSram[n])     // handy shorthand
#define VC_DESCRIPTOR(n, v)                       \
{                                                 \
    int dw=(n)/16;                                \
    int ofs=(n)%16;                               \
    out_vregs->cfeVcSram[dw] |= ((v) << (ofs*2)); \
}

static void gvlSetupCfeVertexFormats(
    vParamCfe   cfeIn[],                // cfe input vertex stream description
    int         cfeInVpCount,           // number of vertex params in cfe stream
    int        *cfeOutVpCount,          // size of a cfe output vertex in qdwords
    vRegs      *out_vregs)              // IN/OUT: structure of register values
{
    unsigned int dwPerParam, dwPerVertex=0, vcCount=0, vfCount=0, vfDesc;
    int numParamDescriptors, numCompPerDescriptor;
    int i, ignoredParams=0, compRemaining, lane, d;

    // Must clear vertex component descriptor SRAM, since
    // the cleared value is default for non-bypass parameters
    for (i=0; i<8; i++)
        out_vregs->cfeVcSram[i] = 0;

    // VERTEX COMPONENT DESCRIPTOR and VERTEX FORMAT DESCRIPTOR loop
    for (i=0; i<cfeInVpCount; i++) {

        // this is for swallowing empty dwords in the vertex stream
        if (cfeIn[i].vKind == VPARAM_IGNORED) {
            ignoredParams++;
            vfDesc = VFT_BYPASS_CYCLE;
            dwPerParam = cfeIn[i].vNumc;            // ignore this many dwords
            numParamDescriptors = 1;
            numCompPerDescriptor = cfeIn[i].vNumc;
        }
        else {
            // set dequant formats: this assumes all types are the same in a parameter
            switch (cfeIn[i].vType) {

            case VPARAM_BYTE:                       // dequantize bytes into a qdword
                vfDesc = VFT_BYTE;
                dwPerParam = 1;
                numParamDescriptors = dwPerParam;
                numCompPerDescriptor = 4;
                break;

            case VPARAM_UBYTE:                      // dequantize bytes into a qdword
                vfDesc = VFT_UBYTE;
                dwPerParam = 1;
                numParamDescriptors = dwPerParam;
                numCompPerDescriptor = 4;
                break;

            case VPARAM_SHORT:                      // dequantize shorts into a qdword
                vfDesc = VFT_SHORT;
                dwPerParam = (cfeIn[i].vNumc + 1) / 2;
                numParamDescriptors = dwPerParam;
                numCompPerDescriptor = 2;
                break;

            case VPARAM_USHORT:                     // dequantize shorts into a qdword
                vfDesc = VFT_USHORT;
                dwPerParam = (cfeIn[i].vNumc + 1) / 2;
                numParamDescriptors = dwPerParam;
                numCompPerDescriptor = 2;
                break;

            case VPARAM_INT:                        
                dwPerParam = cfeIn[i].vNumc;
                if (cfeIn[i].typecast) {            // typecast int to float
                    vfDesc = VFT_BYPASS_CYCLE;
                    switch (cfeIn[i].vNumc) {       // deliberate fall-throughs
                    case 4:
                        vfDesc |= VFT_TYPECAST_LANED;
                    case 3:
                        vfDesc |= VFT_TYPECAST_LANEC;
                    case 2:
                        vfDesc |= VFT_TYPECAST_LANEB;
                    case 1:
                        vfDesc |= VFT_TYPECAST_LANEA;
                    }
                    numParamDescriptors = 1;
                    numCompPerDescriptor = 4;
                } else {                            // dequantize ints into a qdword
                    vfDesc = VFT_INT;
                    numParamDescriptors = dwPerParam;
                    numCompPerDescriptor = 1;
                }
                break;

            case VPARAM_UINT:                       // dequantize ints into a qdword
                dwPerParam = cfeIn[i].vNumc;
                vfDesc = VFT_UINT;
                numParamDescriptors = dwPerParam;
                numCompPerDescriptor = 1;
                break;

            case VPARAM_FLOAT:
                dwPerParam = cfeIn[i].vNumc;
                vfDesc = VFT_BYPASS_CYCLE;
                numParamDescriptors = 1;
                numCompPerDescriptor = cfeIn[i].vNumc;
                break;
            }
        }

        dwPerVertex += dwPerParam;

        // fill in VF and VC descriptors
        compRemaining = cfeIn[i].vNumc;
        for (d=0; d<numParamDescriptors; d++,vfCount++,vcCount++) {
            VF_DESCRIPTOR(vfCount) = vfDesc;
            if (vfDesc & VFT_BYPASS_CYCLE) {
                VC_DESCRIPTOR(vcCount, compRemaining-1);
            }
            if (cfeIn[i].vKind != VPARAM_IGNORED) {
                // fill in valid lanes for this dword
                for (lane=0; lane<numCompPerDescriptor && compRemaining>0; lane++,compRemaining--) {
                    VF_DESCRIPTOR(vfCount) |= GVL_LANE_VALID << (lane*2);
                }
            }
        }

        if (cfeIn[i].vKind != VPARAM_IGNORED) {
            // fill in implied lanes
            switch (cfeIn[i].vNumc) {   // deliberate fall-throughs
            case 1:
                VF_DESCRIPTOR(vfCount-1) |= cfeIn[i].LaneB << (lane*2); lane++;
            case 2:
                VF_DESCRIPTOR(vfCount-1) |= cfeIn[i].LaneC << (lane*2); lane++;
            case 3:
                VF_DESCRIPTOR(vfCount-1) |= cfeIn[i].LaneD << (lane*2);
            }
        }
    }

    out_vregs->numCfeVfDesc = vfCount;

    // VERTEX COMPONENT START REGISTER (always use descriptor 0 for now)
    // count 0 in HW means 1 descriptor, not documented
    out_vregs->cfeVcStart = ((vcCount-1) << SAGECP(CfeVcNumShift)) | VC_DESC_INDEX_0;

    // VERTEX FORMAT START REGISTER (always use descriptor 0 for now)
    // count 0 in HW means 1 descriptor, not documented
    out_vregs->cfeVfStart = (vfCount-1) << SAGECP(CfeVfNumShift);

    // PACKET 6 SIZE REG SETUP
    out_vregs->cfePkt6Size = dwPerVertex;
    *cfeOutVpCount = cfeInVpCount - ignoredParams;
}

//-----------------------------------------------------------------------------------
//
// gvlSetupVpeVertexFormats()
// 
//-----------------------------------------------------------------------------------

static void gvlSetupVpeVertexFormats(
    int         vpeOutVpCount,          // number of vertex qdwords OUTPUT from vpe
    int         cfeInVpCount,           // number of vertex qdwords INPUT from cfe
    vRegs      *out_vregs)              // IN/OUT: structure of register values
{
    unsigned int vpeStride = cfeInVpCount > vpeOutVpCount ? cfeInVpCount : vpeOutVpCount;

    // TEMP: am I missing something?
    // VPE_IN_SIZE 0 in HW means 1 parameter, not documented
    out_vregs->vpeVertexSize = (vpeStride << SAGECP(VpeVertexStrideShift)) | (cfeInVpCount-1);
}

//-----------------------------------------------------------------------------------
//
// gvlSetupPpeVertexFormats()
// 
//-----------------------------------------------------------------------------------

#define VPARAM_KIND (vpeOut[i].vKind) // handy shorthand
#define VPARAM_NUM  (vpeOut[i].iNum)

static unsigned int intState[] = {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0xffffff00, //D3D wrap mode
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000010  //Pointer to FP_STATE wrap enables
};

static void gvlSetupPpeVertexFormats(
    vParamVpe   vpeOut[],               // vpe output vertex stream description
    int         vpeOutVpCount,          // number of vertex qdwords in vpe output stream
    int        *ppeOutVpCount,          // OUT: number of vertex qdwords output from ppe
    int         bTwoSided,              // boolean: enable two-sided colors
    vRegs      *out_vregs)              // IN/OUT: structure of register values
{
    int i, pos, ef_index=0, uc0_index=0, uc1_index=0;
    unsigned int mb_ld_count=0, mb_ld_mask=0, mb_xfer_size=0;
    unsigned char *pCpOR;
    int pcIdx=0, npcIdx=0, pcStart=1, npcStart=0;
    int xferStride = bTwoSided ? 3 : 2;
    unsigned int color0CP=0, color1CP=0;
    bTwoSided = bTwoSided ? 1 : 0;  //make sure it's a 1 or 0 for math below

    out_vregs->ppeMeshLoadMask = 0;
    (*ppeOutVpCount) = 0;

    // disable PPE-CBE transfer, by default
    for (i=0; i<32; i++)
        out_vregs->ppeXferDesc[i] = 1 << SAGECP(PpeXferDescTransferEnableShift);

    // search vpeOut stream for params the ppe won't use in the mesh buffer
    for (i=0; i<vpeOutVpCount; i++) {

        if (VPARAM_KIND == VPARAM_EDGE_FLAG) {
            ef_index = i;
            continue;
        }
        if (VPARAM_KIND == VPARAM_UCLIP) {
            if (VPARAM_NUM == 1)
                uc1_index = i;
            else
                uc0_index = i;
            continue;
        }
        if (VPARAM_KIND == VPARAM_BLEND)
            continue;

        // TEMP: just in case the vpe sends the normal for some reason
        if (VPARAM_KIND == VPARAM_NORMAL)
            continue;

        // MESH LOAD MASK REGISTER SETUP
        out_vregs->ppeMeshLoadMask |= 1 << i;
        if (vpeOut[i].pCorr == 0)
            npcStart++;
        if (VPARAM_KIND == VPARAM_PT_VIEW)
            pcStart = 2;

        // TEMP: assume these are the only params ppe will use but not xfer to cbe
        if ((VPARAM_KIND != VPARAM_PT) && (VPARAM_KIND != VPARAM_PT_VIEW)) {
            mb_xfer_size++;
            
            // XFER DESCRIPTOR SRAM SETUP
            // Rampage order: (Q==Q-coord, C==Color, T==texture-coord)
            // Convention: EVEN color is front color, ODD color is back color
            // Two-sided mode: Clip Q C0 C1 T0 C2 C3 T1 ...
            // One-sided mode: Clip Q C0 T0 C2 T1 ...
            // Holes fixed after descriptors built
            // FIXME: Q coordinate support
            switch (VPARAM_KIND) {
            case VPARAM_PT_CLIP:
                // clip-coord always goes first
                assert(mb_ld_count==0);
                out_vregs->ppeXferDesc[0] = mb_ld_count;
                (*ppeOutVpCount) += 1;
                break;
            case VPARAM_COLOR:
                // two-sided: XX XX C0 C1 XX C2 C3 XX ...
                // one-sided: XX XX C0 XX C2 XX ...
                pos = ((VPARAM_NUM>>1)*xferStride + (VPARAM_NUM&bTwoSided) + 2) & 31;
                out_vregs->ppeXferDesc[pos] = mb_ld_count;
                out_vregs->ppeXferDesc[pos] |= bTwoSided << SAGECP(PpeXferDescConditionalShift);
                out_vregs->ppeXferDesc[pos] |= (VPARAM_NUM&1) << SAGECP(PpeXferDescPolarityShift);
                if ((VPARAM_NUM&1) == 0)
                    (*ppeOutVpCount) += 1; //only output one of the two colors
                break;
            case VPARAM_TEXTURE:
            case VPARAM_GENERIC:
                // stride=3: XX XX XX XX T0 XX XX T1 XX XX T2 ...
                // stride=2: XX XX XX T0 XX T1 XX T2 ...
                out_vregs->ppeXferDesc[VPARAM_NUM*xferStride+bTwoSided+3] = mb_ld_count;
                (*ppeOutVpCount) += 1;
                break;
            }
        }

        mb_ld_count++;
    }

    // remove gaps in transfer table
    pos = 0;
    for (i=0; i<32; i++) {
        if ((out_vregs->ppeXferDesc[i] & (1 << SAGECP(PpeXferDescTransferEnableShift))) == 0) {
            if (i != pos) {
                // moving descriptor to fill a hole
                out_vregs->ppeXferDesc[pos++] = out_vregs->ppeXferDesc[i];
                out_vregs->ppeXferDesc[i] = 1 << SAGECP(PpeXferDescTransferEnableShift);
            } else {
                pos++;
            }
        }
    }

    // PPE CP OFFSET REMAP SETUP
    // get a byte pointer to the cp offset remap table
    pCpOR = (unsigned char *)&out_vregs->ppeCPOffsetRemap[0];

    // initialize the cp offset remap table
    for (i=0; i<32; i++) pCpOR[ i ] = 0x1f;

    // cycle through the vertex parameters again to set up cp offset remap table
    pos = 0; //use pos to indicate mesh load number, not parameter number
    for (i=0; i<vpeOutVpCount; i++) {
        switch (VPARAM_KIND) {
        case VPARAM_PT_CLIP:
            // ucode offset 0 == vertex offset 0
            assert(pos==0);
            pCpOR[0] = pos++;
            break;
        case VPARAM_PT_VIEW:
            // ucode offset 1 is view coordinates
            pCpOR[1] = pos++;
            break;
        case VPARAM_TEXTURE:
            // map textures to perspective correct ucode offset remap group
            assert( vpeOut[i].pCorr == 1 );
            pCpOR[ pcIdx+pcStart ] = pos++;
            pcIdx++;
            break;
        case VPARAM_COLOR:
            // put into the perspective-incorrect ucode offset remap group
            assert( vpeOut[i].pCorr == 0 );
            pCpOR[ npcIdx+npcStart ] = pos++;
            if (VPARAM_NUM==0) color0CP = npcIdx+npcStart;
            else if (VPARAM_NUM==1) color1CP = npcIdx+npcStart;
            npcIdx++;
            break;
        case VPARAM_GENERIC:
            // check whether the parameter is perspective-correct or not
            if ( vpeOut[i].pCorr == 1 ) {
                // put into the perspective-correct ucode offset remap group
                pCpOR[ pcIdx+pcStart ] = pos++;
                pcIdx++;
            }
            else {
                // put into the perspective-incorrect ucode offset remap group
                pCpOR[ npcIdx+npcStart ] = pos++;
                npcIdx++;
            }
            break;
        case VPARAM_UCLIP:
        case VPARAM_EDGE_FLAG:
        case VPARAM_BLEND:
            // legal vertex parameters, but not placed in meshbuffer
            break;
        case VPARAM_NORMAL:
        case VPARAM_PT:
            // illegal: these should NOT be in the mesh buffer
            assert(0);
            break;
        }
    }

    // MESH BUFFER CONTROL REGISTER SETUP
    // TEMP: no known cases of the ppe creating data, so stride == load count
    out_vregs->ppeMBControl |= ((mb_ld_count-1) << SAGECP(PpeMeshBufControlLoadCntShift)) | mb_ld_count;

    // MESH LOAD TRAP OFFSET REGISTER SETUP
    out_vregs->ppeMeshLoadTrapOffset = (ef_index  << SAGECP(PpeMlLoadTrapOffsetsEdgeFlagShift)) |
                                       (uc0_index << SAGECP(PpeMlLoadTrapOffsetsUCP0Shift))     |
                                       (uc1_index << SAGECP(PpeMlLoadTrapOffsetsUCP1Shift));
    // PPE VERTEX SIZE REGISTER SETUP
    out_vregs->ppeVertexSize |= ((vpeOutVpCount-1) << SAGECP(PpeVertexSizeInputShift)) | (mb_xfer_size-1);

    // PPE Integer state
    for (i=0; i<SAGECP(PpeIntStateCnt); i++)
        out_vregs->ppeIntState[i] = intState[i];
    out_vregs->ppeIntState[7] = color0CP;                           // Color0 for flat shading
    out_vregs->ppeIntState[8] = (color1CP==0)?color0CP:color1CP;    // Color1 for flat shading
    out_vregs->ppeIntState[9] = npcStart;                           // pointer to start of persp. incorrect
    out_vregs->ppeIntState[10] = npcIdx;                            // # of persp. incorrect
    out_vregs->ppeIntState[11] = 0;                                 // pointer to start of persp. correct
    out_vregs->ppeIntState[12] = pcIdx+pcStart;                     // # of persp. correct
    out_vregs->numPpeXferDesc = mb_xfer_size;
}

// Rampage expects vertex parameters in the
// following order (each is a dword):
//
// q7  nth+7             direction of stream
// t7  nth+6                    v
// s7  nth+5                    |
// a7  nth+4                    |
// b7  nth+3                    v
// g7  nth+2
// r7  nth (or packed argb)     v
// ...                          |
// q0  12th                     |
// t0  11th                     v
// s0  10th
// a0   9th                     v
// b0   8th                     |
// g0   7th                     |
// r0   6th (or packed argb)    v
// q    5th
// z    4th                     v
// y    3rd                     |
// x    2nd                     |
// w    1st                     v

//-----------------------------------------------------------------------------------
//
// gvlSetupCbeVertexFormats()
// 
//-----------------------------------------------------------------------------------

#define VFDESC (out_vregs->cbeVfSram[desc])

static void gvlSetupCbeVertexFormats(
    vParamVpe   vpeOut[],               // vpe output vertex stream description
    int         vpeOutVpCount,          // number of params in vertex from vpe
    int         cbeInCount,             // number of params in vertex from ppe
    vRegs      *out_vregs)              // IN/OUT: structure of register values
{
    int i=0, entry, desc, enable;
    int meshMap[32];
    int paramSent=0;
    
    // map mesh locations to VPE outputs
    for (entry=0; entry<out_vregs->numPpeXferDesc; entry++) {
        int pos = out_vregs->ppeXferDesc[entry] & 31;   // mesh location
        int i = 0;                                      // VPE out location
        int j = pos;
        // skip entries not put in meshbuffer
        while (j--) {
            i++;
            // skip any non-mesh entries
            while ((out_vregs->ppeMeshLoadMask & (1<<i)) == 0) i++;
        }
        meshMap[pos] = i;
    }

    // CBE PACKET 6 SIZE REGISTER and DESCRIPTOR SETUP
    out_vregs->cbePkt6Size = cbeInCount;
    out_vregs->cbeVfdStart = 0;         // always use descriptor 0 for now
    out_vregs->numCbeVfDesc = cbeInCount;
    out_vregs->numPkt3Dwords = 0;

    // initialize all to idle
    for (desc=0; desc<64; desc++)
        VFDESC = VPARAM_CBE_FORMAT(CBE_UNPACKED, CBE_IDLE, CBE_IDLE, CBE_IDLE, CBE_IDLE);

    desc = 0;

    // look at each vertex parameter and decide how to send it out
    for (entry=0; entry<out_vregs->numPpeXferDesc; entry++) {
        i = meshMap[out_vregs->ppeXferDesc[entry] & 31];
        // skip parameters not transferred to CBE
        enable = out_vregs->ppeXferDesc[entry] & (SAGECP(PpeXferDescTransferEnableMask)
                                               << SAGECP(PpeXferDescTransferEnableShift));
        if (enable == 1)
            continue;
        // send parameters only once, used for conditional PPE transfers
        if (paramSent & (1<<i))
                continue;
        paramSent |= 1<<i;
        switch (VPARAM_KIND) {
        case VPARAM_PT_CLIP:
            VFDESC = vpeOut[i].cbeFlags;
            out_vregs->numPkt3Dwords += VPARAM_CBE_DESC_COUNT_DW( VFDESC );
            desc++;
            break;
            
        case VPARAM_COLOR:
            VFDESC = vpeOut[i].cbeFlags;
            out_vregs->numPkt3Dwords += VPARAM_CBE_DESC_COUNT_DW( VFDESC );
            desc++;
            break;

        case VPARAM_TEXTURE:
            VFDESC = vpeOut[i].cbeFlags;
            out_vregs->numPkt3Dwords += VPARAM_CBE_DESC_COUNT_DW( VFDESC );
            desc++;
            break;

        case VPARAM_GENERIC:
            VFDESC = vpeOut[i].cbeFlags;
            out_vregs->numPkt3Dwords += VPARAM_CBE_DESC_COUNT_DW( VFDESC );
            desc++;
            break;

        case VPARAM_PT:
        case VPARAM_PT_VIEW:
        case VPARAM_UCLIP:
        case VPARAM_EDGE_FLAG:
        case VPARAM_NORMAL:
            // these should be stripped from the vertex by now
            assert (0);
            break;
        }
    }
}
