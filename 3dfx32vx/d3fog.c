/*   
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
**  9    3dfx      1.8         8/22/00  Brian Danielson Changed the renderstate 
**       management from being tied to the current render context to being a 
**       global concept, now accessable from DD.
**  8    3dfx      1.7         7/24/00  Brian Danielson Changes to implement 
**       renderstate and shadow register management.
**  7    3dfx      1.6         3/10/00  Miles Smith     Removed some redundant 
**       code to make the fog table generation simpler to understand
**  6    3dfx      1.5         12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  5    3dfx      1.4         10/19/99 Miles Smith     Added some tracking 
**       variables so that if 2 or more different apps are running with 
**       different fog table parameters the driver will switch back and forth 
**       between the tables as needed.
**  4    3dfx      1.3         10/14/99 Miles Smith     Fixing a problem with 
**       state changes and table Fog.
**  3    3dfx      1.2         10/1/99  Mark Einkauf    Complete HW_ACCESS 
**       macros work
**  2    3dfx      1.1         9/24/99  Miles Smith     Update for table fog. It
**       is basically correct now pending resolution of the w clamping issue. 24
**       bit z buffer support not enabled yet. Still looking at a bug in that.
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
** 31    9/03/99 3:05p Einkauf
** add fifo pointer to SETMOP parm list
** 
** 30    8/31/99 2:03p Einkauf
** DirectX CMD FIFO
** 
** 29    8/13/99 10:09a Mconrad
** Sync to CSIM DLL code base.
** 
** 28    8/10/99 10:33p Mconrad
** Fix warnings, convert SST2_* to SST_, make multimon happy.
**
** 28    5/28/99 11:07a Edwinh
** Fixes for vertex and table fog - both now work.  Can be TYPE_Q, TYPE_Z,
** TYPE_TABLE_ON_Q, and TYPE_TABLE_ON_Z (dependent on W vs. Z buffer and
** vertex vs. pixel fog).
**
** 
** 27    6/03/99 11:03p Peterm
** Made changes to get component to build with merged dd32 from h3 tot
** 
** 27    5/13/99 1:05a Mconrad
** Branch Prior to Retirement in users folder.
** 
*/


// uncomment the following to use the fifo for this file only
//#define CMDFIFO	// TEMP-ME until can enable in makefile


#include "precomp.h"

#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "ddglobal.h" 
#include "fxglobal.h" 
#include "fifomgr.h"
#include "math.h"
#include "d3contxt.h"

#ifdef REALLIBC
extern double fxPow(double x, double y);
#endif
extern int    float2int(float f);
extern double fxExp(double x);


#define FOG_TABLE_SIZE 80   
#define INDEXTOW_16( i ) (*(float*)&indexToW_16[i])
#define INDEXTOW_24( i ) (*(float*)&indexToW_24[i])
//#define INDEX_TO_W( i ) pow( 2.0f, 3.0f+(double)(i>>2)) / (8-(i&3))


static long indexToW_16[FOG_TABLE_SIZE] ={		// this one is 2^(3+(int)i/4)/(8-(i&3)) 
0x3f800000, 0x3f924925, 0x3faaaaab, 0x3fcccccd, 0x40000000, 0x40124925, 0x402aaaab, 0x404ccccd,
0x40800000, 0x40924925, 0x40aaaaab, 0x40cccccd, 0x41000000, 0x41124925, 0x412aaaab, 0x414ccccd,
0x41800000, 0x41924925, 0x41aaaaab ,0x41cccccd, 0x42000000, 0x42124925, 0x422aaaab, 0x424ccccd,
0x42800000, 0x42924925, 0x42aaaaab, 0x42cccccd, 0x43000000, 0x43124925, 0x432aaaab, 0x434ccccd,
0x43800000, 0x43924925, 0x43aaaaab, 0x43cccccd, 0x44000000, 0x44124925, 0x442aaaab, 0x444ccccd,
0x44800000, 0x44924925, 0x44aaaaab, 0x44cccccd, 0x45000000, 0x45124925, 0x452aaaab, 0x454ccccd,
0x45800000, 0x45924925, 0x45aaaaab, 0x45cccccd, 0x46000000, 0x46124925, 0x462aaaab, 0x464ccccd,
0x46800000, 0x46924925, 0x46aaaaab, 0x46cccccd, 0x47000000, 0x47124925, 0x472aaaab, 0x474ccccd,
0x47800000, 0x47924925, 0x47aaaaab, 0x47cccccd, 0x48000000, 0x47124925, 0x482aaaab, 0x484ccccd,
0x48800000, 0x48924925, 0x48aaaaab, 0x48cccccd, 0x49000000, 0x49124925, 0x492aaaab, 0x494ccccd }; 



