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
** /devel/sst2/Win95/dx/d3d/d6mt.c
**
** $Log: 
**  52   3dfx      1.51        11/30/00 Brent Burton    3 minor debug level 
**       changes.
**  51   3dfx      1.50        11/7/00  Brent Burton    Removed 1 line that set 
**       wrap[].  wrap[] is now set directly in d3rstate.c
**  50   3dfx      1.49        10/23/00 Brent Burton    More DX8-specific 
**       changes.
**  49   3dfx      1.48        10/11/00 Brent Burton    DX8 code integration.  
**       Changes to headers, new code.
**  48   3dfx      1.47        9/14/00  Michel Conrad   Fix bogus failures in 
**       Validate TSS. Make sure block linear is set in NPT for DXT formats. 
**       Sanity check alpha replicate selections before changing CCU/TCU values.
**  47   3dfx      1.46        9/6/00   Don Fowler      Adjusted texture filter 
**       to use trillinear with mipmaps to pass dct and winbench.    Altered the
**       code to choose between the Renderstate and the TextureStageState LOD 
**       Bias  
**  46   3dfx      1.45        9/6/00   Michel Conrad   Change to texture 
**       coordinate dimension variables -- needed for cubemapping. Uncomment 
**       return code so validate texture stage state can return errors.
**  45   3dfx      1.44        8/22/00  Brian Danielson Changed the renderstate 
**       management from being tied to the current render context to being a 
**       global concept, now accessable from DD.
**  44   3dfx      1.43        8/17/00  Don Fowler      Altered previous 
**       checking that used hardcoded values to use defines... 
**  43   3dfx      1.42        8/3/00   Don Fowler      Added code to handle 
**       legacy ALPHA MODULATE state. Alpha modulate will now get alpha 
**       information from the iterator if there is no alpha information in the 
**       pixel format.    Added support for texture SRC colorkeying    Fixed Z 
**       bias to support Rampage 24 bit bias and added an arbitrary multiplier 
**       to "fixup" the value of 0-15 passed by ddraw. The number I chose was 
**       entirely arbitrary and was just enough to get the tests to pass.    
**       Coupled the AlphaDepthTest with the AlphaColorTest because of a bug in 
**       CSIM and possibly in the hardware.
**  42   3dfx      1.41        7/24/00  Brian Danielson Changes to implement 
**       renderstate and shadow register management.
**  41   3dfx      1.40        7/10/00  Michel Conrad   Adding cubemap support. 
**       Clean up of filter mode (some in progress work of linear MIP nearest 
**       problem).
**  40   3dfx      1.39        6/16/00  Evan Leland     Use new texture palette 
**       manipulation routines during setupTextureStage (better 
**       readability/modularity)
**  39   3dfx      1.38        6/15/00  Michel Conrad   Remove some dead code.
**  38   3dfx      1.37        6/13/00  Evan Leland     Fix for PRS 14436. 
**       Problem was our mis-handling of palettized alpha texture format. Now we
**       check the palette and if it has alpha we change the texture format to 
**       ALPHA_P8_RGB.
**  37   3dfx      1.36        5/22/00  Evan Leland     removed dx7-specific 
**       ifdefs and code targeted to the pre-dx7 driver
**  36   3dfx      1.35        5/11/00  Evan Leland     dx7 structure cleanup 
**       effort complete
**  35   3dfx      1.34        3/8/00   Brent Burton    Added extra-texture UV 
**       support.  Fixed the VTA configuration for bumpmapping (all) to properly
**       handle signed texel values.
**  34   3dfx      1.33        3/6/00   Brent Burton    Rearranged where TS[0] 
**       is being modified.
**  33   3dfx      1.32        3/6/00   Brent Burton    Fixed conversion of 
**       CURRENT argument to DIFFUSE at stage 0.  The wrong copy was modified 
**       which only affected bumpmapping.
**  32   3dfx      1.31        2/29/00  Brian Danielson Fixed texture coordinate
**       indexing for FVFs.
**  31   3dfx      1.30        2/28/00  Brian Danielson Fixed FvFOffsetTable 
**       Contents and FVF work for DX7.
**  30   3dfx      1.29        2/28/00  Brian Danielson Fix for DX7 where added 
**       var to incorrect place in d3global.h.
**  29   3dfx      1.28        2/25/00  Brian Danielson Fixed problem with 
**       texture coordinate dimensions for DX7.
**  28   3dfx      1.27        2/15/00  Brent Burton    Removed OUT_CLAMP on the
**       TCU.  The CCU is still clamped like refrast.  This change fixes 
**       DOTPRODUCT3.
**  27   3dfx      1.26        2/14/00  Brent Burton    Final UV/UVL bumpmap 
**       support (lots of changes).  Rearranged the order of functions in this 
**       file a bit.
**  26   3dfx      1.25        2/7/00   Brent Burton    Removed change comments 
**       from top.  Rearranged some pipeline configuration code from 
**       setupTexturing() into add_extra_stages().  Changed the reporting 
**       dumpStages() does to be more useful.
**  25   3dfx      1.24        2/7/00   Evan Leland     setupTextureStage now 
**       checks for NULL txtrDesc
**  24   3dfx      1.23        1/28/00  Evan Leland     more dx7 upgrade work
**  23   3dfx      1.22        1/27/00  Evan Leland     DX7 changes
**  22   3dfx      1.21        1/25/00  Evan Leland     updates for dx7
**  21   3dfx      1.20        1/25/00  Evan Leland     part of 
**       txtrCreateSurface reorg, texture struct integration, DX7 bring-up
**  20   3dfx      1.19        1/10/00  Michel Conrad   Adding comment that last
**       checkin include Brent's fix for U addressing problem.
**  19   3dfx      1.18        1/10/00  Michel Conrad   Adjust to new d3global 
**       structure for hw regs. Really clear taMode to avoid sticky block linear
**       state problem. Better select of mag filter for trilinear and 
**       anisotropic.
**  18   3dfx      1.17        12/17/99 Brian Danielson Added FVF, Projected 
**       Textures, line&point VTA loop fix, PRS 11233 & 11290 fix, disbaled DX7 
**       Clear and remapped to ddiClear2, cleanups.
**  17   3dfx      1.16        11/11/99 Brent Burton    Fixed specular color 
**       when texturing is disabled.
**  16   3dfx      1.15        11/11/99 Miles Smith     Fixing w-buffer support 
**       - whql w-buffer tests were all failing, basically because it wasn't 
**       turned on at all.
**  15   3dfx      1.14        10/19/99 Evan Leland     minor cleanup prior to 
**       enabling npt blt download
**  14   3dfx      1.13        10/8/99  Evan Leland     fixes problem with 
**       palettized textures
**  13   3dfx      1.12        10/7/99  Brent Burton    Fix for bump size.  
**       Multiply 2x2 matrix coefficients by 2.0f to get an extra S,T_SHIFT.
**  12   3dfx      1.11        10/5/99  Brent Burton    Added a debug level and 
**       message parameters to dumpStages() for better output.  Added call to 
**       dumpStages() in ddiValidateTextureStageState().  Adjusted the 
**       conditions in setup_suParamMask() to look for more color and alpha 
**       operations that require diffuse color, etc.  Fixed a bug at stage 0 
**       when BLENDCURRENTALPHA is used.
**  11   3dfx      1.10        10/1/99  Brent Burton    Removed some dead code. 
**       Turned on OUT_CLAMP for TCU, TCA, CCU, CCA for each stage.
**  10   3dfx      1.9         9/29/99  Brent Burton    Fixed stage-counting 
**       condition to fail if either colorArg uses a texture input  but no 
**       alphareplicate when the textureHandle is zero (disabled).
**  9    3dfx      1.8         9/28/99  Brent Burton    Added function 
**       countTextureStages() that uses the same method to count stages as 
**       refrast.  Renamed prc->texture to prc->dx5texture, and cleaned up the 
**       usage.
**  8    3dfx      1.7         9/27/99  Brent Burton    Added code to set C1 to 
**       2.0 or 4.0 for color and alpha Mod2x, Mod4x, DotProduct3 and 
**       AddSigned2x blend ops.  This really fixes these.
**  7    3dfx      1.6         9/25/99  Brent Burton    Fixes Alpha and Color 
**       blend operations: Modulate2X, Modulate4X.
**  6    3dfx      1.5         9/24/99  Miles Smith     Added a fix for fog 
**       modes (doesn' reset sst_su_q bit in mode ) - mls
**  5    3dfx      1.4         9/21/99  Brent Burton    Changed all texture op 
**       argument comparisons to use the D3DTA_IS and D3DTA_ISNOT macros.  Added
**       necessary extra alpha-state support.
**  4    3dfx      1.3         9/15/99  Brent Burton    Added log back.
**  3    3dfx      1.2         9/15/99  Brent Burton    Moved setting of 
**       prc->suParamMask until after the main VTA configuration block.  
**       suParamMask is set from D3D state which may be slightly altered during 
**       VTA configuration.
**  2    3dfx      1.1         9/13/99  Philip Zheng    
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
*/

#include <ddrawi.h>
#include <d3dhal.h>
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "d3npt.h"
#include "d6fvf.h"

// DEBUG Static function prototypes
#if defined( DEBUG )
void checkTextureOp( RC *pRc );
static void dumpStages(RC *pRc, int ns, int level, char *msg);
#endif


//-------------------------------------------------------------------

#define ARG_VALID   0
#define TCU_OP      1
#define CCU_OP      2
#define TCA_OP      1
#define CCA_OP      2


typedef struct
{
  BOOL  enabled;
  DWORD arg[16][3];          // valid, tcc/tca, ccc/cca arg1 & arg2
  DWORD complement_arg1[3];  // valid, tcc/tca, ccc/cca complement mask
  DWORD complement_arg2[3];  // valid, tcc/tca, ccc/cca complement mask
  DWORD areplicate_arg1[3];  // valid, tcc/tca, ccc/cca alpha replicate mask
  DWORD areplicate_arg2[3];  // valid, tcc/tca, ccc/cca alpha replicate mask
  DWORD increment_arg1[3];   // valid, tcc/tca, ccc/cca increment PREV -> ITER
  DWORD increment_arg2[3];   // valid, tcc/tca, ccc/cca increment PREV -> ITER
} TEXTUREOP, *LPTEXTUREOP;

typedef TEXTUREOP STAGEOP[25];

#define COLOROP  0
#define ALPHAOP  1

#include "d6mt.h"

// Note stageOp[0] is the "last" stage out of the VTA

LPTEXTUREOP stageOp[8][2] =
{
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },
  { stageOp_color, stageOp_alpha },  
};

// ----------------------------------------------------------------
// Count the number of active DX6 stages, or return 1 if legacy texturing (<=Dx5).
// Returns:  * integer count of stages
//           * *haveBumpMap = 1  if bumpmapping is used

