/* $Header: MOCODVA.C, 12, 11/6/00 9:55:32 AM PST, Xing Cong$ */
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
** File Name:	MOCOMP32.C
**
** Description: Direct Draw MoComp32 Extensions and
**              supporting functions.
**
** $Revision: 12$
** $Date: 11/6/00 9:55:32 AM PST$
**
** $History$
** 
** *****************  Initial  *****************
** User: Xing Cong      Date: 8/16/2000
**
*/
#ifdef AGP_EXECUTE

#include "precomp.h"
#include "ddglobal.h"
#include "ddkernel.h" // for guid
#include "header.h"
#include "hw.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "h3g.h"
#include "ddovl32.h"

//#define USING_DDK

#include "initguid.h"
#include "mocoDVA.h"
#define DEBUG_MOCOMP 0

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)

#define DUMP_DATA
#ifdef DUMP_DATA
typedef struct _dumpdata
{
  int x;
  int y;
  BOOL ref_data;
  BOOL idct_data;
  BOOL out_data;
} dumpdata;

dumpdata check_data;
DWORD dwDump;
DWORD dwCurrent;
BOOL fAllFrames;
BOOL fFirstField;
BOOL fField;
FILE *hFileMap;
#endif

extern BOOL myUnlock(LPDDRAWI_DIRECTDRAW_GBL lpDD,FXSURFACEDATA *surfaceData);
extern PVOID myLock(LPDDRAWI_DIRECTDRAW_GBL lpDD,FXSURFACEDATA *surfaceData);

/*----------------------------------------------------------------------
Function name: mcGetMoCompGuids

Description:   DDRAW MoComp32 callback mcGetMoCompGuids
               Retrieves the number of GUIDs the driver supports.

Parameters:
			   pMcData - Points to a DD_GETMOCOMPGUIDSDATA structure
			   
Return:        DWORD DDRAW result
               DDHAL_DRIVER_HANDLED  
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_GETMOCOMPGUIDSDATA
{
    LPDDRAWI_DIRECTDRAW_LCL lpDD;
    DWORD               dwNumGuids;
    LPGUID              lpGuids;
    HRESULT             ddRVal;
    LPDDHALMOCOMPCB_GETGUIDS GetMoCompGuids;
} DDHAL_GETMOCOMPGUIDSDATA;

*/
DWORD __stdcall mcGetMoCompGuids(LPDDHAL_GETMOCOMPGUIDSDATA pMcData)
{
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP+1, "mcGetMoCompGuids" );
#endif

	pMcData->dwNumGuids = NUM_MOCO_GUIDS;
	if (pMcData->lpGuids)
	{
		UINT n;
		for (n = 0; n < pMcData->dwNumGuids; n++)
			memcpy(&pMcData->lpGuids[n], GuidList[n], sizeof(GUID));
	}

	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcGetMoCompGuids


