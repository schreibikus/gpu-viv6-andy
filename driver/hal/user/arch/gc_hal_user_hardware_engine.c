/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_hal_user_hardware_precomp.h"
#include "gc_hal_engine.h"

#if gcdENABLE_3D

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

#define gcdAUTO_SHIFT 0

#define gcmMAKE_F16(Sign, Exponent, Mantissa)  (gctUINT16)(((Sign) << 15) | ((Exponent) << 10) | (Mantissa))
static gctUINT16 _Float2Float16(IN gctUINT32 Value)
{
    gctUINT16 result;
    gctINT32 exponent = ((Value & 0x7f800000) >> 23) - 127;
    gctINT32 sign  = (Value & 0x80000000) >> 31;

    if (exponent <= -15)
    {
        /* Undeflow; set to zero. */
        result = (gctUINT16)gcmMAKE_F16(sign, 0, 0);
    }
    else if (exponent > 15)
    {
        /* Overflow; set to infinity. */
        result = (gctUINT16)gcmMAKE_F16(sign, 31, 0);
    }
    else
    {
        /* Strip a number of bits. */
        result = (gctUINT16)gcmMAKE_F16(sign, exponent + 15, (Value >> 13) & 0x03ff);
    }

    /* Return result. */
    return result;
}

static gceSTATUS
_AutoSetEarlyDepth(
    IN gcoHARDWARE Hardware,
    OUT gctBOOL_PTR Enable OPTIONAL
    )
{
    gctBOOL enable = Hardware->PEStates->earlyDepth;

    if (!Hardware->features[gcvFEATURE_EARLY_Z]
        && !Hardware->features[gcvFEATURE_NEW_RA])
    {
        enable = gcvFALSE;
    }

    /* GC500 does not support early-depth when tile status is enabled at D16. */
    else
    if ((Hardware->config->chipModel == gcv500) &&
        (Hardware->config->chipRevision < 3) &&
        (Hardware->PEStates->depthStates.surface != gcvNULL) &&
        (Hardware->PEStates->depthStates.surface->format == gcvSURF_D16))
    {
        enable = gcvFALSE;
    }

    /* Early-depth doesn't work if depth compare is NOT EQUAL. */
    else
    if (Hardware->PEStates->depthStates.compare == gcvCOMPARE_NOT_EQUAL)
    {
        enable = gcvFALSE;
    }

    /* Early-depth should be disabled when stencil mode is not set to keep. */
    else
    if ((Hardware->PEStates->stencilStates.mode != gcvSTENCIL_NONE) &&
        (!Hardware->PEStates->stencilKeepFront[0] ||
         !Hardware->PEStates->stencilKeepFront[1] ||
         !Hardware->PEStates->stencilKeepFront[2]))
    {
        enable = gcvFALSE;
    }

    else
    if ((Hardware->PEStates->depthStates.surface != gcvNULL) &&
        (!Hardware->PEStates->depthStates.surface->hzDisabled))
    {
        enable = gcvFALSE;
    }

    /* Early-depth should be disabled if no depth component. */
    else
    if ((Hardware->PEStates->depthStates.surface != gcvNULL)
    &&  (Hardware->PEStates->depthStates.surface->format == gcvSURF_X24S8)
    )
    {
        enable = gcvFALSE;
    }

#if defined(ANDROID)
    if (Hardware->patchID == gcvPATCH_FISHNOODLE)
    {
        enable = gcvFALSE;
    }
#endif

    if (Enable != gcvNULL)
    {
        *Enable = enable;
    }

    if (Hardware->PEStates->depthStates.early != enable)
    {
        Hardware->PEStates->depthStates.early = enable;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
    }

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
_AutoSetColorAddressing(
    IN gcoHARDWARE Hardware
    )
{
    gctUINT i;
    gctBOOL singleBuffer8x4 = gcvFALSE;

    if ((Hardware->PEStates->depthStates.surface) &&
        (Hardware->PEStates->depthStates.surface->formatInfo.bitsPerPixel <= 16))
    {
        singleBuffer8x4 = gcvTRUE;
    }
    for (i = 0; i < Hardware->config->renderTargets; ++i)
    {
        gcsSURF_INFO_PTR surface = Hardware->PEStates->colorStates.target[i].surface;
        if ((surface != gcvNULL) &&
            (surface->formatInfo.bitsPerPixel <= 16))
        {
            singleBuffer8x4 = gcvTRUE;
            break;
        }
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX])
    {
        gctBOOL has8BitRT = gcvFALSE;
        gctBOOL all8BitRT = gcvTRUE;

        for (i = 0; i < Hardware->config->renderTargets; ++i)
        {
            gcsSURF_INFO_PTR surface = Hardware->PEStates->colorStates.target[i].surface;
            if (surface != gcvNULL)
            {
                if ((surface->formatInfo.bitsPerPixel <= 8) &&
                    (surface->sampleInfo.product == 1))
                {
                    has8BitRT = gcvTRUE;
                }
                else
                {
                    all8BitRT = gcvFALSE;
                }
            }
        }

        Hardware->PEStates->hasOne8BitRT = has8BitRT && (!all8BitRT);
    }

    Hardware->PEStates->singleBuffer8x4 = singleBuffer8x4;

    return gcvSTATUS_OK;
}

#define SANP 0x10
static gctINT8 _GuardbandTable[4][16] =
    {{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f},
     {SANP,SANP,SANP,SANP,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f},
     {SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f},
     {SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP,SANP},
    };

