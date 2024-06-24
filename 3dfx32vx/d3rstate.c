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
**  51   3dfx      1.50        12/8/00  Miles Smith     fixed comments from last
**       checkin
**  50   3dfx      1.49        12/8/00  Miles Smith     Added state changes for 
**       COLORWRITEENABLE, BLENDOP, and MULTISAMPLEMASK
**  49   3dfx      1.48        12/7/00  Brian Danielson Added D3DBlendOp 
**       functionality for DX8.
**  48   3dfx      1.47        11/30/00 Brent Burton    Two minor debug message 
**       changes.
**  47   3dfx      1.46        11/16/00 Miles Smith     Adding DX8 renderstate 
**       code for multisample buffers
**  46   3dfx      1.45        11/14/00 Brent Burton    DX8: Added PointSprite 
**       support through new render functions.  RC now contains new fields 
**       RndrSprites and RndrIndexedSprites.  Renderstates dealing with 
**       PointSprites are now processed.
**  45   3dfx      1.44        11/7/00  Brent Burton    New DX8 renderstate 
**       support (18 new renderstates).  The new states have stub functions for 
**       later implementation.  Removal of wrap member from TSS[] array; using 
**       preexisting pRc->wrap[].
**  44   3dfx      1.43        10/24/00 Don Fowler      Forgot a line in my 
**       previous checkin... the Z bias functionality is now correct.
**  43   3dfx      1.42        10/17/00 Miles Smith     fixed code which reports
**       the state of antialias render state.
**  42   3dfx      1.41        10/11/00 Brent Burton    DX8 code integration.  
**       Changes to headers, new code.
**  41   3dfx      1.40        9/28/00  Dale  Kenaston  Rampage rendering 
**       function pointers. Changed fillMore state from update only to check 
**       first so that its function can set the function pointer dirty flag. 
**       Modified wrapU, wrapV, shadeMode, fillMode and specular functions to 
**       set the function pointer dirty flag. Modified initNewRC to set the 
**       function pointer dirty flag in any new rc.
**  40   3dfx      1.39        9/14/00  Don Fowler      Forgot a line of code 
**       from previous checkin
**  39   3dfx      1.38        9/14/00  Don Fowler      Added support for a 
**       destination blend of invdestalpha
**  38   3dfx      1.37        9/14/00  Don Fowler      Fixed a problem with the
**       renderstate storing mulitple stages of wrap information. To match the 
**       reference rasterizer's output, I iterated the wrap state of stage 0 to 
**       all of the other states. I'm not positive this is correct, but we pass 
**       DCT's and WB2000.
**  37   3dfx      1.36        9/6/00   Don Fowler      Added code to correctly 
**       calculate the LOD Bias 
**  36   3dfx      1.35        9/6/00   Michel Conrad   Correct the sense of 
**       some bits in the peCache default setting.
**  35   3dfx      1.34        8/30/00  Tim Little      More changes to remove 
**       #ifdef HW_TNL conditional compilation
**  34   3dfx      1.33        8/22/00  Brian Danielson Changed the renderstate 
**       management from being tied to the current render context to being a 
**       global concept, now accessable from DD.
**  33   3dfx      1.32        8/16/00  Miles Smith     Added a line to 
**       sceneCapture to force update of HW state for FSAA
**  32   3dfx      1.31        8/11/00  Brian Danielson Fixed setDX6State bug, 
**       enabled peCache, added Central Services code.
**  31   3dfx      1.30        8/3/00   Don Fowler      Added code to handle 
**       legacy ALPHA MODULATE state. Alpha modulate will now get alpha 
**       information from the iterator if there is no alpha information in the 
**       pixel format.    Added support for texture SRC colorkeying    Fixed Z 
**       bias to support Rampage 24 bit bias and added an arbitrary multiplier 
**       to "fixup" the value of 0-15 passed by ddraw. The number I chose was 
**       entirely arbitrary and was just enough to get the tests to pass.    
**       Coupled the AlphaDepthTest with the AlphaColorTest because of a bug in 
**       CSIM and possibly in the hardware.
**  30   3dfx      1.29        7/24/00  Brian Danielson Changes to implement 
**       renderstate and shadow register management.
**  29   3dfx      1.28        7/10/00  Michel Conrad   Remove dead dx5 code.
**  28   3dfx      1.27        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  27   3dfx      1.26        5/11/00  Evan Leland     dx7 structure cleanup 
**       effort complete
**  26   3dfx      1.25        3/30/00  Miles Smith     Fixed some init code for
**       the fog table variables.
**  25   3dfx      1.24        3/16/00  Michel Conrad   Clear depth write mask 
**       whenever depth buffering is disabled. Clear depth write mask when depth
**       buffer is disabled. ZwriteEnable renderstate can be true even though 
**       Zenable is false. Fixes a QT hang when alphablending with depth write 
**       mask enabled and depth buffer disabled.
**  24   3dfx      1.23        2/29/00  Evan Leland     added some D3DPRINT 
**       statements to render state routines
**  23   3dfx      1.22        2/24/00  Brent Burton    Fixed zEnable().  The 
**       existing DX6 code works just fine under DX7, and the DX7 code from 
**       Napalm was broken.  Removed the Napalm code.
**  22   3dfx      1.21        2/23/00  Brian Danielson Added hardware init for 
**       3D triangle clear in Clear2.
**  21   3dfx      1.20        1/27/00  Evan Leland     DX7 changes
**  20   3dfx      1.19        1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  19   3dfx      1.18        1/10/00  Michel Conrad   Opps--comment about 
**       taMode was intended for d6mt.c
**  18   3dfx      1.17        1/10/00  Michel Conrad   Added support to 
**       initialize all 3d registers on context init. Fix mag filter selection 
**       problem for anisotropic. Really clear taMode, fixes sticky block linear
**       state problem. Adjust to changes in d3global structure. Fix typo in 
**       peAlphaTest setup.
**  17   3dfx      1.16        12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  16   3dfx      1.15        11/11/99 Miles Smith     Adding fixes for 
**       w-buffering.
**  15   3dfx      1.14        11/11/99 Brian Danielson Another fix for trivial 
**       rejection clipping. Initialze clip rect, proper vertex processing.
**  14   3dfx      1.13        11/4/99  Ping Zheng      Removed T&L related 
**       state functions and put them into gestate.c    
**  13   3dfx      1.12        11/1/99  Michel Conrad   Improved lodBias 
**       support.
**  12   3dfx      1.11        11/1/99  Miles Smith     fixed a typo in the 
**       debug printf
**  11   3dfx      1.10        11/1/99  Miles Smith     Added some basic code to
**       handle AA renderstate calls
**  10   3dfx      1.9         10/21/99 Brian Danielson Added Triangle based 
**       Clear2 code.
**  9    3dfx      1.8         10/14/99 Miles Smith     Fixing a problem with 
**       state changes and table fog.
**  8    3dfx      1.7         10/13/99 Philip Zheng    Added T&L hooks for R3
**  7    3dfx      1.6         9/28/99  Brent Burton    Renamed prc->texture to 
**       prc->dx5texture.
**  6    3dfx      1.5         9/27/99  Brent Burton    Set the C1 initial 
**       constant color value to 0.5 instead of 1.0.  'Twas originally 0.5.
**  5    3dfx      1.4         9/17/99  Andrew Sobczyk  Added code to enable SLI
**       in raControl and peFbzMode if SLI is enabled
**  4    3dfx      1.3         9/17/99  Michel Conrad   ReEnable Dither setting 
**       via renderstate.
**  3    3dfx      1.2         9/16/99  Brian Danielson Fixes to stencil code : 
**       typo, control output, initial value setup.
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
*/

#include <ddrawi.h>
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "ddglobal.h" 
#include "fxglobal.h" 
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3tri.h"
#include "d3txtr2.h"
#include "geglobal.h"

#ifdef CSERVICE
#include <shared.h>
#endif

// Util from d6mt.c
extern DWORD setColorConstant(float alphagreen, float redblue);

//--------------------------------------------------------------------
//
// Renderstate processing:
//
// Each renderstate is stored into the rendering context. In addition, the
// renderstate is mapped to the SST-1 registers. We have a shadow of each
// SST-1 register and can just set the register when it is time to render
// a primitive. 
//
// Currently, the renderstate processing does not set the register here
// because there is no logic to "context switch" when we change contexts
// or when directdraw changes the state of the hardware. The rendering 
// routines are responsible for setting the state each execute buffer.
//
//--------------------------------------------------------------------
void __stdcall textureHandle(RC *pRc, ULONG state) ;
void __stdcall texturePerspective(RC *pRc, ULONG state) ;
void __stdcall textureFactor(RC *pRc, ULONG state) ;
void __stdcall wrapU(RC *pRc, ULONG state) ;
void __stdcall wrapV(RC *pRc, ULONG state) ;
void __stdcall zEnable(RC *pRc, ULONG state) ;
void __stdcall shadeMode(RC *pRc, ULONG state) ;
void __stdcall linePattern(RC *pRc, ULONG state) ;
void __stdcall rop2(RC *pRc, ULONG state);
void __stdcall zWriteEnable(RC *pRc, ULONG state) ;
void __stdcall alphaTestEnable(RC *pRc, ULONG state) ;
void __stdcall texMag(RC *pRc, ULONG state) ;
void __stdcall texMin(RC *pRc, ULONG state) ;
void __stdcall srcBlend(RC *pRc, ULONG state) ;
void __stdcall dstBlend(RC *pRc, ULONG state) ;
void __stdcall texMapBlend(RC *pRc, ULONG state) ;
void __stdcall cullMode(RC *pRc, ULONG state) ;
void __stdcall zFunc(RC *pRc, ULONG state) ;
void __stdcall alphaRef(RC *pRc, ULONG state) ;
void __stdcall alphaFunc(RC *pRc, ULONG state) ;
void __stdcall ditherEnable(RC *pRc, ULONG state) ;
void __stdcall blendEnable(RC *pRc, ULONG state) ;
void __stdcall fogEnable(RC *pRc, ULONG state) ;
void __stdcall fogColor(RC *pRc, ULONG state) ;
void __stdcall fogTableMode(RC *pRc, ULONG state) ;
void __stdcall fogTableStart(RC *pRc, ULONG state) ;
void __stdcall fogTableEnd(RC *pRc, ULONG state) ;
void __stdcall fogDensity(RC *pRc, ULONG state) ;
void __stdcall subPixel(RC *pRc, ULONG state) ;
void __stdcall specular(RC *pRc, ULONG state) ;
void __stdcall textureAddress(RC *pRc, ULONG state) ;
void __stdcall zVisible(RC *pRc, ULONG state) ;
void __stdcall fillMode(RC *pRc, ULONG state) ;
void __stdcall colorKeyEnable(RC *pRc, ULONG state) ;
void __stdcall alphaBlendEnable(RC *pRc, ULONG state) ;
void __stdcall textureAddressU(RC *pRc, ULONG state) ;
void __stdcall textureAddressV(RC *pRc, ULONG state) ;
void __stdcall lodBias(RC *pRc, ULONG state) ;
void __stdcall zbias(RC *pRc, ULONG state) ;
void __stdcall anisotropy(RC *pRc, ULONG state);
void __stdcall antiAliasEnable(RC *pRc, ULONG state );  

void __stdcall wrap0(RC *pRc, ULONG state) ;
void __stdcall wrap1(RC *pRc, ULONG state) ;
void __stdcall wrap2(RC *pRc, ULONG state) ;
void __stdcall wrap3(RC *pRc, ULONG state) ;
void __stdcall wrap4(RC *pRc, ULONG state) ;
void __stdcall wrap5(RC *pRc, ULONG state) ;
void __stdcall wrap6(RC *pRc, ULONG state) ;
void __stdcall wrap7(RC *pRc, ULONG state) ;

void __stdcall stencilEnable( RC *pRc, ULONG state );
void __stdcall stencilFail( RC *pRc, ULONG state );
void __stdcall stencilZFail( RC *pRc, ULONG state );
void __stdcall stencilPass( RC *pRc, ULONG state );
void __stdcall stencilFunc( RC *pRc, ULONG state );
void __stdcall stencilRef( RC *pRc, ULONG state );
void __stdcall stencilMask( RC *pRc, ULONG state );
void __stdcall stencilWriteMask( RC *pRc, ULONG state );

#if (DIRECT3D_VERSION >= 0x0800)
void __stdcall swVertexProcessing ( RC *pRc, ULONG state );
void __stdcall pointSize ( RC *pRc, ULONG state );
void __stdcall pointSizeMax ( RC *pRc, ULONG state );
void __stdcall pointSizeMin ( RC *pRc, ULONG state );
void __stdcall pointSpriteEnable ( RC *pRc, ULONG state );
void __stdcall pointScaleEnable ( RC *pRc, ULONG state );
void __stdcall pointScaleA ( RC *pRc, ULONG state );
void __stdcall pointScaleB ( RC *pRc, ULONG state );
void __stdcall pointScaleC ( RC *pRc, ULONG state );
void __stdcall multiSampleAA ( RC *pRc, ULONG state );
void __stdcall multiSampleMask ( RC *pRc, ULONG state );
void __stdcall patchEdgeStyle ( RC *pRc, ULONG state );
void __stdcall patchSegments ( RC *pRc, ULONG state );
void __stdcall debugMonitorToken ( RC *pRc, ULONG state );
void __stdcall indexedVertexBlendEnable ( RC *pRc, ULONG state );
void __stdcall colorWriteEnable ( RC *pRc, ULONG state );
void __stdcall tweenFactor ( RC *pRc, ULONG state );
void __stdcall blendOp ( RC *pRc, ULONG state );
#endif

RENDERFXN_RETVAL __stdcall sceneCapture(RC *pRc, ULONG state);

//-----------------------------------------------------------------------
// This is the renderstate function jump table for state changes. if there is a
// dummy in the table then we don't support it yet.
//
// Each entry in the table is a structure (see d3global.h). The first field is
// the renderstate function. The next field is an offset (in bytes) of the
// place in the pRc structure of the field that contains the previous state.
// The third field is a flag field denoting if we should just call the
// renderstate function straight out, or should we check before calling.
//
//-----------------------------------------------------------------------