/*----------------------------------------------------------------------
Function name: mcGetMoCompFormats

Description:   DDRAW MoComp32 callback mcGetMoCompFormats
               Indicates the uncompressed formats to which the hardware can decode the data. 

Parameters:
			   pMcData - Points to a DD_GETMOCOMPFORMATSDATA structure
			   

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_GETMOCOMPFORMATSDATA
{
    LPDDRAWI_DIRECTDRAW_LCL lpDD;
    LPGUID              lpGuid;
    DWORD               dwNumFormats;
    LPDDPIXELFORMAT     lpFormats;
    HRESULT             ddRVal;
    LPDDHALMOCOMPCB_GETFORMATS   GetMoCompFormats;
} DDHAL_GETMOCOMPFORMATSDATA;
*/
DWORD __stdcall mcGetMoCompFormats(LPDDHAL_GETMOCOMPFORMATSDATA pMcData)
{
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcGetMoCompFormats, guid %08.8x", pMcData->lpGuid->Data1 );
#endif
	if (pMcData->lpFormats)
	{
		// our final format is always 4:2:2 YUYV
		pMcData->lpFormats[0].dwSize = sizeof(DDPIXELFORMAT);
		pMcData->lpFormats[0].dwFlags = DDPF_FOURCC;
		pMcData->lpFormats[0].dwFourCC = FOURCC_YUY2;
		pMcData->lpFormats[0].dwYUVBitCount = 16;
        _DD(dwOVLFlags) |= PRE_DXVA;
	}
	pMcData->dwNumFormats = 1;


	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcGetMoCompFormats


/*----------------------------------------------------------------------
Function name: mcGetMoCompBuffInfo

Description:   DDRAW MoComp32 callback mcGetMoCompBuffInfo

               Allows the driver to specify how many interim surfaces are 
			   required to support the specified GUID, and the size, location, 
			   and format of each of these surfaces.

Parameters:
			   pMcData - Points to a DD_GETMOCOMPCOMPBUFFDATA structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDMCCOMPBUFFERINFO
{
    DWORD                       dwSize;             // [in]   size of the struct
    DWORD                       dwNumCompBuffers;   // [out]  number of buffers required for compressed data
    DWORD                       dwWidthToCreate;    // [out]    Width of surface to create
    DWORD                       dwHeightToCreate;   // [out]    Height of surface to create
    DWORD                       dwBytesToAllocate;  // [out]    Total number of bytes used by each surface
    DDSCAPS2                    ddCompCaps;         // [out]    caps to create surfaces to store compressed data
    DDPIXELFORMAT               ddPixelFormat;      // [out]  format to create surfaces to store compressed data
} DDMCCOMPBUFFERINFO, *LPDDMCCOMPBUFFERINFO;

typedef struct _DDHAL_GETMOCOMPCOMPBUFFDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPGUID                      lpGuid;
    DWORD                       dwWidth;            // [in]   width of uncompressed data
    DWORD                       dwHeight;           // [in]   height of uncompressed data
    DDPIXELFORMAT               ddPixelFormat;      // [in]   pixel-format of uncompressed data
    DWORD                       dwNumTypesCompBuffs;// [in/out] number of memory types required for comp buffers
    LPDDMCCOMPBUFFERINFO        lpCompBuffInfo;     // [in]   driver supplied info regarding comp buffers (allocated by client)
    HRESULT                     ddRVal;             // [out]
    LPDDHALMOCOMPCB_GETCOMPBUFFINFO  GetMoCompBuffInfo;
} DDHAL_GETMOCOMPCOMPBUFFDATA;
*/
DWORD __stdcall mcGetMoCompBuffInfo(LPDDHAL_GETMOCOMPCOMPBUFFDATA pMcData)
{
	LPDDMCCOMPBUFFERINFO pBufInfo;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcGetMoCompBuffInfo - guid %08.8x", pMcData->lpGuid->Data1 );
#endif
	pBufInfo = pMcData->lpCompBuffInfo;
 	// this is where we tell how many interim surfaces are needed, and their size, location and format

	// the compressed data needs to be in system memory for the driver manipulate
	pMcData->dwNumTypesCompBuffs = NUM_COMPDATA_BUF_TYPES;		// DVA has 6
	if (pBufInfo)
	{
		UINT type, bytePP;
 	Msg(ppdev, DEBUG_MOCOMP, "mcGetMoCompBuffInfo: guid: %08.8x,  w=%d h=%d",
							pMcData->lpGuid->Data1, pMcData->dwWidth, pMcData->dwHeight);
		bytePP = 2;
		for (type = 0;type < NUM_COMPDATA_BUF_TYPES; type++)
		{
			pBufInfo[type].dwSize = sizeof(DDMCCOMPBUFFERINFO);
			pBufInfo[type].dwNumCompBuffers = NUM_COMPDATA_BUFS;			// 2 compressed work buffers for a ping/pong affect
			pBufInfo[type].ddPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			// for system memory must use an rgb format
			pBufInfo[type].ddPixelFormat.dwFlags = DDPF_RGB;				// use a format the driver supports
			pBufInfo[type].ddPixelFormat.dwRGBBitCount = bytePP * 8;
            //reg 565
			pBufInfo[type].ddPixelFormat.dwRBitMask = 0xf800;
			pBufInfo[type].ddPixelFormat.dwGBitMask = 0x7e0;
			pBufInfo[type].ddPixelFormat.dwBBitMask = 0x1f;
			pBufInfo[type].ddCompCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			switch (type)
			{
			case DXVA_PICTURE_DECODE_BUFFER:
				pBufInfo[type].dwBytesToAllocate = sizeof(struct _DXVA_PictureParameters);
				pBufInfo[type].dwNumCompBuffers = 1;
				break;
			case DXVA_MACROBLOCK_CONTROL_BUFFER:
				pBufInfo[type].dwBytesToAllocate = 0x20000;
				pBufInfo[type].dwNumCompBuffers = 1;	
				break;
			case DXVA_RESIDUAL_DIFFERENCE_BUFFER:
				pBufInfo[type].dwBytesToAllocate = 0x40000;
				pBufInfo[type].dwNumCompBuffers = 1;	
				break;
            case DXVA_AYUV_BUFFER:
				pBufInfo[type].dwBytesToAllocate = 16 * sizeof(DXVA_AYUVsample);
				pBufInfo[type].dwNumCompBuffers = 1;	
				break;
            case DXVA_IA44_SURFACE_BUFFER:
				pBufInfo[type].dwBytesToAllocate = 
					ON_16_BYTE_BOUNDARY(pMcData->dwWidth) * pMcData->dwHeight;
				//from AGP memory
			    pBufInfo[type].ddCompCaps.dwCaps = DDSCAPS_VIDEOMEMORY |
					DDSCAPS_OFFSCREENPLAIN | DDSCAPS_NONLOCALVIDMEM;
				pBufInfo[type].dwNumCompBuffers = 1;	
				break;
            case DXVA_ALPHA_BLEND_COMBINATION_BUFFER: 
   				pBufInfo[type].dwBytesToAllocate = sizeof(DXVA_BlendCombination);
				pBufInfo[type].dwNumCompBuffers = 1;
				break;

            default:
				pBufInfo[type].dwBytesToAllocate = 0x80;
				pBufInfo[type].dwNumCompBuffers = 1;	

			}
			if(type != DXVA_IA44_SURFACE_BUFFER)
			{
				pBufInfo[type].dwWidthToCreate = pBufInfo[type].dwBytesToAllocate / (16 * bytePP) ;
				pBufInfo[type].dwHeightToCreate = 16;
			}
			else
			{
				pBufInfo[type].dwWidthToCreate = ON_16_BYTE_BOUNDARY(pMcData->dwWidth) / bytePP;
				pBufInfo[type].dwHeightToCreate = pMcData->dwHeight;
			}
		}
	}

	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcGetMoCompBuffInfo


/*----------------------------------------------------------------------
Function name: mcGetInternalMoCompInfo

Description:   DDRAW MoComp32 callback mcGetInternalMoCompInfo

               Allows the driver to report that it internally allocates display memory
			   to perform motion compensation. 

			   This function allows the decoder and DirectShow to make better-informed 
			   decisions regarding which GUID to choose.

Parameters:
			   pMcData -  Points to a DD_GETINTERNALMOCOMPDATA structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_GETINTERNALMOCOMPDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPGUID                      lpGuid;
    DWORD                       dwWidth;            // [in]   width of uncompressed data
    DWORD                       dwHeight;           // [in]   height of uncompressed data
    DDPIXELFORMAT               ddPixelFormat;      // [in]   pixel-format of uncompressed data
    DWORD                       dwScratchMemAlloc;  // [out]  amount of scratch memory will the hal allocate for its private use
    HRESULT                     ddRVal;             // [out]
    LPDDHALMOCOMPCB_GETINTERNALINFO  GetInternalMoCompInfo;
} DDHAL_GETINTERNALMOCOMPDATA;
*/
DWORD __stdcall mcGetInternalMoCompInfo(LPDDHAL_GETINTERNALMOCOMPDATA pMcData)
{
	DWORD coded_width, coded_height;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcGetInternalMoCompInfo" );
#endif
 	Msg(ppdev, DEBUG_MOCOMP, "mcGetInternalMoCompInfo: guid: %08.8x, w=%d h=%d",
							pMcData->lpGuid->Data1, pMcData->dwWidth, pMcData->dwHeight);

	coded_width = ON_16_BYTE_BOUNDARY(pMcData->dwWidth);
	coded_height = pMcData->dwHeight;
	//  frame buffer memory we allocate internally
    // first we need two YUV422 reference surfaces 
    // a IDCT buffer -- at AGP space
	pMcData->dwScratchMemAlloc = 6 * coded_width * coded_height;
    // for HD stream, we also need two extra UV reference surfaces
    if(coded_width > MAX_WIDTH)
    	pMcData->dwScratchMemAlloc += 2 * coded_width * coded_height;

    // sub_picture support
    pMcData->dwScratchMemAlloc += 2 * coded_width * coded_height;
	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcGetInternalMoCompInfo

 

/*----------------------------------------------------------------------
Function name: mcCreateMoComp

Description:   DDRAW MoComp32 callback mcCreateMoComp

               Notifies the driver that a software decoder will start 
			   using motion compensation with the specified GUID.

			   Also reports the width, height, and format of the output frame. 
			   The driver can fail this call if it cannot support motion compensation 
			   with these dimensions.


Parameters:
			   pMcData - Points to a DD_CREATEMOCOMPDATA structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_CREATEMOCOMPDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPDDRAWI_DDMOTIONCOMP_LCL   lpMoComp;
    LPGUID                      lpGuid;
    DWORD                       dwUncompWidth;
    DWORD                       dwUncompHeight;
    DDPIXELFORMAT               ddUncompPixelFormat;
    LPVOID                      lpData;
    DWORD                       dwDataSize;
    HRESULT                     ddRVal;
    LPDDHALMOCOMPCB_CREATE      CreateMoComp;
} DDHAL_CREATEMOCOMPDATA;


*/
DWORD __stdcall mcCreateMoComp(LPDDHAL_CREATEMOCOMPDATA pMcData)
{
	DWORD ddrVal;
	pmcDecodingInfo pInfo;
//	DXVA_ConnectConfig *pDvaConnect;
//	DXVA_ConnectInfo *pDvaConnectInfo;
	DXVA_ConnectMode *pDvaConnectMode;
	int i;
	BOOL mpeg_hardware_is_busy = FALSE;
	BOOL vta_hardware_is_busy = FALSE;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcCreateMoComp: guid: %08.8x, uncomp w=%d h=%d moco-lcl %08.8x datasize %d",
							pMcData->lpGuid->Data1, pMcData->dwUncompWidth, pMcData->dwUncompHeight, pMcData->lpMoComp, pMcData->dwDataSize);
#endif

//	if (pMcData->dwDataSize != sizeof(struct _DXVA_ConnectInfo) || !pMcData->lpData)
	if (pMcData->dwDataSize != sizeof(DXVA_ConnectMode) || !pMcData->lpData)
	{
		pMcData->ddRVal = DDERR_UNSUPPORTED; 
		return DDHAL_DRIVER_HANDLED;
	}


	pMcData->ddRVal = DD_OK; 
	pDvaConnectMode = (DXVA_ConnectMode *)pMcData->lpData;

	// init system
	pMcData->lpMoComp->dwDriverReserved1 = 0;
	pMcData->lpMoComp->dwDriverReserved2 = 0;
	pMcData->lpMoComp->dwDriverReserved3 = 0;
	pMcData->lpMoComp->lpDriverReserved1 = NULL;
	pMcData->lpMoComp->lpDriverReserved2 = NULL;
	pMcData->lpMoComp->lpDriverReserved3 = NULL;

	// initialize our variables
	// alloc a structure in normal memory
	// lpReserved1 = pointer to struct we alloc to keep track of all info
	pInfo = (pmcDecodingInfo)DXMALLOCZ(sizeof(mcDecodingInfo));
	if (!pInfo)
	{
		pMcData->ddRVal = DDERR_OUTOFVIDEOMEMORY;
		return DDHAL_DRIVER_HANDLED;
	}
	
	pMcData->lpMoComp->lpDriverReserved1 = (PVOID)pInfo;
    pInfo->wRestrictedMode = pDvaConnectMode->wRestrictedMode;

	pInfo->State = 0;
	pInfo->NeedPicParms = TRUE;

	pInfo->CodedWidth = ON_16_BYTE_BOUNDARY(pMcData->dwUncompWidth);
	pInfo->CodedHeight = pMcData->dwUncompHeight ;
//	pInfo->CodedWidth = 0x160;
//	pInfo->CodedHeight = 0xe0;
//	pInfo->CodedHeight = 0xb0;

 	Msg(ppdev, DEBUG_MOCOMP, "specified width %d height %d",pInfo->CodedWidth, pInfo->CodedHeight);

	pInfo->YSize = pInfo->CodedWidth * pInfo->CodedHeight;

	pInfo->lpMBBuffer = (pmcMBInfo)DXMALLOC( pInfo->YSize / (16 * 16) * sizeof (mcMBInfo));

	if (!pInfo->lpMBBuffer)
	{
        ClearMoCo(ppdev, &pInfo, pMcData->lpDD->lpGbl);
		pMcData->ddRVal = DDERR_OUTOFVIDEOMEMORY;
		return DDHAL_DRIVER_HANDLED;
	}

	// For Rampage, since we use VTA, which alway outputs to a tile surface
	// we need to allocate extra reference buffers in the frame buffer in 
    // linear memory so that field data can be read.

    for(i = 0; i < 2; i++)
    {
	    pInfo->refSurface[i].tileFlag = MEM_IN_LINEAR;	// MEM_IN_TILE1
	    ddrVal = surfMgr_allocSurface(
				    pMcData->lpDD->lpGbl,			// Ddraw VidMemAlloc may need this
				    REF_BUFFER_CAPS,				// User's type
				    0,								// unused
				    pInfo->CodedWidth * 2,			// width of surface in byte
				    pInfo->CodedHeight,				// height of surface
				    0,								// linear address mask
				    &pInfo->refSurface[i].lfbPtr,		// lfb start address of allocation
				    &pInfo->refSurface[i].endlfbPtr,	// lfb stop address of allocation
				    &pInfo->refSurface[i].hwPtr,		// hw vidmem address
				    &pInfo->refSurface[i].dwStride,	// pitch in bytes
				    &pInfo->refSurface[i].tileFlag);	// Memory type, linear
        if (ddrVal != DD_OK)
	    {
            ClearMoCo(ppdev, &pInfo, pMcData->lpDD->lpGbl);
		    pMcData->ddRVal = DDERR_OUTOFVIDEOMEMORY;
		    return DDHAL_DRIVER_HANDLED;
	    }
   
    	pInfo->refSurface[i].dwBytesPerPixel =  2;
    	pInfo->refSurface[i].dwPStride =
        pInfo->refSurface[i].dwStride;
    }
	// second allocates IDCT texture surface, it is at  AGP space
	
    pInfo->IDCTBuffer.tileFlag = MEM_IN_LINEAR | MEM_AT_32;	// MEM_IN_LINEAR
	ddrVal = surfMgr_allocSurface(
				pMcData->lpDD->lpGbl,			// Ddraw VidMemAlloc may need this
				DDSCAPS_NONLOCALVIDMEM,			// User's type
				0,								// unused
				(pInfo->CodedWidth * 2 + 31)&~31,// width of surface in byte
				(pInfo->CodedHeight * 3 + 1)/ 2,// height of surface
				0,								// linear address mask
				&pInfo->IDCTBuffer.lfbPtr,		// lfb start address of allocation
				&pInfo->IDCTBuffer.endlfbPtr,							// lfb stop address of allocation
				&pInfo->IDCTBuffer.hwPtr,		// hw vidmem address
				&pInfo->IDCTBuffer.dwStride, 	// pitch in bytes
				&pInfo->IDCTBuffer.tileFlag);	// Memory type, linear
	if (ddrVal != DD_OK)
	{ 
        ClearMoCo(ppdev, &pInfo, pMcData->lpDD->lpGbl);
		pMcData->ddRVal = DDERR_OUTOFVIDEOMEMORY;
		return DDHAL_DRIVER_HANDLED;
	}

    pInfo->IDCTBuffer.dwStride = ( pInfo->CodedWidth * 2 + 31 ) & ~31; 
    pInfo->IDCTBuffer.dwPStride =pInfo->IDCTBuffer.dwStride;

	pInfo->pUVIDCT = (PBYTE)(pInfo->IDCTBuffer.lfbPtr +
                pInfo->IDCTBuffer.dwStride * pInfo->CodedHeight);

    pInfo->IA88Buffer.tileFlag = MEM_IN_TILE1 | MEM_AT_16K;
	ddrVal = surfMgr_allocSurface(
				pMcData->lpDD->lpGbl,			// Ddraw VidMemAlloc may need this
				0,                  			// User's type
				0,								// unused
				(pInfo->CodedWidth * 2  + 31)&~31,// width of surface in byte
				(pInfo->CodedHeight + 1)/ 2,// height of surface
				0,								// linear address mask
				&pInfo->IA88Buffer.lfbPtr,		// lfb start address of allocation
				&pInfo->IA88Buffer.endlfbPtr,							// lfb stop address of allocation
				&pInfo->IA88Buffer.hwPtr,		// hw vidmem address
				&pInfo->IA88Buffer.dwStride, 	// pitch in bytes
				&pInfo->IA88Buffer.tileFlag);	// Memory type, linear
	if (ddrVal != DD_OK)
	{ 
        ClearMoCo(ppdev, &pInfo, pMcData->lpDD->lpGbl);
		pMcData->ddRVal = DDERR_OUTOFVIDEOMEMORY;
		return DDHAL_DRIVER_HANDLED;
	}

    pInfo->IA88Buffer.dwPStride =pInfo->IA88Buffer.dwStride>> SST2_TILE_WIDTH_BITS;
    
    if(pInfo->CodedWidth > MAX_WIDTH)
    {
	    //last allocate UV reference buffers
	    for(i = 0; i < 2; i++)
	    {
		    pInfo->refUVBuffer[i].tileFlag = MEM_IN_LINEAR | MEM_AT_32;	// MEM_IN_LINEAR
		    ddrVal = surfMgr_allocSurface(
				    pMcData->lpDD->lpGbl,			// Ddraw VidMemAlloc may need this
				    REF_BUFFER_CAPS,				// User's type
				    0,								// unused
				    (pInfo->CodedWidth * 2 + 31)&~31,// width of surface in byte
				    (pInfo->CodedHeight)/ 2	,		// height of surface
				    0,								// linear address mask
				    &pInfo->refUVBuffer[i].lfbPtr,		// lfb start address of allocation
				    &pInfo->refUVBuffer[i].endlfbPtr,							// lfb stop address of allocation
				    &pInfo->refUVBuffer[i].hwPtr,		// hw vidmem address
				    &pInfo->refUVBuffer[i].dwStride, 	// pitch in bytes
				    &pInfo->refUVBuffer[i].tileFlag);	// Memory type, linear
		    if (ddrVal != DD_OK)
		    { 
			    ClearMoCo(ppdev, &pInfo, pMcData->lpDD->lpGbl);
			    pMcData->ddRVal = DDERR_OUTOFVIDEOMEMORY;
			    return DDHAL_DRIVER_HANDLED;
		    }
		    pInfo->refUVBuffer[i].dwBytesPerPixel =  2;
		    pInfo->refUVBuffer[i].dwPStride = pInfo->refUVBuffer[i].dwStride;

	    }
    }

	pMcData->lpMoComp->lpDriverReserved1 = (PVOID)pInfo;

   _DD(dwOVLFlags) |= DXVA_ON;

#ifdef DUMP_DATA	  
		dwCurrent = 0;
		fFirstField = 0;
	  
		hFileMap = fopen("c:\\Moco.txt","w");
        check_data.x=256;
        check_data.y=0;
        check_data.idct_data= 0;
        check_data.ref_data = 0;
        check_data.out_data = 0;
    	dwDump = 1;
		fField = 0; //TOP_FIELD;BOTTOM_FIELD;
		fAllFrames = 1;
		
#endif

	return DDHAL_DRIVER_HANDLED;
}// mcCreateMoComp


/*----------------------------------------------------------------------
Function name: mcDestroyMoComp

Description:   DDRAW MoComp32 callback mcDestroyMoComp

               Notifies the driver that this motion compensation object 
			   will no longer be used. The driver now needs to perform 
			   any necessary cleanup.

Parameters:
			   pMcData - Points to a DD_DESTROYMOCOMPDATA structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_DESTROYMOCOMPDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPDDRAWI_DDMOTIONCOMP_LCL   lpMoComp;
    HRESULT                     ddRVal;
    LPDDHALMOCOMPCB_DESTROY     DestroyMoComp;
} DDHAL_DESTROYMOCOMPDATA;
*/
DWORD __stdcall mcDestroyMoComp(LPDDHAL_DESTROYMOCOMPDATA pMcData)
{
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcDestroyMoComp" );
#endif

	// free data allocated
	if (pMcData->lpMoComp->lpDriverReserved1)
	{
		pmcDecodingInfo pInfo = (pmcDecodingInfo)pMcData->lpMoComp->lpDriverReserved1;
        ClearMoCo(ppdev, &pInfo, pMcData->lpDD->lpGbl);

	}
    _DD(dwOVLFlags) &= ~(PRE_DXVA | DXVA_ON);
#ifdef DUMP_DATA
	if(hFileMap)
		 fclose(hFileMap);
#endif

    _DD(dwOVLFlags) &= ~DXVA_ON;
	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcDestroyMoComp



/*----------------------------------------------------------------------
Function name: mcBeginMoCompFrame

Description:   DDRAW MoComp32 callback mcBeginMoCompFrame

               Starts decoding a new frame.

			   DirectDraw ensures that begin and end frames will be properly paired.

Parameters:
			   pMcData - Points to a DD_BEGINMOCOMPFRAMEDATA structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_BEGINMOCOMPFRAMEDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPDDRAWI_DDMOTIONCOMP_LCL   lpMoComp;
    LPDDRAWI_DDRAWSURFACE_LCL   lpDestSurface;        // [in]  destination buffer in which to decoding this frame
    DWORD                       dwInputDataSize;      // [in]  size of other misc input data to begin frame
    LPVOID                      lpInputData;          // [in]  pointer to misc input data
    DWORD                       dwOutputDataSize;     // [in]  size of other misc output data to begin frame
    LPVOID                      lpOutputData;         // [in]  pointer to output misc data (allocated by client)
    HRESULT                     ddRVal;               // [out]
    LPDDHALMOCOMPCB_BEGINFRAME  BeginMoCompFrame;
} DDHAL_BEGINMOCOMPFRAMEDATA;
*/
DWORD __stdcall mcBeginMoCompFrame(LPDDHAL_BEGINMOCOMPFRAMEDATA pMcData)
{
	pmcDecodingInfo pInfo;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcBeginMoCompFrame" );
#endif
	if (pMcData->dwInputDataSize < sizeof(WORD) || (*(WORD *)pMcData->lpInputData >= NUM_REF_BUFS))
	{
		pMcData->ddRVal = DDERR_INVALIDPARAMS; 
		return DDHAL_DRIVER_HANDLED;
	}
	pInfo = (pmcDecodingInfo)pMcData->lpMoComp->lpDriverReserved1;
	if (pInfo)
	{
		pInfo->CurrentIndex = *(WORD *)pMcData->lpInputData;
		pInfo->DestSurface[pInfo->CurrentIndex] = GET_SURF_DATA(pMcData->lpDestSurface);

	} 

	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcBeginMoCompFrame



/*----------------------------------------------------------------------
Function name: mcEndMoCompFrame

Description:   DDRAW MoComp32 callback mcEndMoCompFrame

               Completes a decoded frame.

			   DirectDraw ensures that begin and end frames will be properly paired

Parameters:
			   pMcData - Points to a DD_ENDMOCOMPFRAMEDATA structure.

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_ENDMOCOMPFRAMEDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPDDRAWI_DDMOTIONCOMP_LCL   lpMoComp;
    LPVOID                      lpInputData;
    DWORD                       dwInputDataSize;
    HRESULT                     ddRVal;
    LPDDHALMOCOMPCB_ENDFRAME    EndMoCompFrame;
} DDHAL_ENDMOCOMPFRAMEDATA;
*/



DWORD __stdcall mcEndMoCompFrame(LPDDHAL_ENDMOCOMPFRAMEDATA pMcData)
{
	pmcDecodingInfo pInfo;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP, "mcEndMoCompFrame" );
#endif
	// we can use this call as the trigger to convert from 4:2:0 to 4:2:2
	pInfo = (pmcDecodingInfo)pMcData->lpMoComp->lpDriverReserved1;
	if (pInfo)
	{
		CMDFIFO_PROLOG(cmdFifo);
        HW_ACCESS_ENTRY(cmdFifo,ACCESS_2D);

		CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE*2 );
		SETMOP(cmdFifo, SST_MOP_STALL_2D);
		SETMOP(cmdFifo, SST_MOP_AGP_FLUSH );
	   	HW_ACCESS_EXIT(ACCESS_2D);
		CMDFIFO_EPILOG(cmdFifo);

		if(pInfo->PicParms.bPicIntra)
		{
			if(pInfo->PicParms.bPicStructure == FRAME_PICTURE)
			{
				pInfo->wNewBufferIndex = (pInfo->wNewBufferIndex+1) % 2;
				pInfo->wFlags[pInfo->CurrentIndex] = pInfo->wNewBufferIndex;

				mcCopySurface(ppdev, pInfo, &(pInfo->IDCTBuffer), TRUE,
                    FRAME_PICTURE,
                    &(pInfo->refSurface[pInfo->wNewBufferIndex]));

                //also copied to the current buffer
				mcCopySurface(ppdev, pInfo, &(pInfo->IDCTBuffer), TRUE,
                    FRAME_PICTURE, pInfo->DestSurface[pInfo->CurrentIndex]);

				if(pInfo->CodedWidth > MAX_WIDTH)
				{
					mcCopyUV(ppdev, pInfo, &(pInfo->IDCTBuffer),
                        &(pInfo->refUVBuffer[pInfo->wNewBufferIndex]),TRUE, FRAME_PICTURE);
					pInfo->wFlags[pInfo->CurrentIndex] |= UV_COPIED;
					
				}
			}
			else
			{

				if(!pInfo->PicParms.bSecondField)
				{
                	pInfo->wNewBufferIndex = (pInfo->wNewBufferIndex+1) % 2;

			    	pInfo->wFlags[pInfo->CurrentIndex] = pInfo->wNewBufferIndex;

					if(pInfo->CodedWidth > MAX_WIDTH)
					{
						mcCopyUV(ppdev, pInfo, &(pInfo->IDCTBuffer),
							&(pInfo->refUVBuffer[pInfo->wNewBufferIndex]),
							TRUE,pInfo->PicParms.bPicStructure);
						pInfo->wFlags[pInfo->CurrentIndex] |= UV_COPIED;
					}
				}
				else
				{
					if(pInfo->CodedWidth > MAX_WIDTH)
					{
						mcCopyUV(ppdev, pInfo, &(pInfo->IDCTBuffer),
							&(pInfo->refUVBuffer[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK]),
							TRUE,pInfo->PicParms.bPicStructure);
						
					}
				}
				

				// All the orginal UV data are at even lines
				//copy IDCT into refSurface, so the UV data are inside
				
		    	mcCopySurface(ppdev, pInfo, &(pInfo->IDCTBuffer), TRUE,
					(BYTE)((pInfo->PicParms.bPicStructure == TOP_FIELD)? TOP_FIELD : FRAME_PICTURE),
                     &(pInfo->refSurface[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK]));

                //copy IDCT to current buffer too
                //the current buffer is in tile1 memory
                //so the whole buffer need to be copied

        		mcCopySurface(ppdev, pInfo, &(pInfo->IDCTBuffer), TRUE,
                    FRAME_PICTURE, pInfo->DestSurface[pInfo->CurrentIndex]);

				pInfo->fFieldIndexReOrder = !pInfo->fFieldIndexReOrder;

							
			}

#ifdef DUMP_DATA
 			Msg(ppdev, DEBUG_MOCOMP, "I Picture Index = %d, Frame %ld, ref=%d", pInfo->PicParms.wDecodedPictureIndex,
				dwCurrent,pInfo->wFlags[pInfo->CurrentIndex] );
#endif

			goto dumpdata;
		}
		else
		{

			if(pInfo->wMBEntry[0] +pInfo->wMBEntry[1] +
				pInfo->wMBEntry[2] +pInfo->wMBEntry[3] +pInfo->wMBEntry[4]
				!= pInfo->wMBTotal)
	 			Msg(ppdev, DEBUG_MOCOMP, 
				"Missing MB!!!! MB0=%ld MB1=%ld MB2 =%ld, MB3=%ld MB4=%ld MB5=%ld,Total=%ld",
				(DWORD)pInfo->wMBEntry[0],(DWORD)pInfo->wMBEntry[1],
				(DWORD)pInfo->wMBEntry[2],(DWORD)pInfo->wMBEntry[3],
				(DWORD)pInfo->wMBEntry[4],(DWORD)pInfo->wMBTotal);

			mcProcessPBFrame(ppdev, pInfo);
 		
			pInfo->fFieldIndexReOrder = FALSE;
		    //clear counters	
		    RtlFillMemory((PVOID)pInfo->wMBEntry, sizeof(pInfo->wMBEntry), 0x00);
		
#ifdef DUMP_DATA
 			Msg(ppdev, DEBUG_MOCOMP, "PB Picture, FwIndex=%d, BwIndex=%d, curr=%d, F=%ld",
 			pInfo->PicParms.wForwardRefPictureIndex,
 			pInfo->PicParms.wBackwardRefPictureIndex,
 			pInfo->CurrentIndex , dwCurrent);
#endif
		}	
dumpdata:


#ifdef DUMP_DATA
		if(((pInfo->PicParms.bPicStructure == FRAME_PICTURE) ||(pInfo->PicParms.bPicStructure == fField) || (!fField &&!fFirstField) )&&
		   ((dwDump== dwCurrent) || (fAllFrames && 
			!(check_data.out_data || check_data.idct_data || check_data.ref_data))))
		{           
       
		  PBYTE pSrc;
		  DWORD dwStride;  
		  int i, j;
		  int factor;

		   while( GET(ghwIO->status) & (SST2_BUSY) )
			;

		   if(pInfo->PicParms.bPicStructure != FRAME_PICTURE)
			   factor = 2;
			else
				factor= 1;
			dwStride = pInfo->DestSurface[pInfo->CurrentIndex]->dwPitch;
            if(check_data.idct_data)
            {
               short *pSrc16;
                fprintf(hFileMap,"\nIDCT second");

                pSrc16 =(short *)(pInfo->IDCTBuffer.lfbPtr);
                fprintf(hFileMap,"Y data \n");
                for( j = check_data.y; j  < 16+check_data.y; j ++)
                {
                    for(i= check_data.x; i <16+check_data.x; i++)
                    {
		            	fprintf(hFileMap,"%d ", pSrc16[i + j * pInfo->IDCTBuffer.dwStride/2/factor]);
                    }

	            	fprintf(hFileMap,"\n");
                }

                fprintf(hFileMap,"UV data \n");
                pSrc16 =(short *)(pInfo->pUVIDCT);
                for( j = check_data.y/2; j < 16 * factor +check_data.y/2; j ++)
                {
                    for(i= check_data.x; i <16+check_data.x; i++)
                    {
		            	fprintf(hFileMap,"%d ", pSrc16[i + j * pInfo->IDCTBuffer.dwStride/2/(factor)]);
                    }

	            	fprintf(hFileMap,"\n");
                }


            }


            if(check_data.out_data)
            {
                fprintf(hFileMap,"\nOut put  x= %d, y=%d\n",check_data.x,check_data.y * factor);
	            fprintf(hFileMap,"Y data \n");
	    	    pSrc = (PBYTE)myLock( pMcData->lpDD->lpGbl,
					pInfo->DestSurface[pInfo->CurrentIndex]);

                for( j = check_data.y * factor; j < (16+check_data.y) * factor; j ++)
                {
                    for(i= check_data.x; i <16+check_data.x; i++)
                    {
		            	fprintf(hFileMap,"%02x", pSrc[i*2 +j * dwStride]);
                    }

	            	fprintf(hFileMap,"\n");
                }

                fprintf(hFileMap,"\nUV data \n");
                for( j = check_data.y * factor; j < (16 + check_data.y) * factor; j ++)
                {
                    for(i= check_data.x; i <16+check_data.x; i++)
                    {
                     //   if(!(j&1))
		            	fprintf(hFileMap,"%02x", pSrc[i*2 + 1+j * dwStride]);
                    }

                   // if(!(j&1))
	            	fprintf(hFileMap,"\n");
                }
		        myUnlock( pMcData->lpDD->lpGbl,
						pInfo->DestSurface[pInfo->CurrentIndex]);


            }

            if(check_data.ref_data)
            {
              FXSURFACEDATA * FwSurface, *BwSurface;
              DWORD wFwIndex, wBwIndex;
              WORD wMBType;
			  WORD motion_type;
              pmcMBInfo lpMBInfo;
              int x, y;

              wFwIndex = pInfo->PicParms.wForwardRefPictureIndex;
              FwSurface = pInfo->DestSurface[wFwIndex];

              wBwIndex = pInfo->PicParms.wBackwardRefPictureIndex;
              BwSurface = pInfo->DestSurface[wBwIndex];
              x = check_data.x;
              y = check_data.y;
			  

		      lpMBInfo = pInfo->lpMBBuffer;
              lpMBInfo += (x + y * pInfo->CodedWidth / 16 ) /16;
              wMBType = lpMBInfo->wMBType ;
			  motion_type = (wMBType >> 8) & 3;
              if(wMBType & MB_INTRA)
              {  
	            	fprintf(hFileMap,"\n Intra Block-- no Reference data\n");
                    goto closefile;
              }

              if(wMBType & MB_FORWARD)
              {
        	    pSrc = (PBYTE)myLock( pMcData->lpDD->lpGbl,FwSurface);

	            fprintf(hFileMap,"\nForward reference\n");
                x += lpMBInfo->PMV[0][0][0] >> 1;
                y += lpMBInfo->PMV[0][0][1] >> 1;
                fprintf(hFileMap,"\n x= %d, y=%d dx=%d, dy=%d\n",
                    x,y * factor,lpMBInfo->PMV[0][0][0],lpMBInfo->PMV[0][0][1]);

                fprintf(hFileMap,"Y data \n");

                for( j = y * factor; j < (16 + y) * factor; j ++)
                {
                    for(i= x; i <16+ x; i++)
                    {
		            	fprintf(hFileMap,"%02x", pSrc[i* 2+j * dwStride]);
                    }

	            	fprintf(hFileMap,"\n");
                }

				if(motion_type == MC_DMV)
				{

					fprintf(hFileMap,"\nSecond Y\n");
					x = check_data.x;
					y = check_data.y;
					x += lpMBInfo->PMV[0][1][0] >> 1;
					y += lpMBInfo->PMV[0][1][1] >> 1;
					fprintf(hFileMap,"\n x= %d, y=%d dx=%d, dy=%d\n",
                    	x,y * factor,lpMBInfo->PMV[0][1][0],lpMBInfo->PMV[0][1][1]);

					fprintf(hFileMap,"Y data \n");

					for( j = y * factor; j < (16 + y) * factor; j ++)
					{
						for(i= x; i <16+ x; i++)
						{
		            		fprintf(hFileMap,"%02x", pSrc[i* 2+j * dwStride]);
						}

	            		fprintf(hFileMap,"\n");
					}
				}
                fprintf(hFileMap,"\nUV data \n");
				x = check_data.x;
                y = check_data.y;
				x += (lpMBInfo->PMV[0][0][0] /2) ;
                y += (lpMBInfo->PMV[0][0][1] /2) ;
				x = x & ~1;
				y = y & ~1;
                fprintf(hFileMap,"\n  x= %d, y=%d\n",x,y * factor);

                for( j = y * factor; j < (16 + y) * factor; j ++)
                {
                    for(i= x; i <16+ x; i++)
                    {   if(!(j&1))
		            	fprintf(hFileMap,"%02x", pSrc[i* 2+1+j * dwStride]);
                    }

                    if(!(j&1))
	            	fprintf(hFileMap,"\n");
                }


				if(motion_type == MC_DMV)
				{

					fprintf(hFileMap,"\nSecond UV\n");
					x = check_data.x;
					y = check_data.y;
					x += (lpMBInfo->PMV[0][1][0] /2) ;
					y += (lpMBInfo->PMV[0][1][1] /2) ;
					x = x & ~1;
					y = y & ~1;
					fprintf(hFileMap,"\n  x= %d, y=%d\n",x,y * factor);

					for( j = y * factor; j < (16 + y) * factor; j ++)
					{
						for(i= x; i <16+ x; i++)
						{   if(!(j&1))
		            		fprintf(hFileMap,"%02x", pSrc[i* 2+1+j * dwStride]);
						}

						if(!(j&1))
	            		fprintf(hFileMap,"\n");
					}
				}
        	   myUnlock( pMcData->lpDD->lpGbl,FwSurface);
              }

              if(wMBType & MB_BACKWARD)
              {

        	    pSrc = (PBYTE)myLock( pMcData->lpDD->lpGbl,BwSurface);

	            fprintf(hFileMap,"\nBackward reference\n");
                x += lpMBInfo->PMV[0][1][0] >> 1;
                y += lpMBInfo->PMV[0][1][1] >> 1;
                fprintf(hFileMap,"\n x= %d, y=%d dx=%d, dy=%d\n",
                    x,y * factor,lpMBInfo->PMV[0][1][0],lpMBInfo->PMV[0][1][1]);


                fprintf(hFileMap,"Y data \n");

                for( j = y * factor; j < (16 + y) * factor; j ++)
                {
                    for(i= x; i <16+ x; i++)
                    {
		            	fprintf(hFileMap,"%02x", pSrc[i* 2+j * dwStride]);
                    }

	            	fprintf(hFileMap,"\n");
                }
				x = check_data.x;
                y = check_data.y;
				x += (lpMBInfo->PMV[0][1][0] /2);
                y += (lpMBInfo->PMV[0][1][1] /2);
				x = x & ~1;
				y = y & ~1;
                fprintf(hFileMap,"\nUV data \n");
                fprintf(hFileMap,"\n  x= %d, y=%d\n",x,y * factor);

                for( j = y * factor; j < (16 + y) * factor; j ++)
                {
                    for(i= x; i <16+ x; i++)
                    { 
						if(!(j&1))
		            	fprintf(hFileMap,"%02x", pSrc[i* 2+1 +j * dwStride]);
                    }

                    if(!(j&1))
	            	fprintf(hFileMap,"\n");
                }



        	   myUnlock( pMcData->lpDD->lpGbl,BwSurface);
               }  


            }
            
			else if(!(check_data.idct_data || check_data.out_data))
            {
				
	    	    pSrc = (PBYTE)myLock( pMcData->lpDD->lpGbl,
					pInfo->DestSurface[pInfo->CurrentIndex]);
				if(fAllFrames)
				{   
					if(dwCurrent) 
					{
					   	char tmp[200];
					 	sprintf( tmp,"c:\\Moco%d.txt",dwCurrent);
						hFileMap = fopen(tmp,"w");
					}
					
				}
                for( j = 0; j <(int) pInfo->CodedHeight ; j ++)
                {
                    for(i= 0; i <(int) pInfo->CodedWidth * 2 ; i++)
                    {
		            	fprintf(hFileMap,"%02x", pSrc[i]);
                    }

	            	fprintf(hFileMap,"\n");
                    pSrc += dwStride;
                // pSrc += pInfo->CodedWidth * 2;
                }
				myUnlock( pMcData->lpDD->lpGbl,
					pInfo->DestSurface[pInfo->CurrentIndex]);

            }               	 
closefile:
	 
		  fclose(hFileMap);
          hFileMap = 0;
		}
		if(!fFirstField)
			dwCurrent++;
 	    mcCopySurface(ppdev, pInfo,pInfo->DestSurface[pInfo->CurrentIndex], FALSE, 0,
            (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData)) );
#endif
		pInfo->NeedPicParms = TRUE;
		pInfo->wMBTotal = 0;
		
	}

	pMcData->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;
}// mcEndMoCompFrame



/*----------------------------------------------------------------------
Function name: mcRenderMoComp

Description:   DDRAW MoComp32 callback mcRenderMoComp

               Tells the driver which macroblocks to render by specifying 
			   the surfaces containing the macroblocks, the offsets in 
			   each surface where the macroblocks exist, and the size of 
			   the macroblock data to be rendered.

			   DdMoCompRender can be called multiple times between the 
			   DdMoCompBeginFrame and DdMoCompEndFrame sequence.

			   If a previous render operation is not yet finished, the driver
			   should set the DirectDraw return code to DDERR_WASSTILLDRAWING.


Parameters:
			   pMcData - Points to a DD_RENDERMOCOMPDATA structure.
			   

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDMCBUFFERINFO
{
    DWORD                       dwSize;         // [in]    size of the struct
    LPDDRAWI_DDRAWSURFACE_LCL   lpCompSurface;  // [in]    pointer to buffer containing compressed data
    DWORD                       dwDataOffset;   // [in]    offset of relevant data from the beginning of buffer
    DWORD                       dwDataSize;     // [in]    size of relevant data
    LPVOID                      lpPrivate;      // Reserved for DirectDraw;
} DDMCBUFFERINFO, *LPDDMCBUFFERINFO;


typedef struct _DDHAL_RENDERMOCOMPDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPDDRAWI_DDMOTIONCOMP_LCL   lpMoComp;
    DWORD                       dwNumBuffers;   // [in]  Number of entries in the lpMacroBlockInfo array
    LPDDMCBUFFERINFO            lpBufferInfo;   // [in]  Surfaces containing macro block info
    DWORD                       dwFunction;     // [in]  Function
    LPVOID                      lpInputData;
    DWORD                       dwInputDataSize;
    LPVOID                      lpOutputData;
    DWORD                       dwOutputDataSize;
    HRESULT                     ddRVal;         // [out]
    LPDDHALMOCOMPCB_RENDER      RenderMoComp;
} DDHAL_RENDERMOCOMPDATA;
*/
DWORD __stdcall mcRenderMoComp(LPDDHAL_RENDERMOCOMPDATA pMcData)
{
	pmcDecodingInfo pInfo;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP+1, "mcRenderMoComp" );
#endif
	pInfo = (pmcDecodingInfo)pMcData->lpMoComp->lpDriverReserved1;
	if (pInfo)
	{
		DWORD QueryFlags = pMcData->dwFunction >> 8;


        if((QueryFlags >> 16)== DXVA_PICTURE_DECODING_FUNCTION)
        {
		    LPDXVA_PictureParameters lpParms= NULL;
		    LPDXVA_TCoefSingle lpSrcIDCT = NULL;
		    PBYTE lpSrcMB = NULL;
		    DWORD dwSizeMB;
		    DWORD dwSizeIDCT = 0;
		    LPDXVA_BufferDescription pDvaHeader;
		    DWORD bufnum;
 

		    for (bufnum = 0; bufnum < pMcData->dwNumBuffers; bufnum++)
		    {

			    //pDvaHeader = (DXVA_BufferDescription *)pMcData->lpBufferInfo[bufnum].lpCompSurface->lpGbl->fpVidMem;
			    pDvaHeader = (DXVA_BufferDescription *)pMcData->lpInputData;
				pDvaHeader += bufnum;
				switch (pDvaHeader->dwTypeIndex)
			    {
			    case DXVA_PICTURE_DECODE_BUFFER:
				    lpParms = (DXVA_PictureParameters *)(pMcData->lpBufferInfo[bufnum].lpCompSurface->lpGbl->fpVidMem);
					//+ sizeof(DXVA_BufferDescription));
				    if (pMcData->lpBufferInfo[bufnum].dwDataSize != sizeof(DXVA_PictureParameters)) // + sizeof(DXVA_BufferDescription)))
				    {
					    pMcData->ddRVal =DXVA_EXECUTE_RETURN_OTHER_ERROR_SEVERE;
					    return DDHAL_DRIVER_HANDLED;
				    }

	
				    break;
			    case DXVA_MACROBLOCK_CONTROL_BUFFER:
				    lpSrcMB = (PBYTE)(pMcData->lpBufferInfo[bufnum].lpCompSurface->lpGbl->fpVidMem ); //+ sizeof(DXVA_BufferDescription));
				    dwSizeMB = pMcData->lpBufferInfo[bufnum].dwDataSize; //- sizeof(DXVA_BufferDescription);
				    break;
			    case DXVA_RESIDUAL_DIFFERENCE_BUFFER:
				    lpSrcIDCT = (LPDXVA_TCoefSingle)(pMcData->lpBufferInfo[bufnum].lpCompSurface->lpGbl->fpVidMem ); //+ sizeof(DXVA_BufferDescription));
				    dwSizeIDCT = pMcData->lpBufferInfo[bufnum].dwDataSize ; //- sizeof(DXVA_BufferDescription);
				    break;

			    default:
				    break;

			    }
		    }



            if (lpParms && pInfo->NeedPicParms )	// must occur at the start of the frame - always
		    {
			    DWORD mbW, mbH, mbWsz, mbHsz;
			    pInfo->NeedPicParms = FALSE;
			    //use HW to do the clearing later
			    RtlMoveMemory(&pInfo->PicParms, lpParms,
                    sizeof(DXVA_PictureParameters));
			    mbW = lpParms->wPicWidthInMBminus1 + 1;
			    mbH = lpParms->wPicHeightInMBminus1 + 1;
			    mbWsz = lpParms->bMacroblockWidthMinus1 + 1;
			    mbHsz = lpParms->bMacroblockHeightMinus1 + 1;
//            			Msg(ppdev, DEBUG_MOCOMP, "actual width %d height %d",mbW*16, mbH*16);
	

    #ifdef DUMP_DATA
			    if(lpParms->bPicStructure != FRAME_PICTURE)
			    {
				    fFirstField = !fFirstField;
			    }
			    else
				    fFirstField = 0;
    #endif
				if (((DWORD)((lpParms->wPicWidthInMBminus1 + 1) * 16) > pInfo->CodedWidth)
					|| ((DWORD)((lpParms->wPicHeightInMBminus1 + 1) * 16) > pInfo->CodedHeight))
				{
					pMcData->ddRVal = DDERR_INVALIDPARAMS; 
					return DDHAL_DRIVER_HANDLED;
				}

        		pInfo->CurrentIndex = lpParms->wDecodedPictureIndex;
				//wait for idle
        		while( GET(ghwIO->status) & (SST2_BUSY) )
        		 ;
    	        //clear buffers
        		if(!pInfo->PicParms.bPicIntra)
				    RtlFillMemory((PVOID)pInfo->IDCTBuffer.lfbPtr, 3* pInfo->YSize, 0x00);

            }	   
		    // we do any work required at this point
			if(lpSrcMB && lpSrcIDCT)
			{
				if(pInfo->PicParms.bPicIntra)
					mcCopyIData(ppdev, pInfo,  lpSrcMB, dwSizeMB, lpSrcIDCT, dwSizeIDCT);
				else
		   			mcCreatePBData(ppdev, pInfo,  lpSrcMB, dwSizeMB, lpSrcIDCT, dwSizeIDCT);
			}
 
        	pMcData->ddRVal = DXVA_EXECUTE_RETURN_OK;

        }
        else if((QueryFlags >> 16)== DXVA_ALPHA_BLEND_DATA_LOAD_FUNCTION)
        {

		    LPDXVA_BufferDescription pDvaHeader;
		    DWORD bufnum;

		    pDvaHeader = (DXVA_BufferDescription *)pMcData->lpInputData;
		    for (bufnum = 0; bufnum < pMcData->dwNumBuffers; bufnum++)
		    {

			    pDvaHeader = (DXVA_BufferDescription *)pMcData->lpInputData;
				pDvaHeader += bufnum;
				switch (pDvaHeader->dwTypeIndex)
			    {
                case DXVA_AYUV_BUFFER:
        		    SetUpPallete(ppdev,
		        	 (LPDXVA_AYUVsample)(pMcData->lpBufferInfo[bufnum].lpCompSurface->lpGbl->fpVidMem));
                    break;
 
                case DXVA_IA44_SURFACE_BUFFER:
                    pInfo->SubPicBuffer =
                     GET_SURF_DATA(pMcData->lpBufferInfo[bufnum].lpCompSurface);
                     
    				break;
                default:
                    break;
                }
            }       
        	pMcData->ddRVal = DXVA_EXECUTE_RETURN_OK;

        }
        else if((QueryFlags >> 16)==DXVA_ALPHA_BLEND_COMBINATION_FUNCTION)
        {

		    LPDXVA_BufferDescription pDvaHeader;
 
		    pDvaHeader = (DXVA_BufferDescription *)pMcData->lpInputData;

            if((pDvaHeader->dwTypeIndex != DXVA_ALPHA_BLEND_COMBINATION_BUFFER)||
                (pMcData->lpBufferInfo->dwDataSize < sizeof(DXVA_BlendCombination))||
                !pInfo->SubPicBuffer)
            {
                pMcData->ddRVal =DXVA_EXECUTE_RETURN_OTHER_ERROR_SEVERE;
			    return DDHAL_DRIVER_HANDLED;

            }

			AlphaCombine(ppdev, pInfo,
                (LPDXVA_BlendCombination)(pMcData->lpBufferInfo->lpCompSurface->lpGbl->fpVidMem));

        	pMcData->ddRVal = DXVA_EXECUTE_RETURN_OK;

        }
        else if((QueryFlags == 0xFFFFF1) || (QueryFlags == 0xFFFFF5))
        {
			DWORD DXVA_Func = pMcData->dwFunction& 0xFF;
			//probing command / locking command
            if(DXVA_Func == DXVA_PICTURE_DECODING_FUNCTION)
            {
                LPDXVA_ConfigPictureDecode lpCfig;

			    if(pMcData->dwInputDataSize < sizeof (DXVA_ConfigPictureDecode))
			    {

        		    pMcData->ddRVal = DDERR_UNSUPPORTED; 
        		    return DDHAL_DRIVER_HANDLED;
			    }
			    lpCfig = (LPDXVA_ConfigPictureDecode)pMcData->lpInputData;

	            if(lpCfig->bConfigBitstreamRaw ||
		            !lpCfig->bConfigResidDiffHost ||	// host IDCT
			        lpCfig->bConfigSpatialResid8 || // 16 bit IDCT
    			    lpCfig->bConfigResidDiffAccelerator ||		// request for IDCT
				    lpCfig->bConfigSpecificIDCT ||		// use 0 for compliant IDCT
                    !lpCfig->bConfigIntraResidUnsigned)
			    {
 				    Msg(ppdev,DEBUG_MOCOMP, 
    "BitstreamRaw =%d,ResidDiffHost = %d,SpatialResid8 = %d, ResidDiffAccelerator = %d,SpecificIDCT= %d",
				    lpCfig->bConfigBitstreamRaw,
		            lpCfig->bConfigResidDiffHost,
			        lpCfig->bConfigSpatialResid8,
    			    lpCfig->bConfigResidDiffAccelerator,
				    lpCfig->bConfigSpecificIDCT);

				    if(pMcData->dwOutputDataSize >= sizeof (DXVA_ConfigPictureDecode))
				    {
					    RtlMoveMemory(pMcData->lpOutputData, pMcData->lpInputData,
					    sizeof(DXVA_ConfigPictureDecode));

					    lpCfig = (LPDXVA_ConfigPictureDecode)pMcData->lpOutputData;

					    lpCfig->bConfigBitstreamRaw= 0;
					    lpCfig->bConfigResidDiffHost = 1;
					    lpCfig->bConfigSpatialResid8 = 0;
    				    lpCfig->bConfigResidDiffAccelerator= 0;
					    lpCfig->bConfigSpecificIDCT = 0;
                        lpCfig->bConfigIntraResidUnsigned = 1;
					    if(QueryFlags ==0xFFF1)
						    lpCfig->dwFunction = 0xFFFFFB | DXVA_Func;
					    else
						    lpCfig->dwFunction = 0xFFFFFF | DXVA_Func;
				    }
				    pMcData->ddRVal = S_FALSE; 
			    }
			    else
			    {
				    //we get it
				   
				    RtlMoveMemory(&(pInfo->DecodeConfig), pMcData->lpInputData,
                        sizeof(DXVA_ConfigPictureDecode));

                    if(pMcData->dwOutputDataSize >= sizeof (DXVA_ConfigPictureDecode))
				    {

                        if(QueryFlags == 0xFFFFF5) //locking
					     RtlMoveMemory(pMcData->lpOutputData, pMcData->lpInputData,
					      sizeof(DXVA_ConfigPictureDecode));

					    lpCfig = (LPDXVA_ConfigPictureDecode)pMcData->lpOutputData;

					    if(QueryFlags ==0xFFF1)
						    lpCfig->dwFunction = 0xFFFFF800 | DXVA_Func;
					    else
						    lpCfig->dwFunction = 0xFFFFFC00 | DXVA_Func;
				    }
				    pMcData->ddRVal = S_OK;

			    }
            }
            else if(DXVA_Func == DXVA_ALPHA_BLEND_DATA_LOAD_FUNCTION)
            {
                //Alpha Blend Data Configuration
			    LPDXVA_ConfigAlphaLoad lpCfig;
			    //probing command / locking command
			    if(pMcData->dwInputDataSize < sizeof (DXVA_ConfigAlphaLoad))
			    {

        		    pMcData->ddRVal = DDERR_UNSUPPORTED; 
        		    return DDHAL_DRIVER_HANDLED;
			    }
			    lpCfig = (LPDXVA_ConfigAlphaLoad)pMcData->lpInputData;

                if(lpCfig->bConfigDataType == DXVA_CONFIG_DATA_TYPE_AI44)
    				pMcData->ddRVal = S_OK;     //only support AI44
                else
    				pMcData->ddRVal = S_FALSE;
			    if(pMcData->dwOutputDataSize >= sizeof (DXVA_ConfigAlphaLoad))
                {
                    RtlMoveMemory(pMcData->lpOutputData, pMcData->lpInputData,
				    	sizeof(DXVA_ConfigAlphaLoad));

			        lpCfig = (LPDXVA_ConfigAlphaLoad)pMcData->lpOutputData;
                    lpCfig->bConfigDataType = DXVA_CONFIG_DATA_TYPE_AI44;
                    if(pMcData->ddRVal == S_OK)
                    {
					    if(QueryFlags ==0xFFF1)
						    lpCfig->dwFunction = 0xFFFFF800 | DXVA_Func;
					    else
						    lpCfig->dwFunction = 0xFFFFFC00 | DXVA_Func;

                    }
                    else
                    {
					    if(QueryFlags ==0xFFF1)
						    lpCfig->dwFunction = 0xFFFFFB00 | DXVA_Func;
					    else
						    lpCfig->dwFunction = 0xFFFFFF00 | DXVA_Func;

                    }


                }

            }
            else if(DXVA_Func ==DXVA_ALPHA_BLEND_COMBINATION_FUNCTION)
            {
                //Alpha Blend Data Combination

			    LPDXVA_ConfigAlphaCombine lpCfig;
			    //probing command / locking command
			    if(pMcData->dwInputDataSize < sizeof (DXVA_ConfigAlphaCombine))
			    {

        		    pMcData->ddRVal = DDERR_UNSUPPORTED; 
        		    return DDHAL_DRIVER_HANDLED;
			    }
			    lpCfig = (LPDXVA_ConfigAlphaCombine)pMcData->lpInputData;

                if(lpCfig->bConfigWholePlaneAlpha
                    || !lpCfig->bConfigOnlyUsePicDestRectArea
                    ||(lpCfig->bConfigBlendType == DXVA_CONFIG_BLEND_TYPE_BACK_HARDWARE))
    				pMcData->ddRVal = S_FALSE;
                else
    				pMcData->ddRVal = S_OK;


				if(pMcData->dwOutputDataSize >= sizeof (DXVA_ConfigAlphaCombine))
				{
					RtlMoveMemory(pMcData->lpOutputData, pMcData->lpInputData,
					sizeof(DXVA_ConfigAlphaCombine));

        			lpCfig = (LPDXVA_ConfigAlphaCombine)pMcData->lpOutputData;
                    lpCfig->bConfigWholePlaneAlpha =0;
                    lpCfig->bConfigOnlyUsePicDestRectArea = 1;
                    lpCfig->bConfigGraphicResizing = 1;
                    lpCfig->bConfigBlendType = DXVA_CONFIG_BLEND_TYPE_FRONT_BUFFER;
                    if(pMcData->ddRVal == S_OK)
                    {
					    if(QueryFlags ==0xFFF1)
						    lpCfig->dwFunction = 0xFFFFF800 | DXVA_Func;
					    else
						    lpCfig->dwFunction = 0xFFFFFC00 | DXVA_Func;

                    }
                    else
                    {
					    if(QueryFlags ==0xFFF1)
						    lpCfig->dwFunction = 0xFFFFFB00 | DXVA_Func;
					    else
						    lpCfig->dwFunction = 0xFFFFFF00 | DXVA_Func;

                    }

				}
	
            }

        }

	    return DDHAL_DRIVER_HANDLED;
        
	}

	pMcData->ddRVal = DXVA_EXECUTE_RETURN_OK;
	return DDHAL_DRIVER_NOTHANDLED;
}// mcRenderMoComp