static int countTextureStages (RC *pRc, BOOL *haveBumpMap)
{
    int ns = 0;

    *haveBumpMap = FALSE;

    for (ns=0; ns<D3DHAL_TSS_MAXSTAGES; ns++)
    {
        // check for disabled stage:
        if (TS[ns].colorOp == D3DTOP_DISABLE)
            break;

        // check for incorrectly enabled stage:
        if (TS[ns].textureHandle == 0 &&
            // Check if alpharep is enabled when colorarg is texture
            ((D3DTA_IS(TS[ns].colorArg1, D3DTA_TEXTURE) && !(TS[ns].colorArg1 & D3DTA_ALPHAREPLICATE)) ||
             (D3DTA_IS(TS[ns].colorArg2, D3DTA_TEXTURE) && !(TS[ns].colorArg2 & D3DTA_ALPHAREPLICATE)) ||
             // If alpha is enabled make sure those don't use texture
             (TS[ns].alphaOp != D3DTOP_DISABLE            &&
              (D3DTA_IS(TS[ns].alphaArg1, D3DTA_TEXTURE)  ||
               D3DTA_IS(TS[ns].alphaArg2, D3DTA_TEXTURE)))
             ))
        {
            break;
        }

        if (TS[ns].colorOp == D3DTOP_BUMPENVMAP ||
            TS[ns].colorOp == D3DTOP_BUMPENVMAPLUMINANCE)
            *haveBumpMap = TRUE;
    }

    return ns;
}

//-------------------------------------------------------------------

