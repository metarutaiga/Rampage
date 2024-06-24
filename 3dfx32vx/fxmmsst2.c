/* $Header: fxmmsst2.c, 6, 12/7/00 9:43:23 AM PST, Ryan Bissell$ */
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
** File name:   fxmmsst2.c
**
** Description: Contains the HW-specific portions of the
**              3dfx heap management MRI.
**
** $Log: 
**  6    Rampage   1.5         12/7/00  Ryan Bissell    Miscellaneous cleanup, 
**       and CS work
**  5    Rampage   1.4         11/23/00 Ryan Bissell    Lost-surface 
**       notification in Central Services, and new CS features requested by OGL 
**       Core.
**  4    Rampage   1.3         10/31/00 Ryan Bissell    Phase I of development 
**       of the "No Man's Land" reclamation heap.
**  3    Rampage   1.2         7/21/00  Ryan Bissell    Different configuration 
**       for heaps when using AGP command FIFO.
**  2    Rampage   1.1         7/14/00  Ryan Bissell    Partial fix for "report 
**       free memory" size bug in fxmm
**  1    Rampage   1.0         5/3/00   Ryan Bissell    
** $
**
*/


#include "precomp.h"
#include <sst2glob.h>                //Must be included into sst2.h
#include "fxmmsst2.h"
#include "cservice.h"

void __cdecl QT_Thunk (void);

