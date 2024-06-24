/* $Header: cservice.c, 19, 12/7/00 9:43:18 AM PST, Ryan Bissell$ */
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
** File name:   cservice.c
**
** Description: Central Services implementation for Win9x.
**
** $Log: 
**  19   Rampage   1.18        12/7/00  Ryan Bissell    Miscellaneous cleanup, 
**       and CS work
**  18   Rampage   1.17        11/23/00 Ryan Bissell    Lost-surface 
**       notification in Central Services, and new CS features requested by OGL 
**       Core.
**  17   Rampage   1.16        10/31/00 Ryan Bissell    Phase I of development 
**       of the "No Man's Land" reclamation heap.
**  16   Rampage   1.15        10/20/00 Ryan Bissell    Miscellaneous bug fixes 
**       (CSIM Server, csLock, FXWAITFORIDLE)
**  15   Rampage   1.14        10/17/00 Ryan Bissell    Fixed tile mode bug, and
**       cleaned up size calculation code.
**  14   Rampage   1.13        10/2/00  Ryan Bissell    Various bug fixes (AA, 
**       Stagger Mode)
**  13   Rampage   1.12        8/28/00  Ryan Bissell    Fixed FSAA oversight in 
**       cs_SwapBufferToDisplay(), and added initial support for staggered 
**       surfaces.
**  12   Rampage   1.11        8/24/00  Ryan Bissell    Initial stab at FSAA 
**       support in Central Services.
**  11   Rampage   1.10        8/21/00  Ryan Bissell    Changed units of 
**       u32PhysicalStride to tiles for tiled surfaces
**  10   Rampage   1.9         8/15/00  Ryan Bissell    Retail build was failing
**       due to an "internal compiler error."
**  9    Rampage   1.8         8/4/00   Ryan Bissell    Added support for true 
**       AGP command FIFOs, and long JSR support in Central Services (needed 
**       because of AGP command FIFOs.)
**  8    Rampage   1.7         7/21/00  Ryan Bissell    Removed work-around for 
**       bug fix in CS test application.
**  7    Rampage   1.6         7/14/00  Ryan Bissell    Sentinel buffer support
**  6    Rampage   1.5         7/13/00  Ryan Bissell    Beginnings of D3D / 
**       Central Services cooperation
**  5    Rampage   1.4         6/26/00  Ryan Bissell    Incremental CS 
**       development.  Also removed some message pragmas, in anticipation of CS 
**       activation.
**  4    Rampage   1.3         6/12/00  Ryan Bissell    Removed guard in 
**       cs_Initialize() that disallowed use of Central Services interface for 
**       builds that don't have CSERVICE defined.
**  3    Rampage   1.2         5/3/00   Ryan Bissell    Continued deployment of 
**       Central Services and related changes.
**  2    Rampage   1.1         4/14/00  Ryan Bissell    Central Services 
**       deployment
**  1    Rampage   1.0         3/6/00   Ryan Bissell    
** $
**
*/

#include "header.h"
#include <windows.h>
#include "wownt16.h"
#include "cservice.h"
#include <olenls.h>

#define FXPRIVATE static

#undef DEBUGINT3
__inline void cs_debugint3() { __asm int 3 }   //lets us do things like "return DEBUGINT3, 0;"
#define DEBUGINT3 cs_debugint3()

#define FXEMIT66   _asm _emit 0x66
#define FXEMIT67 _asm _emit 0x67

static UINT FlatSel;

extern UINT GetFlatSel(void);
extern GLOBALDATA* lpDriverData;

#pragma message("RYAN@TODO, Me thinks this is bad news. Find a better solution.")
#define DDFXS32_DLLNAME "3dfx32vx.dll"

#include <camdefs.h>

#define __min(x,y) ((x) < (y) ? (x) : (y))
#define __max(x,y) ((x) > (y) ? (x) : (y))

#pragma message("RYAN@TODO, Make macros for all constants.")


void FXWAITFORIDLE();


FXPRIVATE CS9X_PCONTEXTNODE cs9x_AllocateContextNode()
{
  HGLOBAL hMemory;
  CS9X_PCONTEXTNODE node;

  hMemory = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, sizeof(CS9X_CONTEXTNODE));
  if (hMemory == 0UL) return DEBUGINT3, NULL;

  node = (CS9X_PCONTEXTNODE)GlobalLock(hMemory);
  if (node == NULL) return DEBUGINT3, NULL;

  memset(node, 0, sizeof(*node));
  node->signature = ~(FxU32)node; //sanity checking value

  node->prev = NULL;
  node->next = (CS9X_PCONTEXTNODE)_FF(csContextList);
  _FF(csContextList) = (DWORD)node;
  if (node->next)
  {
    node->prev = node->next->prev;
    node->next->prev = node;
  }

  return node;
}



FXPRIVATE void cs9x_RemoveContextNode(CS9X_PCONTEXTNODE context)
{
  HGLOBAL hMemory;

  if (!context) { DEBUGINT3;  return; }

  if (_FF(csContextList) == (DWORD)context)
    _FF(csContextList) = (DWORD)context->next;

  if (context->next)
    context->next->prev = context->prev;

  if (context->prev)
    context->prev->next = context->next;

  hMemory = (HGLOBAL)GlobalHandle((UINT)((DWORD)context >> 16));
  GlobalUnlock(hMemory);
  GlobalFree(hMemory);
}



FXPRIVATE CS9X_PRESOURCENODE cs9x_AllocateResourceNode(CS9X_PCONTEXTNODE context)
{
  HGLOBAL hMemory;
  CS9X_PRESOURCENODE node;

  hMemory = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, sizeof(CS9X_RESOURCENODE));
  if (hMemory == 0UL) return DEBUGINT3, NULL;

  node = (CS9X_PRESOURCENODE)GlobalLock(hMemory);
  if (node == NULL) return DEBUGINT3, NULL;

  memset(node, 0, sizeof(*node));
  node->signature = ~(FxU32)node; //sanity checking value

  node->prev = NULL;
  node->next = context->list;
  context->list = node;
  if (node->next)
  {
    node->prev = node->next->prev;
    node->next->prev = node;
  }

  node->context = context;
  return node;
}