DWORD  __stdcall ddiValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA lpd )
{
  SETUP_PPDEV(lpd->dwhContext)
  RC           *pRc;
  DWORD        opOffset, argOffset, ns = 0, i;
  HRESULT      r1val = D3D_OK;
  LPTEXTUREOP  lpStage;
  BOOL         haveBumpMap = 0;             // any bumpmapping used?

    
  D3D_ENTRY( "ddiValidatetextureStageState" );

#ifdef FXTRACE
  if (CONTEXT_VALIDATE(lpd->dwhContext)) 
  {
    D3DPRINT( 0, "ValidateTextureStageState bad context = 0x08lx", lpd->dwhContext );
    lpd->ddrval = D3DHAL_CONTEXT_BAD;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
 
  pRc = CONTEXT_PTR(lpd->dwhContext);

  ns = countTextureStages (pRc, &haveBumpMap);
  
  // If all stages are disabled setup up for flat or gouraud shading.
  if( ns == 0 )
  {
    lpd->ddrval = D3D_OK;
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

#if defined(DEBUG)
  dumpStages (pRc, ns, DL_TSS_VALIDATE, "Validation");
#endif

  for( i = 0; i < ns; i++ )
  {
    // Pointer to stage operation table
    // lpStage = stageOp[ns][i + COLOROP];
    lpStage = stageOp[ns-i-1][COLOROP];

    // Check the color operation is supported
    opOffset = TS[i].colorOp;

    if( lpStage[opOffset].enabled )
    {
      argOffset = (((TS[i].colorArg1 & D3DTA_SELECTMASK) << 2)
                   |(TS[i].colorArg2 & D3DTA_SELECTMASK));
      
      // Now check the arguments
      if( !lpStage[opOffset].arg[argOffset][ARG_VALID] )
      {
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
      }
    }
    else
    {
      r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
      break;
    }
    
    // Pointer to stage operation table
    // lpStage = stageOp[ns][i + ALPHAOP];
    lpStage = stageOp[ns-i-1][ALPHAOP];

    
    // Check the alpha operation
    opOffset = TS[i].alphaOp;

    if( lpStage[opOffset].enabled )
    {
      // Calculate the alpha arg offset
      argOffset = (((TS[i].alphaArg1 & D3DTA_SELECTMASK) << 2)
                   |(TS[i].alphaArg2 & D3DTA_SELECTMASK));
      
      // Now check the arguments
      if( !lpStage[opOffset].arg[argOffset][ARG_VALID] )
      {
        r1val = D3DERR_UNSUPPORTEDALPHAARG;
        break;
      }
    }
    else
    {
      r1val = D3DERR_UNSUPPORTEDALPHAOPERATION;
      break;
    }

    // If we are using texture make check the handle
    if( (TS[i].colorOp == D3DTOP_SELECTARG1 ) &&
        (D3DTA_IS( TS[i].colorArg1, D3DTA_TEXTURE ) || 
         D3DTA_IS( TS[i].alphaArg1, D3DTA_TEXTURE ))
      )
    {
      // for now just check the texture exists ...
      if (!(TS[i].textureHandle && 
           TXTRHNDL_TO_TXHNDLSTRUCT(pRc, TS[i].textureHandle)->flags & HandleInUse))
      {    
          //r1val = D3DERR_TEXTURE_NO_SUPPORT;
          r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
          break;        
      }        
    }

    // If we are using texture make check the handle
    if( (TS[i].colorOp == D3DTOP_SELECTARG2 ) &&
        (D3DTA_IS( TS[i].colorArg2, D3DTA_TEXTURE ) || 
         D3DTA_IS( TS[i].alphaArg2, D3DTA_TEXTURE ))
      )
    {
      // for now just check the texture exists ...
      if (!(TS[i].textureHandle && 
           TXTRHNDL_TO_TXHNDLSTRUCT(pRc, TS[i].textureHandle)->flags & HandleInUse))
      {    
          //r1val = D3DERR_TEXTURE_NO_SUPPORT;
          r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
          break;        
      }        
    }
  } // end for ns loop

  // The last stage (ns-1) cannot be a bump map because it
  // needs a subsequent texture to perturb.
  // Further, make sure that we have room for extra stages
  // if bumpmapping is enabled.
  if (TS[ns-1].colorOp == D3DTOP_BUMPENVMAP          ||
      TS[ns-1].colorOp == D3DTOP_BUMPENVMAPLUMINANCE ||
      (haveBumpMap && ns == 8))
  {
      r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
  }
  
  // Now set our return code
  lpd->ddrval = r1val;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}


// ----------------------------------------------------------------
// EXTRASTAGEINFO contains information that describes additional
// stages inserted into the application-specified pipeline.

enum BUMPTYPE {BUMPMAP_NONE=0, BUMPMAP_UV_ON=1, BUMPMAP_UVL_ON=2};
typedef struct EXTRASTAGEINFO {
    FxU8 bump_enabled;                  // Is bumpmapping enabled and what type?
    FxU8 load_reg;                      // D3D stage that loads CDR (output of CCU)
    FxU8 read_reg;                      // D3D stage that reads CDR (in to TCU/CCU)
    FxU8 UV_stage_1;                    // 1st UV stage for UV/UVL bumpmapping. (always 0?)
    FxU8 UV_stage_2;                    // 2nd UV stage, only for UVL bumpmapping.
    FxU8 env_stage;                     // Stage for perturbed environment map.
    FxU8 L_stage;                       // L stage, only for UVL bumpmapping.
    FxU8 op_stage;                      // extra OP stage, for any reordered B.M. pipeline
    FxU8 need_extra_stage;              // Is the extra op_stage (and CDR) needed?
} EXTRASTAGEINFO;


//-------------------------------------------------------------------
// setup_suParamMask()
// Go through the D3D stages and set the parameters needed
// by each stage.  Return the new register value.
// NOTE: fields of suParamMask are set in VTA indexing order.

DWORD setup_suParamMask (RC* pRc, DWORD numstages, EXTRASTAGEINFO *esinfo)
{
    static DWORD spm_q[8] = {SST_SU_Q0, SST_SU_Q1, SST_SU_Q2, SST_SU_Q3, 
                             SST_SU_Q4, SST_SU_Q5, SST_SU_Q6, SST_SU_Q7};
    DWORD i, vi;                        // i=D3D index, vi=VTA index
    DWORD reg = 0;
    DWORD any_set = 0;                  // have any bits been set?
    TEXTURESTAGESTATE *ts;

    for (i=0, vi=numstages-1;  i<numstages;  i++, vi--)
    {
        ts = &(TS[i]);

#define ARG_IS_USED(arg) \
           ( (ts->colorOp != D3DTOP_DISABLE && \
              ((D3DTA_IS(ts->colorArg1,(arg)) && ts->colorOp != D3DTOP_SELECTARG2) || \
               (D3DTA_IS(ts->colorArg2,(arg)) && ts->colorOp != D3DTOP_SELECTARG1))) \
             || \
              ((D3DTA_IS(ts->alphaArg1,(arg)) && ts->alphaOp != D3DTOP_SELECTARG2) || \
               (D3DTA_IS(ts->alphaArg2,(arg)) && ts->alphaOp != D3DTOP_SELECTARG1)) )

        any_set = 0;
        if (ARG_IS_USED(D3DTA_DIFFUSE)
            || (ts->alphaOp == D3DTOP_DISABLE) // send ARGB even if alpha is disabled...
            || (ts->alphaOp == D3DTOP_BLENDDIFFUSEALPHA) // or alpha blending needs it...
            || (ts->colorOp == D3DTOP_BLENDDIFFUSEALPHA)
            )
        {
            reg |= pRc->su_parammask_rgba_flags[vi];
            any_set=1;
        }
        if (ARG_IS_USED(D3DTA_TEXTURE)
            || (ts->colorOp == D3DTOP_BLENDTEXTUREALPHA)
            || (ts->colorOp == D3DTOP_BLENDTEXTUREALPHAPM)
            || (ts->alphaOp == D3DTOP_BLENDTEXTUREALPHA)
            || (ts->alphaOp == D3DTOP_BLENDTEXTUREALPHAPM)
            )
        {
            reg |= pRc->su_parammask_tmu_flags[vi];
            any_set=1;
        }

        if (!any_set)
        {
            // Mark W-P said that Glide uses Q as a dummy param, but we'll
            // use RGBA and pass that as a dummy, since it may not be
            // selected for use by the VTA anyway.
            reg |= pRc->su_parammask_rgba_flags[vi];
        }

        if (esinfo && i == esinfo->read_reg)
        {
            // This was the extra stage added to handle bumpmapping and is
            // a special case.
            DWORD mask = 0x0f << (vi*4);
            reg = (reg & ~mask) | pRc->su_parammask_rgba_flags[vi];
        }
        // Add in Q value into "su_parammask" if transform flags indicate it.
        if ( ts->textureTransformFlags == (D3DTTFF_PROJECTED | D3DTTFF_COUNT3) )
        {
            reg |= pRc->su_parammask_q_flags[vi];
        }
#ifdef CUBEMAP
        // Assume 3-coordinate textures without transforms are cubemaps.
        if ( (ts->textureTransformFlags == D3DTTFF_DISABLE)
             && (pRc->tCoordDimension[i] == 3))
        {
            reg |= pRc->su_parammask_q_flags[vi];
        } 

        // Treat 4-coordinate textures as projected cubemaps. Can't fail this
        // so ingnore the Q in d3d's {U, V, W, Q}. Hhere sst2 q maps to d3d W.
        if ( (ts->textureTransformFlags == (D3DTTFF_PROJECTED | D3DTTFF_COUNT4))
             && (pRc->tCoordDimension[i] == 4)) 
        {
            #ifdef DEBUG
            _asm int 3  // Didn't expect this to ever happen....
            #endif
            reg |= pRc->su_parammask_q_flags[vi];
        } 
#endif

    } // for stage

    if (numstages == 0)                 // No texturing
    {
        // Primitive routines use packed color so make sure RGB0 and A0 are
        // set.  This is consistent with the fact that when texturing is
        // disabled, diffuse color is used.
        reg = SST_SU_RGB0 | SST_SU_A0;
    }

    return reg;
} // setup_suParamMask()


//-------------------------------------------------------------------
// setColorConstant() - given two float values, convert to s.4.8
// format, preserving the sign, and pack for storage into constant
// color registers (AR0, GB0, AR1, GB1)
// No side effects.

DWORD setColorConstant(float alphagreen, float redblue)
{
#define LIMIT_RANGE(x)		(MIN(4095, MAX(-4096, x)))
    FxI32 ag, rb;                       // must be signed int32's

    ag = 0x1fff & (ddftol( alphagreen * 256L));
    rb = 0x1fff & (ddftol( redblue    * 256L));

    ag = SIGN_EXTEND(ag, 13);
    rb = SIGN_EXTEND(rb, 13);

    return (( (LIMIT_RANGE(ag) << SST_TA_CONSTANT_COLOR0_ALPHA_SHIFT) & SST_TA_CONSTANT_COLOR0_ALPHA)
            |((LIMIT_RANGE(rb) << SST_TA_CONSTANT_COLOR0_RED_SHIFT)   & SST_TA_CONSTANT_COLOR0_RED));

#undef LIMIT_RANGE
} // setColorConstant()

//----------------
// add_extra_stages()
// This function adds extra stages to support
// general bumpmapping and extra-stage specular.
// origtss points to a copy of the original 8 texturestagestates,
// so that those in pRc->textureStage[] can be changed.
// *esinfo comes into this function initialized with everything
// disabled and stages set to 9; see setupTexturing().

static DWORD
add_extra_stages ( RC* pRc, int ns, TEXTURESTAGESTATE *origtss, EXTRASTAGEINFO *esinfo,
                   BOOL haveBumpMap)
{
    SETUP_PPDEV(pRc)                    // zzz - remove this crap
    int i;
    FxU8 remap[8] = {0,1,2,3,4,5,6,7};
    TEXTURESTAGESTATE *tsi;

    if (haveBumpMap)
    {
        // Divide the current pipeline into three parts:
        // "A" includes all stages prior to bumpenvmap stage
        // "B" includes the bumpenvmap and following stage (2 stages always)
        // "C" includes all stages following the 2 of "B".
        int a_start =  0, a_end = -1,
            b_start = -1, b_end = -1,
            c_start = -1, c_end = -1;
        int stage, extra_stage;
        txtrDesc *txtr = NULL;

        for (i=0; i<ns; i++)
        {
            if (TS[i].colorOp == D3DTOP_BUMPENVMAP ||
                TS[i].colorOp == D3DTOP_BUMPENVMAPLUMINANCE)
            {
                txtr = TXTRHNDL_TO_TXTRDESC(pRc, TS[i].textureHandle);
                esinfo->bump_enabled = (TXTR_IS_UVL(txtr)) ? BUMPMAP_UVL_ON : BUMPMAP_UV_ON;

                a_end = i-1;
                b_start = i;
                b_end = i+1;
                c_start = i+2;          // These can be past ns
                c_end = ns-1;           // so check later.
                break;
            }
        }

        // The pipeline must be reordered if either
        // 1) UV bumpmapping and the bumpmap stage is not 0 (i>0).
        // 2) or, UVL bumpmapping at any stage.
        if (i || (esinfo->bump_enabled == BUMPMAP_UVL_ON))
        {
            if (esinfo->bump_enabled == BUMPMAP_UVL_ON)
            {
                esinfo->need_extra_stage            = (i > 0);
                stage = 0;
                esinfo->UV_stage_1 = remap[b_start] = stage++; // Insert "B"
                esinfo->UV_stage_2                  = stage++;
                esinfo->env_stage  = remap[b_end]   = stage++;
                esinfo->L_stage                     = stage++;

                if (esinfo->need_extra_stage)
                {
                    if (a_start <= a_end) // insert "A"
                        for (i=a_start; i<=a_end; i++)
                            remap[i] = stage++;

                    esinfo->op_stage = stage++; // Extra operation stage

                    esinfo->load_reg      = esinfo->L_stage;
                    esinfo->read_reg      = esinfo->op_stage;
                }

                if (c_start < ns)       // insert "C"
                {
                    c_end = MIN(c_end, ns-1);
                    for (i=c_start; i<=c_end; i++)
                        remap[i] = stage++;
                }

                // Perform the remapping by copying our TS[]'s.
                for (i=0; i<ns; i++)
                {
                    TS[remap[i]].changed = 1;
                    TS[remap[i]]         = origtss[i];
                }
                ns = stage;

                // Configure the stages that were not remappings:
                // The operations and args here are really place holders
                // for setting up suParamMask later.  The actual operation
                // set in the VTA is bumpmapping, configged in setupTexturing().
                // Entire stages are copied here to propagate texture handles,
                // wrapping and addressing modes, etc, but the ops are changed.

                // setup second bumpmap stage
                tsi = &TS[esinfo->UV_stage_2];
                *tsi = TS[esinfo->UV_stage_1];
                tsi->changed   = 1;
                tsi->colorOp   = D3DTOP_SELECTARG1;
                tsi->colorArg1 = D3DTA_TEXTURE;
                tsi->colorArg2 = D3DTA_CURRENT;
                tsi->alphaOp   = D3DTOP_SELECTARG1;
                tsi->alphaArg1 = D3DTA_TEXTURE;
                tsi->alphaArg2 = D3DTA_CURRENT;

                // Cleanup inputs for b_end, the envmap:
                tsi = &TS[esinfo->env_stage];// no copy here -- already done above.
                tsi->changed   = 1;
                tsi->colorOp   = D3DTOP_SELECTARG1;
                tsi->colorArg1 = D3DTA_TEXTURE;
                tsi->colorArg2 = D3DTA_CURRENT;
                tsi->alphaOp   = D3DTOP_SELECTARG1;
                tsi->alphaArg1 = D3DTA_TEXTURE;
                tsi->alphaArg2 = D3DTA_CURRENT;

                // setup the Luminance stage
                tsi = &TS[esinfo->L_stage];
                *tsi = TS[esinfo->UV_stage_2]; // same as UV_stage_2
                tsi->changed   = 1;
                tsi->colorOp   = D3DTOP_MODULATE;
                tsi->colorArg1 = D3DTA_TEXTURE;
                tsi->colorArg2 = D3DTA_CURRENT;
                tsi->alphaOp   = D3DTOP_SELECTARG1;
                tsi->alphaArg1 = D3DTA_CURRENT;

                // Configure the extra operation stage:
                if (esinfo->need_extra_stage) {
                    tsi = &TS[esinfo->op_stage];
                    tsi->changed   = 1;
                    tsi->colorOp   = origtss[b_end].colorOp;
                    tsi->colorArg1 = origtss[b_end].colorArg1;
                    tsi->colorArg2 = origtss[b_end].colorArg2;
                    tsi->alphaOp   = origtss[b_end].alphaOp;
                    tsi->alphaArg1 = origtss[b_end].alphaArg1;
                    tsi->alphaArg2 = origtss[b_end].alphaArg2;
                }

            } else {                    // UV format...

                // Define the remapping: remap[original] = new
                // Reorder the sub-pipelines as : "A B C" -> "B A extra C"
                stage = 0;
                esinfo->UV_stage_1 = remap[b_start] = stage++;       // insert "B"
                esinfo->env_stage  = remap[b_end]   = stage++;
                if (a_start <= a_end)           // insert "A"
                    for (i=a_start; i<=a_end; i++)
                        remap[i] = stage++;
                extra_stage = stage++;          // inserted stage
                if (c_start < ns)               // insert "C"
                {
                    c_end = MIN(c_end, ns-1);
                    for (i=c_start; i<=c_end; i++)
                        remap[i] = stage++;
                }

                // Perform the remapping by copying our TS[]'s.
                for (i=0; i<ns; i++)
                {
                    TS[remap[i]].changed = 1;
                    TS[remap[i]]         = origtss[i];
                }
                ns++;

                // Configure the extra stage:
                TS[extra_stage].changed   = 1;
                TS[extra_stage].colorOp   = origtss[b_end].colorOp;
                TS[extra_stage].colorArg1 = origtss[b_end].colorArg1;
                TS[extra_stage].colorArg2 = origtss[b_end].colorArg2;
                TS[extra_stage].alphaOp   = origtss[b_end].alphaOp;
                TS[extra_stage].alphaArg1 = origtss[b_end].alphaArg1;
                TS[extra_stage].alphaArg2 = origtss[b_end].alphaArg2;
                // Cleanup inputs for b_end:
                TS[remap[b_end]].changed   = 1;
                TS[remap[b_end]].colorOp   = D3DTOP_SELECTARG1;
                TS[remap[b_end]].colorArg1 = D3DTA_TEXTURE;
                TS[remap[b_end]].alphaOp   = D3DTOP_SELECTARG1;
                TS[remap[b_end]].alphaArg1 = D3DTA_TEXTURE;

                // Pass back bump description
                esinfo->load_reg         = remap[b_end];
                esinfo->read_reg         = extra_stage;
                esinfo->op_stage         = extra_stage;
                esinfo->need_extra_stage = TRUE;
            } // else UV reordering
        } // need reordering
        else                            // No reordering, UV setup:
        {
            esinfo->UV_stage_1 = 0;     // These will always be this way
            esinfo->env_stage  = 1;
            esinfo->need_extra_stage = FALSE;
        }
    } // if (haveBumpMap)


    // If specular is enabled, add a stage if possible.
    if (pRc->state & STATE_REQUIRES_SPECULAR)
    {
        if (ns < D3DHAL_TSS_MAXSTAGES)
        {
            pRc->specular = 1 << ns;
            TS[ns].changed = 1;
            TS[ns].colorOp = D3DTOP_ADD;
            TS[ns].colorArg1 = D3DTA_CURRENT;
            TS[ns].colorArg2 = D3DTA_DIFFUSE;
            TS[ns].alphaOp = D3DTOP_SELECTARG1;
            TS[ns].alphaArg1 = D3DTA_CURRENT;
            ns++;
        } else {
            // No space for an extra stage, so we're forced to
            // do a second pass.
            pRc->specular = SST_TWOPASS_SPECULAR;
        }
    }

    return ns;
} // add_extra_stages()


//----------------
// postprocess_stages()
// After possibly adding stages, the registers are set by setupTexturing()
// which in turn calls this routine.  Here we may need to tweak some
// registers to direct flow around for bumpmapping, specular, and other
// extra-stage features.
static void
postprocess_stages (RC* pRc, DWORD ns, EXTRASTAGEINFO *esinfo)
{
    DWORD regtemp;
    SETUP_PPDEV(pRc)

    if ((esinfo->bump_enabled != BUMPMAP_NONE) && esinfo->need_extra_stage)
    {
        // At the stage esinfo->load_reg, direct CCU output to CDR.
        pRc->sst.TMU[esinfo->load_reg].taCcuControl.vFxU32 |= (1 << SST_TA_CUC_REG_LOAD_SHIFT |
                                                              1 << SST_TA_CUA_REG_LOAD_SHIFT);
        UPDATE_HW_STATE( reg3D.TMU[esinfo->load_reg].taCcuControl.group );

        // At the stage esinfo->read_reg, colorArg* with D3DTA_TEXTURE
        // should be set to AREG/CREG instead of ATEX/CTEX
        if (D3DTA_IS(TS[esinfo->read_reg].colorArg1, D3DTA_TEXTURE) ||
            D3DTA_IS(TS[esinfo->read_reg].colorArg2, D3DTA_TEXTURE) ||
            D3DTA_IS(TS[esinfo->read_reg].alphaArg1, D3DTA_TEXTURE) ||
            D3DTA_IS(TS[esinfo->read_reg].alphaArg2, D3DTA_TEXTURE))
        {

#define CHANGE_THIS_TO_THAT(reg,ccc,abcd,THIS,THAT) \
        if (((reg & SST_TA_##ccc##_##abcd##_SELECT) >> \
            SST_TA_##ccc##_##abcd##_SELECT_SHIFT) == SST_TA_##ccc##_##THIS) \
            reg = ((reg & ~SST_TA_##ccc##_##abcd##_SELECT) | \
                   SST_TA_##ccc##_##THAT << SST_TA_##ccc##_##abcd##_SELECT_SHIFT)
#define CHANGE_CTEX_TO_CREG(reg,ccc,abcd) \
        CHANGE_THIS_TO_THAT(reg,ccc,abcd,CTEX,CREG)
#define CHANGE_ATEX_TO_AREG(reg,ccc,abcd) \
        CHANGE_THIS_TO_THAT(reg,ccc,abcd,ATEX,AREG)

            regtemp = pRc->sst.TMU[esinfo->read_reg].taCcuColor.vFxU32;
            CHANGE_CTEX_TO_CREG(regtemp,CCC,A);
            CHANGE_CTEX_TO_CREG(regtemp,CCC,B);
            CHANGE_CTEX_TO_CREG(regtemp,CCC,C);
            CHANGE_CTEX_TO_CREG(regtemp,CCC,D);

            CHANGE_ATEX_TO_AREG(regtemp,CCC,A);
            CHANGE_ATEX_TO_AREG(regtemp,CCC,B);
            CHANGE_ATEX_TO_AREG(regtemp,CCC,C);
            CHANGE_ATEX_TO_AREG(regtemp,CCC,D);
            pRc->sst.TMU[esinfo->read_reg].taCcuColor.vFxU32 = regtemp;
            UPDATE_HW_STATE( reg3D.TMU[esinfo->load_reg].taCcuColor.group );

            regtemp = pRc->sst.TMU[esinfo->read_reg].taTcuColor.vFxU32;
            CHANGE_CTEX_TO_CREG(regtemp,TCC,A);
            CHANGE_CTEX_TO_CREG(regtemp,TCC,B);
            CHANGE_CTEX_TO_CREG(regtemp,TCC,C);
            CHANGE_CTEX_TO_CREG(regtemp,TCC,D);

            CHANGE_ATEX_TO_AREG(regtemp,TCC,A);
            CHANGE_ATEX_TO_AREG(regtemp,TCC,B);
            CHANGE_ATEX_TO_AREG(regtemp,TCC,C);
            CHANGE_ATEX_TO_AREG(regtemp,TCC,D);
            pRc->sst.TMU[esinfo->read_reg].taTcuColor.vFxU32 = regtemp;
            UPDATE_HW_STATE( reg3D.TMU[esinfo->load_reg].taTcuColor.group );

            regtemp = pRc->sst.TMU[esinfo->read_reg].taCcuAlpha.vFxU32;
            CHANGE_ATEX_TO_AREG(regtemp,CCA,A);
            CHANGE_ATEX_TO_AREG(regtemp,CCA,B);
            CHANGE_ATEX_TO_AREG(regtemp,CCA,C);
            CHANGE_ATEX_TO_AREG(regtemp,CCA,D);
            pRc->sst.TMU[esinfo->read_reg].taCcuAlpha.vFxU32 = regtemp;
            UPDATE_HW_STATE( reg3D.TMU[esinfo->load_reg].taCcuAlpha.group );

            regtemp = pRc->sst.TMU[esinfo->read_reg].taTcuAlpha.vFxU32;
            CHANGE_ATEX_TO_AREG(regtemp,TCA,A);
            CHANGE_ATEX_TO_AREG(regtemp,TCA,B);
            CHANGE_ATEX_TO_AREG(regtemp,TCA,C);
            CHANGE_ATEX_TO_AREG(regtemp,TCA,D);
            pRc->sst.TMU[esinfo->read_reg].taTcuAlpha.vFxU32 = regtemp;
            UPDATE_HW_STATE( reg3D.TMU[esinfo->load_reg].taTcuAlpha.group );

        }
    }

} // postprocess_stages()


//-------------------------------------------------------------------

DWORD setupTextureStage (RC *pRc, FxU32 stage, FxU32 numstages,
                         EXTRASTAGEINFO *esinfo)
{
  SETUP_PPDEV(pRc)
  txtrDesc  *txtr;
  TXHNDLSTRUCT *txhndlstruct;
  FxU32 txhndl = TS[stage].textureHandle;

  if (txhndl == 0)
    return 0;

  txhndlstruct = TXTRHNDL_TO_TXHNDLSTRUCT(pRc, txhndl);
  if (txhndlstruct->surfData == 0)
    return 0;

  txtr = TXTRHNDL_TO_TXTRDESC(pRc, txhndl);
  if (txtr == 0)
    return 0;

  if ((txhndl && txhndlstruct->flags & HandleInUse) &&
      (TS[stage].colorOp != D3DTOP_DISABLE || TS[stage].alphaOp != D3DTOP_DISABLE))
  {    
    pRc->sst.TMU[stage].taMode.vFxU32 = 0; // clear everything

    // Get the correct txtrDesc pointer if UVL bumpmapping is being used.
    if (esinfo->bump_enabled == BUMPMAP_UVL_ON)
    {
        if (stage == esinfo->UV_stage_1 ||
            stage == esinfo->UV_stage_2)
        {
            txtr = txtr->txtrDescNext;      // txtr now points to UV texture.
        }
        else
        if (stage == esinfo->L_stage)
        {
            txtr = txtr->txtrDescNext;      // txtr now points to UV texture.
            txtr = txtr->txtrDescNext;      // txtr now points to L  texture.
        }
    } else if (esinfo->bump_enabled == BUMPMAP_UV_ON &&
               stage == esinfo->UV_stage_1)
    {
        txtr = txtr->txtrDescNext;          // txtr now points to UV texture.
    }

    // Calculate the texture coordinate index into the FVF structure
    if ( pRc->fvfVertexType == FVFOT_CUSTOM )
    {
        pRc->tCoordIndex[stage] = pRc->tCoordDimensionOffset[TS[stage].texCoordIndex];
    }
    else
    {
        pRc->tCoordIndex[stage] = TS[stage].texCoordIndex * 2;
    }

    // Set the texture magnification filtering        

    switch( TS[stage].magFilter )
    {
      case D3DTFG_POINT:
        pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_POINT << SST_TA_MAGFILTER_SHIFT);
        break;
      
      case D3DTFG_LINEAR:
        if (TS[stage].minFilter == D3DTFN_LINEAR )
          pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_TRILINEAR << SST_TA_MAGFILTER_SHIFT);
        else
          pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT);
        break;

      case D3DTFG_ANISOTROPIC:
        // Ani ratio is multipled by 2 inside VTA, so we support ratios
        // up to 16, in steps of 2. Clamp until there's a reason not to.
        
        if (TS[stage].maxAnisotropy > 16)
            TS[stage].maxAnisotropy = 16;
        if (TS[stage].maxAnisotropy < 0)
            TS[stage].maxAnisotropy = 0;

        pRc->sst.TMU[stage].taMode.vFxU32 |= 
             ((TS[stage].maxAnisotropy >> 1) << SST_TA_ANI_MAX_RATIO_SHIFT)
             & SST_TA_ANI_MAX_RATIO;
        pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_BILINEAR << SST_TA_MAGFILTER_SHIFT);
        break;
    }   

    // Set the texture minification filtering        

    switch( TS[stage].minFilter )
    {
        case D3DTFN_POINT:
          switch( TS[stage].mipFilter )
          {
            case D3DTFP_NONE:
                 pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_POINT << SST_TA_MINFILTER_SHIFT) ;
                 break;

           case D3DTFP_POINT:
                pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_POINT << SST_TA_MINFILTER_SHIFT) ;
                break;

           case D3DTFP_LINEAR:
                // WB2K detected Nearest MIP Nearest
                //pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_POINT << SST_TA_MINFILTER_SHIFT) ;

                // WB2K detected Nearest MIP Linear
                //pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) ;

                // WB2K detected Linear MIP Linear
                pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_TRILINEAR << SST_TA_MINFILTER_SHIFT);

                // WB2K detected Mipmap Level Dithering
                // pRc->sst.TMU[stage].taMode.vFxU32 |= ((SST_TA_POINT << SST_TA_MINFILTER_SHIFT) 
                //                                      | SST_TA_LMS_DITHER);
                // pRc->sst.TMU[stage].taMode.vFxU32 |= ((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) 
                //                                      | SST_TA_LMS_DITHER);
                // WB2K detected 5x5 kernel with dithering
                // pRc->sst.TMU[stage].taMode.vFxU32 |= ((SST_TA_TRILINEAR << SST_TA_MINFILTER_SHIFT) 
                //                                      | SST_TA_LMS_DITHER);

                // WB2K detected Nearest MIP Linear & didn't
                // complain about early level transitons.
                // pRc->sst.TMU[stage].taMode.vFxU32 |= ((SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT);
                // pRc->lmsBias = 0 | SST_TA_LMS_ZERO_FRAC;
                // pRc->sst.TMU[stage].taLMS.vFxU32   = txtr->txTaLMS | pRc->lmsBias; 

                break;
          } 
          break;

        case D3DTFN_LINEAR:
          switch( TS[stage].mipFilter )
          {
            case D3DTFP_NONE:
                 pRc->sst.TMU[stage].taMode.vFxU32   |= (SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) ;
                 break;

            case D3DTFP_POINT:
                 pRc->sst.TMU[stage].taMode.vFxU32   |= (SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) ;
                 break;

            case D3DTFP_LINEAR:
                 pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_TRILINEAR << SST_TA_MINFILTER_SHIFT) ;
                 break;
          } 
          break;

        case D3DTFN_ANISOTROPIC:
           // Ani ratio is multipled by 2 inside VTA, so program max ratios
           // from 0 to 16, in steps of 2. 
        
          if (TS[stage].maxAnisotropy > 16)
              TS[stage].maxAnisotropy = 16;
          if (TS[stage].maxAnisotropy < 0)
              TS[stage].maxAnisotropy = 0;

          pRc->sst.TMU[stage].taMode.vFxU32 |= 
             ((TS[stage].maxAnisotropy >> 1) << SST_TA_ANI_MAX_RATIO_SHIFT)
             & SST_TA_ANI_MAX_RATIO;

          switch( TS[stage].mipFilter )
          {
            case D3DTFP_NONE:
              pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) ;
              break;

           case D3DTFP_POINT:
              pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_BILINEAR << SST_TA_MINFILTER_SHIFT) ;
              break;

           case D3DTFP_LINEAR:
              pRc->sst.TMU[stage].taMode.vFxU32 |= (SST_TA_TRILINEAR << SST_TA_MINFILTER_SHIFT) ;
              break;
          } 
          break;

      default:
        break;
    }   

    // Get the base address and any mip info for the texture
    if( TS[stage].mipFilter == D3DTFP_NONE )
    {
      pRc->sst.TMU[stage].taLMS.vFxU32       = txtr->txTaLMS_noMipMaps;
      pRc->sst.TMU[stage].taBaseAddr0.vFxU32 = txtr->txTaBaseAddr0;
    }
    else
    {
      // if the TSS LOD bias has been set then use it, otherwise use the RC LOD bias
      if( TS[stage].mipmapLodBias )
      {  
        pRc->sst.TMU[stage].taLMS.vFxU32        = txtr->txTaLMS | TS[stage].mipmapLodBias; 
      }
      else
      {
        pRc->sst.TMU[stage].taLMS.vFxU32        = txtr->txTaLMS | pRc->lmsBias; 
      }
      pRc->sst.TMU[stage].taBaseAddr0.vFxU32  = txtr->txTaBaseAddr0;
    }
    UPDATE_HW_STATE( reg3D.TMU[stage].taLMS.group | reg3D.TMU[stage].taBaseAddr0.group );

    // Check for texture wrapping in U(s)
    switch( TS[stage].addressU  )
    {
      case D3DTADDRESS_WRAP:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_REPEAT << SST_TA_WRAP_S_SHIFT);
        break;
    
      case D3DTADDRESS_MIRROR:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_MIRROR << SST_TA_WRAP_S_SHIFT);
        break;
    
      case D3DTADDRESS_BORDER:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_CLAMP_TO_BLACK << SST_TA_WRAP_S_SHIFT);
        break;
    
      case D3DTADDRESS_CLAMP:
        // Make clamp the default 
      default:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_CLAMP << SST_TA_WRAP_S_SHIFT);
        break;
    }

    // Check for texture in V(t)
    switch( TS[stage].addressV  )
    {
      case D3DTADDRESS_WRAP:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_REPEAT << SST_TA_WRAP_T_SHIFT);
        break;

      case D3DTADDRESS_MIRROR:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_MIRROR << SST_TA_WRAP_T_SHIFT);
        break;

      case D3DTADDRESS_BORDER:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_CLAMP_TO_BLACK << SST_TA_WRAP_T_SHIFT);
        break;
      
      case D3DTADDRESS_CLAMP:
        // Make clamp the default 
      default:
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( SST_TA_CLAMP << SST_TA_WRAP_T_SHIFT);
        break;
    }

    // setup NPT texturing if necessary
    if (txtr->txFormatFlags & TEXFMTFLG_NPT)
        txtrSetNptHwRegs(stage, pRc, txtr);

    // Store off the texture scale components
    // Each texture has initial scalings in the S0/T0 fields of
    // its txtr->STscale register, so shift these to the appropriate
    // position.  Textures are at D3D stages so ns-i-1 is the correct
    // TMU index.
    {
        static const int stscale_shift[8] = {
            SST_VP_S0_SCALE_SHIFT,
            SST_VP_S1_SCALE_SHIFT,
            SST_VP_S2_SCALE_SHIFT,
            SST_VP_S3_SCALE_SHIFT,
            SST_VP_S4_SCALE_SHIFT,
            SST_VP_S5_SCALE_SHIFT,
            SST_VP_S6_SCALE_SHIFT,
            SST_VP_S7_SCALE_SHIFT,
        };
        DWORD tmu = numstages - stage - 1;
        DWORD scale = txtr->txSTscale << stscale_shift[tmu];

        if (tmu >= 4)
            pRc->sst.vpSTscale1.vFxU32 |= scale;
        else
            pRc->sst.vpSTscale0.vFxU32 |= scale;
        UPDATE_HW_STATE( reg3D.vpSTscale0.group );
    }

    // Store off constant color registers
    // with default of 0.5 color and 0.5 alpha

    // 0x100F or 0x10FFF same effect 
    pRc->sst.TMU[stage].taColorAR0.vFxU32 = 
         ( 0x7F << SST_TA_CONSTANT_COLOR0_ALPHA_SHIFT) |
         ( 0x7F << SST_TA_CONSTANT_COLOR0_RED_SHIFT);

    pRc->sst.TMU[stage].taColorGB0.vFxU32 = 
         ( 0x7F << SST_TA_CONSTANT_COLOR0_GREEN_SHIFT) |
         ( 0x7F << SST_TA_CONSTANT_COLOR0_BLUE_SHIFT);

    pRc->sst.TMU[stage].taColorAR1.vFxU32 = 
         ( 0x7F << SST_TA_CONSTANT_COLOR1_ALPHA_SHIFT) |
         ( 0x7F << SST_TA_CONSTANT_COLOR1_RED_SHIFT);

    pRc->sst.TMU[stage].taColorGB1.vFxU32 = 
         ( 0x7F << SST_TA_CONSTANT_COLOR1_GREEN_SHIFT) |
         ( 0x7F << SST_TA_CONSTANT_COLOR1_BLUE_SHIFT);

    pRc->sst.TMU[stage].taShiftBias.vFxU32 = txtr->txTaShiftBias;

    UPDATE_HW_STATE( reg3D.TMU[stage].taColorAR0.group | reg3D.TMU[stage].taColorGB0.group |
                     reg3D.TMU[stage].taColorAR1.group | reg3D.TMU[stage].taColorGB1.group |
                     reg3D.TMU[stage].taShiftBias.group );

    // Setup the texture format
    pRc->sst.TMU[stage].taMode.vFxU32 |= txtr->txFormat;

    if (TXTR_IS_PALETTIZED(txtr))
    {
      PALHNDL *palDesc = TXTR_GET_PAL_DESCRIPTOR(txtr, pRc, txhndl);

      txtrNewPalette( ppdev, palDesc );

      if (PAL_HAS_ALPHA( palDesc ))
      {
        // change the texture format from P8_RGB to P8_RGBA
        pRc->sst.TMU[stage].taMode.vFxU32 &= ~(SST_TA_FORMAT);
        pRc->sst.TMU[stage].taMode.vFxU32 |= ( TEXFMT_P8_RGBA << SST_TA_FORMAT_SHIFT );
        txtr->txFormat &= ~(SST_TA_FORMAT);
        txtr->txFormat |= (TEXFMT_ALPHA_P8_RGB << SST_TA_FORMAT_SHIFT);
        txtr->txFormatFlags |= TEXFMTFLG_PALETTIZED_ALPHA;
      }
    }

    // Setup any state requirements and command packet3 data components
    pRc->state |= pRc->state_requires_tmu_flags[stage];
    // sst.suParamMask now set above in setup_suParamMask()

    // Enable Texturing
    pRc->sst.TMU[stage].taMode.vFxU32 |= SST_TA_EN_TEXTUREMAP;

    // Clear extra control flags
    pRc->sst.TMU[stage].taCcuControl.vFxU32 = 0;

    UPDATE_HW_STATE( reg3D.TMU[stage].taCcuControl.group );
  }
  else
  {
    // Disable Texturing
    pRc->sst.TMU[stage].taMode.vFxU32 &= ~ SST_TA_EN_TEXTUREMAP;
    
    // Reset any states that aren't required
    pRc->state &= ~(pRc->state_requires_tmu_flags[stage]);
    // sst.suParamMask now set above in setup_suParamMask()
  }

  UPDATE_HW_STATE( reg3D.TMU[stage].taMode.group );

  // Reset texture stage state change flag
  TS[stage].changed = 0;

  return D3D_OK;  
} // setupTextureStage()