/*----------------------------------------------------------------------
Function name: mcQueryMoCompStatus

Description:   DDRAW MoComp32 callback mcQueryMoCompStatus

               Queries the status of the most recent rendering operation to 
			   the specified surface.  
			   
Parameters:
			   pMcData - Points to a DD_QUERYMOCOMPSTATUSDATA structure. 

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
/*
typedef struct _DDHAL_QUERYMOCOMPSTATUSDATA
{
    LPDDRAWI_DIRECTDRAW_LCL     lpDD;
    LPDDRAWI_DDMOTIONCOMP_LCL   lpMoComp;
    LPDDRAWI_DDRAWSURFACE_LCL   lpSurface;      // [in]  Surface being queried
    DWORD                       dwFlags;        // [in]  DDMCQUERY_XXX falgs
    HRESULT                     ddRVal;         // [out]
    LPDDHALMOCOMPCB_QUERYSTATUS QueryMoCompStatus;
} DDHAL_QUERYMOCOMPSTATUSDATA;

#define DDMCQUERY_READ          0x00000001
*/
DWORD __stdcall mcQueryMoCompStatus(LPDDHAL_QUERYMOCOMPSTATUSDATA pMcData)
{
	pmcDecodingInfo pInfo;
   DD_ENTRY_SETUP(pMcData->lpDD->lpGbl);
#ifdef FXTRACE
 	Msg(ppdev, DEBUG_MOCOMP+1, "mcQueryMoCompStatus on surface %08.8x, flags (ro=1) %x",
		pMcData->lpSurface, pMcData->dwFlags);
#endif

	pInfo = (pmcDecodingInfo)pMcData->lpMoComp->lpDriverReserved1;
	if (pInfo)
    {
      FXSURFACEDATA * thisSurface =
           GET_SURF_DATA(pMcData->lpSurface);

        if(thisSurface == pInfo->DestSurface[pInfo->CurrentIndex])
        {
    		if(GET(ghwIO->status) & (SST2_BUSY) )
              	pMcData->ddRVal = DDERR_WASSTILLDRAWING; 
            else
              	pMcData->ddRVal = DD_OK;
        }
        else
            pMcData->ddRVal = DD_OK;

    }
    else
    {
    	if(GET(ghwIO->status) & (SST2_BUSY) )
          	pMcData->ddRVal = DDERR_WASSTILLDRAWING; 
        else
          	pMcData->ddRVal = DD_OK;

    }

	return DDHAL_DRIVER_HANDLED;
}// mcQueryMoCompStatus


