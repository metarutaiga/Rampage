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
*/


#include <ddrawi.h>
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "geglobal.h"
#include "geucode.h"

#ifdef SLI
#include <ddsli2d.h>
#endif

//-----------------------------------------------------------------------------------
//
// geInit()
// 
//-----------------------------------------------------------------------------------

void __stdcall geInit(NT9XDEVICEDATA *ppdev)
{
    FxU32 i, j, n;
    CMDFIFO_PROLOG(cmdFifo);

    // Download the PPE uCode
    for(j=0; j<TC2_UCODE_LEN; j+=kPkt7wCountMask)
    {
        n = MIN(kPkt7wCountMask, TC2_UCODE_LEN - j);

        CMDFIFO_CHECKROOM( cmdFifo, PH7_SIZE+n);
        SETPH(cmdFifo, CMDFIFO_BUILD_PK7_w( n, kPpeMicroCodeAddr + j, 1, 0 ));
        for(i=0; i<n; i++)
            SETCF(cmdFifo, TC2_UCODE[j+i]);
    }

    CMDFIFO_EPILOG(cmdFifo);
}