/*
static long indexToW[FOG_TABLE_SIZE] = {	// this one is 2^(i/5)  this is for 16 bit z depth
0x3f800000, 0x3f93088c, 0x3fa8e5a3, 0x3fc20300, 0x3fdedc67,
0x40000000, 0x4013088c, 0x4028e5a3, 0x40420300, 0x405edc67,
0x40800000, 0x4093088c, 0x40a8e5a3, 0x40c20300, 0x40dedc67,
0x41000000, 0x4113088c, 0x4128e5a3, 0x41420300, 0x415edc67,
0x41800000, 0x4193088c, 0x41a8e5a3, 0x41c20300, 0x41dedc67,
0x42000000, 0x4213088c, 0x4228e5a3, 0x42420300, 0x425edc67,
0x42800000, 0x4293088c, 0x42a8e5a3, 0x42c20300, 0x42dedc67,
0x43000000, 0x4313088c, 0x4328e5a3, 0x43420300, 0x435edc67,
0x43800000, 0x4393088c, 0x43a8e5a3, 0x43c20300, 0x43dedc67,
0x44000000, 0x4413088c, 0x4428e5a3, 0x44420300, 0x445edc67,
0x44800000, 0x4493088c, 0x44a8e5a3, 0x44c20300, 0x44dedc67,
0x45000000, 0x4513088c, 0x4528e5a3, 0x45420300, 0x455edc67,
0x45800000, 0x4593088c, 0x45a8e5a3, 0x45c20300, 0x45dedc67,
0x46000000, 0x4613088c, 0x4628e5a3, 0x46420300, 0x465edc67,
0x46800000, 0x4693088c, 0x46a8e5a3, 0x46c20300, 0x46dedc67,
0x47000000, 0x4713088c, 0x4728e5a3, 0x47420300, 0x475edc67 }; 
*/

// I generated the 24 bit table with this:
// for ( i=0; i<80; i++ ) {
// 	oow = pow( 2.0, (double) -(	i/(10.0/3.0)));  // if i = 80, then exponent will equal 24, because 80/3.333 = 24
//	table[i] = 1/oow 
// }
static long indexToW_24[FOG_TABLE_SIZE] = { 	  // this is for 24 bit z depth
0x3F800000,  0x3F9D9624,  0x3FC20300,  0x3FEEDB40,  0x4013088C,  0x403504F3,  0x405EDC67,  0x40892FDF,  
0x40A8E5A3,  0x40CFEFC6,  0x41000000,  0x411D9624,  0x41420300,  0x416EDB40,  0x4193088C,  0x41B504F3,  
0x41DEDC67,  0x42092FDF,  0x4228E5A3,  0x424FEFC6,  0x42800000,  0x429D9624,  0x42C20300,  0x42EEDB40,  
0x4313088C,  0x433504F3,  0x435EDC67,  0x43892FDF,  0x43A8E5A3,  0x43CFEFC6,  0x44000000,  0x441D9624,  
0x44420300,  0x446EDB40,  0x4493088C,  0x44B504F3,  0x44DEDC67,  0x45092FDF,  0x4528E5A3,  0x454FEFC6,  
0x45800000,  0x459D9624,  0x45C20300,  0x45EEDB40,  0x4613088C,  0x463504F3,  0x465EDC67,  0x46892FDF,  
0x46A8E5A3,  0x46CFEFC6,  0x47000000,  0x471D9624,  0x47420300,  0x476EDB40,  0x4793088C,  0x47B504F3,  
0x47DEDC67,  0x48092FDF,  0x4828E5A3,  0x484FEFC6,  0x48800000,  0x489D9624,  0x48C20300,  0x48EEDB40,  
0x4913088C,  0x493504F3,  0x495EDC67,  0x49892FDF,  0x49A8E5A3,  0x49CFEFC6,  0x4A000000,  0x4A1D9624,  
0x4A420300,  0x4A6EDB40,  0x4A93088C,  0x4AB504F3,  0x4ADEDC67,  0x4B092FDF,  0x4B28E5A3,  0x4B4FEFC6
};



