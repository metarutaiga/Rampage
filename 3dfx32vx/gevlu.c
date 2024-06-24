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
** $Log
** $Revision: 23$
** $Date: 12/8/00 3:53:36 PM PST$
*/

// ge vertex library utility routines

// include files

#ifdef DEBUG
#pragma optimize("",off)
#endif

#include <ddrawi.h>
#include <gevlu.h>
#ifdef SAGE_TEST_ENVIRONMENT
#include <fvfot.h>
#else
#include <d6fvf.h>
#endif
#include <d3dtypes.h>
#include <assert.h>

// procedure definitions

//-----------------------------------------------------------------------------------
//
// gvluQuerySageState()
// 
// "sanitize" ge state info that we need to gather from various sources
//
//-----------------------------------------------------------------------------------

// TEMP: random thoughts about various vertex components

// DECIDE WHETHER TO SEND W (do we always want to send w?)
//
// if w is present in the RC, send it (?)
// if ( pRc->sst.suMode.vFxU32 & SST_SU_W )
//     ss->send_w = 1;
// something to watch for: Rampage turns off w if fog is enabled?
// this occurs after setgestate() gets its chance
//
// if (pRc->fogEnable)
//     pRc->sst.suMode.vFxU32 &= ~(SST_SU_W);	// fog needs SST_SU_Q
// if ( ss->send_w )
//     ss->send_z = 1;
// else
//     ss->send_z = 0;
// if z is present in the RC, send it (?)
// if ( pRc->sst.suMode.vFxU32 & SST_SU_Z )
//     ss->send_z = 1;
// if ( pRc->zEnable && pRc->DDSZHndl )
//     ss->send_z = 1;
// if ( pRc->zEnable && !pRc->DDSZHndl )
//     ss->send_z = 0;

// DECIDE WHETHER TO SEND Q
// if q is present in the RC, send it (?)
// if ( pRc->sst.suMode.vFxU32 & SST_SU_Q )
//     ss->send_q = 1;
// but there's also a Q per stage possibly

// LIGHT INFO: TEMP: is this anywhere near correct?
// if (pRc->GERC.bDfusMaterialSrc)
//     ss->num_light_colors++;
// if (pRc->GERC.bSpecMaterialSrc)
//     ss->num_light_colors++;

// if ( pRc->sst.suParamMask.vFxU32 & SST_SU_RGB0 | SST_SU_A0 )
//     ss->packed_colors = 1;
// else
//     ss->packed_colors = 0;

