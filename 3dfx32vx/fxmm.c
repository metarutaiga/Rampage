/* $Header: fxmm.c, 15, 12/7/00 9:43:30 AM PST, Ryan Bissell$ */
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
** File name:   fxmm.c
**
** Description: Contains the OS-independent portion of the 3dfx heap
**              management MRI.
**
** $Log: 
**  15   Rampage   1.14        12/7/00  Ryan Bissell    Miscellaneous cleanup, 
**       and CS work
**  14   Rampage   1.13        11/23/00 Ryan Bissell    Lost-surface 
**       notification in Central Services, and new CS features requested by OGL 
**       Core.
**  13   Rampage   1.12        11/13/00 Ryan Bissell    Accidentally used 
**       "#ifdef" instead of the intended "#if".  This broke the non-debug 
**       flavor of the build.
**  12   Rampage   1.11        11/12/00 Ryan Bissell    Fixed a list-walking bug
**       in fxmm_resetheap(), that was causing us to spinlock in Verdict's 
**       DirectDraw Surface Locking test frame.  Also added some new STAT code 
**       that assigns serial numbers to each allocation.  This is sometimes 
**       useful for debugging purposes.
**  11   Rampage   1.10        11/5/00  Ryan Bissell    trivial reject for 
**       fxmm_freeblock(), new fxmm_queryheap() function, and misc cleanup.
**  10   Rampage   1.9         10/31/00 Ryan Bissell    Phase I of development 
**       of the "No Man's Land" reclamation heap.
**  9    Rampage   1.8         10/17/00 Ryan Bissell    Temporarily removed a 
**       DEBUGINT3 from fxmm_freeblock().
**  8    Rampage   1.7         8/28/00  Ryan Bissell    I used to fail calls to 
**       fxmm_freeblock() if the block in questions was already free.  I now 
**       considered this a "trivial accept" case.
**  7    Rampage   1.6         7/21/00  Ryan Bissell    Different configuration 
**       for heaps when using AGP command FIFO.
**  6    Rampage   1.5         7/14/00  Ryan Bissell    Partial fix for "report 
**       free memory" size bug in fxmm
**  5    Rampage   1.4         7/13/00  Ryan Bissell    Cruft removal, and the 
**       addition of "fxmm_heapcontains()"
**  4    Rampage   1.3         6/26/00  Ryan Bissell    Incremental CS 
**       development.  Also removed some message pragmas, in anticipation of CS 
**       activation.
**  3    Rampage   1.2         6/15/00  Michel Conrad   Change pragma to 
**       comment.
**  2    Rampage   1.1         5/3/00   Ryan Bissell    Continued deployment of 
**       Central Services and related changes.
**  1    Rampage   1.0         4/14/00  Ryan Bissell    
** $
**
*/


#include <stdlib.h>
#include <stdarg.h>

#define FXMM_DEBUGGUARD 1 /* turn off optimizations if DEBUG build of fxmm.c */
#include "fxmm.h"
#include "fxmmstat.h"

#ifndef FXMM_HEAPPARANOIA
#error "FXMM_HEAPPARANOIA must be defined before this file will compile."
#endif

#ifndef FXMM_FREEFACILITY
#error "FXMM_FREEFACILITY must be defined before this file will compile."
#endif

#ifndef FXMM_MALLOCFACILITY
#error "FXMM_MALLOCFACILITY must be defined before this file will compile."
#endif


#define FXMM_INRANGE(val, lo, hi)  (((val) >= (lo)) && ((val) <= ((hi)-1)))



