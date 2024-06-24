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
*/


#include "precomp.h"
#include "ddovl32.h"
#include "ddvpe32.h"
#include "fxglobal.h"
#include "sst2bwe.h"
#include "sst2glob.h"
#include "fifomgr.h"

#define DEBUG_VPEENTRY 0
#define FXTRACE 1
/**************************************************************************
* Function prototyp
**************************************************************************/
void ResetVIP( NT9XDEVICEDATA *ppdev);

static DWORD __stdcall vpCanCreateVideoPort32 (LPDDHAL_CANCREATEVPORTDATA lpInput);
static DWORD __stdcall vpCreateVideoPort32 (LPDDHAL_CREATEVPORTDATA lpInput);
static DWORD __stdcall vpDestroyVideoPort32 (LPDDHAL_DESTROYVPORTDATA lpInput);
static DWORD __stdcall vpFlipVideoPort32 (LPDDHAL_FLIPVPORTDATA lpInput);
static DWORD __stdcall vpGetVideoPortBandwidth32 (LPDDHAL_GETVPORTBANDWIDTHDATA lpInput);
static DWORD __stdcall vpGetVideoPortField32 (LPDDHAL_GETVPORTFIELDDATA lpInput);
static DWORD __stdcall vpGetVideoPortFlipStatus32 (LPDDHAL_GETVPORTFLIPSTATUSDATA lpInput);
static DWORD __stdcall vpGetVideoPortInputFormats32 (LPDDHAL_GETVPORTINPUTFORMATDATA lpInput);
static DWORD __stdcall vpGetVideoPortLine32 (LPDDHAL_GETVPORTLINEDATA lpInput);
static DWORD __stdcall vpGetVideoPortOutputFormats32 (LPDDHAL_GETVPORTOUTPUTFORMATDATA lpInput);
static DWORD __stdcall vpGetVideoPortConnectInfo32 (LPDDHAL_GETVPORTCONNECTDATA lpInput);
static DWORD __stdcall vpUpdateVideoPort32 (LPDDHAL_UPDATEVPORTDATA lpInput);
static DWORD __stdcall vpWaitForVideoPortSync32 (LPDDHAL_WAITFORVPORTSYNCDATA lpInput);
static DWORD __stdcall vpGetVideoSignalStatus32 (LPDDHAL_GETVPORTSIGNALDATA lpInput);
static DWORD __stdcall vpVideoPortColorControl32 (LPDDHAL_VPORTCOLORDATA lpInput);
static DWORD __stdcall vpSyncSurfaceData32 (LPDDHAL_SYNCSURFACEDATA lpInput);
static DWORD __stdcall vpSyncVideoPortData32 (LPDDHAL_SYNCVIDEOPORTDATA lpInput);
static void WaitForVSYNC( NT9XDEVICEDATA *, __int64 * ,__int64 *);
static DDVIDEOPORTCONNECT sConnect[ ] =
{

    {
    // DDVPTYPE_CCIR656, 8 bit
    sizeof (DDVIDEOPORTCONNECT),
    8,
    {0xFCA326A0L,0xDA60,0x11CF,0x9B,0x06,0x00,0xA0,0xC9,0x03,0xA3,0xB8},
    DDVPCONNECT_DISCARDSVREFDATA |DDVPCONNECT_INVERTPOLARITY,
    0
    }
};


/*
----------------------------------------------------------------------------
  Fill the INFODATA sturcture
  Return: 0 if the INFODATA structure cannot be filled
          otherwise 1.
----------------------------------------------------------------------------
*/

DWORD __stdcall VideoGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpInput)
{

    DWORD dwSize, dwRetCode;

    dwRetCode = 0;

    if (IsEqualIID(&lpInput->guidInfo, &GUID_VideoPortCallbacks))
    {
        DDHAL_DDVIDEOPORTCALLBACKS vpCallbacks;
        memset(&vpCallbacks, 0, sizeof(vpCallbacks));

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDVIDEOPORTCALLBACKS));
        lpInput->dwActualSize = sizeof(DDHAL_DDVIDEOPORTCALLBACKS);
        vpCallbacks.dwSize = dwSize;

        vpCallbacks.dwFlags =  DDHAL_VPORT32_CANCREATEVIDEOPORT |
                               DDHAL_VPORT32_CREATEVIDEOPORT    |
                               DDHAL_VPORT32_FLIP               |
                               DDHAL_VPORT32_GETBANDWIDTH       |
                               DDHAL_VPORT32_GETINPUTFORMATS    |
                               DDHAL_VPORT32_GETOUTPUTFORMATS   |
                               DDHAL_VPORT32_GETFIELD           |
                               DDHAL_VPORT32_GETLINE            |
                               DDHAL_VPORT32_GETCONNECT         |
                               DDHAL_VPORT32_DESTROY            |
                               DDHAL_VPORT32_GETFLIPSTATUS      |
                               DDHAL_VPORT32_UPDATE             |
                               DDHAL_VPORT32_WAITFORSYNC        |
                               DDHAL_VPORT32_GETSIGNALSTATUS    |
                             //DDHAL_VPORT32_COLORCONTROL       |
                               0;

        vpCallbacks.CanCreateVideoPort = vpCanCreateVideoPort32;
        vpCallbacks.CreateVideoPort = vpCreateVideoPort32;
        vpCallbacks.FlipVideoPort = vpFlipVideoPort32;
        vpCallbacks.GetVideoPortBandwidth = vpGetVideoPortBandwidth32;
        vpCallbacks.GetVideoPortInputFormats = vpGetVideoPortInputFormats32;
        vpCallbacks.GetVideoPortOutputFormats = vpGetVideoPortOutputFormats32;
        vpCallbacks.GetVideoPortField = vpGetVideoPortField32;
        vpCallbacks.GetVideoPortLine = vpGetVideoPortLine32;
        vpCallbacks.GetVideoPortConnectInfo = vpGetVideoPortConnectInfo32;
        vpCallbacks.DestroyVideoPort = vpDestroyVideoPort32;
        vpCallbacks.GetVideoPortFlipStatus = vpGetVideoPortFlipStatus32;
        vpCallbacks.UpdateVideoPort = vpUpdateVideoPort32;
        vpCallbacks.WaitForVideoPortSync = vpWaitForVideoPortSync32;
        vpCallbacks.GetVideoSignalStatus = vpGetVideoSignalStatus32;
        //vpCallbacks.ColorControl = vpColorControl32;

        memcpy(lpInput->lpvData, &vpCallbacks, dwSize);
        dwRetCode = 1;
    }
    else  if (IsEqualIID(&lpInput->guidInfo, &GUID_VideoPortCaps))
    {
        DDVIDEOPORTCAPS vpCaps;
        memset(&vpCaps, 0, sizeof(vpCaps));

        dwSize = min(lpInput->dwExpectedSize, sizeof(DDVIDEOPORTCAPS));
        lpInput->dwActualSize = sizeof(DDVIDEOPORTCAPS);
        vpCaps.dwSize = dwSize;           // size of the DDVIDEOPORTCAPS structure
        vpCaps.dwFlags = DDVPD_WIDTH   |  // indicates which fields contain data
                         DDVPD_HEIGHT  |
                         DDVPD_ID      |
                         DDVPD_CAPS    |
                         DDVPD_FX      |
                         DDVPD_AUTOFLIP|
                         DDVPD_ALIGN   |
                         DDVPD_PREFERREDAUTOFLIP |
                         0;

        vpCaps.dwMaxWidth = SST2_MAX_VID_IN_X; // max width of the video port field
        vpCaps.dwMaxVBIWidth = 0;            // max width of the VBI data
        vpCaps.dwMaxHeight = SST2_MAX_VID_IN_Y;// max height of the video port field
        vpCaps.dwVideoPortID = SST2_PORT_ID;   // Video port ID (0 - (dwMaxVideoPorts -1))
        vpCaps.dwCaps =
                        DDVPCAPS_AUTOFLIP               |
                        DDVPCAPS_INTERLACED             |
                        DDVPCAPS_NONINTERLACED          |
                        DDVPCAPS_READBACKFIELD          |
                        DDVPCAPS_READBACKLINE           |
            //          DDVPCAPS_SHAREABLE              |
                        DDVPCAPS_SKIPEVENFIELDS         |
                        DDVPCAPS_SKIPODDFIELDS          |
            //          DDVPCAPS_SYNCMASTER             |
                        DDVPCAPS_VBISURFACE             |
            //          DDVPCAPS_COLORCONTROL           |
                        DDVPCAPS_OVERSAMPLEDVBI         |
#ifdef AGP_EXECUTE
                        DDVPCAPS_SYSTEMMEMORY           |  //Go to AGP ?
#endif
                        DDVPCAPS_VBIANDVIDEOINDEPENDENT |
           //             DDVPCAPS_HARDWAREDEINTERLACE    |  //We might have this
                        0;

        vpCaps.dwFX =   DDVPFX_CROPTOPDATA |   // More video port capabilities
                        DDVPFX_CROPX |
                        DDVPFX_CROPY |
                        DDVPFX_INTERLEAVE |
                        // DDVPFX_MIRRORLEFTRIGHT |
                        // DDVPFX_MIRRORUPDOWN    |
                        DDVPFX_PRESHRINKX |
                        DDVPFX_PRESHRINKY |
                        // DDVPFX_PRESHRINKXB |
                        // DDVPFX_PRESHRINKYB |
                        // DDVPFX_PRESHRINKXS |
                        // DDVPFX_PRESHRINKYS |
                        // DDVPFX_PRESTRETCHX |
                        // DDVPFX_PRESTRETCHY |
                        // DDVPFX_PRESTRETCHXN|
                        // DDVPFX_PRESTRETCHYN|
                        DDVPFX_VBICONVERT  |
                        DDVPFX_VBINOSCALE  |
                        DDVPFX_IGNOREVBIXCROP |
                        DDVPFX_VBINOINTERLEAVE|
                        0;

        vpCaps.dwNumAutoFlipSurfaces = 4;    // Number of autoflippable surfaces
        vpCaps.dwAlignVideoPortBoundary = 4; // Byte restriction of placement within the surface
        vpCaps.dwAlignVideoPortPrescaleWidth = 4;// Byte restriction of width after prescaling
        vpCaps.dwAlignVideoPortCropBoundary = 4; // Byte restriction of left cropping
        vpCaps.dwAlignVideoPortCropWidth = 4;    // Byte restriction of cropping width
        vpCaps.dwNumVBIAutoFlipSurfaces = 2;     // Number of VBI autoflippable surfaces
        vpCaps.dwNumPreferredAutoflip = 4;    // Number of autoflippable surfaces

        memcpy(lpInput->lpvData, &vpCaps, dwSize);
        dwRetCode = 1;
    }
    else if (IsEqualIID(&lpInput->guidInfo, &GUID_KernelCallbacks))
    {
         DDHAL_DDKERNELCALLBACKS kCallbacks;

         memset(&kCallbacks, 0, sizeof(kCallbacks));
         dwSize = min(lpInput->dwExpectedSize, sizeof(DDHAL_DDKERNELCALLBACKS));
         lpInput->dwActualSize = sizeof(DDHAL_DDKERNELCALLBACKS);
         kCallbacks.dwSize = dwSize;               // size of the DDHAL_DDKERNELCALLBACKS structure
         kCallbacks.dwFlags = DDHAL_KERNEL_SYNCSURFACEDATA |
                              DDHAL_KERNEL_SYNCVIDEOPORTDATA |
                              0;
         kCallbacks.SyncSurfaceData = vpSyncSurfaceData32;
         kCallbacks.SyncVideoPortData = vpSyncVideoPortData32;
         memcpy(lpInput->lpvData, &kCallbacks, dwSize);
    }
    else  if (IsEqualIID(&lpInput->guidInfo, &GUID_KernelCaps))
    {
        DDKERNELCAPS kCaps;

        memset(&kCaps, 0, sizeof(kCaps));
        dwSize = min(lpInput->dwExpectedSize, sizeof(DDKERNELCAPS));
        lpInput->dwActualSize = sizeof(DDKERNELCAPS);
        kCaps.dwSize = dwSize;                 // size of the DDKERNELCAPS structure
        kCaps.dwCaps =  DDKERNELCAPS_SKIPFIELDS |        //Skip odd or even fields by using hw or vddSkipNextField
//Useing HW                        DDKERNELCAPS_AUTOFLIP |        //Support vddFlipVideoPort and vddFlipOverlay
                        DDKERNELCAPS_SETSTATE |        //Support vddSetState to switch between BOB & WEAVE
                        //DDKERNELCAPS_LOCK     |       //In tile mode this is not supported 
                        DDKERNELCAPS_FLIPVIDEOPORT |     //Support vddFlipVideoPort
                        DDKERNELCAPS_FLIPOVERLAY |        //Support vddFlipOverlay
                        //DDKERNELCAPS_I2C |                //This flag is NOT defined anywhere
                        //DDKERNELCAPS_GPIO |            //This flag is NOT defined anywhere
                        //DDKERNELCAPS_CAPTURE_SYSMEM |     //Kernel support capture to system memory
                        //DDKERNELCAPS_CAPTURE_NONLOCALVIDMEM | //Kernel support capture to non local video memory
                        DDKERNELCAPS_FIELDPOLARITY |    //Can report polarity of video field
                        //DDKERNELCAPS_CAPTURE_INVERTED |//Can capture and invert the DIB
                        //DDKERNELCAPS_TRANSFER |        //Support data xfer between system memory and fb
                        0;
        kCaps.dwIRQCaps =   DDIRQ_DISPLAY_VSYNC |        //Device generates display VSync
                            DDIRQ_VPORT0_VSYNC  |        //Device generates video VSync on the Video Port 0
                        0;
         memcpy(lpInput->lpvData, &kCaps, dwSize);
         dwRetCode = 1;

    }
    return (dwRetCode);
}

