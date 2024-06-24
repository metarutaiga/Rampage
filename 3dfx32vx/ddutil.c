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

#include "precomp.h"
#include "ddovl32.h"
#include "fifomgr.h"
#include "ddvpe32.h"

#include "sst2glob.h"

//
// Usage rules for 3D MOPS:
//
// peFbzMode:
//    FB_FORMAT, EN_ALPHA_BUFFER, STAGGER_EN --> Pcache flush & Stall 3D PE
//    ARGB_WRMASK --> Stall 3D only
//
// peStencil:
//    if to/from 0 then Pcache flush
//
// peCache:
//    Any change --> Pcache flush & Stall 3D PE
//
// peColBuffeAddr:
// peAuxBufferAddr:
// peBufferSize:
//    Any change --> Pcache flush & Stall 3D PE
//
// peFogTable:
//    Any write --> Stall 3D PE
//
// texture Palette space
//    Any write --> Stall 3D TA
//
/*
----------------------------------------------------------------------------
  preSetRegs()
  PreSet 3D registers
----------------------------------------------------------------------------
*/

static DWORD dwVtIndex;
static DWORD maxVt[]={32,32,28,23,19,17,15,13};  //256 /(3+2*tmus)

DWORD preSetRegs( NT9XDEVICEDATA *ppdev, FXSURFACEDATA* surfaceData,
        DWORD wHeight, DWORD **pcmdFifo, DWORD *phwIndex)
{
    DWORD  dwvpMode,dwAlphaTest, dwAlphaMode;
    DWORD i;
  	FIFO_PROLOG;
	 
	// D3D-Bringup, be more seletive later XXXX
	CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );

