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

#include "d3contxt.h"                   // context routines
#include "dxins.h"                      // INSC_ ... TEMP: no dependencies in makefile
#include "ddins.h"                      // self
#include "ddglobal.h"                   // 

// TEMP: why aren't these prototypes in a header file somewhere?
extern DWORD __stdcall ddiClear2 ( LPD3DHAL_CLEAR2DATA pcd );
extern DWORD __stdcall ddiValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pcd );
extern DWORD __stdcall ddiDrawPrimitives2( LPD3DHAL_DRAWPRIMITIVES2DATA pcd );
extern DWORD __stdcall ddiGetDriverState( LPDDHAL_GETDRIVERSTATEDATA pgdsd );
extern DWORD __stdcall ddiCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA pcsxd );
extern DWORD __stdcall ddiDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA pdddd );
#ifdef AGP_EXECUTE
extern DWORD __stdcall DdUpdateNonLocalHeap ( LPDDHAL_UPDATENONLOCALHEAPDATA lpd );
#endif
extern DWORD __stdcall GetAvailDriverMemory (LPDDHAL_GETAVAILDRIVERMEMORYDATA lpData );

// instrumented d3d callbacks ///////////////////////////////////////////////////////////

DWORD __stdcall ddiClear2Ins (LPD3DHAL_CLEAR2DATA pcd)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DCLEAR2);
    rc = ddiClear2(pcd);
    INS_EXIT();
    return rc;
}

// CREATESURFACEEX              LPDDHAL_CREATESURFACEEXDATA );
DWORD __stdcall ddiCreateSurfaceExIns (LPDDHAL_CREATESURFACEEXDATA pcsxd)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DCREATESURFACEEX);
    rc = ddiCreateSurfaceEx(pcsxd);
    INS_EXIT();
    return rc;
}

// DESTROYDDLOCAL               LPDDHAL_DESTROYDDLOCALDATA );
DWORD __stdcall ddiDestroyDDLocalIns (LPDDHAL_DESTROYDDLOCALDATA pdddd)
{
    DWORD rc;
    INS_ENTRY (INSC_DDDESTROYDDLOCAL);
    rc = ddiDestroyDDLocal(pdddd);
    INS_EXIT();
    return rc;
}

DWORD __stdcall ddiDrawPrimitives2Ins (LPD3DHAL_DRAWPRIMITIVES2DATA lpdp2d)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DDRAWPRIMITIVES2);
    rc = ddiDrawPrimitives2(lpdp2d);
    INS_EXIT();
    return rc;
}

DWORD __stdcall ddiGetDriverStateIns (LPDDHAL_GETDRIVERSTATEDATA pgdsd)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DGETDRIVERSTATE);
    rc = ddiGetDriverState(pgdsd);
    INS_EXIT();
    return rc;
}

DWORD __stdcall ddiValidateTextureStageStateIns (LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA lpd)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DVALIDATETEXTURESTAGESTATE);
    rc = ddiValidateTextureStageState(lpd);
    INS_EXIT();
    return rc;
}

// instrumented d3d context callbacks ///////////////////////////////////////////////////

DWORD __stdcall D3dContextCreateIns(LPD3DHAL_CONTEXTCREATEDATA pccd)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DCONTEXTCREATE);
    rc = ddiContextCreate(pccd);
    INS_EXIT();
    return rc;
}

DWORD __stdcall D3dContextDestroyIns(LPD3DHAL_CONTEXTDESTROYDATA pcdd)
{
    DWORD rc;
    INS_ENTRY (INSC_D3DCONTEXTDESTROY);
    rc = ddiContextDestroy(pcdd);
    INS_EXIT();
    return rc;
}

// instrumented d3d vertex buffer callbacks /////////////////////////////////////////////

DWORD __stdcall CanCreateExecuteBuffer32Ins(LPDDHAL_CANCREATESURFACEDATA lpd)
{
    DWORD rc;
    // Need valid INSC token
    INS_ENTRY (INSC_D3DDRIVER);
    rc = CanCreateExecuteBuffer32Ins( lpd );
    INS_EXIT();
    return rc;
}