/*----------------------------------------------------------------------
Function name:  fxmm_sst2_persistentlender()

Description:    a lending method for the persistent heap.

                This is identical to fxmm_lefthandlender(), except that
                (1) it assumes the heap is already in last-fit order,
                    so that the re-sorting to first-fit can be
                    optimized to O(n).
                (2) it honors a fence beyond which no loans will be made.
                
Side effects:   On entry:   none.
                On exit:    space is lent, if available

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_sst2_persistentlender(FXMM_PHEAPINFO this, U032 size, U032 align, U032 malign, FXMM_PHEAPNODE pnode)
{
  int done;
  U032 slop, trim;
  FXMM_PHEAPNODE here, node, best, firstfit=NULL;

  if (!(this && pnode)) { return DEBUGINT3, 0; }

  //if no alignment was specified, use the heap's granularity as an alignment
  align = align ? align : this->granularity;

  //RYAN@COMMENT, some definitions would be helpful.

  //Slop \slop\, n. The amount of buffer at the beginning of a heap node that won't
  //                be used, due to alignment requirements.  If the node's slop is
  //                greater in magnitude than 'this->granularity', it will be split
  //                off into a new node, and it will cease to be slop.  Otherwise, 
  //                it is left attached to the current node, as excess baggage.

  //Trim \trim\, n. The extent to which the size of a heap node exceeds the needed
  //                buffer size, (where any remaining slop is considered to be part 
  //                of the buffer.)  If the node's trim is greater in magnitude than
  //                that 'this->granularity', it will be split off into a new node,
  //                and it will cease to be trim.  Otherwise, it is left attached to
  //                the current node, as excess baggage.


  //round size up to a multiple of granularity, if necessary.
  size += ((this->granularity - (size % this->granularity)) % this->granularity);

  //make a reversed copy of the heap that is first-fit instead of last-fit.
  for (firstfit=NULL, here=this->head; here; here=here->next)
  {
    if (!(node = (FXMM_PHEAPNODE)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
      return DEBUGINT3, fxmm_destroylist(this->context, firstfit), 0;

    *node = *here;
    node->next = firstfit;
    firstfit = node;
  }
    

  //now search the temporary copy for a node that meets our needs.
  done = 0;
  best = firstfit;
  while (best && !done)
  {
    if (!best->flags && (best->size >= size))
    {
      slop = (align - (best->offset % align)) % align;

      //FIND THE LEFT-MOST VALID BOUNDARY. "VALID" MEANS:
      //1) aligned to 'align'       { !((best->offset+slop)%align) }
      //2) not aligned to 'malign'  {  malign && ((best->offset+slop)%malign) }
      //3) big enough for 'size'    {  (size <= best->size-slop) }
      do
      {
        if (!malign || ((best->offset+slop)%malign))
          break;

        slop += align;
      } while (size < best->size-slop);

      done = (size <= best->size-slop);
    }

    //don't loan out a block if it extends beyond the loan fence.
    if ((best->offset+slop+size) > FXMM_SST2_LOANBOUNDARY+this->heapstart)
      done = 0, best = NULL;
    else
      best = (done ? best : best->next);
  }

  if (!done)
  {
    //apparently, no offset exists which meets the criteria
    fxmm_destroylist(this->context, firstfit);
    return 0;
  }

  //now that we know which block is the best to use, and the extent of slop,
  //find the corresponding block node in the original list.
  here = this->head;
  while (here && (here->offset != best->offset))
    here = here->next;

  if (!here)
  {
    //this should be impossible; it indicates a disparity
    //between the original heap, and our re-sorted copy.
    return DEBUGINT3, fxmm_destroylist(this->context, firstfit), 0;
  }

  //only bother to split slop if it will not be less than the granularity
  //RYAN@BUG, perhaps we should trim slop to be a multiple of the granularity, too...?
  if (slop >= this->granularity)
  { 
    if (!fxmm_splithelper(this, here, slop, &here))
      return DEBUGINT3, fxmm_destroylist(this->context, firstfit), 0;

    slop =0;  //forget about it.
  }

  //note that, as a side-effect of calling fxmm_splithelper above, "here->size" is now smaller.
  size += slop;
  trim = here->size - size;

  //just like slop, if the trim is big enough to reuse, split it off.
  if (trim >= this->granularity)
    if (!fxmm_splithelper(this, here, size, NULL)) 
      return DEBUGINT3, fxmm_destroylist(this->context, firstfit), 0;

  pnode->flags = 0;
  pnode->next = NULL;
  pnode->size = here->size;
  pnode->offset = here->offset;

  here->flags = (FXMM_FLAGS_ALLOCATED | FXMM_FLAGS_LOANEDOUT);  
  
  fxmm_destroylist(this->context, firstfit);
  FXMM_HEAPWATCHDOG(this);
  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_sst2_reportfreememory()

Description:    returns the sum of the sizes of all free nodes in the
                heap that are loanable to other heaps.
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (U032)  the amount of freespace, in bytes
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
U032 fxmm_sst2_reportfreememory(FXMM_PHEAPINFO this)
{
  U032 free;
  FXMM_PHEAPNODE here;

  if (!this) { return DEBUGINT3, 0; }

  //first call the overloaded default method
  free = fxmm_default_reportfreememory(this);

  if (!this->lender) return free;

  //now add in the space that the lender is willing to loan us, at this point in time.
  for (here=this->lender->head; here; here=here->next)
  {
    if (!here->flags)
    {
      if (here->offset+here->size < FXMM_SST2_LOANBOUNDARY+this->lender->heapstart)
        free += here->size;
      else
        if (here->offset < FXMM_SST2_LOANBOUNDARY+this->lender->heapstart)
          free += (here->offset+here->size)-(FXMM_SST2_LOANBOUNDARY+this->lender->heapstart);
    }
  }

  return free;
}



void cs9x_Notification32(NT9XDEVICEDATA * ppdev, DWORD message, DWORD resource)
{
#ifdef CSERVICE //[

  // QT_Thunk is documented in Windows 95 System Programming
  //   Secrets, Pietrek.  pp. 191-195, 208-211.
  // NOTE that QT_Thunk will die without fixing up EBP.  I
  //   don't know exactly how big the stack should be but
  //   it appears the more the 16:16 uses, the larger it must be!

  DWORD Addr1616;

  // Get 16:16 function address from shared data
  Addr1616 = _FF(csNotification1616);

  // QT_Thunk will do all the fixups and pass control to
  //   the 16:16 in edx.
  __asm
  {
    push  DWORD PTR message
    push  DWORD PTR resource
    mov   edx,Addr1616     ; QT_Thunk expects 16:16 proc addr in EDX
    sub   ebp,200h         ; QT_Thunk builds convoluted 16:16 stack frame
    call  QT_Thunk
    add   ebp,200h         ; QT_Thunk stack stuff
  }

#endif //] CSERVICE
}



void fxmm_sst2_heapnotify(FXMM_PHEAPINFO this, U032 message, FXMM_PHEAPNODE here)
{
  NT9XDEVICEDATA * ppdev;

  if (!this) { DEBUGINT3; return; }

  ppdev = this->context;
  switch (message)
  {
    case FXMM_NOTIFY_BLOCKLOST:
      if (here->owner != _D3(csContextID))
        cs9x_Notification32(this->context, CS9X_STATE_RESOURCELOST, here->owner); 
      break;
  }
}



