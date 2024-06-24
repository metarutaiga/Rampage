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
**  30   3dfx      1.29        12/8/00  Miles Smith     Added state changes for 
**       COLORWRITEENABLE, BLENDOP, and MULTISAMPLEMASK
**  29   3dfx      1.28        11/17/00 Miles Smith     Adding multisample 
**       support to textureformat definitions.
**  28   3dfx      1.27        10/20/00 Dale  Kenaston  Sage PPE uCode Download.
**       Modified D3DHALCreateDriver to call geInit is IS_SAGE_ACTIVE.
**  27   3dfx      1.26        10/11/00 Brent Burton    DX8 code integration.  
**       Changes to headers, new code.
**  26   3dfx      1.25        10/4/00  Dale  Kenaston  New Sage macros. Changed
**       IS_RAGE in D3DHALCreateDriver to IS_SAGE_ACTIVE.
**  25   3dfx      1.24        9/6/00   Michel Conrad   Added switch for 
**       toggling cube map translation  on/off.
**  24   3dfx      1.23        8/30/00  Tim Little      More changes to remove 
**       #ifdef HW_TNL conditional compilation
**  23   3dfx      1.22        8/11/00  Brian Danielson Fixed setDX6State bug, 
**       enabled peCache, added Central Services code.
**  22   3dfx      1.21        8/10/00  Evan Leland     Adds support for 
**       explicit vertex buffers; removes the VERT_BUFF compile option.
**  21   3dfx      1.20        8/3/00   Don Fowler      Removed unsupported 555 
**       pixel format. 
**  20   3dfx      1.19        7/10/00  Michel Conrad   Remove dead code.
**  19   3dfx      1.18        6/15/00  Michel Conrad   Delete really old check 
**       in comments and drawglobal setting.
**  18   3dfx      1.17        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  17   3dfx      1.16        3/1/00   Evan Leland     Moved the 8-bit 
**       palettized format above the 8+8 Alpha Palettized format. In our texture
**       formats structure. This odd change fixes a problem with Winbench 2000: 
**       it was choosing the first palettized format it finds for some of the 
**       quality tests, but assuming it is an 8-bit format, not 16-bit as 88AP 
**       is. This is an app problem.
**  16   3dfx      1.15        2/11/00  Brian Danielson Added Z Write Mask cap 
**       bit. PRS 12451.
**  15   3dfx      1.14        2/7/00   Michel Conrad   Clean/Update Cap options
**       for DX7. Enabled ROP2 for lines (for consistency w/tirnagles). Enable 
**       D3DPEXTURECAPS_CUBEMAP ifdef CUBEMAP is defined.
**  14   3dfx      1.13        1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  13   3dfx      1.12        12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  12   3dfx      1.11        12/16/99 Evan Leland     changed function names 
**       and put prototypes in d3txtr.h for all texture routines that are 
**       exported from the texture module
**  11   3dfx      1.10        11/23/99 Brent Burton    Reset pixelCenter to 1 
**       by default.  texelCenter remains 0.  I am now convinced these are 
**       correct
**  10   3dfx      1.9         11/19/99 Brent Burton    Returned pixelCenter to 
**       0 by default.  This issue is not solved to my satisfaction.
**  9    3dfx      1.8         11/11/99 Brent Burton    Enable PixelCenter 
**       offsets by default.  Still overridden by envariable 
**       SST2_PIXELCENTER=1/0.
**  8    3dfx      1.7         10/31/99 Michel Conrad   Expose excute mode 
**       textures capability.
**  7    3dfx      1.6         10/26/99 Evan Leland     Implements FXT1 texture 
**       mode support
**  6    3dfx      1.5         10/13/99 Philip Zheng    Added T&L cap for R3
**  5    3dfx      1.4         10/7/99  Philip Zheng    Added vertex buffer 
**       support.  Only affects R3.   The compiler flag is VERT_BUFF
**  4    3dfx      1.3         9/17/99  Michel Conrad   Pixel/Texel centers off
**  3    3dfx      1.2         9/17/99  Brent Burton    Added texture 
**       descriptors for UV88, UVL556 and UVL888 formats.
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
*/

#include <ddrawi.h>
#include <d3dhal.h>

#include "runtime.h"
#include "hw.h"
#include "fxglobal.h"

#include "d3contxt.h"
#include "d3txtr2.h"

extern DWORD __stdcall ddiSceneCapture(LPD3DHAL_SCENECAPTUREDATA psc) ;

//--------------------------------------------------------------------
// Direct3D HAL Table - Indicates which HAL calls this driver supports
// This is the entry points for this driver therefore it is the roadmap
// for understanding when we get control.
//-------------------------------------------------------------------- 
static D3DHAL_CALLBACKS myD3DHALCallbacks = {
    sizeof(D3DHAL_CALLBACKS),

    // Device context
    ddiContextCreate,            // Required.
    ddiContextDestroy,           // Required.
    ddiContextDestroyAll,        // Required.

    // Scene capture
    NULL,
    
    // Execution
    NULL,                       // dx5 execute buffer
    NULL,                       // dx5 myExecuteClipped
    NULL,                       // dx5 render state handler
    NULL,                       // dx5 primitive handler
    0L,                         // must be zero
    
    // Textures
    NULL,
    NULL,
    NULL,
    NULL,
  
    // Transform - must be supported if lighting is supported.
    NULL, //myMatrixCreate,     // If any of these calls are supported,
    NULL, //myMatrixDestroy,    // they must all be.
    NULL, //myMatrixSetData,    // ditto
    NULL, //myMatrixGetData,    // ditto
    NULL, //mySetViewportData,  // ditto
    
    // Lighting
    NULL,                       // If any of these calls are supported,
    NULL, //myMaterialCreate,   // they must all be.
    NULL, //myMaterialDestroy,  // ditto
    NULL, //myMaterialSetData,  // ditto
    NULL, //myMaterialGetData,  // ditto

    // Pipeline state
    ddiGetState,                 // Required.

    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
    0L,                         // Reserved, must be zero
};

#define nullPrimCaps {                                          \
    sizeof(D3DPRIMCAPS), 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0     \
}