DWORD __stdcall CreateExecuteBuffer32Ins(LPDDHAL_CREATESURFACEDATA pcsd)
{
    DWORD rc;
    // Need valid INSC token
    INS_ENTRY (INSC_D3DDRIVER);
    rc = CreateExecuteBuffer32Ins( pcsd);
    INS_EXIT();
    return rc;
}

DWORD __stdcall DestroyExecuteBuffer32Ins(LPDDHAL_DESTROYSURFACEDATA pdsd)
{
    DWORD rc;
    // Need valid INSC token
    INS_ENTRY (INSC_D3DDRIVER);
    rc = DestroyExecuteBuffer32Ins( pdsd );
    INS_EXIT();
    return rc;
}

DWORD __stdcall LockExecuteBuffer32Ins(LPDDHAL_LOCKDATA lpd)
{
    DWORD rc;
    // Need valid INSC token
    INS_ENTRY (INSC_D3DDRIVER);
    rc = LockExecuteBuffer32Ins( lpd );
    INS_EXIT();
    return rc;
}

DWORD __stdcall UnlockExecuteBuffer32Ins(LPDDHAL_UNLOCKDATA puld)
{
    DWORD rc;
    // Need valid INSC token
    INS_ENTRY (INSC_D3DDRIVER);
    rc = UnlockExecuteBuffer32Ins( puld );
    INS_EXIT();
    return rc;
}

// instrumented dd surface callbacks ////////////////////////////////////////////////////

// CANCREATESURFACE            LPDDHAL_CANCREATESURFACEDATA );
DWORD __stdcall DdCanCreateSurfaceIns( LPDDHAL_CANCREATESURFACEDATA pcsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDCANCREATESURFACE);
    rc = DdCanCreateSurface( pcsd );
    INS_EXIT();
    return rc;
}

// CREATESURFACE               LPDDHAL_CREATESURFACEDATA);
DWORD __stdcall DdCreateSurfaceIns( LPDDHAL_CREATESURFACEDATA pcsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDCREATESURFACE);
    rc = DdCreateSurface( pcsd );
    INS_EXIT();
    return rc;
}

// DESTROYSURFACE              LPDDHAL_DESTROYSURFACEDATA);
DWORD __stdcall DdDestroySurfaceIns( LPDDHAL_DESTROYSURFACEDATA pdsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDDESTROYSURFACE);
    rc = DdDestroySurface( pdsd );
    INS_EXIT();
    return rc;
}

// ADDATTACHEDSURFACE          LPDDHAL_ADDATTACHEDSURFACEDATA);
DWORD __stdcall DdAddAttachedSurfaceIns( LPDDHAL_ADDATTACHEDSURFACEDATA pasd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDADDATTACHEDSURFACE);
    rc = DdAddAttachedSurface( pasd );
    INS_EXIT();
    return rc;
}

// LOCK                        LPDDHAL_LOCKDATA);
DWORD __stdcall DdLockIns( LPDDHAL_LOCKDATA pld )
{
    DWORD rc;
    INS_ENTRY (INSC_DDLOCK);
    rc = DdLock( pld );
    INS_EXIT();
    return rc;
}

// UNLOCK                      LPDDHAL_UNLOCKDATA);
DWORD __stdcall DdUnlockIns( LPDDHAL_UNLOCKDATA puld )
{
    DWORD rc;
    INS_ENTRY (INSC_DDUNLOCK);
    rc = DdUnlock( puld );
    INS_EXIT();
    return rc;
}

// instrumented dd palette callbacks ////////////////////////////////////////////////////

// CREATEPALETTE               LPDDHAL_CREATEPALETTEDATA);
DWORD __stdcall DdCreatePaletteIns( LPDDHAL_CREATEPALETTEDATA pcpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDCREATEPALETTE);
    rc = DdCreatePalette( pcpd );
    INS_EXIT();
    return rc;
}