gceSTATUS
gcoHARDWARE_SetPSOutputMapping(
    IN gcoHARDWARE Hardware,
    IN gctINT32 * psOutputMapping
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x psOutputMapping=0x%x", Hardware, psOutputMapping);

    gcmGETHARDWARE(Hardware);

    gcoOS_MemCopy(Hardware->SHStates->psOutputMapping, psOutputMapping, Hardware->config->renderTargets * sizeof(gctINT32));

    /* Dirty shader to reprogram ps output control */
    Hardware->SHDirty->shaderDirty |= gcvPROGRAM_STAGE_FRAGMENT_BIT;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetShaderLayered(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->SHStates->shaderLayered != Enable)
    {
        Hardware->SHStates->shaderLayered = Enable;
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
    }

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_IsProgramSwitched(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->SHDirty->programSwitched)
    {
        status = gcvTRUE;
    }
    else
    {
        status = gcvFALSE;
    }

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_SetRenderLayered(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable,
    IN gctUINT  MaxLayers
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d MaxLayers=0x%x", Hardware, Enable, MaxLayers);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if ((Hardware->SHStates->rtLayered != Enable) ||
        (Hardware->SHStates->maxLayers != MaxLayers))
    {
        Hardware->SHStates->rtLayered = Enable;
        Hardware->SHStates->maxLayers = MaxLayers;

        Hardware->PEDirty->colorConfigDirty = gcvTRUE;
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetRenderTarget(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 TargetIndex,
    IN gcsSURF_INFO_PTR Surface,
    IN gctUINT32 SliceIndex,
    IN gctUINT32 LayerIndex
    )
{
    gcsSURF_INFO_PTR oldSurf;
    gcsCOLOR_TARGET *pColorTarget;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x TargetIndex=%d Surface=0x%x LayerIndex=%d",
                   Hardware, TargetIndex, Surface, LayerIndex);

    gcmASSERT(TargetIndex < Hardware->config->renderTargets);

    gcmGETHARDWARE(Hardware);

    pColorTarget = &Hardware->PEStates->colorStates.target[TargetIndex];
    oldSurf = pColorTarget->surface;

   /* Set current render target. */
    pColorTarget->surface = Surface;
    pColorTarget->sliceIndex = SliceIndex;
    pColorTarget->layerIndex = LayerIndex;

    if (Surface != gcvNULL)
    {
        /* Make sure the surface is locked. */
        gcmASSERT(Surface->node.valid);

        /* Save configuration. Keep it for non-oes3 driver*/
        Hardware->MsaaStates->sampleInfo = Surface->sampleInfo;

        /* Alpha blend should be turned off explicitly by driver for integer RT*/
        if (Hardware->features[gcvFEATURE_HALTI2])
        {
            Hardware->PEDirty->alphaDirty = gcvTRUE;
        }
        gcoHARDWARE_SetHWSlot(Hardware, gcvENGINE_RENDER, gcvHWSLOT_RT, Surface->node.u.normal.node, TargetIndex);

    }
    else
    {
        gcoHARDWARE_SetHWSlot(Hardware, gcvENGINE_RENDER, gcvHWSLOT_RT, 0, TargetIndex);
    }

    _AutoSetColorAddressing(Hardware);

    /* Schedule the surface to be reprogrammed. */
    Hardware->PEDirty->colorTargetDirty = gcvTRUE;
    Hardware->PEDirty->colorConfigDirty = gcvTRUE;

    if (TargetIndex == 0)
    {
        gctBOOL oldFakeFmt = oldSurf && oldSurf->formatInfo.fakedFormat;
        gctBOOL newFakeFmt = Surface && Surface->formatInfo.fakedFormat;

        /* Mark dither dirty if fake format changed */
        if (oldFakeFmt != newFakeFmt)
        {
            Hardware->PEDirty->peDitherDirty = gcvTRUE;
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthBuffer(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR Surface,
    IN gctUINT32 SliceIndex
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x", Hardware, Surface);

    gcmGETHARDWARE(Hardware);

    if (Hardware->features[gcvFEATURE_128BTILE] &&
        Hardware->PEStates->depthStates.surface != Surface)
    {
        /* Surface change, we need flush CacheMode in FlushTarget. */
        Hardware->PEDirty->colorConfigDirty = gcvTRUE;
    }

    /* Set current depth buffer. */
    Hardware->PEStates->depthStates.surface = Surface;
    Hardware->PEStates->depthStates.sliceIndex = SliceIndex;

    if (Surface)
    {
        /* Save configuration. Keep it for non-es3 driver */
        Hardware->MsaaStates->sampleInfo = Surface->sampleInfo;

        gcoHARDWARE_SetHWSlot(Hardware, gcvENGINE_RENDER, gcvHWSLOT_DEPTH_STENCIL, Surface->node.u.normal.node, 0);
    }
    else
    {
        gcoHARDWARE_SetHWSlot(Hardware, gcvENGINE_RENDER, gcvHWSLOT_DEPTH_STENCIL, 0, 0);
    }

    /* Set super-tiling. */
    Hardware->PEStates->depthStates.regDepthConfig =
        ((((gctUINT32) (Hardware->PEStates->depthStates.regDepthConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) ((Surface == gcvNULL) ? 0 : Surface->superTiled) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)));

    /* Enable/disable Early Depth. */
    gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

    gcmONERROR(_AutoSetColorAddressing(Hardware));

    /* Make depth and stencil dirty. */
    Hardware->PEDirty->depthConfigDirty        = gcvTRUE;
    Hardware->PEDirty->depthTargetDirty        = gcvTRUE;
    Hardware->PEDirty->depthNormalizationDirty = gcvTRUE;
    Hardware->PEDirty->stencilDirty            = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetViewport(
    IN gcoHARDWARE Hardware,
    IN gctINT32 Left,
    IN gctINT32 Top,
    IN gctINT32 Right,
    IN gctINT32 Bottom
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Left=%d Top=%d Right=%d Bottom=%d",
                  Hardware, Left, Top, Right, Bottom);

    gcmGETHARDWARE(Hardware);

    /* Save the states. */
    Hardware->PAAndSEStates->viewportStates.left   = Left;
    Hardware->PAAndSEStates->viewportStates.top    = Top;
    Hardware->PAAndSEStates->viewportStates.right  = Right;
    Hardware->PAAndSEStates->viewportStates.bottom = Bottom;
    Hardware->PAAndSEDirty->viewportDirty         = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetScissors(
    IN gcoHARDWARE Hardware,
    IN gctINT32 Left,
    IN gctINT32 Top,
    IN gctINT32 Right,
    IN gctINT32 Bottom
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Left=%d Top=%d Right=%d Bottom=%d",
                  Hardware, Left, Top, Right, Bottom);

    gcmGETHARDWARE(Hardware);

    /* Save the states. */
    Hardware->PAAndSEStates->scissorStates.left   = Left;
    Hardware->PAAndSEStates->scissorStates.top    = Top;
    Hardware->PAAndSEStates->scissorStates.right  = Right;
    Hardware->PAAndSEStates->scissorStates.bottom = Bottom;
    Hardware->PAAndSEDirty->scissorDirty         = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----- Alpha Blend States ---------------------------------------------------*/
static gceSTATUS
_AutoSetGolobalBlendEnabled(
    IN gcoHARDWARE Hardware
    )
{
    gctUINT i;
    gctBOOL anyBlendEnabled = gcvFALSE;

    for (i = 0; i < Hardware->config->renderTargets; ++i)
    {
        if (Hardware->PEStates->alphaStates.blend[i])
        {
            anyBlendEnabled = gcvTRUE;
            break;
        }
    }
    Hardware->PEStates->alphaStates.anyBlendEnabled = anyBlendEnabled;

    return gcvSTATUS_OK;
}

gceSTATUS
gcoHARDWARE_SetBlendEnable(
    IN gcoHARDWARE Hardware,
    IN gctUINT Index,
    IN gctBOOL Enabled
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Index=%d Enabled=%d", Hardware, Index, Enabled);

    gcmVERIFY_ARGUMENT(Index < Hardware->config->renderTargets);

    gcmGETHARDWARE(Hardware);

    /* Save the state. */
    Hardware->PEStates->alphaStates.blend[Index] = Enabled;

    _AutoSetGolobalBlendEnabled(Hardware);

    Hardware->PEDirty->alphaDirty        = gcvTRUE;
    Hardware->PEDirty->colorConfigDirty  = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetBlendFunctionSource(
    IN gcoHARDWARE Hardware,
    IN gctUINT Index,
    IN gceBLEND_FUNCTION FunctionRGB,
    IN gceBLEND_FUNCTION FunctionAlpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Index=%d FunctionRGB=%d FunctionAlpha=%d",
                  Hardware, FunctionRGB, FunctionAlpha);

    gcmVERIFY_ARGUMENT(Index < Hardware->config->renderTargets);

    gcmGETHARDWARE(Hardware);

    /* Save the states. */
    Hardware->PEStates->alphaStates.srcFuncColor[Index] = FunctionRGB;
    Hardware->PEStates->alphaStates.srcFuncAlpha[Index] = FunctionAlpha;
    Hardware->PEDirty->alphaDirty               = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetBlendFunctionTarget(
    IN gcoHARDWARE Hardware,
    IN gctUINT Index,
    IN gceBLEND_FUNCTION FunctionRGB,
    IN gceBLEND_FUNCTION FunctionAlpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Index=%d FunctionRGB=%d FunctionAlpha=%d",
                  Hardware, Index, FunctionRGB, FunctionAlpha);

    gcmVERIFY_ARGUMENT(Index < Hardware->config->renderTargets);

    gcmGETHARDWARE(Hardware);

    /* Save the states. */
    Hardware->PEStates->alphaStates.trgFuncColor[Index] = FunctionRGB;
    Hardware->PEStates->alphaStates.trgFuncAlpha[Index] = FunctionAlpha;
    Hardware->PEDirty->alphaDirty               = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetBlendMode(
    IN gcoHARDWARE Hardware,
    IN gctUINT  Index,
    IN gceBLEND_MODE ModeRGB,
    IN gceBLEND_MODE ModeAlpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Index=%d ModeRGB=%d ModeAlpha=%d",
                  Hardware, Index, ModeRGB, ModeAlpha);

    gcmVERIFY_ARGUMENT(Index < Hardware->config->renderTargets);

    gcmGETHARDWARE(Hardware);

    /* Save the states. */
    Hardware->PEStates->alphaStates.modeColor[Index] = ModeRGB;
    Hardware->PEStates->alphaStates.modeAlpha[Index] = ModeAlpha;
    Hardware->PEDirty->alphaDirty            = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetBlendColor(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Red,
    IN gctUINT8 Green,
    IN gctUINT8 Blue,
    IN gctUINT8 Alpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Red=%d Green=%d Blue=%d Alpha=%d",
                  Hardware, Red, Green, Blue, Alpha);

    gcmGETHARDWARE(Hardware);

    /* Save the state. */
    Hardware->PEStates->alphaStates.color
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (Red) & ((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (Green) & ((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (Blue) & ((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:24) - (0 ?
 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (Alpha) & ((gctUINT32) ((((1 ? 31:24) - (0 ?
 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24)));
    Hardware->PEDirty->alphaDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetBlendColorX(
    IN gcoHARDWARE Hardware,
    IN gctFIXED_POINT Red,
    IN gctFIXED_POINT Green,
    IN gctFIXED_POINT Blue,
    IN gctFIXED_POINT Alpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8 red, green, blue, alpha;
    gctFIXED_POINT redSat, greenSat, blueSat, alphaSat;

    gcmHEADER_ARG("Hardware=0x%x Red=%d Green=%d Blue=%d Alpha=%d",
                  Hardware, Red, Green, Blue, Alpha);

    gcmGETHARDWARE(Hardware);

    /* Saturate fixed point values. */
    redSat   = gcmMIN(gcmMAX(Red, 0), gcvONE_X);
    greenSat = gcmMIN(gcmMAX(Green, 0), gcvONE_X);
    blueSat  = gcmMIN(gcmMAX(Blue, 0), gcvONE_X);
    alphaSat = gcmMIN(gcmMAX(Alpha, 0), gcvONE_X);

    /* Convert fixed point to 8-bit. */
    red   = (gctUINT8) (gcmXMultiply(redSat, (255 << 16)) >> 16);
    green = (gctUINT8) (gcmXMultiply(greenSat, (255 << 16)) >> 16);
    blue  = (gctUINT8) (gcmXMultiply(blueSat, (255 << 16)) >> 16);
    alpha = (gctUINT8) (gcmXMultiply(alphaSat, (255 << 16)) >> 16);

    /* Save the state. */
    Hardware->PEStates->alphaStates.color
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (red) & ((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (green) & ((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (blue) & ((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:24) - (0 ?
 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (alpha) & ((gctUINT32) ((((1 ? 31:24) - (0 ?
 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24)));
    Hardware->PEDirty->alphaDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetBlendColorF(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT Red,
    IN gctFLOAT Green,
    IN gctFLOAT Blue,
    IN gctFLOAT Alpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8 red, green, blue, alpha;

    gcmHEADER_ARG("Hardware=0x%x Red=%f Green=%f Blue=%f Alpha=%f",
                  Hardware, Red, Green, Blue, Alpha);

    gcmGETHARDWARE(Hardware);

    /* Convert floating point to 8-bit. */
    red   = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(Red, 0.0f), 1.0f));
    green = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(Green, 0.0f), 1.0f));
    blue  = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(Blue, 0.0f), 1.0f));
    alpha = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(Alpha, 0.0f), 1.0f));

    /* Save the state. */
    Hardware->PEStates->alphaStates.color
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (red) & ((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (green) & ((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (blue) & ((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:24) - (0 ?
 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (alpha) & ((gctUINT32) ((((1 ? 31:24) - (0 ?
 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24)));
    Hardware->PEDirty->alphaDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----- Depth States. --------------------------------------------------------*/

gceSTATUS
gcoHARDWARE_SetDepthCompare(
    IN gcoHARDWARE Hardware,
    IN gceCOMPARE DepthCompare
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x DepthCompare=%d", Hardware, DepthCompare);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the depth compare. */
    if (Hardware->PEStates->depthStates.compare != DepthCompare)
    {
        Hardware->PEStates->depthStates.compare = DepthCompare;

        /* Enable/disable Eearly Depth. */
        gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

        Hardware->PEDirty->depthConfigDirty    = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthWrite(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save write enable. */
    if (Hardware->PEStates->depthStates.write != Enable)
    {
        Hardware->PEStates->depthStates.write = Enable;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthMode(
    IN gcoHARDWARE Hardware,
    IN gceDEPTH_MODE DepthMode
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x DepthMode=%d", Hardware, DepthMode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save depth type. */
    if (Hardware->PEStates->depthStates.mode != DepthMode)
    {
        Hardware->PEStates->depthStates.mode = DepthMode;
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthRangeX(
    IN gcoHARDWARE Hardware,
    IN gceDEPTH_MODE DepthMode,
    IN gctFIXED_POINT Near,
    IN gctFIXED_POINT Far
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x DepthMode=%d Near=%f Far=%f",
                  Hardware, DepthMode,
                  gcoMATH_Fixed2Float(Near),
                  gcoMATH_Fixed2Float(Far));

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save depth type. */
    Hardware->PEStates->depthStates.mode = DepthMode;
    Hardware->PEStates->depthStates.near = gcoMATH_Fixed2Float(Near);
    Hardware->PEStates->depthStates.far  = gcoMATH_Fixed2Float(Far);
    Hardware->PEDirty->depthConfigDirty = gcvTRUE;
    Hardware->PEDirty->depthRangeDirty  = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthRangeF(
    IN gcoHARDWARE Hardware,
    IN gceDEPTH_MODE DepthMode,
    IN gctFLOAT Near,
    IN gctFLOAT Far
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x DepthMode=%d Near=%f Far=%f",
                  Hardware, DepthMode, Near, Far);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save depth type. */
    if (Hardware->PEStates->depthStates.mode != DepthMode)
    {
        Hardware->PEStates->depthStates.mode = DepthMode;
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
    }

    if ((Hardware->PEStates->depthStates.near != Near)
     || (Hardware->PEStates->depthStates.far  != Far))
    {
        Hardware->PEStates->depthStates.near = Near;
        Hardware->PEStates->depthStates.far  = Far;
        Hardware->PEDirty->depthRangeDirty  = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthPlaneF(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT Near,
    IN gctFLOAT Far
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Near=%f Far=%f",
                  Hardware, Near, Far);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save depth plane. */
    Hardware->PEStates->depthNear.f      = Near;
    Hardware->PEStates->depthFar.f       = Far;
    Hardware->PEDirty->depthConfigDirty = gcvTRUE;
    Hardware->PEDirty->depthRangeDirty  = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetLastPixelEnable(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);


    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));
    gcmONERROR(gcoHARDWARE_LoadState32(
        Hardware, 0x00C18, Enable
        ));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthScaleBiasX(
    IN gcoHARDWARE Hardware,
    IN gctFIXED_POINT DepthScale,
    IN gctFIXED_POINT DepthBias
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x DepthScale=%d DepthBias=%d",
                  Hardware, DepthScale, DepthBias);

    gcmGETHARDWARE(Hardware);

    if (!Hardware->features[gcvFEATURE_DEPTH_BIAS_FIX])
    {
        DepthScale = 0x0;
        DepthBias  = 0x0;
    }

    /* Switch to 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

    /* Program the scale register. */
    gcmONERROR(gcoHARDWARE_LoadState32x(
        Hardware, 0x00C10, DepthScale
        ));

    /* Program the bias register. */
    gcmONERROR(gcoHARDWARE_LoadState32x(
        Hardware, 0x00C14, DepthBias
        ));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthScaleBiasF(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT DepthScale,
    IN gctFLOAT DepthBias
    )
{
    gceSTATUS status;
    gcuFLOAT_UINT32 scale, bias;

    gcmHEADER_ARG("Hardware=0x%x DepthScale=%f DepthBias=%f",
                  Hardware, DepthScale, DepthBias);

    gcmGETHARDWARE(Hardware);

    if (!Hardware->features[gcvFEATURE_DEPTH_BIAS_FIX])
    {
        scale.f = 0.0f;
        bias.f  = 0.0f;
    }
    else
    {
        scale.f = DepthScale;
        bias.f  = DepthBias;
    }

    /* Switch to 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

    /* Program the scale register. */
    gcmONERROR(gcoHARDWARE_LoadState32(
        gcvNULL, 0x00C10, scale.u
        ));

    /* Program the bias register. */
    gcmONERROR(gcoHARDWARE_LoadState32(
        gcvNULL, 0x00C14, bias.u
        ));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_AutoSetGolobalColorWrite(
    IN gcoHARDWARE Hardware
    )
{
    gctUINT i;
    gctBOOL anyPartialColorWrite = gcvFALSE;
    gctBOOL allColorWriteOff = gcvTRUE;

    for (i = 0; i < Hardware->config->renderTargets; ++i)
    {
        if (Hardware->PEStates->colorStates.target[i].colorWrite != 0xF)
        {
            anyPartialColorWrite = gcvTRUE;
            break;
        }
    }
    for (i = 0; i < Hardware->config->renderTargets; ++i)
    {
        if (Hardware->PEStates->colorStates.target[i].colorWrite != 0)
        {
            allColorWriteOff = gcvFALSE;
        }
    }

    Hardware->PEStates->colorStates.anyPartialColorWrite = anyPartialColorWrite;
    Hardware->PEStates->colorStates.allColorWriteOff = allColorWriteOff;

    return gcvSTATUS_OK;
}

gceSTATUS
gcoHARDWARE_SetColorWrite(
    IN gcoHARDWARE Hardware,
    IN gctUINT Index,
    IN gctUINT8 Enable
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Index=%d Enable=%d", Hardware, Index, Enable);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_ARGUMENT(Index < Hardware->config->renderTargets);

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, gcvNULL));

    if (Hardware->PEStates->colorStates.target[Index].colorWrite != Enable &&
        !Hardware->features[gcvFEATURE_PE_DITHER_COLORMASK_FIX])
    {
        /* Reprogram PE dither table. */
        Hardware->PEDirty->peDitherDirty        = gcvTRUE;
    }


    /* Save the color write. */
    Hardware->PEStates->colorStates.target[Index].colorWrite = Enable;

    _AutoSetGolobalColorWrite(Hardware);

    Hardware->PEDirty->colorConfigDirty              = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetEarlyDepth(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Only handle when the IP has Early Depth or NEW_RA. */
    if (Hardware->features[gcvFEATURE_EARLY_Z]
        || Hardware->features[gcvFEATURE_NEW_RA])
    {
        /* Set early depth flag. */
        Hardware->PEStates->earlyDepth = Enable;

        /* Auto enable/disable early depth. */
        gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/* Used to disable all early-z culling modes
   and manage raster Z control states. */
gceSTATUS
gcoHARDWARE_SetAllEarlyDepthModes(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Disable,
    IN gctBOOL DisableRAModifyZ,
    IN gctBOOL DisableRAPassZ
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Disable=%d DisableRAModifyZ=%d DisableRAPassZ=%d ",
                   Hardware, Disable, DisableRAModifyZ, DisableRAPassZ);

    gcmGETHARDWARE(Hardware);

    if ((Hardware->PEStates->disableAllEarlyDepth != Disable)
     || (Hardware->PEStates->disableRAModifyZ     != DisableRAModifyZ)
     || (Hardware->PEStates->disableRAPassZ       != DisableRAPassZ))
    {
        /* Set disable all early depth flag. */
        Hardware->PEStates->disableAllEarlyDepth = Disable;
        Hardware->PEStates->disableRAModifyZ     = DisableRAModifyZ;
        Hardware->PEStates->disableRAPassZ       = DisableRAPassZ;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty  = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetEarlyDepthFromAPP(
    IN gcoHARDWARE Hardware,
    IN gctBOOL EarlyDepthFromAPP
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Disable=%d", Hardware, EarlyDepthFromAPP);

    gcmGETHARDWARE(Hardware);

    if (Hardware->MsaaStates->EarlyDepthFromAPP != EarlyDepthFromAPP)
    {
        Hardware->MsaaStates->EarlyDepthFromAPP = EarlyDepthFromAPP;
        Hardware->MsaaDirty->msaaConfigDirty  = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/* Used to disable RA Depth Write. */
gceSTATUS
gcoHARDWARE_SetRADepthWrite(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Disable,
    IN gctBOOL psReadZ,
    IN gctBOOL psReadW
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Disable=%d", Hardware, Disable);

    gcmGETHARDWARE(Hardware);

    if (Hardware->MsaaStates->disableRAWriteDepth != Disable)
    {
        Hardware->MsaaStates->disableRAWriteDepth = Disable;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty  = gcvTRUE;
    }

    if(Hardware->MsaaStates->psReadZ != psReadZ)
    {
        Hardware->MsaaStates->psReadZ = psReadZ;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty  = gcvTRUE;
    }

    if(Hardware->MsaaStates->psReadW != psReadW)
    {
        Hardware->MsaaStates->psReadW = psReadW;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty  = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_SetPatchVertices(
    IN gcoHARDWARE Hardware,
    IN gctINT PatchVertices
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Hardware=0x%x PatchVertices=%d", Hardware, PatchVertices);

    if(Hardware->features[gcvFEATURE_TESSELLATION])
    {
        status = gcoHARDWARE_LoadState32(Hardware, 0x007C0, PatchVertices);
    }
    gcmFOOTER();
    return status;
}


/* Used to switch early-z culling mode. */
gceSTATUS
gcoHARDWARE_SwitchDynamicEarlyDepthMode(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Set disable all early depth flag. */
    Hardware->PEStates->disableAllEarlyDepthFromStatistics = !Hardware->PEStates->disableAllEarlyDepthFromStatistics;
    Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
    Hardware->PEDirty->depthTargetDirty  = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/* Used to set dynamic early-z culling mode. */
gceSTATUS
gcoHARDWARE_DisableDynamicEarlyDepthMode(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Disable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Disable=%d", Hardware, Disable);

    gcmGETHARDWARE(Hardware);

    if (Hardware->PEStates->disableAllEarlyDepthFromStatistics != Disable)
    {
        /* Set disable all early depth flag. */
        Hardware->PEStates->disableAllEarlyDepthFromStatistics = Disable;
        Hardware->PEDirty->depthConfigDirty  = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty  = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}



/*------------------------------------------------------------------------------
**----- Stencil States -------------------------------------------------------*/

gceSTATUS
gcoHARDWARE_SetStencilMode(
    IN gcoHARDWARE Hardware,
    IN gceSTENCIL_MODE Mode
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Mode=%d", Hardware, Mode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save stencil mode. */
    Hardware->PEStates->stencilStates.mode = Mode;
    Hardware->PEDirty->stencilDirty       = gcvTRUE;

    /* Enable/disable Eearly Depth. */
    gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilMask(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Mask
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mask=0x%x", Hardware, Mask);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil mask. */
    Hardware->PEStates->stencilStates.maskFront = Mask;
    Hardware->PEDirty->stencilDirty            = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilMaskBack(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Mask
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mask=0x%x", Hardware, Mask);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil mask. */
    Hardware->PEStates->stencilStates.maskBack = Mask;
    Hardware->PEDirty->stencilDirty           = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilWriteMask(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Mask
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mask=0x%x", Hardware, Mask);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil write mask. */
    Hardware->PEStates->stencilStates.writeMaskFront = Mask;
    Hardware->PEDirty->stencilDirty                 = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilWriteMaskBack(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Mask
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mask=0x%x", Hardware, Mask);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil write mask. */
    Hardware->PEStates->stencilStates.writeMaskBack = Mask;
    Hardware->PEDirty->stencilDirty                = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilReference(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Reference,
    IN gctBOOL Front
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Reference=0x%x Front=%d",
                  Hardware, Reference, Front);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil reference. */
    if (Front)
    {
        Hardware->PEStates->stencilStates.referenceFront = Reference;
    }
    else
    {
        Hardware->PEStates->stencilStates.referenceBack = Reference;
    }
    Hardware->PEDirty->stencilDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilCompare(
    IN gcoHARDWARE Hardware,
    IN gceSTENCIL_WHERE Where,
    IN gceCOMPARE Compare
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Where=%d Compare=%d",
                  Hardware, Where, Compare);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil compare. */
    if (Where == gcvSTENCIL_FRONT)
    {
        Hardware->PEStates->stencilStates.compareFront = Compare;
    }
    else
    {
        Hardware->PEStates->stencilStates.compareBack = Compare;
    }
    Hardware->PEDirty->stencilDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilPass(
    IN gcoHARDWARE Hardware,
    IN gceSTENCIL_WHERE Where,
    IN gceSTENCIL_OPERATION Operation
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Where=%d Operation=%d",
                  Hardware, Where, Operation);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil pass operation. */
    if (Where == gcvSTENCIL_FRONT)
    {
        Hardware->PEStates->stencilStates.passFront = Operation;
        Hardware->PEStates->stencilKeepFront[0]     = (Operation == gcvSTENCIL_KEEP);
    }
    else
    {
        Hardware->PEStates->stencilStates.passBack = Operation;
        Hardware->PEStates->stencilKeepBack[0]     = (Operation == gcvSTENCIL_KEEP);
    }

    /* Enable/disable Eearly Depth. */
    gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

    /* Invalidate stencil. */
    Hardware->PEDirty->stencilDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilFail(
    IN gcoHARDWARE Hardware,
    IN gceSTENCIL_WHERE Where,
    IN gceSTENCIL_OPERATION Operation
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Where=%d Operation=%d",
                  Hardware, Where, Operation);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil fail operation. */
    if (Where == gcvSTENCIL_FRONT)
    {
        Hardware->PEStates->stencilStates.failFront = Operation;
        Hardware->PEStates->stencilKeepFront[1]     = (Operation == gcvSTENCIL_KEEP);
    }
    else
    {
        Hardware->PEStates->stencilStates.failBack = Operation;
        Hardware->PEStates->stencilKeepBack[1]     = (Operation == gcvSTENCIL_KEEP);
    }

    /* Enable/disable Eearly Depth. */
    gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

    /* Invalidate stencil. */
    Hardware->PEDirty->stencilDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilDepthFail(
    IN gcoHARDWARE Hardware,
    IN gceSTENCIL_WHERE Where,
    IN gceSTENCIL_OPERATION Operation
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Where=%d Operation=%d",
                  Hardware, Where, Operation);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the stencil depth fail operation. */
    if (Where == gcvSTENCIL_FRONT)
    {
        Hardware->PEStates->stencilStates.depthFailFront = Operation;
        Hardware->PEStates->stencilKeepFront[2]          = (Operation == gcvSTENCIL_KEEP);
    }
    else
    {
        Hardware->PEStates->stencilStates.depthFailBack = Operation;
        Hardware->PEStates->stencilKeepBack[2]          = (Operation == gcvSTENCIL_KEEP);
    }

    /* Enable/disable Eearly Depth. */
    gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

    /* Invalidate stencil. */
    Hardware->PEDirty->stencilDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetStencilAll(
    IN gcoHARDWARE Hardware,
    IN gcsSTENCIL_INFO_PTR Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Info=0x%x", Hardware, Info);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(Info != gcvNULL);

    /* We only need to update stencil states if there is a stencil buffer. */
    if ((Hardware->PEStates->depthStates.surface         != gcvNULL)
    &&  (Hardware->PEStates->depthStates.surface->format == gcvSURF_D24S8 ||
         Hardware->PEStates->depthStates.surface->format == gcvSURF_S8 ||
         Hardware->PEStates->depthStates.surface->format == gcvSURF_X24S8)
    )
    {
        /* Copy the states. */
       Hardware->PEStates->stencilStates = *Info;

        /* Some more updated based on a few states. */
        Hardware->PEStates->stencilKeepFront[0]
            = (Info->passFront      == gcvSTENCIL_KEEP);
        Hardware->PEStates->stencilKeepFront[1]
            = (Info->failFront      == gcvSTENCIL_KEEP);
        Hardware->PEStates->stencilKeepFront[2]
            = (Info->depthFailFront == gcvSTENCIL_KEEP);

        Hardware->PEStates->stencilKeepBack[0]
            = (Info->passBack      == gcvSTENCIL_KEEP);
        Hardware->PEStates->stencilKeepBack[1]
            = (Info->failBack      == gcvSTENCIL_KEEP);
        Hardware->PEStates->stencilKeepBack[2]
            = (Info->depthFailBack == gcvSTENCIL_KEEP);

        /* Enable/disable Eearly Depth. */
        gcmONERROR(_AutoSetEarlyDepth(Hardware, gcvNULL));

        /* Mark stencil states as dirty. */
        Hardware->PEDirty->stencilDirty = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----- Alpha Test States ----------------------------------------------------*/

gceSTATUS
gcoHARDWARE_SetAlphaTest(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the state. */
    Hardware->PEStates->alphaStates.test = Enable;
    Hardware->PEDirty->alphaDirty       = gcvTRUE;

    /* RAZ not support alpha test, switch to PEZ. */
    if(Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
    {
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAlphaCompare(
    IN gcoHARDWARE Hardware,
    IN gceCOMPARE Compare
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Compare=%d", Hardware, Compare);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the state. */
    Hardware->PEStates->alphaStates.compare = Compare;
    Hardware->PEDirty->alphaDirty          = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAlphaReference(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Reference,
    IN gctFLOAT FloatReference
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Reference=%u FloatReference=%f", Hardware, Reference, FloatReference);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the state. */
    Hardware->PEStates->alphaStates.reference = Reference;
    Hardware->PEStates->alphaStates.floatReference = FloatReference;
    Hardware->PEDirty->alphaDirty            = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAlphaReferenceX(
    IN gcoHARDWARE Hardware,
    IN gctFIXED_POINT Reference
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctFIXED_POINT saturated;
    gctUINT8 reference;

    gcmHEADER_ARG("Hardware=0x%x Reference=%f",
                  Hardware, gcoMATH_Fixed2Float(Reference));

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Saturate fixed point between 0 and 1. */
    saturated = gcmMIN(gcmMAX(Reference, 0), gcvONE_X);

    /* Convert fixed point to gctUINT8. */
    reference = (gctUINT8) (gcmXMultiply(saturated, 255 << 16) >> 16);

    /* Save the state. */
    Hardware->PEStates->alphaStates.reference = reference;
    *(gctUINT32 *)&(Hardware->PEStates->alphaStates.floatReference) = 0xFFFFFFFF; /*Set a Invalid value for fixed pipe*/
    Hardware->PEDirty->alphaDirty            = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAlphaReferenceF(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT Reference
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctFLOAT saturated;
    gctUINT8 reference;

    gcmHEADER_ARG("Hardware=0x%x Reference=%f", Hardware, Reference);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Saturate floating point between 0 and 1. */
    saturated = gcmMIN(gcmMAX(Reference, 0.0f), 1.0f);

    /* Convert floating point to gctUINT8. */
    reference = gcoMATH_Float2NormalizedUInt8(saturated);

    /* Save the state. */
    Hardware->PEStates->alphaStates.reference = reference;
    Hardware->PEStates->alphaStates.floatReference = Reference;
    Hardware->PEDirty->alphaDirty            = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAlphaAll(
    IN gcoHARDWARE Hardware,
    IN gcsALPHA_INFO_PTR Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Info=0x%x", Hardware, Info);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(Info != gcvNULL);

    /* Copy the states. */
    gcoOS_MemCopy(&Hardware->PEStates->alphaStates, Info, gcmSIZEOF(Hardware->PEStates->alphaStates));

    /* Mark alpha states as dirty. */
    Hardware->PEDirty->alphaDirty = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----------------------------------------------------------------------------*/

gceSTATUS
gcoHARDWARE_BindIndex(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 HeadAddress,
    IN gctUINT32 TailAddress,
    IN gceINDEX_TYPE IndexType,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x HeadAddress=0x%x TailAddress=0x%x IndexType=%d",
                  Hardware, HeadAddress, TailAddress, IndexType);


    gcmGETHARDWARE(Hardware);

    Hardware->FEStates->indexEndian = 0x0;

    switch (IndexType)
    {
    case gcvINDEX_8:
        Hardware->FEStates->indexFormat = 0x0;
        Hardware->FEStates->restartElement = 0xFF;
        break;

    case gcvINDEX_16:
        Hardware->FEStates->indexFormat = 0x1;
        Hardware->FEStates->restartElement = 0xFFFF;

        if (Hardware->bigEndian)
        {
            Hardware->FEStates->indexEndian = 0x1;
        }
        break;

    case gcvINDEX_32:
        if (Hardware->features[gcvFEATURE_FE20_BIT_INDEX])
        {
            Hardware->FEStates->indexFormat = 0x2;
            Hardware->FEStates->restartElement = 0xFFFFFFFF;

            if (Hardware->bigEndian)
            {
                Hardware->FEStates->indexEndian = 0x1;
            }
            break;
        }

        /* Fall through. */
    default:
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Save AQ_INDEX_STREAM_CTRL register value. */
    if(HeadAddress)
    {
        Hardware->FEStates->indexHeadAddress = HeadAddress;
        Hardware->FEDirty->indexHeadDirty = gcvTRUE;
    }
    if(TailAddress)
    {
        Hardware->FEStates->indexTailAddress = TailAddress;
        Hardware->FEDirty->indexTailDirty = gcvTRUE;
    }

    Hardware->FEDirty->indexDirty = Hardware->FEDirty->indexHeadDirty | Hardware->FEDirty->indexTailDirty;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAPI(
    IN gcoHARDWARE Hardware,
    IN gceAPI Api
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 system = 0;

    gcmHEADER_ARG("Hardware=0x%x Api=%d", Hardware, Api);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Don't do anything if the pipe has already been selected. */
    if (Hardware->currentApi != Api)
    {
        /* Set new pipe. */
        Hardware->currentApi = Api;

        /* Set PA System Mode: D3D or OPENGL. */
        switch (Api)
        {
        case gcvAPI_OPENGL_ES11:
        case gcvAPI_OPENGL_ES20:
        case gcvAPI_OPENGL_ES30:
        case gcvAPI_OPENGL:
        case gcvAPI_OPENVG:
        case gcvAPI_OPENCL:
            Hardware->api = gcvAPI_OPENGL;
            system = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
            break;

        case gcvAPI_D3D:
            Hardware->api = gcvAPI_D3D;
            system = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        Hardware->MsaaDirty->centroidsDirty = gcvTRUE;

        gcmONERROR(gcoHARDWARE_LoadState32(Hardware, 0x00A28, system));

        /* Set Common API Mode: OpenGL, OpenVG, or OpenCL.
           This is for VS and PS to handle texel conversion. */
        switch (Api)
        {
        case gcvAPI_OPENGL_ES11:
        case gcvAPI_OPENGL_ES20:
        case gcvAPI_OPENGL_ES30:
        case gcvAPI_OPENGL:
            system = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)));
            break;

        case gcvAPI_OPENVG:
            system = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)));
            break;

        case gcvAPI_OPENCL:
            Hardware->api = gcvAPI_OPENCL;
            system = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)));
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmONERROR(gcoHARDWARE_LoadState32(Hardware, 0x0384C, system));
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetAPI(
    IN gcoHARDWARE Hardware,
    OUT gceAPI * CurrentApi,
    OUT gceAPI * Api
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (CurrentApi)
    {
        *CurrentApi = Hardware->currentApi;
    }

    if (Api)
    {
        *Api = Hardware->api;
    }

    /* Success. */
    gcmFOOTER_ARG("*CurrentApi=0x%x *Api=0x%x",
                  gcmOPT_VALUE(CurrentApi), gcmOPT_VALUE(Api));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----- Primitive assembly states. -------------------------------------------*/

gceSTATUS
gcoHARDWARE_SetAntiAliasLine(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.aaLine = Enable;
    Hardware->PAAndSEDirty->paConfigDirty   = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAALineTexSlot(
    IN gcoHARDWARE Hardware,
    IN gctUINT TexSlot
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x TexSlot=%d", Hardware, TexSlot);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.aaLineTexSlot = TexSlot;
    Hardware->PAAndSEDirty->paConfigDirty          = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetShading(
    IN gcoHARDWARE Hardware,
    IN gceSHADING Shading
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Shading=%d", Hardware, Shading);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Shading != Hardware->PAAndSEStates->paStates.shading)
    {
        /* Set the state. */
        Hardware->PAAndSEStates->paStates.shading = Shading;
        Hardware->PAAndSEDirty->paConfigDirty    = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetCulling(
    IN gcoHARDWARE Hardware,
    IN gceCULL Mode
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mode=%d", Hardware, Mode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    if (Hardware->PAAndSEStates->paStates.culling != Mode)
    {
        Hardware->PAAndSEStates->paStates.culling = Mode;
        Hardware->PAAndSEDirty->paConfigDirty    = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetPointSizeEnable(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.pointSize = Enable;
    Hardware->PAAndSEDirty->paConfigDirty      = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetPointSprite(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.pointSprite = Enable;
    Hardware->PAAndSEDirty->paConfigDirty        = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetPrimitiveIdEnable(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.primitiveId = Enable;
    Hardware->PAAndSEDirty->paConfigDirty        = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetFill(
    IN gcoHARDWARE Hardware,
    IN gceFILL Mode
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mode=%d", Hardware, Mode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.fillMode = Mode;
    Hardware->PAAndSEDirty->paConfigDirty     = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetAALineWidth(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT Width
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Width=%f", Hardware, Width);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    Hardware->PAAndSEStates->paStates.aaLineWidth = Width;
    Hardware->PAAndSEDirty->paLineDirty          = gcvTRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetMultiGPURenderingMode(
    IN gcoHARDWARE Hardware,
    IN gceMULTI_GPU_RENDERING_MODE Mode
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Mode=%d", Hardware, Mode);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the multi GPU rendering mode. */
    Hardware->gpuRenderingMode = Mode;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_SetMultiGPURenderingMode(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR Surface,
    IN gceMULTI_GPU_RENDERING_MODE Mode,
    INOUT gctPOINTER * Memory
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gctUINT32    gpuCoreCount;
    gcsRECT     *renderWindow = gcvNULL;
    gctUINT32   control;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x Mode=%d Memory=0x%x", Hardware, Surface, Mode, Memory);

    gpuCoreCount = Hardware->config->gpuCoreCount;

    if (Surface == gcvNULL || gcoHARDWARE_DrawOnOneCore(Hardware))
    {
        Mode = gcvMULTI_GPU_RENDERING_MODE_OFF;
    }

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    switch (Mode)
    {
    case gcvMULTI_GPU_RENDERING_MODE_OFF:
        control = 0x0;
        break;

    case gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_64x64:
        control = 0x4;
        break;

    case gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x64:
        control = 0x5;
        break;

    case gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x128:
        control = 0x6;
        break;
    case gcvMULTI_GPU_RENDERING_MODE_SPLIT_WIDTH:
    case gcvMULTI_GPU_RENDERING_MODE_SPLIT_HEIGHT:
    default:
        {
            gctUINT32 i;

            gcmASSERT(Surface);

            gcmVERIFY_OK(
                gcoOS_AllocateMemory(gcvNULL,
                                     sizeof(gcsRECT) * gpuCoreCount,
                                     (gctPOINTER *)&renderWindow));

            if (Mode == gcvMULTI_GPU_RENDERING_MODE_SPLIT_WIDTH)
            {
                for (i = 0; i < gpuCoreCount; i++)
                {
                    if (i == 0)
                    {
                        renderWindow[i].left   = 0;
                        renderWindow[i].top    = 0;
                        renderWindow[i].right  = gcmALIGN(Surface->alignedW / gpuCoreCount, 64);
                        renderWindow[i].bottom = Surface->alignedH;
                    }
                    else
                    {
                        renderWindow[i].left   = renderWindow[i - 1].right;
                        renderWindow[i].top    = 0;
                        renderWindow[i].right  =
                            (i == (gpuCoreCount - 1)) ? Surface->alignedW
                                                      : (gcmALIGN(Surface->alignedW
                                                         / gpuCoreCount, 64)
                                                        * (i + 1));
                        renderWindow[i].bottom = Surface->alignedH;
                    }
                }
            }
            else if (Mode == gcvMULTI_GPU_RENDERING_MODE_SPLIT_HEIGHT)
            {
                for (i = 0; i < gpuCoreCount; i++)
                {
                    if (i == 0)
                    {
                        renderWindow[i].left   = 0;
                        renderWindow[i].top    = 0;
                        renderWindow[i].right  = Surface->alignedW;
                        renderWindow[i].bottom = gcmALIGN(Surface->alignedH / gpuCoreCount, 64);
                    }
                    else
                    {
                        renderWindow[i].left   = 0;
                        renderWindow[i].top    = renderWindow[i - 1].bottom;
                        renderWindow[i].right  = Surface->alignedW;
                        renderWindow[i].bottom =
                            (i == (gpuCoreCount - 1)) ? Surface->alignedH
                                                      : (gcmALIGN(Surface->alignedH / gpuCoreCount, 64)
                                                        * (i + 1));
                    }
                }
            }

            control = 0x1;

            /* Set render windows for each 3D core. */
            for (i = 0; i < gpuCoreCount; i++)
            {
                { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (1 << i);
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", (1 << i));
 } };



                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E81) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E81, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (renderWindow[i].left) & ((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ?
 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (renderWindow[i].top) & ((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E82) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E82, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (renderWindow[i].right) & ((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ?
 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (renderWindow[i].bottom) & ((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            }

            /* Enable all 3D cores. */
            { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK;
 memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };


            break;
        }
    }

    if (control != 0x1)
    {
        /* Flat map set[i] to core[i]. */
        control |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (gpuCoreCount - 1) & ((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 13:13) - (0 ?
 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ?
 13:13)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 18:18) - (0 ?
 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)))
                 ;
    }

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E80) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E80, control);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    if (renderWindow != gcvNULL)
    {
        gcoOS_FreeMemory(gcvNULL, (gctPOINTER)renderWindow);
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----------------------------------------------------------------------------*/

gceSTATUS
gcoHARDWARE_InitializeCL(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->features[gcvFEATURE_TX_DESCRIPTOR])
    {
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                           0x14C40,
                                           ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))));
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*------------------------------------------------------------------------------
**----------------------------------------------------------------------------*/

gceSTATUS
gcoHARDWARE_Initialize3D(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;
    gctFLOAT limit;
    gctUINT32 paControl = 0x00000000;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->features[gcvFEATURE_32_ATTRIBUTES])
    {
        gcmONERROR(gcoHARDWARE_LoadState32(
            Hardware, 0x007D8,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ?
 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))));
    }
    else
    {
        /* AQVertexElementConfig. */
        gcmONERROR(gcoHARDWARE_LoadState32(
            Hardware, 0x03814,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
            ));
    }

    /* AQRasterControl. */
    gcmONERROR(gcoHARDWARE_LoadState32(
        Hardware, 0x00E00,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        ));

    /* W Clip limit. */
    limit = 1.0f / 8388607.0f;
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                       0x00A2C,
                                       *(gctUINT32 *) &limit));

    /* Color conversion control initial state. */
    gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
        0x014A4,
        (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10)))),
        (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10))))
        ));

    if (Hardware->config->chipModel == gcv1000 && Hardware->config->chipRevision < 0x5035)
    {
        /* 0x0382. */
        gcmONERROR(gcoHARDWARE_LoadState32(
            gcvNULL, 0x00E08,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            ));
    }

    if (Hardware->features[gcvFEATURE_HALTI2])
    {
        /* 0x0383 */
        gcmONERROR(gcoHARDWARE_LoadState32(
            Hardware, 0x00E0C, 0));
    }

    if (Hardware->features[gcvFEATURE_PA_FARZCLIPPING_FIX])
    {
        Hardware->PEStates->depthStates.regDepthConfig = ((((gctUINT32) (Hardware->PEStates->depthStates.regDepthConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18)));

    }
    else
    {
        paControl = ((((gctUINT32) (paControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
        /* Disable Far Z clipping. */
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                           0x00A88,
                                           paControl));
    }

    if (Hardware->features[gcvFEATURE_ZSCALE_FIX] &&
        gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_ZCONVERT_BYPASS))
    {
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                           0x00A88,
                                           ((((gctUINT32) (paControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))));
    }

    /* Set rounding mode */
    if (Hardware->features[gcvFEATURE_SHADER_HAS_RTNE])
    {
        gcmVERIFY_OK(gcoHARDWARE_SetRTNERounding(Hardware, gcvTRUE));
    }

    if (Hardware->features[gcvFEATURE_TX_LOD_GUARDBAND] &&
        gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_GUARDBAND))
    {
        Hardware->TXStates->hwTxLODGuardbandEnable = gcvTRUE;
        Hardware->TXStates->hwTxLODGuardbandIndex = 0; /* TODO: can choice a better mode later */

        /* Program the guardband table. */
        gcmONERROR(gcoHARDWARE_LoadState(Hardware,
                                         0x14C00, 4*4, _GuardbandTable));
    }

    if (Hardware->config->chipRevision >= 0x5245)
    {
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                           0x00884,
                                           0x0808));
    }

    if (Hardware->features[gcvFEATURE_TX_DESCRIPTOR])
    {
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
                                           0x14C40,
                                           ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))));
    }

    if (Hardware->features[gcvFEATURE_HALTI5])
    {
        gcmONERROR(gcoHARDWARE_LoadCtrlState(Hardware,
                                             0x03848,
                                             ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 22:22) - (0 ?
 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)))
                                           | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19)))));

    }


OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetDepthOnly(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Save the state. */
    Hardware->PEStates->depthStates.only = Enable;
    Hardware->PEDirty->depthConfigDirty = gcvTRUE;

    /* Mark shaders as dirty. */
    Hardware->SHDirty->shaderDirty |= (gcvPROGRAM_STAGE_VERTEX_BIT | gcvPROGRAM_STAGE_FRAGMENT_BIT);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetCentroids(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Index,
    IN gctPOINTER Centroids
    )
{
    gceSTATUS status;
    /*gcoHARDWARE hardware;*/
    gctUINT32 data[4], i;
    gctUINT8_PTR inputCentroids = (gctUINT8_PTR) Centroids;

    gcmHEADER_ARG("Hardware=0x%x Index=%d Centroids=0x%x",
                    Hardware, Index, Centroids);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcoOS_ZeroMemory(data, sizeof(data));

    for (i = 0; i < 16; ++i)
    {
        switch (i & 3)
        {
        case 0:
            /* Set for centroid 0, 4, 8, or 12. */
            data[i / 4] = ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                             | ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2 + 1]) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)));
            break;

        case 1:
            /* Set for centroid 1, 5, 9, or 13. */
            data[i / 4] = ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2]) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                             | ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2 + 1]) & ((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)));
            break;

        case 2:
            /* Set for centroid 2, 6, 10, or 14. */
            data[i / 4] = ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2]) & ((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
                             | ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2 + 1]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)));
            break;

        case 3:
            /* Set for centroid 3, 7, 11, or 15. */
            data[i / 4] = ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
                             | ((((gctUINT32) (data[i / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:28) - (0 ? 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (inputCentroids[i * 2 + 1]) & ((gctUINT32) ((((1 ?
 31:28) - (0 ? 31:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));
            break;
        }
    }

    /* Program the centroid table. */
    gcmONERROR(gcoHARDWARE_LoadState(Hardware,
                                     0x00E40 + Index * 16, 4, data));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_GetSampleCoords(
    IN gcoHARDWARE Hardware,
    IN gctUINT SampleIndex,
    IN gctBOOL yInverted,
    OUT gctFLOAT_PTR Coords
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32 sampleCoords;

    gcmHEADER_ARG("Hardware=0x%x SampleIndex=%d yInverted=%d Coords=0x%x",
                   Hardware, SampleIndex, yInverted, Coords);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    sampleCoords = yInverted ? Hardware->MsaaStates->sampleCoords4[1] : Hardware->MsaaStates->sampleCoords4[0];

    sampleCoords = sampleCoords >> (8 * SampleIndex);

    Coords[0] = (((((gctUINT32) (sampleCoords)) >> (0 ? 3:0)) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1)))))) ) / 16.0f;
    Coords[1] = (((((gctUINT32) (sampleCoords)) >> (0 ? 7:4)) & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1)))))) ) / 16.0f;

OnError:
    gcmFOOTER();
    return status;

}


gceSTATUS
gcoHARDWARE_QuerySurfaceRenderable(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR Surface
    )
{
    gceSTATUS status;
    gctUINT32 format;
    gcsSURF_FORMAT_INFO_PTR info[2];

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x", Hardware, Surface);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmDEBUG_VERIFY_ARGUMENT(Surface != gcvNULL);

    gcmONERROR(gcoSURF_QueryFormat(Surface->format, info));

    do
    {
        if ((Surface->type == gcvSURF_TEXTURE) &&
            (!(Surface->tiling & gcvTILING_SPLIT_BUFFER)) &&
            (Hardware->config->pixelPipes > 1) &&
            (!Hardware->features[gcvFEATURE_SUPERTILED_TEXTURE]))
        {
            status = gcvSTATUS_NOT_ALIGNED;
            break;
        }

        if ((Hardware->config->pixelPipes > 1)
        && (!(Surface->tiling & gcvTILING_SPLIT_BUFFER))
        && !Hardware->features[gcvFEATURE_SINGLE_BUFFER])
        {
            status = gcvSTATUS_NOT_MULTI_PIPE_ALIGNED;
            break;
        }

        if ((Surface->tiling == gcvLINEAR)
        && !Hardware->features[gcvFEATURE_LINEAR_RENDER_TARGET])
        {
            /* Linear render target not supported. */
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* If surface is 16-bit, it has to be 8x4 aligned. */
        if (Surface->bitsPerPixel == 16)
        {
            if ((Surface->alignedW  & 7)
            ||  (Surface->alignedH & 3)
            )
            {
                 status = gcvSTATUS_NOT_ALIGNED;
                 break;
            }
        }

        /* Otherwise, the surface has to be 16x4 aligned. */
        else
        {
            if ((Surface->alignedW  & 15)
            ||  (Surface->alignedH &  3)
            )
            {
                 status = gcvSTATUS_NOT_ALIGNED;
                 break;
            }
        }

        /* Convert the surface format to a hardware format. */
        format = Surface->formatInfo.renderFormat;

        if (format == gcvINVALID_RENDER_FORMAT)
        {
            switch (Surface->format)
            {
            case gcvSURF_D16:
            case gcvSURF_D24X8:
            case gcvSURF_D24S8:
            case gcvSURF_D32:
            case gcvSURF_S8:
            case gcvSURF_X24S8:
                status = gcvSTATUS_OK;
                break;

            default:
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }
        }
    } while (gcvFALSE);

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetLogicOp(
    IN gcoHARDWARE Hardware,
    IN gctUINT8 Rop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Rop=%d", Hardware, Rop);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Update ROP2 code. */
    Hardware->PEStates->colorStates.rop = Rop & 0xF;

    /* Schedule the surface to be reprogrammed. */
    if (Hardware->PEStates->colorStates.rop != 0xC)
    {
        Hardware->PEDirty->colorConfigDirty = gcvTRUE;
    }

    /* Program the scale register. */
    gcmONERROR(gcoHARDWARE_LoadState32WithMask(
        gcvNULL,
        0x014A4,
        (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 4:4) - (0 ?
 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)))),
        (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->colorStates.rop) & ((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ?
 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))))
        ));

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/******************************************************************************\
******************************** State Flushers ********************************
\******************************************************************************/

gceSTATUS
gcoHARDWARE_FlushViewport(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS   status;
    gcsRECT     viewport;
    gctUINT32   xScale, yScale, xOffset, yOffset;
    gctFLOAT    wClip;
    gctFLOAT    wSmall;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if(Hardware->features[gcvFEATURE_PA_FARZCLIPPING_FIX])
    {
        wSmall = 0.0f;
    }
    else
    {
        wSmall = 1.0f / 32768.0f;
    }


    if (Hardware->PAAndSEDirty->viewportDirty)
    {
        if (!Hardware->features[gcvFEATURE_MSAA])
        {
            viewport.left   = Hardware->PAAndSEStates->viewportStates.left   * Hardware->MsaaStates->sampleInfo.x;
            viewport.top    = Hardware->PAAndSEStates->viewportStates.top    * Hardware->MsaaStates->sampleInfo.y;
            viewport.right  = Hardware->PAAndSEStates->viewportStates.right  * Hardware->MsaaStates->sampleInfo.x;
            viewport.bottom = Hardware->PAAndSEStates->viewportStates.bottom * Hardware->MsaaStates->sampleInfo.y;
        }
        else
        {
            viewport.left   = Hardware->PAAndSEStates->viewportStates.left;
            viewport.top    = Hardware->PAAndSEStates->viewportStates.top;
            viewport.right  = Hardware->PAAndSEStates->viewportStates.right;
            viewport.bottom = Hardware->PAAndSEStates->viewportStates.bottom;
        }

        /* Compute viewport states. */
        xScale  = (viewport.right - viewport.left) << 15;
        xOffset = (viewport.left << 16) + xScale;

        if (viewport.top < viewport.bottom)
        {
            yScale = (Hardware->api == gcvAPI_OPENGL)
                         ? (-(viewport.bottom - viewport.top) << 15)
                         : ((viewport.bottom - viewport.top) << 15);
            yOffset = (Hardware->api == gcvAPI_OPENGL)
                          ? (viewport.top << 16) - yScale
                          : (viewport.top << 16) + yScale;
        }
        else
        {
            yScale = (Hardware->api == gcvAPI_OPENGL)
                         ? ((viewport.top - viewport.bottom) << 15)
                         : (-(viewport.top - viewport.bottom) << 15);
            yOffset = (Hardware->api == gcvAPI_OPENGL)
                          ? (viewport.bottom << 16) + yScale
                          : (viewport.bottom << 16) - yScale;
        }

        if ((Hardware->config->chipModel == gcv500) &&
             (Hardware->api == gcvAPI_OPENGL) )
        {
            xOffset -= 0x8000;
            yOffset -= 0x8000;
        }

        wClip = gcmMAX((Hardware->PAAndSEStates->viewportStates.right - Hardware->PAAndSEStates->viewportStates.left),
                       (Hardware->PAAndSEStates->viewportStates.bottom - Hardware->PAAndSEStates->viewportStates.top
                        )) / (2.0f * 8388607.0f - 8192.0f);


        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0280) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0280,
                xScale
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0281,
                yScale
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0283) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0283,
                xOffset
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0284,
                yOffset
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* New PA clip registers. */
        if (Hardware->config->chipModel == gcv4000 && Hardware->config->chipRevision == 0x5222)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x02A0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x02A0, 0xFFFFFFFF);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x02A0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x02A0, *(gctUINT32_PTR) &wClip);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x02A1) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvTRUE, 0x02A1, 8192 << 16);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x02A3) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x02A3, *(gctUINT32_PTR) &wSmall);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

        /* Mark viewport as updated. */
        Hardware->PAAndSEDirty->viewportDirty = gcvFALSE;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushScissor(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gcsRECT scissor;
    gctINT fixRightClip;
    gctINT fixBottomClip;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    if (Hardware->PAAndSEDirty->scissorDirty)
    {
        if (!Hardware->features[gcvFEATURE_MSAA])
        {
            scissor.left   = Hardware->PAAndSEStates->scissorStates.left   * Hardware->MsaaStates->sampleInfo.x;
            scissor.top    = Hardware->PAAndSEStates->scissorStates.top    * Hardware->MsaaStates->sampleInfo.y;
            scissor.right  = Hardware->PAAndSEStates->scissorStates.right  * Hardware->MsaaStates->sampleInfo.x;
            scissor.bottom = Hardware->PAAndSEStates->scissorStates.bottom * Hardware->MsaaStates->sampleInfo.y;
        }
        else
        {
            scissor.left   = Hardware->PAAndSEStates->scissorStates.left;
            scissor.top    = Hardware->PAAndSEStates->scissorStates.top;
            scissor.right  = Hardware->PAAndSEStates->scissorStates.right;
            scissor.bottom = Hardware->PAAndSEStates->scissorStates.bottom;
        }

        /* Trivial case. */
        if ((scissor.left >= scissor.right) || (scissor.top >= scissor.bottom))
        {
            scissor.left   = 1;
            scissor.top    = 1;
            scissor.right  = 1;
            scissor.bottom = 1;
        }

        fixRightClip    = 0;
        fixBottomClip   = 0x1111;

        /* If right or bottom equal 8192, hw will overflow.
           We should decrease the size of Scissor.*/
        if (scissor.right == 8192)

        {

            fixRightClip = -0x119;

        }


        if (scissor.bottom == 8192)

        {

            fixBottomClip = -0x111;

        }

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)4  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (4 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0300) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0300,
                scissor.left << 16
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0301,
                scissor.top << 16
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0302,
                (scissor.right << 16) + fixRightClip
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvTRUE, 0x0303,
                (scissor.bottom << 16) + fixBottomClip
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Program new states for setup clipping. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0308) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvTRUE, 0x0308, (scissor.right << 16) + 0xFFFF);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0309) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvTRUE, 0x0309, (scissor.bottom << 16) + 0xFFFF);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

        /* Mark viewport as updated. */
        Hardware->PAAndSEDirty->scissorDirty = gcvFALSE;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gctBOOL _canEnableAlphaBlend(gcoHARDWARE Hardware, gcsSURF_INFO_PTR rtSurf, gctBOOL originAlphaBlend)
{
    gctBOOL canAlphaBlend;

    if (rtSurf == gcvNULL)
    {
        return originAlphaBlend;
    }

    switch (rtSurf->formatInfo.fmtDataType)
    {
    case gcvFORMAT_DATATYPE_UNSIGNED_INTEGER:
    case gcvFORMAT_DATATYPE_SIGNED_INTEGER:
        canAlphaBlend = gcvFALSE;
        break;
    default:
        canAlphaBlend = originAlphaBlend;
        break;
    }

    return canAlphaBlend;

}


#define gcvINVALID_BLEND_MODE   ~0U

static const gctINT32 xlateAlphaCompare[] =
{
    /* gcvCOMPARE_INVALID */
    -1,
    /* gcvCOMPARE_NEVER */
    0x0,
    /* gcvCOMPARE_NOT_EQUAL */
    0x5,
    /* gcvCOMPARE_LESS */
    0x1,
    /* gcvCOMPARE_LESS_OR_EQUAL */
    0x3,
    /* gcvCOMPARE_EQUAL */
    0x2,
    /* gcvCOMPARE_GREATER */
    0x4,
    /* gcvCOMPARE_GREATER_OR_EQUAL */
    0x6,
    /* gcvCOMPARE_ALWAYS */
    0x7,
};

static const gctUINT32 xlateAlphaFuncSource[] =
{
    /* gcvBLEND_ZERO */
    0x0,
    /* gcvBLEND_ONE */
    0x1,
    /* gcvBLEND_SOURCE_COLOR */
    0x2,
    /* gcvBLEND_INV_SOURCE_COLOR */
    0x3,
    /* gcvBLEND_SOURCE_ALPHA */
    0x4,
    /* gcvBLEND_INV_SOURCE_ALPHA */
    0x5,
    /* gcvBLEND_TARGET_COLOR */
    0x8,
    /* gcvBLEND_INV_TARGET_COLOR */
    0x9,
    /* gcvBLEND_TARGET_ALPHA */
    0x6,
    /* gcvBLEND_INV_TARGET_ALPHA */
    0x7,
    /* gcvBLEND_SOURCE_ALPHA_SATURATE */
    0xA,
    /* gcvBLEND_CONST_COLOR */
    0xD,
    /* gcvBLEND_INV_CONST_COLOR */
    0xE,
    /* gcvBLEND_CONST_ALPHA */
    0xB,
    /* gcvBLEND_INV_CONST_ALPHA */
    0xC,
};

static const gctUINT32 xlateAlphaFuncTarget[] =
{
    /* gcvBLEND_ZERO */
    0x0,
    /* gcvBLEND_ONE */
    0x1,
    /* gcvBLEND_SOURCE_COLOR */
    0x2,
    /* gcvBLEND_INV_SOURCE_COLOR */
    0x3,
    /* gcvBLEND_SOURCE_ALPHA */
    0x4,
    /* gcvBLEND_INV_SOURCE_ALPHA */
    0x5,
    /* gcvBLEND_TARGET_COLOR */
    0x8,
    /* gcvBLEND_INV_TARGET_COLOR */
    0x9,
    /* gcvBLEND_TARGET_ALPHA */
    0x6,
    /* gcvBLEND_INV_TARGET_ALPHA */
    0x7,
    /* gcvBLEND_SOURCE_ALPHA_SATURATE */
    0xA,
    /* gcvBLEND_CONST_COLOR */
    0xD,
    /* gcvBLEND_INV_CONST_COLOR */
    0xE,
    /* gcvBLEND_CONST_ALPHA */
    0xB,
    /* gcvBLEND_INV_CONST_ALPHA */
    0xC,
};

static const gctUINT32 xlateAlphaMode[gcvBLEND_TOTAL] =
{
    0x0, /* gcvBLEND_ADD */
    0x1, /* gcvBLEND_SUBTRACT */
    0x2, /* gcvBLEND_REVERSE_SUBTRACT */
    0x3, /* gcvBLEND_MIN */
    0x4, /* gcvBLEND_MAX */
};


gceSTATUS
_FlushMultiTargetAlpha(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gctUINT32 i;
    gceSTATUS status = gcvSTATUS_OK;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    for (i = 1; i < Hardware->PEStates->colorOutCount; ++i)
    {
        gcsSURF_INFO_PTR surface = Hardware->PEStates->colorStates.target[i].surface;
        gctBOOL canEnableAlphaBlend;
        gctUINT16 reference;
        gctUINT32 colorLow;
        gctUINT32 colorHeigh;
        gctUINT16 colorR;
        gctUINT16 colorG;
        gctUINT16 colorB;
        gctUINT16 colorA;
        gctUINT32 value;
        gctUINT32 alphaControl;

        gcmASSERT(surface);

        canEnableAlphaBlend = _canEnableAlphaBlend(Hardware, surface, Hardware->PEStates->alphaStates.blend[i]);

        value = *(gctUINT32 *) &(Hardware->PEStates->alphaStates.floatReference);

        if (value == 0xFFFFFFFF)
        {
            reference = gcoMATH_UInt8AsFloat16(Hardware->PEStates->alphaStates.reference);
        }
        else
        {
            reference = _Float2Float16(value);
        }

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5250 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5250 + (i-1)), ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->alphaStates.test) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:4) - (0 ?
 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (xlateAlphaCompare[Hardware->PEStates->alphaStates.compare]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (reference) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        alphaControl = 0;

        if (Hardware->features[gcvFEATURE_ADVANCED_BLEND_OPT] &&
            canEnableAlphaBlend)
        {
            gctBOOL modeAdd = Hardware->PEStates->alphaStates.modeColor[i] == gcvBLEND_ADD &&
                                         Hardware->PEStates->alphaStates.modeAlpha[i] == gcvBLEND_ADD;
            gctBOOL modeSub = Hardware->PEStates->alphaStates.modeColor[i] == gcvBLEND_SUBTRACT &&
                                         Hardware->PEStates->alphaStates.modeAlpha[i] == gcvBLEND_SUBTRACT;

            if (Hardware->PEStates->alphaStates.trgFuncAlpha[i] == gcvBLEND_SOURCE_ALPHA_SATURATE &&
                Hardware->PEStates->alphaStates.trgFuncColor[i] == gcvBLEND_SOURCE_ALPHA_SATURATE)
            {
                 /* Src * SrcAlpha + Dst * (1 - SrcAlpha)*/
                 if (modeAdd)
                {
                    /* when (1- srcAlpha) * Dst*/
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)));

                    if (Hardware->PEStates->alphaStates.srcFuncAlpha[i] == gcvBLEND_SOURCE_ALPHA&&
                        Hardware->PEStates->alphaStates.srcFuncColor[i] == gcvBLEND_SOURCE_ALPHA)
                    {
                         /* Src * SrcAlpha + Dst * (1 - SrcAlpha)*/
                        alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
                    }
                }
                else if (modeSub)
                {
                    /* when (1- srcAlpha) * Dst*/
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)));
                }
            }
            else if (modeAdd &&
                       Hardware->PEStates->alphaStates.srcFuncAlpha[i] == gcvBLEND_ONE &&
                       Hardware->PEStates->alphaStates.srcFuncColor[i] == gcvBLEND_ONE &&
                       Hardware->PEStates->alphaStates.trgFuncAlpha[i] == gcvBLEND_ONE &&
                       Hardware->PEStates->alphaStates.trgFuncColor[i] == gcvBLEND_ONE)
            {
                /*src * 1 + Dst * 1*/
                 alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
            }
            else if ((modeAdd || modeSub) &&
                       Hardware->PEStates->alphaStates.trgFuncAlpha[i] == gcvBLEND_INV_SOURCE_COLOR &&
                       Hardware->PEStates->alphaStates.trgFuncColor[i] == gcvBLEND_INV_SOURCE_COLOR)
            {
                /* SrcAlpha = 1 and srccolor = 1, and blend func = (1 - SrcColor) * Dst*/
                 alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)));
            }
        }

        alphaControl |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (xlateAlphaMode[Hardware->PEStates->alphaStates.modeAlpha[i]]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncTarget[Hardware->PEStates->alphaStates.trgFuncAlpha[i]]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncSource[Hardware->PEStates->alphaStates.srcFuncAlpha[i]]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (xlateAlphaMode[Hardware->PEStates->alphaStates.modeColor[i]]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncTarget[Hardware->PEStates->alphaStates.trgFuncColor[i]]) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncSource[Hardware->PEStates->alphaStates.srcFuncColor[i]]) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (canEnableAlphaBlend) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)));

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5258 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5258 + (i-1)),
 alphaControl);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        colorR = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 23:16)) & ((gctUINT32) ((((1 ? 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1)))))) ));
        colorG = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 15:8)) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1)))))) ));
        colorB = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 7:0)) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1)))))) ));
        colorA = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 31:24)) & ((gctUINT32) ((((1 ? 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1)))))) ));
        colorLow = colorR | (colorG << 16);
        colorHeigh = colorB | (colorA << 16);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5260 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5260 + (i-1)),
 colorLow);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5268 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5268 + (i-1)),
 colorHeigh);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    }

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_FlushAlpha(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gctBOOL floatPipe = gcvFALSE;

    static const gctUINT32 xlateAlphaModeAdvanced[gcvBLEND_TOTAL] =
    {
        0x0, /* gcvBLEND_ADD */
        0x0, /* gcvBLEND_SUBTRACT */
        0x0, /* gcvBLEND_REVERSE_SUBTRACT */
        0x0, /* gcvBLEND_MIN */
        0x0, /* gcvBLEND_MAX */
        0x1, /* gcvBLEND_MULTIPLY */
        0x2, /* gcvBLEND_SCREEN */
        0x3, /* gcvBLEND_OVERLAY */
        0x4, /* gcvBLEND_DARKEN */
        0x5, /* gcvBLEND_LIGHTEN */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_COLORDODGE */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_COLORBURN */
        0x6, /* gcvBLEND_HARDLIGHT */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_SOFTLIGHT */
        0x7, /* gcvBLEND_DIFFERENCE */
        0x8, /* gcvBLEND_EXCLUSION */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_HSL_HUE */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_HSL_SATURATION */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_HSL_COLOR */
        gcvINVALID_BLEND_MODE, /* gcvBLEND_HSL_LUMINOSITY */
    };

    gceSTATUS status;

    gctBOOL canEnableAlphaBlend;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->PEDirty->alphaDirty)
    {
        /* Determine the size of the buffer to reserve. */
        floatPipe = Hardware->features[gcvFEATURE_HALF_FLOAT_PIPE];

        if (xlateAlphaModeAdvanced[Hardware->PEStates->alphaStates.modeColor[0]] == gcvINVALID_BLEND_MODE)
        {
            canEnableAlphaBlend = gcvFALSE;
        }
        else
        {
            canEnableAlphaBlend = _canEnableAlphaBlend(Hardware,
                                                       Hardware->PEStates->colorStates.target[0].surface,
                                                       Hardware->PEStates->alphaStates.blend[0]);
        }

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0508) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0508,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->alphaStates.test) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (xlateAlphaCompare[Hardware->PEStates->alphaStates.compare]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->alphaStates.reference) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0509,
                Hardware->PEStates->alphaStates.color
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x050A,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (canEnableAlphaBlend) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (canEnableAlphaBlend) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncSource[Hardware->PEStates->alphaStates.srcFuncColor[0]]) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncSource[Hardware->PEStates->alphaStates.srcFuncAlpha[0]]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncTarget[Hardware->PEStates->alphaStates.trgFuncColor[0]]) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (xlateAlphaFuncTarget[Hardware->PEStates->alphaStates.trgFuncAlpha[0]]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (xlateAlphaMode[Hardware->PEStates->alphaStates.modeColor[0]]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (xlateAlphaMode[Hardware->PEStates->alphaStates.modeAlpha[0]]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)))
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        if (Hardware->features[gcvFEATURE_ADVANCED_BLEND_MODE_PART0] ||
            Hardware->features[gcvFEATURE_ADVANCED_BLEND_OPT])
        {
            gctUINT32 alphaConfig = 0x00000000;

            if (Hardware->features[gcvFEATURE_ADVANCED_BLEND_MODE_PART0] )
            {
                alphaConfig  = ((((gctUINT32) (alphaConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (xlateAlphaModeAdvanced[Hardware->PEStates->alphaStates.modeColor[0]]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
            }

            if (Hardware->features[gcvFEATURE_ADVANCED_BLEND_OPT] &&
                canEnableAlphaBlend)
            {
                gctBOOL modeAdd = Hardware->PEStates->alphaStates.modeColor[0] == gcvBLEND_ADD &&
                                             Hardware->PEStates->alphaStates.modeAlpha[0] == gcvBLEND_ADD;
                gctBOOL modeSub = Hardware->PEStates->alphaStates.modeColor[0] == gcvBLEND_SUBTRACT&&
                                             Hardware->PEStates->alphaStates.modeAlpha[0] == gcvBLEND_SUBTRACT;

                if (Hardware->PEStates->alphaStates.trgFuncAlpha[0] == gcvBLEND_SOURCE_ALPHA_SATURATE &&
                    Hardware->PEStates->alphaStates.trgFuncColor[0] == gcvBLEND_SOURCE_ALPHA_SATURATE)
                {
                     /* Src * SrcAlpha + Dst * (1 - SrcAlpha)*/
                     if (modeAdd)
                    {
                        /* when (1- srcAlpha) * Dst*/
                        alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

                        if (Hardware->PEStates->alphaStates.srcFuncAlpha[0] == gcvBLEND_SOURCE_ALPHA&&
                            Hardware->PEStates->alphaStates.srcFuncColor[0] == gcvBLEND_SOURCE_ALPHA)
                        {
                             /* Src * SrcAlpha + Dst * (1 - SrcAlpha)*/
                            alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));
                        }
                    }
                    else if (modeSub)
                    {
                        /* when (1- srcAlpha) * Dst*/
                        alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));
                    }
                }
                else if (modeAdd &&
                           Hardware->PEStates->alphaStates.srcFuncAlpha[0] == gcvBLEND_ONE &&
                           Hardware->PEStates->alphaStates.srcFuncColor[0] == gcvBLEND_ONE &&
                           Hardware->PEStates->alphaStates.trgFuncAlpha[0] == gcvBLEND_ONE &&
                           Hardware->PEStates->alphaStates.trgFuncColor[0] == gcvBLEND_ONE)
                {
                    /*src * 1 + Dst * 1*/
                     alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));
                }
                else if ((modeAdd || modeSub) &&
                           Hardware->PEStates->alphaStates.trgFuncAlpha[0] == gcvBLEND_INV_SOURCE_COLOR &&
                           Hardware->PEStates->alphaStates.trgFuncColor[0] == gcvBLEND_INV_SOURCE_COLOR)
                {
                    /* SrcAlpha = 1 and srccolor = 1, and blend func = (1 - SrcColor) * Dst*/
                     alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));
                }
            }

            /* Setup the advanced blend mode */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0530) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0530, alphaConfig);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        if (floatPipe)
        {
            gctUINT32 colorLow;
            gctUINT32 colorHeigh;
            gctUINT16 colorR;
            gctUINT16 colorG;
            gctUINT16 colorB;
            gctUINT16 colorA;
            colorR = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 23:16)) & ((gctUINT32) ((((1 ? 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1)))))) ));
            colorG = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 15:8)) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1)))))) ));
            colorB = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 7:0)) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1)))))) ));
            colorA = gcoMATH_UInt8AsFloat16((((((gctUINT32) (Hardware->PEStates->alphaStates.color)) >> (0 ? 31:24)) & ((gctUINT32) ((((1 ? 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1)))))) ));
            colorLow = colorR | (colorG << 16);
            colorHeigh = colorB | (colorA << 16);

            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x052C,
                colorLow
                );
            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );

            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x052D,
                colorHeigh
                );
            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );
        }

        {
            gctUINT16 reference;

            if (floatPipe)
            {
                gctUINT32 value = *(gctUINT32 *) &(Hardware->PEStates->alphaStates.floatReference);

                if (value == 0xFFFFFFFF)
                {
                    reference = gcoMATH_UInt8AsFloat16(Hardware->PEStates->alphaStates.reference);
                }
                else
                {
                    reference = _Float2Float16(value);
                }
            }
            else
            {
                /* Change to 12bit. */
                reference = (Hardware->PEStates->alphaStates.reference << 4) | (Hardware->PEStates->alphaStates.reference >> 4);
            }

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0528) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0528, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 9:9) - (0 ?
 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ?
 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (reference) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ?
 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};



        }

        /* Color conversion control. */
        if (Hardware->PEStates->alphaStates.blend[0]
            && (Hardware->PEStates->alphaStates.srcFuncColor[0] == gcvBLEND_SOURCE_ALPHA)
            && (Hardware->PEStates->alphaStates.trgFuncColor[0] == gcvBLEND_INV_SOURCE_ALPHA))
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

        if (Hardware->features[gcvFEATURE_SEPARATE_RT_CTRL])
        {
            _FlushMultiTargetAlpha(Hardware, Memory);
        }

        /* Mark alpha as updated. */
        Hardware->PEDirty->alphaDirty = gcvFALSE;