//	SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
//	  ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));

	SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
	  ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));


    dwvpMode =  SST_VP_EN_STQ_PROJ | SST_VP_EN_RGBA_SCALE |
	                                 SST_VP_WINDOW_SCALE 
                                  | (0x87 << SST_VP_COLOR_CLAMP_EXP_SHIFT);

    dwAlphaTest = 0;
                     
    dwAlphaMode =  0;

	CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 7+
                                3*(PH1_SIZE + 1) +
                                PH1_SIZE + 2 + PH4_SIZE + 2+
                                8*(PH1_SIZE + 2) +
                                PH1_SIZE+2 +    // vpSTscale0, vpSTscale1
	                            PH1_SIZE+1 +    // fog,
	                            PH1_SIZE+2 +    // alpha
	                            PH1_SIZE+1 +    // SDConst
	                            PH1_SIZE+1 +    // line width
	                            PH1_SIZE+2 +    // raControl,stipple
	                            PH1_SIZE+2);    // stencil

	SETPH( cmdFifo, CMDFIFO_BUILD_PK1(7, SST_UNIT_FBI, vpMode));
	SETPD( cmdFifo, ghw0->vpMode,  dwvpMode);
    
    // With XY Projection disabled set viewport size
	// to pass screen coordinates thru without scaling.

	// XY offset is set to 0.5 for d3d pixel centers.
	SETFPD( cmdFifo, ghw0->vpSizeX,   1.0f );
	SETFPD( cmdFifo, ghw0->vpCenterX, 0.0f );

	SETFPD( cmdFifo, ghw0->vpSizeY,   1.0f );
	SETFPD( cmdFifo, ghw0->vpCenterY, 0.0f );

	SETFPD( cmdFifo, ghw0->vpSizeZ,   1.0f );
	SETFPD( cmdFifo, ghw0->vpCenterZ, 0.0f );

    // Set up HW Clip Register. Will clip primitives that pass through Guard band.
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipMinXMaxX[0]));
    SETPD( cmdFifo, ghw0->suClipMinXMaxX[0], 2048 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipMinYMaxY[0]));
    SETPD( cmdFifo, ghw0->suClipMinYMaxY[0], 2048 );
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, suClipEnables));
    SETPD( cmdFifo, ghw0->suClipEnables, 1 );


	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, raControl ) );      
	SETPD( cmdFifo, ghw0->raControl, (1 << SST_RA_STIPPLE_REPEAT_SHIFT) |
	                          ( SST_COL_OF_8 << SST_RA_COL_WIDTH_SHIFT ) |
	                          ( SST_ROW_OF_16 << SST_RA_ROW_HEIGHT_SHIFT ));
	SETPD( cmdFifo, ghw0->raStipple, 0xffffffff);
	SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2, SST_UNIT_FBI, peColBufferAddr));
	SETPD( cmdFifo, ghw0->peColBufferAddr, surfaceData->hwPtr);
    if(surfaceData->tileFlag & MEM_IN_TILE1)
    {
    	SETPD( cmdFifo, ghw0->peBufferSize, SET_PE_BUFFER_SIZE_TILED( 
	             surfaceData->dwStride,
	             wHeight ));
    }
    else
    	SETPD( cmdFifo, ghw0->peBufferSize,
	             ((surfaceData->dwStride>> 5) <<SST_PE_BUFFER_STRIDE_SHIFT) |
	             (wHeight << SST_PE_BUFFER_HEIGHT_SHIFT));


    for(i =0; i< 8; i++)
    {
	 SETPH(cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_TMU_0, taChromaKey));
	 SETPD(cmdFifo, SST_TREX(ghw0,i)->taChromaKey,0);
     SETPD(cmdFifo, SST_TREX(ghw0,i)->taChromaRange,0);
    }


	SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, vpSTscale0));
	SETPD( cmdFifo, ghw0->vpSTscale0, 0xbbbbbbbb); //for NPT
	SETPD( cmdFifo, ghw0->vpSTscale1, 0xbbbbbbbb);

	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFogMode ) );
	SETPD( cmdFifo, ghw0->peFogMode, 0 );

	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, peAlphaTest ) );
	SETPD( cmdFifo, ghw0->peAlphaTest,dwAlphaTest);
	SETPD( cmdFifo, ghw0->peAlphaMode,dwAlphaMode);

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peSDConst ) );
    SETPD( cmdFifo, ghw0->peSDConst, 0);


	// send line state - width is constant, but we may have swapped w/ a non D3D 3D context (OpenGL??)
	//  so reload it - line pattern is a D3D state member
	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, suLineWidth ) );
	SETPD( cmdFifo, ghw0->suLineWidth, DEFAULT_LINEWIDTH );

	// Stencil Buffer Setup and Operation
	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, SST_UNIT_FBI, peStencil ) );
	SETPD( cmdFifo, ghw0->peStencil, 0 );
	SETPD( cmdFifo, ghw0->peStencilOp, 0 );

	_D3(lastContext) = 0;
    UPDATE_HW_STATE (SC_ALL_CHANGED);

    FIFO_EPILOG;
    return D3D_OK;
}

/*
----------------------------------------------------------------------------
  Change stride of color buffer
----------------------------------------------------------------------------
*/
void SetUpColBuffSize( NT9XDEVICEDATA *ppdev, DWORD tileFlag, 
	DWORD dwStride, DWORD dwHeight,DWORD **pcmdFifo, DWORD *phwIndex)
{
  	FIFO_PROLOG;

    // D3D-Bringup, be more seletive later XXXX
	CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE + PH1_SIZE + 1);
    
//   	SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
//	  ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));

   	SETMOP( cmdFifo,  SST_MOP_FLUSH_PCACHE |
	  ((SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));

	SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, peBufferSize));
    if(tileFlag & MEM_IN_TILE1)
    {
    	SETPD( cmdFifo, ghw0->peBufferSize, SET_PE_BUFFER_SIZE_TILED( 
	             dwStride, dwHeight ));
    }
    else
    	SETPD( cmdFifo, ghw0->peBufferSize,
	             ((dwStride>> 5) <<SST_PE_BUFFER_STRIDE_SHIFT) |
	             (dwHeight << SST_PE_BUFFER_HEIGHT_SHIFT));

    UPDATE_HW_STATE (SC_PE_SETUP);
    
	FIFO_EPILOG;        
}