// DESTROYPALETTE              LPDDHAL_DESTROYPALETTEDATA );
DWORD __stdcall DdDestroyPaletteIns( LPDDHAL_DESTROYPALETTEDATA pdpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDDESTROYPALETTE);
    rc = DdDestroyPalette( pdpd );
    INS_EXIT();
    return rc;
}

// SETPALETTE                  LPDDHAL_SETPALETTEDATA);
DWORD __stdcall DdSetPaletteIns( LPDDHAL_SETPALETTEDATA pspd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETPALETTE);
    rc = DdSetPalette( pspd );
    INS_EXIT();
    return rc;
}

// instrumented dd blt callbacks ////////////////////////////////////////////////////////

// BLT                         LPDDHAL_BLTDATA);
DWORD __stdcall DdBltIns( LPDDHAL_BLTDATA pbd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDBLT);
    rc = DdBlt( pbd );
    INS_EXIT();
    return rc;
}

// GETBLTSTATUS                LPDDHAL_GETBLTSTATUSDATA);
DWORD __stdcall DdGetBltStatusIns( LPDDHAL_GETBLTSTATUSDATA pgbsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDGETBLTSTATUS);
    rc = DdGetBltStatus( pgbsd );
    INS_EXIT();
    return rc;
}

// instrumented dd flip callbacks ///////////////////////////////////////////////////////

// FLIP                        LPDDHAL_FLIPDATA);
DWORD __stdcall DdFlipIns( LPDDHAL_FLIPDATA pfd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDFLIP);
    rc = DdFlip( pfd );
    INS_EXIT();
    return rc;
}

// FLIPTOGDISURFACE            LPDDHAL_FLIPTOGDISURFACEDATA);
DWORD __stdcall DdFlipToGDISurfaceIns( LPDDHAL_FLIPTOGDISURFACEDATA pftgsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDFLIPTOGDISURFACE);
    rc = DdFlipToGDISurface( pftgsd );
    INS_EXIT();
    return rc;
}

// GETFLIPSTATUS               LPDDHAL_GETVPORTFLIPSTATUSDATA);
DWORD __stdcall DdGetFlipStatusIns( LPDDHAL_GETFLIPSTATUSDATA pgfsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDGETFLIPSTATUS);
    rc = DdGetFlipStatus( pgfsd );
    INS_EXIT();
    return rc;
}

// WAITFORVERTICALBLANK        LPDDHAL_WAITFORVERTICALBLANKDATA );
DWORD __stdcall DdWaitForVerticalBlankIns( LPDDHAL_WAITFORVERTICALBLANKDATA pwfvbd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDWAITFORVERTICALBLANK);
    rc = DdWaitForVerticalBlank( pwfvbd );
    INS_EXIT();
    return rc;
}

// GETSCANLINE                 LPDDHAL_GETSCANLINEDATA);
DWORD __stdcall DdGetScanLineIns( LPDDHAL_GETSCANLINEDATA pgsld )
{
    DWORD rc;
    INS_ENTRY (INSC_DDGETSCANLINE);
    rc = DdGetScanLine( pgsld );
    INS_EXIT();
    return rc;
}

// instrumented dd color callbacks //////////////////////////////////////////////////////

#if 0
// COLORCONTROL                LPDDHAL_COLORCONTROLDATA);
DWORD __stdcall DdControlColorIns( LPDDHAL_COLORCONTROLDATA pccd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDCONTROLCOLOR);
    rc = DdControlColor( pccd );
    INS_EXIT();
    return rc;
}
#endif

// SETCOLORKEY                 LPDDHAL_DRVSETCOLORKEYDATA );
DWORD __stdcall DdSetDrvColorKeyIns( LPDDHAL_DRVSETCOLORKEYDATA psdck )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETCOLORKEY);
    rc = DdSetDrvColorKey( psdck );
    INS_EXIT();
    return rc;
}

