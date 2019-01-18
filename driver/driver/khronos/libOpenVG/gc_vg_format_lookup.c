/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*******************************************************************************
** Table access macros.
*/

#define vgmCHANNELMASK(Format, Mask) \
    _ ## Format ## _channelMask[Mask]

#define vgmZERO2ONE(Value, BitWidth) \
    * ((gctFLOAT_PTR) & _zero2one_ ## BitWidth ## bit [Value])

#define vgmZERO2ONE_L2S(Value, BitWidth) \
    * ((gctFLOAT_PTR) & _zero2one_l2s_ ## BitWidth ## bit [Value])

#define vgmZERO2ONE_S2L(Value, BitWidth) \
    * ((gctFLOAT_PTR) & _zero2one_s2l_ ## BitWidth ## bit [Value])


/*******************************************************************************
** Format-dependent channel mask lookup tables.
*/

static gctUINT32 _sRGBX_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0xFFFF00FF,
    /* xxBA */  0xFFFF00FF,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FFFF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF0000FF,
    /* Rxxx */  0x00FFFFFF,
    /* RxxA */  0x00FFFFFF,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF00FF,
    /* RGxx */  0x0000FFFF,
    /* RGxA */  0x0000FFFF,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x000000FF
};

static gctUINT32 _sRGBA_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0xFFFF00FF,
    /* xxBA */  0xFFFF0000,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF000000,
    /* Rxxx */  0x00FFFFFF,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0x0000FFFF,
    /* RGxA */  0x0000FF00,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT32 _sRGBA_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0xFFFF00FF,
    /* xxBA */  0xFFFF0000,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF000000,
    /* Rxxx */  0x00FFFFFF,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0x0000FFFF,
    /* RGxA */  0x0000FF00,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT16 _sRGB_565_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFFF,
    /* xxBx */  0xFFE0,
    /* xxBA */  0xFFE0,
    /* xGxx */  0xF81F,
    /* xGxA */  0xF81F,
    /* xGBx */  0xF800,
    /* xGBA */  0xF800,
    /* Rxxx */  0x07FF,
    /* RxxA */  0x07FF,
    /* RxBx */  0x07E0,
    /* RxBA */  0x07E0,
    /* RGxx */  0x001F,
    /* RGxA */  0x001F,
    /* RGBx */  0x0000,
    /* RGBA */  0x0000
};

static gctUINT16 _sRGBA_5551_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFFE,
    /* xxBx */  0xFFC1,
    /* xxBA */  0xFFC0,
    /* xGxx */  0xF83F,
    /* xGxA */  0xF83E,
    /* xGBx */  0xF801,
    /* xGBA */  0xF800,
    /* Rxxx */  0x07FF,
    /* RxxA */  0x07FE,
    /* RxBx */  0x07C1,
    /* RxBA */  0x07C0,
    /* RGxx */  0x003F,
    /* RGxA */  0x003E,
    /* RGBx */  0x0001,
    /* RGBA */  0x0000
};

static gctUINT16 _sRGBA_4444_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFF0,
    /* xxBx */  0xFF0F,
    /* xxBA */  0xFF00,
    /* xGxx */  0xF0FF,
    /* xGxA */  0xF0F0,
    /* xGBx */  0xF00F,
    /* xGBA */  0xF000,
    /* Rxxx */  0x0FFF,
    /* RxxA */  0x0FF0,
    /* RxBx */  0x0F0F,
    /* RxBA */  0x0F00,
    /* RGxx */  0x00FF,
    /* RGxA */  0x00F0,
    /* RGBx */  0x000F,
    /* RGBA */  0x0000
};

static gctUINT32 _lRGBX_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0xFFFF00FF,
    /* xxBA */  0xFFFF00FF,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FFFF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF0000FF,
    /* Rxxx */  0x00FFFFFF,
    /* RxxA */  0x00FFFFFF,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF00FF,
    /* RGxx */  0x0000FFFF,
    /* RGxA */  0x0000FFFF,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x000000FF
};

static gctUINT32 _lRGBA_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0xFFFF00FF,
    /* xxBA */  0xFFFF0000,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF000000,
    /* Rxxx */  0x00FFFFFF,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0x0000FFFF,
    /* RGxA */  0x0000FF00,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT32 _lRGBA_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0xFFFF00FF,
    /* xxBA */  0xFFFF0000,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF000000,
    /* Rxxx */  0x00FFFFFF,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0x0000FFFF,
    /* RGxA */  0x0000FF00,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT16 _lRGB_565_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFFF,
    /* xxBx */  0xFFE0,
    /* xxBA */  0xFFE0,
    /* xGxx */  0xF81F,
    /* xGxA */  0xF81F,
    /* xGBx */  0xF800,
    /* xGBA */  0xF800,
    /* Rxxx */  0x07FF,
    /* RxxA */  0x07FF,
    /* RxBx */  0x07E0,
    /* RxBA */  0x07E0,
    /* RGxx */  0x001F,
    /* RGxA */  0x001F,
    /* RGBx */  0x0000,
    /* RGBA */  0x0000
};

static gctUINT32 _sXRGB_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0xFFFFFF00,
    /* xxBA */  0xFFFFFF00,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0xFFFF00FF,
    /* xGBx */  0xFFFF0000,
    /* xGBA */  0xFFFF0000,
    /* Rxxx */  0xFF00FFFF,
    /* RxxA */  0xFF00FFFF,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0xFF00FF00,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF0000FF,
    /* RGBx */  0xFF000000,
    /* RGBA */  0xFF000000
};