#if gcdALPHA_KILL_IN_SHADER
        if (Hardware->SHStates->programState.hints &&
            Hardware->SHStates->programState.hints->killStateAddress != 0)
        {
            /* Mark shaders as dirty. */
            Hardware->SHDirty->shaderDirty |= gcvPROGRAM_STAGE_FRAGMENT_BIT;
        }
#endif
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushStencil(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    static const gctINT8 xlateCompare[] =
    {
        /* gcvCOMPARE_INVALID */
        -1,
        /* gcvCOMPARE_NEVER */
        0x0,
        /* gcvCOMPARE_NOT_EQUAL */
        0x5,
        /* gcvCOMPARE_LESS */
        0x1,
        /* gcvCOMPARE_LESS_OR_EQUAL */
        0x3,
        /* gcvCOMPARE_EQUAL */
        0x2,
        /* gcvCOMPARE_GREATER */
        0x4,
        /* gcvCOMPARE_GREATER_OR_EQUAL */
        0x6,
        /* gcvCOMPARE_ALWAYS */
        0x7
    };

    static const gctUINT8 xlateOperation[] =
    {
        /* gcvSTENCIL_KEEP */
        0x0,
        /* gcvSTENCIL_REPLACE */
        0x2,
        /* gcvSTENCIL_ZERO */
        0x1,
        /* gcvSTENCIL_INVERT */
        0x5,
        /* gcvSTENCIL_INCREMENT */
        0x6,
        /* gcvSTENCIL_DECREMENT */
        0x7,
        /* gcvSTENCIL_INCREMENT_SATURATE */
        0x3,
        /* gcvSTENCIL_DECREMENT_SATURATE */
        0x4,
    };

    gceSTATUS status;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->PEDirty->stencilDirty)
    {
        if (Hardware->PEStates->stencilEnabled)
        {
            /* Setup the stencil config state.
            ** Here we ignored stencilStates.mode but always program TWO_SIDE except gcvFEATURE_MC_STENCIL_CTRL
            ** is there which means MC has separate state to tell D24S8 from D24X8 without PE stencil mode.
            ** As if Hardware->PEStates->stencilEnabled is true, we need tell MC to handle stencil plane
            ** as it exsited, or MC decoding will wrong if D24S8 is compressed in old hardware.
            ** we don't set stencilStates.mode always to be TWO_SIDE as this state also affect earlyZ
            */
            gctINT stencilMode;

            if (Hardware->features[gcvFEATURE_MC_STENCIL_CTRL])
            {
                stencilMode = (Hardware->PEStates->depthStates.mode == gcvDEPTH_NONE)
                            ? gcvSTENCIL_NONE : Hardware->PEStates->stencilStates.mode;
            }
            else
            {
                stencilMode = (Hardware->PEStates->depthStates.mode == gcvDEPTH_Z)
                            ? gcvSTENCIL_DOUBLE_SIDED : gcvSTENCIL_NONE;

            }

            if (Hardware->PEStates->stencilStates.mode != gcvSTENCIL_NONE)
            {
                Hardware->PEStates->depthStates.surface->canDropStencilPlane = gcvFALSE;
            }

            /* Reserve space in the command buffer. */
            gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

            if (Hardware->prevStencilMode != stencilMode)
            {
                if (!Hardware->flushedDepth)
                {
                    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

                    Hardware->flushedDepth = gcvTRUE;
                }
                Hardware->prevStencilMode = stencilMode;
            }

            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0506) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                /* Setup the stencil operation state. */
                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0506,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (xlateCompare[Hardware->PEStates->stencilStates.compareFront]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (xlateOperation[Hardware->PEStates->stencilStates.passFront]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (xlateOperation[Hardware->PEStates->stencilStates.failFront]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (xlateOperation[Hardware->PEStates->stencilStates.depthFailFront]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (xlateCompare[Hardware->PEStates->stencilStates.compareBack]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ?
 18:16)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (xlateOperation[Hardware->PEStates->stencilStates.passBack]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (xlateOperation[Hardware->PEStates->stencilStates.failBack]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ?
 26:24)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (xlateOperation[Hardware->PEStates->stencilStates.depthFailBack]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)))
                    );

                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0507,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (stencilMode) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->stencilStates.referenceFront) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->stencilStates.maskFront) & ((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->stencilStates.writeMaskFront) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24)))
                    );

                gcmSETFILLER_NEW(
                    reserve, memory
                    );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );

            /* Setup the extra reference state. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0528) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0528, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->stencilStates.referenceBack) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};



            /* Setup the extra reference state. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052E) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x052E, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->stencilStates.writeMaskBack) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->stencilStates.maskBack) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            /* No need to flush depth cache bc earlyZ was only enabled when all
            ** depth op are KEEP.
            */

            /* Validate the state buffer. */
            gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);
        }
        else
        {
            /* Reserve space in the command buffer. */
            gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

            /* Disable stencil. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0507) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0507, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            /* Validate the state buffer. */
            gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);
        }

        /* Mark stencil as updated. */
        Hardware->PEDirty->stencilDirty = gcvFALSE;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gctUINT8
_GetColorMask(
    gctUINT8 oldValue,
    gctUINT32 hwFormat,
    gctUINT32 layerIndex
    )
{
    gctUINT8 newColorMask = oldValue;
    if (oldValue != 0xF)
    {
        gctUINT8 rMask = (oldValue & 0x1) >> 0;
        gctUINT8 gMask = (oldValue & 0x2) >> 1;
        gctUINT8 bMask = (oldValue & 0x4) >> 2;
        gctUINT8 aMask = (oldValue & 0x8) >> 3;
        switch(hwFormat)
        {
        case 0x14:
            newColorMask = (aMask << 3) | (bMask << 2) | (rMask << 1) | rMask;
            break;

        case 0x15:
            switch (layerIndex)
            {
            case 0:
                newColorMask =  (gMask << 3) | (gMask << 2) | (rMask << 1) | rMask;
                break;
            case 1:
                newColorMask =  (aMask << 3) | (aMask << 2) | (bMask << 1) | bMask;
                break;

            default:
                gcmASSERT(0);
                break;
            }
        }
    }

    return newColorMask;
}