//------------------------------------------------------------------------
//
// Fogtable support.
//
//
// This driver will fill in the hardware fog table with blending factors
// computed based on the type (linear...). SST will use W to index into
// the table and compute f. Then f will be blended into the color.
// 
//
//------------------------------------------------------------------------


// this routine now generates all the tables instead of having a separate routine 
// for each fog equation.
void  _stdcall fogGenerateTable (unsigned char fogtable[FOG_TABLE_SIZE], RC *pRc)
{
	float f;
	float world_z;
	float nearZ;
	float farZ;
    float density;
	int i;


	nearZ = pRc->fogTableStart;	// these are always in world z units
	farZ  = pRc->fogTableEnd;	
	density = pRc->fogDensity;

	for (i=0;i<FOG_TABLE_SIZE;++i)
	{
		world_z = INDEXTOW_16(i);  
            
  	    switch(pRc->fogTableMode)
  	    {
    	    case  D3DFOG_LINEAR :
    		    f = 1 - (farZ - world_z) / (farZ - nearZ);
                break;
    	    case  D3DFOG_EXP :
 		        f = 1.0f - (float)fxExp(-density * world_z);
      		    break;
    	    case  D3DFOG_EXP2 :
		        f = 1.0f - (float)fxExp(-(density * world_z) * (density * world_z));
      		    break;
    	    default:
      		    D3DPRINT( 255, "WARNING: trying to set invalid fog type" ) ;
      		    return;
        }
        if (f > 1.0f)
		{
			f = 1.0f;
		}
		else if (f < 0.0f)
		{
			f = 0.0f;
		}

        f *= 255.0f;

		fogtable[i] = (unsigned char)float2int(f);
	}

}



//-----------------------
//
// download the fog table
//
//-----------------------
void __stdcall fogTable(NT9XDEVICEDATA *ppdev, unsigned char peFogTable[FOG_TABLE_SIZE] )
{
    int i;
    unsigned char *locTable = &peFogTable[0];
    CMDFIFO_PROLOG(cmdFifo);

    HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D);

    CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE + PH1_SIZE + (FOG_TABLE_SIZE / 4) );

    // Must MOP before fog table download
    SETMOP( cmdFifo, (SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT) |
        SST_MOP_FLUSH_PCACHE | SST_MOP_STALL_3D );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1((FOG_TABLE_SIZE / 4), SST_UNIT_FBI, peFogTable));

    for ( i = 0; i < (FOG_TABLE_SIZE / 4); i++ )
    {
		
       	unsigned long e0, e1, e2, e3;

    	e0 = locTable[0];                     /* lower entry */
    	e1 = locTable[1];                     /* upper entry */
    	e2 = locTable[2];                     /* lower entry */
    	e3 = locTable[3];                     /* upper entry */

    	SETPD( cmdFifo, ghw0->peFogTable[i], (e3 << 24) | (e2 << 16) | (e1 << 8) | e0 );
    	locTable += 4;
    }

    HW_ACCESS_EXIT(ACCESS_2D);

    CMDFIFO_EPILOG( cmdFifo );

} 