static gctUINT32 _sARGB_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFFFFFF00,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFFFF0000,
    /* xGBA */  0x00FF0000,
    /* Rxxx */  0xFF00FFFF,
    /* RxxA */  0x0000FFFF,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0x000000FF,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT32 _sARGB_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFFFFFF00,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFFFF0000,
    /* xGBA */  0x00FF0000,
    /* Rxxx */  0xFF00FFFF,
    /* RxxA */  0x0000FFFF,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0x000000FF,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT16 _sARGB_1555_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0x7FFF,
    /* xxBx */  0xFFE0,
    /* xxBA */  0x7FE0,
    /* xGxx */  0xFC1F,
    /* xGxA */  0x7C1F,
    /* xGBx */  0xFC00,
    /* xGBA */  0x7C00,
    /* Rxxx */  0x83FF,
    /* RxxA */  0x03FF,
    /* RxBx */  0x83E0,
    /* RxBA */  0x03E0,
    /* RGxx */  0x801F,
    /* RGxA */  0x001F,
    /* RGBx */  0x8000,
    /* RGBA */  0x0000
};

static gctUINT16 _sARGB_4444_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0x0FFF,
    /* xxBx */  0xFFF0,
    /* xxBA */  0x0FF0,
    /* xGxx */  0xFF0F,
    /* xGxA */  0x0F0F,
    /* xGBx */  0xFF00,
    /* xGBA */  0x0F00,
    /* Rxxx */  0xF0FF,
    /* RxxA */  0x00FF,
    /* RxBx */  0xF0F0,
    /* RxBA */  0x00F0,
    /* RGxx */  0xF00F,
    /* RGxA */  0x000F,
    /* RGBx */  0xF000,
    /* RGBA */  0x0000
};

static gctUINT32 _lXRGB_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0xFFFFFF00,
    /* xxBA */  0xFFFFFF00,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0xFFFF00FF,
    /* xGBx */  0xFFFF0000,
    /* xGBA */  0xFFFF0000,
    /* Rxxx */  0xFF00FFFF,
    /* RxxA */  0xFF00FFFF,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0xFF00FF00,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF0000FF,
    /* RGBx */  0xFF000000,
    /* RGBA */  0xFF000000
};

static gctUINT32 _lARGB_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFFFFFF00,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFFFF0000,
    /* xGBA */  0x00FF0000,
    /* Rxxx */  0xFF00FFFF,
    /* RxxA */  0x0000FFFF,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0x000000FF,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT32 _lARGB_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFFFFFF00,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFFFF0000,
    /* xGBA */  0x00FF0000,
    /* Rxxx */  0xFF00FFFF,
    /* RxxA */  0x0000FFFF,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0x000000FF,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT32 _sBGRX_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0x00FFFFFF,
    /* xxBA */  0x00FFFFFF,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FFFF,
    /* xGBx */  0x0000FFFF,
    /* xGBA */  0x0000FFFF,
    /* Rxxx */  0xFFFF00FF,
    /* RxxA */  0xFFFF00FF,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF00FF,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF0000FF,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x000000FF
};

static gctUINT32 _sBGRA_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0x00FFFFFF,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0x0000FFFF,
    /* xGBA */  0x0000FF00,
    /* Rxxx */  0xFFFF00FF,
    /* RxxA */  0xFFFF0000,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF000000,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT32 _sBGRA_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0x00FFFFFF,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0x0000FFFF,
    /* xGBA */  0x0000FF00,
    /* Rxxx */  0xFFFF00FF,
    /* RxxA */  0xFFFF0000,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF000000,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT16 _sBGR_565_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFFF,
    /* xxBx */  0x07FF,
    /* xxBA */  0x07FF,
    /* xGxx */  0xF81F,
    /* xGxA */  0xF81F,
    /* xGBx */  0x001F,
    /* xGBA */  0x001F,
    /* Rxxx */  0xFFE0,
    /* RxxA */  0xFFE0,
    /* RxBx */  0x07E0,
    /* RxBA */  0x07E0,
    /* RGxx */  0xF800,
    /* RGxA */  0xF800,
    /* RGBx */  0x0000,
    /* RGBA */  0x0000
};

static gctUINT16 _sBGRA_5551_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFFE,
    /* xxBx */  0x07FF,
    /* xxBA */  0x07FE,
    /* xGxx */  0xF83F,
    /* xGxA */  0xF83E,
    /* xGBx */  0x003F,
    /* xGBA */  0x003E,
    /* Rxxx */  0xFFC1,
    /* RxxA */  0xFFC0,
    /* RxBx */  0x07C1,
    /* RxBA */  0x07C0,
    /* RGxx */  0xF801,
    /* RGxA */  0xF800,
    /* RGBx */  0x0001,
    /* RGBA */  0x0000
};

static gctUINT16 _sBGRA_4444_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0xFFF0,
    /* xxBx */  0x0FFF,
    /* xxBA */  0x0FF0,
    /* xGxx */  0xF0FF,
    /* xGxA */  0xF0F0,
    /* xGBx */  0x00FF,
    /* xGBA */  0x00F0,
    /* Rxxx */  0xFF0F,
    /* RxxA */  0xFF00,
    /* RxBx */  0x0F0F,
    /* RxBA */  0x0F00,
    /* RGxx */  0xF00F,
    /* RGxA */  0xF000,
    /* RGBx */  0x000F,
    /* RGBA */  0x0000
};

static gctUINT32 _lBGRX_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0x00FFFFFF,
    /* xxBA */  0x00FFFFFF,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FFFF,
    /* xGBx */  0x0000FFFF,
    /* xGBA */  0x0000FFFF,
    /* Rxxx */  0xFFFF00FF,
    /* RxxA */  0xFFFF00FF,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF00FF,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF0000FF,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x000000FF
};