FXPRIVATE void cs9x_RemoveResourceNode(CS9X_PRESOURCENODE resource)
{
  HGLOBAL hMemory;
  CS9X_PRESOURCENODE node;

  if (!resource || !resource->context) { DEBUGINT3; return; }

  if (resource->context->list == resource)
    resource->context->list = resource->next;

  if (resource->next)
    resource->next->prev = resource->prev;

  if (resource->prev)
    resource->prev->next = resource->next;

  //if this was a lost resource, decrement lost-resource count
  if (resource->state & CS9X_STATE_RESOURCELOST)
  {
    if (!(resource->context->lost -= (resource->context->lost ? 1 : 0)))
      resource->context->state &= (CS9X_STATE_RESOURCELOST | CS9X_STATE_COMPLETELOSS);
  }

  hMemory = (HGLOBAL)GlobalHandle((UINT)((DWORD)resource >> 16));
  GlobalUnlock(hMemory);
  GlobalFree(hMemory);
}



FXPRIVATE CSRESULT cs9x_VerifyContextState(CS9X_PCONTEXTNODE context, CS9X_PRESOURCENODE resource, int reset)
{
  CS9X_PRESOURCENODE res;

  if (!context) return DEBUGINT3, CS_SERVERERROR_INTERNALERROR;

  if (context->state & CS9X_STATE_COMPLETELOSS)
  {
    if (reset)
    {
      for (res=context->list; res; )
        cs9x_RemoveResourceNode(res);

      context->lost = 0;
      context->state &= ~(CS9X_STATE_COMPLETELOSS | CS9X_STATE_RESOURCELOST);
    }
    return CS_APIERROR_ALLSURFACESLOST;
  }

 
  if (resource && (resource->state & CS9X_STATE_RESOURCELOST))
  {
    if (reset)
    {
      context = resource->context;
      cs9x_RemoveResourceNode(resource);
      context->lost = (context->lost ? context->lost-1 : 0);
      if (!context->lost)
        context->state &= ~CS9X_STATE_RESOURCELOST;
    }

    return CS_APIERROR_SURFACELOST;
  }

  return 0;
}
 


CSRESULT cs9x_GetProtocolRevision( FxU32 FxFAR*pu32Major, FxU32 FxFAR*pu32Minor )
{
  #pragma message("RYAN@???, Do we really need to duplicate all error codes?")
  if (!(pu32Major && pu32Minor))
    return CS_APIERROR_INVALIDPARAM;

  *pu32Major = CS_PROTOCOL_MAJOR;
  *pu32Minor = CS_PROTOCOL_MINOR;

  return CS_SUCCESS;
}



CSRESULT cs9x_GetGraphicalContext( PCSGRAPHICALCONTEXT psGraphicalContextReq, PCSGRAPHICALCONTEXT psGraphicalContextRes )
{
  CS9X_PCONTEXTNODE ctx;

  if (!(psGraphicalContextReq && psGraphicalContextRes))
    return CS_APIERROR_INVALIDPARAM;

  //the obligatory copy from req to res
  memcpy(psGraphicalContextRes, psGraphicalContextReq, sizeof(CSGRAPHICALCONTEXT));
  memset(&psGraphicalContextRes->sChipSpecificData, 0, sizeof(psGraphicalContextRes->sChipSpecificData));

  ctx = cs9x_AllocateContextNode();
  if (!ctx) return DEBUGINT3, CS_APIERROR_IMPOSSIBILITY;

  psGraphicalContextRes->sDeviceConfig.u32Flags = 0UL;
  psGraphicalContextRes->sDeviceConfig.u32DeviceID = _FF(VendorDeviceID);
  psGraphicalContextRes->sChipSpecificData.u32ChipType = CSCHIPSPECIFICDATA_CHIPTYPE_SST2;

#ifdef AGP_EXECUTE
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32AGPRam = _FF(agpHeapSize)*1024*1024;
#else
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32AGPRam = 0;
#endif
  
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32LFBRam = _FF(TotalVRAM);
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32DeviceRev = _FF(RevisionID);

#ifdef SLI
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32NumSLIChips = _FF(dwNumSlaves) +1;
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32SLIAvailable = (_FF(dwNumUnits) > _FF(dwNumMasters));
#else
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32NumSLIChips = 0;
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32SLIAvailable = FALSE;
#endif

  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.pvIOBase = (PFxVOID)_FF(ioBase);
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.pvMemBase0 = (PFxVOID)_FF(regBase);
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.pvMemBase1 = (PFxVOID)_FF(lfbBase);

  //assign a unique ID for this context.
  _FF(csNewestContextID) += 1;
  psGraphicalContextRes->u32ContextID = _FF(csNewestContextID);

  ctx->id = psGraphicalContextRes->u32ContextID;
  psGraphicalContextRes->sChipSpecificData.unionChipType.sSST2.u32Reserved[0] = (FxU32)ctx;

  return CS_SUCCESS;
}



FxU32 GCD(FxU32 a, FxU32 b)  //find greatest common divisor
{
  FxU32 r;

  if (!b) return a;

  do  //euclid's GCD algorithm
  {
    r = a % b;    //compute remainder
    a = b;        //divisor becomes dividend
    b = r;        //remainder becomes divisor
  } while (r);

  return a;       //return last used divisor
}


//least common multiple
#define LCM(a,b) ((a)*(b)/GCD((a),(b)))