static gceSTATUS
_FlushMultiTarget(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gctUINT32 i;
    gceSTATUS status = gcvSTATUS_OK;
    gcsCOLOR_TARGET *pColorTarget0;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    pColorTarget0 = &Hardware->PEStates->colorStates.target[0];

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    for (i = 1; i < Hardware->PEStates->colorOutCount; ++i)
    {
        gcsCOLOR_TARGET *pColorTarget = &Hardware->PEStates->colorStates.target[i];
        gcsSURF_INFO_PTR surface = pColorTarget->surface;
        gctUINT32  surfaceBase = 0, physicalBaseAddr0 = 0, physicalBaseAddr1 = 0;
        gctUINT32 rtExtConfig = 0;

        gcmASSERT(surface);

        pColorTarget->format = surface->formatInfo.renderFormat;

        if (pColorTarget->format == gcvINVALID_RENDER_FORMAT)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

        physicalBaseAddr0 = surfaceBase /* surface base */
                          + pColorTarget->sliceIndex * surface->sliceSize  /* slice offset inside one surface or texture mipmap */
                          + pColorTarget->layerIndex * surface->layerSize; /* layer offset inside one slice */

        physicalBaseAddr1 = physicalBaseAddr0 + surface->bottomBufferOffset;

        if (Hardware->features[gcvFEATURE_SRGB_RT_SUPPORT])
        {
            rtExtConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (surface->formatInfo.sRGB) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30)));
        }

        gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", physicalBaseAddr0, surface->node.size);

        if (Hardware->config->renderTargets == 8)
        {
            gctINT tileMode = 0x0;

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5200 + (i-1)*8)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5200 + (i-1)*8),
 physicalBaseAddr0);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            /* even for single buffer PE, we still need to program two same address */
            if (Hardware->config->pixelPipes > 1)
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5200 + (i-1)*8+1)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5200 + (i-1)*8+1),
 physicalBaseAddr1);    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }

            if (Hardware->features[gcvFEATURE_128BTILE])
            {
                if (surface->tiling == gcvSUPERTILED)
                {
                    tileMode = 0x1;
                }
                else if (surface->tiling == gcvYMAJOR_SUPERTILED)
                {
                    tileMode = 0x2;
                }
            }

            if (Hardware->features[gcvFEATURE_PE_MULTI_RT_BLEND_ENABLE_CONTROL])
            {
                gctBOOL canEnableAlphaBlend = _canEnableAlphaBlend(Hardware, surface, Hardware->PEStates->alphaStates.blend[0]);

                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5240 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5240 + (i-1)), ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (surface->stride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ? 25:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (pColorTarget->format) & ((gctUINT32) ((((1 ?
 25:20) - (0 ? 25:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ?
 25:20))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (surface->superTiled) & ((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (canEnableAlphaBlend) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ? 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (tileMode) & ((gctUINT32) ((((1 ?
 27:26) - (0 ? 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }
            else
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x5240 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x5240 + (i-1)), ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (surface->stride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ? 25:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (pColorTarget->format) & ((gctUINT32) ((((1 ?
 25:20) - (0 ? 25:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ?
 25:20))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (surface->superTiled) & ((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | ((((gctUINT32) (rtExtConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ? 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (tileMode) & ((gctUINT32) ((((1 ?
 27:26) - (0 ? 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }

            if (Hardware->features[gcvFEATURE_GEOMETRY_SHADER])
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E28 + i) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E28 + i, (Hardware->SHStates->rtLayered ?
 surface->sliceSize : 0) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }
        }
        else
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x0540 + (i-1)*8)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x0540 + (i-1)*8),
 physicalBaseAddr0);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            if (Hardware->config->pixelPipes > 1)
            {
               {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x0540 + (i-1)*8+1)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x0540 + (i-1)*8+1),
 physicalBaseAddr1);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            }
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((0x0560 + (i-1))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, (0x0560 + (i-1)), ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (surface->stride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:20) - (0 ?
 25:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (pColorTarget->format) & ((gctUINT32) ((((1 ?
 25:20) - (0 ? 25:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ?
 25:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:28) - (0 ?
 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (surface->superTiled) & ((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        }

        pColorTarget->hwColorWrite = _GetColorMask(pColorTarget->colorWrite, pColorTarget->format, pColorTarget->layerIndex);

        if (Hardware->features[gcvFEATURE_SEPARATE_RT_CTRL] ||
            Hardware->features[gcvFEATURE_128BTILE])
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x5248 + (i-1)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x5248 + (i-1), ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (pColorTarget->hwColorWrite) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->colorStates.destinationRead) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            if (pColorTarget->hwColorWrite != pColorTarget0->hwColorWrite)
            {
                gcmPRINT("GAL: different colorMask requirment but only one's set");
            }
        }
    }

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushTarget(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gcsSURF_INFO_PTR surface = gcvNULL;
    gctUINT32 destinationRead = 0;
    gctBOOL flushNeeded = gcvFALSE;
    gctBOOL flushDepthNeeded = gcvFALSE;
    gctUINT batchCount;
    gctBOOL colorBatchLoad;
    gctBOOL colorSplitLoad;
    gctINT colorAddressMode = -1;
    gctUINT physicalBaseAddr0 = 0, physicalBaseAddr1 = 0;
    gcsCOLOR_TARGET *pColorTarget = &Hardware->PEStates->colorStates.target[0];
    gctUINT8 colorWrite = pColorTarget->colorWrite;
    gctBOOL  superTiled = pColorTarget->superTiled;
    gctUINT format = pColorTarget->format;
    gctUINT32 stride = 0;
    gctBOOL colorPipeEnable = ((!Hardware->PEStates->colorStates.allColorWriteOff) && (Hardware->PEStates->colorOutCount != 0))
                              || (Hardware->PEStates->alphaStates.test != 0);

    gctBOOL  sRGB = gcvFALSE;
    gceCACHE_MODE colorCacheMode = gcvCACHE_NONE;
    gctINT tileMode = 0x0;
    gceTILING tiling = gcvINVALIDTILED;
    gctBOOL edgeAA = gcvFALSE;

#if gcdDUMP
    gctUINT32 surfaceSize = 0;
#endif

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* At least configuration has to be dirty. */
    gcmASSERT(Hardware->PEDirty->colorConfigDirty);

    /* Determine destination read control. */
    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        destinationRead = (!Hardware->PEStates->depthStates.realOnly)
                          &&
                          (Hardware->PEStates->alphaStates.anyBlendEnabled      ||
                           Hardware->PEStates->colorStates.anyPartialColorWrite ||
                           Hardware->PEStates->colorStates.rop != 0xC
                          )
                        ? 0x0
                        : 0x1;
    }
    else
    {
        destinationRead = (!Hardware->PEStates->depthStates.realOnly)
                          &&
                          ((Hardware->PEStates->colorStates.colorCompression &&
                            !Hardware->features[gcvFEATURE_COMPRESSION_V4])     ||
                           Hardware->MsaaStates->sampleInfo.product > 1         ||
                           Hardware->PEStates->alphaStates.anyBlendEnabled      ||
                           Hardware->PEStates->colorStates.anyPartialColorWrite ||
                           Hardware->PEStates->colorStates.rop != 0xC
                          )
                        ? 0x0
                        : 0x1;
    }

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
             "Read control: compression=%d anyBlendEnabled=%d anyPartialColorWrite=%d ==> %d",
             Hardware->PEStates->colorStates.colorCompression,
             Hardware->PEStates->alphaStates.anyBlendEnabled,
             Hardware->PEStates->colorStates.anyPartialColorWrite,
             destinationRead);

    /* Determine whether the flush is needed. */
    if (destinationRead != Hardware->PEStates->colorStates.destinationRead)
    {
        /* Flush the cache. */
        flushNeeded = !Hardware->flushedColor;

        /* Update the read control value. */
        Hardware->PEStates->colorStates.destinationRead = destinationRead;
    }


    if (Hardware->PEDirty->colorTargetDirty)
    {
        gctUINT32 cacheMode;

        gcmASSERT(Hardware->PEDirty->colorConfigDirty);

        /* Get a shortcut to the surface. */
        surface = pColorTarget->surface;

        if (surface == gcvNULL)
        {
            /* Set the batch count. */
            batchCount = 1;

            if (Hardware->features[gcvFEATURE_PE_DISABLE_COLOR_PIPE])
            {
                /* Setup load flags. */
                colorBatchLoad = gcvFALSE;
                colorSplitLoad = gcvFALSE;
                colorPipeEnable = gcvFALSE;
            }
            else
            {
                gcsSURF_INFO_PTR tmpRT = &Hardware->tmpRT.info;

                /* Setup load flags. */
                colorBatchLoad = gcvFALSE;
                colorSplitLoad = gcvTRUE;

                physicalBaseAddr0 = physicalBaseAddr1 = 0;

                if (!Hardware->PEStates->depthStates.realOnly)
                {
                    gcmGETHARDWAREADDRESS(tmpRT->node, physicalBaseAddr0);
                    physicalBaseAddr1 = physicalBaseAddr0;
                }

                stride = 0;
#if gcdDUMP
                surfaceSize = (gctUINT32)tmpRT->node.size;
#endif

                /* When RT is NULL, we need set colorMask zero to not touch surface.
                */
                colorWrite = 0;
                superTiled = gcvFALSE;
                format = 0x06;

                if (tmpRT->tiling & gcvTILING_SPLIT_BUFFER)
                {
                    colorAddressMode = 0x0;
                }
                /* Single buffer mode. */
                else if (Hardware->features[gcvFEATURE_SINGLE_BUFFER])
                {

                    if (tmpRT->formatInfo.bitsPerPixel <= 16 || Hardware->PEStates->singleBuffer8x4)
                    {
                        colorAddressMode =
                            0x3;
                    }
                    else
                    {
                        colorAddressMode =
                            0x2;
                    }
                }
                /* tile */
                else
                {
                    colorAddressMode = 0x0;
                }
            }
        }
        else
        {
            gctUINT32 surfaceBase;

            gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

            if (Hardware->config->gpuCoreCount > 1)
            {
                gcmONERROR(
                    _SetMultiGPURenderingMode(Hardware, surface, Hardware->gpuRenderingMode, Memory));
            }

            /* Convert format to pixel engine format. */
            format               =
            pColorTarget->format = surface->formatInfo.renderFormat;

            sRGB = surface->formatInfo.sRGB;

            physicalBaseAddr0 = surfaceBase /* surface base */
                              + pColorTarget->sliceIndex * surface->sliceSize  /* slice offset inside one surface or texture mipmap */
                              + pColorTarget->layerIndex * surface->layerSize; /* layer offset inside one slice */

            physicalBaseAddr1 = physicalBaseAddr0 + surface->bottomBufferOffset;

            colorWrite =
            pColorTarget->hwColorWrite = _GetColorMask(pColorTarget->colorWrite, format, pColorTarget->layerIndex);

            stride = surface->stride;
            surface->dither3D = Hardware->PEStates->ditherEnable;
#if gcdDUMP
            surfaceSize = (gctUINT32)surface->node.size;
#endif
            if (surface->tiling == gcvLINEAR)
            {
                physicalBaseAddr1 = physicalBaseAddr0;
            }

            if (format == gcvINVALID_RENDER_FORMAT)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            /* Set the supertiled state. */
            superTiled               =
            pColorTarget->superTiled = surface->superTiled;

            /* Determine the cache mode (equation is taken from RTL). */
            if (Hardware->features[gcvFEATURE_FAST_MSAA] ||
                Hardware->features[gcvFEATURE_SMALL_MSAA])
            {
                cacheMode = surface->isMsaa ? 1 : 0;
            }
            else
            {
                cacheMode = surface->isMsaa && surface->bitsPerPixel != 16;
            }

            /* Determine whether the flush is needed. */
            if (cacheMode != Hardware->PEStates->colorStates.cacheMode)
            {
                /* Flush the cache. */
                flushNeeded = !Hardware->flushedColor;

                /* Update the cache mode. */
                Hardware->PEStates->colorStates.cacheMode = cacheMode;
            }

            if (Hardware->config->pixelPipes > 1
             || Hardware->config->renderTargets > 1)
            {
                /* Set the batch count. */
                batchCount = 1;

                /* Setup load flags. */
                colorBatchLoad = gcvFALSE;
                colorSplitLoad = gcvTRUE;
            }
            else
            {
                /* Set the batch count. */
                batchCount = 3;

                /* Setup load flags. */
                colorBatchLoad = gcvTRUE;
                colorSplitLoad = gcvFALSE;
            }

            if (surface->tiling == gcvLINEAR)
            {
                /* Linear render target. */
                gcmASSERT(Hardware->features[gcvFEATURE_LINEAR_RENDER_TARGET]);

                colorAddressMode = 0x1;
            }
            else if (surface->tiling & gcvTILING_SPLIT_BUFFER)
            {
                colorAddressMode = 0x0;
            }
            /* Single buffer mode. */
            else if (Hardware->features[gcvFEATURE_SINGLE_BUFFER])
            {
                colorAddressMode = Hardware->PEStates->singleBuffer8x4 ?
                                   0x3 :
                                   0x2;
            }
            /* tile */
            else
            {
                colorAddressMode = 0x0;
            }
        }
    }
    else
    {
        /* Set the batch count. */
        batchCount = 1;

        /* Setup load flags. */
        colorBatchLoad = gcvFALSE;
        colorSplitLoad = gcvFALSE;
    }

    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX])
    {
        gctBOOL enableSinglePipe = (Hardware->PEStates->colorOutCount > 1) && (Hardware->PEStates->hasOne8BitRT);

        if (Hardware->PEStates->singlePEpipe != enableSinglePipe)
        {
            Hardware->PEStates->singlePEpipe = enableSinglePipe;
            flushNeeded = !Hardware->flushedColor;
            flushDepthNeeded = !Hardware->flushedDepth;
        }
    }

    if (flushNeeded)
    {
        /* Mark color cache as flushed. */
        Hardware->flushedColor = gcvTRUE;
    }

    if (flushDepthNeeded)
    {
        /* Mark depth cache as flushed. */
        Hardware->flushedDepth = gcvTRUE;
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    /* Flush must be issued before changing DESTINATION_READ. */
    if (flushNeeded)
    {
        /* Flush the cache. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (flushDepthNeeded)
    {
        /* Flush the cache. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    /* Program single/dual pipe state first */
    if ((Hardware->features[gcvFEATURE_HALTI5] &&
        !Hardware->features[gcvFEATURE_MRT_8BIT_DUAL_PIPE_FIX]) ||
        Hardware->features[gcvFEATURE_128BTILE] ||
        Hardware->features[gcvFEATURE_EDGEAA])
    {
        gcsSURF_INFO_PTR depth = Hardware->PEStates->depthStates.surface;

        surface = pColorTarget->surface;

        if (surface)
        {
            colorCacheMode = Hardware->PEStates->colorStates.cacheMode128;
            edgeAA = Hardware->PEStates->colorStates.edgeAA;
            tiling = surface->tiling;
        }
        else if (!Hardware->features[gcvFEATURE_PE_DISABLE_COLOR_PIPE])
        {
            colorCacheMode = Hardware->tmpRT.info.cacheMode;
            edgeAA = Hardware->tmpRT.info.edgeAA;
            tiling = Hardware->tmpRT.info.tiling;
        }

        if (depth)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x052F, ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->singlePEpipe ?
 0x1 : 0x0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (colorCacheMode == gcvCACHE_128 ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (depth->cacheMode == gcvCACHE_128 ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (edgeAA ? 0x1 : 0x0) & ((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (depth->edgeAA ? 0x1 : 0x0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x052F, ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->singlePEpipe ?
 0x1 : 0x0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (colorCacheMode == gcvCACHE_128 ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (edgeAA ? 0x1 : 0x0) & ((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
    }

    if (Hardware->features[gcvFEATURE_128BTILE])
    {
        if (tiling == gcvSUPERTILED)
        {
            tileMode = 0x1;
        }
        else if (tiling == gcvYMAJOR_SUPERTILED)
        {
            tileMode = 0x2;
        }
    }

    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)batchCount  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (batchCount ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x050B) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        /* Program AQPixelConfig.Format field. */
        if (Hardware->features[gcvFEATURE_HALTI0] ||
            ((Hardware->config->chipModel == gcv1000) && (Hardware->config->chipRevision >= 0x5039)))
        {
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x050B,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 29:24) - (0 ?
 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (superTiled) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->colorStates.destinationRead) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (colorWrite) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (!colorPipeEnable) & ((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:13) - (0 ? 14:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ?
 14:13))) | (((gctUINT32) ((gctUINT32) (tileMode) & ((gctUINT32) ((((1 ?
 14:13) - (0 ? 14:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ?
 14:13)))
                );
        }
        else
        {
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x050B,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (superTiled) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (Hardware->PEStates->colorStates.destinationRead) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (colorWrite) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:13) - (0 ? 14:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ?
 14:13))) | (((gctUINT32) ((gctUINT32) (tileMode) & ((gctUINT32) ((((1 ?
 14:13) - (0 ? 14:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ?
 14:13)))
                );
        }

        if (colorBatchLoad)
        {
            /* Program AQPixelAddress register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x050C,
                physicalBaseAddr0
                );

            /* Program AQPixelStride register. */
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x050D,
                stride
                );

            gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", physicalBaseAddr0, surfaceSize);
        }

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    if (Hardware->features[gcvFEATURE_PE_DITHER_FIX])
    {
        if (Hardware->PEDirty->peDitherDirty)
        {
            gctUINT32 *pDitherTable;

            gcsSURF_INFO_PTR rtSurf = pColorTarget->surface;
            gctBOOL fakeFmt = rtSurf && rtSurf->formatInfo.fakedFormat;
            gctBOOL tableIdx = Hardware->PEStates->ditherEnable ? 1 : 0;

            if (fakeFmt || (Hardware->PEStates->colorStates.anyPartialColorWrite &&
                            !Hardware->features[gcvFEATURE_PE_DITHER_COLORMASK_FIX]))
            {
                tableIdx = 0;
            }

            pDitherTable = &Hardware->PEStates->ditherTable[tableIdx][0];

            /* Append RESOLVE_DITHER commands. */
            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052A) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x052A,
                    pDitherTable[0]
                    );

                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x052B,
                    pDitherTable[1]
                    );

                gcmSETFILLER_NEW(
                    reserve, memory
                    );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );

            /* Reset dirty flags. */
            Hardware->PEDirty->peDitherDirty = gcvFALSE;
        }
    }
    else
    {
        /* If the draw required dither, while PE cannot support, defer until resolve time.
        ** If the draw didn't require, do not reset the flag, unless we are sure it's the
        ** draw can overwrite all of the surface.
        */
        gcsSURF_INFO_PTR rtSurf = pColorTarget->surface;
        gctBOOL fakeFmt = rtSurf && rtSurf->formatInfo.fakedFormat;

        /* Disable dither for fake formats */
        if (Hardware->PEStates->ditherEnable && !fakeFmt)
        {
            gctUINT32 i;

            if (rtSurf)
            {
                rtSurf->deferDither3D = gcvTRUE;
            }

            for (i = 1; i < Hardware->PEStates->colorOutCount; ++i)
            {
                gcmASSERT(Hardware->PEStates->colorStates.target[i].surface);
                Hardware->PEStates->colorStates.target[i].surface->deferDither3D = gcvTRUE;
            }
        }
    }

    if (colorSplitLoad)
    {
       if(Hardware->config->pixelPipes > 1)
       {
            gcmASSERT(Hardware->config->pixelPipes == 2);

            /* even for single buffer PE, we still need to program two same address */
            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0518) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0518,
                    physicalBaseAddr0
                    );

                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0518 + 1,
                    physicalBaseAddr1
                    );

                gcmSETFILLER_NEW(
                    reserve, memory
                    );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );
       }
       else
       {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0518) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0518, physicalBaseAddr0 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            /* Program AQPixelAddress register. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x050C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x050C, physicalBaseAddr0 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

       }

       gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", physicalBaseAddr0, surfaceSize);

       {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x050D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x050D, stride );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


       if (Hardware->features[gcvFEATURE_SRGB_RT_SUPPORT])
       {
           {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 30:30) - (0 ?
 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (sRGB) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:30) - (0 ?
 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

       }
    }

    if (colorAddressMode != -1)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (colorAddressMode) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (Hardware->features[gcvFEATURE_GEOMETRY_SHADER])
    {
        gctUINT32 sliceSize = pColorTarget->surface ? pColorTarget->surface->sliceSize : 0;
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0426) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0426, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) ((Hardware->SHStates->maxLayers - 1)) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E28) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E28, (Hardware->SHStates->rtLayered ?
 sliceSize : 0) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    if (Hardware->PEDirty->colorTargetDirty && (surface != gcvNULL))
    {
        /* Setup multi-sampling. */
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
        Hardware->MsaaDirty->msaaModeDirty   = gcvTRUE;
    }

    if (Hardware->PEStates->colorOutCount > 1)
    {
        _FlushMultiTarget(Hardware, Memory);
    }

    /* Reset dirty flags. */
    Hardware->PEDirty->colorConfigDirty = gcvFALSE;
    Hardware->PEDirty->colorTargetDirty = gcvFALSE;

    if (Hardware->features[gcvFEATURE_HALTI2])
    {
        Hardware->SHDirty->shaderDirty |= gcvPROGRAM_STAGE_FRAGMENT_BIT;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushDepth(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    static const gctINT32 xlateCompare[] =
    {
        /* gcvCOMPARE_INVALID */
        -1,
        /* gcvCOMPARE_NEVER */
        0x0,
        /* gcvCOMPARE_NOT_EQUAL */
        0x5,
        /* gcvCOMPARE_LESS */
        0x1,
        /* gcvCOMPARE_LESS_OR_EQUAL */
        0x3,
        /* gcvCOMPARE_EQUAL */
        0x2,
        /* gcvCOMPARE_GREATER */
        0x4,
        /* gcvCOMPARE_GREATER_OR_EQUAL */
        0x6,
        /* gcvCOMPARE_ALWAYS */
        0x7,
    };

    static const gctUINT32 xlateMode[] =
    {
        /* gcvDEPTH_NONE */
        0x0,
        /* gcvDEPTH_Z */
        0x1,
        /* gcvDEPTH_W */
        0x2,
    };

    gceSTATUS status;
    gcsDEPTH_INFO *depthInfo;
    gcsSURF_INFO_PTR surface;

    gctUINT depthBatchAddress;
    gctUINT depthBatchCount;
    gctBOOL depthBatchLoad;
    gctBOOL depthSplitLoad;
    gctBOOL needFiller;

    gctUINT hzBatchCount = 0;

    gctBOOL flushNeeded = gcvFALSE;
    gctBOOL stallNeeded = gcvFALSE;

    gctINT colorSingleBufferMode = -1;
    gctUINT stencilFormat = 0x0;

    gctUINT32 surfaceBase = 0, surfaceBottom = 0;

    gctBOOL raDepth = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    depthInfo = &Hardware->PEStates->depthStates;
    surface   = depthInfo->surface;

    /* If depthTarget dirtied, depthConfig must dirtied. */
    gcmASSERT(!Hardware->PEDirty->depthTargetDirty || Hardware->PEDirty->depthConfigDirty);

    /* Test if configuration is dirty. */
    if (Hardware->PEDirty->depthConfigDirty)
    {
        gceDEPTH_MODE depthMode = surface ? depthInfo->mode : gcvDEPTH_NONE;
        gceCOMPARE compare = surface ? depthInfo->compare : gcvCOMPARE_ALWAYS;

        gctBOOL ezEnabled = depthInfo->early &&
                             !Hardware->PEStates->disableAllEarlyDepth &&
                             !Hardware->PEStates->disableAllEarlyDepthFromStatistics;

        gctBOOL peDepth = depthInfo->write ||
                           !ezEnabled ||
                           (Hardware->PEStates->stencilStates.mode != gcvSTENCIL_NONE) ||
                           /* If RA doesn't modify Z, then PE needs to do Z. */
                           Hardware->PEStates->disableRAModifyZ;

        /* HZ in fact depends on both depthTargetDirty and depthConfigDirty */
        gctBOOL hzEnabled = surface &&
                            !surface->hzDisabled &&
                            (depthInfo->mode != gcvDEPTH_NONE) &&
                            !Hardware->PEStates->disableAllEarlyDepth &&
                            !Hardware->PEStates->disableAllEarlyDepthFromStatistics;

        if(Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
        {
            raDepth = !Hardware->MsaaStates->disableRAWriteDepth && !Hardware->PEStates->alphaStates.test &&
                      !Hardware->MsaaStates->MsaaFragmentOp && gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_RA_DEPTH_WRITE);

            if (!Hardware->features[gcvFEATURE_RA_DEPTH_WRITE_MSAA1X_FIX] &&
                (Hardware->MsaaStates->sampleInfo.product > 1) &&
                (raDepth) &&
                (Hardware->MsaaStates->sampleMask == 0)
               )
            {
                raDepth = gcvFALSE;
                gcmPRINT("GAL: hit the workaround");
            }

            /* At any time there is only one client to write depth (RA or PE). */
            peDepth = !raDepth;

            /* If switch depth write from RA and PE depth cache must flush first. */
            if(Hardware->preRADepth != raDepth)
            {
                if(Hardware->preRADepth || Hardware->previousPEDepth)
                {
                    stallNeeded = gcvTRUE;
                }
                Hardware->preRADepth = raDepth;
                Hardware->previousPEDepth = peDepth;
            }
        }

        if (Hardware->previousPEDepth != peDepth)
        {
            /* Need flush and stall when switching from PE Depth ON to PE Depth Off? */
            if (Hardware->previousPEDepth)
            {
                stallNeeded = gcvTRUE;
            }

            Hardware->previousPEDepth = peDepth;
        }

        /* If earlyZ or depth compare changed, and earlyZ is enabled, need to flush Zcache and stall */
        /* No need to initialize previous values bc no PE depth write before first draw. */
        if (Hardware->prevEarlyZ != ezEnabled ||
            Hardware->prevDepthCompare != depthInfo->compare)
        {
            if (ezEnabled || hzEnabled)
            {
                flushNeeded = !Hardware->flushedDepth;
                stallNeeded = gcvTRUE;
            }

            Hardware->prevEarlyZ = ezEnabled;
            Hardware->prevDepthCompare = depthInfo->compare;
        }

        depthInfo->regDepthConfig &= ~(((((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
                                       ((((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))) |
                                       ((((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) |
                                       ((((gctUINT32) (((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12))) |
                                       ((((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8))) |
                                       ((((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
                                       );

        depthInfo->regDepthConfig |= (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (xlateMode[depthMode]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (depthInfo->realOnly) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (ezEnabled) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (depthInfo->write) & ((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (xlateCompare[compare]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) |
                                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (!peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)))
                                      );

        depthInfo->regRAControlHZ = ((((gctUINT32) (depthInfo->regRAControlHZ)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (xlateCompare[compare] ) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12)));

        /* Depth config change re-program Depth mode, stencil mode depend it. */
        Hardware->PEDirty->stencilDirty = gcvTRUE;

        if (Hardware->PEDirty->depthTargetDirty)
        {
            /* If depth write is disabled and compare is always, we can turn off HZ as well. */
            if (!depthInfo->write && depthInfo->compare == gcvCOMPARE_ALWAYS)
            {
                /* If HZ is always enabled while now need to disable temporarily, need to flush HZ first
                ** to make sure HZ buffer is coherent with Z buffer for later use.
                */
                if (Hardware->prevHZ && surface && !surface->hzDisabled)
                {
                    flushNeeded = !Hardware->flushedDepth;
                }
                hzEnabled = gcvFALSE;
            }

            Hardware->prevHZ = hzEnabled;

            if (hzEnabled)
            {
                hzBatchCount = 2;

                if (surface->bitsPerPixel == 16)
                {
                    depthInfo->regRAControlHZ = ((((gctUINT32) (depthInfo->regRAControlHZ)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0  & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

                    depthInfo->regControlHZ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                                            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
                }
                else
                {
                    depthInfo->regRAControlHZ = ((((gctUINT32) (depthInfo->regRAControlHZ)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1  & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

                    depthInfo->regControlHZ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) (0x8 & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                                            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) (0x8 & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
                }

                /* RA HZ depends on MC HZBase to access tilestatus, must wait PE->MC received the addr */
                if ((((((gctUINT32) (Hardware->MCStates->memoryConfig)) >> (0 ? 12:12)) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) ))
                {
                    stallNeeded = gcvTRUE;
                }
            }
            else
            {
                hzBatchCount = 1;

                depthInfo->regControlHZ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));

                depthInfo->regRAControlHZ = ((((gctUINT32) (depthInfo->regRAControlHZ)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x0  & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
            }

            if ((Hardware->PEStates->colorStates.target[0].surface == gcvNULL) && (Hardware->config->gpuCoreCount > 1))
            {
                /* Set split mode based on depth surface, if rendering in depth only. */
                gcmONERROR(
                    _SetMultiGPURenderingMode(Hardware, surface, Hardware->gpuRenderingMode, Memory));
            }

            /* Program RA Z control states. */
            /* Early early z has issue in calculating polygon offset on PXA988. we just disable early early z on PXA988 */
            if ((Hardware->config->chipModel == gcv1000 && Hardware->config->chipRevision < 0x5035) ||
                (Hardware->config->chipModel == gcv400 && Hardware->config->chipRevision >= 0x4645))
            {
                depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
                                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)));
            }
            else  if (surface && (surface->format == gcvSURF_S8 || surface->format == gcvSURF_X24S8))
            {
                depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)));
            }
            else
            {
                gctUINT32 eezEnabled = !Hardware->PEStates->disableAllEarlyDepth;

                if (Hardware->patchID == gcvPATCH_DEQP &&
                    !Hardware->features[gcvFEATURE_DEPTH_MATH_FIX] &&
                    (Hardware->features[gcvFEATURE_NEW_HZ] || Hardware->features[gcvFEATURE_FAST_MSAA])
                   )
                {
                    eezEnabled = 0;
                }

                depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (eezEnabled) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
                                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)));
            }

            if (Hardware->features[gcvFEATURE_NEW_RA])
            {
                gctBOOL msaa = Hardware->MsaaStates->sampleInfo.product > 1;
                gctUINT32 shaderZ = !Hardware->PEStates->disableRAPassZ ||    /* Pass Z value or not */
                                    ((msaa & peDepth) << 1);        /* Pass sub samples values or not */

                depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (shaderZ) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)));

                if (!ezEnabled)
                {
                    depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
                }
                else if (Hardware->PEStates->disableRAModifyZ)
                {
                    depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
                }
                else
                {
                    depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
                }
            }

            if (Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
            {
                gctBOOL msaa = surface ? surface->isMsaa : gcvFALSE;
                gctBOOL useSubSampleZInPS = gcvFALSE;
                gctBOOL sampleMaskInPos = gcvFALSE;

                if (raDepth)
                {
                    depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
                }
                else
                {
                    depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
                }

                /* SHADER_SUB_SAMPLES control whether PE need sub sample depth or not. */
                if (peDepth && msaa)
                {
                    depthInfo->regRAControl =
                        ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));

                    useSubSampleZInPS = gcvTRUE;
                }
                else
                {
                    depthInfo->regRAControl =
                        ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));
                }

                /* If compiler use sampleMask, compiler decide sample mask location firstly.*/
                if (Hardware->MsaaStates->sampleMaskLoc != -1)
                {
                    gcmASSERT(!Hardware->PEStates->depthStates.realOnly);

                    if (Hardware->MsaaStates->sampleMaskLoc == 0x0)
                    {
                        depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->psReadZ || peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)));
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)));

                        useSubSampleZInPS = gcvTRUE;
                    }
                    else if (Hardware->MsaaStates->sampleMaskLoc == 0x1)
                    {
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)));

                        sampleMaskInPos = gcvTRUE;

                    }
                    /* 0x2 */
                    else
                    {
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
                        depthInfo->regRAControl =
                            ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->psReadZ || peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)));

                        sampleMaskInPos = gcvTRUE;
                    }
                }
                /* Else, driver decide sample mask location. */
                else
                {
                    depthInfo->regRAControl =
                        ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->psReadZ || peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)));
                    depthInfo->regRAControl =
                        ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)));


                    /* Always put in SubZ if compiler doesn't decide it. As in this case, compiler wont
                    ** reserve r1 for highp under dual16 mode.
                    */
                    depthInfo->regRAControl = ((((gctUINT32) (depthInfo->regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));

                    useSubSampleZInPS = gcvTRUE;

                }

                /* Switch affect PS input and temp register programming */
                if (Hardware->MsaaStates->useSubSampleZInPS != useSubSampleZInPS)
                {
                    Hardware->MsaaStates->useSubSampleZInPS = useSubSampleZInPS;
                    Hardware->SHDirty->shaderDirty = gcvTRUE;
                }

                if (Hardware->MsaaStates->sampleMaskInPos != sampleMaskInPos)
                {
                    Hardware->MsaaStates->sampleMaskInPos = sampleMaskInPos;
                    Hardware->SHDirty->shaderDirty = gcvTRUE;
                }
            }

            if (surface)
            {
                gctUINT32 cacheMode;

                if (surface->bitsPerPixel == 16)
                {
                    /* 16-bit depth buffer. */
                    depthInfo->regDepthConfig = ((((gctUINT32) (depthInfo->regDepthConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0  & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

                    Hardware->PEStates->maxDepth = 0xFFFF;
                    Hardware->PEStates->stencilEnabled = gcvFALSE;
                }
                else
                {
                    /* 24-bit depth buffer. */
                    depthInfo->regDepthConfig = ((((gctUINT32) (depthInfo->regDepthConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1  & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

                    Hardware->PEStates->maxDepth = 0xFFFFFF;
                    Hardware->PEStates->stencilEnabled = ((surface->format == gcvSURF_D24S8) || (surface->format == gcvSURF_S8)
                                                || (surface->format == gcvSURF_X24S8));

                }
                /* Determine the cache mode (equation is taken from RTL). */
                if (Hardware->features[gcvFEATURE_FAST_MSAA] || Hardware->features[gcvFEATURE_SMALL_MSAA])
                {
                    cacheMode = surface->isMsaa ? 1 : 0;
                }
                else
                {
                    cacheMode = (surface->isMsaa) && (surface->bitsPerPixel != 16);
                }

                /* We need to flush the cache if the cache mode is changing. */
                if (cacheMode != depthInfo->cacheMode)
                {
                    /* Flush the cache. */
                    flushNeeded = !Hardware->flushedDepth;

                    /* Update the cache mode. */
                    depthInfo->cacheMode = cacheMode;
                }

                if (Hardware->features[gcvFEATURE_S8_ONLY_RENDERING] &&
                    surface->format == gcvSURF_S8)
                {
                    stencilFormat = 0x1;
                }
            }
            else
            {
                Hardware->PEStates->stencilEnabled = gcvFALSE;
            }
        }
    }

    if (Hardware->PEDirty->depthTargetDirty)
    {
        gcmASSERT(Hardware->PEDirty->depthConfigDirty);

        /* Configuration is always set when the surface is changed. */
        depthBatchAddress = 0x0500;

        if (surface == gcvNULL)
        {
            /* Define depth state load parameters. */
            if (Hardware->PEDirty->depthRangeDirty || Hardware->PEDirty->depthNormalizationDirty)
            {
                depthBatchCount = Hardware->PEDirty->depthRangeDirty ? 4 : 1;
                depthBatchLoad  = gcvFALSE;
                depthSplitLoad  = gcvFALSE;
                needFiller      = Hardware->PEDirty->depthRangeDirty ? gcvTRUE : gcvFALSE;
            }
            else
            {
                depthBatchCount = 1;
                depthBatchLoad  = gcvFALSE;
                depthSplitLoad  = gcvFALSE;
                needFiller      = gcvFALSE;
            }

            /* No stencil. */
            Hardware->PEStates->stencilEnabled = gcvFALSE;
        }
        else
        {
            /* Define depth state load parameters. */
            if (Hardware->PEDirty->depthRangeDirty || Hardware->PEDirty->depthNormalizationDirty)
            {
                if (Hardware->config->pixelPipes > 1
                 || Hardware->config->renderTargets > 1)
                {
                    depthBatchCount = Hardware->PEDirty->depthRangeDirty ? 4 : 1;
                    depthBatchLoad  = gcvFALSE;
                    depthSplitLoad  = gcvTRUE;
                    needFiller      = Hardware->PEDirty->depthRangeDirty ? gcvTRUE : gcvFALSE;
                }
                else
                {
                    depthBatchCount = Hardware->PEDirty->depthRangeDirty ? 6 : 1;
                    depthBatchLoad  = gcvTRUE;
                    depthSplitLoad  = gcvFALSE;
                    needFiller      = Hardware->PEDirty->depthRangeDirty ? gcvTRUE : gcvFALSE;
                }
            }
            else
            {
                depthBatchCount = 1;
                depthBatchLoad  = gcvFALSE;
                depthSplitLoad  = gcvTRUE;
                needFiller      = gcvFALSE;
            }

            /* Single buffer mode. */
            if (depthSplitLoad && Hardware->features[gcvFEATURE_SINGLE_BUFFER])
            {
                /* Program 8x4 if any of the surfaces is 16bit,
                   else program 4x4. */
                if (surface->tiling & gcvTILING_SPLIT_BUFFER)
                {
                    gcmASSERT(Hardware->PEStates->colorStates.target[0].surface == gcvNULL ||
                              Hardware->PEStates->colorStates.target[0].surface->tiling & gcvTILING_SPLIT_BUFFER);

                    colorSingleBufferMode = 0x0;
                }
                else
                {
                    colorSingleBufferMode = Hardware->PEStates->singleBuffer8x4 ?
                                            0x3 :
                                            0x2;
                }

            }

            if (surface)
            {
                gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", gcsSURF_NODE_GetHWAddress(&surface->node), surface->node.size);
            }
        }
    }
    /* Depth surface is not dirty. */
    else
    {
        if (Hardware->PEDirty->depthConfigDirty)
        {
            /* The batch starts with the configuration. */
            depthBatchAddress = 0x0500;

            /* Define depth state load parameters. */
            if (Hardware->PEDirty->depthRangeDirty || Hardware->PEDirty->depthNormalizationDirty)
            {
                depthBatchCount = Hardware->PEDirty->depthRangeDirty ? 4 : 1;
                depthBatchLoad  = gcvFALSE;
                depthSplitLoad  = gcvFALSE;
                needFiller      = Hardware->PEDirty->depthRangeDirty ? gcvTRUE : gcvFALSE;

            }
            else
            {
                depthBatchCount = 1;
                depthBatchLoad  = gcvFALSE;
                depthSplitLoad  = gcvFALSE;
                needFiller      = gcvFALSE;
            }
        }
        else
        {
            /* Define depth state load parameters. */
            if (Hardware->PEDirty->depthRangeDirty || Hardware->PEDirty->depthNormalizationDirty)
            {
                depthBatchAddress = Hardware->PEDirty->depthRangeDirty ? 0x0501 : 0x0503;
                depthBatchCount = Hardware->PEDirty->depthRangeDirty ? 3 : 1;
                depthBatchLoad  = gcvFALSE;
                depthSplitLoad  = gcvFALSE;
                needFiller      = gcvFALSE;
            }
            else
            {
                /* Nothing to be done. */
                gcmFOOTER_NO();
                return gcvSTATUS_OK;
            }
        }
    }

    /* Add the flush state. */
    if (flushNeeded || stallNeeded)
    {
        /* Mark depth cache as flushed. */
        Hardware->flushedDepth = gcvTRUE;
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    if (flushNeeded || stallNeeded)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)depthBatchCount  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (depthBatchCount ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (depthBatchAddress) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


    if (Hardware->PEDirty->depthConfigDirty)
    {
        gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0500, depthInfo->regDepthConfig);
    }

    if (Hardware->PEDirty->depthRangeDirty || Hardware->PEDirty->depthNormalizationDirty)
    {
        gcuFLOAT_UINT32 depthNear, depthFar;
        gcuFLOAT_UINT32 depthNormalize;

        if (depthInfo->mode == gcvDEPTH_W)
        {
            depthNear.f = depthInfo->near;
            depthFar.f  = depthInfo->far;

            depthNormalize.f = (depthFar.u != depthNear.u)
                ? gcoMATH_Divide(
                    gcoMATH_UInt2Float(Hardware->PEStates->maxDepth),
                    gcoMATH_Add(depthFar.f, -depthNear.f)
                    )
                : gcoMATH_UInt2Float(Hardware->PEStates->maxDepth);
        }
        else
        {
            depthNear.f = Hardware->PEStates->depthNear.f;
            depthFar.f  = Hardware->PEStates->depthFar.f;

            depthNormalize.f = gcoMATH_UInt2Float(Hardware->PEStates->maxDepth);
        }

        if(Hardware->PEDirty->depthRangeDirty)
        {
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0501,
                depthNear.u
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0502,
                depthFar.u
                );
        }

        if((!Hardware->PEDirty->depthRangeDirty) && Hardware->PEDirty->depthNormalizationDirty)
        {
            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0503) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0503, depthNormalize.u );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0503,
                depthNormalize.u
                );
        }
    }

    if (depthBatchLoad && (!Hardware->PEDirty->depthRangeDirty) && Hardware->PEDirty->depthNormalizationDirty)
    {
        gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0504) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0504, surfaceBase + depthInfo->sliceIndex * surface->sliceSize );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0505) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0505, surface->stride );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }
    else if(depthBatchLoad)
    {
        gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0504,
            surfaceBase + depthInfo->sliceIndex * surface->sliceSize
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0505,
            surface->stride
            );
    }

    if (needFiller)
    {
        gcmSETFILLER_NEW(
            reserve, memory
            );
    }

    if(Hardware->PEDirty->depthRangeDirty || (!Hardware->PEDirty->depthNormalizationDirty))
    {
        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }

    if (depthSplitLoad)
    {
        if (Hardware->config->pixelPipes > 1
         || Hardware->config->renderTargets > 1)
        {
            if (Hardware->config->pixelPipes > 1)
            {
                gcmASSERT(Hardware->config->pixelPipes == 2);

                gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

                gcmGETHARDWAREADDRESSBOTTOM(surface->node, surfaceBottom);

                /* Program depth addresses. */
                {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0520) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                    gcmSETSTATEDATA_NEW(
                        stateDelta, reserve, memory, gcvFALSE, 0x0520,
                        surfaceBase + depthInfo->sliceIndex * surface->sliceSize
                        );

                    /* TODO: for singleBuffer+MultiPipe case, no need to program the 2nd address */
                    gcmSETSTATEDATA_NEW(
                        stateDelta, reserve, memory, gcvFALSE, 0x0520 + 1,
                        surfaceBottom + depthInfo->sliceIndex * surface->sliceSize
                        );

                    gcmSETFILLER_NEW(
                        reserve, memory
                        );

                gcmENDSTATEBATCH_NEW(
                    reserve, memory
                    );

                if (colorSingleBufferMode != -1)
                {
                    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (colorSingleBufferMode) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

                }
            }
            else /* renderTargets > 1. */
            {
                gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

                /* Program depth addresses. */
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0520) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0520, surfaceBase );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0504) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0504, surfaceBase + depthInfo->sliceIndex * surface->sliceSize );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0505) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0505, surface->stride );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (stencilFormat) & ((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (stencilFormat) & ((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0504) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                gcmGETHARDWAREADDRESS(surface->node, surfaceBase);

                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0504,
                    surfaceBase + depthInfo->sliceIndex * surface->sliceSize
                    );

                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0505,
                    surface->stride
                    );

                gcmSETFILLER_NEW(
                    reserve, memory
                    );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );
        }
    }

    if (Hardware->PEDirty->depthConfigDirty)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0382) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0382, depthInfo->regRAControl );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (hzBatchCount != 0)
    {
        gctUINT32 hzAddress = 0;

        if (hzBatchCount == 2)
        {
            gcmGETHARDWAREADDRESS(surface->hzNode, hzAddress);
        }

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)hzBatchCount  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (hzBatchCount ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0515) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0515,
                depthInfo->regControlHZ
                );

            if (hzBatchCount == 2)
            {
                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0516,
                    hzAddress
                    );

                gcmSETFILLER_NEW(
                    reserve, memory
                    );
            }

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0388) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0388, depthInfo->regRAControlHZ );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        if (hzBatchCount == 2)
        {
            /* NEW_RA register. */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0389) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0389, hzAddress );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        if (surface)
        {
            gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", hzAddress, surface->hzNode.size);
        }
    }

    if (Hardware->PEDirty->depthRangeDirty)
    {
        gcuFLOAT_UINT32 zOffset, zScale;

        if (Hardware->features[gcvFEATURE_ZSCALE_FIX] &&
            gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_ZCONVERT_BYPASS))
        {
            zOffset.f = gcoMATH_Add(
                depthInfo->far, depthInfo->near
                ) / 2.0f;

            zScale.f  = gcoMATH_Add(
                depthInfo->far, -depthInfo->near
                ) / 2.0f;
         }
         else
         {
            zOffset.f = depthInfo->near;
            zScale.f  = gcoMATH_Add(
                depthInfo->far, -depthInfo->near
                );
         }

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0282) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0282, zScale.u );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0285) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0285, zOffset.u );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (Hardware->features[gcvFEATURE_GEOMETRY_SHADER])
    {
        gctUINT32 sliceSize = depthInfo->surface ? depthInfo->surface->sliceSize : 0;

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E23) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E23, (Hardware->SHStates->rtLayered ?
 sliceSize : 0) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    if (stallNeeded)
    {
        /* Send semaphore-stall from RA to PE. */
        gcmONERROR(gcoHARDWARE_Semaphore(Hardware,
                                         gcvWHERE_RASTER,
                                         gcvWHERE_PIXEL,
                                         gcvHOW_SEMAPHORE,
                                         Memory));
    }

    if ((Hardware->PEDirty->depthTargetDirty) &&
        (Hardware->PEStates->colorStates.target[0].surface == gcvNULL) &&
        (depthInfo->surface != gcvNULL))
    {
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
        Hardware->MsaaDirty->msaaModeDirty   = gcvTRUE;
    }

    /* Reset dirty flags. */
    Hardware->PEDirty->depthConfigDirty        = gcvFALSE;
    Hardware->PEDirty->depthRangeDirty         = gcvFALSE;
    Hardware->PEDirty->depthTargetDirty        = gcvFALSE;
    Hardware->PEDirty->depthNormalizationDirty = gcvFALSE;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushSampling(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status;
    gctUINT samples = 0;
    gctBOOL programTables = gcvFALSE;
    gctUINT32 msaaMode = 0;
    gctUINT msaaEnable;
    gctUINT32_PTR sampleCoords = gcvNULL;
    gcsCENTROIDS_PTR centroids = gcvNULL;
    gctUINT32 jitterIndex = 0;
    gctINT tableIndex = 0;
    gctBOOL yInverted = gcvFALSE;
    gctBOOL sampleShadingEnable = gcvFALSE;
    gctUINT minSampleShadingValue = 0;
    gctBOOL sampleCenter = gcvFALSE;
    gctUINT raControlEx = 0;
    gctBOOL programRACtrlEx = gcvFALSE;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Configuration is always loaded. */
    gcmASSERT(Hardware->MsaaDirty->msaaConfigDirty);

    samples = Hardware->MsaaStates->sampleInfo.product;

    if (Hardware->PEStates->colorStates.target[0].surface != gcvNULL)
    {
        yInverted = (Hardware->PEStates->colorStates.target[0].surface->flags &
                     gcvSURF_FLAG_CONTENT_YINVERTED);
    }
    else if (Hardware->PEStates->depthStates.surface)
    {
        yInverted = (Hardware->PEStates->depthStates.surface->flags &
                     gcvSURF_FLAG_CONTENT_YINVERTED);
    }

    /* Compute centroids. */
    if (Hardware->MsaaDirty->centroidsDirty)
    {
        gcmONERROR(gcoHARDWARE_ComputeCentroids(
            Hardware, 1, &Hardware->MsaaStates->sampleCoords2, &Hardware->MsaaStates->centroids2
            ));

        gcmONERROR(gcoHARDWARE_ComputeCentroids(
            Hardware, 3, Hardware->MsaaStates->sampleCoords4, Hardware->MsaaStates->centroids4
            ));

        Hardware->MsaaDirty->centroidsDirty = gcvFALSE;
    }

    switch (samples)
    {
    case 0:
    case 1:
        msaaMode = 0x0;
        Hardware->MsaaStates->sampleEnable = 0x0;
        break;

    case 2:
        msaaMode = 0x1;
        Hardware->MsaaStates->sampleEnable = 0x3;

        if (Hardware->MsaaDirty->msaaModeDirty)
        {
            programTables = gcvTRUE;
            sampleCoords  = &Hardware->MsaaStates->sampleCoords2;
            centroids     = &Hardware->MsaaStates->centroids2;
            tableIndex    = 0;
        }
        break;

    case 4:
        msaaMode = 0x2;
        Hardware->MsaaStates->sampleEnable = 0xF;

        if (Hardware->MsaaDirty->msaaModeDirty)
        {
            programTables = gcvTRUE;
            sampleCoords  = Hardware->MsaaStates->sampleCoords4;
            centroids     = Hardware->MsaaStates->centroids4;
            jitterIndex   = Hardware->MsaaStates->jitterIndex;
            tableIndex    = yInverted ? 1 : 0;
        }
        break;

    default:
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Determine enable value. */
    msaaEnable = Hardware->MsaaStates->sampleMask & Hardware->MsaaStates->sampleEnable;

    if (Hardware->features[gcvFEATURE_MSAA_SHADING])
    {
        if(msaaEnable && (Hardware->MsaaStates->sampleShading || Hardware->MsaaStates->sampleShadingByPS || Hardware->MsaaStates->isSampleIn))
        {
            sampleShadingEnable = gcvTRUE;
            if(Hardware->MsaaStates->sampleShading)
            {
                minSampleShadingValue = Hardware->MsaaStates->minSampleShadingValue;
            }
            else
            {
                minSampleShadingValue = Hardware->MsaaStates->sampleShadingValue;
            }
        }

        if((minSampleShadingValue == 1 && sampleShadingEnable) || !sampleShadingEnable)
        {
            sampleCenter = gcvTRUE;
        }
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    /* Flush C/Z cache and semaphore-stall if cache mode was changed */
    if (Hardware->MsaaDirty->msaaModeDirty &&
        (Hardware->features[gcvFEATURE_FAST_MSAA] || Hardware->features[gcvFEATURE_SMALL_MSAA]))
    {
        gctUINT cacheMode = msaaEnable ? 1 : 0;

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* We also have to sema stall between FE and PE to make sure the flush
        ** has happened. */
        {    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E02, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));    gcmDUMP(gcvNULL, "#[stall 0x%08X 0x%08X]", ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))), ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));    gcmSKIPSECUREUSER();
};


        /* We need to program fast msaa config if the cache mode is changing. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 25:25) - (0 ?
 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25)))) | (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 26:26) - (0 ?
 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 27:27) - (0 ?
 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) ((samples == 4) ? 1 : 0) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:25) - (0 ?
 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)))) & (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) ((samples == 4) ? 1 : 0) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:27) - (0 ?
 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        Hardware->PEStates->colorStates.cacheMode = cacheMode;
        Hardware->PEStates->depthStates.cacheMode = cacheMode;
    }

    /* Set multi-sample mode. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E06) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (msaaMode) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ?
 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (msaaEnable) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (sampleShadingEnable) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (minSampleShadingValue - 1) & ((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:28) - (0 ?
 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (sampleCenter) & ((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    if (Hardware->features[gcvFEATURE_MSAA_SHADING])
    {
        if(sampleShadingEnable)
        {
            raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->EarlyDepthFromAPP) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5)));
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x038D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x038D, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->EarlyDepthFromAPP) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                         | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (Hardware->MsaaStates->EarlyDepthFromAPP) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5)));
        }

        programRACtrlEx = gcvTRUE;
    }

    if (Hardware->features[gcvFEATURE_GEOMETRY_SHADER])
    {
        raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) ((Hardware->SHStates->rtLayered && Hardware->SHStates->shaderLayered) ?
 1 : 0) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
        programRACtrlEx = gcvTRUE;
    }

    if (programRACtrlEx)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x038D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x038D, raControlEx );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (programTables)
    {
        /* Set multi-sample jitter. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0381) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0381, jitterIndex );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Program sample coordinates. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0384) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0384,
            sampleCoords[tableIndex]
        );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Program the centroid table. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)4  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (4 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0390) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0390,
                centroids[tableIndex].value[0]
            );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0390 + 1,
                centroids[tableIndex].value[1]
            );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0390 + 2,
                centroids[tableIndex].value[2]
            );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0390 + 3,
                centroids[tableIndex].value[3]
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    /* msaaConfigDirty will affect PS input control programming,
    ** we should dirty shader, but shader dirty has been dirtied
    ** when RT switch, which also affect msaaConfigDirty.
    */

    /* Reset dirty flags. */
    Hardware->MsaaDirty->msaaConfigDirty = gcvFALSE;
    Hardware->MsaaDirty->msaaModeDirty   = gcvFALSE;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushPA(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    static const gctUINT32 xlateCulling[] =
    {
        /* gcvCULL_NONE */
        0x0,
        /* gcvCULL_CCW*/
        0x2,
        /* gcvCULL_CW*/
        0x1
    };

    static const gctUINT32 xlateFill[] =
    {
        /* gcvFILL_POINT */
        0x0,
        /* gcvFILL_WIRE_FRAME */
        0x1,
        /* gcvFILL_SOLID */
        0x2
    };

    static const gctUINT32 xlateShade[] =
    {
        /* gcvSHADING_SMOOTH */
        0x1,
        /* gcvSHADING_FLAT_D3D */
        0x0,
        /* gcvSHADING_FLAT_OPENGL */
        0x0
    };

    gceSTATUS status;
    gctUINT batchAddress;
    gctUINT batchCount;
    gctBOOL needFiller;

    gcuFLOAT_UINT32 width;
    gcuFLOAT_UINT32 adjustSub;
    gcuFLOAT_UINT32 adjustAdd;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->PAAndSEDirty->paLineDirty)
    {
        if (Hardware->PAAndSEDirty->paConfigDirty)
        {
            batchAddress = 0x028D;
            batchCount   = 3;
            needFiller   = gcvFALSE;
        }
        else
        {
            batchAddress = 0x028E;
            batchCount   = 2;
            needFiller   = gcvTRUE;
        }
    }
    else
    {
        if (Hardware->PAAndSEDirty->paConfigDirty)
        {
            batchAddress = 0x028D;
            batchCount   = 1;
            needFiller   = gcvFALSE;
        }
        else
        {
            /* Nothing to be done. */
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    if (Hardware->PAAndSEDirty->paLineDirty)
    {
        width.f = Hardware->PAAndSEStates->paStates.aaLineWidth / 2.0f;

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0286) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0286, width.u );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)batchCount  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (batchCount ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (batchAddress) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        if (Hardware->PAAndSEDirty->paConfigDirty)
        {
            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x028D,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (Hardware->PAAndSEStates->paStates.pointSize) & ((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (Hardware->PAAndSEStates->paStates.pointSprite) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (Hardware->PAAndSEStates->paStates.primitiveId) & ((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (xlateCulling[Hardware->PAAndSEStates->paStates.culling]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (xlateFill[Hardware->PAAndSEStates->paStates.fillMode]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (xlateShade[Hardware->PAAndSEStates->paStates.shading]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (Hardware->PAAndSEStates->paStates.aaLine) & ((gctUINT32) ((((1 ?
 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
 22:22)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (Hardware->PAAndSEStates->paStates.aaLineTexSlot) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (Hardware->PAAndSEStates->paStates.wclip) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29)))
                );
            Hardware->PAAndSEDirty->paConfigDirty = gcvFALSE;
        }

        if (Hardware->PAAndSEDirty->paLineDirty)
        {
            adjustSub.f = Hardware->PAAndSEStates->paStates.aaLineWidth / 2.0f;
            adjustAdd.f = -adjustSub.f + Hardware->PAAndSEStates->paStates.aaLineWidth;

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x028E,
                adjustSub.u
                );

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x028F,
                adjustAdd.u
                );

            Hardware->PAAndSEDirty->paLineDirty = gcvFALSE;
        }

        if (needFiller)
        {
            gcmSETFILLER_NEW(
                reserve, memory
                );
        }

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
_ResizeTempRT(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR dSurface
    );

gceSTATUS
gcoHARDWARE_FlushQuery(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL raDepth  = !Hardware->MsaaStates->disableRAWriteDepth &&
                       !Hardware->PEStates->alphaStates.test &&
                       !Hardware->MsaaStates->MsaaFragmentOp &&
                       gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_RA_DEPTH_WRITE);
    gctINT32 type;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    for (type = gcvQUERY_OCCLUSION; type < gcvQUERY_MAX_NUM; type++)
    {
        if (type == gcvQUERY_OCCLUSION)
        {
            if (Hardware->QUERYDirty->queryDirty[type])
            {
                switch(Hardware->QUERYStates->queryCmd[type])
                {
                case gcvQUERYCMD_BEGIN:
                    if (Hardware->QUERYStates->queryStatus[type] == gcvQUERY_Disabled)
                    {
                        gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, gcvPIPE_3D, (gctPOINTER *)&memory));

                        /* Before start OQ, should set OQ Mode firstly. */
                        if(Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
                        {
                            if(raDepth)
                            {
                                gcmONERROR(gcoHARDWARE_LoadState32NEW(
                                    Hardware,
                                    0x03860,
                                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))),
                                    (gctPOINTER *)&memory));
                            }
                            else
                            {
                                gcmONERROR(gcoHARDWARE_LoadState32NEW(
                                    Hardware,
                                    0x03860,
                                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))),
                                    (gctPOINTER*)&memory));
                            }
                        }
                        gcmONERROR(gcoHARDWARE_LoadCtrlStateNEW(
                            Hardware,
                            0x03824,
                            (gctUINT32)((gctUINTPTR_T)(Hardware->QUERYStates->queryHeaderPhysical[type])),
                            (gctPOINTER *)&memory
                            ));
                        Hardware->QUERYStates->queryStatus[type] = gcvQUERY_Enabled;
                        Hardware->QUERYStates->queryHeaderIndex[type] = 0;
                    }
                    break;

                case gcvQUERYCMD_RESUME_DELAY:
                    {
                        gcmONERROR(gcoHARDWARE_SetQuery(Hardware, 0, gcvQUERY_OCCLUSION, gcvQUERYCMD_RESUME_IMMEDIATE, (gctPOINTER *)&memory));
                    }
                    break;

                default:
                    break;
                }
                Hardware->QUERYDirty->queryDirty[type] = gcvFALSE;
            }
            else if(Hardware->preRADepth != raDepth && Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
            {
                /* If switch between RAZ and PEZ, OQ mode should be changed. */
                if((Hardware->preRADepth || Hardware->previousPEDepth) && Hardware->QUERYStates->queryStatus[type] == gcvQUERY_Enabled)
                {
                    if (!Hardware->features[gcvFEATURE_HALTI0])
                    {
                        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                    }

                    gcmONERROR(gcoHARDWARE_SetQuery(Hardware, 0, gcvQUERY_OCCLUSION, gcvQUERYCMD_PAUSE, (gctPOINTER *)&memory));

                    gcmASSERT(Hardware->QUERYStates->queryStatus[type] == gcvQUERY_Paused);

                    /* If Z Mode changed, OQ Mode should also be changed. */
                    if(raDepth)
                    {
                        gcmONERROR(gcoHARDWARE_LoadState32NEW(
                            Hardware,
                            0x03860,
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))),
                            (gctPOINTER *)&memory
                            ));
                    }
                    else
                    {
                        gcmONERROR(gcoHARDWARE_LoadState32NEW(
                            Hardware,
                            0x03860,
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))),
                            (gctPOINTER *)&memory
                            ));
                    }

                    gcmONERROR(gcoHARDWARE_SetQuery(Hardware, 0, gcvQUERY_OCCLUSION, gcvQUERYCMD_RESUME_IMMEDIATE, (gctPOINTER *)&memory));
                }
            }
            /* RA OQ checks */
            if (Hardware->QUERYStates->queryStatus[type] == gcvQUERY_Enabled && !Hardware->features[gcvFEATURE_BUG_FIXES18])
            {
                /* RA OQ heavily depends on accurate RA depth, need to flush PE depth before each such draw. */
                if (Hardware->PEStates->depthStates.mode != gcvDEPTH_NONE)
                {
                    Hardware->flushedDepth = gcvTRUE;

                    gcmONERROR(gcoHARDWARE_LoadCtrlStateNEW(
                        Hardware,
                        0x0380C,
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))),
                        (gctPOINTER *)&memory
                        ));
                }
            }
        }
        else if (Hardware->QUERYDirty->queryDirty[type])
        {
            gcmASSERT(Hardware->features[gcvFEATURE_HW_TFB]);

            switch (Hardware->QUERYStates->queryCmd[type])
            {
            case gcvQUERYCMD_BEGIN:
                if (Hardware->QUERYStates->queryStatus[type] != gcvQUERY_Enabled)
                {
                    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7005) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7005, Hardware->QUERYStates->queryHeaderPhysical[type]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7004) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7004, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) ((type == gcvQUERY_XFB_WRITTEN) ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                    Hardware->QUERYStates->queryStatus[type] = gcvQUERY_Enabled;
                    Hardware->QUERYStates->queryHeaderIndex[type] = 0;
                    if (type == gcvQUERY_PRIM_GENERATED)
                    {
                        Hardware->XFBDirty->s.internalXFBDirty = gcvTRUE;
                    }
                }
                break;
            case gcvQUERYCMD_RESUME_DELAY:
                if (Hardware->QUERYStates->queryStatus[type] == gcvQUERY_Paused)
                {
                    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7005) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7005, Hardware->QUERYStates->queryHeaderPhysical[type]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7004) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7004, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) ((type == gcvQUERY_XFB_WRITTEN) ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                    Hardware->QUERYStates->queryStatus[type] = gcvQUERY_Enabled;
                    if (type == gcvQUERY_PRIM_GENERATED)
                    {
                        Hardware->XFBDirty->s.internalXFBDirty = gcvTRUE;
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    stateDelta = stateDelta;

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushDepthOnly(
    IN gcoHARDWARE Hardware
    )
{
    gctBOOL depthOnly = Hardware->PEStates->depthStates.only;
    gctBOOL msaa = (Hardware->MsaaStates->sampleInfo.product > 1) && (Hardware->MsaaStates->sampleMask != 0);
    gcsHINT_PTR hints = Hardware->SHStates->programState.hints;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    if ((msaa && !(Hardware->features[gcvFEATURE_FAST_MSAA] || Hardware->features[gcvFEATURE_SMALL_MSAA])) ||
        (hints && (hints->hasKill           ||
                   hints->psHasFragDepthOut ||
                   hints->useMemoryAccess[gcvPROGRAM_STAGE_FRAGMENT] ||
                   (hints->stageBits == gcvPROGRAM_STAGE_COMPUTE_BIT))) ||
        (Hardware->PEStates->alphaStates.test)        ||
        (Hardware->MsaaStates->MsaaFragmentOp != 0)     ||
        (Hardware->SHStates->shaderLayered)           ||
        (Hardware->MsaaStates->sampleMaskLoc != -1))
    {
        /* Cannot enable depth only b/c ps cannot be bypassed now */
        if (depthOnly)
        {
            /* For MSAA case, color piple not disable, may read tmpRT.*/
            if (!Hardware->features[gcvFEATURE_PE_DISABLE_COLOR_PIPE])
            {
                _ResizeTempRT(Hardware, Hardware->PEStates->depthStates.surface);
                Hardware->PEDirty->colorTargetDirty = gcvTRUE;
                Hardware->PEDirty->colorConfigDirty = gcvTRUE;
            }
            depthOnly = gcvFALSE;
        }
    }
    else if ((Hardware->PEStates->colorStates.allColorWriteOff) &&
             (Hardware->PEStates->depthStates.mode != gcvDEPTH_NONE) &&
             (Hardware->PEStates->colorStates.colorCompression == gcvFALSE))
    {
         /* Enable depth only if no actually color be written */
         depthOnly = gcvTRUE;
    }

    if (Hardware->PEStates->depthStates.realOnly != depthOnly)
    {
        Hardware->PEStates->depthStates.realOnly  = depthOnly;

        /* depthOnly affects color/depth/shader programmings */
        Hardware->PEDirty->colorConfigDirty = gcvTRUE;
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
        Hardware->SHDirty->shaderDirty |= (gcvPROGRAM_STAGE_VERTEX_BIT | gcvPROGRAM_STAGE_FRAGMENT_BIT);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*
** For PS outmode, we can't use map table as RI32/RF32, RI332GI32/RF32GF32 have different setting.
*/
static gctINT32 _GetPsOutPutMode(gcoHARDWARE Hardware, gcsSURF_INFO_PTR surface)
{
    /* set default value */
    gctINT32 outputmode = 0x0;
    gceSURF_FORMAT format;

    if (!surface)
    {
        return outputmode;
    }

    format = surface->format;
    if ((format >= gcvSURF_R8I) && (format <= gcvSURF_A2B10G10R10UI))
    {
        if (((format >= gcvSURF_R8I) && (format <= gcvSURF_R16UI)) ||
            ((format >= gcvSURF_X8R8I) && (format <= gcvSURF_G16R16UI)) ||
            ((format >= gcvSURF_X8G8R8I) && (format <= gcvSURF_B16G16R16UI)) ||
            ((format >= gcvSURF_X8B8G8R8I) && (format <= gcvSURF_A16B16G16R16UI)) ||
            ((format == gcvSURF_A2B10G10R10UI)))
        {
            if(Hardware->features[gcvFEATURE_INTEGER_SIGNEXT_FIX])
            {
                switch(format)
                {
                case gcvSURF_R8I:
                case gcvSURF_X8R8I:
                case gcvSURF_G8R8I:
                case gcvSURF_X8G8R8I:
                case gcvSURF_B8G8R8I:
                case gcvSURF_X8B8G8R8I:
                case gcvSURF_A8B8G8R8I:
                    outputmode = 0x5;
                    break;
                case gcvSURF_R16I:
                case gcvSURF_X16R16I:
                case gcvSURF_G16R16I:
                case gcvSURF_X16G16R16I:
                case gcvSURF_B16G16R16I:
                case gcvSURF_X16B16G16R16I:
                case gcvSURF_A16B16G16R16I:
                    outputmode = 0x6;
                    break;
                case gcvSURF_R8UI:
                case gcvSURF_X8R8UI:
                case gcvSURF_G8R8UI:
                case gcvSURF_X8G8R8UI:
                case gcvSURF_B8G8R8UI:
                case gcvSURF_X8B8G8R8UI:
                case gcvSURF_A8B8G8R8UI:
                    outputmode = 0x3;
                    break;
                case gcvSURF_R16UI:
                case gcvSURF_X16R16UI:
                case gcvSURF_G16R16UI:
                case gcvSURF_X16G16R16UI:
                case gcvSURF_B16G16R16UI:
                case gcvSURF_X16B16G16R16UI:
                case gcvSURF_A16B16G16R16UI:
                case gcvSURF_A2B10G10R10UI:
                    outputmode = 0x4;
                    break;
                default:
                    break;
                }
            }
            else
            {
                outputmode = 0x1;
            }
        }
        else
        {
            outputmode = 0x2;
        }
    }
    else
    {
        switch (format)
        {
        case gcvSURF_G32R32I_1_G32R32F:
        case gcvSURF_G32R32UI_1_G32R32F:
        case gcvSURF_X16B16G16R16I_1_G32R32F:
        case gcvSURF_A16B16G16R16I_1_G32R32F:
        case gcvSURF_X16B16G16R16UI_1_G32R32F:
        case gcvSURF_A16B16G16R16UI_1_G32R32F:
        case gcvSURF_X32B32G32R32I_2_G32R32I:
        case gcvSURF_A32B32G32R32I_2_G32R32I:
        case gcvSURF_A32B32G32R32I_2_G32R32F:
        case gcvSURF_X32B32G32R32UI_2_G32R32UI:
        case gcvSURF_A32B32G32R32UI_2_G32R32UI:
        case gcvSURF_A32B32G32R32UI_2_G32R32F:
            outputmode = 0x2;
            break;
        default:
            break;
        }
    }

    return outputmode;
}

static gceSTATUS
_RemapPsOutput(
    gcoHARDWARE Hardware,
    gcsHINT_PTR psHints,
    gctINT      maxValidOutput,
    gctUINT32   psOutCntl0to3,
    gctUINT32  *psOutCntl4to7,
    gctPOINTER *Memory
    )
{
    gctINT32 tempRegister[gcdMAX_DRAW_BUFFERS];
    gctUINT32 newOutCntl0to3 = 0;
    gctUINT32 newOutCntl4to7 = 0;
    gctUINT index;
    gctUINT rtIndex;


    /* initialize temp register to invalid value */
    for (index = 0; index < gcdMAX_DRAW_BUFFERS; index++)
    {
        tempRegister[index] = -1;
    }

    tempRegister[0] = (((((gctUINT32) (psOutCntl0to3)) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) );
    tempRegister[1] = (((((gctUINT32) (psOutCntl0to3)) >> (0 ? 13:8)) & ((gctUINT32) ((((1 ? 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1)))))) );
    tempRegister[2] = (((((gctUINT32) (psOutCntl0to3)) >> (0 ? 21:16)) & ((gctUINT32) ((((1 ? 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1)))))) );
    tempRegister[3] = (((((gctUINT32) (psOutCntl0to3)) >> (0 ? 29:24)) & ((gctUINT32) ((((1 ? 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1)))))) );

    if (*psOutCntl4to7 != ~0U)
    {
        tempRegister[4] = (((((gctUINT32) (*psOutCntl4to7)) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) );
        tempRegister[5] = (((((gctUINT32) (*psOutCntl4to7)) >> (0 ? 13:8)) & ((gctUINT32) ((((1 ? 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1)))))) );
        tempRegister[6] = (((((gctUINT32) (*psOutCntl4to7)) >> (0 ? 21:16)) & ((gctUINT32) ((((1 ? 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1)))))) );
        tempRegister[7] = (((((gctUINT32) (*psOutCntl4to7)) >> (0 ? 29:24)) & ((gctUINT32) ((((1 ? 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1)))))) );
    }

    for (index = 0; index <= (gctUINT)maxValidOutput; ++index)
    {
        gctUINT j;
        rtIndex = Hardware->SHStates->psOutputMapping[index];

        for (j = 0; j < Hardware->config->renderTargets; ++j)
        {
            if (psHints->psOutput2RtIndex[j] == (gctINT)rtIndex)
            {
                break;
            }
        }

        if (j >= Hardware->config->renderTargets)
        {
            continue;
        }

        switch (index)
        {
        case 0:
            newOutCntl0to3 = ((((gctUINT32) (newOutCntl0to3)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
            break;

        case 1:
            newOutCntl0to3 = ((((gctUINT32) (newOutCntl0to3)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8)));
            break;

        case 2:
            newOutCntl0to3 = ((((gctUINT32) (newOutCntl0to3)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16)));
            break;

        case 3:
            newOutCntl0to3 = ((((gctUINT32) (newOutCntl0to3)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24)));
            break;

        case 4:
            newOutCntl4to7 = ((((gctUINT32) (newOutCntl4to7)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
            break;

        case 5:
            newOutCntl4to7 = ((((gctUINT32) (newOutCntl4to7)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8)));
            break;

        case 6:
            newOutCntl4to7 = ((((gctUINT32) (newOutCntl4to7)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16)));
            break;

        case 7:
            newOutCntl4to7 = ((((gctUINT32) (newOutCntl4to7)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (tempRegister[j]) & ((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24)));
            break;

        default:
            gcmASSERT(0);
        }
    }

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
             "Remapping ps output[0-3], before=0x%x, after=0x%x \n"
             "          ps output[4-7], before=0x%x, after=0x%x",
             psOutCntl0to3, newOutCntl0to3, psOutCntl4to7, newOutCntl4to7);


    /* Overwrite original programming for output 0-3 */
    gcoHARDWARE_LoadState32NEW(
        Hardware,
        0x01004,
        newOutCntl0to3,
        Memory);

    /* Overwrite original value for output 4-7 */
    if (newOutCntl4to7 != 0)
    {
        *psOutCntl4to7 = newOutCntl4to7;
    }


    return gcvSTATUS_OK;

}


static gceSTATUS
_GetPsOutputSetting(
    gcoHARDWARE Hardware,
    gctUINT32 *outputMode,
    gctUINT32 *saturationMode,
    gctUINT32 *psOutCnl4to7)
{
    gctUINT i;

    static const gctUINT32 colorOutSaturation[] =
    {
        gcvTRUE, /* 0x00: X4R4G4B4           */
        gcvTRUE, /* 0x01: A4R4G4B4           */
        gcvTRUE, /* 0x02: X1R5G5B5           */
        gcvTRUE, /* 0x03: A1R5G5B5           */
        gcvTRUE, /* 0x04: R5G6B5             */
        gcvTRUE, /* 0x05: X8R8G8B8           */
        gcvTRUE, /* 0x06: A8R8G8B8           */
        gcvTRUE, /* 0x07: YUY2               */
        gcvTRUE, /* 0x08: UYVY               */
        gcvTRUE, /* 0x09: INDEX8             */
        gcvTRUE, /* 0x0A: MONOCHROME         */
        gcvTRUE, /* 0x0B: HDR7E3             */
        gcvTRUE, /* 0x0C: HDR6E4             */
        gcvTRUE, /* 0x0D: HDR5E5             */
        gcvTRUE, /* 0x0E: HDR6E5             */
        gcvTRUE, /* 0x0F: YV12               */
        gcvTRUE, /* 0x10: A8                 */
        gcvFALSE, /* 0x11: RF16               */
        gcvFALSE, /* 0x12: RF16GF16           */
        gcvFALSE, /* 0x13: RF16GF16BF16AF16   */
        gcvFALSE, /* 0x14: RF32, RI32         */
        gcvFALSE, /* 0x15: RF32GF32, RI32GI32 */
        gcvTRUE, /* 0x16: R10G10B10A2        */
        gcvFALSE, /* 0x17: RI8                */
        gcvFALSE, /* 0x18: RI8GI8             */
        gcvFALSE, /* 0x19: RI8GI8BI8AI8       */
        gcvFALSE, /* 0x1A: RI16               */
        gcvFALSE, /* 0x1B: RI16GI16           */
        gcvFALSE, /* 0x1C: RI16GI16BI16AI16   */
        gcvFALSE, /* 0x1D: RF11GF11BF10       */
        gcvFALSE, /* 0x1E: RI10GI10BI10AI2    */
        gcvTRUE, /* 0x1F: R8G8               */
        gcvFALSE, /* 0x20: placeholder        */
        gcvFALSE, /* 0x21: placeholder        */
        gcvFALSE, /* 0x22: placeholder        */
        gcvTRUE, /* 0x23: R8                 */
    };

    gcmASSERT(outputMode);
    gcmASSERT(saturationMode);

    *outputMode =
    *saturationMode = 0;

    for (i = 0 ; i < Hardware->PEStates->colorOutCount; i++)
    {
        gcmASSERT(Hardware->PEStates->colorStates.target[i].format < gcmCOUNTOF(colorOutSaturation));
        switch (i)
        {
        case 0:
        *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)));
        break;

        case 1:
        *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4)));
        break;

        case 2:
        *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)));
        break;

        case 3:
        *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12)));
        break;

        case 4:
        *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ?
 18:16)));
        break;

        case 5:
        *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20)));
        break;

        case 6:
        *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ?
 26:24)));
        break;

        case 7:
        *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (colorOutSaturation[Hardware->PEStates->colorStates.target[i].format]) & ((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)));

        *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (_GetPsOutPutMode(Hardware, Hardware->PEStates->colorStates.target[i].surface)) & ((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)));
        break;

        default:
            gcmASSERT(0);

        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoHARDWARE_FlushShaders(
    IN gcoHARDWARE Hardware,
    IN gcePRIMITIVE PrimitiveType,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL pointList, bypass;
    gctUINT32 msaa, fastMsaa = 0, scale;
    gctUINT32 output;
    gctSIZE_T bufferCount;
    gctINT32 i;
    gctUINT32 address, count;
    gctUINT32_PTR buffer;
    gctSIZE_T offset;
    gctUINT32 outputMode, saturationMode0to3;
    gctUINT32 msaaExtraTemp = 0;
    gctUINT32 msasExtraInput = 0;
    gcePATCH_ID patchId = gcvPATCH_INVALID;
#if gcdALPHA_KILL_IN_SHADER
    gctBOOL alphaKill;
    gctBOOL colorKill;
#endif
    gctUINT32 numNops = 0;
    gctBOOL  remapPSout = gcvFALSE;
    gcsHINT_PTR hints = gcvNULL;
    gctINT32 maxValidOutput = -1;
    gcsPROGRAM_STATE_PTR programState;
    gctUINT32 psOutCntl0to3 = 0;
    gctUINT32 psOutCntl4to7 = ~0U;
    gctBOOL sampleMaskOut = gcvFALSE;
    gctUINT reservedPages;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Type=%d", Hardware, PrimitiveType);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    patchId = Hardware->patchID;

    reservedPages = Hardware->features[gcvFEATURE_SH_SNAP2PAGE_MAXPAGES_FIX] ? 0 : 1;

    /* Shortcuts. */
    programState = &Hardware->SHStates->programState;
    hints = programState->hints;

    if (Hardware->swwas[gcvSWWA_1163])
    {
        numNops = 8;
    }

    if(hints == gcvNULL)
    {
         gcmONERROR(gcvSTATUS_INVALID_ADDRESS);
    }

    if (hints->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT)
    {
        for (i = (Hardware->PEStates->colorOutCount - 1); i >= 0; i--)
        {
            gctUINT j;
            gcmASSERT(-1 != Hardware->SHStates->psOutputMapping[i]);
            if (maxValidOutput == -1)
            {
                for (j = 0; j < Hardware->config->renderTargets; j++)
                {
                    /* In reverse order, find the first one which has output from SH */
                    if (Hardware->SHStates->psOutputMapping[i] == hints->psOutput2RtIndex[j])
                    {
                        maxValidOutput = i;
                        break;
                    }
                }
            }

            if (!remapPSout)
            {
                /* The RT mapping at PE side is different from shader side, need remap */
                if (Hardware->SHStates->psOutputMapping[i] != hints->psOutput2RtIndex[i])
                {
                    remapPSout = gcvTRUE;
    #if gcdDEBUG
                    gcmPRINT("Shader output mismatch with PE setting, need remap");
    #endif
                }
            }
        }

        if (-1 == maxValidOutput)
        {
            maxValidOutput = 0;

        }
    }

#if gcdALPHA_KILL_IN_SHADER
    /* Determine alpka kill. */
    /*TODO: need review the code to make sure MRT is correctly considered */
    alphaKill = (Hardware->PEStates->alphaStates.blend[0]
                 &&
                 (hints->killStateAddress != 0)
                 &&
                 (Hardware->PEStates->alphaStates.srcFuncColor[0] == gcvBLEND_SOURCE_ALPHA)
                 &&
                 (Hardware->PEStates->alphaStates.srcFuncAlpha[0] == gcvBLEND_SOURCE_ALPHA)
                 &&
                 (Hardware->PEStates->alphaStates.trgFuncColor[0] == gcvBLEND_INV_SOURCE_ALPHA)
                 &&
                 (Hardware->PEStates->alphaStates.trgFuncAlpha[0] == gcvBLEND_INV_SOURCE_ALPHA)
                 &&
                 (Hardware->PEStates->alphaStates.modeColor[0] == gcvBLEND_ADD)
                 &&
                 (Hardware->PEStates->alphaStates.modeAlpha[0] == gcvBLEND_ADD)
                 );

    /* Determine color kill. */
    colorKill = (Hardware->PEStates->alphaStates.blend[0]
                 &&
                 (hints->killStateAddress != 0)
                 &&
                 (Hardware->PEStates->alphaStates.srcFuncColor[0] == gcvBLEND_ONE)
                 &&
                 (Hardware->PEStates->alphaStates.srcFuncAlpha[0] == gcvBLEND_ONE)
                 &&
                 (Hardware->PEStates->alphaStates.trgFuncColor[0] == gcvBLEND_ONE)
                 &&
                 (Hardware->PEStates->alphaStates.trgFuncAlpha[0] == gcvBLEND_ONE)
                 &&
                 (Hardware->PEStates->alphaStates.modeColor[0] == gcvBLEND_ADD)
                 &&
                 (Hardware->PEStates->alphaStates.modeAlpha[0] == gcvBLEND_ADD)
                 &&
                 !Hardware->PEStates->depthStates.write
                 );
#endif

    if (hints->stageBits & gcvPROGRAM_STAGE_VERTEX_BIT)
    {
        gcmDUMP(gcvNULL, "#[vsid %d]", hints->vertexShaderId);
    }

    if (hints->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT)
    {
        gcmDUMP(gcvNULL, "#[psid %d]", hints->fragmentShaderId);
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    /* Init the state buffer. */
    bufferCount = programState->stateBufferSize;
    buffer      = programState->stateBuffer;

    /* Loop through all bytes. */
    for (offset = 0; offset < bufferCount;)
    {
        /* Extract fields. */
        address = (((((gctUINT32) (*buffer)) >> (0 ? 15:0)) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1)))))) );
        count   = (((((gctUINT32) (*buffer)) >> (0 ? 25:16)) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1)))))) );

        /* Up-limiation of load-states count is 1024, and 0 means 1024 */
        count = (count == 0) ? 1024 : count;

        /* Skip the command. */
        buffer += 1;

        if (remapPSout)
        {
            if (address == 0x0401)
            {
                psOutCntl0to3 = *buffer;
            }
        }

        if (address == 0x040B)
        {
            psOutCntl4to7 = *buffer;
        }

        /* Load the states. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)count <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (count) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            for (i = 0; i < (gctINT)count; i += 1)
            {
                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, address + i,
                    *buffer++
                    );
            }

            if ((count & 1) == 0)
            {
                gcmSETFILLER_NEW(reserve, memory);

                buffer += 1;
                offset += 4 * (1 + count + 1);
            }
            else
            {
                offset += 4 * (1 + count);
            }

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }

    if (remapPSout)
    {
        _RemapPsOutput(Hardware,
                       hints,
                       maxValidOutput,
                       psOutCntl0to3,
                       &psOutCntl4to7,
                       (gctPOINTER *)&memory);
    }

    /* Determine MSAA mode. */
    msaa = ((hints->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT)
         && (Hardware->MsaaStates->sampleInfo.product > 1)
         && (Hardware->MsaaStates->sampleMask != 0)
         )
         ? 1
         : 0;

    /* real depth only need to set bypass mode */
    bypass = (hints->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT)
           ? Hardware->PEStates->depthStates.realOnly
           : 0;

    /* Determine Fast MSAA mode. */
    if (Hardware->MsaaStates->sampleInfo.product > 1
     && Hardware->features[gcvFEATURE_FAST_MSAA])
    {
        /* No extra info needed for msaa. */
        fastMsaa = 1;
    }


    if (Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
    {
        if (Hardware->MsaaStates->useSubSampleZInPS && msaa)
        {
            msaaExtraTemp = hints->fsIsDual16 ? 2 : 1;
            msasExtraInput = 1;
        }
    }
    else
    {
        /* For Non-FAST MSAA case, need extra temp register to pass sampleDepth infor.
        ** For Dual16, need another one.
        */
        if (msaa && !fastMsaa)
        {
            msaaExtraTemp = hints->fsIsDual16 ? 2 : 1;
            msasExtraInput = 1;
        }
    }

    if(msaa && Hardware->MsaaStates->sampleMaskOut)
    {
        sampleMaskOut = gcvTRUE;
    }

    /* Point list primitive? */
    pointList = (PrimitiveType == gcvPRIMITIVE_POINT_LIST);

    _GetPsOutputSetting(Hardware, &outputMode, &saturationMode0to3, &psOutCntl4to7);

    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0402) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};



    /* Set input count. */
    gcmSETSTATEDATA_NEW(
        stateDelta, reserve, memory, gcvFALSE, 0x0402,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hints->fsInputCount + msasExtraInput) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:16) - (0 ?
 20:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ?
 20:16))) | (((gctUINT32) ((gctUINT32) (hints->psHighPVaryingCount) & ((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ?
 20:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (hints->psInputControlHighpPosition || Hardware->MsaaStates->sampleMaskInPos || Hardware->SHStates->shaderLayered) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)))
            );
#if TEMP_SHADER_PATCH
    switch (hints->pachedShaderIdentifier)
    {
    case gcvMACHINECODE_GLB27_RELEASE_0:
        hints->fsMaxTemp = 0x00000004;
        break;

    case gcvMACHINECODE_GLB25_RELEASE_0:
        hints->fsMaxTemp = 0x00000008;
        break;

    case gcvMACHINECODE_GLB25_RELEASE_1:
        hints->fsMaxTemp = 0x00000008;
        break;

    default:
        break;
    }
#endif

    /* Set temporary register control. */
    gcmSETSTATEDATA_NEW(
        stateDelta, reserve, memory, gcvFALSE, 0x0403,
        hints->fsMaxTemp + msaaExtraTemp);


    /* Set bypass for depth-only modes. */
    /* COLOR_OUT_COUNT = numRT -1 */
     gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0404,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (bypass) & ((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
            | saturationMode0to3
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (fastMsaa) & ((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) ((gctUINT32)(maxValidOutput)) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (sampleMaskOut) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20)))
            );

    gcmENDSTATEBATCH_NEW(
        reserve, memory
        );

    /* Reprogram 0x0E30 for msaa rendering */
    if (msaa && (Hardware->MsaaStates->sampleShading || Hardware->MsaaStates->sampleShadingByPS || Hardware->MsaaStates->isSampleIn))
    {
        bypass = 0;

        for (i = 0; i < 16; i ++)
        {
            gctUINT state =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 0]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ?
 17:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ? 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ? 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ?
 25:24)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ? 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 3:2) - (0 ?
 3:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ?
 3:2)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ? 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 11:10) - (0 ?
 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
 11:10)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 15:14) - (0 ?
 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 19:18) - (0 ?
 19:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ?
 19:18)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ? 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ? 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 27:26) - (0 ?
 27:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ?
 27:26)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ? 31:30) - (0 ?
 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ?
 31:30)));

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E30 + i) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E30 + i,
 state);    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
    }

    /* Bypass VS */
    if (Hardware->features[gcvFEATURE_HALTI5] &&
        !(programState->hints->stageBits & gcvPROGRAM_STAGE_VERTEX_BIT))
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0228) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0228, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    /* Bypass TS */
    if (Hardware->features[gcvFEATURE_TESSELLATION] &&
        (!(programState->hints->stageBits & gcvPROGRAM_STAGE_TES_BIT)))
    {
        gcmASSERT(!(programState->hints->stageBits & gcvPROGRAM_STAGE_TCS_BIT));
        gcmASSERT(!(programState->hints->stageBits & gcvPROGRAM_STAGE_TES_BIT));

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x52C6) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x52C6, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x52C7) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x52C7, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x5286) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x5286, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    /* Bypass GS */
    if (Hardware->features[gcvFEATURE_GEOMETRY_SHADER] &&
        (!(programState->hints->stageBits & gcvPROGRAM_STAGE_GEOMETRY_BIT)))
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0440) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0440, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0450) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0450, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};



    }

    /* Output count into PA */
    if (Hardware->features[gcvFEATURE_HALTI5])
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x02AA) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x02AA, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (bypass ? 1 : hints->shader2PaOutputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    }

    /* Set element count. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x028C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x028C, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (bypass ? 0 : hints->elementCount) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

#if TEMP_SHADER_PATCH
    if (hints && hints->pachedShaderIdentifier == gcvMACHINECODE_ANTUTU0)
    {
        hints->componentCount = 0x00000006;
    }
#endif

    /* Set component count. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E07) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E07, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ? 6:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (bypass ? 0 : hints->componentCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ? 6:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ?
 6:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    while (numNops--)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E07) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0E07, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ? 6:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (bypass ? 0 : hints->componentCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ? 6:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ?
 6:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x040C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x040C, outputMode );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    if (psOutCntl4to7 != ~0U)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x040B) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x040B, psOutCntl4to7 );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
    if (Hardware->config->chipModel < gcv1000 &&
        !(Hardware->config->chipModel == gcv900 && Hardware->config->chipRevision == 0x5250))
    {
        gcmASSERT(hints->componentCount == (hints->elementCount * 4));
    }
#endif

    if (patchId == gcvPATCH_GLBM27 || patchId == gcvPATCH_GFXBENCH)
    {
        scale = hints->balanceMax;
    }
    /* Scale to 50%. */
    else
    {
        scale
            = ((Hardware->config->chipModel == gcv400) || (Hardware->config->chipRevision >= 0x5240))
                ?  hints->balanceMax
                : (hints->balanceMin + hints->balanceMax) / 2;
    }

    gcmASSERT(scale > 0 ||
        hints->stageBits & (gcvPROGRAM_STAGE_OPENCL_BIT | gcvPROGRAM_STAGE_COMPUTE_BIT));

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x020C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x020C, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (scale) & ((gctUINT32) ((((1 ? 7:0) - (0 ?
 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (hints->balanceMin) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:16) - (0 ?
 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 21:16) - (0 ?
 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


#if gcdAUTO_SHIFT
    if (hints->autoShift)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0216) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0216, ((((gctUINT32) (0x00001005)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (gcmMIN(hints->balanceMax * 2, 255)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:11) - (0 ?
 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }
    else
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0216) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0216, 0x00001005);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }
#endif

    if (!hints->vsUseStoreAttr)
    {
        if (programState->hints->stageBits & (gcvPROGRAM_STAGE_GEOMETRY_BIT | gcvPROGRAM_STAGE_TES_BIT))
        {
            output = hints->vsOutputCount;
        }
        else
        {
            if (bypass)
            {
                /* In bypass mode, we only need the position output and the point
                ** size if present and the primitive is a point list. */
                output
                    = 1 + ((hints->vsHasPointSize && pointList) ? 1 : 0);
            }
            else
            {
                /* If the Vertex Shader generates a point size, and this point size will not be streamed out,
                   and the primitive is not a point, we don't need to send it down. */
                output
                    = (hints->vsHasPointSize && hints->vsPtSizeAtLastLinkLoc &&
                       !pointList && !hints->isPtSizeStreamedOut)
                        ? hints->vsOutputCount - 1
                        : hints->vsOutputCount;
            }
        }

        /* SW WA for transform feedback, when VS has no output. */
        if (output == 0)
        {
            output = 1;
        }

        /* Set output count. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0201) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0201, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (output) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:8) - (0 ?
 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hints->vsOutput16RegNo) & ((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:16) - (0 ?
 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hints->vsOutput17RegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:24) - (0 ?
 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hints->vsOutput18RegNo) & ((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }
    else
    {
        /* USC has been stored by shader, attr-sequencer does not need this info, so just
           set output count to 0 */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0201) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0201, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (Hardware->features[gcvFEATURE_32_ATTRIBUTES])
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E22) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E22, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ? 6:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (hints->ptSzAttrIndex * 4) & ((gctUINT32) ((((1 ?
 6:0) - (0 ? 6:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ?
 6:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:8) - (0 ?
 14:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ?
 14:8))) | (((gctUINT32) ((gctUINT32) (hints->pointCoordComponent) & ((gctUINT32) ((((1 ?
 14:8) - (0 ? 14:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ?
 14:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 22:16) - (0 ?
 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (hints->rtArrayComponent) & ((gctUINT32) ((((1 ?
 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ?
 22:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:24) - (0 ?
 30:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:24) - (0 ? 30:24) + 1))))))) << (0 ?
 30:24))) | (((gctUINT32) ((gctUINT32) (hints->primIdComponent) & ((gctUINT32) ((((1 ?
 30:24) - (0 ? 30:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:24) - (0 ? 30:24) + 1))))))) << (0 ?
 30:24))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

#if gcdALPHA_KILL_IN_SHADER
    if (alphaKill || colorKill)
    {
        gcmASSERT(hints->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT);
        /* Reprogram endPC address. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (hints->killStateAddress) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, hints->killStateAddress, alphaKill ?
 hints->alphaKillStateValue : hints->colorKillStateValue);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Reprogram shader instructions. */
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (hints->killInstructionAddress) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW(stateDelta,
                        reserve,
                        memory,
                        gcvFALSE,
                        hints->killInstructionAddress + 0,
                        alphaKill
                        ? hints->alphaKillInstruction[0]
                        : hints->colorKillInstruction[0]);
        gcmSETSTATEDATA_NEW(stateDelta,
                        reserve,
                        memory,
                        gcvFALSE,
                        hints->killInstructionAddress + 1,
                        alphaKill
                        ? hints->alphaKillInstruction[1]
                        : hints->colorKillInstruction[1]);
        gcmSETSTATEDATA_NEW(stateDelta,
                        reserve,
                        memory,
                        gcvFALSE,
                        hints->killInstructionAddress + 2,
                        alphaKill
                        ? hints->alphaKillInstruction[2]
                        : hints->colorKillInstruction[2]);

        gcmENDSTATEBATCH_NEW(reserve, memory);
    }
#endif

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);


    /* Reset dirty. */
    Hardware->SHDirty->shaderDirty = 0;
#if gcdENABLE_APPCTXT_BLITDRAW
    Hardware->SHDirty->programSwitched = gcvFALSE;
#endif
OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_HzClearValueControl(
    IN gceSURF_FORMAT Format,
    IN gctUINT32 ZClearValue,
    OUT gctUINT32 * HzClearValue OPTIONAL,
    OUT gctUINT32 * HzControl OPTIONAL
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 clearValue = 0, control = 0;

    gcmHEADER_ARG("Format=%d ZClearValue=0x%x", Format, ZClearValue);

    /* Dispatch on format. */
    switch (Format)
    {
    case gcvSURF_D16:
        /* 16-bit Z. */
        clearValue = ZClearValue;
        control    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
        break;

    case gcvSURF_D24X8:
    case gcvSURF_D24S8:
        /* 24-bit Z. */
        clearValue = ZClearValue >> 8;
        control    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) (0x8 & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) (0x8 & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
        break;

    case gcvSURF_S8:
    case gcvSURF_X24S8:
        gcmFOOTER();
        return gcvSTATUS_OK;

    default:
        /* Not supported. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (HzClearValue != gcvNULL)
    {
        /* Return clear value. */
        *HzClearValue = clearValue;
    }

    if (HzControl != gcvNULL)
    {
        /* Return control. */
        *HzControl = control;
    }

    /* Success. */
    gcmFOOTER_ARG("*HzClearValue=0x%x *HzControl=0x%x",
                  gcmOPT_VALUE(HzClearValue), gcmOPT_VALUE(HzControl));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetWClipEnable(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Wclip=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the state. */
    if (Hardware->PAAndSEStates->paStates.wclip != Enable)
    {
        Hardware->PAAndSEStates->paStates.wclip = Enable;
        Hardware->PAAndSEDirty->paConfigDirty    = gcvTRUE;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetWPlaneLimit(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT Value
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Value=%f", Hardware, Value);

    do
    {
        /* Switch to 3D pipe. */
        gcmERR_BREAK(gcoHARDWARE_SelectPipe(Hardware, 0x0, gcvNULL));

        gcmERR_BREAK(gcoHARDWARE_LoadState(Hardware,
                                           0x00A2C,
                                           1,
                                           &Value));
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetRTNERounding(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x RTNERounding=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Check if rtneRounding mode needs to change. */
    if (Hardware->SHStates->rtneRounding != Enable)
    {
        if (Enable)
        {
            /* Only handle when the IP has RTNE */
            if (Hardware->features[gcvFEATURE_SHADER_ENHANCEMENTS2])
            {
                Hardware->SHStates->rtneRounding = Enable;
            }
        }
        else
        {
            Hardware->SHStates->rtneRounding = Enable;
        }

        /* Perform a load state. */
        if (Hardware->features[gcvFEATURE_HALTI5])
        {
            gcmONERROR(gcoHARDWARE_LoadState32(
                Hardware,
                0x15600,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) ((Hardware->SHStates->rtneRounding ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                ));
        }
        else
        {
            gcmONERROR(gcoHARDWARE_LoadState32(
                Hardware,
                0x00860,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) ((Hardware->SHStates->rtneRounding ?
 0x1 : 0x0)) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                ));
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetColorOutCount(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 ColorOutCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x ColorOutCount=%d", Hardware, ColorOutCount);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Assume
        1) 3D pipe,
        2) HALTI0 features are available,
        3) HW supports ColorOutCount buffers as RT.
     */

    gcmASSERT(ColorOutCount <= Hardware->config->renderTargets);

    Hardware->PEStates->colorOutCount = ColorOutCount;
    Hardware->SHDirty->shaderDirty |= gcvPROGRAM_STAGE_FRAGMENT_BIT;

    /* Disable tile status for mRT. */
    if (!Hardware->features[gcvFEATURE_MRT_TILE_STATUS_BUFFER])
    {
        if (Hardware->PEStates->colorOutCount > 1)
        {
            gctUINT32 i;

            for(i = 0; i < Hardware->PEStates->colorOutCount; i++)
            {
                /* Make sure RT is set before color output count is sent */
                if (Hardware->PEStates->colorStates.target[i].surface)
                {
                    gcmONERROR(gcoHARDWARE_DisableTileStatus(Hardware,
                                                             Hardware->PEStates->colorStates.target[i].surface,
                                                             gcvTRUE));
                }
            }
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetColorCacheMode(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->features[gcvFEATURE_128BTILE] ||
        Hardware->features[gcvFEATURE_EDGEAA])
    {
        /* Loop all at set cacheMode */
        if (Hardware->PEStates->colorOutCount >= 1)
        {
            gctUINT32 i;
            gceCACHE_MODE mode = gcvCACHE_NONE;
            gctBOOL edgeAA = gcvFALSE;
            gcsSURF_INFO_PTR surface;
            gctBOOL disableTS = gcvFALSE;

            for(i = 0; i < Hardware->PEStates->colorOutCount; i++)
            {
                disableTS = gcvFALSE;
                surface = Hardware->PEStates->colorStates.target[i].surface;

                if (surface)
                {
                    if (mode == gcvCACHE_NONE)
                    {
                        mode = surface->cacheMode;
                    }
                    else if (mode != surface->cacheMode)
                    {
                        disableTS = gcvTRUE;
                    }

                    if (!edgeAA)
                    {
                        edgeAA = surface->edgeAA;
                    }
                    else if (!surface->edgeAA)
                    {
                        disableTS = gcvTRUE;
                    }

                    if (disableTS)
                    {
                        gcmONERROR(gcoHARDWARE_DisableTileStatus(Hardware, surface, gcvTRUE));
                    }
                }
            }

            if (Hardware->PEStates->colorStates.cacheMode128 != mode)
            {
                Hardware->PEStates->colorStates.cacheMode128 = mode;
                Hardware->PEDirty->colorConfigDirty = gcvTRUE;
            }

            if (Hardware->PEStates->colorStates.edgeAA != edgeAA)
            {
                Hardware->PEStates->colorStates.edgeAA = edgeAA;
                Hardware->PEDirty->colorConfigDirty = gcvTRUE;
            }
        }
    }


OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushCache(
    IN gcoHARDWARE Hardware,
    IN gctUINT64 CommandBuffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32_PTR memory = (gctUINT32_PTR) gcmUINT64_TO_PTR(CommandBuffer);

    gcmHEADER_ARG("Hardware=0x%x CommandBuffer=0x%x", Hardware, CommandBuffer);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmASSERT(Hardware->currentPipe == gcvPIPE_3D);


    memory[0]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    memory[1]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ?
 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ?
 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:11) - (0 ?
 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
        ;

    gcmDUMP(gcvNULL,
            "#[state 0x%04X 0x%08X]",
            0x0E03, memory[1]);
OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;

}

gceSTATUS
gcoHARDWARE_SetQuery(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 QueryHeader,
    IN gceQueryType Type,
    IN gceQueryCmd QueryCmd,
    IN gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32_PTR memory;

    gcmHEADER_ARG("Hardware=0x%x QueryHeader = 0x%x, Type = %u, QueryCmd = %u, Memory=0x%x",
                  Hardware, QueryHeader, Type, QueryCmd, Memory);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmVERIFY_ARGUMENT(Type < gcvQUERY_MAX_NUM);

    if ((Type == gcvQUERY_OCCLUSION && !Hardware->features[gcvFEATURE_HALTI0]) ||
        ((Type == gcvQUERY_XFB_WRITTEN || Type == gcvQUERY_PRIM_GENERATED) && !Hardware->features[gcvFEATURE_HW_TFB]))
    {
        gcmFOOTER();
        return status;
    }

    switch (QueryCmd)
    {
    case gcvQUERYCMD_BEGIN:
        if (Hardware->QUERYStates->queryStatus[Type] == gcvQUERY_Disabled)
        {
            Hardware->QUERYStates->queryCmd[Type] = QueryCmd;
            Hardware->QUERYDirty->queryDirty[Type] = gcvTRUE;
            Hardware->QUERYStates->queryHeaderPhysical[Type] = QueryHeader;

            /* OQ state change could affect current depth HZ status,
            which depend on depth config dirty and Target dirty now,
            TO_DO, refine*/
            if (Type == gcvQUERY_OCCLUSION)
            {
                Hardware->PEDirty->depthConfigDirty = gcvTRUE;
                Hardware->PEDirty->depthTargetDirty = gcvTRUE;
            }
        }
        /* delay it to following draw */
        break;

    case gcvQUERYCMD_PAUSE:
        if (Type == gcvQUERY_OCCLUSION && Hardware->QUERYStates->statusInCmd[Type] == gcvQUERY_Enabled && Memory)
        {
            memory = (gctUINT32_PTR) *Memory;

            memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 26:26) - (0 ?
 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E0C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));
            memory[1] = 31415926;

            *Memory = (memory + 2);

            Hardware->QUERYStates->queryStatus[Type] =
            Hardware->QUERYStates->statusInCmd[Type] = gcvQUERY_Paused;
            Hardware->QUERYDirty->queryDirty[Type]  = gcvTRUE;
        }
        else if ((Type == gcvQUERY_XFB_WRITTEN || Type == gcvQUERY_PRIM_GENERATED)
                 && Hardware->QUERYStates->statusInCmd[Type] == gcvQUERY_Enabled && Memory)
        {
            memory = (gctUINT32_PTR) *Memory;

            memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 26:26) - (0 ?
 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7005) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

            memory[1] = Hardware->QUERYStates->queryHeaderPhysical[Type];

            memory[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 26:26) - (0 ?
 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7004) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

            memory[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) ((Type == gcvQUERY_XFB_WRITTEN) ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));

            *Memory = (memory + 4);

            Hardware->QUERYStates->queryStatus[Type] =
            Hardware->QUERYStates->statusInCmd[Type] = gcvQUERY_Paused;
            Hardware->QUERYDirty->queryDirty[Type] = gcvTRUE;
        }
        break;

    case gcvQUERYCMD_RESUME_DELAY:
        if (Hardware->QUERYStates->queryStatus[Type] == gcvQUERY_Paused)
        {
            Hardware->QUERYStates->queryCmd[Type] = QueryCmd;
            Hardware->QUERYDirty->queryDirty[Type]  = gcvTRUE;
        }
        /* delay it to following draw */
        break;

    case gcvQUERYCMD_RESUME_IMMEDIATE:
        if (Type == gcvQUERY_OCCLUSION && Hardware->QUERYStates->queryStatus[Type] == gcvQUERY_Paused && Memory)
        {
            memory = (gctUINT32_PTR) *Memory;

            Hardware->QUERYStates->queryHeaderIndex[Type]++;

            if (Hardware->QUERYStates->queryHeaderIndex[Type] >= 64)
            {
                gcmFATAL("commit 64 time, when OQ enable!!!!");
                Hardware->QUERYStates->queryHeaderIndex[Type] = 63;
            }

            memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 26:26) - (0 ?
 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))
                               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
                               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E09) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

            memory[1] = gcmALL_TO_UINT32(Hardware->QUERYStates->queryHeaderPhysical[Type])
                      + (Hardware->QUERYStates->queryHeaderIndex[Type]) * gcmSIZEOF(gctUINT64);

            *Memory = (memory + 2);

            Hardware->QUERYStates->queryStatus[Type] = gcvQUERY_Enabled;
        }
        break;

    case gcvQUERYCMD_END:
        if (Type == gcvQUERY_OCCLUSION)
        {
            if (Hardware->QUERYStates->queryStatus[Type] != gcvQUERY_Disabled)
            {
                gctBOOL hzTSEnabled = gcvFALSE;

                gcmONERROR(gcoHARDWARE_InvalidateCache(Hardware));

                hzTSEnabled = (((((gctUINT32) (Hardware->MCStates->memoryConfig)) >> (0 ? 12:12)) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) );

                /* pause HZ TS */
                if (hzTSEnabled)
                {
                    gcoHARDWARE_PauseTileStatus(Hardware,gcvTILE_STATUS_PAUSE);
                }

                if (Hardware->QUERYStates->queryStatus[Type] == gcvQUERY_Enabled)
                {
                    gcmONERROR(gcoHARDWARE_LoadCtrlState(
                        Hardware, 0x03830, 31415926
                        ));
                }

                if (hzTSEnabled)
                {
                    gcoHARDWARE_PauseTileStatus(Hardware,gcvTILE_STATUS_RESUME);
                }
                Hardware->QUERYStates->statusInCmd[Type] = gcvQUERY_Disabled;
            }
            Hardware->QUERYDirty->queryDirty[Type] = gcvTRUE;
            /* OQ state change could affect current depth HZ status,
            which depend on depth config dirty and Target dirty now,
            TO_DO, refine*/
            Hardware->PEDirty->depthConfigDirty = gcvTRUE;
            Hardware->PEDirty->depthTargetDirty = gcvTRUE;

            Hardware->QUERYStates->queryStatus[Type] = gcvQUERY_Disabled;
            Hardware->QUERYStates->queryCmd[Type] = QueryCmd;
        }
        else if ((Type == gcvQUERY_XFB_WRITTEN || Type == gcvQUERY_PRIM_GENERATED)
                 && Hardware->QUERYStates->queryStatus[Type] != gcvQUERY_Disabled)
        {
            gcmONERROR(gcoHARDWARE_LoadCtrlState(Hardware,
                                                 0x1C014,
                                                 Hardware->QUERYStates->queryHeaderPhysical[Type]));
            gcmONERROR(gcoHARDWARE_LoadCtrlState(Hardware,
                                                 0x1C010,
                                                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                                               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
                                               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) ((Type == gcvQUERY_XFB_WRITTEN) ?
 0x0 : 0x1) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))));

            Hardware->QUERYStates->statusInCmd[Type] =
            Hardware->QUERYStates->queryStatus[Type] = gcvQUERY_Disabled;
            Hardware->QUERYStates->queryCmd[Type] = QueryCmd;
        }
        break;

    default:
        gcmASSERT(0);
        gcmPRINT("Invalid Query Command");
        break;
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_GetQueryIndex(
    IN gcoHARDWARE Hardware,
    IN gceQueryType Type,
    IN gctINT32 * Index
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Type=%d, Index=%d", Hardware, Type, Index);

    gcmGETHARDWARE(Hardware);

    if ((Type == gcvQUERY_OCCLUSION && !Hardware->features[gcvFEATURE_HALTI0]) ||
        ((Type == gcvQUERY_XFB_WRITTEN || Type == gcvQUERY_PRIM_GENERATED) && !Hardware->features[gcvFEATURE_HW_TFB]))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (Index)
    {
        *Index = Hardware->QUERYStates->queryHeaderIndex[Type];
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_PrimitiveRestart(
    IN gcoHARDWARE Hardware,
    IN gctBOOL PrimitiveRestart
    )
{
    gceSTATUS   status;

    gcmHEADER_ARG("Hardware=0x%x PrimitiveRestart=%d", Hardware, PrimitiveRestart);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Test if primitive restart is present. */
    if (PrimitiveRestart && !Hardware->features[gcvFEATURE_HALTI0])
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Set primitive restart options. */
    Hardware->FEDirty->indexDirty       = gcvTRUE;
    Hardware->FEStates->primitiveRestart = PrimitiveRestart;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FastDrawIndexedPrimitive(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory)
{
    gceSTATUS status;
    gctUINT primCount = FastFlushInfo->drawCount / 3;

    gcmHEADER_ARG("Hardware=0x%x FastFlushInfo=0x%x", Hardware, FastFlushInfo);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Send command */
    if (FastFlushInfo->hasHalti)
    {
        gctSIZE_T vertexCount;
        gctUINT32 drawCommand;
        gctUINT32 drawCount;
        gctUINT32 drawMode = (FastFlushInfo->indexCount != 0) ?
                0x1 :
                0x0;

        /* Define state buffer variables. */
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        if (!Hardware->features[gcvFEATURE_HALTI2])
        {
            vertexCount = FastFlushInfo->drawCount - (FastFlushInfo->drawCount % 3);
        }
        else
        {
            vertexCount = FastFlushInfo->drawCount;
        }

        /* Determine draw command. */
        drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (drawMode) & ((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (FastFlushInfo->instanceCount) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)));

        drawCount = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:0) - (0 ? 23:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ?
 23:0))) | (((gctUINT32) ((gctUINT32) (vertexCount) & ((gctUINT32) ((((1 ?
 23:0) - (0 ? 23:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ?
 23:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) ((FastFlushInfo->instanceCount >> 16)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24)));

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Program the AQCommandDX8Primitive.Command data. */
        *memory++ = drawCommand;
        *memory++ = drawCount;

        if (Hardware->features[gcvFEATURE_HALTI2])
        {
            *memory++ = 0;
            *memory++ = 0;
        }
        else
        {
            /* start index is not set */
            *memory++ = 0;

            gcmSETFILLER_NEW(
                reserve,
                memory
                );
        }

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }
    else
    {
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E05) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0E05, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Program the AQCommandDX8IndexPrimitive.Command data. */
        memory[0] =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        /* Program the AQCommandDX8IndexPrimitive.Primtype data. */
        memory[1] =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0x4) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)));

        /* Program the AQCommandDX8IndexPrimitive.Start data. */
        memory[2] = 0;

        /* Program the AQCommandDX8IndexPrimitive.Primcount data. */
        gcmSAFECASTSIZET(memory[3], primCount);

        /* Program the AQCommandDX8IndexPrimitive.Basevertex data. */
        memory[4] = 0;

        memory += 5;

        if (5 & 1)
        {
            gcmSETFILLER_NEW(
                reserve,
                memory
                );
        }

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }

    /* Success */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FastFlushDepthCompare(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory)
{
    static const gctINT32 xlateCompare[] =
    {
        /* gcvCOMPARE_INVALID */
        -1,
        /* gcvCOMPARE_NEVER */
        0x0,
        /* gcvCOMPARE_NOT_EQUAL */
        0x5,
        /* gcvCOMPARE_LESS */
        0x1,
        /* gcvCOMPARE_LESS_OR_EQUAL */
        0x3,
        /* gcvCOMPARE_EQUAL */
        0x2,
        /* gcvCOMPARE_GREATER */
        0x4,
        /* gcvCOMPARE_GREATER_OR_EQUAL */
        0x6,
        /* gcvCOMPARE_ALWAYS */
        0x7,
    };

    static const gctUINT32 xlateMode[] =
    {
        /* gcvDEPTH_NONE */
        0x0,
        /* gcvDEPTH_Z */
        0x1,
        /* gcvDEPTH_W */
        0x2,
    };

    gceSTATUS status;
    gcsDEPTH_INFO *depthInfo;

    gctBOOL flushNeeded = gcvFALSE;
    gctBOOL stallNeeded = gcvFALSE;

    gctBOOL ezEnabled = gcvFALSE;
    gctBOOL peDepth = gcvTRUE;

    gcmHEADER_ARG("Hardware=0x%x FastFlushInfo=0x%x", Hardware, FastFlushInfo);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    {
        /* Define state buffer variables. */
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        depthInfo = &Hardware->PEStates->depthStates;

        /* disable EZ, as there plenty of possible to disable it, I don't want to check */
        depthInfo->early = gcvFALSE;

        /* Test if configuration is dirty. */
        depthInfo->compare = FastFlushInfo->depthInfoCompare;

        if (Hardware->previousPEDepth != peDepth)
        {
            /* Need flush and stall when switching from PE Depth ON to PE Depth Off? */
            if (Hardware->previousPEDepth)
            {
                stallNeeded = gcvTRUE;
            }

            Hardware->previousPEDepth = peDepth;
        }

        /* If earlyZ or depth compare changed, and earlyZ is enabled, need to flush Zcache and stall */
        /* No need to initialize previous values bc no PE depth write before first draw. */
        if (Hardware->prevEarlyZ != ezEnabled ||
            Hardware->prevDepthCompare != depthInfo->compare)
        {
            Hardware->prevEarlyZ = ezEnabled;
            Hardware->prevDepthCompare = depthInfo->compare;
        }

        depthInfo->regDepthConfig &= ~(((((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) |
            ((((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) |
            ((((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8))) |
            ((((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
            );

        depthInfo->regDepthConfig |= (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (xlateMode[FastFlushInfo->depthMode]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (ezEnabled) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ?
 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (xlateCompare[FastFlushInfo->compare]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (!peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24)))
            );

        /* Add the flush state. */
        if (flushNeeded || stallNeeded)
        {
            /* Mark depth cache as flushed. */
            Hardware->flushedDepth = gcvTRUE;
        }

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        if (flushNeeded || stallNeeded)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0E03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0500) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0500, depthInfo->regDepthConfig);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0382) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0382,
 depthInfo->regRAControl );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }

    /* Success */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FastFlushAlpha(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory
    )
{
    gctBOOL floatPipe = gcvFALSE;

    static const gctINT32 xlateCompare[] =
    {
        /* gcvCOMPARE_INVALID */
        -1,
        /* gcvCOMPARE_NEVER */
        0x0,
        /* gcvCOMPARE_NOT_EQUAL */
        0x5,
        /* gcvCOMPARE_LESS */
        0x1,
        /* gcvCOMPARE_LESS_OR_EQUAL */
        0x3,
        /* gcvCOMPARE_EQUAL */
        0x2,
        /* gcvCOMPARE_GREATER */
        0x4,
        /* gcvCOMPARE_GREATER_OR_EQUAL */
        0x6,
        /* gcvCOMPARE_ALWAYS */
        0x7,
    };

    static const gctUINT32 xlateFuncSource[] =
    {
        /* gcvBLEND_ZERO */
        0x0,
        /* gcvBLEND_ONE */
        0x1,
        /* gcvBLEND_SOURCE_COLOR */
        0x2,
        /* gcvBLEND_INV_SOURCE_COLOR */
        0x3,
        /* gcvBLEND_SOURCE_ALPHA */
        0x4,
        /* gcvBLEND_INV_SOURCE_ALPHA */
        0x5,
        /* gcvBLEND_TARGET_COLOR */
        0x8,
        /* gcvBLEND_INV_TARGET_COLOR */
        0x9,
        /* gcvBLEND_TARGET_ALPHA */
        0x6,
        /* gcvBLEND_INV_TARGET_ALPHA */
        0x7,
        /* gcvBLEND_SOURCE_ALPHA_SATURATE */
        0xA,
        /* gcvBLEND_CONST_COLOR */
        0xD,
        /* gcvBLEND_INV_CONST_COLOR */
        0xE,
        /* gcvBLEND_CONST_ALPHA */
        0xB,
        /* gcvBLEND_INV_CONST_ALPHA */
        0xC,
    };

    static const gctUINT32 xlateFuncTarget[] =
    {
        /* gcvBLEND_ZERO */
        0x0,
        /* gcvBLEND_ONE */
        0x1,
        /* gcvBLEND_SOURCE_COLOR */
        0x2,
        /* gcvBLEND_INV_SOURCE_COLOR */
        0x3,
        /* gcvBLEND_SOURCE_ALPHA */
        0x4,
        /* gcvBLEND_INV_SOURCE_ALPHA */
        0x5,
        /* gcvBLEND_TARGET_COLOR */
        0x8,
        /* gcvBLEND_INV_TARGET_COLOR */
        0x9,
        /* gcvBLEND_TARGET_ALPHA */
        0x6,
        /* gcvBLEND_INV_TARGET_ALPHA */
        0x7,
        /* gcvBLEND_SOURCE_ALPHA_SATURATE */
        0xA,
        /* gcvBLEND_CONST_COLOR */
        0xD,
        /* gcvBLEND_INV_CONST_COLOR */
        0xE,
        /* gcvBLEND_CONST_ALPHA */
        0xB,
        /* gcvBLEND_INV_CONST_ALPHA */
        0xC,
    };

    static const gctUINT32 xlateMode[] =
    {
        /* gcvBLEND_ADD */
        0x0,
        /* gcvBLEND_SUBTRACT */
        0x1,
        /* gcvBLEND_REVERSE_SUBTRACT */
        0x2,
        /* gcvBLEND_MIN */
        0x3,
        /* gcvBLEND_MAX */
        0x4,
    };
    gceSTATUS status;

    gctBOOL canEnableAlphaBlend;

    gcmHEADER_ARG("Hardware=0x%x FastFlushInfo=0x%x", Hardware, FastFlushInfo);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    {
        /* Define state buffer variables. */
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        /* Determine the size of the buffer to reserve. */
        floatPipe = Hardware->features[gcvFEATURE_HALF_FLOAT_PIPE];

        canEnableAlphaBlend = FastFlushInfo->blend;

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0508) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW_FAST(
            stateDelta, reserve, memory, gcvFALSE, 0x0508,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (xlateCompare[0]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:8) - (0 ?
 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
            );

        gcmSETSTATEDATA_NEW_FAST(
            stateDelta, reserve, memory, gcvFALSE, 0x0509,
            FastFlushInfo->color
            );

        gcmSETSTATEDATA_NEW_FAST(
            stateDelta, reserve, memory, gcvFALSE, 0x050A,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (canEnableAlphaBlend) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (canEnableAlphaBlend) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (xlateFuncSource[FastFlushInfo->srcFuncColor]) & ((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (xlateFuncSource[FastFlushInfo->srcFuncAlpha]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (xlateFuncTarget[FastFlushInfo->trgFuncColor]) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (xlateFuncTarget[FastFlushInfo->trgFuncAlpha]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (xlateMode[FastFlushInfo->modeColor]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (xlateMode[FastFlushInfo->modeAlpha]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ?
 30:28)))
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        if (floatPipe)
        {
            gctUINT32 colorLow;
            gctUINT32 colorHeigh;
            gctUINT16 colorR;
            gctUINT16 colorG;
            gctUINT16 colorB;
            gctUINT16 colorA;

            colorR = gcoMATH_UInt8AsFloat16((((((gctUINT32) (FastFlushInfo->color)) >> (0 ? 23:16)) & ((gctUINT32) ((((1 ? 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1)))))) ));
            colorG = gcoMATH_UInt8AsFloat16((((((gctUINT32) (FastFlushInfo->color)) >> (0 ? 15:8)) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1)))))) ));
            colorB = gcoMATH_UInt8AsFloat16((((((gctUINT32) (FastFlushInfo->color)) >> (0 ? 7:0)) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1)))))) ));
            colorA = gcoMATH_UInt8AsFloat16((((((gctUINT32) (FastFlushInfo->color)) >> (0 ? 31:24)) & ((gctUINT32) ((((1 ? 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1)))))) ));
            colorLow = colorR | (colorG << 16);
            colorHeigh = colorB | (colorA << 16);

            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052C) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW_FAST(
                stateDelta, reserve, memory, gcvFALSE, 0x052C,
                colorLow
                );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );

            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1 ) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x052D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            gcmSETSTATEDATA_NEW_FAST(
                stateDelta, reserve, memory, gcvFALSE, 0x052D,
                colorHeigh
                );

            gcmENDSTATEBATCH_NEW(
                reserve, memory
                );
        }

        /* Color conversion control. */
        if (FastFlushInfo->blend
            && (FastFlushInfo->srcFuncColor == gcvBLEND_SOURCE_ALPHA)
            && (FastFlushInfo->trgFuncColor == gcvBLEND_INV_SOURCE_ALPHA))
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
        else
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0529) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATAWITHMASK_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0529, (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10)))), (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) &  ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }

    /* Success */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FastProgramIndex(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory)
{
    gceSTATUS status;
    gcoBUFOBJ index = FastFlushInfo->bufObj;
    gctUINT32 indexAddress;
    gctUINT32 control;
    gctUINT32 indexEndian = 0x0;
    gctUINT32 indexFormat = 0x1;

    gcmHEADER_ARG("Hardware=0x%x FastFlushInfo=0x%x", Hardware, FastFlushInfo);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    {
        /* Define state buffer variables. */
        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

        if (index)
        {
            gcoBUFOBJ_FastLock(index, &indexAddress, gcvNULL);
            indexAddress += (gctUINT32)(gctUINTPTR_T)(FastFlushInfo->indices);
        }
        else
        {
            indexAddress = (gctUINT32)(gctUINTPTR_T)(FastFlushInfo->indices);
        }

        if (Hardware->bigEndian)
            indexEndian = 0x0;

        /* Compute control state value. */
        control = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (indexFormat) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (indexEndian) & ((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Hardware->FEStates->primitiveRestart) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8)));

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0191) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW_FAST(
            stateDelta, reserve, memory, gcvFALSE, 0x0191,
            indexAddress;
        );

        gcmSETSTATEDATA_NEW_FAST(
            stateDelta, reserve, memory, gcvFALSE, 0x0192,
            control
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Set the state. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x019D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x019D,
 Hardware->FEStates->restartElement);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
    }

    /* Success */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FastFlushUniforms(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory)
{
    gceSTATUS status;
    gctINT i = 0;

    gcmHEADER_ARG("Hardware=0x%x FastFlushInfo=0x%x", Hardware, FastFlushInfo);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (FastFlushInfo->programValid)
    {
        for (i = 0; i < FastFlushInfo->userDefUniformCount; ++i)
        {
            gcsFAST_FLUSH_UNIFORM *uniform = &FastFlushInfo->uniforms[i];

            if(uniform->dirty)
            {
                gctINT stage;

                for (stage = 0; stage < 2; ++stage)
                {
                    gcUNIFORM halUniform = uniform->halUniform[stage];

                    if (halUniform && isUniformUsedInShader(halUniform))
                    {
                        gctUINT col;
                        gctUINT32 address = uniform->physicalAddress[stage] >> 2;
                        gctUINT8_PTR pArray = (gctUINT8_PTR)uniform->data;

                        /* Define state buffer variables. */
                        gcmDEFINESTATEBUFFER_NEW_FAST(reserve, memory);

                        /* Reserve space in the command buffer. */
                        gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);

                        if (Hardware->unifiedConst && !Hardware->features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH])
                        {
                            gctUINT32 shaderConfigData = Hardware->SHStates->programState.hints ?
                                                         Hardware->SHStates->programState.hints->shaderConfigData : 0;

                            shaderConfigData = ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) ((halUniform->shaderKind != gcSHADER_TYPE_VERTEX)) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)));

                            {    {    gcmVERIFYLOADSTATEALIGNED(reserve,
 memory);    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0218) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW_FAST(stateDelta, reserve, memory, gcvFALSE, 0x0218,
 shaderConfigData);    gcmENDSTATEBATCH_NEW(reserve, memory);
};

                        }

                        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)uniform->columns <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (uniform->columns) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


                        /* Walk all columns. */
                        for (col = 0; col < uniform->columns; ++col)
                        {
                            gctUINT8_PTR pData = pArray + (col << 2);

                            gctUINT data = *(gctUINT_PTR)pData;

                            /* Set the state value. */
                            gcmSETSTATEDATA_NEW_FAST(stateDelta,
                                reserve,
                                memory,
                                gcvFALSE,
                                (address >> 2) + col,
                                data);
                        }

                        if ((col & 1) == 0)
                        {
                            /* Align. */
                            gcmSETFILLER_NEW(reserve, memory);
                        }

                        /* End the state batch. */
                        gcmENDSTATEBATCH_NEW(reserve, memory);

                        /* Next row. */
                        address += 4;

                        pArray += uniform->arrayStride;

                        /* Validate the state buffer. */
                        gcmENDSTATEBUFFER_NEW_FAST(Hardware, reserve, memory, Memory);
                    }
                }
            }
        }
    }

    /* Success */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status */
    gcmFOOTER();
    return status;
}