//-------------------------------------------------------------------
// setupTexturing() - Sets up all texturing stages for current D3D state.

void setupTexturing( RC* pRc )
{
  SETUP_PPDEV(pRc)
  DWORD ns = 0;
  LPTEXTUREOP  lpStage;
  LPTEXTUREOP  lpColorOp;
  LPTEXTUREOP  lpAlphaOp;
  BOOL haveBumpMap;

  FxU32  tcu_inc, tcu_arep, tcu_comp, tcuTemp, tcuColor;
  FxU32  tca_inc,           tca_comp, tcaTemp, tcaAlpha;
  FxU32  ccu_inc, ccu_arep, ccu_comp, ccuTemp, ccuColor;
  FxU32  cca_inc,           cca_comp, ccaTemp, ccaAlpha;
  FxU32  opIdx;
  FxU32  texFormat;
  FxU32  abcd_pos;


  ns = countTextureStages (pRc, &haveBumpMap);

  // If all stages are disabled then we are either flat shading or
  // gouraud shading.
  if( ns == 0 )
  {
    // pRc->sst.fbzColorPath.vFxU32 = SST_PARMADJUST;
    
    pRc->state &= ~(STATE_REQUIRES_W_TMU0
                  | STATE_REQUIRES_ST_TMU0
                  | STATE_REQUIRES_ST_TMU1);

    if (pRc->fogEnable)
        pRc->sst.suMode.vFxU32 &= ~(SST_SU_W);	// fog needs SST_SU_Q
    else
        pRc->sst.suMode.vFxU32 &= ~(SST_SU_Q);

    pRc->sst.suParamMask.vFxU32 = setup_suParamMask(pRc, ns, NULL);

    // Ensure texturing is disabled:
    for (ns=0; ns < 8; ns++)
    {
        pRc->sst.TMU[ns].taMode.vFxU32 &= ~SST_TA_EN_TEXTUREMAP;
        UPDATE_HW_STATE( reg3D.TMU[ns].taMode.group );

        // Make csim happy by ensuring block linear is set for
        // DXT and FXT formats even if texturing is disabled.

        texFormat = (pRc->sst.TMU[ns].taMode.vFxU32 & SST_TA_FORMAT)
                    >> SST_TA_FORMAT_SHIFT;
        if (( texFormat == SST_TA_FXT1 ) ||
            (( texFormat >= SST_TA_DXT1 ) && ( texFormat <= SST_TA_DXT5 )))
        {
           pRc->sst.TMU[ns].taNPT.vFxU32 |= SST_BLOCK_LINEAR;
        }
        else
        {
           pRc->sst.TMU[ns].taNPT.vFxU32 &= ~SST_BLOCK_LINEAR;
        }
        // Is taNPT state already dirty since taMode.group is set?
        UPDATE_HW_STATE( reg3D.TMU[ns].taNPT.group );
    }
    ns = 0;
    pRc->sst.taControl.vFxU32 = 0;

    pRc->sst.TMU[0].taTcuColor.vFxU32 = TCC( ZERO, ZERO, ZERO, ZERO);
    pRc->sst.TMU[0].taTcuAlpha.vFxU32 = TCA( ZERO, ZERO, ZERO, ZERO);
    
    // could use constant color here
    // if (pRc->shadeMode == D3DSHADE_FLAT)

    pRc->sst.TMU[0].taCcuColor.vFxU32 = CCC( ZERO, ZERO, ZERO, CITER);
    pRc->sst.TMU[0].taCcuAlpha.vFxU32 = CCA( ZERO, ZERO, ZERO, AITER);
    UPDATE_HW_STATE(reg3D.TMU[0].taTcuColor.group | reg3D.TMU[0].taTcuAlpha.group |
                    reg3D.TMU[0].taCcuColor.group | reg3D.TMU[0].taCcuAlpha.group |
                    reg3D.taControl.group );

      // Add a TMU to do one-pass specular
      if (pRc->state & STATE_REQUIRES_SPECULAR)
      {
          // TMU 0 was for diffuse color;  Set TMU 1 to do
          // specular.
          pRc->sst.TMU[1].taTcuColor.vFxU32 =  TCC( ZERO, ZERO, ZERO, ZERO);
          pRc->sst.TMU[1].taTcuAlpha.vFxU32 =  TCA( ZERO, ZERO, ZERO, ZERO);
          pRc->sst.TMU[1].taCcuColor.vFxU32 = (CCC( ZERO, CPREV, ZERO, CITER) |
                                             (SST_TA_INV_ONE_MINUS << SST_TA_CCC_C_MODE_SHIFT));
          pRc->sst.TMU[1].taCcuAlpha.vFxU32 =  CCA( ZERO, ZERO, ZERO, APREV);

          pRc->sst.taControl.vFxU32    = 1 << SST_TA_NUM_TMUS_SHIFT; // use 2 TMUs
          pRc->sst.suParamMask.vFxU32 |= (SST_SU_RGB1 | SST_SU_A1);
          pRc->specular = 1<<1;           // set bit 1 to denote TMU 1.
          ns++;
          UPDATE_HW_STATE(reg3D.TMU[1].taTcuColor.group | reg3D.TMU[1].taTcuAlpha.group |
                          reg3D.TMU[1].taCcuColor.group | reg3D.TMU[1].taCcuAlpha.group |
                          reg3D.taControl.group );
     }
  }
  else                                  // else texturing enabled (ns>0)
  {
    DWORD i;
    EXTRASTAGEINFO esinfo = {BUMPMAP_NONE, 9, 9, 9, 9, 9, 9, 9, 0};
    RC origrc;                          // backup copy of RC
    TEXTURESTAGESTATE *tsi;

    // Change all of Stage 0's "CURRENT" args to "DIFFUSE" per Refrast:
    tsi = &TS[0];
    if (D3DTA_IS(tsi->colorArg1, D3DTA_CURRENT))
        tsi->colorArg1 = (tsi->colorArg1 & ~D3DTA_SELECTMASK)|D3DTA_DIFFUSE;
    if (D3DTA_IS(tsi->colorArg2, D3DTA_CURRENT))
        tsi->colorArg2 = (tsi->colorArg2 & ~D3DTA_SELECTMASK)|D3DTA_DIFFUSE;
    if (D3DTA_IS(tsi->alphaArg1, D3DTA_CURRENT))
        tsi->alphaArg1 = (tsi->alphaArg1 & ~D3DTA_SELECTMASK)|D3DTA_DIFFUSE;
    if (D3DTA_IS(tsi->alphaArg2, D3DTA_CURRENT))
        tsi->alphaArg2 = (tsi->alphaArg2 & ~D3DTA_SELECTMASK)|D3DTA_DIFFUSE;
    if (tsi->colorOp == D3DTOP_BLENDCURRENTALPHA)
        tsi->colorOp = D3DTOP_BLENDDIFFUSEALPHA;
    if (tsi->alphaOp == D3DTOP_BLENDCURRENTALPHA)
        tsi->alphaOp = D3DTOP_BLENDDIFFUSEALPHA;

    origrc = *pRc;
    ns = add_extra_stages (pRc, ns, origrc.textureStage, &esinfo, haveBumpMap);

    // Check to see if any of the texture stage states have changed 
    if( TS[0].changed || TS[1].changed || TS[2].changed || TS[3].changed ||
        TS[4].changed || TS[5].changed || TS[6].changed || TS[7].changed )
    { 

      // Clear all texture coordinate states
      pRc->state &= ~(STATE_REQUIRES_W_TMU0
                      | STATE_REQUIRES_ST_TMU0
                      | STATE_REQUIRES_ST_TMU1);

      if (pRc->fogEnable)
        pRc->sst.suMode.vFxU32 &= ~(SST_SU_W);	// fog needs SST_SU_Q
      else
        pRc->sst.suMode.vFxU32 &= ~(SST_SU_Q);
      
      pRc->sst.vpSTscale0.vFxU32 = 0;          // Init scales; these are set in
      pRc->sst.vpSTscale1.vFxU32 = 0;          // setupTextureStage()

      UPDATE_HW_STATE( reg3D.suMode.group | reg3D.vpSTscale0.group | reg3D.vpSTscale1.group );


      for(i=0; i < ns; i++)
         setupTextureStage (pRc, i, ns, &esinfo);

      // Set W State if either texture coordinate state is being used
      if( pRc->state & (STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_ST_TMU1) )
      {
        pRc->state            |= STATE_REQUIRES_W_TMU0;
        pRc->sst.suMode.vFxU32 |= SST_SU_W;
      }

      pRc->sst.taControl.vFxU32 = (ns-1) << SST_TA_NUM_TMUS_SHIFT;
      UPDATE_HW_STATE( reg3D.taControl.group );

      // Now we setup the texture and color paths.      

      for( i = 0; i < ns; i++ )
      {
        TEXTURESTAGESTATE *tsi = &TS[i];
        OURTMUREGS *ptmui = &pRc->sst.TMU[i];

        // color STAGEOP table entry
        lpStage = stageOp[ns-i-1][COLOROP];

        // color TEXTUREOP table entry
        lpColorOp = &lpStage[tsi->colorOp];

        // alpha combine operations

        // alpha STAGEOP table entry
        lpStage = stageOp[ns-i-1][ALPHAOP];

        // alpha TEXTUREOP table entry
        lpAlphaOp = &lpStage[tsi->alphaOp];


        tcu_inc = ccu_inc = 0;
        tca_inc = cca_inc = 0;

        // Convert stage 0's "CURRENT" arguments to "DIFFUSE."

        if( i == 0 )
        {
            // CURRENT args were converted to DIFFUSE in
            // add_extra_stages() above.

            // Convert operations from BlendCurrentAlpha
            // to BlendDiffuseAlpha per refrast.
            if (tsi->colorOp == D3DTOP_BLENDCURRENTALPHA)
            {
                tsi->colorOp = D3DTOP_BLENDDIFFUSEALPHA;
                lpColorOp = &stageOp[ns-1][COLOROP][tsi->colorOp];
            }
            if (tsi->alphaOp == D3DTOP_BLENDCURRENTALPHA)
            {
                tsi->alphaOp = D3DTOP_BLENDDIFFUSEALPHA;
                lpAlphaOp = &stageOp[ns-1][ALPHAOP][tsi->alphaOp];
            }
        }

        // Handle alpha replicate using a mask that is
        // added to the selected vta color input to select
        // the corresponding alpha input. Maps CPREV, CITER,
        // CTEX, C0, C1 and CTCU to alpha vairety of same, does
        // not handle ZERO, CTCUSUM or Z. Only needed for the
        // color channel. Validation of color input is needed
        // to avoid bad mappings (eg: TCC_ZERO + 6 = TCC_CREG).

        tcu_arep = ccu_arep = 0;

        if( tsi->colorArg1 & D3DTA_ALPHAREPLICATE &&
            tsi->colorOp != D3DTOP_SELECTARG2)
        {
           tcu_arep = lpColorOp->areplicate_arg1[ TCU_OP ];
           ccu_arep = lpColorOp->areplicate_arg1[ CCU_OP ];
        }

        if( tsi->colorArg2 & D3DTA_ALPHAREPLICATE &&
            tsi->colorOp != D3DTOP_SELECTARG1)
        {
           tcu_arep |= lpColorOp->areplicate_arg2[ TCU_OP ];
           ccu_arep |= lpColorOp->areplicate_arg2[ CCU_OP ];
        }

        // Handle complemented input arguements. This
        // mask is XOR'd with the working tcu/ccu value.

        tcu_comp = ccu_comp = 0;
        tca_comp = cca_comp = 0;

        if( tsi->colorArg1 & D3DTA_COMPLEMENT &&
            tsi->colorOp != D3DTOP_SELECTARG2)
        {
           tcu_comp = lpColorOp->complement_arg1[ TCU_OP ];
           ccu_comp = lpColorOp->complement_arg1[ CCU_OP ];
        }

        if( tsi->colorArg2 & D3DTA_COMPLEMENT &&
            tsi->colorOp != D3DTOP_SELECTARG1)
        {
           tcu_comp |= lpColorOp->complement_arg2[ TCU_OP ];
           ccu_comp |= lpColorOp->complement_arg2[ CCU_OP ];
        }

        if( tsi->alphaArg1 & D3DTA_COMPLEMENT &&
            tsi->alphaOp != D3DTOP_SELECTARG2)
        {
           tca_comp = lpAlphaOp->complement_arg1[ TCA_OP ];
           cca_comp = lpAlphaOp->complement_arg1[ CCA_OP ];
        }

        if( tsi->alphaArg2 & D3DTA_COMPLEMENT &&
            tsi->alphaOp != D3DTOP_SELECTARG1)
        {
           tca_comp |= lpAlphaOp->complement_arg2[ TCA_OP ];
           cca_comp |= lpAlphaOp->complement_arg2[ CCA_OP ];
        }


//----------------
        // Get the initial combine settings for this stage

        if (esinfo.bump_enabled != BUMPMAP_NONE &&
            (esinfo.UV_stage_1 == i ||
             esinfo.UV_stage_2 == i))
        {
            DWORD push, pop;

            // Set the CCU/TCU registers for matrix multiplication:
            tcuColor = (TCC( ATEX, ATEX, C0, ZERO)
                        | (SST_TA_INV_MINUS_HALF << SST_TA_TCC_A_MODE_SHIFT)
                        | (SST_TA_INV_MINUS_HALF << SST_TA_TCC_B_MODE_SHIFT));
            tcaAlpha = TCA( ZERO, ZERO, ZERO, ZERO);
            ccuColor = (CCC( CTEX, CTEX, C1, CTCU)
                        | (SST_TA_INV_MINUS_HALF << SST_TA_CCC_A_MODE_SHIFT)
                        | (SST_TA_INV_MINUS_HALF << SST_TA_CCC_B_MODE_SHIFT));
            ccaAlpha = CCA( ZERO, ZERO, ZERO, ZERO);

            ptmui->taColorAR0.vFxU32 = setColorConstant (0.0,  tsi->bumpEnvMat10);
            ptmui->taColorGB0.vFxU32 = setColorConstant (tsi->bumpEnvMat11,  0.0);
            ptmui->taColorAR1.vFxU32 = setColorConstant (0.0,  tsi->bumpEnvMat00);
            ptmui->taColorGB1.vFxU32 = setColorConstant (tsi->bumpEnvMat01,  0.0);

            // Now that the registers are setup, the VTA should be set to
            // push at D3D stage i and pop at stage i+1 (for UV) or i+2 (for UVL).

            // Extract and amend push & pop registers:
            push = ((pRc->sst.taControl.vFxU32 & SST_TA_REC_PUSH) >> SST_TA_REC_PUSH_SHIFT);
            pop  = ((pRc->sst.taControl.vFxU32 & SST_TA_REC_POP)  >> SST_TA_REC_POP_SHIFT);
            pRc->sst.taControl.vFxU32 &= ~(SST_TA_REC_PUSH | SST_TA_REC_POP);

            if (esinfo.bump_enabled == BUMPMAP_UV_ON)
            {
                push |= 0x1 << (ns-i-2);
                pop  |= 0x1 << (ns-i-2);
            } else {                    // UVL_ON
                push |= 0x1 << (ns-i-2);
                pop  |= 0x1 << (ns-i-3);
            }
            pRc->sst.taControl.vFxU32 |= ((push << SST_TA_REC_PUSH_SHIFT) |
                                          (pop  << SST_TA_REC_POP_SHIFT));
        }
        else                            // not bumpmapping at D3D stage i
        {
            opIdx = (((tsi->colorArg1 & 0x03) << 2)
                     | (tsi->colorArg2 & 0x03));
            tcuColor  = lpColorOp->arg[ opIdx ][ TCU_OP ];
            ccuColor  = lpColorOp->arg[ opIdx ][ CCU_OP ];

            opIdx = (((tsi->alphaArg1 & 0x03) << 2)
                     | (tsi->alphaArg2 & 0x03));
            tcaAlpha  = lpAlphaOp->arg[ opIdx ][ TCA_OP ];
            ccaAlpha  = lpAlphaOp->arg[ opIdx ][ CCA_OP ];

            // Set luminance scaling (specified with D3DTSS_BUMPENVLSCALE
            // and *LOFFSET) if prior stage was non-luminance bumpmapping.
            // NOTE: this will break if the luminance map stage following the
            // bumpmap does something like colorop==Add && colorArg2==factor
            // which uses C0!  Any use of C1 or C0 will break with this code
            // and UV bumpmapping.
            if (i == esinfo.env_stage &&
                esinfo.bump_enabled != BUMPMAP_NONE)
            {
                float lscale = TS[esinfo.UV_stage_1].bumpEnvlScale,
                    loffset = TS[esinfo.UV_stage_1].bumpEnvlOffset;

                // The twist is BUMPENVMAPLUMINANCE is the op that
                // enables this scaling, so use 1.0 and 0.0 for BUMPENVMAP.
                if (TS[esinfo.UV_stage_1].colorOp == D3DTOP_BUMPENVMAP)
                    lscale = 1.0f, loffset = 0.0f;

                ptmui->taColorAR0.vFxU32 = setColorConstant (0.0f,    lscale);
                ptmui->taColorGB0.vFxU32 = setColorConstant (lscale,  lscale);
                ptmui->taColorAR1.vFxU32 = setColorConstant (0.0f,    loffset);
                ptmui->taColorGB1.vFxU32 = setColorConstant (loffset, loffset);
                ccuColor = CCC( CTEX, ZERO, C0, C1);
                // final luminance per texel = ctex * lscale + loffset
            }
            else
            {
                // This should not interfere with bump and bump+1 stages
                ptmui->taColorAR0.vFxU32 = pRc->textureFactorAR;
                ptmui->taColorGB0.vFxU32 = pRc->textureFactorGB;
                ptmui->taColorAR1.vFxU32 = pRc->constantFactorAR;
                ptmui->taColorGB1.vFxU32 = pRc->constantFactorGB;

                // Set up scaling factor for C1 RGB.  Preserve C1 Alpha.
                if (tsi->colorOp == D3DTOP_MODULATE4X ||
                    tsi->colorOp == D3DTOP_DOTPRODUCT3)
                {
                    ptmui->taColorAR1.vFxU32 = ((0x0000ffff & setColorConstant (0.0f, 4.0f)) |
                                                (0xffff0000 & ptmui->taColorAR1.vFxU32));
                    ptmui->taColorGB1.vFxU32 = setColorConstant (4.0f, 4.0f);
                }
                else if (tsi->colorOp == D3DTOP_MODULATE2X ||
                         tsi->colorOp == D3DTOP_ADDSIGNED2X)
                {
                    ptmui->taColorAR1.vFxU32 = ((0x0000ffff & setColorConstant (0.0f, 2.0f)) |
                                                (0xffff0000 & ptmui->taColorAR1.vFxU32));
                    ptmui->taColorGB1.vFxU32 = setColorConstant (2.0f, 2.0f);
                }

                // Set up scaling factor for C1 Alpha.  Preserve C1 RGB.
                if (tsi->alphaOp == D3DTOP_MODULATE4X)
                {
                    ptmui->taColorAR1.vFxU32 = ((0xffff0000 & setColorConstant (4.0f, 0.0f)) |
                                                (0x0000ffff & ptmui->taColorAR1.vFxU32));
                }
                else if (tsi->alphaOp == D3DTOP_MODULATE2X ||
                         tsi->alphaOp == D3DTOP_ADDSIGNED2X)
                {
                    ptmui->taColorAR1.vFxU32 = ((0xffff0000 & setColorConstant (2.0f, 0.0f)) |
                                                (0x0000ffff & ptmui->taColorAR1.vFxU32));
                }
            }
        }  // if
        UPDATE_HW_STATE( reg3D.TMU[i].taColorAR0.group | reg3D.TMU[i].taColorGB0.group |
                         reg3D.TMU[i].taColorAR1.group | reg3D.TMU[i].taColorGB1.group );

//----------------

        // validate that input selction and tcu_arep are compatible

        tcuTemp  = tcuColor & ~TCC_MASK_INPUT;  // Get ABCD
        for( abcd_pos=0; abcd_pos < 16; abcd_pos+=4)
        {
           if( (((tcuTemp >> abcd_pos) & 0xF) < SST_TA_TCC_CPREV) ||
               (((tcuTemp >> abcd_pos) & 0xF) > SST_TA_TCC_C0) )
           {
               // Clear alpha replicate mask in the current position
               tcu_arep &= ~( 0xF << abcd_pos);
           }
        }

        ccuTemp  = ccuColor & ~CCC_MASK_INPUT;  // Get ABCD
        for( abcd_pos=0; abcd_pos < 20; abcd_pos+=5)
        {
           if( (((ccuTemp >> abcd_pos) & 0x1F) < SST_TA_CCC_CPREV) ||
               (((ccuTemp >> abcd_pos) & 0x1F) > SST_TA_CCC_CTCU) )
           {
               // Clear alpha replicate mask in the current position
               ccu_arep &= ~( 0x1F << abcd_pos);
           }
        }

        tcuTemp  = tcuColor & TCC_MASK_INPUT;

        tcuTemp  |= (((tcuColor & SST_TA_TCC_A_SELECT) +
                     (tcu_inc  & SST_TA_TCC_A_SELECT) +
                     (tcu_arep & SST_TA_TCC_A_SELECT))
                    & SST_TA_TCC_A_SELECT );

        tcuTemp |= (((tcuColor & SST_TA_TCC_B_SELECT) +
                     (tcu_inc  & SST_TA_TCC_B_SELECT) +
                     (tcu_arep & SST_TA_TCC_B_SELECT))
                    & SST_TA_TCC_B_SELECT );

        tcuTemp |= (((tcuColor & SST_TA_TCC_C_SELECT) +
                     (tcu_inc  & SST_TA_TCC_C_SELECT) +
                     (tcu_arep & SST_TA_TCC_C_SELECT))
                    & SST_TA_TCC_C_SELECT );

        tcuTemp |= (((tcuColor & SST_TA_TCC_D_SELECT) +
                     (tcu_inc  & SST_TA_TCC_D_SELECT) +
                     (tcu_arep & SST_TA_TCC_D_SELECT))
                    & SST_TA_TCC_D_SELECT );

        ccuTemp  = ccuColor & CCC_MASK_INPUT;

        ccuTemp |= (((ccuColor & SST_TA_CCC_A_SELECT) +
                     (ccu_inc  & SST_TA_CCC_A_SELECT) +
                     (ccu_arep & SST_TA_CCC_A_SELECT))
                    & SST_TA_CCC_A_SELECT );

        ccuTemp |= (((ccuColor & SST_TA_CCC_B_SELECT) +
                     (ccu_inc  & SST_TA_CCC_B_SELECT) +
                     (ccu_arep & SST_TA_CCC_B_SELECT))
                    & SST_TA_CCC_B_SELECT );

        ccuTemp |= (((ccuColor & SST_TA_CCC_C_SELECT) +
                     (ccu_inc  & SST_TA_CCC_C_SELECT) +
                     (ccu_arep & SST_TA_CCC_C_SELECT))
                    & SST_TA_CCC_C_SELECT );

        ccuTemp |= (((ccuColor & SST_TA_CCC_D_SELECT) +
                     (ccu_inc  & SST_TA_CCC_D_SELECT) +
                     (ccu_arep & SST_TA_CCC_D_SELECT))
                    & SST_TA_CCC_D_SELECT );

        tcaTemp  = tcaAlpha & TCA_MASK_INPUT;

        tcaTemp |= (((tcaAlpha & SST_TA_TCA_A_SELECT) +
                     (tca_inc  & SST_TA_TCA_A_SELECT)) &
                    SST_TA_TCA_A_SELECT );

        tcaTemp |= (((tcaAlpha & SST_TA_TCA_B_SELECT) +
                     (tca_inc  & SST_TA_TCA_B_SELECT)) &
                    SST_TA_TCA_B_SELECT );

        tcaTemp |= (((tcaAlpha & SST_TA_TCA_C_SELECT) +
                     (tca_inc  & SST_TA_TCA_C_SELECT)) &
                    SST_TA_TCA_C_SELECT );

        tcaTemp |= (((tcaAlpha & SST_TA_TCA_D_SELECT) +
                     (tca_inc  & SST_TA_TCA_D_SELECT)) &
                    SST_TA_TCA_D_SELECT );

        ccaTemp  = ccaAlpha & CCA_MASK_INPUT;

        ccaTemp |= (((ccaAlpha & SST_TA_CCA_A_SELECT) +
                     (cca_inc  & SST_TA_CCA_A_SELECT)) &
                    SST_TA_CCA_A_SELECT );

        ccaTemp |= (((ccaAlpha & SST_TA_CCA_B_SELECT) +
                     (cca_inc  & SST_TA_CCA_B_SELECT)) &
                    SST_TA_CCA_B_SELECT );

        ccaTemp |= (((ccaAlpha & SST_TA_CCA_C_SELECT) +
                     (cca_inc  & SST_TA_CCA_C_SELECT)) &
                    SST_TA_CCA_C_SELECT );

        ccaTemp |= (((ccaAlpha & SST_TA_CCA_D_SELECT) +
                     (cca_inc  & SST_TA_CCA_D_SELECT)) &
                    SST_TA_CCA_D_SELECT );

        // Finally complement inputs as needed

        ptmui->taTcuColor.vFxU32  =  tcuTemp ^ tcu_comp;
        ptmui->taCcuColor.vFxU32  =  ccuTemp ^ ccu_comp;
        ptmui->taTcuAlpha.vFxU32  =  tcaTemp ^ tca_comp;
        ptmui->taCcuAlpha.vFxU32  =  ccaTemp ^ cca_comp;
        UPDATE_HW_STATE(reg3D.TMU[i].taTcuColor.group | reg3D.TMU[i].taTcuAlpha.group |
                        reg3D.TMU[i].taCcuColor.group | reg3D.TMU[i].taCcuAlpha.group   );

        // Once VTA registers are set, ensure [0,1] clamping is enabled
        // except for bumpmapping, which can have signed values.
        if (i != esinfo.UV_stage_1 &&
            i != esinfo.UV_stage_2)
        {
            ptmui->taCcuColor.vFxU32 |= SST_TA_CCC_OUT_CLAMP;
            ptmui->taCcuAlpha.vFxU32 |= SST_TA_CCA_OUT_CLAMP;
        }

        // Bumpmapping: Implementing bumpmapping on the VTA always causes a
        // recursion data wall (RDW) between the bitmap and the perturbed
        // texture.  We have to set taTcuColor's SST_TA_TCC_POP_DWALL bit
        // because of this.  Update taShiftBias with post-w multiplying and
        // ST-shifting.  taShiftBias' {S,T}_BIAS_LOG2 fields are already
        // set in textureSurfaceCreate(), so copy those.
        if (i == esinfo.env_stage)
        {
            txtrDesc *txtr = TXTRHNDL_TO_TXTRDESC(pRc, tsi->textureHandle);

            ptmui->taTcuColor.vFxU32 |= SST_TA_TCC_POP_DWALL;
            ptmui->taShiftBias.vFxU32 = 
                ((1  << SST_TA_REC_POST_WMULT_SHIFT ) |
                 (13 << SST_TA_REC_ST_SHIFT_SHIFT)    |
                 txtr->txTaShiftBias);
            UPDATE_HW_STATE(reg3D.TMU[i].taShiftBias.group);
        }

        // this only applys to the legacy D3DTBLEND_MODULATE renderstate
        if (pRc->texMapBlend == 0x7ffffffe) // denotes legacy D3DTBLEND_MODULATE - see d3rstate.c
        {
            txtrDesc *txtr = TXTRHNDL_TO_TXTRDESC(pRc, tsi->textureHandle);

            // If alpha from texture is selected, but the texture doesn't contain
            // alpha, then the alpha input needs to be redirected.  In the case
            // of D3DTBLEND_MODULATE, the alpha should come from the alpha
            // interpolators.  In all other cases, alpha is 1.0.
            //
            // So speaketh the spec.
            if ( !(txtr->txFormatFlags & (TEXFMTFLG_ALPHA | TEXFMTFLG_PALETTIZED_ALPHA))) 
            {
               ptmui->taTcuAlpha.vFxU32 = ( ptmui->taTcuAlpha.vFxU32 & ~SST_TA_TCA_D_SELECT ) 
                   | (SST_TA_TCA_AITER << SST_TA_TCA_D_SELECT_SHIFT);
            }
        }
      } // for (i<ns)

      pRc->sst.suParamMask.vFxU32 = setup_suParamMask (pRc, ns, &esinfo);
      UPDATE_HW_STATE(reg3D.suParamMask.group);

      postprocess_stages (pRc, ns, &esinfo);

#ifdef DEBUG
      dumpStages (pRc, ns, DL_TSS_CONFIGURED, "VTA Setup");
#endif

      // Now restore *pRC so the D3D state is what
      // the application originally sent.
      for (i=0; i<ns; i++)
      {
          TS[i] = origrc.textureStage[i];
      }

    } // if (textureStageStates changed)
  } // ns > 0
} // setupTexturing()




