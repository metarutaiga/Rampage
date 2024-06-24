/* $Header: fxmmcfg.c, 2, 5/3/00 3:39:05 PM PDT, Ryan Bissell$ */
/*
** Copyright (c) 1996-2000, 3dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   fxmmcfg.c
**
** Description: Contains the OS-specific and HW-specific portions of the
**              3dfx heap management MRI.
**
** $Log: 
**  2    Rampage   1.1         5/3/00   Ryan Bissell    Continued deployment of 
**       Central Services and related changes.
**  1    Rampage   1.0         4/14/00  Ryan Bissell    
** $
**
*/


#include "precomp.h"
#include <sst2glob.h>                //Must be included into sst2.h
#include "fxmm.h"


void* fxmm_cfg_mallocfacility(GLOBALDATA*  ppdev, U032 size)
{
  return DXMALLOCZ(size);
}


void fxmm_cfg_freefacility(GLOBALDATA* ppdev, void* ptr)
{
  DXFREE(ptr);
}