/*----------------------------------------------------------------------
Function name:  fxmm_strdup()

Description:    duplicates a string, using FXMM_MALLOCFACILITY
                
Side effects:   On entry:   none.
                On exit:    returns pointer to the duplicate string

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE char* fxmm_strdup(FXMM_PHEAPINFO this, char* str)
{
  U032 len;
  char* newstr;

  if (!str) return DEBUGINT3, NULL;

  for (len=0; str[len]; len++)
    NULL;

  newstr = (char*)FXMM_MALLOCFACILITY(this->context, (len*sizeof(char)+1));

  for (len=0; *str; str++, len++)
    newstr[len] = *str;

  newstr[len] = '\x0';
  return newstr;
}

  

/*----------------------------------------------------------------------
Function name:  fxmm_createheap()

Description:    initialized the provided heap structure
                
Side effects:   On entry:   none.
                On exit:    heap is ready for use

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_createheap(char* name,
                    FXMM_PHEAPINFO this, 
                    FXMM_MALLOCHANDLE context, 
                    U032 start, 
                    U032 size, 
                    U032 granularity, 
                    FXMM_PHEAPINFO lender,
                    FXMM_LENDFUNCTION funcLending,
                    FXMM_SIZEFUNCTION funcAvailable,
                    FXMM_SPLITFUNCTION funcDivision,
                    FXMM_INSERTFUNCTION funcInsertion,
                    FXMM_NOTIFYFUNCTION funcNotification)
{

  if (!this) { return DEBUGINT3, 0; }

  //failsafe measures
  memset(this, 0, sizeof(*this));
  granularity = (granularity ? granularity : 1);   //(0 --> 1)
  size -= (size % granularity);

  //initialize
  this->head = NULL;
  this->context = context;
  this->heapsize = size;
  this->heapstart = start;
  this->granularity = granularity;
  
  this->lender           = lender;
  this->funcLending      = funcLending    ? funcLending    : (FXMM_LENDFUNCTION)fxmm_notalender;
  this->funcAvailable    = funcAvailable  ? funcAvailable  : (FXMM_SIZEFUNCTION)fxmm_default_reportfreememory;
  this->funcDivision     = funcDivision   ? funcDivision   : (FXMM_SPLITFUNCTION)fxmm_leftjustifiedsplit;
  this->funcInsertion    = funcInsertion  ? funcInsertion  : (FXMM_INSERTFUNCTION)fxmm_bestfitinsertion;
  this->funcNotification = funcNotification;

  //make sure we only malloc the head once, and only if the heap size is nonzero.
  if (this->heapsize && !this->head)
  {
    if (!(this->head = (FXMM_HEAPNODE*)FXMM_MALLOCFACILITY(context, sizeof(FXMM_HEAPNODE))))
      return DEBUGINT3, 0;

    this->head->loan = NULL;
    this->head->next = NULL;
    this->head->flags = 0UL;
    this->head->size = size;
    this->head->offset = start;
    this->head->owner = 0;
  }

  //don't worry if this allocation fails; it's for visual identification only.
  this->heapname = FXMM_DEBUGGING ? fxmm_strdup(this, name) : NULL;  

  FXMM_STAT_RESETFREE(this);
  FXMM_STAT_TABULATION(this);
  FXMM_HEAPWATCHDOG(this);
  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_destroylist()

Description:    destroys the given heap chain, without regard to 
                content or state.  FOR INTERNAL USE ONLY.
                
Side effects:   On entry:   none.
                On exit:    heap chain is destroyed.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
void fxmm_destroylist(FXMM_MALLOCHANDLE context, FXMM_PHEAPNODE head)
{
  FXMM_PHEAPNODE next;

  for (;head;head=next)
  {
    next = head->next;
    
    if (head->loan && (!--(*head->loan)))
      FXMM_FREEFACILITY(context, head->loan);

    FXMM_FREEFACILITY(context, head);
  }
}



/*----------------------------------------------------------------------
Function name:  fxmm_destroyheap()

Description:    destroys all dynamically-allocated pieces of
                the specified heap info structure, without regard to
                content or state.  FOR INTERNAL USE ONLY.
                
Side effects:   On entry:   none.
                On exit:    heap is destroyed, an must be re-initialized
                            with fxmm_createheap() before being used
                            again.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE void fxmm_destroyheap(FXMM_PHEAPINFO this)
{
  if (this->heapname)
    FXMM_FREEFACILITY(this->context, this->heapname);

  fxmm_destroylist(this->context, this->head);
  this->head = NULL;
  this->heapsize = 0;
  this->heapname = NULL;
  this->heapstart = 0;
}



/*----------------------------------------------------------------------
Function name:  fxmm_removenode()

Description:    removes the given node from the given heap
                
Side effects:   On entry:   none.
                On exit:    node is removed from heap

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE int fxmm_removenode(FXMM_PHEAPINFO this, FXMM_PHEAPNODE node)
{
  FXMM_PHEAPNODE here, prev = NULL;

  if (!(this && node)) { return DEBUGINT3, 0; }

  //find node in the heap
  for (here = this->head; (here && (here != node)); here = here->next)
    prev = here;

  if (!here) { return DEBUGINT3, 0; }  //not found

  //we found 'node' in heap, so remove it
  *(prev ? &prev->next : &this->head) = here->next;

  FXMM_STAT_NUMFREE(this, (( !node->flags) * -1 * !!this->statNumFree));  //subtract 1, but clamp at 0.
  FXMM_STAT_NUMUSED(this, ((!!node->flags) * -1 * !!this->statNumUsed));  //subtract 1, but clamp at 0.
  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_insertnode()

Description:    inserts the given node into the given heap
                
Side effects:   On entry:   none.
                On exit:    node is inserted into heap

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE int fxmm_insertnode(FXMM_PHEAPINFO this, FXMM_PHEAPNODE node)
{
  int result;

  if (!(this && node)) { return DEBUGINT3, 0; }
  
  result = this->funcInsertion(this, node);

  FXMM_STAT_NUMFREE(this,  !node->flags);
  FXMM_STAT_NUMUSED(this, !!node->flags);
  return result;
}



/*----------------------------------------------------------------------
Function name:  fxmm_dividenode()

Description:    divides the given node into 2 pieces
                
Side effects:   On entry:   none.
                On exit:    node is divided within heap

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE int fxmm_dividenode(FXMM_PHEAPINFO this, FXMM_PHEAPNODE* phere, U032 size, U032 align, U032 malign)
{
  int result;

  if (!(this && phere && *phere)) { return DEBUGINT3, 0; }

  result = !!this->funcDivision(this, phere, size, align, malign);

  FXMM_HEAPWATCHDOG(this);
  return result;
}



/*----------------------------------------------------------------------
Function name:  fxmm_splithelper()

Description:    splits 'left' into 2 pieces: a leftmost piece that is
                'leftsize' bytes in size, and a rightmost piece that
                makes up the balance of the original size. If the caller
                so desires, it can obtain the address of the 'right'
                piece by passing in a non-NULL address to a pointer to
                a FXMM_HEAPNODE.
                
Side effects:   On entry:   none.
                On exit:    'left' is split into 'left' and 'right'

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_splithelper(FXMM_PHEAPINFO this, FXMM_PHEAPNODE left, U032 leftsize, FXMM_PHEAPNODE* pright)
{
  FXMM_PHEAPNODE chunk;

  if (pright) *pright = NULL;

  if (!(this && left)) { return DEBUGINT3, 0; }   //it's OK for 'pright' to be NULL
  
  if (!(chunk = (FXMM_HEAPNODE*)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
    return DEBUGINT3, 0; 

  chunk->flags = left->flags;
  chunk->loan = left->loan;
  chunk->size = left->size - leftsize;
  chunk->offset = left->offset + leftsize;

  if (!fxmm_removenode(this, left))   
    return DEBUGINT3, FXMM_FREEFACILITY(this->context, chunk), 0;

  //the insertion facility enforces sort order
  if (!fxmm_insertnode(this, chunk))
    return DEBUGINT3, FXMM_FREEFACILITY(this->context, chunk), 0;

  //the insertion facility enforces sort order
  left->size = leftsize;
  if (!fxmm_insertnode(this, left))
    return DEBUGINT3, FXMM_FREEFACILITY(this->context, chunk), 0;

  //update shared split count, if part of a loaned block
  if (left->loan)
    *(left->loan) += 1;

  if (pright) *pright = chunk;

  FXMM_HEAPWATCHDOG(this);
  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_rightjustifiedsplit()

Description:    finds the right-most position within 'here' that is
                aligned to 'align', NOT aligned to 'malign', and is
                at least 'size' bytes away from the end of 'here'.  
                On success, 'here' is modified to point to the right-
                most half.

                (In memory management parlance, the "left-hand side"
                of a block or heap is its low-address end, and the
                "right-hand side" is, redundantly, the high-address
                end.  So a "right-justified split" is one that justi-
                fies the split to the high-address end of a block,
                (rather than the low-address end, which is more
                customary.))

Side effects:   On entry:   none.
                On exit:    'here' is split into 'here' and 'left'

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_rightjustifiedsplit(FXMM_PHEAPINFO this, FXMM_PHEAPNODE* phere, U032 size, U032 align, U032 malign)
{
  U032 slop;
  U032 trim;
  U032 possible;

  #define HERE (*phere)

  if (!(this && phere && HERE)) { return DEBUGINT3, 0; }

  //RYAN@COMMENT, some definitions would be helpful.

  //Slop \slop\, n. The amount of buffer at the beginning of a heap node that won't
  //                be used, due to alignment requirements.  If the node's slop is
  //                not less than 'this->granularity', it will be split off into a
  //                new node and it will cease to be slop.  Otherwise, it remains 
  //                attached to the current node, as excess baggage.

  //Trim \trim\, n. The extent to which the size of a heap node exceeds the needed
  //                buffer size, (where any remaining slop is considered to be part 
  //                of the needed size.)  If the node's trim is not less than
  //                'this->granularity', it will be split off into a new node, and 
  //                it will cease to be trim.  Otherwise, it remains attached to 
  //                the current node, as excess baggage.


  //initialize alignment slop to left-most alignment offset
  slop = (align - (HERE->offset % align)) % align;

  if (slop > (HERE->size - size))  //no point in trying if this happens.
    return 0;

  //how many possible alignment boundaries exist within this block?
  possible = ((HERE->size - slop) / align) + !!((HERE->size - slop) % align);

  //FIND THE RIGHT-MOST VALID BOUNDARY. "VALID" MEANS:
  //1) aligned to 'align'       { !((HERE->offset+slop+((possible-1)*align))%align) }
  //2) not aligned to 'malign'  { malign && ((HERE->offset+slop+((possible-1)*align))%malign) }
  //3) big enough for 'size'    { (size <= (HERE->size - (slop+((possible-1)*align)))) }
  while ( possible && ((malign && !((HERE->offset+slop+((possible-1)*align))%malign)) || (size > (HERE->size - (slop+((possible-1)*align))))) )
    possible -= 1;

  if (!possible) return 0;

  //increase slop to account for right-justification and malignment compensation
  slop += (--possible)*align;



  //only bother to split slop if it will not be less than the granularity
  //RYAN@BUG, perhaps we should trim slop to be a multiple of the granularity, too...?
  if (slop >= this->granularity)
  {
    if (!fxmm_splithelper(this, HERE, slop, &HERE))
      return DEBUGINT3, 0; 

    slop =0;
  }

  //if the trim is big enough to reuse, split it off.
  size += slop;
  trim = HERE->size - size;
  if (trim >= this->granularity)
    if (!fxmm_splithelper(this, HERE, size, NULL)) 
      return DEBUGINT3, 0; 

  FXMM_HEAPWATCHDOG(this);
  return 1;

  #undef HERE
}



/*----------------------------------------------------------------------
Function name:  fxmm_leftjustifiedsplit()

Description:    finds the left-most position within 'here' that is
                aligned to 'align', NOT aligned to 'malign', and is
                at least 'size' bytes away from the end of 'here'.  
                On success, 'here' is modified to point to the left-
                most half.

                (In memory management parlance, the "left-hand side"
                of a block or heap is its low-address end, and the
                "right-hand side" is, redundantly, the high-address
                end.  So a "left-justified split" is one that justi-
                fies the split to the low-address end-- which is 
                the usual and customary thing to do.)

Side effects:   On entry:   none.
                On exit:    'here' is split into 'here' and 'left'

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_leftjustifiedsplit(FXMM_PHEAPINFO this, FXMM_PHEAPNODE* phere, U032 size, U032 align, U032 malign)
{
  U032 slop, trim;
  FXMM_PHEAPNODE chunk=NULL;

  #define HERE (*phere)

  if (!(this && phere && HERE)) { return DEBUGINT3, 0; }

  //RYAN@COMMENT, some definitions would be helpful.

  //Slop \slop\, n. The amount of buffer at the beginning of a heap node that won't
  //                be used, due to alignment requirements.  If the node's slop is
  //                not less than 'this->granularity', it will be split off into a
  //                new node and it will cease to be slop.  Otherwise, it remains 
  //                attached to the current node, as excess baggage.

  //Trim \trim\, n. The extent to which the size of a heap node exceeds the needed
  //                buffer size, (where any remaining slop is considered to be part 
  //                of the needed size.)  If the node's trim is not less than
  //                'this->granularity', it will be split off into a new node, and 
  //                it will cease to be trim.  Otherwise, it remains attached to 
  //                the current node, as excess baggage.


  //initialize alignment slop to left-most alignment offset
  slop = (align - (HERE->offset % align)) % align;

  if (slop > (HERE->size - size))  //no point in trying if this happens.
    return 0;

  //FIND THE LEFT-MOST VALID BOUNDARY. "VALID" MEANS:
  //1) aligned to 'align'       { !((HERE->offset+slop)%align) }
  //2) not aligned to 'malign'  { !malign || ((HERE->offset+slop)%malign) }
  //3) big enough for 'size'    {  (size <= HERE->size-slop) }
  do
  {
    if (!malign || ((HERE->offset+slop)%malign))
      break;

    slop += align;
  } while (size < HERE->size-slop);

  if (size > HERE->size-slop)
    return 0;

  //only bother to split slop if it will not be less than the granularity
  //RYAN@BUG, perhaps we should trim slop to be a multiple of the granularity, too...?
  if (slop >= this->granularity)
  { 
    if (!fxmm_splithelper(this, HERE, slop, phere))
      return DEBUGINT3, 0; 

    slop =0;  //forget about it.
  }

  //note that, as a side-effect of calling fxmm_splithelper above, "HERE->size" is now smaller.
  size += slop;
  trim = HERE->size - size;

  //just like slop, if the trim is big enough to reuse, split it off.
  if (trim >= this->granularity)
    if (!fxmm_splithelper(this, HERE, size, NULL)) 
      return DEBUGINT3, 0; 


  FXMM_HEAPWATCHDOG(this);
  return 1;

  #undef HERE
}



/*----------------------------------------------------------------------
Function name:  fxmm_notalender()

Description:    a null lending callback for heaps that do not lend
                their space to other heaps.
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_notalender(FXMM_PHEAPINFO this, U032 size, U032 align, U032 malign, FXMM_PHEAPNODE pnode)
{
  if (!(this && pnode)) { return DEBUGINT3, 0; }

  return 0; //indicates no memory available to loan out
}



/*----------------------------------------------------------------------
Function name:  fxmm_lefthandlender()

Description:    a lending method that lends out space to other heaps
                from its heap's low-address ("left hand") end.
                
Side effects:   On entry:   none.
                On exit:    space is lent, if available

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_lefthandlender(FXMM_PHEAPINFO this, U032 size, U032 align, U032 malign, FXMM_PHEAPNODE pnode)
{
  int done;
  U032 slop, trim;
  FXMM_PHEAPNODE here, node, best, prev, firstfit=NULL;

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

  //RYAN@PERFORMANCE, Since this is a generic, canned lending facility, I don't know
  //for certain the associated heap is sorted in first-fit order.  So I'll make a 
  //temporary copy of the heap that is.  You could easily write your own version of this 
  //facility that isn't this cautious, SO LONG AS as you know you are using it on a heap
  //for which that assumption would be valid.  But this is a canned sample, so I gotta be 
  //paranoid.

  //make a temporary copy of the heap, sorted in first-fit order.
  for (here=this->head; here; here=here->next)
  {
    if (!(node = (FXMM_PHEAPNODE)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
      return DEBUGINT3, fxmm_destroylist(this->context, firstfit), 0;
    
    *node = *here;

    //find where the new node should be inserted into the new list
    prev = NULL; best = firstfit;
    while (best && (best->offset < node->offset))
    {
      prev = best;
      best = best->next;
    }

    //insert the node into the spot we found
    node->next = best;
    *(prev ? &prev->next : &firstfit) = node;
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
Function name:  fxmm_bestfitinsertion()

Description:    Insertion callbacks are what control the allocation
                policy of a particular heap; fxmm_allocateblock() gives
                priority to chunks at the beginning of the node list,
                so by maintaining a sort order during insertion, an
                insertion callback can drive allocation policy.

                This insertion callback enforces a "best fit" policy,
                by keeping the nodes sorted by SIZE, in ASCENDING order.
                
Side effects:   On entry:   none.
                On exit:    node is inserted into heap, according to policy

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_bestfitinsertion(FXMM_PHEAPINFO this, FXMM_PHEAPNODE node)
{
  FXMM_PHEAPNODE here;
  FXMM_PHEAPNODE prev = NULL;

  if (!(this && node)) { return DEBUGINT3, 0; }

  //this function enforces a "best fit" policy, by keeping the nodes sorted by SIZE, in ASCENDING order.
  for (here = this->head; (here && (here->size < node->size)); here = here->next)
    prev = here;

  //insert the node
  node->next = here;
  *(prev ? &prev->next : &this->head) = node;

  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_lastfitinsertion()

Description:    Insertion callbacks are what control the allocation
                policy of a particular heap; fxmm_allocateblock() gives
                priority to chunks at the beginning of the node list,
                so by maintaining a sort order during insertion, an
                insertion callback can drive allocation policy.

                This insertion callback enforces a "last fit" policy,
                by keeping the nodes sorted by OFFSET, in DESCENDING 
                order.
                
Side effects:   On entry:   none.
                On exit:    node is inserted into heap, according to policy

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_lastfitinsertion(FXMM_PHEAPINFO this, FXMM_PHEAPNODE node)
{
  FXMM_PHEAPNODE here;
  FXMM_PHEAPNODE prev = NULL;

  if (!(this && node)) { return DEBUGINT3, 0; }

  //this function enforces a "last fit" policy, by keeping the nodes sorted by OFFSET, in DESCENDING order.
  for (here = this->head; (here && (here->offset > node->offset)); here = here->next)
    prev = here;

  //insert the node
  node->next = here;
  *(prev ? &prev->next : &this->head) = node;

  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_freeblock()

Description:    un-allocates a block previously allocated by either
                fxmm_allocateblock(), or a lending callback.  If
                adjacent nodes are also currently free, they
                will be merged together.
                
Side effects:   On entry:   none.
                On exit:    node is free'd within the heap, ready for
                            future allocation.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_freeblock(FXMM_PHEAPINFO this, U032 offset)
{
  FXMM_PHEAPNODE left, right, here;

  if (!this) { return DEBUGINT3, 0; }

  FXMM_STAT_FREEATTEMPT(this);

  left = right = here = this->head;

  //find the node that owns the given offset
  while (here && !FXMM_INRANGE(offset, here->offset, here->offset+here->size))
    here = here->next;

  //hopefully we found it... (RYAN@001017, temporarily removing DEBUGINT3)
  if (!here) { return /* DEBUGINT3, */ FXMM_STAT_FREEFAILURE(this), 0; }

  //...and hopefully it's not already free
  if (!here->flags) { return DEBUGINT3, FXMM_STAT_FREEFAILURE(this), 0; }

  //find its left comrade, for possible defragmentation
  while (left && (left->offset+left->size != here->offset))
    left = left->next;

  //find its right cohort, for possible defragmentation
  while (right && (here->offset+here->size != right->offset))
    right = right->next;

  //we can only defrag if both pieces were loaned from the same chunk (or neither are on loan)
  if (left && !left->flags && (left->loan == here->loan))   //defrag left
  {
    here->offset = left->offset;
    here->size += left->size;
    fxmm_removenode(this, left);
    FXMM_FREEFACILITY(this->context, left);

    //if both pieces are part of a loan, decrement the loan split count.
    if (here->loan)
      *(here->loan) -= 1;
  }

  //we can only defrag if both pieces were loaned from the same chunk (or neither are on loan)
  if (right && !right->flags && (right->loan == here->loan)) //defrag right
  {
    here->size += right->size;
    fxmm_removenode(this, right);
    FXMM_FREEFACILITY(this->context, right);

    //if both pieces are part of a loan, decrement the loan split count.
    if (here->loan)
      *(here->loan) -= 1;
  }

  if (this->lender && here->loan && !(*(here->loan)))
  { 
    //it appears that we no longer need this loaner, so return it to the lender
    fxmm_removenode(this, here);
    fxmm_freeblock(this->lender, here->offset);
    FXMM_FREEFACILITY(this->context, here->loan);
    FXMM_FREEFACILITY(this->context, here);
  }
  else
  { 
    //re-sort the list according to overloaded policy (a simple removal and insertion of the free'd block will cause this)
    fxmm_removenode(this, here);  //updates used count for us
    here->owner = 0;
    here->flags = 0;              //mark the node as free
    fxmm_insertnode(this, here);  //updates free count for us
  }

  FXMM_STAT_TABULATION(this);
  FXMM_HEAPWATCHDOG(this);