#define _gcmSETSTATEDATA_FAST(StateDelta, Memory, Address, Data) \
    do \
    { \
        gctUINT32 __temp_data32__ = Data; \
        \
        *Memory++ = __temp_data32__; \
        \
        gcmDUMPSTATEDATA(StateDelta, gcvFALSE, Address, __temp_data32__); \
        \
        Address += 1; \
    } \
    while (gcvFALSE)

gceSTATUS gcoHARDWARE_FastFlushStream(
    IN gcoHARDWARE Hardware,
    IN gcsFAST_FLUSH_PTR FastFlushInfo,
    INOUT gctPOINTER *Memory)
{
#define __GL_ONE_32 1

    gceSTATUS   status;
    gcsFAST_FLUSH_VERTEX_ARRAY* attArray;
    gctUINT vsInputArrayMask = FastFlushInfo->vsInputArrayMask;
    gctUINT attribMask = FastFlushInfo->attribMask;
    gctINT arrayIdx = -1;
    gctINT attribIdx = -1;
    gctUINT bitMask;
    gctUINT streamCount = 0;

    gctUINT i, stream;
    gctUINT attributeCount;
    gctUINT stride;
    gctUINT divisor;
    gctUINT linkState;
    gctUINT linkCount;
    gctBOOL halti2Support;

    gctUINT32 lastPhysical;
    gctUINT32 fetchSize;
    gctUINT32 format;
    gctUINT32 endian;
    gctUINT32 link;
    gctUINT32 size;
    gctUINT32 normalize;
    gctUINT32 fetchBreak;
    gctUINT32 base;

    gctSIZE_T vertexCtrlStateCount, vertexCtrlReserveCount;
    gctSIZE_T shaderCtrlStateCount, shaderCtrlReserveCount;
    gctSIZE_T streamAddressStateCount, streamAddressReserveCount;
    gctSIZE_T streamStrideStateCount, streamStrideReserveCount;
    gctSIZE_T streamDivisorStateCount, streamDivisorReserveCount;
    gctUINT32_PTR vertexCtrl;
    gctUINT32_PTR shaderCtrl;
    gctUINT32_PTR streamAddress;
    gctUINT32_PTR streamStride;
    gctUINT32_PTR streamDivisor;

    gctUINT vertexCtrlState;
    gctUINT shaderCtrlState;
    gctUINT streamAddressState;
    gctUINT streamStrideState;
    gctUINT streamDivisorState;
    gctINT VertexInstanceIdLinkage = -1;
    gctUINT bytes;
    gctSIZE_T reserveSize;
    gctUINT32 physical;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

        /* Determine hardware capabilities */
    halti2Support = Hardware->features[gcvFEATURE_HALTI2];

    /* Process streams */
    attributeCount = 0;

    /* compute vertex attribte count */
    while (vsInputArrayMask)
    {
        bitMask = (__GL_ONE_32 << ++arrayIdx);
        if (!(vsInputArrayMask & bitMask))
        {
            continue;
        }
        streamCount++;
        vsInputArrayMask &= ~bitMask;
    }

    /*
    if (streamCount == 0)
        return GL_FALSE;
    */
    gcmONERROR(!streamCount);

    while (attribMask)
    {
        bitMask = (__GL_ONE_32 << ++attribIdx);
        if (!(attribMask & bitMask))
        {
            continue;
        }
        attributeCount++;
        attribMask &= ~bitMask;
    }

    gcmONERROR(!attributeCount);
    /***************************************************************************
    ** Determine the counts and reserve size.
    */

    /* State counts. */
    vertexCtrlStateCount = attributeCount;

    shaderCtrlStateCount = gcmALIGN(attributeCount, 4) >> 2;

    /* Reserve counts. */
    vertexCtrlReserveCount = 1 + (vertexCtrlStateCount | 1);
    shaderCtrlReserveCount = 1 + (shaderCtrlStateCount | 1);

    /* Set initial state addresses. */
    vertexCtrlState = 0x0180;
    shaderCtrlState = 0x0208;

    reserveSize
        = (vertexCtrlReserveCount + shaderCtrlReserveCount)
        *  gcmSIZEOF(gctUINT32);

    /* State counts. */
    streamAddressStateCount = Hardware->mixedStreams
        ? streamCount
        : Hardware->config->streamCount;

    streamStrideStateCount  = streamCount;

    streamAddressReserveCount = 1 + (streamAddressStateCount | 1);
    streamStrideReserveCount  = 1 + (streamStrideStateCount  | 1);

    if (halti2Support)
    {
        streamDivisorStateCount = streamCount;
        streamDivisorReserveCount = 1 + (streamDivisorStateCount  | 1);

        /* Set initial state addresses. */
        streamAddressState = 0x5180;
        streamStrideState  = 0x5190;
        streamDivisorState = 0x51A0;
    }
    else if (Hardware->config->streamCount > 1)
    {
        streamDivisorStateCount = 0;
        streamDivisorReserveCount = 0;

        /* Set initial state addresses. */
        streamAddressState = 0x01A0;
        streamStrideState  = 0x01A8;
        streamDivisorState  = 0;
    }
    else
    {
        streamDivisorStateCount = 0;
        streamDivisorReserveCount = 0;

        /* Set initial state addresses. */
        streamAddressState = 0x0193;
        streamStrideState  = 0x0194;
        streamDivisorState  = 0;
    }

    /* Add stream states. */
    reserveSize
        += (streamAddressReserveCount + streamStrideReserveCount + streamDivisorReserveCount)
        *  gcmSIZEOF(gctUINT32);

    /***************************************************************************
    ** Reserve command buffer state and init state commands.
    */
    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    /* Update the number of the elements. */
    stateDelta->elementCount = streamCount;

    /* Determine buffer pointers. */
    vertexCtrl    = memory;
    shaderCtrl    = vertexCtrl + vertexCtrlReserveCount;
    streamAddress = shaderCtrl + shaderCtrlReserveCount;
    streamStride  = streamAddress + streamAddressReserveCount;
    streamDivisor = streamStride + streamStrideReserveCount;

    memory = (gctUINT32_PTR) ((gctUINT8_PTR) memory + reserveSize);

    /* Init load state commands. */
    *vertexCtrl++
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (vertexCtrlStateCount) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (vertexCtrlState) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    *shaderCtrl++
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (shaderCtrlStateCount) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (shaderCtrlState) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    *streamAddress++
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (streamAddressStateCount) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (streamAddressState) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    *streamStride++
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ?
 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (streamStrideStateCount) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (streamStrideState) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));

    if (halti2Support)
    {
        *streamDivisor++
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (streamDivisorStateCount) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (streamDivisorState) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));
    }

    lastPhysical = 0;
    linkCount = 0;
    linkState = 0;

    attribMask = FastFlushInfo->attribMask;

    stream = 0;
    attribIdx = -1;

    /* Program stream/attrib for every mask bit */
    while (attribMask)
    {
        bitMask = (__GL_ONE_32 << ++attribIdx);
        if (!(attribMask & bitMask))
        {
            continue;
        }
        attribMask &= ~bitMask;

        /* Check whether VS required attribute was enabled by apps */
        attArray = &FastFlushInfo->attribute[attribIdx];
        /* Get the internal format */
        format = gcvVERTEX_FLOAT;

        gcoBUFOBJ_FastLock(FastFlushInfo->boundObjInfo[attribIdx], &physical,gcvNULL);

        stride = attArray->stride;
        divisor = attArray->divisor;
        base    = (gctUINT32)(gctUINTPTR_T)attArray->pointer;

        lastPhysical = physical + base;

        /* Store the stream address. */
        _gcmSETSTATEDATA_FAST(stateDelta, streamAddress, streamAddressState, lastPhysical);

        if (halti2Support)
        {
            /* Store the stream stride. */
            _gcmSETSTATEDATA_FAST(
                stateDelta, streamStride, streamStrideState,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:0) - (0 ? 11:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:0) - (0 ? 11:0) + 1))))))) << (0 ?
 11:0))) | (((gctUINT32) ((gctUINT32) (stride) & ((gctUINT32) ((((1 ? 11:0) - (0 ?
 11:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:0) - (0 ? 11:0) + 1))))))) << (0 ?
 11:0)))
                );

            /* Store the stream divisor. */
            _gcmSETSTATEDATA_FAST(
                stateDelta, streamDivisor, streamDivisorState, divisor
                );
        }
        else
        {
            /* Store the stream stride. */
            _gcmSETSTATEDATA_FAST(
                stateDelta, streamStride, streamStrideState,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:0) - (0 ? 8:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:0) - (0 ? 8:0) + 1))))))) << (0 ?
 8:0))) | (((gctUINT32) ((gctUINT32) (stride) & ((gctUINT32) ((((1 ? 8:0) - (0 ?
 8:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:0) - (0 ? 8:0) + 1))))))) << (0 ?
 8:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (divisor) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
 31:16))))
                );
        }

        fetchSize = 0;

        /* every stream have one attribute */

        endian = Hardware->bigEndian
            ? 0x2
            : 0x0;
        bytes = attArray->size * 4;

        /* Get size. */
        size = attArray->size;
        link = FastFlushInfo->attributeLinkage[attribIdx];

        /* Get normalized flag. */
        normalize = 0x0;

        /* Get vertex offset and size. */
        fetchSize  = bytes;
        fetchBreak = 1;

        /* Store the current vertex element control value. */
        _gcmSETSTATEDATA_FAST(
            stateDelta, vertexCtrl, vertexCtrlState,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (size) & ((gctUINT32) ((((1 ? 13:12) - (0 ?
 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ?
 13:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (normalize) & ((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ?
 15:14)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (endian) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (stream) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:16) - (0 ?
 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ?
 23:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) (fetchSize) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ?
 31:24)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (fetchBreak) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))));

        /* Set vertex shader input linkage. */
        switch (linkCount & 3)
        {
        case 0:
            linkState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (link ) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
            break;
        case 1:
            linkState = ((((gctUINT32) (linkState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (link ) & ((gctUINT32) ((((1 ? 13:8) - (0 ?
 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8)));
            break;
        case 2:
            linkState = ((((gctUINT32) (linkState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (link ) & ((gctUINT32) ((((1 ? 21:16) - (0 ?
 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16)));
            break;
        case 3:
            linkState = ((((gctUINT32) (linkState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (link ) & ((gctUINT32) ((((1 ? 29:24) - (0 ?
 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24)));
            /* Store the current shader input control value. */
            _gcmSETSTATEDATA_FAST(
                stateDelta, shaderCtrl, shaderCtrlState, linkState
                );
            break;
        default:
            break;
        }

        /* Next vertex shader linkage. */
        ++linkCount;

        if (fetchBreak)
        {
            /* Reset fetch size on a break. */
            fetchSize = 0;
        }
        stream++;
    }

    /* Check if the IP requires all streams to be programmed. */
    if (!Hardware->mixedStreams)
    {
        for (i = streamCount; i < streamAddressStateCount; ++i)
        {
            /* Replicate the last physical address for unknown stream
            ** addresses. */
            _gcmSETSTATEDATA_FAST(
                stateDelta, streamAddress, streamAddressState,
                lastPhysical
                );
        }
    }

    /* See if there are any attributes left to program in the vertex shader
    ** shader input registers. And also check if we need to add vertex instance linkage
    */
    if (((linkCount & 3) != 0) || (VertexInstanceIdLinkage != -1))
    {
        /* Do we need to add vertex instance linkage? */
        if (VertexInstanceIdLinkage != -1)
        {
            /* Set vertex shader input linkage. */
            switch (linkCount & 3)
            {
            case 0:
                linkState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (VertexInstanceIdLinkage ) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
                break;
            case 1:
                linkState = ((((gctUINT32) (linkState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (VertexInstanceIdLinkage ) & ((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8)));
                break;
            case 2:
                linkState = ((((gctUINT32) (linkState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (VertexInstanceIdLinkage ) & ((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16)));
                break;
            case 3:
                linkState = ((((gctUINT32) (linkState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (VertexInstanceIdLinkage ) & ((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24)));
                break;
            default:
                break;
            }
        }

        /* Program attributes */
        _gcmSETSTATEDATA_FAST(
            stateDelta, shaderCtrl, shaderCtrlState, linkState
            );
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_ProgramIndex(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS   status;
    gctUINT32   control;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->FEDirty->indexDirty)
    {
        /* Compute control state value. */
        control = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (Hardware->FEStates->indexFormat) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (Hardware->FEStates->indexEndian) & ((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Hardware->FEStates->primitiveRestart) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8)));

        /* Reserve space in the command buffer. */
        gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)2 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0191) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            if (Hardware->FEDirty->indexHeadDirty)
            {
                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0191,
                    Hardware->FEStates->indexHeadAddress
                    );

                /* Mark index as updated. */
                Hardware->FEDirty->indexHeadDirty = gcvFALSE;
            }
            else if (Hardware->FEDirty->indexTailDirty)
            {
                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0191,
                    Hardware->FEStates->indexTailAddress
                    );

                Hardware->FEDirty->indexTailDirty = gcvFALSE;
            }
            else
            {
                gcmSETSTATEDATA_NEW(
                    stateDelta, reserve, memory, gcvFALSE, 0x0191,
                    Hardware->FEStates->indexHeadAddress
                    );
            }

            gcmSETSTATEDATA_NEW(
                stateDelta, reserve, memory, gcvFALSE, 0x0192,
                control
                );

            gcmSETFILLER_NEW(
                reserve, memory
                );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );

        /* Set the state. */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x019D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x019D, Hardware->FEStates->restartElement);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Validate the state buffer. */
        gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

        Hardware->FEDirty->indexDirty = Hardware->FEDirty->indexHeadDirty | Hardware->FEDirty->indexTailDirty;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_FlushPrefetchInst(
    IN gcoHARDWARE Hardware,
    IN OUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;
    gcsHINT_PTR hints = gcvNULL;

    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    hints = Hardware->SHStates->programState.hints;

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    for (i = 0; i < GC_ICACHE_PREFETCH_TABLE_SIZE; i++)
    {
        if (hints)
        {
            if (-1 != hints->vsICachePrefetch[i])
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0223) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0223, hints->vsICachePrefetch[i]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }

            if (-1 != hints->tcsICachePrefetch[i])
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x5283) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x5283, hints->tcsICachePrefetch[i]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }

            if (-1 != hints->tesICachePrefetch[i])
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x52C4) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x52C4, hints->tesICachePrefetch[i]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }

             if (-1 != hints->gsICachePrefetch[i])
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0446) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0446, hints->gsICachePrefetch[i]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

            }


            if (-1 != hints->fsICachePrefetch[i])
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0412) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0412, hints->fsICachePrefetch[i]);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            }
        }
    }

    /* Make compiler fine */
    stateDelta = stateDelta;

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);

OnError:
    /* Success. */
    gcmFOOTER_NO();
    return status;
}



gceSTATUS
gcoHARDWARE_EnableAlphaToCoverage(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT msaaFragmentOp;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    msaaFragmentOp = Hardware->MsaaStates->MsaaFragmentOp;

    if (Hardware->features[gcvFEATURE_MSAA_FRAGMENT_OPERATION])
    {
        gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
                0x01054,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 12:12) - (0 ?
 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 15:15) - (0 ?
 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15)))),
                (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (Enable) & ((gctUINT32) ((((1 ? 12:12) - (0 ?
 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ?
 12:12))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:15) - (0 ?
 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))))
                ));
    }

    if (Enable)
    {
        msaaFragmentOp |= gcvMSAA_FRAGOP_ALPHA_TO_COVERAGE;
    }
    else
    {
        msaaFragmentOp &= ~(gcvMSAA_FRAGOP_ALPHA_TO_COVERAGE);
    }

    if (msaaFragmentOp != Hardware->MsaaStates->MsaaFragmentOp)
    {
        Hardware->MsaaStates->MsaaFragmentOp = msaaFragmentOp;

        /* RAZ not support alphaToCoverage, switch to PEZ. */
        if(Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
        {
            Hardware->PEDirty->depthConfigDirty = gcvTRUE;
            Hardware->PEDirty->depthTargetDirty = gcvTRUE;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_EnableSampleCoverage(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT msaaFragmentOp;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    msaaFragmentOp = Hardware->MsaaStates->MsaaFragmentOp;

    if (Hardware->features[gcvFEATURE_MSAA_FRAGMENT_OPERATION])
    {
        gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
                0x01054,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 28:28) - (0 ?
 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)))),
                (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (Enable) & ((gctUINT32) ((((1 ? 28:28) - (0 ?
 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ?
 28:28))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))))
                ));
    }

    if (Enable)
    {
        msaaFragmentOp |= gcvMSAA_FRAGOP_SAMPLE_TO_COVERAGE;
    }
    else
    {
        msaaFragmentOp &= ~(gcvMSAA_FRAGOP_SAMPLE_TO_COVERAGE);
    }

    if (msaaFragmentOp != Hardware->MsaaStates->MsaaFragmentOp)
    {
        Hardware->MsaaStates->MsaaFragmentOp = msaaFragmentOp;

        /* RAZ not support sampleCoverage, switch to PEZ. */
        if(Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
        {
            Hardware->PEDirty->depthConfigDirty = gcvTRUE;
            Hardware->PEDirty->depthTargetDirty = gcvTRUE;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetSampleCoverageValue(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT CoverageValue,
    IN gctBOOL Invert
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x CoverageValue=%lf Invert=%d", Hardware, CoverageValue, Invert);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmVERIFY_ARGUMENT(CoverageValue >= 0.0f && CoverageValue <= 1.0f);

    if (Hardware->features[gcvFEATURE_MSAA_FRAGMENT_OPERATION])
    {
        gctUINT32 fixValue = (gctUINT32) CoverageValue * 16;

        gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
                0x01054,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:20) - (0 ? 24:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:20) - (0 ? 24:20) + 1))))))) << (0 ?
 24:20))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 24:20) - (0 ?
 24:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:20) - (0 ? 24:20) + 1))))))) << (0 ?
 24:20))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 27:27) - (0 ?
 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27)))),
                (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:20) - (0 ? 24:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:20) - (0 ? 24:20) + 1))))))) << (0 ?
 24:20))) | (((gctUINT32) ((gctUINT32) (fixValue) & ((gctUINT32) ((((1 ?
 24:20) - (0 ? 24:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:20) - (0 ? 24:20) + 1))))))) << (0 ?
 24:20))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:27) - (0 ?
 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27))))
                ));

        gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
                0x01054,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19)))),
                (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (Invert) & ((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19))))
                ));
    }