//-------------------------------------------------------------------
#if defined( DEBUG )  

DWORD opsCt = 0;
DWORD ops[16][12];

unsigned char *textureOpTable[25] =
{
  "ZERO_OR_INVALID",
  "D3DTOP_DISABLE",
  "D3DTOP_SELECTARG1",
  "D3DTOP_SELECTARG2", 
  "D3DTOP_MODULATE", 
  "D3DTOP_MODULATE2X",
  "D3DTOP_MODULATE4X",
  "D3DTOP_ADD",
  "D3DTOP_ADDSIGNED",
  "D3DTOP_ADDSIGNED2X",
  "D3DTOP_SUBTRACT",
  "D3DTOP_ADDSMOOTH",
  "D3DTOP_BLENDDIFFUSEALPHA",
  "D3DTOP_BLENDTEXTUREALPHA",
  "D3DTOP_BLENDFACTORALPHA",
  "D3DTOP_BLENDTEXTUREALPHAPM",
  "D3DTOP_BLENDCURRENTALPHA",
  "D3DTOP_PREMODULATE",
  "D3DTOP_MODULATEALPHA_ADDCOLOR",
  "D3DTOP_MODULATECOLOR_ADDALPHA",
  "D3DTOP_MODULATEINVALPHA_ADDCOLOR",
  "D3DTOP_MODULATEINVCOLOR_ADDALPHA",
  "D3DTOP_BUMPENVMAP",
  "D3DTOP_BUMPENVMAPLUMINANCE",
  "D3DTOP_DOTPRODUCT3"
};  