static gctUINT32 _lBGRA_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0x00FFFFFF,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0x0000FFFF,
    /* xGBA */  0x0000FF00,
    /* Rxxx */  0xFFFF00FF,
    /* RxxA */  0xFFFF0000,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF000000,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT32 _lBGRA_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFF00,
    /* xxBx */  0x00FFFFFF,
    /* xxBA */  0x00FFFF00,
    /* xGxx */  0xFF00FFFF,
    /* xGxA */  0xFF00FF00,
    /* xGBx */  0x0000FFFF,
    /* xGBA */  0x0000FF00,
    /* Rxxx */  0xFFFF00FF,
    /* RxxA */  0xFFFF0000,
    /* RxBx */  0x00FF00FF,
    /* RxBA */  0x00FF0000,
    /* RGxx */  0xFF0000FF,
    /* RGxA */  0xFF000000,
    /* RGBx */  0x000000FF,
    /* RGBA */  0x00000000
};

static gctUINT32 _sXBGR_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0xFF00FFFF,
    /* xxBA */  0xFF00FFFF,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0xFFFF00FF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF0000FF,
    /* Rxxx */  0xFFFFFF00,
    /* RxxA */  0xFFFFFF00,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0xFF00FF00,
    /* RGxx */  0xFFFF0000,
    /* RGxA */  0xFFFF0000,
    /* RGBx */  0xFF000000,
    /* RGBA */  0xFF000000
};

static gctUINT32 _sABGR_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFF00FFFF,
    /* xxBA */  0x0000FFFF,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0x000000FF,
    /* Rxxx */  0xFFFFFF00,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFFFF0000,
    /* RGxA */  0x00FF0000,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT32 _sABGR_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFF00FFFF,
    /* xxBA */  0x0000FFFF,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0x000000FF,
    /* Rxxx */  0xFFFFFF00,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFFFF0000,
    /* RGxA */  0x00FF0000,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT16 _sABGR_1555_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0x7FFF,
    /* xxBx */  0x83FF,
    /* xxBA */  0x03FF,
    /* xGxx */  0xFC1F,
    /* xGxA */  0x7C1F,
    /* xGBx */  0x801F,
    /* xGBA */  0x001F,
    /* Rxxx */  0xFFE0,
    /* RxxA */  0x7FE0,
    /* RxBx */  0x83E0,
    /* RxBA */  0x03E0,
    /* RGxx */  0xFC00,
    /* RGxA */  0x7C00,
    /* RGBx */  0x8000,
    /* RGBA */  0x0000
};

static gctUINT16 _sABGR_4444_channelMask[16] =
{
    /* xxxx */  0xFFFF,
    /* xxxA */  0x0FFF,
    /* xxBx */  0xF0FF,
    /* xxBA */  0x00FF,
    /* xGxx */  0xFF0F,
    /* xGxA */  0x0F0F,
    /* xGBx */  0xF00F,
    /* xGBA */  0x000F,
    /* Rxxx */  0xFFF0,
    /* RxxA */  0x0FF0,
    /* RxBx */  0xF0F0,
    /* RxBA */  0x00F0,
    /* RGxx */  0xFF00,
    /* RGxA */  0x0F00,
    /* RGBx */  0xF000,
    /* RGBA */  0x0000
};

static gctUINT32 _lXBGR_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0xFFFFFFFF,
    /* xxBx */  0xFF00FFFF,
    /* xxBA */  0xFF00FFFF,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0xFFFF00FF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0xFF0000FF,
    /* Rxxx */  0xFFFFFF00,
    /* RxxA */  0xFFFFFF00,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0xFF00FF00,
    /* RGxx */  0xFFFF0000,
    /* RGxA */  0xFFFF0000,
    /* RGBx */  0xFF000000,
    /* RGBA */  0xFF000000
};

static gctUINT32 _lABGR_8888_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFF00FFFF,
    /* xxBA */  0x0000FFFF,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0x000000FF,
    /* Rxxx */  0xFFFFFF00,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFFFF0000,
    /* RGxA */  0x00FF0000,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};

static gctUINT32 _lABGR_8888_PRE_channelMask[16] =
{
    /* xxxx */  0xFFFFFFFF,
    /* xxxA */  0x00FFFFFF,
    /* xxBx */  0xFF00FFFF,
    /* xxBA */  0x0000FFFF,
    /* xGxx */  0xFFFF00FF,
    /* xGxA */  0x00FF00FF,
    /* xGBx */  0xFF0000FF,
    /* xGBA */  0x000000FF,
    /* Rxxx */  0xFFFFFF00,
    /* RxxA */  0x00FFFF00,
    /* RxBx */  0xFF00FF00,
    /* RxBA */  0x0000FF00,
    /* RGxx */  0xFFFF0000,
    /* RGxA */  0x00FF0000,
    /* RGBx */  0xFF000000,
    /* RGBA */  0x00000000
};


/*******************************************************************************
**
** Color conversion look-up tables:
**   1. Convert to [0..1] range.
**
** The tables are in 32-bit floating point binary format.
*/

static gctUINT32 _zero2one_1bit[2] =
{
    0x00000000, 0x3F800000
};

static gctUINT32 _zero2one_4bit[16] =
{
    0x00000000, 0x3D888889, 0x3E088889, 0x3E4CCCCD,
    0x3E888889, 0x3EAAAAAB, 0x3ECCCCCD, 0x3EEEEEEF,
    0x3F088889, 0x3F19999A, 0x3F2AAAAB, 0x3F3BBBBC,
    0x3F4CCCCD, 0x3F5DDDDE, 0x3F6EEEEF, 0x3F800000
};

static gctUINT32 _zero2one_5bit[32] =
{
    0x00000000, 0x3D042108, 0x3D842108, 0x3DC6318C,
    0x3E042108, 0x3E25294A, 0x3E46318C, 0x3E6739CE,
    0x3E842108, 0x3E94A529, 0x3EA5294A, 0x3EB5AD6B,
    0x3EC6318C, 0x3ED6B5AD, 0x3EE739CE, 0x3EF7BDEF,
    0x3F042108, 0x3F0C6319, 0x3F14A529, 0x3F1CE73A,
    0x3F25294A, 0x3F2D6B5B, 0x3F35AD6B, 0x3F3DEF7C,
    0x3F46318C, 0x3F4E739D, 0x3F56B5AD, 0x3F5EF7BE,
    0x3F6739CE, 0x3F6F7BDF, 0x3F77BDEF, 0x3F800000
};