/*
----------------------------------------------------------------------------
Set up TMU registers
Note: There are numStages+1 tmus need to be set
----------------------------------------------------------------------------
*/
void SetupTmus( NT9XDEVICEDATA *ppdev, TMURegs *sst, DWORD numStages,
        DWORD **pcmdFifo, DWORD *phwIndex)
{

    int j;
    FxU32         su_param_mask;
    FIFO_PROLOG;

    if (IS_SAGE_ACTIVE)
    {
        // We need to tell SAGE what PKT3 size is implied by suParamMask and suMode below
        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+1);
        SETPH( cmdFifo, CMDFIFO_BUILD_PK7_w(1, kPkt3SizeReg, 0, 0));
        //              XY + W + ST * numStages
        SETCF( cmdFifo, 2  + 1 + 2  * numStages);
    }

    CMDFIFO_CHECKROOM( cmdFifo,
        (numStages+1) * (PH4_SIZE+4 + PH1_SIZE+4 + PH1_SIZE+5 + PH1_SIZE+ 2) +
		PH1_SIZE+1 + 1 + // taControl
        PH1_SIZE + 2 +	// suMode
	MOP_SIZE   +    // MOP
        0);

    su_param_mask = 0;
    for( j=0; j <= (int)numStages; j++)
    {
      su_param_mask <<= SST_SU_RGB1_SHIFT;
      su_param_mask |=  SST_SU_ST0;

    }

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1(2, SST_UNIT_FBI, suMode));
    SETPD( cmdFifo, ghw0->suMode, (0 << SST_SU_INDEX_SHIFT)| SST_SU_W);
    SETPD( cmdFifo, ghw0->suParamMask, su_param_mask);



	 for(j=(int)numStages; j >= 0; j-- ) 
	 {

		SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R5, SST_UNIT_TMU_0+j, taMode));
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taMode, sst->TMU[j].taMode );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taLMS,  sst->TMU[j].taLMS );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taShiftBias, sst->TMU[j].taShiftBias );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taBaseAddr0, sst->TMU[j].taBaseAddr0 );

		SETPH( cmdFifo, CMDFIFO_BUILD_PK1(4, SST_UNIT_TMU_0+j, taColorAR0));
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorAR0,  sst->TMU[j].taColorAR0)
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorGB0,  sst->TMU[j].taColorGB0)
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorAR1,  sst->TMU[j].taColorAR1)
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taColorGB1,  sst->TMU[j].taColorGB1)

		SETPH( cmdFifo, CMDFIFO_BUILD_PK1(5, SST_UNIT_TMU_0+j, taTcuColor));
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taTcuColor,   sst->TMU[j].taTcuColor );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taTcuAlpha,   sst->TMU[j].taTcuAlpha );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taCcuControl, sst->TMU[j].taCcuControl );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taCcuColor,   sst->TMU[j].taCcuColor );
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taCcuAlpha,   sst->TMU[j].taCcuAlpha );


		SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_TMU_0+j, taNPT));
		SETPD( cmdFifo, SST_TREX(ghw0,j)->taNPT, sst->TMU[j].taNPT );

	}

    sst->taControl =  numStages << SST_TA_NUM_TMUS_SHIFT;

	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, taControl ));
	SETPD( cmdFifo, ghw0->taControl, sst->taControl);
    
	SETMOP( cmdFifo, SST_MOP_FLUSH_PCACHE );

    UPDATE_HW_STATE (SC_SETUP_UNIT | SC_TMUGROUP_A | SC_TMUGROUP_B | SC_VTA_CONTROL);

    FIFO_EPILOG;
}

/*---------------------------------------------------------------
*
* Update Tmu Address
*---------------------------------------------------------------*/

void UpdateTmuAddr( NT9XDEVICEDATA *ppdev, TMURegs *sst, DWORD tmu,
    DWORD **pcmdFifo, DWORD *phwIndex)