#if FXMM_STATISTICS  //[
  here->serial = 0;
#endif //] FXMM_STATISTICS

  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_borrowblock()

Description:    if the 'this' heap has a lender, this function will
                attempt to obtain a block of memory from the lender
                that meets the specified size and alignment requirements.
                Since the lending heap may have a different granularity
                or allocation policy that 'this' does, fxmm_borrowblock()
                will attempt to remove additional trim and slop from
                the lent block, even though the lender (supposedly)
                did the same thing.

                It's okay to call this function on a null heap (a heap
                where this->head=NULL and this->heapsize=0) because,
                if the loan is successful, it won't be null anymore.
                
Side effects:   On entry:   none.
                On exit:    free node is allocated

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE int fxmm_borrowblock(FXMM_PHEAPINFO this, FXMM_PHEAPNODE* pnode, U032 size, U032 align, U032 malign, U032 flags)
{
  int result=1;
  #define HERE (*pnode)

  if (!(this && pnode)) return DEBUGINT3, 0;

  if (!this->lender) return 0;

  if (!(HERE = (FXMM_PHEAPNODE)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
    return DEBUGINT3, 0; 

  //do this upfront, so that we won't get caught with our pants down (like we would if it failed after the split)
  if (!(HERE->loan = (U032*)FXMM_MALLOCFACILITY(this->context, sizeof(U032))))
    return DEBUGINT3, FXMM_FREEFACILITY(this->context, HERE), 0; 

  if (!this->lender->funcLending(this->lender, size, align, malign, HERE))
  {
    //usually means that the lender doesn't have anything to give us.
    FXMM_FREEFACILITY(this->context, HERE->loan);
    FXMM_FREEFACILITY(this->context, HERE);
    return 0;
  }

  HERE->flags = *(HERE->loan) = 0;
  if (result &= fxmm_insertnode(this, HERE))                      //insert borrowed block into our own heap.
    result &= fxmm_dividenode(this, &HERE, size, align, malign);  //split the node we were just given.

  if (!result)
  {
    //oops!  It appears that something has gone horribly wrong.
    //there's no sense in keeping what we just borrowed.
    //note that this clean-up assumes that the failure left us in a valid state.
    if (!fxmm_freeblock(this, HERE->offset))
      DEBUGINT3;

    return DEBUGINT3, HERE=NULL, 0;
  }


  FXMM_STAT_TABULATION(this);
  FXMM_HEAPWATCHDOG(this);
  return 1;

  #undef HERE
}



/*----------------------------------------------------------------------
Function name:  fxmm_allocateblock()

Description:    allocates a block of the given size and the given
                offset alignment.  The 'align' and 'malign' parameters
                are NOT masks; IE, if an alignment of 1K is desired, 
                use align=1024, NOT align=0x3FF.

                It's okay to call this function on a null heap (a heap
                where this->head=NULL and this->heapsize=0) because the
                heap may have a lender that can loan some space.
                
Side effects:   On entry:   none.
                On exit:    free node is allocated

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_allocateblock(FXMM_PHEAPINFO this, U032* poffset, U032 size, U032 align, U032 malign, U032 flags, U032 owner)
{
  U032 done=0;
  FXMM_PHEAPNODE here;

  //RYAN@POLICY: It's okay to call this function on a null heap (heap->head = heap->heapsize = 0)
  if (!(this && poffset)) { return DEBUGINT3, 0; }

  FXMM_STAT_ALLOCATTEMPT(this);

  //trivial reject: allocation size is greater than heap size.
  if (size > this->heapsize) return 0;

  //round size up to a multiple of granularity, if necessary.
  size += ((this->granularity - (size % this->granularity)) % this->granularity);

  //use the granularity as the alignment, if no alignment was specified.
  align = align ? align : this->granularity;

  //this function assumes that the ordering of the heap nodes is indicative of the desired allocation policy.
  here = this->head;
  while (here && !done)
  {
    if (!here->flags && (size <= here->size))
      done = fxmm_dividenode(this, &here, size, align, malign);

    here = (done ? here : here->next);
  }

  //if we haven't been able to satisfy the size and/or (m)alignment restrictions, and
  //we have a lender, try to get a block of memory loaned to us that meets the requirements.
  if (!done)
    if (!fxmm_borrowblock(this, &here, size, align, malign, 0))
      return FXMM_STAT_ALLOCFAILURE(this), FXMM_STAT_TABULATION(this), 0;

  here->owner = owner;
  here->flags = flags;
  here->flags |= FXMM_FLAGS_ALLOCATED;
  *poffset = here->offset;

  FXMM_STAT_NUMFREE(this, -1);
  FXMM_STAT_NUMUSED(this, +1);
  FXMM_STAT_TABULATION(this);
  FXMM_HEAPWATCHDOG(this);

#if FXMM_STATISTICS  //[
  here->serial = this->statAllocAttempts - this->statAllocFailures;
#endif //] FXMM_STATISTICS

  return 1;
}



/*----------------------------------------------------------------------
Function name:  fxmm_default_reportfreememory()

Description:    returns the sum of the sizes of all free nodes in the
                'this' heap.  (ie, not including any space that might be
                available in a lending heap.)
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (U032)  the amount of freespace, in bytes
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
U032 fxmm_default_reportfreememory(FXMM_PHEAPINFO this)
{
  U032 free;
  FXMM_PHEAPNODE here;

  if (!this) { return DEBUGINT3, 0; }

  for (free=0, here=this->head; here; here=here->next)
    free += (here->flags ? 0 : here->size);

  return free;
}



/*----------------------------------------------------------------------
Function name:  fxmm_reportfreememory()

Description:    returns the amount of free space available in the heap.
                By default, this is computed with 
                "fxmm_default_reportfreememory()", but can be overloaded
                on a per-heap basis.

Side effects:   On entry:   none.
                On exit:    none.

Return:         (U032)  the amount of freespace, in bytes
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
U032 fxmm_reportfreememory(FXMM_PHEAPINFO this)
{
  if (!this->funcAvailable) { return DEBUGINT3, 0; }

  return this->funcAvailable(this);
}



/*----------------------------------------------------------------------
Function name:  fxmm_resetheap()

Description:    restores the 'this' heap to its virgin state, but
                with (possibly) a different start and size.  Any
                borrowed memory is returned to the lender. 
                
                DANGER: this function should only be called when all of
                the heap's memory is considered to be not in use.
                THIS INCLUDES LOANED-OUT MEMORY!  MAKE SURE THAT THIS
                HEAP DOES NOT HAVE ANY MEMORY LOANED OUT, BEFORE 
                CALLING THIS FUNCTION!
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_resetheap(FXMM_PHEAPINFO this, U032 start, U032 size)
{
  U032 reduced;
  FXMM_PHEAPNODE here;

  if (!this) { return DEBUGINT3, 0; }

  //first, destroy the entire existing heap (returning any loaned space to the lender.)
  while (this->head && this->head->next)  //as long as there's more than 1 node...
  {
    for (reduced=0,here=this->head; here && !reduced; here=here->next)
      if (reduced |= here->flags)
        fxmm_freeblock(this, here->offset);

    if (!reduced && this->head && this->head->next)
    {
      DEBUGINT3;
      //oops-- we made it all the way through a plural list without
      //finding a single alloc'd node.  That typically means this 
      //heap is totally FUBAR'd.  But hey-- we're supposed to be
      //resetting this heap anyway, so just recover (un)gracefully.
      fxmm_destroylist(this->context, this->head);
      this->head = NULL;
    }
  }


  //maybe our old heap was non-zero in length, and the new one isn't.
  if (this->head && !size)
  {
    FXMM_FREEFACILITY(this->context, this->head);
    this->head = NULL;
  }

  //maybe our new heap is non-zero in length, and the previous one wasn't.
  if (size && !this->head)
    if (!(this->head = (FXMM_HEAPNODE*)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
      return 0;

  //now reset the heap.
  this->heapsize = size;
  this->heapstart = start;

  //only applies to a heap with a non-zero length.
  if (this->head)
  {
    this->head->next = NULL;
    this->head->flags = 0UL;
    this->head->size = size;
    this->head->offset = start;
    this->head->loan = NULL;
  }

#if FXMM_STATISTICS  //[
  FXMM_STAT_RESETSTATS(this);
  this->statNumFree = (this->head ? 1 : 0);
  this->statMaxUsed = this->statNumUsed = 0;
  FXMM_STAT_TABULATION(this);
  FXMM_HEAPWATCHDOG(this);
#endif  //] FXMM_STATISTICS

  return 1;

}



/*----------------------------------------------------------------------
Function name:  fxmm_resizehelper()

Description:    a helper function to fxmm_resizeheap().
                
                DANGER: this function should not be called if the 'this'
                heap has loaned any memory out to another heap.
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
FXPRIVATE int fxmm_resizehelper(FXMM_PHEAPINFO this, long deltaStart, long deltaFinish)
{
  FXMM_PHEAPNODE here, next, prev;
  U032 newStart, newFinish;

  newStart = this->heapstart - deltaStart;
  newFinish = (this->heapstart + this->heapsize -1) + deltaFinish;

  //remove all nodes that don't fall entirely between newStart and newFinish
  for (here=this->head,prev=next=NULL; here; here=next)
  {
    next = here->next;

    #pragma message("RYAN@BUG, what about borrowed blocks?")
    if ( /*!here->loan && */ ((here->offset < newStart) || (here->offset+here->size-1 > newFinish)) )
    {
      #pragma message( "RYAN@BUG, this will trip the watchdog (test #1).  Needs to be fixed." )
      *(prev ? &prev->next : &this->head) = next;

      //if this block was in use, notify the owner (if desired)
      if (here->flags && this->funcNotification)
        this->funcNotification(this, FXMM_NOTIFY_BLOCKLOST, here);

      FXMM_FREEFACILITY(this->context, here);
    }
    else
      prev = here;
  }

  this->heapstart -= deltaStart;
  this->heapsize += (deltaStart + deltaFinish);
  return 1;
}

  

