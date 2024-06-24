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
**  3    Rampage   1.2         12/5/00  Dale  Kenaston  Sage points, lines and 
**       microcode fixes. Implemented the point primitive routine.
**  2    Rampage   1.1         10/13/00 Brent Burton    Added ddrawi.h to list 
**       of include files for DX8 build compatibility.  No code changes.
**  1    Rampage   1.0         10/6/00  Dale  Kenaston  
** $
*/

#include <ddrawi.h>
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d6fvf.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "d3txtr2.h"                // txtrDesc


void __stdcall gePoint( RC *pRc, LPDWORD pF, LPDWORD pA, DWORD vertexType )
{
    SETUP_PPDEV(pRc)
    DWORD       i;

    CMDFIFO_PROLOG(cmdFifo);

#if( PERFTEST == TRI_NULL )
    return;
#endif

    HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);

    CMDFIFO_CHECKROOM( cmdFifo, PH6_SIZE+FVFO_SIZE );

    //Send the packet header
    SETPH(cmdFifo,
          CMDFIFO_BUILD_PK6_a(0, 0, 0,
                              1, 0, 0,
                              kPkt6aCmdDrawA,
                              0));

    //Send vertex A
    for(i=0; i<FVFO_SIZE; i++)
        SETCF(cmdFifo, pA[i]);

    HW_ACCESS_EXIT(ACCESS_3D);

    CMDFIFO_EPILOG( cmdFifo );
} 