CSRESULT cs9x_Alloc( PCSGRAPHICALCONTEXT psGraphicalContext, PCSALLOCATIONDESCRIPTOR psAllocationDescriptorIn,  PCSALLOCATIONDESCRIPTOR psAllocationDescriptorOut, FxU32 bOrBiggestAvailable)
{
  CSRESULT ret;
  FxU32 tileWidth;
  FxU32 tileHeight;
  FxU32 sizePhysical;
  FxU32 surfAlignment=1;  //LCM of alignments we favor
  FxU32 surfMalignment=0; //RYAN@PUN, an alignment we avoid with extreme prejudice.
  FxU32 stridePhantom;    //minimum phantom stride allowed by the hardware
  FxU32 stridePhysical;
  FxU32 heightPhysical;
  PCSCHIPSPECIFICDATA  psChipSpecificData;
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE resource;

  if (!_FF(csSurfaceAllocator))
    return CS_APIERROR_DIRECTDRAWFAILED;

  //the obligatory copy from req to res
  memcpy(psAllocationDescriptorOut, psAllocationDescriptorIn, sizeof(CSALLOCATIONDESCRIPTOR));
  psChipSpecificData = &psAllocationDescriptorOut->sChipSpecificData;
  sizePhysical = psAllocationDescriptorOut->u32Size;

  ctx = (CS9X_PCONTEXTNODE)psGraphicalContext->sChipSpecificData.unionChipType.sSST2.u32Reserved[0];
  if ((FxU32)ctx != ~(FxU32)ctx->signature)
    return DEBUGINT3, CS_APIERROR_IMPOSSIBILITY;

  if (ret = cs9x_VerifyContextState(ctx, NULL, 0))
    return ret;

  if (!(resource = cs9x_AllocateResourceNode(ctx)))
    return DEBUGINT3, CS_APIERROR_IMPOSSIBILITY;

  //if the client wants a FIFO buffer, force it to the right locale.
  if (psAllocationDescriptorOut->u32BufferType == CS_BUFFER_FIFO)
    psAllocationDescriptorOut->u32Locale = _FF(doAgpCF) ? CS_LOCALE_AGP : CS_LOCALE_FRAMEBUFFER;

  //the test app isn't setting the locale in some cases... so force 'em to fix it.  :^)
  if (!psAllocationDescriptorOut->u32Locale) return CS_APIERROR_INVALIDPARAM;

  //make sure "u32NumSamples" is kosher
  switch (psAllocationDescriptorOut->u32NumSamples)
  {
    case 0:
    case 1:
      psAllocationDescriptorOut->u32NumSamples = 1;
      break;

    case 4:
      switch (psAllocationDescriptorOut->u32BufferType)
      {
        case CS_BUFFER_RENDER:
        case CS_BUFFER_ZBUFFER:
          break;

        default:
          return CS_APIERROR_INVALIDPARAM;
      }
      break;

    default:
        return CS_APIERROR_INVALIDPARAM;
  }

  switch (psAllocationDescriptorOut->u32MemType)
  {
    case CS_MEMORY_TILED:
      switch (psAllocationDescriptorOut->u32BufferType)
      {
        case CS_BUFFER_RENDER:
          surfAlignment = LCM(surfAlignment, 16*1024);  //3D render buffers aligned on an even 8KB boundary
          psChipSpecificData->unionChipType.sSST2.u32TileMode = 1;
          break;

        case CS_BUFFER_ZBUFFER:
          surfAlignment  = LCM(surfAlignment,  8*1024); //3D z buffers aligned on an odd 8KB boundary
          surfMalignment = 16*1024;                     //...which means not a multiple of 16K
          psChipSpecificData->unionChipType.sSST2.u32TileMode = 1;
          break;

        case CS_BUFFER_TEXTURE:
        case CS_BUFFER_TEXTUREHEAP:
          surfAlignment = LCM(surfAlignment, 128);      //textures should be 128-byte aligned
          psChipSpecificData->unionChipType.sSST2.u32TileMode = 0;  //RYAN@TEMP, this will eventually be dynamic for textures
          break;
      }

      //in any case, a tiled surface must be 128 byte aligned (redundant, but still...)
      surfAlignment = LCM(surfAlignment, 128);
      
      //(in BYTES) see page 7 of "SST-2 Memory Organization Specification".
      tileWidth = 32;
      tileHeight = (psChipSpecificData->unionChipType.sSST2.u32TileMode ? 32 : 8) * ((psAllocationDescriptorOut->u32Depth == 4) ? 2 : 1);

      //(in BYTES) round surface stride up to the next multiple of tileWidth, if necessary.
      stridePhysical = psAllocationDescriptorOut->u32Width * psAllocationDescriptorOut->u32NumSamples * psAllocationDescriptorOut->u32Depth/8;
      stridePhysical += ((tileWidth - (stridePhysical % tileWidth)) % tileWidth);

      //(in SCANLINES) round surface height up to the next multiple of tileHeight, if necessary.
      heightPhysical = psAllocationDescriptorOut->u32Height;
      heightPhysical += ((tileHeight - (heightPhysical % tileHeight)) % tileHeight);

      //(in BYTES) figure out exactly how much space this surface'll take up.
      sizePhysical = stridePhysical * heightPhysical;

      psChipSpecificData->unionChipType.sSST2.u32TileWidth = tileWidth / (psAllocationDescriptorOut->u32Depth/8);
      psChipSpecificData->unionChipType.sSST2.u32TileHeight = tileHeight;
      break;

    case CS_MEMORY_LINEAR:
      switch (psAllocationDescriptorOut->u32BufferType)
      {
        case CS_BUFFER_FIFO:        //on Rampage, FIFO is just a special case of PERSISTENT.
          sizePhysical += 32;       //add a little room for the server-appended RET before we allocate
          surfAlignment = LCM(surfAlignment, 4*1024);    //Rampage doc says FIFOs must be 4K aligned.
          break;

        case CS_BUFFER_PERSISTENT:  //could be used for sentinel buffers
          surfAlignment = LCM(surfAlignment, 4*1024);
          break;
      }

      //for linear surfaces, the start, stride, and size must be a multiple of the pixel depth.
      //but for simplicity, we'll always assume pixel depth is 32, which is OK because 24bpp isn't packed.
      sizePhysical += ((32 - (sizePhysical % 32)) % 32);

      //(in BYTES) since this is linear memory, we want the stride to be the size of the surface.
      stridePhysical = sizePhysical;
      break;
  }


  //for any lockable surface, the alignment must be a multiple of 32 (redundant, but still...)
  surfAlignment = LCM(surfAlignment, 32);  //least common multiple
  
  ret = CallProcEx32W(11, 0x00000111, _FF(csSurfaceAllocator), lpDriverData,
                                                              surfAlignment,
                                                              surfMalignment,
                                                              sizePhysical,
                                                             &stridePhysical,
                                                              psAllocationDescriptorOut->u32Locale,
                                                              psAllocationDescriptorOut->u32MemType,
                                                              psAllocationDescriptorOut->u32BufferType,
                                                             &psAllocationDescriptorOut->u32PhysicalOffset,
                                                              resource,
                                                              bOrBiggestAvailable);

  if (ret != CS_SUCCESS)
  {
    cs9x_RemoveResourceNode(resource);
    return ret;
  }

  //(in BYTES) precalculate the phantom stride needed (will be used at csLock time)  
  //Note that 2^5 is the minimum allowed value.
  stridePhantom = 16;
  do { stridePhantom <<= 1; } while (stridePhantom < stridePhysical);

  //(convert stridePhysical to TILES, for tiled surfaces)
  if (CS_MEMORY_TILED == psAllocationDescriptorOut->u32MemType)
      stridePhysical /= tileWidth;

  //store off the size and stride information, for use at csLock time
  psAllocationDescriptorOut->u32Size = sizePhysical;
  psAllocationDescriptorOut->u32LinearStride = stridePhantom;
  psAllocationDescriptorOut->u32PhysicalStride = stridePhysical;

  //don't let the client know we grew this buffer by 32 bytes.
  if (CS_BUFFER_FIFO == psAllocationDescriptorOut->u32BufferType)
    psAllocationDescriptorOut->u32Size -= 32;

  psAllocationDescriptorOut->u32BufferID = (DWORD)resource;
  resource->type = psAllocationDescriptorOut->u32BufferType;
  resource->locale = psAllocationDescriptorOut->u32Locale;

  return CS_SUCCESS;
}