OnError:
    gcmFOOTER();

    return status;
}

gceSTATUS
gcoHARDWARE_EnableSampleMask(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT msaaFragmentOp;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    msaaFragmentOp = Hardware->MsaaStates->MsaaFragmentOp;

    if (Hardware->features[gcvFEATURE_MSAA_FRAGMENT_OPERATION])
    {
        gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
                0x01054,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 11:11) - (0 ?
 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11)))),
                (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Enable) & ((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:11) - (0 ?
 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))))
                ));
    }

    if (Enable)
    {
        msaaFragmentOp |= gcvMSAA_FRAGOP_SAMPLE_MASK;
    }
    else
    {
        msaaFragmentOp &= ~(gcvMSAA_FRAGOP_SAMPLE_MASK);
    }

    if (msaaFragmentOp != Hardware->MsaaStates->MsaaFragmentOp)
    {
        Hardware->MsaaStates->MsaaFragmentOp = msaaFragmentOp;

        /* RAZ not support sampleMask, switch to PEZ. */
        if(Hardware->features[gcvFEATURE_RA_DEPTH_WRITE])
        {
            Hardware->PEDirty->depthConfigDirty = gcvTRUE;
            Hardware->PEDirty->depthTargetDirty = gcvTRUE;
        }
    }

OnError:
    gcmFOOTER();

    return status;
}

