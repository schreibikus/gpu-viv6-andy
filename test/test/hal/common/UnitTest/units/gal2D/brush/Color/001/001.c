/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


/*
 *  Feature:    ColorBrush - different color
 *  API:        gco2D_ConstructColorBrush()
 *  Check:      different color
*/
#include <galUtil.h>

static gctCONST_STRING s_CaseDescription = \
"Case gal2DColorBrush001\n" \
"Operation: Test blit the screen with ColorBrush of different color.\n" \
"2D API: gco2D_ConstructColorBrush\n" \
"Src: [None]\n"\
"Dst: Size        [configurable]\n"\
"     Rect        [configurable]\n"\
"     Format      [configurable]\n"\
"     Rotation    [0]\n"\
"     Tile        [linear]\n"\
"     Compression [None]\n" \
"Brush: Type   [ColorBrush]\n"\
"       Format [ARGB8888]\n"\
"       Offset [0]\n" \
"Alphablend: [disable]\n" \
"HW feature dependency: ";

typedef struct Test2D {
    GalTest     base;
    GalRuntime  *runtime;

    // dst
    gcoSURF            dstSurf;
    gceSURF_FORMAT    dstFormat;
    gctUINT            dstWidth;
    gctUINT            dstHeight;
    gctINT            dstStride;
    gctUINT32        dstPhyAddr;
    gctPOINTER        dstLgcAddr;

    //brush
    gctUINT8 brushAlpha;
    gctUINT8 brushRed;
    gctUINT8 brushGreen;
    gctUINT8 brushBlue;
} Test2D;

static void CDECL FillColor(Test2D *t2d, gctUINT32 *ColorBuf)
{
    int i, j;

    for (i = 0; i < 8; ++i)
    {
        if (i < 4)
        {
            for (j = 0; j < 8; ++j)
            {
                if (j < 4)
                    ColorBuf[j*8+i] =
                    COLOR_ARGB8(t2d->brushAlpha, t2d->brushRed, t2d->brushGreen, t2d->brushBlue);
                else
                    ColorBuf[j*8+i] =
                    COLOR_ARGB8(t2d->brushAlpha, t2d->brushRed, t2d->brushGreen, ~t2d->brushBlue);
            }
        }
        else
        {
            for (j = 0; j < 8; ++j)
            {
                if (j < 4)
                    ColorBuf[j*8+i] =
                    COLOR_ARGB8(t2d->brushAlpha, t2d->brushRed, ~t2d->brushGreen, t2d->brushBlue);
                else
                    ColorBuf[j*8+i] =
                    COLOR_ARGB8(t2d->brushAlpha, ~t2d->brushRed, t2d->brushGreen, t2d->brushBlue);
            }
        }
    }
}