/*
-----------------------------------------------------------------------------

 FUNCTION:     CanCreateVideoPort32

 DESCRIPTION:  This function is required

-----------------------------------------------------------------------------
*/

DWORD __stdcall
vpCanCreateVideoPort32 (LPDDHAL_CANCREATEVPORTDATA lpInput)
{

  /*
   * If the lpInput->lpDDVideoPortDesc can not be supported, set
   * lpInput->ddRVal to the correct error code and return
   * DDHAL_DRIVER_HANDLED
   */


	RECT rVidIn;
	DDVIDEOPORTDESC vpDesc;

    DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);

#ifdef FXTRACE
    Msg(ppdev, DEBUG_VPEENTRY, "CanCreate VideoPort32" );

	//Dump_VIDOPORTDESC( DEBUG_DDGORY, pccvpd->lpDDVideoPortDesc );
#endif

	vpDesc = *lpInput->lpDDVideoPortDesc;

	rVidIn.top    = rVidIn.left = 0;
	rVidIn.right  = vpDesc.dwFieldWidth;
	rVidIn.bottom = vpDesc.dwFieldHeight;

	if ( (rVidIn.right > SST2_MAX_VID_IN_X) ||
         (rVidIn.bottom > SST2_MAX_VID_IN_Y) ||	//Video-in size g.t. max allowed
		 (vpDesc.dwVideoPortID != SST2_PORT_ID) ||
		 (vpDesc.dwFieldWidth > SST2_MAX_VID_IN_X) ||
		 (vpDesc.dwFieldHeight > SST2_MAX_VID_IN_Y) ||
		 (vpDesc.VideoPortType.dwPortWidth !=  8) ||
		 ( !IsEqualIID( &(vpDesc.VideoPortType.guidTypeID),
            &(sConnect[0].guidTypeID)) ))
	{
		lpInput->ddRVal = DDERR_UNSUPPORTED;
	}	
	else
	{
		lpInput->ddRVal = DD_OK; 
	}


  return DDHAL_DRIVER_HANDLED;
}


/*
-----------------------------------------------------------------------------

 FUNCTION:     CreateVideoPort32

 DESCRIPTION:  This function is optional
-----------------------------------------------------------------------------
*/

DWORD __stdcall
vpCreateVideoPort32 (LPDDHAL_CREATEVPORTDATA lpInput)
{

 DWORD dwFifoCntl;
   DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
        Msg(ppdev, DEBUG_VPEENTRY, "Create VideoPort32" );

  	//Dump_VIDOPORTDESC( DEBUG_DDGORY, pcvpd->lpDDVideoPortDesc );
#endif
	
  _DD(vPortSurfOffset) = 0;
  _DD(dwVPEFlags) = 0;

  _DD(vpeSurf) = 0;
  _DD(lpFilterMem0) = 0;
  _DD(lpFilterMem1) = 0;
  _DD(dwVideoWidth) = 0;
  _DD(dwVideoHeight) = 0;

  SETDW( ghwVD->vdVipConfig, GET( ghwVD->vdVipConfig) | SST_VI_VIP_ENABLE);
/*
  //init vertical filter, which is set in overlay
  SETDW( ghwVD->vdViCfg0,0);
  SETDW( ghwVD->vdViCfg3,0);
  SETDW( ghwVD->vdViCfg5,0);
  dwFifoCntl =  GET( ghwVD->vdVipVideoFifoCntl) &
    ~( SST_VI_VV_VIDEO_FIFO_ENABLE | SST_VI_VV_ANCILLARY_FIFO_ENABLE);
  SETDW( ghwVD->vdVipVideoFifoCntl,  dwFifoCntl | SST_VI_VV_MASTER_ENABLE);
  //so that status is alive
  SETDW( ghwVD->vdViStatus, SST_VI_UPDATE);
*/
  SETDW( ghwVD->vdViCfg0,GET(ghwVD->vdViCfg0) & ~(SST_VI_EN |
        SST_VI_VV_VIDEO_ENABLE | SST_VI_VV_ANCILLARY_ENABLE));
  SETDW( ghwVD->vdViCfg3,0);
  SETDW( ghwVD->vdViCfg5,0);
  dwFifoCntl =  GET( ghwVD->vdVipVideoFifoCntl) &
    ~( SST_VI_VV_VIDEO_FIFO_ENABLE | SST_VI_VV_ANCILLARY_FIFO_ENABLE);
  SETDW( ghwVD->vdVipVideoFifoCntl,  dwFifoCntl | SST_VI_VV_MASTER_ENABLE);
  //so that status is alive
  SETDW( ghwVD->vdViStatus, GET(ghwVD->vdViStatus) | SST_VI_UPDATE);

  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}