void gvluQueryGeState(RC *pRc, gvluState *ss)
{
    FxU32 pbits, lastStage, i;
    FxU8 pos = 0;

    memset( &pRc->GERC.geVertexInfo, 0xff, sizeof( GE_VERTEX_INFO ));

    // (for now) always send w, always send z
    ss->send_w = 1;
    ss->send_z = 1;

    // VERTEX BLEND WEIGHTS
    pbits = pRc->current_FVF & D3DFVF_POSITION_MASK;
    ss->vertex_blend = (unsigned char) pRc->GERC.dwNumVertexBlends;

    // TEMP: DEBUG: make sure these things are in sync
    if ( pbits == D3DFVF_XYZB1 ) assert( ss->vertex_blend == 1 );
    if ( pbits == D3DFVF_XYZB2 ) assert( ss->vertex_blend == 2 );
    if ( pbits == D3DFVF_XYZB3 ) assert( ss->vertex_blend == 3 );
    if ( pbits == D3DFVF_XYZB4 ) assert( ss->vertex_blend == 4 );
    if ( pbits == D3DFVF_XYZB5 ) assert( ss->vertex_blend == 5 );

    // check for normals present in the vertex
    if ( pRc->current_FVF & D3DFVF_NORMAL )
        // TEMP: for now, later we might find a reason to eat the normal
        ss->send_normal = 1;

    // decide how many colors and texture coordinates to send

    lastStage = ((pRc->sst.taControl.vFxU32 & SST_TA_NUM_TMUS) >>
                SST_TA_NUM_TMUS_SHIFT);

    for (i=0; i<lastStage+1; i++) {
        // add a color if the current stage has color enabled
	    if ( pRc->sst.suParamMask.vFxU32 & pRc->su_parammask_rgba_flags[i] )
	        ss->num_light_colors++;

        // how many values for q?
        if ( pRc->sst.suParamMask.vFxU32 & pRc->su_parammask_q_flags[i] )
            ss->num_q_values++;
#if 0
        // texture coordinates: TEMP: this is not working at the moment
        if ((TS[i].colorOp != D3DTOP_DISABLE) &&
            ((TS[i].colorArg1 == D3DTA_TEXTURE || TS[i].colorArg2 == D3DTA_TEXTURE) ||
             (TS[i].alphaOp != D3DTOP_DISABLE &&
              (TS[i].alphaArg1 == D3DTA_TEXTURE || TS[i].alphaArg2 == D3DTA_TEXTURE))))
        {
            ss->num_tex_coords++;
            ss->texturing_on = 1;
            ss->num_tex_dimensions = 2; // TEMP: 2D textures only for the moment
        }
#endif
        // TEMP: just check the handle to decide if we send tex coords
        if ( TS[i].textureHandle != 0 ) {
            ss->num_tex_coords++;
            ss->texturing_on = 1;
            ss->num_tex_dimensions = 2; // TEMP: 2D textures only for the moment
        }
    }

    // if the RC says send packed ARGB
    if ( pRc->sst.suMode.vFxU32 & SST_SU_PACKED_ARGB )
        ss->packed_colors = 1;

    // texgen
    // user clip planes
    // edge flags?

    // NEXT: record qdword offsets of all vertex data to inform the state code

    if ( ss->send_w ) // record w offset
        pRc->GERC.geVertexInfo.wpos = pos++;

    if ( ss->vertex_blend != 0 ) // all blend weights will share 1 qdword
        pRc->GERC.geVertexInfo.blend = pos++;

    // if Rampage expects colors to be sent with the vertex
    if ( ss->num_light_colors == 1 ) {
        // if we are also using the normal
        if ( ss->send_normal == 1 ) {
            // the normal gets replaced by the diffuse color in the vertex
            pRc->GERC.geVertexInfo.normal = pos;
            pRc->GERC.geVertexInfo.diffuse_out = pos++;
            // if the diffuse color is also present in the vertex data
            if ( pRc->current_FVF & D3DFVF_DIFFUSE )
                // set its position, which immediately follows the normal
                pRc->GERC.geVertexInfo.diffuse_in = pos++;
        }
        // else we are not using the normal or it is not present
        else {
            // the output diffuse color takes the next position
            pRc->GERC.geVertexInfo.diffuse_out = pos++;
            // if there is a diffuse color provided with the vertex
            if ( pRc->current_FVF & D3DFVF_DIFFUSE )
                // the diffuse in/out positions are the same
                pRc->GERC.geVertexInfo.diffuse_in = 
                pRc->GERC.geVertexInfo.diffuse_out;
        }
        ss->specular_on = 0;
    }
    // TEMP: for now specular is off
    else if ( ss->num_light_colors == 2 ) {
        // pRc->GERC.geVertexInfo.diffuse = pos++;
        // pRc->GERC.geVertexInfo.specular = pos++;
        // ss->specular_on = 1;
    }
    
    switch ( ss->num_tex_coords ) { // record texture coord offsets
        // note deliberate fall-throughs
        case 8: pRc->GERC.geVertexInfo.tex7 = pos++;
        case 7: pRc->GERC.geVertexInfo.tex6 = pos++;
        case 6: pRc->GERC.geVertexInfo.tex5 = pos++;
        case 5: pRc->GERC.geVertexInfo.tex4 = pos++;
        case 4: pRc->GERC.geVertexInfo.tex3 = pos++;
        case 3: pRc->GERC.geVertexInfo.tex2 = pos++;
        case 2: pRc->GERC.geVertexInfo.tex1 = pos++;
        case 1: pRc->GERC.geVertexInfo.tex0 = pos++;
    }
}

//-----------------------------------------------------------------------------------
//
// gvluSetupVB()
// 
//-----------------------------------------------------------------------------------