DWORD __stdcall DdSetSurfaceColorKeyIns( LPDDHAL_SETCOLORKEYDATA pssck)
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETSURFACECOLORKEY);
    rc = DdSetSurfaceColorKey( pssck );
    INS_EXIT();
    return rc;
}

// instrumented dd misc callbacks ///////////////////////////////////////////////////////

// GETDRIVERINFO               LPDDHAL_GETDRIVERINFODATA);
DWORD __stdcall DdGetDriverInfoIns( LPDDHAL_GETDRIVERINFODATA pgdid )
{
    DWORD rc;
    INS_ENTRY (INSC_DDGETDRIVERINFO);
    rc = DdGetDriverInfo( pgdid );
    INS_EXIT();
    return rc;
}

// SETENTRIES                  LPDDHAL_SETENTRIESDATA );
DWORD __stdcall DdSetEntriesIns( LPDDHAL_SETENTRIESDATA psed )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETENTRIES);
    rc = DdSetEntries( psed );
    INS_EXIT();
    return rc;
}

// SETEXCLUSIVEMODE            LPDDHAL_SETEXCLUSIVEMODEDATA);
DWORD __stdcall DdSetExclusiveModeIns( LPDDHAL_SETEXCLUSIVEMODEDATA psemd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETEXCLUSIVEMODE);
    rc = DdSetExclusiveMode( psemd );
    INS_EXIT();
    return rc;
}

#ifdef AGP_EXECUTE
DWORD __stdcall DdUpdateNonLocalHeapIns (LPDDHAL_UPDATENONLOCALHEAPDATA lpd)
{
    DWORD rc;
    INS_ENTRY (INSC_DDUPDATENONLOCALHEAP);
    rc = DdUpdateNonLocalHeap(lpd);
    INS_EXIT();
    return rc;
}
#endif

DWORD __stdcall GetAvailDriverMemoryIns (LPDDHAL_GETAVAILDRIVERMEMORYDATA lpData)
{
    DWORD rc;
    INS_ENTRY (INSC_DDGETAVAILDRIVERMEMORY);
    rc = GetAvailDriverMemory(lpData);
    INS_EXIT();
    return rc;
}

// instrumented dd overlay callbacks ////////////////////////////////////////////////////

// SETOVERLAYPOSITION          LPDDHAL_SETOVERLAYPOSITIONDATA);
DWORD __stdcall SetOverlayPosition32Ins( LPDDHAL_SETOVERLAYPOSITIONDATA psopd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETOVERLAYPOSITION);
    rc = SetOverlayPosition32( psopd );
    INS_EXIT();
    return rc;
}

// UPDATEOVERLAY               LPDDHAL_UPDATEOVERLAYDATA);
DWORD __stdcall UpdateOverlay32Ins( LPDDHAL_UPDATEOVERLAYDATA puod )
{
    DWORD rc;
    INS_ENTRY (INSC_DDUPDATEOVERLAY);
    rc = UpdateOverlay32( puod );
    INS_EXIT();
    return rc;
}

// no rampage callback exists for the following /////////////////////////////////////////

#if 0

// DESTROYDRIVER               LPDDHAL_DESTROYDRIVERDATA);
// SETMODE                     LPDDHAL_SETMODEDATA);

DWORD __stdcall DdMapMemoryIns( LPDDHAL_MAPMEMORYDATA pmmd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMAPMEMORY);
    rc = DdMapMemory( pmmd );
    INS_EXIT();
    return rc;
}

// SETCLIPLIST                 LPDDHAL_SETCLIPLISTDATA);
DWORD __stdcall DdSetClipListIns( LPDDHAL_SETCLIPLISTDATA pscld )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSETCLIPLIST);
    rc = DdSetClipList( pscld );
    INS_EXIT();
    return rc;
}

// SYNCSURFACE                 LPDDHAL_SYNCSURFACEDATA);
DWORD __stdcall DdSyncSurfaceDataIns( LPDDHAL_SYNCSURFACEDATA pssd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSYNCSURFACEDATA);
    rc = DdSyncSurfaceData( pssd );
    INS_EXIT();
    return rc;
}