static gctUINT32 _zero2one_6bit[64] =
{
    0x00000000, 0x3C820821, 0x3D020821, 0x3D430C31,
    0x3D820821, 0x3DA28A29, 0x3DC30C31, 0x3DE38E39,
    0x3E020821, 0x3E124925, 0x3E228A29, 0x3E32CB2D,
    0x3E430C31, 0x3E534D35, 0x3E638E39, 0x3E73CF3D,
    0x3E820821, 0x3E8A28A3, 0x3E924925, 0x3E9A69A7,
    0x3EA28A29, 0x3EAAAAAB, 0x3EB2CB2D, 0x3EBAEBAF,
    0x3EC30C31, 0x3ECB2CB3, 0x3ED34D35, 0x3EDB6DB7,
    0x3EE38E39, 0x3EEBAEBB, 0x3EF3CF3D, 0x3EFBEFBF,
    0x3F020821, 0x3F061862, 0x3F0A28A3, 0x3F0E38E4,
    0x3F124925, 0x3F165966, 0x3F1A69A7, 0x3F1E79E8,
    0x3F228A29, 0x3F269A6A, 0x3F2AAAAB, 0x3F2EBAEC,
    0x3F32CB2D, 0x3F36DB6E, 0x3F3AEBAF, 0x3F3EFBF0,
    0x3F430C31, 0x3F471C72, 0x3F4B2CB3, 0x3F4F3CF4,
    0x3F534D35, 0x3F575D76, 0x3F5B6DB7, 0x3F5F7DF8,
    0x3F638E39, 0x3F679E7A, 0x3F6BAEBB, 0x3F6FBEFC,
    0x3F73CF3D, 0x3F77DF7E, 0x3F7BEFBF, 0x3F800000
};

static gctUINT32 _zero2one_8bit[256] =
{
    0x00000000, 0x3B808081, 0x3C008081, 0x3C40C0C1,
    0x3C808081, 0x3CA0A0A1, 0x3CC0C0C1, 0x3CE0E0E1,
    0x3D008081, 0x3D109091, 0x3D20A0A1, 0x3D30B0B1,
    0x3D40C0C1, 0x3D50D0D1, 0x3D60E0E1, 0x3D70F0F1,
    0x3D808081, 0x3D888889, 0x3D909091, 0x3D989899,
    0x3DA0A0A1, 0x3DA8A8A9, 0x3DB0B0B1, 0x3DB8B8B9,
    0x3DC0C0C1, 0x3DC8C8C9, 0x3DD0D0D1, 0x3DD8D8D9,
    0x3DE0E0E1, 0x3DE8E8E9, 0x3DF0F0F1, 0x3DF8F8F9,
    0x3E008081, 0x3E048485, 0x3E088889, 0x3E0C8C8D,
    0x3E109091, 0x3E149495, 0x3E189899, 0x3E1C9C9D,
    0x3E20A0A1, 0x3E24A4A5, 0x3E28A8A9, 0x3E2CACAD,
    0x3E30B0B1, 0x3E34B4B5, 0x3E38B8B9, 0x3E3CBCBD,
    0x3E40C0C1, 0x3E44C4C5, 0x3E48C8C9, 0x3E4CCCCD,
    0x3E50D0D1, 0x3E54D4D5, 0x3E58D8D9, 0x3E5CDCDD,
    0x3E60E0E1, 0x3E64E4E5, 0x3E68E8E9, 0x3E6CECED,
    0x3E70F0F1, 0x3E74F4F5, 0x3E78F8F9, 0x3E7CFCFD,
    0x3E808081, 0x3E828283, 0x3E848485, 0x3E868687,
    0x3E888889, 0x3E8A8A8B, 0x3E8C8C8D, 0x3E8E8E8F,
    0x3E909091, 0x3E929293, 0x3E949495, 0x3E969697,
    0x3E989899, 0x3E9A9A9B, 0x3E9C9C9D, 0x3E9E9E9F,
    0x3EA0A0A1, 0x3EA2A2A3, 0x3EA4A4A5, 0x3EA6A6A7,
    0x3EA8A8A9, 0x3EAAAAAB, 0x3EACACAD, 0x3EAEAEAF,
    0x3EB0B0B1, 0x3EB2B2B3, 0x3EB4B4B5, 0x3EB6B6B7,
    0x3EB8B8B9, 0x3EBABABB, 0x3EBCBCBD, 0x3EBEBEBF,
    0x3EC0C0C1, 0x3EC2C2C3, 0x3EC4C4C5, 0x3EC6C6C7,
    0x3EC8C8C9, 0x3ECACACB, 0x3ECCCCCD, 0x3ECECECF,
    0x3ED0D0D1, 0x3ED2D2D3, 0x3ED4D4D5, 0x3ED6D6D7,
    0x3ED8D8D9, 0x3EDADADB, 0x3EDCDCDD, 0x3EDEDEDF,
    0x3EE0E0E1, 0x3EE2E2E3, 0x3EE4E4E5, 0x3EE6E6E7,
    0x3EE8E8E9, 0x3EEAEAEB, 0x3EECECED, 0x3EEEEEEF,
    0x3EF0F0F1, 0x3EF2F2F3, 0x3EF4F4F5, 0x3EF6F6F7,
    0x3EF8F8F9, 0x3EFAFAFB, 0x3EFCFCFD, 0x3EFEFEFF,
    0x3F008081, 0x3F018182, 0x3F028283, 0x3F038384,
    0x3F048485, 0x3F058586, 0x3F068687, 0x3F078788,
    0x3F088889, 0x3F09898A, 0x3F0A8A8B, 0x3F0B8B8C,
    0x3F0C8C8D, 0x3F0D8D8E, 0x3F0E8E8F, 0x3F0F8F90,
    0x3F109091, 0x3F119192, 0x3F129293, 0x3F139394,
    0x3F149495, 0x3F159596, 0x3F169697, 0x3F179798,
    0x3F189899, 0x3F19999A, 0x3F1A9A9B, 0x3F1B9B9C,
    0x3F1C9C9D, 0x3F1D9D9E, 0x3F1E9E9F, 0x3F1F9FA0,
    0x3F20A0A1, 0x3F21A1A2, 0x3F22A2A3, 0x3F23A3A4,
    0x3F24A4A5, 0x3F25A5A6, 0x3F26A6A7, 0x3F27A7A8,
    0x3F28A8A9, 0x3F29A9AA, 0x3F2AAAAB, 0x3F2BABAC,
    0x3F2CACAD, 0x3F2DADAE, 0x3F2EAEAF, 0x3F2FAFB0,
    0x3F30B0B1, 0x3F31B1B2, 0x3F32B2B3, 0x3F33B3B4,
    0x3F34B4B5, 0x3F35B5B6, 0x3F36B6B7, 0x3F37B7B8,
    0x3F38B8B9, 0x3F39B9BA, 0x3F3ABABB, 0x3F3BBBBC,
    0x3F3CBCBD, 0x3F3DBDBE, 0x3F3EBEBF, 0x3F3FBFC0,
    0x3F40C0C1, 0x3F41C1C2, 0x3F42C2C3, 0x3F43C3C4,
    0x3F44C4C5, 0x3F45C5C6, 0x3F46C6C7, 0x3F47C7C8,
    0x3F48C8C9, 0x3F49C9CA, 0x3F4ACACB, 0x3F4BCBCC,
    0x3F4CCCCD, 0x3F4DCDCE, 0x3F4ECECF, 0x3F4FCFD0,
    0x3F50D0D1, 0x3F51D1D2, 0x3F52D2D3, 0x3F53D3D4,
    0x3F54D4D5, 0x3F55D5D6, 0x3F56D6D7, 0x3F57D7D8,
    0x3F58D8D9, 0x3F59D9DA, 0x3F5ADADB, 0x3F5BDBDC,
    0x3F5CDCDD, 0x3F5DDDDE, 0x3F5EDEDF, 0x3F5FDFE0,
    0x3F60E0E1, 0x3F61E1E2, 0x3F62E2E3, 0x3F63E3E4,
    0x3F64E4E5, 0x3F65E5E6, 0x3F66E6E7, 0x3F67E7E8,
    0x3F68E8E9, 0x3F69E9EA, 0x3F6AEAEB, 0x3F6BEBEC,
    0x3F6CECED, 0x3F6DEDEE, 0x3F6EEEEF, 0x3F6FEFF0,
    0x3F70F0F1, 0x3F71F1F2, 0x3F72F2F3, 0x3F73F3F4,
    0x3F74F4F5, 0x3F75F5F6, 0x3F76F6F7, 0x3F77F7F8,
    0x3F78F8F9, 0x3F79F9FA, 0x3F7AFAFB, 0x3F7BFBFC,
    0x3F7CFCFD, 0x3F7DFDFE, 0x3F7EFEFF, 0x3F800000
};