{
	FIFO_PROLOG;

    CMDFIFO_CHECKROOM( cmdFifo,
        (PH4_SIZE+4 + PH1_SIZE+1) +
	 MOP_SIZE   +    // MOP
        0);

        SETMOP( cmdFifo, SST_MOP_FLUSH_PCACHE );
 	SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R5, SST_UNIT_TMU_0+tmu, taMode));
	SETPD( cmdFifo, SST_TREX(ghw0,tmu)->taMode, sst->TMU[tmu].taMode );
	SETPD( cmdFifo, SST_TREX(ghw0,tmu)->taLMS,  sst->TMU[tmu].taLMS );
	SETPD( cmdFifo, SST_TREX(ghw0,tmu)->taShiftBias, sst->TMU[tmu].taShiftBias );
	SETPD( cmdFifo, SST_TREX(ghw0,tmu)->taBaseAddr0, sst->TMU[tmu].taBaseAddr0 );

	SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, SST_UNIT_TMU_0+tmu, taNPT));
	SETPD( cmdFifo, SST_TREX(ghw0,tmu)->taNPT, sst->TMU[tmu].taNPT );

    UPDATE_HW_STATE (SC_TMUGROUP_A | SC_TMUGROUP_B);

	FIFO_EPILOG;  

}
/*
----------------------------------------------------------------------------
 Draw a rectangle
----------------------------------------------------------------------------
*/
void DrawRect(NT9XDEVICEDATA *ppdev, Vertex* vA, Vertex* vB, Vertex *vC , Vertex *vD,
    DWORD numStages,DWORD **pcmdFifo, DWORD *phwIndex)
{
    FxU32         tmui;

    FIFO_PROLOG;

    CMDFIFO_CHECKROOM( cmdFifo,  PH3_SIZE + 12 + 8 * (numStages+1));

	dwVtIndex= (++dwVtIndex)%maxVt[numStages];
    SETPH( cmdFifo,
  	  CMDFIFO_BUILD_PK3((dwVtIndex),((++dwVtIndex)%maxVt[numStages]),
      ((++dwVtIndex)%maxVt[numStages]),((++dwVtIndex)%maxVt[numStages]),
       SSTCP_PKT3_LOAD_A |
	   SSTCP_PKT3_LOAD_B|SSTCP_PKT3_LOAD_C|SSTCP_PKT3_LOAD_D,
		SSTCP_PKT3_TRIANGLE ,
    	SSTCP_PKT3_DRAW_ABC | SSTCP_PKT3_DRAW_CBD ));
     // This must be first vertex parameter for direct writes
     SETFPD( cmdFifo, ghw0->vpW, vA->w );

    // Pixel Offset macros allow comparison between sw and viewport
    // handling of d3d pixel centers.

    SETFPD( cmdFifo, ghw0->vpX, vA->x );
    SETFPD( cmdFifo, ghw0->vpY, vA->y );

    // We have to write out registers in 0..lastStage order.
    for( tmui=0; tmui <= numStages; tmui++)
    {
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, vA->tmuvtx[tmui].s );
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, vA->tmuvtx[tmui].t );
    }

#ifndef CMDFIFO    
    P6FENCE;
     {  //for second vertex
         SETPD( cmdFifo, ghw0->suMode, (1 << SST_SU_INDEX_SHIFT) | SST_SU_W );
     }
     P6FENCE;
#endif    

     //Send 2nd vertex    
           
//#ifndef CMDFIFO
     SETFPD( cmdFifo, ghw0->vpW, vB->w );
//#endif

    SETFPD( cmdFifo, ghw0->vpX, vB->x );
    SETFPD( cmdFifo, ghw0->vpY, vB->y );
    for( tmui=0; tmui <= numStages; tmui++)
    {
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, vB->tmuvtx[tmui].s );
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, vB->tmuvtx[tmui].t );
    }

#ifndef CMDFIFO    
    P6FENCE;
    {
        SETPD( cmdFifo, ghw0->suMode, (2 << SST_SU_INDEX_SHIFT) | SST_SU_W );
    }
    P6FENCE;
#endif

   // Send 3rd vertex
//#ifndef CMDFIFO    
    {
        // This must be first vertex parameter for direct writes
        SETFPD( cmdFifo, ghw0->vpW, vC->w );
    }