// SYNCVIDEOPORT               LPDDHAL_SYNCVIDEOPORTDATA);
DWORD __stdcall DdSyncVideoPortDataIns( LPDDHAL_SYNCVIDEOPORTDATA psvpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDSYNCVIDEOPORTDATA);
    rc = DdSyncVideoPortData( psvpd );
    INS_EXIT();
    return rc;
}

// CANCREATEVIDEOPORT          LPDDHAL_CANCREATEVPORTDATA);
DWORD __stdcall DdVideoPortCanCreateIns( LPDDHAL_CANCREATEVPORTDATA pccvpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTCANCREATE);
    rc = DdVideoPortCanCreate( pccvpd );
    INS_EXIT();
    return rc;
}

// CREATEVIDEOPORT             LPDDHAL_CREATEVPORTDATA);
DWORD __stdcall DdVideoPortCreateIns( LPDDHAL_CREATEVPORTDATA pcvpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTCREATE);
    rc = DdVideoPortCreate( pcvpd );
    INS_EXIT();
    return rc;
}

// DESTROYVPORT                LPDDHAL_DESTROYVPORTDATA);
DWORD __stdcall DdVideoPortDestroyIns( LPDDHAL_DESTROYVPORTDATA pdvpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTDESTROY);
    rc = DdVideoPortDestroy( pdvpd );
    INS_EXIT();
    return rc;
}

// FLIP                        LPDDHAL_FLIPVPORTDATA);
DWORD __stdcall DdVideoPortFlipIns( LPDDHAL_FLIPVPORTDATA pfvpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTFLIP);
    rc = DdVideoPortFlip( pfvpd );
    INS_EXIT();
    return rc;
}

// GETBANDWIDTH                LPDDHAL_GETVPORTBANDWIDTHDATA);
DWORD __stdcall DdVideoPortGetBandwidthIns( LPDDHAL_GETVPORTBANDWIDTHDATA pgvpbwd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETBANDWIDTH);
    rc = DdVideoPortGetBandwidth( pgvpbwd );
    INS_EXIT();
    return rc;
}

// COLORCONTROL                LPDDHAL_VPORTCOLORDATA);
DWORD __stdcall DdVideoPortColorControlIns( LPDDHAL_VPORTCOLORDATA pvpcd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTCOLORCONTROL);
    rc = DdVideoPortColorControl( pvpcd );
    INS_EXIT();
    return rc;
}

// GETVPORTCONNECT             LPDDHAL_GETVPORTCONNECTDATA);
DWORD __stdcall DdVideoPortGetConnectInfoIns( LPDDHAL_GETVPORTCONNECTDATA pgvpcd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETCONNECTINFO);
    rc = DdVideoPortGetConnectInfo( pgvpcd );
    INS_EXIT();
    return rc;
}

// GETFIELD                    LPDDHAL_GETVPORTFIELDDATA);
DWORD __stdcall DdVideoPortGetFieldIns( LPDDHAL_GETVPORTFIELDDATA pgvpfd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETFIELD);
    rc = DdVideoPortGetField( pgvpfd );
    INS_EXIT();
    return rc;
}

DWORD __stdcall DdVideoPortGetFlipStatusIns( LPDDHAL_GETVPORTFLIPSTATUSDATA pgvpfsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETFLIPSTATUS);
    rc = DdVideoPortGetFlipStatus( pgvpfsd );
    INS_EXIT();
    return rc;
}

// GETINPUTFORMATS             LPDDHAL_GETVPORTINPUTFORMATDATA);
DWORD __stdcall DdVideoPortGetInputFormatsIns( LPDDHAL_GETVPORTINPUTFORMATDATA pgvpifd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETINPUTFORMATS);
    rc = DdVideoPortGetInputFormats( pgvpifd );
    INS_EXIT();
    return rc;
}