gceSTATUS
gcoHARDWARE_SetSampleMask(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 SampleMask
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x SampleMask%d", Hardware, SampleMask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->features[gcvFEATURE_MSAA_FRAGMENT_OPERATION])
    {
        gcmONERROR(gcoHARDWARE_LoadState32WithMask(Hardware,
                0x01054,
                (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) |    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)))),
                (((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (SampleMask) & ((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))))
                ));
    }


OnError:
    gcmFOOTER();

    return status;
}


gceSTATUS
gcoHARDWARE_EnableSampleShading(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if(Hardware->MsaaStates->sampleShading != Enable)
    {
        Hardware->MsaaStates->sampleShading = Enable;
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
    }

OnError:
    gcmFOOTER();

    return status;
}

gceSTATUS
gcoHARDWARE_SetSampleShading(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable,
    IN gctBOOL IsSampleIn,
    IN gctFLOAT SampleShadingValue
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT sampleUsed = 0;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d SampleShadingValue=0x%x", Hardware, Enable, SampleShadingValue);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    sampleUsed = gcmMAX(gcmCEIL((SampleShadingValue * Hardware->MsaaStates->sampleInfo.product)), 1);

    if (Hardware->MsaaStates->sampleShadingByPS != Enable)
    {
        Hardware->MsaaStates->sampleShadingByPS = Enable;
        Hardware->MsaaDirty->msaaConfigDirty   = gcvTRUE;
    }

    if (Hardware->MsaaStates->isSampleIn != IsSampleIn)
    {
        Hardware->MsaaStates->isSampleIn = IsSampleIn;
        Hardware->MsaaDirty->msaaConfigDirty   = gcvTRUE;
    }

    if (Hardware->MsaaStates->sampleShadingValue != sampleUsed)
    {
        Hardware->MsaaStates->sampleShadingValue = sampleUsed;
        Hardware->MsaaDirty->msaaConfigDirty    = gcvTRUE;
    }

OnError:
    gcmFOOTER();

    return status;
}