/*
-----------------------------------------------------------------------------
 FlipVideoPort32

    This routine updates the start address of the capture buffer to
    the new buffer address.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpFlipVideoPort32 (LPDDHAL_FLIPVPORTDATA lpInput)
{

   DDHAL_GETVPORTFLIPSTATUSDATA sStatusInput;
   DWORD dwViStatus;
   DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);

#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "Flip VideoPort32" );

#endif
   sStatusInput.lpDD = lpInput->lpDD;
    
   // make sure that we have completed older flips
   vpGetVideoPortFlipStatus32( &sStatusInput);
    
   if (sStatusInput.ddRVal == DD_OK)
   {
        DWORD dwOffset; // Offset in the frame buffer to the capture surface 


        // Determine the new offset in the frame buffer to program and
        // reprogram the vport capture start address.
        
        dwOffset = GET_HW_ADDR(lpInput->lpSurfTarg) +
                    _DD(vPortSurfOffset);

        SETDW( ghwVD->vdViFrameBase0, dwOffset);

        dwViStatus = GET( ghwVD->vdViStatus) | SST_VI_BASE_UPDATE;
        SETDW( ghwVD->vdViStatus, dwViStatus);

        _DD(dwVPEFlags) |= VPE_FLP;
        lpInput->ddRVal = DD_OK;
    }
    else
        lpInput->ddRVal = DDERR_WASSTILLDRAWING;


  return DDHAL_DRIVER_HANDLED;

}


/*
-----------------------------------------------------------------------------
 SyncSurfaceData32

    This routine...

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpSyncSurfaceData32 (LPDDHAL_SYNCSURFACEDATA lpInput)
{
  
  FXSURFACEDATA * pSurfaceData;
  DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "SyncSurfaceData32" );
#endif

  pSurfaceData = GET_SURF_DATA(lpInput->lpDDSurface);

  if( pSurfaceData == 0)
  {

      lpInput->ddRVal = DDERR_INVALIDPARAMS;
      return DDHAL_DRIVER_HANDLED;

  }
  

  lpInput->dwSize = sizeof( DDHAL_SYNCSURFACEDATA);


  lpInput->dwSurfaceOffset = GET_HW_ADDR(lpInput->lpDDSurface);
  lpInput->lPitch = pSurfaceData->dwStride;
  lpInput->dwOverlayOffset = 0;
  lpInput->dwDriverReserved1 = _DD(dwOVLFlags);

  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

}

/*
-----------------------------------------------------------------------------
 SyncVideoPortData32

    This routine...

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpSyncVideoPortData32 (LPDDHAL_SYNCVIDEOPORTDATA lpInput)
{

  DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "SyncSurfaceData32" );
#endif
  lpInput->dwSize = sizeof( DDHAL_SYNCVIDEOPORTDATA);
  lpInput->dwOriginOffset = _DD(vPortSurfOffset);

  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

}

/*
-----------------------------------------------------------------------------
 GetVideoPortBandwidth32

    This routine will determine a base set of minimum zoom factors
    necessary to support three different capture scenarios: normal,
    with color key enabled, and with y-interpolation enabled.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortBandwidth32
    (LPDDHAL_GETVPORTBANDWIDTHDATA lpInput)
{
    BW_CONFIG bw_Config;
    DWORD lZoom;
    DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Bandwidth32" );
#endif

    lpInput->lpBandwidth->dwCaps = DDVPBCAPS_DESTINATION;

    if( lpInput->dwFlags & DDVPB_TYPE )
    {
    	lpInput->ddRVal = DD_OK;
    	return DDHAL_DRIVER_HANDLED;
    }

    if( !( lpInput->dwFlags & DDVPB_VIDEOPORT ) )
    {
    	lpInput->ddRVal = DDERR_INVALIDPARAMS;
    	return DDHAL_DRIVER_HANDLED;
    }

    bw_Config = _DD( sBWConfig);

    if(!(bw_Config.sDesktop.dwByteCount))
    {
    	bw_Config.sDesktop.dwByteCount = 2;
    	bw_Config.sDesktop.dwScaleX = 0x10000;
    	bw_Config.sDesktop.dwVCLK = 157000;
    }


    GetMVCLK(&bw_Config);
    if(!(bw_Config.sOverlay.dwFlags & OVL_ON))
    {
    	bw_Config.sOverlay.dwFlags = OVL_ON;
    	bw_Config.sOverlay.dwByteCount = 2;

    }

	bw_Config.sVideoPort.dwTransferRate =
        lpInput->lpVideoPort->ddvpDesc.dwMaxPixelsPerSecond;
	bw_Config.sVideoPort.dwFlags = VPE_ON;
	bw_Config.sVideoPort.dwScale =
        (lpInput->lpVideoPort->ddvpDesc.dwFieldWidth << 12) /
                    lpInput->dwWidth;


   // Check bandwidth to find the minimum zoom factor

   lpInput->lpBandwidth->dwOverlay = (DWORD) -1;
   lpInput->lpBandwidth->dwYInterpolate = (DWORD) -1;
   lpInput->lpBandwidth->dwColorkey = (DWORD)-1;
   lpInput->lpBandwidth->dwYInterpAndColorkey =(DWORD) -1;

   lZoom = 200;     //start with less than 1/4 scale

   do
   {
        bw_Config.sOverlay.dwScaleX = (1000 << 16)/ lZoom;
        bw_Config.sOverlay.dwScaleY = (1000 << 16)/ lZoom;
        bw_Config.sOverlay.dwScaleX = (1000 << 16)/ lZoom;
        bw_Config.sOverlay.dwScaleY = (1000 << 16)/ lZoom;
//        if ( EnoughBandWidth( &bw_Config))  //try the real one
        {
Msg(ppdev, DEBUG_VPEENTRY, "Minimum zoom factor (normal): %d", lZoom);
            lpInput->lpBandwidth->dwOverlay = lZoom;
            lpInput->lpBandwidth->dwYInterpolate = lZoom;
            lpInput->lpBandwidth->dwColorkey = lZoom;
            lpInput->lpBandwidth->dwYInterpAndColorkey = lZoom;
            break;

        }
        lZoom += 100;
   } while (lZoom < 4000);


  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}



/*
-----------------------------------------------------------------------------
 GetVideoPortInputFormats32

    This routine returns the supported video format (DDPIXELFORMATs)
    that can be input into our video port.  This has nothing to do with
    what they can be converted to before being written into memory,
    however (please see GetVideoPortOutputFormats32 below).

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortInputFormats32
    (LPDDHAL_GETVPORTINPUTFORMATDATA lpInput)
{
#ifdef FXTRACE
    DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort InputFormats32" );
#endif

	if(lpInput->dwFlags & DDVPFORMAT_VBI )
    {
       lpInput->dwNumFormats = 1;
    }
    else
    {
      lpInput->dwNumFormats = 2;
    }


    if (lpInput->lpddpfFormat != NULL)
    {

        DDPIXELFORMAT pf[] =
        {
           {
           sizeof(DDPIXELFORMAT), DDPF_FOURCC, FOURCC_UYVY,
           16, (DWORD)-1, (DWORD)-1, (DWORD)-1
           },

           {
           sizeof(DDPIXELFORMAT), DDPF_FOURCC, FOURCC_YUY2,
           16, (DWORD)-1, (DWORD)-1, (DWORD)-1
           },

           {
           sizeof(DDPIXELFORMAT), DDPF_FOURCC, FOURCC_RAW8,
           8, (DWORD)-1, (DWORD)-1, (DWORD)-1
           }
        };

        //We must tell WebTV VBISURF that we can support RAW8
        // otherwise it will not work.
        if(lpInput->dwFlags & DDVPFORMAT_VBI )
        {
           memcpy (lpInput->lpddpfFormat, &pf[2], sizeof(DDPIXELFORMAT));
        }
        else
        {
           memcpy (lpInput->lpddpfFormat, &pf[0],  2 * sizeof(DDPIXELFORMAT));
        }
    }
	
    lpInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}



/*
-----------------------------------------------------------------------------
 GetVideoPortOutputFormats32

    This routine will return the "output" (into the frame buffer)
    video formats based on the specified input video format.  This is
    what determines the "color conversions" that can occur in our
    video port.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortOutputFormats32
    (LPDDHAL_GETVPORTOUTPUTFORMATDATA lpInput)
{

  DDPIXELFORMAT pf[] =
  {
           {
           sizeof(DDPIXELFORMAT), DDPF_FOURCC, FOURCC_UYVY,
           16, (DWORD)-1, (DWORD)-1, (DWORD)-1
           },

           {
            sizeof(DDPIXELFORMAT), DDPF_FOURCC, FOURCC_RAW8,
            8, (DWORD)-1, (DWORD)-1, (DWORD)-1
           }
   };
#ifdef FXTRACE
    DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort OutputFormats32" );
#endif

   lpInput->dwNumFormats = 0;

   if ((lpInput->lpddpfInputFormat->dwFlags & DDPF_FOURCC) &&
       ((lpInput->lpddpfInputFormat->dwFourCC == FOURCC_UYVY ) ||
	    (lpInput->lpddpfInputFormat->dwFourCC == FOURCC_YUY2 )))
    {
       lpInput->dwNumFormats = 1;

       if (lpInput->lpddpfOutputFormats != NULL)
       {
           memcpy(lpInput->lpddpfOutputFormats, &pf,
             sizeof(DDPIXELFORMAT));
 
       }
    }
    else if ((lpInput->lpddpfInputFormat->dwFlags & DDPF_FOURCC) &&
        (lpInput->lpddpfInputFormat->dwFourCC == FOURCC_RAW8 ) &&
        (lpInput->dwFlags  && DDVPFORMAT_VBI ) )
    {
       lpInput->dwNumFormats = 1;
       if (lpInput->lpddpfOutputFormats != NULL)
       {
         memcpy(lpInput->lpddpfOutputFormats, lpInput->lpddpfInputFormat,
             sizeof(DDPIXELFORMAT));
       }
    }

	lpInput->ddRVal = DD_OK; 
	return DDHAL_DRIVER_HANDLED;

}


/*
-----------------------------------------------------------------------------
 GetVideoPortField32

    Returns TRUE in lpInput->bField if current field is an even field of
    an interlaced video signal.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortField32
    (LPDDHAL_GETVPORTFIELDDATA lpInput)
{
   DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Field32" );
#endif

  if( GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_FIELD)
    lpInput->bField =  FALSE;
  else
    lpInput->bField = TRUE;  // true if even field which is field 1
    
  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}


/*
-----------------------------------------------------------------------------
 GetVideoPortLine32


-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortLine32 (LPDDHAL_GETVPORTLINEDATA lpInput)
{
   DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);

#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Line32" );
#endif

    lpInput->dwLine = SST_GET_FIELD(GET(ghwVD->vdViStatus), SST_VI_YLINE);
    if(lpInput->dwLine == 0 )
      lpInput->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
    else
      lpInput->ddRVal = DD_OK;

  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

}


/*
-----------------------------------------------------------------------------
 GetVideoPortConnectInfo32

    This routine will fill in the supported connection types for
    this device and return the total count of the connection types
    supported.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortConnectInfo32
    (LPDDHAL_GETVPORTCONNECTDATA lpInput)
{
#ifdef FXTRACE
    DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort ConnectInfo32" );
#endif

	if(lpInput->dwPortId != SST2_PORT_ID)
    {
        lpInput->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }
    if (lpInput->lpConnect != NULL)
    {
       memcpy (lpInput->lpConnect, sConnect, sizeof(sConnect));
    }

    lpInput->dwNumEntries = sizeof( sConnect ) / sizeof(DDVIDEOPORTCONNECT);

    lpInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}

/*
-----------------------------------------------------------------------------
 DestroyVideoPort32

    NOTE: We need to make sure that the video is stopped

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpDestroyVideoPort32 (LPDDHAL_DESTROYVPORTDATA lpInput)
{
  LPDDRAWI_DIRECTDRAW_GBL lpDD = lpInput->lpDD->lpGbl; // Surf alloc/free needs this
  DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "Destroy VideoPort32" );
#endif

  if(_DD(dwVPEFlags) && VPE_ON)
  {
     //disable Video Port
      _DD(dwVPEFlags) = 0;	
 }
  ResetVIP(ppdev);	

  if(!(_DD(dwOVLFlags) & OVL_ON) &&  _DD(lpFilterMem0))
  {
    //Free this memory
    surfMgr_freeSurface( lpDD, 0, _DD(lpFilterMem0), 0);
  }

  _DD(vpeSurf) = 0;
  _DD(dwVPEFlags) = 0;
  lpInput->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
}


/*
-----------------------------------------------------------------------------
 GetVideoPortFlipStatus32

    This routine will return whether the flip has occurred.
-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoPortFlipStatus32
    (LPDDHAL_GETVPORTFLIPSTATUSDATA lpInput)
{
  DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);
#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoPort Flipstatus32" );
#endif
  if(_DD(dwVPEFlags) & VPE_FLP )
  {
     //see the address has be updated

     if(GET( ghwVD->vdViStatus) & SST_VI_BASE_UPDATE)
        lpInput->ddRVal = DDERR_WASSTILLDRAWING;
     else
     {
        _DD(dwVPEFlags) &= ~VPE_FLP;
        lpInput->ddRVal = DD_OK;
     }

  }
  else
  {
    lpInput->ddRVal = DD_OK;
     _DD(dwVPEFlags) &= ~VPE_FLP;
  }

  return DDHAL_DRIVER_HANDLED;
}


/*
-----------------------------------------------------------------------------
 UpdateVideoPort32

     This routine will update the chip's video port controls based on
     the current state variables negotiated with the client application.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpUpdateVideoPort32 (LPDDHAL_UPDATEVPORTDATA lpInput)
{
  DWORD dwViStatus;
  LPDDRAWI_DIRECTDRAW_GBL lpDD = lpInput->lpDD->lpGbl; // Surf alloc/free needs this
  DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);

#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "Update VideoPort32" );
#endif

  if (lpInput->dwFlags & DDRAWI_VPORTSTOP)
  {
#ifdef FXTRACE
		Msg(ppdev, DEBUG_VPEENTRY, "Stop VideoPort" );
#endif
      _DD(dwVPEFlags) &= ~(VPE_ON |VPE_TWO_BUFFERS | VPE_THREE_BUFFERS
                            |VPE_VBI_ONLY);

      SETDW( ghwVD->vdVipConfig, GET(ghwVD->vdVipConfig)&~SST_VI_VIP_ENABLE);
      SETDW( ghwVD->vdViAncDataAddrCtl, 0);
      SETDW( ghwVD->vdVipVideoFifoCntl, GET( ghwVD->vdVipVideoFifoCntl) &
        ~(SST_VI_VV_VIDEO_FIFO_ENABLE | SST_VI_VV_ANCILLARY_FIFO_ENABLE));

      SETDW( ghwVD->vdViCfg0,GET(ghwVD->vdViCfg0) & ~(SST_VI_EN |
            SST_VI_VV_VIDEO_ENABLE | SST_VI_VV_ANCILLARY_ENABLE));

      dwViStatus = GET( ghwVD->vdViStatus) | SST_VI_UPDATE;
      SETDW( ghwVD->vdViStatus, dwViStatus);

      lpInput->ddRVal = DD_OK;
  }
  else  if (lpInput->dwFlags & (DDRAWI_VPORTSTART | DDRAWI_VPORTUPDATE))
  {

     LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;
     LPDDPIXELFORMAT lpOutputFormat;
     LPDDPIXELFORMAT lpInputFormat;
     FXSURFACEDATA * pSurfaceData;
     DWORD dwVBIWidth, dwVBIHeight;
     BOOL fVBIOnly = FALSE;
     VIP_REG vipReg;
     DWORD dwStride,dwTmpStride;
     DWORD dwVIFlags;
     __int64 liEnd, liTemp,lFr;


#ifdef FXTRACE
     Msg(ppdev, DEBUG_VPEENTRY, "Start VideoPort" );
#endif



     if((lpInput->lplpDDSurface == NULL ) ||( *lpInput->lplpDDSurface == NULL ))
     {
        if((lpInput->lplpDDVBISurface == NULL) || ( *lpInput->lplpDDVBISurface == NULL))
        {

                DPF ("UpdateVideoPort32: no surface");
                lpInput->ddRVal = DDERR_OUTOFCAPS;
                return DDHAL_DRIVER_HANDLED;

        }  
           surf_lcl = (* (lpInput->lplpDDVBISurface))->lpLcl;
           fVBIOnly = TRUE;
     }
     else
     {
            surf_lcl = (* (lpInput->lplpDDSurface))->lpLcl;
     }

    memset(&vipReg, 0, sizeof(VIP_REG));
    
    vipReg.dwVipVideoFifoCntl= GET( ghwVD->vdVipVideoFifoCntl) &
        ~(SST_VI_VV_YC_ORDER |SST_VI_VV_IP_MUX_SEL |
          SST_VI_HSRC_WIDTH | SST_VI_VV_ANC_MODE);

    vipReg.dwVipVideoFifoCntl |=
                           SST_VI_VV_V_ANCILLARY_DISABLE
                          |SST_VI_VV_H_ANCILLARY_DISABLE
                          |SST_VI_VV_MASTER_ENABLE  // make sure it is set
                          |(SST_VD_EXTERNAL_INPUT << SST_VI_VV_IP_MUX_SEL)
                           ;

    lpInputFormat = lpInput->lpVideoInfo->lpddpfInputFormat;

    // We need to get the output format from the attached (target)
    // surface.

   	if( surf_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
   	{
          lpOutputFormat = &(surf_lcl->lpGbl->ddpfSurface);
    }
    else
    {
          lpOutputFormat = &(surf_lcl->lpSurfMore->lpDD_lcl->lpGbl->vmiData.ddpfDisplay);
    }
     
     // Check color format

    if((lpInputFormat == NULL) || ( lpOutputFormat == NULL))
    {

        DPF("No Input or output Format!");
        lpInput->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }

    if( fVBIOnly)
    {
        if (lpInputFormat->dwFlags & DDPF_FOURCC )
        {
           if( lpOutputFormat->dwFourCC != FOURCC_RAW8 )
           {
               DPF("Not correct VBI output Format!");
               lpInput->ddRVal = DDERR_INVALIDPARAMS;
               return DDHAL_DRIVER_HANDLED;
           }
        }
        else
        {

            DPF("Not correct VBI output Format!");
            lpInput->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }
    }
    else      //video format
    {
          if (lpOutputFormat->dwFourCC != FOURCC_UYVY )
          {

                 DPF("Not correct Video input Format!");
                 lpInput->ddRVal = DDERR_INVALIDPARAMS;
                 return DDHAL_DRIVER_HANDLED;
          }
          else if(lpInputFormat->dwFourCC == FOURCC_YUY2 )
          {
              vipReg.dwVipVideoFifoCntl |= SST_VI_VV_YC_ORDER;
          }
    }

      //Check video port connection
    if( !IsEqualIID( &(lpInput->lpVideoPort->ddvpDesc.VideoPortType.guidTypeID),
    	    &(sConnect[0].guidTypeID)) )
    {

         DPF("Not correct Port connection type!");
         lpInput->ddRVal = DDERR_INVALIDPARAMS;
         return DDHAL_DRIVER_HANDLED;

    }

    if (lpInput->lpVideoPort->ddvpDesc.VideoPortType.dwFlags &
            DDVPCONNECT_INVERTPOLARITY)
    {
          DPF(" Invert Polarity");
          vipReg.dwViCfg0 |= SST_VI_FID_INVERT;
    }


    if (lpInput->lpVideoPort->ddvpDesc.VideoPortType.dwFlags &
            DDVPCONNECT_INTERLACED)
    {

        _DD(dwVPEFlags) |= VPE_INTERLACED;
    
    }
    else
    {
        _DD(dwVPEFlags) &= ~VPE_INTERLACED;

    }


    if (lpInput->lpVideoInfo->dwVPFlags & DDVP_SKIPODDFIELDS )
    {
        if( vipReg.dwViCfg0 & SST_VI_FID_INVERT)
          vipReg.dwViCfg4 = SST_VI_V_FIELD_EN_1;
        else
          vipReg.dwViCfg4 = SST_VI_V_FIELD_EN_0;
    }
    else if (lpInput->lpVideoInfo->dwVPFlags & DDVP_SKIPEVENFIELDS )
    {

       if( vipReg.dwViCfg0 & SST_VI_FID_INVERT)
          vipReg.dwViCfg4 = SST_VI_V_FIELD_EN_0;
       else
          vipReg.dwViCfg4 = SST_VI_V_FIELD_EN_1;

    }
	else
	    vipReg.dwViCfg4 = SST_VI_V_FIELD_EN_0 | SST_VI_V_FIELD_EN_1;

    // Get VBI width and Height;
    dwVBIHeight = lpInput->lpVideoInfo->dwVBIHeight;
    dwVBIWidth  = lpInput->lpVideoPort->ddvpDesc.dwVBIWidth;

    _DD(dwVPEFlags) &= ~(VPE_TWO_BUFFERS | VPE_THREE_BUFFERS |
                          VPE_FOUR_BUFFERS | VPE_VBI_ONLY);
    _DD(vpeSurf) = 0;

    if( !fVBIOnly )
    {
        DWORD dwVideoWidth, dwVideoHeight;
        DWORD dwPScaleWidth, dwPScaleHeight;
        DWORD dwCropLeft, dwCropTop;
        DWORD dwXScale,dwYScale;
        // find video width, height 

        vipReg.dwViCfg0 |= SST_VI_EN | SST_VI_VV_VIDEO_ENABLE;

        vipReg.dwVipVideoFifoCntl |= SST_VI_VV_VIDEO_FIFO_ENABLE;

        if (lpInput->lpVideoInfo->dwVPFlags & DDVP_CROP)
        {
            dwCropLeft = (DWORD)(lpInput->lpVideoInfo->rCrop.left);
            dwVideoWidth = (DWORD)(lpInput->lpVideoInfo->rCrop.right -
                                    lpInput->lpVideoInfo->rCrop.left);

           dwCropTop = (DWORD)(lpInput->lpVideoInfo->rCrop.top);
           dwVideoHeight = (DWORD)(lpInput->lpVideoInfo->rCrop.bottom -
                                    lpInput->lpVideoInfo->rCrop.top);
        }
        else
        {
           dwCropLeft = dwCropTop = 0;
           dwVideoWidth = lpInput->lpVideoPort->ddvpDesc.dwFieldWidth;
           dwVideoHeight = lpInput->lpVideoPort->ddvpDesc.dwFieldHeight;
                           //-dwVBIHeight;  check this later
        }
        
        if (lpInput->lpVideoInfo->dwVPFlags & DDVP_PRESCALE)
        {
             dwPScaleWidth  = lpInput->lpVideoInfo->dwPrescaleWidth;
             dwPScaleHeight =  lpInput->lpVideoInfo->dwPrescaleHeight;
        }
        else
        {
            dwPScaleWidth = dwVideoWidth;
            dwPScaleHeight = dwVideoHeight;
        }


        //find scale factor
        dwXScale = ((dwVideoWidth << 12)
                           + dwPScaleWidth -1l ) / dwPScaleWidth;
 
        dwYScale = ((dwVideoHeight << 12)
                           + dwPScaleHeight -1l ) / dwPScaleHeight;

        //check bandwith
        FillDesktopInfo(ppdev, &(_DD(sBWConfig)) );
        _DD(sBWConfig).sVideoPort.dwFlags |= VPE_ON;
        _DD(sBWConfig).sVideoPort.dwScale = dwXScale;
        _DD(sBWConfig).sVideoPort.dwTransferRate =
            lpInput->lpVideoPort->ddvpDesc.dwMaxPixelsPerSecond;

/*        if(!EnoughBandWidth( &(_DD(sBWConfig))))
        {

            _DD(sBWConfig).sVideoPort.dwFlags &= ~VPE_ON;
            DPF ("UpdateVideoPort32: B/W Failed!");
            lpInput->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;

        }
*/
        _DD(dwViYScale) = dwYScale;

        pSurfaceData = GET_SURF_DATA((*(lpInput->lplpDDSurface))->lpLcl);
		//dwStride = (*(lpInput->lplpDDSurface))->lpLcl->lpGbl->lPitch;
		dwStride = pSurfaceData->dwStride;
        //find surface offset
        _DD(vPortSurfOffset) =
            lpInput->lpVideoInfo->dwOriginY * dwStride +
            lpInput->lpVideoInfo->dwOriginY * 2;

        vipReg.dwFrameBase0 = _DD(vPortSurfOffset) +
                GET_HW_ADDR((*(lpInput->lplpDDSurface))->lpLcl);

        if (lpInput->lpVideoPort->ddvpInfo.dwVPFlags & DDVP_INTERLEAVE)
        {

           //interleave storage
		   _DD(dwVPEFlags) |= VPE_WEAVE;
        }
        else
        {
           //Non-interleave mode
           _DD(dwVPEFlags) &= ~VPE_WEAVE;
        }

        if((dwVideoWidth != _DD(dwVideoWidth)) ||
           (dwVideoHeight != _DD(dwVideoHeight)) )
        {
           DWORD dwTmpAddr, dwLinearAddr, dwEndLinearAddr, tileFlag;
           _DD(dwVideoWidth) = dwVideoWidth;
           _DD(dwVideoHeight) = dwVideoHeight;
           //Free temporay memory
           if( _DD(lpFilterMem0) != 0)
           {
               //Free this memory
               surfMgr_freeSurface( lpDD, 0, _DD(lpFilterMem0), 0 );
           }

           //Allocate temporay memory with the new size
           tileFlag = MEM_IN_LINEAR;
           
           if ( surfMgr_allocSurface(
                    lpDD,                 // Ddraw VidMemAlloc may need this
                    0,                    // User's type
                    0,                    // unused
                    dwPScaleWidth * 2,        // width of surface in byte
                    (dwPScaleHeight +4)* 2 + 6,   // height of surface
                    0,                    // linear address mask
                    &dwLinearAddr,        // lfb start address of allocation
                    &dwEndLinearAddr,     // lfb start address of allocation
                    &dwTmpAddr,           // hw vidmem address
                    &dwTmpStride,         // pitch in bytes
                    &tileFlag)            // Memory type, linear
                != DD_OK )
           {
                _DD(lpFilterMem0) = 0;
                lpInput->ddRVal = DDERR_INVALIDPARAMS;
                return DDHAL_DRIVER_HANDLED;

           }

           _DD(lpFilterMem0) = dwTmpAddr;
           _DD(lpFilterMem1) = dwTmpAddr + (dwPScaleHeight + 4) * dwTmpStride;
           _DD(filterMemStride) = dwTmpStride;
        }
        else
          dwTmpStride = _DD(filterMemStride);

        //First 3 DOUBLE lines for VI delay
        vipReg.dwViVIntDlyBase0 = _DD(lpFilterMem1) +
                (dwPScaleHeight +4)* dwTmpStride;
        //Last one field + 1 line for LPF delay
        vipReg.dwViLpfDlyBase0 = _DD(lpFilterMem0);
        vipReg.dwViLpfDlyBase1 = _DD(lpFilterMem1);
                
        vipReg.dwVIntDlyAddrCtl |= dwTmpStride * 2;  //double line
        vipReg.dwLpfDlyAddrCtl  |= dwTmpStride;
        
        // see auto flip
        if (lpInput->lpVideoInfo->dwVPFlags & DDVP_AUTOFLIP)
        {
            switch(lpInput->dwNumAutoflip)
            {

             case 2:
             {
                 vipReg.dwFrameBase1 = _DD(vPortSurfOffset) +
                     GET_HW_ADDR((*(lpInput->lplpDDSurface + 1))->lpLcl);

                 _DD(dwVPEFlags) |= VPE_TWO_BUFFERS;
                 VD_SET_FIELD( vipReg.dwViFrameAddrCtl, SST_VI_FRAME_CTL,1);

                 break;
             }
             case 3:
             {
                 vipReg.dwFrameBase1 = _DD(vPortSurfOffset) +
                    GET_HW_ADDR((*(lpInput->lplpDDSurface + 1))->lpLcl);

                 vipReg.dwFrameBase2 = _DD(vPortSurfOffset) +
                    GET_HW_ADDR((*(lpInput->lplpDDSurface + 2))->lpLcl);

                _DD(dwVPEFlags) |= VPE_THREE_BUFFERS;
                VD_SET_FIELD( vipReg.dwViFrameAddrCtl, SST_VI_FRAME_CTL,2);
                  break;
             }
            case 4:
            {
                 vipReg.dwFrameBase1 = _DD(vPortSurfOffset) +
                    GET_HW_ADDR((*(lpInput->lplpDDSurface + 1))->lpLcl);

                 vipReg.dwFrameBase2 = _DD(vPortSurfOffset) +
                    GET_HW_ADDR((*(lpInput->lplpDDSurface + 2))->lpLcl);

                 vipReg.dwFrameBase3 = _DD(vPortSurfOffset) +
                    GET_HW_ADDR((*(lpInput->lplpDDSurface + 3))->lpLcl);

                 _DD(dwVPEFlags) |= VPE_FOUR_BUFFERS;
                 VD_SET_FIELD( vipReg.dwViFrameAddrCtl, SST_VI_FRAME_CTL,3);
                 break;

            }
            default:
  
               DPF ("UpdateVideoPort32: Video can only autoflip two or three surfaces");
               lpInput->ddRVal = DDERR_OUTOFCAPS;
               return DDHAL_DRIVER_HANDLED;
          }
        }

        _DD(vpeSurf) = surf_lcl;
        //check VBI data
        if(dwVBIHeight)
        {
           fVBIOnly = TRUE;     //set it so that ancillary data
                                //register can be set
        }

		dwVIFlags = (_DD(dwVPEFlags) & (VPE_INTERLACED | VPE_WEAVE));
		if(dwVIFlags & VPE_WEAVE)
			dwVIFlags |= VPE_ROF;  //always set a mode for overlay
        else if( dwVIFlags & VPE_INTERLACED)
        {
           //for interlaced video, HW always store it as a frame
           // instead of field. For field storage, shrink the video
            dwYScale <<=1;
        }
        //Set up video registers
		SetVIRegs( dwPScaleWidth, dwPScaleHeight * 2, dwXScale, dwYScale,
			dwCropLeft & ~1, dwCropTop, dwCropTop, dwVIFlags, &vipReg);
       
        VD_SET_FIELD( vipReg.dwViFrameAddrCtl, SST_VI_FRAME_STRIDE, dwStride);

        SETDW( ghwVD->vdViCfg1, vipReg.dwViCfg1);
        SETDW( ghwVD->vdViCfg2, vipReg.dwViCfg2);
        SETDW( ghwVD->vdViCfg3, vipReg.dwViCfg3);
        SETDW( ghwVD->vdViCfg4, vipReg.dwViCfg4);
        SETDW( ghwVD->vdViCfg5, vipReg.dwViCfg5);
                                           
        SETDW( ghwVD->vdViFrameBase0, vipReg.dwFrameBase0);
        SETDW( ghwVD->vdViFrameBase1, vipReg.dwFrameBase1);
        SETDW( ghwVD->vdViFrameBase2, vipReg.dwFrameBase2);
        SETDW( ghwVD->vdViFrameBase3, vipReg.dwFrameBase3);
        SETDW( ghwVD->vdViFrameAddrCtl, vipReg.dwViFrameAddrCtl);
        SETDW( ghwVD->vdViLpfDlyAddrCtl, vipReg.dwLpfDlyAddrCtl);
        SETDW( ghwVD->vdViVIntDlyAddrCtl, vipReg.dwVIntDlyAddrCtl);
        if(!(_DD(dwVPEFlags) & VPE_ON))
        {
         //for the first time starting, don't enable  vi first
          SETDW( ghwVD->vdViCfg0, vipReg.dwViCfg0 & ~(SST_VI_VV_VIDEO_ENABLE
                |SST_VI_VV_ANCILLARY_ENABLE));
        }
		SETDW( ghwVD->vdVipVideoFifoCntl, vipReg.dwVipVideoFifoCntl);
       
        SETDW( ghwVD->vdViStatus, SST_VI_BASE_UPDATE | SST_VI_UPDATE);
        _DD(dwVPEFlags) |= VPE_ON;
    }

    if(fVBIOnly)
    {
        vipReg.dwVipVideoFifoCntl &= ~(SST_VI_VV_V_ANCILLARY_DISABLE|
                                SST_VI_VV_H_ANCILLARY_DISABLE);

        vipReg.dwVipVideoFifoCntl |=
                          SST_VD_ANC_WINDOW << SST_VI_VV_ANC_MODE_SHIFT;

        if((lpInput->lplpDDVBISurface != NULL) &&
            ( *lpInput->lplpDDVBISurface != NULL))
        {
#ifdef AGP_EXECUTE
            if((*(lpInput->lplpDDVBISurface))->lpLcl->ddsCaps.dwCaps &
                    DDSCAPS_NONLOCALVIDMEM)
                vipReg.dwAncBase0 = SST_VI_ANC_AGP_0|
                    GET_AGP_ADDR((* (lpInput->lplpDDVBISurface))->lpLcl);
            else
#endif
                vipReg.dwAncBase0 = 
                    GET_HW_ADDR((* (lpInput->lplpDDVBISurface))->lpLcl);
            // see auto flip
            if (lpInput->lpVideoInfo->dwVPFlags & DDVP_AUTOFLIP)
            {
                if(lpInput->dwNumVBIAutoflip != 2)
                {
                DPF ("UpdateVideoPort32: VBI can only autoflip two surfaces");
                lpInput->ddRVal = DDERR_OUTOFCAPS;
                return DDHAL_DRIVER_HANDLED;
                }
                else
                {
                //Set autoflip
#ifdef AGP_EXECUTE
                if((*(lpInput->lplpDDVBISurface + 1))->lpLcl->ddsCaps.dwCaps &
                    DDSCAPS_NONLOCALVIDMEM)
                    vipReg.dwAncBase1=SST_VI_ANC_AGP_1 | 
                        GET_AGP_ADDR((*(lpInput->lplpDDVBISurface + 1))->lpLcl);
                else
#endif
                    vipReg.dwAncBase1= 
                        GET_HW_ADDR((*(lpInput->lplpDDVBISurface + 1))->lpLcl);


               }
            }
            else
                vipReg.dwAncBase1 = vipReg.dwAncBase0;
         }
         else
         {
           //VBI and video are in the same surface

            vipReg.dwAncBase0 =  GET_HW_ADDR((*(lpInput->lplpDDSurface))->lpLcl);
            if ((lpInput->lpVideoInfo->dwVPFlags & DDVP_AUTOFLIP) &&
                lpInput->dwNumVBIAutoflip)
            {
                if((lpInput->dwNumVBIAutoflip != 2) ||
                    (lpInput->dwNumAutoflip != 2))
                {

                    DPF ("UpdateVideoPort32: VBI can only autoflip two surfaces");
                    lpInput->ddRVal = DDERR_OUTOFCAPS;
                    return DDHAL_DRIVER_HANDLED;
                }


                vipReg.dwAncBase1= 
                        GET_HW_ADDR((*(lpInput->lplpDDSurface + 1))->lpLcl);
            }else
                vipReg.dwAncBase1= vipReg.dwAncBase1;

         }

         //Set up VBI registers
         vipReg.dwVipVideoFifoCntl |= SST_VI_VV_ANCILLARY_FIFO_ENABLE;
         vipReg.dwViCfg0 |= SST_VI_VV_ANCILLARY_ENABLE;
         vipReg.dwAncDataAddrCtl = dwVBIWidth | SST_VI_ANC_EN;
         VD_SET_FIELD( vipReg.dwAncDataWindow, SST_VI_ANC_STOP_0, dwVBIHeight);
         VD_SET_FIELD( vipReg.dwAncDataWindow, SST_VI_ANC_STOP_1, dwVBIHeight);

		 SETDW( ghwVD->vdViAncDataAddrCtl, vipReg.dwAncDataAddrCtl);
		 SETDW( ghwVD->vdViAncDataWindow,  vipReg.dwAncDataWindow);
         SETDW(ghwVD->vdViAncDataBase0,vipReg.dwAncBase0);
         SETDW(ghwVD->vdViAncDataBase1,vipReg.dwAncBase1);
		 if(!(_DD(dwVPEFlags) & VPE_VBI_ONLY))
         {
          //for the first time starting, enable  vi first
           SETDW( ghwVD->vdViCfg0, vipReg.dwViCfg0 & ~(SST_VI_VV_VIDEO_ENABLE
                |SST_VI_VV_ANCILLARY_ENABLE));
         
         }
         SETDW( ghwVD->vdVipVideoFifoCntl,vipReg.dwVipVideoFifoCntl);

         SETDW( ghwVD->vdViStatus, SST_VI_BASE_UPDATE | SST_VI_UPDATE);
        _DD(dwVPEFlags) |= VPE_VBI_ONLY;
    }
         
    
    //before set register set FifoCntl first
    //wait for a VSYNC
    QueryPerformanceFrequency( (LARGE_INTEGER *) & lFr);
    QueryPerformanceCounter ((LARGE_INTEGER *) &liEnd);
   //lFr >> 5 is 32ms, which is worst case for time out    