//#endif

    SETFPD( cmdFifo, ghw0->vpX, vC->x);
    SETFPD( cmdFifo, ghw0->vpY, vC->y);
    // We have to write out registers in 0..lastStage order.
    for( tmui=0; tmui <= numStages; tmui++)
    {
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, vC->tmuvtx[tmui].s );
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, vC->tmuvtx[tmui].t );
    }
#ifndef CMDFIFO    
    P6FENCE;
    { 
          SETPD( cmdFifo, ghw0->suDrawCmd, SST_SU_DRAWTRI
                       | (0<<SST_SU_VERTEX_A_SHIFT)
                       | (1<<SST_SU_VERTEX_B_SHIFT)
                       | (2<<SST_SU_VERTEX_C_SHIFT));
    } 
    P6FENCE;
#endif
   // Send 4th vertex
//#ifndef CMDFIFO    
    {
        // This must be first vertex parameter for direct writes
        SETFPD( cmdFifo, ghw0->vpW, vD->w );
    }
//#endif

    SETFPD( cmdFifo, ghw0->vpX, vD->x);
    SETFPD( cmdFifo, ghw0->vpY, vD->y);
    // We have to write out registers in 0..lastStage order.
    for( tmui=0; tmui <= numStages; tmui++)
    {
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpS, vD->tmuvtx[tmui].s );
       SETFPD( cmdFifo, SST_TREX(ghw0,tmui)->vpT, vD->tmuvtx[tmui].t );
    }
#ifndef CMDFIFO    
    P6FENCE;
    { 
          SETPD( cmdFifo, ghw0->suDrawCmd, SST_SU_DRAWTRI
                       | (0<<SST_SU_VERTEX_A_SHIFT)
                       | (1<<SST_SU_VERTEX_B_SHIFT)
                       | (2<<SST_SU_VERTEX_C_SHIFT));
    } 
    P6FENCE;
#endif

    UPDATE_HW_STATE (SC_ALL_CHANGED);  // can get more specific later.

    FIFO_EPILOG;
}

/*
----------------------------------------------------------------------------
Disable tmus
----------------------------------------------------------------------------
*/
void TurnOffTmus( NT9XDEVICEDATA *ppdev, TMURegs *sst, DWORD numStages,
    DWORD **pcmdFifo, DWORD *phwIndex)

{

  int j;
  FIFO_PROLOG;

     for( j=numStages; j>= 0; j-- ) 
     {

        CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE+4 + PH1_SIZE+4 + PH1_SIZE+5 + PH1_SIZE+ 2 );


        SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R5, SST_UNIT_TMU_0+j, taMode));
        SETPD( cmdFifo, SST_TREX(ghw0,j)->taMode, 0 );
        SETPD( cmdFifo, SST_TREX(ghw0,j)->taLMS,  0 );
        SETPD( cmdFifo, SST_TREX(ghw0,j)->taShiftBias, 0 );
        SETPD( cmdFifo, SST_TREX(ghw0,j)->taBaseAddr0, 0 );

    }
    //----------------------  
        
    SETMOP( cmdFifo, SST_MOP_FLUSH_PCACHE );

    UPDATE_HW_STATE (SC_TMUGROUP_A | SC_TMUGROUP_B);

	FIFO_EPILOG;  

}