/*----------------------------------------------------------------------
Function name:  fxmm_resizeheap()

Description:    resizes the 'this' heap to the new specified start and
                size, preserving as many existing allocations as possible.
                
                DANGER: this function should not be called if the 'this'
                heap has loaned any memory out to another heap.
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_resizeheap(FXMM_PHEAPINFO this, U032 start, U032 size)
{
  FXMM_PHEAPNODE here;
  long deltaStart, deltaFinish;

  if (!this) { return DEBUGINT3, 0; }

  //first, determine if this resize represents growth, or shrinkage.
  deltaStart  = (this->heapstart - start);
  deltaFinish = (start + size) - (this->heapstart + this->heapsize);

  if (deltaStart) //positive deltaStart means the heap starts sooner (growth)
  {
    //if deltaStart is positive, we can just add a new node to the existing heap.
    //otherwise, we'll have to remove one or more allocated blocks.
    if (deltaStart > 0)
    {
      if (!(here = (FXMM_HEAPNODE*)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
        return DEBUGINT3, 0;

      here->size = deltaStart;
      here->offset = start;
      here->flags = 0UL;
      here->next = NULL;
      here->loan = NULL;
      here->owner = 0;

      this->heapstart = start;
      this->heapsize += deltaStart;
      fxmm_insertnode(this, here);
    }
    else 
      fxmm_resizehelper(this, deltaStart, 0);
  }

  if (deltaFinish) //positive deltaFinish means the heap ends later (growth)
  {
    //if deltaFinish is positive, we can just add a new node to the existing heap.
    //otherwise, we'll have to remove one or more allocated blocks.
    if (deltaFinish > 0)
    {
      if (!(here = (FXMM_HEAPNODE*)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE))))
        return DEBUGINT3, 0;

      here->size = deltaFinish;
      here->offset = this->heapstart+this->heapsize-1;
      here->flags = 0UL;
      here->next = NULL;
      here->loan = NULL;
      here->owner = 0;

      this->heapsize += deltaFinish;
      fxmm_insertnode(this, here);
    }
    else 
      fxmm_resizehelper(this, 0, deltaFinish);
  }

  FXMM_HEAPWATCHDOG(this);
  return 1;
}




/*----------------------------------------------------------------------
Function name:  fxmm_heapcontains()

Description:    This function returns nonzero if the given offset
                falls within the range of the 'this' heap.  This
                does not include blocks that the 'this' heap
                borrowed from other heaps.
                
Side effects:   On entry:   none.
                On exit:    free node is allocated

Return:         (int)  nonzero if true; zero otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_heapcontains(FXMM_PHEAPINFO this, U032 offset)
{
  #pragma message( "RYAN@BUG, needs to return false for offsets loaned out to other heaps." )
  return !!((offset >= this->heapstart) && (offset < (this->heapstart + this->heapsize)));
}


// ***WORK IN PROGRESS***
int fxmm_queryheap(FXMM_PHEAPINFO this, U032 num, ...)
{
  U032 i;
  int result=1;
  va_list vargs;
  U032 size, align, malign;
  U032* offsets;

  if (!(offsets = (U032*)FXMM_MALLOCFACILITY(this->context, num*sizeof(U032))))
    return DEBUGINT3, 0;

  va_start(vargs, num);
  for (i=0; i<num; i++)
  {
    size = va_arg(vargs, U032);
    align = va_arg(vargs, U032);
    malign = va_arg(vargs, U032);
    result &= fxmm_allocateblock(this, &offsets[i], size, align, malign, FXMM_FLAGS_QUERYHEAP, 0);
  }
  va_end(vargs);

  for (i=0; i<num; i++)
    result &= fxmm_freeblock(this, offsets[i]);

  return (!result ? DEBUGINT3, result : result);
}


// ***WORK IN PROGRESS***
U032 fxmm_largestfreeblock(FXMM_PHEAPINFO this, U032 size, U032 align, U032 malign)
{
  U032 slop;
  U032 largest=0;
  FXMM_PHEAPNODE here;

  align = align ? align : this->granularity;

  for (here=this->head; here; here=here->next)
  {
    if (!here->flags)
    {
      //make sure we can meet the (m)alignment requirements
      //if caller gave us a size, we can stop once we've satisfied 'size'.
      slop = (align - (here->offset % align)) % align;
      while (malign && !((here->offset+slop)%malign) && (slop < here->size) && (!size || size > here->size-slop))
        slop += align;

      largest = __max(largest, here->size-slop);
    }
  }

  return largest;
}



/*----------------------------------------------------------------------
Function name:  fxmm_heapwatchdog()

Description:    This is a debugging function that performs 4 sanity
                checks on the 'this' heap:
                  (1) summation test
                  (2) start/end drift test
                  (3) test for holes
                  (4) test for overlapping blocks
                
Side effects:   On entry:   none.
                On exit:    free node is allocated

Return:         (int)  nonzero on success
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int fxmm_heapwatchdog(FXMM_PHEAPINFO this)
{
  int result=0;
  U032 sum, start, end, numnodes;
  FXMM_PHEAPNODE here, copy, prev;
  FXMM_HEAPINFO test;

  //DANGER DANGER DANGER DANGER DANGER DANGER DANGER
  static cling=0;
  if (cling) return 1; //make sure this isn't a recursive call from 'fxmm_freeblock()'
  //DANGER DANGER DANGER DANGER DANGER DANGER DANGER

  if (!this) { return DEBUGINT3, 0; }

  //nothing to do if the heapsize is 0... but in that case, there should be no head.
  if (!this->heapsize)
      return (!this->head ? 1: DEBUGINT3, 0);

  if (!FXMM_HEAPPARANOIA) return 1;



  //TESTS (1) AND (2):  SUMMATION AND START/END DRIFTING
  here = this->head;
  sum=end=0; start=0xFFFFFFFF;
  while (here)
  {
    if (!here->loan)   //don't worry about pieces we borrowed from someone else
    {
      sum  += here->size;
      end   = __max(end, here->offset+here->size);
      start = __min(start, here->offset);
    }

    here = here->next;
  }

  if (sum - this->heapsize)                   { DEBUGINT3; return 0; }
  if (start - this->heapstart)                { DEBUGINT3; return 0; }
  if (end - (this->heapstart+this->heapsize)) { DEBUGINT3; return 0; }

  //TEST (3) AND (4): TEST FOR HOLES/OVERLAPPING
  //The premise is that, if there are any holes or overlapping nodes, then 
  //we won't be able to coalesce the list into 1 node via fxmm_freeblock().
  //These tests require a copy of the heap to manipulate, which we'll generate here.
  numnodes = 0;
  test = *this;
  here = this->head;
  prev = test.head = NULL;
  while (here)
  {
    if (!here->loan)   //disregard pieces we borrowed from someone else
    {
      numnodes += 1;
      copy = (FXMM_PHEAPNODE)FXMM_MALLOCFACILITY(this->context, sizeof(FXMM_HEAPNODE));

      if (!copy)
      {
        DEBUGINT3;  //WATCHDOG ERROR: an allocation error occurred
        goto hwd_unwinderror;
      }

      *copy = *here;
      copy->next = NULL;
      copy->flags = FXMM_FLAGS_ALLOCATED; //mark all nodes in the test heap as allocated

      *(prev ? &prev->next : &test.head) = copy;
      prev = copy;    
    }

    here = here->next;
  }

  test.heapname = FXMM_NAME_WATCHDOGHEAP;
  if (numnodes)
  {
    //now that we have a test copy of the heap, use "fxmm_freeblock()" 
    //to coalesce all the blocks in the test copy into one big honkin' block.
    for (;numnodes;numnodes--)
    {
      here = test.head;
      while (here && !here->flags)  //find the first non-free block
        here = here->next;

      if (!here)
      {
        DEBUGINT3;  //WATCHDOG ERROR: we ran out of unfree'd nodes too quickly
        goto hwd_unwinderror;
      }

      cling = 1;  //don't let freeblock call us recursively.
      fxmm_freeblock(&test, here->offset);
      cling = 0;
    }

    //now... if we have any overlapping blocks, or holes, there will be more than one node.
    if (test.head->next)
    {
      DEBUGINT3;  //oops! we either have overlapping blocks, or holes between them.
      goto hwd_unwinderror;
    }
  }

  result = 1;

hwd_unwinderror:
  here = test.head; // destroy the test copy
  while (here)
  {
    prev = here;
    here = here->next;
    FXMM_FREEFACILITY(this->context, prev);
  }

  return result;
}