gceSTATUS
gcoHARDWARE_SetMinSampleShadingValue(
    IN gcoHARDWARE Hardware,
    IN gctFLOAT MinSampleShadingValue
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT sampleUsed = 0;

    gcmHEADER_ARG("Hardware=0x%x MinSampleShadingValue=0x%x", Hardware, MinSampleShadingValue);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    sampleUsed = gcmMAX(gcmCEIL((MinSampleShadingValue * Hardware->MsaaStates->sampleInfo.product)), 1);

    if (Hardware->MsaaStates->minSampleShadingValue != sampleUsed)
    {
        Hardware->MsaaStates->minSampleShadingValue = sampleUsed;
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
    }

OnError:
    gcmFOOTER();

    return status;
}

gceSTATUS
gcoHARDWARE_EnableSampleMaskOut(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable,
    IN gctINT  SampleMaskLoc
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d SampleMaskLoc=0x%x", Hardware, Enable, SampleMaskLoc);

    gcmGETHARDWARE(Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->MsaaStates->sampleMaskOut != Enable)
    {
        Hardware->MsaaStates->sampleMaskOut = Enable;
        Hardware->SHDirty->shaderDirty  |= gcvPROGRAM_STAGE_FRAGMENT_BIT;
    }

    if (Hardware->MsaaStates->sampleMaskLoc != SampleMaskLoc)
    {
        Hardware->MsaaStates->sampleMaskLoc    = SampleMaskLoc;
        Hardware->PEDirty->depthConfigDirty = gcvTRUE;
        Hardware->PEDirty->depthTargetDirty = gcvTRUE;
    }

OnError:
    gcmFOOTER();

    return status;
}

gceSTATUS
gcoHARDWARE_SetXfbHeader(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 HeaderPhysical
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x HeaderPhysical=0x%x", Hardware, HeaderPhysical);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmASSERT(Hardware->features[gcvFEATURE_HW_TFB]);

    Hardware->XFBStates->headerPhysical = HeaderPhysical;

    Hardware->XFBDirty->s.headerDirty = gcvTRUE;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetXfbBuffer(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Index,
    IN gctUINT32 BufferAddr,
    IN gctUINT32 BufferStride,
    IN gctUINT32 BufferSize
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Index=%d BufferAddr=0x%x BufferStride=%d BufferSize=%d",
                   Hardware, Index, BufferAddr, BufferStride, BufferSize);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmVERIFY_ARGUMENT(Index < gcdMAX_XFB_BUFFERS);

    gcmASSERT(Hardware->features[gcvFEATURE_HW_TFB]);

    Hardware->XFBStates->buffer[Index].address = BufferAddr;
    Hardware->XFBStates->buffer[Index].stride = BufferStride;
    Hardware->XFBStates->buffer[Index].size = BufferSize;

    Hardware->XFBDirty->s.bufferDirty = gcvTRUE;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_SetXfbCmd(
    IN gcoHARDWARE Hardware,
    IN gceXfbCmd Cmd,
    IN gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Cmd=%d", Hardware, Cmd);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmASSERT(Hardware->features[gcvFEATURE_HW_TFB]);

    switch (Cmd)
    {
    case gcvXFBCMD_BEGIN:
        /* delay it to following draw */
        Hardware->XFBStates->cmd = Cmd;
        Hardware->XFBDirty->s.cmdDirty = gcvTRUE;
        break;

    case gcvXFBCMD_PAUSE:
        /* pause xfb in commit */
        if ((Hardware->XFBStates->statusInCmd == gcvXFB_Enabled || Hardware->XFBStates->internalXFB) && Memory)
        {
            gcoHARDWARE_LoadCtrlStateNEW(Hardware, 0x1C004,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))), Memory);

            Hardware->XFBStates->status =
            Hardware->XFBStates->statusInCmd = gcvXFB_Paused;
            Hardware->XFBStates->cmd = Cmd;
            Hardware->XFBDirty->s.cmdDirty = gcvTRUE;
        }
        /* pause xfb by application */
        else if (Memory == gcvNULL)
        {
            if (Hardware->XFBStates->status == gcvXFB_Enabled)
            {
                gcoHARDWARE_LoadCtrlStateNEW(Hardware, 0x1C004,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))), Memory);

                Hardware->XFBStates->status =
                Hardware->XFBStates->statusInCmd = gcvXFB_Paused;
            }
            Hardware->XFBStates->cmd = Cmd;
            Hardware->XFBDirty->s.cmdDirty = gcvTRUE;
        }
        break;

    case gcvXFBCMD_RESUME:
        /* delay it to following draw */
        Hardware->XFBStates->cmd = Cmd;
        Hardware->XFBDirty->s.cmdDirty = gcvTRUE;
        break;

    case gcvXFBCMD_END:
        if (Hardware->XFBStates->status == gcvXFB_Enabled)
        {
            gcoHARDWARE_LoadCtrlState(Hardware, 0x1C004,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))));
            Hardware->XFBStates->statusInCmd = gcvXFB_Disabled;
        }
        Hardware->XFBStates->status = gcvXFB_Disabled;
        Hardware->XFBStates->cmd = Cmd;
        Hardware->XFBDirty->s.cmdDirty = gcvTRUE;
        break;

    default:
        gcmASSERT(0);
        gcmPRINT("Invalid XFB command");
        break;
    }

OnError:
    gcmFOOTER();
    return status;

}

gceSTATUS
gcoHARDWARE_SetRasterDiscard(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Hardware->XFBStates->enableDiscard != Enable)
    {
        Hardware->XFBStates->enableDiscard = Enable;
        Hardware->XFBDirty->s.discardDirty = gcvTRUE;
    }

OnError:
    gcmFOOTER();
    return status;

}

gceSTATUS
gcoHARDWARE_FlushXfb(
    IN gcoHARDWARE Hardware,
    INOUT gctPOINTER *Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmASSERT(Hardware->features[gcvFEATURE_HW_TFB]);

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, Memory);

    if (Hardware->XFBDirty->s.headerDirty)
    {
        gcmASSERT((Hardware->XFBStates->status != gcvXFB_Enabled) || Hardware->XFBStates->internalXFB);

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7002) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7002, Hardware->XFBStates->headerPhysical);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (Hardware->XFBDirty->s.bufferDirty)
    {
        gctINT i;

        gcmASSERT(Hardware->XFBStates->status != gcvXFB_Enabled);

        for (i = 0; i < gcdMAX_XFB_BUFFERS; i++)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7010 + i) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7010 + i,
 Hardware->XFBStates->buffer[i].address);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7030 + i) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7030 + i,
 Hardware->XFBStates->buffer[i].stride);    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7020 + i) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7020 + i,
 Hardware->XFBStates->buffer[i].size);    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }
    }


    if (Hardware->XFBDirty->s.cmdDirty)
    {
        switch (Hardware->XFBStates->cmd)
        {
        case gcvXFBCMD_BEGIN:
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7001) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7001, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                Hardware->XFBStates->status = gcvXFB_Enabled;

                if (Hardware->XFBStates->internalXFB)
                {
                    Hardware->XFBStates->internalXFB = gcvFALSE;
                    gcmASSERT(Hardware->XFBDirty->s.bufferDirty);
                }
            }
            break;

        case gcvXFBCMD_RESUME:
            if (Hardware->XFBStates->status == gcvXFB_Disabled)
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7001) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7001, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                Hardware->XFBStates->status = gcvXFB_Enabled;

            }
            else if (Hardware->XFBStates->status == gcvXFB_Paused)
            {
                {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7001) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7001, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


                Hardware->XFBStates->status = gcvXFB_Enabled;
            }
            break;
        default:
            break;
        }
    }

    if (Hardware->XFBDirty->s.discardDirty)
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7000) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7000, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (Hardware->XFBStates->enableDiscard ?
 0x1 : 0x0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ?
 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    if (Hardware->XFBDirty->s.internalXFBDirty && Hardware->QUERYStates->queryStatus[gcvQUERY_PRIM_GENERATED] == gcvQUERY_Enabled)
    {
        if (Hardware->XFBStates->status != gcvXFB_Enabled)
        {
            gctUINT32  physical;

            if (gcvNULL == Hardware->XFBStates->internalXFBNode)
            {
                gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcsSURF_NODE), (gctPOINTER *)&Hardware->XFBStates->internalXFBNode);
                gcoOS_ZeroMemory((gctPOINTER)Hardware->XFBStates->internalXFBNode, gcmSIZEOF(gcsSURF_NODE));
                gcmONERROR(gcsSURF_NODE_Construct(Hardware->XFBStates->internalXFBNode,
                                                  64,
                                                  64,
                                                  gcvSURF_TFBHEADER,
                                                  0,
                                                  gcvPOOL_DEFAULT
                                                  ));

                gcmONERROR(gcoSURF_LockNode(Hardware->XFBStates->internalXFBNode, gcvNULL, &Hardware->XFBStates->internalXFBLocked));

                gcoOS_ZeroMemory(Hardware->XFBStates->internalXFBLocked, 64);

                gcmASSERT(Hardware->XFBStates->internalXFBLocked);

            }

            gcmGETHARDWAREADDRESSP(Hardware->XFBStates->internalXFBNode, physical);

            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7002) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7002, physical);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7020) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7020, 0);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7030) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x7030, 16);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7001) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7001, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            Hardware->XFBStates->internalXFB = gcvTRUE;
        }
    }
    else if (Hardware->QUERYStates->queryStatus[gcvQUERY_PRIM_GENERATED] != gcvQUERY_Enabled)
    {
        if (Hardware->XFBStates->internalXFB)
        {
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ?
 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7001) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x7001, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


            Hardware->XFBStates->internalXFB = gcvFALSE;
        }
    }

    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, Memory);


    Hardware->XFBDirty->xfbDirty = 0;

OnError:

    gcmFOOTER();
    return status;
}

#endif


