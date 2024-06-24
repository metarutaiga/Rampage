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
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
** 5     6/03/99 11:13p Peterm
** changed to enable dd32 to build with h3 tot deltas
** 
** 5     5/13/99 1:54a Mconrad
** Branch Prior to Retirement in users folder.
** 
** 3     5/06/98 6:10p Adrians
** Changes for DX6 into DX5 driver.
** 
** 1     4/29/98 6:31p Adrians
** Created
 * 
 * 2     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#include <windows.h>

int float2int(float f)
{      
  int  expnt, mantissa;
  BOOL neg = FALSE;
  mantissa = expnt = *(int *)&f;
  
//  D3DPRINT( 255,"float2Int float=0x%x ",*(int *)&f );
 
  // special case
  if (mantissa == 0)
    return 0;
  
  if (expnt & 0x80000000)
     neg = TRUE;
  
  // remove the exponent and add in implicit one 
  mantissa &= 0x007fffff;
  mantissa |= 0x00800000;
       
  // exponent - remove mantissa and adjust for bias 
  expnt &= 0x7F800000;
  expnt >>= 23;
  expnt -= 127;
  
  // shift by the exponent to get rid of the exponent. Need to use
  // 23 - expnt because expnt is number of positions to the left of
  // decimal and we want to get rid of the positions to the right of decimal
  mantissa >>= (23 - expnt);
  
  if (neg)
    mantissa = -mantissa;

//  D3DPRINT( 0,"float2Int int=%d ",mantissa );
   
  return mantissa;            
      
}

