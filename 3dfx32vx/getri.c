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
**  8    Rampage   1.7         12/7/00  Dale  Kenaston  Sage triangle fill mode 
**       rendering functions. Implemented the fill mode triangle list rendering 
**       function.
**  7    Rampage   1.6         11/22/00 Dale  Kenaston  Sage packet 6b triangle 
**       list and strip/fan bug fix. Rewrote routines to use packet 6bs.
**  6    Rampage   1.5         11/22/00 Dale  Kenaston  Sage triangle fan 
**       primitive. Added vstart as a parameter to the rendering functions.
**  5    Rampage   1.4         11/14/00 Dale  Kenaston  Sage triangle list 
**       fixes. Fixed a bug in indexed triangle cases. Switched code to use 16 
**       bit indices.
**  4    Rampage   1.3         11/13/00 Brent Burton    Changed arg ordering and
**       added new args to render functions.  Added 16 and 32 bit index support 
**       for indexed prims.  Better DX7 to DX8 render function sharing.
**  3    Rampage   1.2         11/10/00 Dale  Kenaston  Sage AGP Vertex Buffer 
**       Fixes. Fix the unused variable warning message. Reduced the size of the
**       mesh buffer to 8 verts to match assumptions of the rest of the driver. 
**       Added support for indirect vertices.
**  2    Rampage   1.1         10/31/00 Brent Burton    Added DWORD vstride 
**       argument to rasterizer functions to replace FVFO_SIZE usage for DX8.
**  1    Rampage   1.0         10/12/00 Dale  Kenaston  
** $
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

#include "d3txtr2.h"                // txtrDesc


void __stdcall  geTriangleAllFill (RC *pRc, DWORD count, DWORD vertexType,
                                   VINDEX  idx,      DWORD istride,
                                   LPDWORD vertices, DWORD vstride,
                                   DWORD vstart)
{
    SETUP_PPDEV(pRc)
    int      iNext, iNext2, iNext3;
    LPDWORD  pNext, pNext2, pNext3;
    DWORD    i;

    //Initialize the indices and pointers
    TRI_INIT();

    //For each triangle
    for(i=0; i<count; i++)
    {
        //Get the next triangle
        TRI_NEXT();

        switch( pRc->fillMode )
        {
        case D3DFILL_WIREFRAME:
            //Draw the lines
            geLine( pRc, pNext, pNext, pNext2, vertexType );
            geLine( pRc, pNext, pNext2, pNext3, vertexType );
            geLine( pRc, pNext, pNext3, pNext, vertexType );
            break;

        case D3DFILL_POINT:
            //Draw the points
            gePoint( pRc, pNext, pNext, vertexType );
            gePoint( pRc, pNext, pNext2, vertexType );
            gePoint( pRc, pNext, pNext3, vertexType );
            break;

        default:
            D3DPRINT( 0, "Illegal D3D Fill Mode %d", pRc->fillMode );
            break;
        }
    }
}

#define TRI_PER_PACKET 8

void __stdcall  geTriangleAll (RC *pRc, DWORD count, DWORD vertexType,
                               VINDEX  idx,      DWORD istride,
                               LPDWORD vertices, DWORD vstride,
                               DWORD vstart)
{
    SETUP_PPDEV(pRc)
    int      iNext, iNext2, iNext3;
    LPDWORD  pNext, pNext2, pNext3;
    DWORD    i, j, k, n;
    DWORD    l, indices;
    LPWORD   pIndices = (LPWORD)&indices;

    CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
    return;
#endif

    HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D);

    //Initialize the indices and pointers
    TRI_INIT();

    //For each chunk of up to 8 triangles
    for( j=0; j<count; j+=TRI_PER_PACKET )
    {
        CMDFIFO_CHECKROOM( cmdFifo, PH6_SIZE+(FVFO_SIZE*TRI_PER_PACKET*3) );

        //Figure out the number of tris in this packet
        n = MIN(count-j, TRI_PER_PACKET);

        //Reset the 16bit index toggle
        l=0;

        //Send the packet header
        SETPH(cmdFifo,
              CMDFIFO_BUILD_PK6_b(0,
                                  kPkt6bWindCW,
                                  n*3,
                                  kPkt6bCmdTri,
                                  (pRc->GERC.bUseVertexBuffers ? 1 : 0)));

        //For each triangle in this packet
        for( i=0; i<n; i++ )
        {
            //Get the next triangle
            TRI_NEXT();

            //Check for a CFE bug, for now assert int1
            if((iNext==iNext2)||(iNext==iNext3)||(iNext2==iNext3))
                _asm int 1;

            //For each vertex in the triangle
            for( k=0; k<3; k++ )
            {
                //If we're using vertex buffers
                if( pRc->GERC.bUseVertexBuffers )
                {
                    //If the indices are 16 bits
                    if( istride == 2 )
                    {
                        //Put the index in the dword
                        pIndices[l] = (WORD)iNext;
                        //Toggle the current word
                        l ^= 0x00000001;
                        //If the word is full
                        if(!l)
                        {
                            SETCF(cmdFifo, indices);
                            indices = 0;
                        }
                    }
                    //If the indices are 32 bits
                    else
                    {
                        SETCF(cmdFifo, iNext);
                    }
                }
                //It we're not using vertex buffers
                else
                {
                    //Send the vertex data in the fifo
                    for( l=0; l<FVFO_SIZE; l++ )
                        SETCF(cmdFifo, pNext[l]);
                }

                //Advance the indices and pointers
                iNext = iNext2;
                iNext2 = iNext3;

                pNext = pNext2;
                pNext2 = pNext3;
            }
        }

        //If we have a word left over to flush
        if( pRc->GERC.bUseVertexBuffers && l )
            SETCF(cmdFifo, indices);
    }

    HW_ACCESS_EXIT(ACCESS_3D);

    CMDFIFO_EPILOG( cmdFifo ); 
}