// GETOUTPUTFORMATS            LPDDHAL_GETVPORTOUTPUTFORMATDATA);
DWORD __stdcall DdVideoPortGetOutputFormatsIns( LPDDHAL_GETVPORTOUTPUTFORMATDATA pgvpofd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETOUTPUTFORMATS);
    rc = DdVideoPortGetOutputFormats( pgvpofd );
    INS_EXIT();
    return rc;
}

// GETLINE                     LPDDHAL_GETVPORTLINEDATA);
DWORD __stdcall DdVideoPortGetLineIns( LPDDHAL_GETVPORTLINEDATA pvpld )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETLINE);
    rc = DdVideoPortGetLine( pvpld );
    INS_EXIT();
    return rc;
}

// GETSIGNALSTATUS             LPDDHAL_GETVPORTSIGNALDATA);
DWORD __stdcall DdVideoPortGetSignalStatusIns( LPDDHAL_GETVPORTSIGNALDATA pgvpsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTGETSIGNALSTATUS);
    rc = DdVideoPortGetSignalStatus( pgvpsd );
    INS_EXIT();
    return rc;
}

// UPDATE                      LPDDHAL_UPDATEVPORTDATA);
DWORD __stdcall DdVideoPortUpdateIns( LPDDHAL_UPDATEVPORTDATA puvpd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTUPDATE);
    rc = DdVideoPortUpdate( puvpd );
    INS_EXIT();
    return rc;
}

// WAITFORSYNC                 LPDDHAL_WAITFORVPORTSYNCDATA);
DWORD __stdcall DdVideoPortWaitForSyncIns( LPDDHAL_WAITFORVPORTSYNCDATA pwfvpsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDVIDEOPORTWAITFORSYNC);
    rc = DdVideoPortWaitForSync( pwfvpsd );
    INS_EXIT();
    return rc;
}

DWORD __stdcall DrvDisableDirectDrawIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DRVDISABLEDIRECTDRAW);
    rc = DrvDisableDirectDraw();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DrvGetDirectDrawInfoIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DRVGETDIRECTDRAWINFO);
    rc = DrvGetDirectDrawInfo();
    INS_EXIT();
    return rc;
}

// GETDRIVERSTATE               LPDDHAL_GETDRIVERSTATEDATA );

DWORD __stdcall DdFreeDriverMemoryIns( LPDDHAL_FREEDRIVERMEMORYDATA pfdmd )
{
    return 0;
}

// BEGINFRAME                  LPDDHAL_BEGINMOCOMPFRAMEDATA);
DWORD __stdcall DdMoCompBeginFrameIns( LPDDHAL_BEGINMOCOMPFRAMEDATA pmcbfd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPBEGINFRAME);
    rc = DdMoCompBeginFrame( pmcbfd );
    INS_EXIT();
    return rc;
}

// CREATE                      LPDDHAL_CREATEMOCOMPDATA);
DWORD __stdcall DdMoCompCreateIns( LPDDHAL_CREATEMOCOMPDATA pmccd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPCREATE);
    rc = DdMoCompCreate( pmccd );
    INS_EXIT();
    return rc;
}

// DESTROY                     LPDDHAL_DESTROYMOCOMPDATA);
DWORD __stdcall DdMoCompDestroyIns( LPDDHAL_DESTROYMOCOMPDATA pmcdd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPDESTROY);
    rc = DdMoCompDestroy( pmcdd );
    INS_EXIT();
    return rc;
}

// ENDFRAME                    LPDDHAL_ENDMOCOMPFRAMEDATA);
DWORD __stdcall DdMoCompEndFrameIns( LPDDHAL_ENDMOCOMPFRAMEDATA pemcfd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPENDFRAME);
    rc = DdMoCompEndFrame( pemcfd );
    INS_EXIT();
    return rc;
}

// GETCOMPBUFFINFO             LPDDHAL_GETMOCOMPCOMPBUFFDATA);
DWORD __stdcall DdMoCompGetBuffInfoIns( LPDDHAL_GETMOCOMPCOMPBUFFDATA pgmccbd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPGETBUFFINFO);
    rc = DdMoCompGetBuffInfo( pgmccbd );
    INS_EXIT();
    return rc;
}