RENDERFXN _renderFuncs[] =
{
    { dummy,              0,                                        RF_DUMMY       },   // First entry is not used
    { textureHandle,      FIELD_OFFSET(RC, textureStage[0].textureHandle),   RF_CHECK_FIRST },   // D3DRENDERSTATE_TEXTURE_HANDLE       
    { antiAliasEnable,    FIELD_OFFSET(RC, antialias),              RF_CHECK_FIRST },   // D3DRENDERSTATE_ANTIALIAS
    { textureAddress,     FIELD_OFFSET(RC, textureAddress),         RF_CHECK_FIRST },   // D3DTEXTUREADDRESS
    { texturePerspective, FIELD_OFFSET(RC, texturePerspective),     RF_CHECK_FIRST },   // D3DRENDERSTATE_TEXTURE_PERSPECTIVE  
    { wrapU,              FIELD_OFFSET(RC, wrapU),                  RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP_U
    { wrapV,              FIELD_OFFSET(RC, wrapV),                  RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP_V          
    { zEnable,            FIELD_OFFSET(RC, zEnable),                RF_CHECK_FIRST },   // D3DRENDERSTATE_Z_ENABLE             
    { fillMode,           FIELD_OFFSET(RC, fillMode),               RF_CHECK_FIRST },   // D3DRENDERSTATE_FILL_MODE            
    { shadeMode,          FIELD_OFFSET(RC, shadeMode),              RF_CHECK_FIRST },   // D3DRENDERSTATE_SHADE_MODE           
    { linePattern,        0,                                        RF_ALWAYS_CALL },   // D3DRENDERSTATE_LINE_PATTERN         
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_MONOENABLE (just ignore)     
    { rop2,               FIELD_OFFSET(RC, rop2),                   RF_CHECK_FIRST },   // D3DRENDERSTATE_ROP2                 
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_PLANE_MASK           
    { zWriteEnable,       FIELD_OFFSET(RC, zWriteEnable),           RF_CHECK_FIRST },   // D3DRENDERSTATE_Z_WRITE_ENABLE       
    { alphaTestEnable,    FIELD_OFFSET(RC, alphaTestEnable),        RF_CHECK_FIRST },   // D3DRENDERSTATE_ALPHA_TEST_ENABLE    
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_LAST_PIXEL           
    { texMag,             FIELD_OFFSET(RC, texMag),                 RF_CHECK_FIRST },   // D3DRENDERSTATE_TEX_MAG              
    { texMin,             FIELD_OFFSET(RC, texMin),                 RF_ALWAYS_CALL },   // D3DRENDERSTATE_TEX_MIN              
    { srcBlend,           FIELD_OFFSET(RC, srcBlend),               RF_CHECK_FIRST },   // D3DRENDERSTATE_SRC_BLEND            
    { dstBlend,           FIELD_OFFSET(RC, dstBlend),               RF_CHECK_FIRST },   // D3DRENDERSTATE_DST_BLEND            
    { texMapBlend,        FIELD_OFFSET(RC, texMapBlend),            RF_CHECK_FIRST },   // D3DRENDERSTATE_TEX_MAP_BLEND ???        
    { cullMode,           FIELD_OFFSET(RC, cullMode),               RF_CHECK_FIRST },   // D3DRENDERSTATE_CULL_MODE            
    { zFunc,              FIELD_OFFSET(RC, zFunc),                  RF_CHECK_FIRST },   // D3DRENDERSTATE_Z_FUNC               
    { alphaRef,           FIELD_OFFSET(RC, alphaRef),               RF_CHECK_FIRST },   // D3DRENDERSTATE_ALPHA_REF            
    { alphaFunc,          FIELD_OFFSET(RC, alphaFunc),              RF_CHECK_FIRST },   // D3DRENDERSTATE_ALPHA_FUNC           
    { ditherEnable,       FIELD_OFFSET(RC, ditherEnable),           RF_CHECK_FIRST },   // D3DRENDERSTATE_DITHER_ENABLE        
    { blendEnable,        FIELD_OFFSET(RC, blendEnable),            RF_CHECK_FIRST },   // D3DRENDERSTATE_BLEND_ENABLE         
    { fogEnable,          FIELD_OFFSET(RC, fogEnable),              RF_CHECK_FIRST },   // D3DRENDERSTATE_FOG_ENABLE           
    { specular,           FIELD_OFFSET(RC, specular),               RF_ALWAYS_CALL },   // D3DRENDERSTATE_SPECULARENABLE  
    { zVisible,           FIELD_OFFSET(RC, zVisible),               RF_UPDATE_ONLY },   // D3DRENDERSTATE_ZVISIBLE        
    { subPixel,           FIELD_OFFSET(RC, subPixel),               RF_DUMMY       },   // D3DRENDERSTATE_SUBPIXEL        
                // D3DRENDERSTATE_SUBPIXEL is permanently on because Direct X 3.0 sdk samples turn off
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_SUBPIXELX       
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEDALPHA   
    { fogColor,           FIELD_OFFSET(RC, fogColor),               RF_CHECK_FIRST },   // D3DRENDERSTATE_FOGCOLOR        
    { fogTableMode,       FIELD_OFFSET(RC, fogTableMode),           RF_CHECK_FIRST },   // D3DRENDERSTATE_FOGTABLEMODE    
    { fogTableStart,      FIELD_OFFSET(RC, fogTableStart),          RF_ALWAYS_CALL }, // BHD RF_UPDATE_ONLY },   // D3DRENDERSTATE_FOGTABLESTART   
    { fogTableEnd,        FIELD_OFFSET(RC, fogTableEnd),            RF_ALWAYS_CALL }, // BHD RF_UPDATE_ONLY },   // D3DRENDERSTATE_FOGTABLEEND     
    { fogDensity,         FIELD_OFFSET(RC, fogDensity),             RF_ALWAYS_CALL }, // BHD RF_UPDATE_ONLY },   // D3DRENDERSTATE_FOGTABLEDENSITY 
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEENABLE
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_EDGEANTIALIAS
    { colorKeyEnable,     FIELD_OFFSET(RC, colorKeyEnable),         RF_UPDATE_ONLY },   // D3DRENDERSTATE_COLORKEYENABLE
    { alphaBlendEnable,   FIELD_OFFSET(RC, alphaBlendEnable),       RF_CHECK_FIRST },   // D3DRENDERSTATE_ALPHABLENDENABLE
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_BORDERCOLOR
    { textureAddressU,    FIELD_OFFSET(RC, textureAddressU),        RF_CHECK_FIRST },   // D3DRENDERSTATE_TEXTUREADDRESSU
    { textureAddressV,    FIELD_OFFSET(RC, textureAddressV),        RF_CHECK_FIRST },   // D3DRENDERSTATE_TEXTUREADDRESSV
    { lodBias,            FIELD_OFFSET(RC, lodBias),                RF_ALWAYS_CALL },   // D3DRENDERSTATE_MIPMAPLODBIAS
    { zbias,              FIELD_OFFSET(RC, zBias),                  RF_CHECK_FIRST },   // D3DRENDERSTATE_ZBIAS
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_RANGEFOGENABLE     = 48,   /* Enables range-based fog */
    { anisotropy,         FIELD_OFFSET(RC, anisotropy),             RF_UPDATE_ONLY },   // D3DRENDERSTATE_ANISOTROPY         = 49,   /* Max. anisotropy. 1 = no anisotropy */
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_FLUSHBATCH         = 50,   /* Explicit flush for DP batching (DX5 Only) */
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT=51, /* BOOL enable sort-independent transparency */
    { stencilEnable,      FIELD_OFFSET(RC, stencilEnable),          RF_ALWAYS_CALL },   // D3DRENDERSTATE_STENCILENABLE      = 52,   /* BOOL enable/disable stenciling */
    { stencilFail,        FIELD_OFFSET(RC, stencilFail),            RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILFAIL        = 53,   /* D3DSTENCILOP to do if stencil test fails */
    { stencilZFail,       FIELD_OFFSET(RC, stencilZFail),           RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILZFAIL       = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
    { stencilPass,        FIELD_OFFSET(RC, stencilPass),            RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILPASS        = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
    { stencilFunc,        FIELD_OFFSET(RC, stencilFunc),            RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILFUNC        = 56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
    { stencilRef,         FIELD_OFFSET(RC, stencilRef),             RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILREF         = 57,   /* Reference value used in stencil test */
    { stencilMask,        FIELD_OFFSET(RC, stencilMask),            RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILMASK        = 58,   /* Mask value used in stencil test */
    { stencilWriteMask,   FIELD_OFFSET(RC, stencilWriteMask),       RF_CHECK_FIRST },   // D3DRENDERSTATE_STENCILWRITEMASK   = 59,   /* Write mask applied to values written to stencil buffer */
    { textureFactor,      FIELD_OFFSET(RC, textureFactor),          RF_CHECK_FIRST },   // D3DRENDERSTATE_TEXTUREFACTOR      = 60,   /* D3DCOLOR used for multi-texture blend */
    { dummy,              0,                                        RF_DUMMY       },   // Not Used = 61
    { sceneCapture,       0,                                        RF_ALWAYS_CALL },   // D3DRENDERSTATE_SCENECAPTURE       = 62
    { dummy,              0,                                        RF_DUMMY       },   // Not Used = 63
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN00   = 64,   /* Stipple pattern 01...  */
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN01   = 65,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN02   = 66,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN03   = 67,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN04   = 68,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN05   = 69,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN06   = 70,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN07   = 71,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN08   = 72,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN09   = 73,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN10   = 74,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN11   = 75,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN12   = 76,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN13   = 77,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN14   = 78,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN15   = 79,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN16   = 80,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN17   = 81,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN18   = 82,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN19   = 83,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN20   = 84,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN21   = 85,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN22   = 86,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN23   = 87,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN24   = 88,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN25   = 89,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN26   = 90,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN27   = 91,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN28   = 92,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN29   = 93,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN30   = 94,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_STIPPLEPATTERN31   = 95,

    { dummy,              0,                                        RF_DUMMY       },   // Not used = 96
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 97
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 98
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 99
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 100
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 101
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 102
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 103
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 104
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 105
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 106
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 107
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 108
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 109
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 110
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 111
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 112
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 113
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 114
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 115
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 116
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 117
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 118
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 119
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 120
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 121
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 122
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 123
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 124
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 125
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 126
    { dummy,              0,                                        RF_DUMMY       },   // Not used = 127
    { wrap0,              FIELD_OFFSET(RC, wrap[0]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP0 = 128,  /* wrap for 1st texture coord. set */
    { wrap1,              FIELD_OFFSET(RC, wrap[1]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP1 = 129,  /* wrap for 2nd texture coord. set */
    { wrap2,              FIELD_OFFSET(RC, wrap[2]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP2 = 130,  /* wrap for 3rd texture coord. set */
    { wrap3,              FIELD_OFFSET(RC, wrap[3]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP3 = 131,  /* wrap for 4th texture coord. set */
    { wrap4,              FIELD_OFFSET(RC, wrap[4]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP4 = 132,  /* wrap for 5th texture coord. set */
    { wrap5,              FIELD_OFFSET(RC, wrap[5]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP5 = 133,  /* wrap for 6th texture coord. set */
    { wrap6,              FIELD_OFFSET(RC, wrap[6]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP6 = 134,  /* wrap for 7th texture coord. set */
    { wrap7,              FIELD_OFFSET(RC, wrap[7]),   RF_CHECK_FIRST },   // D3DRENDERSTATE_WRAP7 = 135,  /* wrap for 8th texture coord. set */
#if 1
//#ifdef HW_TNL
    { GE_State_Clipping,        0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_CLIPPING               = 136,
    { GE_State_Lighting,        0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_LIGHTING               = 137,
//  { GE_State_Extents,         0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_EXTENTS                = 138,
    { dummy,                    0,                                  RF_DUMMY       },   // D3DRENDERSTATE_EXTENTS                = 138,
    { GE_State_Ambient,         0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_AMBIENT                = 139,
    { GE_State_FogVertexMode,   0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_FOGVERTEXMODE          = 140,
    { GE_State_ColorVertex,     0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_COLORVERTEX            = 141,
    { GE_State_LocalViewer,     0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_LOCALVIEWER            = 142,
    { GE_State_NormNormals,     0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_NORMALIZENORMALS       = 143,
    { GE_State_CKeyBlend,       0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_COLORKEYBLENDENABLE    = 144,
    { GE_State_DfusMaterialSrc, 0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_DIFFUSEMATERIALSOURCE  = 145,
    { GE_State_SpecMaterialSrc, 0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_SPECULARMATERIALSOURCE = 146,
    { GE_State_AmbMaterialSrc,  0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_AMBIENTMATERIALSOURCE  = 147,
    { GE_State_EmisMaterialSrc, 0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_EMISSIVEMATERIALSOURCE = 148,
    { dummy,                    0,                                  RF_DUMMY       },   // D3DRENDERSTATE_ALPHASOURCE            = 149,
    { dummy,                    0,                                  RF_DUMMY       },   // D3DRENDERSTATE_FOGFACTORSOURCE        = 150,
    { GE_State_VertexBlend,     0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_VERTEXBLEND            = 151,
    { GE_State_ClipPlanEnable,  0,                                  RF_ALWAYS_CALL },   // D3DRENDERSTATE_CLIPPLANEENABLE        = 152,
#else
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_CLIPPING               = 136,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_LIGHTING               = 137,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_EXTENTS                = 138,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_AMBIENT                = 139,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_FOGVERTEXMODE          = 140,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_COLORVERTEX            = 141,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_LOCALVIEWER            = 142,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_NORMALIZENORMALS       = 143,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_COLORKEYBLENDENABLE    = 144,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_DIFFUSEMATERIALSOURCE  = 145,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_SPECULARMATERIALSOURCE = 146,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_AMBIENTMATERIALSOURCE  = 147,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_EMISSIVEMATERIALSOURCE = 148,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_ALPHASOURCE            = 149,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_FOGFACTORSOURCE        = 150,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_VERTEXBLEND            = 151,
    { dummy,              0,                                        RF_DUMMY       },   // D3DRENDERSTATE_CLIPPLANEENABLE        = 152,
#endif

#if (DIRECT3D_VERSION >= 0x0800)
    { swVertexProcessing,       FIELD_OFFSET(RC, swVertexProcessing),       RF_CHECK_FIRST}, // D3DRS_SOFTWAREVERTEXPROCESSING  = 153
    { pointSize,                FIELD_OFFSET(RC, pointSize),                RF_CHECK_FIRST}, // D3DRS_POINTSIZE                 = 154
    { pointSizeMin,             FIELD_OFFSET(RC, pointSizeMin),             RF_CHECK_FIRST}, // D3DRS_POINTSIZE_MIN             = 155
    { pointSpriteEnable,        FIELD_OFFSET(RC, pointSpriteEnable),        RF_CHECK_FIRST}, // D3DRS_POINTSPRITEENABLE         = 156
    { pointScaleEnable,         FIELD_OFFSET(RC, pointScaleEnable),         RF_CHECK_FIRST}, // D3DRS_POINTSCALEENABLE          = 157
    { pointScaleA,              FIELD_OFFSET(RC, pointScaleA),              RF_CHECK_FIRST}, // D3DRS_POINTSCALE_A              = 158
    { pointScaleB,              FIELD_OFFSET(RC, pointScaleB),              RF_CHECK_FIRST}, // D3DRS_POINTSCALE_B              = 159
    { pointScaleC,              FIELD_OFFSET(RC, pointScaleC),              RF_CHECK_FIRST}, // D3DRS_POINTSCALE_C              = 160
    { multiSampleAA,            FIELD_OFFSET(RC, multiSampleAA),            RF_CHECK_FIRST}, // D3DRS_MULTISAMPLEANTIALIAS      = 161
    { multiSampleMask,          FIELD_OFFSET(RC, multiSampleMask),          RF_CHECK_FIRST}, // D3DRS_MULTISAMPLEMASK           = 162
    { patchEdgeStyle,           FIELD_OFFSET(RC, patchEdgeStyle),           RF_CHECK_FIRST}, // D3DRS_PATCHEDGESTYLE            = 163
    { patchSegments,            FIELD_OFFSET(RC, patchSegments),            RF_CHECK_FIRST}, // D3DRS_PATCHSEGMENTS             = 164
    { debugMonitorToken,        FIELD_OFFSET(RC, debugMonitorToken),        RF_CHECK_FIRST}, // D3DRS_DEBUGMONITORTOKEN         = 165
    { pointSizeMax,             FIELD_OFFSET(RC, pointSizeMax),             RF_CHECK_FIRST}, // D3DRS_POINTSIZE_MAX             = 166
    { indexedVertexBlendEnable, FIELD_OFFSET(RC, indexedVertexBlendEnable), RF_CHECK_FIRST}, // D3DRS_INDEXEDVERTEXBLENDENABLE  = 167
    { colorWriteEnable,         FIELD_OFFSET(RC, colorWriteEnable),         RF_CHECK_FIRST}, // D3DRS_COLORWRITEENABLE          = 168
    { tweenFactor,              FIELD_OFFSET(RC, tweenFactor),              RF_CHECK_FIRST}, // D3DRS_TWEENFACTOR               = 170
    { blendOp,                  FIELD_OFFSET(RC, blendOp),                  RF_CHECK_FIRST}, // D3DRS_BLENDOP                   = 171
#endif // DX8 renderstates (#153-171)
};

// This table provides the register group value for a specific register.
// NOTE : this table MUST be updated if the SST2_3DREGS structure is changed!!!!
SST2_3DREGS reg3D =
    {//         Group Type              // Register name
                SC_NOTUSED,             // mopCMD
                SC_NOTUSED,             // yLine
                SC_DIRECT,              // leftOverlayBuf
                SC_DIRECT,              // rightOverlayBuf

                SC_DIRECT,              // vpW
                SC_DIRECT,              // vpX
                SC_DIRECT,              // vpY
                SC_DIRECT,              // vpZ
                SC_NOTUSED,             // vpR
                SC_NOTUSED,             // vpG
                SC_NOTUSED,             // vpB
                SC_NOTUSED,             // vpA
                SC_DIRECT,              // vpS
                SC_DIRECT,              // vpT
                SC_DIRECT,              // vpQ
                SC_DIRECT,              // vpARGB

                SC_VIEWPORT,            // vpMode
                SC_VIEWPORT,            // vpSizeX
                SC_VIEWPORT,            // vpCenterX
                SC_VIEWPORT,            // vpSizeY
                SC_VIEWPORT,            // vpCenterY
                SC_VIEWPORT,            // vpSizeZ
                SC_VIEWPORT,            // vpCenterZ
                SC_NOTUSED,             // reservedB

                SC_VIEWPORT,            // vpSTscale0
                SC_VIEWPORT,            // vpSTscale1
        {
                SC_NOTUSED,             // reservedC[0]
                SC_NOTUSED,             // reservedC[1]
        },
                SC_SETUP_UNIT,          // suMode
                SC_SETUP_UNIT,          // suParamMask
        {
                SC_NOTUSED,             // reservedD[0]
                SC_NOTUSED,             // reservedD[1]
        },

        {
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[0]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[1]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[2]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[3]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[4]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[5]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[6]
                SC_SETUP_UNIT_CLIP,     // suClipMinXMaxX[7]
        },
        {
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[0]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[1]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[2]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[3]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[4]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[5]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[6]
                SC_SETUP_UNIT_CLIP,     // suClipMinYMaxY[7]
        },
                SC_SETUP_UNIT_CLIP,     // suClipEnables

                SC_SETUP_UNIT,          // suLineWidth

                SC_NOTUSED,             // reserved
                SC_DIRECT,              // suDrawCmd
                SC_NOTUSED,             // suOow
                SC_NOTUSED,             // suXow
                SC_NOTUSED,             // suYow
                SC_NOTUSED,             // suZow
                SC_NOTUSED,             // suR
                SC_NOTUSED,             // suG
                SC_NOTUSED,             // suB
                SC_NOTUSED,             // suA
                SC_NOTUSED,             // suSow
                SC_NOTUSED,             // suTow
                SC_NOTUSED,             // suQow
                SC_NOTUSED,             // suARGB
                SC_NOTUSED,             // suBypass
        {
                SC_NOTUSED,             // reservedE[0]
                SC_NOTUSED,             // reservedE[1]
                SC_NOTUSED,             // reservedE[2]
        },
                SC_NOTUSED,             // suRA0;      
                SC_NOTUSED,             // suRA1;      
                SC_NOTUSED,             // suRA2;      
                SC_NOTUSED,             // suRA3;      
                SC_NOTUSED,             // suVTASlopeHi
                SC_NOTUSED,             // suVTASlopeLo
                SC_NOTUSED,             // suVTAStartHi
                SC_NOTUSED,             // suVTAStartLo

                SC_RA_SETUP,            // raControl
                SC_RA_SETUP,            // raStipple

        {
                SC_NOTUSED,             // reservedF [0]
                SC_NOTUSED,             // reservedF [1]
                SC_NOTUSED,             // reservedF [2]
                SC_NOTUSED,             // reservedF [3]
                SC_NOTUSED,             // reservedF [4]
                SC_NOTUSED,             // reservedF [5]
                SC_NOTUSED,             // reservedF [6]
                SC_NOTUSED,             // reservedF [7]
        },
        {
                SC_NOTUSED,             // reservedG [0]
                SC_NOTUSED,             // reservedG [1]
                SC_NOTUSED,             // reservedG [2]
                SC_NOTUSED,             // reservedG [3]
                SC_NOTUSED,             // reservedG [4]
                SC_NOTUSED,             // reservedG [5]
                SC_NOTUSED,             // reservedG [6]
                SC_NOTUSED,             // reservedG [7]
        },
        {
                SC_NOTUSED,             // reservedH [0]
                SC_NOTUSED,             // reservedH [1]
                SC_NOTUSED,             // reservedH [2]
                SC_NOTUSED,             // reservedH [3]
                SC_NOTUSED,             // reservedH [4]
                SC_NOTUSED,             // reservedH [5]
                SC_NOTUSED,             // reservedH [6]
                SC_NOTUSED,             // reservedH [7]
        },
        {
                SC_NOTUSED,             // reservedI [0]
                SC_NOTUSED,             // reservedI [1]
                SC_NOTUSED,             // reservedI [2]
                SC_NOTUSED,             // reservedI [3]
                SC_NOTUSED,             // reservedI [4]
                SC_NOTUSED,             // reservedI [5]
                SC_NOTUSED,             // reservedI [6]
                SC_NOTUSED,             // reservedI [7]
        },
        {
                SC_NOTUSED,             // reservedO [0]
                SC_NOTUSED,             // reservedO [1]
                SC_NOTUSED,             // reservedO [2]
                SC_NOTUSED,             // reservedO [3]
                SC_NOTUSED,             // reservedO [4]
                SC_NOTUSED,             // reservedO [5]
                SC_NOTUSED,             // reservedO [6]
                SC_NOTUSED,             // reservedO [7]
        },
        {
                SC_NOTUSED,             // reservedP [0]
                SC_NOTUSED,             // reservedP [1]
                SC_NOTUSED,             // reservedP [2]
                SC_NOTUSED,             // reservedP [3]
                SC_NOTUSED,             // reservedP [4]
                SC_NOTUSED,             // reservedP [5]
                SC_NOTUSED,             // reservedP [6]
                SC_NOTUSED,             // reservedP [7]
        },
        {
                SC_NOTUSED,             // reservedQ [0]
                SC_NOTUSED,             // reservedQ [1]
        },

        {
                SC_FOG,                 // peFogTable [00]
                SC_FOG,                 // peFogTable [01]
                SC_FOG,                 // peFogTable [02]
                SC_FOG,                 // peFogTable [03]
                SC_FOG,                 // peFogTable [04]
                SC_FOG,                 // peFogTable [05]
                SC_FOG,                 // peFogTable [06]
                SC_FOG,                 // peFogTable [07]
                SC_FOG,                 // peFogTable [08]
                SC_FOG,                 // peFogTable [09]
                SC_FOG,                 // peFogTable [10]
                SC_FOG,                 // peFogTable [11]
                SC_FOG,                 // peFogTable [12]
                SC_FOG,                 // peFogTable [13]
                SC_FOG,                 // peFogTable [14]
                SC_FOG,                 // peFogTable [15]
                SC_FOG,                 // peFogTable [16]
                SC_FOG,                 // peFogTable [17]
                SC_FOG,                 // peFogTable [18]
                SC_FOG,                 // peFogTable [19]
        },
                SC_FOG,                 // peFogColor
                SC_FOG,                 // peFogMode
                SC_PE_SETUP,            // peFbzMode
                SC_PE_SETUP,            // peAlphaTest
                SC_PE_SETUP,            // peAlphaMode
                SC_PE_SETUP,            // peSDConst
                SC_FOG,                 // peFogBias
                SC_PE_SETUP,            // peStencil
                SC_PE_SETUP,            // peStencilOp

                SC_PE_SETUP,            // peCache - best performance setting :
        {
                SC_NOTUSED,             // reservedJ [0]
                SC_NOTUSED,             // reservedJ [1]
                SC_NOTUSED,             // reservedJ [2]
                SC_NOTUSED,             // reservedJ [3]
        },

                SC_PE_SETUP,            // peColBufferAddr
                SC_PE_SETUP,            // peAuxBufferAddr
                SC_PE_SETUP,            // peBufferSize
                SC_PE_SETUP,            // peClipMinXMaxX
                SC_PE_SETUP,            // peClipMinYMaxY
                SC_PE_SETUP,            // peExMask

        {
                SC_NOTUSED,             // reservedK [0]
                SC_NOTUSED,             // reservedK [1]
                SC_NOTUSED,             // reservedK [2]
                SC_NOTUSED,             // reservedK [3]
                SC_NOTUSED,             // reservedK [4]
                SC_NOTUSED,             // reservedK [5]
                SC_NOTUSED,             // reservedK [6]
                SC_NOTUSED,             // reservedK [7]
        },
        {
                SC_NOTUSED,             // reservedL [0]
                SC_NOTUSED,             // reservedL [1]
                SC_NOTUSED,             // reservedL [2]
                SC_NOTUSED,             // reservedL [3]
                SC_NOTUSED,             // reservedL [4]
                SC_NOTUSED,             // reservedL [5]
                SC_NOTUSED,             // reservedL [6]
                SC_NOTUSED,             // reservedL [7]
        },
        {
                SC_NOTUSED,             // reservedN [0]
                SC_NOTUSED,             // reservedN [1]
                SC_NOTUSED,             // reservedN [2]
                SC_NOTUSED,             // reservedN [3]
                SC_NOTUSED,             // reservedN [4]
                SC_NOTUSED,             // reservedN [5]
                SC_NOTUSED,             // reservedN [6]
                SC_NOTUSED,             // reservedN [7]
        },

                SC_VTA_CONTROL,         // taControl

        // 8 TMU sub structures
        {
            {   // TMU #0
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #1
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #2
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #3
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #4
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #5
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #6
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            },

            {   // TMU #7
                SC_TMUGROUP_A,          // taMode
                SC_TMUGROUP_A,          // taLMS
                SC_TMUGROUP_A,          // taShiftBias
                SC_TMUGROUP_A,          // taDetail
                SC_TMUGROUP_A,          // taNPT
                SC_TMUGROUP_A,          // taBaseAddr0
                SC_TMUGROUP_A,          // taBaseAddr1
                SC_TMUGROUP_A,          // taBaseAddr2
                SC_TMUGROUP_A,          // taBaseAddr3
                SC_TMUGROUP_A,          // taTcuColor
                SC_TMUGROUP_A,          // taTcuAlpha
                SC_TMUGROUP_A,          // taCcuControl
                SC_TMUGROUP_A,          // taCcuColor
                SC_TMUGROUP_A,          // taCcuAlpha
                SC_TMUGROUP_B,          // taTexChromaKey
                SC_TMUGROUP_B,          // taTexChromaKeyRange
                SC_TMUGROUP_B,          // taChromaKey
                SC_TMUGROUP_B,          // taChromaRange
                SC_TMUGROUP_B,          // taColorAR0
                SC_TMUGROUP_B,          // taColorGB0
                SC_TMUGROUP_B,          // taColorAR1
                SC_TMUGROUP_B,          // taColorGB1
            }
        },

                SC_MISC,                    // taLfbMode
                SC_MISC,                    // taLfbADConst
    };



// PKT3 header tables - stolen from Glide3

/*---------------------------------------------------------------------------
** BuildPacket3Headers
**
** Builds all the packet3 headers for all combinations of primitives,
** maximum number of vertex registers available, and the current vertex
** register that the first vertex in the primitive will be written to.
** You can think of this as a triple dimensioned array:
**
**     header[PRIM_TYPE][MAX_VERT_REG][CUR_VERT_REG]
**
** However, since this array is pretty big, we use the fact that there
** isn't any interesting data when CUR_VERT_REG > MAX_VERT_REG.  Really
** the data is one of those lower-triangular thingies.  So the array is:
**
**     header[PRIM_TYPE][BIG_BLOB_OFF_STUFF]
**
** and we need to intelligently index into it.  Whereas
** MAX_VERT_REG * CUR_VERT_REG would have been 1024 entries, the
** lower-triangular matrix is only 528 entries (1+2+...+31+32).
**
** The more observant of you will have noticed that we're storing
** away information for the cases where MAX_VERT_REG < 4, which is
** impossible on SST2.  Since it's such a small amount of extra data,
** and the code is already convoluted, I elected to just leave it in
** and keep things that much simpler.
**
** The next vertex id array is conceptually the same as the header
** array, except that it is really only a function of the number of
** vertices sent in the packet.  Since we can only send 0-4 verts per
** packet, the next vertex id table only needs to be 5 entries big,
** instead of the full GR_NUM_PACKET3_TYPES.
*/

FxU8  g_nvi_table   [5][528];                      // next vertex ids
FxU32 g_header_table[GR_NUM_PACKET3_TYPES][528];   // packet3 headers

// This array holds the offset into the nvi/header tables.  It's indexed
// by the maximum number of vertex registers available (1..32).  It
// simply represents n*(n-1)/2.

FxU32 g_header_offset[33] = {
      0,   0,   1,   3,   6,  10,  15,  21,  28,  36,
     45,  55,  66,  78,  91, 105, 120, 136, 153, 171,
    190, 210, 231, 253, 276, 300, 325, 351, 378, 406,
    435, 465, 496 };

static void BuildPacket3Headers(void)
{

  int i, j, offset, mod_result;
  int mod_data[40*33];
  int **mod, *mod_ptrs[40];

  // thread pointers to get mod[-3..36][0..32]
  mod = mod_ptrs+3;
  mod[-3] = mod_data;
  for ( j=-2 ; j<37 ; j++ )
    mod[j] = mod[j-1] + 33;

  // build modulo table
  for ( i=0 ; i<33 ; i++ ) {

    // do the negative numbers
    mod_result=-1;
    for ( j=-1 ; j>-4 ; j-- ) {
      if (mod_result < 0)
        mod_result = MAX(0, i-1);
      mod[j][i] = mod_result;
      mod_result--;
    }

    // do the positive numbers
    mod_result=0;
    for ( j=0 ; j<37 ; j++ ) {
      mod[j][i] = mod_result;
      if (++mod_result >= i)
        mod_result = 0;
    }
  }

  // fill in the header tables
  for ( i=0 ; i<=32 ; i++ ) {

    offset = g_header_offset[i];

    for ( j=0 ; j<i ; j++, offset++ ) {

      // two independent line segments per packet
      g_header_table[GR_PKT3_TWO_LINE_SEGMENTS][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_AB  |
              SSTCP_PKT3_DRAW_CD  |
              (mod[j+0][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+3][i] << SSTCP_PKT3_IDX_D_SHIFT);

      // linestrip, load AB--, draw A->B
      g_header_table[GR_PKT3_LINESTRIP_LOAD_AB_DRAW_AB][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_DRAW_AB  |
              (mod[j+0][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT);

      // linestrip, load -B--, draw A->B
      g_header_table[GR_PKT3_LINESTRIP_LOAD_B_DRAW_AB][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_DRAW_AB  |
              (mod[j-1][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_B_SHIFT);

      // linestrip, load ABC-, draw A->B->C
      g_header_table[GR_PKT3_LINESTRIP_LOAD_ABC_DRAW_ABC][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_DRAW_AB  |
              SSTCP_PKT3_DRAW_BC  |
              (mod[j+0][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT);

      // linestrip, load -BC-, draw A->B->C
      g_header_table[GR_PKT3_LINESTRIP_LOAD_BC_DRAW_ABC][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_DRAW_AB  |
              SSTCP_PKT3_DRAW_BC  |
              (mod[j-1][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_C_SHIFT);

      // linestrip, load ABCD, draw A->B->C->D
      g_header_table[GR_PKT3_LINESTRIP_LOAD_ABCD_DRAW_ABCD][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_AB  |
              SSTCP_PKT3_DRAW_BC  |
              SSTCP_PKT3_DRAW_CD  |
              (mod[j+0][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+3][i] << SSTCP_PKT3_IDX_D_SHIFT);

      // linestrip, load -BCD, draw A->B->C->D
      g_header_table[GR_PKT3_LINESTRIP_LOAD_BCD_DRAW_ABCD][offset] =
              SSTCP_PKT3          |
              /* SSTCP_PKT3_LINE */
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_AB  |
              SSTCP_PKT3_DRAW_BC  |
              SSTCP_PKT3_DRAW_CD  |
              (mod[j-1][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_D_SHIFT);

      // one triangle in a tristrip, load all required vertices
      g_header_table[GR_PKT3_TRISTRIP_LOAD_ABC_DRAW_ABC][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_DRAW_ABC |
              (mod[j+0][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT);

      // two triangles in a tristrip, load all required vertices
      g_header_table[GR_PKT3_TRISTRIP_LOAD_ABCD_DRAW_ABCD][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_ABC |
              SSTCP_PKT3_DRAW_CBD |
              (mod[j+0][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+3][i] << SSTCP_PKT3_IDX_D_SHIFT);

      // one triangle in a tristrip, reuse initial 2 vertices
      g_header_table[GR_PKT3_TRISTRIP_LOAD_C_DRAW_ABC][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_DRAW_ABC |
              (mod[j-2][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j-1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_C_SHIFT);

      // two triangles in a tristrip, reuse initial 2 vertices
      g_header_table[GR_PKT3_TRISTRIP_LOAD_CD_DRAW_ABCD][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_ABC |
              SSTCP_PKT3_DRAW_CBD |
              (mod[j-2][i] << SSTCP_PKT3_IDX_A_SHIFT) |
              (mod[j-1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_D_SHIFT);

      // one triangle in a trifan, load all required vertices
      g_header_table[GR_PKT3_TRIFAN_LOAD_ABC_DRAW_ABC][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_DRAW_ABC |
              // index A needs to be or'ed in explicitly at run time
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT);

      // two triangles in a trifan, load all required vertices
      g_header_table[GR_PKT3_TRIFAN_LOAD_ABCD_DRAW_ABCD][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_A   |
              SSTCP_PKT3_LOAD_B   |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_ABC |
              SSTCP_PKT3_DRAW_ACD |
              // index A needs to be or'ed in explicitly at run time
              (mod[j+1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+2][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+3][i] << SSTCP_PKT3_IDX_D_SHIFT);

      // one triangle in a trifan, reuse initial 2 vertices
      g_header_table[GR_PKT3_TRIFAN_LOAD_C_DRAW_ABC][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_DRAW_ABC |
              // index A needs to be or'ed in explicitly at run time
              (mod[j-1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_C_SHIFT);

      // two triangles in a trifan, reuse initial 2 vertices
      g_header_table[GR_PKT3_TRIFAN_LOAD_CD_DRAW_ABCD][offset] =
              SSTCP_PKT3          |
              SSTCP_PKT3_TRIANGLE |
              SSTCP_PKT3_LOAD_C   |
              SSTCP_PKT3_LOAD_D   |
              SSTCP_PKT3_DRAW_ABC |
              SSTCP_PKT3_DRAW_ACD |
              // index A needs to be or'ed in explicitly at run time
              (mod[j-1][i] << SSTCP_PKT3_IDX_B_SHIFT) |
              (mod[j+0][i] << SSTCP_PKT3_IDX_C_SHIFT) |
              (mod[j+1][i] << SSTCP_PKT3_IDX_D_SHIFT);

    }

  }

  // build up the next-vertex-id table
  for ( i=0 ; i<=32 ; i++ ) {
    offset = g_header_offset[i];
    for ( j=0 ; j<i ; j++, offset++ ) {
      g_nvi_table[0][offset] = mod[j+0][i];
      g_nvi_table[1][offset] = mod[j+1][i];
      g_nvi_table[2][offset] = mod[j+2][i];
      g_nvi_table[3][offset] = mod[j+3][i];
      g_nvi_table[4][offset] = mod[j+4][i];
    }
  }
}
                      
//------------------------------------------------------------------------------
//
//  Unsupported attribute
//  
//------------------------------------------------------------------------------

// This attribute is not supported and the application didn't look at the
// capability flags to see that we can't do this. D3D does not support
// punting back for rasterization. So we have to ignore this attribute.
void __stdcall dummy(RC *pRc, ULONG state)
{
  D3DPRINT( 255, "Warning: State %d, is unsupported. Check capability flags.", state );
}


//----------------------
//
// Z buffer attributes                             
//
//----------------------
void __stdcall zFunc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zFunc %u", state);
  
  // TODO: Only set Depth bias if Z buffer is 16-bit
  pRc->zFunc = state;
  
  // clear the existing z comparison state  
  pRc->sst.peFbzMode.vFxU32 &= ~(SST_PE_ZFUNC_LT | SST_PE_ZFUNC_EQ | SST_PE_ZFUNC_GT) ;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set Invalid Z compare value" );
    case D3DCMP_LESS:
      pRc->sst.peFbzMode.vFxU32 |= SST_PE_ZFUNC_LT;
      pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias) & 0xFFFFFF;
      break;
    case D3DCMP_NEVER:
      // clearing the field above is compare never
      break;
    case D3DCMP_EQUAL:
      pRc->sst.peFbzMode.vFxU32 |= SST_PE_ZFUNC_EQ;
      pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias) & 0xFFFFFF;
      break;
    case D3DCMP_LESSEQUAL:
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_ZFUNC_LT | SST_PE_ZFUNC_EQ);
      pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias) & 0xFFFFFF;
      break;
    case D3DCMP_GREATER:
      pRc->sst.peFbzMode.vFxU32 |= SST_PE_ZFUNC_GT;
      pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias) & 0xFFFFFF;
      break;
    case D3DCMP_NOTEQUAL:
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_ZFUNC_LT | SST_PE_ZFUNC_GT);
      pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias) & 0xFFFFFF;
      break;
    case D3DCMP_GREATEREQUAL:
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_ZFUNC_GT | SST_PE_ZFUNC_EQ);
      pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias) & 0xFFFFFF;
      break;
    case D3DCMP_ALWAYS:
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_ZFUNC_LT | SST_PE_ZFUNC_EQ | SST_PE_ZFUNC_GT);
      break;
  }

  pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_COPY<<SST_PE_LOGIC_OP_SHIFT);
  
  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peFbzMode.group | reg3D.peSDConst.group );
}


void __stdcall zEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zEnable %u", state);
  
  pRc->zEnable = state;
    
  // hardware will perform depth comparisons before drawing into frame
  // buffer. 
  // Currently we chose to use integer depth for both z and q. For
  // Z and W buffering.
  // TODO: registry option to select floating point z and q

  switch( state )
  {
    case D3DZB_TRUE :
      pRc->sst.peFbzMode.vFxU32 &= ~(SST_PE_DEPTH_FLOAT | 
                                    SST_PE_DEPTH_WRMASK);

      pRc->sst.peFbzMode.vFxU32 |=  (SST_PE_EN_DEPTH_BUFFER |
                                    SST_PE_DEPTH_ZQ_SEL);

      if( pRc->zWriteEnable )
          pRc->sst.peFbzMode.vFxU32 |= SST_PE_DEPTH_WRMASK;

      if(!(pRc->state & (STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG)))
      {
          pRc->state &= ~STATE_REQUIRES_W_FBI;
          pRc->sst.suMode.vFxU32 &= ~SST_SU_Q;
      }
      pRc->state &= ~(STATE_REQUIRES_WBUFFER);
      pRc->state |= (STATE_REQUIRES_OOZ | STATE_REQUIRES_ZBUFFER);
      pRc->sst.suMode.vFxU32 |= SST_SU_Z;
      break;

    case D3DZB_FALSE :
      pRc->sst.peFbzMode.vFxU32 &= ~(SST_PE_DEPTH_FLOAT |
                                    SST_PE_EN_DEPTH_BUFFER |
                                    SST_PE_DEPTH_WRMASK |
                                    SST_PE_DEPTH_ZQ_SEL);

      if(!(pRc->state & (STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG)))
      {
          pRc->state &= ~STATE_REQUIRES_W_FBI;
          pRc->sst.suMode.vFxU32 &= ~SST_SU_Q;
      }
      pRc->state &= ~(STATE_REQUIRES_OOZ |
                      STATE_REQUIRES_ZBUFFER |
                      STATE_REQUIRES_WBUFFER);
      pRc->sst.suMode.vFxU32 &= ~SST_SU_Z;
      break;

    case D3DZB_USEW :
      pRc->sst.peFbzMode.vFxU32 &= ~(SST_PE_DEPTH_FLOAT  |
                                    SST_PE_DEPTH_WRMASK |
                                    SST_PE_DEPTH_ZQ_SEL);

      pRc->sst.peFbzMode.vFxU32 |= SST_PE_EN_DEPTH_BUFFER;

      if( pRc->zWriteEnable )
          pRc->sst.peFbzMode.vFxU32 |= SST_PE_DEPTH_WRMASK;

      if(!(pRc->state & (STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG)))
      {
          pRc->state            &= ~STATE_REQUIRES_OOZ;
          pRc->sst.suMode.vFxU32 &= ~SST_SU_Z;
      }
      pRc->state            &= ~STATE_REQUIRES_ZBUFFER;
      pRc->state            |= (STATE_REQUIRES_W_FBI | STATE_REQUIRES_WBUFFER);
      pRc->sst.suMode.vFxU32 |= SST_SU_W; //Set flag bit to indicate W is present
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peFbzMode.group | reg3D.suMode.group );
}


void __stdcall rop2(RC *pRc, ULONG state)
{   
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "rop2 %u", state);
  
  pRc->rop2 = state;
  pRc->sst.peFbzMode.vFxU32 &= (SST_PE_LOGIC_OP);

  switch( state )
  {          
    case R2_BLACK:         /* 0 */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_CLEAR<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_NOTMERGEPEN:   /* DPon */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_NOR<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_MASKNOTPEN:    /* DPna */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_AND_REVERSE<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_NOTCOPYPEN:    /* PN */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_COPY_INVERTED<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_MASKPENNOT:    /* PDna */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_AND_INVERTED<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_NOT:           /* Dn */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_INVERT<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_XORPEN:        /* DPx */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_XOR<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_NOTMASKPEN:    /* DPan */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_NAND<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_MASKPEN:       /* DPa */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_AND<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_NOTXORPEN:     /* DPxn */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_EQUIV<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_NOP:           /* D */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_NOOP<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_MERGENOTPEN:   /* DPno */
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_OR_REVERSE<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_COPYPEN:       /* P */ 
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_COPY<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_MERGEPENNOT:   /* PDno */ 
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_OR_INVERTED<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_MERGEPEN:      /* DPo */ 
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_OR<<SST_PE_LOGIC_OP_SHIFT);
      break;

    case R2_WHITE:         /* 1 */ 
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_SET<<SST_PE_LOGIC_OP_SHIFT);
      break;

    default:
      pRc->sst.peFbzMode.vFxU32 |= (SST_PE_LO_COPY<<SST_PE_LOGIC_OP_SHIFT);
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peFbzMode.group );
}

void __stdcall zWriteEnable(RC *pRc, ULONG state)
{    
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zWriteEnable %u", state);
  
  pRc->zWriteEnable = state;

  // hardware will allow writes to the depth buffer
  if (state)
    pRc->sst.peFbzMode.vFxU32 |= SST_PE_DEPTH_WRMASK ;
  else
    pRc->sst.peFbzMode.vFxU32 &= ~SST_PE_DEPTH_WRMASK ;
    
  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peFbzMode.group );
}


void __stdcall zVisible(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "zVisible %u", state);
  
  pRc->zVisible = state;
}

void __stdcall zbias(RC *pRc, ULONG state)
{
  // TODO: Only set Depth bias if Z buffer is 16-bit
  
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zbias %u", state);
  
  pRc->zBias = state;

  pRc->sst.peFbzMode.vFxU32 |= SST_PE_EN_DEPTH_BIAS;

  // multiply the zbias passed by ddraw by an arbitrary number to cause an inflated separation
  // of z bias levels. this was necessary to pass 3D WinBench 2000
#define _ARBITRARY_ZBIAS_SEPARATION 1000;
  pRc->zBias = state * _ARBITRARY_ZBIAS_SEPARATION;

  // If our zFunc is using LESS in the comparison then we need
  // to invert the zBias value to compensate.  
  if( (pRc->zFunc == D3DCMP_LESS) || (pRc->zFunc == D3DCMP_LESSEQUAL) )
    pRc->sst.peSDConst.vFxU32 = (0x1000000 - pRc->zBias ) & 0xFFFFFF;
  else
    pRc->sst.peSDConst.vFxU32 = pRc->zBias;

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peSDConst.group );
}

// initialize the bits to set in the LMS from the lodBias passed in
// the lodBias is a float being passed as an ULONG
ULONG __stdcall calculateLMSFromLODBias(RC *pRc, ULONG uLODBias)
{
  float bias_f;
  float tempLODBias;
 
  // Mipmap LOD Bias is a D3DVLAUE (float), typically in the range 
  // of -1.0 to 1.0. Spec says each unit of +/- 1.0 should bias the
  // the selection by exactly one level. Spec says a postive bias 
  // causes larger levels to be used, a negative bias uses smaller 
  // levels and may result in higher performance. To match the
  // ref rast it's the exact opposite, so we invert the sign of
  // the incoming bias.
  tempLODBias = *(float *)&uLODBias;

  // Rampage LMS Bias is a signed 4.2 number. And we want to account 
  // for any defualt bias we need to set. Multiply by -4.0f so it looks
  // like a 6-bit integer when converted from floating point.
  bias_f = (tempLODBias + pRc->lmsDefaultBias) * -4.0f;

  // Clamp to +/- 7.75 range. Could extend the range by stepping the
  // base lms. 

  if ( bias_f > 31.0f )
     bias_f = 31.0f;
  else if ( bias_f < -31.0f )
     bias_f = -31.0f;

  // for some reason the function ddftol takes -0 and converts it to 0xffffffe which is -2 so 
  // adjust the -0 to 0 here if bias_f is 0
  if( bias_f == -0.00 )
      bias_f = ( float )0.00;

  // return the bits that need to be set in the LMS register
  return( ( ddftol( bias_f ) << SST_TA_LMS_BIAS_SHIFT ) & SST_TA_LMS_BIAS );
}

void __stdcall lodBias(RC *pRc, ULONG state)
{
  D3DPRINT( 0, "lodBias %f", state);

  // Mipmap LOD Bias is a D3DVLAUE (float), typically in the range 
  // of -1.0 to 1.0. Spec says each unit of +/- 1.0 should bias the
  // the selection by exactly one level. Spec says a postive bias 
  // causes larger levels to be used, a negative bias uses smaller 
  // levels and may result in higher performance. To match the
  // ref rast it's the exact opposite, so we invert the sign of
  // the incoming bias.

  pRc->lodBias = *(float *)&state;
  pRc->lmsBias = calculateLMSFromLODBias( pRc, state ); 
}

/*-------------------------------------------------------------------
Function Name:  antiAliasEnable
Description:    Updates the D3DRENDERSTATE to turn on and off AA
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall antiAliasEnable(RC *pRc, ULONG state )
{
  D3DPRINT( RSTATE_DBG_LVL, "antiAliasEnable Renderstate %d", state );
  // mls - just the basic code to get this started

  pRc->antialias = state;
}


//-------------------------------------------------------------------
// STENCIL functions

/*-------------------------------------------------------------------
Function Name:  stencilEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilEnable Renderstate %d", state );

  if (pRc->DDSZHndl == 0)
    state = 0;

  // nothing changed
  if (state == pRc->stencilEnable)
    return;

  pRc->stencilEnable = state;
  if ( pRc->stencilEnable )
      pRc->stencilInUse  = TRUE;    // Used in ddiClear2 to indicate that app is using stencil

  if (state)
     pRc->sst.peStencil.vFxU32 |= SST_PE_EN_STENCIL;
  else
     pRc->sst.peStencil.vFxU32 &= ~SST_PE_EN_STENCIL;

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencil.group );
}


/*-------------------------------------------------------------------
Function Name:  stencilFail
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilFail( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilFail Renderstate %d", state );

  pRc->stencilFail = state;

  // clear the existing stencil fail state
  pRc->sst.peStencilOp.vFxU32 &= ~SST_PE_ST_OP_FAIL;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set Invalid stencil Fail value" );
     case D3DSTENCILOP_KEEP:
      // clearing the field is to set it to keep
      break;
    case D3DSTENCILOP_ZERO:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FZERO;
      break;
    case D3DSTENCILOP_REPLACE:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FREPLACE;
      break;
    case D3DSTENCILOP_INCRSAT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FINCSAT;
      break;
    case D3DSTENCILOP_DECRSAT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FDECSAT;
      break;
    case D3DSTENCILOP_INVERT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FINVERT;
      break;
    case D3DSTENCILOP_INCR:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FINCR;
      break;
    case D3DSTENCILOP_DECR:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_FDECR;
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencilOp.group );
}

/*-------------------------------------------------------------------
Function Name:  stencilZFail
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilZFail( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilZFail Renderstate %d", state );

  pRc->stencilZFail = state;

  // clear the existing stencil fail state
  pRc->sst.peStencilOp.vFxU32 &= ~SST_PE_ST_OP_ZFAIL;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set Invalid stencil Z Fail value" );
     case D3DSTENCILOP_KEEP:
      // clearing the field is to set it to keep
      break;
    case D3DSTENCILOP_ZERO:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFZERO;
      break;
    case D3DSTENCILOP_REPLACE:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFREPLACE;
      break;
    case D3DSTENCILOP_INCRSAT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFINCSAT;
      break;
    case D3DSTENCILOP_DECRSAT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFDECSAT;
      break;
    case D3DSTENCILOP_INVERT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFINVERT;
      break;
    case D3DSTENCILOP_INCR:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFINCR;
      break;
    case D3DSTENCILOP_DECR:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZFDECR;
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencilOp.group );
}

/*-------------------------------------------------------------------
Function Name:  stencilPass
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilPass( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilPass Renderstate %d", state );

  pRc->stencilPass = state;

  // clear the existing stencil fail state
  pRc->sst.peStencilOp.vFxU32 &= ~SST_PE_ST_OP_ZPASS;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set Invalid stencil Z Pass value" );
     case D3DSTENCILOP_KEEP:
      // clearing the field is to set it to keep
      break;
    case D3DSTENCILOP_ZERO:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPZERO;
      break;
    case D3DSTENCILOP_REPLACE:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPREPLACE;
      break;
    case D3DSTENCILOP_INCRSAT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPINCSAT;
      break;
    case D3DSTENCILOP_DECRSAT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPDECSAT;
      break;
    case D3DSTENCILOP_INVERT:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPINVERT;
      break;
    case D3DSTENCILOP_INCR:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPINCR;
      break;
    case D3DSTENCILOP_DECR:
      pRc->sst.peStencilOp.vFxU32 |= SST_PE_ST_ZPDECR;
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencilOp.group );
}

/*-------------------------------------------------------------------
Function Name:  stencilFunc
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilFunc( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilFunc Renderstate %d", state );

  pRc->stencilFunc = state;

  // clear the existing stencil function state
  pRc->sst.peStencil.vFxU32 &= ~SST_PE_ST_FUNC;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set Invalid stencil compare value" );
    case D3DCMP_LESS:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_LT;
      break;
    case D3DCMP_NEVER:
      // clearing the field above is compare never
      break;
    case D3DCMP_EQUAL:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_EQ;
      break;
    case D3DCMP_LESSEQUAL:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_LEQ;
      break;
    case D3DCMP_GREATER:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_GT;
      break;
    case D3DCMP_NOTEQUAL:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_NE;
      break;
    case D3DCMP_GREATEREQUAL:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_GTE;
      break;
    case D3DCMP_ALWAYS:
      pRc->sst.peStencil.vFxU32 |= SST_PE_ST_FUNC_ALWAYS;
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencil.group );
}

/*-------------------------------------------------------------------
Function Name:  stencilRef
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilRef( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilRef Renderstate %d", state );

  pRc->stencilRef = state;

  // clear the existing stencil reference state and set the new one
  pRc->sst.peStencil.vFxU32 &= ~SST_PE_ST_REF;
  pRc->sst.peStencil.vFxU32 |= ((state << SST_PE_ST_REF_SHIFT) & SST_PE_ST_REF);

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencil.group );
}

/*-------------------------------------------------------------------
Function Name:  stencilMask
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilMask( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilMask Renderstate %d", state );

  pRc->stencilMask = state;

  // clear the existing stencil mask state and set the new one
  pRc->sst.peStencil.vFxU32 &= ~SST_PE_ST_MASK;
  pRc->sst.peStencil.vFxU32 |= ((state << SST_PE_ST_MASK_SHIFT) & SST_PE_ST_MASK);

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencil.group );
}

/*-------------------------------------------------------------------
Function Name:  stencilWriteMask
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall stencilWriteMask( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilWriteMask Renderstate %d", state );

  pRc->stencilWriteMask = state;

  // clear the existing stencil mask state and set the new one
  pRc->sst.peStencil.vFxU32 &= ~SST_PE_ST_WRITE_MASK;
  pRc->sst.peStencil.vFxU32 |= ((state << SST_PE_ST_WRITE_MASK_SHIFT) & SST_PE_ST_WRITE_MASK);

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peStencil.group );
}

// end of STENCIL functions
//-------------------------------------------------------------------


//-----------
//
// Textures
//
//-----------

void __stdcall textureHandle(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "textureHandle %u", state);
  
  TS[0].textureHandle = state;
  TS[0].changed = 1;
}

/*-------------------------------------------------------------------
Function Name:  texMag
Description:    Updates the D3DRENDERSTATE. For filtering when maximizing textures.
Information:    (RC *pRc, ULONG state)(RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/

void __stdcall texMag(RC *pRc, ULONG state)
{
  pRc->texMag = state;

  D3DPRINT( RSTATE_DBG_LVL, "texMag %u", state);
  
  switch( state )
  {
    case D3DFILTER_NEAREST:
    case D3DFILTER_MIPNEAREST:
    case D3DFILTER_LINEARMIPNEAREST:
      TS[0].magFilter = D3DTFG_POINT;
      break;
                      
    case D3DFILTER_LINEAR :
    case D3DFILTER_MIPLINEAR:
    case D3DFILTER_LINEARMIPLINEAR:
		TS[0].magFilter = (pRc->anisotropy > 1) ? D3DTFG_ANISOTROPIC : D3DTFG_LINEAR;
      break;
  }

  TS[0].changed = 1;
}

/*-------------------------------------------------------------------
Function Name:  texMin
Description:    Updates the D3DRENDERSTATE. For filtering when minimizing textures.
Information:    (RC *pRc, ULONG state)(RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/

void __stdcall texMin(RC *pRc, ULONG state)
{
  txtrDesc *txtr=0;
  SETUP_PPDEV(pRc);

  D3DPRINT( RSTATE_DBG_LVL, "texMin %u", state);
  
  if( _FX( flags ) & FORCE_BILINEAR )
  {
    if( D3DFILTER_NEAREST == state )
      state = D3DFILTER_LINEAR;
    else if( D3DFILTER_MIPNEAREST == state )
      state = D3DFILTER_MIPLINEAR;
  }
  
  // If the control panel has force trilinear, convert any
  // MIPMAPPING filter state into its corresponding TRILINEAR state
  if( _FX( flags ) & FORCE_TRILINEAR )
  {
    if( D3DFILTER_MIPNEAREST == state )
      state = D3DFILTER_LINEARMIPNEAREST;
    else if( D3DFILTER_MIPLINEAR == state )
      state = D3DFILTER_LINEARMIPLINEAR;
  }

  pRc->texMin = state;

  // again, check for anisotropy here

  switch( state )
  {
    case D3DFILTER_NEAREST:
      TS[0].minFilter = D3DTFN_POINT;
      TS[0].mipFilter = D3DTFP_NONE;
      break;

    case D3DFILTER_LINEAR:
		TS[0].minFilter = (pRc->anisotropy > 1) ? D3DTFN_ANISOTROPIC : D3DTFN_LINEAR;
		TS[0].mipFilter = D3DTFP_NONE;
		break;

    case D3DFILTER_MIPNEAREST:
      TS[0].minFilter = D3DTFN_POINT;
      TS[0].mipFilter = D3DTFP_POINT;
      break;

    case D3DFILTER_MIPLINEAR:
		TS[0].minFilter = (pRc->anisotropy > 1) ? D3DTFN_ANISOTROPIC : D3DTFN_LINEAR;
		TS[0].mipFilter = D3DTFP_POINT;
		break;

    case D3DFILTER_LINEARMIPNEAREST:
      TS[0].minFilter = D3DTFN_POINT;
      TS[0].mipFilter = D3DTFP_LINEAR;
      break;
      
    case D3DFILTER_LINEARMIPLINEAR:
		TS[0].minFilter = (pRc->anisotropy > 1) ? D3DTFN_ANISOTROPIC : D3DTFN_LINEAR;
		TS[0].mipFilter = D3DTFP_LINEAR;
		break;
  }
  
  TS[0].changed = 1;
}

void __stdcall anisotropy(RC *pRc, ULONG state)
{
    D3DPRINT( RSTATE_DBG_LVL, "anisotropy %u", state);

	pRc->anisotropy = state;
}

void __stdcall texturePerspective(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "texturePerspective %u", state);
  
  pRc->texturePerspective = state;

  if( state )
  {
    // pRc->sst.TMU[0].taMode.vFxU32 |= SST_TPERSP_ST;
    pRc->state             |= STATE_REQUIRES_PERSPECTIVE;
  }
  else
  {
    // pRc->sst.TMU[0].taMode.vFxU32 &= ~SST_TPERSP_ST;
    pRc->state             &= ~STATE_REQUIRES_PERSPECTIVE;
  }

}


//-------------------------------------------------------------------

void __stdcall textureFactor(RC *pRc, ULONG state)
{
  DWORD  a,r,g,b;

  D3DPRINT( RSTATE_DBG_LVL, "textureFactor %u", state);
  
  pRc->textureFactor = state;

  // Store current texture factor in format useable by C0

  // Scale d3d color by 1/256 and map 255/256 to 256/256.
  r = RGBA_GETRED( pRc->textureFactor);
  a = RGBA_GETALPHA( pRc->textureFactor);
  pRc->textureFactorAR = setColorConstant(  
    ( a == 255 ? 1.0F : a/256.0F),
    ( r == 255 ? 1.0F : r/256.0F));

  b = RGBA_GETBLUE( pRc->textureFactor);
  g = RGBA_GETGREEN( pRc->textureFactor);
  pRc->textureFactorGB = setColorConstant(  
    ( g == 255 ? 1.0F : g/256.0F),
    ( b == 255 ? 1.0F : b/256.0F));
}

//-------------------------------------------------------------------
void __stdcall wrap0(RC *pRc, ULONG state)
{
  ULONG dwIndex;

  D3DPRINT( RSTATE_DBG_LVL, "wrap0 %u", state);

  for( dwIndex = 0x00; dwIndex < D3DHAL_TSS_MAXSTAGES; dwIndex++ )
  {
    if( pRc->wrap[dwIndex] != state )
    {
        pRc->wrap[dwIndex] = state;
        TS[dwIndex].changed = 1;
    } 
  } 
}

//-------------------------------------------------------------------

void __stdcall wrap1(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap1 %u", state);

  if( pRc->wrap[1] != state )
  {
    pRc->wrap[1] = state;
    TS[1].changed = 1;
  }
}

//-------------------------------------------------------------------

void __stdcall wrap2(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap2 %u", state);
  
  if( pRc->wrap[2] != state )
  {
    pRc->wrap[2] = state;
    TS[2].changed = 1;
  }
}
//-------------------------------------------------------------------

void __stdcall wrap3(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap3 %u", state);

  if( pRc->wrap[3] != state )
  {
    pRc->wrap[3] = state;
    TS[3].changed = 1;
  }
}
//-------------------------------------------------------------------

void __stdcall wrap4(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap4 %u", state);

  if( pRc->wrap[4] != state )
  {
    pRc->wrap[4] = state;
    TS[4].changed = 1;
  }
}
//-------------------------------------------------------------------

void __stdcall wrap5(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap5 %u", state);

  if( pRc->wrap[5] != state )
  {
    pRc->wrap[5] = state;
    TS[5].changed = 1;
  }
}
//-------------------------------------------------------------------

void __stdcall wrap6(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap6 %u", state);

  if( pRc->wrap[6] != state )
  {
    pRc->wrap[6] = state;
    TS[6].changed = 1;
  }
}
//-------------------------------------------------------------------

void __stdcall wrap7(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap7 %u", state);

  if( pRc->wrap[7] != state )
  {
    pRc->wrap[7] = state;
    TS[7].changed = 1;
  }
}

//-------------------------------------------------------------------

void __stdcall wrapU(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrapu %u", state);

  pRc->wrapU = state;

  if( state )
  {
    pRc->state |= STATE_REQUIRES_WRAP;
    pRc->bRecalcRndrFn = TRUE;
    pRc->wrap[0] |= D3DWRAP_U;
    TS[0].changed = 1;
  } 
  else if( !pRc->wrapV )
  {
    pRc->state &= ~STATE_REQUIRES_WRAP;
    pRc->bRecalcRndrFn = TRUE;
    pRc->wrap[0] &= ~D3DWRAP_U;
    TS[0].changed = 1;
  }
}


void __stdcall wrapV(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrapv %u", state);

  pRc->wrapV = state;
  
  if( state )
  { 
    pRc->state |= STATE_REQUIRES_WRAP;
    pRc->bRecalcRndrFn = TRUE;
    pRc->wrap[0] |= D3DWRAP_V;
    TS[0].changed = 1;
  }
  else if( !pRc->wrapU )
  {
    pRc->state &= ~STATE_REQUIRES_WRAP;
    pRc->bRecalcRndrFn = TRUE;
    pRc->wrap[0] &= ~D3DWRAP_V;
    TS[0].changed = 1;
  }
}


// DirectX 5.0 - users can now set U and V control separately as defined
// below. The problem is that the textureAddress state can be opposite of
// the addressU and addressV and incorrect. Users should be careful mixing
// and matching the three renderstates.

void __stdcall textureAddress(RC *pRc, ULONG state) 
{
  D3DPRINT( RSTATE_DBG_LVL, "textureAddress %u", state);

  pRc->textureAddress = state;

  textureAddressU (pRc, state);
  textureAddressV (pRc, state);
} // End textureAddress


void __stdcall textureAddressU(RC *pRc, ULONG state) 
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "textureAddressU %u", state);

  pRc->textureAddressU = state;

  TS[0].addressU = state;
  TS[0].changed = 1;

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.TMU[0].taMode.group );
} // End textureAddressU


void __stdcall textureAddressV(RC *pRc, ULONG state) 
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "textureAddressV %u", state);

  pRc->textureAddressV = state;

  TS[0].addressV = state;
  TS[0].changed = 1;

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.TMU[0].taMode.group );
} // End textureAddressV


void __stdcall texMapBlend(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "texMapBlend %u", state);

  pRc->texMapBlend = state;
  switch( state )
  {
    case D3DTBLEND_DECALALPHA:
    
      TS[0].colorOp   = D3DTOP_BLENDTEXTUREALPHA;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_SELECTARG2;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;      
      // pRc->sst.fbzMode.vFxU32 &= ~SST_ENALPHAMASK ;
      break;

    case D3DTBLEND_MODULATEALPHA:
    
      TS[0].colorOp   = D3DTOP_MODULATE;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_MODULATE;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;      
      // pRc->sst.fbzMode.vFxU32 &= ~SST_ENALPHAMASK ;
      break;

    case D3DTBLEND_COPY:
    case D3DTBLEND_DECAL:
    
      TS[0].colorOp   = D3DTOP_SELECTARG1;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].alphaOp   = D3DTOP_SELECTARG1;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      pRc->texMapBlend = 0x7ffffffe; // internal legacy setting for D3DTBLEND_MODULATE used in setupTexturing()
      // pRc->sst.fbzMode.vFxU32 &= ~SST_ENALPHAMASK;
      break;

    case D3DTBLEND_MODULATE:
    
      TS[0].colorOp   = D3DTOP_MODULATE;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_SELECTARG1;
      TS[0].alphaArg1 = D3DTA_TEXTURE;      
      // pRc->sst.fbzMode.vFxU32 &= ~SST_ENALPHAMASK;
      break;

    case D3DTBLEND_DECALMASK:
    
      TS[0].colorOp   = D3DTOP_SELECTARG1;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].alphaOp   = D3DTOP_SELECTARG1;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      // pRc->sst.fbzMode.vFxU32 |= SST_ENALPHAMASK;
      break;

    case D3DTBLEND_MODULATEMASK:
    
      TS[0].colorOp   = D3DTOP_MODULATE;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_MODULATE;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;      
      // pRc->sst.fbzMode.vFxU32 |= SST_ENALPHAMASK;
      break;

    case D3DTBLEND_ADD:
    
      TS[0].colorOp   = D3DTOP_ADD;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_SELECTARG2;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;      
      // pRc->sst.fbzMode.vFxU32 &= ~SST_ENALPHAMASK;
      break;      
  }
  
  TS[0].changed = 1;
}


//------------------------------
//
// Alpha Comparison and Blending
//
//------------------------------
void __stdcall blendEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "blendEnable %u", state);

  pRc->blendEnable = state;

  // Treat BlendEnable as alpha enable only
  // for backwards compatiblity with DX 3.0 apps
  alphaBlendEnable (pRc, state);

} // End blendEnable


void __stdcall alphaBlendEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "alphaBlendEnable %u", state);

  pRc->alphaBlendEnable = state;

  // no alpha blend or alpha test (clear it first)
  if (!pRc->alphaTestEnable)
    pRc->state &= ~STATE_REQUIRES_IT_ALPHA;

  // Don't enable or disable suParamMask's SST_SU_A0
  // here!  It's not what controls the usage of Alpha,
  // so don't change it.  The primitive drawing routines
  // use packed color and thus require this flag.
  if (state)
  {
    pRc->sst.peAlphaMode.vFxU32 |= SST_PE_EN_ALPHA_BLEND;
    
    pRc->state |= STATE_REQUIRES_IT_ALPHA;
  }
  else
  {
    pRc->sst.peAlphaMode.vFxU32 &= ~SST_PE_EN_ALPHA_BLEND;
  }

  if (pRc->fogEnable)
    setFogMode(pRc);

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peAlphaMode.group );
}


// in DX 5.0 colorkey control and alpha blendenable are controlled via
// separate renderstates.
void __stdcall colorKeyEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "colorkeyenable %u", state);

  pRc->colorKeyEnable = state;
} 


//---------------------------------------------------------
//  Alpha blend:
//
//  Cout = ( Cpix * srcBlend) + (Cfb * dstBlend)
//  where Cpix is the pixel coming out of the pipeline and
//        Cfb  is the pixel in the frame buffer 
//
void __stdcall srcBlend(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "srcblend %u", state);

  pRc->srcBlend = state ;   
    
  // clear the existing Blend mode
  pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_SRC_FACT);

  // TODO: Dest Alpha, InvDest Alpha support in 32bpp mode

  switch (state)
  {
    default : 
      D3DPRINT(1,"Warning: trying to set invalid source blend value") ;

    case  D3DBLEND_ZERO         :  // (0,0,0,0)                
      // clearing the existing Blend mode is blend zero
      break;

    case  D3DBLEND_ONE          :  // (1,1,1,1)              
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      

    case  D3DBLEND_SRCCOLOR     :  // (Rpix, Gpix, Bpix, Apix)     
      // Ref rasterizer does Csrc*Csrc 
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_SAME_COLOR << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_INVSRCCOLOR  :  // (1-Rpix, 1-Gpix, 1-Bpix, 1-Apix)     
      // Ref rasterizer does Csrc*1-Csrc 
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_SAME_COLOR << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_SRCALPHA     :  // (Apix, Apix, Apix, Apix)
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_SRC_ALPHA << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_INVSRCALPHA  :  // (1-Apix, 1-Apix, 1-Apix, 1-Apix)
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_SRC_ALPHA << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_DESTALPHA    : //  (Afb, Afb, Afb, Afb
      // only if there is an alpha buffer
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_DST_ALPHA << SST_PE_RGB_SRC_FACT_SHIFT) |
                               (SST_PE_ABLEND_DST_ALPHA << SST_PE_A_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_INVDESTALPHA : //  (1-Afb, 1-Afb, 1-Afb, 1-Afb)
      // only if there is an alpha buffer
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_DST_ALPHA << SST_PE_RGB_SRC_FACT_SHIFT) |
                              (SST_PE_ABLEND_ONE_MINUS_DST_ALPHA << SST_PE_A_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_DESTCOLOR    : //  (Rfb, Gfb, Bfb, Afb)
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_OTHER_COLOR << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_INVDESTCOLOR : //  (1-Rb, 1-Gfb, 1-Bfb, 1-Afb)
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_OTHER_COLOR << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_SRCALPHASAT :  //  f=min(Apix,1-Afb) (f, f, f, 1)
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_SATURATE << SST_PE_RGB_SRC_FACT_SHIFT);
      break;
      
    case  D3DBLEND_BOTHSRCALPHA: // srcBlend = (Apix, Apix, Apix, Apix)
                                 // dstBlend = (1-Apix, 1-Apix, 1-Apix, 1-Apix)
      pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_SRC_FACT | SST_PE_RGB_DST_FACT);
      pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_SRC_ALPHA   << SST_PE_RGB_SRC_FACT_SHIFT) 
                                 |  (SST_PE_ABLEND_ONE_MINUS_SRC_ALPHA << SST_PE_RGB_DST_FACT_SHIFT) ;
      break;
      
    case  D3DBLEND_BOTHINVSRCALPHA : // srcBlend = (1-Apix, 1-Apix, 1-Apix, 1-Apix) 
                                     // dstblend = (Apix, Apix, Apix, Apix)
      pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_DST_FACT | SST_PE_A_DST_FACT);
      pRc->sst.peAlphaMode.vFxU32 |=  (SST_PE_ABLEND_ONE_MINUS_SRC_ALPHA << SST_PE_RGB_SRC_FACT_SHIFT)          
                                 |   (SST_PE_ABLEND_SRC_ALPHA   << SST_PE_RGB_DST_FACT_SHIFT) ;
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peAlphaMode.group );
}


void __stdcall dstBlend(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "dstblend %u", state);

  pRc->dstBlend = state ;
   
  // if application has set SRC blend to BOTHxx then it overrides destination
  // clear the existing Blend mode
  pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_DST_FACT);

  switch(state)
  {
    default:      
      D3DPRINT( 255,"Warning: trying to set invalid destination blend value" );
    case  D3DBLEND_ZERO          :   // (0,0,0,0)   
      // clearing the existing Blend mode is blend zero
      break;
      
     case  D3DBLEND_ONE           :   // (1,1,1,1)
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
           
     case  D3DBLEND_SRCCOLOR     :    // (Rpix, Gpix, Bpix, Apix) 
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_OTHER_COLOR << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_INVSRCCOLOR :     // (1-Rpix, 1-Gpix, 1-Bpix, 1-Apix)
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_OTHER_COLOR << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_SRCALPHA     :    // (Apix, Apix, Apix, Apix) 
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_SRC_ALPHA << SST_PE_RGB_DST_FACT_SHIFT);
       break;
       
     case  D3DBLEND_INVSRCALPHA :     // (1-Apix, 1-Apix, 1-Apix, 1-Apix)
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_SRC_ALPHA << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_DESTALPHA     :   // (Afb, Afb, Afb, Afb
       // only if there is an alpha buffer
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_DST_ALPHA << SST_PE_RGB_DST_FACT_SHIFT) |
                                     (SST_PE_ABLEND_DST_ALPHA << SST_PE_A_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_INVDESTALPHA :    // (1-Afb, 1-Afb, 1-Afb, 1-Afb)
       // only if there is an alpha buffer
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_DST_ALPHA << SST_PE_RGB_DST_FACT_SHIFT) |
                                     (SST_PE_ABLEND_ONE_MINUS_DST_ALPHA << SST_PE_A_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_DESTCOLOR    :   //  (Rfb, Gfb, Bfb, Afb)
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_SAME_COLOR << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_INVDESTCOLOR : //  (1-Rb, 1-Gfb, 1-Bfb, 1-Afb)
       pRc->sst.peAlphaMode.vFxU32 |= (SST_PE_ABLEND_ONE_MINUS_SAME_COLOR << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
       
     case  D3DBLEND_SRCALPHASAT  :   //  f=min(Apix,1-Afb) (f, f, f, 1)
       // Ref raterizer use r=g=b= min(SrcColor.A, ~DstColor.A )
       // Could turn this into a SRC factor?
       break;

     case  D3DBLEND_BOTHSRCALPHA :   // see srcBlend
       // Ref Rasterizer doesn't support this
       // pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_DST_FACT);
       // pRc->sst.peAlphaMode.vFxU32 |=  (SST_PE_ABLEND_SRC_ALPHA   << SST_PE_RGB_SRC_FACT_SHIFT) 
       //                              | (SST_PE_ABLEND_ONE_MINUS_SRC_ALPHA << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
     case  D3DBLEND_BOTHINVSRCALPHA : // see srcBlend 
       // Ref Rasterizer doesn't support this
       // pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_RGB_DST_FACT);
       // pRc->sst.peAlphaMode.vFxU32 |=  (SST_PE_ABLEND_ONE_MINUS_SRC_ALPHA << SST_PE_RGB_SRC_FACT_SHIFT)          
       //                              | (SST_PE_ABLEND_SRC_ALPHA   << SST_PE_RGB_DST_FACT_SHIFT) ;
       break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peAlphaMode.group );
}


void __stdcall alphaTestEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "alphatestenable %u", state);

  pRc->alphaTestEnable = state;

  // TODO: Alpha Func for depth buffer
      
  // Don't enable or disable suParamMask's SST_SU_A0
  // here!  It's not what controls the usage of Alpha,
  // so don't change it.  The primitive drawing routines
  // use packed color and thus require this flag.
  if (state)
  {
    pRc->sst.peAlphaTest.vFxU32 |= ( SST_PE_EN_ALPHA_CFUNC | SST_PE_EN_ALPHA_DFUNC );
    pRc->state |= STATE_REQUIRES_IT_ALPHA;
  }
  else 
  {
    pRc->sst.peAlphaTest.vFxU32 &= ~( SST_PE_EN_ALPHA_CFUNC | SST_PE_EN_ALPHA_DFUNC ); 

    // no alpha blending or alpha test
    if (!pRc->alphaBlendEnable)
    {
      pRc->state &= ~STATE_REQUIRES_IT_ALPHA;
    }
  }
  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peAlphaTest.group );
}

void __stdcall alphaRef(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "alphaRef %u", state);

  // D3D alpha ranges from 0 to 255 which is the same as our hardware

  pRc->alphaRef       = state;
  pRc->sst.peAlphaTest.vFxU32 &= ~( SST_PE_ALPHA_CREF | SST_PE_ALPHA_DREF );
  pRc->sst.peAlphaTest.vFxU32 |= ((state & 0xFF) << SST_PE_ALPHA_CREF_SHIFT) ;
  pRc->sst.peAlphaTest.vFxU32 |= ((state & 0xFF) << SST_PE_ALPHA_DREF_SHIFT) ;

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peAlphaTest.group );
}


void __stdcall alphaFunc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "alphaFunc %u", state);

  // Nothing changed
  if(state == pRc->alphaFunc)

  // TODO: Alpha Func for depth buffer
    
  pRc->alphaFunc = state;

  // clear the existing alpha comparison state  
  pRc->sst.peAlphaTest.vFxU32 &= ~(SST_PE_ALPHA_CFUNC | SST_PE_ALPHA_DFUNC);

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set invalid alpha comparison value" );
    case D3DCMP_LESS:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_LT | SST_PE_ALPHA_DFUNC_LT);
      break;
      
    case D3DCMP_NEVER:
      // clearing the field above is compare never
      break;
      
    case D3DCMP_EQUAL:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_EQ | SST_PE_ALPHA_DFUNC_EQ);
      break;
      
    case D3DCMP_LESSEQUAL:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_LT | SST_PE_ALPHA_CFUNC_EQ | SST_PE_ALPHA_DFUNC_LT | SST_PE_ALPHA_DFUNC_EQ);
      break;
      
    case D3DCMP_GREATER:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_GT | SST_PE_ALPHA_DFUNC_GT);
      break;
      
    case D3DCMP_NOTEQUAL:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_LT | SST_PE_ALPHA_CFUNC_GT | SST_PE_ALPHA_DFUNC_LT | SST_PE_ALPHA_DFUNC_GT);
      break;
      
    case D3DCMP_GREATEREQUAL:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_GT | SST_PE_ALPHA_CFUNC_EQ | SST_PE_ALPHA_DFUNC_GT | SST_PE_ALPHA_DFUNC_EQ);
      break;
      
    case D3DCMP_ALWAYS:
      pRc->sst.peAlphaTest.vFxU32 |= (SST_PE_ALPHA_CFUNC_LT | SST_PE_ALPHA_CFUNC_EQ | SST_PE_ALPHA_CFUNC_GT |
                               SST_PE_ALPHA_DFUNC_LT | SST_PE_ALPHA_DFUNC_EQ | SST_PE_ALPHA_DFUNC_GT);
      break;
  }

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peAlphaTest.group );
}

 
//-------------------
//
//  Misc.
//
//-------------------

void __stdcall cullMode(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "cullmode %u", state);

  pRc->cullMode = state;

  switch(state)
  {
    case D3DCULL_NONE:
      pRc->cullMask = 0xFFFFFFFF;
      pRc->sst.suMode.vFxU32 &= ~(SST_SU_EN_CULLING | SST_SU_CULL_NEGATIVE);
      break;
      
    case D3DCULL_CW:
      pRc->cullMask = 0x80000000;
      pRc->sst.suMode.vFxU32 = (pRc->sst.suMode.vFxU32 & ~SST_SU_CULL_NEGATIVE) | SST_SU_EN_CULLING;
      break;
      
    default:
      pRc->cullMask = 0x00000000;
      pRc->sst.suMode.vFxU32 |= SST_SU_EN_CULLING | SST_SU_CULL_NEGATIVE;
      break;
  }
  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.suMode.group );
}


void __stdcall ditherEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "ditherEnable %u", state);

  // State is True to enable dither  
  pRc->ditherEnable = state;
  
  // D3D won't let you chose the dither matrix size so hardcode something
  if (state)
    pRc->sst.peFbzMode.vFxU32 |=  (SST_PE_EN_DITHER)  ;
  else
    pRc->sst.peFbzMode.vFxU32 &= ~(SST_PE_EN_DITHER)  ;

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.peFbzMode.group );
}

void __stdcall shadeMode(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "shadeMode %u", state);

  pRc->shadeMode = state;
  
  if(state == D3DSHADE_FLAT)
    pRc->state = (pRc->state & ~STATE_REQUIRES_IT_RGB) | STATE_REQUIRES_RGB;
  else
    pRc->state = (pRc->state & ~STATE_REQUIRES_RGB) | STATE_REQUIRES_IT_RGB;

  pRc->bRecalcRndrFn = TRUE;
}

void __stdcall linePattern(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "linePattern %u", state);

  // TODO: turn on CAP bits if we want to support line pattern
  // TODO: note that 1 cap bit specifies solid and line pattern, so must support both if either
  // upper 16bits is repeat factor, lower is pattern
  if (state && ((state & 0xffff0000) != 0xffff0000))
  {
      // pattern is not solid, so enable linepats in raControl
      pRc->sst.raControl.vFxU32 |= SST_RA_EN_STIPPLE;
      // set the repeat factor - note that we only support 8 bit repeat factor
      pRc->sst.raControl.vFxU32 |= (state & SST_RA_STIPPLE_REPEAT) << SST_RA_STIPPLE_REPEAT_SHIFT;
  }
  else
  {
      // pattern is solid, so turn it off - repeat pattern is don't care
      pRc->sst.raControl.vFxU32 &= ~SST_RA_EN_STIPPLE;
  }

  pRc->sst.raStipple.vFxU32 = (state & 0xffff0000) | ((state>>16) & 0xffff);

  // Update state change for shadows changed in thie routine
  UPDATE_HW_STATE( reg3D.raControl.group | reg3D.raStipple.group);
}


void __stdcall fillMode(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fillMode %u", state);

  pRc->fillMode = state ;

  pRc->bRecalcRndrFn = TRUE;
}

void __stdcall specular(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "specular %u", state);

  // nothing has changed
  if ((state && (pRc->state & STATE_REQUIRES_SPECULAR)) ||
      (!state && !(pRc->state & STATE_REQUIRES_SPECULAR)))
    return;

  pRc->specular = 0;                    // Gets fully set in d6mt.c
    
  if (state) 
    pRc->state |= STATE_REQUIRES_SPECULAR;
  else
    pRc->state &= ~STATE_REQUIRES_SPECULAR;

  pRc->bRecalcRndrFn = TRUE;
}

void __stdcall subPixel(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "subpixel %u", state);

  pRc->subPixel = state;
}

/*-------------------------------------------------------------------
Function Name:  sceneCapture
Description:    This state passes TRUE or FALSE to replace the
                functionality of D3DHALCallbacks->SceneCapture(),
Information:    (RC *pRc, ULONG state)
Return:         RENDERFXN_OK
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall sceneCapture(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)

  D3DPRINT(RSTATE_DBG_LVL, "sceneCapture Renderstate %d", state);

  if (FALSE == state)
  {
    // Processing an EndScene ...
    D3DPRINT(RSTATE_DBG_LVL, "  EndScene");
    _D3(flags) &= ~IN_RENDER_SCENE;
    
#ifdef CUBEMAP
    if (pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_CUBEMAP)
    {
       // Assume we're done rendering to a cubemap texture. Mark it dirty
       // so we can do a copy/translate when the texture handle is 
       // referenced for rendering. 

       pRc->lpSurfData->pTxtrDesc->txDirtyBits =
            pRc->lpSurfData->ddsDwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES;
    }
#endif

  }
  else
  {
    // Processing a BeginScene ...
    D3DPRINT(RSTATE_DBG_LVL, "  BeginScene");
    _D3(flags) |= IN_RENDER_SCENE;
  }

  // mls - 8/15/00 With the new state management code we found that we needed to do 
  // this at least once per frame to ensure
  // that FSAA has the correct addresses for the color buffer.
  // Otherwise every other frame rendered would be blank.
  UPDATE_HW_STATE( reg3D.peColBufferAddr.group | reg3D.peAuxBufferAddr.group | reg3D.peBufferSize.group);

  RENDERFXN_OK;
}


#if (DIRECT3D_VERSION >= 0x0800)
void __stdcall swVertexProcessing ( RC *pRc, ULONG state )
{
}

void __stdcall pointSize ( RC *pRc, ULONG state )
{
    float size = *FLTP(&state);

    D3DPRINT( RSTATE_DBG_LVL, "pointSize %f", size);

    pRc->pointSize = size;
}

void __stdcall pointSizeMax ( RC *pRc, ULONG state )
{
    float size = *FLTP(&state);

    D3DPRINT( RSTATE_DBG_LVL, "pointSizeMax %f", size);

    pRc->pointSizeMax = size;
}

void __stdcall pointSizeMin ( RC *pRc, ULONG state )
{
    float size = *FLTP(&state);

    D3DPRINT( RSTATE_DBG_LVL, "pointSizeMin %f", size);

    pRc->pointSizeMin = size;
}

void __stdcall pointSpriteEnable ( RC *pRc, ULONG state )
{
    D3DPRINT( RSTATE_DBG_LVL, "pointSpriteEnable %u", state);

    pRc->pointSpriteEnable = state;
}

void __stdcall pointScaleEnable ( RC *pRc, ULONG state )
{
    D3DPRINT( RSTATE_DBG_LVL, "pointScaleEnable %u", state);

    pRc->pointScaleEnable = state;
}

void __stdcall pointScaleA ( RC *pRc, ULONG state )
{
    float size = *FLTP(&state);

    D3DPRINT( RSTATE_DBG_LVL, "pointScaleA %f", size);

    pRc->pointScaleA = size;
}

void __stdcall pointScaleB ( RC *pRc, ULONG state )
{
    float size = *FLTP(&state);

    D3DPRINT( RSTATE_DBG_LVL, "pointScaleB %f", size);

    pRc->pointScaleB = size;
}

void __stdcall pointScaleC ( RC *pRc, ULONG state )
{
    float size = *FLTP(&state);

    D3DPRINT( RSTATE_DBG_LVL, "pointScaleC %f", size);

    pRc->pointScaleC = size;
}

void __stdcall multiSampleAA ( RC *pRc, ULONG state )
{
    D3DPRINT( RSTATE_DBG_LVL, "Multi-Sample AA Renderstate %d", state );

    pRc->multiSampleAA = (BOOL) state;
}

void __stdcall multiSampleMask ( RC *pRc, ULONG state )
{
    FxU32 exMask;

    SETUP_PPDEV(pRc)
    D3DPRINT( RSTATE_DBG_LVL, "Multi-Sample Mask Renderstate  %d", state );

    pRc->multiSampleMask = state;

	// ExMask defaults to 0 - i.e. don't exclude any samples
    exMask = 0; 

    // check accumulation buffer mask to see if app wants multisample effect ( blur, focus etc...)
    if( state != 0xffffffff )  // d3d sets this to all 1's by default
    {
        //if bit 0 = 0, exclude upper left subsamples
        if (!(state & 0x01))
              exMask |= (1<<0) | (1<<5) | (1<<10) | (1<<15); 
  
        //if bit 1 = 0, exclude upper right subsamples
        if (!(state & 0x02))
              exMask |= (1<<1) | (1<<6) | (1<<11) | (1<<16); 

        //if bit 2 = 0, exclude lower left subsamples
        if (!(state & 0x04))
              exMask |= (1<<2) | (1<<7) | (1<<12) | (1<<17); 

        //if bit 3 = 0, exclude lower right subsamples
        if (!(state & 0x08))
              exMask |= (1<<3) | (1<<8) | (1<<13) | (1<<18); 
    }

	pRc->sst.peExMask.vFxU32 = exMask;

    
	UPDATE_HW_STATE( reg3D.peExMask.group );
}

void __stdcall patchEdgeStyle ( RC *pRc, ULONG state )
{
}

void __stdcall patchSegments ( RC *pRc, ULONG state )
{
}

void __stdcall debugMonitorToken ( RC *pRc, ULONG state )
{
}

void __stdcall indexedVertexBlendEnable ( RC *pRc, ULONG state )
{
}


void __stdcall colorWriteEnable ( RC *pRc, ULONG state ) 
{
    SETUP_PPDEV(pRc)
    D3DPRINT( RSTATE_DBG_LVL, "colorWriteEnable %d", state);

    pRc->colorWriteEnable = state;  

	pRc->sst.peFbzMode.vFxU32 &= ~SST_PE_ARGB_WRMASK;  // clear all 4 bits in shadow reg

	// check the 4 d3d color mask bits and translate them
	// to rampage peFbzMode mask bits.
	if( state & D3DCOLORWRITEENABLE_ALPHA )
    {
		 pRc->sst.peFbzMode.vFxU32 |= SST_PE_A_WRMASK;
	}
	if( state & D3DCOLORWRITEENABLE_RED )
    {
		 pRc->sst.peFbzMode.vFxU32 |= SST_PE_R_WRMASK;
	}
	if( state & D3DCOLORWRITEENABLE_GREEN )
    {
		 pRc->sst.peFbzMode.vFxU32 |= SST_PE_G_WRMASK;
	}
	if( state & D3DCOLORWRITEENABLE_BLUE )
    {
		 pRc->sst.peFbzMode.vFxU32 |= SST_PE_B_WRMASK;
	}

    // Update state change for shadows changed in the routine
    UPDATE_HW_STATE( reg3D.peFbzMode.group );

}

void __stdcall tweenFactor ( RC *pRc, ULONG state )
{
}

void __stdcall blendOp ( RC *pRc, ULONG state )
{
    SETUP_PPDEV(pRc)
    D3DPRINT( RSTATE_DBG_LVL, "blendOp %d", state);

    pRc->blendOp = state;
    pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_BLEND_ASEL << SST_PE_BLEND_ASEL_SHIFT);
    pRc->sst.peAlphaMode.vFxU32 &= ~(SST_PE_BLEND_CSEL << SST_PE_BLEND_CSEL_SHIFT);

    switch (state)
    {
        case D3DBLENDOP_ADD :
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_ADD    << SST_PE_BLEND_ASEL_SHIFT;
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_ADD    << SST_PE_BLEND_CSEL_SHIFT;
            break;
        case D3DBLENDOP_SUBTRACT :
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_SUB    << SST_PE_BLEND_ASEL_SHIFT;
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_SUB    << SST_PE_BLEND_CSEL_SHIFT;
            break;
        case D3DBLENDOP_REVSUBTRACT :
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_REVSUB << SST_PE_BLEND_ASEL_SHIFT;
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_REVSUB << SST_PE_BLEND_CSEL_SHIFT;
            break;
        case D3DBLENDOP_MIN :
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_CMIN   << SST_PE_BLEND_ASEL_SHIFT;
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_CMIN   << SST_PE_BLEND_CSEL_SHIFT;
            break;
        case D3DBLENDOP_MAX :
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_CMAX   << SST_PE_BLEND_ASEL_SHIFT;
            pRc->sst.peAlphaMode.vFxU32 |= SST_PE_ABLEND_CMAX   << SST_PE_BLEND_CSEL_SHIFT;
            break;
        default :
            D3DPRINT( 255,"Warning: trying to set invalid D3DBLENDOP" );
    }
    UPDATE_HW_STATE( reg3D.peAlphaMode.group );
}

#endif // end DX8 renderstates

extern void __stdcall init3Dregs(RC *pRc);
extern BOOLEAN bNeedClear3DInitialization;  // From ddiClear2, used by newInitRC

//-------------------------------------------------------
//
//  This is the default setting for the rendering context
//
//-------------------------------------------------------
void __stdcall initNewRC(RC *pRc)
{
  DWORD  a,r,g,b;

  SETUP_PPDEV(pRc)

  // To insure a known starting condition for this context, initialize all
  // 3d hw registers and write out that state to hardware. This per
  // context initilization may be trimmed by moving the static register 
  // sets into halCreateDriver. Cooperation with other 3d hw clients may
  // require a certain registers to be left alone or save/restored.

  init3Dregs( pRc );

  // Make sure ddiClear2 initializes the 3D hardware for a clear because in some cases ddiClear
  // is called before setDX6state is called. 
  bNeedClear3DInitialization = TRUE;

  // Renderstate changes above will update the shadow copy of
  // the hw registers and will be written to hardware when needed.

  pRc->shadeMode          = D3DSHADE_GOURAUD;
  pRc->rop2               = R2_COPYPEN;
  pRc->zWriteEnable       = FALSE;
  pRc->zEnable            = FALSE;
  pRc->zFunc              = D3DCMP_NEVER;
  pRc->zBias              = 0;
  pRc->lodBias            = 0.0f;
  pRc->lmsDefaultBias     = 0.0f;  // driver's default lms bias
  pRc->lmsBias            = 0;
  pRc->textureAddress     = D3DTADDRESS_WRAP;
  pRc->textureAddressU    = D3DTADDRESS_WRAP;
  pRc->textureAddressV    = D3DTADDRESS_WRAP;
  pRc->texMag             = D3DFILTER_NEAREST;
  pRc->texMin             = D3DFILTER_NEAREST;
  pRc->texturePerspective = FALSE;
  pRc->textureFactor      = 0x80FFFFFF;
  pRc->blendEnable        = FALSE;
  pRc->colorKeyEnable     = TRUE;
  pRc->alphaBlendEnable   = FALSE;
  pRc->alphaTestEnable    = FALSE;
  pRc->srcBlend           = D3DBLEND_ONE; 
  pRc->dstBlend           = D3DBLEND_ZERO;
  pRc->alphaRef           = 0;
  pRc->alphaFunc          = D3DCMP_NEVER;
  pRc->fogEnable          = FALSE;
  pRc->fogColor           = 0;
  pRc->cullMode           = D3DCULL_NONE;
  pRc->cullMask           = 0xFFFFFFFF;       // Cull Mask (CCW)
  pRc->ditherEnable       = FALSE;
  pRc->wrapU              = FALSE;
  pRc->wrapV              = FALSE;
  pRc->texMapBlend        = D3DTBLEND_MODULATE;
  pRc->subPixel           = TRUE;
  pRc->specular           = 0;
  pRc->fogTableMode       = D3DFOG_NONE;
  pRc->fogDensity         = 1.0f;
  pRc->fogTableStart      = 0.0f;
  pRc->fogTableEnd        = 1.0f;
  pRc->ditherEnable       = FALSE;  
  
  pRc->stencilEnable      = 0;
  pRc->stencilFail        = 0;
  pRc->stencilZFail       = 0;
  pRc->stencilPass        = 0;
  pRc->stencilRef         = 0;
  pRc->stencilFunc        = 0;
  pRc->stencilMask        = 0xFFFFFFFF;
  pRc->stencilWriteMask   = 0xFFFFFFFF;
  pRc->stencilInUse       = FALSE;

  pRc->sst.peStencil.vFxU32    = ( ((pRc->stencilMask      << SST_PE_ST_MASK_SHIFT      ) & SST_PE_ST_MASK      ) |
                                  ((pRc->stencilWriteMask << SST_PE_ST_WRITE_MASK_SHIFT) & SST_PE_ST_WRITE_MASK)
                                );
  pRc->sst.peStencilOp.vFxU32  = 0;

  pRc->clipRect.MinX = (D3DVALUE) GUARDBAND_LEFT;
  pRc->clipRect.MaxX = (D3DVALUE) GUARDBAND_RIGHT;
  pRc->clipRect.MinY = (D3DVALUE) GUARDBAND_TOP;
  pRc->clipRect.MaxY = (D3DVALUE) GUARDBAND_BOTTOM;

#if (DIRECT3D_VERSION >= 0x0800)
  pRc->swVertexProcessing           = 0;
  pRc->pointSize                    = 0;
  pRc->pointSizeMin                 = 0;
  pRc->pointSpriteEnable            = 0;
  pRc->pointScaleEnable             = 0;
  pRc->pointScaleA                  = 0;
  pRc->pointScaleB                  = 0;
  pRc->pointScaleC                  = 0;        // please fill in the defaults when implementing these states
  pRc->multiSampleAA                = 0;
  pRc->multiSampleMask              = 0;
  pRc->patchEdgeStyle               = 0;
  pRc->patchSegments                = 0;
  pRc->debugMonitorToken            = 0;
  pRc->pointSizeMax                 = 0;
  pRc->indexedVertexBlendEnable     = 0;
  pRc->colorWriteEnable             = D3DCOLORWRITEENABLE_ALPHA |   
	                                  D3DCOLORWRITEENABLE_RED   |
									  D3DCOLORWRITEENABLE_GREEN |
									  D3DCOLORWRITEENABLE_BLUE;
  pRc->tweenFactor                  = 0;
  pRc->blendOp                      = D3DBLENDOP_ADD;
#endif // end DX8


  pRc->state              = (STATE_REQUIRES_IT_RGB | STATE_REQUIRES_VERTS_AREA);

  // default write buffer is the front buffer
  // enable the clip rectangle on Voodoo so if any software does
  // not clip properly then the FIFO won't be overwritten
  // need to set clip regs on H3...
  pRc->sst.peFbzMode.vFxU32  = SST_PE_RGB_WRMASK 
                              // | SST_PE_EN_DEPTH_BIAS
                              // | SST_DEPTH_FLOAT_SEL 
                              | SST_PE_EN_DITHER;

  // default suMode.  Using packed ARGB requires RGB0 and A0 by default.
  pRc->sst.suMode.vFxU32      = SST_SU_PACKED_ARGB;
  pRc->sst.suParamMask.vFxU32 = SST_SU_RGB0 | SST_SU_A0;
  
  // default alpha function and blending is disabled
  // default alpha src blend = blend zero
  // default alpha dst blend = blend never 
  pRc->sst.peAlphaMode.vFxU32 = ((SST_PE_ABLEND_ONE  << SST_PE_RGB_SRC_FACT_SHIFT) |
                                (SST_PE_ABLEND_ZERO << SST_PE_RGB_DST_FACT_SHIFT) |
                                (SST_PE_ABLEND_ZERO << SST_PE_A_SRC_FACT_SHIFT) |
                                (SST_PE_ABLEND_ZERO << SST_PE_A_DST_FACT_SHIFT) |
                                (SST_PE_ABLEND_ADD  << SST_PE_BLEND_CSEL_SHIFT));

 
  // default alphatest func color/depth is "always" but disabled
  pRc->sst.peAlphaTest.vFxU32 = ((SST_PE_ALPHA_CFUNC_LT) |
                          (SST_PE_ALPHA_CFUNC_EQ) |
                          (SST_PE_ALPHA_CFUNC_GT) | 
                          (SST_PE_ALPHA_DFUNC_LT) |
                          (SST_PE_ALPHA_DFUNC_EQ) |
                          (SST_PE_ALPHA_DFUNC_GT));

  pRc->sst.peFogMode.vFxU32 = 0 ;

  // default of 8x16 row/column bands. Need registry control for setting
  // row options 0 (disable), 16, 32, 64 and
  // column options 0 (disable), 8, 16, 32

  pRc->sst.raControl.vFxU32  = (1 << SST_RA_STIPPLE_REPEAT_SHIFT) |
                              ( SST_COL_OF_8 << SST_RA_COL_WIDTH_SHIFT ) |
                              ( SST_ROW_OF_16 << SST_RA_ROW_HEIGHT_SHIFT ); 
  pRc->sst.raStipple.vFxU32  = 0xffffffff;


  // Default value for Zbias is 0 (Zbias range is 0-15)
  pRc->sst.peSDConst.vFxU32  = 0 ;
  
  // default  texMag & texMin       = point sampled
  //          chroma (transparency) = disabled
  //          chroma color          = 0
  //          blend                 = tex_map_decal = (1-Atex)Cpix + (Atex)(Ctex)
  //          perspective           = false 
  //
  //          When we do texture mapping we need the texture and alpha to
  //          come from the local texture memory

  // TODO: Set the default taMode
  //pRc->sst.TMU[0].taMode.vFxU32

  // default gouraud shading
  pRc->sst.TMU[0].taTcuColor.vFxU32 = TCC( ZERO, ZERO, ZERO, ZERO );   
  pRc->sst.TMU[0].taTcuAlpha.vFxU32 = TCA( ZERO, ZERO, ZERO, ZERO );   
  pRc->sst.TMU[0].taCcuColor.vFxU32 = CCC( ZERO, ZERO, ZERO, CITER );
  pRc->sst.TMU[0].taCcuAlpha.vFxU32 = CCA( ZERO, ZERO, ZERO, ZERO );

  // This follows convention of C0 used for the texture factor and C1
  // is 0.5 for use in VTA math. Policy is that these values are not
  // available when the regs are used by other operations that require
  // C0 and/or C1 (eg. 2x2 scaling for bumpmaping).

  // Set texture factor in C0. Scale by 1/256 and map 255/256 to 256/256.
  r = RGBA_GETRED( pRc->textureFactor);
  a = RGBA_GETALPHA( pRc->textureFactor);
  pRc->textureFactorAR = setColorConstant(  
    ( r == 255 ? 1.0F : r/256.0F),
    ( a == 255 ? 1.0F : a/256.0F));

  b = RGBA_GETBLUE( pRc->textureFactor);
  g = RGBA_GETGREEN( pRc->textureFactor);
  pRc->textureFactorGB = setColorConstant(  
    ( b == 255 ? 1.0F : b/256.0F),
    ( g == 255 ? 1.0F : g/256.0F));

  // Set constant factor default of 0.5 in C1
  pRc->constantFactorAR = setColorConstant (0.5f, 0.5f);
  pRc->constantFactorGB = setColorConstant (0.5f, 0.5f);

  pRc->sst.TMU[0].taColorAR0.vFxU32 = pRc->textureFactorAR;
  pRc->sst.TMU[0].taColorGB0.vFxU32 = pRc->textureFactorGB;
  pRc->sst.TMU[0].taColorAR1.vFxU32 = pRc->constantFactorAR;
  pRc->sst.TMU[0].taColorGB1.vFxU32 = pRc->constantFactorGB;


  if( _D3( pixelCenter ) )
    pRc->pixelOffset = 0.5f;
  else
    pRc->pixelOffset = 0.0f;

  {
    int i;
    
    for( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
      TS[i].textureHandle = 0;
      TS[i].colorOp = D3DTOP_DISABLE;
      TS[i].alphaOp = D3DTOP_DISABLE;

      // Initialise texture stages
      TS[i].changed = 0;

      // Initialise texture wrapping
      pRc->wrap[i] = 0;
    }
  }
  

  // Initialise W parameters just in case we don't get any w info
  pRc->aW = 1.f;
  pRc->bW = 0.f;

  pRc->textureFactor = 0x80FFFFFF;
    
  // different triangle routines to handle different attributes
//  setDrawTriangle(pRc);
  pRc->drawIndex = 0;

  BuildPacket3Headers();

  pRc->vertexSize = 0; // set to trigger Pkt3 size and header recompute on first primitive

  pRc->state_requires_tmu_flags[0] = STATE_REQUIRES_ST_TMU0;
  pRc->state_requires_tmu_flags[1] = STATE_REQUIRES_ST_TMU1;
  pRc->state_requires_tmu_flags[2] = STATE_REQUIRES_ST_TMU2;
  pRc->state_requires_tmu_flags[3] = STATE_REQUIRES_ST_TMU3;
  pRc->state_requires_tmu_flags[4] = STATE_REQUIRES_ST_TMU4;
  pRc->state_requires_tmu_flags[5] = STATE_REQUIRES_ST_TMU5;
  pRc->state_requires_tmu_flags[6] = STATE_REQUIRES_ST_TMU6;
  pRc->state_requires_tmu_flags[7] = STATE_REQUIRES_ST_TMU7;

  pRc->su_parammask_tmu_flags[0] =   SST_SU_ST0;
  pRc->su_parammask_tmu_flags[1] =   SST_SU_ST1;
  pRc->su_parammask_tmu_flags[2] =   SST_SU_ST2;
  pRc->su_parammask_tmu_flags[3] =   SST_SU_ST3;
  pRc->su_parammask_tmu_flags[4] =   SST_SU_ST4;
  pRc->su_parammask_tmu_flags[5] =   SST_SU_ST5;
  pRc->su_parammask_tmu_flags[6] =   SST_SU_ST6;
  pRc->su_parammask_tmu_flags[7] =   SST_SU_ST7;

  pRc->su_parammask_rgba_flags[0] =  (SST_SU_RGB0 | SST_SU_A0);
  pRc->su_parammask_rgba_flags[1] =  (SST_SU_RGB1 | SST_SU_A1);
  pRc->su_parammask_rgba_flags[2] =  (SST_SU_RGB2 | SST_SU_A2);
  pRc->su_parammask_rgba_flags[3] =  (SST_SU_RGB3 | SST_SU_A3);
  pRc->su_parammask_rgba_flags[4] =  (SST_SU_RGB4 | SST_SU_A4);
  pRc->su_parammask_rgba_flags[5] =  (SST_SU_RGB5 | SST_SU_A5);
  pRc->su_parammask_rgba_flags[6] =  (SST_SU_RGB6 | SST_SU_A6);
  pRc->su_parammask_rgba_flags[7] =  (SST_SU_RGB7 | SST_SU_A7);

  // Initialize Q flag for each stage.
  pRc->su_parammask_q_flags[0] = SST_SU_Q0;
  pRc->su_parammask_q_flags[1] = SST_SU_Q1;
  pRc->su_parammask_q_flags[2] = SST_SU_Q2;
  pRc->su_parammask_q_flags[3] = SST_SU_Q3;
  pRc->su_parammask_q_flags[4] = SST_SU_Q4;
  pRc->su_parammask_q_flags[5] = SST_SU_Q5;
  pRc->su_parammask_q_flags[6] = SST_SU_Q6;
  pRc->su_parammask_q_flags[7] = SST_SU_Q7;

#ifdef SLI
   // Turn on the SLI bits
   if (SLI_MODE_ENABLED == _DD(sliMode))
      {
      pRc->sst.raControl.vFxU32 |= SST_RA_ENABLE_SLI;
      pRc->sst.peFbzMode.vFxU32 |= SST_PE_SLI_EN;
      }
#endif

  // init render function pointers by setting the dirty flag
  pRc->bRecalcRndrFn = TRUE;

  // These may seem like duplicates of existing rc variables but they aren't really.
  // They take care of the case where you might have 2 sample apps running table fog with
  // different parameters in each one. The table has to be reloaded every frame for each app.
  // Since the rc variables won't change in that case, these flags will indicate that the table 
  // needs to be reloaded because they will not match the context variables- mls 3/30/00
  _D3( currentFogTableMode  = 0 );
  _D3( currentFogTableStart = 0 );
  _D3( currentFogTableEnd   = 0 );
  _D3( currentFogTableDensity = 0 );
  
  //  Since we have a new context, be sure all registers are updated in setDX6State !
  UPDATE_HW_STATE( SC_ALL_CHANGED );

  _D3(lastContext) = 0;
}

//-------------------------------------------------------------
// Application is inquiring the current state of some attribute 
//-------------------------------------------------------------
DWORD __stdcall ddiGetState(LPD3DHAL_GETSTATEDATA pgsd)
{
  RC  *pRc ;
  
  D3D_ENTRY( "ddiGetState" );
  
  /*
   * NOTES:
   *
   * This callback is called when Direct3D requires information about
   * the state of a particular stage in the pipeline. If you only handle
   * rasterisation then you only need to respond to D3DHALSTATE_GET_RENDER
   * calls.
   * The state wanted is in pgsd->ddState.renderStateType.
   * Return the answer in pgsd->ddState.ulArg[0].
   */
  if (CONTEXT_VALIDATE(pgsd->dwhContext))
  {
    D3DPRINT( 255,"GetState, bad context =0x08lx", pgsd->dwhContext );
    pgsd->ddrval = D3DHAL_CONTEXT_BAD;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

  pRc = CONTEXT_PTR(pgsd->dwhContext);
  D3DPRINT( 255,"GetState, pgsd->dwhContext =%08lx, pgsd->dwWhich =%08lx",
                        pgsd->dwhContext, pgsd->dwWhich );

  if (pgsd->dwWhich != D3DHALSTATE_GET_RENDER)
  {
    // You must be able to do transform/lighting
  }
  else 
  {
  #ifdef D3TRACE
//    printSetState(pgsd->ddState.drstRenderStateType,0) ;
    D3DPRINT( 255, "\n" );
  #endif

    // what should I fill in if I don't support one? MIRIAM ???
    switch(pgsd->ddState.drstRenderStateType)
    {                   
      case D3DRENDERSTATE_TEXTUREHANDLE :
        pgsd->ddState.dwArg[0] = TS[0].textureHandle;
        break;
        
      case D3DRENDERSTATE_ANTIALIAS :
        pgsd->ddState.dwArg[0] = pRc->antialias;
        break;

      case D3DRENDERSTATE_TEXTUREPERSPECTIVE : 
        pgsd->ddState.dwArg[0] = pRc->texturePerspective;
        break;

      case D3DRENDERSTATE_WRAPU :
        pgsd->ddState.dwArg[0] = pRc->wrapU ;
        break;

      case D3DRENDERSTATE_WRAPV : 
        pgsd->ddState.dwArg[0] = pRc->wrapV ;
        break;
        
      case D3DRENDERSTATE_ZENABLE :
        pgsd->ddState.dwArg[0] = pRc->zEnable ;
        break;
        
      case D3DRENDERSTATE_FILLMODE : 
        pgsd->ddState.dwArg[0] = pRc->fillMode ;
        break;
          
      case D3DRENDERSTATE_SHADEMODE :
        pgsd->ddState.dwArg[0] = pRc->shadeMode ;
        break;
          
      case D3DRENDERSTATE_LINEPATTERN : 
        // TODO: - add proper feedback if we want to support this 
        pgsd->ddState.dwArg[0] = 0 ;
        break;
          
      case D3DRENDERSTATE_ROP2 : 
        pgsd->ddState.dwArg[0] = pRc->rop2;
        break;
          
      case D3DRENDERSTATE_PLANEMASK :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_ZWRITEENABLE : 
        pgsd->ddState.dwArg[0] = pRc->zWriteEnable ;
        break;
          
      case D3DRENDERSTATE_ALPHATESTENABLE :
        pgsd->ddState.dwArg[0] = pRc->alphaTestEnable ;
        break;
          
      case D3DRENDERSTATE_LASTPIXEL : 
        // Not supported
        pgsd->ddState.dwArg[0] = 0;
        break;
          
      case D3DRENDERSTATE_TEXTUREMAG :
        pgsd->ddState.dwArg[0] = pRc->texMag ;
        break;
          
      case D3DRENDERSTATE_TEXTUREMIN : 
        pgsd->ddState.dwArg[0] = pRc->texMin ;
        break;

	  case D3DRENDERSTATE_ANISOTROPY:
		  pgsd->ddState.dwArg[0] = pRc->anisotropy;
		  break;
          
      case D3DRENDERSTATE_SRCBLEND :
        pgsd->ddState.dwArg[0] = pRc->srcBlend ;
        break;
          
      case D3DRENDERSTATE_DESTBLEND : 
        pgsd->ddState.dwArg[0] = pRc->dstBlend ;
        break;
        
      case D3DRENDERSTATE_TEXTUREMAPBLEND :
        pgsd->ddState.dwArg[0] = pRc->texMapBlend ;
        break;
        
      case D3DRENDERSTATE_CULLMODE : 
        pgsd->ddState.dwArg[0] = pRc->cullMode ;
        break;
        
      case D3DRENDERSTATE_ZFUNC :
        pgsd->ddState.dwArg[0] = pRc->zFunc ;
        break;
        
      case D3DRENDERSTATE_ALPHAREF : 
        pgsd->ddState.dwArg[0] = pRc->alphaRef ;
        break;
        
      case D3DRENDERSTATE_ALPHAFUNC :
        pgsd->ddState.dwArg[0] = pRc->alphaFunc ;
        break;
        
      case D3DRENDERSTATE_DITHERENABLE : 
        pgsd->ddState.dwArg[0] = pRc->ditherEnable ;
        break;
        
      case D3DRENDERSTATE_BLENDENABLE :
        pgsd->ddState.dwArg[0] = pRc->blendEnable ;
        break;
        
      case D3DRENDERSTATE_FOGENABLE : 
        pgsd->ddState.dwArg[0] = pRc->fogEnable ;
        break;
        
      case D3DRENDERSTATE_SPECULARENABLE : 
        pgsd->ddState.dwArg[0] = pRc->specular ;
        break;
        
      case D3DRENDERSTATE_ZVISIBLE :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;
        
      case D3DRENDERSTATE_SUBPIXEL :
        pgsd->ddState.dwArg[0] = pRc->subPixel ;
        break;
        
      case D3DRENDERSTATE_SUBPIXELX :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;
        
      case D3DRENDERSTATE_STIPPLEDALPHA :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;
        
      case D3DRENDERSTATE_FOGCOLOR :
        pgsd->ddState.dwArg[0] = pRc->fogColor ;
        break;
        
      case D3DRENDERSTATE_FOGTABLEMODE :
        pgsd->ddState.dwArg[0] = pRc->fogTableMode ;
        break;
        
      case D3DRENDERSTATE_FOGTABLESTART :
        pgsd->ddState.dvArg[0] = pRc->fogTableStart ;
        break;
        
      case D3DRENDERSTATE_FOGTABLEEND :
        pgsd->ddState.dvArg[0] = pRc->fogTableEnd ;
        break;
        
      case D3DRENDERSTATE_FOGTABLEDENSITY :
        pgsd->ddState.dvArg[0] = pRc->fogDensity ;
        break;
        
      case D3DRENDERSTATE_STIPPLEENABLE :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;
        
      case D3DRENDERSTATE_EDGEANTIALIAS :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;
        
      case D3DRENDERSTATE_COLORKEYENABLE :
        pgsd->ddState.dwArg[0] = pRc->colorKeyEnable ;
        break;
        
      case D3DRENDERSTATE_BORDERCOLOR :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;
        
      case D3DRENDERSTATE_TEXTUREADDRESS : 
        pgsd->ddState.dwArg[0] = pRc->textureAddress;
        break;
        
      case D3DRENDERSTATE_TEXTUREADDRESSU :
        pgsd->ddState.dwArg[0] = pRc->textureAddressU ;
        break;
        
      case D3DRENDERSTATE_TEXTUREADDRESSV :
        pgsd->ddState.dwArg[0] = pRc->textureAddressV ;
        break;
        
      default :
        // Unknown Render state
        pgsd->ddState.dwArg[0] = 0 ;
        break;
    } // switch
  }

#ifdef D3TRACE
//  printSetState(pgsd->ddState.drstRenderStateType, pgsd->ddState.dwArg[0]) ;
#endif

  pgsd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
} 

//-------------------------------------------------------
//
//  This function initializes every Rampage 3D register
//  in the shadow copy and writes that state to hardware.
//
//-------------------------------------------------------
void __stdcall init3Dregs(RC *pRc)
{
  FxU32  i;

  SETUP_PPDEV(pRc)

  CMDFIFO_PROLOG(cmdFifo);

  HW_ACCESS_ENTRY(cmdFifo, ACCESS_3D);

  //
  // SST2 3D Hardware Register state
  //

  CMDFIFO_CHECKROOM( cmdFifo, 273);

  // Initialize all shadow registers to ZERO
  memset(&(pRc->sst), 0, sizeof(SST2_3DREGS));

  // Now set special non-zero initialization values for some registers :
  // -------------------------------------------------------------------------------
  pRc->sst.suLineWidth.vFxU32 = DEFAULT_LINEWIDTH;
  //
  // For peCache :
  // From Rajeshwaran Selvanesan 8/4/2000 : cache_en, merge_en, outoforder_en must
  // be set. Write_thru must be off. Aux and Col buf enables default to on, but
  // set by driver based upon surface present and Z buffer enabled. Col and Aux
  // buffer enables are active low.
  pRc->sst.peCache.vFxU32 = SST_PE_CACHE_EN |
                            SST_PE_MERGE_EN |
                            SST_PE_OUT_OF_ORDER_EN;
                            // 0 << SST_PE_READ_COL_EN_N_SHIFT |
                            // 0 << SST_PE_READ_AUX_EN_N_SHIFT |
                            // 0 << SST_PE_WRITE_THRU_EN_SHIFT;
  // -------------------------------------------------------------------------------

#ifdef CSERVICE
    D3D_GRABBING_CS_CONTEXT();
#endif

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5|R6,
                  SST_UNIT_FBI, vpMode));
  SETPD( cmdFifo, ghw0->vpMode,     pRc->sst.vpMode.vFxU32);
  SETPD( cmdFifo, ghw0->vpSizeX,    pRc->sst.vpSizeX.vFxU32);
  SETPD( cmdFifo, ghw0->vpCenterX,  pRc->sst.vpCenterX.vFxU32);
  SETPD( cmdFifo, ghw0->vpSizeY,    pRc->sst.vpSizeY.vFxU32);
  SETPD( cmdFifo, ghw0->vpCenterY,  pRc->sst.vpCenterY.vFxU32);
  SETPD( cmdFifo, ghw0->vpSizeZ,    pRc->sst.vpSizeZ.vFxU32);
  SETPD( cmdFifo, ghw0->vpCenterZ,  pRc->sst.vpCenterZ.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, SST_UNIT_FBI, vpSTscale0));
  SETPD( cmdFifo, ghw0->vpSTscale0, pRc->sst.vpSTscale0.vFxU32);
  SETPD( cmdFifo, ghw0->vpSTscale1, pRc->sst.vpSTscale1.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, SST_UNIT_FBI, suMode));
  SETPD( cmdFifo, ghw0->suMode,         pRc->sst.suMode.vFxU32);
  SETPD( cmdFifo, ghw0->suParamMask,    pRc->sst.suParamMask.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5|R6|R7,
                  SST_UNIT_FBI, suClipMinXMaxX[0]));
  for( i=0; i<8; i++)
    SETPD( cmdFifo, ghw0->suClipMinXMaxX[i], pRc->sst.suClipMinXMaxX[i].vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5|R6|R7,
                  SST_UNIT_FBI, suClipMinYMaxY[0]));
  for( i=0; i<8; i++)
    SETPD( cmdFifo, ghw0->suClipMinYMaxY[i], pRc->sst.suClipMinYMaxY[i].vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1, SST_UNIT_FBI, suClipEnables));
  SETPD( cmdFifo, ghw0->suClipEnables, pRc->sst.suClipEnables.vFxU32);
  SETPD( cmdFifo, ghw0->suLineWidth, pRc->sst.suLineWidth.vFxU32);

  // count=35 

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1,
                  SST_UNIT_FBI, raControl));
  SETPD( cmdFifo, ghw0->raControl, pRc->sst.raControl.vFxU32);
  SETPD( cmdFifo, ghw0->raStipple, pRc->sst.raStipple.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK1(20, SST_UNIT_FBI, peFogTable));
  for ( i = 0; i < 20; i++ )
  	 SETPD( cmdFifo, ghw0->peFogTable[i], pRc->sst.peFogTable[i].vFxU32);

  // count = 59

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5|R6|R7|R8|R9,
                  SST_UNIT_FBI, peFogColor));
  SETPD( cmdFifo, ghw0->peFogColor,     pRc->sst.peFogColor.vFxU32);
  SETPD( cmdFifo, ghw0->peFogMode,      pRc->sst.peFogMode.vFxU32);
  SETPD( cmdFifo, ghw0->peFbzMode,      pRc->sst.peFbzMode.vFxU32);
  SETPD( cmdFifo, ghw0->peAlphaTest,    pRc->sst.peAlphaTest.vFxU32);
  SETPD( cmdFifo, ghw0->peAlphaMode,    pRc->sst.peAlphaMode.vFxU32);
  SETPD( cmdFifo, ghw0->peSDConst,      pRc->sst.peSDConst.vFxU32);
  SETPD( cmdFifo, ghw0->peFogBias,      pRc->sst.peFogBias.vFxU32);
  SETPD( cmdFifo, ghw0->peStencil,      pRc->sst.peStencil.vFxU32);
  SETPD( cmdFifo, ghw0->peStencilOp,    pRc->sst.peStencilOp.vFxU32);
  SETPD( cmdFifo, ghw0->peCache,        pRc->sst.peCache.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5,
                  SST_UNIT_FBI, peColBufferAddr));
  SETPD( cmdFifo, ghw0->peColBufferAddr,    pRc->sst.peColBufferAddr.vFxU32);
  SETPD( cmdFifo, ghw0->peAuxBufferAddr,    pRc->sst.peAuxBufferAddr.vFxU32);
  SETPD( cmdFifo, ghw0->peBufferSize,       pRc->sst.peBufferSize.vFxU32);
  SETPD( cmdFifo, ghw0->peClipMinXMaxX,     pRc->sst.peClipMinXMaxX.vFxU32);
  SETPD( cmdFifo, ghw0->peClipMinYMaxY,     pRc->sst.peClipMinYMaxY.vFxU32);
  SETPD( cmdFifo, ghw0->peExMask,           pRc->sst.peExMask.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0,
                  SST_UNIT_FBI, taControl));
  SETPD( cmdFifo, ghw0->taControl, pRc->sst.taControl.vFxU32);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0,
                  SST_UNIT_FBI, taLfbMode));
  SETPD( cmdFifo, ghw0->taLfbMode, pRc->sst.taLfbMode.vFxU32);

  // count = 81

  for( i=0; i<8; i++)
  {
     SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5|R6|R7|R8|R9|R10,
                     SST_UNIT_TMU_0+i, taMode));
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taMode,      pRc->sst.TMU[i].taMode.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taLMS,       pRc->sst.TMU[i].taLMS.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taShiftBias, pRc->sst.TMU[i].taShiftBias.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taDetail,    pRc->sst.TMU[i].taDetail.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taNPT,       pRc->sst.TMU[i].taNPT.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taBaseAddr0, pRc->sst.TMU[i].taBaseAddr0.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taBaseAddr1, pRc->sst.TMU[i].taBaseAddr1.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taBaseAddr2, pRc->sst.TMU[i].taBaseAddr2.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taBaseAddr3, pRc->sst.TMU[i].taBaseAddr3.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taTcuColor,  pRc->sst.TMU[i].taTcuColor.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taTcuAlpha,  pRc->sst.TMU[i].taTcuAlpha.vFxU32);


     SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R4|R5|R6|R7|R8|R9|R10,
                     SST_UNIT_TMU_0+i, taCcuControl));
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taCcuControl,        pRc->sst.TMU[i].taCcuControl.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taCcuColor,          pRc->sst.TMU[i].taCcuColor.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taCcuAlpha,          pRc->sst.TMU[i].taCcuAlpha.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taTexChromaKey,      pRc->sst.TMU[i].taTexChromaKey.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taTexChromaRange,    pRc->sst.TMU[i].taTexChromaRange.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taChromaKey,         pRc->sst.TMU[i].taChromaKey.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taChromaRange,       pRc->sst.TMU[i].taChromaRange.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taColorAR0,          pRc->sst.TMU[i].taColorAR0.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taColorGB0,          pRc->sst.TMU[i].taColorGB0.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taColorAR1,          pRc->sst.TMU[i].taColorAR1.vFxU32);
     SETPD( cmdFifo, SST_TREX(ghw0,i)->taColorGB1,          pRc->sst.TMU[i].taColorGB1.vFxU32);
  }

  // count = 81 + 192 = 273

  HW_ACCESS_EXIT(ACCESS_3D);

  CMDFIFO_EPILOG( cmdFifo );  

} 