/*******************************************************************************
**
** Color conversion look-up tables:
**   1. Convert to [0..1] range.
**   2. Convert non-linear to linear color space.
**
** The tables are in 32-bit floating point binary format.
*/

static gctUINT32 _zero2one_s2l_4bit[16] =
{
    0x00000000, 0x3BB99AFD, 0x3C83DDA0, 0x3D082D9A,
    0x3D6D820D, 0x3DBA7AD3, 0x3E084D51, 0x3E3D2A7A,
    0x3E7C620F, 0x3EA33B5B, 0x3ECDF13C, 0x3EFE8C10,
    0x3F1AA10D, 0x3F39237C, 0x3F5AE5F1, 0x3F7FFFFE
};

static gctUINT32 _zero2one_s2l_5bit[32] =
{
    0x00000000, 0x3B23A095, 0x3BB1DDDB, 0x3C1D66C4,
    0x3C798D13, 0x3CB7A3E3, 0x3D0012ED, 0x3D2B89FF,
    0x3D5E9370, 0x3D8CC2AF, 0x3DAE5855, 0x3DD430EE,
    0x3DFE70D3, 0x3E169D52, 0x3E3057C9, 0x3E4C77BA,
    0x3E6B0C7A, 0x3E861268, 0x3E97E781, 0x3EAB0C78,
    0x3EBF8800, 0x3ED5609F, 0x3EEC9CAA, 0x3F02A12A,
    0x3F0FABCF, 0x3F1D7136, 0x3F2BF43B, 0x3F3B37B4,
    0x3F4B3E57, 0x3F5C0ADB, 0x3F6D9FDF, 0x3F7FFFFE
};

