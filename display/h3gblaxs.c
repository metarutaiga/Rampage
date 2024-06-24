/* -*-c++-*- */
/* $Header: h3gblaxs.c, 1, 9/11/99 10:14:47 PM PDT, StarTeam VTS Administrator$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   h3gblaxs.c
**
** Description: The GETGBL_dwOvlOffset function.
**
** $Revision: 1$
** $Date: 9/11/99 10:14:47 PM PDT$
**
** $History: h3gblaxs.c $
** 
** *****************  Version 1  *****************
** User: Peterm       Date: 5/18/99    Time: 1:54p
** Created in $/devel/sst2/Win95/dx/hostvdd
** initial sst2 hostvdd checkin of v3 minivdd file
** 
** *****************  Version 2  *****************
** User: Michael      Date: 1/07/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
**
*/
#include "thunk32.h"


/*----------------------------------------------------------------------
Function name:  GETGBL_dwOvlOffset

Description:    Get the lpDriverData->dwOvlOffset from GLOBALDATA.
                
Information:

Return:         DWORD   Return lpDriverData->dwOvlOffset
----------------------------------------------------------------------*/
DWORD GETGBL_dwOvlOffset(DWORD lpDriverData)
{
   return( ((GLOBALDATA *)lpDriverData)->dwOvlOffset );
}
