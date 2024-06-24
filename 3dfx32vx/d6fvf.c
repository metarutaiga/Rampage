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
**  13   3dfx      1.12        11/16/00 Brent Burton    Added D3DFVF_PSIZE 
**       support for DX8 point sprites.
**  12   3dfx      1.11        10/28/00 Evan Leland     Sage changes: fix vertex
**       toggle flag that tells Sage code when to become active based on the 
**       vertex type.
**  11   3dfx      1.10        10/20/00 Dale  Kenaston  Viewport setup for Sage,
**       etc. Fixed the code that sets RC.bVtxGeStateToggle.
**  10   3dfx      1.9         10/11/00 Brent Burton    DX8 code integration.  
**       Changes to headers, new code.
**  9    3dfx      1.8         10/10/00 Evan Leland     support for tracking 
**       vertex transitions from tl to non-tl vertices
**  8    3dfx      1.7         9/27/00  Evan Leland     Added new routine 
**       setFVFOffsetTable() which encapsulates changes to the fvf offset table.
**  7    3dfx      1.6         8/28/00  Evan Leland     modified fvf offset 
**       table to include pre tnl vertex parameters
**  6    3dfx      1.5         6/15/00  Michel Conrad   Delete really old 
**       check-in comments. Delete dead code.
**  5    3dfx      1.4         5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  4    3dfx      1.3         2/28/00  Brian Danielson Fixed FvFOffsetTable 
**       Contents and FVF work for DX7.
**  3    3dfx      1.2         12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
*/
//------------------------------------------------------------------------------

#include <ddrawi.h>
#include <d3dhal.h>
#include "d3global.h"
#include "d6fvf.h"
#include "d3contxt.h"

#define DV  FVF_VALUE_NOT_PRESENT    // Default table entry value for "no vertex value present"