/*----------------------------------------------------------------------
Function name: mcCopyIData

Description:   Copy I macroblocks into AGP memory at pInfo->IDCTBuffer
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/

void mcCopyIData(GLOBALDATA *ppdev, pmcDecodingInfo pInfo, 
							PBYTE lpSrcMB, DWORD dwSizeMB, 
							LPDXVA_TCoefSingle lpSrcIDCT,  DWORD dwSizeIDCT)
{
	DWORD indexMB = 0;
	DWORD indexIDCT = 0;
	LPDXVA_TCoefSingle lpIdctBuff = lpSrcIDCT;
	DXVA_MBctrl_I_HostResidDiff_1 *pdvaMB;
	DWORD comp = 0;
	int MBA; 
	int mb_width;

	// loop loading mb into cmd stream
	while (indexMB < dwSizeMB)
	{

		int bx, by;
		int dct_type;
		int picture_structure;
		PBYTE pSrc;  
		PBYTE pDest;
        DWORD dwDestStride;
		DWORD dwDestStart;
		BOOL interleaved_idct;
		//////////////////////////////////////////////
		// now the fun begins

		//////////////////////////////////////////////
		// set up the macroblock pointers
		pdvaMB = (DXVA_MBctrl_I_HostResidDiff_1 *)lpSrcMB;

		// now create the variables need for decode
		MBA = pdvaMB->wMBaddress;
		//Msg(ppdev, DEBUG_MOCOMP, "MBA %d", MBA);

	  // no loop filter
	  // not 4mv

		lpIdctBuff = &lpSrcIDCT[pdvaMB->dwMB_SNL & 0x00FFFFFF];
		if ((pdvaMB->dwMB_SNL & 0x00FFFFFF) != indexIDCT)
		{
 			Msg(ppdev, DEBUG_MOCOMP, "Non-matching IDCT base - index %d, location %d", indexIDCT, (pdvaMB->dwMB_SNL & 0x00FFFFFF));
		}

		
		dct_type = ((pdvaMB->wMBtype >> 5) & 0x01);
		picture_structure = pInfo->PicParms.bPicStructure;
        dwDestStride = pInfo->IDCTBuffer.dwStride;
		dwDestStart = pInfo->IDCTBuffer.lfbPtr;

		interleaved_idct = (picture_structure == FRAME_PICTURE) && ( dct_type != 0);
		mb_width = pInfo->PicParms.wPicWidthInMBminus1 + 1;
		bx = 16*(MBA%mb_width);
		by = 16*(MBA/mb_width);
		if(picture_structure == BOTTOM_FIELD)
		{
			dwDestStart+= dwDestStride;
			dwDestStride <<= 1;
		}
		else if(picture_structure == TOP_FIELD)
        {
            dwDestStride <<= 1;
		}
		pSrc = (char *)lpIdctBuff;

		for (comp = 0; comp < 6; comp++) //copy from 4:2:0 source
		{								 //into 4:2:2 destination
			int i, j;
		
			if(comp < 4)	//copy Y
			{	
				if(!interleaved_idct)
					pDest = (PBYTE)dwDestStart+
						(by + (comp /2) * 8) * dwDestStride+
						(comp % 2)* 16 + bx * 2;
				else
					pDest = (PBYTE)dwDestStart+
						(by + (comp /2)) * dwDestStride +
						(comp % 2)* 16 + bx * 2;

				for (i = 0; i< 8; i ++)
				{
					for(j = 0; j < 8; j ++)
					{
						//*pDest = 0x80 + *pSrc++;
						*pDest =*pSrc++;

						pDest += 2;		
					}
					if(!interleaved_idct)	
						pDest += dwDestStride -16;
					else
						pDest += dwDestStride * 2 - 16;
				}
				
			}
			else
			{
				//copy UV
				if(picture_structure == FRAME_PICTURE)
				{
					pDest = (PBYTE)dwDestStart +
						by  * dwDestStride +
						bx * 2 + 1 + (comp % 2) * 2;

					for (i = 0; i< 8; i ++)
					{
						for(j = 0; j < 8; j ++)
						{
							*pDest = *pSrc;
							//*pDest = 0x80+ *pSrc;
//							*(pDest + dwDestStride) = 0x80 + *pSrc++;//replicate U V
							*(pDest + dwDestStride) = *pSrc++;//replicate U V
							pDest += 4;		
						}

						pDest +=  2* (dwDestStride -16);
					}
				}
				else
				{
					if(comp==4)
					{
						dwDestStride >>=1;
						if(picture_structure == BOTTOM_FIELD)
						{
							dwDestStart+= dwDestStride;
						}
					}
					pDest = (PBYTE)dwDestStart +
						by  * dwDestStride * 2+
						bx * 2 + 1 + (comp % 2) * 2;

					for (i = 0; i< 8; i ++)
					{
						for(j = 0; j < 8; j ++)
						{
//							*pDest = 0x80+ *pSrc;
//							*(pDest + dwDestStride) = 0x80 + *pSrc++;//replicate U V
							*pDest = *pSrc;
							*(pDest + dwDestStride) = *pSrc++;//replicate U V
							pDest += 4;		
						}

						pDest +=   4* dwDestStride -32;
					}

				}
			}
		}	
				
		//////////////////////////////////////////////
		// update pointers
		lpSrcMB += sizeof(DXVA_MBctrl_I_HostResidDiff_1);
		indexMB += sizeof(DXVA_MBctrl_I_HostResidDiff_1);
		indexIDCT += 16 * 6;

	}
   
}

/*----------------------------------------------------------------------
Function name: mcCreatePBData

Description:   Create MBinfo for each macroblock of P and B picture
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/

void mcCreatePBData(GLOBALDATA *ppdev, pmcDecodingInfo pInfo, 
							PBYTE lpSrcMB, DWORD dwSizeMB, 
							LPDXVA_TCoefSingle lpSrcIDCT,  DWORD dwSizeIDCT)
{
	DWORD bumpMB = 0;
	DWORD bumpIDCT = 0;
	DWORD indexMB = 0;
	DWORD indexIDCT = 0;
	LPDXVA_TCoefSingle lpIdctBuff = lpSrcIDCT;
	DXVA_MBctrl_P_OffHostIDCT_1 *pdvaMB;
	DWORD comp = 0;
	pmcMBInfo pMBInfo, pMBStart;
	int MBA; 
	int macroblock_type, motion_type;
	int mb_width;
	int picture_structure;
	DWORD dwDestStart;
	DWORD dwDestStride;
    DXVA_MVvalue *pDXVA_MVvalue;
	DXVA_MBctrl_P_HostResidDiff_1 *pdvaHRDmb;
	int by, bx;
	int coded_block_pattern;
	short *pSrc16;
	short *pDest16;
	PBYTE pDest8;
	BOOL interleaved_idct;
	int i;

	pMBStart = pMBInfo = pInfo->lpMBBuffer;
	pMBInfo += pInfo->wMBTotal;
	dwDestStride = pInfo->IDCTBuffer.dwStride;
	picture_structure = pInfo->PicParms.bPicStructure;
	mb_width = pInfo->PicParms.wPicWidthInMBminus1 + 1;
		
	// loop loading mb into cmd stream
	while (indexMB < dwSizeMB)
	{

        if(pInfo->wMBTotal > 
            (pInfo->PicParms.wPicWidthInMBminus1 + 1)*
            (pInfo->PicParms.wPicHeightInMBminus1 + 1) )
        {
            _asm int 3
            return;
        }

		//////////////////////////////////////////////
		// now the fun begins

		//////////////////////////////////////////////
		// set up the macroblock pointers
		bumpMB = 0;
		pdvaMB = (DXVA_MBctrl_P_OffHostIDCT_1 *)lpSrcMB;

		// now create the variables need for decode
		MBA = pdvaMB->wMBaddress;
		//Msg(ppdev, DEBUG_MOCOMP, "MBA %d", MBA);
	
		bx = 16*(MBA%mb_width);
		by = 16*(MBA/mb_width);
#ifdef DUMP_DATA
		if((check_data.x == bx) && (check_data.y == by)&& (dwDump == dwCurrent))
		{
			motion_type = 0;
		}
#endif		
		pMBInfo->wMBType = pdvaMB->wMBtype;	
	
		motion_type = ((pdvaMB->wMBtype >> 8) & 0x03);
		macroblock_type = pdvaMB->wMBtype & 7;

		if(macroblock_type == MB_INTRA )
			i = 0;
		else if((motion_type != MC_DMV) && (macroblock_type == MB_FORWARD ) )
			i = 1;
		else if (macroblock_type == MB_BACKWARD)
			i = 2;
		else if(macroblock_type == (MB_FORWARD | MB_BACKWARD))
			i = 3;
		else if(motion_type == MC_DMV)
			i = 4;
		else if(!macroblock_type)
		{
			//if zero, then useing forward prediction
			// for field case, prediction coming from the same field
			i = 1;
			pDXVA_MVvalue[0].horz = 0;		//just make sure
			pDXVA_MVvalue[0].vert = 0;
			if(picture_structure != FRAME_PICTURE)
			{
				pMBInfo->wMBType = ((picture_structure == BOTTOM_FIELD) << 12) | (pMBInfo->wMBType & 0xEFFF);	
			}
			pMBInfo->wMBType |= 2;		//forward
		}
		else
		{
			i = 1;
			_asm int 3		//shouldn't be here
		}
	
		if(pInfo->wMBEntry[i])
		{
			pMBStart[ pInfo->wMBCurrent[i]].wNext = pInfo->wMBTotal;
		}
		else
		{
			pInfo->lpMBStart[i] = pMBInfo;
		}

		pInfo->wMBCurrent[i] = pInfo->wMBTotal;
		pInfo->wMBEntry[i]++;

//		pdvaMB->wMBtype |= 0;//(field_residual &0x01) << 5;	// ?? is this field residual - see if field IDCT (not for 4:2:0 i think)
	  // no loop filter
	  // not 4mv
	
		
		if (!(macroblock_type & MB_INTRA))
		{
			pdvaHRDmb = (DXVA_MBctrl_P_HostResidDiff_1 *)pdvaMB;
			pDXVA_MVvalue = (DXVA_MVvalue *)pdvaHRDmb->MVector;
			pMBInfo->PMV[0][0][0] = pDXVA_MVvalue[0].horz;
			pMBInfo->PMV[0][0][1] = pDXVA_MVvalue[0].vert;
			pMBInfo->PMV[0][1][0] = pDXVA_MVvalue[1].horz;
			pMBInfo->PMV[0][1][1] = pDXVA_MVvalue[1].vert;
			pMBInfo->PMV[1][0][0] = pDXVA_MVvalue[2].horz;
			pMBInfo->PMV[1][0][1] = pDXVA_MVvalue[2].vert;
			pMBInfo->PMV[1][1][0] = pDXVA_MVvalue[3].horz;
			pMBInfo->PMV[1][1][1] = pDXVA_MVvalue[3].vert;
//			bumpMB += sizeof(DXVA_MBctrl_P_HostResidDiff_1);
		}
//		else
//		  bumpMB += sizeof(DXVA_MBctrl_I_HostResidDiff_1);
		  
		bumpMB += sizeof(DXVA_MBctrl_P_HostResidDiff_1);

		coded_block_pattern = pdvaMB->wPatternCode >> 6;

		interleaved_idct = (picture_structure == FRAME_PICTURE) && ( (pdvaMB->wMBtype & 0x20));			
	
		pMBInfo->bx = bx;
        pMBInfo->by = by;

#ifdef DUMP_DATA
		if((bx == check_data.x) && (by == check_data.y))
			bumpIDCT = 0;
#endif
		bumpIDCT = 0;

		if(coded_block_pattern)
		{
    		lpIdctBuff = &lpSrcIDCT[pdvaMB->dwMB_SNL & 0x00FFFFFF];
    		if ((pdvaMB->dwMB_SNL & 0x00FFFFFF) != indexIDCT)
    		{
 			Msg(ppdev, DEBUG_MOCOMP, "Non-matcatching IDCT base - index %d, location %d", indexIDCT, (pdvaMB->dwMB_SNL & 0x00FFFFFF));
    		}

	    	pSrc16 = (short *)lpIdctBuff;
	
        	dwDestStart = pInfo->IDCTBuffer.lfbPtr;
			for (comp = 0; comp < 6; comp++)		// 4:2:0 
			{
			
				int i, j;
				//copy IDCT into IDCTBuffer in raster-scan order
				if (coded_block_pattern & (1<<(6-1-comp)))
				{
#ifdef DUMP_DATA

           if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
                fprintf(hFileMap,"\n IDCT block %d\n",comp);
#endif
					if(comp < 4)	//Y 
	    			{
		    			if (macroblock_type & MB_INTRA)
						{
							if(!interleaved_idct)
								pDest8 = (PBYTE)dwDestStart+
								(by + (comp /2) * 8) * dwDestStride+
								(comp % 2)* 8 + bx * 2;
							else
								pDest8 =(PBYTE)dwDestStart+
								(by + (comp /2)) * dwDestStride +
								(comp % 2)* 8 + bx * 2;

							for( i = 0; i< 8; i ++)
							{
								for(j= 0; j< 8; j++)
								{
#ifdef DUMP_DATA
if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))				
			   fprintf(hFileMap,"%x ", *pSrc16);
#endif
                                    if(*pSrc16 <= 0xFF)
     									*pDest8++ =(BYTE) *pSrc16;
                                    else
                                        *pDest8++ = 0xFF;
                                    pSrc16++;

								}
								if(!interleaved_idct)	
									 pDest8 += dwDestStride -8;
								 else
									 pDest8 += dwDestStride * 2 -8;
#ifdef DUMP_DATA
          if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"\n");
#endif
 							}
						}
						else
						{
							if(!interleaved_idct)
								pDest16 = (short *)dwDestStart+
								(by + (comp /2) * 8) * dwDestStride / 2+
								(comp % 2)* 8 + bx;
							else
								pDest16 = (short*)dwDestStart+
								(by + (comp /2)) * dwDestStride/ 2 +
								(comp % 2)* 8 + bx;

							for( i = 0; i< 8; i ++)
							{
								for(j= 0; j< 8; j++)
								{
#ifdef DUMP_DATA
          if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"%d ", *pSrc16);
#endif
 									*pDest16++ = *pSrc16++;

								}
								if(!interleaved_idct)	
									 pDest16 += dwDestStride/2 -8;
								 else
									 pDest16 += dwDestStride -8;
#ifdef DUMP_DATA
           if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"\n ");
#endif
 							}
						}
					}
					else	// UV
					{
						dwDestStart = (DWORD)pInfo->pUVIDCT;
						if (macroblock_type & MB_INTRA)
						{
							pDest8 = (PBYTE)dwDestStart+
								by  * dwDestStride / 2 +
								(comp % 2) + bx * 2;  //start at 16pbb boundary
						
							for( i = 0; i< 8; i ++)
							{
								for(j= 0; j< 8; j++)
								{
#ifdef DUMP_DATA
          if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"%x ", *pSrc16);
#endif
                                    if(*pSrc16 <= 0xFF)
     									*pDest8 = (BYTE)*pSrc16;
                                    else
                                        *pDest8 = 0xFF;
                                    pSrc16++;
                                    pDest8 += 2;       //skip one for UV pari

								}
#ifdef DUMP_DATA
          if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"\n ");
#endif
	    						 pDest8 += dwDestStride -16;
							}
						}
						else
						{
							pDest16 = (short *)dwDestStart+
									by * dwDestStride / 4 +
									(comp % 2) + bx;
							
							for( i = 0; i< 8; i ++)
							{
								for(j= 0; j< 8; j++)
								{
#ifdef DUMP_DATA
           if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"%d ", *pSrc16);
#endif
									*pDest16 = *pSrc16++;
                                    pDest16+=2;     //skip one for UV pair

								}
#ifdef DUMP_DATA
           if((dwDump==dwCurrent)&& check_data.idct_data && (check_data.x == bx) && 
			   (((pInfo->PicParms.bPicStructure == FRAME_PICTURE) && (check_data.y == by)) ||
			   ((pInfo->PicParms.bPicStructure == fField) && (check_data.y /2 == by))))
			   fprintf(hFileMap,"\n ");
#endif
	    						 pDest16 += dwDestStride/2 -16;
							}
						}
						
					}

 				    bumpIDCT += 32;
				}
			}
		}

		pMBInfo++;
		pInfo->wMBTotal++;


		// now check for skips
		if (pdvaMB->dwMB_SNL & 0xFF000000)
		{
			skipped_macroblocks(ppdev, pdvaMB->dwMB_SNL >> 24, &pMBInfo, pInfo,pdvaMB,(pMBInfo -1)->PMV);
		}
		//////////////////////////////////////////////
		// update pointers
		lpSrcMB += bumpMB;
		indexMB += bumpMB;
		indexIDCT += bumpIDCT;

	}
   
}
 

/*----------------------------------------------------------------------
Function name: skipped_macroblocks

Description:   deals with skipped macroblocks
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/
 
void skipped_macroblocks(GLOBALDATA *ppdev, 
								int NumToSkip, 
								pmcMBInfo *ppMBInfo,
								pmcDecodingInfo pInfo, 
								DXVA_MBctrl_P_OffHostIDCT_1 *pdvaMB,
								short PMV[2][2][2]
								)
								
{
	int MBA; 
	int mb_width;
	int cnt, i;
	int dct_type = ((pdvaMB->wMBtype >> 5) & 0x01);
	int coded_block_pattern = 0;
	int picture_structure = pInfo->PicParms.bPicStructure;
	WORD  motion_type;
	WORD  wMBType = pdvaMB->wMBtype;
	WORD  macroblock_type;


	// derive motion_type
	if (picture_structure==FRAME_PICTURE)
		motion_type = MC_FRAME;
	else
	{
		motion_type = MC_FIELD;

		// predict from field of same parity 
		// ISO/IEC 13818-2 section 7.6.6.1 and 7.6.6.3: P field picture and B field
    	// picture 
		wMBType &= ~0x3000;
		if(picture_structure==BOTTOM_FIELD)
		 wMBType |= 0x3000;
	}


	// IMPLEMENTATION: clear MB_INTRA 
	wMBType &= ~MB_INTRA;
	wMBType &= ~0x300;
	wMBType |= motion_type << 8;

	macroblock_type = wMBType & 7;

	if(macroblock_type == MB_FORWARD )
		i = 1;
	else if (macroblock_type == MB_BACKWARD)
		i = 2;
	else if(!macroblock_type) 
	{
		i = 1;
		wMBType |= 2;		//forward
	}
	else//if(macroblock_type == (MB_FORWARD | MB_BACKWARD))
		i = 3;


	mb_width = pInfo->PicParms.wPicWidthInMBminus1 + 1;
	MBA = pdvaMB->wMBaddress + 1;

	// now loop adding blocks for decoder
	for (cnt = 0; cnt < NumToSkip; cnt++, MBA++)
	{
		(*ppMBInfo)->PMV[0][0][0] = PMV[0][0][0];
		(*ppMBInfo)->PMV[0][0][1] = PMV[0][0][1];
		(*ppMBInfo)->PMV[0][1][0] = PMV[0][1][0];
		(*ppMBInfo)->PMV[0][1][1] = PMV[0][1][1];
		(*ppMBInfo)->PMV[1][0][0] = PMV[1][0][0];
		(*ppMBInfo)->PMV[1][0][1] = PMV[1][0][1];
		(*ppMBInfo)->PMV[1][1][0] = PMV[1][1][0];
		(*ppMBInfo)->PMV[1][1][1] = PMV[1][1][1];
		(*ppMBInfo)->by = 16*(MBA/mb_width);
		(*ppMBInfo)->bx = 16*(MBA%mb_width);
		(*ppMBInfo)->wMBType = wMBType;


		if(pInfo->wMBEntry[i])
		{
			pInfo->lpMBBuffer[ pInfo->wMBCurrent[i]].wNext = pInfo->wMBTotal;
		}
		else
		{
			pInfo->lpMBStart[i] = *ppMBInfo;
		}

		pInfo->wMBCurrent[i] = pInfo->wMBTotal;	
		pInfo->wMBEntry[i]++;

		(*ppMBInfo)++;
		pInfo->wMBTotal++;
		
	}

}


/*----------------------------------------------------------------------
Function name: mcCopySurface

Description:   Only copy frome AGP or TILE1 memory into linear or tile 1 memory
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/
void mcCopySurface(GLOBALDATA *ppdev, pmcDecodingInfo pInfo,
				FXSURFACEDATA *srcSurf,BOOL fAGP, BYTE PStructure,
				FXSURFACEDATA *dstSurf)
{
  DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
  DWORD bltRop, bltCommand, bltCommandExtra;
  DWORD bltSrcBaseAddr, bltSrcFormat, bltSrcSize, bltSrcXY;
  DWORD dstPitch, srcPitch, clip1min, clip1max;
  DWORD packetHeader = 0;
  DWORD bumpNum = 14;
  DWORD dwWidth, dwHeight;
  
  CMDFIFO_PROLOG(hwPtr);
  HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

  //copy the frame into DestSurface
  bltCommand =(SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) |
            SST_WX_BLT | SST_WX_GO | SST_WX_CLIPSELECT;

  dwWidth = pInfo->CodedWidth;
  dwHeight = pInfo->CodedHeight;
  // get base address
  if(fAGP)
	  bltSrcBaseAddr = srcSurf->lfbPtr -_FF(agpHeapLinBaseAddr) + 
					_FF(agpHeapPhysBaseAddr);
  else
	  bltSrcBaseAddr = srcSurf->hwPtr;
  bltDstBaseAddr = dstSurf->hwPtr;

  srcPitch = srcSurf->dwPStride;
  dstPitch = dstSurf->dwPStride;

  if( PStructure == TOP_FIELD)
  {
  	Msg(ppdev, DEBUG_MOCOMP, "Copy Top");
     dstPitch <<= 1;
	 dwHeight >>=1;
     //if(!( srcSurf->tileFlag & MEM_IN_TILE1))
     	srcPitch <<= 1;
  }
  else if(PStructure == BOTTOM_FIELD)
  {

  	Msg(ppdev, DEBUG_MOCOMP, "Copy Bottom");
	bltDstBaseAddr += dstPitch;
	dstPitch <<= 1;
    //if(!( srcSurf->tileFlag & MEM_IN_TILE1))
    {
   		bltSrcBaseAddr += srcPitch;
   		srcPitch <<= 1;
    }
	dwHeight >>= 1;
  }

  BLTCLIP(0, 0, clip1min);
  BLTCLIP(dwWidth, dwHeight, clip1max);
  if(dstSurf != &(_FF(ddPrimarySurfaceData)))
  	BLTFMT_SRC(srcPitch, SST_WX_PIXFMT_16BPP, bltSrcFormat);   
  else
  {
	if(dwWidth > 1280)
		dwWidth = 1280;
	if(dwHeight > 1024)
		dwHeight = 1024;

  	BLTFMT_SRC(srcPitch, SST_WX_PIXFMT_422YUV, bltSrcFormat);   

  }
  BLTFMT_DST(dstPitch, SST_WX_PIXFMT_16BPP, bltDstFormat);

  if( fAGP)
    bltSrcFormat |= SST_WX_SRC_LINEAR | SST_WX_SRC_AGP;
  else if( srcSurf->tileFlag & MEM_IN_LINEAR)
    bltSrcFormat |= SST_WX_SRC_LINEAR;
  else if( srcSurf->tileFlag & MEM_IN_TILE1)
    bltSrcFormat |= SST_WX_SRC_TILE_MODE;


  if( dstSurf->tileFlag & MEM_IN_LINEAR)
    bltDstFormat |= SST_WX_DST_LINEAR;
  else if( dstSurf->tileFlag & MEM_IN_TILE1)
    bltDstFormat |= SST_WX_DST_TILE_MODE;

  BLTSIZE(dwWidth, dwHeight, bltDstSize);
  BLTXY(0, 0, bltDstXY);

  BLTSIZE(dwWidth, dwHeight, bltSrcSize);
  BLTXY(0, 0, bltSrcXY);

  bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | SST_WX_ROP_SRC;
  bltCommandExtra = 0;
  // write to hw
  packetHeader |= dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | srcBaseAddrBit
                  | commandExBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;
 
  CMDFIFO_CHECKROOM(hwPtr, bumpNum);

	SETPH( hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
	SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
	SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
	SETPD(hwPtr, ghw2D->rop, bltRop);
	SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
	SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
	SETPD(hwPtr, ghw2D->clip1min, clip1min);
	SETPD(hwPtr, ghw2D->clip1max, clip1max);
	SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
	SETPD(hwPtr, ghw2D->srcSize, bltSrcSize);
	SETPD(hwPtr, ghw2D->srcXY,bltSrcXY);
	SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
	SETPD(hwPtr, ghw2D->dstXY,bltDstXY);
	SETPD(hwPtr, ghw2D->command, bltCommand);
  
   
 
    HW_ACCESS_EXIT(ACCESS_2D);

    //wait for done
    while( GET(ghwIO->status) & (SST2_BUSY) )
        ;

    CMDFIFO_EPILOG(hwPtr);
    
    


}

/*----------------------------------------------------------------------
Function name: mcCopyUV

Description:   copy YUV422 lines that only have original UV data into dstSurfaceData
			   this is for HD stream only, because for field picture
			   UV data are fetched on every 4 lines, NPT stride is not big enough
			   to do that.
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/
void mcCopyUV(GLOBALDATA *ppdev, pmcDecodingInfo pInfo,
				FXSURFACEDATA *srcBuff,FXSURFACEDATA *dstSurfaceData,
				BOOL fAGP, BYTE PStructure)
{
  DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
  DWORD bltRop, bltCommand, bltCommandExtra;
  DWORD bltSrcBaseAddr, bltSrcFormat, bltSrcSize, bltSrcXY;
  DWORD dstPitch, srcPitch, clip1min, clip1max;
  DWORD packetHeader = 0;
  DWORD bumpNum = 14;
  DWORD dwWidth, dwHeight;
  
  CMDFIFO_PROLOG(hwPtr);
  HW_ACCESS_ENTRY(hwPtr,ACCESS_2D);

   //copy the frame into DestSurface
  bltCommand =(SST_WX_ROP_SRC << SST_WX_ROP0_SHIFT) |
            SST_WX_BLT | SST_WX_GO | SST_WX_CLIPSELECT;

  dwWidth = pInfo->CodedWidth;
  dwHeight = pInfo->CodedHeight /2;
  // get base address
  if(fAGP)
	  bltSrcBaseAddr = srcBuff->lfbPtr -_FF(agpHeapLinBaseAddr) + 
					_FF(agpHeapPhysBaseAddr);
  else
	  bltSrcBaseAddr = srcBuff->hwPtr;
  bltDstBaseAddr = dstSurfaceData->hwPtr;

  srcPitch = srcBuff->dwPStride  << 1;
  dstPitch = dstSurfaceData->dwPStride;

  if( PStructure == TOP_FIELD)
  {
  	Msg(ppdev, DEBUG_MOCOMP, "UVCopy Top");
     dstPitch <<= 1;
	 dwHeight >>=1;
     srcPitch <<= 1;
  }
  else if(PStructure == BOTTOM_FIELD)
  {

  	Msg(ppdev, DEBUG_MOCOMP, "UVCopy Bottom");
	bltDstBaseAddr += dstPitch;
	dstPitch <<= 1;
	bltSrcBaseAddr += srcPitch;
	srcPitch <<= 1;
	dwHeight >>= 1;
  }

  BLTCLIP(0, 0, clip1min);
  BLTCLIP(dwWidth, dwHeight, clip1max);
  BLTFMT_SRC(srcPitch, SST_WX_PIXFMT_16BPP, bltSrcFormat);   
  
  BLTFMT_DST(dstPitch, SST_WX_PIXFMT_16BPP, bltDstFormat);

  if( fAGP)
    bltSrcFormat |= SST_WX_SRC_LINEAR | SST_WX_SRC_AGP;
  else 
    bltSrcFormat |= SST_WX_SRC_LINEAR;

  bltDstFormat |= SST_WX_DST_LINEAR;

  BLTSIZE(dwWidth, dwHeight, bltDstSize);
  BLTXY(0, 0, bltDstXY);

  BLTSIZE(dwWidth, dwHeight, bltSrcSize);
  BLTXY(0, 0, bltSrcXY);

  bltRop = (SST_WX_ROP_SRC << 16 )| (SST_WX_ROP_SRC << 8 ) | SST_WX_ROP_SRC;
  bltCommandExtra = 0;
  // write to hw
  packetHeader |= dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | srcBaseAddrBit
                  | commandExBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;
 
  CMDFIFO_CHECKROOM(hwPtr, bumpNum);

	SETPH( hwPtr, CMDFIFO_BUILD_PK2(packetHeader));
	SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
	SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
	SETPD(hwPtr, ghw2D->rop, bltRop);
	SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
	SETPD(hwPtr, ghw2D->commandEx, bltCommandExtra);
	SETPD(hwPtr, ghw2D->clip1min, clip1min);
	SETPD(hwPtr, ghw2D->clip1max, clip1max);
	SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
	SETPD(hwPtr, ghw2D->srcSize, bltSrcSize);
	SETPD(hwPtr, ghw2D->srcXY,bltSrcXY);
	SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
	SETPD(hwPtr, ghw2D->dstXY,bltDstXY);
	SETPD(hwPtr, ghw2D->command, bltCommand);
  
   
 
    HW_ACCESS_EXIT(ACCESS_2D);

    //wait for done
  //  while( GET(ghwIO->status) & (SST2_BUSY) )
  //      ;

    CMDFIFO_EPILOG(hwPtr);

}
						
/*----------------------------------------------------------------------
Function name: LoadIDCT

Description:   Load IDCT surface as texture for VTA
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/
void LoadIDCT(GLOBALDATA *ppdev,pmcDecodingInfo pInfo, DWORD tmuNum,
        BOOL fYBlock,BOOL fIntra, TMURegs *sstReg)
{
  GrTexNPTInfoExt     sTexture;

    if(fYBlock)
    {
		sTexture.format          = SST_TA_AI88;
		sTexture.maxS            = pInfo->CodedWidth;
		sTexture.maxT            = pInfo->CodedHeight;
	    sTexture.baseAddr        = pInfo->IDCTBuffer.lfbPtr -_FF(agpHeapLinBaseAddr) + 
					_FF(agpHeapPhysBaseAddr);
    }
    else
    {
		if(!fIntra)
		{
			sTexture.format          = SST_TA_ARGB8888;
			sTexture.maxS            = pInfo->CodedWidth /2;
			sTexture.maxT            = pInfo->CodedHeight/2;
		}
		else
		{
			sTexture.format          = SST_TA_AI88;
			sTexture.maxS            = pInfo->CodedWidth ;
			sTexture.maxT            = pInfo->CodedHeight /2;
		}
		sTexture.baseAddr        = (DWORD)pInfo->pUVIDCT -
                                _FF(agpHeapLinBaseAddr) + 
					            _FF(agpHeapPhysBaseAddr);

    }
//	if(!fIntra || fYBlock)
	    sTexture.bFilter         = 0;
//	else
//	    sTexture.bFilter         = 1;

    sTexture.nptStride       = pInfo->IDCTBuffer.dwStride;
	sTexture.baseAddr		 |= SST_TA_TEX_AGP;
    SetNPTSourceExt(tmuNum, &sTexture,MEM_IN_LINEAR, sstReg);

}

/*----------------------------------------------------------------------
Function name: LoadRef

Description:   Load reference surface as texture for VTA
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/
void LoadRef(FXSURFACEDATA *srcSurf, DWORD tmuNum,
    BOOL fYBlock, DWORD width, DWORD height,
    BYTE bPicStruct, TMURegs *sstReg)
{
  GrTexNPTInfoExt     sTexture;

    sTexture.baseAddr        = srcSurf->hwPtr;
 
	if(bPicStruct == FRAME_PICTURE)
	{
		sTexture.nptStride       = srcSurf->dwStride;
	   	sTexture.maxT            = height;
	}
	else
	{
		sTexture.nptStride       =  2 * srcSurf->dwStride;
	 	sTexture.maxT            = height >>1;
	
	}
    if(fYBlock)
    {
		sTexture.format          = SST_TA_AI88;
	    sTexture.maxS            = width;
    }
    else
    {    
        sTexture.format          = SST_TA_ARGB8888;
	    sTexture.maxS            = width >>1;
   		sTexture.maxT			 >>= 1;
        if(width <= MAX_WIDTH)
        {
    		sTexture.nptStride		 <<= 1;		//double the stride to only read every other line
        }
    } 

	sTexture.bFilter         = 1;
   
    SetNPTSourceExt(tmuNum, &sTexture,srcSurf->tileFlag, sstReg);
	 
}


/*----------------------------------------------------------------------
Function name: ProcessPB

Description:   Process P and B macroblocks
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/
void mcProcessPBFrame(GLOBALDATA *ppdev, pmcDecodingInfo pInfo)
{
  FXSURFACEDATA * FwSurface=NULL, *BwSurface= NULL, *currentSurf; //*DstSurface;
  pmcMBInfo lpMBInfo, pMBStart;
  Vertex vtxA, vtxB, vtxC, vtxD;
  TMURegs  sstReg;
  DWORD width, height;
  DWORD dwUpdate0, dwUpdate1;
  WORD wFwIndex, wBwIndex;
  WORD wMBControl, wMB_Type;
  WORD wLoop,wMBNum;
  BYTE bPStructure = pInfo->PicParms.bPicStructure;
  BYTE motion_type;
  int iDrawTime, i;
  int x, y, y_offset;
  int dy, dy1;
  int srcX, srcY, srcX1, srcY1;
  float y_adjust;
  float half_x, half_y, half_x1, half_y1;
  DWORD dwDeltaY;
  DWORD dwYStride, dwUVStride;
  BOOL fHalfMB;

  if((bPStructure == FRAME_PICTURE) || !pInfo->fFieldIndexReOrder)
  {
	  wFwIndex = pInfo->PicParms.wForwardRefPictureIndex;
  }
  else
  { 
	  wFwIndex = pInfo->CurrentIndex;
//	  pInfo->PicParms.wForwardRefPictureIndex = wFwIndex; // for test
  }

  if(wFwIndex< NUM_REF_BUFS)
	  FwSurface = &(pInfo->refSurface[pInfo->wFlags[wFwIndex] & SURF_MASK]);

  if (pInfo->PicParms.bPicBackwardPrediction)
  {
	  wBwIndex = pInfo->PicParms.wBackwardRefPictureIndex;
	 if(wBwIndex < NUM_REF_BUFS)
	      BwSurface =
            &(pInfo->refSurface[pInfo->wFlags[wBwIndex] & SURF_MASK]);
  }
  else
     wBwIndex = 0;

  Msg(ppdev, DEBUG_MOCOMP, "FwIndex=%d, BwIndex=%d, Fref=%d, Bref=%d",
    wFwIndex, wBwIndex,	pInfo->wFlags[wFwIndex],pInfo->wFlags[wBwIndex]);

  dwYStride = pInfo->refSurface[0].dwStride;
  if(pInfo->CodedWidth >MAX_WIDTH)
  {
	    dwUVStride = pInfo->refUVBuffer[0].dwStride;
		if(FwSurface && !(pInfo->wFlags[wFwIndex] & UV_COPIED))
		{
			pInfo->wNewBufferIndex = (pInfo->wNewBufferIndex+1) % 2;
			mcCopyUV(ppdev, pInfo, FwSurface,
                &(pInfo->refUVBuffer[pInfo->wNewBufferIndex]),
                FALSE,FRAME_PICTURE);
			pInfo->wFlags[wFwIndex] |= UV_COPIED | pInfo->wNewBufferIndex;
		}

		if(BwSurface && !(pInfo->wFlags[wBwIndex] & UV_COPIED))
		{
			pInfo->wNewBufferIndex = (pInfo->wNewBufferIndex+1) % 2;
			mcCopyUV(ppdev, pInfo, BwSurface,
                &(pInfo->refUVBuffer[pInfo->wNewBufferIndex]),
                FALSE, FRAME_PICTURE);
			pInfo->wFlags[wBwIndex] |= UV_COPIED | pInfo->wNewBufferIndex;
		}
		if(!pInfo->fFieldIndexReOrder && (wFwIndex != pInfo->CurrentIndex) &&
			!(pInfo->wFlags[pInfo->CurrentIndex] & UV_HALF))
			pInfo->wFlags[pInfo->CurrentIndex] &= ~UV_COPIED;
  }
  else
  // assume all surfaces have the same stride
  	  dwUVStride = dwYStride * 2; 

  pMBStart = pInfo->lpMBBuffer;
  width = pInfo->CodedWidth;
  height = pInfo->CodedHeight;
//  width = (pInfo->PicParms.wPicWidthInMBminus1 + 1) * 16;
//  height = (pInfo->PicParms.wPicHeightInMBminus1 + 1) * 16;

  wLoop = 0;  

  vtxA.w = 1.0f;
  vtxB = vtxC = vtxD = vtxA;
  //clear something in the floating pointer uniter
  //MediaMatics player can leave something in FPU before call into
  //the driver.
  vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(width);
  {
  #ifdef CMDFIFO
  CMDFIFO_PROLOG(cmdFifo);
#else
  FxU32 *cmdFifo;
  FxU32  hwIndex;
#endif

  HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);
  preSetRegs(ppdev, pInfo->DestSurface[pInfo->CurrentIndex],height, &cmdFifo, &hwIndex);
  while( wLoop < 10)
  {
		if(!pInfo->wMBEntry[wLoop % 5])
			goto NextPB;

		lpMBInfo = pInfo->lpMBStart[wLoop % 5];
		//pre setting registers
		//if((wLoop != 1) && (wLoop != 5))
		memset( &sstReg, 0, sizeof( TMURegs));

		dwUpdate0 = 0;
		dwUpdate1 = 0;

		if( wLoop == 0)
		{
			   //Y INTRA
   
			   //IDCT macroblock Y in TMU0
			LoadIDCT(ppdev, pInfo,0,TRUE,TRUE, &sstReg);

			TMULoadY(0,&sstReg);		//in ddovl32.c
			SetupTmus( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
			if(bPStructure == FRAME_PICTURE)
			   ConfigBuffer( ppdev, SST_PE_R_WRMASK|SST_PE_B_WRMASK,4, 0,&cmdFifo, &hwIndex);  //16 bit mode
   			else if(bPStructure == TOP_FIELD)
			   ConfigBuffer( ppdev, SST_PE_R_WRMASK|SST_PE_B_WRMASK,4,EVEN_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
			else
			   ConfigBuffer( ppdev, SST_PE_R_WRMASK|SST_PE_B_WRMASK,4,ODD_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode

		}
		else if(wLoop < 3)
		{
			//Y Forward or Backward
		   if(wLoop == 1 )
		   {
				currentSurf = FwSurface;
		   }
		   else
			   currentSurf = BwSurface;

   		   LoadRef(currentSurf,1, TRUE,width,height, bPStructure,&sstReg);
		   
		   LoadIDCT( ppdev,pInfo,0,TRUE,FALSE, &sstReg);
		   TMULoad(1, &sstReg);
		   TMUAddYDelta( 0, &sstReg);

		   if(bPStructure == FRAME_PICTURE)
 			   ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,0,&cmdFifo, &hwIndex);  //16 bit mode
   		   else if(bPStructure == TOP_FIELD)
			   ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,EVEN_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
		   else
   			   ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,ODD_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode

			SetupTmus( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
   
		}
		else if(wLoop  < 5)
		{
		   //Y Forward and Backword		wLoop == 3
		   // and Dual prime		wLoop == 4
			//load referenced data  in TMU1 and TMU2
			if(wLoop == 3)
			{
				LoadRef(FwSurface,1, TRUE,width,height, bPStructure,&sstReg);
				LoadRef(BwSurface,2, TRUE,width,height, bPStructure,&sstReg);
			}
			else
			{

				if((bPStructure != FRAME_PICTURE) && pInfo->PicParms.bSecondField)
				{
					BwSurface = &pInfo->refSurface[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK];
				}
				else
					BwSurface = FwSurface;
				LoadRef(FwSurface,1, TRUE,width,height, TOP_FIELD,&sstReg);
   				LoadRef(BwSurface,2, TRUE,width,height, TOP_FIELD,&sstReg);
			}

		   //IDCT macroblock Y in TMU0
		   LoadIDCT(ppdev, pInfo,0,TRUE,FALSE, &sstReg);
		
		   TMULoad(2, &sstReg);
		   TMUAverage(1, &sstReg);
		   TMUAddYDelta( 0, &sstReg);
		   SetupTmus( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
		   if(bPStructure == FRAME_PICTURE)
   			   ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,0,&cmdFifo, &hwIndex);  //16 bit mode         
   		   else if(bPStructure == TOP_FIELD)
			   ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,EVEN_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
		   else 
   			   ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,ODD_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
		  
		}
		else if(wLoop == 5) 
		{
		   //V U INTRA

		   //set up V U macroblock texture info
		  //IDCT macroblock UV in TMU0
   		   LoadIDCT( ppdev,pInfo,0,FALSE,TRUE, &sstReg);

		   TMULoad(0,&sstReg);		//in ddovl32.c
		   SetupTmus( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
		   if(bPStructure == FRAME_PICTURE)
			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, 0,&cmdFifo, &hwIndex);  //32 bit mode        
		   else if(bPStructure == TOP_FIELD)
			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, TOP_TWO,&cmdFifo, &hwIndex);  //32 bit mode 
		   else
   			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, BOTTOM_TWO,&cmdFifo, &hwIndex);  //32 bit mode 
   
		}
		else if(wLoop < 8)
		{
		   //V U Forward or backward
		   if(wLoop == 6)
		   {
				if( width > MAX_WIDTH)
					 FwSurface =
				 &(pInfo->refUVBuffer[pInfo->wFlags[wFwIndex] & SURF_MASK]);

			   currentSurf = FwSurface;
		   }
		   else
           {  
				if( width > MAX_WIDTH)
					 BwSurface =
				 &(pInfo->refUVBuffer[pInfo->wFlags[wBwIndex] & SURF_MASK]);
			   
			   currentSurf = BwSurface;
           }

		   //set reference frame
  		   LoadRef(currentSurf,3, FALSE,width,height, bPStructure,&sstReg);
           
		   //set up IDCT V U macroblock texture info
		   //IDCT macroblock VU in TMU0-2
		   LoadIDCT(ppdev, pInfo,0,FALSE, FALSE, &sstReg);
		   LoadIDCT(ppdev, pInfo,1,FALSE, FALSE, &sstReg);
		   LoadIDCT(ppdev, pInfo,2,FALSE, FALSE, &sstReg);

		   TMULoad( 3, &sstReg);
 	 	   TMUAddVUDelta(2, &sstReg);

		   SetupTmus( ppdev, &sstReg, 3,&cmdFifo, &hwIndex);
		   if(bPStructure == FRAME_PICTURE)
			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, 0,&cmdFifo, &hwIndex);  //32 bit mode        
		   else  if(bPStructure == TOP_FIELD)
			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, TOP_TWO,&cmdFifo, &hwIndex);  //32 bit mode 
		   else
   			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, BOTTOM_TWO,&cmdFifo, &hwIndex);  //32 bit mode 

		}
		else			//if( wLoop == 8 or 9)
		{

		   //Y  UForward and Backword		wLoop == 8
		   // and Dual prime		wLoop == 9
			//load referenced data  in TMU3 and TMU4
			if(wLoop == 8)
			{
                if( width > MAX_WIDTH)
                {   
					 FwSurface =
						 &(pInfo->refUVBuffer[pInfo->wFlags[wFwIndex] & SURF_MASK]);
					 BwSurface =
						 &(pInfo->refUVBuffer[pInfo->wFlags[wBwIndex] & SURF_MASK]);

  	            }    
                
              	LoadRef(FwSurface,3, FALSE,width,height, bPStructure,&sstReg);
		    	LoadRef(BwSurface,4, FALSE,width,height, bPStructure,&sstReg);
			}
			else
			{
	 			
                if( width > MAX_WIDTH)
                {
					 FwSurface =
						 &(pInfo->refUVBuffer[pInfo->wFlags[wFwIndex] & SURF_MASK]);
				}
				if((bPStructure != FRAME_PICTURE) && pInfo->PicParms.bSecondField)
				{
					//second field uses the first field for reference
					if( width <= MAX_WIDTH)
					{
						BwSurface = &pInfo->refSurface[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK];
					}
					else
					 BwSurface =
						 &(pInfo->refUVBuffer[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK]);
				}
				else
					BwSurface = FwSurface;
		
               	LoadRef(FwSurface,3, FALSE,width,height, TOP_FIELD,&sstReg);
			   	LoadRef(BwSurface,4, FALSE,width,height, TOP_FIELD,&sstReg);
	           
			}

		   //set up IDCT V U macroblock texture info
		   //IDCT macroblock VU in TMU0-2
		   LoadIDCT(ppdev, pInfo,0,FALSE, FALSE, &sstReg);
		   LoadIDCT(ppdev, pInfo,1,FALSE, FALSE, &sstReg);
		   LoadIDCT(ppdev, pInfo,2,FALSE, FALSE, &sstReg);
	
		   TMULoad( 4, &sstReg);
		   TMUAverage(3,&sstReg);

 		   TMUAddVUDelta(2, &sstReg); 
 
		   SetupTmus( ppdev, &sstReg, 4,&cmdFifo, &hwIndex);
		   if(bPStructure == FRAME_PICTURE)
			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, 0,&cmdFifo, &hwIndex);  //32 bit mode        
		   else  if(bPStructure == TOP_FIELD)
			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, TOP_TWO,&cmdFifo, &hwIndex);  //32 bit mode 
		   else
   			   ConfigBuffer( ppdev, SST_PE_A_WRMASK | SST_PE_G_WRMASK,4, BOTTOM_TWO,&cmdFifo, &hwIndex);  //32 bit mode 
   
		}
		

		for(wMBNum = pInfo->wMBEntry[(wLoop % 5)]; wMBNum > 0; wMBNum--)
		{
		  x = lpMBInfo->bx;
		  y = lpMBInfo->by;

		  iDrawTime = 0;
		  y_offset = 16;	  
		  wMBControl = lpMBInfo->wMBType;
		  wMB_Type = wMBControl & 7;
		  motion_type = (wMBControl >> 8) & 3;
		  switch( wLoop)
		  {		  
		  case 0:		//Y INTRA

//		   if( wMB_Type == MB_INTRA)
		   {
#ifdef DUMP_DATA
				if((check_data.x == x) && (check_data.y == y))
				{
					dwDeltaY = 16;

				}
#endif
				//IDCT Macroblock 
				vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x);
				vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x+ 8);
				vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y);
				vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y+ 16);
				
				vtxA.x = vtxC.x = (float)(x >> 1);   //32 bit destination -- two pixels a time
				vtxB.x = vtxD.x = (float)((x >> 1) + 8);
		 		if(bPStructure == FRAME_PICTURE)
				{
					vtxA.y = vtxB.y = (float)(y);
					vtxC.y = vtxD.y = (float)(y + 16);
				}
		 		else 
				{
					vtxA.y = vtxB.y = (float)(y <<1);
					vtxC.y = vtxD.y = (float)((y + 16)<<1);
				}
			
				
				DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,0,&cmdFifo, &hwIndex);
				
			}
     
  			break;
      
		  case 1:		//Y Forward
//		        if( (wMB_Type == MB_FORWARD) &&
//					(motion_type != MC_DMV))
//                   goto    P_Yprediction;
//                break;
		  case 2:
                    //Y Backward
//		    if((wMB_Type == MB_BACKWARD) &&
//				(motion_type != MC_DMV))
		    {
//P_Yprediction:
				dwDeltaY = 16;	//for Y
#ifdef DUMP_DATA
				if((check_data.x == x) && (check_data.y == y)&& (dwDump == dwCurrent))
				{
					dwDeltaY = 16;

				}
#endif
				fHalfMB = FALSE;
			   	if(bPStructure != FRAME_PICTURE) 
				{
				   	if(bPStructure == TOP_FIELD) 
						y_adjust = 0.25f;
					else
						y_adjust = -0.25f;

					if(motion_type == MC_FRAME)
					{
						//16X8 MC
						dwDeltaY = 8;
						iDrawTime ++;
						y_offset = 8;
						fHalfMB = TRUE;
					}
					
					if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x1000)) ||
					   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x2000)))
					{
						//fetch MB from top field
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
						}
					}
					else
					{
						//fetch MB from bottom field
						// change start address
						if(!(dwUpdate0 & SRC0_BOTTOM))
						{
							 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 += dwYStride;
						}
						
					}

					
				}
				else	//frame picture
				{
			  		y_adjust= 0.0f;
				   	if(motion_type == MC_FIELD)
					{
						//field prediction
						iDrawTime++; 
						dwDeltaY = 8;
						// change stride as field
						if( !( dwUpdate0 & DOUBLE_STRIDE))
						{
						   //need to double the stride of reference picture
							//and IDCT buffer stride
						   	ChangeNPTStride(1, &sstReg, TRUE);
						   	ChangeNPTStride(0, &sstReg, TRUE);

						  	dwUpdate0 |= DOUBLE_STRIDE | NEED_UPDATE |UPDATE_IDCT;
						} 

					}
					else
					{
						//don't double reference stride						
						if( dwUpdate0 & DOUBLE_STRIDE )
						{
						   	ChangeNPTStride(1, &sstReg, FALSE);
						   	ChangeNPTStride(0, &sstReg, FALSE);
							dwUpdate0 &= ~DOUBLE_STRIDE;
							dwUpdate0 |= NEED_UPDATE| UPDATE_IDCT;

						}
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							 sstReg.TMU[1].taBaseAddr0 -= dwYStride;
						 }
					}
				}	 

		
 			   	for( i = 0; i <= iDrawTime; i ++)
				{
				   
					if(fHalfMB && i)
					{
						y+= 8;
						if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x4000)) ||
						   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x8000)))
						{
							//fetch MB from top field
							if(dwUpdate0 & SRC0_BOTTOM)
							{
								 dwUpdate0 &= ~SRC0_BOTTOM;
								 dwUpdate0 |= NEED_UPDATE;
 								  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
							}
						}
						else
						{
							//fetch MB from bottom field
							// change start address
							if(!(dwUpdate0 & SRC0_BOTTOM))
							{
								 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[1].taBaseAddr0 += dwYStride;
							}
							
						}

					}

					if(wMB_Type == MB_FORWARD)
					{
						dy = lpMBInfo->PMV[i][0][1];
						srcX = x   + (lpMBInfo->PMV[i][0][0] >>1);
						half_x = (float)(lpMBInfo->PMV[i][0][0] &1) /2.0f;

					}
					else
					{
						dy = lpMBInfo->PMV[i][1][1];
						srcX = x   + (lpMBInfo->PMV[i][1][0] >>1);
						half_x = (float)(lpMBInfo->PMV[i][1][0] &1) /2.0f;

					}
				

					if(iDrawTime && !fHalfMB)
					{

						if(!i) //write to top
						{
					   	    y_adjust = 0.25f;
						   	if(!(dwUpdate0& DST_TOP_FIELD))	
							{
						    	ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,EVEN_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode

								dwUpdate0 |= DST_TOP_FIELD | UPDATE_IDCT;
								if( dwUpdate0 & DST_BOTTOM_FIELD)
								{
									sstReg.TMU[0].taBaseAddr0 -=
										pInfo->IDCTBuffer.dwStride;
									dwUpdate0 &= ~DST_BOTTOM_FIELD;
								}
							}

							if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x1000)) ||
							   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x2000)))
							{
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
								}
						
							}
							else
							{
								//fetch MB from bottom field
								// and change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 += dwYStride;
								}
							}
					
						}
						else
						{
						   //write to bottom
				   		   y_adjust = -0.25f;
						   if(!(dwUpdate0& DST_BOTTOM_FIELD))	
						   {
							   	ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,ODD_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
								dwUpdate0 &= ~DST_TOP_FIELD;
								dwUpdate0 |= DST_BOTTOM_FIELD|UPDATE_IDCT;
								sstReg.TMU[0].taBaseAddr0 += 
									pInfo->IDCTBuffer.dwStride;
						  
						   }
						   //check the new MB top /bottom for read
						   if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x4000)) ||
						   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x8000)))
						   {
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
								}
								
							}
							else
							{
							//fetch MB from bottom field
								// change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 += dwYStride;
								} 
							}
								
						} // i==1

						half_y = (float)((dy >>1) &1) /2.0f;
						//adjust Y
						srcY = (y >>1)   + (dy >>2);	//do that to macth golden image

						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/2) ;
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /2+ 8);


					}
					else 
					{
						half_y = (float)(dy &1) /2.0f;
						srcY = y   + (dy >>1) ;

						if( (bPStructure == FRAME_PICTURE) &&
							(dwUpdate0 & ( DST_BOTTOM_FIELD | DST_TOP_FIELD)))
						{
							//Change it back
							ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,0,&cmdFifo, &hwIndex);  //16 bit mode
							if(dwUpdate0& DST_BOTTOM_FIELD)
							{
								dwUpdate0 |= UPDATE_IDCT;
								sstReg.TMU[0].taBaseAddr0 -=
									pInfo->IDCTBuffer.dwStride;
							}

							dwUpdate0 &= ~(DST_TOP_FIELD|DST_BOTTOM_FIELD);
						}
						//IDCT Macroblock 
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y+ dwDeltaY);

						
					}

					if( dwUpdate0 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~NEED_UPDATE;
					}

					if(dwUpdate0 & UPDATE_IDCT)
					{
						UpdateTmuAddr( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~UPDATE_IDCT;
					}
					//IDCT Macroblock 
					vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x);
					vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x+16);

					//Reference Block
					vtxA.tmuvtx[1].s = vtxC.tmuvtx[1].s = (float)(srcX) + half_x;
					vtxB.tmuvtx[1].s = vtxD.tmuvtx[1].s = (float)(srcX + 16) + half_x;				   
					vtxA.tmuvtx[1].t = vtxB.tmuvtx[1].t = (float)(srcY) + 
											half_y + y_adjust;
					vtxC.tmuvtx[1].t = vtxD.tmuvtx[1].t = (float)(srcY + dwDeltaY) + 
											half_y + y_adjust;
					
					vtxA.x = vtxC.x = (float)(x);
					vtxB.x = vtxD.x = (float)(x + 16);
					
					if(bPStructure == FRAME_PICTURE)
					{
						vtxA.y = vtxB.y = (float)(y);
						vtxC.y = vtxD.y = (float)(y + y_offset);
					}
					else
					{
						vtxA.y = vtxB.y = (float)(y <<1);
						vtxC.y = vtxD.y = (float)((y + y_offset)<<1);
					}
					
					DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,1,&cmdFifo, &hwIndex);
				}

			 }
     
			 break;
		  case 3:				//Y Forward and Backward
//			 if( wMB_Type == (MB_FORWARD |MB_BACKWARD))
			 {

				dwDeltaY = 16;	//for Y
				fHalfMB = FALSE;
#ifdef DUMP_DATA
				if((check_data.x == x) && (check_data.y == y) &&(dwDump == dwCurrent))
				{
					dwDeltaY = 16;
					
				}
#endif
			   	if(bPStructure != FRAME_PICTURE) 
				{
					if(bPStructure == TOP_FIELD) 
						y_adjust = 0.25f;
					else
						y_adjust = -0.25f;

					if(motion_type == MC_FRAME)
					{
						//16X8 MC
						dwDeltaY = 8;
						fHalfMB = TRUE;
						iDrawTime ++;
						y_offset = 8;
				
					}

					if(!(wMBControl & 0x1000)) 
					{
						//fetch MB from top field
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
						}
					}
					else
					{
						//fetch MB from bottom field
						// change start address
						if(!(dwUpdate0 & SRC0_BOTTOM))
						{
							 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 += dwYStride;
						}
						
					}
					
					if(!(wMBControl & 0x2000)) 
					{
						//fetch MB from top field
						if(dwUpdate1 & SRC1_BOTTOM)
						{
							 dwUpdate1 &= ~SRC1_BOTTOM;
							 dwUpdate1 |= NEED_UPDATE;
 							  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
						}
					}
					else
					{
						//fetch MB from bottom field
						// change start address
						if(!(dwUpdate1 & SRC1_BOTTOM))
						{
							 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[2].taBaseAddr0 += dwYStride;
						}
					}
				}
				else	//frame picture
				{
			  		y_adjust= 0.0f;
					if(motion_type == MC_FIELD)
					{
						//field prediction

						if( !( dwUpdate0 & DOUBLE_STRIDE))
						{
						   //need to double the stride of reference picture
							//and IDCT buffer stride
							ChangeNPTStride(2, &sstReg, TRUE);
							ChangeNPTStride(1, &sstReg, TRUE);
							ChangeNPTStride(0, &sstReg, TRUE);

							dwUpdate0 |= DOUBLE_STRIDE | NEED_UPDATE|UPDATE_IDCT;
							dwUpdate1 |= NEED_UPDATE;
						} 

						iDrawTime++; 
						dwDeltaY = 8;
					}
					else
					{
						//don't double reference stride						
						if( dwUpdate0 & DOUBLE_STRIDE )
						{
							ChangeNPTStride(2, &sstReg, FALSE);
							ChangeNPTStride(1, &sstReg, FALSE);
							ChangeNPTStride(0, &sstReg, FALSE);
							dwUpdate0 &= ~DOUBLE_STRIDE;
							dwUpdate0 |= NEED_UPDATE | UPDATE_IDCT;
							dwUpdate1 |= NEED_UPDATE;


						}
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
						 }
						 if(dwUpdate1 & SRC1_BOTTOM)
						 {
							 dwUpdate1 &= ~SRC1_BOTTOM;
							 dwUpdate1 |= NEED_UPDATE;
 							  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
						 }
					}
				}

				
				for( i = 0; i <= iDrawTime; i ++)
				{
					if(fHalfMB && i)
					{
						y+=8;
						if(!(wMBControl & 0x4000)) 
						{
							//fetch MB from top field
							if(dwUpdate0 & SRC0_BOTTOM)
							{
								 dwUpdate0 &= ~SRC0_BOTTOM;
								 dwUpdate0 |= NEED_UPDATE;
 								  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
							}
						}
						else
						{
							//fetch MB from bottom field
							// change start address
							if(!(dwUpdate0 & SRC0_BOTTOM))
							{
								 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[1].taBaseAddr0 += dwYStride;
							}
							
						}
						
						if(!(wMBControl & 0x8000)) 
						{
							//fetch MB from top field
							if(dwUpdate1 & SRC1_BOTTOM)
							{
								 dwUpdate1 &= ~SRC1_BOTTOM;
								 dwUpdate1 |= NEED_UPDATE;
 								  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
							}
						}
						else
						{
							//fetch MB from bottom field
							// change start address
							if(!(dwUpdate1 & SRC1_BOTTOM))
							{
								 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[2].taBaseAddr0 += dwYStride;
							}
						}
					}
					dy  = lpMBInfo->PMV[i][0][1];
					dy1 = lpMBInfo->PMV[i][1][1];

					srcX = x  + (lpMBInfo->PMV[i][0][0] >>1);
					half_x = (float)(lpMBInfo->PMV[i][0][0] &1) /2.0f;
				  
					srcX1 = x + (lpMBInfo->PMV[i][1][0]>>1);
					half_x1 = (float)(lpMBInfo->PMV[i][1][0] &1) /2.0f;
				   

					if(iDrawTime && !fHalfMB)
					{

						if(!i) //write to top
						{
							y_adjust = 0.25f;
						   	if(!(dwUpdate0& DST_TOP_FIELD))	
							{
								ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,EVEN_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
								dwUpdate0 |= DST_TOP_FIELD | UPDATE_IDCT;
								if( dwUpdate0 & DST_BOTTOM_FIELD)
								{
									sstReg.TMU[0].taBaseAddr0 -=
										pInfo->IDCTBuffer.dwStride;
									dwUpdate0 &= ~DST_BOTTOM_FIELD;
								}
							}

						
							if(!(wMBControl & 0x1000)) 
							{
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 += dwYStride;
								}
								
							}
								
							if(!(wMBControl & 0x2000)) 
							{
								//fetch MB from top field
								if(dwUpdate1 & SRC1_BOTTOM)
								{
									 dwUpdate1 &= ~SRC1_BOTTOM;
									 dwUpdate1 |= NEED_UPDATE;
 									  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate1 & SRC1_BOTTOM))
								{
									 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[2].taBaseAddr0 += dwYStride;
								}
							}

						}
						else
						{
						   //write to bottom
						   y_adjust = -0.25f;
						   if(!(dwUpdate0& DST_BOTTOM_FIELD))	
						   {
								ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,ODD_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
								dwUpdate0 &= ~DST_TOP_FIELD;
								dwUpdate0 |= DST_BOTTOM_FIELD|UPDATE_IDCT;
								 sstReg.TMU[0].taBaseAddr0 +=
									pInfo->IDCTBuffer.dwStride;
						  
						   }
						   //check the new MB top /bottom for read
							if(!(wMBControl & 0x4000)) 
							{
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[1].taBaseAddr0 += dwYStride;
								}
								
							}
								
							if(!(wMBControl & 0x8000)) 
							{
								//fetch MB from top field
								if(dwUpdate1 & SRC1_BOTTOM)
								{
									 dwUpdate1 &= ~SRC1_BOTTOM;
									 dwUpdate1 |= NEED_UPDATE;
 									  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate1 & SRC1_BOTTOM))
								{
									 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[2].taBaseAddr0 += dwYStride;
								}
							}

						} // i==1

						half_y = (float)((dy>>1) &1) /2.0f;
						half_y1 = (float)((dy1>>1) &1) /2.0f;

						//adjust Y for field fetch
						srcY = (y >>1)   + (dy >>2);	//do that to macth golden image
						srcY1 = (y >>1)   + (dy1 >>2);	//do that to macth golden image
						
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/2);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /2+ 8);


					}
					else
					{
						half_y = (float)(dy &1) /2.0f;
						half_y1 = (float)(dy1 &1) /2.0f;
				        srcY = y    + (dy>>1);
    					srcY1 = y + (dy1 >> 1);
	
						if( (bPStructure == FRAME_PICTURE) &&
							(dwUpdate0 & ( DST_BOTTOM_FIELD | DST_TOP_FIELD)))
						{
							//Change it back
							ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,0,&cmdFifo, &hwIndex);  //16 bit mode
							if(dwUpdate0& DST_BOTTOM_FIELD)
							{
								dwUpdate0 |= UPDATE_IDCT;
								sstReg.TMU[0].taBaseAddr0 -= 
									pInfo->IDCTBuffer.dwStride;
							}

							dwUpdate0 &= ~(DST_TOP_FIELD|DST_BOTTOM_FIELD);
						}
						//IDCT Macroblock 
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y+ dwDeltaY);

					}

					if(dwUpdate0 & UPDATE_IDCT)
					{
						UpdateTmuAddr( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~UPDATE_IDCT;
					}

					if( dwUpdate0 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~NEED_UPDATE;
					}

					if( dwUpdate1 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
						dwUpdate1 &= ~NEED_UPDATE;
					}
					//IDCT Macroblock 
					vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x);
					vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x+16);
					//Reference Block
					vtxA.tmuvtx[1].s = vtxC.tmuvtx[1].s = (float)(srcX) + half_x;
					vtxB.tmuvtx[1].s = vtxD.tmuvtx[1].s = (float)(srcX + 16) + half_x;
					vtxA.tmuvtx[1].t = vtxB.tmuvtx[1].t = (float)(srcY) +
								half_y + y_adjust;
					vtxC.tmuvtx[1].t = vtxD.tmuvtx[1].t = (float)(srcY+dwDeltaY)+
								half_y + y_adjust;
					vtxA.tmuvtx[2].s = vtxC.tmuvtx[2].s = (float)(srcX1) + half_x1;
					vtxB.tmuvtx[2].s = vtxD.tmuvtx[2].s = (float)(srcX1 + 16) + half_x1;
					vtxA.tmuvtx[2].t = vtxB.tmuvtx[2].t = (float)(srcY1) + 
								half_y1 + y_adjust;
					vtxC.tmuvtx[2].t = vtxD.tmuvtx[2].t = (float)(srcY1+dwDeltaY) +
								half_y1 + y_adjust;
					
					vtxA.x = vtxC.x = (float)(x);
					vtxB.x = vtxD.x = (float)(x + 16);
					if(bPStructure == FRAME_PICTURE)
					{
						vtxA.y = vtxB.y = (float)(y);
						vtxC.y = vtxD.y = (float)(y + y_offset);
					}
					else
					{
						vtxA.y = vtxB.y = (float)(y <<1);
						vtxC.y = vtxD.y = (float)((y + y_offset)<<1);
					}
					
					DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,2,&cmdFifo, &hwIndex);
				}
			
			 }

			 break;	
     
		  case 4:				//Y Forward or Backward dual prime
//			 if( (wMB_Type &(MB_FORWARD |MB_BACKWARD) )
//				 &&(motion_type == MC_DMV))
			 {
			
				dwDeltaY = 16;	//for Y
		  		
			   	if(bPStructure != FRAME_PICTURE) 
				{
					if(bPStructure == TOP_FIELD) 
					{
						//fetch first MB from top field
						y_adjust = 0.25f;
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 -= dwYStride;
						}

						//fetch second MB from bottom field
						// change start address
						if(!(dwUpdate1 & SRC1_BOTTOM))
						{
							 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[2].taBaseAddr0 += dwYStride;
						}
					}
					else
					{
						y_adjust = -0.25f;
						//fetch first MB from bottom field
						// change start address
						if(!(dwUpdate0 & SRC0_BOTTOM))
						{
							 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[1].taBaseAddr0 += dwYStride;
						}
						
						//fetch second MB from top field
						if(dwUpdate1 & SRC1_BOTTOM)
						{
							 dwUpdate1 &= ~SRC1_BOTTOM;
							 dwUpdate1 |= NEED_UPDATE;
 							  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
						}

					}
					
				}
				else	//frame picture
				{
					dwDeltaY = 8;
					iDrawTime++; 
				}

				for( i = 0; i <= iDrawTime; i ++)
				{
				   
				  	dy = lpMBInfo->PMV[0][0][1];
				  	dy1 = lpMBInfo->PMV[i][1][1];

					srcX = x + (lpMBInfo->PMV[0][0][0]>>1);
					half_x = (float)(lpMBInfo->PMV[0][0][0] &1) /2.0f;
				  
					srcX1 = x + (lpMBInfo->PMV[i][1][0]>>1);
					half_x1 = (float)(lpMBInfo->PMV[i][1][0] &1) /2.0f;
				
				   	if(iDrawTime)
					{
						if(!i) //write to top
						{
				  	    	y_adjust= 0.25f;
						   	if(!(dwUpdate0& DST_TOP_FIELD))	
							{
								ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,EVEN_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
								dwUpdate0 |= DST_TOP_FIELD | UPDATE_IDCT;
								if(dwUpdate0 & DST_BOTTOM_FIELD)
								{
									//fetch IDCT from top field
									sstReg.TMU[0].taBaseAddr0 -=
										pInfo->IDCTBuffer.dwStride;
									dwUpdate0 &= ~DST_BOTTOM_FIELD;
								}

							}

							//fetch first MB from top field
							if(dwUpdate0 & SRC0_BOTTOM)
							{
								 dwUpdate0 &= ~SRC0_BOTTOM;
								 dwUpdate0 |= NEED_UPDATE;
 								 sstReg.TMU[1].taBaseAddr0 -= dwYStride;
							}
							//fetch second MB from bottom field
							// change start address
							if(!(dwUpdate1 & SRC1_BOTTOM))
							{
								 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[2].taBaseAddr0 += dwYStride;
							}

							if(!(dwUpdate0 & DOUBLE_STRIDE))
							{
								//double IDCT stride for field access
								ChangeNPTStride(0, &sstReg, TRUE);
								dwUpdate0 |= DOUBLE_STRIDE |UPDATE_IDCT;
							}

						}
						else
						{
						   //write to bottom field

					  		y_adjust= -0.25f;

						   if(!(dwUpdate0& DST_BOTTOM_FIELD))	
						   {
								ConfigBuffer( ppdev, SST_PE_RGB_WRMASK,2,ODD_FIELD ,&cmdFifo, &hwIndex);  //16 bit mode
								dwUpdate0 &= ~DST_TOP_FIELD;
								dwUpdate0 |= DST_BOTTOM_FIELD|UPDATE_IDCT;
								//fetch IDCT from bottom field
								 sstReg.TMU[0].taBaseAddr0 +=
									pInfo->IDCTBuffer.dwStride;
						   }
						
							//fetch first MB from bottom field
							// change start address
							if(!(dwUpdate0 & SRC0_BOTTOM))
							{
								 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[1].taBaseAddr0 += dwYStride;
							}
							
							
							//fetch second MB from top field
							if(dwUpdate1 & SRC1_BOTTOM)
							{
								 dwUpdate1 &= ~SRC1_BOTTOM;
								 dwUpdate1 |= NEED_UPDATE;
 								  sstReg.TMU[2].taBaseAddr0 -= dwYStride;
							}
						}
						half_y = (float)((dy>>1) &1) /2.0f;
					   //	half_y1 = (float)(dy1 &1) /2.0f;
						half_y1 = (float)((dy1>>1) &1) /2.0f;
						//adjust Y for field fetch
						srcY = (y >>1)   + (dy >>2);	//do that to macth golden image
						srcY1 = (y >>1)  + (dy1 >>2);	//do that to macth golden image
						//srcY1 = (y >>1)  + (dy1 >>1);	//do that to macth golden image
						
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/2);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /2+ 8);

					}
					else
					{
						half_y = (float)(dy &1) /2.0f;
						half_y1 = (float)(dy1 &1) /2.0f;

		    			srcY = y + ( dy >>1);
				    	srcY1 = y + (dy1>>1);
	
						//field picture
						//IDCT Macroblock 
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y+16);

					}

					if( dwUpdate0 & UPDATE_IDCT)
					{
						UpdateTmuAddr( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~UPDATE_IDCT;
					}
					
					if( dwUpdate0 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~NEED_UPDATE;
					}

					if( dwUpdate1 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
						dwUpdate1 &= ~NEED_UPDATE;
					}

					//IDCT Macroblock 
					vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x);
					vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x+16);
					//Reference Block
					vtxA.tmuvtx[1].s = vtxC.tmuvtx[1].s = (float)(srcX) + half_x;
					vtxB.tmuvtx[1].s = vtxD.tmuvtx[1].s = (float)(srcX+ 16) + half_x;
					vtxA.tmuvtx[1].t = vtxB.tmuvtx[1].t = (float)(srcY) +
								half_y + y_adjust;
					vtxC.tmuvtx[1].t = vtxD.tmuvtx[1].t = (float)(srcY+dwDeltaY)+
								half_y + y_adjust;
					vtxA.tmuvtx[2].s = vtxC.tmuvtx[2].s = (float)(srcX1) + half_x1;
					vtxB.tmuvtx[2].s = vtxD.tmuvtx[2].s = (float)(srcX1 + 16) + half_x1;
					vtxA.tmuvtx[2].t = vtxB.tmuvtx[2].t = (float)(srcY1) + 
								half_y1 + y_adjust;
					vtxC.tmuvtx[2].t = vtxD.tmuvtx[2].t = (float)(srcY1+dwDeltaY) +
								half_y1 + y_adjust;

					vtxA.x = vtxC.x = (float)(x);
					vtxB.x = vtxD.x = (float)(x + 16);
					if(bPStructure == FRAME_PICTURE)
					{
						vtxA.y = vtxB.y = (float)(y);
						vtxC.y = vtxD.y = (float)(y + y_offset);
					}
					else
					{
						vtxA.y = vtxB.y = (float)(y <<1);
						vtxC.y = vtxD.y = (float)((y + y_offset)<<1);
					}
									
					DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,2,&cmdFifo, &hwIndex);
				}
			
			 }

			 break;	

		  case 5:				//VU INTRA
//			if( wMB_Type == MB_INTRA)
			{
				//IDCT Macroblock 
				vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x);  //start at 16bpp boundary
				vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x+ 8);
				vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y >> 1);
				vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)((y >> 1)+ 8);
				
				vtxA.x = vtxC.x = (float)(x >> 1);  //32 bit mode -- two pixels a time
				vtxB.x = vtxD.x = (float)((x >> 1) + 8);
				if(bPStructure == FRAME_PICTURE)
				{
					vtxA.y = vtxB.y = (float)(y);
					vtxC.y = vtxD.y = (float)(y + 16);
				}
		 		else 
				{
					vtxA.y = vtxB.y = (float)(y<<1);
					vtxC.y = vtxD.y = (float)((y + 16)<<1);
				}
			
				DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,0,&cmdFifo, &hwIndex);
			
			 }
			 break;

		  case 6:		//UV Forward
//		        if( (wMB_Type == MB_FORWARD) &&
//					(motion_type != MC_DMV))
//                    goto    P_UVprediction;
//                break;
		  case 7:
                    //UV Backward
//		    if((wMB_Type == MB_BACKWARD) &&
//				(motion_type != MC_DMV))
		    {
//P_UVprediction:
#ifdef DUMP_DATA
				if((check_data.x == x) && (check_data.y == y))
					dwDeltaY = 8;
#endif
				dwDeltaY = 8;	//for Y
				fHalfMB = FALSE;		  
                if(bPStructure != FRAME_PICTURE) 
				{
				    if(bPStructure == TOP_FIELD) 
						y_adjust = 0.375f;
					else
						y_adjust = -0.125f;
					if(motion_type == MC_FRAME)
					{
						//16X8 MC
						dwDeltaY = 4;
						fHalfMB = TRUE;
						iDrawTime++;
						y_offset = 8;
					}

					if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x1000)) ||
					   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x2000)))
					{
						//fetch MB from top field
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							 sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
						}
					}
					else
					{
						//fetch MB from bottom field
						// change start address
						if(!(dwUpdate0 & SRC0_BOTTOM))
						{
							 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 							 sstReg.TMU[3].taBaseAddr0 += dwUVStride;
						}
						
					}
					
				}
				else	//frame picture
				{
					y_adjust = 0.25f;
	   				if(motion_type == MC_FIELD)
					{
						//field prediction
						iDrawTime++; 
						dwDeltaY = 4;
						
						if( !( dwUpdate0 & DOUBLE_STRIDE))
						{
						   //need to double the stride of reference picture
							//and IDCT buffer stride
							ChangeNPTStride(3, &sstReg, TRUE);
							ChangeNPTStride(2, &sstReg, TRUE);
							ChangeNPTStride(1, &sstReg, TRUE);
							ChangeNPTStride(0, &sstReg, TRUE);

							dwUpdate0 |= DOUBLE_STRIDE | NEED_UPDATE |UPDATE_IDCT;
						} 


					}
					else
					{
						
						//don't double reference stride						
						if( dwUpdate0 & DOUBLE_STRIDE )
						{
							ChangeNPTStride(3, &sstReg, FALSE);
							ChangeNPTStride(2, &sstReg, FALSE);
							ChangeNPTStride(1, &sstReg, FALSE);
							ChangeNPTStride(0, &sstReg, FALSE);
							dwUpdate0 &= ~DOUBLE_STRIDE;
							dwUpdate0 |= NEED_UPDATE| UPDATE_IDCT;

						}
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							 sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
						}
					}
				}

				
				for( i = 0; i <= iDrawTime; i ++)
				{
					if(fHalfMB && i )
					{
						y +=8;
						if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x4000)) ||
						   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x8000)))
						{
							//fetch MB from top field
							if(dwUpdate0 & SRC0_BOTTOM)
							{
								 dwUpdate0 &= ~SRC0_BOTTOM;
								 dwUpdate0 |= NEED_UPDATE;
 								 sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
							}
						}
						else
						{
							//fetch MB from bottom field
							// change start address
							if(!(dwUpdate0 & SRC0_BOTTOM))
							{
								 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 								 sstReg.TMU[3].taBaseAddr0 += dwUVStride;
							}
							
						}
					}
					if(wMB_Type == MB_FORWARD)
					{
						dy = lpMBInfo->PMV[i][0][1];

						srcX = x /2  + ((lpMBInfo->PMV[i][0][0] /2) >>1);
						half_x = (float)((lpMBInfo->PMV[i][0][0] /2) &1) /2.0f;
					}
					else
					{
						dy = lpMBInfo->PMV[i][1][1];

						srcX = x /2  + ((lpMBInfo->PMV[i][1][0] /2) >>1);
						half_x = (float)((lpMBInfo->PMV[i][1][0]/2) &1) /2.0f;

					}

				   
					if(iDrawTime && !fHalfMB)
					{
						// set correct height to prevent fecth 
						//invalid data from bottom
						if( ((DWORD)(y+ 16) >= height) &&!(dwUpdate0 & HALF_HEIGHT))
						{
							sstReg.TMU[3].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[3].taNPT |= ( height /4 -1 ) << SST_TA_NPT_T_MAX_SHIFT;
			 	 		//	sstReg.TMU[3].taMode &= ~((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
                		 // 		 (SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));
							dwUpdate0 |= NEED_UPDATE | HALF_HEIGHT;
						}


						if(!i) //write to top
						{
							y_adjust = 0.375f;

						   	//if(!(dwUpdate0& DST_TOP_FIELD))	
							{
								ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,TOP_TWO ,&cmdFifo, &hwIndex);  //32 bit mode
								dwUpdate0 |= DST_TOP_FIELD | UPDATE_IDCT;
								if(dwUpdate0 &DST_BOTTOM_FIELD)
								{
									dwUpdate0 &= ~DST_BOTTOM_FIELD;
									sstReg.TMU[0].taBaseAddr0 -=
										pInfo->IDCTBuffer.dwStride;
									sstReg.TMU[1].taBaseAddr0 =
									sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
								}
							}

							if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x1000)) ||
							   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x2000)))
							{
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									 sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
								}
								
							}
							else
							{
								//fetch MB from bottom field
								// need double reference stride
								// and change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									 sstReg.TMU[3].taBaseAddr0 += dwUVStride;
								}
							}

						}
						else
						{
						   //write to bottom
						   	y_adjust = -0.125f;

						   //if(!(dwUpdate0& DST_BOTTOM_FIELD))	
						   {
								ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,BOTTOM_TWO ,&cmdFifo, &hwIndex);  //32 bit mode
								dwUpdate0 &= ~DST_TOP_FIELD;
								dwUpdate0 |= DST_BOTTOM_FIELD|UPDATE_IDCT;
								sstReg.TMU[0].taBaseAddr0 +=
									pInfo->IDCTBuffer.dwStride;
								sstReg.TMU[1].taBaseAddr0 =
								sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
						   }
						   //check the new MB top /bottom for read
						   if(((wMB_Type== MB_FORWARD) &&!(wMBControl & 0x4000)) ||
						   ((wMB_Type== MB_BACKWARD) &&!(wMBControl & 0x8000)))
						   {
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									 sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
								}
								
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									 sstReg.TMU[3].taBaseAddr0 += dwUVStride;
								}
							}
							
						} // i==1
						if(((dy>>1) /2)&1)  //y half pixel
						{
							   half_y = 0.5f;
						}
						else
							half_y = 0.0f;

						//adjust Y for field
						srcY = y /4  + (((dy >>1)/2)>>1);	//do that to macth golden image
						//also IDCT
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/4);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /4+ 4);


					}
					else 
					{
						half_y = (float)((dy /2) &1) /2.0f;
						srcY = y /2  + ((dy /2) >>1);

						if(dwUpdate0 & HALF_HEIGHT)
						{
							sstReg.TMU[3].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[3].taNPT |= ( height /2 -1) << SST_TA_NPT_T_MAX_SHIFT;
						//	sstReg.TMU[3].taMode |= ((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
	                    //	   (SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));
							dwUpdate0 |= NEED_UPDATE;
							dwUpdate0 &= ~HALF_HEIGHT;
						}

						if( (bPStructure == FRAME_PICTURE) &&
							(dwUpdate0 & ( DST_BOTTOM_FIELD | DST_TOP_FIELD)))
						{
							//Change it back

							ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,0,&cmdFifo, &hwIndex);  //32 bit mode
							if(dwUpdate0& DST_BOTTOM_FIELD)
							{
								dwUpdate0 |= UPDATE_IDCT;
								sstReg.TMU[0].taBaseAddr0 -=
									pInfo->IDCTBuffer.dwStride;
								sstReg.TMU[1].taBaseAddr0 =
								sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
							}

							dwUpdate0 &= ~(DST_TOP_FIELD|DST_BOTTOM_FIELD);
						}
						//IDCT Macroblock 
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/2);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /2 + dwDeltaY );

					}

					if( dwUpdate0 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 3,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~NEED_UPDATE;
					}

					if(dwUpdate0 & UPDATE_IDCT)
					{
						UpdateTmuAddr( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
						UpdateTmuAddr( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
						UpdateTmuAddr( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~UPDATE_IDCT;
					}
					//IDCT Macroblock 
					vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x/2);
					vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x/2+8);
					vtxA.tmuvtx[2] = vtxA.tmuvtx[1] = vtxA.tmuvtx[0];
					vtxB.tmuvtx[2] = vtxB.tmuvtx[1] = vtxB.tmuvtx[0];
					vtxC.tmuvtx[2] = vtxC.tmuvtx[1] = vtxC.tmuvtx[0];
					vtxD.tmuvtx[2] = vtxD.tmuvtx[1] = vtxD.tmuvtx[0];

					//Reference Block
					vtxA.tmuvtx[3].s = vtxC.tmuvtx[3].s = (float)(srcX) + half_x;
					vtxB.tmuvtx[3].s = vtxD.tmuvtx[3].s = (float)(srcX + 8) + half_x;
                    //add 0.25f to offset bi-linear filter
					vtxA.tmuvtx[3].t = vtxB.tmuvtx[3].t = (float)(srcY) + 
								half_y + y_adjust;
					vtxC.tmuvtx[3].t = vtxD.tmuvtx[3].t = (float)(srcY + dwDeltaY) +
                                half_y + y_adjust;
					
					
					vtxA.x = vtxC.x = (float)(x >> 1);		//32 bit mode -- two pixels a time
					vtxB.x = vtxD.x = (float)((x >> 1)+ 8);
					if(bPStructure == FRAME_PICTURE)
					{
						vtxA.y = vtxB.y = (float)(y);
						vtxC.y = vtxD.y = (float)(y + y_offset);
					}
					else
					{
						vtxA.y = vtxB.y = (float)(y <<1);
						vtxC.y = vtxD.y = (float)((y + y_offset)<<1);
					}					
					DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,3,&cmdFifo, &hwIndex);
				}
			
			 }

			 break;
		  case 8:  			//VU Forward and Backword
//			 if( wMB_Type == (MB_FORWARD |MB_BACKWARD))
			 {
#ifdef DUMP_DATA
				if((check_data.x == x) && (check_data.y == y))
				dwDeltaY = 8;
#endif
				dwDeltaY = 8;	//for Y
				fHalfMB = 0;		  
				if(bPStructure != FRAME_PICTURE) 
				{
					if(bPStructure == TOP_FIELD) 
						y_adjust = 0.375f;
					else
						y_adjust = -0.125f;
					if(motion_type == MC_FRAME)
					{
						//16X8 MC
						dwDeltaY = 4;
						iDrawTime++;
						fHalfMB = TRUE;
						y_offset = 8;

					}
					
					if(!(wMBControl & 0x1000)) 
					{
						//fetch MB from top field
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
						}

					}
					else
					{
						//fetch MB from bottom field
						// change start address
						if(!(dwUpdate0 & SRC0_BOTTOM))
						{
							 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[3].taBaseAddr0 += dwUVStride;
						}
						
					}
					
					if(!(wMBControl & 0x2000)) 
					{
						//fetch MB from top field
						if(dwUpdate1 & SRC1_BOTTOM)
						{
							 dwUpdate1 &= ~SRC1_BOTTOM;
							 dwUpdate1 |= NEED_UPDATE;
 							  sstReg.TMU[4].taBaseAddr0 -= dwUVStride;
						}
					}
					else
					{
						//fetch MB from bottom field
						// change start address
						if(!(dwUpdate1 & SRC1_BOTTOM))
						{
							 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[4].taBaseAddr0 += dwUVStride;
						}
					}
				}
				else	//frame picture
				{
					y_adjust = 0.25f;
					if(motion_type == MC_FIELD)
					{
						//field prediction
						iDrawTime++; 
						dwDeltaY = 4;

						if( !( dwUpdate0 & DOUBLE_STRIDE))
						{
						   //need to double the stride of reference picture
							//and IDCT buffer stride
							ChangeNPTStride(4, &sstReg, TRUE);
							ChangeNPTStride(3, &sstReg, TRUE);
							ChangeNPTStride(2, &sstReg, TRUE);
							ChangeNPTStride(1, &sstReg, TRUE);
							ChangeNPTStride(0, &sstReg, TRUE);

							dwUpdate0 |= DOUBLE_STRIDE | NEED_UPDATE|UPDATE_IDCT;
							dwUpdate1 |=  NEED_UPDATE;
						} 


					}
					else
					{
						//don't double reference stride						
						if( dwUpdate0 & DOUBLE_STRIDE )
						{
							ChangeNPTStride(4, &sstReg, FALSE);
							ChangeNPTStride(3, &sstReg, FALSE);
							ChangeNPTStride(2, &sstReg, FALSE);
							ChangeNPTStride(1, &sstReg, FALSE);
							ChangeNPTStride(0, &sstReg, FALSE);
							dwUpdate0 &= ~DOUBLE_STRIDE;
							dwUpdate0 |= NEED_UPDATE | UPDATE_IDCT;
							dwUpdate1 |=  NEED_UPDATE;

						}
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
						 }
						 if(dwUpdate1 & SRC1_BOTTOM)
						 {
							 dwUpdate1 &= ~SRC1_BOTTOM;
							 dwUpdate1 |= NEED_UPDATE;
 							  sstReg.TMU[4].taBaseAddr0 -= dwUVStride;
						 }
					}
				}

				
				for( i = 0; i <= iDrawTime; i ++)
				{
					if(fHalfMB && i)
					{
						y += 8;
						if(!(wMBControl & 0x4000)) 
						{
							//fetch MB from top field
							if(dwUpdate0 & SRC0_BOTTOM)
							{
								 dwUpdate0 &= ~SRC0_BOTTOM;
								 dwUpdate0 |= NEED_UPDATE;
 								  sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
							}

						}
						else
						{
							//fetch MB from bottom field
							// change start address
							if(!(dwUpdate0 & SRC0_BOTTOM))
							{
								 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[3].taBaseAddr0 += dwUVStride;
							}
							
						}
						
						if(!(wMBControl & 0x8000)) 
						{
							//fetch MB from top field
							if(dwUpdate1 & SRC1_BOTTOM)
							{
								 dwUpdate1 &= ~SRC1_BOTTOM;
								 dwUpdate1 |= NEED_UPDATE;
 								  sstReg.TMU[4].taBaseAddr0 -= dwUVStride;
							}
						}
						else
						{
							//fetch MB from bottom field
							// change start address
							if(!(dwUpdate1 & SRC1_BOTTOM))
							{
								 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[4].taBaseAddr0 += dwUVStride;
							}
						}

					}
					dy = lpMBInfo->PMV[i][0][1];
					dy1 =lpMBInfo->PMV[i][1][1];

					srcX = x /2   + ((lpMBInfo->PMV[i][0][0] /2)>>1);
					half_x = (float)((lpMBInfo->PMV[i][0][0] /2) &1)/2.0f;
					srcX1 = x /2  + ((lpMBInfo->PMV[i][1][0] /2) >>1);
					half_x1 = (float)((lpMBInfo->PMV[i][1][0] /2) &1)/2.0f;
					
				   
					if(iDrawTime & !fHalfMB)
					{
						if( ((DWORD)(y+ 16) >= height) &&!(dwUpdate0 & HALF_HEIGHT))
						{
							sstReg.TMU[3].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[3].taNPT |= ( height /4 -1) << SST_TA_NPT_T_MAX_SHIFT;

							sstReg.TMU[4].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[4].taNPT |= ( height /4 -1) << SST_TA_NPT_T_MAX_SHIFT;
						    //sstReg.TMU[3].taMode &= ~((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
	                        //(SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));

							//sstReg.TMU[4].taMode &= ~((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
	                        //(SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));

							dwUpdate0 |= HALF_HEIGHT | NEED_UPDATE;
							dwUpdate1 |= NEED_UPDATE;
						}
						if(!i) //write to top
						{
							y_adjust = 0.375f;
						   //	if(!(dwUpdate0& DST_TOP_FIELD))	
							{
								ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,TOP_TWO ,&cmdFifo, &hwIndex);  //32 bit mode
								dwUpdate0 |= DST_TOP_FIELD | UPDATE_IDCT;
								if(dwUpdate0 &DST_BOTTOM_FIELD)
								{
									dwUpdate0 &= ~DST_BOTTOM_FIELD;
									sstReg.TMU[0].taBaseAddr0 -=
										pInfo->IDCTBuffer.dwStride;
									sstReg.TMU[1].taBaseAddr0 =
									sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
								}
							}
							
							if(!(wMBControl & 0x1000)) 
							{
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									  sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[3].taBaseAddr0 += dwUVStride;
								}
								
							}
								
							if(!(wMBControl & 0x2000)) 
							{
								//fetch MB from top field
								if(dwUpdate1 & SRC1_BOTTOM)
								{
									 dwUpdate1 &= ~SRC1_BOTTOM;
									 dwUpdate1 |= NEED_UPDATE;
 									  sstReg.TMU[4].taBaseAddr0 -= dwUVStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate1 & SRC1_BOTTOM))
								{
									 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[4].taBaseAddr0 += dwUVStride;
								}
							}

						}
						else
						{
						   //write to bottom
							y_adjust = -0.125f;

						   //if(!(dwUpdate0& DST_BOTTOM_FIELD))	
						   {
								ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,BOTTOM_TWO ,&cmdFifo, &hwIndex);  //32 bit mode
								dwUpdate0 &= ~DST_TOP_FIELD;
								dwUpdate0 |= DST_BOTTOM_FIELD|UPDATE_IDCT;
								 sstReg.TMU[0].taBaseAddr0 +=
									pInfo->IDCTBuffer.dwStride;
								sstReg.TMU[1].taBaseAddr0 =
								sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
						   }
						   //check the new MB top /bottom for read
							if(!(wMBControl & 0x4000)) 
							{
								//fetch MB from top field
								if(dwUpdate0 & SRC0_BOTTOM)
								{
									 dwUpdate0 &= ~SRC0_BOTTOM;
									 dwUpdate0 |= NEED_UPDATE;
 									  sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate0 & SRC0_BOTTOM))
								{
									 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[3].taBaseAddr0 += dwUVStride;
								}
								
							}
								
							if(!(wMBControl & 0x8000)) 
							{
								//fetch MB from top field
								if(dwUpdate1 & SRC1_BOTTOM)
								{
									 dwUpdate1 &= ~SRC1_BOTTOM;
									 dwUpdate1 |= NEED_UPDATE;
 									  sstReg.TMU[4].taBaseAddr0 -= dwUVStride;
								}
							}
							else
							{
								//fetch MB from bottom field
								// change start address
								if(!(dwUpdate1 & SRC1_BOTTOM))
								{
									 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 									  sstReg.TMU[4].taBaseAddr0 += dwUVStride;
								}
							}

						} // i==1

						half_y = (float)(((dy>>1)/2)&1)/2.0f;
						half_y1 = (float)(((dy1>>1)/2)&1)/2.0f;
						//adjust Y for field fetch
						srcY = y/4  + (((dy >>1)/2)>>1);
						srcY1 = y /4 + (((dy1>>1)/2)>>1);
						
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/4);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /4+ 4);


					}
					else
					{
						half_y = (float)((dy /2)&1)/2.0f;
						half_y1 = (float)((dy1/2)&1)/2.0f;
    					srcY = y /2   + ((dy /2)>>1);
    					srcY1 = y /2  + ((dy1/2) >>1);
	
						//enable filtering
						if(dwUpdate0 & HALF_HEIGHT)
						{
							sstReg.TMU[3].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[3].taNPT |= ( height /2 -1 ) << SST_TA_NPT_T_MAX_SHIFT;

							sstReg.TMU[4].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[4].taNPT |= ( height /2 -1) << SST_TA_NPT_T_MAX_SHIFT;
							dwUpdate0 |= NEED_UPDATE;
							dwUpdate0 &= ~ HALF_HEIGHT;
							dwUpdate1 |= NEED_UPDATE;
						}

						if( (bPStructure == FRAME_PICTURE) &&
							(dwUpdate0 & ( DST_BOTTOM_FIELD | DST_TOP_FIELD)))
						{
							//Change it back

							ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,0,&cmdFifo, &hwIndex);  //32 bit mode
							if(dwUpdate0& DST_BOTTOM_FIELD)
							{
								dwUpdate0 |= UPDATE_IDCT;
								sstReg.TMU[0].taBaseAddr0 -=
									pInfo->IDCTBuffer.dwStride;
								sstReg.TMU[1].taBaseAddr0 =
								sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
							}

							dwUpdate0 &= ~(DST_TOP_FIELD|DST_BOTTOM_FIELD);
						}
						//IDCT Macroblock 
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y /2);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y/2+ dwDeltaY);

					}

					if( dwUpdate0 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 3,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~NEED_UPDATE;
					}

					if( dwUpdate1 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 4,&cmdFifo, &hwIndex);
						dwUpdate1 &= ~NEED_UPDATE;
					}

					if(dwUpdate0 & UPDATE_IDCT)
					{
						UpdateTmuAddr( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
						UpdateTmuAddr( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
						UpdateTmuAddr( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~UPDATE_IDCT;
					}
					//IDCT Macroblock 
					vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x/2);
					vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x/2+8);
					vtxA.tmuvtx[2] = vtxA.tmuvtx[1] = vtxA.tmuvtx[0];
					vtxB.tmuvtx[2] = vtxB.tmuvtx[1] = vtxB.tmuvtx[0];
					vtxC.tmuvtx[2] = vtxC.tmuvtx[1] = vtxC.tmuvtx[0];
					vtxD.tmuvtx[2] = vtxD.tmuvtx[1] = vtxD.tmuvtx[0];

					//Reference Block
					vtxA.tmuvtx[3].s = vtxC.tmuvtx[3].s = (float)(srcX) + half_x;
					vtxB.tmuvtx[3].s = vtxD.tmuvtx[3].s = (float)(srcX + 8) + half_x;
					vtxA.tmuvtx[3].t = vtxB.tmuvtx[3].t = (float)(srcY) +
										half_y + y_adjust;
					vtxC.tmuvtx[3].t = vtxD.tmuvtx[3].t = (float)(srcY + dwDeltaY) +
										half_y + y_adjust;
					
					vtxA.tmuvtx[4].s = vtxC.tmuvtx[4].s = (float)(srcX1) + half_x1;
					vtxB.tmuvtx[4].s = vtxD.tmuvtx[4].s = (float)(srcX1+8) + half_x1;
					vtxA.tmuvtx[4].t = vtxB.tmuvtx[4].t = (float)(srcY1) +
										half_y1 + y_adjust;
					vtxC.tmuvtx[4].t = vtxD.tmuvtx[4].t = (float)(srcY1 + dwDeltaY) +
										half_y1 + y_adjust;
					
					vtxA.x = vtxC.x = (float)(x >> 1);		//32bit mode -- two pixels a time
					vtxB.x = vtxD.x = (float)((x >> 1) + 8);
					if(bPStructure == FRAME_PICTURE)
					{
						vtxA.y = vtxB.y = (float)(y);
						vtxC.y = vtxD.y = (float)(y + y_offset);
					}
					else
					{
						vtxA.y = vtxB.y = (float)(y <<1);
						vtxC.y = vtxD.y = (float)((y + y_offset)<<1);
					}					
					DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,4,&cmdFifo, &hwIndex);
				}
				
			 }

			 break;

		  case 9:  			//Dual prime
//			  if( (wMB_Type & (MB_FORWARD |MB_BACKWARD) )
//				 &&(motion_type == MC_DMV))
			 {
#ifdef DUMP_DATA
				if((check_data.x == x) && (check_data.y == y))
				dwDeltaY = 8;
#endif
				dwDeltaY = 8;	
				fHalfMB = 0;		  
				if(bPStructure != FRAME_PICTURE) 
				{
					if(bPStructure == TOP_FIELD) 
					{
						y_adjust = 0.375f;
						//fetch first MB from top field
						if(dwUpdate0 & SRC0_BOTTOM)
						{
							 dwUpdate0 &= ~SRC0_BOTTOM;
							 dwUpdate0 |= NEED_UPDATE;
 							  sstReg.TMU[3].taBaseAddr0 -= dwUVStride;
						}

						//fetch second MB from bottom field
						// change start address
						if(!(dwUpdate1 & SRC1_BOTTOM))
						{
							 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[4].taBaseAddr0 += dwUVStride;
						}
					}
					else
					{
						y_adjust = -0.125f;
						//fetch first MB from bottom field
						// change start address
						if(!(dwUpdate0 & SRC0_BOTTOM))
						{
							 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 							  sstReg.TMU[3].taBaseAddr0 += dwUVStride;
						}
						
						//fetch second MB from top field
						if(dwUpdate1 & SRC1_BOTTOM)
						{
							 dwUpdate1 &= ~SRC1_BOTTOM;
							 dwUpdate1 |= NEED_UPDATE;
 							  sstReg.TMU[4].taBaseAddr0 -=dwUVStride;
						}
					}

				}
				else	//frame picture
				{
					
					//field prediction
					iDrawTime++; 
					dwDeltaY = 4;
					//change referenced frame stride for field
					// need double reference stride
					if( !( dwUpdate0 & DOUBLE_STRIDE))
					{
						ChangeNPTStride(2, &sstReg, TRUE);
						ChangeNPTStride(1, &sstReg, TRUE);
						ChangeNPTStride(0, &sstReg, TRUE);

						dwUpdate0 |= DOUBLE_STRIDE | UPDATE_IDCT;
					} 


					
				}

				for( i = 0; i <= iDrawTime; i ++)
				{
					
					dy = lpMBInfo->PMV[0][0][1];
					dy1 = lpMBInfo->PMV[i][1][1];

					srcX = (x >>1)   + ((lpMBInfo->PMV[0][0][0] /2) >>1);
					half_x = (float)((lpMBInfo->PMV[0][0][0] /2) &1)/2.0f;
					srcX1 = (x >>1)  + ((lpMBInfo->PMV[i][1][0] /2)>>1);
					half_x1 = (float)((lpMBInfo->PMV[i][1][0] /2)&1) /2.0f;
				   
					if(iDrawTime)
					{
						if( (DWORD)(y+ 16) >= height)
						{
							sstReg.TMU[3].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[3].taNPT |= ( height /4 -1 ) << SST_TA_NPT_T_MAX_SHIFT;

							sstReg.TMU[4].taNPT &= ~ SST_TA_NPT_T_MAX;
							sstReg.TMU[4].taNPT |= ( height /4 -1 ) << SST_TA_NPT_T_MAX_SHIFT;
						    //sstReg.TMU[3].taMode &= ~((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
	                        //(SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));

							//sstReg.TMU[4].taMode &= ~((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) |
	                        //(SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT));

							dwUpdate0 |= HALF_HEIGHT | NEED_UPDATE;
							dwUpdate1 |= NEED_UPDATE;
						}
						if(!i) //write to top
						{
							y_adjust = 0.375f;
						   	//if(!(dwUpdate0& DST_TOP_FIELD))	
							{
								ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,TOP_TWO ,&cmdFifo, &hwIndex);  //32 bit mode
								dwUpdate0 |= DST_TOP_FIELD | UPDATE_IDCT;
								if(dwUpdate0 &DST_BOTTOM_FIELD)
								{
									dwUpdate0 &= ~DST_BOTTOM_FIELD;
									sstReg.TMU[0].taBaseAddr0 -=
										pInfo->IDCTBuffer.dwStride;
									sstReg.TMU[1].taBaseAddr0 =
									sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
								}
							}
					
							//fetch first MB from top field
							if(dwUpdate0 & SRC0_BOTTOM)
							{
								 dwUpdate0 &= ~SRC0_BOTTOM;
								 dwUpdate0 |= NEED_UPDATE;
 								 sstReg.TMU[3].taBaseAddr0 -=dwUVStride;
							}
							//fetch second MB from bottom field
							// change start address
							if(!(dwUpdate1 & SRC1_BOTTOM))
							{
								 dwUpdate1 |= SRC1_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[4].taBaseAddr0 +=dwUVStride;
							}

						}
						else
						{
						   //write to bottom
							y_adjust = -0.125f;

						   //if(!(dwUpdate0& DST_BOTTOM_FIELD))	
						   {
								ConfigBuffer( ppdev, SST_PE_A_WRMASK|SST_PE_G_WRMASK,4,BOTTOM_TWO ,&cmdFifo, &hwIndex);  //32 bit mode
								dwUpdate0 &= ~DST_TOP_FIELD;
								dwUpdate0 |= DST_BOTTOM_FIELD|UPDATE_IDCT;
								 sstReg.TMU[0].taBaseAddr0 +=
									pInfo->IDCTBuffer.dwStride;
								sstReg.TMU[1].taBaseAddr0 =
								sstReg.TMU[2].taBaseAddr0 = sstReg.TMU[0].taBaseAddr0;
						   }
							//fetch first MB from bottom field
							// change start address
							if(!(dwUpdate0 & SRC0_BOTTOM))
							{
								 dwUpdate0 |= SRC0_BOTTOM | NEED_UPDATE;
 								  sstReg.TMU[3].taBaseAddr0 +=dwUVStride;
							}
							
							
							//fetch second MB from top field
							if(dwUpdate1 & SRC1_BOTTOM)
							{
								 dwUpdate1 &= ~SRC1_BOTTOM;
								 dwUpdate1 |= NEED_UPDATE;
 								  sstReg.TMU[4].taBaseAddr0 -=dwUVStride;
							} 
						} // i==1

						half_y = (float)(((dy>>1)/2)&1)/2.0f;
					  	half_y1 = (float)(((dy1>>1)/2)&1)/2.0f;
					  //	half_y1 = (float)((dy1/2)&1)/2.0f;
						//adjust Y for field fetch
						srcY = y/4  + (((dy >>1)/2)>>1);
					  //	srcY1 = y /4 + ((dy1/2)>>1);
						srcY1 = y /4 + (((dy1>>1)/2)>>1);
						
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y/4);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y /4+ 4);


					}
					else
					{
						half_y = (float)((dy /2)&1)/2.0f;
						half_y1 = (float)((dy1/2)&1)/2.0f;
		    			srcY = (y >>1)   + ((dy /2) >> 1);
					    srcY1 = (y >>1)  + ((dy1/2)>>1);
							
						//IDCT Macroblock 
						vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(y /2);
						vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(y/2+ dwDeltaY);

					}

					if( dwUpdate0 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 3,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~NEED_UPDATE;
					}

					if( dwUpdate1 & NEED_UPDATE)
					{
						UpdateTmuAddr( ppdev, &sstReg, 4,&cmdFifo, &hwIndex);
						dwUpdate1 &= ~NEED_UPDATE;
					}

					if(dwUpdate0 & UPDATE_IDCT)
					{
						UpdateTmuAddr( ppdev, &sstReg, 2,&cmdFifo, &hwIndex);
						UpdateTmuAddr( ppdev, &sstReg, 1,&cmdFifo, &hwIndex);
						UpdateTmuAddr( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
						dwUpdate0 &= ~UPDATE_IDCT;
					}
					//IDCT Macroblock 
					vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(x/2);
					vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(x/2+8);
					vtxA.tmuvtx[2] = vtxA.tmuvtx[1] = vtxA.tmuvtx[0];
					vtxB.tmuvtx[2] = vtxB.tmuvtx[1] = vtxB.tmuvtx[0];
					vtxC.tmuvtx[2] = vtxC.tmuvtx[1] = vtxC.tmuvtx[0];
					vtxD.tmuvtx[2] = vtxD.tmuvtx[1] = vtxD.tmuvtx[0];

					//Reference Block
					vtxA.tmuvtx[3].s = vtxC.tmuvtx[3].s = (float)(srcX) + half_x;
					vtxB.tmuvtx[3].s = vtxD.tmuvtx[3].s = (float)(srcX+ 8) + half_x;
					vtxA.tmuvtx[3].t = vtxB.tmuvtx[3].t = (float)(srcY) +
										half_y + y_adjust;
					vtxC.tmuvtx[3].t = vtxD.tmuvtx[3].t = (float)(srcY + dwDeltaY) +
										half_y + y_adjust;
					
					vtxA.tmuvtx[4].s = vtxC.tmuvtx[4].s = (float)(srcX1) + half_x1;
					vtxB.tmuvtx[4].s = vtxD.tmuvtx[4].s = (float)(srcX1 +8) + half_x1;
					vtxA.tmuvtx[4].t = vtxB.tmuvtx[4].t = (float)(srcY1) +
										half_y1 + y_adjust;
					vtxC.tmuvtx[4].t = vtxD.tmuvtx[4].t = (float)(srcY1 + dwDeltaY) +
										half_y1 + y_adjust;
					
					vtxA.x = vtxC.x = (float)(x >> 1);		//32bit mode -- two pixels a time
					vtxB.x = vtxD.x = (float)((x >> 1) + 8);
					if(bPStructure == FRAME_PICTURE)
					{
						vtxA.y = vtxB.y = (float)(y);
						vtxC.y = vtxD.y = (float)(y + y_offset);
					}
					else
					{
						vtxA.y = vtxB.y = (float)(y <<1);
						vtxC.y = vtxD.y = (float)((y + y_offset)<<1);
					}					
					DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,4,&cmdFifo, &hwIndex);
				}
			
			 }

			 break;


		 } //switch 
		 lpMBInfo = pMBStart + lpMBInfo->wNext; 

		} //for loop
		
NextPB:
	    
		wLoop++;
    
  } // while loop


  HW_ACCESS_EXIT(ACCESS_3D);

  CMDFIFO_EPILOG( cmdFifo );  
  }

 // do{
 //       dwUpdate0 = GET(ghwIO->status);
 //   }while( dwUpdate0 & SST2_BUSY);

    //for P picture copy the data to the reference surface
	if(!pInfo->PicParms.bPicBackwardPrediction)
	{

		if ((bPStructure == FRAME_PICTURE) ||
           	 ((bPStructure != FRAME_PICTURE) && !pInfo->PicParms.bSecondField))
        {
    		pInfo->wNewBufferIndex = (pInfo->wNewBufferIndex+1) % 2;
	    	pInfo->wFlags[pInfo->CurrentIndex] =pInfo->wNewBufferIndex;
			Msg(ppdev, DEBUG_MOCOMP, "P Picture Index = %d, ref=%d",
				  pInfo->PicParms.wDecodedPictureIndex,
				  pInfo->wFlags[pInfo->CurrentIndex] );
        }

		mcCopySurface(ppdev, pInfo, pInfo->DestSurface[pInfo->CurrentIndex],
            FALSE, FRAME_PICTURE,
            &(pInfo->refSurface[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK]));

	}
	
	//copy UV from a P field for HD stream
	if(width > MAX_WIDTH)
	{
		if(pInfo->fFieldIndexReOrder)
			mcCopyUV(ppdev, pInfo, FwSurface,
			    &(pInfo->refUVBuffer[pInfo->wFlags[wFwIndex] & SURF_MASK]), FALSE,
				 bPStructure);
		else if ((bPStructure != FRAME_PICTURE) && 
				 !pInfo->PicParms.bPicBackwardPrediction)
		{
			if( !pInfo->PicParms.bSecondField)
			{
				//if it is a first P field also copy UV
				//because it can be referenced by the second field
				// Note: data is already in current destSurface, because of the previous
				// mcCopySurface()
				mcCopyUV(ppdev, pInfo, pInfo->DestSurface[pInfo->CurrentIndex],
					&(pInfo->refUVBuffer[pInfo->wNewBufferIndex]),
					FALSE,bPStructure);
				pInfo->wFlags[pInfo->CurrentIndex] |= UV_COPIED | UV_HALF;
			}
			else if(pInfo->wFlags[pInfo->CurrentIndex] & UV_HALF)
			{
				//copy the other half
				mcCopyUV(ppdev, pInfo, pInfo->DestSurface[pInfo->CurrentIndex],
					&(pInfo->refUVBuffer[pInfo->wFlags[pInfo->CurrentIndex] & SURF_MASK]),
					FALSE,bPStructure);
				pInfo->wFlags[pInfo->CurrentIndex] &= ~UV_HALF;
			}
		}
	}
	
}
/*----------------------------------------------------------------------
Function name: ClearMoCo

Description:   Delete all the surface allocated before
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/							
void ClearMoCo(GLOBALDATA *ppdev, pmcDecodingInfo *pInfo,LPDDRAWI_DIRECTDRAW_GBL lpDD)
{
	int i;
       if( (*pInfo)->lpMBBuffer)
         DXFREE((*pInfo)->lpMBBuffer);

		if((*pInfo)->IDCTBuffer.lfbPtr)
		{
			surfMgr_freeSurface(
				lpDD, 
				(*pInfo)->IDCTBuffer.lfbPtr,
				(*pInfo)->IDCTBuffer.hwPtr,
				DDSCAPS_NONLOCALVIDMEM);

		}

/*		if((*pInfo)->SubPicBuffer.lfbPtr)
		{
			surfMgr_freeSurface(
				lpDD, 
				(*pInfo)->SubPicBuffer.lfbPtr,
				(*pInfo)->SubPicBuffer.hwPtr,
				DDSCAPS_NONLOCALVIDMEM);

		}
*/
		if((*pInfo)->IA88Buffer.lfbPtr)
		{
			surfMgr_freeSurface(
				lpDD, 
				(*pInfo)->IA88Buffer.lfbPtr,
				(*pInfo)->IA88Buffer.hwPtr,
                0);

		}

		for(i = 0; i< 2; i++)
		{
			if((*pInfo)->refSurface[i].lfbPtr)
			{
				surfMgr_freeSurface(
					lpDD, 
					(*pInfo)->refSurface[i].lfbPtr,
					(*pInfo)->refSurface[i].hwPtr,
					0);

			}
		}

		for(i = 0; i< 2; i++)
		{
			if((*pInfo)->refUVBuffer[i].lfbPtr)
			{
				surfMgr_freeSurface(
					lpDD, 
					(*pInfo)->refUVBuffer[i].lfbPtr,
					(*pInfo)->refUVBuffer[i].hwPtr,
					0);

			}
		}

		DXFREE(*pInfo);
        *pInfo = 0;
}