// GETFORMATS                  LPDDHAL_GETMOCOMPFORMATSDATA);
DWORD __stdcall DdMoCompGetFormatsIns( LPDDHAL_GETMOCOMPFORMATSDATA pgmcfd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPGETFORMATS);
    rc = DdMoCompGetFormats( pgmcfd );
    INS_EXIT();
    return rc;
}

// GETGUIDS                    LPDDHAL_GETMOCOMPGUIDSDATA);
DWORD __stdcall DdMoCompGetGuidsIns( LPDDHAL_GETMOCOMPGUIDSDATA pgmcgd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPGETGUIDS);
    rc = DdMoCompGetGuids( pgmcgd );
    INS_EXIT();
    return rc;
}

// GETINTERNALINFO             LPDDHAL_GETINTERNALMOCOMPDATA);
DWORD __stdcall DdMoCompGetInternalInfoIns( LPDDHAL_GETINTERNALMOCOMPDATA pgimcd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPGETINTERNALINFO);
    rc = DdMoCompGetInternalInfo( pgimcd );
    INS_EXIT();
    return rc;
}

// QUERYSTATUS                 LPDDHAL_QUERYMOCOMPSTATUSDATA);
DWORD __stdcall DdMoCompQueryStatusIns( LPDDHAL_QUERYMOCOMPSTATUSDATA pqmcsd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPQUERYSTATUS);
    rc = DdMoCompQueryStatus( pqmcsd );
    INS_EXIT();
    return rc;
}

// RENDER                      LPDDHAL_RENDERMOCOMPDATA);
DWORD __stdcall DdMoCompRenderIns( LPDDHAL_RENDERMOCOMPDATA prmcd )
{
    DWORD rc;
    INS_ENTRY (INSC_DDMOCOMPRENDER);
    rc = DdMoCompRender( prmcd );
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxApiIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXAPIINS);
    rc = DxApi();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxBobNextFieldIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXBOBNEXTFIELD);
    rc = DxBobNextField();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxEnableIRQIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXENABLEIRQ);
    rc = DxEnableIRQ();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxFlipOverlayIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXFLIPOVERLAY);
    rc = DxFlipOverlay();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxFlipVideoPortIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXFLIPVIDEOPORT);
    rc = DxFlipVideoPort();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxGetCurrentAutoflipIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXGETCURRENTAUTOFLIP);
    rc = DxGetCurrentAutoflip();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxGetIRQInfoIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXGETIRQINFO);
    rc = DxGetIRQInfo();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxGetPolarityIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXGETPOLARITY);
    rc = DxGetPolarity();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxGetPreviousAutoFlipIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXGETPREVIOUSAUTOFLIP);
    rc = DxGetPreviousAutoFlip();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxGetTransferStatusIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXGETTRANSFERSTATUS);
    rc = DxGetTransferStatus();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxLockIns( LPDDHAL_LOCKDATA pld )
{
    DWORD rc;
    INS_ENTRY (INSC_DXLOCK);
    rc = DxLock();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxSetStateIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXSETSTATE);
    rc = DxSetState();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxSkipNextFieldIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXSKIPNEXTFIELD);
    rc = DxSkipNextField();
    INS_EXIT();
    return rc;
}

DWORD __stdcall DxTransferIns()
{
    DWORD rc;
    INS_ENTRY (INSC_DXTRANSFER);
    rc = DxTransfer();
    INS_EXIT();
    return rc;
}

DWORD __stdcall IRQCallbackIns()
{
    DWORD rc;
    INS_ENTRY (INSC_IRQCALLBACK);
    rc = IRQCallback();
    INS_EXIT();
    return rc;
}

DWORD __stdcall NotifyCallbackIns()
{
    DWORD rc;
    INS_ENTRY (INSC_NOTIFYCALLBACK);
    rc = NotifyCallback();
    INS_EXIT();
    return rc;
}

#endif