void __stdcall setFogMode(RC *pRc)
{
    SETUP_PPDEV(pRc)
  	if( pRc->state & STATE_REQUIRES_WBUFFER )
  		pRc->state &= ~(STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG);
  	else
  	{
    	pRc->state &= ~(STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG | STATE_REQUIRES_W_FBI);
    	pRc->sst.suMode.vFxU32 &= ~(SST_SU_Q);   // was ~(SST_SETUP_Wfbi)
  	}
  
  	pRc->sst.peFogMode.vFxU32 &= ~(SST_PE_EN_FOGGING | 
                                   SST_PE_FOG_ADD    | SST_PE_FOG_MULT |
                                   SST_PE_FOG_TYPE   | 
                                   SST_PE_FOG_DITHER | SST_PE_FOG_INVERT );
  
    if (pRc->fogEnable == TRUE)
    {
	    // Application is attempting to fog using the fog per vertex software
	    // method. We have three options: iterated alpha, interated z, or 
	    // iterated Q. Using iterated alpha implies that if the applic. is using 
	    // both alpha blending and Z buffering then you're in trouble. 
	    //
	    // Cout = Iteratedfog*Cfog + (1-Iteratedfog)Cin
	    if (pRc->fogTableMode == D3DFOG_NONE)	// vertex fog
	    {
		  	if (pRc->state & STATE_REQUIRES_WBUFFER)	// Q holds depth, Z holds fog iterator
				pRc->sst.peFogMode.vFxU32 = SST_PE_EN_FOGGING | SST_PE_FOG_DITHER |
								           (SST_PE_FOG_TYPE_Z << SST_PE_FOG_TYPE_SHIFT);
		  	else		// Z holds depth, Q holds fog iterator			
			  	pRc->sst.peFogMode.vFxU32 = SST_PE_EN_FOGGING | SST_PE_FOG_DITHER |
								           (SST_PE_FOG_TYPE_Q << SST_PE_FOG_TYPE_SHIFT);
	      	pRc->state |= (STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_W_FBI);
	      	pRc->sst.suMode.vFxU32 |= SST_SU_Q;   // was SST_SETUP_Wfbi
	    }
	    else	// pixel fog								
	    // user is trying to use the hardware fog table
	    // Only set fog enable because fogadd and fogmult must be zero in 
	    // order to have Cout = Afog*Cfog  +  (1-Afog)*Cin
	    {
		    // Since the hw alway iterates off of Q  - I'm forcing the mode to type Q if pixel fog is selected - mls
	  	    pRc->sst.peFogMode.vFxU32 = SST_PE_EN_FOGGING | SST_PE_FOG_DITHER |
				                       (SST_PE_FOG_TYPE_Q_TABLE << SST_PE_FOG_TYPE_SHIFT);

	      	pRc->state |= (STATE_REQUIRES_HWFOG | STATE_REQUIRES_W_FBI);
	      	pRc->sst.suMode.vFxU32 |= SST_SU_Q;     // was SST_SETUP_Wfbi;
	    }
	 }

    // Update state change for shadows changed in thie routine
    UPDATE_HW_STATE( reg3D.suMode.group | reg3D.peFogMode.group );
}

//-----------------------------
//
// Create the fog table entries
//
//-----------------------------
void __stdcall createTableAndLoad(RC *pRc)
{
  	SETUP_PPDEV(pRc)
  	unsigned char localFogTable[FOG_TABLE_SIZE];

    fogGenerateTable( localFogTable, pRc );

  	fogTable(ppdev, localFogTable);

    _D3(currentFogTableMode)    = pRc->fogTableMode;
    _D3(currentFogTableStart)   = pRc->fogTableStart;
    _D3(currentFogTableEnd)     = pRc->fogTableEnd;
    _D3(currentFogTableDensity) = pRc->fogDensity;
}

void __stdcall fogColor(RC *pRc, ULONG state)
{
    SETUP_PPDEV(pRc)
  	pRc->fogColor              = (D3DCOLOR) state;
  	pRc->sst.peFogColor.vFxU32 = (ULONG)( (RGBA_GETRED(pRc->fogColor)   	<< 16)    
                                        | (RGBA_GETGREEN(pRc->fogColor) 		<< 8 )     
                                        | (RGBA_GETBLUE(pRc->fogColor)       )); 
    // Update state change for shadows changed in thie routine
    UPDATE_HW_STATE( reg3D.peFogColor.group );
}

void __stdcall fogEnable(RC *pRc, ULONG state)
{
  	pRc->fogEnable = state;
  	setFogMode(pRc);           
}

void __stdcall fogTableMode(RC *pRc, ULONG state)
{
  	pRc->fogTableMode  = state;
  	setFogMode(pRc);           
}
        
void __stdcall fogTableStart(RC *pRc, ULONG state)
{
	// this is used by linear fog mode
	// In theory it could range from 0 to max z range ( ie 65535 for 16bit z mode)
	// a typical value would be something between .5 and 10
  	pRc->fogTableStart = *(float *)&state;
}     

void __stdcall fogTableEnd(RC *pRc, ULONG state)
{
	// this is used by linear fog mode
	// In theory it could range from 0 to max z range ( ie 65535 for 16bit z mode)
	// a typical value would be something between 10 and 100
  	pRc->fogTableEnd   = *(float *)&state;
}
               
void __stdcall fogDensity(RC *pRc, ULONG state)
{            
	// this is only used by exponential fog modes
	// It should be a float value between 0 and 1.0
  	pRc->fogDensity    = *(float *)&state;
}