static gctUINT32 _zero2one_s2l_6bit[64] =
{
    0x00000000, 0x3AA107B0, 0x3B2107B0, 0x3B773B6C,
    0x3BAE3FB8, 0x3BEB475D, 0x3C199EB9, 0x3C4350B5,
    0x3C72F572, 0x3C9462B0, 0x3CB27B0F, 0x3CD3DD59,
    0x3CF8A1FD, 0x3D107033, 0x3D265791, 0x3D3E11F5,
    0x3D57A9DF, 0x3D732979, 0x3D884D4D, 0x3D98036D,
    0x3DA8BBC2, 0x3DBA7AD3, 0x3DCD450E, 0x3DE11EBE,
    0x3DF60C1D, 0x3E0608A3, 0x3E11991F, 0x3E1DB97D,
    0x3E2A6BAB, 0x3E37B18C, 0x3E458CFD, 0x3E53FFD4,
    0x3E630BDA, 0x3E72B2CC, 0x3E817B35, 0x3E89EC33,
    0x3E92AD3A, 0x3E9BBF18, 0x3EA522A0, 0x3EAED8A1,
    0x3EB8E1E0, 0x3EC33F27, 0x3ECDF13C, 0x3ED8F8DD,
    0x3EE456CB, 0x3EF00BC7, 0x3EFC1883, 0x3F043EDE,
    0x3F0A9E16, 0x3F112A40, 0x3F17E3B5, 0x3F1ECACF,
    0x3F25DFE5, 0x3F2D234A, 0x3F349555, 0x3F3C365B,
    0x3F4406AD, 0x3F4C069E, 0x3F543682, 0x3F5C96A6,
    0x3F65275C, 0x3F6DE8F5, 0x3F76DBBB, 0x3F7FFFFE
};

static gctUINT32 _zero2one_s2l_8bit[256] =
{
    0x00000000, 0x399F22B4, 0x3A1F22B4, 0x3A6EB40E,
    0x3A9F22B4, 0x3AC6EB61, 0x3AEEB40E, 0x3B0B3E5D,
    0x3B1F22B4, 0x3B33070B, 0x3B46EB61, 0x3B5E3F38,
    0x3B74061F, 0x3B857FA6, 0x3B9197B3, 0x3B9E4D79,
    0x3BABA32E, 0x3BB99AFD, 0x3BC83709, 0x3BD77968,
    0x3BE76424, 0x3BF7F946, 0x3C049D60, 0x3C0D9544,
    0x3C16E545, 0x3C208E50, 0x3C2A9155, 0x3C34EF3F,
    0x3C3FA8EF, 0x3C4ABF4A, 0x3C563332, 0x3C62057F,
    0x3C6E370E, 0x3C7AC8B2, 0x3C83DDA0, 0x3C8A87C7,
    0x3C916330, 0x3C987045, 0x3C9FAF6C, 0x3CA72103,
    0x3CAEC571, 0x3CB69D18, 0x3CBEA854, 0x3CC6E786,
    0x3CCF5B10, 0x3CD8034A, 0x3CE0E092, 0x3CE9F346,
    0x3CF33BBC, 0x3CFCBA4F, 0x3D0337AE, 0x3D082D9A,
    0x3D0D3F19, 0x3D126C57, 0x3D17B57D, 0x3D1D1AB5,
    0x3D229C2A, 0x3D283A05, 0x3D2DF470, 0x3D33CB94,
    0x3D39BF9C, 0x3D3FD0AA, 0x3D45FEEA, 0x3D4C4A82,
    0x3D52B39F, 0x3D593A5F, 0x3D5FDEED, 0x3D66A170,
    0x3D6D820D, 0x3D7480EB, 0x3D7B9E32, 0x3D816D00,
    0x3D851A40, 0x3D88D6EA, 0x3D8CA310, 0x3D907EC6,
    0x3D946A1E, 0x3D986528, 0x3D9C6FF5, 0x3DA08A99,
    0x3DA4B525, 0x3DA8EFAA, 0x3DAD3A3B, 0x3DB194E6,
    0x3DB5FFBE, 0x3DBA7AD3, 0x3DBF0637, 0x3DC3A1FB,
    0x3DC84E31, 0x3DCD0AE5, 0x3DD1D82A, 0x3DD6B611,
    0x3DDBA4AA, 0x3DE0A405, 0x3DE5B435, 0x3DEAD545,
    0x3DF00746, 0x3DF54A4A, 0x3DFA9E5F, 0x3E0001CB,
    0x3E02BD01, 0x3E0580D5, 0x3E084D51, 0x3E0B227D,
    0x3E0E0060, 0x3E10E703, 0x3E13D66D, 0x3E16CEA3,
    0x3E19CFB0, 0x3E1CD999, 0x3E1FEC66, 0x3E23081F,
    0x3E262CCD, 0x3E295A73, 0x3E2C911C, 0x3E2FD0CD,
    0x3E33198E, 0x3E366B66, 0x3E39C65E, 0x3E3D2A7A,
    0x3E4097C3, 0x3E440E41, 0x3E478DF6, 0x3E4B16F0,
    0x3E4EA92F, 0x3E5244C1, 0x3E55E9A6, 0x3E5997EB,
    0x3E5D4F95, 0x3E6110A9, 0x3E64DB2B, 0x3E68AF29,
    0x3E6C8CA3, 0x3E7073A6, 0x3E746435, 0x3E785E53,
    0x3E7C620F, 0x3E8037B3, 0x3E824335, 0x3E84538D,
    0x3E8668BC, 0x3E8882CB, 0x3E8AA1B8, 0x3E8CC58B,
    0x3E8EEE43, 0x3E911BE6, 0x3E934E77, 0x3E9585F5,
    0x3E97C269, 0x3E9A03D2, 0x3E9C4A36, 0x3E9E9598,
    0x3EA0E5F6, 0x3EA33B5B, 0x3EA595C3, 0x3EA7F536,
    0x3EAA59B3, 0x3EACC342, 0x3EAF31E3, 0x3EB1A597,
    0x3EB41E66, 0x3EB69C4E, 0x3EB91F56, 0x3EBBA780,
    0x3EBE34CC, 0x3EC0C742, 0x3EC35EDF, 0x3EC5FBAC,
    0x3EC89DA6, 0x3ECB44D6, 0x3ECDF13C, 0x3ED0A2D9,
    0x3ED359B3, 0x3ED615CA, 0x3ED8D725, 0x3EDB9DC4,
    0x3EDE69A8, 0x3EE13AD9, 0x3EE41154, 0x3EE6ED21,
    0x3EE9CE3F, 0x3EECB4B4, 0x3EEFA082, 0x3EF291A8,
    0x3EF5882F, 0x3EF88414, 0x3EFB855F, 0x3EFE8C10,
    0x3F00CC13, 0x3F0254D6, 0x3F03E04F, 0x3F056E81,
    0x3F06FF6C, 0x3F089314, 0x3F0A2979, 0x3F0BC29A,
    0x3F0D5E7B, 0x3F0EFD1C, 0x3F109E81, 0x3F1242A9,
    0x3F13E994, 0x3F159347, 0x3F173FBF, 0x3F18EF02,
    0x3F1AA10D, 0x3F1C55E5, 0x3F1E0D8A, 0x3F1FC7FA,
    0x3F21853C, 0x3F23454D, 0x3F250831, 0x3F26CDE8,
    0x3F289672, 0x3F2A61D3, 0x3F2C300A, 0x3F2E011A,
    0x3F2FD502, 0x3F31ABC7, 0x3F338568, 0x3F3561E5,
    0x3F374141, 0x3F39237C, 0x3F3B089A, 0x3F3CF09A,
    0x3F3EDB7C, 0x3F40C944, 0x3F42B9F1, 0x3F44AD87,
    0x3F46A404, 0x3F489D6C, 0x3F4A99BF, 0x3F4C98FD,
    0x3F4E9B2A, 0x3F50A044, 0x3F52A84F, 0x3F54B34C,
    0x3F56C139, 0x3F58D21C, 0x3F5AE5F1, 0x3F5CFCBF,
    0x3F5F1681, 0x3F61333E, 0x3F6352F5, 0x3F6575A4,
    0x3F679B51, 0x3F69C3FA, 0x3F6BEFA2, 0x3F6E1E4B,
    0x3F704FF1, 0x3F72849C, 0x3F74BC47, 0x3F76F6F9,
    0x3F7934AE, 0x3F7B756C, 0x3F7DB932, 0x3F7FFFFE
};