FVFOFFSETTABLE fvfOffsetTable[12] = {
  //size, x,  y,  z,  w, b1, b2, b3, b4, b5,  d,  s, nx, ny, nz,    T0           T1            T2            T3            T4            T5            T6            T7          psize
  //                                                               u  v  w  q
  // VERTEX format
    { 8,  0,  1,  2, DV, DV, DV, DV, DV, DV, DV, DV,  3,  4,  5, { 6, 7,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // LVERTEX
    { 8,  0,  1,  2, DV, DV, DV, DV, DV, DV,  4,  5, DV, DV, DV, { 6, 7,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},  3},
  // TLVERTEX
    { 8,  0,  1,  2,  3, DV, DV, DV, DV, DV,  4,  5, DV, DV, DV, { 6, 7,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWDT1
    { 7,  0,  1,  2,  3, DV, DV, DV, DV, DV,  4, DV, DV, DV, DV, { 5, 6,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWDT2
    { 9,  0,  1,  2,  3, DV, DV, DV, DV, DV,  4, DV, DV, DV, DV, { 5, 6,DV,DV},{7, 8, DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWDST2
    {10,  0,  1,  2,  3, DV, DV, DV, DV, DV,  4,  5, DV, DV, DV, { 6, 7,DV,DV},{8, 9, DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWST2
    { 9,  0,  1,  2,  3, DV, DV, DV, DV, DV, DV,  4, DV, DV, DV, { 5, 6,DV,DV},{7, 8, DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWDS
    { 6,  0,  1,  2,  3, DV, DV, DV, DV, DV,  4,  5, DV, DV, DV, { 0, 0,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWT2
    { 8,  0,  1,  2,  3, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, { 4, 5,DV,DV},{6, 7, DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWD
    { 5,  0,  1,  2,  3, DV, DV, DV, DV, DV,  4, DV, DV, DV, DV, { 0, 0,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // XYZWT1
    { 6,  0,  1,  2,  3, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, { 4, 5,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
  // CUSTOM
    { 0, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, DV, {DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV},{DV,DV,DV,DV}, DV},
};

#undef DV

DWORD setFVFOffsetTable(DWORD fvf, RC *pRc, DWORD *fvfotVertexType)
{
  DWORD position_bits, dwDimensionFormat, dwTexCoordIndex;
  DWORD size, vertexType, i;

  SETUP_PPDEV(pRc)

  // if the vertex type is not changing we assume we don't need to do anything
  if ( fvf == pRc->current_FVF ) {
    *fvfotVertexType = pRc->fvfVertexType;
    pRc->bVtxGeStateToggle = FVF_NO_CHANGE;
    return D3D_OK;
  }

  // set flags if the fvf goes from TL to non-TL or vice versa
  {
  DWORD old_fvf = pRc->current_FVF;
  DWORD new_fvf = fvf;

  //
  // _   = GE_FVF_NOTL(fvf)
  // L   = GE_FVF_LIT(fvf) == 1
  // TNL = GE_FVF_TNL(fvf) == 1
  //
  // N   = FVF_NO_CHANGE
  // TL  = FVF_CHANGE_TO_TL_VERT
  // NTL = FVF_CHANGE_TO_NON_TL_VERT
  //
  //            New FVF
  //
  //   O       |  _  | L   | TNL
  //   l  -----------------------
  //   d    _  |  N  |  N  |  TL
  //      -----------------------
  //   F    L  |  N  |  N  |  TL
  //   V  -----------------------
  //   F   TNL | NTL | NTL |  N
  //
  pRc->bVtxGeStateToggle = FVF_NO_CHANGE;

  if ( old_fvf == 0 )
  {
      if ( GE_FVF_TNL( new_fvf ) )
          pRc->bVtxGeStateToggle = FVF_CHANGE_TO_TL_VERT;
      else
          pRc->bVtxGeStateToggle = FVF_CHANGE_TO_NON_TL_VERT;
  }
  else if ( GE_FVF_NOTL( old_fvf ) )
  {
      if ( GE_FVF_TNL( new_fvf ) )
          pRc->bVtxGeStateToggle = FVF_CHANGE_TO_TL_VERT;
  }
  else
  {
      if ( GE_FVF_NOTL( new_fvf ) )
          pRc->bVtxGeStateToggle = FVF_CHANGE_TO_NON_TL_VERT;
  }
  }

  switch( fvf ) {
    case D3DFVF_VERTEX:
      vertexType = FVFOT_VERTEX;
      break;
    case D3DFVF_LVERTEX:
      vertexType = FVFOT_LVERTEX;
      break;
    case D3DFVF_TLVERTEX :
      vertexType = FVFOT_TLVERTEX;
      break;
    case 0x44:
      vertexType = FVFOT_XYZWD;
      break;
    case 0x144 :
      vertexType = FVFOT_XYZWDT1;
      break;
    case 0x244 :
      vertexType = FVFOT_XYZWDT2;
      break;
    case 0x2C4 :
      vertexType = FVFOT_XYZWDST2;
      break;
    case 0x284 :
      vertexType = FVFOT_XYZWST2;
      break;
    case 0xC4 :
      vertexType = FVFOT_XYZWDS;
      break;
    case 0x104 :
      vertexType = FVFOT_XYZWT1;
      break;
    case 0x204 :
      vertexType = FVFOT_XYZWT2;
      break;
    default :
      // We need to create a custom table entry based on the FVF flags
      D3DPRINT( 0, "  <WARNING> Generating a new FVF table entry" );
            
      vertexType = FVFOT_CUSTOM;
      position_bits = fvf & D3DFVF_POSITION_MASK;
      size = 0;

      // if no vertex blending
      if ((position_bits != D3DFVF_XYZB1) && (position_bits != D3DFVF_XYZB2) &&
          (position_bits != D3DFVF_XYZB3) && (position_bits != D3DFVF_XYZB4) &&
          (position_bits != D3DFVF_XYZB5))
      {
        // position still is not guaranteed to be present
        if (position_bits & D3DFVF_XYZ) {
          FVFO_HX = size++;
          FVFO_HY = size++;
          FVFO_HZ = size++;
          FVFO_RHW = FVF_VALUE_NOT_PRESENT;
        }
        else if (position_bits & D3DFVF_XYZRHW) {
          FVFO_SX = size++;
          FVFO_SY = size++;
          FVFO_SZ = size++;
          FVFO_RHW = size++;
        }
      }
      // else vertex blending is ON, encode untransformed position + blend weights
      else {
        FVFO_HX = size++;
        FVFO_HY = size++;
        FVFO_HZ = size++;
        FVFO_RHW = FVF_VALUE_NOT_PRESENT;

        FVFO_B1 = FVF_VALUE_NOT_PRESENT;
        FVFO_B2 = FVF_VALUE_NOT_PRESENT;
        FVFO_B3 = FVF_VALUE_NOT_PRESENT;
        FVFO_B4 = FVF_VALUE_NOT_PRESENT;
        FVFO_B5 = FVF_VALUE_NOT_PRESENT;

        if (position_bits == D3DFVF_XYZB1) {
          FVFO_B1 = size++;
        }
        else if (position_bits == D3DFVF_XYZB2) {
          FVFO_B1 = size++;
          FVFO_B2 = size++;
        }
        else if (position_bits == D3DFVF_XYZB3) {
          FVFO_B1 = size++;
          FVFO_B2 = size++;
          FVFO_B3 = size++;
        }
        else if (position_bits == D3DFVF_XYZB4) {
          FVFO_B1 = size++;
          FVFO_B2 = size++;
          FVFO_B3 = size++;
          FVFO_B4 = size++;
        }
        else if (position_bits == D3DFVF_XYZB5) {
          // actually dx7 only allows up to 4 vertex blend weights
          FVFO_B1 = size++;
          FVFO_B2 = size++;
          FVFO_B3 = size++;
          FVFO_B4 = size++;
          FVFO_B5 = size++;
        }
      }

      if( fvf & D3DFVF_NORMAL ) {
        FVFO_NX = size++;
        FVFO_NY = size++;
        FVFO_NZ = size++;
      }
      else {
        FVFO_NX = FVF_VALUE_NOT_PRESENT;
        FVFO_NY = FVF_VALUE_NOT_PRESENT;
        FVFO_NZ = FVF_VALUE_NOT_PRESENT;
      }

#if (DIRECT3D_VERSION >= 0x0800)
      if (fvf & D3DFVF_PSIZE)
          FVFO_PSIZE = size++;
#else  // dx < 8
      if (fvf & D3DFVF_RESERVED1)
          FVFO_PSIZE = size++;
#endif // dx < 8

      if( fvf & D3DFVF_DIFFUSE )
        FVFO_COLOR = size++;
      else
        FVFO_COLOR = FVF_VALUE_NOT_PRESENT;

      if( fvf & D3DFVF_SPECULAR )
        FVFO_SPECULAR = size++;
      else
        FVFO_SPECULAR = FVF_VALUE_NOT_PRESENT;

#define TEXTURE_COORDSET_MASK   0xFFFF0000

      dwTexCoordIndex = 0;
      dwDimensionFormat = (fvf & TEXTURE_COORDSET_MASK) >> 16;     // Isolate the texture coordinate dimensionality
      for( i = 0; i < ((fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT); i++ )
      {
          DWORD dwCoordDimension;
          BOOL  bUnsupported;    // Is this texture coordinate supported in our hardware?
                                 // We support 2, 3 (projected or cube) coordinate dimensions.
                                 // Dimensions 1 and 4 should map to 2 and 3 respectively.

          dwCoordDimension    = dwDimensionFormat & 0x03;
          dwDimensionFormat >>= 2;

          switch ( dwCoordDimension ) // Convert to number of texture coords in this stage's FVF structure
          {
            case 2 :  dwCoordDimension = 4;  bUnsupported = TRUE;   break;  // four  floating point coordinates : u,v,q,r
            case 1 :  dwCoordDimension = 3;  bUnsupported = FALSE;  break;  // three floating point coordinates : u,v,q
            case 0 :  dwCoordDimension = 2;  bUnsupported = FALSE;  break;  // two   floating point coordinates : u,v
            case 3 :  dwCoordDimension = 1;  bUnsupported = TRUE;   break;  // one   floating point coordinates : u
          }
          // MC:XXX This should shunt case 1 or 4 into some form of 2 or 3 that we support.
          // ------------------------------------------------------------------------------------------------------------
          //    Following notes from Brian's and Michels emails (7/31/2000) on the subject of how to implement the remaining cases :
          //    (types in table below are : # floating point coords and whether last is projected or not)
          //
          // Type 1  - using 2d texture hardware, fix second hw coordinate to zero?
          // Type 2  - already implemented
          // Type 2P - a 1d projected texture, fix the second hw coordintate to zero? What if  s=nozero, t=zero, q=zero?
          // Type 3  - cube mapping
          // Type 3P - this is the projected texture already implemented
          // Type 4  - Just use two coords - ignore the others
          // Type 4P - This one could be one of couple things we don't suppport:
          //            a.) a 3 coordinate projected texture ==> ignore last or next to last coordinate
          //            b.) a projected cubemap ==> ingnore last coordinate
          // -------------------------------------------------------------------------------------------------------------

          if ( bUnsupported )
            return D3DERR_INVALIDVERTEXFORMAT;

#ifdef CUBEMAP
          pRc->tCoordDimension[i] = dwCoordDimension; 
#endif
          switch ( dwCoordDimension ) // Fill in the offset table
          {
            case 4 :    *((LPDWORD)&FVFO_TU(0) + (i << 2) ) = size++;
                        *((LPDWORD)&FVFO_TV(0) + (i << 2) ) = size++;
                        *((LPDWORD)&FVFO_TW(0) + (i << 2) ) = size++;
                        *((LPDWORD)&FVFO_TQ(0) + (i << 2) ) = size++;
                        pRc->tCoordDimensionOffset[i] = dwTexCoordIndex;
                        dwTexCoordIndex += 4;
                        break;
            case 3 :    *((LPDWORD)&FVFO_TU(0) + (i << 2) ) = size++;
                        *((LPDWORD)&FVFO_TV(0) + (i << 2) ) = size++;
                        *((LPDWORD)&FVFO_TW(0) + (i << 2) ) = size++;
                        pRc->tCoordDimensionOffset[i] = dwTexCoordIndex;
                        dwTexCoordIndex += 3;
                        break;
            case 2 :    *((LPDWORD)&FVFO_TU(0) + (i << 2) ) = size++;
                        *((LPDWORD)&FVFO_TV(0) + (i << 2) ) = size++;
                        pRc->tCoordDimensionOffset[i] = dwTexCoordIndex;
                        dwTexCoordIndex += 2;
                        break;
            case 1 :    *((LPDWORD)&FVFO_TU(0) + (i << 2) ) = size++;
                        pRc->tCoordDimensionOffset[i] = dwTexCoordIndex;
                        dwTexCoordIndex += 1;
                        break;
          }
      }
      FVFO_SIZE = size;      
      break;            
  }

  pRc->current_FVF = fvf;           // store the new fvf
  pRc->fvfVertexType = vertexType;  // Save this type - needed for processing texture coordinate dimensionality

  if ( FVFO_COLOR == FVF_VALUE_NOT_PRESENT )
    pRc->bFVFDiffusePresent = FALSE;
  else
    pRc->bFVFDiffusePresent = TRUE;

  if ( FVFO_SPECULAR == FVF_VALUE_NOT_PRESENT )
    pRc->bFVFSpecularPresent = FALSE;
  else
    pRc->bFVFSpecularPresent = TRUE;

  // return the fvf offset table vertex type, an index into the fvf offset table
  UPDATE_HW_STATE( SC_FVF );  // should only affect Sage but make it a Rampage state change
  *fvfotVertexType = vertexType;
  return D3D_OK;
}