static gctBOOL CDECL Render(Test2D *t2d, gctUINT frameNo)
{
    gcsRECT dstRect = {0, 0, t2d->dstWidth, t2d->dstHeight};
    gco2D egn2D = t2d->runtime->engine2d;
    gcoBRUSH brush;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 color[64];

    gcmONERROR(gco2D_SetClipping(egn2D, &dstRect));

    FillColor(t2d, color);

    gcmONERROR(gco2D_ConstructColorBrush(egn2D, 0, 0,
                color, gcvSURF_A8R8G8B8, 0, &brush));

    gcmONERROR(gco2D_FlushBrush(egn2D, brush, t2d->dstFormat));

    gcmONERROR(gco2D_SetTarget(egn2D, t2d->dstPhyAddr, t2d->dstStride, gcvSURF_0_DEGREE, t2d->dstWidth));

    gcmONERROR(gco2D_Blit(egn2D, 1, &dstRect, 0xF0, 0xF0, t2d->dstFormat));

    gcmONERROR(gco2D_Flush(egn2D));

    gcmONERROR(gcoHAL_Commit(t2d->runtime->hal, gcvTRUE));

    gcmONERROR(gcoBRUSH_Destroy(brush));

    /* Cycle to the next color. */
    if (t2d->brushBlue == 0)
    {
        t2d->brushBlue = ~0;
    }
    else
    {
        t2d->brushBlue = 0;

        if (t2d->brushGreen == 0)
        {
            t2d->brushGreen = ~0;
        }
        else
        {
            t2d->brushGreen = 0;

            if (t2d->brushRed == 0)
            {
                t2d->brushRed = ~0;
            }
            else
            {
                t2d->brushRed = 0;
            }
        }
    }

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

static void CDECL Destroy(Test2D *t2d)
{
    gceSTATUS status = gcvSTATUS_OK;
    if ((t2d->dstSurf != gcvNULL) && (t2d->dstLgcAddr != gcvNULL))
    {
        if (gcmIS_ERROR(gcoSURF_Unlock(t2d->dstSurf, t2d->dstLgcAddr)))
        {
            GalOutput(GalOutputType_Error | GalOutputType_Console, "Unlock dstSurf failed:%s\n", GalStatusString(status));
        }
        t2d->dstLgcAddr = gcvNULL;
    }

    free(t2d);
}

const gceFEATURE FeatureList[]=
{
    gcvFEATURE_2D_NO_COLORBRUSH_INDEX8,
};

static gctBOOL CDECL Init(Test2D *t2d, GalRuntime *runtime)
{
    gceSTATUS status = gcvSTATUS_OK;
    int i;
    gctINT argc = runtime->argc;
    gctSTRING *argv = runtime->argv;

    gctUINT32 k, listLen = sizeof(FeatureList)/sizeof(gctINT);
    gctBOOL featureStatus;
    char featureName[FEATURE_NAME_LEN], featureMsg[FEATURE_MSG_LEN];

    runtime->wholeDescription = (char*)malloc(FEATURE_NAME_LEN * listLen + strlen(s_CaseDescription) + 1);

    if (runtime->wholeDescription == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    memcpy(runtime->wholeDescription, s_CaseDescription, strlen(s_CaseDescription) + 1);

    for(k = 0; k < listLen; k++)
    {
        gcmONERROR(GalQueryFeatureStr(FeatureList[k], featureName, featureMsg, &featureStatus));
        if (gcoHAL_IsFeatureAvailable(runtime->hal, FeatureList[k]) == featureStatus)
        {
            GalOutput(GalOutputType_Result | GalOutputType_Console, "%s is not supported.\n", featureMsg);
            runtime->notSupport = gcvTRUE;
        }
        strncat(runtime->wholeDescription, featureName, k==listLen-1 ? strlen(featureName)+1:strlen(featureName));
    }

    if (runtime->notSupport)
        return gcvFALSE;

    t2d->runtime = runtime;

    t2d->dstSurf    = runtime->target;
    t2d->dstFormat = runtime->format;
    t2d->dstWidth = 0;
    t2d->dstHeight = 0;
    t2d->dstStride = 0;
    t2d->dstPhyAddr = 0;
    t2d->dstLgcAddr = 0;

    t2d->brushAlpha = 0;
    t2d->brushRed = 0;
    t2d->brushGreen = 0;
    t2d->brushBlue = 0xff;

    gcmONERROR(gcoSURF_GetAlignedSize(t2d->dstSurf,
                                        &t2d->dstWidth,
                                        &t2d->dstHeight,
                                        &t2d->dstStride));

    gcmONERROR(gcoSURF_Lock(t2d->dstSurf, &t2d->dstPhyAddr, &t2d->dstLgcAddr));

    t2d->base.render     = (PGalRender)Render;
    t2d->base.destroy    = (PGalDestroy)Destroy;
    t2d->base.frameCount = 8;
    t2d->base.description = s_CaseDescription;

    for (i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-n"))
            t2d->base.frameCount = atoi(argv[++i]);
    }

    return gcvTRUE;

OnError:
    GalOutput(GalOutputType_Error | GalOutputType_Console,
        "%s(%d) failed:%s\n",__FUNCTION__, __LINE__, gcoOS_DebugStatus2Name(status));
    return gcvFALSE;
}

GalTest * CDECL GalCreateTestObject(GalRuntime *runtime)
{
    Test2D *t2d = (Test2D *)malloc(sizeof(Test2D));

    if (!Init(t2d, runtime)) {
        free(t2d);
        return NULL;
    }

    return &t2d->base;
}