//    liEnd += lFr >> 5;
    //for QT test
    liEnd += lFr >> 10;

    QueryPerformanceCounter ((LARGE_INTEGER *) &liTemp);

//    WaitForVSYNC( ppdev, &liEnd,&liTemp);
    Msg(ppdev, DEBUG_VPEENTRY, "Update VideoPort waiting for V_BLANK" );
    while( !(GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_V_BLANK))
        ;

    SETDW( ghwVD->vdViCfg0, vipReg.dwViCfg0);
    SETDW( ghwVD->vdViStatus, SST_VI_UPDATE);

    Msg(ppdev, DEBUG_VPEENTRY, "Update VideoPort Got V_BLANK" );

    lpInput->ddRVal = DD_OK;
  }
  else
    lpInput->ddRVal = DDERR_INVALIDPARAMS;

  return DDHAL_DRIVER_HANDLED;

}


/*
-----------------------------------------------------------------------------
 WaitForVideoPortSync32

    This routine will poll for the beginning or the end of the
    VREF period of the incoming video stream.  In case of no signal,
    we also need to provide a timeout mechanism so that we do not
    lock up anyone.

-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpWaitForVideoPortSync32
    (LPDDHAL_WAITFORVPORTSYNCDATA lpInput)
{
    __int64 liEnd, liTemp;
   __int64 lFr,liTimeOut;

   DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);

#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "WaitFor VideoPortSync32" );
#endif

    QueryPerformanceFrequency( (LARGE_INTEGER *) & lFr);

    liTimeOut =  (_int64) lpInput->dwTimeOut * lFr / 1000; //as ticker
    if( liTimeOut > (lFr >> 5) )     //lFr >> 5 is 33ms, which is worst case
                                    //for time out    
        liTimeOut = lFr >> 5;

    switch (lpInput->dwFlags)
    {
        case DDVPWAIT_BEGIN:
            
             // We're going to use a timer as the backup timeout mechanism
             // so that if the VREF is not pulsing that we do not lockup the
             // task/thread or worse, the machine.

            QueryPerformanceCounter ((LARGE_INTEGER *) &liEnd);
            liTemp = liEnd;
            liEnd += liTimeOut;


            // The VREF signal is active low (pulsing high), which means
            // if DDVPWAIT_BEGIN is set, we only need to look for the low
            // to high transition
            
            //First Wait for  low
            while(liTemp < liEnd)
            {
                QueryPerformanceCounter ((LARGE_INTEGER *) &liTemp);

                if( !(GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_V_BLANK))
                {
                    break;
                }
            }

            if(liTemp >= liEnd)
            {
                lpInput->ddRVal = DDERR_VIDEONOTACTIVE;
                return DDHAL_DRIVER_HANDLED;
            }
            //Then wait for high
            WaitForVSYNC( ppdev, &liEnd,&liTemp);

            if(liTemp >= liEnd)
            {
                lpInput->ddRVal = DDERR_VIDEONOTACTIVE;
                return DDHAL_DRIVER_HANDLED;
            }

            break;

        case DDVPWAIT_END:

             // We're going to use a timer as the backup timeout mechanism
             //so that if the VREF is not pulsing that we do not lockup the
             //task/thread or worse, the machine.

            QueryPerformanceCounter ((LARGE_INTEGER *) &liEnd);
            liTemp = liEnd;
            liEnd +=  liTimeOut ;   // PAL (20ms) might be worst case, but...

             // The VREF signal is active low (pulsing high), which means
             // if DDVPWAIT_BEGIN is set, we only need to look for the high
             // to low transition

            WaitForVSYNC( ppdev, &liEnd,&liTemp);
            
            // Let's wait for for the signal to pulse low again.
            
            while(liTemp < liEnd)
            {
                QueryPerformanceCounter ((LARGE_INTEGER *) &liTemp);

                if( !(GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_V_BLANK))
                {
                    break;
                }
            }

            if(liTemp >= liEnd)
            {
                lpInput->ddRVal = DDERR_VIDEONOTACTIVE;
                return DDHAL_DRIVER_HANDLED;
            }
            break;

        case DDVPWAIT_LINE:
        {
            DWORD dwLineCount;
            QueryPerformanceCounter ((LARGE_INTEGER *) &liEnd);
            liTemp = liEnd;
            liEnd +=  liTimeOut ;   // PAL (20ms) might be worst case, but...

            dwLineCount =
                SST_GET_FIELD(GET(ghwVD->vdViStatus), SST_VI_YLINE);

            if( dwLineCount == lpInput->dwLine)
                break;

            else if( dwLineCount > lpInput->dwLine)
            {                              
               //  if the current line counter is high, wait it go to
               //  low first.
               
              while(liTemp < liEnd)
              {
                    QueryPerformanceCounter ((LARGE_INTEGER *) &liTemp);

                    if( SST_GET_FIELD(GET(ghwVD->vdViStatus), SST_VI_YLINE)
                         < lpInput->dwLine)
                    {
                        break;
                    }
              }

              if(liTemp >= liEnd)
              {
                    lpInput->ddRVal = DDERR_VIDEONOTACTIVE;
                    return DDHAL_DRIVER_HANDLED;
              }

            }
            
             // Right now the current line counter is low
             // so if it ever goes high then waiting is over.
             
            while(liTemp < liEnd)
            {
                QueryPerformanceCounter ((LARGE_INTEGER *) &liTemp);

                 if( SST_GET_FIELD(GET(ghwVD->vdViStatus), SST_VI_YLINE)
                        >= lpInput->dwLine)
                {
                    break;
                }
            }

            if(liTemp >= liEnd)
            {
                lpInput->ddRVal = DDERR_VIDEONOTACTIVE;
                return DDHAL_DRIVER_HANDLED;
            }
        }
        break;

        default:
            lpInput->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_HANDLED;
            break;
    }

    lpInput->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}

/*
-----------------------------------------------------------------------------
 GetVideoSignalStatus32

    This routine will return the status of whether a valid video signal
    is present on the video port currently.
-----------------------------------------------------------------------------
*/
static DWORD __stdcall vpGetVideoSignalStatus32 (LPDDHAL_GETVPORTSIGNALDATA lpInput)
{

    BOOL fVREF;          /* VREF signal status */
    __int64 liEnd, liTemp;  /* timeouts if no VREF present */
    __int64 lFr;
   DD_ENTRY_SETUP(lpInput->lpDD->lpGbl);

#ifdef FXTRACE
	Msg(ppdev, DEBUG_VPEENTRY, "GetVideoSignalStatus32" );
#endif
    /* We cannot relay on VSYNC to find out video signal, because when VSYNC
     * happens, the kernel mode driver could serve IRQ that was generated by
     * the VSYNC. When this IRQ routin returns, VSYNC pulse may be gone, so 
     * we cannot find any VSYNC pulse at this ring3 driver. Instead we check
     * on the field ID
     */
    fVREF = GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_FIELD;

    /*
     * We're going to use a timer as the backup timeout mechanism
     * so that if the VREF is not pulsing that we do not lockup the
     * task/thread or worse, the machine.
     */
    QueryPerformanceFrequency( (LARGE_INTEGER * )&lFr);
    QueryPerformanceCounter ((LARGE_INTEGER *) &liEnd);
    liTemp = liEnd;
    liEnd += lFr >> 5;   // PAL (20ms) might be worst case, but...

    /*
     * Let's wait for for the signal to transition (any kind of
     * transition will suffice for this test).
     */
    while(liTemp < liEnd)
    {
        QueryPerformanceCounter ((LARGE_INTEGER *) &liTemp);

       if( (BOOL) (GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_FIELD)
             != fVREF )
        {
            break;
        }
    }

    /*
     * Was it the signal or the timeout?
     */
    lpInput->ddRVal = DD_OK;
    if(liTemp >= liEnd)
    {
        DPF ("GetVideoSignalStatus32. No Signal");
        lpInput->dwStatus = DDVPSQ_NOSIGNAL;
    }
    else
        lpInput->dwStatus = DDVPSQ_SIGNALOK;

    return DDHAL_DRIVER_HANDLED;

}
/*
-----------------------------------------------------------------------------
   WaitForVSYNC
    wait for V_BLANK bit
-----------------------------------------------------------------------------
*/
void WaitForVSYNC( NT9XDEVICEDATA *ppdev,
    __int64 * liEnd,__int64 *liTemp)
{
      while(*liTemp < *liEnd)
      {
           QueryPerformanceCounter ((LARGE_INTEGER *) liTemp);
           if( GET( ghwVD->vdVipVideoFifoCntl) & SST_VI_VV_V_BLANK)
           {
               break;
           }
      }
}