CSRESULT cs9x_Free( PCSGRAPHICALCONTEXT psGraphicalContext, PCSALLOCATIONDESCRIPTOR psAllocationDescriptorIn,  PCSALLOCATIONDESCRIPTOR psAllocationDescriptorOut )
{
  CSRESULT ret;
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE res;

  if (!_FF(csSurfaceLiberator))
    return CS_APIERROR_DIRECTDRAWFAILED;

  //the obligatory copy from req to res
  memcpy(psAllocationDescriptorOut, psAllocationDescriptorIn, sizeof(CSALLOCATIONDESCRIPTOR));
  
  if (psAllocationDescriptorOut->u32LockCount)
    return CSFREE_APIERROR_STILLLOCKED;

  ctx = (CS9X_PCONTEXTNODE)psGraphicalContext->sChipSpecificData.unionChipType.sSST2.u32Reserved[0];
  res = (CS9X_PRESOURCENODE)psAllocationDescriptorOut->u32BufferID;
  if (!(ctx && res)) return DEBUGINT3, CS_SERVERERROR_INTERNALERROR;

  if (ret = cs9x_VerifyContextState(ctx, NULL, 0))
    return ret;

  ret = CallProcEx32W(4, 0x00000001, _FF(csSurfaceLiberator),  lpDriverData,
                                                               psAllocationDescriptorOut->u32Locale,
                                                               psAllocationDescriptorOut->u32BufferType,
                                                               psAllocationDescriptorOut->u32PhysicalOffset );


  memset(psAllocationDescriptorIn, 0, sizeof(*psAllocationDescriptorIn));
  cs9x_RemoveResourceNode(res);
  return ret;
}


CSRESULT cs9x_Lock( PCSGRAPHICALCONTEXT psGraphicalContext, PCSALLOCATIONDESCRIPTOR psAllocationDescriptorIn, PCSALLOCATIONDESCRIPTOR psAllocationDescriptorOut )
{
  CSRESULT ret;
  FxU32 camFlags = (CAM_ALLOC_PHANTOM_MEM | CAM_ALLOC_CAM_ENTRY);
  FxU32 physStride;
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE res;

  CMDFIFO_PROLOG(cf);
  CMDFIFO_SETUP(cf);

  if (!_FF(csCamAllocator))
    return CS_APIERROR_DIRECTDRAWFAILED;

  //the obligatory copy from req to res
  memcpy(psAllocationDescriptorOut, psAllocationDescriptorIn, sizeof(CSALLOCATIONDESCRIPTOR));

  ctx = (CS9X_PCONTEXTNODE)psGraphicalContext->sChipSpecificData.unionChipType.sSST2.u32Reserved[0];
  res = (CS9X_PRESOURCENODE)psAllocationDescriptorOut->u32BufferID;
  if (!(ctx && res)) return DEBUGINT3, CS_SERVERERROR_INTERNALERROR;

  if (ret = cs9x_VerifyContextState(ctx, res, 0))
    return ret;

  if (!psAllocationDescriptorOut->u32Depth)
    psAllocationDescriptorOut->u32Depth = 32;

  if (!psAllocationDescriptorOut->u32LFBDepth)
    psAllocationDescriptorOut->u32LFBDepth = psAllocationDescriptorOut->u32Depth;

  physStride   = psAllocationDescriptorOut->u32PhysicalStride;

  switch (psAllocationDescriptorOut->u32MemType)
  {
    case CS_MEMORY_LINEAR:
      camFlags |= CAM_PURE_LINEAR;
      break;

    case CS_MEMORY_TILED:
      camFlags |= CAM_TILED;
      camFlags |= (psAllocationDescriptorOut->sChipSpecificData.unionChipType.sSST2.u32TileMode ? CAM_TILE_MODE_1 : 0);
      //we reported pStride to client in tiles, but the cam allocator requires it in bytes (always).
      //physStride *= (psAllocationDescriptorOut->u32TileWidth * psAllocationDescriptorOut->u32Depth/8);
      physStride *= 32;
      break;

    default:
      return CS_APIERROR_INVALIDPARAM;
  }

  //halve the physical stride if the surface is AA.
  physStride >>= !!(psAllocationDescriptorOut->u32NumSamples >> 1);

  //turn on the AA flag if the surface is AA.
  camFlags    |= CAM_EN_AA * !!(psAllocationDescriptorOut->u32NumSamples >> 1);
  
  //turn on STAGGER flag if the surface is staggered.
  camFlags    |= CAM_EN_STAGGER * !!psAllocationDescriptorOut->u32Staggering;

  switch (psAllocationDescriptorOut->u32BufferType)
  {
    
    case CS_BUFFER_FIFO:  //don't need a CAM for this
      if (psAllocationDescriptorOut->u32Locale == CS_LOCALE_AGP)
      {
        psAllocationDescriptorOut->pvLinearAddress = (PFxVOID)(psAllocationDescriptorOut->u32PhysicalOffset
                                                   - (FxU32)_FF(agpHeapPhysBaseAddr)
                                                   + (FxU32)_FF(agpHeapLinBaseAddr));
        break;
      }
      //intentional fall-through

    case CS_BUFFER_PERSISTENT:  //don't need a CAM for this
      psAllocationDescriptorOut->pvLinearAddress = (PFxVOID)((FxU32)_FF(LFBBASE) + psAllocationDescriptorOut->u32PhysicalOffset);
      break;

    default:
      CMDFIFO_CHECKROOM(cf, 2);
      SETPH( cf, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, mopCMD));
      SETPD( cf, FBI->mopCMD,   SST_MOP_STALL_3D 
                              | SST_MOP_FLUSH_TCACHE 
                              | SST_MOP_FLUSH_PCACHE 
                              |(SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT) );
      BUMP(2);
      FXWAITFORIDLE();
      if (!CallProcEx32W(8, 0x00000081, _FF(csCamAllocator),  lpDriverData,
                                                              camFlags,
                                                              psAllocationDescriptorOut->u32PhysicalOffset,
                                                              psAllocationDescriptorOut->u32Size,
                                                              physStride,
                                                              psAllocationDescriptorOut->u32Depth,
                                                              psAllocationDescriptorOut->u32LFBDepth,
                                                             &psAllocationDescriptorOut->pvLinearAddress))
      {
        CMDFIFO_EPILOG(cf);
        return CSLOCK_APIERROR_NOLOCKAVAILABLE;
      }
      break;
  }

  //oops-- I don't have a convenient way to get this information, but the client doesn't really need it anyway.
  psAllocationDescriptorOut->sChipSpecificData.unionChipType.sSST2.u32CAMEntry = 1;

  CMDFIFO_EPILOG(cf);
  return CS_SUCCESS;
}