/*----------------------------------------------------------------------
Function name: SetupPallete

Description:   Down Load textrue pallete
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/							
void SetUpPallete(GLOBALDATA *ppdev,  LPDXVA_AYUVsample lpAYUV)
{
  PALETTEENTRY sPal[256];
  int i;
#ifdef CMDFIFO
  CMDFIFO_PROLOG(cmdFifo);
#else
  FxU32 *cmdFifo;
  FxU32  hwIndex;
#endif

    HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);
    
	// load color table
    for(i = 0; i < 16; i++)
	{
		sPal[i].peRed   = lpAYUV->bCrValue;
		sPal[i].peGreen = lpAYUV->bCbValue;
		sPal[i].peBlue  = lpAYUV->bY_Value;
        lpAYUV++;

	}
	//duplicate the rest palettes
  	for( i = 1; i < (256) /16; i ++)
         memcpy( &sPal[i*16], sPal, 16 * sizeof(PALETTEENTRY));

	DownloadPalette(ppdev,sPal,&cmdFifo, &hwIndex);

   	HW_ACCESS_EXIT(ACCESS_3D);

   	CMDFIFO_EPILOG( cmdFifo );  

}


/*----------------------------------------------------------------------
Function name: AlphaCombine

Description:   Alpha Blending sub-picture
Parameters:
			   

Return:        NONE
----------------------------------------------------------------------*/							
void AlphaCombine(GLOBALDATA * ppdev,pmcDecodingInfo pInfo,
                     LPDXVA_BlendCombination pBlend )
{
  FXSURFACEDATA *dstSurface, *srcSurface;
  DWORD dwSrcAddress, dwDstAddress, dwStride;
  DWORD src_width, src_height;
  DWORD dst_width, dst_height;
  WORD wSourceIndex, wDestIndex;
  GrTexNPTInfoExt     sTexture;
  Vertex vtxA, vtxB, vtxC, vtxD;
  TMURegs  sstReg;

#ifdef CMDFIFO
  CMDFIFO_PROLOG(cmdFifo);
#else
  FxU32 *cmdFifo;
  FxU32  hwIndex;
#endif

  wSourceIndex = pBlend->wPictureSourceIndex;
  wDestIndex   = pBlend->wBlendedDestinationIndex;

  if((wSourceIndex == 0xFFFF) ||( wDestIndex == 0xFFFF))
     goto BlendDone;

  //flush AGP
  HW_ACCESS_ENTRY(cmdFifo,ACCESS_2D);

  CMDFIFO_CHECKROOM( cmdFifo, MOP_SIZE*2 );
  SETMOP(cmdFifo, SST_MOP_STALL_2D);
  SETMOP(cmdFifo, SST_MOP_AGP_FLUSH );
  HW_ACCESS_EXIT(ACCESS_2D);

  memset( &sstReg, 0, sizeof( TMURegs));


  srcSurface = pInfo->SubPicBuffer;
  dstSurface = &pInfo->IA88Buffer;
  //AGP address
  dwSrcAddress = srcSurface->lfbPtr -_FF(agpHeapLinBaseAddr) + 
					_FF(agpHeapPhysBaseAddr);
  dwDstAddress = dstSurface->hwPtr;
  dwStride = srcSurface->dwStride;

  src_width = (pBlend->PictureSourceRect16thPel.right -
          pBlend->PictureSourceRect16thPel.left) / 16;
  src_height = (pBlend->PictureSourceRect16thPel.bottom -
          pBlend->PictureSourceRect16thPel.top) /16;

  dst_width = (pBlend->PictureDestinationRect.right -
          pBlend->PictureDestinationRect.left) ;
  dst_height = (pBlend->PictureDestinationRect.bottom -
          pBlend->PictureDestinationRect.top) ;


 //first change 8 bit alpha-index AI44 surface into 16 bit AP88 surface

  sTexture.format          = SST_TA_AI44;
  sTexture.maxS            = src_width; 
  sTexture.maxT            = src_height;
  sTexture.baseAddr        = dwSrcAddress;
  sTexture.nptStride       = dwStride;
  sTexture.bFilter         = 1;

  SetNPTSourceExt(0, &sTexture,srcSurface->tileFlag, &sstReg);

  /* 3D Transformations */
  /*---- 
  	A-B
  	|\|
  	C-D
  -----*/
  vtxA.w = 1.0f;
  vtxB = vtxC = vtxD = vtxA;

  vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = 0.0f;
  vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(src_width);
  vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = 0.0f;
  vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(src_height);

  vtxA.x = vtxC.x = 0.0f;
  vtxB.x = vtxD.x = (float)(dst_width);
  vtxA.y = vtxB.y = 0.0f;
  vtxC.y = vtxD.y = (float)(dst_height);

  TMULoad(0, &sstReg);
  //set registers
  HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);
  preSetRegs( ppdev, dstSurface, dst_height,&cmdFifo, &hwIndex);

  SetupTmus( ppdev, &sstReg, 0,&cmdFifo, &hwIndex);
  ConfigBuffer( ppdev, SST_PE_ARGB_WRMASK,2, 0,&cmdFifo, &hwIndex);  //16bit mode

  DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,0,&cmdFifo, &hwIndex);

  //set up alpha blending
  memset( &sstReg, 0, sizeof( TMURegs));

  srcSurface = dstSurface;
  dstSurface = pInfo->DestSurface[wDestIndex];
  dwSrcAddress = srcSurface->hwPtr;
  dwStride = srcSurface->dwStride;
  src_width = dst_width;
  src_height = dst_height;
  sTexture.format          = SST_TA_AP88;
  sTexture.maxS            = src_width; 
  sTexture.maxT            = src_height;
  sTexture.baseAddr        = dwSrcAddress;
  sTexture.nptStride       = dwStride;
  sTexture.bFilter         = 0;

  SetNPTSourceExt(4, &sTexture,srcSurface->tileFlag, &sstReg);
  SetNPTSourceExt(3, &sTexture,srcSurface->tileFlag, &sstReg);
  SetNPTSourceExt(2, &sTexture,srcSurface->tileFlag, &sstReg);
  SetNPTSourceExt(1, &sTexture,srcSurface->tileFlag, &sstReg);

  dwDstAddress = dstSurface->hwPtr;
  dwStride = dstSurface->dwStride;

  sTexture.format          = SST_TA_ARGB8888;
  sTexture.maxS            = pInfo->CodedWidth/2; 
  sTexture.maxT            = pInfo->CodedHeight;
  sTexture.baseAddr        = dwDstAddress;
  sTexture.nptStride       = dwStride;
  sTexture.bFilter         = 0;
	 
  SetNPTSourceExt(0, &sTexture,dstSurface->tileFlag, &sstReg);
	
  /* 3D Transformations */
  /*---- 
   	A-B
	|\|
	C-D
	-----*/
  vtxA.w = 1.0f;
  vtxB = vtxC = vtxD = vtxA;
  vtxA.tmuvtx[4].s = vtxC.tmuvtx[4].s = 0.0f;
  vtxB.tmuvtx[4].s = vtxD.tmuvtx[4].s = (float)(src_width);
  vtxA.tmuvtx[4].t = vtxB.tmuvtx[4].t = 0.0f;
  vtxC.tmuvtx[4].t = vtxD.tmuvtx[4].t = (float)(src_height);

  vtxA.tmuvtx[3].s = vtxC.tmuvtx[3].s = 1.0f;
  vtxB.tmuvtx[3].s = vtxD.tmuvtx[3].s = (float)(src_width+1);		//1 offset
  vtxA.tmuvtx[3].t = vtxB.tmuvtx[3].t = 0.0f;
  vtxC.tmuvtx[3].t = vtxD.tmuvtx[3].t = (float)(src_height);

  vtxA.tmuvtx[2] = vtxA.tmuvtx[1] = vtxA.tmuvtx[3];
  vtxB.tmuvtx[2] = vtxB.tmuvtx[1] = vtxB.tmuvtx[3];
  vtxC.tmuvtx[2] = vtxC.tmuvtx[1] = vtxC.tmuvtx[3];
  vtxD.tmuvtx[2] = vtxD.tmuvtx[1] = vtxD.tmuvtx[3];
        
  vtxA.tmuvtx[0].s = vtxC.tmuvtx[0].s = (float)(pBlend->GraphicSourceRect.left/2);
  vtxB.tmuvtx[0].s = vtxD.tmuvtx[0].s = (float)(pBlend->GraphicSourceRect.right/2);
  vtxA.tmuvtx[0].t = vtxB.tmuvtx[0].t = (float)(pBlend->GraphicSourceRect.top);
  vtxC.tmuvtx[0].t = vtxD.tmuvtx[0].t = (float)(pBlend->GraphicSourceRect.bottom);
	   
  vtxA.x = vtxC.x = (float)(pBlend->GraphicSourceRect.left/2);
  vtxB.x = vtxD.x = (float)(pBlend->GraphicSourceRect.right/2);
  vtxA.y = vtxB.y = (float)(pBlend->GraphicSourceRect.top);
  vtxC.y = vtxD.y = (float)(pBlend->GraphicSourceRect.bottom);

  TMUBlend(4, &sstReg);
  //wait for done
  while( GET(ghwIO->status) & (SST2_BUSY) )
  		;

  //set registers
  preSetRegs( ppdev, dstSurface, pInfo->CodedHeight,&cmdFifo, &hwIndex);
  ConfigBuffer( ppdev, SST_PE_ARGB_WRMASK,4, 0,&cmdFifo, &hwIndex);  //32bit mode

  SetupTmus( ppdev, &sstReg, 4,&cmdFifo, &hwIndex);
  DrawRect(ppdev, &vtxA, &vtxB, &vtxC, &vtxD ,4,&cmdFifo, &hwIndex);


  HW_ACCESS_EXIT(ACCESS_3D);