/*
-----------------------------------------------------------------------------
   SetVIRegs
    Setup VI registers based on De-Interlace Modes
-----------------------------------------------------------------------------
*/

void SetVIRegs( DWORD dst_width, DWORD dst_height,DWORD h_inc, DWORD v_inc,
 DWORD h_start_offset, DWORD v_start_offset_0, DWORD v_start_offset_1,
 DWORD dwFlags, VIP_REG *regs)

{
	DWORD    offset_0, offset_1;
    DWORD    v_ph_offset_0, v_ph_offset_1;
	DWORD h_fil_factor, v_fil_factor;
	
	v_ph_offset_0 = v_ph_offset_1= 0;

   if( v_inc > 0x1000)
   {
      //y scale down case
      if(v_inc >= 0x3000)
           v_fil_factor = 0x8;
       else
       {
         //just use linear function
         v_fil_factor = (v_inc - 0x1000) >> 10;  //in a range 8-0
       }

    }
    else
    {
        v_fil_factor = 0;
    }
   //set horizontal filter
    h_fil_factor = 0;

    if( h_inc > 0x1000 )
    {
       //x scale down case
       if( h_inc>= 0x4000)
           h_fil_factor= 0x1F;
       else
       {
         //find 3log2 dwXScale

         while( (DWORD)(0x1000 << h_fil_factor) < h_inc)
            h_fil_factor ++;

        h_fil_factor <<= 3;
       }
    }

	if (dwFlags & VPE_INTERLACED)
		regs->dwViCfg0 |= SST_VI_INTERLACE;

	regs->dwViCfg0 |= v_fil_factor << SST_VI_VF_CTL_SHIFT;
	regs->dwViCfg1 |= h_start_offset << SST_VI_H_INC_START_SHIFT;
	regs->dwViCfg1 |= h_fil_factor << SST_VI_HFIL_CTL_SHIFT;
	regs->dwViCfg1 |= dst_width << SST_VI_HFIL_WIDTH_SHIFT;
	regs->dwViCfg2 |= h_inc << SST_VI_H_INC_SHIFT;
	regs->dwViCfg3 |= dst_height << SST_VI_VFIL_HEIGHT_SHIFT;

	if (!(dwFlags & VPE_WEAVE) || (v_inc >= 0x2000)) {

		// Stuff that's common to progressive and SF <= 0.5 ...

		regs->dwVIntDlyAddrCtl |= SST_VI_LIN_DLY_MODE;
		regs->dwLpfDlyAddrCtl |= SST_VI_FLD_DLY_MODE;
		regs->dwViCfg0 |= SST_VI_VF_DI_BYPASS;
	   //	regs->sst_vi_vf_mux1_ctrl = 0;
	   //	regs->sst_vi_vf_mux2_ctrl = 0;
	   //	regs->sst_vi_di_thrsh1    = 0; // Don't care
	   //	regs->sst_vi_di_thrsh2    = 0; // Don't care

		if (!(dwFlags & VPE_INTERLACED)) {
			
			//#### Progressive video ...

			v_inc    <<= 1;
            if(v_inc > 0xFFFF )
                v_inc= 0xFFFF;

			offset_0 = ((8192 * v_start_offset_0) + 20480 + (v_inc / 2) + v_ph_offset_0);
			offset_1 = ((8192 * v_start_offset_1) + 20480 + (v_inc / 2) + v_ph_offset_1);
		}
		else {

			//#### Interlaced video with Vertical Scale Factor <= 0.5 ...

			offset_0 = ((4096 * v_start_offset_0) + 20480 + (v_inc / 2) + v_ph_offset_0);
			offset_1 = ((4096 * v_start_offset_1) + 16384 + (v_inc / 2) + v_ph_offset_1);
		}
	}
	else { 
		
		//#### Interlaced video with scale factor > 0.5 ...

	   //	regs->sst_vi_lin_dly_mode = 0;
	   //	regs->sst_vi_fld_dly_mode = 0;
        dwFlags &=(VPE_BOB | VPE_WEAVE | VPE_BOB_PLUS | VPE_ROF);
		switch (dwFlags) {
			case VPE_BOB :
				
				v_inc    >>= 1;
				offset_0 = ((2048 * v_start_offset_0) + 3072 + (v_inc / 2) + v_ph_offset_0);
				offset_1 = ((2048 * v_start_offset_1) + 1024 + (v_inc / 2) + v_ph_offset_1);

				//regs->sst_vi_vf_di_bypass = 0;
    			regs->dwViCfg0 |= SST_VI_VF_MUX1_CTRL;
				regs->dwViCfg0 |= SST_VI_VF_MUX2_CTRL;
				//regs->sst_vi_di_thrsh1    = 0; // Don't care
				//regs->sst_vi_di_thrsh2    = 0; // Don't care
				break;

			case VPE_WEAVE :
				
				offset_0 = ((4096 * v_start_offset_0) + 10240 + (v_inc / 2) + v_ph_offset_0);
				offset_1 = ((4096 * v_start_offset_1) +  6144 + (v_inc / 2) + v_ph_offset_1);

				regs->dwViCfg0 |= SST_VI_VF_DI_BYPASS;
				//regs->sst_vi_vf_mux1_ctrl = 0;
				//regs->sst_vi_vf_mux2_ctrl = 0;
				//regs->sst_vi_di_thrsh1    = 0; // Don't care
				//regs->sst_vi_di_thrsh2    = 0; // Don't care
				break;

			case VPE_BOB_PLUS :
				
				v_inc    >>=1;
				offset_0 = ((2048 * v_start_offset_0) + 3072 + (v_inc / 2) + v_ph_offset_0 + 1024);
				offset_1 = ((2048 * v_start_offset_1) + 1024 + (v_inc / 2) + v_ph_offset_1 + 1024);

				//regs->sst_vi_vf_di_bypass = 0;
				regs->dwViCfg0 |= SST_VI_VF_MUX1_CTRL;
				regs->dwViCfg0 |= SST_VI_VF_MUX2_CTRL;
				//regs->sst_vi_di_thrsh1    = 0; // Don't care
			    //regs->sst_vi_di_thrsh2    = 0; // Don't care
				break;

			case VPE_ROF :
			default:

				offset_0 = ((4096 * v_start_offset_0) + 10240 + (v_inc / 2) + v_ph_offset_0);
				offset_1 = ((4096 * v_start_offset_1) +  6144 + (v_inc / 2) + v_ph_offset_1);

				//regs->sst_vi_vf_di_bypass = 0;
				//regs->sst_vi_vf_mux1_ctrl = 0;
				//regs->sst_vi_vf_mux2_ctrl = 0;
              //don't set any value until Wayne porvide more info.  
//				regs->dwViCfg3    |= 32 << SST_VI_DI_THRSH1_SHIFT; //as in DOC
//				regs->dwViCfg3    |= 48 << SST_VI_DI_THRSH2_SHIFT; //as in
				break;
		}
	}
	regs->dwViCfg4   |= (offset_0 / v_inc) << SST_VI_V_START_0_SHIFT; // Integer part
	regs->dwViCfg4   |= (offset_1 / v_inc) << SST_VI_V_START_1_SHIFT;
	regs->dwViCfg5   |= (offset_0 % v_inc) << SST_VI_V_START_PH_0_SHIFT; // Fractional part
	regs->dwViCfg5   |= (offset_1 % v_inc) << SST_VI_V_START_PH_1_SHIFT;
	regs->dwViCfg2   |= v_inc<< SST_VI_V_INC_SHIFT;
}