CSRESULT cs9x_Unlock( PCSGRAPHICALCONTEXT psGraphicalContext, PCSALLOCATIONDESCRIPTOR psAllocationDescriptorIn,  PCSALLOCATIONDESCRIPTOR psAllocationDescriptorOut )
{
  CSRESULT ret;
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE res;

  if (!_FF(csSurfaceLiberator))
    return CS_APIERROR_DIRECTDRAWFAILED;

  //the obligatory copy from req to res
  memcpy(psAllocationDescriptorOut, psAllocationDescriptorIn, sizeof(CSALLOCATIONDESCRIPTOR));

  ctx = (CS9X_PCONTEXTNODE)psGraphicalContext->sChipSpecificData.unionChipType.sSST2.u32Reserved[0];
  res = (CS9X_PRESOURCENODE)psAllocationDescriptorOut->u32BufferID;
  if (!(ctx && res)) return DEBUGINT3, CS_SERVERERROR_INTERNALERROR;

  if (ret = cs9x_VerifyContextState(ctx, res, 0))
    return ret;

  switch (psAllocationDescriptorOut->u32BufferType)
  {
    case CS_BUFFER_FIFO:  //didn't use a CAM for these
    case CS_BUFFER_PERSISTENT:
      psAllocationDescriptorOut->pvLinearAddress = (PFxVOID)0UL;
      break;

    default:
      if (!CallProcEx32W(2, 0x00000001, _FF(csCamLiberator),  lpDriverData,
                                                              psAllocationDescriptorOut->pvLinearAddress))
      {
        return CSUNLOCK_APIERROR_NOTLOCKED;
      }
      break;
  }

  psAllocationDescriptorOut->sChipSpecificData.unionChipType.sSST2.u32CAMEntry = 0;
  return CS_SUCCESS;
}


#define R32(p1, p2) ((DWORD)(p1) << 16 | ((DWORD)(p2) & 0xffff))


CSRESULT cs9x_ExecuteCommands(PCSGRAPHICALCONTEXT psGraphicalContext, PCSEXECUTECOMMANDSREQ psExecuteCommandsReq, PCSEXECUTECOMMANDSRES psExecuteCommandsRes)
{
  CSRESULT ret;
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE res;

  FxU32 mutex0;
  FxU32 foobar;
  FxU32 bumpSize;
  FxU32 retCommand;
  FxU32 retLocation;
  FxU32 commandWord1;
  FxU32 commandWord2;
  PCSALLOCATIONDESCRIPTOR psSentinel;

  CMDFIFO_PROLOG(cf);
  CMDFIFO_SETUP(cf);


  //the obligatory copy from req to res
  memcpy(psExecuteCommandsRes, psExecuteCommandsReq, sizeof(CSEXECUTECOMMANDSREQ));

  //just check the context.  No need to check the command buffers, because persistent
  //buffers are guaranteed never to be lost.
  ctx = (CS9X_PCONTEXTNODE)psGraphicalContext->sChipSpecificData.unionChipType.sSST2.u32Reserved[0];
  if (ret = cs9x_VerifyContextState(ctx, NULL, 1))
    return ret;

  //important sanity check
  switch (psExecuteCommandsReq->sStateAllocationDescriptor.u32Locale)
  {
    case CS_LOCALE_AGP:
    case CS_LOCALE_FRAMEBUFFER:
      break;

    default:
      return CS_APIERROR_INVALIDPARAM;
  }


  //if the state is dirty, complain if no state given
  if (_FF(csLastContextID) != psGraphicalContext->u32ContextID)
    if (!psExecuteCommandsReq->u32StateSize)
      return CSEXECUTEBUF_APIERROR_STATELOST;

  //the caller may want us to manage state at a thread-level.
  if (ctx->thread != psExecuteCommandsReq->idThread)
    if (!psExecuteCommandsReq->u32StateSize)
      return CSEXECUTEBUF_APIERROR_STATELOST;

  #pragma message( "This exists to work around a bug in CSIM Server.  It won't work on real SAGE!!!" )
  #define CS_JSRFIXUP (!_FF(isHardware) && IS_SAGE_ACTIVE)
  #define CS_JSROFFSET(adesc) *(CS_JSRFIXUP ? &((FxU32)(adesc).pvLinearAddress) : &((adesc).u32PhysicalOffset))

  //if we were given state information, restore to the given state
  if (psExecuteCommandsReq->u32StateSize)
  {
    switch (psExecuteCommandsReq->sStateAllocationDescriptor.u32Locale)
    {
      case CS_LOCALE_AGP:
        commandWord2 = CS_JSROFFSET(psExecuteCommandsReq->sStateAllocationDescriptor) 
                     + psExecuteCommandsReq->u32StateOffset;
        commandWord2 >>= 26;
        //intentional fall-through

      case CS_LOCALE_FRAMEBUFFER:
        commandWord1 = CS_JSROFFSET(psExecuteCommandsReq->sStateAllocationDescriptor) 
                     + psExecuteCommandsReq->u32StateOffset;
        //commandWord1 >>= 2; //lose the lower 2 bits
        commandWord1 <<= (SSTCP_PKT0_ADDR_SHIFT -2);
        commandWord1 &=  SSTCP_PKT0_ADDR;                                              
        commandWord1 |=  SSTCP_PKT0 | (SSTCP_PKT0_JSR << SSTCP_PKT0_FUNC_SHIFT);

        //add the return code to the end of the state buffer
        retCommand = ((SSTCP_PKT0_RET << SSTCP_PKT0_FUNC_SHIFT) | SSTCP_PKT0);

        //need to write the return command to the end of the state buffer. This should be at 
        //LFBpointer + u32PhysicalOffset	+ u32StateOffset + u32StateSize( in bytes )
        retLocation = (FxU32)0
                    + CS_JSROFFSET(psExecuteCommandsReq->sStateAllocationDescriptor)
                    + psExecuteCommandsReq->u32StateOffset
                    + psExecuteCommandsReq->u32StateSize;
        break;
    }

    //final fixups based on locale
    switch (psExecuteCommandsReq->sStateAllocationDescriptor.u32Locale)
    {
      case CS_LOCALE_AGP:
        bumpSize = 2;
        retLocation -= CS_JSRFIXUP ? 0 : _FF(agpHeapPhysBaseAddr);
        retLocation += CS_JSRFIXUP ? 0 : _FF(agpHeapLinBaseAddr);
        commandWord1 |= SSTCP_PKT0_LONGADDR;
        break;

      default:
        bumpSize = 1;
        retLocation += _FF(LFBBASE);
    }

    h3WRITE((DWORD FAR*)NULL, (DWORD FAR*)retLocation, (DWORD)retCommand);

    P6FENCE;
    CMDFIFO_CHECKROOM(cf, bumpSize);
    SETPH(cf,                     commandWord1);
    if (bumpSize == 2)  SETPH(cf, commandWord2);
    BUMP(bumpSize);
    P6FENCE;
  }


  if (psExecuteCommandsReq->u32CommandSize)
  {
    switch (psExecuteCommandsReq->sCommandAllocationDescriptor.u32Locale)
    {
      case CS_LOCALE_AGP:
        commandWord2 = CS_JSROFFSET(psExecuteCommandsReq->sCommandAllocationDescriptor)
                     + psExecuteCommandsReq->u32CommandOffset;
        commandWord2 >>= 26;
        //intentional fall-through

      case CS_LOCALE_FRAMEBUFFER:
        commandWord1 = CS_JSROFFSET(psExecuteCommandsReq->sCommandAllocationDescriptor) 
                     + psExecuteCommandsReq->u32CommandOffset;
        //commandWord1 >>= 2; //lose the lower 2 bits
        commandWord1 <<= (SSTCP_PKT0_ADDR_SHIFT -2);
        commandWord1 &=  SSTCP_PKT0_ADDR;                                              
        commandWord1 |=  SSTCP_PKT0 | (SSTCP_PKT0_JSR << SSTCP_PKT0_FUNC_SHIFT);

        //add the return code to the end of the command buffer
        retCommand = ((SSTCP_PKT0_RET << SSTCP_PKT0_FUNC_SHIFT) | SSTCP_PKT0);

        //need to write the return command to the end of the command buffer. This should be at 
        //LFBpointer + u32PhysicalOffset	+ u32CommandOffset + u32CommandSize( in bytes )
        retLocation = (FxU32)0
                    + CS_JSROFFSET(psExecuteCommandsReq->sCommandAllocationDescriptor)
                    + psExecuteCommandsReq->u32CommandOffset
                    + psExecuteCommandsReq->u32CommandSize;
        break;
    }

    //final fixups based on locale
    switch (psExecuteCommandsReq->sCommandAllocationDescriptor.u32Locale)
    {
      case CS_LOCALE_AGP:
        bumpSize = 2;
        retLocation -= CS_JSRFIXUP ? 0 : _FF(agpHeapPhysBaseAddr);
        retLocation += CS_JSRFIXUP ? 0 : _FF(agpHeapLinBaseAddr);
        commandWord1 |= SSTCP_PKT0_LONGADDR;
        break;

      default:
        bumpSize = 1;
        retLocation += _FF(LFBBASE);
    }

    h3WRITE((DWORD FAR*)NULL, (DWORD FAR*)retLocation, (DWORD)retCommand);

    P6FENCE;
    CMDFIFO_CHECKROOM(cf, bumpSize);
    SETPH(cf,                     commandWord1);
    if (bumpSize == 2)  SETPH(cf, commandWord2);
    BUMP(bumpSize);
    P6FENCE;
  }


  psSentinel = &psExecuteCommandsRes->sSentinelAllocationDescriptor;

  P6FENCE;
  CMDFIFO_CHECKROOM(cf, 16);

  //start with a STALL_ALL which will stall any pending 2D or 3D operations because we
  //have no idea what operation was performed last
  SETPH( cf, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, mopCMD));
  SETPD( cf, FBI->mopCMD, SST_MOP_STALL_3D 
                             | SST_MOP_FLUSH_TCACHE 
                             | SST_MOP_FLUSH_PCACHE 
                             |(SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT) );


  SETPH(cf, (SSTCP_PKT2 | dstBaseAddrBit 
                        | dstFormatBit 
                        | clip1minBit
                        | clip1maxBit
                        | colorForeBit
                        | dstSizeBit 
                        | dstXYBit 
                        | commandBit));

