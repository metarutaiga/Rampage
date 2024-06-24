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
**  3    Rampage   1.2         12/7/00  Dale  Kenaston  Sage triangle fill mode 
**       rendering functions. Implemented the fill mode fan rendering function. 
**       Added locals to the solid fill function to support the macro changes.
**  2    Rampage   1.1         11/22/00 Dale  Kenaston  Sage packet 6b triangle 
**       list and strip/fan bug fix. Bug fix for VB 16bit indicies with an odd 
**       vertex count.
**  1    Rampage   1.0         11/22/00 Dale  Kenaston  
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



void __stdcall geFanAllFill( RC *pRc, DWORD count, DWORD vertexType,
                             VINDEX idx,       DWORD istride,
                             LPDWORD vertices, DWORD vstride,
                             DWORD vstart )
{
    SETUP_PPDEV(pRc)
    int     iRoot=-3, iPrev=-2, iNext=-1;
    LPDWORD pRoot,    pPrev,    pNext;
    DWORD   i=0, j=0;
    DWORD   numVerts;

    numVerts = count+2;

    //Initialize the indices
    VTX_INIT();

    //Get the first two vertices
    VTX_NEXT();
    i++;
    VTX_NEXT();
    i++;

    //For each vertex - 2
    for(; i<numVerts; i++)
    {
        //Get the next vertex
        VTX_NEXT();

        switch( pRc->fillMode )
        {
        case D3DFILL_WIREFRAME:
            //Draw the lines
            geLine( pRc, pRoot, pRoot, pPrev, vertexType );
            geLine( pRc, pRoot, pPrev, pNext, vertexType );
            geLine( pRc, pRoot, pNext, pRoot, vertexType );
            break;

        case D3DFILL_POINT:
            //Draw the points
            gePoint( pRc, pRoot, pRoot, vertexType );
            gePoint( pRc, pRoot, pPrev, vertexType );
            gePoint( pRc, pRoot, pNext, vertexType );
            break;

        default:
            D3DPRINT( 0, "Illegal D3D Fill Mode %d", pRc->fillMode );
            break;
        }
    }
}

#define VERTEX_PER_PACKET 16
#define PACKET_MASK       (VERTEX_PER_PACKET-1)

void __stdcall geFanAll( RC *pRc, DWORD count, DWORD vertexType,
                         VINDEX idx,       DWORD istride,
                         LPDWORD vertices, DWORD vstride,
                         DWORD vstart )
{
    SETUP_PPDEV(pRc)
    int     iRoot=-3, iPrev=-2, iNext=-1;
    LPDWORD pRoot,    pPrev,    pNext;
    DWORD   i, j, n;
    DWORD   k, indices;
    LPWORD  pIndices = (LPWORD)&indices;
    DWORD   numVerts;

    CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
    return;
#endif

    HW_ACCESS_ENTRY(cmdFifo,ACCESS_3D);

    numVerts = count+2;

    //Initialize the indices
    VTX_INIT();

    //For each chunk of up to 16 vertices
    for( j=0; j<numVerts; j+=VERTEX_PER_PACKET )
    {
        CMDFIFO_CHECKROOM( cmdFifo, PH6_SIZE+(FVFO_SIZE*VERTEX_PER_PACKET));

        //Figure out the number of verts in this packet
        n = MIN(numVerts-j, VERTEX_PER_PACKET);

        //Reset the 16bit index toggle
        k = 0;

        //Send the packet header
        SETPH(cmdFifo,
              CMDFIFO_BUILD_PK6_b(0,
                                  kPkt6bWindCW,
                                  n,
                                  (j==0 ? kPkt6bCmdTFan : kPkt6bCmdTFanC),
                                  pRc->GERC.bUseVertexBuffers ? 1 : 0));

        //For each vertex in this packet
        for(i=0; i<n; i++)
        {
            //Get the next vertex
            VTX_NEXT();

            //Check for a CFE bug, for now assert int1
            if((iNext==iPrev)||(iNext==iRoot)||(iPrev==iRoot))
                _asm int 1;

            //If we're using vertex buffers
            if( pRc->GERC.bUseVertexBuffers )
            {
                //If the indices are 16 bits
                if(istride == 2)
                {
                    //Put the index in the dword
                    pIndices[k] = iNext;
                    //Toggle the current word
                    k ^= 0x00000001;
                    //If the word is full
                    if(!k)
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
            //If we're not using vertex buffers
            else
            {
                //Send the vertex data in the fifo
				for(k=0; k<FVFO_SIZE; k++)
					SETCF(cmdFifo, pNext[k]);
            }
        }

        //If we have a word left over to flush
        if( pRc->GERC.bUseVertexBuffers && k )
            SETCF(cmdFifo, indices);
    }

    HW_ACCESS_EXIT(ACCESS_3D);

    CMDFIFO_EPILOG( cmdFifo );
}