void gvluSetupVB(
    FXSURFACEDATA  *surfDataVB,
    FVFOFFSETTABLE  fvfOffsetTable[],
    unsigned int    vertexType,
    bParam         *vbRecord,
    int            *streamCount)
{
	vbRecord->vbStartAddr = surfDataVB->lfbPtr; // TEMP: should be hwPtr on real hw
    vbRecord->vbStreamID  = 0;                  // TEMP: not used under dx7
    vbRecord->vbOffset    = 0;
    vbRecord->vbLength    = FVFO_SIZE;          // TEMP: temp, vertex length in dwords
    vbRecord->vbStride    = FVFO_SIZE<<2;       // TEMP: stride equals length in bytes
	vbRecord->vbNumVerts  = ( surfDataVB->endlfbPtr - surfDataVB->lfbPtr ) / vbRecord->vbStride;

    // TEMP: until we support strided or dx8 vertex streams
    *streamCount = 1;
}

//-----------------------------------------------------------------------------------
//
// gvluSetupCfeVList()
// 
//-----------------------------------------------------------------------------------

void gvluSetupCfeVList(
    unsigned int    vertexType,             // driver's vertex type from offset table
    FVFOFFSETTABLE  fvfOffsetTable[],       // the actual fvf offset table
    gvluState      *ss,                     // sage state information
    int            *cfeParams,              // OUT: parameters (qdwords) this vertex
    vParamCfe       cfeList[])              // OUT: sagevlib cfe vertex list
{
    FVFOFFSETTABLE *fvfOffsetEntry;
    TEXTURE_PARAMETERS *currTexParam;
    int i=0, txDimensions, tex;
    vParamCfe *currCfe = &cfeList[0];
    vExpandLane expand_z;
    vExpandLane LaneA, LaneB, LaneC;
    unsigned char numc;

    fvfOffsetEntry = &fvfOffsetTable[ vertexType ];

    // check for vertex position: TEMP: could position be absent?
    if ( FVFO_SX != FVF_VALUE_NOT_PRESENT ) {
        // if w is present, the vertex has been transformed already
        // TEMP: will we ever get this case?
        if ( FVFO_RHW != FVF_VALUE_NOT_PRESENT ) {
            currCfe->vKind = VPARAM_PT_CLIP;
            currCfe->vType = VPARAM_FLOAT;
            currCfe->vNumc = 4;
            currCfe->LaneD = GVL_LANE_VALID;
            currCfe->LaneC = GVL_LANE_VALID;
            currCfe->LaneB = GVL_LANE_VALID;
            currCfe->LaneA = GVL_LANE_VALID;
        }
        // w is not present in the vertex, expand w to 1.0
        else {
            // if z is not present, expand z to 0.0
            if ( FVFO_SZ == FVF_VALUE_NOT_PRESENT ) {
                numc = 2;
                expand_z = GVL_EXPAND_ZERO;
            }
            else {
                numc = 3;
                expand_z = GVL_LANE_VALID;
            }
            currCfe->vKind = VPARAM_PT;
            currCfe->vType = VPARAM_FLOAT;
            currCfe->vNumc = numc;
            currCfe->LaneD = GVL_EXPAND_ONE;
            currCfe->LaneC = expand_z;
            currCfe->LaneB = GVL_LANE_VALID;
            currCfe->LaneA = GVL_LANE_VALID;

            // consume an extra dword present in the vertex for this case
            if ( vertexType == FVFOT_LVERTEX ) {
                currCfe++; i++;
                currCfe->vKind = VPARAM_IGNORED;
                currCfe->vType = 0;
                currCfe->vNumc = 1;
                currCfe->LaneD = GVL_LANE_IDLE;
                currCfe->LaneC = GVL_LANE_IDLE;
                currCfe->LaneB = GVL_LANE_IDLE;
                currCfe->LaneA = GVL_LANE_IDLE;
            }
        }
        currCfe++; i++;
    }

    // check for vertex blend weights
    if ( ss->vertex_blend != 0 ) {

        LaneA = LaneB = LaneC = GVL_EXPAND_ZERO;

        switch ( ss->vertex_blend ) {
        case 5: // TEMP: not supported by d3d under dx7
        case 4: // TEMP: not supported by d3d under dx7
        case 3: // note deliberate fall-thrus
            LaneC = GVL_LANE_VALID;
        case 2:
            LaneB = GVL_LANE_VALID;
        case 1:
            LaneA = GVL_LANE_VALID;
        }

        currCfe->vKind = VPARAM_BLEND;
        currCfe->vType = VPARAM_FLOAT;
        currCfe->vNumc = ss->vertex_blend;
        currCfe->LaneD = GVL_EXPAND_ZERO;
        currCfe->LaneC = LaneC;
        currCfe->LaneB = LaneB;
        currCfe->LaneA = LaneA;
        currCfe++; i++;
    }

    // check for vertex normals
    if ( FVFO_NX != FVF_VALUE_NOT_PRESENT ) {
        currCfe->vKind = VPARAM_NORMAL;
        currCfe->vType = VPARAM_FLOAT;
        currCfe->vNumc = 3;
        currCfe->LaneD = GVL_EXPAND_ZERO;
        currCfe->LaneC = GVL_LANE_VALID;
        currCfe->LaneB = GVL_LANE_VALID;
        currCfe->LaneA = GVL_LANE_VALID;
        currCfe++; i++;
    }

    // check for vertex diffuse color
    if ( FVFO_COLOR != FVF_VALUE_NOT_PRESENT ) {
        currCfe->vKind = VPARAM_COLOR;
        currCfe->vType = VPARAM_UBYTE;
        currCfe->vNumc = 4;
        currCfe->LaneD = GVL_LANE_VALID;
        currCfe->LaneC = GVL_LANE_VALID;
        currCfe->LaneB = GVL_LANE_VALID;
        currCfe->LaneA = GVL_LANE_VALID;
        currCfe++; i++;
    }

    // check for vertex specular color    
    if ( FVFO_SPECULAR != FVF_VALUE_NOT_PRESENT ) {
        if ( ss->specular_on ) {
            currCfe->vKind = VPARAM_COLOR;
            currCfe->vType = VPARAM_UBYTE;
            currCfe->vNumc = 4;
            currCfe->LaneD = GVL_LANE_VALID;
            currCfe->LaneC = GVL_LANE_VALID;
            currCfe->LaneB = GVL_LANE_VALID;
            currCfe->LaneA = GVL_LANE_VALID;
            currCfe++; i++;
        }
        else {
            currCfe->vKind = VPARAM_IGNORED;
            currCfe->vNumc = 1;
            currCfe->LaneD = GVL_LANE_IDLE;
            currCfe->LaneC = GVL_LANE_IDLE;
            currCfe->LaneB = GVL_LANE_IDLE;
            currCfe->LaneA = GVL_LANE_IDLE;
            currCfe++; i++;
        }
    }

    // check for the existence of texture coordinates
    for (tex=0; tex<8; tex++) {
        switch (tex) {
            case 0: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(0); break;
            case 1: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(1); break;
            case 2: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(2); break;
            case 3: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(3); break;
            case 4: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(4); break;
            case 5: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(5); break;
            case 6: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(6); break;
            case 7: currTexParam = (TEXTURE_PARAMETERS*) &FVFO_TU(7); break;
        }

        // check for vertex texture coordinates
        if (currTexParam->u != FVF_VALUE_NOT_PRESENT) {
            txDimensions = 1;
            if ( currTexParam->v != FVF_VALUE_NOT_PRESENT ) txDimensions++;
            if ( currTexParam->w != FVF_VALUE_NOT_PRESENT ) txDimensions++;
            if ( currTexParam->q != FVF_VALUE_NOT_PRESENT ) txDimensions++;

            if ( ss->texturing_on ) {
                currCfe->vKind = VPARAM_TEXTURE;
                currCfe->vType = VPARAM_FLOAT;
                currCfe->vNumc = txDimensions;
                switch (txDimensions) {
                case 1:
                    currCfe->LaneD = GVL_EXPAND_ZERO;
                    currCfe->LaneC = GVL_EXPAND_ZERO;
                    currCfe->LaneB = GVL_EXPAND_ZERO;
                    currCfe->LaneA = GVL_LANE_VALID;
                    break;
                case 2:
                    currCfe->LaneD = GVL_EXPAND_ZERO;
                    currCfe->LaneC = GVL_EXPAND_ZERO;
                    currCfe->LaneB = GVL_LANE_VALID;
                    currCfe->LaneA = GVL_LANE_VALID;
                    break;
                case 3:
                    currCfe->LaneD = GVL_EXPAND_ZERO;
                    currCfe->LaneC = GVL_LANE_VALID;
                    currCfe->LaneB = GVL_LANE_VALID;
                    currCfe->LaneA = GVL_LANE_VALID;
                    break;
                case 4:
                    currCfe->LaneD = GVL_LANE_VALID;
                    currCfe->LaneC = GVL_LANE_VALID;
                    currCfe->LaneB = GVL_LANE_VALID;
                    currCfe->LaneA = GVL_LANE_VALID;
                    break;
                }
                currCfe++; i++;
            }
            else {
                // swallow the tex coordinates in the cfe since texturing is off
                currCfe->vKind = VPARAM_IGNORED;
                currCfe->vNumc = txDimensions;
                currCfe->LaneD = GVL_LANE_IDLE;
                currCfe->LaneC = GVL_LANE_IDLE;
                currCfe->LaneB = GVL_LANE_IDLE;
                currCfe->LaneA = GVL_LANE_IDLE;
                currCfe++; i++;
            }
        }
    }

    // return the number of cfe vertex parameters
    *cfeParams = i;
}