#define CS_LINEARDSTFMT(stride, bpp)   (stride & SST_WX_DST_STRIDE)        \
                                     | ((bpp) << SST_WX_DST_FORMAT_SHIFT)  \
                                     | (SST_WX_DST_LINEAR)

  SET( cf, _FF(lpGRegs)->dstBaseAddr,  psSentinel->u32PhysicalOffset);
  SET( cf, _FF(lpGRegs)->dstFormat,    CS_LINEARDSTFMT(4, SST_WX_PIXFMT_32BPP) );
  SET( cf, _FF(lpGRegs)->clip1minBit,  0x00000000 );
  SET( cf, _FF(lpGRegs)->clip1maxBit,  0x0FFF0FFF );
  SET( cf, _FF(lpGRegs)->colorFore,    psExecuteCommandsRes->u32SentinelSerial );
  SET( cf, _FF(lpGRegs)->dstSize,      R32(1, 1));
  SET( cf, _FF(lpGRegs)->dstXY,        R32(0, 0));
  SETC(cf, _FF(lpGRegs)->command,   (  SST_WX_ROP_SRC    << SST_WX_ROP0_SHIFT) 
                                     | SST_WX_CLIPSELECT //Win9x uses clipSelect=0 for the screen clip rect
                                     | SST_WX_RECTFILL 
                                     | SST_WX_GO);

  //restore dstBaseAddr and dstFormat
  SETPH(cf, (SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit));
  SET(  cf, _FF(lpGRegs)->dstBaseAddr,  _FF(gdiDesktopStart));
  SET(  cf, _FF(lpGRegs)->dstFormat,    _FF(ddPrimarySurfaceData.dwFormat));


  // End with a STALL_2D because our last operation was a 2D one
  SETPH( cf, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, mopCMD));
  SETPD( cf, FBI->mopCMD, SST_MOP_STALL_2D );

  BUMP(16);
  P6FENCE;
  CMDFIFO_EPILOG(cf);

  _FF(csLastContextID) = psGraphicalContext->u32ContextID;
  ctx->thread = psExecuteCommandsReq->idThread;
  return CS_SUCCESS;
}