BlendDone:
  CMDFIFO_EPILOG( cmdFifo );  

}


 /*---------------------------------------
 * add macroblock with previous one
 *
---------------------------------------*/

void TMUAddYDelta(FxU8 tmuNum, TMURegs *sst)
{

   
   //load current texture
    //MC is 16bpp with the heigh byte as 
    //sign byte, which is loaded in alpha
	
   //to not overbright color components
   //first shift color input by 4 then devide them by 16
   //set constant color to filter out A= R = G = 1/16, B= 1/16
   sst->TMU[tmuNum].taColorAR0 = (0x10<< SST_TA_CONSTANT_COLOR0_ALPHA_SHIFT) |
    								(0x10<< SST_TA_CONSTANT_COLOR0_RED_SHIFT);
   sst->TMU[tmuNum].taColorGB0 = (0x10<< SST_TA_CONSTANT_COLOR0_GREEN_SHIFT) |
    					(0x10<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT);	
	
    // for YUYV
    // a = Y  Y  Y				  
    // b = 0  0  0
    // c=  C0  C0  C0
    // d=  0  0  0
	//output Y... 

   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         4,				//shift left by 4
                         0,
                         0,
                         sst);
	//load alpha   with the sign byte
	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);


  // a = -s_Y  -s_Y  -sY
  // b = Yp      Yp    Yp
  // c=  1      1     1
  // d=   Y     Y     Y	
  //output Y + Yp -s_Y  ...

	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ATEX,
	                SST_TA_INV_MINUS,
	                SST_TA_CCC_CPREV,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_ONE_MINUS,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //zero alpha
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);



}
 /*---------------------------------------
 * average with previous one
 *
---------------------------------------*/
 