/*******************************************************************************
**
** Color conversion look-up tables:
**   1. Convert to [0..1] range.
**   2. Convert linear to non-linear color space.
**
** The tables are in 32-bit floating point binary format.
*/

static gctUINT32 _zero2one_l2s_5bit[32] =
{
    0x00000000, 0x3E49875D, 0x3E900949, 0x3EAFC941,
    0x3EC9CC1C, 0x3EE03BBC, 0x3EF42DA5, 0x3F0321E5,
    0x3F0B7313, 0x3F132E0B, 0x3F1A6C7C, 0x3F2141A1,
    0x3F27BC46, 0x3F2DE81A, 0x3F33CE86, 0x3F39773C,
    0x3F3EE8A4, 0x3F44281E, 0x3F493A3E, 0x3F4E22F2,
    0x3F52E5A1, 0x3F578543, 0x3F5C0474, 0x3F606582,
    0x3F64AA7B, 0x3F68D531, 0x3F6CE74C, 0x3F70E247,
    0x3F74C778, 0x3F789818, 0x3F7C5544, 0x3F800000
};

static gctUINT32 _zero2one_l2s_6bit[64] =
{
    0x00000000, 0x3E07685A, 0x3E47CFB7, 0x3E7710FD,
    0x3E8EE3DA, 0x3E9F961E, 0x3EAE6DD1, 0x3EBBE375,
    0x3EC8446D, 0x3ED3C5D2, 0x3EDE8DE3, 0x3EE8B93A,
    0x3EF25DE0, 0x3EFB8D37, 0x3F022AA0, 0x3F0660B9,
    0x3F0A6DA9, 0x3F0E5571, 0x3F121B79, 0x3F15C2AF,
    0x3F194D98, 0x3F1CBE6D, 0x3F20171E, 0x3F235961,
    0x3F2686BD, 0x3F29A090, 0x3F2CA812, 0x3F2F9E5F,
    0x3F328475, 0x3F355B3D, 0x3F38238B, 0x3F3ADE20,
    0x3F3D8BB0, 0x3F402CDC, 0x3F42C23C, 0x3F454C5C,
    0x3F47CBBD, 0x3F4A40D5, 0x3F4CAC16, 0x3F4F0DE9,
    0x3F5166AD, 0x3F53B6BE, 0x3F55FE72, 0x3F583E18,
    0x3F5A75FC, 0x3F5CA666, 0x3F5ECF98, 0x3F60F1D1,
    0x3F630D4D, 0x3F652245, 0x3F6730EE, 0x3F69397B,
    0x3F6B3C1C, 0x3F6D38FF, 0x3F6F3050, 0x3F712239,
    0x3F730EE1, 0x3F74F66E, 0x3F76D904, 0x3F78B6C6,
    0x3F7A8FD5, 0x3F7C644F, 0x3F7E3455, 0x3F800000
};