FXPRIVATE FxU32 cs9x_MakeSrcFormat2D(PCSALLOCATIONDESCRIPTOR pBufferInfo)
{
  FxU32 value;
  FxU32 stride;

  //RYAN@NOTE, Note: this code doesn't support the YUV formats.
  //           If we end up needing to support them, this must change.

  //convert the bpp value into the srcFormat[19:16] value
  switch (pBufferInfo->u32ColorFormat)
  {
    case CS_COLORFORMAT_8BPP:     value = 1; break;
    case CS_COLORFORMAT_RGB1555:  value = 2; break;
    case CS_COLORFORMAT_RGB565:   value = 3; break;
    case CS_COLORFORMAT_RGB32:    value = 5; break;
    case CS_COLORFORMAT_YUYV:     value = 8; break;
    case CS_COLORFORMAT_UYVY:     value = 9; break;
  }
  value <<= SST_WX_SRC_FORMAT_SHIFT;

  stride = pBufferInfo->u32PhysicalStride;

  //or in the surface type
  switch (pBufferInfo->u32MemType)
  {
    case CS_MEMORY_TILED:
      value |= (pBufferInfo->sChipSpecificData.unionChipType.sSST2.u32TileMode << SST_WX_SRC_TILE_MODE_SHIFT) & SST_WX_SRC_TILE_MODE;
      break;

    default:
    case CS_MEMORY_LINEAR:
      value |= SST_WX_SRC_LINEAR;
      break;
  }

  //or in the stride
  value |= (stride << SST_WX_SRC_STRIDE_SHIFT) & SST_WX_SRC_STRIDE;

  //set AA and stagger, if needed.
  value |= (SST_WX_SRC_EN_AA      * !!(pBufferInfo->u32NumSamples == 4));
  value |= (SST_WX_SRC_EN_STAGGER * !!(pBufferInfo->u32Staggering));

  return value;
}




FXPRIVATE int cs9x_NoIntersection(CSRECT* prectA, CSRECT* prectB, CSRECT* prectX)
{
  prectX->u32Top    = __max(prectA->u32Top,    prectB->u32Top);
  prectX->u32Left   = __max(prectA->u32Left,   prectB->u32Left);
  prectX->u32Right  = __min(prectA->u32Right,  prectB->u32Right);
  prectX->u32Bottom = __min(prectA->u32Bottom, prectB->u32Bottom);

  return ((prectX->u32Left > prectX->u32Right) || (prectX->u32Top > prectX->u32Bottom));
}


#pragma message("RYAN@TODO, Don't forget about FXENTER and FXLEAVE.")


CSRESULT cs9x_SwapBufferToDisplay(PCSGRAPHICALCONTEXT psGraphicalContext, PCSSWAPBUFFERTODISPLAYREQ psSwapBufferToDisplayReq, PCSSWAPBUFFERTODISPLAYRES psSwapBufferToDisplayRes)
{
  CSRESULT ret;
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE res;

  FxU32 i;
  FxU32 bltCommand;
  FxU32 srcTop, srcLeft;
  FxU32 clipTop, clipLeft;
  FxU32 clipWidth, clipHeight;
  PCSCLIPLIST pClipList;
  PCSSWAPBUFFERTODISPLAY pSwapInfo;
  PCSALLOCATIONDESCRIPTOR pBufferInfo;
  CSRECT* pClientRegion;

  CMDFIFO_PROLOG(cf);
  CMDFIFO_SETUP(cf);

  //syntactical shortcuts
  pSwapInfo   = &(psSwapBufferToDisplayRes->sSwapBufferToDisplay);
  pBufferInfo = &(psSwapBufferToDisplayRes->sSrcAllocationDescriptor);
  pClipList   = &(psSwapBufferToDisplayRes->sSwapBufferToDisplay.sClipList);

  //the obligatory copy from req to res
  memcpy(pSwapInfo,   &psSwapBufferToDisplayReq->sSwapBufferToDisplay,     sizeof(*pSwapInfo));
  memcpy(pBufferInfo, &psSwapBufferToDisplayReq->sSrcAllocationDescriptor, sizeof(*pBufferInfo));

  ctx = (CS9X_PCONTEXTNODE)psGraphicalContext->sChipSpecificData.unionChipType.sSST2.u32Reserved[0];
  res = (CS9X_PRESOURCENODE)pBufferInfo->u32BufferID;
  if (!(ctx && res)) return DEBUGINT3, CS_SERVERERROR_INTERNALERROR;

  if (ret = cs9x_VerifyContextState(ctx, res, 0))
    return ret;

  if (!pClipList->u32TotalRegions)
    return CS_SUCCESS;

  CMDFIFO_CHECKROOM(cf, 2);
  SETPH( cf, CMDFIFO_BUILD_PK1(1, SST_UNIT_FBI, mopCMD));
  SETPD( cf, FBI->mopCMD,   SST_MOP_STALL_3D 
                          | SST_MOP_FLUSH_TCACHE 
                          | SST_MOP_FLUSH_PCACHE 
                          |(SST_MOP_STALL_3D_PE << SST_MOP_STALL_3D_SEL_SHIFT) );
  BUMP(2);
  FXWAITFORIDLE();

  pClientRegion = &pSwapInfo->sDestClipRegion;
  for (i=0; i<pClipList->u32TotalRegions; i++)
  {
    clipTop    = pClipList->sClipRegion[i].u32Top;
    clipLeft   = pClipList->sClipRegion[i].u32Left;
    clipWidth  = pClipList->sClipRegion[i].u32Right  - pClipList->sClipRegion[i].u32Left;
    clipHeight = pClipList->sClipRegion[i].u32Bottom - pClipList->sClipRegion[i].u32Top;

    //assumes that the dstClip information is screen relative.
    srcTop    = clipTop  - pClientRegion->u32Top;
    srcLeft   = clipLeft - pClientRegion->u32Left;

    srcTop   &= 0x00000FFF;  //needs to be a positive signed 13-bit value
    srcLeft  &= 0x00000FFF;  //needs to be a positive signed 13-bit value
    clipTop  &= 0x00000FFF;  //needs to be a positive signed 13-bit value
    clipLeft &= 0x00000FFF;  //needs to be a positive signed 13-bit value

    CMDFIFO_CHECKROOM(cf, 9);

    #pragma message("RYAN@TODO, Page 7-96 says tiled surfaces must be 128-byte aligned.")
    #pragma message("           This implies that srcBaseAddr[6:0] must be zero.")

    bltCommand = (  SST_WX_ROP_SRC  << SST_WX_ROP0_SHIFT) 
                 | !SST_WX_CLIPSELECT //it appears Win9x driver uses clipSelect=0 for the screen clip rect
                 |  SST_WX_GO 
                 |  SST_WX_BLT;

    SETPH(cf, (SSTCP_PKT2 | dstBaseAddrBit 
                          | dstFormatBit 
                          | srcBaseAddrBit 
                          | srcFormatBit
                          | srcXYBit 
                          | dstSizeBit 
                          | dstXYBit 
                          | commandBit));

    SET( cf, _FF(lpGRegs)->dstBaseAddr,  _FF(gdiDesktopStart));
    SET( cf, _FF(lpGRegs)->dstFormat,    _FF(ddPrimarySurfaceData.dwFormat));
    SET( cf, _FF(lpGRegs)->srcBaseAddr,  pBufferInfo->u32PhysicalOffset);
    SET( cf, _FF(lpGRegs)->srcFormat,    cs9x_MakeSrcFormat2D(pBufferInfo));
    SET( cf, _FF(lpGRegs)->srcXY,        R32(srcTop,     srcLeft));
    SET( cf, _FF(lpGRegs)->dstSize,      R32(clipHeight, clipWidth));
    SET( cf, _FF(lpGRegs)->dstXY,        R32(clipTop,    clipLeft));
    SETC(cf, _FF(lpGRegs)->command,      bltCommand);
    BUMP(9);

    //RYAN@2D:  Something elsewhere in the driver erroneously assumes that srcBaseAddr always points to the desktop.
    CMDFIFO_CHECKROOM(cf, 2);
    SETPH(cf, (SSTCP_PKT2 | srcBaseAddrBit));
    SET(  cf, _FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart));
    BUMP(2);
  }

  CMDFIFO_EPILOG(cf);
  return CS_SUCCESS;
}



