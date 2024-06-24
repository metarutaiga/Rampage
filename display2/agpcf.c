/* -*-c++-*- */
/* $Header: agpcf.c, 5, 10/4/00 10:57:41 AM PDT, Dale  Kenaston$ */
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
** File name:   agpcf.c
**
** Description: AGP CmdFifo related functions.
**
** $Revision: 5$
** $Date: 10/4/00 10:57:41 AM PDT$
**
** $History: agpcf.c $
** 
** *****************  Version 3  *****************
** User: Xingc        Date: 6/11/99    Time: 5:05p
** Updated in $/devel/sst2/Win95/dx/minivdd
** Delete DoKMTV() function
** 
** *****************  Version 2  *****************
** User: Peterm       Date: 5/21/99    Time: 4:44p
** Updated in $/devel/sst2/Win95/dx/minivdd
** merged in v3 tot changes
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 5/06/99    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Modified to work with Direct Write
** 
** *****************  Version 6  *****************
** User: Xingc        Date: 2/26/99    Time: 3:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Use KMVTBUFF structure to set command fifo.
** 
** *****************  Version 5  *****************
** User: Xingc        Date: 2/25/99    Time: 8:05p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add DoKMTV( ) macros for  DD32 and minivdd
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/04/99    Time: 1:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 11/22/98   Time: 8:50p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changes to support Multi-Monitor
** 
** *****************  Version 2  *****************
** User: Martin       Date: 10/29/98   Time: 2:14p
** Updated in $/devel/h3/Win95/dx/minivdd
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** *****************  Version 1  *****************
** User: Ken          Date: 7/23/98    Time: 2:08p
** Created in $/devel/h3/win95/dx/minivdd
** common agp routines for all driver modules (#included directly by
** d3d/dd and dd16)
**
*/

#ifdef WINNT
#define NT9XDEVICEDATA PDEV
#else
#define NT9XDEVICEDATA GLOBALDATA
#endif

#ifdef MM
#define BUMPAGP(arg1,arg2) bumpAgp(arg1,arg2)
#define FLUSHAGP(arg1) flushAgp(arg1)
#define MYWRAPAGP(arg1,arg2) myWrapAgp(arg1,arg2)
#define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg1,arg2)
#else
#define BUMPAGP(arg1,arg2) bumpAgp(arg2)
#define FLUSHAGP(arg1) flushAgp()
#define MYWRAPAGP(arg1,arg2) myWrapAgp(arg2)
#define MYAGPEPILOG(arg1,arg2) myAgpEpilog(arg2)
#endif


/*----------------------------------------------------------------------
Function name:  BUMPAGP

Description:    Update the fifo.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void BUMPAGP(NT9XDEVICEDATA *ppdev, FxU32 nWords)
{
   MYFENCVAR;

   DEBUG_FIX;

   MYP6FENCE;

   while (nWords > 0xffffL)
   {
      if (IS_SAGE_ACTIVE)
         SETDW(_FF(lpGeCfeRegs)->cfeGeCmdBump, 0xFFFFL)
      else
         SETDW(MYCMDFIFO.bump, 0xFFFFL);
      nWords -= 0xffffL;
   }

   if (nWords > 0)
   {
      if (IS_SAGE_ACTIVE)
         SETDW(_FF(lpGeCfeRegs)->cfeGeCmdBump, nWords)
      else
         SETDW(MYCMDFIFO.bump, nWords);
   }

   CMDFIFOUNBUMPEDWORDS = 0;        
}


/*----------------------------------------------------------------------
Function name:  FLUSHAGP

Description:    Flush the fifo.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void FLUSHAGP(NT9XDEVICEDATA * ppdev)
{
  FxU32 nWords;

  DEBUG_FIX;

  nWords = CMDFIFOUNBUMPEDWORDS;

  if (nWords == 0)
    return;

  BUMPAGP(ppdev, nWords);
}

//
// agp command fifo stuff
//

/*----------------------------------------------------------------------
Function name:  MYWRAPAGP

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void MYWRAPAGP(NT9XDEVICEDATA * ppdev, FxU32 hwPtr)
{
   FxU32 nWords;

   DEBUG_FIX;

   // calculate the # of words to bump
   // remember to add 2 words for the AGP jump!
   //
   nWords = (hwPtr + 8 - CMDFIFOEPILOGPTR) / 4;

   nWords += CMDFIFOUNBUMPEDWORDS;

   BUMPAGP(ppdev, nWords);

   CMDFIFOEPILOGPTR = CMDFIFOSTART;
}


/*----------------------------------------------------------------------
Function name:  MYAGPEPILOG

Description:    

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void MYAGPEPILOG(NT9XDEVICEDATA *ppdev, FxU32 hwPtr)
{
  FxU32 nWords;

  DEBUG_FIX;
  
  nWords = (hwPtr - CMDFIFOEPILOGPTR) / 4;
  
  CMDFIFOEPILOGPTR = hwPtr;

  CMDFIFOUNBUMPEDWORDS += nWords;

  if (CMDFIFOUNBUMPEDWORDS < CMDFIFO_BUMPTHRESH)
    return;

  nWords = CMDFIFOUNBUMPEDWORDS;

  BUMPAGP(ppdev, nWords);
}