static gctUINT32 _zero2one_l2s_8bit[256] =
{
    0x00000000, 0x3D49EE84, 0x3DACE6AC, 0x3DE1AE9B,
    0x3E067625, 0x3E191C3F, 0x3E29B049, 0x3E38B8DB,
    0x3E468C6A, 0x3E53663E, 0x3E5F7112, 0x3E6ACCDA,
    0x3E75922E, 0x3E7FD47D, 0x3E84D1B5, 0x3E8985E1,
    0x3E8E0C13, 0x3E9268C5, 0x3E969FC5, 0x3E9AB458,
    0x3E9EA951, 0x3EA28127, 0x3EA63E05, 0x3EA9E1D8,
    0x3EAD6E52, 0x3EB0E4F9, 0x3EB4472C, 0x3EB79626,
    0x3EBAD303, 0x3EBDFEC9, 0x3EC11A63, 0x3EC426AA,
    0x3EC72465, 0x3ECA144A, 0x3ECCF702, 0x3ECFCD27,
    0x3ED2974D, 0x3ED555F8, 0x3ED809A3, 0x3EDAB2C5,
    0x3EDD51CA, 0x3EDFE718, 0x3EE2730E, 0x3EE4F606,
    0x3EE77053, 0x3EE9E246, 0x3EEC4C29, 0x3EEEAE41,
    0x3EF108D4, 0x3EF35C1E, 0x3EF5A85E, 0x3EF7EDCC,
    0x3EFA2C9A, 0x3EFC6501, 0x3EFE972D, 0x3F0061A8,
    0x3F0174CB, 0x3F028512, 0x3F039295, 0x3F049D65,
    0x3F05A595, 0x3F06AB37, 0x3F07AE5B, 0x3F08AF13,
    0x3F09AD6B, 0x3F0AA976, 0x3F0BA340, 0x3F0C9AD8,
    0x3F0D9049, 0x3F0E83A2, 0x3F0F74EE, 0x3F10643B,
    0x3F115191, 0x3F123CFD, 0x3F132689, 0x3F140E41,
    0x3F14F42C, 0x3F15D856, 0x3F16BAC7, 0x3F179B88,
    0x3F187AA1, 0x3F19581B, 0x3F1A33FF, 0x3F1B0E53,
    0x3F1BE720, 0x3F1CBE6D, 0x3F1D9442, 0x3F1E68A3,
    0x3F1F3B9A, 0x3F200D2C, 0x3F20DD5F, 0x3F21AC3A,
    0x3F2279C3, 0x3F2345FF, 0x3F2410F3, 0x3F24DAA7,
    0x3F25A31F, 0x3F266A60, 0x3F27306E, 0x3F27F550,
    0x3F28B90A, 0x3F297BA0, 0x3F2A3D18, 0x3F2AFD75,
    0x3F2BBCBC, 0x3F2C7AF1, 0x3F2D3818, 0x3F2DF436,
    0x3F2EAF4D, 0x3F2F6962, 0x3F30227A, 0x3F30DA97,
    0x3F3191BC, 0x3F3247EF, 0x3F32FD31, 0x3F33B186,
    0x3F3464F2, 0x3F351777, 0x3F35C918, 0x3F3679DB,
    0x3F3729BE, 0x3F37D8C9, 0x3F3886FC, 0x3F393459,
    0x3F39E0E5, 0x3F3A8CA1, 0x3F3B3791, 0x3F3BE1B7,
    0x3F3C8B15, 0x3F3D33AE, 0x3F3DDB84, 0x3F3E829A,
    0x3F3F28F2, 0x3F3FCE8E, 0x3F407370, 0x3F41179C,
    0x3F41BB11, 0x3F425DD3, 0x3F42FFE6, 0x3F43A148,
    0x3F4441FE, 0x3F44E209, 0x3F45816A, 0x3F462024,
    0x3F46BE39, 0x3F475BAB, 0x3F47F87A, 0x3F4894A9,
    0x3F49303A, 0x3F49CB2E, 0x3F4A6587, 0x3F4AFF47,
    0x3F4B9870, 0x3F4C3102, 0x3F4CC8FE, 0x3F4D6069,
    0x3F4DF742, 0x3F4E8D8A, 0x3F4F2344, 0x3F4FB871,
    0x3F504D11, 0x3F50E127, 0x3F5174B4, 0x3F5207B9,
    0x3F529A39, 0x3F532C32, 0x3F53BDA9, 0x3F544E9C,
    0x3F54DF0E, 0x3F556EFF, 0x3F55FE72, 0x3F568D66,
    0x3F571BDE, 0x3F57A9DC, 0x3F58375E, 0x3F58C467,
    0x3F5950F8, 0x3F59DD12, 0x3F5A68B6, 0x3F5AF3E5,
    0x3F5B7EA1, 0x3F5C08E8, 0x3F5C92BF, 0x3F5D1C24,
    0x3F5DA519, 0x3F5E2DA0, 0x3F5EB5B8, 0x3F5F3D63,
    0x3F5FC4A2, 0x3F604B76, 0x3F60D1E0, 0x3F6157DF,
    0x3F61DD77, 0x3F6262A7, 0x3F62E770, 0x3F636BD2,
    0x3F63EFD0, 0x3F647368, 0x3F64F69F, 0x3F657970,
    0x3F65FBE1, 0x3F667DF1, 0x3F66FFA0, 0x3F6780EE,
    0x3F6801DF, 0x3F688270, 0x3F6902A5, 0x3F69827C,
    0x3F6A01F8, 0x3F6A8118, 0x3F6AFFDD, 0x3F6B7E49,
    0x3F6BFC5B, 0x3F6C7A14, 0x3F6CF775, 0x3F6D747F,
    0x3F6DF132, 0x3F6E6D8F, 0x3F6EE997, 0x3F6F654A,
    0x3F6FE0A8, 0x3F705BB3, 0x3F70D66B, 0x3F7150D0,
    0x3F71CAE3, 0x3F7244A6, 0x3F72BE18, 0x3F733739,
    0x3F73B00B, 0x3F74288D, 0x3F74A0C2, 0x3F7518A9,
    0x3F759041, 0x3F76078D, 0x3F767E8C, 0x3F76F541,
    0x3F776BAA, 0x3F77E1C7, 0x3F78579A, 0x3F78CD24,
    0x3F794264, 0x3F79B75B, 0x3F7A2C0A, 0x3F7AA071,
    0x3F7B1491, 0x3F7B8869, 0x3F7BFBFB, 0x3F7C6F47,
    0x3F7CE24E, 0x3F7D550E, 0x3F7DC78B, 0x3F7E39C3,
    0x3F7EABB7, 0x3F7F1D68, 0x3F7F8ED6, 0x3F800000
};