/*
----------------------------------------------------------------------------
 Set NPT texture info
----------------------------------------------------------------------------
*/
void SetNPTSourceExt(DWORD  vtu, GrTexNPTInfoExt *texInfo,
        DWORD tileFlags , TMURegs *sst)
{
 DWORD maxS, maxT;
 DWORD baseAddress;

 if(tileFlags & MEM_IN_LINEAR)
     sst->TMU[vtu].taMode = (texInfo->format << SST_TA_FORMAT_SHIFT) |
                            SST_TA_EN_TEXTUREMAP|
                            SST_TA_CLAMPW | SST_TA_WNEG_LMSMAX |
                            (SST_TA_CLAMP << SST_TA_WRAP_S_SHIFT)|
                            (SST_TA_CLAMP << SST_TA_WRAP_T_SHIFT);
 else
     sst->TMU[vtu].taMode =  SST_TA_TEX_IS_TILED | SST_TA_EN_TEXTUREMAP|
                             (texInfo->format << SST_TA_FORMAT_SHIFT)|
                             SST_TA_CLAMPW | SST_TA_WNEG_LMSMAX |
                            (SST_TA_CLAMP << SST_TA_WRAP_S_SHIFT)|
                            (SST_TA_CLAMP << SST_TA_WRAP_T_SHIFT);
  if(texInfo->bFilter)
      sst->TMU[vtu].taMode |= ((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
                             (SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));


  sst->TMU[vtu].taLMS =
        ((11 << (SST_TA_LMS_FRACBITS + SST_TA_LMS_MIN_SHIFT)) |
         (11 << (SST_TA_LMS_FRACBITS + SST_TA_LMS_MAX_SHIFT)) |
         (0x00UL << SST_TA_LMS_BIAS_SHIFT)     |
         (0x00UL << SST_TA_LMS_LAR_SHIFT)      |
         (0x00UL << SST_TA_LMS_TSPLIT)         |
         (0x00UL << SST_TA_LMS_MBA_MODE_SHIFT) |
          SST_TA_EN_NPT);

  maxS = texInfo->maxS - 0x01UL,
  maxT = texInfo->maxT - 0x01UL;

  baseAddress = texInfo->baseAddr;


  if ( tileFlags & MEM_STAGGER )
      baseAddress |= SST_TA_TEX_STAGGERED;

  if ( tileFlags & MEM_IN_TILE1) {
      baseAddress |= SST_TA_NPT_TM;
    }

  sst->TMU[vtu].taBaseAddr0 = baseAddress;
  if(tileFlags & MEM_IN_LINEAR)
     sst->TMU[vtu].taNPT       =
                 (maxS << SST_TA_NPT_S_MAX_SHIFT) |
                 (maxT << SST_TA_NPT_T_MAX_SHIFT) |
                 (((texInfo->nptStride >> 4) -1) << SST_TA_NPT_S_STRIDE_SHIFT);
  else
     sst->TMU[vtu].taNPT       =
                 (maxS << SST_TA_NPT_S_MAX_SHIFT) |
                 (maxT << SST_TA_NPT_T_MAX_SHIFT) |
                 (((texInfo->nptStride >> 5) -1) << SST_TA_NPT_S_STRIDE_SHIFT);


//  sst->TMU[vtu].taShiftBias = 0;
//  sst->TMU[vtu].taCcuControl = 0x000;

}
/*------------------------------------------------------
* Change the stride of texture
*
*------------------------------------------------------*/
void ChangeNPTStride(DWORD  vtu, TMURegs *sst, BOOL fDouble)
{
  DWORD dwStride;

  dwStride = ((sst->TMU[vtu].taNPT & SST_TA_NPT_S_STRIDE )
        >> SST_TA_NPT_S_STRIDE_SHIFT) + 1;
  sst->TMU[vtu].taNPT &= ~ SST_TA_NPT_S_STRIDE;


 
  if(fDouble)
  {
       dwStride <<= 1;
  }
  else
  {
       dwStride >>=1;
  
  }               
  sst->TMU[vtu].taNPT       |=   (dwStride -1) << SST_TA_NPT_S_STRIDE_SHIFT;
}


/*
----------------------------------------------------------------------------
  ConfigBuffer()
  Setup Buffer depth and interleave mode
----------------------------------------------------------------------------
*/
void ConfigBuffer(NT9XDEVICEDATA *ppdev, DWORD dwMask,DWORD dwBytePerPixel,
    DWORD dwField,DWORD **pcmdFifo, DWORD *phwIndex )
{
    DWORD dwFbzMode, dwExMask;
    FIFO_PROLOG;
  
  
	CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE + 2 *(PH1_SIZE + 1));

//	SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE | SST_MOP_FLUSH_PCACHE |
//	  ((SST_MOP_STALL_3D_TA|SST_MOP_STALL_3D_TD|SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));

	SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_PCACHE |
	  ((SST_MOP_STALL_3D_PE) << SST_MOP_STALL_3D_SEL_SHIFT));


    if(dwBytePerPixel == 4)
        dwFbzMode = ( SST_PE_FB_32BPP << SST_PE_FB_FORMAT_SHIFT)|  
        ( SST_PE_LO_COPY << SST_PE_LOGIC_OP_SHIFT) | dwMask;
    else if(dwBytePerPixel == 2)
        dwFbzMode = ( SST_PE_FB_AI_88 << SST_PE_FB_FORMAT_SHIFT)|  
        ( SST_PE_LO_COPY << SST_PE_LOGIC_OP_SHIFT) | dwMask;
	else if(dwBytePerPixel == 3)   //a special case
	    dwFbzMode = ( SST_PE_FB_16BPP << SST_PE_FB_FORMAT_SHIFT)|  
        ( SST_PE_LO_COPY << SST_PE_LOGIC_OP_SHIFT) | dwMask;
    else    
     dwFbzMode = ( SST_PE_FB_8BPP << SST_PE_FB_FORMAT_SHIFT)|  
        ( SST_PE_LO_COPY << SST_PE_LOGIC_OP_SHIFT) | dwMask;

    if(dwField == EVEN_FIELD)
        dwExMask =  0x6318C;
    else if(dwField == ODD_FIELD)
        dwExMask =  0x18C63;
    else if(dwField == TOP_TWO)
        dwExMask =  0x7BC00;
    else if(dwField  == BOTTOM_TWO)
        dwExMask =  0x001EF;
    else
        dwExMask = 0;


	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peFbzMode) );
	SETPD( cmdFifo, ghw0->peFbzMode, dwFbzMode);
	SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, SST_UNIT_FBI, peExMask) );
	SETPD( cmdFifo, ghw0->peExMask, dwExMask);

    UPDATE_HW_STATE (SC_PE_SETUP);

    FIFO_EPILOG;
}