//-----------------
// D3D capabilities
//-----------------
static D3DDEVICEDESC_V1 caps = {
    sizeof(D3DDEVICEDESC_V1),      /* dwSize */
    // valid fields
    D3DDD_COLORMODEL                   |    
    D3DDD_DEVCAPS                      |    
    //D3DDD_TRANSFORMCAPS              |
    //D3DDD_LIGHTINGCAPS               |
    //D3DDD_BCLIPPING                  |
    D3DDD_LINECAPS                     |      
    D3DDD_TRICAPS                      |
    D3DDD_DEVICERENDERBITDEPTH         |  
    D3DDD_DEVICEZBUFFERBITDEPTH        |
    //D3DDD_MAXBUFFERSIZE              |
    //D3DDD_MAXVERTEXCOUNT             |
    0,

    // color model
    // D3DCOLOR_MONO                   |
    D3DCOLOR_RGB                       |
    0,
    
    // device caps
    D3DDEVCAPS_FLOATTLVERTEX           |
    //D3DDEVCAPS_SORTINCREASINGZ       |
    //D3DDEVCAPS_SORTDECREASINGZ       |
    //D3DDEVCAPS_SORTEXACT             |
    D3DDEVCAPS_EXECUTESYSTEMMEMORY     |  
    // D3DDEVCAPS_EXECUTEVIDEOMEMORY   |    // set thru a runtime check now
    D3DDEVCAPS_TLVERTEXSYSTEMMEMORY    |
    // D3DDEVCAPS_TLVERTEXVIDEOMEMORY  |    // set thru a runtime check now
    //D3DDEVCAPS_TEXTURESYSTEMMEMORY 
    D3DDEVCAPS_TEXTUREVIDEOMEMORY      |
    D3DDEVCAPS_DRAWPRIMTLVERTEX        |
    D3DDEVCAPS_CANRENDERAFTERFLIP      |
#ifdef AGP_EXECUTE
    D3DDEVCAPS_TEXTURENONLOCALVIDMEM   |
#endif
    D3DDEVCAPS_DRAWPRIMITIVES2         |
    // D3DDEVCAPS_SEPARATETEXTUREMEMORIES | // Voodoo2 uses this
    D3DDEVCAPS_DRAWPRIMITIVES2EX       |
    // D3DDEVCAPS_CANBLTSYSTONONLOCAL  |
    // D3DDEVCAPS_HWRASTERIZATION      |
    0,
    // This cap will get overwritten if we are on SAGE
    //transformation capabilities (none)
    { sizeof(D3DTRANSFORMCAPS), 0 },                
        
    // This cap will get overwritten if we are on SAGE
    FALSE,                           /* bClipping */
    // This caps will get overwritten if we are on SAGE
    { sizeof(D3DLIGHTINGCAPS), 0 },  /* lightingCaps */

    //---------- 
    //  line
    //---------- 
    {
      sizeof(D3DPRIMCAPS),
      
      //miscCaps
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      D3DPMISCCAPS_BLENDOP			   |  
	  D3DPMISCCAPS_COLORWRITEENABLE    |
#endif
      //D3DPMISCCAPS_MASKPLANES        |
      D3DPMISCCAPS_MASKZ               |
      //D3DPMISCCAPS_LINEPATTERNREP    |
      //D3DPMISCCAPS_CONFORMANT        |
      D3DPMISCCAPS_CULLNONE            |     
      D3DPMISCCAPS_CULLCW              |     
      D3DPMISCCAPS_CULLCCW             |
      0,
      
      //rasterCaps
      D3DPRASTERCAPS_DITHER            |
      D3DPRASTERCAPS_ROP2              |
      //D3DPRASTERCAPS_XOR             |
      //D3DPRASTERCAPS_PAT             |
      //D3DPRASTERCAPS_ZTEST           |
      D3DPRASTERCAPS_SUBPIXEL          | 
      //D3DPRASTERCAPS_SUBPIXELX       |
      D3DPRASTERCAPS_FOGVERTEX         | 
      D3DPRASTERCAPS_FOGTABLE          |
      //D3DPRASTERCAPS_STIPPLE         |
      //D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
      D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
      //D3DPRASTERCAPS_ANTIALIASEDGES  |
      D3DPRASTERCAPS_MIPMAPLODBIAS     |
      D3DPRASTERCAPS_ZBIAS             |
      //D3DPRASTERCAPS_ZBUFFERLESSHSR  |
      //D3DPRASTERCAPS_FOGRANGE        |
      D3DPRASTERCAPS_ANISOTROPY        |
      D3DPRASTERCAPS_WBUFFER           |
      // D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT  |
      D3DPRASTERCAPS_WFOG              |
      //D3DPRASTERCAPS_ZFOG            |
      0,
      
      //zCmpCaps
      D3DPCMPCAPS_NEVER                |     
      D3DPCMPCAPS_LESS                 |     
      D3DPCMPCAPS_EQUAL                |     
      D3DPCMPCAPS_LESSEQUAL            |
      D3DPCMPCAPS_GREATER              | 
      D3DPCMPCAPS_NOTEQUAL             | 
      D3DPCMPCAPS_GREATEREQUAL         |
      D3DPCMPCAPS_ALWAYS               |
      0, 

      //sourceBlendCaps
      D3DPBLENDCAPS_ZERO               | 
      D3DPBLENDCAPS_ONE                | 
      D3DPBLENDCAPS_SRCCOLOR           | 
      D3DPBLENDCAPS_INVSRCCOLOR        | 
      D3DPBLENDCAPS_SRCALPHA           | 
      D3DPBLENDCAPS_INVSRCALPHA        | 
      D3DPBLENDCAPS_DESTALPHA          | 
      D3DPBLENDCAPS_INVDESTALPHA       | 
      D3DPBLENDCAPS_DESTCOLOR          | 
      D3DPBLENDCAPS_INVDESTCOLOR       | 
      D3DPBLENDCAPS_SRCALPHASAT        | 
      D3DPBLENDCAPS_BOTHSRCALPHA       | 
      D3DPBLENDCAPS_BOTHINVSRCALPHA    |
      0,

      //destBlend
      D3DPBLENDCAPS_ZERO               | 
      D3DPBLENDCAPS_ONE                | 
      D3DPBLENDCAPS_SRCCOLOR           | 
      D3DPBLENDCAPS_INVSRCCOLOR        | 
      D3DPBLENDCAPS_SRCALPHA           | 
      D3DPBLENDCAPS_INVSRCALPHA        | 
      D3DPBLENDCAPS_DESTALPHA          | 
      D3DPBLENDCAPS_INVDESTALPHA       | 
      D3DPBLENDCAPS_DESTCOLOR          | 
      D3DPBLENDCAPS_INVDESTCOLOR       | 
      D3DPBLENDCAPS_SRCALPHASAT        |
      D3DPBLENDCAPS_BOTHSRCALPHA       | 
      D3DPBLENDCAPS_BOTHINVSRCALPHA    |
      0,

      // alphaCmpCaps
      D3DPCMPCAPS_NEVER                |
      D3DPCMPCAPS_LESS                 |
      D3DPCMPCAPS_EQUAL                |
      D3DPCMPCAPS_LESSEQUAL            |
      D3DPCMPCAPS_GREATER              |
      D3DPCMPCAPS_NOTEQUAL             |
      D3DPCMPCAPS_GREATEREQUAL         |
      D3DPCMPCAPS_ALWAYS               |
      0, 

      // shadeCaps                           
      //D3DPSHADECAPS_COLORFLATMONO          |
      D3DPSHADECAPS_COLORFLATRGB             |       
      //D3DPSHADECAPS_COLORGOURAUDMONO       |
      D3DPSHADECAPS_COLORGOURAUDRGB          |     
      //D3DPSHADECAPS_COLORPHONGMONO         |
      //D3DPSHADECAPS_COLORPHONGRGB          |
      D3DPSHADECAPS_SPECULARFLATMONO         |  
      D3DPSHADECAPS_SPECULARFLATRGB          |  
      D3DPSHADECAPS_SPECULARGOURAUDMONO      | 
      D3DPSHADECAPS_SPECULARGOURAUDRGB       |
      //D3DPSHADECAPS_SPECULARPHONGMONO      |
      //D3DPSHADECAPS_SPECULARPHONGRGB       |
      D3DPSHADECAPS_ALPHAFLATBLEND           |
      //D3DPSHADECAPS_ALPHAFLATSTIPPLED      |
      D3DPSHADECAPS_ALPHAGOURAUDBLEND        |
      //D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED   |
      //D3DPSHADECAPS_ALPHAPHONGBLEND        |
      //D3DPSHADECAPS_ALPHAPHONGSTIPPLED     |
      D3DPSHADECAPS_FOGFLAT                  |         
      D3DPSHADECAPS_FOGGOURAUD               |
      //D3DPSHADECAPS_FOGPHONG               |
      0,
              
      // textureCaps      
      D3DPTEXTURECAPS_PERSPECTIVE            |
      //D3DPTEXTURECAPS_POW2                 |
      D3DPTEXTURECAPS_ALPHA                  |
      D3DPTEXTURECAPS_TRANSPARENCY           |
      //obsolete D3DPTEXTURECAPS_BORDER      |
      //D3DPTEXTURECAPS_SQUAREONLY           |
      //D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
      D3DPTEXTURECAPS_ALPHAPALETTE           |
      //D3DPTEXTURECAPS_NONPOW2CONDITIONAL   |
      D3DPTEXTURECAPS_PROJECTED              |
#ifdef CUBEMAP
      D3DPTEXTURECAPS_CUBEMAP                |
#endif
      //D3DPTEXTURECAPS_COLORKEYBLEND        |
      0,

      // textureFilterCaps
      D3DPTFILTERCAPS_NEAREST                |
      D3DPTFILTERCAPS_LINEAR                 |
      D3DPTFILTERCAPS_MIPNEAREST             |
      D3DPTFILTERCAPS_MIPLINEAR              | 
      D3DPTFILTERCAPS_LINEARMIPNEAREST       |
      D3DPTFILTERCAPS_LINEARMIPLINEAR        |
      D3DPTFILTERCAPS_MINFPOINT              |
      D3DPTFILTERCAPS_MINFLINEAR             |
      D3DPTFILTERCAPS_MINFANISOTROPIC		   |
      D3DPTFILTERCAPS_MIPFPOINT              |
      D3DPTFILTERCAPS_MIPFLINEAR             |
      D3DPTFILTERCAPS_MAGFPOINT              |
      D3DPTFILTERCAPS_MAGFLINEAR             |
      D3DPTFILTERCAPS_MAGFANISOTROPIC		   |
      //D3DPTFILTERCAPS_MAGFALATCUBIC		   |
      //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC		|
      0,

      // textureBlend
      D3DPTBLENDCAPS_DECAL                   |
      D3DPTBLENDCAPS_MODULATE                |
      D3DPTBLENDCAPS_DECALALPHA              |
      D3DPTBLENDCAPS_MODULATEALPHA           |
      D3DPTBLENDCAPS_DECALMASK               |
      D3DPTBLENDCAPS_MODULATEMASK            |
      D3DPTBLENDCAPS_COPY                    |
      D3DPTBLENDCAPS_ADD                     |
      0,

      // Texture AddressCaps
      D3DPTADDRESSCAPS_WRAP                  |
      D3DPTADDRESSCAPS_MIRROR                |
      D3DPTADDRESSCAPS_CLAMP                 |
      //D3DPTADDRESSCAPS_BORDER              |
      D3DPTADDRESSCAPS_INDEPENDENTUV         |
      0,
      0,                          // stippleWidth      
      0                           // stippleHeight     
    },                                                    
   
    //---------- 
    //  triangle
    //---------- 
    {
      sizeof(D3DPRIMCAPS),
      
      //miscCaps
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      D3DPMISCCAPS_BLENDOP			   |  //MILESXX
	  D3DPMISCCAPS_COLORWRITEENABLE    |
#endif
      //D3DPMISCCAPS_MASKPLANES        |
      D3DPMISCCAPS_MASKZ               |
      //D3DPMISCCAPS_LINEPATTERNREP    |
      //D3DPMISCCAPS_CONFORMANT        |
      D3DPMISCCAPS_CULLNONE            |     
      D3DPMISCCAPS_CULLCW              |     
      D3DPMISCCAPS_CULLCCW             |
      0,
      
      //rasterCaps
      D3DPRASTERCAPS_DITHER            |
      D3DPRASTERCAPS_ROP2              |
      //D3DPRASTERCAPS_XOR             |
      //D3DPRASTERCAPS_PAT             |
      //D3DPRASTERCAPS_ZTEST           |
      D3DPRASTERCAPS_SUBPIXEL          | 
      //D3DPRASTERCAPS_SUBPIXELX       |
      D3DPRASTERCAPS_FOGVERTEX         | 
      D3DPRASTERCAPS_FOGTABLE          |
      //D3DPRASTERCAPS_STIPPLE         |
      //D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
      D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
      //D3DPRASTERCAPS_ANTIALIASEDGES  |
      D3DPRASTERCAPS_MIPMAPLODBIAS     |
      D3DPRASTERCAPS_ZBIAS             |
      //D3DPRASTERCAPS_ZBUFFERLESSHSR  |
      //D3DPRASTERCAPS_FOGRANGE        |
      D3DPRASTERCAPS_ANISOTROPY        |
      D3DPRASTERCAPS_WBUFFER           |
      //D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT  |
      D3DPRASTERCAPS_WFOG              |
      //D3DPRASTERCAPS_ZFOG            |
      0,
      
      //zCmpCaps
      D3DPCMPCAPS_NEVER                |     
      D3DPCMPCAPS_LESS                 |     
      D3DPCMPCAPS_EQUAL                |     
      D3DPCMPCAPS_LESSEQUAL            |
      D3DPCMPCAPS_GREATER              | 
      D3DPCMPCAPS_NOTEQUAL             | 
      D3DPCMPCAPS_GREATEREQUAL         |
      D3DPCMPCAPS_ALWAYS               |
      0,

      //sourceBlendCaps
      D3DPBLENDCAPS_ZERO               | 
      D3DPBLENDCAPS_ONE                | 
      D3DPBLENDCAPS_SRCCOLOR           | 
      D3DPBLENDCAPS_INVSRCCOLOR        | 
      D3DPBLENDCAPS_SRCALPHA           | 
      D3DPBLENDCAPS_INVSRCALPHA        | 
      D3DPBLENDCAPS_DESTALPHA          | 
      D3DPBLENDCAPS_INVDESTALPHA       | 
      D3DPBLENDCAPS_DESTCOLOR          | 
      D3DPBLENDCAPS_INVDESTCOLOR       | 
      D3DPBLENDCAPS_SRCALPHASAT        | 
      D3DPBLENDCAPS_BOTHSRCALPHA       | 
      D3DPBLENDCAPS_BOTHINVSRCALPHA    |
      0,

      //destBlend
      D3DPBLENDCAPS_ZERO               | 
      D3DPBLENDCAPS_ONE                | 
      D3DPBLENDCAPS_SRCCOLOR           | 
      D3DPBLENDCAPS_INVSRCCOLOR        | 
      D3DPBLENDCAPS_SRCALPHA           | 
      D3DPBLENDCAPS_INVSRCALPHA        | 
      D3DPBLENDCAPS_DESTALPHA          | 
      D3DPBLENDCAPS_INVDESTALPHA       | 
      D3DPBLENDCAPS_DESTCOLOR          | 
      D3DPBLENDCAPS_INVDESTCOLOR       | 
      D3DPBLENDCAPS_SRCALPHASAT        |
      D3DPBLENDCAPS_BOTHSRCALPHA       | 
      D3DPBLENDCAPS_BOTHINVSRCALPHA    |
      0,

      // alphaCmpCaps
      D3DPCMPCAPS_NEVER                |
      D3DPCMPCAPS_LESS                 |
      D3DPCMPCAPS_EQUAL                |
      D3DPCMPCAPS_LESSEQUAL            |
      D3DPCMPCAPS_GREATER              |
      D3DPCMPCAPS_NOTEQUAL             |
      D3DPCMPCAPS_GREATEREQUAL         |
      D3DPCMPCAPS_ALWAYS               |
      0, 

      // shadeCaps                           
      //D3DPSHADECAPS_COLORFLATMONO          |
      D3DPSHADECAPS_COLORFLATRGB             |       
      //D3DPSHADECAPS_COLORGOURAUDMONO       |
      D3DPSHADECAPS_COLORGOURAUDRGB          |     
      //D3DPSHADECAPS_COLORPHONGMONO         |
      //D3DPSHADECAPS_COLORPHONGRGB          |
      D3DPSHADECAPS_SPECULARFLATMONO         |  
      D3DPSHADECAPS_SPECULARFLATRGB          |  
      D3DPSHADECAPS_SPECULARGOURAUDMONO      | 
      D3DPSHADECAPS_SPECULARGOURAUDRGB       |
      //D3DPSHADECAPS_SPECULARPHONGMONO      |
      //D3DPSHADECAPS_SPECULARPHONGRGB       |
      D3DPSHADECAPS_ALPHAFLATBLEND           |
      //D3DPSHADECAPS_ALPHAFLATSTIPPLED      |
      D3DPSHADECAPS_ALPHAGOURAUDBLEND        |
      //D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED   |
      //D3DPSHADECAPS_ALPHAPHONGBLEND        |
      //D3DPSHADECAPS_ALPHAPHONGSTIPPLED     |
      D3DPSHADECAPS_FOGFLAT                  |         
      D3DPSHADECAPS_FOGGOURAUD               |
      //D3DPSHADECAPS_FOGPHONG               |
      0,

      // textureCaps      
      D3DPTEXTURECAPS_PERSPECTIVE            |
      //D3DPTEXTURECAPS_POW2                 |
      D3DPTEXTURECAPS_ALPHA                  |
      D3DPTEXTURECAPS_TRANSPARENCY           |
      //obsolete D3DPTEXTURECAPS_BORDER      |
      //D3DPTEXTURECAPS_SQUAREONLY           |
      //D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
      D3DPTEXTURECAPS_ALPHAPALETTE           |
      //D3DPTEXTURECAPS_NONPOW2CONDITIONAL   |
      D3DPTEXTURECAPS_PROJECTED              |
#ifdef CUBEMAP
      D3DPTEXTURECAPS_CUBEMAP                |
#endif
      //D3DPTEXTURECAPS_COLORKEYBLEND        |
      0,

      // textureFilterCaps
      D3DPTFILTERCAPS_NEAREST                |
      D3DPTFILTERCAPS_LINEAR                 |
      D3DPTFILTERCAPS_MIPNEAREST             |
      D3DPTFILTERCAPS_MIPLINEAR              | 
      D3DPTFILTERCAPS_LINEARMIPNEAREST       |
      D3DPTFILTERCAPS_LINEARMIPLINEAR        |
      D3DPTFILTERCAPS_MINFPOINT              |
      D3DPTFILTERCAPS_MINFLINEAR             |
      D3DPTFILTERCAPS_MINFANISOTROPIC		   |
      D3DPTFILTERCAPS_MIPFPOINT              |
      D3DPTFILTERCAPS_MIPFLINEAR             |
      D3DPTFILTERCAPS_MAGFPOINT              |
      D3DPTFILTERCAPS_MAGFLINEAR             |
      D3DPTFILTERCAPS_MAGFANISOTROPIC		   |
      //D3DPTFILTERCAPS_MAGFALATCUBIC		   |
      //D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC		|
      0,

      // textureBlend
      D3DPTBLENDCAPS_DECAL                   |
      D3DPTBLENDCAPS_MODULATE                |
      D3DPTBLENDCAPS_DECALALPHA              |
      D3DPTBLENDCAPS_MODULATEALPHA           |
      D3DPTBLENDCAPS_DECALMASK               |
      D3DPTBLENDCAPS_MODULATEMASK            |
      D3DPTBLENDCAPS_COPY                    |
      D3DPTBLENDCAPS_ADD                     |
      0,

      // Texture AddressCaps
      D3DPTADDRESSCAPS_WRAP                  |
      D3DPTADDRESSCAPS_MIRROR                |
      D3DPTADDRESSCAPS_CLAMP                 |
      //D3DPTADDRESSCAPS_BORDER              |
      D3DPTADDRESSCAPS_INDEPENDENTUV         |
      0,
      0,                          // stippleWidth      
      0                           // stippleHeight     
    },                                                    
    DDBD_16 | DDBD_32,            // RenderBitDepth
    DDBD_16,                      // ZbufferBitDepth, Although handled by GUID_ZPIXELFORMATS, some
                                  //  apps don't follow new conventions and rely on old norms.
    0,                            // Maximum execute buffer size 
    0,                            // Maximum vertex count 
};