unsigned char *textureArgTable[64] =
{
  "D3DTA_DIFFUSE ",
  "D3DTA_CURRENT ",
  "D3DTA_TEXTURE ",
  "D3DTA_TFACTOR ",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "D3DTA_DIFFUSE+COMPLEMENT ",
  "D3DTA_CURRENT+COMPLEMENT ",
  "D3DTA_TEXTURE+COMPLEMENT ",
  "D3DTA_TFACTOR+COMPLEMENT ",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "D3DTA_DIFFUSE+ALPHAREP ",
  "D3DTA_CURRENT+ALPHAREP ",
  "D3DTA_TEXTURE+ALPHAREP ",
  "D3DTA_TFACTOR+ALPHAREP ",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "D3DTA_DIFFUSE+ALPHAREP+COMPLEMENT ",
  "D3DTA_CURRENT+ALPHAREP+COMPLEMENT ",
  "D3DTA_TEXTURE+ALPHAREP+COMPLEMENT ",
  "D3DTA_TFACTOR+ALPHAREP+COMPLEMENT ",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a",
  "n/a", " n/a", "n/a", " n/a"
}; 

void checkTextureOp( RC *pRc )
{
  DWORD i;

 
  
  // This is used to determine the texture stage setup modes being used
  for(i = 0; i < opsCt; i++)
  {
    if( ops[i][0] == TS[0].colorOp && ops[i][1]  == TS[0].colorArg1 && ops[i][2]  == TS[0].colorArg2 &&
        ops[i][3] == TS[0].alphaOp && ops[i][4]  == TS[0].alphaArg1 && ops[i][5]  == TS[0].alphaArg2 &&
        ops[i][6] == TS[1].colorOp && ops[i][7]  == TS[1].colorArg1 && ops[i][8]  == TS[1].colorArg2 &&
        ops[i][9] == TS[1].alphaOp && ops[i][10] == TS[1].alphaArg1 && ops[i][11] == TS[1].alphaArg2 )
          break;
  }
  
  if( i == opsCt )
  {
    // New entry
    ops[i][0]  = TS[0].colorOp;
    ops[i][1]  = TS[0].colorArg1;
    ops[i][2]  = TS[0].colorArg2;
    ops[i][3]  = TS[0].alphaOp;
    ops[i][4]  = TS[0].alphaArg1;
    ops[i][5]  = TS[0].alphaArg2;
    ops[i][6]  = TS[1].colorOp;
    ops[i][7]  = TS[1].colorArg1;
    ops[i][8]  = TS[1].colorArg2;
    ops[i][9]  = TS[1].alphaOp;
    ops[i][10] = TS[1].alphaArg1;
    ops[i][11] = TS[1].alphaArg2;
    opsCt++;
    
    D3DPRINT( 56, "Stage 0" );
    D3DPRINT( 56, "  ColorOp =%d, Arg1 =%d, Arg2 =%d", TS[0].colorOp, TS[0].colorArg1, TS[0].colorArg2 );
    D3DPRINT( 56, "  AlphaOp =%d, Arg1 =%d, Arg2 =%d", TS[0].alphaOp, TS[0].alphaArg1, TS[0].alphaArg2 );      
    D3DPRINT( 56, "Stage 1" );
    D3DPRINT( 56, "  ColorOp =%d, Arg1 =%d, Arg2 =%d", TS[1].colorOp, TS[1].colorArg1, TS[1].colorArg2 );
    D3DPRINT( 56, "  AlphaOp =%d, Arg1 =%d, Arg2 =%d", TS[1].alphaOp, TS[1].alphaArg1, TS[1].alphaArg2 );      
    D3DPRINT( 56, "" );

    D3DPRINT( 56, "Stage 0" );
    D3DPRINT( 56, "ColorOp= %s, Arg1= %s, Arg2= %s", 
                     textureOpTable[TS[0].colorOp ], 
                     textureArgTable[TS[0].colorArg1 ], 
                     textureArgTable[TS[0].colorArg2 ] );
    D3DPRINT( 56, "AlphaOp= %s, Arg1= %s, Arg2= %s", 
                  textureOpTable[TS[0].alphaOp ], 
                  textureArgTable[TS[0].alphaArg1 ], 
                  textureArgTable[TS[0].alphaArg2 ] );

    D3DPRINT( 56, "Stage 1" );
    D3DPRINT( 56, "ColorOp= %s, Arg1= %s, Arg2= %s", 
                     textureOpTable[TS[1].colorOp ], 
                     textureArgTable[TS[1].colorArg1 ], 
                     textureArgTable[TS[1].colorArg2 ] );
    D3DPRINT( 56, "AlphaOp= %s, Arg1= %s, Arg2= %s", 
                     textureOpTable[TS[1].alphaOp ], 
                     textureArgTable[TS[1].alphaArg1 ], 
                     textureArgTable[TS[1].alphaArg2 ] );
  }
}