//-----------------------------------------------------------------------------------
//
// gvluSetupVpeVList()
// 
//-----------------------------------------------------------------------------------

void gvluSetupVpeVList(
    int             cfeParams,              // count of vertex parameters in cfe list
    vParamCfe       cfeList[],              // cfe's vertex parameter list
    gvluState      *ss,                     // sage state information
    int            *vpeParams,              // OUT: number of vertex params in vpe
    vParamVpe       vpeList[])              // OUT: sagevlib vpe vertex list
{
    unsigned short lane3, lane2, lane1, lane0;
    int i, k, num_colors;
    int num_texcoords, num_texdimensions;
    int paramCount=0, texNum=0, genNum=0;
    vParamCfe *currCfe = &cfeList[0];
    vParamVpe *currVpe = &vpeList[0];
    i=0, k;

    while (paramCount < cfeParams) {

        switch ( currCfe->vKind ) {
        case VPARAM_PT:                     // vpe transforms points into clip space
        case VPARAM_PT_CLIP:                // vpe will leave these points alone
            currVpe->vKind = VPARAM_PT_CLIP;
            currVpe->vType = VPARAM_FLOAT;
            currVpe->vNumc = 4;
            currVpe->iNum  = 0;
            currVpe->pCorr = 0;

            // set up cbe flags to send the right data to Rampage
            lane2 = ss->send_z ? CBE_DW2 : CBE_IDLE;
            lane3 = ss->send_w ? CBE_DW3 : CBE_IDLE;
            currVpe->cbeFlags = VPARAM_CBE_FORMAT( // z        y        x      w
                                    CBE_UNPACKED, lane2, CBE_DW1, CBE_DW0, lane3 );
            currVpe++; i++;

            // is user clipping turned on? if so send clip codes next
            if ( ss->user_clip_on ) {
                for (k=0; k<ss->user_clip_planes; k++) {
                    currVpe->vKind = VPARAM_UCLIP;
                    currVpe->vType = VPARAM_FLOAT;
                    currVpe->vNumc = 4;
                    currVpe->iNum  = 0;
                    currVpe->pCorr = 0;
                    // TEMP: untested
                    currVpe->cbeFlags = VPARAM_CBE_FORMAT( 0,0,0,0,0 );
                    currVpe++; i++;
                }
            }
            // are edge flags turned on? if so send edge flags next
            if ( ss->edge_flags_on ) {
                currVpe->vKind = VPARAM_EDGE_FLAG;
                currVpe->vType = VPARAM_FLOAT;
                currVpe->vNumc = 4;
                currVpe->cbeFlags = VPARAM_CBE_FORMAT( 0,0,0,0,0 );
                currVpe++; i++;
            }
            currCfe++;
            break;

        case VPARAM_PT_VIEW:
            // will never happen in d3d?
            _asm int 3;
            break;

        case VPARAM_BLEND:
            currVpe->vKind = VPARAM_BLEND;
            currVpe->vType = VPARAM_FLOAT;
            currVpe->vNumc = 4; // it's an entire qdword
            currVpe->iNum  = 0;
            currVpe->pCorr = 0;
            currVpe->cbeFlags = 0; // this data is discarded before the cbe
            currCfe++; currVpe++; i++;
            break;

        case VPARAM_NORMAL:                 // vpe consumes normals, generates colors
            num_colors = ss->num_light_colors;
            for (k=0; k<num_colors; k++) {
                currVpe->vKind = VPARAM_COLOR;
                currVpe->vType = VPARAM_FLOAT;
                currVpe->vNumc = 4;
                currVpe->iNum  = 0;
                currVpe->pCorr = 0;
                // set up cbe to send the right amount of data to Rampage
                currVpe->cbeFlags = VPARAM_CBE_FORMAT( CBE_PACKED, CBE_DW3, CBE_DW2,
                                                       CBE_DW1, CBE_DW0 );
                currVpe++; i++;
            }
            currCfe++;
            break;

        case VPARAM_COLOR:                  // if color is present vpe sends it on
            currVpe->vKind = VPARAM_COLOR;
            currVpe->vType = VPARAM_FLOAT;
            currVpe->vNumc = 4;
            currVpe->iNum  = 0;
            currVpe->pCorr = 0;
            // set up cbe to send the right amount of data to Rampage
            currVpe->cbeFlags = VPARAM_CBE_FORMAT( CBE_PACKED, CBE_DW3, CBE_DW2,
                                                   CBE_DW1, CBE_DW0 );
            currCfe++; currVpe++; i++;
            break;

        case VPARAM_TEXTURE:                // if any are present, vpe just sends on
            currVpe->vKind = currCfe->vKind;
            currVpe->vType = currCfe->vType;
            currVpe->vNumc = currCfe->vNumc;
            currVpe->iNum  = texNum++;
            currVpe->pCorr = 1;

            lane3 = lane2 = lane1 = lane0 = CBE_IDLE;

            switch (ss->num_tex_dimensions) { // note deliberate fall-throughs
                case 4: lane3 = CBE_DW3;
                case 3: lane2 = CBE_DW2;
                case 2: lane1 = CBE_DW1;
                case 1: lane0 = CBE_DW0;
            }
            // set up cbe to send the right amount of data to Rampage
            currVpe->cbeFlags = VPARAM_CBE_FORMAT( CBE_UNPACKED, lane3, lane2,
                                                   lane1, lane0 );
            currCfe++; currVpe++; i++;
            break;

        case VPARAM_GENERIC:
            // TEMP: this will have to be checked case-by-case
            currVpe->vKind = currCfe->vKind;
            currVpe->vType = currCfe->vType;
            currVpe->vNumc = currCfe->vNumc;
            currVpe->iNum  = genNum++;
            currVpe->pCorr = 0;
            lane3 = lane2 = lane1 = lane0 = CBE_IDLE;
            switch (currVpe->vNumc) {
                case 4: lane3 = CBE_DW3; // note deliberate fall-throughs
                case 3: lane2 = CBE_DW2;
                case 2: lane1 = CBE_DW1;
                case 1: lane0 = CBE_DW0;
            }
            // set up cbe to send the right amount of data to Rampage
            currVpe->cbeFlags = VPARAM_CBE_FORMAT( CBE_UNPACKED, lane3, lane2,
                                                   lane1, lane0 );
            currVpe->cbeFlags = 0;
            currCfe++; currVpe++; i++;
            break;

        // increment the cfeList but not the vpeList
        case VPARAM_IGNORED:
            currCfe++;
            break;
        }
        paramCount++;
    }

    // if texgen is enabled
    // TEMP: where in the vertex are these stored?
    if ( ss->texgen_on ) {
        num_texcoords = ss->num_tex_coords;
        num_texdimensions = ss->num_tex_dimensions;
        for (k=0; k<num_texcoords; k++) {
            currVpe->vKind = VPARAM_TEXTURE;
            currVpe->vType = VPARAM_FLOAT;
            currVpe->vNumc = num_texdimensions;
            currVpe->iNum  = 0;     // TEMP: what stage are they bound to?
            currVpe->pCorr = 0;     // TEMP: ?
            // TEMP: figure this out when we get to texgen
            currVpe->cbeFlags = 0;
            currVpe++; i++;
        }
    }

    // return the number of vpe vertex parameters
    *vpeParams=i;
}