/*
----------------------------------------------------------------------------
void DownloadPalette(
    Down Load 256 entries of texture Palette
----------------------------------------------------------------------------
*/
void DownloadPalette( NT9XDEVICEDATA *ppdev, 
  LPPALETTEENTRY pal,DWORD **pcmdFifo, DWORD *phwIndex)
{
  ULONG burst, entry;
  ULONG line;
  FIFO_PROLOG;

  // try and burst 8 entry tables at a time (256/8=32)
  #define BURSTSIZE	8
  #define NUMBURSTS (256/BURSTSIZE)

  CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE );

  SETMOP( cmdFifo, SST_MOP_STALL_3D | SST_MOP_FLUSH_TCACHE |
        ((SST_MOP_STALL_3D_TA | SST_MOP_STALL_3D_PE ) << SST_MOP_STALL_3D_SEL_SHIFT));

  entry = 0;
  for( burst = 0; burst < NUMBURSTS; burst++ )
  {
    CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + BURSTSIZE );
	SETPH( cmdFifo, CMDFIFO_BUILD_PAL_PK1(BURSTSIZE, SST_UNIT_PALETTE, entry));
    
    for( line = 0; line < BURSTSIZE; line++ )
    {
      ULONG data =
        // R G B 888
        (pal[entry].peRed << 16) | (pal[entry].peGreen << 8) | (pal[entry].peBlue);

      SETPD( cmdFifo, *((volatile FxU32 *)(ghwTP) + entry), data );
      entry++;
    }
  }
    // palette is downloaded
  _D3(flags) &= ~PALETTECHANGED; 


  FIFO_EPILOG;

}

void TMULoad( FxU8 tmuNum, TMURegs *sst )
{
    //load current texture

   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//load alpha
	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);


    // just pass the output of TCU
	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

}


void TMUPass( FxU8 tmuNum ,TMURegs *sst)
{
   //pass previous texture

	TexCombineColorExt( tmuNum,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCC_CPREV,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

  	//pass TCU
  
	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

}