void ResetVIP( NT9XDEVICEDATA *ppdev)

{
	DWORD vipVideoFifoCntl;
	DWORD viCfg0;
	DWORD viStatus;

	// Turn off video and ancillary input data streams and Video Input block ......

	viCfg0 = GET (ghwVD->vdViCfg0);
	viCfg0 &= ~SST_VI_EN;
	viCfg0 &= ~SST_VI_VV_VIDEO_ENABLE;
	viCfg0 &= ~SST_VI_VV_ANCILLARY_ENABLE;
	SETDW (ghwVD->vdViCfg0, viCfg0);

	// Turn off the video and ancillary data input FIFO's. This will clear any overrun bits.....

	vipVideoFifoCntl = GET (ghwVD->vdVipVideoFifoCntl);
	vipVideoFifoCntl &= ~SST_VI_VV_VIDEO_FIFO_ENABLE;
	vipVideoFifoCntl &= ~SST_VI_VV_ANCILLARY_FIFO_ENABLE;
	SETDW(ghwVD->vdVipVideoFifoCntl, vipVideoFifoCntl);

	// Set the update bits so it takes effect .....
	
	viStatus = GET (ghwVD->vdViStatus);
	viStatus |= ((0x1 << SST_VI_UPDATE_SHIFT) & SST_VI_UPDATE);
	viStatus |= ((0x1 << SST_VI_BASE_UPDATE_SHIFT) & SST_VI_BASE_UPDATE);
	SETDW (ghwVD->vdViStatus, viStatus);

//	waitUpdateClear();
}