void TMUAverage(FxU8 tmuNum ,TMURegs *sst)
{

   //set constant color to filter out A= R = G = 1, B= 1
   sst->TMU[tmuNum].taColorAR0 = (0x1<< SST_TA_CONSTANT_COLOR0_ALPHA_SHIFT) |
    								(0x1<< SST_TA_CONSTANT_COLOR0_RED_SHIFT);
   sst->TMU[tmuNum].taColorGB0 = (0x1<< SST_TA_CONSTANT_COLOR0_GREEN_SHIFT) |
    					(0x1<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT);	
     //load current texture

   	// for YUYV
    // a = Yp  Yp  Yp		//previous Y
    // b = color0 .....
    // c=  1  1  1
    // d=  Y  Y  Y
  //output Y + Yp +1 .....   

   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CPREV,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,         
	                     SST_TA_INV_ONE_MINUS,
                         SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//same for alpha 
	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_A0,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_ONE_MINUS,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

	//devide result by half
    // a = -TCU result
    // c = -0.5
    
 	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_MINUS,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_MINUS_HALF,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //zero alpha
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_MINUS,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_MINUS_HALF,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);


}

 /*---------------------------------------
 * add macroblock with previous one
 *
---------------------------------------*/

void TMUAddVUDelta(FxU8 tmuNum, TMURegs *sst)
{

   
   //Fisrt save the prevouse result
   
   sst->TMU[tmuNum+1].taCcuControl = SST_TA_CUC_REG_LOAD |SST_TA_CUA_REG_LOAD;
   //load current texture
   //MC is 16bpp with the heigh byte as 
   //signe byte, which is loaded in alpha
   
    //set constant color to filter out  R G
	sst->TMU[tmuNum].taColorAR0 = 0;
    sst->TMU[tmuNum].taColorGB0 = 0x10<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT;	
    // 
    // a =   V  sU  U
    // b =   0  0   0 
    // c=    0  0   1/16														+
    // d=    0  0   0
  //output   0  0   U

   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         4,		//shift 4
                         0,
                         0,
                         sst);
   //nothing in alpha
    
   TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

   // nothing for color   
   ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //alpha	from TCU R+G+B = U
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_CTCUSUM,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

    tmuNum--;			//next TMU
    //set constant color to filter out  R B
	sst->TMU[tmuNum].taColorAR0 = 0;
    sst->TMU[tmuNum].taColorGB0 = 0x100<< SST_TA_CONSTANT_COLOR0_GREEN_SHIFT;	
    // 
    // a =   V  sU  U
    // b =   0  1   0 
    // c=    0  0   0
    // d=    0  0   0
  //output   0 sU   0

   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
   //nothing in alpha
    
   TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

   // nothing for color   
   ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //alpha	from  a= -( TCU R+G+B) = -sU
    //			  d = prev alpha = U
    //output			U- sU = Umc
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_CTCUSUM,
	                SST_TA_INV_MINUS,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_ONE_MINUS,
	                SST_TA_CCA_APREV,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

	tmuNum--;
    
   //set constant color to filter out  G B
	sst->TMU[tmuNum].taColorAR0 = 0x10<< SST_TA_CONSTANT_COLOR0_RED_SHIFT;
    sst->TMU[tmuNum].taColorGB0 = 0;	
    // 
    // a =   V  sU  U
    // b =   0  0   0 
    // c=    1/16  0   0
    // d=    0  0   0
  //output   V  0   0

   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         4,			//shift 4
                         0,
                         0,
                         sst);
	//alpha = 0
    
   TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);


  // a =			  CREG  CREG  CREG
  // b = 		      0     0	  0
  // c=  		      1     1	  1
  // d=(prev alpha)  Umc  Umc  Umc
  //output			 Umc+CREG  ...

	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_CREG,
	                SST_TA_INV_NONE,
                    SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_ONE_MINUS,
	                SST_TA_CCC_APREV,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

    //alpha = -sV + (R+G+B) = V- sV + AREG
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ATEX,
	                SST_TA_INV_MINUS,
	                SST_TA_CCA_AREG,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_ONE_MINUS,
	                SST_TA_CCA_CTCUSUM,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);



}

 /*---------------------------------------
 * add two texture
 *
---------------------------------------*/
void TMUAdd(FxU8 tmuNum ,TMURegs *sst)
{
   //load current texture

    // a = U  U  U
    // b = 0  0  0
    // c=  1  1  1
    // d=  Umc Umc Umc 
  //output Umc+1 Umc+U Umc+U  

   TexCombineColorExt( tmuNum,
                         SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_ONE_MINUS,
	                     SST_TA_TCC_CPREV,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//load alpha with V
    //add with Vmc
	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_ONE_MINUS,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
                      0,
                      0,
                      0,
                      sst);

	//pass result
 	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //pass alpha
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);


}

 /*---------------------------------------
 * Alpha blending sub-picture
 *
---------------------------------------*/