//-------------------------- 
// Supported texture formats
//-------------------------- 
static DDSURFACEDESC textureFormats[] = {
   // 16 bit RGB 565
   { sizeof(DDSURFACEDESC),             /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB,                         /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      16,                               /* ddpfPixelFormat.dwRGBBitCount */
      RGB565_RMASK,                     // dwRBitMask
      RGB565_GMASK,                     // dwGBitMask
      RGB565_BMASK,                     // dwBBitMask
      RGB565_AMASK                      // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // 16 bit RGB 4444
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB |  DDPF_ALPHAPIXELS,     /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      16,                               /* ddpfPixelFormat.dwRGBBitCount */
      RGB4444_RMASK,                    // dwRBitMask
      RGB4444_GMASK,                    // dwGBitMask
      RGB4444_BMASK,                    // dwBBitMask
      RGB4444_AMASK                     // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // 16 bit RGB 1555
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB | DDPF_ALPHAPIXELS,      /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      16,                               /* ddpfPixelFormat.dwRGBBitCount */
      RGB1555_RMASK,                    // dwRBitMask
      RGB1555_GMASK,                    // dwGBitMask
      RGB1555_BMASK,                    // dwBBitMask
      RGB1555_AMASK                     // alpha channel mask
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // 8 bit RGB 332
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB,                         /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      8,                                /* ddpfPixelFormat.dwRGBBitCount */
      RGB332_RMASK,                     // dwRBitMask
      RGB332_GMASK,                     // dwGBitMask
      RGB332_BMASK,                     // dwBBitMask
      RGB332_AMASK                      // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // 16 bit ARGB 8332
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB |
      DDPF_ALPHAPIXELS,                 /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      16,                               /* ddpfPixelFormat.dwRGBBitCount */
      RGB8332_RMASK,                    // dwRBitMask
      RGB8332_GMASK,                    // dwGBitMask
      RGB8332_BMASK,                    // dwBBitMask
      RGB8332_AMASK                     // alpha channel mask
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // Palettized texture - 8Bit P
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB | 
      DDPF_PALETTEINDEXED8,             /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      8,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0                                 // alpha channel mask
    },
    DDSCAPS_TEXTURE
  },
  // Alpha + Palettized texture - 16Bit AP
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB |
      DDPF_PALETTEINDEXED8 |
      DDPF_ALPHAPIXELS,                 /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      16,                               /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      ALPHA_P8_AMASK                    // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // Luminance texture - 8Bit L
  { sizeof(DDSURFACEDESC),              // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,       // dwFlags
    0,                                  // dwHeight
    0,                                  // dwWidth
    0,                                  // lPitch
    0,                                  // dwBackBufferCount
    0,                                  // dwZBufferBitDepth
    0,                                  // dwAlphaBitDepth
    0,                                  // dwReserved
    NULL,                               // lpSurface
    { 0, 0 },                           // ddckCKDestOverlay
    { 0, 0 },                           // ddckCKDestBlt
    { 0, 0 },                           // ddckCKSrcOverlay
    { 0, 0 },                           // ddckCKSrcBlt
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            // ddpfPixelFormat.dwSize
      DDPF_LUMINANCE,                   // ddpfPixelFormat.dwFlags
      0,                                // FOURCC code
      8,                                // ddpfPixelFormat.dwLuminanceBitCount
      L8_LMASK,                         // ddpfPixelFormat.dwLuminanceBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0                                 // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // Luminance Alpha texture - 8Bit AL
  { sizeof(DDSURFACEDESC),                // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,         // dwFlags
    0,                                    // dwHeight
    0,                                    // dwWidth
    0,                                    // lPitch
    0,                                    // dwBackBufferCount
    0,                                    // dwZBufferBitDepth
    0,                                    // dwAlphaBitDepth
    0,                                    // dwReserved
    NULL,                                 // lpSurface
    { 0, 0 },                             // ddckCKDestOverlay
    { 0, 0 },                             // ddckCKDestBlt
    { 0, 0 },                             // ddckCKSrcOverlay
    { 0, 0 },                             // ddckCKSrcBlt
    {                                     // ddpixelformat
      sizeof(DDPIXELFORMAT),              // ddpfPixelFormat.dwSize
      DDPF_LUMINANCE |
      DDPF_ALPHAPIXELS,                   // ddpfPixelFormat.dwFlags
      0,                                  // FOURCC code
      8,                                  // ddpfPixelFormat.dwLuminanceBitCount
      LA8_LMASK,                          // ddpfPixelFormat.dwLuminanceBitMask
      0,                                  // dwGBitMask
      0,                                  // dwBBitMask
      LA8_AMASK                           // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // Luminance + Alpha texture - 16Bit AL
  { sizeof(DDSURFACEDESC),                // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,         // dwFlags
    0,                                    // dwHeight
    0,                                    // dwWidth
    0,                                    // lPitch
    0,                                    // dwBackBufferCount
    0,                                    // dwZBufferBitDepth
    0,                                    // dwAlphaBitDepth
    0,                                    // dwReserved
    NULL,                                 // lpSurface
    { 0, 0 },                             // ddckCKDestOverlay
    { 0, 0 },                             // ddckCKDestBlt
    { 0, 0 },                             // ddckCKSrcOverlay
    { 0, 0 },                             // ddckCKSrcBlt
    {                                     // ddpixelformat
      sizeof(DDPIXELFORMAT),              // ddpfPixelFormat.dwSize
      DDPF_LUMINANCE |
      DDPF_ALPHAPIXELS,                   // ddpfPixelFormat.dwFlags
      0,                                  // FOURCC code
      16,                                 // ddpfPixelFormat.dwLuminanceBitCount
      LA16_LMASK,                         // ddpfPixelFormat.dwLuminanceBitMask
      0,                                  // dwGBitMask
      0,                                  // dwBBitMask
      LA16_AMASK                          // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // Alpha - 8Bit
  { sizeof(DDSURFACEDESC),                // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,         // dwFlags
    0,                                    // dwHeight
    0,                                    // dwWidth
    0,                                    // lPitch
    0,                                    // dwBackBufferCount
    0,                                    // dwZBufferBitDepth
    0,                                    // dwAlphaBitDepth
    0,                                    // dwReserved
    NULL,                                 // lpSurface
    { 0, 0 },                             // ddckCKDestOverlay
    { 0, 0 },                             // ddckCKDestBlt
    { 0, 0 },                             // ddckCKSrcOverlay
    { 0, 0 },                             // ddckCKSrcBlt
    {                                     // ddpixelformat
      sizeof(DDPIXELFORMAT),              // ddpfPixelFormat.dwSize
      DDPF_ALPHAPIXELS,                   // ddpfPixelFormat.dwFlags
      0,                                  // FOURCC code
      8,                                  // ddpfPixelFormat.dwLuminanceBitCount
      0,                                  // dwRBitMask
      0,                                  // dwGBitMask
      0,                                  // dwBBitMask
      A8_AMASK                            // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // UV88 - 16bit
  { sizeof(DDSURFACEDESC),                // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,         // dwFlags
    0,                                    // dwHeight
    0,                                    // dwWidth
    0,                                    // lPitch
    0,                                    // dwBackBufferCount
    0,                                    // dwZBufferBitDepth
    0,                                    // dwAlphaBitDepth
    0,                                    // dwReserved
    NULL,                                 // lpSurface
    { 0, 0 },                             // ddckCKDestOverlay
    { 0, 0 },                             // ddckCKDestBlt
    { 0, 0 },                             // ddckCKSrcOverlay
    { 0, 0 },                             // ddckCKSrcBlt
    {                                     // ddpixelformat
      sizeof(DDPIXELFORMAT),              // ddpfPixelFormat.dwSize
      DDPF_BUMPDUDV,                      // ddpfPixelFormat.dwFlags
      0,                                  // FOURCC code
      16,                                 // ddpfPixelFormat.dwBumpBitCount
      UV88_UMASK,                         // ddpfPixelFormat.dwBumpDuBitMask
      UV88_VMASK,                         // ddpfPixelFormat.dwBumpDvBitMask
      0,                                  // ddpfPixelFormat.dwBumpLuminanceBitMask
      0                                   // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // UVL556 - 16bit
  { sizeof(DDSURFACEDESC),                // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,         // dwFlags
    0,                                    // dwHeight
    0,                                    // dwWidth
    0,                                    // lPitch
    0,                                    // dwBackBufferCount
    0,                                    // dwZBufferBitDepth
    0,                                    // dwAlphaBitDepth
    0,                                    // dwReserved
    NULL,                                 // lpSurface
    { 0, 0 },                             // ddckCKDestOverlay
    { 0, 0 },                             // ddckCKDestBlt
    { 0, 0 },                             // ddckCKSrcOverlay
    { 0, 0 },                             // ddckCKSrcBlt
    {                                     // ddpixelformat
      sizeof(DDPIXELFORMAT),              // ddpfPixelFormat.dwSize
      DDPF_BUMPDUDV |
      DDPF_BUMPLUMINANCE,                 // ddpfPixelFormat.dwFlags
      0,                                  // FOURCC code
      16,                                 // ddpfPixelFormat.dwBumpBitCount
      UVL556_UMASK,                       // ddpfPixelFormat.dwBumpDuBitMask
      UVL556_VMASK,                       // ddpfPixelFormat.dwBumpDvBitMask
      UVL556_LMASK,                       // ddpfPixelFormat.dwBumpLuminanceBitMask
      0                                   // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // UVL888 - 24bit
  { sizeof(DDSURFACEDESC),                // dwSize
    DDSD_CAPS | DDSD_PIXELFORMAT,         // dwFlags
    0,                                    // dwHeight
    0,                                    // dwWidth
    0,                                    // lPitch
    0,                                    // dwBackBufferCount
    0,                                    // dwZBufferBitDepth
    0,                                    // dwAlphaBitDepth
    0,                                    // dwReserved
    NULL,                                 // lpSurface
    { 0, 0 },                             // ddckCKDestOverlay
    { 0, 0 },                             // ddckCKDestBlt
    { 0, 0 },                             // ddckCKSrcOverlay
    { 0, 0 },                             // ddckCKSrcBlt
    {                                     // ddpixelformat
      sizeof(DDPIXELFORMAT),              // ddpfPixelFormat.dwSize
      DDPF_BUMPDUDV |
      DDPF_BUMPLUMINANCE,                 // ddpfPixelFormat.dwFlags
      0,                                  // FOURCC code
      24,                                 // ddpfPixelFormat.dwBumpBitCount
      UVL888_UMASK,                       // ddpfPixelFormat.dwBumpDuBitMask
      UVL888_VMASK,                       // ddpfPixelFormat.dwBumpDvBitMask
      UVL888_LMASK,                       // ddpfPixelFormat.dwBumpLuminanceBitMask
      0                                   // alpha channel mask
    },
    DDSCAPS_TEXTURE 
  },
  // 32 bit ARGB 8888
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_RGB |  DDPF_ALPHAPIXELS,     /* ddpfPixelFormat.dwFlags */
      0,                                /* FOURCC code */
      32,                               /* ddpfPixelFormat.dwRGBBitCount */
      RGB8888_RMASK,                    // dwRBitMask
      RGB8888_GMASK,                    // dwGBitMask
      RGB8888_BMASK,                    // dwBBitMask
      RGB8888_AMASK                     // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // dxt1 compressed texture
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_FOURCC,                      /* ddpfPixelFormat.dwFlags */
      FOURCC_DXT1,                      /* FOURCC code */
      4,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0,                                // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // dxt2 compressed texture
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_FOURCC,                      // ddpfPixelFormat.dwFlag */
      FOURCC_DXT2,                      /* FOURCC code */
      8,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0,                                // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // dxt3 compressed texture
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_FOURCC,                      /* ddpfPixelFormat.dwFlags */
      FOURCC_DXT3,                      /* FOURCC code */
      8,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0,                                // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // dxt4 compressed texture
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_FOURCC,                      /* ddpfPixelFormat.dwFlags */
      FOURCC_DXT4,                      /* FOURCC code */
      8,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0,                                // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // dxt5 compressed texture
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_FOURCC,                      /* ddpfPixelFormat.dwFlags */
      FOURCC_DXT5,                      /* FOURCC code */
      8,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0,                                // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  },
  // fxt1 compressed texture
  { sizeof(DDSURFACEDESC),              /* dwSize */
    DDSD_CAPS | DDSD_PIXELFORMAT,       /* dwFlags */
    0,                                  /* dwHeight */
    0,                                  /* dwWidth */
    0,                                  /* lPitch */
    0,                                  /* dwBackBufferCount */
    0,                                  /* dwZBufferBitDepth */
    0,                                  /* dwAlphaBitDepth */
    0,                                  /* dwReserved */
    NULL,                               /* lpSurface */
    { 0, 0 },                           /* ddckCKDestOverlay */
    { 0, 0 },                           /* ddckCKDestBlt */
    { 0, 0 },                           /* ddckCKSrcOverlay */
    { 0, 0 },                           /* ddckCKSrcBlt */
    {                                   // ddpixelformat
      sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
      DDPF_FOURCC,                      /* ddpfPixelFormat.dwFlags */
      MAKEFOURCC('F','X','T','1'),      /* FOURCC code */
      4,                                /* ddpfPixelFormat.dwRGBBitCount */
      0,                                // dwRBitMask
      0,                                // dwGBitMask
      0,                                // dwBBitMask
      0,                                // mask for alpha channel
    },
    DDSCAPS_TEXTURE                     // ddscaps.dwCaps 
  }
};

#if (DIRECT3D_VERSION >= 0x0800)

static DDSURFACEDESC textureFormats8[] = {

// W2DW combines two WORD values into a dword for setting the DX8
// field, MultiSampleCaps.wFlipMSTypes and .wBltMSTypes.  The problem
// is that a static initializer can't be used to init these values
// since they are members of a struct, as part of a union and there's
// no C notation to specify which union's member it is at this point.
// Note the WORDs are swapped for endianness, and this is compiler dependency.
#define W2DW(lo,hi)    (((WORD)(hi)<<16)|(WORD)(lo))

   // Define pixel formats using proper DX8 notation.
   {                                        // 16 bit RGB 565
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_R5G6B5,                   // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           D3DFORMAT_OP_OFFSCREEN_RENDERTARGET |
           D3DFORMAT_OP_DISPLAYMODE |
           D3DFORMAT_OP_3DACCELERATION |
           0,
           // MultiSampleCaps aka dwGBitMask
           W2DW((1 << (D3DMULTISAMPLE_4_SAMPLES - 1)),(1 << (D3DMULTISAMPLE_4_SAMPLES - 1))), 
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // 16 bit ARGB 4444
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A4R4G4B4,                 // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // 16 bit A1R5G5B5
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A1R5G5B5,                 // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // 8 bit RGB 332
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_R3G3B2,                   // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // 16 bit ARGB 8332
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A8R3G3B2,                 // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // 8 bit palettized
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_P8,                       // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // 16 bit AP 88
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A8P8,                   // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // Luminance 8
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_L8,                       // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // Luminance w/ Alpha 88
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A8L8,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // Alpha 8
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A8,                       // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // UV88
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_V8U8,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // UVL556
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_L6V5U5,                   // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // UVL888
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_X8L8V8U8,                 // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // ARGB 8888
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_A8R8G8B8,                 // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           D3DFORMAT_OP_OFFSCREEN_RENDERTARGET |
           0,
           // MultiSampleCaps aka dwGBitMask
           W2DW((1 << (D3DMULTISAMPLE_4_SAMPLES - 1)),(1 << (D3DMULTISAMPLE_4_SAMPLES - 1))), 
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // xRGB 8888
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_X8R8G8B8,                 // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_DISPLAYMODE |
           D3DFORMAT_OP_OFFSCREEN_RENDERTARGET |
           D3DFORMAT_OP_3DACCELERATION |
           0,
           // MultiSampleCaps aka dwGBitMask
           W2DW((1 << (D3DMULTISAMPLE_4_SAMPLES - 1)),(1 << (D3DMULTISAMPLE_4_SAMPLES - 1))),  
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // DXT1
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_DXT1,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // DXT2
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_DXT2,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // DXT3
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_DXT3,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // DXT4
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_DXT4,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // DXT5
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_DXT5,                     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // FXT1
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           MAKEFOURCC('F','X','T','1'),     // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_TEXTURE |
           0,
           W2DW(0,0),                       // MultiSampleCaps aka dwGBitMask
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   // Define Depth buffers and Stencil-capable surfaces:
   {                                        // D16
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_D16,                      // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_ZSTENCIL |
           0,
           // MultiSampleCaps aka dwGBitMask
           W2DW((1 << (D3DMULTISAMPLE_4_SAMPLES - 1)),(1 << (D3DMULTISAMPLE_4_SAMPLES - 1))), 
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },
   {                                        // D24S8  24 Z + 8 Stencil
       sizeof(DDSURFACEDESC),
       DDSD_CAPS | DDSD_PIXELFORMAT,
       0, 0, 0, 0, 0, 0, 0,                 // H, W, pitch, backbuffer, z-bit, a-bit, reserved
       NULL, {0,0}, {0,0}, {0,0}, {0,0},    // lpSurface, ckdestoverlay,ckdestblt,srcoverlay,srcblt
       {                                    // DDPIXELFORMAT...
           sizeof(DDPIXELFORMAT),           // dwSize
           DDPF_D3DFORMAT,                  // DX8 notation
           D3DFMT_D24S8,                    // FOURCC code
           0,                               // dwPrivateFormatBitCount aka dwRGBBitCount
           // dwOperations:      aka dwRBitMask
           D3DFORMAT_OP_ZSTENCIL |
           0,
           // MultiSampleCaps aka dwGBitMask
           W2DW((1 << (D3DMULTISAMPLE_4_SAMPLES - 1)),(1 << (D3DMULTISAMPLE_4_SAMPLES - 1))), 
           0,                               // Reserved1, aka dwBBitMask
           0,                               // Reserved2, aka dwRGBAlphaBitMask
       },
       DDSCAPS_TEXTURE                      // ddscaps.dwCaps
   },

#undef W2DW
};
#endif // DIRECT3D_VERSION >= 0x0800


#if (DIRECT3D_VERSION >= 0x0800)
/*----------------
 * Simply return the number of surface types we support for DX8.
 */
DWORD d3GetFormatCount(void)
{
    return sizeof(textureFormats8)/sizeof(textureFormats8[0]);
}

/*
 * Return the format type for this texture descriptor.
 */
DDPIXELFORMAT* d3GetFormat (DWORD idx)
{
    return &(textureFormats8[idx].ddpfPixelFormat);
}
#endif // DX8

//-------------------------------------------------------------------
// Functions used to instantiate the 3D portion of the DirectDraw HAL
//-------------------------------------------------------------------
d3Global mytempd3global;
BOOL __stdcall D3DHALCreateDriver(NT9XDEVICEDATA *ppdev,
                                  LPD3DHAL_GLOBALDRIVERDATA* lplpGlobal,
                                  LPD3DHAL_CALLBACKS* lplpHALCallbacks)
{

  _D3(d3d).dwSize = sizeof(D3DHAL_GLOBALDRIVERDATA);

  // these caps are for vertex buffer support
  if (IS_SAGE_ACTIVE) {
    caps.dwDevCaps |= D3DDEVCAPS_EXECUTEVIDEOMEMORY;
    caps.dwDevCaps |= D3DDEVCAPS_TLVERTEXVIDEOMEMORY;
    caps.dwDevCaps |= D3DDEVCAPS_HWTRANSFORMANDLIGHT;
    caps.dwFlags |= D3DDD_TRANSFORMCAPS | D3DDD_LIGHTINGCAPS | D3DDD_BCLIPPING;
    caps.dtcTransformCaps.dwCaps |= D3DTRANSFORMCAPS_CLIP;
    caps.bClipping = TRUE;
    caps.dlcLightingCaps.dwCaps = D3DLIGHTCAPS_POINT | D3DLIGHTCAPS_SPOT | D3DLIGHTCAPS_DIRECTIONAL;
    caps.dlcLightingCaps.dwLightingModel = D3DLIGHTINGMODEL_RGB; 
    caps.dlcLightingCaps.dwNumLights = GE_MAX_ACTIVE_LIGHTS;
  }

  _D3(d3d).hwCaps = caps;
  _D3(d3d).dwNumVertices     = 0;     // let D3D allocate its own local buffer
  _D3(d3d).dwNumClipVertices = 0;

  // Declare texture formats
  _D3(d3d).dwNumTextureFormats = sizeof(textureFormats) / sizeof(textureFormats[0]);
  _D3(d3d).lpTextureFormats = &textureFormats[0];

#if !RC_LINKED_LIST
  if (NULL == _D3(contexts))
  {
      if (!CONTEXT_INIT_ARRAY(ppdev))
         return FALSE;
  }
#endif

#if defined( DEBUG )
  // Get the debug level output from the win.ini file
  if( NULL != GETENV( ("D3D_DebugLevel") ) )
  {
    char *str;
    str = GETENV( "D3D_DebugLevel" );
    setDebugLevel( str );
  }
#endif

  _D3( cubeMapSignFix ) = 0;   // Support testing of proposed fix
  _D3( csContextID ) = 0;      // Set the context to zero (illegal by CS rules) for initial value.

  // Pixel centers are a registry option, texel centers are no longer used.

  if( NULL == GETENV( ("SST2_PIXELCENTER") ) )
  {
    _D3( pixelCenter ) = 1;   // enable pixelcenter offsets by default. [bpb]
    _D3( texelCenter ) = 0;
  }
  else
  {
    _D3( pixelCenter ) = ATOI( GETENV( ("SST2_PIXELCENTER") ) );
    _D3( texelCenter ) = 0;
  }

#if defined( FLAVOR_PROFILE )
  renderTypes = 0;
#endif
        
  if (IS_SAGE_ACTIVE)
      geInit(ppdev);

  // Return callbacks and global data area
  *lplpGlobal = (LPD3DHAL_GLOBALDRIVERDATA)&_D3G;
  *lplpHALCallbacks = &myD3DHALCallbacks;
  return TRUE;
}