void cs9x_Notification1616(FxU32 state, FxU32 param1)
{
  CS9X_PCONTEXTNODE ctx;
  CS9X_PRESOURCENODE res;

  switch (state)
  {
    case CS9X_STATE_RESOURCELOST:
      res = (CS9X_PRESOURCENODE)param1;
      ctx = res->context;
      ctx->state |= CS9X_STATE_RESOURCELOST;
      res->state |= CS9X_STATE_RESOURCELOST;
      break;

    case CS9X_STATE_COMPLETELOSS:
      for (ctx=(CS9X_PCONTEXTNODE)_FF(csContextList); ctx; ctx=ctx->next)
        ctx->state |= CS9X_STATE_COMPLETELOSS;
  }
}



int cs9x_Initialize()
{
  FxU32 errorcode;

  if (_FF(csInitialized))
    return 1;

  _FF(csInitialized) = LoadLibraryEx32W(DDFXS32_DLLNAME, NULL, NULL);
  if (_FF(csInitialized))
  {
    _FF(csNotification1616) = (FxU32)cs9x_Notification1616;

    _FF(csCamAllocator)     = GetProcAddress32W(_FF(csInitialized), "cs9x_CamAllocator");
    _FF(csCamLiberator)     = GetProcAddress32W(_FF(csInitialized), "cs9x_CamLiberator");
    _FF(csSurfaceAllocator) = GetProcAddress32W(_FF(csInitialized), "cs9x_SurfaceAllocator");
    _FF(csSurfaceLiberator) = GetProcAddress32W(_FF(csInitialized), "cs9x_SurfaceLiberator");
//  _FF(csSurfacePromotion) = GetProcAddress32W(_FF(csInitialized), "promote_PrimaryToOverlay");
  }
  else
  {
    errorcode = GetLastError();
    return 0;
  }


  return 1;
}




int CentralServices(PCSSERVERREQ lpRequest, PCSSERVERRES lpResult)
{
  long i;

  if (!cs9x_Initialize())
  {
    lpResult->u32Status = CS_APIERROR_SYSTEMNOTINITIALIZED;
    return 1;  //means we handled the ExtEscape call
  }

  lpResult->u32Flags = lpRequest->u32Flags;
  lpResult->u32ReqID = lpRequest->u32ReqID;
  memcpy(&lpResult->sGraphicalContext, &lpRequest->sGraphicalContext, sizeof(CSGRAPHICALCONTEXT));

  switch (lpRequest->u32ReqID)
  {
    case CSREQ_GETPROTOCOLREVISION:
      lpResult->u32Status = cs9x_GetProtocolRevision(&lpResult->unionResData.sGetProtocolRevisionRes.u32Major, &lpResult->unionResData.sGetProtocolRevisionRes.u32Minor);
      break;

    case CSREQ_GETGRAPHICALCONTEXT:
      lpResult->u32Status = cs9x_GetGraphicalContext(&lpRequest->unionReqData.sGetGraphicalContextReq.sGraphicalContext, &lpResult->unionResData.sGetGraphicalContextRes.sGraphicalContext);
      break;

    case CSREQ_ALLOC:
      lpResult->u32Status = cs9x_Alloc(&lpRequest->sGraphicalContext, &lpRequest->unionReqData.sAllocReq.sAllocationDescriptor, &lpResult->unionResData.sAllocRes.sAllocationDescriptor, lpRequest->unionReqData.sAllocReq.bOrBiggestAvailable);
      break;

    case CSREQ_FREE:
      lpResult->u32Status = cs9x_Free(&lpRequest->sGraphicalContext, &lpRequest->unionReqData.sAllocReq.sAllocationDescriptor, &lpResult->unionResData.sAllocRes.sAllocationDescriptor);
      break;

    case CSREQ_LOCK:
      lpResult->u32Status = cs9x_Lock(&lpRequest->sGraphicalContext, &lpRequest->unionReqData.sLockReq.sAllocationDescriptor, &lpResult->unionResData.sLockRes.sAllocationDescriptor);
      break;

    case CSREQ_UNLOCK:
      lpResult->u32Status = cs9x_Unlock(&lpRequest->sGraphicalContext, &lpRequest->unionReqData.sUnLockReq.sAllocationDescriptor, &lpResult->unionResData.sUnLockRes.sAllocationDescriptor);
      break;

    case CSREQ_EXECUTECOMMANDS:
      lpResult->u32Status = cs9x_ExecuteCommands(&lpRequest->sGraphicalContext, &lpRequest->unionReqData.sExecuteCommandsReq, &lpResult->unionResData.sExecuteCommandsRes);
      break;

    case CSREQ_SWAPBUFFERTODISPLAY:
      lpResult->u32Status = cs9x_SwapBufferToDisplay(&lpRequest->sGraphicalContext, &lpRequest->unionReqData.sSwapBufferToDisplayReq, &lpResult->unionResData.sSwapBufferToDisplayRes);
      break;

    case CSREQ_RELEASEGRAPHICALCONTEXT:
      lpResult->u32Status = CS_SUCCESS;
      break;

    case CSREQ_SETVIDEOMODE:
    case CSREQ_ACQUIREOVERLAY:
    default:
      lpResult->u32Status = CS_APIERROR_NOTSUPPORTED;   //undefined or unsupported request ID
      break;
  }

  return 1;  //tells Control1 to short-circuit
}