void TMUBlend(FxU8 tmuNum, TMURegs *sst)
{

   //TMU4 -- texture 0
   //TMU3 -- texture 1
   //TMU2 -- don't care
   //TMU1 -- texture 1
   //TMU0 -- overlay surface
   
   
   //load texture 0	  in TMU 4
   
    // 
    // a =   0  0   0
    // b =   0  0   0 
    // c=    0  0   0
    // d=    V0 U0  Y0
  //output   V0 U0  Y0
  	TMULoad( tmuNum, sst);
   tmuNum--;			//next TMU 3
     
  //load texture1 and average with texture0

  // for YUYV
  // a = V0 U0  Y0		//previous Y
  // b = V1 U1  Y1
  // c=  .5 .5  0
  // d=  0   0  0
  //output (V0+V1)/2 (U0+U1)/2 0   

   //set constant color to filter out A= R = G = 1, B= .5
   sst->TMU[tmuNum].taColorAR0 = (0x100<< SST_TA_CONSTANT_COLOR0_ALPHA_SHIFT) |
    								(0x100<< SST_TA_CONSTANT_COLOR0_RED_SHIFT);
   sst->TMU[tmuNum].taColorGB0 = (0x100<< SST_TA_CONSTANT_COLOR0_GREEN_SHIFT) |
    					(0x80<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT);	
  
   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CPREV,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,         
	                     SST_TA_INV_MINUS_HALF,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//same for alpha 
	TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ATEX,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_A0,
	                  SST_TA_INV_MINUS_HALF,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

   //set constant color to filter out A= R = G = 0 , B= 1
    sst->TMU[tmuNum].taColorAR1 = 0;
    sst->TMU[tmuNum].taColorGB1 = 0x100<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT;
  
  	// a =    V0   U0   Y0
    // b =    0    0     0
    // c =	  0    0     1
    // d =    VA   UA    0
    //output  VA   UA    Y0
 	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_CPREV,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_C1,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //pass alpha
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);


   
   //output   (a1+a0)/2 (V1+V0)/2= VA (U0 + U1)/2= UA   Y0
   //save the alpha
  
   sst->TMU[tmuNum].taCcuControl = SST_TA_CUA_REG_LOAD;
 
   tmuNum--;			//next TMU	2
   
  
   //move VA into alpha
   //set constant color to filter out  G B
	sst->TMU[tmuNum].taColorAR0 = 0x100<< SST_TA_CONSTANT_COLOR0_RED_SHIFT;
    sst->TMU[tmuNum].taColorGB0 = 0;	
   
   //	a =  VA  UA  Y0
   //	b =  0   0    0
   //	c =  1   0    0
   //	d =  0   0    0
   //result  VA  0    0
   
   TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CPREV,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
   //nothing in alpha
    
   TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

   // 	a =  VA  UA  Y0
   //	b =  0   0   0
   //   c =  0   1   1
   //   d =  0	 0	 0
   //result  0	 UA	 Y0
 
   //set constant color to filter out  R
	sst->TMU[tmuNum].taColorAR1 = 0;
    sst->TMU[tmuNum].taColorGB1 = (0x100<< SST_TA_CONSTANT_COLOR0_GREEN_SHIFT) |
    						(0x100<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT);
     
   ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_CPREV,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_C1,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
    //alpha	from  a = b = c = 0
    //		d= ( TCU R+G+B) = VA
    //output		  VA
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_CTCUSUM,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);
							
    tmuNum--;			//next TMU1
     
    // 
    // a =   V1 U1  Y1
    // b =   0  0   0 
    // c=    0  0   1
    // d=    0  0   0
    //output   0  0   Y1

    //set constant color to filter out  R G
	sst->TMU[tmuNum].taColorAR0 = 0;
    sst->TMU[tmuNum].taColorGB0 = (0x100<< SST_TA_CONSTANT_COLOR0_BLUE_SHIFT);
    TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CTEX,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_C0,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//alpha = previous alpha = VA
    						 
   TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

    //set constant color to filter out  G B
  	sst->TMU[tmuNum].taColorAR1 = 0x100<< SST_TA_CONSTANT_COLOR0_RED_SHIFT;
    sst->TMU[tmuNum].taColorGB1 = 0;
  
  // a =(tcu_sum =Y1) Y1	Y1	  Y1
  // b = 		      0     0	  0
  // c=  		      1     0	  0
  // d=(previous)  	  0     UA    Y0
  //output			  Y1    UA	  Y0  

	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_CTCUSUM,
	                SST_TA_INV_NONE,
                    SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_C1,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_CPREV,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

    //alpha = prev output = VA
	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

	tmuNum--;			//next tmu 0
  
   //do the alpha combine
   
   // a =  Y1 UA  Y0
   // b =  0  0   0
   // c = al al  al
   // d =  0  0   0
   
    TexCombineColorExt( tmuNum,
	                     SST_TA_TCC_CPREV,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
	                     SST_TA_TCC_AREG,
	                     SST_TA_INV_NONE,
                         SST_TA_TCC_ZERO,
	                     SST_TA_INV_NONE,
                         0,
                         0,
                         0,
                         sst);
	//alpha  do the same
    						 
   TexCombineAlphaExt( tmuNum,
	                  SST_TA_TCA_APREV,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_AREG,
	                  SST_TA_INV_NONE,
	                  SST_TA_TCA_ZERO,
	                  SST_TA_INV_NONE,
	                  0,
                      0,
                      0,
                      sst);

  
  // a =			  y1    u	  y0
  // b = 		      0     0	  0
  // c=  		      1-al  1-al  1-la
  // d=(previous)  	  

	ColorCombineColorExt( tmuNum,
	                SST_TA_CCC_CTEX,
	                SST_TA_INV_NONE,
                    SST_TA_CCC_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCC_AREG,
	                SST_TA_INV_ONE_MINUS,
	                SST_TA_CCC_CTCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

    //alpha  do the same
   	ColorCombineAlphaExt( tmuNum,
	                SST_TA_CCA_ATEX,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_ZERO,
	                SST_TA_INV_NONE,
	                SST_TA_CCA_AREG,
	                SST_TA_INV_ONE_MINUS,
	                SST_TA_CCA_ATCU,
	                SST_TA_INV_NONE,
                    0,
                    0,
                    sst);

    
   
}

  
#endif	// dx7

#endif