static void
dumpStages(RC *pRc, int ns, int level, char *msg)
{
    int i,vi;                           // i=d3d, vi=tmu
    OURTMUREGS *ptmui;

    D3DPRINT(level, "--- Texture Stages (VTA) for %s:", msg);
    for (i=0,vi=ns-1; i<ns; i++,vi--)
    {
        TEXTURESTAGESTATE *ts = &TS[i];
        ptmui = &pRc->sst.TMU[i];

        D3DPRINT(level, "D3D Stage %d (TMU %d):  paramMask=0x%x", i, vi,
                 (unsigned)(pRc->sst.suParamMask.vFxU32 & (0xf << (4*vi))) >> (4*vi));
        D3DPRINT(level, "  Color:%2d(%2d,%2d) %10s(%s,%s)",
                 ts->colorOp,
                 ts->colorArg1,
                 ts->colorArg2,
                 &textureOpTable[ts->colorOp][7],
                 &textureArgTable[ts->colorArg1][6],
                 &textureArgTable[ts->colorArg2][6]);
        D3DPRINT(level, "  Alpha:%2d(%2d,%2d) %10s(%s,%s)",
                 ts->alphaOp,
                 ts->alphaArg1,
                 ts->alphaArg2,
                 &textureOpTable[ts->alphaOp][7],
                 &textureArgTable[ts->alphaArg1][6],
                 &textureArgTable[ts->alphaArg2][6]);
    }
    if (level == DL_TSS_CONFIGURED)         // post-configuration settings
    {
        D3DPRINT(level, "VTA Register settings:   taControl = %08x",
                 pRc->sst.taControl.vFxU32);
        D3DPRINT(level, "  TMU tcuAlpha tcuColor ccuAlpha ccuColor PM taCcuControl");
        for (i=0,vi=ns-1; i<ns; i++,vi--)
        {
            ptmui = &pRc->sst.TMU[i];
            D3DPRINT(level, "  %2d: %08x %08x %08x %08x %2x %08x", vi,
                     ptmui->taTcuAlpha.vFxU32, ptmui->taTcuColor.vFxU32,
                     ptmui->taCcuAlpha.vFxU32, ptmui->taCcuColor.vFxU32,
                     (unsigned)(pRc->sst.suParamMask.vFxU32 & (0xf << (4*vi))) >> (4*vi),
                     ptmui->taCcuControl.vFxU32);
        }
    }
} // dumpStages ()

#endif
//-------------------------------------------------------------------
